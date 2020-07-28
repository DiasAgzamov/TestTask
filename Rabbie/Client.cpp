#include <iostream>
#include "MyConnectionHandler.h"
#include "boost/asio.hpp"

using namespace boost::asio;
using boost::asio::ip::tcp;
int main(int argc, char* argv[])
{
    boost::asio::io_context io_context;
    tcp::socket server_socket(io_context);
    tcp::resolver resolver(io_context);
    boost::asio::connect(server_socket, resolver.resolve(argv[1], argv[2]));

    MyConnectionHandler handler(io_context, argv[3], argv[4]);
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    while (true) {
        int message;
        const size_t message_length_read = boost::asio::read(server_socket,
                                                             boost::asio::buffer(&message, sizeof message));
        std::cout << "Message is: " << message << std::endl;

        channel.onReady([&]() {
            if (handler.connected()) {
                channel.publish("", "Queue", std::to_string(message));
                handler.quit();
            }
        });
        handler.loop();
    }
}
