#include <shared/filesystem/disk_file_system.hpp>
#include <shared/assert/assert.hpp>

#include <stdexcept>
#include <boost/filesystem.hpp>

#include <windows.h>

// TODO: Throw proper exceptions here. Stormlib provides a shim GetlastError/SetLastError even on non-windows systems
namespace shared::filesystem
{
    using tstring = std::basic_string<TCHAR>;

    disk_file_system::~disk_file_system()
    {

    }

    void disk_file_system::Initialize(const std::string& rootFolder)
    {
        _rootFolder = rootFolder;
    }

    std::shared_ptr<disk_file> disk_file_system::OpenFile(const std::string& relFilePath, LoadStrategy loadStrategy)
    {
        boost::filesystem::path filePath = _rootFolder;
        filePath /= relFilePath;

        return OpenDirectFile(filePath.string(), loadStrategy);
    }

    std::shared_ptr<disk_file> disk_file_system::OpenDirectFile(const std::string& filePath, LoadStrategy loadStrategy)
    {
        return std::shared_ptr<disk_file>(new disk_file(filePath, loadStrategy));
    }

    bool disk_file_system::FileExists(const std::string& relFilePath) const
    {
        boost::filesystem::path p = _rootFolder;
        p /= relFilePath;

        return boost::filesystem::is_regular_file(p);
    }

    bool disk_file_system::FileExists(const std::string& relFilePath, const std::string& root) const
    {
        boost::filesystem::path p = root;
        p /= relFilePath;

        return boost::filesystem::is_regular_file(p);
    }

    disk_file::disk_file(const std::string& fileName, LoadStrategy loadStrategy) : _loadStrategy(loadStrategy), _mapped(nullptr)
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

    void disk_file::_LoadToMemory(const std::string& fileName)
    {
        std::ifstream fs(fileName, std::ios::binary);
        fs.unsetf(std::ios::skipws);

        fs.seekg(0, std::ios::end);
        _fileSize = fs.tellg();
        fs.seekg(0, std::ios::beg);

        _fileData.resize(_fileSize);
        fs.read(reinterpret_cast<char*>(&_fileData[0]), _fileSize);
    }

    disk_file::~disk_file()
    {
        Close();
    }

    size_t disk_file::GetFileSize() const
    {
        return _fileSize;
    }

    size_t disk_file::ReadBytes(size_t offset, size_t length, uint8_t* buffer, size_t bufferSize)
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

    LoadStrategy disk_file::GetLoadStrategy() const
    {
        return _loadStrategy;
    }

    uint8_t const* disk_file::GetData()
    {
        if (GetLoadStrategy() == LoadStrategy::Memory)
            return _fileData.data();

        return reinterpret_cast<uint8_t*>(_mapped);
    }

    void disk_file::Close()
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
