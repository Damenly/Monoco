#include <iostream>
#include <memory>
#include <string>
#include "file.hpp"
#include "zl_entry.hpp"
#include "mlist.hpp"
#include "mset.hpp"

using namespace std;
using namespace monoco;

struct A
{
	int a = 10;
};

int
main()
{
	uint32_t ck;
	{

		if (fs::is_exists("fuck"))
		fs::rm("fuck");
		ofstream os("fuck", std::ios::binary);
		
		boost::crc_32_type crc;
		mset m;
		m.insert(1);
		m.insert("2");
		m.insert(A{10});
		m.write_to(os, crc);
		ck = crc.checksum();
	}
	{
		ifstream is("fuck", std::ios::binary);

		boost::crc_32_type crc;
		
		mset m;
		m.read_from(is, crc);
		vector<zl_entry> vz;
		m.getall(vz);
		for (auto && ele : vz)
			cout << ele << " ";
		cout << m.size() << " "<< (crc.checksum() == ck )<< endl;
		cout << types::M_SET << endl;
	}
   
	
}
	
