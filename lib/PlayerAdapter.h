#pragma once
#include <string>
#include <vector>

#include "BinaryReader.h"


struct Player
{
    int32_t health;
    int32_t mana;
    int32_t level;
};


std::vector<std::byte> WritePlayerV1(const Player& player);

std::vector<std::byte> WritePlayerV2(const Player& player);

Player ReadPlayer(BinaryReader& reader, uint32_t version);