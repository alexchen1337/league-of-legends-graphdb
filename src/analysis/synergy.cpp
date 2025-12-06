#include "analysis/synergy.hpp"

#include <algorithm>

namespace analysis {

SynergyMap build_synergy_map(Database& db) {
    SynergyMap map;
    const char* sql = "SELECT a,b,games,wins FROM synergy_edges;";
    sqlite3_stmt* stmt{};
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare failed select synergy_edges");
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string a = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string b = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        SynergyStats s;
        s.games = sqlite3_column_int(stmt, 2);
        s.wins = sqlite3_column_int(stmt, 3);
        map[a][b] = s;
        map[b][a] = s;
    }
    sqlite3_finalize(stmt);
    return map;
}

std::vector<SynergyEdge> top_duos(const SynergyMap& map, int limit, int min_games) {
    std::vector<SynergyEdge> edges;
    for (const auto& [a, row] : map) {
        for (const auto& [b, stats] : row) {
            if (a >= b) continue; // avoid duplicates
            if (stats.games < min_games) continue;
            SynergyEdge e;
            e.a = a;
            e.b = b;
            e.games = stats.games;
            e.wins = stats.wins;
            edges.push_back(e);
        }
    }
    std::sort(edges.begin(), edges.end(), [&](const SynergyEdge& lhs, const SynergyEdge& rhs) {
        double l = lhs.games == 0 ? 0.0 : static_cast<double>(lhs.wins) / lhs.games;
        double r = rhs.games == 0 ? 0.0 : static_cast<double>(rhs.wins) / rhs.games;
        if (l == r) return lhs.games > rhs.games;
        return l > r;
    });
    if (edges.size() > static_cast<size_t>(limit)) {
        edges.resize(limit);
    }
    return edges;
}

} // namespace analysis

