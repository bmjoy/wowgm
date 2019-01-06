#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include <shared/filesystem/file_system.hpp>
#include <shared/defines.hpp>

#include <windows.h>

#if (PLATFORM == PLATFORM_UNIX) || (PLATFORM == PLATFORM_APPLE)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#endif

namespace shared::filesystem
{
    class disk_file_system;

    class disk_file : public file_handle<disk_file>
    {
        friend class disk_file_system;

        disk_file(const std::string& fileName, bool writable);

    public:
        ~disk_file();

        void Close() override;
        size_t GetFileSize() const override;
        uint8_t const* GetData() override;
        size_t ReadBytes(size_t offset, size_t length, uint8_t* buffer, size_t bufferSize) override;

    private:
        size_t _fileSize = 0;

#if PLATFORM == PLATFORM_WINDOWS
        HANDLE _fileHandle = INVALID_HANDLE_VALUE;
        HANDLE _mapFile = INVALID_HANDLE_VALUE;
        uint8_t* _mapped{ nullptr };
#elif (PLATFORM == PLATFORM_APPLE) || (PLATFORM == PLATFORM_UNIX)
        int _fileDescriptor;
        void* _mapped;
#endif
    };

    class disk_file_system final : public file_system<disk_file>
    {
    public:
        static disk_file_system* Instance()
        {
            static disk_file_system instance;

            return &instance;
        }

        ~disk_file_system();

        void Initialize(const std::string& rootFolder) override;
        std::shared_ptr<disk_file> OpenFile(const std::string& relFilePath) override;
        bool FileExists(const std::string& relFilePath) const override;

    private:
        std::string _rootFolder;
    };

}
