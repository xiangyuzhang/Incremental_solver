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
// declear static 



lbool IncreSolver::ret = l_False;

const char *  IncreSolver::Orac_file_path;
const char * IncreSolver::Came_file_path;
const char * IncreSolver::Solver_solution = "Solver_solution";
const char * IncreSolver::target_cnf = "target.cnf";

SimpSolver IncreSolver::S;
SimpSolver IncreSolver::S_final;

int IncreSolver::miterOutIndex = 0;
int IncreSolver::cktTotVarNum = 0;                // number of wire including miter and oracle circuit 
int IncreSolver::camVarNum = 0;                   // total number of wires + inputs + CBs + outputs in the original cam ckt   

vector<int> IncreSolver::nodes2grab;
vector<int> IncreSolver::camCBindex;              // CB expect duplicated circuit
vector<int> IncreSolver::miterCBindex;            // CB include duplicated circuit 
vector<int> IncreSolver::camCB2index;             // suplication's CB
vector<int> IncreSolver::camPIndex;               // miter first circuit's PI, and also it the oracle's PI
vector<int> IncreSolver::camPOindex;
vector<int> IncreSolver::OracPOndex;
vector<string> IncreSolver::camCNFile;            // original Camouflaged circuit CNF

map<int, string> IncreSolver::indexVarDict;       // store map of index to netname
map<string, int> IncreSolver::varIndexDict;       // store map of netname to index


vector<map<int, string> > IncreSolver::OracPIs;   // store all temp PIs 
vector<map<int, string> > IncreSolver::OracPOs;   // store all temp POs              
//=================================================================================================
//implementation of IncreSolver
IncreSolver::IncreSolver()
{
//	cout << "IncreSolver is created" << endl;
}

IncreSolver::~IncreSolver()
{
//	cout << "IncreSolver is deleted" << endl;
}

vector<string> IncreSolver::assign_value(map<int, string> &value_map, vector<int> what)
{
    vector<string> result;
    vector<int>::iterator position = what.begin();
    for(map<int, string>::iterator index = value_map.begin(); index != value_map.end(); ++index)
    {
        if(index->second == "1") result.push_back(tostring(*position) + " 0\n");
        else if(index->second == "0") result.push_back("-" + tostring(*position) + " 0\n");
        position++;
    }
    return result;
}

vector<int> IncreSolver::get_index(vector<int> source, int correction)
{
    vector<int> target;
    for(vector<int>::iterator index = source.begin(); index != source.end(); ++index)
    {
        target.push_back(*index + correction);
    }
    return target;
}


vector<string> IncreSolver::duplicateCircuit(vector<string> cnFile, int start_index)
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

vector<string> IncreSolver::connectNets(vector<int> &piVec, int start_index)           
{
    vector<string> new_cnFile;
    for(vector<int>::iterator i = piVec.begin(); i != piVec.end(); ++i)
    {
        string piConsLine1 = tostring(*i) + " -" + tostring(*i + start_index) + " 0\n";
        new_cnFile.push_back(piConsLine1);
        string piConsLine2 = "-" + tostring(*i) + " " + tostring(*i + start_index) + " 0\n";
        new_cnFile.push_back(piConsLine2);
    } 
    return  new_cnFile; 
}


void IncreSolver::grabnodes()     
{
    if(ret == l_True)
    {

        grab(camPIndex, PItemp);
        grab(OracPOndex, POtemp);
        grab(camCBindex, CB1temp);
        grab(camCB2index, CB2temp);

        OracPIs.push_back(PItemp);
        OracPOs.push_back(POtemp);
    }

}

void IncreSolver::grab(vector<int> &list, map<int,string> &target)
{
    for(vector<int>::iterator node = list.begin(); node != list.end(); ++node)
    {
        if(S.model[*node - 1] != l_Undef)
        {
            if(S.model[*node - 1] == l_True) target.insert(std::pair<int,string>(*node, "1"));
            else target.insert(std::pair<int, string>(*node, "0"));
        }
    }
}
//=================================================================================================
//implementation of MiterSolver
MiterSolver::MiterSolver(char const * path1, char const * path2)
{
    Orac_file_path = path1;
    Came_file_path = path2;
}

MiterSolver::~MiterSolver()
{
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
    map<string, int> varIndexDict;      
    int varIndex = start;
    vector<string> cnFile;
    vector<int> posIndex;
    vector<string> PIs;
    vector<string> POs;
    vector<string> wires;
    int gateCnt = 0;
    int varNum = 0;
    int clauseNum = 0;
    vector<string> cnfLine;     
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
//              cout << *pi << endl;
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
                indexVarDict.insert(std::pair<int, string>(varIndex, *pi));
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
                varIndexDict.insert(std::pair<string,int>(*w, varIndex));
                indexVarDict.insert(std::pair<int, string>(varIndex, *w));
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
//    OR_connection.push_back(tostring(orIndex) + " 0\n");
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
    for(vector<int>::iterator pbitIndex = camCBindex.begin(); pbitIndex != camCBindex.end(); ++pbitIndex)
    {
        camCB2index.push_back(*pbitIndex + camVarNum);

    } 
    miterCBindex = camCBindex + camCB2index;
//========================================================================================================================    
//update oracPOnodes2grab with the linked oracle circuit
    for(vector<int>::iterator po = OracPOndex.begin(); po != OracPOndex.end(); ++po)
    {
        int newPO = *po + miterOutIndex;
        oracPONodes2grab.push_back(newPO);
    }
//========================================================================================================================
//nodes2grab now includes miterCBindex, camPIndex, OracPOndex 
    nodes2grab = camPIndex; 
    nodes2grab += miterCBindex;
    nodes2grab += OracPOndex;
    nodes2grab.push_back(miterOutIndex);
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
//    cout << "AddonSolver created" << endl;
}
AddonSolver::~AddonSolver()
{
//    cout << "AddonSolver is deleted" << endl;
}

void AddonSolver::freeze()
{
    static int already_frozen = 0;
    if(already_frozen == 0)
    {
        for(vector<int>::iterator index = nodes2grab.begin(); index != nodes2grab.end(); ++index)
        {
            S.setFrozen(*index - 1, true);
        }        
    }

}

void AddonSolver::start_solving()
{
    solve();        
    grabnodes();
    print_solution("iteration_solu");
    addconstrains();
}


void AddonSolver::print_solution(const char * path)
{

    ofstream outfile(path);
    outfile << "------The found PI vector that differentiate CB" << endl;
//    cout << "------The found PI vector that differentiate CB" << endl;
    outfile << "\n-----is\n";
//    cout << "\n-----is\n";
//    print_map(PItemp, outfile);
    outfile << "The corresponding primary outputs are:" << endl;
//    cout << "The corresponding primary outputs are:" << endl;
//    print_map(POtemp, outfile);    
    
}

void AddonSolver::print_map(map<int,string> &container, ofstream &outfile)
{
    for(map<int, string>::iterator index = container.begin(); index != container.end(); ++index)
    {
        outfile << index->first << "\t";
        cout << index->first << "\t";
    }
    outfile << endl;
    cout << endl;
    for(map<int, string>::iterator value = container.begin(); value != container.end(); ++value)
    {
        outfile << value->second << "\t";
        cout << value->second << "\t";
    }
    outfile << endl; 
    cout << endl;
}

void AddonSolver::solve()   // used to solve both miter and addons
{
    ofstream outfile(Solver_solution);
    gzFile in = gzopen(target_cnf, "rb");
    parse_DIMACS(in, S);
    gzclose(in);
    freeze();
    S.eliminate(true);
    if(!S.okay())
    {
        outfile << "UNSAT\n" << endl;
    }

    bool myRes = S.solve(mkLit(miterOutIndex - 1, false));
    if(myRes == true)   ret = l_True;
    else ret = l_False; 

    if(ret == l_True)
    {
        outfile << "SAT" << endl;
        for(int i = 0; i < S.nVars(); i++)
        {
            if(S.model[i] != l_Undef)
            {
                outfile << " ";
                if(S.model[i] == l_True) outfile << "" + tostring(i + 1);
                else if(S.model[i] == l_False) outfile << "-" + tostring(i + 1);
                
            }
        }
        outfile << " 0" << endl;
    }
    else if(ret == l_False) outfile << "UNSAT" << endl;
    else    outfile << "INDET" << endl;
    outfile.close();
}

void AddonSolver::addconstrains()
{
    if(ret == l_True)
    {
        //========================================================================================================================
        //create 2 duplication of original cam circuit
        int correction1 = cktTotVarNum;
        int correction2 = cktTotVarNum + camVarNum;
        vector<string> dupCkt1 =  duplicateCircuit(camCNFile, correction1);
        vector<string> dupCkt2 =  duplicateCircuit(camCNFile, correction2);
        dupCkt1.insert(dupCkt1.begin(), "c this is 1st duplication\n");
        dupCkt2.insert(dupCkt2.begin(), "c this is 2nd duplication\n");
        cktTotVarNum = correction2 + camVarNum; // update the total variable number
        //========================================================================================================================
        //connect CB of the two new duplications to miter
        vector<string> connect_CB1 = connectNets(camCBindex, correction1); 
        vector<string> connect_CB2 = connectNets(camCB2index, correction1); 
        connect_CB1.insert(connect_CB1.begin(), "c connect CB for 1st\n");
        connect_CB2.insert(connect_CB2.begin(), "c connect CB fot 2nd\n");
        addon_CB1 = get_index(camCBindex, correction1);
        addon_CB2 = get_index(camCBindex, correction2);
        addon_PI1 = get_index(camPIndex, correction1);
        addon_PI2 = get_index(camPIndex, correction2);
        addon_PO1 = get_index(camPOindex, correction1);
        addon_PO2 = get_index(camPOindex, correction2);

        //========================================================================================================================
        //assign new temp PI vector to new duplicated circuit
        vector<string> assign_PI1 = assign_value(PItemp, addon_PI1);
        vector<string> assign_PI2 = assign_value(PItemp, addon_PI2);
        assign_PI1.insert(assign_PI1.begin(), "c assign PI for 1st\n");
        assign_PI2.insert(assign_PI2.begin(), "c assign PI for 2nd\n");
        //========================================================================================================================
        //assign new temp PO vector to new duplicated circuit
        vector<string> assign_PO1 = assign_value(POtemp, addon_PO1);
        vector<string> assign_PO2 = assign_value(POtemp, addon_PO2);
        assign_PO1.insert(assign_PO1.begin(), "c assign PO for 1st\n");
        assign_PO2.insert(assign_PO2.begin(), "c assign PO for 2nd\n");
        //========================================================================================================================
        //merge all
        dupCkt1 += connect_CB1 + assign_PI1 + assign_PO1;
        dupCkt2 += connect_CB2 + assign_PI2 + assign_PO2;
        vector<string> addon = dupCkt1 + dupCkt2;
        int clauseNum = addon.size() - 8;
        string cmmtline1 = "c this file is generated by AddonSolver::addconstrains\n";
        string cmmtline2 = "c generated on " + get_localtime();
        string firstLine = "p cnf " + tostring(cktTotVarNum) + " " + tostring(clauseNum) + "\n";
        addon.insert(addon.begin(), firstLine);
        addon.insert(addon.begin(), cmmtline2);
        addon.insert(addon.begin(), cmmtline1); 
        print_vector(addon, target_cnf);
    }

}



//========================================================================================================================




//========================================================================================================================
//implementation of soluFinder
SoluFinder::SoluFinder()
{
}

SoluFinder::~SoluFinder()
{
}
void SoluFinder::freeze()
{
    for(vector<int>::iterator index = camCBindex.begin(); index != camCBindex.end(); ++index)
    {
        S.setFrozen(*index - 1, true);
    }            
}
void SoluFinder::find_solu()
{
    num2dup = OracPIs.size();
    totVarNum = num2dup*camVarNum;

    if(num2dup > 1) case_1();
    else case_2();
    string cmmtLine1 = "c This file is generated by SoluFinder\n";
    string cmmtLine2 = "c Generated on " + get_localtime();
    string firstLine = "p cnf " + tostring(totVarNum) + " " + tostring(clauseNum) + "\n";
    finalCNF.insert(finalCNF.begin(), firstLine);
    finalCNF.insert(finalCNF.begin(), cmmtLine1);
    finalCNF.insert(finalCNF.begin(), cmmtLine2);
    print_vector(finalCNF, "finalSolu.cnf");
    solve_it();
    print_solution();
}
void SoluFinder::case_1()
{
    vector<vector<string> > tmpCnfLs;
    for(int iteration = 0; iteration < num2dup; iteration++)
    {
        vector<string> duplication = duplicateCircuit(camCNFile, (iteration)*camVarNum);
        duplication.insert(duplication.begin(), "c this is the duplication\n");
        if(iteration != 0) 
        {
            vector<string> connection = connectNets(camCBindex, iteration*camVarNum);
            connection.insert(connection.begin(), "c this is connection\n");
            duplication += connection;
        }
        vector<int> PI_index = get_index(camPIndex, iteration*camVarNum);
        vector<int> PO_index = get_index(camPOindex, iteration*camVarNum);
        vector<string> assign_PI = assign_value(OracPIs.at(iteration), PI_index);
        assign_PI.insert(assign_PI.begin(), "c this is assign PI\n");
        vector<string> assign_PO= assign_value(OracPOs.at(iteration), PO_index);
        assign_PO.insert(assign_PO.begin(), "c this is assign PO\n");
        finalCNF+=duplication  + assign_PI + assign_PO;
    }

    totVarNum = num2dup * camVarNum;
    clauseNum = finalCNF.size() - 3*num2dup - num2dup + 1;
}

void SoluFinder::case_2()
{
    vector<string> duplication = camCNFile;
    vector<string> assign_PI = assign_value(OracPIs.at(0), camPIndex);
    vector<string> assign_PO = assign_value(OracPOs.at(0), camPOindex);
    camCNFile.insert(camCNFile.begin(), "c this is duplication\n");
    assign_PI.insert(assign_PI.begin(), "c this is assign_PI\n");
    assign_PO.insert(assign_PO.begin(), "c this is assign_PO\n");
    finalCNF+= duplication + assign_PI + assign_PO;
    totVarNum = camVarNum;
    clauseNum = finalCNF.size() - 3;
}

void SoluFinder::solve_it()
{
    cout << "start find finalSolu" << endl;
    fstream outfile(Solver_solution);
    gzFile in = gzopen("finalSolu.cnf", "rb");
    parse_DIMACS(in, S_final);
    gzclose(in);
    freeze();
    S_final.eliminate(true);
    if(!S_final.okay())
    {
        outfile << "UNSAT\n" << endl;
    }
    
    vec<Lit> dummy;
    lbool ret = S_final.solveLimited(dummy);

    if(ret == l_True)
    {
        outfile << "SAT" << endl;
        cout << "SAT" << endl;
        for(int i = 0; i < S_final.nVars(); i++)
        {
            if(S_final.model[i] != l_Undef)
            {
                outfile << " ";
                if(S_final.model[i] == l_True) outfile << "" + tostring(i + 1);
                else outfile << "-" + tostring(i + 1);
                
            }
        }
        outfile << " 0" << endl;
    }
    else if(ret == l_False) outfile << "UNSAT" << endl;
    else    outfile << "INDET" << endl;
    outfile.close();   
} 

void SoluFinder::print_solution()
{
    cout << "Solution is:" << endl;
    for(vector<int>::iterator node = camCBindex.begin(); node != camCBindex.end(); ++node)
    {
        if(S_final.model[*node - 1] != l_Undef)
        {
            if(S_final.model[*node - 1] == l_True) Solution.insert(std::pair<int,string>(*node, "1"));
            else Solution.insert(std::pair<int, string>(*node, "0"));
        }
    }
    for(map<int, string>::iterator index = Solution.begin(); index != Solution.end(); ++index)
    {
        cout << indexVarDict[index->first] << "\t";
    } 
    cout << endl;
    for(map<int, string>::iterator index = Solution.begin(); index != Solution.end(); ++index)
    {
        cout << index->second << "\t";
    }
    cout << endl;

}                                                                          
