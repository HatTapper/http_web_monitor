#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>

// handles dealing with the response buffer provided by the client
namespace http_response
{
    bool get_response_status(const std::string &response, std::string &status_buffer);

    // finds header with name header_name and puts value into value_buffer
    bool get_header(const std::string &response, const std::string &header_name, std::string &value_buffer);

    bool get_image_src(const std::string &response, std::string &src_buffer);
}

#endif