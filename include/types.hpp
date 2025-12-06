#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct Player {
    std::string summoner_id;
    std::string name;
    std::string region;
};

struct Team {
    std::string team_id;
    std::string name;
};

struct Champion {
    std::int32_t key;
    std::string name;
};

struct ParticipantRecord {
    std::string player_id;
    std::int32_t champion_key{};
    std::string team_id;
    std::string role;
    bool win{};
};

struct MatchRecord {
    std::string match_id;
    std::string region;
    std::string queue;
    std::vector<ParticipantRecord> participants;
};

struct SynergyEdge {
    std::string a;
    std::string b;
    int games{};
    int wins{};
};

struct SynergyStats {
    int games{};
    int wins{};
    double win_rate() const { return games == 0 ? 0.0 : static_cast<double>(wins) / games; }
};

using SynergyMap = std::unordered_map<std::string, std::unordered_map<std::string, SynergyStats>>;

struct Node {
    std::string id;
    std::string type;
    std::string label;
};

struct Edge {
    std::string src;
    std::string dst;
    std::string type;
    double weight{1.0};
};

struct FetchConfig {
    std::string base_url;
    std::string region;
    std::string queue;
    int sample_pages{1};
    int sleep_ms{500};
    std::string user_agent{"lolgraph/0.1"};
};

