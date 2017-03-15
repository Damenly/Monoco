#ifndef __M_DB_HPP_
#define __M_DB_HPP_

#include <memory>
#include <vector>
#include <algorithm>
#include <fstream>
#include <mutex>

#include <boost/crc.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "mdict.hpp"
#include "mdict.cpp"
#include "mbj.hpp"
#include "mstr.hpp"
#include "mset.hpp"
#include "mzset.hpp"
#include "mht.hpp"
#include "mlist.hpp"
#include "zl_entry.hpp"
#include "common.hpp"
#include "config.hpp"
#include "file.hpp"

NAMESPACE_BEGIN(monoco)

class mdb
{
public:
	typedef mdict<mstr, std::shared_ptr<mbj>> dict_type;
	typedef mdict<mstr, std::time_t>     expr_type;
	
	typedef expr_type::value_type                  ttl_type;
	typedef dict_type::key_type                    key_type;
	typedef dict_type::value_type                  value_type;
	
	typedef boost::unique_lock<boost::shared_mutex> write_lock;
	typedef boost::shared_lock<boost::shared_mutex> read_lock;
private:
	
	std::shared_ptr<dict_type> _dict = nullptr;
	std::shared_ptr<expr_type> _exprs = nullptr;
	boost::shared_mutex _lock;
	
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

	size_t key_nums() const {return _dict == nullptr ? 0 : _dict->size();}
	size_t empty() const {return _dict == nullptr;}

	std::ofstream&
	write_to(std::ofstream& os)
		{
			write_lock wlock(_lock);
			size_t holder;
			boost::crc_32_type crc32;
			
			holder = _dict->size();
			fs::write_to(os, holder, crc32).flush();
					
			for (auto && p : *_dict) {
				/* write key */
				p.first.write_to(os, crc32);

				/* write value */
				if (std::dynamic_pointer_cast<zl_entry>(p.second) != nullptr)
				{
					holder = types::hash_idx<zl_entry>();
					fs::write_to(os, holder, crc32);
				}
				p.second->write_to(os, crc32);
			}
			
			uint32_t checksum = crc32.checksum();
			fs::write_to(os, checksum, crc32).flush();
			return os;
		}

	std::ofstream&
	write_aof(std::ofstream& os)
		{
			using std::endl;
			errs::error("start");
			read_lock rlock(_lock);
			string cmd;
			const char* key;

			auto _write_ze = [](std::ofstream& os,
								zl_entry ze)
			{
				if (ze.encode == types::M_STR)
					os << '"' << ze << '"';
				else
					os << ze;
			};
			
			for (auto && p : *_dict) {
				key = p.first.c_str();

				if (typeid(*p.second) == typeid(zl_entry)) {
					os << "set " << key << " ";
					auto ptr = std::static_pointer_cast<zl_entry>(p.second);
					_write_ze(os, *ptr);
					os << endl;
				}
				else if (typeid(*p.second) == typeid(mlist)) {
					os <<"rpush " << p.first << " ";
					auto ptr = std::static_pointer_cast<mlist>(p.second);
					std::vector<zl_entry> vz;
					lrange(vz, p.first, 0, -1);
					for (auto && ze : vz) {
						_write_ze(os, ze);
						os << " ";
					}
					os << endl;
				}
				else if (typeid(*p.second) == typeid(mht)) {
					auto ptr = std::static_pointer_cast<mht>(p.second);
					std::vector<std::pair<string, zl_entry>> vec;

					hgetall(vec, p.first);
					for (auto && ele : vec) {
						os << "hset " << p.first << " ";
						os << ele.first  << " ";
						_write_ze(os, ele.second);
						os << endl;
					}
				}
				else if (typeid(*p.second) == typeid(mset)) {
					os <<"sadd " << p.first << " ";
					auto ptr = std::static_pointer_cast<mlist>(p.second);
					
					std::vector<zl_entry> vz;
					smembers(vz, p.first);
					for (auto && ze : vz) {
						_write_ze(os, ze);
						os << " ";
					}
					os << endl;
				}
				else if (typeid(*p.second) == typeid(zset)) {
					auto ptr = std::static_pointer_cast<mht>(p.second);
					std::vector<std::pair<string, long double>> vec;
					zrange(vec, p.first,
						   std::numeric_limits<long double>::min(),
						   std::numeric_limits<long double>::max());
			
					for (auto && ele : vec) {
						os << "zadd " << p.first << " "
						   << " " << ele.second <<" "
						   << ele.first << endl;
					}
				}
				else
					throw std::runtime_error("unknow type while backup aof");
			}
						
			return os;
		}

	std::ifstream&
	read_from(std::ifstream& is)
		{
			read_lock rlock(_lock);
			size_t holder, sz;
			boost::crc_32_type crc32;
			
			fs::read_from(is, sz, crc32);
			for (size_t i = 0; i != sz; ++i) {
				mstr key;
				key.read_from(is, crc32);

				fs::read_from(is, holder, crc32);
				std::shared_ptr<mbj> ptr = nullptr;

				if (holder == types::hash_idx<zl_entry>())
					ptr = std::make_shared<zl_entry>();
				else if (holder == types::hash_idx<mset>())
					ptr = std::make_shared<mset>();
				else if (holder == types::hash_idx<zset>())
					ptr = std::make_shared<zset>();
				else if (holder == types::hash_idx<mht>())
					ptr = std::make_shared<mht>();
				else if (holder == types::hash_idx<mlist>())
					ptr = std::make_shared<mlist>();
				else 
					ptr = std::make_shared<zl_entry>();
				
				ptr->read_from(is, crc32);
				_dict->insert(key, ptr);
			}
			
			uint32_t sum;
			uint32_t oldsum = crc32.checksum();
			fs::read_from(is, sum, crc32);
			if (oldsum != sum) {
				throw std::runtime_error("db checksum error");
			}

			return is;
		}
	
	int remove(const key_type& key)
		{
			write_lock wlock(_lock);
			_dict->erase(key);

			auto iter = _dict->find(key);
			if (iter != _dict->end()) {
				_dict->erase(key);
				return 1;
			}
			else {
				return -1;
			}
		}

	void clear()
		{
			write_lock wlock(_lock);
			_dict->clear();
			_exprs->clear();
		}
   
	
	string type(const key_type& key)
		{
			read_lock rlock(_lock);
			auto iter = _dict->find(key);
			if (iter != _dict->end()) {
				return iter->second->type_name();
			}
			else {
				return "Unknow type";
			}
		}

	bool exists(const key_type& key)
		{
			read_lock rlock(_lock);
			auto iter = _dict->find(key);
			return iter != _dict->end();
		}

	int set_expr(const key_type& key, ttl_type ttl)
		{
			write_lock wlock(_lock);
			if (!exists(key))
				return 1;
			_exprs->insert_assign(key, ttl);
			return 0;
		}

	void remove_if_expred(const key_type& key)
		{
			write_lock wlock(_lock);
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
				return nullptr;
			}
			
			auto ptr = std::dynamic_pointer_cast<T>
				(iter->second);
			return ptr;
		}
public:

	/******************************************
     set
	*****************************************/
	int
	sadd(const key_type& key, const std::vector<zl_entry>& args)
		{
			write_lock wlock(_lock);
			auto iter = _dict->find(key);
			if (iter == _dict->end()) {
				_dict->insert(key,
							  std::make_shared<mset>());
			}

			auto ptr = find_cast<mset>(key);
			if (ptr == nullptr) {
				return -1;
			}

			for (auto && arg : args)
				ptr->insert(arg);
			return args.size();
		}

	size_t
	scard(const key_type& key)
		{
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
			auto ptr = find_cast<mset>(key);
			if (ptr == nullptr) {
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
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
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
			write_lock wlock(_lock);
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
			read_lock rlock(_lock);
			auto ptr = find_cast<zl_entry>(key);
			if (ptr == nullptr && ptr->encode == types::M_STR) {
				return types::size_t_max;
			}
			
			return ptr->safe_data<string>().size();
		}

	int
	set(const key_type&key, const char* val)
		{
			write_lock wlock(_lock);
			return set(key, string(val));
		}
		
	template <typename T>
	int
	set(const key_type&key, const T& val)
		{
			write_lock wlock(_lock);
			auto iter = _dict->find(key);
	
			if (iter == _dict->end()) {
				_dict->insert(key,
							  std::make_shared<zl_entry>(val));
				return 1;
			}

			auto ptr = std::static_pointer_cast<zl_entry>(iter->second);
			if (ptr == nullptr) {
				return -1;
			}

			*ptr = zl_entry(val);
			return 1;
		}

	int
	append(const key_type&key, const string& val)
		{
			write_lock wlock(_lock);
			auto iter = _dict->find(key);
	
			if (iter == _dict->end()) {
				_dict->insert(key,
							  std::make_shared<zl_entry>(val));
				return 1;
			}

			auto ptr = std::static_pointer_cast<zl_entry>(iter->second);
			if (ptr == nullptr || ptr->encode != types::M_STR) {
				return -1;
			}
			
			string *str = ptr->unsafe_data<string>();
			str->append(val);
			return 1;
		}

	zl_entry
	get(const key_type& key)
		{
			read_lock rlock(_lock);
			auto ptr = find_cast<zl_entry>(key);
			if (ptr == nullptr) {
				return zl_entry();
			}
			
			return *ptr;
		}

	int
	incr_by(const key_type& key, long double diff)
		{
			write_lock wlock(_lock);
			auto ptr = find_cast<zl_entry>(key);
			if (ptr == nullptr) {
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
			write_lock wlock(_lock);
			
			auto iter = _dict->find(key);
			if (iter == _dict->end()) {
				_dict->insert(key,
							  std::make_shared<zset>());
			}

			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
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
			read_lock rlock(_lock);
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				return types::size_t_max;
			}
			return ptr->size();
		}

	zvalue_type
	zscore(const key_type& key, const zkey_type& pos)
		{
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
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
			write_lock wlock(_lock);
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
			read_lock rlock(_lock);
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				return types::size_t_max;
			}

			return ptr->rank(pos);
		}

	int	
	zrem(const key_type& key, const zkey_type& pos)
		{
			write_lock wlock(_lock);
			auto ptr = find_cast<zset>(key);
			if (ptr == nullptr) {
				return -1;
			}
			ptr->remove(pos);
			return 1;
		}

	template <typename CON>
	int zrange(CON &con, const key_type& key,
				size_t start, size_t finsh)
		{
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
			auto ptr = find_cast<mht>(key);
			if (ptr == nullptr) {
				return types::size_t_max;
			}
			return ptr->size();
		}
	
	int
	hdel(const key_type& key, const mkey_type& pos)
		{
			write_lock wlock(_lock);
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
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
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
			read_lock rlock(_lock);
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
			write_lock wlock(_lock);
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
			read_lock rlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				return zl_entry();
			}
			if (pos >= ptr->size())
				return zl_entry();
			return ptr->operator[](pos);
		}

	int linsert(const key_type& key, bool is_before,
				const zl_entry& t, const zl_entry& a)
		{
			write_lock wlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				return -1;
			}

			auto pos = ptr->find(t);
			if (pos == types::size_t_max) {
				return -1;
			}

			if (!is_before)
				++pos;
			ptr->insert(pos, a);
			return 1;
		}

	size_t
	llen(const key_type& key)
		{
			read_lock rlock(_lock);
			auto iter = _dict->find(key);
			if (iter == _dict->end()) {
				return types::size_t_max;
			}

			return iter->second->size();
		}

	zl_entry
	lpop(const key_type& key)
		{
			write_lock wlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {

				return zl_entry();
			}

			if (ptr->size() == 0) {
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
			write_lock wlock(_lock);
			if (_dict->find(key) == _dict->end())
				_dict->insert(key, std::make_shared<mlist>());
			
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				return -1;
			}

			for (auto && ele : vals)
				ptr->push_front(ele);
			return vals.size();
		}

	zl_entry
	rpop(const key_type& key)
		{
			write_lock wlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {

				return zl_entry();
			}

			if (ptr->size() == 0) {
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
			write_lock wlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
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
			write_lock wlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {

				return -1;
			}

			auto ptr2 = find_cast<mlist>(key2);
			if (ptr2 == nullptr) {

				return -1;
			}

			for (size_t i = ptr->size() - 1; i != -1; --i)
				ptr->push_front(ptr2->operator[](i));
			
			return ptr2->size();
		}

	int
	rpushx(const key_type& key, const key_type& key2)
		{
			write_lock wlock(_lock);
			if (key == key2)
				return 0;
			
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
				return -1;
			}

			auto ptr2 = find_cast<mlist>(key2);
			if (ptr2 == nullptr) {
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
			read_lock rlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
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
			write_lock wlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
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
			write_lock wlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
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
			write_lock wlock(_lock);
			auto ptr = find_cast<mlist>(key);
			if (ptr == nullptr) {
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
