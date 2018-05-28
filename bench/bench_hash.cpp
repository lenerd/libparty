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
#include <botan/blake2b.h>
#include "curve25519/util.h"


static void BM_Hash_Blake_copy_state(benchmark::State& state)
{
    std::array<uint8_t, 3*32> hash_input;
    std::array<uint8_t, 16> hash_output_1;
    std::array<uint8_t, 16> hash_output_2;

    random_bytes(hash_input.data(), hash_input.size());

    for (auto _ : state)
    {
        auto hash = Botan::Blake2b(128);
        hash.update(hash_input.data(), hash_input.size() - 32);
        auto hash2 = hash.copy_state();
        hash.final(hash_output_1.data());
        hash2->update(hash_input.data() + hash_input.size() - 32, 32);
        hash2->final(hash_output_2.data());
    }
}
BENCHMARK(BM_Hash_Blake_copy_state);


static void BM_Hash_Blake_clear(benchmark::State& state)
{
    std::array<uint8_t, 3*32> hash_input;
    std::array<uint8_t, 16> hash_output_1;
    std::array<uint8_t, 16> hash_output_2;

    random_bytes(hash_input.data(), hash_input.size());

    for (auto _ : state)
    {
        auto hash = Botan::Blake2b(128);
        hash.update(hash_input.data(), hash_input.size() - 32);
        hash.final(hash_output_1.data());
        // hash.clear();  // already done by final
        hash.update(hash_input.data(), hash_input.size());
        hash.final(hash_output_2.data());
    }
}
BENCHMARK(BM_Hash_Blake_clear);
