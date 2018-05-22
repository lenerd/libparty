#include <benchmark/benchmark.h>
#include <botan/blake2b.h>
#include "util/util.h"


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
