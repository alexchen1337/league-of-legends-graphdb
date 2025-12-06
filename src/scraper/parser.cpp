#include "scraper/parser.hpp"

#include <regex>
#include <sstream>

#ifdef LOL_USE_GUMBO
#include <gumbo.h>
#endif

namespace scraper {

namespace {

// naive helper for when Gumbo is unavailable
std::vector<std::string> extract_matches_regex(const std::string& html, const std::regex& re, int group) {
    std::vector<std::string> out;
    std::sregex_iterator it(html.begin(), html.end(), re);
    std::sregex_iterator end;
    for (; it != end; ++it) {
        if (group < static_cast<int>(it->size())) {
            out.push_back((*it)[group].str());
        }
    }
    return out;
}

#ifdef LOL_USE_GUMBO
void traverse_players(GumboNode* node, const std::string& region, std::vector<Player>& players) {
    if (node->type != GUMBO_NODE_ELEMENT) return;
    if (node->v.element.tag == GUMBO_TAG_A) {
        const char* href = nullptr;
        const GumboVector* attrs = &node->v.element.attributes;
        for (unsigned i = 0; i < attrs->length; ++i) {
            auto* attr = static_cast<GumboAttribute*>(attrs->data[i]);
            if (std::string(attr->name) == "href") href = attr->value;
        }
        if (href && std::string(href).find("/summoners/") != std::string::npos) {
            std::string name;
            if (node->v.element.children.length > 0) {
                GumboNode* txt = static_cast<GumboNode*>(node->v.element.children.data[0]);
                if (txt->type == GUMBO_NODE_TEXT) name = txt->v.text.text;
            }
            if (!name.empty()) {
                Player p;
                p.name = name;
                p.region = region;
                p.summoner_id = name; // fallback when no id in URL
                players.push_back(p);
            }
        }
    }
    const GumboVector* children = &node->v.element.children;
    for (unsigned i = 0; i < children->length; ++i) {
        traverse_players(static_cast<GumboNode*>(children->data[i]), region, players);
    }
}
#endif

} // namespace

std::vector<Player> parse_players(const std::string& html, const std::string& region) {
#ifdef LOL_USE_GUMBO
    GumboOutput* output = gumbo_parse(html.c_str());
    std::vector<Player> players;
    traverse_players(output->root, region, players);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    return players;
#else
    std::regex re(R"(summoner/([^\"/\?]+))");
    auto ids = extract_matches_regex(html, re, 1);
    std::vector<Player> players;
    for (const auto& id : ids) {
        Player p;
        p.summoner_id = id;
        p.name = id;
        p.region = region;
        players.push_back(p);
    }
    return players;
#endif
}

std::vector<MatchRecord> parse_matches(const std::string& html, const std::string& region, const std::string& queue) {
    // This is a heuristic parser; real pages will need more selectors.
    std::regex game_re(R"(data-game-id=\"([^\"]+)\")");
    std::regex player_re(R"(data-summoner-id=\"([^\"]+)\".*?data-champion-key=\"([0-9]+)\".*?data-team-win=\"([01]))");

    auto game_ids = extract_matches_regex(html, game_re, 1);
    std::vector<MatchRecord> matches;
    for (const auto& gid : game_ids) {
        MatchRecord m;
        m.match_id = gid;
        m.region = region;
        m.queue = queue;
        matches.push_back(m);
    }

    std::sregex_iterator it(html.begin(), html.end(), player_re);
    std::sregex_iterator end;
    size_t match_idx = 0;
    while (it != end && match_idx < matches.size()) {
        ParticipantRecord pr;
        pr.player_id = (*it)[1].str();
        pr.champion_key = std::stoi((*it)[2].str());
        pr.win = (*it)[3] == "1";
        pr.role = "unknown";
        matches[match_idx].participants.push_back(pr);
        ++it;
        if (matches[match_idx].participants.size() >= 10) {
            ++match_idx;
        }
    }
    return matches;
}

} // namespace scraper

