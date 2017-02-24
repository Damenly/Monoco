#ifndef __ZL_ENTRY_HPP_
#define __ZL_ENTRY_HPP_

#include <string>
#include <boost/any.hpp>
#include "config.hpp"
#include "any.hpp"
#include "common.hpp"

NAMESPACE_BEGIN(monoco)

struct zl_entry
{
	any _content;
	std::size_t encode = 0;
	
	zl_entry() = default;
	zl_entry(const zl_entry& other)
		{
			encode = other.encode;
			_content = other._content;
		}
	/*
	zl_entry(zl_entry&& other)
		{
			std::swap(_content, other._content);
			std::swap(encode, other.encode);
		}
	*/
	~zl_entry() {}

	bool empty() const {return _content.empty();}

	zl_entry(const char *str)
		{
			encode = types::M_STR;
			_content = any(std::move(std::string(str)));
		}
	
	template <typename T>
	zl_entry(const T& val)
		{
			encode = types::hash_type<T>();
			_content = any(val);
		}
		
	template <class T>
	auto safe_data() const
		{
			return any_cast<T>(_content);
		}

	template <class T>
	auto unsafe_data() const
		{
			return *(unsafe_any_cast<T>(&_content));
		}

	bool operator==(const zl_entry& other) const
		{
			if (other.encode != encode)
				return false;
			if (types::is_int(encode))
				return to_s64() == other.to_s64();
			if (types::is_uint(encode))
				return to_u64() == other.to_u64();
			if (types::M_STR == encode)
				return safe_data<std::string>()
					== other.safe_data<std::string>();

			return false;
		}

	bool operator !=(const zl_entry& other) const
		{
			return !operator==(other);
		}

	std::string to_string() const
		{
			return safe_data<std::string>();
		}

	template <typename T>
	bool is_safe() const
		{
			return encode == types::hash_type<T>();
		}
	
	int64_t to_s64() const;
	uint64_t to_u64() const;
};

int64_t zl_entry::to_s64() const
{
	int64_t res = INT64_MAX;
	if(encode == types::M_INT8)
		res = safe_data<int8_t>();
	else if(encode == types::M_INT16)
		res = safe_data<int16_t>();
	else if(encode == types::M_INT32)
		res = safe_data<int32_t>();
	else if(encode == types::M_INT64)
		res = safe_data<int64_t>();
	return res;
}

uint64_t zl_entry::to_u64() const
{
	uint64_t res = UINT64_MAX;
	if(encode == types::M_UINT8)
		res = safe_data<uint8_t>();
	else if(encode == types::M_UINT16)
		res = safe_data<uint16_t>();
	else if(encode == types::M_UINT32)
		res = safe_data<uint32_t>();
	else if(encode == types::M_UINT64)
		res = safe_data<uint64_t>();
	return res;
}

NAMESPACE_END(monoco)

namespace std {
	template <>
	struct hash<monoco::zl_entry>
	{
		std::size_t operator()(const monoco::zl_entry& ze) const
			{
				std::size_t res = hash<size_t>{}(ze.encode);
				if (monoco::types::is_int(ze.encode))
					res ^= hash<int64_t>{}(ze.to_s64());
				if (monoco::types::is_uint(ze.encode))
					res ^= hash<uint64_t>{}(ze.to_u64());
				if (monoco::types::M_STR == ze.encode)
					res ^= hash<string>{}(ze.to_string());
				return res;
			}
	};
}
#endif
