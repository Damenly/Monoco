#ifndef __MHT_HPP_
#define __MHT_HPP_

#include <vector>
#include <unordered_map>
#include <functional>
#include "zl_entry.hpp"
#include "mbj.hpp"

NAMESPACE_BEGIN(monoco)

class mht : public mbj
{
public:
	typedef std::string key_type;
	typedef monoco::zl_entry    value_type;
	
	static constexpr uint8_t VEC_TAG = 1;
	static constexpr uint8_t MAP_TAG = 2;
	uint8_t tag = VEC_TAG;
	
private:
	std::vector<std::pair<key_type, value_type>> _vec;
	std::unordered_map<key_type, value_type> _map;

	void _evolve()
		{
			if (tag == MAP_TAG)
				return ;
			for (auto && ele: _vec)
				_map.emplace(ele.first, ele.second);
			_vec.clear();
			tag = MAP_TAG;
		}

	template <typename T>
	void try_evolve(const key_type& key, const T& val)
		{
			if (key.size() > configs::mht_max_size ||
				_vec.size() > configs::mht_max_len)
				_evolve();
		}

public:

	virtual string type_name() const {return "mht";}
	
	void clear()
		{
			_vec.clear();
			_map.clear();
			tag = VEC_TAG;
		}

	template <typename T>
	void insert(const key_type& key, const T& val)
		{
			try_evolve(key, val);
			
			if (tag == VEC_TAG)
				_vec.emplace_back(key, val);
			else
				_map.emplace(key, val);
		}
	
	void remove(const key_type& key)
		{
			auto _erase = [&key](auto &con)
				{
					auto iter = std::find_if(con.begin(), con.end(),
											 [&key](auto & p)
			                                 {
												 return p.first == key;
											 });
					if (iter != con.end())
						con.erase(iter);
				};

			if (tag == VEC_TAG)
				_erase(_vec);
			else
				_erase(_map);
		}

	auto get(const key_type& key)
		{
			if (tag == VEC_TAG) {
				auto iter = std::find_if(_vec.begin(), _vec.end(),
										 [&key](auto& p)
										 {
											 return (p.first == key);
										 });
				if (iter == _vec.end())
					return value_type();
				else
					return iter->second;
			}
			else {
				auto iter = _map.find(key);
				if (iter == _map.end())
					return value_type();
				else
					return iter->second;
			}
		}

	void update(const key_type& key, const value_type& val)
		{
			if (tag == VEC_TAG) {
				auto iter = std::find_if(_vec.begin(), _vec.end(),
										 [&key](auto& p)
										 {
											 return (p.first == key);
										 });
				if (iter == _vec.end())
					_vec.emplace_back(key, val);
				else
					iter->second = val;
			}
			else {
				_map[key] = val;
			}
		}


	virtual std::size_t size() const
		{return tag == VEC_TAG ? _vec.size() : _map.size();}

	bool exists(const key_type& key)
		{
			if (tag == VEC_TAG) {
				auto iter = std::find_if(_vec.begin(), _vec.end(),
										 [&key](auto& p)
										 {
											 return (p.first == key);
										 });
				return iter == _vec.end();
			}
			else {
				return _map.end() == _map.find(key);
			}
		}

	template <typename CON>
	void getall(CON &con)
		{
			if (tag == VEC_TAG)
				for(auto &&ele : _vec)
					con.push_back(ele);
			else
				for(auto &&ele : _map)
					con.push_back(ele);
		}

	virtual
	std::ofstream&
	write_to(std::ofstream& os, boost::crc_32_type& crc) const
		{
			size_t holder;
			holder = types::hash_idx<mht>();
			fs::write_to(os, holder, crc);

			holder = size();
			fs::write_to(os, holder, crc).flush();
			
			if (tag == VEC_TAG) {
				for (auto && ele : _vec) {
					fs::write_to(os, ele.first, crc);
					ele.second.write_to(os, crc);
				}
			}
			else {
				for (auto && ele : _map) {
					fs::write_to(os, ele.first, crc);
					ele.second.write_to(os, crc);
				}
			}
			os.flush();
			return os;
		}

	virtual std::ifstream&
	read_from(std::ifstream& is, boost::crc_32_type& crc)
		{
			size_t holder;
			
			// read size
			fs::read_from(is, holder, crc);

			if (tag == VEC_TAG) {
				std::pair<key_type, value_type> pr;
				for (size_t i = 0; i != holder; ++i) {
					fs::read_from(is, pr.first, crc);
					pr.second.read_from(is, crc);
					
					_vec.push_back(pr);
				}
			}
			else {
				string str;
				zl_entry ze;

				for (size_t i = 0; i != holder; ++i) {
					fs::read_from(is, str, crc);
					ze.read_from(is, crc);

					_map.emplace(str, ze);
				}
			}
			
			return is;
		}
};

template <>
void
mht::try_evolve<std::string>(const key_type& key, const string& value)
{
	if (key.size() > configs::mht_max_size ||
		value.size() > configs::mht_max_size ||
		_vec.size() > configs::mht_max_len)
		_evolve();
}

NAMESPACE_BEGIN(types)
template <>
string type_name<mht>() {return "mht";}

static const size_t M_HT = hash_idx<mht>();

NAMESPACE_END(types)
	
NAMESPACE_END(monoco)
#endif
