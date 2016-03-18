#include <iostream>
#include <map>
#include <fstream>
#include <algorithm>

#include "incre/incre.h"
#include "simp/SimpSolver.h"
#include "incre/tools.h"
#include "incre/dict.h"
using namespace Minisat;
using namespace Incre;
using namespace std;

//=================================================================================================
// declear static variable
char const * IncreSolver::Orac_file_path;
char const * IncreSolver::Came_file_path;
char const * MiterSolver::Miter_file_path;



//=================================================================================================
//implementation of IncreSolver
IncreSolver::IncreSolver(char const * path1, char const * path2)
{
	cout << "IncreSolver is created" << endl;
	Orac_file_path = path1;
	Came_file_path = path2;
}

IncreSolver::~IncreSolver()
{
	cout << "IncreSolver is deleted" << endl;
}

//=================================================================================================
//implementation of MiterSolver
MiterSolver::MiterSolver(char const * path1, char const * path2, char const * path3):IncreSolver(path1, path2)
{
	cout << "MiterSolver is created" <<endl;
	Miter_file_path = path3;
}

MiterSolver::~MiterSolver()
{
	cout << "MiterSolver is deleted" << endl;
}

void MiterSolver::genOracCNF(char const * OracPath)
{
    map<string, int> gateTypeDict;
    ifstream infile;
    vector<string> Vlines;

    load_gateTypeDict(gateTypeDict);

    infile.open(OracPath);
    cout << "reading data from " << OracPath << endl;

    Vlines = ReadByColon(OracPath);
/*
    for(vector<string>::iterator iter = Vlines.begin(); iter != Vlines.end(); ++iter)
    {
        cout << *iter << endl;
    }
*/	
    // 1.1 Convert the original circuit to CNF format
    vector<vector<int> > inputs;
    map<string, int> varIndexDict;		//use name to find index, so string should be key
    int varIndex = 1;
    vector<string> cnFile;
    vector<int> posIndex;
    vector<string> PIs;
    vector<string> POs;
    vector<string> wires;
    int gateCnt = 0;
    int varNum = 0;
    int clauseNum = 0;
    vector<string> cnfLine;     //why we need cnfLines and cnfLine???

    for(vector<string>::iterator iter = Vlines.begin(); iter != Vlines.end(); ++iter)
    {
    	string line = *iter;
    	strip_all(line, "\n");


    	if((line.find("input") != string::npos) && (line.find("//") == string::npos))
    	{
    		cout << "Processing input (PI or CB)" << endl;
    		// use regex to find input netname 
    		strip_all(line, "input");
    		strip_all(line, " ");
    		SplitString(line, PIs, ",");
    		vector<int> tmpPis;
    		for(vector<string>::iterator pi = PIs.begin(); pi != PIs.end(); ++pi)
    		{
    			strip_all(*pi, "\\");
    			strip_all(*pi, "[");
    			strip_all(*pi, "]");
    			varIndexDict.insert(std::pair<string, int>(*pi, varIndex));
//    			cout << *pi << endl;
    			tmpPis.push_back(varIndex);
    			varIndex++;

    		}
    		inputs.push_back(tmpPis);
    	}

    	else if((line.find("output") != string::npos) && (line.find("//") == string::npos))
    	{
    		cout << "Processing output " << endl;
    		strip_all(line, "output");
    		strip_all(line, " ");
    		SplitString(line, POs, ",");
    		for(vector<string>::iterator po = POs.begin(); po != POs.end(); ++po)
    		{
    			strip_all(*po, "\\");
    			strip_all(*po, "[");
    			strip_all(*po, "]"); 
//    			cout << *po << endl;
    			posIndex.push_back(varIndex);
    			varIndexDict.insert(std::pair<string,int>(*po, varIndex));
    			varIndex++;   			
    		}

    	}

    	else if((line.find("wire") != string::npos) && (line.find("//") == string::npos))
    	{
    		cout << "Processing wire" << endl;
    		strip_all(line, "wire");
    		strip_all(line, " ");
    		SplitString(line, wires, ",");
    		for(vector<string>::iterator w = wires.begin(); w != wires.end(); ++w)
    		{
    			strip_all(*w, "\\");
    			strip_all(*w, "[");
    			strip_all(*w, "]"); 
//    			cout << *w << endl;
    			varIndexDict.insert(std::pair<string,int>(*w, varIndex));
    			varIndex++;   			
    		}
    	}

        else if( (line != "") && (line.front() != '/') && (line.find("module") == string:: npos))
        {
            string gate;
            vector<string> cnfLines;
//            cout << line << endl;
            if((line.find(".") != string::npos) && (line.find("(") != string::npos))
            {
                gate = find_gatetype(line);
                cout << "Gate type is " << gate << endl;
            }
            else
            {
                cout << "verilog format is not acceptable!!!" << endl;
                exit(-1);
            }
            vector<string> netname;
            vector<int> lineIn;
            int lineOut;

            netname = find_netname(line);
            lineOut = varIndexDict[netname.back()];
            for(vector<string>::iterator iter = netname.begin(); iter != netname.end(); ++iter)
            {
                cout << varIndexDict[*iter] << endl;
                lineIn.push_back(varIndexDict[*iter]);
            }

            int caseNo = gateTypeDict[gate];
            cnfLines = transGATE(caseNo, lineIn, lineOut);
            for(vector<string>::iterator iter = cnfLines.begin(); iter != cnfLines.end(); ++iter)
            {
                cnfLine.push_back(*iter);
            }
            gateCnt++;
        }
        varNum = varIndex - 1;
    }
    clauseNum = cnfLine.size();
    string cmmtline1 = "c This file is generated by oracV2cnf\n";
    string cmmtline2 = "c Generated on " + get_localtime() + " \n";
    clauseNum += inputs.front().size();
    string firstLine = "p cnf " + tostring(varNum) + " " + tostring(clauseNum) + " \n";
    cnfLine.insert(cnfLine.begin(), firstLine);
    cnfLine.insert(cnfLine.begin(), cmmtline2);
    cnfLine.insert(cnfLine.begin(), cmmtline1);


    OracPIndex = inputs.front();
    OracPOndex = posIndex;
    OraCNFile = cnFile;
    OracVarNum = varNum;

}
//void MiterSolver::genCameCNF(char const * CamePath);

void MiterSolver::buildmiter()
{
	cout << "start to buildmiter" << endl;
	cout << "The Miter_file_path is " << Miter_file_path << endl;
	cout << "The Oracle file is " << Orac_file_path << endl;
	cout << "The Came_file_path is " << Came_file_path << endl;
	genOracCNF(Orac_file_path);
}