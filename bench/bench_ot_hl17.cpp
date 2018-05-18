#include <benchmark/benchmark.h>
#include "network/devnull_connection.hpp"
#include "ot/ot_hl17.hpp"


static void BM_OT_HL17_no_network(benchmark::State& state) {
    DevNullConnection connection;
    OT_HL17 ot{connection};

    int choice = state.range(0);

    for (auto _ : state)
    {
        OT_HL17::Sender_State ss;
        OT_HL17::Receiver_State rs;

        auto msg_s0 = ot.send_0(ss);
        ot.send_1(ss);
        ot.recv_0(rs, choice);
        auto msg_r1 = ot.recv_1(rs, msg_s0);

        auto res_s = ot.send_2(ss, msg_r1);
        auto res_r = ot.recv_2(rs);
    }
}
BENCHMARK(BM_OT_HL17_no_network)->Arg(0)->Arg(1)->Unit(benchmark::kMicrosecond);

