#ifndef __M_UTILITY_
#define __M_UTILITY_

#include <random>
#include <cassert>
#include <cstdint>
#include <typeindex>
#include <functional>
#include <vector>
#include <cctype>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

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

		if(try_convert<int16_t>(res, arg) == 0) continue;
		if(try_convert<int32_t>(res, arg) == 0) continue;
		if(try_convert<int64_t>(res, arg) == 0) continue;

		if(try_convert<long double>(res, arg) == 0) continue;
		if (try_convert<string>(res, arg) == 0) continue;
		
		res.emplace_back(arg);
	}

	return 0;
}

NAMESPACE_END(utility)

NAMESPACE_END(monoco)


#endif // __M_UTILITY_
