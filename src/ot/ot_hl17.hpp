#ifndef OT_HL17_HPP
#define OT_HL17_HPP

#include "ot.hpp"
#include "network/connection.hpp"
#include "curve25519/mycurve25519.h"


class OT_HL17 : public RandomOT
{
public:
    OT_HL17(Connection& connection);
    // void send(const std::vector<bytes_t>&) override;

    std::pair<bytes_t, bytes_t> send() override;
    bytes_t recv(bool) override;

    std::vector<std::pair<bytes_t, bytes_t>> send(size_t) override;
    std::vector<bytes_t> recv(std::vector<bool>) override;
private:

    Connection& connection_;

public: // for testing
    struct Sender_State
    {
        // y
        uint8_t y[32];
        // // S
        curve25519::ge_p3 S;
        // // T
        curve25519::ge_p3 T;
        // // R
        curve25519::ge_p3 R;
    };
    struct Receiver_State
    {
        bool choice;
        // x
        uint8_t x[32];
        // S
        curve25519::ge_p3 S;
        // T
        curve25519::ge_p3 T;
        // R
        curve25519::ge_p3 R;
        // k_R
        // e_c
    };
    static const size_t curve25519_ge_byte_size = 32;
    void send_0(Sender_State& state,
                std::array<uint8_t, curve25519_ge_byte_size>& message_out);
    void send_1(Sender_State& state);
    std::pair<bytes_t, bytes_t> send_2(Sender_State& state,
                                       const std::array<uint8_t, curve25519_ge_byte_size>& message_in);

    void recv_0(Receiver_State& state, bool choice);
    void recv_1(Receiver_State& state,
                std::array<uint8_t, curve25519_ge_byte_size>& message_out,
                const std::array<uint8_t, curve25519_ge_byte_size>& message_in);
    bytes_t recv_2(Receiver_State& state);
};


#endif // OT_HL17_HPP
