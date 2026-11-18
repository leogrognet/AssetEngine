#include <gtest/gtest.h>
#include "TestHelpers.h"
#include "ArchiveWriter.h"
#include "ArchiveReader.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include "FileIO.h"

TEST(ArchiveTest, MultiAssetRoundTrip)
{
    TempFile file("test_roundtrip.bin");

    BinaryWriter payload1;
    payload1.WriteString("MeshCube");
    payload1.WriteInt<uint32_t>(36);

    BinaryWriter payload2;
    payload2.WriteFloat(9.81f);

    ArchiveWriter writer;
    writer.AddAsset(1, 0x4D455348, payload1.GetBuffer());
    writer.AddAsset(2, 0x50485953, payload2.GetBuffer());
    writer.Save(file.Str());

    ArchiveReader reader;
    reader.Load(file.Str());
    EXPECT_TRUE(reader.HasAsset(1));
    EXPECT_TRUE(reader.HasAsset(2));
    EXPECT_FALSE(reader.HasAsset(99));
    EXPECT_THROW((void)reader.GetAssetData(99), std::invalid_argument);

    BinaryReader r1(reader.GetAssetData(1));
    EXPECT_EQ(r1.ReadString(), "MeshCube");
    EXPECT_EQ(r1.ReadInt<uint32_t>(), 36u);

    BinaryReader r2(reader.GetAssetData(2));
    EXPECT_FLOAT_EQ(r2.ReadFloat(), 9.81f);
}

TEST(ArchiveTest, RejectsAlteredByteAndTruncatedFile)
{
    TempFile validFile("test_valid.bin");
    TempFile corruptedFile("test_corrupted.bin");
    TempFile truncatedFile("test_truncated.bin");

    BinaryWriter payload;
    payload.WriteString("PayloadData");

    ArchiveWriter writer;
    writer.AddAsset(10, 1, payload.GetBuffer());
    writer.Save(validFile.Str());

    auto corrupted = LoadFile(validFile.Str());
    corrupted[corrupted.size() / 2] ^= std::byte{ 0xFF };
    SaveFile(corruptedFile.Str(), corrupted);

    ArchiveReader reader;
    EXPECT_THROW(reader.Load(corruptedFile.Str()), std::runtime_error);

    auto truncated = LoadFile(validFile.Str());
    truncated.pop_back();
    SaveFile(truncatedFile.Str(), truncated);

    EXPECT_THROW(reader.Load(truncatedFile.Str()), std::runtime_error);
}

TEST(ArchiveTest, AtomicLoadPreservesPreviousStateOnFailure)
{
    TempFile goodFile("test_atomic_good.bin");
    TempFile badFile("test_atomic_bad.bin");

    BinaryWriter payload;
    payload.WriteInt<uint32_t>(1234);

    ArchiveWriter writer;
    writer.AddAsset(1, 1, payload.GetBuffer());
    writer.Save(goodFile.Str());

    SaveFile(badFile.Str(), std::vector<std::byte>{ std::byte{ 0x00 }, std::byte{ 0x01 } });

    ArchiveReader reader;
    reader.Load(goodFile.Str());
    ASSERT_TRUE(reader.HasAsset(1));

    EXPECT_THROW(reader.Load(badFile.Str()), std::runtime_error);
    EXPECT_TRUE(reader.HasAsset(1));
}