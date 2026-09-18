#include <iostream>
#include "url_parser.h"

namespace url_parser
{
    // first time using constexpr!
    constexpr int HTTP_PORT = 80;
    constexpr int HTTPS_PORT = 443;

    std::string slice_url(const std::string &url, const char *slice_operand, size_t start, size_t &match_pos)
    {
        match_pos = url.find(slice_operand, start);

        if (match_pos == std::string::npos)
            return url.substr(start);

        return url.substr(start, match_pos - start);
    }

    void url_data_from_url(const std::string &url, url_data &data)
    {
        size_t match_pos;

        data.protocol = slice_url(url, "://", 0, match_pos);

        // more for myself to sanity check my code since urls.txt doesn't have any incorrectly formatted urls
        if (match_pos == std::string::npos)
        {
            std::cerr << "Failed to get protocol" << std::endl;
            std::exit(1);
        }

        size_t host_name_start_pos = match_pos + 3;

        data.host_name = slice_url(url, "/", host_name_start_pos, match_pos);
        if (match_pos == std::string::npos)
            data.object_path = "/";
        else
            data.object_path = url.substr(match_pos);

        // ports are set as described in the manual, https is 443, http is 80
        // addrinfo needs port as a string, so convert here
        data.port = (data.protocol == "https") ? std::to_string(HTTPS_PORT) : std::to_string(HTTP_PORT);
    }

    std::string resolve_url(const url_data &base, const std::string &src)
    {
        // skip processing if already absolute
        if (src.substr(0, 7) == "http://" || src.substr(0, 8) == "https://")
        {
            return src;
        }

        return base.protocol + "://" + base.host_name + src;
    }
}