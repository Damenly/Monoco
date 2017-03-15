#ifndef __MONOCO_HPP
#define __MONOCO_HPP

#include <list>
#include <memory>
#include <thread>
#include <fstream>
#include <algorithm>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <openssl/md5.h>

#include "db.hpp"
#include "common.hpp"
#include "config.hpp"
#include "file.hpp"

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
	session(tcp::socket& socket, std::shared_ptr<T> ser)
		: _socket(socket),
		  _server(ser)
		{
			ser->_dbs.size();
		}

	void start()
		{
			if (login() != 0) {
				_socket.close();
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
			auto self(this->shared_from_this());
			_socket.async_read_some(boost::asio::buffer(_data),
									[this, self](boost::system::error_code ec,
												 std::size_t length)
									{
										int ret = this->handle_request(_data.data(),
																	   _data.data() + length,
																	   _reply);
										if (ret > 0) {
											string s = "select " + std::to_string(cur) + "\n";
											s.append(_data.begin(), _data.begin() + length);
											_server->write_aof(s);
										}
										
										if (!ec)
										{
											do_write();
										}
									});
		}

	void do_write()
		{
			auto self(this->shared_from_this());
			boost::asio::async_write(_socket, boost::asio::buffer(_reply.data(),
																  _reply.size()),
									 [this, self](boost::system::error_code ec,
												  std::size_t /*length*/)
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
			string str = "-1";
			size_t n = _socket.read_some(boost::asio::buffer(_data), ec);

			if (std::all_of(std::begin(_server->_password),
							std::end(_server->_password), [](char ch) {return ch == 0;})) {
				str = "0";
				_socket.write_some(boost::asio::buffer(str.c_str(), str.size()));
				return 0;
			}
			
			if (n==0 || ec) {
				_socket.close();
				return 1;
			}
			
			if (_server->login(string(_data.begin(), _data.begin() + n)) != 0) {
				_socket.write_some(boost::asio::buffer(str.c_str(), str.size()));
				_socket.close();
				return 1;
			} else {
				str = "0";
				_socket.write_some(boost::asio::buffer(str.c_str(), str.size()));
				return 0;
			}
			return 1;
		}

private:
	size_t cur = 0;
	tcp::socket& _socket;
	std::array<char, 8192> _data;
	std::string _reply;
	std::shared_ptr<T> _server;
};


class server : public std::enable_shared_from_this<server>
{
private:
	friend class session<server>;
	typedef boost::unique_lock<boost::shared_mutex> write_lock;
	typedef boost::shared_lock<boost::shared_mutex> read_lock;
	
private:
	boost::shared_mutex _lock;
	
	std::vector<std::shared_ptr<mdb>> _dbs;
	unsigned char _password[MD5_DIGEST_LENGTH] = {0};

	std::vector<tcp::socket> _master;
	std::vector<tcp::socket> _sentry;
	std::vector<tcp::socket> _clients;
	std::map<std::pair<string, unsigned short>, tcp::socket> _slaves;

	std::vector<string> _aof_buffers;
	std::ofstream _aof_stream;
private:
	boost::asio::io_service _service;

  	boost::asio::ip::tcp::acceptor _acceptor;

  	boost::asio::ip::tcp::socket _socket;

public:

	server(const server&) = delete;
	server& operator=(const server&) = delete;

	explicit server(const std::string& address,
		   			const std::string& port)
		:
		_service(),
		_acceptor(_service),
		_socket(_service)
		{
			boost::asio::ip::tcp::resolver resolver(_service);
			boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
			_acceptor.open(endpoint.protocol());
			_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
			_acceptor.bind(endpoint);
			_acceptor.listen();
		}

	int
	login(const string& pwd)
		{
			read_lock rlock(_lock);
			unsigned char buffer[MD5_DIGEST_LENGTH] = {};
			MD5_CTX ctx;
			MD5_Init(&ctx);

			MD5_Update(&ctx, pwd.c_str(), pwd.size());
			MD5_Final(buffer, &ctx);
			int ret = memcmp(_password, buffer, MD5_DIGEST_LENGTH);
			
			return ret ? -1 : 0;
		}

	void
	set_pwd(const string& new_pwd)
		{
			write_lock wlock(_lock);
			MD5_CTX ctx;
			MD5_Init(&ctx);

			MD5_Update(&ctx, new_pwd.c_str(), new_pwd.size());
			MD5_Final(_password, &ctx);
		}

	void
	set_rawpwd(const string& new_pwd)
		{
			write_lock wlock(_lock);
			memcpy(_password, new_pwd.c_str(), MD5_DIGEST_LENGTH);
		}
	
	void run()
		{
			int ret = 0;
			if (configs::mdf_restore ||
				configs::aof_restore) {
				clear();
				restore();
			}
			else {
				log("creating empty server");
				for(size_t i = 0; i != configs::init_db_num; ++i)
					_dbs.push_back(std::make_shared<mdb>());
			}

			if (configs::backup_mdf_seconds > 0) {
				boost::posix_time::seconds tick(configs::backup_mdf_seconds);
				boost::asio::deadline_timer timer(_service, tick);

				timer.async_wait(boost::bind(&server::backup_frq, this,
											 &timer, tick));
				
			}
		    accept();
			_service.run();
		}

	void accept()
		{
			auto self = shared_from_this();
			_acceptor.async_accept(_socket,
								   [this](boost::system::error_code ec)
								   {
									   if (!ec)
									   {
										   _clients.push_back(std::move(_socket));
										   auto start_session = [&](){
											   std::make_shared<session<server>>(
												   _sock_counts++,
												   this->shared_from_this())->start();
										   };
										   std::thread(start_session).detach();
									   }

									   accept();
								   });
		}
	
	int remove_db(size_t start, size_t finsh) {
		write_lock wlock(_lock);
		if (start >= _dbs.size() || start >= finsh)
			return -1;
		finsh = std::min(_dbs.size(), finsh);
		_dbs.erase(_dbs.begin() + start,
				   _dbs.begin() + finsh);
		return 0;
	}

	void
	dominate(tcp::socket&& _sock)
		{
			write_lock wlock(_lock);
			string hostname = _sock.remote_endpoint().address().to_string();
			unsigned short port  = _sock.remote_endpoint().port();

			_slaves.emplace(std::make_pair(hostname, port), std::move(_sock));
			wlock.unlock();
			send_to_sentry(hostname, port);
		}

	void send_to_sentry(const string& hostname, unsigned short port)
		{
			if (_sentry.empty())
				return ;
			string pr = hostname;
			pr.append(" ");
			pr.append(std::to_string(port));
			pr.append("\n");

			for (size_t i = 0; i != _sentry.size(); ++i)
			if (_sentry.at(i).is_open())
				boost::asio::write(_sentry[i],
								   boost::asio::buffer(pr.c_str(), pr.size()));
			else
				_sentry.erase(_sentry.begin() + i);
		}

	void add_sentry(tcp::socket&& sock)
		{
			write_lock wlock(_lock);
			/* TO DO now support only one sentry */
			_sentry.push_back(std::move(sock));
			wlock.unlock();

			read_lock rlock(_lock);
			for (auto && slave : _slaves) {
				send_to_sentry(slave.first.first, slave.first.second);
			}
		}
	
	int shutdown()
		{
			backup();
			backup_aof();
			exit(1);
		}
	
	int add_db() {
		write_lock wlock(_lock);
		try {
			_dbs.push_back(std::make_shared<mdb>());
		}catch (...) {
			return -1;
		}
		return 1;
	}

	void resize(size_t new_sz) {
		write_lock wlock(_lock);
		if (new_sz <= _dbs.size())
			return ;
		for (size_t i = _dbs.size(); i != new_sz; ++i)
			_dbs.push_back(std::make_shared<mdb>());
	}

	size_t
	size() {
		read_lock rlock(_lock);
		return _dbs.size();
	}
	
	size_t db_nums() const {
		return _dbs.size();
	}
	
	void clear() {
		write_lock wlock(_lock);
		_aof_buffers.clear();
		_dbs.clear();
		_dbs.shrink_to_fit();
	}

	void clear(size_t pos) {
		write_lock wlock(_lock);
		_dbs.at(pos)->clear();
	}

	void backup_frq(boost::asio::deadline_timer* t,
					boost::posix_time::seconds tick)
		{
			auto _backup_thread = [&]() {
				backup();
			t->expires_at(t->expires_at() + tick);
			t->async_wait(boost::bind(&server::backup_frq,
									  this, t, tick));
			};
			boost::thread(_backup_thread).detach();
		}

	void
	write_aof(const string& cmd)
		{
			write_lock wlock(_lock);
			_aof_buffers.push_back(cmd);

			if (_aof_buffers.size() < configs::cmd_aof_counts) {
				return ;
			}
			
			if (!_aof_stream.is_open()) {
				_aof_stream.open(configs::aof_path,
								 std::ios::app);
			}
			
			if (!_aof_stream) {
				log("open ", configs::aof_path, " error");
				return ;
			}

			for (auto && ele : _aof_buffers) {
				_aof_stream << ele << "\n";
				_aof_stream.flush();
			}

			_aof_buffers.clear();
		}
	
	void
	backup() {
		read_lock rlock(_lock);
		if (fs::is_exists(configs::mdf_path)) {
			log("file exists, removing it");
			fs::rm(configs::mdf_path);
		}

		std::ofstream os(configs::mdf_path, std::ios::binary);

		boost::crc_32_type crc;
		fs::write_to(os, configs::MONOCO, crc);
		fs::write_to(os, configs::VERSION, crc);

		os.write((char*)_password, sizeof(_password));
		crc.process_bytes(_password, sizeof(_password));
		
		size_t holder = _dbs.size();
		fs::write_to(os, holder, crc);

		for (size_t dbid = 0; dbid != _dbs.size(); ++dbid) {
			fs::write_to(os, dbid, crc);
			_dbs[dbid]->write_to(os);
		}

		fs::write_to(os, crc.checksum(), crc).flush();
	}

	void
	backup_aof() {
		read_lock rlock(_lock);
		if (fs::is_exists(configs::aof_path)) {
			log(configs::aof_path, " exists, removing it");
			fs::rm(configs::aof_path);
		}

		std::ofstream os(configs::aof_path, std::ios::app);
		os << "rawpwd "
		   << string(std::begin(_password), std::end(_password))
		   << std::endl;
		
		os << "resize " << _dbs.size() << std::endl;
		string cmd;
		for (size_t dbid = 0; dbid != _dbs.size(); ++dbid) {
			cmd = "select " + std::to_string(dbid) + "\n";
			os << cmd;
			_dbs[dbid]->write_aof(os);
		}
	}

	int
    restore_from_mdf() {
		clear();
		write_lock wlock(_lock);
		std::ifstream is(configs::mdf_path, std::ios::binary);
		if (!is || !is.is_open()) {
			log(configs::mdf_path, " open error");
			return 1;
		}
		
		size_t holder;
		string str;
		boost::crc_32_type crc;
		fs::read_from(is, str, crc);
		if (str != configs::MONOCO) {
			log("NOT support file");
			return 1;
		}

		long double version;
		fs::read_from(is, version, crc);
		if (version < configs::VERSION) {
			log("NOT support version");
			return 1;
		}

		is.read((char*)_password, sizeof(_password));
		crc.process_bytes(_password, sizeof(_password));
		
		size_t sz;
		fs::read_from(is, sz, crc);

		for (size_t dbid = 0; dbid != sz; ++dbid)
			_dbs.push_back(std::make_shared<mdb>());
		
		for (size_t dbid = 0; dbid != sz; ++dbid) {
			fs::read_from(is, holder, crc);
			_dbs[holder]->read_from(is);
		}

		uint32_t sum;
		auto oldsum = crc.checksum();
		fs::read_from(is, sum, crc);
		if (sum != oldsum)
			log("checksum diffs");
		
		return 0;
	}

	void
	restore_from_aof()
		{
			clear();
			/* FIX ME since call handle_cmd will require wlock again*/
			//write_lock wlock(_lock);
			std::ifstream is(configs::aof_path);
			if (!is || !is.is_open()) {
				log("can't open ", configs::aof_path);
				return exit(1);
			}

			tcp::socket tmp_sock(_service);
			auto ptr = std::make_shared<session<server>>(
				tmp_sock,
				this->shared_from_this());
			  
			char buffer[8196];
			while (is.getline(buffer, sizeof(buffer))) {
				string cmd(buffer);
				string reply;
	
				int ret = ptr->handle_request(cmd.begin(), cmd.end(),
											  reply);
				if (ret == -1) {
					log("restore from aof error");
					exit(1);
				}
			}
		}

	void
	restore()
		{
			if (configs::aof_restore) {
				restore_from_aof();
				return ;
			}
			if (configs::mdf_restore) {
				restore_from_mdf();
				return ;
			}
		}
};

NAMESPACE_END(monoco)

#endif // end of __MONOCO_HPP
