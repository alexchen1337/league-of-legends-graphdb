#pragma once

#include "types.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class GraphStore {
public:
    explicit GraphStore(const std::string& root);

    void load();
    void save() const;

    void upsert_player(const Player& p);
    void upsert_team(const Team& t);
    void upsert_champion(const Champion& c);
    void insert_match(const MatchRecord& match);

    const std::unordered_map<std::string, Node>& nodes() const { return nodes_; }
    const std::unordered_map<std::string, std::vector<Edge>>& adjacency() const { return adjacency_; }
    const SynergyMap& synergy() const { return synergy_; }

private:
    std::string root_;
    std::unordered_map<std::string, Player> players_;
    std::unordered_map<std::string, Team> teams_;
    std::unordered_map<std::string, Champion> champions_;
    std::unordered_map<std::string, Node> nodes_;
    std::unordered_map<std::string, std::vector<Edge>> adjacency_;
    SynergyMap synergy_;

    void add_edge(const Edge& e);
    void save_nodes() const;
    void save_edges() const;
    void save_synergy() const;
    void load_nodes();
    void load_edges();
    void load_synergy();
};

