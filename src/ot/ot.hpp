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

#ifndef OT_HPP
#define OT_HPP

#include <vector>
#include "util/util.hpp"

// forward declare boost::asio::threadpool
namespace boost { namespace asio {
    class thread_pool;
} }


class OT
{
public:
    virtual ~OT() = default;
    virtual void send(const std::vector<bytes_t>&) = 0;
    virtual bytes_t recv(size_t) = 0;
};

class RandomOT
{
public:
    virtual ~RandomOT() = default;

    /**
     * Send/receive for a single random OT.
     */
    virtual std::pair<bytes_t, bytes_t> send() = 0;
    virtual bytes_t recv(bool) = 0;

    /**
     * Send/receive parts of the random OT protocol (batch version).
     */
    virtual std::vector<std::pair<bytes_t, bytes_t>> send(size_t) = 0;
    virtual std::vector<bytes_t> recv(const std::vector<bool>&) = 0;
    /**
     * Parallelized version of batch send/receive.
     * These methods will create a new thread pool.
     */
    virtual std::vector<std::pair<bytes_t, bytes_t>> parallel_send(size_t, size_t number_threads);
    virtual std::vector<bytes_t> parallel_recv(const std::vector<bool>&, size_t number_threads);
    /**
     * Parallelized version of batch send/receive.
     * These methods will use the given thread pool.
     */
    virtual std::vector<std::pair<bytes_t, bytes_t>> parallel_send(size_t, size_t number_threads, boost::asio::thread_pool& thread_pool) = 0;
    virtual std::vector<bytes_t> parallel_recv(const std::vector<bool>&, size_t number_threads, boost::asio::thread_pool& thread_pool) = 0;
};


#endif // OT_HPP
