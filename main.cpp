#include <iostream>
#include <string>
#include <fstream>

#include "url_parser.h"
#include "http_client.h"
#include "https_client.h"
#include "http_response.h"

// HTTP status codes
const std::string RESPONSE_MOVED_PERMANENT = "301";
const std::string RESPONSE_MOVED_TEMPORARY = "302";

// saves the inline hassle of running the correct fetch_url call according to the
// protocol of the url_data
bool fetch_with_protocol(const url_parser::url_data &parsed_url, std::string &response_buffer)
{
    if (parsed_url.protocol == "https")
    {
        return https_client::fetch_url(parsed_url, response_buffer);
    }
    else
    {
        return http_client::fetch_url(parsed_url, response_buffer);
    }
}

void print_network_error(const std::string &url)
{
    std::cout << "URL: " << url << std::endl;
    std::cout << "Status: Network Error" << std::endl
              << std::endl; // needs extra endl for a linebreak to fit expected output
}

void process_object_data(const url_parser::url_data &page_data, const std::string &response)
{
    std::string image_src;
    if (!http_response::get_image_src(response, image_src))
    {
        // failed to get img element
        return;
    }

    // image_src will either be a relative or absolute path, so force absolute path
    std::string image_url = url_parser::resolve_url(page_data, image_src);

    url_parser::url_data image_url_data{};
    url_parser::url_data_from_url(image_url, image_url_data);

    std::string image_response;
    if (!fetch_with_protocol(image_url_data, image_response))
    {
        print_network_error(image_url);
        return;
    }

    std::string image_status;
    if (!http_response::get_response_status(image_response, image_status))
    {
        print_network_error(image_url);
        return;
    }

    std::cout << "Referenced URL: " << image_url << std::endl;
    std::cout << "Status: " << image_status << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Missing arg" << std::endl;
        std::exit(1);
    }

    const char *file_name = argv[1];

    std::ifstream text_file(file_name);
    if (!text_file.is_open())
    {
        std::cerr << "Failed to open file" << std::endl;
        std::exit(1);
    }

    std::string url;
    while (std::getline(text_file, url))
    {
        url_parser::url_data parsed_url{};
        url_parser::url_data_from_url(url, parsed_url);

        std::string response;
        if (!fetch_with_protocol(parsed_url, response))
        {
            print_network_error(url);
            continue;
        }

        std::string status;
        if (!http_response::get_response_status(response, status))
        {
            print_network_error(url);
            continue;
        }

        // hooray! we successfully got the URL and status!
        std::cout << "URL: " << url << std::endl;
        std::cout << "Status: " << status << std::endl;

        process_object_data(parsed_url, response);

        // after getting the expected url, need to check if redirected
        if (status.substr(0, 3) == RESPONSE_MOVED_PERMANENT || status.substr(0, 3) == RESPONSE_MOVED_TEMPORARY)
        {
            std::string location_url;
            if (!http_response::get_header(response, "Location", location_url))
            {
                continue;
            }

            // need to redo the previous steps to connect to the new redirect url
            url_parser::url_data redirected_url_data{};
            url_parser::url_data_from_url(location_url, redirected_url_data);

            std::string redirected_response;
            if (!fetch_with_protocol(redirected_url_data, redirected_response))
            {
                print_network_error(location_url);
                continue;
            }

            std::string redirected_status;
            if (!http_response::get_response_status(redirected_response, redirected_status))
            {
                print_network_error(location_url);
                continue;
            }

            // hooray! we successfully got the redirected URL and status!
            std::cout << "Redirected URL: " << location_url << std::endl;
            std::cout << "Status: " << redirected_status << std::endl;
        }

        // line break according to expected output
        std::cout << std::endl;
    }

    // cleanup
    text_file.close();

    return 0;
}