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
