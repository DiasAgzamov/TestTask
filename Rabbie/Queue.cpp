#include <iostream>
#include "MyConnectionHandler.h"

int main(int argc, char* argv[])
{
    boost::asio::io_context io_context;
    MyConnectionHandler handler(io_context, argv[1],argv[2]);

    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");

    AMQP::Channel channel(&connection);
    channel.declareQueue("Queue");
    channel.consume("Queue", AMQP::noack).onReceived(
            [](const AMQP::Message& message,
            uint64_t deliveryTag,
            bool redelivered)
    {
        std::cout.write(&message.body()[0], message.bodySize());
        std::cout << std::endl;
    });
    handler.loop();
    return 0;
}