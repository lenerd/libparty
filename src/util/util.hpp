// MIT License
//
// Copyright (c) 2017 Lennart Braun
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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * Representation of a byte string
 */
using bytes_t = std::vector<uint8_t>;

/**
 * Generate n uniformly random bytes
 */
bytes_t random_bytes(size_t n);


/**
 * Encode bytes in hexadecimal representation
 */
std::string hexlify(const bytes_t &data, bool upper=false);
/**
 * Decode hexadecimal into bytes
 */
bytes_t unhexlify(const std::string &hex);
/**
 * Convert a string to bytes
 */
bytes_t string_to_bytes(const std::string &str);

#endif // UTIL_HPP
