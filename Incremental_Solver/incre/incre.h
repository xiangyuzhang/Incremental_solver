#include <map>
#include <string>
#include <iostream>
#include <vector>
#include "simp/SimpSolver.h"
#include "utils/System.h"
#include "utils/ParseUtils.h"
#include "utils/Options.h"
#include "core/Dimacs.h"

using namespace std;
using namespace Minisat;

namespace Incre
{


class IncreSolver
{
public:
	static lbool ret;							// indicator: indicate whether this iteration in addon is sat or not
	IncreSolver();
	~IncreSolver();
protected:
	static const char  * Orac_file_path;		// input Oracle file path
	static const char  * Came_file_path;		// input Camouflage file path
	static const char * Solver_solution;
	static const char  * target_cnf;			// output of buildmiter, input of solver, and output of addon

	static vector<int> camPIndex;				// miter first circuit's PI, and also it the oracle's PI
	static vector<int> camPOindex;				// PO index list
	static vector<int> OracPOndex;				
	static vector<int> camCBindex;				// CB except duplicated circuit
	static vector<int> miterCBindex;			// CB include duplicated circuit 
	static vector<int> camCB2index;				// suplication's CB
	static vector<int> nodes2grab;

	map<int, string> CB1temp;			// store temporary (only in this iteration) original CB index->value 
	map<int, string> CB2temp;			// store temporary (only in this iteration) duplication CB index->value
	map<int, string> PItemp;				// store temporary (only in this iteration) oracle PI index->value
	map<int, string> POtemp;				// store temporary (only in this iteration) oracle PO index->value
	map<int, string> Solution;			// store solution for addon in this iteration

	vector<int> addon_CB1;				// store temporary (only in this iteration) first duplication CB index
	vector<int> addon_CB2;				// store temporary (only in this iteration) second duplication CB index
	vector<int> addon_PI1;				// store temporary (only in this iteration) first duplication CB index	
	vector<int> addon_PI2;				// store temporary (only in this iteration) second duplication PI index
	vector<int> addon_PO1;				// store temporary (only in this iteration) first duplication PO index
	vector<int> addon_PO2;				// store temporary (only in this iteration) second duplication PO index

	static int cktTotVarNum;					// number of wire including miter and oracle circuit 
	static int camVarNum;						// total number of wires + inputs + CBs + outputs in the original cam ckt
	static int miterOutIndex; 					// last index of miter

	static vector<string> camCNFile;			// original Camouflaged circuit CNF

	static vector<map<int, string> > OracPIs;	// store all temp PIs 
	static vector<map<int, string> > OracPOs;	// store all temp POs

	static SimpSolver S;						// used for solve add on
	static SimpSolver S_final;					// used for solve finalSolue

protected:
	inline vector<string> duplicateCircuit(vector<string> cnFile, int start_index);		// tools: duplicate a circuit based on "cnFile", index start from "start_index"
	inline vector<string> assign_value(map<int, string> &value_map, vector<int> what);	// tools: use "value_map" value to assign elements in "what"
	inline vector<string> connectNets(vector<int> &piVec, int start_index);				// tools  using known start_index to connect two circuit
	inline void grab(vector<int> &list, map<int,string> &target);						// tools: searching elements in list in solution list(S.model[], index left shifted by 1), assign into target
	inline void grabnodes();															// main: obtain values from "S"  
	inline vector<int> get_index(vector<int> source, int correction);					// tools: used to get duplication's PI, PO, CB index. based on "source", calculate with "correction", store in "target" 

};


class MiterSolver : public IncreSolver {

private:
	int OracVarNum;
	int pbitsNum;																				// #CB
	int ObfGateNum;																				// #obfusgates
	int baseMtrVarNum;																			// total variable number (original + duplicated + XOR + OR)
	int PInum2grab;																				// #PI, equal to original camouflaged circuit #PI

	vector<string> OraCNFile;
	vector<string> baseCnfMtrLs;																// completed miter (including original Cam, duplicated Cam, XOR, Or)
	vector<vector<int> > inputsInt;																// vector of PI (or CB) list
	vector<vector<int> > inputs;																// same to inputsInt
	vector<int> oracPONodes2grab;
	vector<int> OracPIndex;
		
public:
	MiterSolver(char const * path1, char const * path2);										//constructor: initialize base class and milterSolver
	~MiterSolver();																				//deconstructor
	void buildmiter();																			// main: build CNF formatted miter and export to Miter_file_path
private:
	void genOracCNF(char const * OracPath, int start);											// main: parse "OracPath", generate CNF, index start on "start"
	void genCameCNF(char const * CamePath);														// main: parse "CamePath" and generate CNF
	void genCameCNF(char const * CamePath, string Muxstyle);									// main: used for later

	vector<string> connectPO_xor(vector<int> &posIndex, int &camVarNum, int &xorInt);			// tools: connect POs using xor, used only for two duplicated circuit
};



class SoluFinder : public IncreSolver 
{
public:
	SoluFinder();
	~SoluFinder();
	void find_solu();

private:
	int num2dup = 0;
	int totVarNum = 0;
	int clauseNum = 0;
	vector<string> finalCNF;
	void case_1();																					// main: build circuit, for the case that need more than one duplication
	void case_2();																					// main: build circuit, for the case that only one duplication needed
	void solve_it();																				// main: used to solve and find finalSolution
	void freeze();																					// main: used to setFrozen for camCBindex, so they will not be removed during simplification 

};



class AddonSolver: public IncreSolver {
	
public:
	AddonSolver();											
	~AddonSolver();
	void start_solving();																// main: start a iteration
	inline void print_solution(const char * path);										// tools: in current iteration, print out solution into "path"


private:
	inline void freeze();																// main: used to setFrozen for node2grab, so they will not be removed during simplification
	inline void print_map(map<int,string> &container, ofstream &outfile);				// tools: export content of "container" to "outfile"
	inline void addconstrains();														// main: based on solution, generate new addon circuit
	inline void solve();																// main: used to solve both miter and addconstrains
};




//=================================================================================================
// Implementation of inline methods:





}