#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <cstdint>
#include <istream>
#include <ostream>


/**
 * A role in the client-server setting
 */
enum class Role
{
    client,
    server,
};

/**
 * iostream support for the Role and UIType enums
 */
std::istream& operator>>(std::istream &is, Role &role);
std::ostream& operator<<(std::ostream &os, const Role &role);

#endif // OPTIONS_HPP
