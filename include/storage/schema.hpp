#pragma once

#include "types.hpp"
#include "storage/database.hpp"
#include <string>

namespace storage {

void migrate(Database& db);
void upsert_player(Database& db, const Player& p);
void upsert_champion(Database& db, const Champion& c);
void insert_match(Database& db, const MatchRecord& match);
SynergyStats load_synergy(Database& db, const std::string& a, const std::string& b);

}
