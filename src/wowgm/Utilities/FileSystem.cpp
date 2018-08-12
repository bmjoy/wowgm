#include "FileSystem.hpp"
#include "Assert.hpp"

#include <boost/filesystem.hpp>

namespace wowgm::filesystem
{
    using tstring = std::basic_string<TCHAR>;

    void MpqFileSystem::Initialize(const std::string& rootFolder)
    {
        boost::filesystem::path rootPath = rootFolder;
        auto dataPath = rootPath / "Data" / GetLocaleString();

        boost::filesystem::directory_iterator end;
        for (boost::filesystem::directory_iterator itr(dataPath); itr != end; ++itr)
        {
            if (!boost::filesystem::is_directory(itr->path()))
                continue;

            if (boost::filesystem::extension(itr->path()) != ".MPQ")
                continue;

            HANDLE fileHandle;
            if (SFileOpenArchive(itr->path().string<tstring>().c_str(), 0, MPQ_OPEN_READ_ONLY, &fileHandle))
                _archiveHandles.push_back(fileHandle);
            else
                wowgm::exceptions::throw_with_trace<std::runtime_error>("Error loading archive.");
        }
    }

    MpqFileSystem::~MpqFileSystem()
    {
        for (HANDLE archiveHandle : _archiveHandles)
            SFileCloseArchive(archiveHandle);

        _archiveHandles.clear();
    }

    std::shared_ptr<FileHandle<MpqFile>> MpqFileSystem::OpenFile(const std::string& filePath)
    {
        for (HANDLE archiveHandle : _archiveHandles)
        {
            HANDLE fileHandle;
            if (SFileOpenFileEx(archiveHandle, filePath.c_str(), SFILE_OPEN_PATCHED_FILE, &fileHandle))
                return std::shared_ptr<MpqFile>(new MpqFile(fileHandle));
        }

        return { };
    }

    MpqFile::MpqFile(HANDLE fileHandle)
    {
        _fileHandle = fileHandle;
    }

    MpqFile::~MpqFile()
    {
        Close();
    }

    void MpqFile::Close()
    {
        SFileCloseFile(_fileHandle);
        _fileHandle = nullptr;
    }

    size_t MpqFile::GetFileSize() const
    {
        return SFileGetFileSize(_fileHandle);
    }

    DiskFileSystem::~DiskFileSystem()
    {

    }

    void DiskFileSystem::Initialize(const std::string& rootFolder)
    {
        _rootFolder = rootFolder;
    }

    std::shared_ptr<FileHandle<DiskFile>> DiskFileSystem::OpenFile(const std::string& relFilePath)
    {
        boost::filesystem::path filePath = _rootFolder;
        filePath /= relFilePath;

        return std::shared_ptr<DiskFile>(new DiskFile(filePath.string()));
    }

    DiskFile::DiskFile(const std::string& fileName)
    {
#if PLATFORM == PLATFORM_WINDOWS
        _fileHandle = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (_fileHandle == INVALID_HANDLE_VALUE)
            return;

        DWORD dwFileSizeHigh;
        _fileSize = ::GetFileSize(_fileHandle, &dwFileSizeHigh);
        _fileSize += (((size_t)dwFileSizeHigh) << 32);

        _mapFile = CreateFileMappingA(_fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
        if (_mapFile == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Failed to create file mapping");

        _mapped = static_cast<uint8_t*>(MapViewOfFile(_mapFile, FILE_MAP_READ, 0, 0, 0));
#elif (PLATFORM == PLATFORM_UNIX) || (PLATFORM == PLATFORM_APPLE)
        _fileDescriptor = open(fileName.c_str(), O_RDONLY, 0);
        BOOST_ASSERT_MSG(_fileDescriptor != -1, "Failed to open the file");

        struct stat st;
        ::stat(fileName.c_str(), &str);

        _fileSize = st.st_size;

        _map = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, _fileDescriptor, 0);
        BOOST_ASSERT_MSG(_map != MAP_FAILED, "Failed to map the file to memory");
#else
        std::ifstream fs(fileName, std::ios::binary);
        fs.unsetf(std::ios::skipws);

        fs.seekg(0, std::ios::end);
        _fileSize = file.tellg();
        fs.seekg(0, std::ios::beg);

        _data.resize(_fileSize);
        fs.read(&_data[0], fileSize);
#endif
    }

    DiskFile::~DiskFile()
    {
        Close();
    }

    size_t DiskFile::GetFileSize() const
    {
        return _fileSize;
    }

    void DiskFile::Close()
    {
#if PLATFORM == PLATFORM_WINDOWS
        UnmapViewOfFile(_mapped);
        CloseHandle(_mapFile);
        CloseHandle(_fileHandle);

        _mapped = nullptr;
        _mapFile = INVALID_HANDLE_VALUE;
        _fileHandle = INVALID_HANDLE_VALUE;
#elif PLATFORM == PLATFORM_UNIX || PLATFORM == PLATFORM_APPLE
        BOOST_ASSERT_MSG(munmap(_fileDescriptor, _fileSize) == 0, "Failed to unmap file from memory");
        close(_fileDescriptor);
#else
        _data.clear();
#endif
    }
}
