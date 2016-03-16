#include <iostream>
#include <map>

#include "incre/incre.h"
#include "simp/SimpSolver.h"
#include "incre/tools.h"
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

void MiterSolver::buildmiter()
{
	cout << "start to buildmiter" << endl;
	cout << "The Miter_file_path is " << Miter_file_path << endl;
	cout << "The Oracle file is " << Orac_file_path << endl;
	cout << "The Came_file_path is " << Came_file_path << endl;
}

void genOracCNF(char const * OracPath)
{
	map<int, string> gateTypeDict;
	load_gateTypeDict(gateTypeDict);

}
void genCameCNF(char const * CamePath);