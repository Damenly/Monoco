#ifndef __MONOCO_HPP
#define __MONOCO_HPP

#include <list>
#include <memory>
#include <thread>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <cstdio>

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
#include "passwd.hpp"
#include "session.hpp"

NAMESPACE_BEGIN(monoco)

using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace monoco::errs;

class server : public std::enable_shared_from_this<server>
{
protected:
	friend class session<server>;
	typedef boost::unique_lock<boost::shared_mutex> write_lock;
	typedef boost::shared_lock<boost::shared_mutex> read_lock;
	
protected:
	boost::shared_mutex _lock;
	
	std::vector<std::shared_ptr<mdb>> _dbs;

	std::map<std::pair<string, unsigned short>, tcp::socket> _sentry;
	std::map<std::pair<string, unsigned short>, tcp::socket> _clients;
	std::map<std::pair<string, unsigned short>, tcp::socket> _slaves;
	
	unsigned char _password[MD5_DIGEST_LENGTH] = {0};
	
	std::vector<string> _aof_buffers;
	std::ofstream _aof_stream;
	size_t _sock_counts = 0;
protected:
	string _address;
	string _port;
	
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
		_socket(_service),
		_address(address),
		_port(port)
		{
			
		}

	int
	login(const string& pwd)
		{
			read_lock rlock(_lock);
			
			int ret = memcmp(_password, pwd.c_str(), MD5_DIGEST_LENGTH);
			
			return ret ? -1 : 0;
		}

	void
	rm_cli(const string& addr, unsigned short port)
		{
			write_lock wlock(_lock);
			auto iter = _clients.find(std::make_pair(addr, port));
			if (iter != _clients.end())
				_clients.erase(iter);
		}
	
	void
	evolve()
		{
			write_lock wlock(_lock);
			clear();
			
			wlock.unlock();
			restore();
			run();
		}
	
	void
	set_pwd(const string& new_pwd)
		{
			write_lock wlock(_lock);
			memcpy(_password, new_pwd.c_str(), MD5_DIGEST_LENGTH);
		}

	string
	get_rawpwd()
		{
			read_lock rlock(_lock);
			return string(std::begin(_password), std::end(_password));
		}

	static
	std::string make_string(boost::asio::streambuf& streambuf)
		{
			return {buffers_begin(streambuf.data()), 
					buffers_end(streambuf.data())};
		}

	int
	send_file(const string& addr, unsigned short port, const string& file)
		{
			tcp::socket& sock = get_client(addr, port);
			
			read_lock rlock(_lock);
			std::ifstream is(file, std::ios::binary|std::ios::ate);
			if (!is)
			{
				log("failed to open ", file);
				return -1;
			}

			log("open ", file);
			size_t file_size = is.tellg();
			is.seekg(0);

			string str;
			str.append(std::to_string(file_size));
			str.append("\n");
			
			char buffer[configs::BUFF_SIZE] = {0};
			log("start to send ", file, "(", file_size, ") to ", addr, " ", port);
			sock.write_some(boost::asio::buffer(str.c_str(), str.size()));

			while (is) {
				is.read(buffer, sizeof(buffer));
				if (is.gcount() <= 0) {
					log("read ", file, " error");
					return -1;
				}

				sock.write_some(boost::asio::buffer(buffer, is.gcount()));
			}
			
			log("sent ", file, " already");
			return 0;
		}
	
	virtual void run()
		{
			restore();
			backup_cron();
			listen();
		    accept();
			
			_service.run();
		}

	void restore()
		{
			if (configs::mdf_restore ||
				configs::aof_restore) {
				clear();
				_restore();
			}
			else {
				log("creating empty server");
				for(size_t i = 0; i != configs::init_db_num; ++i)
					_dbs.push_back(std::make_shared<mdb>());
			}
		}

	void backup_cron()
		{
			if (configs::backup_mdf_seconds > 0) {
				boost::posix_time::seconds tick(configs::backup_mdf_seconds);
				auto timer = std::make_shared<boost::asio::deadline_timer>(_service, tick);

				timer->async_wait(boost::bind(&server::backup_frq, this,
											 timer, tick));
				
			}

		}
	
	void listen()
		{
			boost::asio::ip::tcp::resolver resolver(_service);
			boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({_address, _port});
			_acceptor.open(endpoint.protocol());
			_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
			_acceptor.bind(endpoint);
			_acceptor.listen();
		}

	void accept()
		{
			auto self = shared_from_this();
			_acceptor.async_accept(_socket,
								   [this](boost::system::error_code ec)
								   {
									   if (!ec)
									   {
										   string host = _socket.remote_endpoint().address().to_string();
										   auto port = _socket.remote_endpoint().port();
										   _clients.emplace(std::make_pair(host, port), std::move(_socket));
										   
										   auto start_session = [&](){
											   std::make_shared<session<server>>(
												   host, port,
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
	
	int monitor(const string& host, unsigned short port)
		{
			write_lock wlock(_lock);
			auto iter = _clients.find(std::make_pair(host, port));
			if (iter == _clients.end())
				return -1;

			_sentry.emplace(std::make_pair(host, port),
							std::move(iter->second));
			_clients.erase(iter);

			return 0;
		}
	
	void send_slaves()
		{
			read_lock rlock(_lock);
			string str = std::to_string(_slaves.size());
			str.append("\n");
			
			for (auto && se : _sentry) {
					se.second.write_some(boost::asio::buffer(str.c_str(),
															 str.size()));
					log("send slaves size ", str);
				}
			
			for (auto && pis : _slaves) {
				str = pis.first.first;
				str.append(":");
				str.append(std::to_string(pis.first.second));
				str.append("\n");

				for (auto && se : _sentry) {
					se.second.write_some(boost::asio::buffer(str.c_str(),
															 str.size()));
					log("send ", str);
				}
			}
			
			log("write all slaves to ", " sentry");
		}
	
	tcp::socket&
	get_client(const string& host, unsigned short port)
		{
			read_lock rlock(_lock);
			auto iter = _clients.find(std::make_pair(host, port));
			if (iter != _clients.end())
				return iter->second;
			auto iter2 = _slaves.find(std::make_pair(host, port));
			if (iter2 != _slaves.end())
				return iter2->second;
			return _sentry.at(std::make_pair(host, port));
		}
	
	
	void
	remove_client(const string& host, unsigned short port)
		{
			write_lock wlock(_lock);
			auto iter = _clients.find({host, port});
			if (iter != _clients.end())
				_clients.erase(iter);
		}
		
	void
	add_slave(const string& hostname, unsigned short port)
		{
			write_lock wlock(_lock);
			log("start add slave");
			
			auto iter = _clients.find(std::make_pair(hostname, port));
			if (iter == _clients.end())
				return ;
			
			tcp::socket& sock = iter->second;

			_slaves.emplace(std::make_pair(hostname, port), std::move(sock));
			_clients.erase(iter);
			
			send_to_sentry(hostname, port);
		}

	void send_to_sentry(const string& hostname, unsigned short port)
		{
			if (_sentry.empty()) {
				return ;
			}
			string pr = hostname;
			pr.append(" ");
			pr.append(std::to_string(port));
			pr.append("\n");

			for (size_t i = 0; i != _sentry.size(); ++i) {
				auto key = _sentry.begin()->first;
				tcp::socket& sock = _sentry.begin()->second;
				
				if (sock.is_open()) {
					boost::asio::write(sock,
									   boost::asio::buffer(pr.c_str(), pr.size()));
				}
				else {
					_sentry.erase(key);
					--i;
				}
			}
		}

	void add_sentry(tcp::socket&& sock)
		{
			write_lock wlock(_lock);
			/* TO DO now support only one sentry */
			_sentry.emplace(std::make_pair(sock.remote_endpoint().address().to_string(),
										   sock.remote_endpoint().port()),
							std::move(sock));
			wlock.unlock();

			read_lock rlock(_lock);
			for (auto && slave : _slaves) {
				send_to_sentry(slave.first.first, slave.first.second);
			}
		}

	int shutdown()
		{
			backup_mdf();
			backup_aof();
			exit(0);
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

	std::shared_ptr<mdb>
	get_db(size_t pos)
		{
			read_lock rlock(_lock);
			return _dbs.at(pos);
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

	static void
	_backup_thread(std::shared_ptr<server> ptr,
				   std::shared_ptr<boost::asio::deadline_timer> t,
				   boost::posix_time::seconds tick)
		{
			ptr->backup_mdf();
			t->expires_at(t->expires_at() + tick);
			t->async_wait(boost::bind(_backup_thread,
									  ptr, t, tick));
			
		}
	
	void backup_frq(std::shared_ptr<boost::asio::deadline_timer> t,
					boost::posix_time::seconds tick)
		{
			boost::thread(boost::bind(_backup_thread,
									  this->shared_from_this(),
									  t, tick)).detach();
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
				ele.append("\n");
				_aof_stream << ele;
				_aof_stream.flush();

				for (auto && pis : _slaves) {
					pis.second.write_some(boost::asio::buffer(ele.c_str(),
															  ele.size()));
					log("sent cmd: ", ele);
				}
			}

			_aof_buffers.clear();
		}
	
	void
	backup_mdf() {
		write_lock rlock(_lock);
		log("start backup mdf");

		string tmpfile = std::tmpnam(nullptr);

		std::ofstream os(tmpfile, std::ios::binary);

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

		fs::mv(tmpfile, configs::mdf_path);
		log("end of backup mdf");
	}

	void
	backup_aof() {
		write_lock rlock(_lock);
		log("start saving aof");

		string tmpfile = std::tmpnam(nullptr);
		std::ofstream os(tmpfile);
		os << "resize " << _dbs.size() << std::endl;
		
		string cmd;
		for (size_t dbid = 0; dbid != _dbs.size(); ++dbid) {
			cmd = "select " + std::to_string(dbid) + "\n";
			os << cmd;
			_dbs[dbid]->write_aof(os);
		}

		fs::mv(tmpfile, configs::aof_path);
		log("end of aof backup");
	}

	int
    restore_from_mdf() {
		clear();
		write_lock wlock(_lock);
		log("start mdf restore");
		std::ifstream is(configs::mdf_path, std::ios::binary);
		if (!is || !is.is_open()) {
			log(configs::mdf_path, " open error");
			return -1;
		}
		
		size_t holder;
		string str;
		boost::crc_32_type crc;
		fs::read_from(is, str, crc);
		if (str != configs::MONOCO) {
			log("NOT support file");
			return -1;
		}

		long double version;
		fs::read_from(is, version, crc);
		if (version < configs::VERSION) {
			log("NOT support version");
			return -1;
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

		log("end of mdf restore");
		return 0;
	}

	int
	restore_from_aof()
		{
			clear();
			
			/* FIX ME since call handle_cmd will require wlock again*/
			//write_lock wlock(_lock);
			log("start aof restore");
			std::ifstream is(configs::aof_path);
			if (!is || !is.is_open()) {
				log("can't open ", configs::aof_path);
				//return exit(1);
				return -1;
			}

			auto ptr = std::make_shared<session<server>>(
				"", 0,
				this->shared_from_this());
			  
			char buffer[8196];
			while (is.getline(buffer, sizeof(buffer))) {
				string cmd(buffer, buffer + is.gcount());
				string reply;
	
				int ret = ptr->handle_request(cmd.begin(), cmd.end(),
											  reply);
				if (ret == -1) {
					log("restore from aof error");
					exit(-1);
				}
			}

			log("end of aof restore");
			return 0;
		}

	void
	_restore()
		{
			int ret = -1;
			if (configs::aof_restore) {
				ret = restore_from_aof();
			}
			if (ret == -1  && configs::mdf_restore) {
				ret = restore_from_mdf();
			}

			if (ret == -1) {
				log("restore error, quit");
				exit(1);
			}
				
		}
};

NAMESPACE_END(monoco)

#endif // end of __MONOCO_HPP
