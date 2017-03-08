#include <iostream>
#include <memory>
#include <string>
#include "mlist.hpp"
#include "mht.hpp"
#include "mset.hpp"
#include "mzset.hpp"
#include "mstr.hpp"
#include "mdict.cpp"
#include <functional>
#include <unordered_set>
#include "db.hpp"
#include "common.hpp"

using namespace std;
using namespace monoco;

int
main()
{
	vector<zl_entry> zl;
	vector<string> ss = {string("2")};
	utility::args_to_zls(ss.begin(), ss.end(), zl);

	//errs::print(types::is_uint(zl[0].encode));
	errs::print(zl[0].to_u64());
}
	
