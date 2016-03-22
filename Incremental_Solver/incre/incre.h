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
	IncreSolver();
	~IncreSolver();

protected:

	static const char  * Orac_file_path;		// input Oracle file path
	static const char  * Came_file_path;		// input Camouflage file path
	static const char * Solver_solution;
	static const char  * target_cnf;			// output of buildmiter, input of solver, and output of addon
	static map<int, string> Solution;			// store solution for addon in this iteration
	static SimpSolver S;						// used for solve add on
	static SimpSolver S_final;					// used for solve finalSolue
	static lbool ret;							// indicator: indicate whether this iteration in addon is sat or not
	static	int miterOutIndex; 					// last index of miter
	static vector<int> nodes2grab;
	vector<string> duplicateCircuit(vector<string> &cnFile, int &start_index);
	vector<string> connectNets(vector<int> &piVec, int &start_index);			//using known start_index to connect two circuit
	void grabnodes();
};


class MiterSolver : public IncreSolver {

private:
	vector<int> OracPIndex;
	vector<int> OracPOndex;
	vector<string> OraCNFile;
	unsigned int OracVarNum;

	vector<string> baseCnfMtrLs;		// completed miter (including original Cam, duplicated Cam, XOR, Or)
	vector<vector<int> > inputsInt;		// vector of PI (or CB) list
	vector<vector<int> > inputs;		// same to inputsInt
	vector<int> camPIndex;				//
	vector<int> camCBindex;				// CB expect duplicated circuit
	vector<int> miterCBindex;			// CB include duplicated circuit 
	int pbitsNum;						// #CB
	int ObfGateNum;						// #obfusgates
	vector<int> camPOindex;				// PO index list
	int camVarNum;						// total number of wires + inputs + CBs + outputs in the original cam ckt	
	vector<string> camCNFile;			// original Camouflaged circuit CNF
	int baseMtrVarNum;					// total variable number (original + duplicated + XOR + OR)
	int PInum2grab;						// #PI, equal to original camouflaged circuit #PI
	int cktTotVarNum;					// number of wire including miter and oracle circuit 
	vector<int> oracPONodes2grab;		
public:
	MiterSolver(char const * path1, char const * path2);	//constructor: initialize base class and milterSolver
	~MiterSolver();		//deconstructor

	void buildmiter();		// build CNF formatted miter and export to Miter_file_path
private:
	void genOracCNF(char const * OracPath, int start);
	void genCameCNF(char const * CamePath);
	void genCameCNF(char const * CamePath, string Muxstyle);	// used for later

	vector<string> connectPO_xor(vector<int> &posIndex, int &camVarNum, int &xorInt);			//used only for two duplicated circuit
};



class SoluFinder : public IncreSolver 
{
public:
	SoluFinder();
	~SoluFinder();
};



class AddonSolver: public IncreSolver {
public:
	AddonSolver();		// constructor: for copy and init
	AddonSolver(AddonSolver &pre_AddonSolver);
	~AddonSolver();
	void start_solving();


protected:
	static Lit miterOut;
//	static vector<>	PItemp;
//	static vector<> POtemp;
//	static vector<> CBPtemp
	void addconstrains();
	void solve();	// used to solve both miter and addons
};




//=================================================================================================
// Implementation of inline methods:





}