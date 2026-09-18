#include "http_response.h"

namespace http_response
{
    // status is probably coming out as "HTTP/1.1 200 OK", so needs to be cleaned up
    bool get_response_status(const std::string &response, std::string &status_buffer)
    {
        size_t line_end = response.find("\r\n");
        if (line_end == std::string::npos)
            return false;

        std::string status_line = response.substr(0, line_end);

        size_t first_space = status_line.find(' '); // skip past "HTTP/1.1 "
        if (first_space == std::string::npos)
            return false;

        // should set to something like "200 OK"
        status_buffer = status_line.substr(first_space + 1);
        return true;
    }

    bool get_header(const std::string &response, const std::string &header_name, std::string &value_buffer)
    {
        // first occurrence of header
        size_t pos = response.find(header_name + ":");
        if (pos == std::string::npos)
            return false;

        size_t value_start = pos + header_name.length() + 1;
        size_t line_end = response.find("\r\n", value_start);
        if (line_end == std::string::npos)
            return false;

        value_buffer = response.substr(value_start, line_end - value_start);

        // trim leading whitespace until value field
        size_t first_non_space = value_buffer.find_first_not_of(' ');
        if (first_non_space != std::string::npos)
            value_buffer = value_buffer.substr(first_non_space);

        return true;
    }

    bool get_image_src(const std::string &response, std::string &src_buffer)
    {
        // looking for first instance of an img in the html
        size_t img_pos = response.find("<img");
        if (img_pos == std::string::npos)
        {
            return false;
        }

        size_t src_pos = response.find("src=", img_pos);
        if (src_pos == std::string::npos)
        {
            return false;
        }

        size_t value_start = src_pos + 4;
        if (value_start >= response.length())
        {
            return false;
        }

        // it could be a ' or " or even nothing, so need this dumb check
        char quote = response[value_start];
        if (quote == '"' || quote == '\'')
        {
            value_start++;
            size_t value_end = response.find(quote, value_start);
            if (value_end == std::string::npos)
            {
                return false;
            }

            src_buffer = response.substr(value_start, value_end - value_start);
        }
        else
        {
            // find_first_of searches for the first instance of EACH char
            // so it'll stop at either ' ' or '>'
            // it'll safely stop before any future attributes or element closure
            size_t value_end = response.find_first_of(" >", value_start);
            if (value_end == std::string::npos)
            {
                return false;
            }

            src_buffer = response.substr(value_start, value_end - value_start);
        }

        return true;
    }
}