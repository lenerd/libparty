#ifndef DEVNULL_CONNECTION_HPP
#define DEVNULL_CONNECTION_HPP

#include "connection.hpp"

/**
 * DevNull connection for testing purposes
 */
class DevNullConnection : public Connection
{
public:
    DevNullConnection() = default;
    virtual ~DevNullConnection() = default;

    DevNullConnection(DevNullConnection&&) = default;
    DevNullConnection& operator=(DevNullConnection&&) = default;

    virtual void send_message(const uint8_t*, size_t) override
    {
    }
    virtual bytes_t recv_message() override
    {
        return bytes_t{};
    }
};

#endif // DEVNULL_CONNECTION_HPP
