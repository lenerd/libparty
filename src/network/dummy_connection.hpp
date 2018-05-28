// MIT License
//
// Copyright (c) 2017 Lennart Braun
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

#ifndef DUMMY_CONNECTION_HPP
#define DUMMY_CONNECTION_HPP

#include "connection.hpp"
#include "util/queue.hpp"

/**
 * Dummy connection for testing purposes
 */
class DummyConnection : public Connection
{
public:
    using message_queue_t = std::shared_ptr<Queue<bytes_t>>;

    /**
     * Create a pair of connection objects that are connection to each other
     */
    static std::pair<Conn_p, Conn_p> make_dummies();

    DummyConnection(message_queue_t send_queue, message_queue_t recv_queue);
    virtual ~DummyConnection();

    DummyConnection(DummyConnection&&) = default;
    DummyConnection& operator=(DummyConnection&&) = default;

    virtual void send_message(const uint8_t*, size_t) override;
    virtual bytes_t recv_message() override;
    virtual void send(const uint8_t* buffer, size_t length) override;
    virtual void recv(uint8_t* buffer, size_t length) override;

// private:
    message_queue_t send_queue_;
    message_queue_t recv_queue_;
};

#endif // DUMMY_CONNECTION_HPP
