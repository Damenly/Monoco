#ifndef __MONOCO_HPP
#define __MONOCO_HPP

#include <list>
#include <memory>

#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>

#include "common.hpp"
#include "config.hpp"
#include "db.hpp"
#include "client.hpp"
#include "cmds.cpp"
#include "file.hpp"

NAMESPACE_BEGIN(monoco)

using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace monoco::errs;

template <typename T>
class session
	: public std::enable_shared_from_this<session<T>>
{
public:
	session(tcp::socket socket, T* ser)
		: _socket(std::move(socket)),
		  _server(ser)
  {
  }

  void start()
  {
    do_read();
  }

private:
  void do_read()
  {
	  //auto self(shared_from_this());
	  _socket.async_read_some(boost::asio::buffer(_data),
							  [this](boost::system::error_code ec, std::size_t length)
							  {
								  handle_request(_server->cur_db(), _data.data(), _data.data() + length, _reply);
								  std::cout << ec.message() << _reply << std::endl;
								  if (!ec)
								  {
									  do_write();
								  }
							  });
  }

  void do_write()
  {
	  //auto self(shared_from_this());
	  boost::asio::async_write(_socket, boost::asio::buffer(_reply.data(),
														  _reply.size()),
							   [this](boost::system::error_code ec, std::size_t /*length*/)
							   {
								   if (!ec)
								   {
								   }
							   });
  }
	
  template <typename IT>
  static void 
  handle_request (std::shared_ptr<mdb> _db, IT first, IT last, string& reply)
  {
	  if (_db == nullptr) {
		  reply = "Invalid db";
		  return ;
	  }
	  reply.clear();
	  string request(first, last);
	  std::vector<std::string> args;
	  boost::char_separator<char> sep(" ");
	  boost::tokenizer<boost::char_separator<char>> tokens(request, sep);
	  for (const auto& t : tokens) {
		  args.push_back(t);
	  }
	  
	  int ret = handle_cmds(_db, args, reply);
	  if (reply.empty()) {
		  reply = std::to_string(ret);
		  reply.append("\n");
	  }
  }

	tcp::socket _socket;
	std::array<char, 8192> _data;
	std::string _reply;
	T* _server;
};

class server
{
private:
	
private:
	std::vector<std::shared_ptr<mdb>> _dbs;

	size_t current_db = 0;
	
private:
	boost::asio::io_service _service;

  	boost::asio::ip::tcp::acceptor _acceptor;

  	boost::asio::ip::tcp::socket _socket;

public:

	server(const server&) = delete;
	server& operator=(const server&) = delete;

	static
	void
	print(){}
	explicit server(const std::string& address,
		   			const std::string& port)
	:
	  _service(),
	  _acceptor(_service),
	  _socket(_service)
			{
				for(size_t i = 0; i != configs::init_db_num; ++i)
					_dbs.push_back(std::make_shared<mdb>());
				current_db = 0;
				  
				  boost::asio::ip::tcp::resolver resolver(_service);
				  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
				  _acceptor.open(endpoint.protocol());
				  _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
				  _acceptor.bind(endpoint);
				  _acceptor.listen();

				  accept();
			}
	  
	void run()
		{
			_service.run();
		}

	void accept()
	{
		_acceptor.async_accept(_socket,
							   [this](boost::system::error_code ec)
							   {
								   
								   if (!ec)
								   {
									   std::make_shared<session<server>>(
										   std::move(_socket),
										   this)->start();
								   }

								   accept();
							   });
	}

	int select_db(size_t pos) {
		if (pos >= _dbs.size())
			return -1;
		current_db = pos;
		return 1;
	}

	std::shared_ptr<mdb>
	cur_db() {
		if (_dbs.empty())
			return nullptr;
		return _dbs.at(current_db);
	}

	int move_cur(size_t pos) {
		if (pos >= _dbs.size())
			return -1;
		current_db = pos;
		return 0;
	}
	
	int remove_db(size_t pos) {
		if (pos >= _dbs.size())
			return -1;
		_dbs.erase(_dbs.begin() + pos);
		current_db = 0;
		return 0;
	}

	int add_db() {
		try {
		_dbs.push_back(std::make_shared<mdb>());
		}catch (...) {
			return -1;
		}
		return 1;
	}

	size_t db_nums() const {
		return _dbs.size();
	}
	
	void clear() {
		_dbs.clear();
		current_db = 0;
	}
	
	int
	backup_mdb(string& res) {
		int ret;
		if (fs::is_exists(configs::mdf_path)) {
			log("file exists");
			res.append("file exists");
		}

		ret = fs::touch(configs::mdf_path);
		if (ret) {
			log("touch file error");
			res.append("touch file error");
		}

		std::ofstream ofs(configs::mdf_path);

		ofs << configs::MONOCO;
		ofs << configs::VERSION;
		ofs.flush();
		
		for (size_t dbid = 0; dbid != _dbs.size(); ++dbid) {
			ofs << dbid << (_dbs[dbid])->key_nums();
			ofs.flush();
			for (auto && db : _dbs) {
				
			}
				
		}
	}
		
};

NAMESPACE_END(monoco)

#endif // end of __MONOCO_HPP
