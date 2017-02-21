#ifndef __ZLIST_HPP_
#define __ZLIST_HPP_

#include <cstring>
#include <vector>
#include "config.hpp"
#include "common.hpp"

NAMESPACE_BEGIN(monoco)

#define ZL_CSTR               sizeof(char)
#define ZL_INT                sizeof(int)
/*
 * zl only store pod data,so use malloc not new
 */
struct zl_entry
{
	int32_t encode = 0;
	void *ptr = nullptr;
	
	zl_entry() = default;
	zl_entry(int8_t val)  {creat(val);}
	zl_entry(int16_t val) {creat(val);}
	zl_entry(int32_t val) {creat(val);}
	zl_entry(int64_t val) {creat(val);}
	zl_entry(const char* cstr) {creat(cstr);}
	
	~zl_entry() {destroy();}

	bool empty() const {return ptr == nullptr;}
	void destroy()
		{
			if (encode == ZL_CSTR)
				delete[] static_cast<char *>(ptr);
			ptr = nullptr;
			encode = 0;
		}
	
	template <class T>
	void creat(const T& val)
		{
			destroy();
			ptr = reinterpret_cast<void*>(val);
			encode = ZL_INT;
		}
	
	void creat(const char *str)
		{
			auto len = strlen(str);
			ptr = new char[len + 1];
			memcpy(ptr, str, len + 1);
			encode = ZL_CSTR;
		}
	
	template <class T>
	T data()
		{
			return *(T*)(&ptr);
		}

	int64_t data() {return data<int64_t>();}
};

typedef std::vector<zl_entry> zlist;

NAMESPACE_END(monoco)

#endif // __ZLIST_HPP_
