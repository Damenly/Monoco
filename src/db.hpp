#ifndef __M_DB_HPP_
#define __M_DB_HPP_

#include <memory>
#include <vector>
#include <algorithm>

#include "mdict.hpp"
#include "mbj.hpp"
#include "mstr.hpp"
#include "mset.hpp"
#include "mzset.hpp"
#include "mht.hpp"
#include "mlist.hpp"
#include "zl_entry.hpp"
#include "common.hpp"
#include "config.hpp"

NAMESPACE_BEGIN(monoco)

class mdb
{
public:
	typedef mdict<mstr, std::shared_ptr<mbj>> dict_type;
	typedef mdict<mstr, std::time_t>     expr_type;
	
	typedef expr_type::value_type                  ttl_type;
	typedef dict_type::key_type                    key_type;
	typedef dict_type::value_type                  value_type;
//private:
	
	std::shared_ptr<dict_type> _dict = nullptr;
	std::shared_ptr<expr_type> _exprs = nullptr;

	static void _update_lru(mbj& p) 
		{
			p.update_lru();
		}
public:
	mdb()
		{
			_dict = std::make_shared<dict_type>();
			_exprs = std::make_shared<expr_type>();
		}

	size_t empty() const {return _dict == nullptr;}

	void remove(const key_type& key)
		{
			_dict->erase(key);
		}
	
	string type(const key_type& key) const
		{
			auto iter = _dict->find(key);
			if (iter != _dict->end()) {
				//iter->first->uopda
				return iter->second->type_name();
			}
			else {
				return "Unknow type";
			}
		}

	bool exists(const key_type& key) const
		{
			auto iter = _dict->find(key);
			return iter != _dict->end();
		}

	int set_expr(const key_type& key, ttl_type ttl)
		{
			if (!exists(key))
				return 1;
			_exprs->insert_assign(key, ttl);
			return 0;
		}

	void remove_if_expred(const key_type& key)
		{
			ttl_type now = std::time(nullptr);
			auto iter = _exprs->find(key);
			if (iter == _exprs->end() ||
				iter->second >= now)
				return ;
			_dict->erase(key);
			_exprs->erase(key);
		}

	template <typename T>
	std::shared_ptr<T>
	find_cast(const key_type& key)
		{
			auto iter = _dict->find(key);
			if (iter == _dict->end()) {
				errs::error(key, "not found");
				return nullptr;
			}
			
			auto ptr = std::static_pointer_cast<T>
				(iter->second);
			return ptr;
		}
public:

	/******************************************
     set
	*****************************************/
	int
	sadd(const key_type& key, std::vector<zl_entry>& args)
		{
			auto iter = _dict->find(key);
			if (iter == _dict->end()) {
				_dict->insert(key,
							  std::make_shared<mset>());
			}

			auto ptr = find_cast<mset>(key);
			if (ptr == nullptr) {
				errs::error("error type");
				return -1;
			}

			for (auto && arg : args)
				ptr->insert(arg);
			return args.size();
		}

	size_t
	scard(const key_type& key)
		{
			auto ptr = find_cast<mset>(key);
			if (ptr == nullptr) {
				return types::size_t_max;
			}
			return ptr->size();
		}

	template <typename CON>
	int
	smembers(CON& con, const key_type& key)
		{
			auto ptr = find_cast<mset>(key);
			if (ptr == nullptr) {
				errs::error("wrong type");
				return -1;
			}
			ptr->getall(con);
			return 0;
		}

	template <typename CON>
	int
	sdiff(CON& con, const key_type& key,
			 const key_type& key2)
		{
			std::vector<zl_entry> v1;
			std::vector<zl_entry> v2;
			if (smembers(v1, key) != 0)
				return -1;
			if (smembers(v2, key2) != 0)
				return -1;
			std::set_difference(v1.begin(), v1.end(),
								v2.begin(), v2.end(),
								std::back_inserter(con));

			return 0;
		}

	template <typename CON>
	int
	sinter(CON& con, const key_type& key,
		   const key_type& key2)
		{
			std::vector<zl_entry> v1;
			std::vector<zl_entry> v2;
			if (smembers(v1, key) != 0)
				return -1;
			if (smembers(v2, key2) != 0)
				return -1;
			std::set_intersection(v1.begin(), v1.end(),
								  v2.begin(), v2.end(),
								  std::back_inserter(con));
			return 0;
		}

	template <typename CON>
	int
	sunion(CON& con, const key_type& key,
		   const key_type& key2)
		{
			std::vector<zl_entry> v1;
			std::vector<zl_entry> v2;
			if (smembers(v1, key) != 0)
				return -1;
			if (smembers(v2, key2) != 0)
				return -1;
			std::set_union(v1.begin(), v1.end(),
						   v2.begin(), v2.end(),
						   std::back_inserter(con));
			return 0;
		}

	template <typename T>
	bool
	sismember(const key_type& key, const T& val)
		{
			auto ptr = find_cast<mset>(key);
			if (ptr == nullptr) {
				return false;
			}

			return ptr->exists(val);
		}

	template <typename T>
	int
	srem(const key_type& key, const T& val)
		{
			auto ptr = find_cast<mset>(key);
			if (ptr == nullptr) {
				return -1;
			}

			ptr->remove(val);
			return 1;
		}
	
	/******************************************
     set
	*****************************************/
	
	/******************************************
     string inteager float
	*****************************************/
	size_t
	strlen(const key_type& key)
		{
			auto ptr = find_cast<zl_entry>(key);
			if (ptr == nullptr && ptr->encode == types::M_STR) {
				errs::error("only string can do strlen");
				return types::size_t_max;
			}
			
			string &str = *boost::unsafe_any_cast<string>(&(ptr->_content));
			return str.size();
		}

	int
	set(const key_type&key, const char* val)
		{
			return set(key, string(val));
		}
		
	template <typename T>
	int
	set(const key_type&key, const T& val)
		{
			auto iter = _dict->find(key);
	
			if (iter == _dict->end()) {
				_dict->insert(key,
							  std::make_shared<zl_entry>(val));
				return 1;
			}

			auto ptr = std::static_pointer_cast<zl_entry>(iter->second);
			if (ptr == nullptr) {
				errs::error("wrong type");
				return -1;
			}

			*ptr = zl_entry(val);
			return 1;
		}

	int
	append(const key_type&key, const string& val)
		{
			auto iter = _dict->find(key);
	
			if (iter == _dict->end()) {
				_dict->insert(key,
							  std::make_shared<zl_entry>(val));
				return 1;
			}

			auto ptr = std::static_pointer_cast<zl_entry>(iter->second);
			if (ptr == nullptr || ptr->encode != types::M_STR) {
				errs::error("wrong type");
				return -1;
			}
			
			string &str = *boost::unsafe_any_cast<string>(&(ptr->_content));
			str.append(val);
			return 1;
		}

	zl_entry
	get(const key_type& key)
		{
			auto ptr = find_cast<zl_entry>(key);
			if (ptr == nullptr) {
				errs::error("Do get on single object");
				return zl_entry();
			}

			return *ptr;
		}

	int
	incr_by(const key_type& key, long double diff)
		{
			auto ptr = find_cast<zl_entry>(key);
			if (ptr == nullptr) {
				errs::error("Do get on single object");
				return -1;
			}

			/* FIX ME to improve memory effency */
			if (types::is_int(ptr->encode)) {
				int64_t tmp = ptr->to_s64();
				tmp += diff;
				*ptr = tmp;
				return 1;
			}

			if (types::is_uint(ptr->encode)) {
				uint64_t tmp = ptr->to_u64();
				tmp += diff;
				*ptr = tmp;
				return 1;
			}

			if (types::is_float(ptr->encode)) {
				auto tmp = ptr->to_ld();
				tmp += diff;
				*ptr = tmp;
				return 1;
			}
			
			errs::error(key, "type error");
			return -1;
		}

	int
	decr_by(const key_type& key, long double diff)
		{
			return incr_by(key, -diff);
		}
	
	int
	decr(const key_type& key)
		{
			return incr_by(key, -1);
		}
	
	int
	incr(const key_type& key)
		{
			return incr_by(key, 1);
		}
	
	/*****************************************/
	
   
	/******************************************
     zset operations 
	*****************************************/

	typedef zset::key_type           zkey_type;
	typedef zset::value_type         zvalue_type;
	typedef std::pair<zkey_type, zvalue_type>          zpir_type;
	int
	zadd(const key_type& key, const std::vector<zpir_type>& ps)
		{
			auto iter = _dict->find(key);
			if (iter == _dict->end()) {
				_dict->insert(key,
							  std::make_shared<zset>());
			}

			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				errs::error("error type");
				return -1;
			}

			for (auto && arg : ps) {
				ptr->insert(arg);
			}

			return ps.size();
		}

	size_t
	zcard(const key_type& key)
		{
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				return types::size_t_max;
			}
			return ptr->size();
		}

	zvalue_type
	zscore(const key_type& key, const zkey_type& pos)
		{
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				return types::size_t_max;
			}
			
			return ptr->get(pos);
		}

	size_t
	zcount(const key_type& key, long double start,
		  long double finsh)
		{
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				return types::size_t_max;
			}
			
			return ptr->count(start, finsh);
		}

	int
	zincr_by(const key_type& key, const zkey_type& pos,
			 long double diff)
		{
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				return -1;
			}

			auto val = ptr->get(pos);
			val += diff;
			ptr->remove(pos);
			ptr->insert(pos, val);
			return 1;
		}

	size_t
	zrank(const key_type& key, const zkey_type& pos)
		{
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				return types::size_t_max;
			}

			return ptr->rank(pos);
		}

	int	
	zrem(const key_type& key, const zkey_type& pos)
		{
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				errs::error(key, "not found");
				return -1;
			}
			ptr->remove(pos);
			return 1;
		}

	template <typename CON>
	int zrange(CON &con, const key_type& key,
				size_t start, size_t finsh)
		{
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				return -1;
			}

			ptr->get(con, start, finsh);
			return 0;
		}
	
	/*                                      *
	 *zset operations                       *
	 ****************************************/

	/******************************************
     HT operations 
	*****************************************/
	typedef mht::key_type          mkey_type;
	size_t
	hcard(const key_type& key)
		{
			auto ptr = find_cast<mht>(key);
			if (ptr == nullptr) {
				return types::size_t_max;
			}
			return ptr->size();
		}
	
	int
	hdel(const key_type& key, const mkey_type& pos)
		{
			auto ptr = find_cast<mht>(key);
			if (ptr == nullptr) {
				return -1;
			}
			ptr->remove(pos);
			return 1;
		}

	zl_entry
	hget(const key_type& key, const mkey_type& pos)
		{
			auto ptr = find_cast<mht>(key);
			if (ptr == nullptr) {
				return zl_entry();
			}
			return ptr->get(pos);
		}

	template <typename CON>
	int
	hgetall(CON& con, const key_type& key)
		{
			auto ptr = find_cast<mht>(key);
			if (ptr == nullptr) {
				return -1;
			}
			ptr->getall(con);
			return 0;
		}

	template <typename CON>
	int
	hvals(CON& con, const key_type& key)
		{
			auto ptr = find_cast<mht>(key);
			if (ptr == nullptr) {
				return -1;
			}

			std::vector<std::pair<mkey_type, zl_entry>>
				tmp;
			
			ptr->getall(tmp);

			for (auto && ele : tmp)
				con.push_back(ele.second);
			return 0;
		}

	template <typename CON>
	int
	hkeys(CON& con, const key_type& key)
		{
			auto ptr = find_cast<mht>(key);
			if (ptr == nullptr) {
				return -1;
			}

			std::vector<std::pair<mkey_type, zl_entry>>
				tmp;
			
			ptr->getall(tmp);

			for (auto && ele : tmp)
				con.push_back(ele.first);

			return 0;
		}

	bool
	hexists(const key_type& key, const mkey_type& pos)
		{
			auto ptr = find_cast<mht>(key);
			if (ptr == nullptr) {
				return false;
			}
			return ptr->exists(pos);
		}

	int
	hset(const key_type& key, const mkey_type&pos,
		 const zl_entry& val )
		{
			auto iter = _dict->find(key);
			if (iter == _dict->end())
				_dict->insert(key,
							  std::make_shared<mht>());
			auto ptr = find_cast<mht>(key);
			if (ptr == nullptr) {
				return -1;
			}

			ptr->update(pos, val);
			return 1;
		}

	
	/******************************************
     list operations 
	*****************************************/
	zl_entry
	lindex(const key_type& key, size_t pos)
		{
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error("only list can do lindex");
				return zl_entry();
			}

			if (pos >= ptr->size())
				return zl_entry();
			return ptr->operator[](pos);
		}

	int linsert(const key_type& key, bool is_before,
				const zl_entry& t, const zl_entry& a)
		{
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error("only list can do linsert");
				return -1;
			}

			auto pos = ptr->find(t);
			if (pos == types::size_t_max) {
				errs::error(t, "not found");
				return -1;
			}

			if (!is_before)
				++pos;
			ptr->insert(pos, a);
			return 1;
		}

	size_t
	llen(const key_type& key) const
		{
			auto iter = _dict->find(key);
			if (iter == _dict->end()) {
				errs::error(key, "not found");
				return types::size_t_max;
			}

			return iter->second->size();
		}

	zl_entry
	lpop(const key_type& key)
		{
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error("only list can do lpop");
				return zl_entry();
			}

			if (ptr->size() == 0) {
				errs::error(key, " is empty");
				return -1;
			}
			
			zl_entry res;
			try {
				res = ptr->operator[](0);
				ptr->pop_front();
			}
			catch (...) {}

			return res;
		}

	int
	lpush(const key_type& key, std::vector<zl_entry> vals)
		{
			if (_dict->find(key) == _dict->end())
				_dict->insert(key, std::make_shared<mlist>());
			
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error("only list can do lpush");
				return -1;
			}

			for (auto && ele : vals)
				ptr->push_front(ele);
			return vals.size();
		}

	zl_entry
	rpop(const key_type& key)
		{
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error("only list can do rpop");
				return zl_entry();
			}

			if (ptr->size() == 0) {
				errs::error(key, " is empty");
				return -1;
			}
			zl_entry res;
			try {
				res = ptr->operator[](ptr->size() - 1);
				ptr->pop_back();
			}
			catch (...) {}

			return res;
		}

	int
	rpush(const key_type& key, std::vector<zl_entry> vals)
		{
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error("only list can do rpush");
				return -1;
			}

			for (auto && ele : vals)
				ptr->push_back(ele);

			return vals.size();
		}

	int
	lpushx(const key_type& key, const key_type& key2)
		{
			//		if (key == key2)
			//	return 0;
			
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error("only list can do rpush");
				return -1;
			}

			auto ptr2 = find_cast<mlist>(key2);
			if (ptr2 == nullptr) {
				errs::error(key2, "is not a list");
				return -1;
			}

			for (size_t i = ptr->size() - 1; i != -1; --i)
				ptr->push_front(ptr2->operator[](i));
			
			return ptr2->size();
		}

	int
	rpushx(const key_type& key, const key_type& key2)
		{
			if (key == key2)
				return 0;
			
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error("only list can do rpush");
				return -1;
			}

			auto ptr2 = find_cast<mlist>(key2);
			if (ptr2 == nullptr) {
				errs::error(key2, "is not a list");
				return -1;
			}
			
			for (size_t i = 0; i != ptr2->size(); ++i)
				ptr->push_back(ptr2->operator[](i));
			
			return ptr2->size();
		}

	template <typename CON>
	int
	lrange(CON& con, const key_type& key, size_t l, size_t r)
		{
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error(key, "is not a list");
				return -1;
			}
			if (r != types::size_t_max)
				++r;
			r = std::min(r, ptr->size());

			while(l != r)
				con.push_back(ptr->operator[](l++));
			
			return 0;
		}

	template <typename T>
	int
	lrem(const key_type& key, size_t count, const T& val)
		{
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error(key, "is not a list");
				return -1;
			}

			for (size_t i = 0; i != ptr->size() && count > 0;
				++i)
			{
				if (ptr->operator[](i) == val) {
					ptr->erase(i);
					--i;
					--count;
				}
			}

			return count;
		}

	template <typename T>
	int
	lset(const key_type& key, size_t pos, const T& val)
		{
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error(key, "is not a list");
				return -1;
			}

			if (pos >= ptr->size())
				return -1;
			
			ptr->operator[](pos) = zl_entry(val);
			return 1;
		}

	int
	ltrim(const key_type& key, size_t pos, size_t finsh)
		{
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				errs::error(key, "is not a list");
				return -1;
			}

			if (finsh != types::size_t_max)
				++finsh;
			if (pos >= finsh ||
				pos > ptr->size() ||
				finsh > ptr->size())
				return -1;
			ptr->erase(pos, finsh);
			return 1;
		}
	
	/******************************************
     list operations 
	*****************************************/
};

NAMESPACE_END(monoco)

#endif // __M_DB_HPP_
