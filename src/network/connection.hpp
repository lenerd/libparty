#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <future>
#include <memory>
#include "util/util.hpp"

/**
 * Interface for a bidirectional channel
 */
class Connection
{
public:
    /**
     * Constructor
     */
    Connection() = default;
    /**
     * Destructor
     */
    virtual ~Connection() = default;
    /**
     * Move constructor
     */
    Connection(Connection&&) = default;
    /**
     * Move assignment
     */
    Connection& operator=(Connection&&) = default;

    /**
     * Send a message
     */
    virtual void send_message(const bytes_t &buffer)
    {
        send_message(buffer.data(), buffer.size());
    }
    /**
     * Send a message from a raw buffer
     */
    virtual void send_message(const uint8_t*, size_t) = 0;
    /**
     * Receive a message
     */
    virtual bytes_t recv_message() = 0;

    /**
     * Send/receive without length prefix.
     */
    virtual void send(const uint8_t* buffer, size_t length) = 0;
    virtual void recv(uint8_t* buffer, size_t length) = 0;
    virtual std::future<size_t> async_send(const uint8_t* buffer, size_t length)
    {
        std::promise<size_t> promise;
        send(buffer, length);
        promise.set_value(length);
        return promise.get_future();
    }
    virtual std::future<size_t> async_recv(uint8_t* buffer, size_t length)
    {
        std::promise<size_t> promise;
        recv(buffer, length);
        promise.set_value(length);
        return promise.get_future();
    }
};

using Conn_p = std::shared_ptr<Connection>;

#endif // CONNECTION_HPP
