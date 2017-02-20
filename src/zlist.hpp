#ifndef __ZLIST_HPP_
#define __ZLIST_HPP_

#include "config.hpp"
#include "common.hpp"

NAMESPACE_BEGIN(monoco)

/*
 * zl only store pod data,so use malloc not new
 */
struct zl_entry
{
	zl_entry() = default;
	~zl_entry() {m_free(ptr);}

	type_encode ee;
	template <class T>
	void creat(const T& val)
		{
			T* tmp = m_alloc(sizeof(T));
			*tmp = val;
			ptr = reinterpret_cast<void*>(tmp);
			ee = get_encode<T>();
		}
	
	void *ptr = nullptr;
	template <class T>
	T& data()
		{
			return *reinterpret_cast<T*>(ptr);
		}

};

NAMESPACE_END(monoco)

#endif // __ZLIST_HPP_
