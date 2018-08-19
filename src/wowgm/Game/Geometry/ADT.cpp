#include "ADT.hpp"
#include "Assert.hpp"
#include "Logger.hpp"

#include <sstream>
#include <cstring>
#include <string_view>

#ifdef min
#undef min
#endif

namespace wowgm::game::geometry
{
    using namespace wowgm::filesystem;

    ADT::MapChunk::MapChunk(std::uint32_t x, std::uint32_t y, const std::string& directoryName)
    {
        auto manager = MpqFileSystem::Instance();

        std::stringstream filePath;
        filePath << "world/maps/" << directoryName << '/' << directoryName << '_' << x << '_' << y;

        _boundingBox.Minimum.X = x * CHUNK_SIZE;
        _boundingBox.Minimum.Y = y * CHUNK_SIZE;
        _boundingBox.Maximum.X = _boundingBox.Minimum.X + CHUNK_SIZE;
        _boundingBox.Maximum.Y = _boundingBox.Minimum.Y + CHUNK_SIZE;

        // Marker for validity, set as long as we don't find vertices
        _boundingBox.Minimum.Z = std::numeric_limits<float>::min();

        std::shared_ptr<FileHandle<MpqFile>> rootFile = manager->OpenFile(filePath.str() + ".adt", LoadStrategy::Mapped);
        std::shared_ptr<FileHandle<MpqFile>> objFile = manager->OpenFile(filePath.str() + "_obj0.adt", LoadStrategy::Mapped);
        std::shared_ptr<FileHandle<MpqFile>> texFile = manager->OpenFile(filePath.str() + "_tex0.adt", LoadStrategy::Mapped);

        if (!rootFile || !objFile || !texFile)
            return;

        ParseFile(rootFile);
        ParseFile(objFile);
        ParseFile(texFile);
    }

    ADT::MapChunk::~MapChunk()
    {
        for (Chunk* itr : _chunks)
            delete itr;

        _chunks.clear();
    }

    void ADT::MapChunk::ParseFile(std::shared_ptr<FileHandle<MpqFile>> const& fileHandle)
    {
        std::uint8_t const* rootData = fileHandle->GetData();
        std::uint8_t const* position = rootData;

        while ((position - rootData) < fileHandle->GetFileSize())
        {
            std::uint32_t chunkIdentifier = *reinterpret_cast<std::uint32_t const*>(position);
            std::uint32_t chunkSize = *reinterpret_cast<std::uint32_t const*>(position + 4);

            std::vector<std::uint8_t> chunkData(position + 8, position + 8 + chunkSize);
            HandleTerrainChunk(chunkIdentifier, chunkData);

            position += 8 + chunkSize;
        }
    }

    void ADT::MapChunk::HandleTerrainChunk(std::uint32_t identifier, std::vector<std::uint8_t> const& content)
    {
        switch (identifier)
        {
            case 'MVER':
                break;
            case 'MTEX':
                _textureFilenames = std::move(content);
                break;
            // M2
            case 'MMDX':
                _modelFilenames = std::move(content);
                break;
            case 'MMID':
                _modelFilenamesOffset.resize(content.size() / 4);
                std::memmove(_modelFilenamesOffset.data(), content.data(), content.size());
                break;
            case 'MDDF':
                _doodadDefinitions.resize(content.size() / sizeof(SMDoodadDef));
                std::memmove(_doodadDefinitions.data(), content.data(), content.size());
                break;
            // WMO
            case 'MWMO':
                _worldMapObjectFilenames = std::move(content);
                break;
            case 'MWID':
                _worldMapObjectFilenamesOffset.resize(content.size() / 4);
                std::memmove(_worldMapObjectFilenamesOffset.data(), content.data(), content.size());
                break;
            case 'MODF':
                _mapObjDefinitions.resize(content.size() / sizeof(SMMapObjDef));
                std::memmove(_mapObjDefinitions.data(), content.data(), content.size());
                break;

            case 'MCNK':
                _chunks.push_back(new Chunk(content.data(), content.size()));
                break;

            default:
            {
                std::string_view chunkIdentifier(reinterpret_cast<char const*>(&identifier), 4);
                LOG_INFO << "ADT Loading - Skipped " << chunkIdentifier << " chunk (" << content.size() << " bytes)";
                break;
            }
        }
    }

    const char* ADT::MapChunk::GetModelFilename(std::uint32_t index) const
    {
        if (index >= _modelFilenamesOffset.size())
            return nullptr;

        auto strOffset = _modelFilenamesOffset[index];
        return reinterpret_cast<const char*>(_modelFilenames.data() + strOffset);
    }

    const char* ADT::MapChunk::GetWorldModelFilename(std::uint32_t index) const
    {
        if (index >= _worldMapObjectFilenamesOffset.size())
            return nullptr;

        auto strOffset = _worldMapObjectFilenamesOffset[index];
        return reinterpret_cast<const char*>(_worldMapObjectFilenames.data() + strOffset);
    }

    CAaBox const& ADT::MapChunk::GetBoundingBox() const
    {
        return _boundingBox;
    }

    bool ADT::MapChunk::IsValid() const
    {
        return _boundingBox.Minimum.Z != std::numeric_limits<float>::min();
    }

    ADT::MapChunk::Chunk::Chunk(std::uint8_t const* data, size_t length)
    {
        memcpy(&_header, data, sizeof(SMChunk));
        std::uint8_t const* start = data + sizeof(SMChunk);
        std::uint8_t const* position = start;

        while ((position - start) < length)
        {
            std::uint32_t chunkIdentifier = *reinterpret_cast<std::uint32_t const*>(position);
            std::uint32_t chunkSize = *reinterpret_cast<std::uint32_t const*>(position + 4);

            std::vector<std::uint8_t> chunkData(position + 8, position + 8 + chunkSize);
            HandleTerrainChunk(chunkIdentifier, chunkData);

            position += 8 + chunkSize;
        }
    }

    void ADT::MapChunk::Chunk::Chunk::HandleTerrainChunk(std::uint32_t identifier, std::vector<std::uint8_t> const& content)
    {
        switch (identifier)
        {
            case 'MCVT':
                _vertices.resize(9 * 9 + 8 * 8);
                std::memmove(_vertices.data(), content.data(), content.size());
                break;
            case 'MCLV':
                _mclv.resize(9 * 9 + 8 * 8);
                std::memmove(_mclv.data(), content.data(), content.size());
                break;
            case 'MCCV':
                break;
            case 'MCNR':
            {
                _normals.resize(9 * 9 + 8 * 8);

                std::uint8_t const* normalData = content.data();
                std::uint8_t const* position = normalData;

                std::uint32_t itr;
                while ((position - normalData) < content.size())
                {
                    _normals[itr].X = float(std::int8_t(position[0])) / 127.0f;
                    _normals[itr].Y = float(std::int8_t(position[1])) / 127.0f;
                    _normals[itr].Z = float(std::int8_t(position[2])) / 127.0f;
                    position += 3;
                }
                break;
            }
        }
    }

    ADT::ADT(const std::string& directoryName)
    {
        for (std::uint32_t x = 0; x < 64; ++x)
        {
            for (std::uint32_t y = 0; y < 64; ++y)
            {
                MapChunk* newChunk = new MapChunk(x, y, directoryName);
                if (newChunk->IsValid())
                    _chunks.push_back(newChunk);
                else
                    delete newChunk;
            }
        }
    }

    ADT::~ADT()
    {
        for (MapChunk* itr : _chunks)
            delete itr;

        _chunks.clear();
    }
}
