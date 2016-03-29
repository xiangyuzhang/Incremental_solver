#include <iostream>
#include <string>
#include "queryOrac/queryOrac.h"



int main(int argc, char ** argv) {
    const char * orac = argv[1];
    const char * PI = argv[2];
    const char * PO = argv[3];
    cout << "start" << endl;
    Oracle ORA(orac, PI, PO);
    ORA.process();
    return 0;
}
