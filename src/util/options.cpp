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

#include <boost/algorithm/string.hpp>
#include <boost/program_options/errors.hpp>
#include "options.hpp"

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


std::istream& operator>>(std::istream &is, OT_Protocol &ot)
{
    std::string token;
    is >> token;
    boost::algorithm::to_lower(token);
    if (token == "co15" || token == "simpleot")
        ot = OT_Protocol::CO15;
    else if (token == "hl17")
        ot = OT_Protocol::HL17;
    else
        throw po::invalid_option_value(token);
    return is;
}

std::ostream& operator<<(std::ostream &os, const OT_Protocol &ot)
{
    switch (ot)
    {
        case OT_Protocol::CO15:
            os << "CO15";
            break;
        case OT_Protocol::HL17:
            os << "HL17";
            break;
    }
    return os;
}
