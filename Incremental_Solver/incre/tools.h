#include <map>
#include <string>
#include <vector>
#include <sstream>

#include "incre/dict.h"



using namespace std;

namespace Incre{
void load_gateTypeDict(map<int, string>& gateTypeDict)
{
	cout << "load gateTypeDict" << endl;
	gateTypeDict.insert(std::pair<int, string>(1,"and"));
	gateTypeDict.insert(std::pair<int, string>(2,"or"));
	gateTypeDict.insert(std::pair<int, string>(3,"xor"));
	gateTypeDict.insert(std::pair<int, string>(4,"inv"));
	gateTypeDict.insert(std::pair<int, string>(5,"buf"));
	gateTypeDict.insert(std::pair<int, string>(6,"nand"));
	gateTypeDict.insert(std::pair<int, string>(7,"nor"));
	gateTypeDict.insert(std::pair<int, string>(8,"one"));
	gateTypeDict.insert(std::pair<int, string>(9,"zero"));
} 


}