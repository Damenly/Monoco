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
	string s;
	errs::print(s, 1, "2", 3, 4, 5, 6);
	cout << s;
}
	
