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

#ifndef __CONFIG_MONOCO_
#define __CONFIG_MONOCO_

//#define DEBUG_M

#include <string>
#include <cctype>

#ifdef DEBUG_M
#include <iostream>
using namespace std;
#endif

#define NAMESPACE_BEGIN(name) namespace name {

#define NAMESPACE_END(name) }

NAMESPACE_BEGIN(configs)

static constexpr size_t BUFF_SIZE = 8196;

static std::size_t mzset_max_size = 64;
static std::size_t mzset_max_len = 2 << 8;
static std::size_t mset_max_size = 64;
static std::size_t mset_max_len = 2 << 8;
static std::size_t mht_max_size = 64;
static std::size_t mht_max_len = 2 << 8;
static std::size_t mlist_max_size = 64;
static std::size_t mlist_max_len = 2 << 9;
static std::size_t intvec_max_len = 2 << 8;

static const std::string MONOCO = "Monoco";
static long double VERSION = 0.01;
static size_t heartbeat_tick = 5;
static size_t expire_time = 60;
static int64_t cmd_aof_counts = 1;
static int64_t backup_mdf_seconds = 60;
static bool mdf_restore = false;
static bool aof_restore = true;
static std::string mdf_path = ".mdf";
static std::string aof_path = ".aof";
static std::string config_path = "monoco.conf";
static size_t init_db_num = 16;


NAMESPACE_END(configs)

NAMESPACE_BEGIN(monoco)

typedef std::string string;

NAMESPACE_END(monoco)

#endif // __CONFIG_MONOCO_
