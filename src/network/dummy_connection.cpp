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

#include <cassert>
#include <cstring>
#include "dummy_connection.hpp"

DummyConnection::DummyConnection(message_queue_t send_queue, message_queue_t recv_queue)
    : send_queue_(send_queue), recv_queue_(recv_queue) {}

DummyConnection::~DummyConnection() = default;

std::pair<Conn_p, Conn_p> DummyConnection::make_dummies()
{
    auto queue_12{std::make_shared<message_queue_t::element_type>()};
    auto queue_21{std::make_shared<message_queue_t::element_type>()};
    auto conn1{std::make_shared<DummyConnection>(queue_12, queue_21)};
    auto conn2{std::make_shared<DummyConnection>(queue_21, queue_12)};
    assert(conn1->send_queue_ == conn2->recv_queue_);
    assert(conn1->recv_queue_ == conn2->send_queue_);
    return {conn1, conn2};
}


void DummyConnection::send_message(const uint8_t *buffer, size_t size)
{
    send_queue_->enqueue(bytes_t(buffer, buffer + size));
}

bytes_t DummyConnection::recv_message()
{
    return recv_queue_->dequeue();
}

void DummyConnection::send(const uint8_t* buffer, size_t length)
{
    send_queue_->enqueue(bytes_t(buffer, buffer + length));
}
void DummyConnection::recv(uint8_t* buffer, size_t length)
{
    auto tmp = recv_queue_->dequeue();
    assert(tmp.size() == length);
    std::memcpy(buffer, tmp.data(), length);
}
