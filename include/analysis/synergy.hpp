#pragma once

#include "types.hpp"
#include "storage/database.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace analysis {

using Adjacency = std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>;

SynergyMap build_synergy_map(const GraphStore& store);
std::vector<SynergyEdge> top_duos(const SynergyMap& map, int limit, int min_games = 3);
Adjacency build_adjacency(const GraphStore& store);
std::vector<std::string> bfs_path(const Adjacency& adj, const std::string& start, const std::string& goal);

}

