#include "mdict.hpp"
#include "common.hpp"
#include "utility.cpp"

NAMESPACE_BEGIN(monoco)

/* I am lazy man, So use macro below */
#define __MDICT_TMP_ \
template<\
	class Key,\
	class T,\
	class Hash,\
	class KeyEqual,\
	class Allocator\
	>
#define __MDICT__ mdict<Key, T, Hash, KeyEqual, Allocator>

__MDICT_TMP_
typename __MDICT__::size_type
__MDICT__::next_size(size_type sz)
{
	auto i = init_sz;
	while(i < sz)
		i << 1;
	return i;
}

__MDICT_TMP_
int
__MDICT__::shrink_to_fit()
{
	if(!_can_resize || is_rehashing())
		return MDICT_ERR;
	auto newsz = 0;
	if (newsz < init_sz)
		newsz = init_sz;
	
}

__MDICT_TMP_
int
__MDICT__::rehash_step(size_type n)
{
	auto sz = std::begin(_ht)->size();
	
	while (n-- && _ht[0].size()) {
		auto iter = _ht[0].begin();
		
	}

	if(_ht[0].size() == 0) {
		std::swap(_ht[0], _ht[1]);
		_ht[1].clear();
		_is_rehashing = false;
	}
	return MDICT_FINE;
}

__MDICT_TMP_
void
__MDICT__::rehash()
{
	if (is_rehashing())
		return ;
	
	_is_rehashing = true;
	rehash_step(size());
	_is_rehashing = false;
}


/*
__MDICT_TMP_
int __MDICT__::expand(size_type sz)
{
	auto newsz = next_size();
	if (is_rehashing() || _ht[0].size() > sz)
		return 1;
	
}
*/
__MDICT_TMP_
typename __MDICT__::size_type
__MDICT__::rehash_ms(size_type ms) {
	/*
    long long start = timeInMilliseconds();                                     
    int rehashes = 0;                                                           
                                                                                
    while(rehash_step(1000)) {                                                  
        rehashes += 1000;                                                        
        if (timeInMilliseconds()-start > ms) break;                             
    }     
                                                                      
    return rehashes;                                                            
	*/
	return 0;
}

__MDICT_TMP_
void
__MDICT__::continue_hash()
{
	if (!is_rehashing())
		return ;
	
	rehash(_ht[0].size());
}

__MDICT_TMP_
int
__MDICT__::resize(size_type sz)
{
	if (is_rehashing() || _ht[0].size() > sz)
		return 1;
	
}

__MDICT_TMP_
void __MDICT__::clear()
{
	for(auto &a : _ht)
		_ht->clear();
}

__MDICT_TMP_
void
__MDICT__::insert(const key_type& key,
		  const value_type& val)
{
	if(is_rehashing())
		_ht[1].insert(std::make_pair(key, val));
	else
		_ht[0].emplace(std::make_pair(key, val));
}

__MDICT_TMP_
typename __MDICT__::iterator
__MDICT__::begin()
{
	return iterator(std::begin(_ht[0]), this);
}

__MDICT_TMP_
typename __MDICT__::const_iterator
__MDICT__::begin() const
{
	return const_iterator(std::begin(_ht[0]), this);
}

__MDICT_TMP_
typename __MDICT__::iterator
__MDICT__::find(const key_type& key)
{
	auto iter = _ht[0].find(key);
	if (iter != _ht[0].end())
		return iterator(iter, this);
	iter = _ht[1].find(key);

	return iterator(iter, this);
}

__MDICT_TMP_
typename __MDICT__::const_iterator
__MDICT__::find(const key_type& key) const
{
	auto iter = _ht[0].find(key);
	if (iter != _ht[0].end())
		return const_iterator(iter, this);
	iter = _ht[1].find(key);

	return const_iterator(iter, this);
}

__MDICT_TMP_
void
__MDICT__::update(const key_type& key,                                        
		  const value_type& value)
{
	auto iter = find(key);
	iter->second = value;
}

__MDICT_TMP_
void
__MDICT__::erase(const key_type& key)
{
	for(auto &a : _ht)
		a.erase(key);
}

__MDICT_TMP_
typename __MDICT__::iterator
__MDICT__::random()
{
	auto pos = random_machine::gen_uint(0, size() - 1);
	auto iter = begin();
	std::advance(iter, pos);
	return iter;
}

__MDICT_TMP_
typename __MDICT__::const_iterator
__MDICT__::random() const
{
	auto pos = random_machine::gen_uint(0, size() - 1);
	auto iter = begin();
	std::advance(iter, pos);
	return iter;
}

NAMESPACE_END(monoco)

















