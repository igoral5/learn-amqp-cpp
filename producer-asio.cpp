/**
 * @file producer-asio.cpp
 * @author igor
 * @date 26 окт. 2015 г.
 */

#include <iostream>
#include <exception>
#include <cstdlib>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#include <amqpcpp.h>

#include "asiohandler.h"

class Producer
{
public:
    Producer(boost::asio::io_service& io,
            const std::string& host,
            uint16_t port,
            const std::string& exchange,
            const std::string& routing_key,
            const std::string& message,
            size_t count) :
                m_handler(io),
                m_connection(&m_handler),
                m_channel(&m_connection),
                m_exchange(exchange),
                m_routing_key(routing_key),
                m_message(message),
                m_max(count),
                m_count(0)

    {
        m_handler.connect(host, port);
        m_channel.onReady(boost::bind(
                &Producer::handle_channel_ready,
                this));
        m_channel.onError(boost::bind(
                &Producer::handle_channel_error,
                this,
                _1));

    }
    virtual ~Producer() noexcept = default;
private:
    void handle_channel_error(const char* message)
    {
        std::cerr << "error channel: " << message << std::endl;
    }
    void handle_channel_ready()
    {
        if (m_handler.connected())
        {
            m_channel.declareQueue("hello", AMQP::durable);
            m_channel.onReady(boost::bind(
                    &Producer::send_message,
                    this));
        }
    }
    void send_message()
    {
        if (m_count < m_max)
        {
            std::ostringstream oss;
            oss << m_count << " - " << m_message;
            if (m_channel.publish(m_exchange, m_routing_key, oss.str()))
            {
                std::cout << "Send: '" << oss.str() << "'" << std::endl;
                m_count++;
                m_channel.onReady(boost::bind(
                        &Producer::send_message,
                        this));
            }
        }
        else
        {
            m_connection.close();
        }
    }
    AsioHandler m_handler;
    AMQP::Connection m_connection;
    AMQP::Channel m_channel;
    const std::string m_exchange;
    const std::string m_routing_key;
    const std::string m_message;
    const size_t m_max;
    size_t m_count;
};

int
main(int argc, char *argv[])
try
{
    if (argc < 6)
    {
        std::cerr << "Usage: producer-asio <host> <port> <exchange> <routing_key> <message> <count>" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string host(argv[1]);
    uint16_t port = boost::lexical_cast<uint16_t>(argv[2]);
    const std::string exchange(argv[3]);
    const std::string routing_key(argv[4]);
    const std::string message(argv[5]);
    //size_t count = boost::lexical_cast<size_t>(argv[6]);
    boost::asio::io_service io;
    //Producer producer(io, host, port, exchange, routing_key, message, count);
    AsioHandler handler(io);
    handler.connect(host, port);
    AMQP::Connection connection(&handler);
    AMQP::Channel channel(&connection);
    channel.onReady([&]()
        {
            channel.declareQueue(routing_key, AMQP::durable);
            channel.publish(exchange, routing_key, message);
            std::cout << "[x] Sent " << message << std::endl;
            connection.close();
        });
    io.run();
    return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch(...)
{
    std::cerr << "Unknown exceprion" << std::endl;
    return EXIT_FAILURE;
}
