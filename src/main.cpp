#include <cstdio>
#include <cstdint>
#include <vector>

#include "ArchiveReader.h"
#include "ArchiveWriter.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"


struct Player { int32_t health; int32_t mana; int32_t level; };

std::vector<std::byte> WritePlayer(const Player& player)   // format v2
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
    player.level  = (version >= 2) ? reader.ReadInt<int32_t>() : 1;   // valeur par défaut pour la v1
    return player;
}

int main()
{
    constexpr uint32_t PLAYER_ID   = 7;
    constexpr uint32_t TYPE_PLAYER = 1;

    // 1. Ecriture
    const Player original{100, 42, 5};
    std::printf("Joueur ecrit : health = %d, mana = %d, level = %d\n", original.health, original.mana, original.level);


    ArchiveWriter writer;
    writer.AddAsset(PLAYER_ID, TYPE_PLAYER, WritePlayer(original));
    writer.Save("player.aset");
    std::printf("player.aset ecrit\n");

    // 2. Lecture
    ArchiveReader archive;
    archive.Load("player.aset");

    auto bytes = archive.GetAssetData(PLAYER_ID);
    std::printf("L'asset %u contient %zu octets\n", PLAYER_ID, bytes.size());

    BinaryReader reader(bytes);
    const Player loaded = ReadPlayer(reader, archive.GetVersion());
    std::printf("Joueur lu    : health = %d, mana = %d, level = %d\n", loaded.health, loaded.mana, loaded.level);

    // 3. Comparaison
    const bool same = loaded.health == original.health && loaded.mana == original.mana
               && loaded.level == original.level;
    std::printf(same ? "OK : le joueur relu est identique\n"
                     : "ECHEC : le joueur relu est different\n");
    return same ? 0 : 1;
}