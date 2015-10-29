/**
 * @file producer.cpp
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
            const std::string& message,
            size_t count) :
                m_io(io),
                m_timer(io, boost::posix_time::milliseconds(10000)),
                m_handler(io),
                m_connection(&m_handler, AMQP::Login("guest", "guest"), "/"),
                m_channel(&m_connection),
                m_message(message),
                m_count(count)
    {
        m_handler.connect(host, port);
        m_channel.onReady(boost::bind(
                &Producer::handle_channel_ready,
                this));
    }
    virtual ~Producer() noexcept = default;
private:
    void handle_channel_ready()
    {
        if (m_handler.connected())
        {
            for (size_t i = 0; i < m_count; i++)
            {
                std::ostringstream oss;
                oss << i << " - " << m_message;
                m_channel.publish("", "hello", oss.str());
                std::cout << "Send: '" << oss.str() << "'" << std::endl;
            }
        }
        m_timer.async_wait(boost::bind(
                &Producer::handle_timer,
                this,
                boost::asio::placeholders::error));
    }
    void handle_timer(const boost::system::error_code& e)
    {
        m_io.stop();
    }
    boost::asio::io_service& m_io;
    boost::asio::deadline_timer m_timer;
    AsioHandler m_handler;
    AMQP::Connection m_connection;
    AMQP::Channel m_channel;
    std::string m_message;
    size_t m_count;
};

int
main(int argc, char *argv[])
try
{
    if (argc < 5)
    {
        std::cerr << "Usage: producer <host> <port> <message> <count>" << std::endl;
        return EXIT_FAILURE;
    }
    uint64_t port = 5672;
    try
    {
        port = boost::lexical_cast<uint64_t>(argv[2]);
    }
    catch(...)
    {
    }
    boost::asio::io_service io;
    Producer producer(io, argv[1], port, argv[3], boost::lexical_cast<uint64_t>(argv[4]));
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
