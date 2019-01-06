#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include <shared/defines.hpp>
#include <shared/filesystem/file_system.hpp>

#include <windows.h>

namespace shared::filesystem
{
    class mpq_file_system;

    class mpq_file : public file_handle<mpq_file>
    {
        friend class mpq_file_system;

        using Basefile_handle = file_handle<mpq_file>;

        mpq_file(HANDLE fileHandle);

    public:
        ~mpq_file();

        void Close() override;
        size_t GetFileSize() const override;
        uint8_t const* GetData() override;
        size_t ReadBytes(size_t offset, size_t length, uint8_t* buffer, size_t bufferSize) override;

    private:
        HANDLE _fileHandle;

        std::vector<uint8_t> _fileData;
    };

    class mpq_file_system final : public file_system<mpq_file>
    {
    public:
        static mpq_file_system* Instance()
        {
            static mpq_file_system instance;

            return &instance;
        }

        ~mpq_file_system();

        void Initialize(const std::string& rootFolder) override;
        std::shared_ptr<mpq_file> OpenFile(const std::string& filePath) override;

        bool FileExists(const std::string& relFilePath) const override;

    private:
        std::vector<HANDLE> _archiveHandles;
        std::string _currentRootFolder;
    };
}
