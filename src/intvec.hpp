#ifndef INTVEC
#define INTVEC

#define INTVEC

#include <vector>
#include <iterator>
#include <memory>
#include <typeinfo>
#include <cstdint>
#include <iostream>

NAMESPACE_BEGIN(monoco)

#define INTVEC_ENC_INT16 (sizeof(int16_t))
#define INTVEC_ENC_INT32 (sizeof(int32_t))
#define INTVEC_ENC_INT64 (sizeof(int64_t))

template <class IT>
struct intvec_iterator
{

};


class intvec
{
public:
	typedef std::size_t                   size_type;
	typedef std::vector<int16_t>          Vec_INT_16;
	typedef std::vector<int32_t>          Vec_INT_32;
	typedef std::vector<int64_t>          Vec_INT_64;
	typedef uint8_t                       encode_type;

	intvec() = default;
	~intvec();
	
	size_type size() const;
	int64_t random() const;
	void add(int16_t val);
	void add(int32_t val);
	void add(int64_t val);
	void remove(int64_t val);
	static encode_type propert_encode(int64_t sz);
private:
	void *_vec = nullptr; //ptr_to vector;
	encode_type _encode = 0;
	
private:
	void alloc_vec();
	void _evolve(encode_type new_code);
	template <class T>
	std::vector<T>* _typed_vec() const;


public:

	template <class T>
	void creat()
		{
			_vec = reinterpret_cast<void *>(new std::vector<T>());
			_encode = sizeof(T);
		}
	template <class T>
	void get(size_type pos, T* val_ret) const;
	int64_t get(size_type pos) const;
};

NAMESPACE_END(monoco)
#endif  // INTVEC
