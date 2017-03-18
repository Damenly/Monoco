#ifndef __M_UTILITY_
#define __M_UTILITY_

#include <random>
#include <cassert>
#include <cstdint>
#include <typeindex>
#include <functional>
#include <vector>
#include <cctype>
#include <fstream>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <openssl/md5.h>

#include "zl_entry.hpp"
#include "config.hpp"


NAMESPACE_BEGIN(monoco)

class random_machine
{
public:
	static size_t gen_uint(const size_t& start, const size_t& end)
		{
			assert(start < end);
			std::random_device rd;
			std::mt19937_64 gen(rd()); 
			std::uniform_int_distribution<> dis(start, end);
			return dis(gen);
		}

	static double gen_real(const double& start, const double& end)
		{
			assert(start < end);
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dis(start, end);
			return dis(gen);
		}
};

NAMESPACE_BEGIN(utility)

string
md5(const string& pwd) {
	unsigned char buffer[MD5_DIGEST_LENGTH] = {};
	MD5_CTX ctx;
	MD5_Init(&ctx);
	
	MD5_Update(&ctx, pwd.c_str(), pwd.size());
	MD5_Final(buffer, &ctx);

	return string(std::begin(buffer), std::end(buffer));
}

int 
check_arg(string& arg)
{
	for (auto && ch : arg) {
		if (!(std::isdigit(ch) ||
			  ch == '.'))
			return 1;
	}

	if (arg.empty())
		return 1;
	return 0;
}

template <typename T>
static int
_try_convert(const string & str, T* res)
{
	try {
	    *res = boost::lexical_cast<T>(str);
	}
	catch (...)
	{
		return 1;
	}
	return 0;
}

template <typename T>
int try_convert(std::vector<zl_entry>& res,
				string& arg)
{
	int fail = 0;
	T val;
	
	fail = _try_convert(arg, &val);

	if (fail == 0) {
		res.emplace_back(val);
	}

	return fail;
}


template <>
int
try_convert<string>(std::vector<zl_entry>& res,
					string& arg)
{
	return 1;

	try {
		if (arg.front() == '\'' ||
			arg.front() == '"') {
			arg.erase(arg.begin());
			arg.erase(arg.begin() + (arg.size() - 1));
		}
		res.emplace_back(arg);
		return 0;
	}
	catch (...) {
		return 1;
	}
	return 1;
}

int
args_to_zls(std::vector<string>::iterator first,
			std::vector<string>::iterator last,
			std::vector<zl_entry>& res)
{
	while(first != last) {
		auto arg = *first;
		std::advance(first, 1);

		if (arg.size() >= 3 &&
			((arg.front() == '"' && arg.back() == '"') ||
			 (arg.front() == '\'' && arg.back() == '\'')))
		{
			arg.erase(0, 1);
			arg.erase(arg.size() - 1);
			res.push_back(zl_entry(arg));
			continue;
		}
		
		if(try_convert<int16_t>(res, arg) == 0) continue;
		if(try_convert<int32_t>(res, arg) == 0) continue;
		if(try_convert<int64_t>(res, arg) == 0) continue;

		if(try_convert<long double>(res, arg) == 0) continue;
		if (try_convert<string>(res, arg) == 0) continue;
		
		res.emplace_back(arg);
	}

	return 0;
}

#define PSULL(a)  ((a)=(std::stoull(pmap.at(#a))))
#define PS(a)  ((a)=(pmap.at(#a)))

void parse_config()
{
	std::ifstream is(configs::config_path);
	if (!is) {
		errs::log("cannot open config file");
		exit(1);
	}

	std::unordered_map<string, string> pmap;
	
	while(1) {
		string line;
		std::getline(is, line);
		if (!is)
			break;
		std::istringstream iss(line);
		
		string key, val;
		iss >> key;
		iss >> val;

		if (val.size() >= 3 &&
			((val.front() == '\'' && val.back() == '\'') ||
			 (val.front() == '"' && val.back() == '"')))
		{
			val.erase(val.begin());
			val.erase(val.end() -1 );
		}
			
		pmap.emplace(key, val);
	}

	using namespace configs;
	try{

	VERSION = std::stold(pmap.at("VERSION"));

	PSULL(mzset_max_size);
	PSULL(mzset_max_len);
	PSULL(mset_max_size);
	PSULL(mset_max_len);
	PSULL(mht_max_size);
	PSULL(mht_max_len);
	PSULL(mlist_max_size);
	PSULL(mlist_max_len);

	PSULL(heartbeat_tick);

	cmd_aof_counts = std::stoll(pmap.at("cmd_aof_counts"));
	backup_mdf_seconds = std::stoll(pmap.at("backup_mdf_seconds"));
	mdf_restore = pmap.at("mdf_restore").front() == 't';
	aof_restore= pmap.at("aof_restore").front() == 't';
	PS(mdf_path);
	PS(aof_path);
	PS(config_path);
	PSULL(init_db_num);
	}
	catch (...)
	{
		errs::log("config error");
		exit(1);
	}
}
NAMESPACE_END(utility)

NAMESPACE_END(monoco)


#endif // __M_UTILITY_
