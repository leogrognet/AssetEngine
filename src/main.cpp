#include "BinaryWriter.h"
#include "../lib/BinaryReader.h"
#include <cassert>
#include <iostream>

int main()
{

    BinaryWriter w;
    w.WriteInt<uint32_t>(0x01020304);
    w.WriteInt<int32_t>(-5);
    w.WriteBool(true);
    w.WriteFloat(100.0f);
    w.WriteString("Test");
    w.WriteVector(std::vector<uint16_t>{10, 20});

    for (std::byte b : w.getBuffer())
        std::cout << std::hex << static_cast<int>(b) << ' ';
    std::cout << '\n';

    BinaryReader r(w.getBuffer());
    assert(r.ReadInt<uint32_t>() == 0x01020304);
    assert(r.ReadInt<int32_t>() == -5);
    assert(r.ReadBool() == true);
    assert(r.ReadFloat() == 100.0f);
    assert(r.ReadString() == "Test");
    auto v = r.ReadVector<uint16_t>();
    assert(v.size() == 2 && v[0] == 10 && v[1] == 20);

    std::cout << "OK\n";

}
