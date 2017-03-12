#ifndef __ZL_ENTRY_HPP_
#define __ZL_ENTRY_HPP_

#include <string>
#include <boost/any.hpp>
#include "config.hpp"
#include "any.hpp"
#include "common.hpp"
#include "mbj.hpp"
#include "file.hpp"

NAMESPACE_BEGIN(monoco)

template <size_t T>
struct foo
{
	union {
		char ptr[T];
	};
};

struct zl_entry : public mbj
{
	std::shared_ptr<void> _content = nullptr;
	std::size_t encode = 0;
	
	zl_entry() = default;
	zl_entry(const zl_entry& other)
		{
			encode = other.encode;
			_content = other._content;
		}
	
	virtual string type_name() const {return "zl_entry";}

	virtual ~zl_entry() {}

	bool empty() const {return _content == nullptr;}

	zl_entry(const char *str)
		{
			encode = types::M_STR;
			_content = std::make_shared<string>(str);
		}
	
	template <typename T>
	zl_entry(const T& val)
		{
			encode = types::hash_type<T>();
			_content = std::make_shared<T>(val);
		}
		
	template <class T>
	T safe_data() const
		{
			return *std::static_pointer_cast<T>(_content);
		}

	template <class T>
	T& safe_data()
		{
			return *std::static_pointer_cast<T>(_content);
		}

	template <typename T>
	T* unsafe_data()
		{
			return reinterpret_cast<T*>(_content.get());
		}

	char* get() {return reinterpret_cast<char*>(_content.get());}
	const char* get() const {return reinterpret_cast<char*>(_content.get());}
	
	bool operator==(const zl_entry& other) const
		{
			if (other.encode != encode)
				return false;
			if (types::is_int(encode)) {
				return to_s64() == other.to_s64();
			}
			if (types::is_uint(encode))
				return to_u64() == other.to_u64();
			if (types::M_STR == encode)
				return safe_data<std::string>()
					== other.safe_data<std::string>();
			return false;
		}

	bool operator<(const zl_entry& other) const
		{
			if (other.encode != encode)
				return false;
			if (types::is_int(encode))
				return to_s64() < other.to_s64();
			if (types::is_uint(encode))
				return to_u64() < other.to_u64();
			if (types::M_STR == encode)
				return safe_data<std::string>()
					< other.safe_data<std::string>();
			if (memcmp(_content.get(), other._content.get(), encode) < 0)
				return true;
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
	
	virtual std::ofstream&
	write_to(std::ofstream& os, boost::crc_32_type& crc) const
		{
			size_t holder = this->encode;
			fs::write_to(os, holder, crc);

			const char* desc = this->get();
			size_t sz;
	
			if (holder == types::M_STR) {
				desc = this->to_string().c_str();
				sz  = this->to_string().size();
				fs::write_to(os, sz, crc);
			}
			else if (holder == types::M_BOOL)
				sz = sizeof(bool);
			else if (holder == types::M_INT8 ||
					 holder == types::M_UINT8)
				sz = sizeof(int8_t);
			else if (holder == types::M_INT16 ||
					 holder == types::M_UINT16)
				sz = sizeof(int16_t);
			else if (holder == types::M_INT32 ||
					 holder == types::M_UINT32)
				sz = sizeof(int32_t);
			else if (holder == types::M_INT64 ||
					 holder == types::M_UINT64)
				sz = sizeof(int64_t);
			else if (holder == types::M_FT)
				sz = sizeof(float);
			else if (holder == types::M_DB)
				sz = sizeof(double);
			else if (holder == types::M_LD)
				sz = sizeof(long double);
			else
				sz = this->encode;
			
			os.write(desc, sz);
			crc.process_bytes(desc, sz);
			return os;
		}

	virtual std::ifstream&
	read_from(std::ifstream & is, crc_32_type& crc)
		{
			size_t holder;
			fs::read_from(is, holder, crc);
			this->encode = holder;
			
			size_t sz;
	
			if (holder == types::M_STR) {
				fs::read_from(is, sz, crc);
			}
			else if (holder == types::M_BOOL)
				sz = sizeof(bool);
			else if (holder == types::M_INT8 ||
			 holder == types::M_UINT8)
				sz = sizeof(int8_t);
			else if (holder == types::M_INT16 ||
					 holder == types::M_UINT16)
				sz = sizeof(int16_t);
			else if (holder == types::M_INT32 ||
					 holder == types::M_UINT32)
				sz = sizeof(int32_t);
			else if (holder == types::M_INT64 ||
					 holder == types::M_UINT64)
				sz = sizeof(int64_t);
			else if (holder == types::M_FT)
				sz = sizeof(float);
			else if (holder == types::M_DB)
		sz = sizeof(double);
			else if (holder == types::M_LD)
				sz = sizeof(long double);
			else
				sz = this->encode;

			auto buffer = std::shared_ptr<char>(new char[sz]);

			is.read(buffer.get(), sz);
			crc.process_bytes(buffer.get(), sz);
			
			if (this->encode == types::M_STR) {
				this->_content = std::make_shared<string>
					(buffer.get(), buffer.get() + sz);
			}
			else {
				this->_content = buffer;
			}

			return is;
		}
   
	int64_t to_s64() const;
	uint64_t to_u64() const;
	long double to_ld() const;
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
		res = unsigned(safe_data<uint8_t>());
	else if(encode == types::M_UINT16)
		res = safe_data<uint16_t>();
	else if(encode == types::M_UINT32)
		res = safe_data<uint32_t>();
	else if(encode == types::M_UINT64)
		res = safe_data<uint64_t>();
	return res;
}

long double zl_entry::to_ld() const
{
	long double res = std::numeric_limits<long double>::max();
	if(encode == types::M_FT)
		res = safe_data<float>();
	else if(encode == types::M_DB)
		res = safe_data<double>();
	else if(encode == types::M_LD)
		res = safe_data<long double>();
	return res;
}

std::ostream & operator<<(std::ostream & os, const zl_entry & ze)
{
	if (types::is_int(ze.encode))
		os << ze.to_s64();
	if (types::is_uint(ze.encode))
		os << ze.to_u64();
	if (types::is_float(ze.encode))
		os << ze.to_ld();
	if (types::M_STR == ze.encode)
		os << ze.to_string();
	return os;
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
