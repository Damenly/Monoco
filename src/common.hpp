/*
 * Copyright (c) 2017, Damenly Su <Damenly at live dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __COMMON_M_HPP
#define __COMMON_M_HPP

#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <functional>
#include <limits>
#include <sstream>

#include <boost/system/error_code.hpp>

#include "config.hpp"


NAMESPACE_BEGIN(monoco)

NAMESPACE_BEGIN(errs)

template <typename T>
void error(const T& msg)
{
	std::cerr << msg << std::endl;
}

template <typename T, typename... Args>
void error(const T& arg, Args... args)
{
	std::cerr << arg;
	error(args...);
}

template <typename T>
void log(const T& msg)
{
	std::cerr << msg << std::endl;
}

template <typename T, typename... Args>
void log(const T& arg, Args... args)
{
	std::cerr << arg;
	error(args...);
}


template <typename T>
void print(string &str, const T& msg)
{
	std::stringstream ss;
	ss << msg << "\n";
	str.append(ss.str());
}

template <typename T, typename... Args>
void print(string & str, const T& arg, Args... args)
{
	std::stringstream ss;
	ss << arg;
	str.append(ss.str());
	print(str, args...);
}

typedef boost::system::error_code err_code;
using namespace boost::system::errc;

auto
make_err(errc_t e)
{
	return make_error_code(e);
}
	
NAMESPACE_END(err)

NAMESPACE_BEGIN(types)

using std::hash;
using std::type_index;



template <typename T>
string type_name()
{
	return "unknown type";
}

template <>
string type_name<string>() {return "string";}

template <>
string type_name<std::int8_t>() {return "int8_t";}

template <>
string type_name<std::int16_t>() {return "int16_t";}

template <>
string type_name<std::int32_t>() {return "int32_t";}

template <>
string type_name<std::uint8_t>() {return "uint8_t";}

template <>
string type_name<std::uint16_t>() {return "uint16_t";}

template <>
string type_name<std::uint32_t>() {return "uint32_t";}

template <>
string type_name<bool>() {return "bool";}

template <typename T>
inline size_t hash_idx()
{
	return hash<type_index>{}(type_index(typeid(T)));
}

static const size_t M_INT64 = hash<type_index>{}(type_index(typeid(int64_t)));
static const size_t M_INT32 = hash<type_index>{}(type_index(typeid(int32_t)));
static const size_t M_INT16 = hash<type_index>{}(type_index(typeid(int16_t)));
static const size_t M_INT8 = hash<type_index>{}(type_index(typeid(int8_t)));

static const size_t M_UINT64 = hash<type_index>{}(type_index(typeid(uint64_t)));
static const size_t M_UINT32 = hash<type_index>{}(type_index(typeid(uint32_t)));
static const size_t M_UINT16 = hash<type_index>{}(type_index(typeid(uint16_t)));
static const size_t M_UINT8 = hash<type_index>{}(type_index(typeid(uint8_t)));

static const size_t M_FT = hash<type_index>{}(type_index(typeid(float)));
static const size_t M_DB = hash<type_index>{}(type_index(typeid(double)));
static const size_t M_LD = hash<type_index>{}(type_index(typeid(long double)));

static const size_t M_BOOL = hash<type_index>{}(type_index(typeid(bool)));

static const size_t M_STR = hash<type_index>{}(type_index(typeid(string)));

static constexpr size_t size_t_max = std::numeric_limits<std::size_t>::max();

bool is_int(size_t encode)
{
	if (//encode == M_INT8 ||
		encode == M_INT16 ||
		encode == M_INT32 ||
		encode == M_INT64)
		return true;
	return false;
}

bool is_float(size_t encode)
{
	if (encode == M_FT ||
		encode == M_DB ||
		encode == M_LD)
		return true;
	return false;
}

bool is_uint(size_t encode)
{
	if (//encode == M_UINT8 ||
		encode == M_UINT16 ||
		encode == M_UINT32 ||
		encode == M_UINT64)
		return true;
	return false;
}

template <typename T>
std::size_t hash_type()
{
	return sizeof(T);
}

template <>
std::size_t hash_type<int8_t>()
{
	return M_INT8;
}

template <>
std::size_t hash_type<int16_t>()
{
	return M_INT16;
}

template <>
std::size_t hash_type<int32_t>()
{
	return M_INT32;
}

template <>
std::size_t hash_type<int64_t>()
{
	return M_INT64;
}


template <>
std::size_t hash_type<uint8_t>()
{
	return M_UINT8;
}

template <>
std::size_t hash_type<uint16_t>()
{
	return M_UINT16;
}

template <>
std::size_t hash_type<uint32_t>()
{
	return M_UINT32;
}

template <>
std::size_t hash_type<uint64_t>()
{
	return M_UINT64;
}

template <>
std::size_t hash_type<string>()
{
	return M_STR;
}

template <>
std::size_t hash_type<float>()
{
	return M_FT;
}

template <>
std::size_t hash_type<double>()
{
	return M_DB;
}
template <>
std::size_t hash_type<long double>()
{
	return M_LD;
}
NAMESPACE_END(types)

NAMESPACE_BEGIN(hashers)

struct pairhash {
public:
  template <typename T, typename U>
  std::size_t operator()(const std::pair<T, U> &x) const
  {
    return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
  }
};

NAMESPACE_END(hashers)
NAMESPACE_END(monoco)
#endif // __COMMON_M_HPP
