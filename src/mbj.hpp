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

#include <memory>
#include <ctime>
#include <limits>
#include "config.hpp"


NAMESPACE_BEGIN(monoco)

class mbj
{
public:
	static constexpr uint8_t LRU_BITS = sizeof(std::time_t) * 8;
	static constexpr size_t  MAX_CLOCK =  (1 << 24) - 1;
	static constexpr size_t CLOCK_RESLUTION = 1000;
	typedef std::size_t           size_type;

	virtual ~mbj() {};
	virtual string type_name() const {return "mbj";}
    virtual size_t size() const
    {return std::numeric_limits<size_t>::max();}

	void update_lru() const
		{
			_lru = std::time(nullptr);
		}
private:
	mutable std::time_t _lru = std::time(nullptr);
};

NAMESPACE_END(monoco)
#endif  // __MBJ_HPP_
