#include "storage/database.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <stdexcept>

namespace {
std::string path_join(const std::string& root, const std::string& file) {
    std::filesystem::path p(root);
    p /= file;
    return p.string();
}

void write_string(std::ofstream& out, const std::string& s) {
    std::uint32_t len = static_cast<std::uint32_t>(s.size());
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    out.write(s.data(), len);
}

std::string read_string(std::ifstream& in) {
    std::uint32_t len{};
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string s(len, '\0');
    in.read(s.data(), len);
    return s;
}
} // namespace

GraphStore::GraphStore(const std::string& root) : root_(root) {
    std::filesystem::create_directories(root_);
}

void GraphStore::load() {
    load_nodes();
    load_edges();
    load_synergy();
}

void GraphStore::save() const {
    save_nodes();
    save_edges();
    save_synergy();
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

void GraphStore::save_nodes() const {
    std::ofstream out(path_join(root_, "nodes.bin"), std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("cannot write nodes");
    std::uint32_t count = static_cast<std::uint32_t>(nodes_.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& [_, n] : nodes_) {
        write_string(out, n.id);
        write_string(out, n.type);
        write_string(out, n.label);
    }
}

void GraphStore::save_edges() const {
    std::ofstream out(path_join(root_, "edges.bin"), std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("cannot write edges");
    std::uint32_t count = 0;
    for (const auto& [_, list] : adjacency_) count += static_cast<std::uint32_t>(list.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& [src, list] : adjacency_) {
        for (const auto& e : list) {
            write_string(out, src);
            write_string(out, e.dst);
            write_string(out, e.type);
            out.write(reinterpret_cast<const char*>(&e.weight), sizeof(double));
        }
    }
}

void GraphStore::save_synergy() const {
    std::ofstream out(path_join(root_, "synergy.bin"), std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("cannot write synergy");
    std::uint32_t count = 0;
    for (const auto& [a, row] : synergy_) {
        for (const auto& [b, _] : row) {
            if (a >= b) continue;
            ++count;
        }
    }
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& [a, row] : synergy_) {
        for (const auto& [b, s] : row) {
            if (a >= b) continue;
            write_string(out, a);
            write_string(out, b);
            out.write(reinterpret_cast<const char*>(&s.games), sizeof(int));
            out.write(reinterpret_cast<const char*>(&s.wins), sizeof(int));
        }
    }
}

void GraphStore::load_nodes() {
    std::ifstream in(path_join(root_, "nodes.bin"), std::ios::binary);
    if (!in) return;
    std::uint32_t count{};
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    for (std::uint32_t i = 0; i < count; ++i) {
        Node n;
        n.id = read_string(in);
        n.type = read_string(in);
        n.label = read_string(in);
        nodes_[n.id] = n;
    }
}

void GraphStore::load_edges() {
    std::ifstream in(path_join(root_, "edges.bin"), std::ios::binary);
    if (!in) return;
    std::uint32_t count{};
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    for (std::uint32_t i = 0; i < count; ++i) {
        Edge e;
        e.src = read_string(in);
        e.dst = read_string(in);
        e.type = read_string(in);
        in.read(reinterpret_cast<char*>(&e.weight), sizeof(double));
        add_edge(e);
    }
}

void GraphStore::load_synergy() {
    std::ifstream in(path_join(root_, "synergy.bin"), std::ios::binary);
    if (!in) return;
    std::uint32_t count{};
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    for (std::uint32_t i = 0; i < count; ++i) {
        std::string a = read_string(in);
        std::string b = read_string(in);
        SynergyStats s;
        in.read(reinterpret_cast<char*>(&s.games), sizeof(int));
        in.read(reinterpret_cast<char*>(&s.wins), sizeof(int));
        synergy_[a][b] = s;
        synergy_[b][a] = s;
    }
}

