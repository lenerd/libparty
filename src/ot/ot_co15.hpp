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

#ifndef OT_CO15_HPP
#define OT_CO15_HPP

#include "ot.hpp"
#include "network/connection.hpp"
#include "curve25519/mycurve25519.h"


/**
 * A random OT implementation based on the SimpleOT protocol by Chou and
 * Orlandi (2015).
 * https://eprint.iacr.org/2015/267
 */
class OT_CO15 : public RandomOT
{
public:
    OT_CO15(Connection& connection);

    /**
     * Send/receive for a single random OT.
     */
    std::pair<bytes_t, bytes_t> send() override;
    bytes_t recv(bool) override;

    /**
     * Send/receive parts of the random OT protocol (batch version).
     */
    std::vector<std::pair<bytes_t, bytes_t>> send(size_t) override;
    std::vector<bytes_t> recv(const std::vector<bool>&) override;
    /**
     * Parallelized version of batch send/receive.
     * These methods will create a new thread pool.
     */
    using RandomOT::parallel_send;
    using RandomOT::parallel_recv;
    /**
     * Parallelized version of batch send/receive.
     * These methods will use the given thread pool.
     */
    std::vector<std::pair<bytes_t, bytes_t>> parallel_send(size_t, size_t number_threads, boost::asio::thread_pool& thread_pool) override;
    std::vector<bytes_t> parallel_recv(const std::vector<bool>&, size_t number_threads, boost::asio::thread_pool& thread_pool) override;
private:

    Connection& connection_;

public: // for testing
    struct Sender_State
    {
        // a
        uint8_t a[32];
        // A
        curve25519::ge_p3 A;
        // B
        curve25519::ge_p3 B;
    };
    struct Receiver_State
    {
        bool choice;
        // b
        uint8_t b[32];
        // A
        curve25519::ge_p3 A;
        // B
        curve25519::ge_p3 B;
    };
    static const size_t curve25519_ge_byte_size = 32;

    /**
     * Parts of the sender side.
     */
    void send_0(Sender_State& state,
                std::array<uint8_t, curve25519_ge_byte_size>& message_out);
    std::pair<bytes_t, bytes_t> send_1(Sender_State& state,
                                       const std::array<uint8_t, curve25519_ge_byte_size>& message_in);

    /**
     * Parts of the receiver side.
     */
    void recv_0(Receiver_State& state, bool choice);
    void recv_1(Receiver_State& state,
                std::array<uint8_t, curve25519_ge_byte_size>& message_out,
                const std::array<uint8_t, curve25519_ge_byte_size>& message_in);
    bytes_t recv_2(Receiver_State& state);
};


#endif // OT_CO15_HPP
