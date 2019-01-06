#include <shared/filesystem/mpq_file_system.hpp>
#include <shared/assert/assert.hpp>

#include <stdexcept>
#include <boost/filesystem.hpp>

#include <StormLib.h>

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
                    shared::assert::throw_with_trace("Error loading archive.");
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

    std::shared_ptr<mpq_file> mpq_file_system::OpenFile(const std::string& filePath)
    {
        for (HANDLE archiveHandle : _archiveHandles)
        {
            HANDLE fileHandle;
            if (SFileOpenFileEx(archiveHandle, filePath.c_str(), SFILE_OPEN_PATCHED_FILE, &fileHandle))
                return std::shared_ptr<mpq_file>(new mpq_file(fileHandle));
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

    mpq_file::mpq_file(HANDLE fileHandle)
    {
        _fileHandle = fileHandle;

        _fileData.resize(GetFileSize());
        if (!SFileReadFile(_fileHandle, _fileData.data(), GetFileSize()))
            shared::assert::throw_with_trace("Unable to read file");

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

    uint8_t const* mpq_file::GetData()
    {
        return _fileData.data();
    }

    size_t mpq_file::ReadBytes(size_t offset, size_t length, uint8_t* buffer, size_t bufferSize)
    {
        auto availableDataLength = GetFileSize() - offset;
        if (bufferSize < availableDataLength)
            availableDataLength = bufferSize;

        memcpy(buffer, _fileData.data() + offset, availableDataLength);

        return availableDataLength;
    }
}
