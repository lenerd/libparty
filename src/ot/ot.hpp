#ifndef OT_HPP
#define OT_HPP

#include <vector>
#include "util/util.hpp"


class OT
{
public:
    virtual void send(const std::vector<bytes_t>&) = 0;
    virtual bytes_t recv(size_t) = 0;
};

class RandomOT
{
public:
    virtual std::pair<bytes_t, bytes_t> send() = 0;
    virtual bytes_t recv(bool) = 0;

    // batch
    virtual std::vector<std::pair<bytes_t, bytes_t>> send(size_t) = 0;
    virtual std::vector<bytes_t> recv(std::vector<bool>) = 0;
};


#endif // OT_HPP
