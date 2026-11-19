#include "PlayerAdapter.h"

#include "BinaryReader.h"
#include "BinaryWriter.h"


std::vector<std::byte> WritePlayerV1(const Player& player)
{
    BinaryWriter writer;
    writer.WriteValue(player.health);
    writer.WriteValue(player.mana);
    return writer.TakeBuffer();
}

std::vector<std::byte> WritePlayerV2(const Player& player)
{
    BinaryWriter writer;
    writer.WriteValue(player.health);
    writer.WriteValue(player.mana);
    writer.WriteValue(player.level);
    return writer.TakeBuffer();
}

Player ReadPlayer(BinaryReader& reader, uint32_t version)
{
    Player player;
    player.health = reader.ReadInt<int32_t>();
    player.mana   = reader.ReadInt<int32_t>();
    player.level  = (version >= 2) ? reader.ReadInt<int32_t>() : 1;
    return player;
}
