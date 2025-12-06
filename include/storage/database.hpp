#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <functional>

class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void exec(const std::string& sql);
    void transaction(const std::function<void()>& fn);

    sqlite3* handle() { return db_; }

private:
    sqlite3* db_{};
};

