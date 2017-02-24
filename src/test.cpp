#include <iostream>
#include <memory>
#include <string>
#include "mlist.hpp"
#include "mht.hpp"
#include "mset.hpp"
#include "mzet.hpp"
#include <functional>
#include <unordered_set>

using namespace std;
using namespace monoco;
int
main()
{

	using namespace monoco::types;
	
	mset m;
	_mzset<int, int> z;
	z.size();
	
	vector<zl_entry> vz;
	unordered_set<size_t> hs;
	m.insert(1);
//	m.remove(1);
//	m.insert("2334", "string");
	for (int i = 0; i != 10000; ++i) {
		m.insert(i);
		if (i % 3)
			m.remove(to_string(i));
	}
	
	m.getall(vz);
	
	for (int i = 0; i != vz.size(); ++i) {
		auto ele = vz[i];

		if (types::is_int(ele.encode))
			cout << ele.to_s64() << endl;
		if (types::is_uint(ele.encode))
			cout << ele.to_u64() << endl;
		if (ele.encode == types::M_STR)
			cout << ele.safe_data<std::string>() << endl;

	}

}
	
