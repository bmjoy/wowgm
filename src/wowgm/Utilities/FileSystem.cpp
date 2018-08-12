#include "FileSystem.hpp"
#include "Assert.hpp"

#include <boost/filesystem.hpp>

// TODO: Throw proper exceptions here. Stormlib provides a shim GetlastError/SetLastError even on non-windows systems
namespace wowgm::filesystem
{
    using tstring = std::basic_string<TCHAR>;

    void MpqFileSystem::Initialize(const std::string& rootFolder)
    {
        for (HANDLE archiveHandle : _archiveHandles)
            SFileCloseArchive(archiveHandle);

        _archiveHandles.clear();

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

    std::shared_ptr<FileHandle<MpqFile>> MpqFileSystem::OpenFile(const std::string& filePath, LoadStrategy loadStrategy)
    {
        for (HANDLE archiveHandle : _archiveHandles)
        {
            HANDLE fileHandle;
            if (SFileOpenFileEx(archiveHandle, filePath.c_str(), SFILE_OPEN_PATCHED_FILE, &fileHandle))
                return std::shared_ptr<MpqFile>(new MpqFile(fileHandle, loadStrategy));
        }

        return { };
    }

    MpqFile::MpqFile(HANDLE fileHandle, LoadStrategy loadStrategy) : _loadStrategy(loadStrategy)
    {
        _fileHandle = fileHandle;

        // Don't load to memory if the file is loaded as memory-mapped
        if (loadStrategy == LoadStrategy::Mapped)
            return;

        _fileData.resize(GetFileSize());
        if (!SFileReadFile(_fileHandle, _fileData.data(), GetFileSize()))
            wowgm::exceptions::throw_with_trace<std::runtime_error>("Unable to read file");

        // Immediately close the handle, but don't call Close() - this would clear the buffer
        SFileCloseFile(_fileHandle);
        _fileHandle = nullptr;
    }

    MpqFile::~MpqFile()
    {
        Close();
    }

    void MpqFile::Close()
    {
        _fileData.clear();

        if (_fileHandle == nullptr)
            return;

        SFileCloseFile(_fileHandle);
        _fileHandle = nullptr;
    }

    size_t MpqFile::GetFileSize() const
    {
        return SFileGetFileSize(_fileHandle);
    }

    LoadStrategy MpqFile::GetLoadStrategy() const
    {
        return _loadStrategy;
    }

    std::uint8_t const* MpqFile::GetData()
    {
        if (GetLoadStrategy() == LoadStrategy::Memory)
            return _fileData.data();

        // This is lame, but we need this
        _loadStrategy = LoadStrategy::Memory;

        _fileData.resize(GetFileSize());

        SFileSetFilePointer(_fileHandle, 0, nullptr, FILE_BEGIN);
        SFileReadFile(_fileHandle, _fileData.data(), GetFileSize());
        SFileCloseFile(_fileHandle);
        _fileHandle = nullptr;

        return _fileData.data();
    }

    size_t MpqFile::ReadBytes(size_t offset, size_t length, std::uint8_t* buffer, size_t bufferSize)
    {
        auto availableDataLength = GetFileSize() - offset;
        if (bufferSize < availableDataLength)
            availableDataLength = bufferSize;

        if (GetLoadStrategy() == LoadStrategy::Memory)
        {
            memcpy(buffer, _fileData.data() + offset, availableDataLength);
        }
        else
        {
            SFileSetFilePointer(_fileHandle, offset, nullptr, FILE_BEGIN);
            SFileReadFile(_fileHandle, buffer, availableDataLength);
        }

        return availableDataLength;
    }

    DiskFileSystem::~DiskFileSystem()
    {

    }

    void DiskFileSystem::Initialize(const std::string& rootFolder)
    {
        _rootFolder = rootFolder;
    }

    std::shared_ptr<FileHandle<DiskFile>> DiskFileSystem::OpenFile(const std::string& relFilePath, LoadStrategy loadStrategy)
    {
        boost::filesystem::path filePath = _rootFolder;
        filePath /= relFilePath;

        return std::shared_ptr<DiskFile>(new DiskFile(filePath.string(), loadStrategy));
    }

    DiskFile::DiskFile(const std::string& fileName, LoadStrategy loadStrategy) : _loadStrategy(loadStrategy), _mapped(nullptr)
    {
        if (_loadStrategy == LoadStrategy::Memory)
        {
            _LoadToMemory(fileName);
            return;
        }

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

        _mapped = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, _fileDescriptor, 0);
        BOOST_ASSERT_MSG(_map != MAP_FAILED, "Failed to map the file to memory");
#else
        _LoadToMemory(fileName);
#endif
    }

    void DiskFile::_LoadToMemory(const std::string& fileName)
    {
        std::ifstream fs(fileName, std::ios::binary);
        fs.unsetf(std::ios::skipws);

        fs.seekg(0, std::ios::end);
        _fileSize = fs.tellg();
        fs.seekg(0, std::ios::beg);

        _fileData.resize(_fileSize);
        fs.read(reinterpret_cast<char*>(&_fileData[0]), _fileSize);
    }

    DiskFile::~DiskFile()
    {
        Close();
    }

    size_t DiskFile::GetFileSize() const
    {
        return _fileSize;
    }

    size_t DiskFile::ReadBytes(size_t offset, size_t length, std::uint8_t* buffer, size_t bufferSize)
    {
        auto availableDataLength = GetFileSize() - offset;
        if (bufferSize < availableDataLength)
            availableDataLength = bufferSize;

        if (GetLoadStrategy() == LoadStrategy::Memory)
            memcpy(buffer, _fileData.data() + offset, availableDataLength);
        else
            memcpy(buffer, _mapped, availableDataLength);

        return availableDataLength;
    }

    LoadStrategy DiskFile::GetLoadStrategy() const
    {
        return _loadStrategy;
    }

    std::uint8_t const* DiskFile::GetData()
    {
        if (GetLoadStrategy() == LoadStrategy::Memory)
            return _fileData.data();

        return reinterpret_cast<std::uint8_t*>(_mapped);
    }

    void DiskFile::Close()
    {
        _fileData.clear();

#if PLATFORM == PLATFORM_WINDOWS
        if (_mapped != nullptr)
            UnmapViewOfFile(_mapped);

        if (_mapFile != INVALID_HANDLE_VALUE)
            CloseHandle(_mapFile);

        if (_fileHandle != INVALID_HANDLE_VALUE)
            CloseHandle(_fileHandle);

        _mapped = nullptr;
        _mapFile = INVALID_HANDLE_VALUE;
        _fileHandle = INVALID_HANDLE_VALUE;
#elif PLATFORM == PLATFORM_UNIX || PLATFORM == PLATFORM_APPLE
        BOOST_ASSERT_MSG(munmap(_fileDescriptor, _fileSize) == 0, "Failed to unmap file from memory");
        close(_fileDescriptor);
        _mapped = nullptr;
#endif
    }
}
