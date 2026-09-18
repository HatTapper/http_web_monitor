#include "https_client.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

namespace https_client
{
    // same as in http_client
    addrinfo *resolve_host_name(const std::string &host_name, const std::string &port)
    {
        addrinfo hints{};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        addrinfo *result = nullptr;
        if (getaddrinfo(host_name.c_str(), port.c_str(), &hints, &result) != 0)
            return nullptr;

        return result;
    }

    // same as in http_client
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

    bool fetch_url(const url_parser::url_data &data, std::string &response_buffer)
    {
        addrinfo *addr_result = resolve_host_name(data.host_name, data.port);
        if (!addr_result)
            return false;

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

        // now the fun ssl stuff, used https://docs.openssl.org/3.0/man7/ssl/#synopsis as a reference

        // TLS_client_method() inits the context from a client context
        SSL_CTX *ssl_context = SSL_CTX_new(TLS_client_method());
        if (!ssl_context)
        {
            close(socket_descriptor);
            return false;
        }

        // ssl object
        SSL *ssl = SSL_new(ssl_context);
        if (!ssl)
        {
            SSL_CTX_free(ssl_context);
            close(socket_descriptor);
            return false;
        }

        // bind the ssl object to the socket
        SSL_set_fd(ssl, socket_descriptor);

        // need to tell the server what host name we want
        // with some servers hosting multiple domains, we need to specify
        // which as the client for best practice
        SSL_set_tlsext_host_name(ssl, data.host_name.c_str());

        if (SSL_connect(ssl) <= 0)
        {
            SSL_free(ssl);
            SSL_CTX_free(ssl_context);
            close(socket_descriptor);
            return false;
        }

        // sends it over SSL
        std::string request = build_http_request(data);
        int bytes_sent = SSL_write(ssl, request.c_str(), request.length());
        if (bytes_sent <= 0)
        {
            SSL_free(ssl);
            SSL_CTX_free(ssl_context);
            close(socket_descriptor);
            return false;
        }

        // receives via SSL
        char buffer[4096];
        int bytes_received;
        while ((bytes_received = SSL_read(ssl, buffer, sizeof(buffer))) > 0)
        {
            response_buffer.append(buffer, bytes_received);
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ssl_context);
        close(socket_descriptor);

        return true;
    }
}