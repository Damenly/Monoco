/* 
 * Copyright (c) 2017, Damenly Su <Damenly at outlook dot com>
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

#ifndef __MDS_HPP_
#define __MDS_HPP_

#include <cstddef>
#include <cstring>
#include <string>
#include <memory>
#include <iterator>
#include "config.hpp"

NAMESPACE_BEGIN(monoco)

template <class T, class Alloc=std::allocator<T>>
class base_mds
{
public:
	typedef Alloc                                    allocator_type;
	typedef T                                        value_type;
	typedef typename allocator_type::reference       reference;
	typedef typename allocator_type::const_reference const_reference;
	typedef typename allocator_type::size_type       size_type;
	typedef typename allocator_type::difference_type difference_type;
	typedef typename allocator_type::pointer         pointer;
	typedef typename allocator_type::const_pointer   const_pointer;
//	typedef std::reverse_iterator<iterator>          reverse_iterator;
//	typedef std::reverse_iterator<const_iterator>    const_reverse_iterator;

	base_mds();
	base_mds(size_type n);
	base_mds(const char* cstr);
	base_mds(const std::string& str);
	base_mds(const base_mds& str);
	base_mds(base_mds&& str);
	base_mds& operator=(const base_mds& rstr);
	base_mds operator+(const base_mds& rstr);
	base_mds operator+(const base_mds& rstr) const;
	base_mds& operator+=(const base_mds& rstr);
	bool operator!=(const base_mds& rstr);
	bool operator==(const base_mds& rstr);
	~base_mds();
	
	size_type size() const  {return _len;}
	size_type flags() const {return _flags;}
	size_type avail() const {return _alloc_sz - _len;}
	size_type alloc() const {return _alloc_sz;}
	size_type capacity() const {return _alloc;}
	const_pointer c_str() const {return _real;}
	
	void set_flags(unsigned char flags);
	void set_len(size_type len);
	void set_alloc(size_type alloc_sz);

	void shrink_to_fit();
private:
	static constexpr size_type max_prealloc = 1024 * 1024;
	
	size_t _len = 0 ;
	size_t _alloc_sz = 0;
	unsigned char _flags = 0;
	value_type *_real = nullptr;
	allocator_type _alloc;
};

typedef base_mds<char> mds;
	
NAMESPACE_END(name)
#endif // __MDS_HPP
