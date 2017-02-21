
#include "mdict.cpp"
#include "intvec.cpp"
#include <iostream>
#include "zlist.hpp"

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
	intvec s;
	s.creat<int64_t>();
       
}
	
