#include "analysis/synergy.hpp"
#include "pipeline/ingest.hpp"
#include "storage/schema.hpp"

#include <cassert>
#include <iostream>

int main() {
    Database db(":memory:");
    storage::migrate(db);

    MatchRecord m;
    m.match_id = "m1";
    m.region = "na";
    m.queue = "solo";
    m.participants = {
        {"p1", 1, "top", true},
        {"p2", 2, "jg", true},
        {"p3", 3, "mid", false},
        {"p4", 4, "bot", false},
    };

    pipeline::ingest(db, {m});

    auto map = analysis::build_synergy_map(db);
    auto duos = analysis::top_duos(map, 3, 1);

    assert(!duos.empty());
    assert(duos[0].games == 1);
    std::cout << "sanity ok\n";
    return 0;
}

