#include "storage/database.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace {
std::string path_join(const std::string& root, const std::string& file) {
    std::filesystem::path p(root);
    p /= file;
    return p.string();
}

template <class T>
T to_number(const std::string& s) {
    std::istringstream iss(s);
    T v{};
    iss >> v;
    return v;
}
} // namespace

GraphStore::GraphStore(const std::string& root) : root_(root) {
    std::filesystem::create_directories(root_);
}

void GraphStore::load() {
    // nodes
    std::ifstream nf(path_join(root_, "nodes.tsv"));
    if (nf) {
        std::string id, type, label;
        while (std::getline(nf, id, '\t')) {
            if (!std::getline(nf, type, '\t')) break;
            if (!std::getline(nf, label)) break;
            nodes_[id] = Node{id, type, label};
        }
    }

    // edges
    std::ifstream ef(path_join(root_, "edges.tsv"));
    if (ef) {
        std::string line;
        while (std::getline(ef, line)) {
            std::istringstream iss(line);
            std::string src, dst, type, wstr;
            if (!std::getline(iss, src, '\t')) continue;
            if (!std::getline(iss, dst, '\t')) continue;
            if (!std::getline(iss, type, '\t')) continue;
            if (!std::getline(iss, wstr)) continue;
            Edge e{src, dst, type, to_number<double>(wstr)};
            add_edge(e);
        }
    }

    // synergy
    std::ifstream sf(path_join(root_, "synergy.tsv"));
    if (sf) {
        std::string line;
        while (std::getline(sf, line)) {
            std::istringstream iss(line);
            std::string a, b, g, w;
            if (!std::getline(iss, a, '\t')) continue;
            if (!std::getline(iss, b, '\t')) continue;
            if (!std::getline(iss, g, '\t')) continue;
            if (!std::getline(iss, w)) continue;
            SynergyStats s;
            s.games = to_number<int>(g);
            s.wins = to_number<int>(w);
            synergy_[a][b] = s;
            synergy_[b][a] = s;
        }
    }
}

void GraphStore::save() const {
    std::ofstream nf(path_join(root_, "nodes.tsv"), std::ios::trunc);
    for (const auto& [_, n] : nodes_) {
        nf << n.id << '\t' << n.type << '\t' << n.label << '\n';
    }

    std::ofstream ef(path_join(root_, "edges.tsv"), std::ios::trunc);
    for (const auto& [src, list] : adjacency_) {
        for (const auto& e : list) {
            ef << src << '\t' << e.dst << '\t' << e.type << '\t' << e.weight << '\n';
        }
    }

    std::ofstream sf(path_join(root_, "synergy.tsv"), std::ios::trunc);
    for (const auto& [a, row] : synergy_) {
        for (const auto& [b, s] : row) {
            if (a >= b) continue;
            sf << a << '\t' << b << '\t' << s.games << '\t' << s.wins << '\n';
        }
    }
}

void GraphStore::upsert_player(const Player& p) {
    players_[p.summoner_id] = p;
    nodes_[p.summoner_id] = Node{p.summoner_id, "player", p.name.empty() ? p.summoner_id : p.name};
}

void GraphStore::upsert_team(const Team& t) {
    teams_[t.team_id] = t;
    nodes_[t.team_id] = Node{t.team_id, "team", t.name.empty() ? t.team_id : t.name};
}

void GraphStore::upsert_champion(const Champion& c) {
    champions_[std::to_string(c.key)] = c;
    // optional node creation skipped to keep graph focused on people/teams/matches
}

void GraphStore::add_edge(const Edge& e) {
    auto& list = adjacency_[e.src];
    for (auto& existing : list) {
        if (existing.dst == e.dst && existing.type == e.type) {
            existing.weight = e.weight;
            return;
        }
    }
    list.push_back(e);
}

void GraphStore::insert_match(const MatchRecord& match) {
    nodes_[match.match_id] = Node{match.match_id, "match", match.match_id};
    for (const auto& p : match.participants) {
        upsert_player(Player{p.player_id, p.player_id, match.region});
        add_edge(Edge{p.player_id, match.match_id, "played", 1.0});
        if (!p.team_id.empty()) {
            upsert_team(Team{p.team_id, p.team_id});
            add_edge(Edge{p.team_id, p.player_id, "roster", 1.0});
        }
    }

    const auto& parts = match.participants;
    for (size_t i = 0; i < parts.size(); ++i) {
        for (size_t j = i + 1; j < parts.size(); ++j) {
            const auto& a = parts[i];
            const auto& b = parts[j];
            int win = (a.win && b.win) ? 1 : 0;
            synergy_[a.player_id][b.player_id].games += 1;
            synergy_[a.player_id][b.player_id].wins += win;
            synergy_[b.player_id][a.player_id] = synergy_[a.player_id][b.player_id];
        }
    }
}

