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

    constexpr static const float TILE_SIZE  = 1600.0f / 3.0f;
    constexpr static const float MAX_XY     = 32.0f * TILE_SIZE;
    constexpr static const float CHUNK_SIZE = TILE_SIZE / 16.0f;
    constexpr static const float UNIT_SIZE  = CHUNK_SIZE / 8.0f;

    /*  --------------------------------- ADT TILE --------------------------------- */
    MapChunk::MapChunk(uint32_t x, uint32_t y, const std::string& directoryName)
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

    MapChunk::~MapChunk()
    {
        for (Chunk* itr : _chunks)
            delete itr;

        _chunks.clear();
    }

    void MapChunk::ParseFile(std::shared_ptr<FileHandle<MpqFile>> const& fileHandle)
    {
        uint8_t const* rootData = fileHandle->GetData();
        uint8_t const* position = rootData;

        while ((position - rootData) < fileHandle->GetFileSize())
        {
            uint32_t chunkIdentifier = *reinterpret_cast<uint32_t const*>(position);
            uint32_t chunkSize = *reinterpret_cast<uint32_t const*>(position + 4);

            std::vector<uint8_t> chunkData(position + 8, position + 8 + chunkSize);
            HandleTerrainChunk(chunkIdentifier, chunkData);

            position += 8 + chunkSize;
        }
    }

    void MapChunk::HandleTerrainChunk(uint32_t identifier, std::vector<uint8_t> const& content)
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

    const char* MapChunk::GetModelFilename(uint32_t index) const
    {
        if (index >= _modelFilenamesOffset.size())
            return nullptr;

        auto strOffset = _modelFilenamesOffset[index];
        return reinterpret_cast<const char*>(_modelFilenames.data() + strOffset);
    }

    const char* MapChunk::GetWorldModelFilename(uint32_t index) const
    {
        if (index >= _worldMapObjectFilenamesOffset.size())
            return nullptr;

        auto strOffset = _worldMapObjectFilenamesOffset[index];
        return reinterpret_cast<const char*>(_worldMapObjectFilenames.data() + strOffset);
    }

    CAaBox const& MapChunk::GetBoundingBox() const
    {
        return _boundingBox;
    }

    void MapChunk::Render() const
    {
        for (Chunk* chunk : _chunks)
            chunk->Render();
    }

    bool MapChunk::HasGeometry() const
    {
        for (Chunk* chunk : _chunks)
            if (chunk->HasGeometry())
                return true;

        return false;
    }

    /*  --------------------------------- ADT TILE MCNK --------------------------------- */

    Chunk::Chunk(uint8_t const* data, size_t length)
    {
        memcpy(&_header, data, sizeof(SMChunk));
        uint8_t const* start = data + sizeof(SMChunk);
        uint8_t const* position = start;

        while ((position - start) < length)
        {
            uint32_t chunkIdentifier = *reinterpret_cast<uint32_t const*>(position);
            uint32_t chunkSize = *reinterpret_cast<uint32_t const*>(position + 4);

            std::vector<uint8_t> chunkData(position + 8, position + 8 + chunkSize);
            HandleTerrainChunk(chunkIdentifier, chunkData);

            position += 8 + chunkSize;
        }
    }

    void Chunk::HandleTerrainChunk(uint32_t identifier, std::vector<uint8_t> const& content)
    {
        switch (identifier)
        {
            case 'MCVT':
            {
                _hasGeometry = true;

                uint32_t i = 0;
                const float* heightData = reinterpret_cast<float const*>(content.data());
                for (uint32_t x = 0; x < 17; ++x)
                {
                    uint32_t yMax = ((x & 1) != 0) ? 8 : 9;
                    for (uint32_t y = 0; y < yMax; ++y)
                    {
                        VertexData& itr = _vertexData[i];

                        itr.vertice.X = _header.position.X - x * UNIT_SIZE * 0.5f;
                        itr.vertice.Y = _header.position.Y - y * UNIT_SIZE;
                        itr.vertice.Z = heightData[i];
                        if ((i & 1) != 0)
                            itr.vertice.Y -= 0.5f * UNIT_SIZE;

                        ++i;

                        for (uint32_t k = 0; k < 3; ++k)
                        {
                            if (itr.vertice[k] > _boundingBox.Maximum[k])
                                _boundingBox.Maximum[k] = itr.vertice[k];

                            if (itr.vertice[k] < _boundingBox.Minimum[k])
                                _boundingBox.Minimum.Z = itr.vertice[k];
                        }
                    }
                }

                _hasGeometry = std::abs(_boundingBox.height() - 1.0f) > 1.0e-5f;
                break;
            }
            case 'MCNR':
            {
                uint32_t i = 0;
                for (VertexData& itr : _vertexData)
                {
                    itr.normal.X = float(int8_t(content[i + 0])) / 127.0f;
                    itr.normal.Y = float(int8_t(content[i + 1])) / 127.0f;
                    itr.normal.Z = float(int8_t(content[i + 2])) / 127.0f;
                    // if (std::abs(itr.normal.lengthSquared() - 2.0f) < 1.0e-8f) // (l^2 + 1 - 2l) < stddev^2. assume l is close enough to 1 that 2l ~= 2
                    //     itr.normal.normalize();

                    i += 3;
                }
                break;
            }
            default:
            {
                std::string_view chunkIdentifier(reinterpret_cast<char const*>(&identifier), 4);
                LOG_INFO << "ADT [MCNK] Skipped " << chunkIdentifier << " chunk (" << content.size() << " bytes)";
                break;
            }
        }
    }

    void Chunk::Render()
    {
    }

    bool Chunk::HasGeometry() const
    {
        return _hasGeometry;
    }

    /*  --------------------------------- ADT MAP --------------------------------- */

    ADT::ADT(const std::string& directoryName)
    {
        for (uint32_t x = 0; x < 64; ++x)
        {
            for (uint32_t y = 0; y < 64; ++y)
            {
                MapChunk* newChunk = new MapChunk(x, y, directoryName);
                if (newChunk->HasGeometry())
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
