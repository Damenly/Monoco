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
#include "cmd.cpp"

using namespace monoco;
using namespace std;

int
main()
{
	auto db = std::make_shared<mdb>();
	string buffer;
	vector <string> args;
	
	while(getline(cin, buffer)) {
		args.clear();
		boost::split(args, buffer, boost::is_any_of(" "));
		for (int i = 0; i != args.size(); ++i) {
			if (args[i] == " ") {
				args.erase(args.begin() + i);
				--i;
			}
		}
		
		int ret = handle_cmds(db, args);
		if (ret)
			print(ret);
	}
}

	
