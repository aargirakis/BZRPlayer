/**
* 
* @file
*
* @brief  ZIP archives structures
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//common includes
#include <byteorder.h>

namespace Formats
{
  namespace Packed
  {
    namespace Zip
    {
#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
      PACK_PRE struct GenericHeader
      {
        static const uint16_t SIGNATURE = 0x4b50;

        //+0
        uint16_t Signature;
      } PACK_POST;

      PACK_PRE struct FileAttributes
      {
        uint32_t CRC;
        uint32_t CompressedSize;
        uint32_t UncompressedSize;
      } PACK_POST;

      enum
      {
        FILE_CRYPTED = 0x1,
        FILE_COMPRESSION_LEVEL_MASK = 0x6,
        FILE_COMPRESSION_LEVEL_NONE = 0x0,
        FILE_COMPRESSION_LEVEL_NORMAL = 0x2,
        FILE_COMPRESSION_LEVEL_FAST = 0x4,
        FILE_COMPRESSION_LEVEL_EXTRAFAST = 0x6,
        FILE_ATTRIBUTES_IN_FOOTER = 0x8,
        FILE_UTF8 = 0x800
      };

      PACK_PRE struct LocalFileHeader
      {
        static const uint32_t SIGNATURE = 0x04034b50;

        //+0
        uint32_t Signature;
        //+4
        uint16_t VersionToExtract;
        //+6
        uint16_t Flags;
        //+8
        uint16_t CompressionMethod;
        //+a
        uint16_t ModificationTime;
        //+c
        uint16_t ModificationDate;
        //+e
        FileAttributes Attributes;
        //+1a
        uint16_t NameSize;
        //+1c
        uint16_t ExtraSize;
        //+1e
        char Name[1];

        bool IsValid() const
        {
          return fromLE(Signature) == SIGNATURE;
        }

        std::size_t GetSize() const
        {
          return offsetof(LocalFileHeader, Name) + fromLE(NameSize) + fromLE(ExtraSize);
        }

        bool IsSupported() const
        {
          const uint_t flags = fromLE(Flags);
          return 0 == (flags & FILE_CRYPTED);
        }
      } PACK_POST;

      PACK_PRE struct LocalFileFooter
      {
        static const uint32_t SIGNATURE = 0x08074b50;

        //+0
        uint32_t Signature;
        //+4
        FileAttributes Attributes;
      } PACK_POST;

      PACK_PRE struct ExtraDataRecord
      {
        static const uint32_t SIGNATURE = 0x08064b50;

        //+0
        uint32_t Signature;
        //+4
        uint32_t DataSize;
        //+8
        uint8_t Data[1];

        std::size_t GetSize() const
        {
          return sizeof(*this) - 1 + fromLE(DataSize);
        }
      } PACK_POST;

      PACK_PRE struct CentralDirectoryFileHeader
      {
        static const uint32_t SIGNATURE = 0x02014b50;

        //+0
        uint32_t Signature;
        //+4
        uint16_t VersionMadeBy;
        //+6
        uint16_t VersionToExtract;
        //+8
        uint16_t Flags;
        //+a
        uint16_t CompressionMethod;
        //+c
        uint16_t ModificationTime;
        //+e
        uint16_t ModificationDate;
        //+10
        FileAttributes Attributes;
        //+1c
        uint16_t NameSize;
        //+1e
        uint16_t ExtraSize;
        //+20
        uint16_t CommentSize;
        //+22
        uint16_t StartDiskNumber;
        //+24
        uint16_t InternalFileAttributes;
        //+26
        uint32_t ExternalFileAttributes;
        //+2a
        uint32_t LocalHeaderRelOffset;
        //+2e
        char Name[1];

        std::size_t GetSize() const
        {
          return offsetof(CentralDirectoryFileHeader, Name) + fromLE(NameSize) + fromLE(ExtraSize) + fromLE(CommentSize);
        }
      } PACK_POST;

      PACK_PRE struct CentralDirectoryEnd
      {
        static const uint32_t SIGNATURE = 0x06054b50;

        //+0
        uint32_t Signature;
        //+4
        uint16_t ThisDiskNumber;
        //+6
        uint16_t StartDiskNumber;
        //+8
        uint16_t ThisDiskCentralDirectoriesCount;
        //+a
        uint16_t TotalDirectoriesCount;
        //+c
        uint32_t CentralDirectorySize;
        //+10
        uint32_t CentralDirectoryOffset;
        //+12
        uint16_t CommentSize;
        //+14
        //uint8_t Comment[0];

        std::size_t GetSize() const
        {
          return sizeof(*this) + fromLE(CommentSize);
        }
      } PACK_POST;

      PACK_PRE struct DigitalSignature
      {
        static const uint32_t SIGNATURE = 0x05054b50;

        //+0
        uint32_t Signature;
        //+4
        uint16_t DataSize;
        //+6
        uint8_t Data[1];

        std::size_t GetSize() const
        {
          return sizeof(*this) - 1 + fromLE(DataSize);
        }
      } PACK_POST;

      class CompressedFile
      {
      public:
        virtual ~CompressedFile() = default;

        virtual std::size_t GetPackedSize() const = 0;
        virtual std::size_t GetUnpackedSize() const = 0;

        static std::unique_ptr<const CompressedFile> Create(const LocalFileHeader& hdr, std::size_t availSize);
      };
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif
    }
  }
}
