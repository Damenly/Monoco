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

#ifndef __MBJ_HPP_
#define __MBJ_HPP_

#include "config.hpp"
#include <memory>


NAMESPACE_BEGIN(monoco)
using std::string;

class mbj
{
public:
	static constexpr uint8_t LRU_BITS = 24;
	static constexpr size_t  MAX_CLOCK =  (1 << LRU_BITS) - 1;
	static constexpr uint8_t CLOCK_RESLUTION = 1000;

	static constexpr uint8_t RAW_TYPE = 16;
	static constexpr uint8_t STR_TYPE = 0;
	static constexpr uint8_t LS_TYPE = 1;
	static constexpr uint8_t SET_TYPE = 2;
	static constexpr uint8_t HASH_TYPE = 4;
	static constexpr uint8_t ZSET_TYPE = 8;
	

	static constexpr uint8_t INT_ENCODE = 1;
	static constexpr uint8_t FT_ENCODE = 128;
	static constexpr uint8_t STR_ENCODE = 0;
	static constexpr uint8_t VEC_ENCODE = 4;
	static constexpr uint8_t LS_ENCODE = 4;
	static constexpr uint8_t HT_ENCODE = 2;
	static constexpr uint8_t INTVEC_ENCODE = 8;
	static constexpr uint8_t RB_ENCODE = 32;
	
	typedef std::size_t           size_type;
	
private:
	unsigned _type:4;
	unsigned _encode:4;
	unsigned _lru:LRU_BITS;

	std::shared_ptr<void*> _content;
public:
	mbj() {}
	mbj(const std::string &str);
	mbj(std::string &&str);
	mbj(int64_t val);
	mbj(long double val);

	void create_vector();
	void create_list();
};
NAMESPACE_END(monoco)
#endif  // __MBJ_HPP_