#pragma once

#include "types.hpp"
#include "storage/database.hpp"
#include <vector>

namespace pipeline {

void ingest(GraphStore& store, const std::vector<MatchRecord>& matches);

} 

