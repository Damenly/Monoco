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

#ifndef __MDS_CPP_
#define __MDS_CPP_

#include "mds.hpp"

NAMESPACE_BEGIN(monoco)

template<class T, class Alloc>
base_mds<T, Alloc>::base_mds()
{
	base_mds("");
}

template<class T, class Alloc>
base_mds<T, Alloc>::base_mds(const char *cstr)
{
	_len = static_cast<value_type>(strlen(cstr));
	_alloc_sz = _len + 1;
	_real = _alloc.allocate(_alloc_sz);
	memcpy(_real, cstr, _len);
	_real[_len + 1] = 0;
}

template<class T, class Alloc>
base_mds<T, Alloc>::base_mds(size_type sz)
{
	_len = sz;
	_alloc_sz = _len + 1;
	_real = _alloc.allocate(_alloc_sz);
	memset(_real, 0, _alloc_sz);
}

template<class T, class Alloc>
base_mds<T, Alloc>::base_mds(const base_mds& oth)
{
	_len = oth._len;
	_alloc_sz = oth._alloc_sz;
	_flags = oth.flags;
	memcpy(_real, oth._real, _alloc_sz);
}

template <class T, class Alloc>
base_mds<T, Alloc>&
base_mds<T, Alloc>::operator=(const base_mds& rstr)
{
	_alloc.deallocate(_real, _alloc_sz);
	_alloc.allocate(rstr._alloc_sz);
	_alloc_sz = rstr._alloc_sz;
	memcpy(_real, rstr._real, _alloc_sz);
	_len = rstr._len;
	_flags = rstr.flags;
	
	return *this;
}

template <class T, class Alloc>
base_mds<T, Alloc>&
base_mds<T, Alloc>::operator+(const base_mds& rstr)
{
	base_mds<T, Alloc> lstr(*this);
	
	return *this;
}

template <class T, class Alloc>
base_mds<T, Alloc>&
base_mds<T, Alloc>::operator+=(const base_mds& rstr)
{
	
	return *this;
}

template <class T, class Alloc>
void
base_mds<T, Alloc>::set_flags(unsigned char flags)
{
	_flags = flags;
}

template <class T, class Alloc>
void
base_mds<T, Alloc>::set_len(size_type len)
{
	_len = len;
}

template <class T, class Alloc>
base_mds<T, Alloc>::~base_mds()
{
	_len = 0;
	_alloc_sz = 0;
	_real = nullptr;
	_alloc.deallocate(_real, _alloc_sz);
}

template <class T, class Alloc>
base_mds<T, Alloc>::shrink_to_fit()
{
	_len = strlen(_real);
	auto _tmp = _alloc.allocate(_real, _len + 1);
	std::swap(_tmp, _real);
	_alloc.deallocate(_tmp, _alloc_sz);
	_alloc_sz = _len + 1;
}


NAMESPACE_END(monoco)

#endif // __MDS_CPP_
