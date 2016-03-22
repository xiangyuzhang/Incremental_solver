#include <iostream>
#include <map>
#include <fstream>
#include <algorithm>
#include <string>
#include "incre/incre.h"
#include "simp/SimpSolver.h"
#include "incre/tools.h"
#include "incre/dict.h"
#include "utils/System.h"
#include "utils/ParseUtils.h"
#include "utils/Options.h"
#include "core/Dimacs.h"

using namespace Minisat;
using namespace Incre;
using namespace std;

//=================================================================================================
// declear static variable
char const * IncreSolver::Orac_file_path;
char const * IncreSolver::Came_file_path;
SimpSolver IncreSolver::S;
SimpSolver IncreSolver::S_final;
vector<int> IncreSolver::nodes2grab;
const char * IncreSolver::Solver_solution = "Solver_solution";
const char * IncreSolver::target_cnf = "target.cnf";
map<int, string> IncreSolver::Solution;
lbool IncreSolver::ret;
int IncreSolver::miterOutIndex;
Lit AddonSolver::miterOut;
//=================================================================================================
//implementation of IncreSolver
IncreSolver::IncreSolver()
{
	cout << "IncreSolver is created" << endl;
}

IncreSolver::~IncreSolver()
{
	cout << "IncreSolver is deleted" << endl;
}

vector<string> IncreSolver::duplicateCircuit(vector<string> &cnFile, int &start_index)
{
    vector<string> cnFile2;
    for(vector<string>::iterator iter = cnFile.begin(); iter != cnFile.end(); ++iter)
    {
        string tmpClause = *iter;
        vector<string> intIndexLs;
        SplitString(*iter, intIndexLs, " ");
        string newTmpCls;
        for(vector<string>::iterator j = intIndexLs.begin(); j != intIndexLs.end() - 1; ++j)
        {
            string tmpInt(*j);
            if(tmpInt.find("-") != string::npos)
            {
                strip_all(tmpInt, "-");
                newTmpCls += "-" + tostring(stoi(tmpInt) + start_index) + " ";
            }
            else
            {
                newTmpCls += tostring(stoi(tmpInt) + start_index) + " ";
            }
        }
        cnFile2.push_back(newTmpCls + "0\n");
    }  
    return cnFile2;
}

vector<string> IncreSolver::connectNets(vector<int> &piVec, int &start_index)           //piVec: nets to connect, start_index + 1: the first PI in second circuit you want to connect 
{
    vector<string> new_cnFile;
    for(vector<int>::iterator i = piVec.begin(); i != piVec.end(); ++i)
    {
        string piConsLine1 = tostring(*i) + " -" + tostring(*i + start_index) + " 0\n";
        new_cnFile.push_back(piConsLine1);
//        cout << piConsLine1;
        string piConsLine2 = "-" + tostring(*i) + " " + tostring(*i + start_index) + " 0\n";
        new_cnFile.push_back(piConsLine2);
//        cout << piConsLine2;
    } 
    return  new_cnFile; 
}

//=================================================================================================
//implementation of MiterSolver
MiterSolver::MiterSolver(char const * path1, char const * path2)
{
	cout << "MiterSolver is created" <<endl;
    Orac_file_path = path1;
    Came_file_path = path2;
}

MiterSolver::~MiterSolver()
{
	cout << "MiterSolver is deleted" << endl;
}
vector<string> MiterSolver::connectPO_xor(vector<int> &posIndex, int &camVarNum, int &xorInt)
{
    xorInt = camVarNum * 2;
    vector<string> cnfLines;
    for(vector<int>::iterator po = posIndex.begin(); po != posIndex.end(); ++po)
    {
        xorInt++;
        int inV1 = *po;
        int inV2 = *po + camVarNum;
        int outV = xorInt;
        cnfLines.push_back("-" + tostring(inV1) + " -" + tostring(inV2) + " -" + tostring(outV) + " 0\n");
        cnfLines.push_back(tostring(inV1) + ' ' + tostring(inV2) + " -" + tostring(outV) + " 0\n");
        cnfLines.push_back(tostring(inV1) + " -" + tostring(inV2) + " " + tostring(outV) + " 0\n");
        cnfLines.push_back("-" + tostring(inV1) + " " + tostring(inV2) + " " + tostring(outV) + " 0\n");
    }
    print_vector(cnfLines,"connectPO_xor");
    return cnfLines;
}
void MiterSolver::genOracCNF(char const * OracPath, int start)
{
    map<string, int> gateTypeDict;
    ifstream infile;
    vector<string> Vlines;

    load_gateTypeDict(gateTypeDict);

    cout << "reading data from " << OracPath << endl;

    Vlines = ReadByColon(OracPath);


    // 1.1 Convert the original circuit to CNF format
    vector<vector<int> > inputs;
    map<string, int> varIndexDict;		//use name to find index, so string should be key
    int varIndex = start;
    vector<string> cnFile;
    vector<int> posIndex;
    vector<string> PIs;
    vector<string> POs;
    vector<string> wires;
    int gateCnt = 0;
    int varNum = 0;
    int clauseNum = 0;
    vector<string> cnfLine;     //why we need cnfLines and cnfLine???
    cnfLine.push_back("c oracle circuit:\n");
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
                strip_all(*w, "\\t");
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
                strip_all(gate," ");
                strip_all(gate,"\n");
                strip_all(gate,"\t");
//                cout << "gate is: " << gate << endl;
//                cout << "Gate type is " << gate << endl;
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
            cnfLines = transGATE(caseNo, lineIn, lineOut);
            for(vector<string>::iterator iter = cnfLines.begin(); iter != cnfLines.end(); ++iter)
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
    clauseNum += inputs.front().size();
    string firstLine = "p cnf " + tostring(varNum) + " " + tostring(clauseNum - 1)  + " \n";
    //cnFile.insert(cnFile.begin(), firstLine);
    //cnFile.insert(cnFile.begin(), cmmtline2);
    //cnFile.insert(cnFile.begin(), cmmtline1);


    OracPIndex = inputs.front();
    OracPOndex = posIndex;
    OraCNFile = cnFile;
    OracVarNum = varNum - start + 1;

    ofstream outfile("OracCNF");
    for(vector<string>::iterator iter = cnFile.begin(); iter != cnFile.end(); ++iter)
    {
//        cout << *iter;
        outfile << *iter;
    }

}

void MiterSolver::genCameCNF(char const * CamePath)
{
    vector<string> cnfLines;
    vector<string> Vlines;
    vector<string> POs;
    vector<string> wires;
    vector<vector<int> > inputs;
    map<string, int> intVarDict;
    map<string, int> varIndexDict;
    map<string, int> gateTypeDict;
    vector<string> cnFile;
    vector<int> posIndex;
    camVarNum = 0;
    int varIndex = 1;
    int gateCnt = 0;

    load_gateTypeDict(gateTypeDict);

    cout << "Reading data from " << CamePath << endl;

    Vlines = ReadByColon(CamePath);
//======================================================================================================================
//parse the first camouflaged circuit 

    for(vector<string>::iterator iter = Vlines.begin(); iter != Vlines.end(); ++iter)
    {
        string line= *iter;
        strip_all(line, "\n");
        if((line.find("input") != string::npos) && (line.find("//") == string::npos))
        {
            vector<string> PIs;
            cout << "Processing input" << endl;
            strip_all(line, "input");
            strip_all(line, " ");
//            cout << line << endl;
            SplitString(line, PIs, ",");
            vector<int> tmpPis;
            for(vector<string>::iterator pi = PIs.begin(); pi != PIs.end(); ++pi)
            {
                strip_all(*pi, "\\");
                strip_all(*pi, "[");
                strip_all(*pi, "]");
                varIndexDict.insert(std::pair<string, int>(*pi, varIndex));
//                cout << *pi << " = " << varIndex << endl;
                tmpPis.push_back(varIndex);
                varIndex++;                
            }  
//            cout << "varIndex = " << varIndex << endl;
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
//              cout << *po << endl;
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
                strip_all(*w, "\\t");
//              cout << *w << endl;
                varIndexDict.insert(std::pair<string,int>(*w, varIndex));
                varIndex++;             
            }
        }
        
        else if((line != "") && (line.front() != '/') && (line.find("module") == string:: npos))
        {
            string gate;
            vector<string> cnfLines;
//            cout << line << endl;
            if((line.find(".") != string::npos) && (line.find("(") != string::npos))
            {
                gate = find_gatetype(line);
//                cout << "Gate type is " << gate << endl;
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
            strip_all(gate," ");
            strip_all(gate,"\n");
            strip_all(gate,"\t");
            int caseNo = gateTypeDict[gate];

 //           cout << "in mtr " << gate << endl;
            cnfLines = transGATE(caseNo, lineIn, lineOut);
            for(vector<string>::iterator iter = cnfLines.begin(); iter != cnfLines.end(); ++iter)
            {
//                cout << *iter << endl;
                cnFile.push_back(*iter);
            }
            gateCnt++;
        
        }                    
    }
    print_vector(cnFile, "original_cam");
    camVarNum = varIndex - 1;
    camCNFile = cnFile;
//========================================================================================================================
//duplicate another camouflage circuit 
    cout << "duplicate another circuit" << endl;
    vector<string> cnFile2 =  duplicateCircuit(cnFile, camVarNum);
    cout << "size of cnFile" << cnFile.size() << endl;
    cout << "size of cnFile2" << cnFile2.size() << endl;
    cnFile.push_back("c The second camouflage circuit:\n");
    vector<string> cnFile_merged;
    cnFile_merged = cnFile + cnFile2;
    print_vector(cnFile_merged, "cnFile_merged");


//========================================================================================================================
//add constrains
//1, connect PIs
    cout << "connect PIs" << endl;
    cnFile_merged.push_back("c Force PIs of 2 ckts to be the same:\n"); 
    vector<string> connection = connectNets(inputs[0], camVarNum); 
    vector<string> cnFile_PIconnected;          
    cnFile_PIconnected = cnFile_merged + connection;
    print_vector(cnFile_PIconnected, "cnFile_PIconnected");
//========================================================================================================================
//2, add forbidden bits

//========================================================================================================================
//3, use XOR connect POs
    cout << "connect POs using XOR" << endl;
    int xorInt = 0;
    cnFile_PIconnected.push_back("c XOR outputs of 2 ckts:\n");
    vector<string> XOR_connection = connectPO_xor(posIndex, camVarNum, xorInt);
    vector<string> cnFile_POconnected;
    cnFile_POconnected = cnFile_PIconnected + XOR_connection;
    print_vector(cnFile_POconnected, "cnFile_POconnected");
//========================================================================================================================
//4, use Or to connect all the XOR
    cout << "connect all the XORs using Or" << endl; 
    cnFile_POconnected.push_back("c The last OR gate of the miter:\n");
    int orIndex = xorInt + 1;
    vector<int> XOR_outputs;
    for (int XOR_index = camVarNum*2 + 1; XOR_index < orIndex; XOR_index++)
    {
        XOR_outputs.push_back(XOR_index);
    }
    vector<string> OR_connection = transGATE(2, XOR_outputs, orIndex);
    OR_connection.push_back(tostring(orIndex) + " 0\n");
    vector<string> final_miter;
    final_miter = cnFile_POconnected + OR_connection;

    int varNum = orIndex;
//========================================================================================================================
//add file info
    cout << "adding file info" << endl;
    int clauseNum = final_miter.size() - 5;
    string cmmtline1 = "c This file is generated by genCameCNF\n";
    string cmmtline2 = "c Generated on " + get_localtime();
    string firstLine = "p cnf " + tostring(varNum) + " " + tostring(clauseNum) + " \n";
//    final_miter.insert(final_miter.begin(), firstLine);
//    final_miter.insert(final_miter.begin(), cmmtline2);
//    final_miter.insert(final_miter.begin(), cmmtline1);


    baseCnfMtrLs = final_miter;
    inputsInt = inputs;
    camPIndex = inputs.front();
    for(vector<vector<int> >::iterator iter = inputs.begin() + 1; iter != inputs.end(); ++iter)
    {
        camCBindex += *iter;
    }
    pbitsNum = camCBindex.size();
    ObfGateNum = pbitsNum/2;
    camPOindex = posIndex;
    baseMtrVarNum = varNum;
    PInum2grab = camPIndex.size();
    miterOutIndex = varNum; 

    ofstream outfile("miterCNF");
    for(vector<string>::iterator iter = final_miter.begin(); iter != final_miter.end(); ++iter)
    {
//        cout << *iter;
        outfile << *iter;
    }

};


void MiterSolver::buildmiter()
{
	cout << "The miter will export to " << target_cnf << endl;
	cout << "The Oracle file is " << Orac_file_path << endl;
	cout << "The Came_file_path is " << Came_file_path << endl;
    genCameCNF(Came_file_path);
    genOracCNF(Orac_file_path, baseMtrVarNum + 1);
    cout << "start to buildmiter" << endl;
//========================================================================================================================
//add both Oracle and miter to single circuit
    cktTotVarNum = OracVarNum + baseMtrVarNum;
    cout << "OracVarNum = " << OracVarNum << endl;
    cout << "baseMtrVarNum" << baseMtrVarNum << endl;
//========================================================================================================================
//connect PIs oracle and miter
    vector<string> connect_orac_cam;
    vector<string> miter_with_orac;
    connect_orac_cam = connectNets(camPIndex, miterOutIndex);           //using known start_index to connect two circuit
    connect_orac_cam.insert(connect_orac_cam.begin(), "c connect oracle PI with cam PI\n");
    miter_with_orac = baseCnfMtrLs + OraCNFile + connect_orac_cam;
    string cmmtline1 = "c this file is generated by buildmiter\n";
    string cmmtline2 = "c generated on " + get_localtime();
    string problemLn = "p cnf " + tostring(cktTotVarNum) + " " + tostring(miter_with_orac.size() - 5) + "\n";
    miter_with_orac.insert(miter_with_orac.begin(), problemLn);
    miter_with_orac.insert(miter_with_orac.begin(), cmmtline2);
    miter_with_orac.insert(miter_with_orac.begin(), cmmtline1);
    print_vector(miter_with_orac, target_cnf);
//========================================================================================================================
//update miterCBindex to include duplication's CB
    nodes2grab = camPIndex;
    miterCBindex = camCBindex;
    for(vector<int>::iterator pbitIndex = camCBindex.begin(); pbitIndex != camCBindex.end(); ++pbitIndex)
    {
        miterCBindex.push_back(*pbitIndex + camVarNum);

    } 
//========================================================================================================================    
//update oracPOnodes2grab with the linked oracle circuit
    for(vector<int>::iterator po = OracPOndex.begin(); po != OracPOndex.end(); ++po)
    {
        int newPO = *po + miterOutIndex;
        oracPONodes2grab.push_back(newPO);
    }
//========================================================================================================================
//nodes2grab now includes miterCBindex, camPIndex, oracPOndex  
    nodes2grab += miterCBindex;
    nodes2grab += oracPONodes2grab;
    print_vector(camPIndex, "camPIndex");
    print_vector(camCBindex, "camCBindex");
    print_vector(miterCBindex, "miterCBindex");
    print_vector(nodes2grab, "nodes2grab");
    print_vector(OracPOndex, "OracPOndex");
    print_vector(oracPONodes2grab, "oracPONodes2grab");

}


//=================================================================================================
//implementation of AddonSolver


AddonSolver::AddonSolver():IncreSolver()
{
    cout << "AddonSolver created" << endl;
}
AddonSolver::~AddonSolver()
{
    cout << "AddonSolver is deleted" << endl;
}
void AddonSolver::start_solving()
{
    solve();        //testing: use same setting with Duo's version, Solution file is in "increIterationSolution.log"
    grabnodes();
    addconstrains();

}



void IncreSolver::grabnodes()     //Grab new PI, PO nodes from solution given by Minisat and stored in satRes.log
{
    cout << "call grabnodes" << endl;
    if(ret == l_True)
    {
        for(vector<int>::iterator node = nodes2grab.begin(); node != nodes2grab.end(); ++node)
        {
            if(S.model[*node - 1] != l_Undef)
            {
                if(S.model[*node - 1] == l_True) Solution.insert(std::pair<int,string>(*node, "1"));
                else Solution.insert(std::pair<int, string>(*node, "0"));
            }
        }

    }

}
void AddonSolver::addconstrains()
{
    cout << "call addconstrains" << endl;
}

void AddonSolver::solve()   // used to solve both miter and addons
{
    ofstream outfile(Solver_solution);
    cout << "open " << Solver_solution << endl;
    cout << "call solve" << endl;
    gzFile in = gzopen(target_cnf, "rb");
    parse_DIMACS(in, S);
    gzclose(in);
    S.eliminate(true);
    if(!S.okay())
    {
        outfile << "UNSAT\n" << endl;
        cout << "UnSAT" << endl;
        exit(20);
    }
    vec<Lit> dummy;
    ret = S.solveLimited(dummy);

    if(ret == l_True)
    {
        outfile << "SAT" << endl;
        for(int i = 0; i < S.nVars(); i++)
        {
            if(S.model[i] != l_Undef)
            {
                outfile << " ";
                if(S.model[i] == l_True) outfile << "" + tostring(i + 1);
                else outfile << "-" + tostring(i + 1);
                
            }
        }
        outfile << " 0" << endl;
    }
    else if(ret == l_False) outfile << "UNSAT" << endl;
    else    outfile << "INDET" << endl;
    outfile.close();
}
