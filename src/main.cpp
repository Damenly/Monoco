#include <iostream>

#include <memory>
#include <string>
#include <functional>
#include <unordered_set>

#include "cmds.cpp"
#include "server.hpp"

using namespace monoco;
using namespace std;

int
main(int argc, char** argv)
{
	try {
		utility::parse_config();
		auto s = make_shared<server>("127.0.0.1", argv[1]);
		s->run();
	}
	catch(exception &e) { cout <<e.what();}
}


	

