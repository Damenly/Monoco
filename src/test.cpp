
#include "mdict.cpp"
#include "intvec.cpp"
#include <iostream>

using namespace monoco;
using namespace std;

void foo(mdict<int,int> *cm)
{
	int i = 0;
	
	for(auto & a : *cm) {
		auto aa = cm->random();
		
		auto b = aa->first;
		//auto c = aa->second;
	}
	
	cout<<endl;
	
}


int
main()
{
	intvec c;
	c.creat<int16_t>();

	for (int16_t i = 0; i != 10; ++i) {
		c.add(i);
		c.remove(i);
	}
	c.add(INT32_MAX);
	c.add(INT64_MAX);
	c.add(1);
	for (auto i = c.size(); i != 0; --i) {
		cout << c.get(i) << " ";
		}
	cout<<endl;
}

