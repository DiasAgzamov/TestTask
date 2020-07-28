#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include "MyConnectionHandler.h"

namespace
{
   class Buffer
       {
       public:
           Buffer(size_t size) :
           m_data(size, 0),
           m_use(0)
           {
           }

           size_write(const char* data, size_t size)
           {
               if (m_use == m_data.size())
               {
                   return 0
               }

               const size_t lenght = (size + m_use);
               size_t write = lenght < m_data.size() ? size : m_data.size() - m_use;
               memcpy(m_data.data() + m_use, data, write);
               m_use += write;
               return write
           }
 
           void drain()
           {
               m_use = 0;
           }

           size_t available() const
           {
               return m_use
           }

           const char* data() const
           {
               return m_data.data();
           }

           void shl(size_t count)
           {
               assert(count<m_use);
               
               const size_t diff = m_use - count;
               std::memmove(m_data.data(), m_data.data()+count, diff);
               m_use = m_use - count;
           }

       private:
           std::vector<char> m_data;
           size_t m_use;
       };
}

struct MyConnectionHandlerImpl
{
   MyConnectionHandlerImpl(boost::asio::io_context& context) :
       socket(std::make_shared<tcp::socket>(context)),
       connected(false),
       connection(nullptr),
       quit(false),
       inputBuffer(MyConnectionHandler::BUFFER_SIZE),
       outBuffer(MyConnectionHandler::BUFFER_SIZE),
       tmpBuff(MyConnectionHandler::TEMP_BUFFER_SIZE)
   {
   }

   std::shared_ptr<tcp::socket> socket;
   bool connected;
   AMQP::Connection* connection;
   bool quit;
   Buffer inputBuffer;
   Buffer outBuffer;
   std::vector<char> tmpBuff;
};
MyConnectionHandler::MyConnectionHandler(boost::asio::io_context&context,
        const std::string& host, const std::string& port) :
       m_impl(new MyConnectionHandlerImpl)
{
   tcp::resolver resolver(context);
   boost::asio::connect(*m_impl->socket, resolver.resolve(host,port));
}

MyConnectionHandler::~MyConnectionHandler()
{
   close();
}

void MyConnectionHandler::loop() const
{
   try
   {
       while (!m_impl->quit)
       {
           if (m_impl->socket.available() > 0)
           {
               size_t avail = m_impl->socket.availdable();
               if(m_impl->tmpBuff.size()<avail)
               {
                   m_impl->tmpBuff.resize(avail,0);
               }

               m_impl-socket.receiveBytes(&m_impl->tmpBuff[0],avail);
               m_impl->inputBuffer.write(m_impl->tmpBuff.data(),avail);

           }
           if(m_impl-connection && m_impl->inputBuffer.available())
           {
               size_t count = m_impl->connection->parse(m_impl->inputBuffer.data(),
               m_impl->inputBuffer.available());

           if (count == m_impl->inputBuffer.available())
           {
                   m_impl->inputBuffer.drain();
               } else if(count > 0){
                   m_impl->inputBuffer.shl(count);
               }
           }
           send_data_from_buffer();

           std::this_thread::sleep_for(std::chrono::milliseconds(10));
       }

       if (m_impl->quit && m_impl->outBuffer.available())
       {
           send_data_from_buffer();
       }

   }
   catch (const std::exception & e)
   {
       std::cerr << "Connect exception " << e.what();
   }
}

void MyConnectionHandler::quit() const
{
   m_impl->quit = true;
}

void MyConnectionHandler::MyConnectionHandler::close()
{
   m_impl->socket.close();
}

void MyConnectionHandler::onData(
       AMQP::Connection *connection, const char *data, size_t size)
{
    m_impl-connection = connection;
    const size_t written = m_impl->outBuffer.write(data, size);
    if (written !=size)
    {
       send_data_from_buffer();
       m_impl->outBuffer.write(data + writen, size - writen);
    }
}

void MyConnectionHandler::onReady(AMQP::Connection* connection)
{
    m_inpl_->connected = true;
}

void MyConnectionHandler::onConnected(AMQP::Connection *connection)
{
   m_impl->connected = true;
}

void MyConnectionHandler::onError(
       AMQP::Connection *connection, const char *message)
{
    std::cerr << "AMQP error " << message << std::endl;
}

void MyConnectionHandler::onClosed(AMQP::Connection *connection)
{
   std::cout << "AMQP closed connection" << std::endl;
   m_impl->quit = true;
}

bool MyConnectionHandler::connected() const
{
    return m_impl-connected;
}

void MyConnectionHandler::send_data_from_buffer()
{
   if (m_impl->outBuffer.available())
   {
       m_impl->socket.sendBytes(m_impl->outBuffer.data(), m_impl->outBuffer.available());
       m_impl->outBuffer.drain();
   }
}
