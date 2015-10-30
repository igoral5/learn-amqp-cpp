/**
 * @file consumer-asio.cpp
 * @author igor
 * @date 26 окт. 2015 г.
 */

#include <iostream>
#include <exception>
#include <cstdlib>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include <amqpcpp.h>

#include "asiohandler.h"

int
main(int argc, char *argv[])
try
{
    if (argc < 4)
    {
        std::cerr << "Usage: consumer-asio <host> <port> <queue>" << std::endl;
        return EXIT_SUCCESS;
    }
    const std::string host(argv[1]);
    uint16_t port = boost::lexical_cast<uint16_t>(argv[2]);
    const std::string queue(argv[3]);
    boost::asio::io_service io;
    AsioHandler handler(io);
    handler.connect(host, port);
    AMQP::Connection connection(&handler);
    AMQP::Channel channel(&connection);
    channel.declareQueue(queue, AMQP::durable);
    channel.consume(queue, AMQP::noack).onReceived([](AMQP::Message &&message, uint64_t deliveryTag, bool redelivered)
        {
            std::cout <<" [x] Received " << message.message() << std::endl;
        });
    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";
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
