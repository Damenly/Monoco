#ifndef __MDICT_HPP_
#define __MDICT_HPP_

#include <unordered_map>
#include <memory>
#include <limits>
#include <random>
#include <vector>
#include "config.hpp"

NAMESPACE_BEGIN(monoco)

#define MDICT_FINE 0  // fine
#define MDICT_ERR  1  // error 

template <class IT, class DICT>
struct mdict_iterator
{
	typedef typename IT::difference_type difference_type;
	typedef typename IT::value_type value_type;
	typedef typename IT::pointer pointer;
	typedef typename IT::reference reference;
	typedef typename IT::iterator_category iterator_category;
	
	mdict_iterator(IT real, DICT *dt):_real(real),_dt(dt) {}
	mdict_iterator() {}
	std::pair<const int,int>& operator*() {return *_real;}
	auto operator*() const {return *_real;}
	auto operator->() {return _real.operator->();}
	auto operator=(mdict_iterator& oth)
		{
			_real = oth._real;
			_dt = oth._dt;
		}
	bool operator==(const mdict_iterator & oth) const
		{return _real == oth._real && _dt == oth._dt;}
	bool operator!=(const mdict_iterator & oth) const
		{return !operator==(oth);}
	
	mdict_iterator& operator++()
		{
			++_real;
			if (_real == std::end(_dt->_ht[0]))
				_real = std::begin(_dt->_ht[1]);
			return *this;
		}
	
	mdict_iterator operator++(int)
		{
			auto res = *this;
			this->operator++();
			return res;
		}

	
	IT _real;
	DICT* _dt;
};

template <class IT, class DICT>
struct mdict_const_iterator : public std::forward_iterator_tag
{
	typedef typename IT::difference_type difference_type;
	typedef typename IT::value_type value_type;
	typedef typename IT::pointer pointer;
	typedef typename IT::reference reference;
	typedef typename IT::iterator_category iterator_category;
	
	mdict_const_iterator(IT real, const DICT *dt):_real(real),_dt(dt) {}
	mdict_const_iterator() {}
	const std::pair<const int,int>& operator*() {return *_real;}
	auto operator*() const {return *_real;}
	auto operator->() {return _real.operator->();}
	bool operator==(const mdict_const_iterator & oth) const
		{return _real == oth._real && _dt == oth._dt;}
	bool operator!=(const mdict_const_iterator & oth)
		{return !operator==(oth);}
	
	mdict_const_iterator& operator++()
		{
			++_real;
			if (_real == std::end(_dt->_ht[0]))
				_real = std::begin(_dt->_ht[1]);
			return *this;
		}
	
	mdict_const_iterator operator++(int)
		{
			auto res = *this;
			this->operator();
			return res;
		}
	
	IT _real;
	const DICT* _dt;
};

template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class KeyEqual = std::equal_to<Key>,
	class Allocator = std::allocator< std::pair<const Key, T> >
	>
class mdict
{
public:
//	typedef mdict<Key, T, Hash, KeyEqual, KeyEqual, Allocator>  _self;
	typedef std::unordered_map<Key, T, Hash, KeyEqual, Allocator> Htable;
	typedef typename Htable::key_type           key_type;
	typedef T                                   value_type;
	typedef std::pair<const Key, T>             pair_type;
	typedef typename Htable::size_type          size_type;
	typedef typename Htable::allocator_type     allocator_type;
	typedef mdict_iterator<typename Htable::iterator, mdict>
	                                            iterator;
	typedef mdict_const_iterator<typename Htable::const_iterator, mdict>
	                                            const_iterator;

	friend iterator;
	friend const_iterator;
	
	mdict() {}
	mdict(void *privdata);
	~mdict() {};
		
	iterator random();
	const_iterator random() const;
	bool can_resize() const {return _can_resize;}
	int resize(size_type sz);
	int shrink_to_fit();
	bool is_rehashing() const {return _is_rehashing;}
	void clear();
	
	void rehash();
	size_type rehash_ms(size_type ms);
	void continue_hash();
	
	int rehash_step(size_type);
        int insert(const key_type& key,
		   const value_type& value);
	int insert_assign(const key_type& key,
			  const value_type& value);
	const_iterator find(const key_type& key) const;
	iterator find(const key_type& key);
	void randoms(int *count, std::vector<const_iterator> &vi) const;
	void randoms(int *count, std::vector<iterator> &vi); 
	void update(const key_type& key,
		    const value_type& value);
	void erase(const key_type& key);
	iterator begin();
//	const_iterator cbegin() {return begin();}
//	const_iterator cbegin() const {return begin();}
	const_iterator begin() const;
//	const_iterator cend() const {return end();}
	iterator end() {return iterator(_ht[1].end(), this);}
	const_iterator end() const {return const_iterator(_ht[1].end(), this);}

	static size_type next_size(size_type sz);
	static constexpr size_type init_sz = 4;
//	using Htable::v
//	using Htable::value_type;
  
private:
	void  *_privdata = nullptr;
	Htable _ht[2];
	//ssize_t _rehash_idx = -1;          // if -1 means is not rehasing
	size_type _iter_nums = 0;   //num of iterators
	bool _can_resize = true;
	bool _is_rehashing = false;
public:
	size_type size() const
		{
			return _ht[0].size() + _ht[1].size();
		}
};


NAMESPACE_END(monoco)
#endif // __MDICT_HPP_
