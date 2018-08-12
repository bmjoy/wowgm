#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include <StormLib.h>
#include "Defines.hpp"

#if (PLATFORM == PLATFORM_UNIX) || (PLATFORM == PLATFORM_APPLE)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#endif

namespace wowgm::filesystem
{
    enum class Locale
    {
        French,
        English,
        Russian
    };

    class MpqFileSystem;
    class DiskFileSystem;

    template <typename T>
    class FileHandle : public std::enable_shared_from_this<T>
    {
    protected:
        virtual ~FileHandle() { }

    public:
        virtual void Close() = 0;
        virtual size_t GetFileSize() const = 0;
    };

    class MpqFile : public FileHandle<MpqFile>
    {
        friend class MpqFileSystem;

        explicit MpqFile(HANDLE fileHandle);

    public:
        ~MpqFile();

        void Close() override;
        size_t GetFileSize() const override;

    private:
        HANDLE _fileHandle;
    };

    class DiskFile : public FileHandle<DiskFile>
    {
        friend class DiskFileSystem;

        DiskFile(const std::string& fileName);

    public:
        ~DiskFile();

        void Close() override;
        size_t GetFileSize() const override;

    private:
        size_t _fileSize = 0;
        uint8_t* _mapped{ nullptr };
#if PLATFORM == PLATFORM_WINDOWS
        HANDLE _fileHandle = INVALID_HANDLE_VALUE;
        HANDLE _mapFile = INVALID_HANDLE_VALUE;
#elif (PLATFORM == PLATFORM_APPLE) || (PLATFORM == PLATFORM_UNIX)
        int _fileDescriptor;
        void* _map;
#else
        std::vector<uint8_t> _data;
#endif
    };

    template <typename T>
    class FileSystem
    {
        virtual void Initialize(const std::string& rootFolder) = 0;
        virtual std::shared_ptr<FileHandle<T>> OpenFile(const std::string& filePath) = 0;

        void SetLocale(Locale clientLocale) { _clientLocale = clientLocale; }
        Locale GetLocale() const { return _clientLocale; }

        const char* GetLocaleString() const
        {
            switch (_clientLocale)
            {
                case Locale::French: return "frFR";
                case Locale::Russian: return "ruRU";
            }
            return "enGB";
        }

    private:
        Locale _clientLocale = Locale::English;
    };

    class MpqFileSystem : public FileSystem<MpqFile>
    {
    public:
        static std::shared_ptr<MpqFileSystem> Open()
        {
            static std::shared_ptr<MpqFileSystem> instance;
            static std::once_flag once;
            std::call_once(once, []() -> void {
                instance = std::make_shared<MpqFileSystem>();
            });

            return instance;
        }

        ~MpqFileSystem();

        void Initialize(const std::string& rootFolder) override;
        std::shared_ptr<FileHandle<MpqFile>> OpenFile(const std::string& filePath) override;

    private:
        std::vector<HANDLE> _archiveHandles;
    };

    class DiskFileSystem : public FileSystem<DiskFile>
    {
    public:
        static std::shared_ptr<DiskFileSystem> Open()
        {
            static std::shared_ptr<DiskFileSystem> instance;
            static std::once_flag once;
            std::call_once(once, []() -> void {
                instance = std::make_shared<DiskFileSystem>();
            });

            return instance;
        }

        ~DiskFileSystem();

        void Initialize(const std::string& rootFolder) override;
        std::shared_ptr<FileHandle<DiskFile>> OpenFile(const std::string& relFilePath) override;

    private:
        std::string _rootFolder;
    };

}
