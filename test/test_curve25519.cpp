#include "curve25519/mycurve25519.h"

#include <gtest/gtest.h>

TEST(Curve25519_Test, ToFromBytesIdentity)
{
    int ret;
    std::array<uint8_t, 32> buf0;
    std::array<uint8_t, 32> buf1;
    std::array<uint8_t, 32> buf2;

    curve25519::ge_p3 p3;
    curve25519::ge_p3_0(&p3);
    curve25519::ge_p2 p2;
    curve25519::ge_p2_0(&p2);
    curve25519::ge_p3_tobytes(buf0.data(), &p3);
    curve25519::x25519_ge_tobytes(buf1.data(), &p2);
    ASSERT_EQ(buf0, buf1);

    curve25519::ge_p3 p3_read;
    ret = curve25519::x25519_ge_frombytes_vartime(&p3_read, buf0.data());
    ASSERT_EQ(ret, 1);
    curve25519::ge_p3_tobytes(buf2.data(), &p3_read);
    ASSERT_EQ(buf0, buf2);
}

TEST(Curve25519_Test, ToFromBytesRandom)
{
    std::array<uint8_t, 32> sc;
    curve25519::sc_random(sc.data());

    curve25519::ge_p3 S;
    curve25519::x25519_ge_scalarmult_base(&S, sc.data());

    std::array<uint8_t, 32> buf0;
    curve25519::ge_p3_tobytes(buf0.data(), &S);

    curve25519::ge_p3 S_read;
    curve25519::x25519_ge_frombytes_vartime(&S_read, buf0.data());

    std::array<uint8_t, 32> buf1;
    curve25519::ge_p3_tobytes(buf1.data(), &S_read);

    ASSERT_EQ(buf0, buf1);
}
