#ifndef _REQUEST_HPP
#define _REQUEST_HPP

#include <string>
#include <vector>
#include "config.hpp"

NAMESPACE_BEGIN(monoco)

struct request
{
	std::vector<std::string> args;
};

NAMESPACE_END(monoco)

#endif
