using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TexturesShellExtension.Conversion;
using TexturesShellExtension.IO;

namespace TexturesShellExtension
{
    internal sealed class KTX
    {
        public KTX(string fileName)
        {
            using (var fs = File.OpenRead(fileName))
                FromStream(fs);
        }

        public KTX(Stream dataStream)
        {
            FromStream(dataStream);
        }

        private static byte[] _identifier = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

        /// <summary>
        /// For compressed textures, glType must equal 0. For uncompressed textures, glType specifies the type parameter
        /// passed to glTex{,Sub}Image*D, usually one of the values from table 8.2 of the OpenGL 4.4 specification [OPENGL44]
        /// (UNSIGNED_BYTE, UNSIGNED_SHORT_5_6_5, etc.)
        /// </summary>
        private uint _glType;

        /// <summary>
        /// glTypeSize specifies the data type size that should be used when endianness conversion is required for the texture
        /// data stored in the file. If glType is not 0, this should be the size in bytes corresponding to glType. For texture
        /// data which does not depend on platform endianness, including compressed texture data, glTypeSize must equal 1.
        /// </summary>
        private uint _glTypeSize;

        /// <summary>
        /// For compressed textures, glFormat must equal 0. For uncompressed textures, glFormat specifies the format parameter
        /// passed to glTex{,Sub}Image*D, usually one of the values from table 8.3 of the OpenGL 4.4 specification [OPENGL44]
        /// (RGB, RGBA, BGRA, etc.)
        /// </summary>
        private uint _glFormat;

        /// <summary>
        /// For compressed textures, glInternalFormat must equal the compressed internal format, usually one of the values from
        /// table 8.14 of the OpenGL 4.4 specification [OPENGL44]. For uncompressed textures, glInternalFormat specifies the
        /// internalformat parameter passed to glTexStorage*D or glTexImage*D, usually one of the sized internal formats from
        /// tables 8.12 & 8.13 of the OpenGL 4.4 specification [OPENGL44]. The sized format should be chosen to match the bit
        /// depth of the data provided. glInternalFormat is used when loading both compressed and uncompressed textures, except
        /// when loading into a context that does not support sized formats, such as an unextended OpenGL ES 2.0 context where
        /// the internalformat parameter is required to have the same value as the format parameter.
        /// </summary>
        private uint _glInternalFormat;

        /// <summary>
        /// For both compressed and uncompressed textures, glBaseInternalFormat specifies the base internal format of the texture,
        /// usually one of the values from table 8.11 of the OpenGL 4.4 specification [OPENGL44] (RGB, RGBA, ALPHA, etc.). For
        /// uncompressed textures, this value will be the same as glFormat and is used as the internalformat parameter when loading
        /// into a context that does not support sized formats, such as an unextended OpenGL ES 2.0 context.
        /// </summary>
        private uint _glBaseInternalFormat;

        /// <summary>
        /// The width of the texture image for level 0, in pixels. No rounding to block sizes should be applied for block compressed
        /// textures.
        ///
        /// For 1D textures pixelHeight and pixelDepth must be 0. For 2D and cube textures pixelDepth must be 0.
        /// </summary>
        private uint _pixelWidth;

        /// <summary>
        /// The height of the texture image for level 0, in pixels. No rounding to block sizes should be applied for block compressed
        /// textures.
        ///
        /// For 1D textures pixelHeight and pixelDepth must be 0. For 2D and cube textures pixelDepth must be 0.
        /// </summary>
        private uint _pixelHeight;

        private uint _pixelDepth;

        /// <summary>
        /// numberOfArrayElements specifies the number of array elements. If the texture is not an array texture, numberOfArrayElements
        /// must equal 0.
        /// </summary>
        private uint _numberOfArrayElements;

        /// <summary>
        /// numberOfFaces specifies the number of cubemap faces. For cubemaps and cubemap arrays this should be 6. For non cubemaps this
        /// should be 1. Cube map faces are stored in the order: +X, -X, +Y, -Y, +Z, -Z.
        /// </summary>
        private uint _numberOfFaces;

        /// <summary>
        /// numberOfMipmapLevels must equal 1 for non-mipmapped textures. For mipmapped textures, it equals the number of mipmaps.
        /// Mipmaps are stored in order from largest size to smallest size. The first mipmap level is always level 0. A KTX file does
        /// not need to contain a complete mipmap pyramid. If numberOfMipmapLevels equals 0, it indicates that a full mipmap pyramid
        /// should be generated from level 0 at load time (this is usually not allowed for compressed formats).
        /// </summary>
        private uint _numberOfMipmapLevels;

        private Dictionary<string, byte[]> _headerData;

        private void FromStream(Stream dataStream)
        {
            using (var reader = new EndianBinaryReader(EndianBitConverter.Little, dataStream, Encoding.UTF8))
            {
                var identifier = reader.ReadBytes(12);

                if (!identifier.SequenceEqual(_identifier))
                    return;

                var isLittleEndian = reader.ReadInt32() == 0x04030201;
                if (!isLittleEndian)
                    reader.BitConverter = EndianBitConverter.Big;

                _glType = reader.ReadUInt32();
                _glTypeSize = reader.ReadUInt32();
                _glFormat = reader.ReadUInt32();
                _glInternalFormat = reader.ReadUInt32();
                _glBaseInternalFormat = reader.ReadUInt32();
                _pixelWidth = Math.Max(1u, reader.ReadUInt32());
                _pixelHeight = Math.Max(1u, reader.ReadUInt32());
                _pixelDepth = Math.Max(1u, reader.ReadUInt32());
                _numberOfArrayElements = Math.Max(1u, reader.ReadUInt32());
                _numberOfFaces = reader.ReadUInt32();
                _numberOfMipmapLevels = Math.Max(1u, reader.ReadUInt32());

                // An arbitrary number of key/value pairs may follow the header. This can be used to encode any arbitrary data. The bytesOfKeyValueData
                // field indicates the total number of bytes of key/value data including all keyAndValueByteSize fields, all keyAndValue fields,
                // and all valuePadding fields. The file offset of the first imageSize field is located at the file offset of the bytesOfKeyValueData
                // field plus the value of the bytesOfKeyValueData field plus 4.
                long bytesOfKeyValueData = reader.ReadUInt32();

                _headerData = new Dictionary<string, byte[]>();
                while (bytesOfKeyValueData > 0)
                {
                    var blockStart = reader.BaseStream.Position;

                    var keyAndValueByteSize = reader.ReadInt32();
                    var keyAndValue = reader.ReadBytes(keyAndValueByteSize);
                    reader.BaseStream.Position += 3 - ((keyAndValueByteSize + 3) % 4);

                    var blockEnd = reader.BaseStream.Position;

                    bytesOfKeyValueData -= blockEnd - blockStart;

                    var nulTerminator = 0;
                    while (keyAndValue[nulTerminator] != 0)
                        ++nulTerminator;

                    var key = Encoding.UTF8.GetString(keyAndValue, 0, nulTerminator);
                    var value = keyAndValue.Skip(nulTerminator).ToArray();

                    _headerData[key] = value;
                }

                for (var i = 0u; i < _numberOfMipmapLevels; ++i)
                {
                    var imageSize = reader.ReadUInt32();
                    for (var j = 0u; j < _numberOfArrayElements; ++j)
                    {
                        for (var fIndex = 0; fIndex < _numberOfFaces; ++fIndex)
                        {
                            for (var zSlice = 0; zSlice < _pixelDepth; ++zSlice)
                            {
                            }
                        }
                    }
                }
            }
        }
    }
}
