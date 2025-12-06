#include "storage/schema.hpp"

#include <stdexcept>

namespace storage {

void migrate(Database& db) {
    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS players(
            summoner_id TEXT PRIMARY KEY,
            name TEXT,
            region TEXT
        );
    )SQL");

    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS champions(
            key INTEGER PRIMARY KEY,
            name TEXT
        );
    )SQL");

    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS teams(
            team_id TEXT PRIMARY KEY,
            name TEXT
        );
    )SQL");

    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS matches(
            match_id TEXT PRIMARY KEY,
            region TEXT,
            queue TEXT
        );
    )SQL");

    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS participants(
            match_id TEXT,
            summoner_id TEXT,
            champion_key INTEGER,
            role TEXT,
            win INTEGER,
            PRIMARY KEY(match_id, summoner_id),
            FOREIGN KEY(match_id) REFERENCES matches(match_id)
        );
    )SQL");

    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS synergy_edges(
            a TEXT,
            b TEXT,
            games INTEGER,
            wins INTEGER,
            PRIMARY KEY(a, b)
        );
    )SQL");

    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS nodes(
            id TEXT PRIMARY KEY,
            type TEXT,
            label TEXT
        );
    )SQL");

    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS edges(
            src TEXT,
            dst TEXT,
            type TEXT,
            weight REAL,
            PRIMARY KEY(src, dst, type)
        );
    )SQL");
}

void upsert_player(Database& db, const Player& p) {
    sqlite3_stmt* stmt{};
    const char* sql = "INSERT INTO players(summoner_id,name,region) VALUES(?,?,?) "
                      "ON CONFLICT(summoner_id) DO UPDATE SET name=excluded.name, region=excluded.region;";
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare failed players");
    }
    sqlite3_bind_text(stmt, 1, p.summoner_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, p.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, p.region.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void upsert_champion(Database& db, const Champion& c) {
    sqlite3_stmt* stmt{};
    const char* sql = "INSERT INTO champions(key,name) VALUES(?,?) "
                      "ON CONFLICT(key) DO UPDATE SET name=excluded.name;";
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare failed champions");
    }
    sqlite3_bind_int(stmt, 1, c.key);
    sqlite3_bind_text(stmt, 2, c.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void upsert_team(Database& db, const Team& t) {
    sqlite3_stmt* stmt{};
    const char* sql = "INSERT INTO teams(team_id,name) VALUES(?,?) "
                      "ON CONFLICT(team_id) DO UPDATE SET name=excluded.name;";
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare failed teams");
    }
    sqlite3_bind_text(stmt, 1, t.team_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, t.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void insert_match(Database& db, const MatchRecord& match) {
    sqlite3_stmt* insert_match{};
    const char* sql_match = "INSERT OR IGNORE INTO matches(match_id,region,queue) VALUES(?,?,?);";
    if (sqlite3_prepare_v2(db.handle(), sql_match, -1, &insert_match, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare failed matches");
    }
    sqlite3_bind_text(insert_match, 1, match.match_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_match, 2, match.region.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insert_match, 3, match.queue.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(insert_match);
    sqlite3_finalize(insert_match);

    sqlite3_stmt* insert_part{};
    const char* sql_part = "INSERT OR REPLACE INTO participants(match_id,summoner_id,champion_key,role,win) "
                           "VALUES(?,?,?,?,?);";
    if (sqlite3_prepare_v2(db.handle(), sql_part, -1, &insert_part, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare failed participants");
    }
    for (const auto& p : match.participants) {
        sqlite3_bind_text(insert_part, 1, match.match_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insert_part, 2, p.player_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(insert_part, 3, p.champion_key);
        sqlite3_bind_text(insert_part, 4, p.role.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(insert_part, 5, p.win ? 1 : 0);
        sqlite3_step(insert_part);
        sqlite3_reset(insert_part);
    }
    sqlite3_finalize(insert_part);
}

SynergyStats load_synergy(Database& db, const std::string& a, const std::string& b) {
    sqlite3_stmt* stmt{};
    const char* sql = "SELECT games, wins FROM synergy_edges WHERE a=? AND b=?;";
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare failed synergy_edges");
    }
    sqlite3_bind_text(stmt, 1, a.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, b.c_str(), -1, SQLITE_TRANSIENT);
    SynergyStats stats{};
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        stats.games = sqlite3_column_int(stmt, 0);
        stats.wins = sqlite3_column_int(stmt, 1);
    }
    sqlite3_finalize(stmt);
    return stats;
}

void upsert_node(Database& db, const Node& node) {
    sqlite3_stmt* stmt{};
    const char* sql = "INSERT INTO nodes(id,type,label) VALUES(?,?,?) "
                      "ON CONFLICT(id) DO UPDATE SET type=excluded.type, label=excluded.label;";
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare failed nodes");
    }
    sqlite3_bind_text(stmt, 1, node.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, node.type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, node.label.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void insert_edge(Database& db, const Edge& edge) {
    sqlite3_stmt* stmt{};
    const char* sql = "INSERT INTO edges(src,dst,type,weight) VALUES(?,?,?,?) "
                      "ON CONFLICT(src,dst,type) DO UPDATE SET weight=excluded.weight;";
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("prepare failed edges");
    }
    sqlite3_bind_text(stmt, 1, edge.src.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, edge.dst.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, edge.type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, edge.weight);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

} // namespace storage

