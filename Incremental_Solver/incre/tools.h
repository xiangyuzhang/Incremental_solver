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

 inline   string& replace_all(string& str,const string& old_value,const string& new_value)
    {
        while(true)
        {
            unsigned int pos=0;
            if((pos=str.find(old_value,0))!=string::npos)
                str.replace(pos,old_value.length(),new_value);
            else break;
        }
        return str;
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

 inline   vector<string> ReadByLine(const char* path)
    {
        vector<string> result;
        string s = "";
        ifstream infile("c432-abcmap-fmt.v");
        if(infile.fail())
        {
        	cout << "Error opening " << path << endl;
        	exit(-1);
        }
        else
        {
        	while(getline(infile, s))
        	{
        		result.push_back(s);
        	}
	        return result;
    	}

    }

}
