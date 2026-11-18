#include <gtest/gtest.h>
#include "BinaryWriter.h"
#include "BinaryReader.h"

TEST(BinaryIOTest, PrimitivesStringsAndVectorsRoundTrip)
{
    BinaryWriter w;
    w.WriteInt<uint32_t>(0x01020304);
    w.WriteInt<int32_t>(-5);
    w.WriteBool(true);
    w.WriteFloat(100.0f);
    w.WriteString("Test");
    w.WriteVector(std::vector<uint16_t>{ 10, 20 });

    BinaryReader r(w.GetBuffer());
    EXPECT_EQ(r.ReadInt<uint32_t>(), 0x01020304u);
    EXPECT_EQ(r.ReadInt<int32_t>(), -5);
    EXPECT_TRUE(r.ReadBool());
    EXPECT_FLOAT_EQ(r.ReadFloat(), 100.0f);
    EXPECT_EQ(r.ReadString(), "Test");

    const auto v = r.ReadVector<uint16_t>();
    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 10u);
    EXPECT_EQ(v[1], 20u);
}

TEST(BinaryIOTest, EmptyContainersAndOutOfBoundsThrows)
{
    BinaryWriter w;
    w.WriteString("");
    w.WriteVector(std::vector<uint32_t>{});

    BinaryReader r(w.GetBuffer());
    EXPECT_TRUE(r.ReadString().empty());
    EXPECT_TRUE(r.ReadVector<uint32_t>().empty());
    EXPECT_THROW(r.ReadInt<uint8_t>(), std::runtime_error);
}