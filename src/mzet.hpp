#ifndef __M_ZSET_HPP_
#define __M_ZSET_HPP_

#include <vector>
#include <algorithm>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>

#include "common.hpp"
#include "config.hpp"

NAMESPACE_BEGIN(monoco)

template <typename KEY, typename VAL>
class _mzset
{
public:
	typedef string         key_type;
	typedef int          value_type;
	
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
			tag == VEC_TAG;
		}

	void try_evolve()
		{
			if (_vec.size() > configs::mzset_max_len)
				_evolve();
		}
		
public:
	std::size_t size() const
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
				auto iter = std::lower_bound(_vec.begin(), _vec.end(), val,
											 [](const auto &left,
												const auto &right)
											 {
												 return left.second <
												 right.second;
											 });
				_vec.insert(iter, val);
			} else
				_map.insert(position(key, val));
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
				if (iter != _map.end())
					_map.erase(iter);
			}
		}

	value_type get(const key_type& key) const
		{
			if (tag == VEC_TAG) {
				auto iter = std::find_if(_vec.begin(), _vec.end(),
										 [&key](auto &ele)
										 {
											 return ele.first == key;
										 });
				return iter->second;
			}
			else {
				return _map.find(position(key, value_type())).get_right;
			}
		}

	std::size_t rank(const key_type& key) const
		{
			if (tag == VEC_TAG) {
				auto iter = std::lower_bound(_vec.begin(), _vec.end(), key,
											 [](const auto &left,
												const auto &right)
											 {
												 return left.first <
												 right.first;
											 });
				if (iter == _vec.end())
					 return std::numeric_limits<std::size_t>::max();
			}
			else {
				auto iter = _map.left.begin();
				for (size_t i = 0; iter != _map.size(); ++i, ++iter)
					if (*iter == key)
						return i;
				return std::numeric_limits<std::size_t>::max();
			}
		}

	template <class CON>
	void get(CON &con, std::size_t start, std::size_t finsh)
		{
			auto _push = [&] (auto &imp)
				{
					auto iter = imp.begin();
					std::advance(iter, start);
					while(start != finsh) {
						if (tag == VEC_TAG)
							con.push_back(*iter);
						else
							con.emplace(iter->get_left(), iter->get_right());
						++start;
						++iter;
					}
				};
			
			if (tag == VEC_TAG)
				_push(_vec);
			else
				_push(_map);
		}
};

NAMESPACE_END(monoco)
#endif 
