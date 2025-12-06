#pragma once

#include "types.hpp"
#include "storage/database.hpp"
#include <string>

namespace storage {

void migrate(Database& db);
void upsert_player(Database& db, const Player& p);
void upsert_champion(Database& db, const Champion& c);
void upsert_team(Database& db, const Team& t);
void insert_match(Database& db, const MatchRecord& match);
SynergyStats load_synergy(Database& db, const std::string& a, const std::string& b);
void upsert_node(Database& db, const Node& node);
void insert_edge(Database& db, const Edge& edge);

}
