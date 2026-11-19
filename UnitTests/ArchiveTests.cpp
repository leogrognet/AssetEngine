#include <gtest/gtest.h>
#include "TestHelpers.h"
#include "ArchiveWriter.h"
#include "ArchiveReader.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include "FileIO.h"
#include "CRC32.h"
#include "PlayerAdapter.h"


namespace
{
    std::vector<std::byte> FinalizeWithValidCRC(BinaryWriter& writer)
    {
        const uint32_t crc = CRC32::Compute(writer.GetBuffer());
        writer.WriteInt<uint32_t>(crc);
        return writer.TakeBuffer();
    }
}


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

TEST(ArchiveTest, RejectsDuplicateIdEvenWithDifferentAssetType)
{
    ArchiveWriter writer;
    writer.AddAsset(1, AssetType::Mesh, std::vector<std::byte>{ std::byte{ 0x01 } });

    EXPECT_THROW(
        writer.AddAsset(1, AssetType::RigidBody, std::vector<std::byte>{ std::byte{ 0x02 } }),
        std::invalid_argument
    );
}

TEST(ArchiveTest, ReplaceAssetUpdatesAssetTypeAndQueries)
{
    TempFile file("test_replace_type.aset");

    ArchiveWriter writer;
    writer.AddAsset(10, AssetType::Mesh, std::vector<std::byte>{ std::byte{ 0xAA } });
    writer.ReplaceAsset(10, AssetType::Collider, std::vector<std::byte>{ std::byte{ 0xBB }, std::byte{ 0xCC } });
    writer.Save(file.Str());

    ArchiveReader reader;
    reader.Load(file.Str());

    EXPECT_EQ(reader.GetAssetType(10), AssetType::Collider);
    EXPECT_TRUE(reader.GetAssetsByType(AssetType::Mesh).empty());
    EXPECT_EQ(reader.GetAssetsByType(AssetType::Collider), (std::vector<uint32_t>{ 10u }));
}

TEST(ArchiveTest, PreservesUnknownRawAssetTypes)
{
    TempFile file("test_custom_raw_type.aset");
    constexpr uint32_t customEngineType = 999u;

    ArchiveWriter writer;
    writer.AddAsset(5, customEngineType, std::vector<std::byte>{ std::byte{ 0x42 } });
    writer.Save(file.Str());

    ArchiveReader reader;
    reader.Load(file.Str());

    const auto unknownEnum = static_cast<AssetType>(customEngineType);
    EXPECT_EQ(reader.GetAssetType(5), unknownEnum);
    EXPECT_EQ(reader.GetAssetsByType(unknownEnum), (std::vector<uint32_t>{ 5u }));
}

TEST(ArchiveTest, ThrowsWhenQueryingUnloadedReader)
{
    ArchiveReader emptyReader;
    EXPECT_THROW((void)emptyReader.GetAssetType(1), std::invalid_argument);
    EXPECT_TRUE(emptyReader.GetAssetsByType(AssetType::RigidBody).empty());
}

TEST(ArchiveTest, QueriesAssetTypesAndFiltersByType)
{
    TempFile file("test_asset_types.aset");

    BinaryWriter dummy;
    dummy.WriteInt<uint32_t>(42);

    ArchiveWriter writer;
    writer.AddAsset(1, AssetType::Mesh, dummy.GetBuffer());
    writer.AddAsset(2, AssetType::RigidBody, dummy.GetBuffer());
    writer.AddAsset(3, AssetType::RigidBody, dummy.GetBuffer());
    writer.Save(file.Str());

    ArchiveReader reader;
    reader.Load(file.Str());

    EXPECT_EQ(reader.GetAssetType(1), AssetType::Mesh);
    EXPECT_EQ(reader.GetAssetType(2), AssetType::RigidBody);

    const auto bodies = reader.GetAssetsByType(AssetType::RigidBody);
    ASSERT_EQ(bodies.size(), 2u);
    EXPECT_NE(std::find(bodies.begin(), bodies.end(), 2u), bodies.end());
    EXPECT_NE(std::find(bodies.begin(), bodies.end(), 3u), bodies.end());
    EXPECT_TRUE(reader.GetAssetsByType(AssetType::Texture).empty());
}

TEST(ArchiveTest, WriterRemoveAndReplaceValidation)
{
    ArchiveWriter writer;
    writer.AddAsset(1, AssetType::Mesh, std::vector<std::byte>{ std::byte{ 0x01 } });

    EXPECT_THROW(
        writer.ReplaceAsset(99, AssetType::Mesh, std::vector<std::byte>{ std::byte{ 0x02 } }),
        std::invalid_argument
    );

    EXPECT_TRUE(writer.RemoveAsset(1));
    EXPECT_FALSE(writer.RemoveAsset(1));
    EXPECT_FALSE(writer.HasAsset(1));
}

TEST(ArchiveTest, EmptyArchiveAndMissingFile)
{
    TempFile emptyArchiveFile("test_empty_archive.aset");

    ArchiveWriter writer;
    writer.Save(emptyArchiveFile.Str());

    ArchiveReader reader;
    EXPECT_NO_THROW(reader.Load(emptyArchiveFile.Str()));
    EXPECT_FALSE(reader.HasAsset(1));
    EXPECT_THROW(reader.Load("non_existent_archive_12345.aset"), std::runtime_error);
}

TEST(ArchiveTest, RejectsInvalidHeaderAndRegistryWithValidCRC)
{
    TempFile forgedFile("test_forged_header.aset");
    ArchiveReader reader;

    {
        BinaryWriter w;
        for (char c : std::string_view("NOPE"))
            w.WriteInt<uint8_t>(static_cast<uint8_t>(c));
        w.WriteInt<uint32_t>(VERSION);
        w.WriteInt<uint32_t>(0);
        w.WriteInt<uint64_t>(HEADER_SIZE);

        SaveFile(forgedFile.Str(), FinalizeWithValidCRC(w));
        EXPECT_THROW(reader.Load(forgedFile.Str()), std::runtime_error);
    }

    {
        BinaryWriter w;
        for (char c : MAGIC)
            w.WriteInt<uint8_t>(static_cast<uint8_t>(c));
        w.WriteInt<uint32_t>(VERSION + 1);
        w.WriteInt<uint32_t>(0);
        w.WriteInt<uint64_t>(HEADER_SIZE);

        SaveFile(forgedFile.Str(), FinalizeWithValidCRC(w));
        EXPECT_THROW(reader.Load(forgedFile.Str()), std::runtime_error);
    }

    {
        BinaryWriter w;
        for (char c : MAGIC)
            w.WriteInt<uint8_t>(static_cast<uint8_t>(c));
        w.WriteInt<uint32_t>(VERSION);
        w.WriteInt<uint32_t>(1);
        w.WriteInt<uint64_t>(HEADER_SIZE);

        w.WriteInt<uint32_t>(1);
        w.WriteInt<uint32_t>(static_cast<uint32_t>(AssetType::Mesh));
        w.WriteInt<uint64_t>(4);
        w.WriteInt<uint64_t>(4);

        SaveFile(forgedFile.Str(), FinalizeWithValidCRC(w));
        EXPECT_THROW(reader.Load(forgedFile.Str()), std::runtime_error);
    }
}

TEST(ArchiveTest, AcceptLatestVersion)
{

    TempFile file("test_version.bin");
    Player player;
    player.health = 100;
    player.mana = 50;
    player.level = 5;

    std::vector<std::byte> playerBuffer = WritePlayerV2(player);


    ArchiveWriter writer;
    writer.AddAsset(1, 1, playerBuffer);
    writer.Save(file.Str());



    ArchiveReader reader;
    reader.Load(file.Str());
    BinaryReader playerReader(reader.GetAssetData(1));
    Player loadedPlayer = ReadPlayer(playerReader, reader.GetVersion());
    EXPECT_EQ(reader.GetVersion(), VERSION);
    EXPECT_FLOAT_EQ(loadedPlayer.health, 100.0f);
    EXPECT_FLOAT_EQ(loadedPlayer.mana, 50.0f);
    EXPECT_FLOAT_EQ(loadedPlayer.level, 5.0f);
}

TEST(ArchiveTest, AcceptOlderVersion)
{
    TempFile file("test_version_old.bin");
    Player player;
    player.health = 80;
    player.mana = 30;
    player.level = 3;

    std::vector<std::byte> playerBuffer = WritePlayerV1(player);

    ArchiveWriter writer;
    writer.AddAsset(1, 1, playerBuffer);
    writer.Save(file.Str(), 1);

    ArchiveReader reader;
    reader.Load(file.Str());
    BinaryReader playerReader(reader.GetAssetData(1));
    Player loadedPlayer = ReadPlayer(playerReader, reader.GetVersion());
    EXPECT_EQ(reader.GetVersion(), 1);
    EXPECT_FLOAT_EQ(loadedPlayer.health, 80.0f);
    EXPECT_FLOAT_EQ(loadedPlayer.mana, 30.0f);
    EXPECT_FLOAT_EQ(loadedPlayer.level, 1.0f);
}