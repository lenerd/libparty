
#include <array>
#include <cassert>
#include <cstdint>
#include <botan/blake2b.h>
#include <botan/hex.h>
#include "ot_hl17.hpp"

#include <iostream>


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

bytes_t OT_HL17::send_0(Sender_State& state)
{
    // sample y <- Zp
    curve25519::sc_random(state.y);

    // S = g^y
    curve25519::x25519_ge_scalarmult_base(&state.S, state.y);

    bytes_t S_bytes(32);
    curve25519::ge_p3_tobytes(reinterpret_cast<uint8_t*>(S_bytes.data()), &state.S);
    return S_bytes;
}

void OT_HL17::send_1(Sender_State& state)
{
    // T = G(S)
    hash_point(state.T, state.S);
}
std::vector<bytes_t> OT_HL17::send_2(Sender_State& state, const bytes_t& message)
{
    // // assert R in GG
    if (!x25519_ge_frombytes_vartime(&state.R, reinterpret_cast<const uint8_t*>(message.data())))
        std::terminate();


    state.ks.resize(2);

    auto hash(Botan::Blake2b(128));
    std::array<uint8_t, 3*32> hash_input;
    curve25519::ge_p3_tobytes(hash_input.data(), &state.S);
    curve25519::ge_p3_tobytes(hash_input.data() + 32, &state.R);

    // j = 0
    // y*R
    curve25519::ge_p2 y_times_R_p2;
    curve25519::x25519_ge_scalarmult(&y_times_R_p2, state.y, &state.R);
    curve25519::x25519_ge_tobytes(hash_input.data() + 64, &y_times_R_p2);

    state.ks[0].resize(hash.output_length());
    hash.update(hash_input.data(), hash_input.size());
    hash.final(reinterpret_cast<uint8_t*>(state.ks[0].data()));


    hash.clear();


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


    state.ks[1].resize(hash.output_length());
    hash.update(hash_input.data(), hash_input.size());
    hash.final(reinterpret_cast<uint8_t*>(state.ks[1].data()));

    return state.ks;
}

std::vector<bytes_t> OT_HL17::send()
{
    Sender_State state;
    auto msg_s0 = send_0(state);
    connection_.send_message(msg_s0);
    send_1(state);
    auto msg_r1 = connection_.recv_message();
    return send_2(state, msg_r1);
}

void OT_HL17::recv_0(Receiver_State& state, int choice)
{
    state.choice = choice;
    // sample x <- Zp
    curve25519::sc_random(state.x);
}

bytes_t OT_HL17::recv_1(Receiver_State& state, const bytes_t& message)
{
    // recv S
    auto res = curve25519::x25519_ge_frombytes_vartime(&state.S, reinterpret_cast<const uint8_t*>(message.data()));
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
    curve25519::ge_p3_tobytes(reinterpret_cast<uint8_t*>(R_bytes.data()), &state.R);

    // send R
    return R_bytes;
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

bytes_t OT_HL17::recv(size_t choice)
{
    Receiver_State state;
    recv_0(state, choice);
    auto msg_s0 = connection_.recv_message();
    auto msg_r1 = recv_1(state, msg_s0);
    return recv_2(state);
}

