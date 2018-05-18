#include "options.hpp"
#include <boost/program_options/errors.hpp>

namespace po = boost::program_options;


std::istream& operator>>(std::istream &is, Role &role)
{
    std::string token;
    is >> token;
    if (token == "server" || token == "0")
        role = Role::server;
    else if (token == "client" || token == "1")
        role = Role::client;
    else
        throw po::invalid_option_value(token);
    return is;
}

std::ostream& operator<<(std::ostream &os, const Role &role)
{
    switch (role)
    {
        case Role::server:
            os << "server";
            break;
        case Role::client:
            os << "client";
            break;
    }
    return os;
}
