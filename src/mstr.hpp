#ifndef _M_STR_HPP_
#define _M_STR_HPP_

#include <string>
#include "common.hpp"
#include "mbj.hpp"

NAMESPACE_BEGIN(monoco)

class mstr : public std::string, public mbj
{
public:
	mstr() {}
	mstr(const char* cstr): string(cstr) {}
	mstr(const string& str): string(str) {}
	mstr(const mstr& other): string(other) {}

	virtual string type_name() const {return "mstr";}
	virtual std::size_t size() const {return string::size();}
	
	virtual std::ofstream&
	write_to(std::ofstream& os, boost::crc_32_type& crc) const
		{
			fs::write_to(os, static_cast<string>(*this), crc);
			return os;
		}

	virtual std::ifstream&
	read_from(std::ifstream& is, boost::crc_32_type& crc)
		{
			fs::read_from(is, *dynamic_cast<string*>(this), crc);
			return is;
		}
	
};



NAMESPACE_END(monoco)

namespace std {
	template <>
	struct hash<monoco::mstr>
	{
		std::size_t operator()(const monoco::mstr& str) const
			{
				return hash<string>{}(*static_cast<const string*>(&str));
			}
	};
}

#endif 
