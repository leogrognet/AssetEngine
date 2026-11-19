#pragma once
#include <string>
#include <vector>

#include "BinaryReader.h"


struct Player
{
    int health;
    int mana;
    int level;
};


std::vector<std::byte> WritePlayerV1(const Player& player);

std::vector<std::byte> WritePlayerV2(const Player& player);

Player ReadPlayer(BinaryReader& reader, uint32_t version);