#include <gtest/gtest.h>
#include "network/devnull_connection.hpp"
#include "ot/ot_hl17.hpp"

TEST(OT_HL17_Test, Receive0)
{
    DevNullConnection connection;
    OT_HL17 ot{connection};

    OT_HL17::Sender_State ss;
    OT_HL17::Receiver_State rs;

    int choice = 0;

    auto msg_s0 = ot.send_0(ss);
    ot.send_1(ss);
    ot.recv_0(rs, choice);
    auto msg_r1 = ot.recv_1(rs, msg_s0);

    auto res_s = ot.send_2(ss, msg_r1);
    auto res_r = ot.recv_2(rs);

    ASSERT_TRUE(res_r == res_s[choice]);
}

TEST(OT_HL17_Test, Receive1)
{
    DevNullConnection connection;
    OT_HL17 ot{connection};

    OT_HL17::Sender_State ss;
    OT_HL17::Receiver_State rs;

    int choice = 1;

    auto msg_s0 = ot.send_0(ss);
    ot.send_1(ss);
    ot.recv_0(rs, choice);
    auto msg_r1 = ot.recv_1(rs, msg_s0);

    auto res_s = ot.send_2(ss, msg_r1);
    auto res_r = ot.recv_2(rs);

    ASSERT_TRUE(res_r == res_s[choice]);
}
