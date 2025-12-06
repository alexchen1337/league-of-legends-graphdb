#pragma once

#include "types.hpp"
#include "storage/database.hpp"
#include <vector>

namespace pipeline {

void ingest(Database& db, const std::vector<MatchRecord>& matches);

} 

