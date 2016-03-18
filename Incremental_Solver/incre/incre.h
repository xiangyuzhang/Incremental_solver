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
	vector<string> duplicateCircuit(vector<string> cnFile, int start_index);
};


class MiterSolver : public IncreSolver {

private:
	static char const * Miter_file_path;			// Export the miter CNF formatted file into the file below
	vector<int> OracPIndex;
	vector<int> OracPOndex;
	vector<string> OraCNFile;
	int OracVarNum;
public:
	MiterSolver(char const * path1, char const * path2, char const * path3);	//constructor: initialize base class and milterSolver
	~MiterSolver();		//deconstructor

	void buildmiter();		// build CNF formatted miter and export to Miter_file_path
private:
	void genOracCNF(char const * OracPath);
	void genCameCNF(char const * CamePath);
	void genCameCNF(char const * CamePath, string Muxstyle);	// used for later
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