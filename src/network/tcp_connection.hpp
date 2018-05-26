#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <boost/asio/ip/tcp.hpp>
#include "connection.hpp"

enum class Role;

/**
 * Implementation of a bidirectional channel over tcp
 */
class TCPConnection : public Connection
{
public:
    /**
     * Constructor given a tcp socket
     */
    TCPConnection(boost::asio::ip::tcp::socket socket);
    ~TCPConnection() = default;

    TCPConnection(TCPConnection&&) = default;
    TCPConnection& operator=(TCPConnection&&) = default;

    /**
     * Dispatch to connect/listen according to role
     */
    static Conn_p from_role(Role role, boost::asio::io_context &io_service,
        std::string address, uint16_t port);
    /**
     * Connect to another party
     */
    static Conn_p connect(boost::asio::io_context& io_service,
            std::string address, uint16_t port);
    /**
     * Listen and wait for connection of another party
     */
    static Conn_p listen(boost::asio::io_context& io_service,
            std::string address, uint16_t port);

    /**
     * Send/receive message prefixed with its length.
     */
    virtual void send_message(const uint8_t * buffer, size_t length) override;
    virtual bytes_t recv_message() override;

    /**
     * Send/receive without length prefix.
     */
    void send(const uint8_t* buffer, size_t length) override;
    void recv(uint8_t* buffer, size_t length) override;

    std::future<size_t> async_send(const uint8_t* buffer, size_t length) override;
    std::future<size_t> async_recv(uint8_t* buffer, size_t length) override;
private:

    void send_length(size_t length);
    size_t recv_length();

    const static size_t header_size = 4;
    boost::asio::ip::tcp::socket socket_;
};

#endif // TCP_CONNECTION_HPP
