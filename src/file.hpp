 #ifndef _M_FILE_HPP
#define _M_FILE_HPP

#include <fstream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/crc.hpp>

#include "config.hpp"
#include "common.hpp"

NAMESPACE_BEGIN(monoco)

NAMESPACE_BEGIN(fs)

using namespace boost::filesystem;
using namespace boost::system;
using boost::system::error_code;

template <typename T>
std::ofstream& write_to(std::ofstream & os,const T& val, boost::crc_32_type& crc)
{
	os.write(reinterpret_cast<const char *>(&val), sizeof(val));
	crc.process_bytes(&val, sizeof(val));
	
	return os;
}

template <>
std::ofstream&
write_to<string>(std::ofstream & os, const string& val, boost::crc_32_type& crc)
{
	size_t holder = val.size();
	write_to(os, holder, crc);
	
	os.write(val.c_str(), val.size());
	crc.process_bytes(val.c_str(), val.size());
	
	return os;
}

template <typename T>
std::ifstream& read_from(std::ifstream & is, T& val, boost::crc_32_type& crc)
{
	is.read(reinterpret_cast<char *>(&val), sizeof(val));
	crc.process_bytes(&val, sizeof(val));
	
	return is;
}

template <>
std::ifstream&
read_from<string>(std::ifstream & is, string& val, boost::crc_32_type& crc)
{
	size_t sz;
	read_from(is, sz, crc);

	auto buffer = std::shared_ptr<char>(new char[sz]);
	is.read(buffer.get(), sz);
	crc.process_bytes(buffer.get(), sz);

	val = string(buffer.get(), buffer.get() + sz);
	return is;
}

bool
is_exists(const string& file_name)
{
	path p(file_name);

	try{
	exists(file_name);
	}
	catch(...) {
		return false;
	}
	return true;
}

bool
is_regular(const string& file_name)
{
	path p(file_name);
	bool res;

	error_code ec;
	try{
		res = is_regular(p, ec);
	}
	catch(...) {
		res = false;
	}
	return res;
}

int
rm(const string& file_name)
{
	try {
		remove(path(file_name));
	}
	catch(...) {
		return -1;
	}

	return 0;
}

string
pwd()
{
	return current_path().string();
}

int
touch(const string& file_name)
{
	try{
	boost::filesystem::ofstream(path(file_name));
	}
	catch(...){
		return -1;
	}
	return 0;
}

void
mv(const string& old_name, const string& new_name)
{
	if(is_exists(new_name)) {
		rm(new_name);
		errs::log(new_name, " exists, removed it");
	}

	rename(path(old_name), path(new_name));
}

NAMESPACE_END(fs)
NAMESPACE_END(monoco)

#endif
