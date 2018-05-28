// MIT License
//
// Copyright (c) 2018 Lennart Braun
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <boost/asio/ip/tcp.hpp>
#include "connection.hpp"

enum class Role;

/**
 * Implementation of a bidirectional channel over tcp
 */
class TCPConnection : public Connection
{
public:
    /**
     * Constructor given a tcp socket
     */
    TCPConnection(boost::asio::ip::tcp::socket socket);
    ~TCPConnection() = default;

    TCPConnection(TCPConnection&&) = default;
    TCPConnection& operator=(TCPConnection&&) = default;

    /**
     * Dispatch to connect/listen according to role
     */
    static Conn_p from_role(Role role, boost::asio::io_context &io_service,
        std::string address, uint16_t port);
    /**
     * Connect to another party
     */
    static Conn_p connect(boost::asio::io_context& io_service,
            std::string address, uint16_t port);
    /**
     * Listen and wait for connection of another party
     */
    static Conn_p listen(boost::asio::io_context& io_service,
            std::string address, uint16_t port);

    /**
     * Send/receive message prefixed with its length.
     */
    virtual void send_message(const uint8_t * buffer, size_t length) override;
    virtual bytes_t recv_message() override;

    /**
     * Send/receive without length prefix.
     */
    void send(const uint8_t* buffer, size_t length) override;
    void recv(uint8_t* buffer, size_t length) override;

    std::future<size_t> async_send(const uint8_t* buffer, size_t length) override;
    std::future<size_t> async_recv(uint8_t* buffer, size_t length) override;
private:

    void send_length(size_t length);
    size_t recv_length();

    const static size_t header_size = 4;
    boost::asio::ip::tcp::socket socket_;
};

#endif // TCP_CONNECTION_HPP
