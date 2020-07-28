#include <iostream>
#include "boost/asio.hpp"
#include "MyConnectionHandler.h"
#include <random>

using boost::asio::ip::tcp;
using namespace boost::asio;

void session(tcp::socket sock, int number) {
    static std::mt19937 random_engine;
    static std::uniform_int_distribution<int> random_chars(0, 9);

    int message = number * 10 + random_chars(random_engine);
    std::cout << sock.remote_endpoint().address().to_string() << ":" << sock.remote_endpoint().port() << ": "
    << message << std::endl;

    boost::asio::write(sock, boost::asio::buffer(&message, sizeof message));

    std::this_thread::sleep_for(std::chrono::seconds(1));
}
void server(boost::asio::io_context io_context, unsigned short port)
{
    auto number = 1;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    while (true)
    {
        std::thread(session, acceptor.accept(), number++).detach();
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: blocking_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        server(io_context, std::stoi(argv[1]));
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}