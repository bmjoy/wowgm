#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

namespace shared::filesystem
{
    enum class Locale
    {
        French,
        English,
        Russian
    };

    template <typename T>
    class file_handle : public std::enable_shared_from_this<T>
    {
    protected:
        virtual ~file_handle() { }

    public:
        virtual void Close() = 0;
        virtual size_t GetFileSize() const = 0;

        virtual uint8_t const* GetData() = 0;
        virtual size_t ReadBytes(size_t offset, size_t length, uint8_t* buffer, size_t bufferSize) = 0;

        template <typename U>
        inline U* Read(size_t byteOffset) const {
            return reinterpret_cast<U*>(GetData() + offset);
        }
    };

    template <typename T>
    class file_system
    {
        virtual void Initialize(const std::string& rootFolder) = 0;
        virtual std::shared_ptr<T> OpenFile(const std::string& filePath) = 0;

        void SetLocale(Locale clientLocale) { _clientLocale = clientLocale; }
        Locale GetLocale() const { return _clientLocale; }

        virtual bool FileExists(const std::string& relFilePath) const = 0;

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
}
