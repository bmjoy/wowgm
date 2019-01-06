#include <shared/filesystem/disk_file_system.hpp>
#include <shared/assert/assert.hpp>

#include <stdexcept>
#include <boost/filesystem.hpp>

#include <windows.h>

// TODO: Throw proper exceptions here. Stormlib provides a shim GetlastError/SetLastError even on non-windows systems
namespace shared::filesystem
{
    disk_file_system::~disk_file_system()
    {
    }

    void disk_file_system::Initialize(const std::string& rootFolder)
    {
        _rootFolder = rootFolder;
    }

    std::shared_ptr<disk_file> disk_file_system::OpenFile(const std::string& relFilePath)
    {
        boost::filesystem::path filePath(_rootFolder);
        boost::filesystem::path relPath(relFilePath);
        if (!relPath.is_absolute())
            filePath /= relPath;
        else
            filePath = relPath;

        return std::shared_ptr<disk_file>(new disk_file(filePath.string(), false));
    }

    bool disk_file_system::FileExists(const std::string& relFilePath) const
    {
        boost::filesystem::path r(_rootFolder);
        boost::filesystem::path p(relFilePath);
        if (p.is_absolute())
            r = p;
        else
            r /= p;

        return boost::filesystem::is_regular_file(r);
    }

    disk_file::disk_file(const std::string& fileName, bool writable) : _mapped(nullptr)
    {
#if PLATFORM == PLATFORM_WINDOWS
        if (_fileHandle == INVALID_HANDLE_VALUE)
            _fileHandle = CreateFileA(fileName.c_str(), GENERIC_READ, writable ? FILE_SHARE_WRITE : FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

        if (_fileHandle == INVALID_HANDLE_VALUE)
            return;

        DWORD dwFileSizeHigh;
        _fileSize = ::GetFileSize(_fileHandle, &dwFileSizeHigh);
        _fileSize += (((size_t)dwFileSizeHigh) << 32);

        _mapFile = CreateFileMappingA(_fileHandle, NULL, writable ? PAGE_READWRITE : PAGE_READONLY, 0, 0, NULL);
        if (_mapFile == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Failed to create file mapping");

        _mapped = static_cast<uint8_t*>(MapViewOfFile(_mapFile, writable ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0));
#elif (PLATFORM == PLATFORM_UNIX) || (PLATFORM == PLATFORM_APPLE)

        if (writable)
            throw std::runtime_error("Sorry, I don't know how to map writable on nix");

        _fileDescriptor = open(fileName.c_str(), O_RDONLY, 0);
        BOOST_ASSERT_MSG(_fileDescriptor != -1, "Failed to open the file");

        struct stat st;
        ::stat(fileName.c_str(), &str);

        _fileSize = st.st_size;

        _mapped = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, _fileDescriptor, 0);
        BOOST_ASSERT_MSG(_map != MAP_FAILED, "Failed to map the file to memory");
#endif
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

        memcpy(buffer, _mapped + offset, availableDataLength);

        return availableDataLength;
    }

    uint8_t const* disk_file::GetData()
    {
        return reinterpret_cast<uint8_t*>(_mapped);
    }

    void disk_file::Close()
    {
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
