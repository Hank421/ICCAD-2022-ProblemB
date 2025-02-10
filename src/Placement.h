// #########################################################################################################
// this class is used to save all the information of placement for 2022 ICCAD CAD contest problem B
// all the things are setting public, so it is available to use them directly
// however, it is still available to use the functions to get the values
// #########################################################################################################

#ifndef PLACEMENT_H
#define PLACEMENT_H

#include <vector>
#include <string>
#include <string.h>
#include <map>
#include<bits/stdc++.h> 
#include "Inst.h"
#include "LibCell.h"
#include "Net.h"
#include "Pin.h"
#include "Row.h"
#include "Terminal.h"

using namespace std;

class Placement
{
public:
    Placement();

    void readFile(char *argv[]); // read input file
    void outputFile(char *argv[]); // output file
    long long computeApproxHpwl(); // compute approximate wire length
    long long computeHpwl(); // compute for HPWL
    void checkDataLegal(Placement dontTouchplace); // check whether there is illegal modify for the data
    void checkforResultCorrectness(); // check for the correctness of the placement result
    void printNetlistInformation();
    void printInformation(bool detail); // print related information
    void DiePartition(); // partition Inst into two dies
    void randomPlace(); // random place the cells
    void runRandomPartition(); // use partition-based method to place the cells
    void randomPartition(int start, int end, vector<int> &mod, int left, int bottom, int right, int top, vector<int> &net_cnt_1, vector<int> &net_cnt_2); // recursive call
    void random_SA(int swap_time, double T, double T_rate);
    int estimate_one_inst_WL(int rand0);
    int estimate_two_inst_WL(int rand0, int rand1);
    int estimate_one_die_inst_WL(int dieId, int rand0);
    void bruteForcePlacement(); // try to do the brute force placement if the case is small (ex: case1)
    // legalization
    int calculatePriority(int k1, int k2, int x, int width); // calculate priority for tetris like legalization
    int calculatePositionScore(int dieId, int rowId, int inst_x, int inst_y, int inst_w);
    int calculatePositionScore2(int rowId, Inst inst_x);
    void TetrisLikeLegalization(int legalization_limits); // run tetris-like legalization on both dies
    void singleRowOptimization();
    // bounding terminal
    bool Grid2XY(int grid_x, int grid_y, int &x, int &y); // for bounding terminal (trans (grid_x, grid_y) to (x, y))
    bool XY2Grid(int &grid_x, int &grid_y, int x, int y); // for bounding terminal (trans (x, y) to (grid_x, grid_y))
    void boundingTerminalInitial(); // initial the needed bounding terminals
    void boundingTerminalPlacement(); // place the bounding terminals
    // generate plot file
    void outputTopDiePlotFile();
    void outputBottomDiePlotFile();
    void outputTerminalPlotFile();

    //DP
    void SetModuleLocation( const int& id, int x, int y,bool bottomdie);
    void CalcPinLocation( Pin& pid, int cx, int cy );
    //int GetNetLength( vector<int>& pinsId );  
    long long calculateModuleHPWL(int mid,bool bottomdie);

    Inst inst(int instId) { return _insts[instId]; }
    Net net(int netId) { return _nets[netId]; }

    int numInsts() { return _insts.size(); }
    int numNets() { return _nets.size(); }

    void setNumInsts(unsigned size) { _insts.resize(size); }
    void setNumNets(unsigned size) { _nets.resize(size); }
    void setDieSize(int lowerLeftX, int lowerLeftY, int upperRightX, int upperRightY) {
        _boundryLeft = lowerLeftX;
        _boundryBottom = lowerLeftY;
        _boundryRight = upperRightX;
        _boundryTop = upperRightY;
    }

    void addInst(Inst inst) { _insts.push_back(inst); }
    void addRow(int techId, Row row) { (techId == 0) ? _topRow.push_back(row) : _bottomRow.push_back(row); }

    void clearInsts() { _insts.clear(); }
    void clearNets() { _nets.clear(); }

    // design data
    vector<LibCell> _libcell;
    vector<Inst> _insts;
    vector<Net> _nets;

    // map to ID
    map<string, int> _libcellName2ID;
    map<string, int> _instName2ID;
    map<string, int> _netName2ID;
    
    // techonology
    vector<string> _technologies; // size is at most 2

    // top/bottom die
    double _topDieMaxUtil; // 0 ~ 1
    double _bottomDieMaxUtil; // 0 ~ 1
    int _topDieTech; // 0 -> first technology, 1 -> second technology
    int _bottomDieTech; // 0 -> first technology, 1 -> second technology

    // row
    vector<Row> _topRow;
    vector<Row> _bottomRow;

    // terminal
    int _terminalSizeX;
    int _terminalSizeY;
    int _terminalSpacing;
    vector<Terminal> _terminals; // need to save terminals here

    // design statistics
    int _boundryTop;
    int _boundryLeft;
    int _boundryBottom;
    int _boundryRight;

    //DetailPlacement_TOP
    int topRow_h;
    int top_y2rowID(int y)
    {
        return (int)((y-_boundryBottom)/topRow_h);
    }

    //DetailPlacement_BOTTOM
    int bottomRow_h;
    int bottom_y2rowID(int y)
    {
        return (int)((y-_boundryBottom)/bottomRow_h);
    }

    //DP
    bool pIndepent;
    bool pRW;
};

#endif // PLACEMENT_H
