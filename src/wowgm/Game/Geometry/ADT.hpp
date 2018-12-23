#pragma once

#include "FileSystem.hpp"
#include "C3Vector.hpp"
#include "CAaBox.hpp"
#include "CArgb.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace wowgm::game::geometry
{
    using namespace shared::filesystem;
    using namespace wowgm::game::structures;

    struct SMChunk
    {
        uint32_t X;
        uint32_t Y;
        float radius; // If version < ?
        uint32_t nLayers;
        uint32_t nDoodadRefs;
        uint32_t ofsHeight;
        uint32_t ofsNormal;
        uint32_t ofsLayer;
        uint32_t ofsRefs;
        uint32_t ofsAlpha;
        uint32_t sizeAlpha;
        uint32_t AreaID;
        uint32_t nMapObjRefs;
        uint16_t LowResolutionHoles;
        uint16_t ReallyLowQualityTextureingMap[8];
        uint64_t noEffectDoodad;
        uint32_t ofsSoundEmitters;
        uint32_t nSoundEmitters;
        uint32_t ofsLiquid;
        uint32_t sizeLiquid;
        C3Vector position;
        uint32_t ofsMCCV;
        uint32_t ofsMCLV;
        uint32_t _;
    };

    struct SMDoodadDef
    {
        uint32_t Index;
        uint32_t UniqueID;
        C3Vector Position;
        C3Vector Rotation; // Degrees.
        uint16_t Scale;
        uint16_t Flags;

        float GetScale() const {
            return float(Scale) / 1024.0f;
        }
    };

    struct SMMapObjDef
    {
        uint32_t Index;
        uint32_t UniqueID;
        C3Vector Position;
        C3Vector Rotation;
        CAaBox Extents; // Position plus the transformed wmo bounding box. used for defining if they are rendered as well as collision.
        uint16_t Flags;
        uint16_t DoodadSet;
        uint16_t NameSet;
        uint16_t Scale;

        float GetScale() const {
            return float(Scale) / 1024.0f;
        }
    };

    // A representation of a single MCNK element
    class Chunk
    {
        struct VertexData
        {
            C3Vector vertice;
            C3Vector normal;
        };

    public:
        Chunk(uint8_t const* data, size_t length);
        Chunk(Chunk&&) = delete;
        Chunk(const Chunk&) = delete;

        void Render();
        void PushToGPU();

        bool HasGeometry() const;

    private:
        void HandleTerrainChunk(uint32_t identifier, std::vector<uint8_t> const& content);

    private:
        SMChunk _header;
        CAaBox _boundingBox;
        bool _hasGeometry = false;

        std::array<VertexData, 145> _vertexData;
    };

    // A representation of a single adt tile (%s_%n_%n)
    class MapChunk
    {
    public:

        MapChunk(uint32_t x, uint32_t y, const std::string& directoryName);
        ~MapChunk();

        MapChunk(MapChunk&&) = delete;
        MapChunk(const MapChunk&) = delete;

        constexpr static const float CHUNK_SIZE = 533.33333f;

        CAaBox const& GetBoundingBox() const;

        void Render() const;
        bool HasGeometry() const;

    private:
        void ParseFile(std::shared_ptr<FileHandle<MpqFile>> const& fileHandle);

        void HandleTerrainChunk(uint32_t identifier, std::vector<uint8_t> const& content);

        /// Return the name of an M2 model, given the provided index (usually from MDDF)
        const char* GetModelFilename(uint32_t index) const;
        /// Return the name of a WMO model, given the provided index (usually from MODF)
        const char* GetWorldModelFilename(uint32_t index) const;

    private:
        CAaBox _boundingBox;
        std::vector<Chunk*> _chunks;

        std::vector<uint8_t> _textureFilenames;

        std::vector<uint8_t> _modelFilenames;
        std::vector<uint32_t> _modelFilenamesOffset;

        std::vector<uint8_t> _worldMapObjectFilenames;
        std::vector<uint32_t> _worldMapObjectFilenamesOffset;

        std::vector<SMDoodadDef> _doodadDefinitions;
        std::vector<SMMapObjDef> _mapObjDefinitions;
    };


    class ADT
    {
    public:
        ADT(const std::string& directoryName);
        ~ADT();

        ADT(ADT&&) = delete;
        ADT(const ADT&) = delete;

        typedef std::vector<MapChunk*>::iterator iterator;
        typedef std::vector<MapChunk*>::const_iterator const_iterator;

        iterator begin() { return _chunks.begin(); }
        const_iterator begin() const { return _chunks.begin(); }

    private:
        std::vector<MapChunk*> _chunks;
    };
}
