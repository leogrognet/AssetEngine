#include <gtest/gtest.h>
#include "BinaryWriter.h"
#include "BinaryReader.h"

namespace
{
    struct DummyVec3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        bool operator==(const DummyVec3&) const = default;
    };

    void Serialize(BinaryWriter& w, const DummyVec3& v)
    {
        w.WriteFloat(v.x);
        w.WriteFloat(v.y);
        w.WriteFloat(v.z);
    }

    void Deserialize(BinaryReader& r, DummyVec3& v)
    {
        v.x = r.ReadFloat();
        v.y = r.ReadFloat();
        v.z = r.ReadFloat();
    }
}

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

TEST(BinaryIOTest, RejectsCorruptedBoolInsideVector)
{
    BinaryWriter forged;
    forged.WriteInt<uint32_t>(1);
    forged.WriteInt<uint8_t>(2);

    BinaryReader r(forged.GetBuffer());
    EXPECT_THROW((void)r.ReadVector<bool>(), std::runtime_error);
}

TEST(BinaryIOTest, RejectsAbsurdVectorLengthBeforeAllocation)
{
    BinaryWriter forged;
    forged.WriteInt<uint32_t>(0xFFFFFFFFu);

    BinaryReader r(forged.GetBuffer());
    EXPECT_THROW((void)r.ReadVector<DummyVec3>(), std::runtime_error);
}

TEST(BinaryIOTest, RejectsTruncatedCustomStructMidVector)
{
    BinaryWriter partial;
    partial.WriteInt<uint32_t>(2);
    partial.WriteFloat(1.0f);
    partial.WriteFloat(2.0f);
    partial.WriteFloat(3.0f);
    partial.WriteFloat(4.0f);

    BinaryReader r(partial.GetBuffer());
    EXPECT_THROW((void)r.ReadVector<DummyVec3>(), std::runtime_error);
}

TEST(BinaryIOTest, EmptyVectorsAtExactEndOfBuffer)
{
    BinaryWriter w;
    w.WriteVector(std::vector<std::string>{});
    w.WriteVector(std::vector<DummyVec3>{});

    BinaryReader r(w.GetBuffer());
    EXPECT_TRUE(r.ReadVector<std::string>().empty());
    EXPECT_TRUE(r.ReadVector<DummyVec3>().empty());
}

TEST(BinaryIOTest, CustomStructStringsAndBoolsRoundTrip)
{
    const std::vector<std::string> names{ "", "Mesh_01", "PhysicsCollider" };
    const std::vector<bool> flags{ true, false, true, true };
    const std::vector<DummyVec3> points{ { 1.0f, 2.0f, 3.0f }, { -4.5f, 0.0f, 9.25f } };

    BinaryWriter w;
    w.WriteDouble(3.141592653589793);
    w.WriteDouble(std::numeric_limits<double>::infinity());
    w.WriteValue(DummyVec3{ 7.0f, -8.0f, 9.0f });
    w.WriteVector(names);
    w.WriteVector(flags);
    w.WriteVector(points);

    BinaryReader r(w.GetBuffer());
    EXPECT_DOUBLE_EQ(r.ReadDouble(), 3.141592653589793);
    EXPECT_TRUE(std::isinf(r.ReadDouble()));
    EXPECT_EQ(r.ReadValue<DummyVec3>(), (DummyVec3{ 7.0f, -8.0f, 9.0f }));
    EXPECT_EQ(r.ReadVector<std::string>(), names);
    EXPECT_EQ(r.ReadVector<bool>(), flags);
    EXPECT_EQ(r.ReadVector<DummyVec3>(), points);
}

TEST(BinaryIOTest, RejectsCorruptedBoolAndTruncatedString)
{
    BinaryWriter badBool;
    badBool.WriteInt<uint8_t>(255);
    BinaryReader rBool(badBool.GetBuffer());
    EXPECT_THROW((void)rBool.ReadBool(), std::runtime_error);

    BinaryWriter truncatedStr;
    truncatedStr.WriteInt<uint32_t>(50);
    truncatedStr.WriteInt<uint8_t>('A');
    BinaryReader rStr(truncatedStr.GetBuffer());
    EXPECT_THROW((void)rStr.ReadString(), std::runtime_error);
}