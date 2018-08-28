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
    using namespace wowgm::filesystem;
    using namespace wowgm::game::structures;

    struct SMChunk
    {
        std::uint32_t X;
        std::uint32_t Y;
        float radius; // If version < ?
        std::uint32_t nLayers;
        std::uint32_t nDoodadRefs;
        std::uint32_t ofsHeight;
        std::uint32_t ofsNormal;
        std::uint32_t ofsLayer;
        std::uint32_t ofsRefs;
        std::uint32_t ofsAlpha;
        std::uint32_t sizeAlpha;
        std::uint32_t AreaID;
        std::uint32_t nMapObjRefs;
        std::uint16_t LowResolutionHoles;
        std::uint16_t ReallyLowQualityTextureingMap[8];
        std::uint64_t noEffectDoodad;
        std::uint32_t ofsSoundEmitters;
        std::uint32_t nSoundEmitters;
        std::uint32_t ofsLiquid;
        std::uint32_t sizeLiquid;
        C3Vector position;
        std::uint32_t ofsMCCV;
        std::uint32_t ofsMCLV;
        std::uint32_t _;
    };

    struct SMDoodadDef
    {
        std::uint32_t Index;
        std::uint32_t UniqueID;
        C3Vector Position;
        C3Vector Rotation; // Degrees.
        std::uint16_t Scale;
        std::uint16_t Flags;

        float GetScale() const {
            return float(Scale) / 1024.0f;
        }
    };

    struct SMMapObjDef
    {
        std::uint32_t Index;
        std::uint32_t UniqueID;
        C3Vector Position;
        C3Vector Rotation;
        CAaBox Extents; // Position plus the transformed wmo bounding box. used for defining if they are rendered as well as collision.
        std::uint16_t Flags;
        std::uint16_t DoodadSet;
        std::uint16_t NameSet;
        std::uint16_t Scale;

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
        Chunk(std::uint8_t const* data, size_t length);
        Chunk(Chunk&&) = delete;
        Chunk(const Chunk&) = delete;

        void Render();
        void PushToGPU();

        bool HasGeometry() const;

    private:
        void HandleTerrainChunk(std::uint32_t identifier, std::vector<std::uint8_t> const& content);

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

        MapChunk(std::uint32_t x, std::uint32_t y, const std::string& directoryName);
        ~MapChunk();

        MapChunk(MapChunk&&) = delete;
        MapChunk(const MapChunk&) = delete;

        constexpr static const float CHUNK_SIZE = 533.33333f;

        CAaBox const& GetBoundingBox() const;

        void Render() const;
        bool HasGeometry() const;

    private:
        void ParseFile(std::shared_ptr<FileHandle<MpqFile>> const& fileHandle);

        void HandleTerrainChunk(std::uint32_t identifier, std::vector<std::uint8_t> const& content);

        /// Return the name of an M2 model, given the provided index (usually from MDDF)
        const char* GetModelFilename(std::uint32_t index) const;
        /// Return the name of a WMO model, given the provided index (usually from MODF)
        const char* GetWorldModelFilename(std::uint32_t index) const;

    private:
        CAaBox _boundingBox;
        std::vector<Chunk*> _chunks;

        std::vector<std::uint8_t> _textureFilenames;

        std::vector<std::uint8_t> _modelFilenames;
        std::vector<std::uint32_t> _modelFilenamesOffset;

        std::vector<std::uint8_t> _worldMapObjectFilenames;
        std::vector<std::uint32_t> _worldMapObjectFilenamesOffset;

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
