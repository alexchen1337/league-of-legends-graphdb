#include "analysis/synergy.hpp"

#include <algorithm>

namespace analysis {

SynergyMap build_synergy_map(const GraphStore& store) {
    return store.synergy();
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

Adjacency build_adjacency(const GraphStore& store) {
    Adjacency adj;
    for (const auto& [src, list] : store.adjacency()) {
        for (const auto& e : list) {
            adj[src].push_back({e.dst, e.weight});
        }
    }
    return adj;
}

std::vector<std::string> bfs_path(const Adjacency& adj, const std::string& start, const std::string& goal) {
    std::unordered_map<std::string, std::string> parent;
    std::vector<std::string> queue;
    queue.push_back(start);
    parent[start] = {};
    size_t idx = 0;
    while (idx < queue.size()) {
        auto cur = queue[idx++];
        if (cur == goal) break;
        auto it = adj.find(cur);
        if (it == adj.end()) continue;
        for (const auto& [next, _] : it->second) {
            if (parent.count(next)) continue;
            parent[next] = cur;
            queue.push_back(next);
        }
    }
    if (!parent.count(goal)) return {};
    std::vector<std::string> path;
    for (std::string cur = goal; !cur.empty(); cur = parent[cur]) {
        path.push_back(cur);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

} // namespace analysis

