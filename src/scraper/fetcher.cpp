#include "scraper/fetcher.hpp"

#include <curl/curl.h>
#include <chrono>
#include <stdexcept>
#include <thread>

namespace {
size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* out = static_cast<std::string*>(userdata);
    out->append(ptr, size * nmemb);
    return size * nmemb;
}
} // namespace

namespace scraper {

std::string fetch_html(const std::string& url, const FetchConfig& cfg) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("failed to init curl");
    }

    std::string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, cfg.user_agent.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl perform failed");
    }
    curl_easy_cleanup(curl);

    std::this_thread::sleep_for(std::chrono::milliseconds(cfg.sleep_ms));
    return body;
}

} // namespace scraper

