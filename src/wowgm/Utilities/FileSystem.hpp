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

    enum class LoadStrategy
    {
        Memory,
        Mapped
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

        virtual LoadStrategy GetLoadStrategy() const = 0;
        virtual std::uint8_t const* GetData() = 0;
        virtual size_t ReadBytes(size_t offset, size_t length, std::uint8_t* buffer, size_t bufferSize) = 0;
    };

    class MpqFile : public FileHandle<MpqFile>
    {
        friend class MpqFileSystem;

        using BaseFileHandle = FileHandle<MpqFile>;

        MpqFile(HANDLE fileHandle, LoadStrategy loadStrategy);

    public:
        ~MpqFile();

        void Close() override;
        size_t GetFileSize() const override;
        LoadStrategy GetLoadStrategy() const override;
        std::uint8_t const* GetData() override;
        size_t ReadBytes(size_t offset, size_t length, std::uint8_t* buffer, size_t bufferSize) override;

    private:
        HANDLE _fileHandle;
        LoadStrategy _loadStrategy;

        std::vector<std::uint8_t> _fileData;
    };

    class DiskFile : public FileHandle<DiskFile>
    {
        friend class DiskFileSystem;

        DiskFile(const std::string& fileName, LoadStrategy loadStrategy);

    public:
        ~DiskFile();

        void Close() override;
        size_t GetFileSize() const override;
        LoadStrategy GetLoadStrategy() const override;
        std::uint8_t const* GetData() override;
        size_t ReadBytes(size_t offset, size_t length, std::uint8_t* buffer, size_t bufferSize) override;

    private:
        void _LoadToMemory(const std::string& fileName);

    private:
        size_t _fileSize = 0;
        LoadStrategy _loadStrategy;
        std::vector<uint8_t> _fileData;

#if PLATFORM == PLATFORM_WINDOWS
        HANDLE _fileHandle = INVALID_HANDLE_VALUE;
        HANDLE _mapFile = INVALID_HANDLE_VALUE;
        uint8_t* _mapped{ nullptr };
#elif (PLATFORM == PLATFORM_APPLE) || (PLATFORM == PLATFORM_UNIX)
        int _fileDescriptor;
        void* _mapped;
#endif
    };

    template <typename T>
    class FileSystem
    {
        virtual void Initialize(const std::string& rootFolder) = 0;
        virtual std::shared_ptr<FileHandle<T>> OpenFile(const std::string& filePath, LoadStrategy loadStrategy = LoadStrategy::Mapped) = 0;
        virtual std::shared_ptr<FileHandle<T>> OpenDirectFile(const std::string& filePath, LoadStrategy loadStrategy = LoadStrategy::Mapped) = 0;

        void SetLocale(Locale clientLocale) { _clientLocale = clientLocale; }
        Locale GetLocale() const { return _clientLocale; }

        virtual bool FileExists(const std::string& relFilePath) const = 0;
        virtual bool FileExists(const std::string& relFilePath, const std::string& root) const = 0;

    protected:
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

    class MpqFileSystem final : public FileSystem<MpqFile>
    {
    public:
        static MpqFileSystem* Instance()
        {
            static MpqFileSystem instance;

            return &instance;
        }

        ~MpqFileSystem();

        void Initialize(const std::string& rootFolder) override;
        std::shared_ptr<FileHandle<MpqFile>> OpenFile(const std::string& filePath, LoadStrategy loadStrategy) override;
        std::shared_ptr<FileHandle<MpqFile>> OpenDirectFile(const std::string& filePath, LoadStrategy loadStrategy = LoadStrategy::Mapped) override
        {
            return { };
        }
        bool FileExists(const std::string& relFilePath, const std::string& root) const override;
        bool FileExists(const std::string& relFilePath) const override;

    private:
        std::vector<HANDLE> _archiveHandles;
        std::string _currentRootFolder;
    };

    class DiskFileSystem final : public FileSystem<DiskFile>
    {
    public:
        static DiskFileSystem* Instance()
        {
            static DiskFileSystem instance;

            return &instance;
        }

        ~DiskFileSystem();

        void Initialize(const std::string& rootFolder) override;
        std::shared_ptr<FileHandle<DiskFile>> OpenFile(const std::string& relFilePath, LoadStrategy loadStrategy) override;
        std::shared_ptr<FileHandle<DiskFile>> OpenDirectFile(const std::string& filePath, LoadStrategy loadStrategy = LoadStrategy::Mapped) override;
        bool FileExists(const std::string& relFilePath, const std::string& root) const override;
        bool FileExists(const std::string& relFilePath) const override;

    private:
        std::string _rootFolder;
    };

}
