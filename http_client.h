#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include "url_parser.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

// handles about everything pertaining to talking over the network, in an HTTP context

namespace http_client
{
    bool fetch_url(const url_parser::url_data &data, std::string &response_buffer);
}

#endif