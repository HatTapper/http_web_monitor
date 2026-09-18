#include "http_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

namespace http_client
{
    // port arg is a string since addrinfo requires it
    // apparantely it's so it can handle text-based service names as well?
    addrinfo *resolve_host_name(const std::string &host_name, const std::string &port)
    {
        addrinfo info{};
        info.ai_family = AF_UNSPEC;     // accept either IPV4 or IPV6
        info.ai_socktype = SOCK_STREAM; // TCP connection via sockets

        addrinfo *result{};
        if (getaddrinfo(host_name.c_str(), port.c_str(), &info, &result) != 0)
            return nullptr;

        return result;
    }

    std::string build_http_request(const url_parser::url_data &data)
    {
        std::string request;
        request += "GET " + data.object_path + " HTTP/1.1\r\n";
        request += "Host: " + data.host_name + "\r\n";
        request += "Connection: close\r\n";
        request += "User-agent: Mozilla/5.0\r\n";
        request += "Accept-language: en\r\n";
        request += "\r\n";

        return request;
    }

    bool receive_all(int socket_descriptor, std::string &response_buffer)
    {
        char buffer[4096];
        ssize_t bytes_received;

        // spamming recv in the off-chance my buffer isn't big enough
        while ((bytes_received = recv(socket_descriptor, buffer, sizeof(buffer), 0)) > 0)
            response_buffer.append(buffer, bytes_received);

        return bytes_received >= 0;
    }

    // this does the full process of getting a response from the provided url
    bool fetch_url(const url_parser::url_data &data, std::string &response_buffer)
    {
        addrinfo *addr_result = resolve_host_name(data.host_name, data.port);
        if (!addr_result)
        {
            return false;
        }

        int socket_descriptor = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
        if (socket_descriptor < 0)
        {
            freeaddrinfo(addr_result);
            return false;
        }

        if (connect(socket_descriptor, addr_result->ai_addr, addr_result->ai_addrlen) < 0)
        {
            close(socket_descriptor);
            freeaddrinfo(addr_result);
            return false;
        }

        freeaddrinfo(addr_result);

        std::string request = build_http_request(data);
        ssize_t bytes_sent = send(socket_descriptor, request.c_str(), request.length(), 0);
        if (bytes_sent < 0)
        {
            close(socket_descriptor);
            return false;
        }

        bool success = receive_all(socket_descriptor, response_buffer);
        close(socket_descriptor);

        return success;
    }
}