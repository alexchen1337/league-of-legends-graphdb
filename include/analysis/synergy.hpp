#pragma once

#include "types.hpp"
#include "storage/database.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace analysis {

using SynergyMap = std::unordered_map<std::string, std::unordered_map<std::string, SynergyStats>>;

SynergyMap build_synergy_map(Database& db);
std::vector<SynergyEdge> top_duos(const SynergyMap& map, int limit, int min_games = 3);

}

