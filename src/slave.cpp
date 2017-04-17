#include <iostream>

#include <memory>
#include <string>
#include <functional>
#include <unordered_set>

#include "cmds.cpp"
#include "slave.hpp"

using namespace monoco;
using namespace std;

int
main(int argc, char** argv)
{
	if (argc != 3) {
		std::cerr << "Usage: ./program address port" << std::endl;
		exit(1);
	}
	try {
		//utility::parse_config();
		auto s = make_shared<slave>(argv[1], argv[2]);
		s->run();
	}
	catch(exception &e) { cout <<e.what();}
}


	

