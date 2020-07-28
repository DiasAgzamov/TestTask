#include <amqpcpp.h>
#include <boost/asio.hpp>
#include <memory>
using boost::asio::ip::tcp;

struct MyConnectionHandlerImpl;

class MyConnectionHandler final : public AMQP::ConnectionHandler
{
public:
   static constexpr size_t buffer_size = 8 * 1024 * 1024; //8Mb
   static constexpr size_t temp_buffer_size = 1024 * 1024; //1Mb

   MyConnectionHandler(boost::asio::io_context& io_context, const std::string& host, const std::string& port);
   ~MyConnectionHandler() override;

   void loop() const;
   void quit() const;

   bool connected() const;

private:

   void onData(AMQP::Connection *connection, const char *data, size_t size) override;
   
   void onReady(AMQP::Connection *connection) override;
   
   void onError(AMQP::Connection *connection, const char *message) override;
    
   void onClosed(AMQP::Connection *connection) override;
   
   void send_data_from_buffer();

   std::shared_ptr<MyConnectionHandlerImpl> m_impl;
};
