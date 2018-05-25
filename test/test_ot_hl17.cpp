#include <future>
#include <gtest/gtest.h>
#include "network/devnull_connection.hpp"
#include "network/dummy_connection.hpp"
#include "ot/ot_hl17.hpp"

TEST(OT_HL17_Test, SR0)
{
    DevNullConnection connection;
    OT_HL17 ot{connection};

    OT_HL17::Sender_State ss;
    OT_HL17::Receiver_State rs;

    std::array<uint8_t, OT_HL17::curve25519_ge_byte_size> msg_s0;
    std::array<uint8_t, OT_HL17::curve25519_ge_byte_size> msg_r1;

    int choice = 0;

    ot.send_0(ss, msg_s0);
    ot.send_1(ss);
    ot.recv_0(rs, choice);
    ot.recv_1(rs, msg_r1, msg_s0);

    auto res_s = ot.send_2(ss, msg_r1);
    auto res_r = ot.recv_2(rs);

    ASSERT_EQ(res_r, res_s.first);
}


TEST(OT_HL17_Test, SR1)
{
    DevNullConnection connection;
    OT_HL17 ot{connection};

    OT_HL17::Sender_State ss;
    OT_HL17::Receiver_State rs;

    std::array<uint8_t, OT_HL17::curve25519_ge_byte_size> msg_s0;
    std::array<uint8_t, OT_HL17::curve25519_ge_byte_size> msg_r1;

    int choice = 0;

    ot.send_0(ss, msg_s0);
    ot.send_1(ss);
    ot.recv_0(rs, choice);
    ot.recv_1(rs, msg_r1, msg_s0);

    auto res_s = ot.send_2(ss, msg_r1);
    auto res_r = ot.recv_2(rs);

    ASSERT_EQ(res_r, res_s.first);
}


TEST(OT_HL17_Test, SRConnection0)
{
    auto conn_pair = DummyConnection::make_dummies();
    OT_HL17 ot_sender{*conn_pair.first};
    OT_HL17 ot_receiver{*conn_pair.second};

    int choice = 0;

    auto fut_s_1{std::async(std::launch::async,
        [this, &ot_sender]
        {
            return ot_sender.send();
        })};

    auto fut_r_1{std::async(std::launch::async,
        [this, &ot_receiver, choice]
        {
            return ot_receiver.recv(choice);
        })};
    auto out_s{fut_s_1.get()};
    auto out_r{fut_r_1.get()};

    ASSERT_EQ(out_r, out_s.first);
}

TEST(OT_HL17_Test, SRConnection1)
{
    auto conn_pair = DummyConnection::make_dummies();
    OT_HL17 ot_sender{*conn_pair.first};
    OT_HL17 ot_receiver{*conn_pair.second};

    int choice = 1;

    auto fut_s_1{std::async(std::launch::async,
        [this, &ot_sender]
        {
            return ot_sender.send();
        })};

    auto fut_r_1{std::async(std::launch::async,
        [this, &ot_receiver, choice]
        {
            return ot_receiver.recv(choice);
        })};
    auto out_s{fut_s_1.get()};
    auto out_r{fut_r_1.get()};

    ASSERT_EQ(out_r, out_s.second);
}
