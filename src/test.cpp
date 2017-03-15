#include <iostream>
#include <memory>
#include <string>
#include "zl_entry.hpp"
#include "common.hpp"

using namespace std;
using namespace monoco;

int
main()
{
	shared_ptr<mbj> z = make_shared<zl_entry>();
	cout << (typeid(*z) == typeid(zl_entry)) << endl;
}
