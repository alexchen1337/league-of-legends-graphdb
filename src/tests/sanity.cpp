#include "analysis/synergy.hpp"
#include "pipeline/ingest.hpp"
#include "storage/database.hpp"

#include <cassert>
#include <iostream>

int main() {
    GraphStore store("testdata");
    store.load();

    MatchRecord m;
    m.match_id = "m1";
    m.region = "na";
    m.queue = "solo";
    m.participants = {
        {"p1", 1, "t1", "top", true},
        {"p2", 2, "t1", "jg", true},
        {"p3", 3, "t2", "mid", false},
        {"p4", 4, "t2", "bot", false},
    };

    pipeline::ingest(store, {m});

    auto map = analysis::build_synergy_map(store);
    auto duos = analysis::top_duos(map, 3, 1);

    assert(!duos.empty());
    assert(duos[0].games == 1);
    auto adj = analysis::build_adjacency(store);
    auto path = analysis::bfs_path(adj, "t1", "m1");
    assert(!path.empty());
    store.save();
    std::cout << "sanity ok\n";
    return 0;
}

