#ifndef  _CMDS_CPP
#define  _CMDS_CPP

#include <vector>
#include <string>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include "config.hpp"
#include "common.hpp"
#include "utility.hpp"
#include "server.hpp"
#include "db.hpp"

NAMESPACE_BEGIN(monoco)
using std::vector;
using std::shared_ptr;
using utility::args_to_zls;
using errs::print;

bool
is_valid(const string& cmd_name, size_t counts,
		 const vector<string>& args, bool loose)
{
	bool valid = false;
	if (loose && args.size() >= counts)
		valid = true;
	if (!loose)
		valid = args.size() == counts;
	if (boost::iequals(args[0], cmd_name)) {
		if (!valid) {
			return false;
		}
		return true;
	}
	return false;
}

inline
void
handle_str(string& str)
{
	if (str.size() < 3)
		return ;
	if ((str.front() == '\'' && str.back() == '\'') ||
		(str.front() == '"' && str.back() == '"')) {
		str.erase(0, 1);
		str.erase(str.size() - 1);
	}
}

template <typename T>
int
handle_cmds(std::shared_ptr<T> sess, vector<string>& args, string& reply)
{
	if (args.empty())
		return 0;
	
	if (sess == nullptr || sess->_server == nullptr) {
		print(reply, "invalid arguments");
		return -1;
	}
	
	try {
	vector<zl_entry> zls;
	auto ser = sess->_server;

	if (is_valid("resize", 2, args, false)) {
		ser->resize(stoull(args[1]));
		return 1;
	}

	if (is_valid("monitor", 1, args, false)) {
		return ser->monitor(sess->_host, sess->_port);
	}

	if (is_valid("get_slaves", 1, args, false)) {
		ser->get_slaves();
		return 0;
	}
	
	if (is_valid("add_slave", 1, args, false)) {
		ser->add_slave(sess->_host, sess->_port);
		return 0;
	}

	if (is_valid("send", 2, args, false)) {
		string file_name;
		if (boost::iequals("mdf", args[1]))
			file_name = configs::mdf_path;
		else if(boost::iequals("aof", args[1]))
			file_name = configs::aof_path;
		else if(boost::iequals("config", args[1]))
			file_name = configs::config_path;
		else
			return -1;
		
		return ser->send_file(sess->_host, sess->_port,
							 file_name);
	}

	if (is_valid("rmdb", 2, args, true)) {
		if (args.size() > 3)
			return -1;
		
		size_t start = std::stoull(args[1]);
		size_t finsh = start + 1;
		if (args.size() == 3)
			finsh = std::stoull(args[2]);
		return ser->remove_db(start, finsh);
	}
	
	if (is_valid("passwd", 2, args, false)) {
		ser->set_pwd(args[1]);
		return 0;
	}

	if (is_valid("rawpwd", 2, args, false)) {
		ser->set_pwd(args[1]);
		return 1;
	}

	if (is_valid("cur", 1, args, false)) {
		return sess->cur;
	}

	if (is_valid("size", 1, args, false)) {
		return ser->size();
	}
	
	if (is_valid("clear", 1, args, true)) {
		if (args.size() == 1) {
			ser->clear();
		}
		else {
			auto pos = std::stoull(args[1]);
			ser->clear(pos);
		}
		return 1;
	}

	if (is_valid("save", 1, args, false)) {
		string s;
		ser->backup();
		return 0;
	}

	if (is_valid("save_aof", 1, args, false)) {
		string s;
		ser->backup_aof();
		return 0;
	}

	if (is_valid("restore", 2, args, false)) {
		if (boost::iequals(args[1], "aof")) {
			ser->restore_from_aof();
			return 0;
		}
		if (boost::iequals(args[1], "mdf")) {
			ser->restore_from_mdf();
			return 0;
		}
		
		return -1;
	}
	
	if (is_valid("select", 2, args, false)) {
		auto pos = stoull(args[1]);
		if (pos >= ser->size())
			throw std::runtime_error("error db position");
		
		sess->cur = pos;
		return 1;
	}

	auto db = ser->_dbs.at(sess->cur);
	
	if (is_valid("remkey", 2, args, false)) {
		db->remove(args[1]);
		return 1;
	}

	if (is_valid("type", 2, args, false)) {
		print(reply, db->type(args[1]));
		return 0;
	}

	if (is_valid("exists", 2, args, false)) {
		print(reply, db->exists(args[1]));
		return 0;
	}
	
	if (is_valid("lindex", 3, args, false)) {
		auto res = db->lindex(args[1], std::stoull(args.back()));
		if (res.empty()) {
			print(reply, args[1], "not found");
			return -1;
		}
		print(reply, res);
		return 0;
	}

	if (is_valid("linsert", 5, args, false)) {
		zls.clear();
		utility::args_to_zls(args.begin() + 3, args.end(),
							 zls);
		bool before = false;
		if (boost::iequals("before", args[2]))
			before = true;
		else if (boost::iequals("before", args[2]))
			;
		else {
			print(reply, "invalid arguments");
			return -1;
		}

		return db->linsert(args[1], before, zls[0], zls[1]);
	}

	if (is_valid("llen", 2, args, false)) {
		auto sz = db->llen(args[1]);
		if (sz == types::size_t_max) {
			return -1;
		}
		else {
			print(reply, sz);
			return 0;
		}
	}

	if (is_valid("lpop", 2, args, false)) {
		auto res = db->lpop(args[1]);
		if (res.empty()) {
			return -1;
		}
		print(reply, res);
		return 1;
	}

	if (is_valid("lpush", 3, args, true)) {
		zls.clear();
		args_to_zls(args.begin() + 2, args.end(), zls);
		return db->lpush(args[1],zls);
	}

	if (is_valid("rpop", 2, args, false)) {
		auto res = db->rpop(args[1]);
		if (res.empty()) {
			return -1;
		}
		print(reply, res);
		return 1;
	}

	if (is_valid("rpush", 3, args, true)) {
		zls.clear();
		args_to_zls(args.begin() + 2, args.end(), zls);
		return db->rpush(args[1],zls);
	}

	if (is_valid("lpushx", 3, args, false)) {
		return db->lpushx(args[1], args[2]);
	}

	if (is_valid("rpushx", 3, args, false)) {
	    return db->rpushx(args[1], args[2]);
	}

	if (is_valid("lrange", 4, args, false)) {
		vector<zl_entry> res;
		int ret = db->lrange(res, args[1], std::stoull(args[2]),
							 std::stoull(args[3]));
		if (ret != 0) {
			return -1;
		}

		for (auto && ze : res) {
			print(reply, ze);
		}
		return 0;
	}

	if (is_valid("lrem", 4, args, false)) {
		zls.clear();
		args_to_zls(args.begin() + 3, args.end(), zls);
		return db->lrem(args[1], std::stoull(args[2]),
					   zls.front());
	}

	if (is_valid("lset", 4, args, false)) {
		zls.clear();
		args_to_zls(args.begin() + 3, args.end(), zls);
		return db->lset(args[1], std::stoull(args[2]), zls.front());
	}

	if (is_valid("ltrim", 4, args, false)) {
		return db->ltrim(args[1], std::stoull(args[2]),
						std::stoull(args[3]));
	}

	if (is_valid("sadd", 3, args, true)) {
		zls.clear();
		args_to_zls(args.begin() + 2, args.end(), zls);
		return db->sadd(args[1], zls);

	}

	if (is_valid("scard", 2, args, false)) {
		print(reply, db->scard(args[1]));
		return 0;
	}

	if (is_valid("sismember", 3, args, false)) {
		zls.clear();
		args_to_zls(args.begin() + 2, args.end(), zls);
		print(reply, db->sismember(args[1], zls[0]));
		return 0;
	}

	if (is_valid("srem", 3, args, false)) {
		zls.clear();
		args_to_zls(args.begin() + 2, args.end(), zls);
		return db->srem(args[1], zls[0]);
	}
	
	if (is_valid("smembers", 2, args, false)) {
		vector<zl_entry> res;
		int ret = db->smembers(res, args[1]);
		if (ret != 0) {
			return -1;
		}

		for (auto && ze : res) {
			std::stringstream ss;
			ss << ze << " ";
			reply.append(ss.str());
		}
		reply.append("\n");
		return 0;
	}

	if (is_valid("sdiff", 3, args, false)) {
		vector<zl_entry> res;
		int ret = db->sdiff(res, args[1], args[2]);
		if (ret != 0) {
			return -1;
		}

		for (auto && ze : res) {
			std::stringstream ss;
			ss << ze << " ";
			reply.append(ss.str());
		}
		print(reply, "");
		return 0;
	}

	if (is_valid("sunion", 3, args, false)) {
		vector<zl_entry> res;
		int ret = db->sunion(res, args[1], args[2]);
		if (ret != 0) {
			return -1;
		}

		for (auto && ze : res) {
			std::stringstream ss;
			ss << ze << " ";
			reply.append(ss.str());
		}
		print(reply, "");
		return 0;
	}

	if (is_valid("sinter", 3, args, false)) {
		vector<zl_entry> res;
		int ret = db->sinter(res, args[1], args[2]);
		if (ret != 0) {
			return -1;
		}

		for (auto && ze : res) {
			std::stringstream ss;
			ss << ze << " ";
			reply.append(ss.str());
		}
		print(reply, "");
		return 0;
	}

	if (is_valid("strlen", 2, args, false)) {
		print(reply, db->strlen(args[1]));
		return 0;
	}

	if (is_valid("set", 3, args, false)) {
		zls.clear();
		args_to_zls(args.begin() + 2, args.end(),
					zls);
		return db->set(args[1], zls[0]);
	}

	if (is_valid("append", 3, args, false)) {
		return db->append(args[1], args[2]);
	}

	if (is_valid("get", 2, args, false)) {
		print(reply, db->get(args[1]));
		return 0;
	}

	if (is_valid("incrby", 3, args, false)) {
		return db->incr_by(args[1], std::stold(args[2]));
	}

	if (is_valid("decrby", 3, args, false)) {
		print(reply, db->decr_by(args[1], std::stold(args[2])));
	}

	if (is_valid("decr", 2, args, false)) {
		return db->decr(args[1]);
	}

	if (is_valid("incr", 2, args, false)) {
		return db->incr(args[1]);
	}
	
	if (is_valid("zadd", 4, args, true)) {
		if (args.size() % 2 != 0) {
			print(reply, "invalid arguments");
			return -1;
		}

		vector<std::pair<string, long double>> ps;

		std::pair<string, long double> p;
		for (size_t i = 2; i != args.size(); ++i) {
			if (i % 2 == 0) {
				p.second = stold(args[i]);
			}
			else{
				handle_str(args[i]);
				p.first = args[i];
				ps.push_back(p);
			}
		}
		
		return(db->zadd(args[1], ps));
	}

	if (is_valid("zcard", 2, args, false)) {
		print(reply, db->zcard(args[1]));
		return 0;
	}

	if (is_valid("zscore", 3, args, false)) {
		print(reply, db->zscore(args[1], args[2]));
	}

	if (is_valid("zrank", 3, args, false)) {
		print(reply, db->zrank(args[1], args[2]));
		return 0;
	}

	if (is_valid("zrem", 3, args, false)) {
		return db->zrem(args[1], args[2]);
	}
	
	if (is_valid("zcount", 4, args, false)) {
		print(reply, db->zcount(args[1], std::stold(args[2]),
						 std::stold(args[3])));
		return 0;
	}

	if (is_valid("zincrby", 4, args, false)) {
		return db->zincr_by(args[1], args[2],
						 std::stold(args[3]));
	}
	
	if (is_valid("zrange", 4, args, true)) {
		vector<std::pair<string, long double>> res;
		int ret = db->zrange(res, args[1], std::stold(args[2]),
							 std::stold(args[3]));
		if (ret != 0) {
			return -1;
		}

		for (auto && ze : res) {
			print(reply, ze.first, " ", ze.second);
		}

		return 0;
	}

	if (is_valid("hcard", 2, args, false)) {
		print(reply, db->hcard(args[1]));
		return 0;
	}

	if (is_valid("hdel", 3, args, false)) {
		return db->hdel(args[1], args[2]);
	}

	if (is_valid("hget", 3, args, false)) {
		print(reply, db->hget(args[1], args[2]));
	}

	if (is_valid("hexists", 3, args, false)) {
		print(reply, db->hexists(args[1], args[2]));
		return 0;
	}

	if (is_valid("hset", 4, args, false)) {
		zls.clear();
		args_to_zls(args.begin() + 3, args.end(), zls);
		return db->hset(args[1], args[2], zls[0]);
	}
	
	if (is_valid("hgetall", 2, args, false)) {
		vector<std::pair<string, zl_entry>> res;
		int ret = db->hgetall(res, args[1]);
		if (ret != 0) {
			return -1;
		}

		for (auto && ze : res) {
			print(reply, ze.first, " ", ze.second);
		}
		return 0;
	}

	if (is_valid("hvals", 2, args, false)) {
		vector<zl_entry> res;
		int ret = db->hvals(res, args[1]);
		if (ret != 0) {
			return -1;
		}

		for (auto && ze : res) {
			print(reply, ze);
		}
		return 0;
	}

	if (is_valid("hkeys", 2, args, false)) {
		vector<string> res;
		int ret = db->hkeys(res, args[1]);
		if (ret != 0) {
			return -1;
		}

		for (auto && ze : res) {
			print(reply, ze);
		}
		return 0;
	}
	}
	catch (...) {
		
	}
	return -1;
}

NAMESPACE_END(monoco)

#endif
