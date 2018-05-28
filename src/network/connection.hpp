// MIT License
//
// Copyright (c) 2017-2018 Lennart Braun
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

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <future>
#include <memory>
#include "util/util.hpp"

/**
 * Interface for a bidirectional channel
 */
class Connection
{
public:
    /**
     * Constructor
     */
    Connection() = default;
    /**
     * Destructor
     */
    virtual ~Connection() = default;
    /**
     * Move constructor
     */
    Connection(Connection&&) = default;
    /**
     * Move assignment
     */
    Connection& operator=(Connection&&) = default;

    /**
     * Send a message
     */
    virtual void send_message(const bytes_t &buffer)
    {
        send_message(buffer.data(), buffer.size());
    }
    /**
     * Send a message from a raw buffer
     */
    virtual void send_message(const uint8_t*, size_t) = 0;
    /**
     * Receive a message
     */
    virtual bytes_t recv_message() = 0;

    /**
     * Send/receive without length prefix.
     */
    virtual void send(const uint8_t* buffer, size_t length) = 0;
    virtual void recv(uint8_t* buffer, size_t length) = 0;
    virtual std::future<size_t> async_send(const uint8_t* buffer, size_t length)
    {
        std::promise<size_t> promise;
        send(buffer, length);
        promise.set_value(length);
        return promise.get_future();
    }
    virtual std::future<size_t> async_recv(uint8_t* buffer, size_t length)
    {
        std::promise<size_t> promise;
        recv(buffer, length);
        promise.set_value(length);
        return promise.get_future();
    }
};

using Conn_p = std::shared_ptr<Connection>;

#endif // CONNECTION_HPP
