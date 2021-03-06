#ifndef __M_ZSET_HPP_
#define __M_ZSET_HPP_

#include <vector>
#include <algorithm>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>

#include "common.hpp"
#include "config.hpp"
#include "mbj.hpp"

NAMESPACE_BEGIN(monoco)

template <typename KEY, typename VAL>
class mzset
{
public:
	typedef KEY          key_type;
	typedef VAL          value_type;
	
	typedef boost::bimap
	<
		boost::bimaps::set_of<KEY>,
		boost::bimaps::multiset_of<value_type>
	> mbimap;
	
	typedef typename mbimap::value_type position;
	
	static constexpr int VEC_TAG = 1;
	static constexpr int MAP_TAG = 2;

private:
	int tag = VEC_TAG;
	
	std::vector<std::pair<key_type, value_type>> _vec;
	mbimap _map;

	void _evolve()
		{
			if (tag == MAP_TAG)
				return ;
			for (auto &&ele : _vec)
				_map.insert(position(ele.first, ele.second));
			tag = VEC_TAG;
		}

	void try_evolve()
		{
			if (_vec.size() > configs::mzset_max_len)
				_evolve();
		}
		
public:
	
	
	virtual std::size_t size() const
		{
			return tag == VEC_TAG ? _vec.size() : _map.size();
		}
	
	void clear()
		{
			_vec.clear();
			_map.clear();
			tag = VEC_TAG;
		}

	void insert(const key_type& key, const value_type& val)
		{
			try_evolve();
			if (tag == VEC_TAG) {
				auto iter = std::lower_bound(_vec.begin(), _vec.end(),
											 std::make_pair(key, val),
											 [](const auto &left,
												const auto &right)
											 {
												 return left.second <
												 right.second;
											 });
				_vec.emplace(iter, key, val);
			} else
				_map.insert(position(key, val));
		}

	void insert(const std::pair<key_type, value_type>& pr)
		{
			insert(pr.first, pr.second);
		}
	
	void remove(const key_type& key)
		{
			if (tag == VEC_TAG) {
				auto iter = std::find_if(_vec.begin(), _vec.end(),
										 [&key](auto &ele)
										 {
											 return ele.first == key;
										 });
				if (iter != _vec.end())
					_vec.erase(iter);
			}
			else {
				auto iter = _map.left.find(key);
				if (iter != _map.left.end())
					_map.left.erase(iter);
			}
		}

	value_type get(const key_type& key) const
		{
			auto max_val = std::numeric_limits<value_type>::max();
			
			if (tag == VEC_TAG) {
				auto iter = std::find_if(_vec.begin(), _vec.end(),
										 [&key](auto &ele)
										 {
											 return ele.first == key;
										 });
				if (iter == _vec.end())
					return max_val;
				else
					return iter->second;
			}
			else {
				auto iter = _map.find(position(key, value_type()));
				if (iter == _map.end())
					return max_val;
				else
					return iter->get_right();
			}
		}

	std::size_t rank(const key_type& key) const
		{
			if (tag == VEC_TAG) {
				auto iter = std::find_if(_vec.begin(), _vec.end(),
										 [&key](auto &ele)
										 {
											 return ele.first == key;
										 });

				if (iter != _vec.end())
					return std::distance(_vec.begin(), iter);
			}
			else {
				auto iter = _map.begin();
				for (size_t i = 0; i != _map.size(); ++i, ++iter)
					if (iter->get_left() == key)
						return i;
			}
			
			return std::numeric_limits<std::size_t>::max();
		}

	std::size_t count(const value_type& start,
					  const value_type& finsh) const
		{
			assert(start <= finsh);
			if (tag == VEC_TAG) {
				auto iter1 = std::lower_bound(_vec.begin(), _vec.end(),
											  std::make_pair
											  (key_type(), start),
											  [](auto &left, auto &right)
											  {
												  return left.second <
												  right.second;
											  });
				auto iter2 = std::upper_bound(_vec.begin(), _vec.end(),
											  std::make_pair
											  (key_type(), finsh),
											  [](auto &left, auto &right)
											  {
												  return left.second <
												  right.second;
											  });
				if (iter1 == iter2)
					return 0;
				else
					return std::distance(iter1, iter2);
			}
			else {
				auto iter1 = _map.right.lower_bound(start);
				auto iter2 = _map.right.upper_bound(finsh);
				if (iter1 == iter2)
					return 0;
				else
					return std::distance(iter1, iter2);
			}
		}

	template <class CON>
	void get(CON &con, std::size_t start, std::size_t finsh)
		{
			if (tag == VEC_TAG) {
					auto iter = _vec.begin();
					std::advance(iter, start);
					while(start != finsh && iter != _vec.end()) {
						con.push_back(*iter);
						++start;
						++iter;
					}
			}
			else {
				auto iter = _map.begin();
				std::advance(iter, start);
					while(start != finsh && iter != _map.end()) {
						con.emplace_back(iter->get_left(), iter->get_right());
						++start;
						++iter;
					}
			}
		}
};

class zset : public mzset<string, long double>, public mbj
{
public:
	virtual size_t size() const {return mzset::size();}
	virtual string type_name() const { return "zset";}
	virtual ~zset(){};
};

NAMESPACE_BEGIN(types)

static const std::size_t M_ZSET = hash_type<zset>();

template <>
string type_name<zset>() {return "zset";}

NAMESPACE_END(types)

NAMESPACE_END(monoco)
#endif 
