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

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "tcp_connection.hpp"
#include "util/options.hpp"

using boost::asio::ip::tcp;


TCPConnection::TCPConnection(tcp::socket socket)
    : socket_(std::move(socket))
{
}


Conn_p TCPConnection::from_role(Role role, boost::asio::io_service &io_service,
        std::string address, uint16_t port)
{
    switch (role)
    {
        case Role::server:
            return listen(io_service, address, port);
        case Role::client:
            return connect(io_service, address, port);
    }
}

Conn_p TCPConnection::connect(boost::asio::io_service &io_service,
        std::string address, uint16_t port)
{
    tcp::socket socket(io_service);
    tcp::resolver resolver(socket.get_io_service());
    boost::asio::connect(socket, resolver.resolve({address, std::to_string(port)}));
    return std::make_shared<TCPConnection>(std::move(socket));
}

Conn_p TCPConnection::listen(boost::asio::io_service& io_service,
        std::string address, uint16_t port)
{
    tcp::socket socket(io_service);
    tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
    tcp::acceptor acceptor(socket.get_io_service(), endpoint);
    acceptor.accept(socket);
    return std::make_shared<TCPConnection>(std::move(socket));
}


void TCPConnection::send_length(size_t length)
{
    if (length > std::numeric_limits<uint32_t>::max())
        throw std::out_of_range("message too long");
    uint32_t header{htonl(static_cast<uint32_t>(length))};
    static_assert(sizeof(header) == header_size, "header size mismatch");
    boost::asio::write(socket_, boost::asio::buffer(&header, sizeof(header)));
}

size_t TCPConnection::recv_length()
{
    uint32_t header{0};
    static_assert(sizeof(header) == header_size, "header size mismatch");
    boost::asio::read(socket_, boost::asio::buffer(&header, sizeof(header)));
    return static_cast<size_t>(ntohl(header));
}

void TCPConnection::send_message(const uint8_t *buffer, size_t length)
{
    send_length(length);
    boost::asio::write(socket_, boost::asio::buffer(buffer, length));
}

bytes_t TCPConnection::recv_message()
{
    auto length{recv_length()};
    bytes_t buffer(length);
    boost::asio::read(socket_, boost::asio::buffer(buffer));
    return buffer;
}

void TCPConnection::send(const uint8_t* buffer, size_t length)
{
    boost::asio::write(socket_, boost::asio::buffer(buffer, length));
}

void TCPConnection::recv(uint8_t* buffer, size_t length)
{
    boost::asio::read(socket_, boost::asio::buffer(buffer, length));
}

std::future<size_t> TCPConnection::async_send(const uint8_t* buffer, size_t length)
{
    return boost::asio::async_write(socket_, boost::asio::buffer(buffer, length),
            boost::asio::use_future);
}
std::future<size_t> TCPConnection::async_recv(uint8_t* buffer, size_t length)
{
    return boost::asio::async_read(socket_, boost::asio::buffer(buffer, length), boost::asio::use_future);
}
