#include <map>
#include <string>
#include <iostream>
#include <vector>
#include "simp/SimpSolver.h"
using namespace std;

namespace Incre
{

class IncreSolver
{
public:
	IncreSolver(char const * path1, char const * path2);
	~IncreSolver();

protected:

	static char const * Orac_file_path;		//input Oracle file path
	static char const * Came_file_path;		//input Camouflage file path
	vector<string> duplicateCircuit(vector<string> &cnFile, int &start_index);
	vector<string> connectNets(vector<int> &piVec, int &start_index);			//using known start_index to connect two circuit

};


class MiterSolver : public IncreSolver {

private:
	static char const * Miter_file_path;			// Export the miter CNF formatted file into the file below
	vector<int> OracPIndex;
	vector<int> OracPOndex;
	vector<string> OraCNFile;
	unsigned int OracVarNum;

	vector<string> baseCnfMtrLs;		// completed miter (including original Cam, duplicated Cam, XOR, Or)
	vector<vector<int> > inputsInt;		// vector of PI (or CB) list
	vector<vector<int> > inputs;		// same to inputsInt
	vector<int> camPIndex;				//
	vector<int> camCBindex;				// CB expect duplicated circuit
	unsigned int pbitsNum;				// #CB
	unsigned int ObfGateNum;			// #obfusgates
	vector<int> camPOindex;				// PI index list
	unsigned int camVarNum;				// total number of wires + inputs + CBs + outputs in the original cam ckt	
	vector<string> camCNFile;			// original Camouflaged circuit CNF
	unsigned int baseMtrVarNum;			// total variable number (original + duplicated + XOR + OR)
	unsigned int PInum2grab;			// #PI, equal to original camouflaged circuit #PI
	unsigned int miterOutIndex; 		// last index of miter
public:
	MiterSolver(char const * path1, char const * path2, char const * path3);	//constructor: initialize base class and milterSolver
	~MiterSolver();		//deconstructor

	void buildmiter();		// build CNF formatted miter and export to Miter_file_path
private:
	void genOracCNF(char const * OracPath);
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

	map<int, int> Solution;		// store solution for this iteration

protected:
	static const char * Addon_file_path;		// export Addon circuit to this path

	void grabnodes(char* previous_solution_file);
	void addconstrains();
	void solve(const char* path);	// used to solve both miter and addons
};




//=================================================================================================
// Implementation of inline methods:





}