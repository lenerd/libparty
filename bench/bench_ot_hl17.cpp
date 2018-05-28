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

        std::array<uint8_t, OT_HL17::curve25519_ge_byte_size> msg_s0;
        std::array<uint8_t, OT_HL17::curve25519_ge_byte_size> msg_r1;

        ot.send_0(ss, msg_s0);
        ot.send_1(ss);
        ot.recv_0(rs, choice);
        ot.recv_1(rs, msg_r1, msg_s0);

        auto res_s = ot.send_2(ss, msg_r1);
        auto res_r = ot.recv_2(rs);
    }
}
BENCHMARK(BM_OT_HL17_no_network)->Arg(0)->Arg(1)->Unit(benchmark::kMicrosecond);

