//
//  Created by Bradley Austin Davis on 2016/02/17
//  Copyright 2013-2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <cstdint>
#include <vector>
#include <memory>

namespace vks { namespace storage {

    class Storage;
    using StoragePointer = std::shared_ptr<const Storage>;
    using ByteArray = std::vector<uint8_t>;

    // Abstract class to represent memory that stored _somewhere_ (in system memory or in a file, for example)
    class Storage : public std::enable_shared_from_this<Storage>
    {
        public:
            virtual ~Storage() {}
            virtual const uint8_t* data() const = 0;
            virtual size_t size() const = 0;
            virtual bool isFast() const = 0;

            static StoragePointer create(size_t size, uint8_t* data);
            static StoragePointer readFile(const std::string& filename);
            StoragePointer createView(size_t size = 0, size_t offset = 0) const;

            // Aliases to prevent having to re-write a ton of code
            inline size_t getSize() const { return size(); }
            inline const std::uint8_t* readData() const { return data() + _cursor; }

            template <typename T>
            Storage& operator >> (T& output)
            {
                output = *reinterpret_cast<T*>(readData() + _cursor);
                _cursor += sizeof(T);
                return *this;
            }

            template <typename T, size_t N>
            Storage& operator >> (T (&output)[N])
            {
                T* ptr = *reinterpret_cast<T*>(readData() + _cursor);
                memcpy(output, ptr, sizeof(T) * N);
                _cursor += sizeof(T) * N;
                return *this;
            }

        private:
            std::uint32_t _cursor = 0;
    };

    template <typename T>
    inline StoragePointer& operator >> (StoragePointer& storage, T& v)
    {
        *storage.get() >> v;
        return storage;
    }

    template <typename T, size_t N>
    inline StoragePointer& operator >> (StoragePointer& storage, T (&v)[N])
    {
        *storage.get() >> v;
        return storage;
    }

}}  // namespace vks::storage
