#ifndef _CONNECTION_HPP
#define _CONNECTION_HPP

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include <iostream>
#include <set>

#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include "config.hpp"
#include "connection_manager.hpp"

NAMESPACE_BEGIN(monoco)

class connection
  : public std::enable_shared_from_this<connection>
{
public:
  connection(const connection&) = delete;
  connection& operator=(const connection&) = delete;

	explicit connection(boost::asio::ip::tcp::socket socket,
						connection_manager<connection>& manager, request_handler& handler):
		_socket(std::move(socket)),
		_manager(manager),
		_req_handler(handler)
		{
		}

  void start()
  {
	  read();
  }

  void stop()
  {
	  _socket.close();
  }

private:
  void read()
  {
	  _request.args.clear();
	  _reply.headers.clear();
	  auto self(shared_from_this());
	  
	  _socket.async_read_some(boost::asio::buffer(_buffer),
							  [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
							  {
								  if (!ec)
								  {
									  request_parser::result_type result;
									  std::tie(result, std::ignore) = _req_parser.parse(
										  _request, _buffer.data(), _buffer.data() + bytes_transferred);
									  
									  if (result == request_parser::good)
									  {
										  _req_handler.handle_request(_request, _reply);
										  write();
									  }
									  else
									  {
										  read();
									  }
								  }
								  else if (ec == boost::asio::error::eof)
								  {
									  _manager.stop(shared_from_this());
								  }
							  });
  }

   void write()
  {
	  auto self(shared_from_this());
	  boost::asio::async_write(_socket, _reply.to_buffers(),
							   [this, self](boost::system::error_code ec, std::size_t)
							   {
								   if (!ec)
									   read();
							   });

  }

	boost::asio::ip::tcp::socket _socket;

	connection_manager<connection>& _manager;

	request_handler _req_handler;

	std::array<char, 8192> _buffer;
	
	request _request;

	request_parser _req_parser;
	
	reply _reply;
};

typedef std::shared_ptr<connection> connection_ptr;

NAMESPACE_END(monoco)

#endif
