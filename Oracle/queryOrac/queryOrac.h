#ifndef INCRE_QUERYORAC_QUERYORAC_H
#define INCRE_QUERYORAC_QUERYORAC_H


#include <iostream>
#include <map>
#include <fstream>
#include <algorithm>
#include <string>




using std::vector;
using std::map;
using std::string;
using std::endl;
using std::cout;






class Oracle
{

private:
    const char * Orac_Path;                         // oracle path in
    const char * Orac_temp = "Orac_temp";
    const char * Orac_cnf = "Orac_cnf";
    const char * PI_path;
    const char * PO_path;

    vector<int> pisIndex;
    vector<int> posIndex;

    map<string, int> gateTypeDict;
    map<string, int> varIndexDict;
    map<int, string> indexVarDict;

    vector<string> cnFile;							// oracle cnf
    vector<string> PIs;								// Pis netname
    vector<string> POs;
    vector<string> wires;

    int varIndex = 1;
    int gateCnt = 0;
    int varNum = 0;
    int clauseNum = 0;

    map<int, string> PO_temp;						// store current iteration's PO value
    map<int, string> PI_temp;						// store current iteration's PI value

    vector<string> PI_assignment_cnf;
    vector<string> cnFile_temp;						// store current iteration's cnFile (with assignment)


public:
    Oracle(const char * ora, const char * PI, const char * PO);
    void process();

private:
    void genOracCNF();
    void parse_PI();
    void assign_PI();
    void solve();
    void export_PO();

};



#endif
