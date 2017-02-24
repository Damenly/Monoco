#include "config.hpp"
#include "mbj.hpp"
#include "mlist.hpp"
#include <string>

NAMESPACE_BEGIN(monoco)

mbj::mbj(const std::string& str)
{
	_content = std::make_shared<void>(new std::string(str));
	_type = STR_TYPE;
	_encode = STR_ENCODE;
}

mbj::mbj(std::string&& str)
{	
	_content = std::make_shared<void>(new std::string(str));
	_type = STR_TYPE;
	_encode = STR_ENCODE;
}

mbj::mbj(int64_t val)
{
	_content = std::make_shared<void>(new int64_t(val));
	_type = RAW_TYPE;
	_encode = INT_ENCODE;
}

mbj::mbj(long double val)
{
	std::string tmp = std::to_string(val);
	_content = std::make_shared<void>(new std::string(std::move(tmp)));
	_type = STR_TYPE;
	_encode = FT_ENCODE;
}

void
mbj::create_vector()
{
	_content = std::make_shared<void>(new mvector());
	_type = LS_TYPE;
	_encode = VEC_ENCODE;
}

void
mbj::create_list()
{
	_content = std::make_shared<void>(new mlist());
	_type = LS_TYPE;
	_encode = LS_ENCODE;
}

NAMESPACE_END(monoco)

