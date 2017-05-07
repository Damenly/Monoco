#ifndef _M_SESSION_HPP
#define _M_SESSION_HPP

#include "common.hpp"
#include "config.hpp"


NAMESPACE_BEGIN(monoco)

using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace monoco::errs;

template <typename T>
int
handle_cmds(std::shared_ptr<T> ptr, std::vector<string> &args,
			string& reply);

template <typename T>
int
foo(T val);

template <typename T>
class session
	: public std::enable_shared_from_this<session<T>>
{
public:
	session(string host, short port, std::shared_ptr<T> ser)
		: _host(host), _port(port),
		  _server(ser)
		{
			
		}

	void start()
		{
			if (login() != 0) {
				get_server()->get_client(_host, _port).close();
				return ;
			}
				
			do_read();
		}
	
	friend int handle_cmds<session<T>>(
		std::shared_ptr<session<T>>, std::vector<string> &args,
		string& reply);
	
	template <typename IT>
	int
	handle_request (IT first, IT last, string& reply)
		{
			reply.clear();
			string request(first, last);
			std::vector<std::string> args;
			boost::char_separator<char> sep(" ");
			boost::tokenizer<boost::char_separator<char>> tokens(request, sep);
			for (const auto& t : tokens) {
				args.push_back(t);
			}

			int ret = handle_cmds(this->shared_from_this(), args, reply);
			if (reply.empty()) {
				reply = std::to_string(ret);
				reply.append("\n");
			}

			return ret;
		}
	
	void do_read()
		{
			try{
			auto self(this->shared_from_this());
			auto ser = get_server();
			ser->get_client(_host, _port).async_read_some(boost::asio::buffer(_data),
														  [this, self](boost::system::error_code ec,
																	   std::size_t length)
														  {
															  log(string(_data.data(), _data.data() + length));
															  int ret = this->handle_request(_data.data(),
																							 _data.data() + length,
																							 _reply);
															  if (ret > 0) {
																  string s = "select " + std::to_string(cur) + "\n";
																  s.append(_data.begin(), _data.begin() + length);
																  get_server()->write_aof(s);
															  }
															  
															  if (!ec)
															  {
																  do_write();
															  }
														  });
			} catch(...)
			{
				log("....");
			}
		}

	void do_write()
		{
			auto self(this->shared_from_this());
			boost::asio::async_write(get_server()->get_client(_host, _port), boost::asio::buffer(_reply.data(),
																  _reply.size()),
									 [this, self](boost::system::error_code ec,
												  std::size_t length)
									 {
										 if (!ec)
										 {
											 do_read();
										 }
									 });
		}

	int login()
		{
			boost::system::error_code ec;
			auto ser = get_server();
			string str = "-1";
			size_t n = ser->get_client(_host, _port).read_some(boost::asio::buffer(_data), ec);

			errs::log("start login");
			if (std::all_of(std::begin(ser->_password),
							std::end(ser->_password), [](char ch) {return ch == 0;})) {
				str = "0";
				ser->get_client(_host, _port).write_some(boost::asio::buffer(str.c_str(), str.size()));
				return 0;
			}
			
			if (n == 0 || ec) {
				ser->get_client(_host, _port).close();
				return 1;
			}
			
			if (ser->login(string(_data.begin(), _data.begin() + n)) != 0) {
				ser->get_client(_host, _port).write_some(boost::asio::buffer(str.c_str(), str.size()));
				ser->get_client(_host, _port).close();
				return 1;
			}
			else {
				str = "0";
				ser->get_client(_host, _port).write_some(boost::asio::buffer(str.c_str(), str.size()));
				return 0;
			}
			return 1;
		}

	std::shared_ptr<T> get_server()
		{
			if (_server.expired()) {
				log("session for host ", _host,  "port", _port, "terminated",
					"server dead");
				exit(0);
			}

			return _server.lock();
		}
	
protected:
	//cur
	size_t cur = 0;
	// host of db
	string _host; 
	short _port;
	std::array<char, 8192> _data;
	std::string _reply;
	std::weak_ptr<T> _server;
};

NAMESPACE_END(monoco)
#endif
