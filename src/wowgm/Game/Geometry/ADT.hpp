#pragma once

#include "FileSystem.hpp"
#include "C3Vector.hpp"
#include "CAaBox.hpp"

#include <cstdint>
#include <string>
#include <vector>

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

    class ADT
    {
    public:
        class MapChunk
        {
        public:
            class Chunk
            {
            public:
                explicit Chunk(std::uint8_t const* data);
                Chunk(Chunk&&) = delete;
                Chunk(const Chunk&) = delete;

            private:
                SMChunk _header;
            };

            MapChunk(std::uint32_t x, std::uint32_t y, const std::string& directoryName);
            ~MapChunk();

            MapChunk(MapChunk&&) = delete;
            MapChunk(const MapChunk&) = delete;

            constexpr static const float CHUNK_SIZE = 533.33333f;

            bool IsValid() const;
            CAaBox const& GetBoundingBox() const;

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

        };

        ADT(const std::string& directoryName);
        ~ADT();

        ADT(ADT&&) = delete;
        ADT(const ADT&) = delete;

    private:
        std::vector<MapChunk*> _chunks;
    };
}
