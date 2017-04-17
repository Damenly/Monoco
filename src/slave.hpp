#ifndef _M_SLAVE_HPP
#define _M_SLAVE_HPP

#include <boost/asio.hpp>

#include "server.hpp"
#include "common.hpp"
#include "config.hpp"

NAMESPACE_BEGIN(monoco)

class slave : public server
{
protected:
	vector<tcp::socket>_master;
	
public:
	void
	remove_master()
		{
			write_lock wlock(_lock);
			_master.clear();
		}
	
	explicit slave(const std::string& address,
				   const std::string& port) : server(address, port)
		{
			
		}

	virtual void run()
		{
			connect_to_master();
			_master.front().write_some(boost::asio::buffer("send aof",
														   strlen("send aof")));
			receive_file(configs::aof_path);
			restore_from_aof();


			_master.front().write_some(boost::asio::buffer("send mdf",
														   strlen("send mdf")));
			receive_file(configs::mdf_path);

			_master.front().write_some(boost::asio::buffer("send config",
														   strlen("send config")));
			receive_file(configs::config_path);
			
			_slave();
			
			string foo;
			for(;;) {
				foo.clear();
				boost::asio::streambuf reply;
				size_t n = boost::asio::read_until(_master.front(), reply, '\n');
				std::string cmd = make_string(reply);
				log("recive cmd: ", cmd);
				//auto sess = std::make_shared<session<server>>("fake", -1,
				//											  this->shared_from_this());
				
				//sess->handle_request(cmd.begin(), cmd.end(), foo);
				write_aof(cmd);
			}
		}

	void
	_slave()
		{
			char buffer[configs::BUFF_SIZE];
			_master.front().write_some(boost::asio::buffer("add_slave",
														   strlen("add_slave")));
			size_t n = _master.front().read_some(boost::asio::buffer(buffer, sizeof(buffer)));
			if (buffer[0] != '0') {
				string str = "failed to be the slave of ";
				str.append(_address);
				str.append(" ");
				str.append(_port);
				
				throw std::runtime_error(str);
			}
		}
	void connect_to_master()
		{
			tcp::socket s(_service);
			tcp::resolver resolver(_service);
			log(_address, " ", _port);
			boost::asio::connect(s, resolver.resolve({_address, _port}));
			char request[configs::BUFF_SIZE];
			auto pwd = utility::md5(getpass("Please enter the password: ",true));
			strcpy(request, pwd.c_str());
			size_t request_length = std::strlen(request);
			boost::asio::write(s, boost::asio::buffer(request, request_length));
	
			char reply[8192];
			boost::system::error_code ec;
			size_t reply_length = s.read_some(boost::asio::buffer(reply));
			if (memcmp(reply, "0", 1)) {
				cout << "wrong password" << endl;
				exit(1);
			}
			else
				cout << "connected"<<endl;

			add_master(std::move(s));
		}
	
	void
	add_master(tcp::socket sock)
		{
			write_lock wlock(_lock);
			_master.push_back(std::move(sock));
		}

	
	void
	receive_file(const string& file_name)
		{
			tcp::socket& sock = _master.front();
			boost::asio::streambuf buf;
			
			size_t len = boost::asio::read_until(sock, buf, '\n');
			log("satrt to receive ", file_name);

			int64_t file_size = std::stoll(make_string(buf));
			buf.consume(len);

			if (fs::exists(file_name)) {
				log(file_name, " exists.\n removing it");
				fs::rm(file_name);
			}

			log(file_name, " size: ", file_size);
			
			std::ofstream os(file_name, std::ios::binary | std::ios::app);
			char buffer[configs::BUFF_SIZE];
			
			while (file_size > 0) {

				len = sock.read_some(boost::asio::buffer(buffer,
															sizeof(buffer)));
				log("receive ", len, " bytes ", file_size);
				file_size -= len;
				if (file_size < 0)
					os.write(buffer, len + file_size);
			}
			
			os.flush();
			log(file_name, " received");
		}
};

NAMESPACE_END(monoco)

#endif // end of _M_SLAVE_HPP
