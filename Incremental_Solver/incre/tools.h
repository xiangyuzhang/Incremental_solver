//
// Created by parallels on 3/16/16.
//


#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>




using namespace std;

namespace Incre{

inline void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)   //source, result, seperator
{
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while(std::string::npos != pos2)      //if we can find c in s
  {
    v.push_back(s.substr(pos1, pos2-pos1));
 
    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}


inline string Readall(const char* path)
{

    fstream in;
    in.open(path, ios::in);
    istreambuf_iterator<char> beg(in), end;
    string strdata(beg, end);
    in.close(); 

//	cout << "Now start test" << endl;
//	cout << strdata << endl;
    return strdata;
}


inline void strip_all(string& str,const string old_value)
    {
    	while(true)
    	{
    		if(str.find(old_value) != string::npos)
    		{
    			str.replace(str.find(old_value), old_value.length(), "");
 //   			cout << "replaced string is " << str << endl;
    		}
    		else
    		{
    			break;
    		}
    	}

    }

inline    void load_gateTypeDict(map<int, string>& gateTypeDict)
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



 inline   vector<string> ReadByColon(const char* path)
    {
    	vector<string> result;
    	string read_all;
    	read_all = Readall(path);
    	strip_all(read_all, "\r");
    	SplitString(read_all, result, ";\n");
    	return result;
    }

}
