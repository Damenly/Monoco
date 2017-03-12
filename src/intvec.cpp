#include <cassert>
#include <algorithm>
#include "config.hpp"
#include "intvec.hpp"
#include "utility.cpp"
#include "common.hpp"

NAMESPACE_BEGIN(monoco)

void
intvec::_evolve(encode_type new_code)
{
	assert(new_code != _encode);
	std::vector<int16_t>* old16 = nullptr;
	std::vector<int32_t>* old32 = nullptr;

	std::vector<int32_t>* new32 = nullptr;
	std::vector<int64_t>* new64 = nullptr;

	if(_encode == INTVEC_ENC_INT16) {
		old16 = reinterpret_cast<std::vector<int16_t>*>(_vec);
		if (new_code == INTVEC_ENC_INT32) {
			new32 = new std::vector<int32_t>
				(old16->begin(), old16->end());
			_vec = reinterpret_cast<void*>(new32);
		}
		else {
			new64 = new std::vector<int64_t>
				(old16->begin(), old16->end());
			_vec = reinterpret_cast<void*>(new64);
		}
		delete old16;

	} else {
		auto old32 = reinterpret_cast<std::vector<int32_t>*>(_vec);
		auto new64 = new std::vector<int64_t>
			(old32->begin(), old32->end());
		delete old32;
		_vec = reinterpret_cast<void*>(new64);
	}
	_encode = new_code;
}

template <class T>
void intvec::add(const T& val)
{
	using namespace monoco::utility;
	if (_encode == INTVEC_ENC_INT64) {
		_add<T, int64_t>(val);
		return ;
	}

	if (_encode == INTVEC_ENC_INT32) {
	    if (hash_type(val) == types::M_INT64) {
			_evolve(INTVEC_ENC_INT64);
			_add<T, int64_t>(val);
	    }
	    else {
		    _add<T, int32_t>(val);
		    return ;
	    }
	}

	if (_encode == INTVEC_ENC_INT16) {
		if (hash_type(val) == types::M_INT64) {
			_evolve(INTVEC_ENC_INT64);
			_add<T, int64_t>(val);
	    }
		else if (hash_type(val) == types::M_INT32){
			_evolve(INTVEC_ENC_INT32);
			_add<T, int32_t>(val);
		}
	    else {
		    _add<T, int16_t>(val);
	    }
	}
}

template <typename T, typename VECT>
void intvec::_add(const T& val)
{
	auto vecp = _typed_vec<VECT>();
	vecp->insert(std::upper_bound(vecp->begin(), vecp->end(), val), val);
}

template <class T>
std::vector<T>*
intvec::_typed_vec() const
{
	return reinterpret_cast<std::vector<T>*>(_vec);
}

void
intvec::remove(int64_t val)
{
	if (_encode == INTVEC_ENC_INT64) {
		auto vec = _typed_vec<int64_t>();
		vec->erase(std::find(vec->begin(), vec->end(), val));
	}

	if (_encode == INTVEC_ENC_INT32) {
		auto vec = _typed_vec<int32_t>();
		vec->erase(std::find(vec->begin(), vec->end(),
				     static_cast<int32_t>(val)));
	}

	if (_encode == INTVEC_ENC_INT16) {
		auto vec = _typed_vec<int16_t>();
		vec->erase(std::find(vec->begin(), vec->end(),
				     static_cast<int16_t>(val)));
	}
}

template <class T>                                                     
void
intvec::get(size_type pos, T* val_ret) const
{
	assert(val_ret);
	auto vec = _typed_vec<T>();
	*val_ret = *(vec->begin() + pos);
}

int64_t
intvec::random() const
{
	auto pos = random_machine::gen_uint(0, size() - 1);
	return get(pos);
}

int64_t
intvec::get(size_type pos) const
{
	int64_t res;

	if (_encode == INTVEC_ENC_INT64) {
		auto vec = _typed_vec<int64_t>();
		res = vec->operator[](pos);
	}

	if (_encode == INTVEC_ENC_INT32) {
		auto vec = _typed_vec<int32_t>();
		res = vec->operator[](pos);
	}

	if (_encode == INTVEC_ENC_INT16) {
		auto vec = _typed_vec<int16_t>();
		res = vec->operator[](pos);
	}

	return res;
}

intvec::~intvec()
{
	if (_vec == nullptr)
		return ;
	if (_encode == INTVEC_ENC_INT16) {
		auto vec = _typed_vec<int16_t>();
		delete vec;
	}
	if (_encode == INTVEC_ENC_INT32) {
		auto vec = _typed_vec<int32_t>();
		delete vec;
	}
	if (_encode == INTVEC_ENC_INT64) {
		auto vec = _typed_vec<int64_t>();
		delete vec;
	}
}

intvec::size_type
intvec::size() const
{
	if (_encode == INTVEC_ENC_INT16) {
		auto vec = _typed_vec<int16_t>();
		return vec->size();
	}
	if (_encode == INTVEC_ENC_INT32) {
		auto vec = _typed_vec<int32_t>();
		return vec->size();
	}
	if (_encode == INTVEC_ENC_INT64) {
		auto vec = _typed_vec<int64_t>();
		return vec->size();
	}
	return 0;
}

NAMESPACE_END(monoco)
