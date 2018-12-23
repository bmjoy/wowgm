#include <shared/filesystem/mpq_file_system.hpp>
#include <shared/assert/assert.hpp>

#include <stdexcept>
#include <boost/filesystem.hpp>

#include <windows.h>

// TODO: Throw proper exceptions here. Stormlib provides a shim GetlastError/SetLastError even on non-windows systems
namespace shared::filesystem
{
    using tstring = std::basic_string<TCHAR>;

    void mpq_file_system::Initialize(const std::string& rootFolder)
    {
        if (rootFolder.length() == 0)
            return;

        if (rootFolder == _currentRootFolder)
            return;

        _currentRootFolder = rootFolder;

        for (HANDLE archiveHandle : _archiveHandles)
            SFileCloseArchive(archiveHandle);

        try {
            boost::filesystem::path rootPath = rootFolder;
            auto dataPath = rootPath / "Data" / GetLocaleString();

            _archiveHandles.clear();

            boost::filesystem::directory_iterator end;
            for (boost::filesystem::directory_iterator itr(dataPath); itr != end; ++itr)
            {
                if (boost::filesystem::is_directory(itr->path()))
                    continue;

                if (boost::filesystem::extension(itr->path()) != ".MPQ")
                    continue;

                HANDLE fileHandle;
                if (SFileOpenArchive(itr->path().string<tstring>().c_str(), 0, MPQ_OPEN_READ_ONLY, &fileHandle))
                    _archiveHandles.push_back(fileHandle);
                else
                    shared::exceptions::throw_with_trace<std::runtime_error>("Error loading archive.");
            }
        }
        catch (const std::exception& e) {
            return; // Check for more specific exceptions layer
        }
    }

    mpq_file_system::~mpq_file_system()
    {
        for (HANDLE archiveHandle : _archiveHandles)
            SFileCloseArchive(archiveHandle);

        _archiveHandles.clear();
    }

    std::shared_ptr<file_handle<mpq_file>> mpq_file_system::OpenFile(const std::string& filePath, LoadStrategy loadStrategy)
    {
        for (HANDLE archiveHandle : _archiveHandles)
        {
            HANDLE fileHandle;
            if (SFileOpenFileEx(archiveHandle, filePath.c_str(), SFILE_OPEN_PATCHED_FILE, &fileHandle))
                return std::shared_ptr<mpq_file>(new mpq_file(fileHandle, loadStrategy));
        }

        return { };
    }

    bool mpq_file_system::FileExists(const std::string& relFilePath) const
    {
        for (HANDLE archiveHandle : _archiveHandles)
        {
            HANDLE fileHandle;
            if (SFileOpenFileEx(archiveHandle, relFilePath.c_str(), SFILE_OPEN_PATCHED_FILE, &fileHandle))
            {
                SFileCloseFile(archiveHandle);
                return true;
            }
        }

        return false;
    }

    bool mpq_file_system::FileExists(const std::string& relFilePath, const std::string& root) const
    {
        return false;
    }

    mpq_file::mpq_file(HANDLE fileHandle, LoadStrategy loadStrategy) : _loadStrategy(loadStrategy)
    {
        _fileHandle = fileHandle;

        // Don't load to memory if the file is loaded as memory-mapped
        if (loadStrategy == LoadStrategy::Mapped)
            return;

        _fileData.resize(GetFileSize());
        if (!SFileReadFile(_fileHandle, _fileData.data(), GetFileSize()))
            shared::exceptions::throw_with_trace<std::runtime_error>("Unable to read file");

        // Immediately close the handle, but don't call Close() - this would clear the buffer
        SFileCloseFile(_fileHandle);
        _fileHandle = nullptr;
    }

    mpq_file::~mpq_file()
    {
        Close();
    }

    void mpq_file::Close()
    {
        _fileData.clear();

        if (_fileHandle == nullptr)
            return;

        SFileCloseFile(_fileHandle);
        _fileHandle = nullptr;
    }

    size_t mpq_file::GetFileSize() const
    {
        return SFileGetFileSize(_fileHandle);
    }

    LoadStrategy mpq_file::GetLoadStrategy() const
    {
        return _loadStrategy;
    }

    uint8_t const* mpq_file::GetData()
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

    size_t mpq_file::ReadBytes(size_t offset, size_t length, uint8_t* buffer, size_t bufferSize)
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
}
