#include "pipeline/ingest.hpp"

#include "storage/database.hpp"

namespace pipeline {

void ingest(GraphStore& store, const std::vector<MatchRecord>& matches) {
    for (const auto& match : matches) {
        store.insert_match(match);
    }
}

} // namespace pipeline

