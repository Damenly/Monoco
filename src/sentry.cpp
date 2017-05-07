#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_set>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <array>

#include "config.hpp"
#include "passwd.hpp"
#include "common.hpp"
#include "utility.hpp"

NAMESPACE_BEGIN(monoco)

using boost::asio::ip::tcp;
using std::string;
using errs::log;

class sentry
	: public std::enable_shared_from_this<sentry>
{
private:

	//std::unordered_set<std::pair<string, int>, hash_si> _slaves;

	boost::asio::io_service _service;

	char _buffer[configs::BUFF_SIZE];
	char _passwd[MD5_DIGEST_LENGTH];
	size_t _passwd_len;
	
	tcp::socket _socket;
	tcp::resolver _resolver;

	boost::asio::deadline_timer _dead_time;
	boost::asio::deadline_timer _heartbeat;
	
	string _addr;
	string _port;

	std::unordered_set<std::pair<string, string>,
					   monoco::hashers::pairhash> _slaves;
public:
	sentry(const string& addr, const string& port): _socket(_service),
													_resolver(_service),
													_addr(addr),
													_port(port),
													_heartbeat(_service),
													_dead_time(_service)
		{
			
		}

	void connect()
		{
			boost::asio::connect(_socket, _resolver.resolve({_addr, _port}));
		}

	void login()
		{
			auto pwd = utility::md5(getpass("Please enter the password: ",true));
			
			memcpy(_passwd, pwd.c_str(), pwd.size());

			_socket.write_some(boost::asio::buffer(_passwd, pwd.size()));

			auto len = _socket.read_some(boost::asio::buffer(_buffer, sizeof(_buffer)));

			if (_buffer[0] != '0') {
				std::cerr << "Wrong password" << std::endl;
				exit(1);
			}
		}

	void
	start()
		{
			connect();
			login();
			monitor();
			get_slaves();
			write();
			_service.run();
		}

	void
	get_slaves()
		{
			boost::asio::write(_socket, boost::asio::buffer("get_slaves",
															strlen("get_slaves")));
			boost::asio::streambuf buf;
			
			size_t len = boost::asio::read_until(_socket, buf, '\n');
			_slaves.clear();
			errs::log("start to get slaves ");

			int64_t sz = std::stoll(utility::make_string(buf));
			buf.consume(len);

			errs::log("size: ", sz);
			char buffer[configs::BUFF_SIZE];
			
			while (sz-- > 0) {
				len = boost::asio::read_until(_socket, buf, '\n');
				
				string str = utility::make_string(buf);
				buf.consume(len);
				
				log("got slaves", str);
				auto index = str.find(':');
				if (index == -1) {
					log("error format of slaves");
					return ;
				}
				
				_slaves.emplace(str.substr(0, index),
								str.substr(index + 1, -1));
			}
		}
	
	void
	monitor()
		{
			boost::asio::write(_socket, boost::asio::buffer("monitor",
															strlen("monitor")));

			auto sz = _socket.read_some(boost::asio::buffer(_buffer,
															sizeof(_buffer)));
			
			if (sz == 0 || _buffer[0] != '0') {
				throw std::runtime_error("monitor server error");
			}
		}

	void login_to_slave()
		{

			_socket.write_some(boost::asio::buffer(_passwd, sizeof(_passwd)));

			_socket.read_some(boost::asio::buffer(_buffer, sizeof(_buffer)));
			
			if (_buffer[0] != '0') {
				std::cerr << "Wrong password" << std::endl;
				exit(1);
			}
		}

	void start_countdown()
		{
			//	_dead_time.expires_from_now(boost::posix_time::seconds(configs::expire_time));
			//_dead_time.async_wait([](auto ec){ throw std::runtime_error("time expired");});
		}

	void cancel_countdown()
		{
			_dead_time.cancel();
		}
	
	void
	heartbeats(const boost::system::error_code& ec)
		{
			if (!ec) {
				_heartbeat.expires_from_now(boost::posix_time::seconds
										(configs::heartbeat_tick));
				_heartbeat.async_wait(boost::bind(&sentry::read, this));
			}
			else
			{
				log("the server is down");
			}
		}

	void
	write()
		{
			
			boost::asio::async_write(_socket, boost::asio::buffer("heartbeats",
															 strlen("heartbeats")),
									 boost::bind(&sentry::heartbeats, this,
												 _1));
			
		}
	
	void
	read()
		{
			bool fail = false;
			auto self = this->shared_from_this();
			try{
			_socket.async_read_some(boost::asio::buffer(_buffer,
														sizeof(_buffer)),
									[&fail, this, self](boost::system::error_code ec,
												 size_t length)
									{
										if (!ec)
											write();
										else {
											errs::log("server is down");
											_service.stop();
											prompt_slave();
										}
									});
			}
			catch (...)
			{
				_service.stop();
				prompt_slave();
			}
		}

	void
	prompt_slave()
		{
			bool available_salve = false;
			
			for (auto && pr : _slaves) {
				try{
					start_countdown();
					errs::log("try connect ", pr.first, ":", pr.second);
					//boost::asio::connect(_socket,
					//					 _resolver.resolve({pr.first,
					//								 pr.second}));
					boost::asio::connect(_socket,
										 _resolver.resolve({_addr, _port}));
					cancel_countdown();
					available_salve = true;
					break;
				}
				catch (...) {
					continue;
				}
			}

			if (!available_salve) {
				errs::log("No availabel slave");
				exit(1);
			}
			
			login_to_slave();
			monitor();
			get_slaves();
			//write();
			_service.reset();
			write();
			_service.run();
			log("reach end");
		}
	

	void run()
		{
			
			_service.run();
		}
};



NAMESPACE_END(monoco)

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cerr << "Usage: ./program address port" << std::endl;
		exit(1);
	}
	
	try {
	auto s = std::make_shared<monoco::sentry>(argv[1], argv[2]);
	s->start();
	}
	catch (exception& e)
	{
		std::cerr << e.what() << endl;
	}
	
	return 0;
}
