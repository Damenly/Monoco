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

#ifndef INTVEC
#define INTVEC

#define INTVEC

#include <vector>
#include <iterator>
#include <memory>
#include <typeinfo>
#include <cstdint>
#include <iostream>

NAMESPACE_BEGIN(monoco)

#define INTVEC_ENC_INT16 (sizeof(int16_t))
#define INTVEC_ENC_INT32 (sizeof(int32_t))
#define INTVEC_ENC_INT64 (sizeof(int64_t))

class intvec
{
public:
	typedef std::size_t                   size_type;
	typedef std::vector<int16_t>          Vec_INT_16;
	typedef std::vector<int32_t>          Vec_INT_32;
	typedef std::vector<int64_t>          Vec_INT_64;
	typedef uint8_t                       encode_type;

	intvec() = default;
	~intvec();
	
	size_type size() const;
	int64_t random() const;
	void add(int16_t val);
	void add(int32_t val);
	void add(int64_t val);
	void remove(int64_t val);
private:
	void *_vec = nullptr;
	encode_type _encode = 0;
	
private:
	void _evolve(encode_type new_code);
	template <class T>
	std::vector<T>* _typed_vec() const;

public:

	template <class T>
	void creat()
		{
			_vec = reinterpret_cast<void *>(new std::vector<T>());
			_encode = sizeof(T);
		}
	template <class T>
	void get(size_type pos, T* val_ret) const;
	int64_t get(size_type pos) const;
};

NAMESPACE_END(monoco)
#endif  // INTVEC
