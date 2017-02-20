#ifndef __M_UTILITY_
#define __M_UTILITY_

#include <random>
#include <cassert>
#include <cstdint>
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

NAMESPACE_END(monoco)


#endif // __M_UTILITY_
