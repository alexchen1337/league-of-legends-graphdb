#pragma once

#include "types.hpp"
#include <string>
#include <vector>

namespace scraper {

std::vector<Player> parse_players(const std::string& html, const std::string& region);
std::vector<MatchRecord> parse_matches(const std::string& html, const std::string& region, const std::string& queue);

} 
