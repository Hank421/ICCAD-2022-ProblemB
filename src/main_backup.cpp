#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>
#include <string>
#include <string.h>
#include <map>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <limits.h>
#include "Placement.h"
#include "Inst.h"
#include "LibCell.h"
#include "Net.h"
#include "Pin.h"
#include "Row.h"
#include "Terminal.h"
#include "../gsrc/GlobalPlacer.h"

using namespace std;

bool handleArgument(const int& argc, char* argv[])
{
    int i;
    if( argc != 3 ) {
        cout << "Usage: ./$binary_name <input.txt> <output.txt>" << endl;
        cout << "Ex: ./place ProblemB_case1_0506.txt output.txt" << endl;
        return false;
    }
    while( i < argc ) {
        if(strlen(argv[i]) <= 1 ) {
            i++;
            continue;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
////////////////////////////////////////////////////////////////
// Read Input File
////////////////////////////////////////////////////////////////
    time_t time_start = time(NULL);
    if (!handleArgument(argc, argv)) exit(-1);
    Placement placement;
    Placement dontTouchplace; // this is used to check for the correctness, please don't use it
    placement.readFile(argv);
    dontTouchplace.readFile(argv);
    cout << "\n============= Info Initial =============\n";
    placement.printInformation(false);

////////////////////////////////////////////////////////////////
// Partition
////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
// Global Placement
////////////////////////////////////////////////////////////////
    placement.randomPlace();
    placement.runRandomPartition();
    placement.random_SA(1000000, 10000, 0.99999);
    // GlobalPlacer globalPlacer(placement);
	// globalPlacer.place();

    cout << "\n============= Info After Global Placement =============\n";
    placement.printInformation(false);
    placement.DiePartition();

    cout << "\n============= Info After Die Partition =============\n";
    placement.printInformation(false);

////////////////////////////////////////////////////////////////
// Legalization
////////////////////////////////////////////////////////////////
    placement.TetrisLikeLegalization(100); // legalization_limits = 10
    placement.singleRowOptimization();
    // cout << endl << "////// Legalization ///////" << endl;
    // double lg_wirelength = 0;
    // bool bLegal = false;
    // time_t legal_time_start = time(NULL);
    // time_t total_legal_time = 0;
    // orig_wirelength = placement.computeHpwl();
    // CTetrisLegal legal(placement);
    // bLegal = legal.Solve( 0.8 );
    // if (bLegal) cout << "legalization success!" << endl;
    // else cout << "legalization fail!" << endl;
    // placement.outputBookshelfFormat(placement.name()+".lg.pl");
    // lg_wirelength = placement.computeHpwl();
    // printf( "\nHPWL: %.0f (%3.2f%%)\n", lg_wirelength, ((lg_wirelength - orig_wirelength)/orig_wirelength)*100.0);
    // total_legal_time = time(NULL) - legal_time_start;
    // total_time += total_legal_time;
    cout << "\n============= Info After Legalization =============\n";
    placement.printInformation(false);

////////////////////////////////////////////////////////////////
// Detail Placement
////////////////////////////////////////////////////////////////
    
    // cout << endl << "////// Detail Placement ///////" << endl;
    // double dp_wirelength = 0;
    // time_t detail_time_start = time(NULL);
    // time_t total_detail_time = 0;
    // orig_wirelength = placement.computeHpwl();
    // CDetailPlacer dplacer(placement);
    // dplacer.DetailPlace();
    // placement.outputBookshelfFormat(placement.name()+".dp.pl");
    // dp_wirelength = placement.computeHpwl();
    // printf( "\nHPWL: %.0f (%3.2f%%)\n", dp_wirelength, ((dp_wirelength - orig_wirelength)/orig_wirelength)*100.0);
    // total_detail_time = time(NULL) - detail_time_start;
    // total_time += total_detail_time;

////////////////////////////////////////////////////////////////
// Bounding Terminal Placement
////////////////////////////////////////////////////////////////

    placement.boundingTerminalInitial();
    placement.boundingTerminalPlacement();
    cout << "\n============= Info After Bounding Terminal Placement =============\n";
    placement.printInformation(false);

////////////////////////////////////////////////////////////////
// Output Final Result
////////////////////////////////////////////////////////////////
    
    // placement.outputTopDiePlotFile();
    placement.outputBottomDiePlotFile();
    // placement.outputTerminalPlotFile();
    cout << "\n============== Info Final =============\n";
    placement.printInformation(false);
    // placement.printNetlistInformation();
    placement.outputFile(argv);
    time_t total_time = time(NULL) - time_start;
    cout << "\nTotal runtime = " << total_time << endl;

    return 0;
}
