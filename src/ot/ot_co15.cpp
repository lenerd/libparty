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

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <boost/asio.hpp>
#include <botan/blake2b.h>
#include <botan/hex.h>
#include "ot_co15.hpp"
#include "util/threading.hpp"


OT_CO15::OT_CO15(Connection& connection) : connection_(connection)
{
}

// Notation
// * Group GG
// * of prime order p
// * with generator g
//
// * random oracle G: GG -> GG
// * random oracle H: GG^3 -> K


// void hash_point(curve25519::ge_p3& output, const curve25519::ge_p3& input)
// {
//     std::array<uint8_t, 32> hash_input;
//     std::array<uint8_t, 64> hash_output{};
//     auto hash(Botan::Blake2b(256));
//
//     curve25519::ge_p3_tobytes(hash_input.data(), &input);
//     hash.update(hash_input.data(), hash_input.size());
//     hash.final(hash_output.data());
//     curve25519::x25519_sc_reduce(hash_output.data());
//
//     curve25519::x25519_ge_scalarmult_base(&output, hash_output.data());
// }

void OT_CO15::send_0(Sender_State& state,
                     std::array<uint8_t, curve25519_ge_byte_size>& message_out)
{
    // sample a <- Zp
    curve25519::sc_random(state.a);

    // A = g^a
    curve25519::x25519_ge_scalarmult_base(&state.A, state.a);

    curve25519::ge_p3_tobytes(message_out.data(), &state.A);
}

std::pair<bytes_t, bytes_t> OT_CO15::send_1(Sender_State& state,
                                            const std::array<uint8_t, curve25519_ge_byte_size>& message_in)
{
    // assert B in GG
    if (!x25519_ge_frombytes_vartime(&state.B, message_in.data()))
        std::terminate();

    auto hash(Botan::Blake2b(128));

    auto output = std::make_pair<>(bytes_t(16), bytes_t(16));
    assert(output.first.size() == hash.output_length());
    assert(output.second.size() == hash.output_length());

    std::array<uint8_t, curve25519_ge_byte_size> hash_input;

    // j = 0:
    // a*B
    curve25519::ge_p2 a_times_B_p2;
    curve25519::x25519_ge_scalarmult(&a_times_B_p2, state.a, &state.B);
    curve25519::x25519_ge_tobytes(hash_input.data(), &a_times_B_p2);

    // H(a*B)
    hash.update(hash_input.data(), hash_input.size());
    hash.final(output.first.data());


    // j = 1:
    // a*(B-A)
    {
        curve25519::ge_cached A_cached;
        curve25519::x25519_ge_p3_to_cached(&A_cached, &state.A);

        curve25519::ge_p1p1 B_minus_A_p1p1;
        curve25519::x25519_ge_sub(&B_minus_A_p1p1, &state.B, &A_cached);

        curve25519::ge_p3 B_minus_A_p3;
        curve25519::x25519_ge_p1p1_to_p3(&B_minus_A_p3, &B_minus_A_p1p1);

        curve25519::ge_p2 a_times_B_minus_A_p2;
        curve25519::x25519_ge_scalarmult(&a_times_B_minus_A_p2, state.a, &B_minus_A_p3);
        curve25519::x25519_ge_tobytes(hash_input.data(), &a_times_B_minus_A_p2);

    }

    // H(a*(B - A))
    hash.update(hash_input.data(), hash_input.size());
    hash.final(output.second.data());

    return output;
}

void OT_CO15::recv_0(Receiver_State& state, bool choice)
{
    state.choice = choice;
    // sample b <- Zp
    curve25519::sc_random(state.b);
}


void OT_CO15::recv_1(Receiver_State& state,
                     std::array<uint8_t, curve25519_ge_byte_size>& message_out,
                     const std::array<uint8_t, curve25519_ge_byte_size>& message_in)
{
    // recv A
    auto res = curve25519::x25519_ge_frombytes_vartime(&state.A, message_in.data());
    // assert A in GG
    if (res == 0)
        std::terminate();

    curve25519::x25519_ge_scalarmult_base(&state.B, state.b);
    // FIXME: not constant time
    if (state.choice == 1)
    {
        curve25519::ge_p1p1 B_p1p1;
        curve25519::ge_cached A_cached;
        curve25519::x25519_ge_p3_to_cached(&A_cached, &state.A);
        curve25519::x25519_ge_add(&B_p1p1, &state.B, &A_cached);
        curve25519::x25519_ge_p1p1_to_p3(&state.B, &B_p1p1);
    }

    curve25519::ge_p3_tobytes(message_out.data(), &state.B);
}

bytes_t OT_CO15::recv_2(Receiver_State& state)
{
    // k_R = H_(b*A)

    bytes_t hash_output(16);

    std::array<uint8_t, curve25519_ge_byte_size> hash_input;

    curve25519::ge_p2 b_times_A;
    curve25519::x25519_ge_scalarmult(&b_times_A, state.b, &state.A);
    // curve25519::x25519_ge_tobytes(hash_input.data() + 64, &b_times_A);
    curve25519::x25519_ge_tobytes(hash_input.data(), &b_times_A);

    auto hash(Botan::Blake2b(128));
    assert(hash_output.size() == hash.output_length());
    hash.update(hash_input.data(), hash_input.size());
    hash.final(hash_output.data());

    return hash_output;
}


std::vector<std::pair<bytes_t, bytes_t>> OT_CO15::send(size_t number_ots)
{
    std::vector<Sender_State> states(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_s0(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_r1(number_ots);
    std::vector<std::pair<bytes_t, bytes_t>> output(number_ots);

    for (size_t i = 0; i < number_ots; ++i)
    {
        send_0(states[i], msgs_s0[i]);
    }

    auto fut_send_msg_s0 = connection_.async_send(reinterpret_cast<uint8_t*>(msgs_s0.data()), msgs_s0.size() * curve25519_ge_byte_size);
    auto fut_recv_msg_r1 = connection_.async_recv(reinterpret_cast<uint8_t*>(msgs_r1.data()), msgs_r1.size() * curve25519_ge_byte_size);

    auto msg_r1_size = fut_recv_msg_r1.get();
    assert(msg_r1_size == msgs_r1.size() * curve25519_ge_byte_size);

    for (size_t i = 0; i < number_ots; ++i)
    {
        output[i] = send_1(states[i], msgs_r1[i]);
    }

    auto msg_s0_size = fut_send_msg_s0.get();
    assert(msg_s0_size == msgs_s0.size() * curve25519_ge_byte_size);

    return output;
}

std::vector<bytes_t> OT_CO15::recv(const std::vector<bool>& choices)
{
    auto number_ots = choices.size();
    std::vector<Receiver_State> states(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_s0(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_r1(number_ots);
    std::vector<bytes_t> output(number_ots);

    auto fut_recv_msg_s0 = connection_.async_recv(reinterpret_cast<uint8_t*>(msgs_s0.data()), msgs_s0.size() * curve25519_ge_byte_size);

    for (size_t i = 0; i < number_ots; ++i)
    {
        recv_0(states[i], choices[i]);
    }

    auto msg_s0_size = fut_recv_msg_s0.get();
    assert(msg_s0_size == msgs_s0.size() * curve25519_ge_byte_size);

    for (size_t i = 0; i < number_ots; ++i)
    {
        recv_1(states[i], msgs_r1[i], msgs_s0[i]);
    }

    auto fut_send_msg_r1 = connection_.async_send(reinterpret_cast<uint8_t*>(msgs_r1.data()), msgs_r1.size() * curve25519_ge_byte_size);

    for (size_t i = 0; i < number_ots; ++i)
    {
        output[i] = recv_2(states[i]);
    }

    auto msg_r1_size = fut_send_msg_r1.get();
    assert(msg_r1_size == msgs_s0.size() * curve25519_ge_byte_size);

    return output;
}


std::pair<bytes_t, bytes_t> OT_CO15::send()
{
    Sender_State state;
    std::array<uint8_t, curve25519_ge_byte_size> msg_s0;
    std::array<uint8_t, curve25519_ge_byte_size> msg_r1;

    send_0(state, msg_s0);
    connection_.send(msg_s0.data(), msg_s0.size());
    connection_.recv(msg_r1.data(), msg_r1.size());
    return send_1(state, msg_r1);
}


bytes_t OT_CO15::recv(bool choice)
{
    Receiver_State state;
    std::array<uint8_t, curve25519_ge_byte_size> msg_s0;
    std::array<uint8_t, curve25519_ge_byte_size> msg_r1;

    recv_0(state, choice);
    connection_.recv(msg_s0.data(), msg_s0.size());
    recv_1(state, msg_r1, msg_s0);
    connection_.send(msg_r1.data(), msg_r1.size());
    return recv_2(state);
}


std::vector<std::pair<bytes_t, bytes_t>> OT_CO15::parallel_send(size_t number_ots, size_t number_threads, boost::asio::thread_pool& thread_pool)
{
    std::vector<Sender_State> states(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_s0(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_r1(number_ots);
    std::vector<std::pair<bytes_t, bytes_t>> output(number_ots);

    compute(thread_pool, number_ots, number_threads, [this, &states, &msgs_s0](size_t index){ send_0(states[index], msgs_s0[index]); });

    auto fut_send_msg_s0 = connection_.async_send(reinterpret_cast<uint8_t*>(msgs_s0.data()), msgs_s0.size() * curve25519_ge_byte_size);
    auto fut_recv_msg_r1 = connection_.async_recv(reinterpret_cast<uint8_t*>(msgs_r1.data()), msgs_r1.size() * curve25519_ge_byte_size);

    auto msg_r1_size = fut_recv_msg_r1.get();
    assert(msg_r1_size == msgs_r1.size() * curve25519_ge_byte_size);

    compute(thread_pool, number_ots, number_threads, [this, &states, &msgs_r1, &output](size_t index){ output[index] = send_1(states[index], msgs_r1[index]); });

    auto msg_s0_size = fut_send_msg_s0.get();
    assert(msg_s0_size == msgs_s0.size() * curve25519_ge_byte_size);

    return output;
}


std::vector<bytes_t> OT_CO15::parallel_recv(const std::vector<bool>& choices, size_t number_threads, boost::asio::thread_pool& thread_pool)
{
    auto number_ots = choices.size();
    std::vector<Receiver_State> states(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_s0(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_r1(number_ots);
    std::vector<bytes_t> output(number_ots);

    auto fut_recv_msg_s0 = connection_.async_recv(reinterpret_cast<uint8_t*>(msgs_s0.data()), msgs_s0.size() * curve25519_ge_byte_size);

    compute(thread_pool, number_ots, number_threads, [this, &states, &choices](size_t index){ recv_0(states[index], choices[index]); });

    auto msg_s0_size = fut_recv_msg_s0.get();
    assert(msg_s0_size == msgs_s0.size() * curve25519_ge_byte_size);

    compute(thread_pool, number_ots, number_threads, [this, &states, &msgs_r1, &msgs_s0](size_t index){ recv_1(states[index], msgs_r1[index], msgs_s0[index]); });

    auto fut_send_msg_r1 = connection_.async_send(reinterpret_cast<uint8_t*>(msgs_r1.data()), msgs_r1.size() * curve25519_ge_byte_size);

    compute(thread_pool, number_ots, number_threads, [this, &states, &output](size_t index){ output[index] = recv_2(states[index]); });

    auto msg_r1_size = fut_send_msg_r1.get();
    assert(msg_r1_size == msgs_s0.size() * curve25519_ge_byte_size);

    return output;
}
