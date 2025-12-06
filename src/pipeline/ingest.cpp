#include "pipeline/ingest.hpp"

#include "storage/schema.hpp"
#include <unordered_map>

namespace pipeline {

void ingest(Database& db, const std::vector<MatchRecord>& matches) {
    db.transaction([&]() {
        for (const auto& match : matches) {
            storage::insert_match(db, match);
            // build pairwise edges
            for (size_t i = 0; i < match.participants.size(); ++i) {
                for (size_t j = i + 1; j < match.participants.size(); ++j) {
                    const auto& a = match.participants[i];
                    const auto& b = match.participants[j];
                    sqlite3_stmt* stmt{};
                    const char* sql = "INSERT INTO synergy_edges(a,b,games,wins) VALUES(?,?,1,?) "
                                      "ON CONFLICT(a,b) DO UPDATE SET games=synergy_edges.games+1, "
                                      "wins=synergy_edges.wins+excluded.wins;";
                    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
                        throw std::runtime_error("prepare failed synergy_edges");
                    }
                    sqlite3_bind_text(stmt, 1, a.player_id.c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, b.player_id.c_str(), -1, SQLITE_TRANSIENT);
                    int win = (a.win && b.win) ? 1 : 0;
                    sqlite3_bind_int(stmt, 3, win);
                    sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                }
            }
        }
    });
}

} // namespace pipeline

