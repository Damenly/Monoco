#ifndef __COMMON_M_HPP
#define __COMMON_M_HPP

#include <unordered_map>
#include <typeinfo>
#include "config.hpp"

NAMESPACE_BEGIN(monoco)

inline auto m_alloc(size_t sz)
{
	return malloc(sz);
}

inline auto m_free(void *ptr)
{
	return free(ptr);
}

enum type_encode
{
	te_cstr = 1,
	te_uint16 = 3,
	te_int16 = 4,
	te_uint32 = 5,
	te_int32 = 6,
	te_uint64 = 7,
	te_int64 = 8
};

uno
template <class T>
type_encode goo()
{
	auto foo = typeid(T);
	type_encode res;
	if (foo == typeid(char)
	    res = d
}
NAMESPACE_END(monoco)
#endif // __COMMON_M_HPP
