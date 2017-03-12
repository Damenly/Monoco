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
#include "server.hpp"

using namespace monoco;
using namespace std;

int
main(int argc, char** argv)
{
	try {
	server s("127.0.0.1", argv[1]);
	s.run();
	}
	catch(...) {}
}


	

