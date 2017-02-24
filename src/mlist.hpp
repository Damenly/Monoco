#ifndef __MLIST_HPP_
#define __MLIST_HPP_

#include <cstring>
#include <vector>
#include <list>
#include <boost/any.hpp>
#include "config.hpp"
#include "common.hpp"
#include "any.hpp"
#include "utility.cpp"
#include "zl_entry.hpp"

NAMESPACE_BEGIN(monoco)

class mlist
{
public:
	static constexpr int VEC_TAG = 1;
	static constexpr int LS_TAG = 2;
	
	int tag = VEC_TAG;
	
private:
	void _evlove() {
		if (tag == LS_TAG)
			return;
		for (auto &&ele : _vec)
			_list.push_back(std::move(ele));
		_vec.clear();
		tag = LS_TAG;
	}

	template <typename T>
	void try_evlove(const T& val) {
		if (_vec.size() > configs::mlist_max_len)
			_evlove();
	}
	
	std::vector<zl_entry> _vec;
	std::list<zl_entry> _list;
	
public:
	
	template <typename T>
	void push_back(const T& val)
		{
			try_evlove(val);
			if (tag == LS_TAG)
				_list.emplace_back(val);
			else
				_vec.emplace_back(val);
		}

	template <typename T>
	void push_front(const T& val)
		{
			try_evlove(val);
			if (tag == LS_TAG)
				_list.emplace_front(val);
			else
				_vec.emplace(_vec.begin(), val);
		}

	void pop_front()
		{
			if (tag == LS_TAG)
				_list.pop_front();
			else
				_vec.erase(_vec.begin());
		}

	void pop_back()
		{
			if (tag == LS_TAG)
				_list.pop_back();
			else
				_vec.pop_back();
		}

	auto operator[] (std::size_t pos)
		{
			if (tag == LS_TAG) {
				auto iter = _list.begin();
				std::advance(iter, pos);
				return *iter;
			}
			else {
				return _vec[pos];
			}
		}

	auto operator[] (std::size_t pos) const
		{
			if (tag == LS_TAG) {
				auto iter = _list.begin();
				std::advance(iter, pos);
				return *iter;
			}
			else {
				return _vec[pos];
			}
		}

	std::size_t size() const {return std::max(_vec.size(), _list.size());}

	template <typename T>
	void insert(std::size_t pos, const T& val)
		{
			try_evlove(val);
			auto _insert = [pos, &val](auto &con)
				{
					auto iter = con.begin();
					std::advance(iter, pos);
					con.emplace(iter, val);
				};
			
			if (tag == LS_TAG) 
				_insert(_list);
			else
				_insert(_vec);
		}

	template <typename T>
	void update(std::size_t pos, const T& val)
		{
			try_evlove(val);
			auto _update = [pos, &val](auto &con)
				{
					auto iter = con.begin();
					std::advance(iter, pos);
					decltype(con.back()) tmp(val);
					*iter = std::move(tmp);
				};
			
			if (tag == VEC_TAG) 
				_update(_vec);
			else
				_update(_list);
		}
	
	
	void erase(std::size_t pos)
		{
			auto _erase = [pos](auto &ls)
				{
					auto iter = ls.begin();
					std::advance(iter, pos);
					ls.erase(iter);
				};
			
			if (tag == VEC_TAG)
				_erase(_vec);
			else
				_erase(_list);
		}

	void erase(std::size_t start, std::size_t finsh)
		{
			auto _erase = [start, finsh](auto &ls)
				{
					auto iter1 = ls.begin();
					auto iter2 = ls.begin();
					std::advance(iter1, start);
					std::advance(iter1, finsh);
					ls.erase(iter1, iter2);
				};
			if (tag == VEC_TAG)
				_erase(_vec);
			else
				_erase(_list);
		}
	
	/*
	template <typename T>
	void remove(const T& val) {
		auto _remove = [&val](auto con)
			{
				std::remove(con.begin(), con.end(), val);
			};
		
		if (tag == VEC_TAG)
			_remove(_vec);
		else
			_remove(_list);
	}
	*/
	
};

template <>
void mlist::try_evlove<std::string>(const std::string& val)
{
	if (_vec.size() > configs::mlist_max_len) {
		_evlove();
	}
	
	if (val.size() > configs::mlist_max_size)
		_evlove();
}

NAMESPACE_END(monoco)

#endif // __MLIST_HPP_
