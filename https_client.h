#ifndef HTTPS_CLIENT_H
#define HTTPS_CLIENT_H

#include <string>
#include "url_parser.h"

// functionally the same as http_client, just has the extra internal SSL layer
// to handle the HTTPS context which has a bunch of extra steps that OpenSSL
// thankfully does for me!

namespace https_client
{
    bool fetch_url(const url_parser::url_data &data, std::string &response_buffer);
}

#endif