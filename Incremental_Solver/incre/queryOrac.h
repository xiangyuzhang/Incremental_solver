#include <iostream>
#include <map>
#include <fstream>
#include <algorithm>
#include <string>
#include "simp/SimpSolver.h"
#include "incre/tools.h"
#include "incre/dict.h"
#include "utils/System.h"
#include "utils/ParseUtils.h"
#include "utils/Options.h"
#include "core/Dimacs.h"



#ifndef INCRE_INCRE_QUERYORAC_H
#define INCRE_INCRE_QUERYORAC_H


using namespace Incre;
using namespace std;
using namespace Minisat;
class Oracle 
{

private:
	static const char * Orac_Path;
	const char * Orac_temp = "Orac_temp";
	const char * Orac_cnf = "Orac_cnf";

    static vector<int> pisIndex;
    static vector<int> posIndex;

	static map<string, int> gateTypeDict;
    static map<string, int> varIndexDict;   
    static map<int, string> indexVarDict; 

    static vector<string> cnFile;							// oracle cnf 
    static vector<string> PIs;								// Pis netname
    static vector<string> POs;
    static vector<string> wires;

    static int varIndex;
    static int gateCnt;
    static int varNum; 
    static int clauseNum;

    map<int, string> PO_temp;						// store current iteration's PO value
    map<int, string> PI_temp;						// store current iteration's PI value

   	map<string, string> PI_from_outside;				
   	map<string, string> PO_to_outside; 
    vector<string> PI_assignment_cnf;
   	vector<string> cnFile_temp;						// store current iteration's cnFile (with assignment)


public:
	Oracle();
	map<string, string>  getPOs();
	void getPIs(map<string, string> input);			// main: get PI_temp from outside
	static void set_path(const char * path);
private:
	void genOracCNF();	
	void convert_PI();			// tools: convert name->value to index->value
	void assign_PIs();			// tools: use map<index,value> to generate assignment for PI_value
	void solve_oracle();
	void convert_PO();

};

//=========================================================================================================================
// declare static 

static const char * Orac_Path;



static vector<int> pisIndex;
static vector<int> posIndex;
static map<string, int> gateTypeDict;
static map<string, int> varIndexDict;   
static map<int, string> indexVarDict; 
static vector<string> cnFile;
static vector<string> PIs;                              // Pis netname
static vector<string> POs;
static vector<string> wires;
static int varIndex = 1;
static int gateCnt = 0;
static int varNum = 0;
static int clauseNum = 0;


//=========================================================================================================================
//implement Oracle

Oracle::Oracle()
	{
		cout << "a Oracle is created" << endl;
	}
void Oracle::set_path(const char * path)
{
        Orac_Path = path;
}

void Oracle::getPIs(map<string, string> input)					// main: get PI_temp from outside
{
	PI_from_outside = input;
}


map<string, string>  Oracle::getPOs()
{
    if(cnFile.size() == 0) genOracCNF();
	convert_PI();			// convert current iteration's PI to index->value
	assign_PIs();
	solve_oracle();		
	convert_PO();
	return PO_to_outside;

}


void Oracle::convert_PI()
{
	int index;
	string value;
	for(auto &x:PI_from_outside)
	{
		index = varIndexDict[x.first];
		value = x.second;
		PI_temp.insert(std::pair<int, string>(index, value));
	}

}

void Oracle::convert_PO()
{
	string netname;
	string value;
	for(auto &x:PO_temp)
	{
		netname = indexVarDict[x.first];
		value = x.second;
		PO_to_outside.insert(std::pair<string, string>(netname, value));
	}
}
void Oracle::solve_oracle()
{
	SimpSolver S;
	map<int, string> result;
	gzFile in = gzopen(Orac_temp, "rb");
	parse_DIMACS(in, S);
	gzclose(in);
    for(vector<int>::iterator index = pisIndex.begin(); index != pisIndex.end(); ++index)
    {
        S.setFrozen(*index - 1, true);
    } 
	S.eliminate(true);
	if(!S.okay())
	{
		cout << "UNSAT" << endl;
	}
	vec<Lit> dummy;
	lbool ret = S.solveLimited(dummy);
	if(ret == l_True)
	{
		cout << "find Oracle POs" << endl;
		for(auto &x:posIndex)
		{
			if(S.model[x] == l_True) result.insert(std::pair<int, string>(x, "1"));
			else result.insert(std::pair<int, string>(x, "0"));
		}
	}
	PO_temp = result;
}
void Oracle::assign_PIs()
{
    vector<int>::iterator position = pisIndex.begin();
    for(map<int, string>::iterator index = PI_temp.begin(); index != PI_temp.end(); ++index)
    {
        if(index->second == "1") PI_assignment_cnf.push_back(tostring(*position) + " 0\n");
        else if(index->second == "0") PI_assignment_cnf.push_back("-" + tostring(*position) + " 0\n");
        position++;
    }
    cnFile_temp = cnFile + PI_assignment_cnf;
    print_vector(cnFile_temp, Orac_temp);
}
void Oracle::genOracCNF()
{
    ifstream infile;
    vector<string> Vlines;
    load_gateTypeDict(gateTypeDict);

    cout << "reading data from " << Orac_Path << endl;

    Vlines = ReadByColon(Orac_Path);

     
    cnFile.push_back("c oracle circuit:\n");
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
            for(vector<string>::iterator pi = PIs.begin(); pi != PIs.end(); ++pi)
            {
                strip_all(*pi, "\\");
                strip_all(*pi, "[");
                strip_all(*pi, "]");
                varIndexDict.insert(std::pair<string, int>(*pi, varIndex));
                indexVarDict.insert(std::pair<int, string>(varIndex, *pi));
                pisIndex.push_back(varIndex);
                varIndex++;

            }
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
//              cout << *po << endl;
                posIndex.push_back(varIndex);
                varIndexDict.insert(std::pair<string, int>(*po, varIndex));
                indexVarDict.insert(std::pair<int, string>(varIndex, *po));
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
                strip_all(*w, "\\t");
//              cout << *w << endl;
                varIndexDict.insert(std::pair<string, int>(*w, varIndex));
                indexVarDict.insert(std::pair<int, string>(varIndex, *w));
                varIndex++;             
            }
        }

        else if( (line != "") && (line.front() != '/') && (line.find("module") == string:: npos))
        {
            string gate;
            vector<string> gate_cnf;
//            cout << line << endl;
            if((line.find(".") != string::npos) && (line.find("(") != string::npos))
            {

                gate = find_gatetype(line);
                strip_all(gate," ");
                strip_all(gate,"\n");
                strip_all(gate,"\t");
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
            for(vector<string>::iterator iter = netname.begin(); iter != netname.end() - 1; ++iter)
            {
//                cout << varIndexDict[*iter] << endl;
                lineIn.push_back(varIndexDict[*iter]);
            }

            int caseNo = gateTypeDict[gate];
            gate_cnf = transGATE(caseNo, lineIn, lineOut);
            for(vector<string>::iterator iter = gate_cnf.begin(); iter != gate_cnf.end(); ++iter)
            {
//                cout << *iter << endl;
                cnFile.push_back(*iter);
            }
            gateCnt++;
        }
        varNum = varIndex - 1;
    }
    clauseNum = cnFile.size();
    string cmmtline1 = "c This file is generated by oracV2cnf\n";
    string cmmtline2 = "c Generated on " + get_localtime() + " \n";
    clauseNum += pisIndex.size();
    string firstLine = "p cnf " + tostring(varNum) + " " + tostring(clauseNum - 1)  + " \n";



    print_vector(cnFile, Orac_cnf);
}

#endif
