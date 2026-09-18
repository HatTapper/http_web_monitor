#ifndef URL_PARSER_H
#define URL_PARSER_H

#include <string>

// helper for splitting a url string into its necessary components for the http client
namespace url_parser
{
    struct url_data
    {
        std::string protocol;
        std::string host_name;
        std::string object_path;
        std::string port;
    };

    void url_data_from_url(const std::string &url, url_data &data);

    std::string resolve_url(const url_data &base, const std::string &src);
}

#endif