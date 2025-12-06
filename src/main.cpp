#include "analysis/synergy.hpp"
#include "pipeline/ingest.hpp"
#include "scraper/fetcher.hpp"
#include "scraper/parser.hpp"
#include "storage/schema.hpp"
#include "types.hpp"

#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: lolgraph <db_path> [region queue url]\n";
        return 1;
    }

    std::string db_path = argv[1];
    std::string region = argc > 2 ? argv[2] : "na";
    std::string queue = argc > 3 ? argv[3] : "solo";
    std::string url = argc > 4 ? argv[4] : "";

    try {
        Database db(db_path);
        storage::migrate(db);

        if (!url.empty()) {
            FetchConfig cfg;
            cfg.region = region;
            cfg.queue = queue;
            std::string html = scraper::fetch_html(url, cfg);
            auto matches = scraper::parse_matches(html, region, queue);
            pipeline::ingest(db, matches);
            std::cout << "ingested " << matches.size() << " matches\n";
        }

        auto map = analysis::build_synergy_map(db);
        auto duos = analysis::top_duos(map, 10);
        for (const auto& e : duos) {
            double wr = e.games == 0 ? 0.0 : static_cast<double>(e.wins) / e.games;
            std::cout << e.a << " + " << e.b << " games=" << e.games << " winrate=" << wr << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

