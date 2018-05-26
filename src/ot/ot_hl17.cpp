
#include <array>
#include <cassert>
#include <cstdint>
#include <botan/blake2b.h>
#include <botan/hex.h>
#include "ot_hl17.hpp"

#include <iostream>
#include <chrono>


OT_HL17::OT_HL17(Connection& connection) : connection_(connection)
{
}

// Notation
// * Group GG
// * of prime order p
// * with generator g
//
// * random oracle G: GG -> GG
// * random oracle H: GG^3 -> K

std::string hexlify(const void* buf, size_t nbytes)
{
    std::string hex(2 * nbytes, '?');
    Botan::hex_encode(hex.data(), reinterpret_cast<const uint8_t*>(buf), nbytes, false);
    return hex;
}

std::string hex_point(const curve25519::ge_p3& p)
{
    std::array<uint8_t, 32> buf;
    curve25519::ge_p3_tobytes(buf.data(), &p);
    return hexlify(buf.data(), buf.size());
}

void hash_point(curve25519::ge_p3& output, const curve25519::ge_p3& input)
{
    std::array<uint8_t, 32> hash_input;
    std::array<uint8_t, 64> hash_output{};
    auto hash(Botan::Blake2b(256));

    curve25519::ge_p3_tobytes(hash_input.data(), &input);
    hash.update(hash_input.data(), hash_input.size());
    hash.final(hash_output.data());
    curve25519::x25519_sc_reduce(hash_output.data());

    curve25519::x25519_ge_scalarmult_base(&output, hash_output.data());
}

void OT_HL17::send_0(Sender_State& state,
                     std::array<uint8_t, curve25519_ge_byte_size>& message_out)
{
    // sample y <- Zp
    curve25519::sc_random(state.y);

    // S = g^y
    curve25519::x25519_ge_scalarmult_base(&state.S, state.y);

    bytes_t S_bytes(curve25519_ge_byte_size);
    curve25519::ge_p3_tobytes(message_out.data(), &state.S);
}

void OT_HL17::send_1(Sender_State& state)
{
    // T = G(S)
    hash_point(state.T, state.S);
}

std::pair<bytes_t, bytes_t> OT_HL17::send_2(Sender_State& state,
                                            const std::array<uint8_t, curve25519_ge_byte_size>& message_in)
{
    // // assert R in GG
    if (!x25519_ge_frombytes_vartime(&state.R, message_in.data()))
        std::terminate();

    auto hash(Botan::Blake2b(128));

    // std::pair<bytes_t, bytes_t> output;
    auto output = std::make_pair<>(bytes_t(16), bytes_t(16));
    output.first.resize(hash.output_length());
    output.second.resize(hash.output_length());

    std::array<uint8_t, 3*curve25519_ge_byte_size> hash_input;
    curve25519::ge_p3_tobytes(hash_input.data(), &state.S);
    curve25519::ge_p3_tobytes(hash_input.data() + 32, &state.R);

    // j = 0
    // y*R
    curve25519::ge_p2 y_times_R_p2;
    curve25519::x25519_ge_scalarmult(&y_times_R_p2, state.y, &state.R);
    curve25519::x25519_ge_tobytes(hash_input.data() + 64, &y_times_R_p2);

    hash.update(hash_input.data(), hash_input.size());
    hash.final(reinterpret_cast<uint8_t*>(output.first.data()));


    // j = 1
    // y*R + (-y)*T = y*(R - T)
    {
        curve25519::ge_cached T_cached;
        curve25519::x25519_ge_p3_to_cached(&T_cached, &state.T);

        curve25519::ge_p1p1 R_minus_T_p1p1;
        curve25519::x25519_ge_sub(&R_minus_T_p1p1, &state.R, &T_cached);

        curve25519::ge_p3 R_minus_T_p3;
        curve25519::x25519_ge_p1p1_to_p3(&R_minus_T_p3, &R_minus_T_p1p1);

        curve25519::ge_p2 y_times_R_minus_T_p2;
        curve25519::x25519_ge_scalarmult(&y_times_R_minus_T_p2, state.y, &R_minus_T_p3);
        curve25519::x25519_ge_tobytes(hash_input.data() + 64, &y_times_R_minus_T_p2);

    }


    // y*R + -y*T
    // {
    //     curve25519::ge_p2 y_times_T_p2;
    //     curve25519::x25519_ge_scalarmult(&y_times_T_p2, state.y, &state.T);
    //
    //     curve25519::ge_p3 y_times_T_p3;
    //     curve25519::x25519_ge_p2_to_p3(&y_times_T_p3, &y_times_T_p2);
    //
    //     curve25519::ge_cached yT_cached;
    //     curve25519::x25519_ge_p3_to_cached(&yT_cached, &y_times_T_p3);
    //
    //     curve25519::ge_p3 y_times_R_p3;
    //     curve25519::x25519_ge_p2_to_p3(&y_times_R_p3, &y_times_R_p2);
    //
    //     curve25519::ge_p1p1 yR_minus_yT_p1p1;
    //     curve25519::x25519_ge_sub(&yR_minus_yT_p1p1, &y_times_R_p3, &yT_cached);
    //
    //     curve25519::ge_p2 yR_minus_yT_p2;
    //     curve25519::x25519_ge_p1p1_to_p2(&yR_minus_yT_p2, &yR_minus_yT_p1p1);
    //     curve25519::x25519_ge_tobytes(hash_input.data() + 64, &yR_minus_yT_p2);
    // }


    hash.update(hash_input.data(), hash_input.size());
    hash.final(reinterpret_cast<uint8_t*>(output.second.data()));

    return output;
}

void OT_HL17::recv_0(Receiver_State& state, bool choice)
{
    state.choice = choice;
    // sample x <- Zp
    curve25519::sc_random(state.x);
}


void OT_HL17::recv_1(Receiver_State& state,
                     std::array<uint8_t, curve25519_ge_byte_size>& message_out,
                     const std::array<uint8_t, curve25519_ge_byte_size>& message_in)
{
    // recv S
    auto res = curve25519::x25519_ge_frombytes_vartime(&state.S, message_in.data());
    // assert S in GG
    if (res == 0)
        std::terminate();

    // T = G(S)
    hash_point(state.T, state.S);

    // R = T^c * g^x

    // R = g^x
    curve25519::x25519_ge_scalarmult_base(&state.R, state.x);

    // XXX: not constant time
    if (state.choice == 1)
    {
        curve25519::ge_p1p1 R_p1p1;
        curve25519::ge_cached T_cached;
        curve25519::x25519_ge_p3_to_cached(&T_cached, &state.T);
        curve25519::x25519_ge_add(&R_p1p1, &state.R, &T_cached);
        curve25519::x25519_ge_p1p1_to_p3(&state.R, &R_p1p1);
    }

    bytes_t R_bytes(32);
    curve25519::ge_p3_tobytes(message_out.data(), &state.R);
}

bytes_t OT_HL17::recv_2(Receiver_State& state)
{
    // k_R = H_(S,R)(S^x)
    //     = H_(S,R)(g^xy)

    bytes_t hash_output(16);

    std::array<uint8_t, 3*32> hash_input;
    curve25519::ge_p3_tobytes(hash_input.data(), &state.S);
    curve25519::ge_p3_tobytes(hash_input.data() + 32, &state.R);

    curve25519::ge_p2 S_to_the_x;
    curve25519::x25519_ge_scalarmult(&S_to_the_x, state.x, &state.S);
    curve25519::x25519_ge_tobytes(hash_input.data() + 64, &S_to_the_x);


    auto hash(Botan::Blake2b(128));
    assert(hash_output.size() == hash.output_length());
    hash.update(hash_input.data(), hash_input.size());
    hash.final(reinterpret_cast<uint8_t*>(hash_output.data()));


    return hash_output;
}

std::pair<bytes_t, bytes_t> OT_HL17::send()
{
    Sender_State state;
    std::array<uint8_t, curve25519_ge_byte_size> msg_s0;
    std::array<uint8_t, curve25519_ge_byte_size> msg_r1;

    send_0(state, msg_s0);
    connection_.send(msg_s0.data(), msg_s0.size());
    send_1(state);
    connection_.recv(msg_r1.data(), msg_r1.size());
    return send_2(state, msg_r1);
}

bytes_t OT_HL17::recv(bool choice)
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


std::vector<std::pair<bytes_t, bytes_t>> OT_HL17::send(size_t number_ots)
{
    std::vector<Sender_State> states(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_s0(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_r1(number_ots);
    std::vector<std::pair<bytes_t, bytes_t>> output(number_ots);
    for (size_t i = 0; i < number_ots; ++i)
    {
        send_0(states[i], msgs_s0[i]);
    }
    connection_.send(reinterpret_cast<uint8_t*>(msgs_s0.data()), msgs_s0.size() * curve25519_ge_byte_size);
    for (size_t i = 0; i < number_ots; ++i)
    {
        send_1(states[i]);
    }
    connection_.recv(reinterpret_cast<uint8_t*>(msgs_r1.data()), msgs_r1.size() * curve25519_ge_byte_size);
    for (size_t i = 0; i < number_ots; ++i)
    {
        output[i] = send_2(states[i], msgs_r1[i]);
    }
    return output;
}

std::vector<bytes_t> OT_HL17::recv(std::vector<bool> choices)
{
    auto number_ots = choices.size();
    std::vector<Receiver_State> states(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_s0(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_r1(number_ots);
    std::vector<bytes_t> output(number_ots);
    for (size_t i = 0; i < number_ots; ++i)
    {
        recv_0(states[i], choices[i]);
    }
    connection_.recv(reinterpret_cast<uint8_t*>(msgs_s0.data()), msgs_s0.size() * curve25519_ge_byte_size);
    for (size_t i = 0; i < number_ots; ++i)
    {
        recv_1(states[i], msgs_r1[i], msgs_s0[i]);
    }
    connection_.send(reinterpret_cast<uint8_t*>(msgs_r1.data()), msgs_r1.size() * curve25519_ge_byte_size);
    for (size_t i = 0; i < number_ots; ++i)
    {
        output[i] = recv_2(states[i]);
    }
    return output;
}


std::string get_time(std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
    auto now = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
    return std::to_string(us);
}


std::vector<std::pair<bytes_t, bytes_t>> OT_HL17::async_send(size_t number_ots)
{
    std::vector<Sender_State> states(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_s0(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_r1(number_ots);
    std::vector<std::pair<bytes_t, bytes_t>> output(number_ots);

    auto t_start = std::chrono::high_resolution_clock::now();

    std::cerr << get_time(t_start) << " [S] compute send_0\n";
    for (size_t i = 0; i < number_ots; ++i)
    {
        send_0(states[i], msgs_s0[i]);
    }

    std::cerr << get_time(t_start) << " [S] async send msg_s0\n";
    auto fut_send_msg_s0 = connection_.async_send(reinterpret_cast<uint8_t*>(msgs_s0.data()), msgs_s0.size() * curve25519_ge_byte_size);
    std::cerr << get_time(t_start) << " [S] async recv msg_r1\n";
    auto fut_recv_msg_r1 = connection_.async_recv(reinterpret_cast<uint8_t*>(msgs_r1.data()), msgs_r1.size() * curve25519_ge_byte_size);


    std::cerr << get_time(t_start) << " [S] compute send_1\n";
    for (size_t i = 0; i < number_ots; ++i)
    {
        send_1(states[i]);
    }

    std::cerr << get_time(t_start) << " [S] wait for recv msg_r1\n";
    auto msg_r1_size = fut_recv_msg_r1.get();
    assert(msg_r1_size == msgs_r1.size() * curve25519_ge_byte_size);

    std::cerr << get_time(t_start) << " [S] compute send_2\n";
    for (size_t i = 0; i < number_ots; ++i)
    {
        output[i] = send_2(states[i], msgs_r1[i]);
    }

    std::cerr << get_time(t_start) << " [S] wait for send msg_s0\n";
    auto msg_s0_size = fut_send_msg_s0.get();
    assert(msg_s0_size == msgs_s0.size() * curve25519_ge_byte_size);

    std::cerr << get_time(t_start) << " [S] end\n";

    return output;
}

std::vector<bytes_t> OT_HL17::async_recv(std::vector<bool> choices)
{
    auto number_ots = choices.size();
    std::vector<Receiver_State> states(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_s0(number_ots);
    std::vector<std::array<uint8_t, curve25519_ge_byte_size>> msgs_r1(number_ots);
    std::vector<bytes_t> output(number_ots);

    auto t_start = std::chrono::high_resolution_clock::now();

    std::cerr << get_time(t_start) << " [R] async recv msg_s0\n";
    auto fut_recv_msg_s0 = connection_.async_recv(reinterpret_cast<uint8_t*>(msgs_s0.data()), msgs_s0.size() * curve25519_ge_byte_size);

    std::cerr << get_time(t_start) << " [R] compute recv_0\n";
    for (size_t i = 0; i < number_ots; ++i)
    {
        recv_0(states[i], choices[i]);
    }

    std::cerr << get_time(t_start) << " [R] wait for recv msg_s0\n";
    auto msg_s0_size = fut_recv_msg_s0.get();
    assert(msg_s0_size == msgs_s0.size() * curve25519_ge_byte_size);

    std::cerr << get_time(t_start) << " [R] compute recv_1\n";
    for (size_t i = 0; i < number_ots; ++i)
    {
        recv_1(states[i], msgs_r1[i], msgs_s0[i]);
    }

    std::cerr << get_time(t_start) << " [R] async send msg_r1\n";
    auto fut_send_msg_r1 = connection_.async_send(reinterpret_cast<uint8_t*>(msgs_r1.data()), msgs_r1.size() * curve25519_ge_byte_size);

    std::cerr << get_time(t_start) << " [R] compute recv_2\n";
    for (size_t i = 0; i < number_ots; ++i)
    {
        output[i] = recv_2(states[i]);
    }

    std::cerr << get_time(t_start) << " [R] wait for send msg_r1\n";
    auto msg_r1_size = fut_send_msg_r1.get();
    assert(msg_r1_size == msgs_s0.size() * curve25519_ge_byte_size);

    std::cerr << get_time(t_start) << " [R] end\n";

    return output;
}
