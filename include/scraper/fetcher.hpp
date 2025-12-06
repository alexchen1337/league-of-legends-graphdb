#pragma once

#include "types.hpp"
#include <string>
#include <vector>

namespace scraper {

std::string fetch_html(const std::string& url, const FetchConfig& cfg);

} 

