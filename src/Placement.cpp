#include <stdlib.h>
#include <numeric>
#include <iostream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <cmath>
#include <limits.h>
#include<bits/stdc++.h> 
#include "Placement.h"
#include "Inst.h"
#include "LibCell.h"
#include "Net.h"
#include "Pin.h"
#include "Row.h"
#include "Terminal.h"

using namespace std;

Placement::Placement() {}

void error_message(int i, string expected = "", string get = "")
{
    if (i == 0) {
        cout << "Error: Cannot open input file" << endl;
        exit(-1);
    }
    if (i == 1) {
        cout << "Error: Wrong keyword (expected: " << expected << ", get: " << get << ")" << endl;
        exit(-1);
    }
}

void Placement::readFile(char *argv[])
{
    ifstream fin;
    fin.open(argv[1], ios::in);
    if (!fin.is_open()) 
        error_message(0);
    // start parsing input file
    string keyword;
    // read in the technology informations and save them with class "LibCell" and "Pin"
    int technologyCount;
    fin >> keyword >> technologyCount;
    if (keyword != "NumTechnologies") 
        error_message(1, "NumTechnologies", keyword);
    for (int i=0; i<technologyCount; i++) {
        string techName;
        int libCellCount;
        fin >> keyword >> techName >> libCellCount;
        if (keyword != "Tech")
            error_message(1, "Tech", keyword);
        _technologies.push_back(techName);
        for (int j=0; j<libCellCount; j++) {
            string libCellName;
            int libCellSizeX, libCellSizeY, pinCount;
            fin >> keyword >> libCellName >> libCellSizeX >> libCellSizeY >> pinCount;
            if (keyword != "LibCell")
                error_message(1, "LibCell", keyword);
            if (i == 0) {
                LibCell lib(libCellName);
                lib.setSize(i, libCellSizeX, libCellSizeY);
                _libcellName2ID[libCellName] = _libcell.size();
                _libcell.push_back(lib);
            }
            else {
                _libcell[j].setSize(i, libCellSizeX, libCellSizeY);
            }
            for (int k=0; k<pinCount; k++) {
                string pinName;
                int pinLocationX, pinLocationY;
                fin >> keyword >> pinName >> pinLocationX >> pinLocationY;
                if (keyword != "Pin") 
                    error_message(1, "Pin", keyword);
                Pin pin(pinName, pinLocationX, pinLocationY);
                _libcell[j].addPin(i, pin);
                _libcell[j]._pinName2Id[pinName] = k;
            }
        }
    }

    // read in the die size information, and set the size for both top and bottom dies
    int lowerLeftX, lowerLeftY, upperRightX, upperRightY;
    fin >> keyword >> lowerLeftX >> lowerLeftY >> upperRightX >> upperRightY;
    if (keyword != "DieSize") 
        error_message(1, "DieSize", keyword);
    setDieSize(lowerLeftX, lowerLeftY, upperRightX, upperRightY);

    // read in the max util rate for both top and bottom dies
    int util;
    fin >> keyword >> util;
    _topDieMaxUtil = util / 100.0;
    if (keyword != "TopDieMaxUtil") 
        error_message(1, "TopDieMaxUtil", keyword);
    fin >> keyword >> util;
    _bottomDieMaxUtil = util / 100.0;
    if (keyword != "BottomDieMaxUtil") 
        error_message(1, "BottomDieMaxUtil", keyword);

    // read in the rows information for both top and bottom dies
    int startX, startY, rowLength, rowHeight, repeatCount;
    fin >> keyword >> startX >> startY >> rowLength >> rowHeight >> repeatCount;
    if (keyword != "TopDieRows") 
        error_message(1, "TopDieRows", keyword);
    for (int i=0; i<repeatCount; i++) {
    	Row row(startX, startY + i * rowHeight, rowLength, rowHeight);
    	_topRow.push_back(row);
    }
    fin >> keyword >> startX >> startY >> rowLength >> rowHeight >> repeatCount;
    if (keyword != "BottomDieRows") 
        error_message(1, "BottomDieRows", keyword);
    for (int i=0; i<repeatCount; i++) {
    	Row row(startX, startY + i * rowHeight, rowLength, rowHeight);
    	_bottomRow.push_back(row);
    }

    // read in the technology mapping for each die
    string TechName;
    fin >> keyword >> TechName;
    if (keyword != "TopDieTech") 
        error_message(1, "TopDieTech", keyword);
    if (_technologies[0] == TechName)
        _topDieTech = 0;
    else _topDieTech = 1;
    fin >> keyword >> TechName;
    if (keyword != "BottomDieTech") 
        error_message(1, "BottomDieTech", keyword);
    if (_technologies[0] == TechName)
        _bottomDieTech = 0;
    else _bottomDieTech = 1;

    // read in the terminal informations
    int sizeX, sizeY, spacing;
    fin >> keyword >> sizeX >> sizeY;
    if (keyword != "TerminalSize") 
        error_message(1, "TerminalSize", keyword);
    _terminalSizeX = sizeX;
    _terminalSizeY = sizeY;
    fin >> keyword >> spacing;
    if (keyword != "TerminalSpacing") 
        error_message(1, "TerminalSpacing", keyword);
    _terminalSpacing = spacing;

    // read in the instance information, and build with class "Inst"
    int instanceCount;
    fin >> keyword >> instanceCount;
    if (keyword != "NumInstances") 
        error_message(1, "NumInstances", keyword);
    for (int i=0; i<instanceCount; i++) {
        string instName, libCellName;
        fin >> keyword >> instName >> libCellName;
        if (keyword != "Inst") 
            error_message(1, "Inst", keyword);
        Inst inst(instName);
        inst._libCellId = _libcellName2ID[libCellName];
        inst.initialize(0, 0, 0, 0, 0); // initialize the position and layer
        inst.cloneFromlibcell(_libcell[_libcellName2ID[libCellName]], _topDieTech, _bottomDieTech); // consider the techonology
        _instName2ID[instName] = _insts.size();
        _insts.push_back(inst);
    }

    // set _instId for each Pin
    for (int i=0; i<numInsts(); i++) {
        for (int j=0; j<_insts[i].numPins(); j++) {
            _insts[i]._Pins0[j]._instId = i;
            _insts[i]._Pins1[j]._instId = i;
        }
    }
    
    // read in the net information, and build with class "Net"
    int netCount;
    fin >> keyword >> netCount;
    if (keyword != "NumNets") 
        error_message(1, "NumNets", keyword);
    for (int i=0; i<netCount; i++) {
        string netName;
        int numPins;
        fin >> keyword >> netName >> numPins;
        if (keyword != "Net") 
            error_message(1, "Net", keyword);
        Net net(netName);
        for (int j=0; j<numPins; j++) {
            string instName_libPinName, instName, libPinName;
            fin >> keyword >> instName_libPinName;
            if (keyword != "Pin")
                error_message(1, "Pin", keyword);
            string delim = "/";
            instName = instName_libPinName.substr(0, instName_libPinName.find(delim));
            libPinName = instName_libPinName.substr(instName_libPinName.find(delim)+1, instName_libPinName.size());
            // connect the information between Net, Pin and Inst
            // Pin -> Net
            int libcellId = _insts[_instName2ID[instName]]._libCellId;
            int pinId = _libcell[libcellId]._pinName2Id[libPinName];
            _insts[_instName2ID[instName]]._Pins0[pinId]._netId = _nets.size();
            _insts[_instName2ID[instName]]._Pins1[pinId]._netId = _nets.size();
            // Net -> Pin
            net._Pins0.push_back(_insts[_instName2ID[instName]]._Pins0[pinId]);
            net._Pins1.push_back(_insts[_instName2ID[instName]]._Pins1[pinId]);
            // Net -> Inst
            net._insts.push_back(_instName2ID[instName]);
            // Inst -> Net
            _insts[_instName2ID[instName]]._nets.push_back(_nets.size());
        }
        _netName2ID[netName] = _nets.size();
        _nets.push_back(net);
    }
    fin.close();
    cout << argv[1] << " is read ..." << endl;
}

void Placement::outputFile(char *argv[])
{
    ofstream fout;
    fout.open(argv[2], ios::out);
    // seperate Inst on top die with Inst on bottom die
    vector<string> topName;
    vector<string> bottomName;
    vector<int> topX;
    vector<int> topY;
    vector<int> bottomX;
    vector<int> bottomY;
	for (int i=0; i<_insts.size(); i++) {
		if (_insts[i]._techId == 0) {
			topName.push_back(_insts[i]._name);
			topX.push_back(_insts[i].x());
			topY.push_back(_insts[i].y());
		}
		else {
			bottomName.push_back(_insts[i]._name);
			bottomX.push_back(_insts[i].x());
			bottomY.push_back(_insts[i].y());
		}
	}    
    // output TopDiePlacement
    fout << "TopDiePlacement " << topName.size() << endl;
    for (int i=0; i<topName.size(); i++) {
    	fout << "Inst " << topName[i] << " " << topX[i] << " " << topY[i] << endl;
    }
    // output BottomDiePlacement
    fout << "\nBottomDiePlacement " << bottomName.size() << endl;
    for (int i=0; i<bottomName.size(); i++) {
    	fout << "Inst " << bottomName[i] << " " << bottomX[i] << " " << bottomY[i] << endl;
    }
    // output Terminal
    fout << "\nNumTerminals " << _terminals.size() << endl;
    for (int i=0; i<_terminals.size(); i++) {
    	fout << "Terminal " << _terminals[i]._Netname << " " << _terminals[i]._locationX << " " << _terminals[i]._locationY << endl;
    }
    fout.close();
    cout << argv[2] << " is saved ..." << endl;
}

// calculate the exact HPWL (may have higher time complexity)
long long Placement::computeHpwl()
{
    long long total_wl = 0;
    for (int i=0; i<_nets.size(); i++) {
        int top_max_x = INT_MIN;
        int top_max_y = INT_MIN;
        int top_min_x = INT_MAX;
        int top_min_y = INT_MAX;
        int bot_max_x = INT_MIN;
        int bot_max_y = INT_MIN;
        int bot_min_x = INT_MAX;
        int bot_min_y = INT_MAX;
        for (int j=0; j<_nets[i]._insts.size(); j++) {
            int pinX, pinY;
            if (_insts[_nets[i]._insts[j]]._techId == 0) {
                for (int k=0; k<_insts[_nets[i]._insts[j]]._Pins0.size(); k++) {
                    if (_insts[_nets[i]._insts[j]]._Pins0[k]._netId == i) {
                        pinX = _insts[_nets[i]._insts[j]].x() + _insts[_nets[i]._insts[j]]._Pins0[k]._xOffset;
                        pinY = _insts[_nets[i]._insts[j]].y() + _insts[_nets[i]._insts[j]]._Pins0[k]._yOffset;
                        break;
                    }
                }
            }
            else {
                for (int k=0; k<_insts[_nets[i]._insts[j]]._Pins1.size(); k++) {
                    if (_insts[_nets[i]._insts[j]]._Pins1[k]._netId == i) {
                        pinX = _insts[_nets[i]._insts[j]].x() + _insts[_nets[i]._insts[j]]._Pins1[k]._xOffset;
                        pinY = _insts[_nets[i]._insts[j]].y() + _insts[_nets[i]._insts[j]]._Pins1[k]._yOffset;
                        break;
                    }
                }
            }
            if ((_insts[_nets[i]._insts[j]]._techId == 0) && (pinX > top_max_x)) top_max_x = pinX;
            if ((_insts[_nets[i]._insts[j]]._techId == 1) && (pinX > bot_max_x)) bot_max_x = pinX;
            if ((_insts[_nets[i]._insts[j]]._techId == 0) && (pinY > top_max_y)) top_max_y = pinY;
            if ((_insts[_nets[i]._insts[j]]._techId == 1) && (pinY > bot_max_y)) bot_max_y = pinY;
            if ((_insts[_nets[i]._insts[j]]._techId == 0) && (pinX < top_min_x)) top_min_x = pinX;
            if ((_insts[_nets[i]._insts[j]]._techId == 1) && (pinX < bot_min_x)) bot_min_x = pinX;
            if ((_insts[_nets[i]._insts[j]]._techId == 0) && (pinY < top_min_y)) top_min_y = pinY;
            if ((_insts[_nets[i]._insts[j]]._techId == 1) && (pinY < bot_min_y)) bot_min_y = pinY;
        }
        // terminal (use the center of terminal)
        if (_nets[i]._needTerminal) {
            int locaX = _nets[i]._terminal._locationX + _terminalSizeX / 2;
            int locaY = _nets[i]._terminal._locationY + _terminalSizeY / 2;
            if (locaX > top_max_x) top_max_x = locaX;
            if (locaX > bot_max_x) bot_max_x = locaX;
            if (locaY > top_max_y) top_max_y = locaY;
            if (locaY > bot_max_y) bot_max_y = locaY;
            if (locaX < top_min_x) top_min_x = locaX;
            if (locaX < bot_min_x) bot_min_x = locaX;
            if (locaY < top_min_y) top_min_y = locaY;
            if (locaY < bot_min_y) bot_min_y = locaY;
        }
        total_wl += (top_max_x - top_min_x) + (top_max_y - top_min_y) + (bot_max_x - bot_min_x) + (bot_max_y - bot_min_y);
    }
    return total_wl;
}

// calculate the approximate HPWL (use Inst center only, not consider Pin offset)
long long Placement::computeApproxHpwl()
{
    long long total_wl = 0;
    for (int i=0; i<_nets.size(); i++) {
        int top_max_x = INT_MIN;
        int top_max_y = INT_MIN;
        int top_min_x = INT_MAX;
        int top_min_y = INT_MAX;
        int bot_max_x = INT_MIN;
        int bot_max_y = INT_MIN;
        int bot_min_x = INT_MAX;
        int bot_min_y = INT_MAX;
        for (int j=0; j<_nets[i]._insts.size(); j++) {
            int centerX = _insts[_nets[i]._insts[j]].centerX();
            int centerY = _insts[_nets[i]._insts[j]].centerY();
            if ((_insts[_nets[i]._insts[j]]._techId == 0) && (centerX > top_max_x)) top_max_x = centerX;
            if ((_insts[_nets[i]._insts[j]]._techId == 1) && (centerX > bot_max_x)) bot_max_x = centerX;
            if ((_insts[_nets[i]._insts[j]]._techId == 0) && (centerY > top_max_y)) top_max_y = centerY;
            if ((_insts[_nets[i]._insts[j]]._techId == 1) && (centerY > bot_max_y)) bot_max_y = centerY;
            if ((_insts[_nets[i]._insts[j]]._techId == 0) && (centerX < top_min_x)) top_min_x = centerX;
            if ((_insts[_nets[i]._insts[j]]._techId == 1) && (centerX < bot_min_x)) bot_min_x = centerX;
            if ((_insts[_nets[i]._insts[j]]._techId == 0) && (centerY < top_min_y)) top_min_y = centerY;
            if ((_insts[_nets[i]._insts[j]]._techId == 1) && (centerY < bot_min_y)) bot_min_y = centerY;
        }
        // terminal (just use the left bottom corner of terminal here)
        if (_nets[i]._needTerminal) {
            int locaX = _nets[i]._terminal._locationX;
            int locaY = _nets[i]._terminal._locationY;
            if (locaX > top_max_x) top_max_x = locaX;
            if (locaX > bot_max_x) bot_max_x = locaX;
            if (locaY > top_max_y) top_max_y = locaY;
            if (locaY > bot_max_y) bot_max_y = locaY;
            if (locaX < top_min_x) top_min_x = locaX;
            if (locaX < bot_min_x) bot_min_x = locaX;
            if (locaY < top_min_y) top_min_y = locaY;
            if (locaY < bot_min_y) bot_min_y = locaY;
        }
        total_wl += (top_max_x - top_min_x) + (top_max_y - top_min_y) + (bot_max_x - bot_min_x) + (bot_max_y - bot_min_y);
    }
    return total_wl;
}

void Placement::checkDataLegal(Placement dontTouchplace)
{
    // TODO:
    // check whether Net, Pin, Inst informations are legal now
    // compare to the original condition (dontTouchplace)
}

void Placement::checkforResultCorrectness()
{
    // TODO:
    // 1. Max placement utilization constraint must be satisfied
    // 2. All the given instances must be placed on either top die or bottom die
    // 3. All the instances must be on row without overlap
    // 4. Hybrid bonding terminal spacing constraint must be satisfied
    // 5. Crossing-die nets must have 1 and only 1 hybrid bonding terminal
}

void Placement::DiePartition()
{
    // consider the die technology and split by random
    double topDieFactor = _topDieMaxUtil / (_topRow[0]._rowHeight * _topRow[0]._rowHeight);
    double bottomDieFactor = _bottomDieMaxUtil / (_bottomRow[0]._rowHeight * _bottomRow[0]._rowHeight);
    double ratio = 10000000.0 * topDieFactor / (topDieFactor + bottomDieFactor);
    double topDieUtil = 0;
    double bottomDieUtil = 0;
    for (int i=0; i<numInsts(); i++) {
        int randnum = rand() % 10000000;
        int randDieId = (randnum < ratio) ? 0 : 1;
        _insts[i].setTechId(randDieId); // random set the techId for each Inst
        if (randDieId == 0) {
            if ((topDieUtil + _insts[i].area()) < (_topDieMaxUtil*(_boundryRight-_boundryLeft)*(_boundryTop-_boundryBottom))) {
                topDieUtil += _insts[i].area();
            }
            else {
                _insts[i].setTechId(1);
                _insts[i]._x[1] = _insts[i]._x[0];
                _insts[i]._y[1] = _insts[i]._y[0];
                bottomDieUtil += _insts[i].area();
            }
        }
        else {
            _insts[i].setTechId(1);
            _insts[i]._x[1] = _insts[i]._x[0];
            _insts[i]._y[1] = _insts[i]._y[0];
            bottomDieUtil += _insts[i].area();
        }
    }
}

void Placement::randomPlace()
{
    for (int i=0; i<numInsts(); i++) {
        // int randDieId = rand() % 2;
        // _insts[i].setTechId(randDieId); // random set the techId for each Inst
        int x = _boundryLeft + (rand() % int(_boundryRight-_boundryLeft));
        int y = _boundryBottom + (rand() % int(_boundryTop-_boundryBottom));
        if ((x - _insts[i].width() / 2 < _boundryLeft) || (y - _insts[i].height() / 2 < _boundryBottom)) {
            i--;
            continue;
        }
        _insts[i].setCenterPosition(x, y);
    }
}

void Placement::runRandomPartition()
{
    vector<int> vec;
    vector<int> net_cnt_1(numNets(), 0);
    vector<int> net_cnt_2(numNets(), 0);
    for (int i=0; i<numInsts(); i++) vec.push_back(i);
    randomPartition(0, numInsts()-1, vec, _boundryLeft, _boundryBottom, _boundryRight, _boundryTop, net_cnt_1, net_cnt_2);
}

void Placement::randomPartition(int start, int end, vector<int> &mod, int left, int bottom, int right, int top, vector<int> &net_cnt_1, vector<int> &net_cnt_2)
{
    // set the position if there is only one module
    if (start == end) { 
        // int randDieId = rand() % 2;
        // _insts[mod[start]].setTechId(randDieId); // random set the techId for each Inst
        _insts[mod[start]].setCenterPosition((left+right)/2, (bottom+top)/2);
        return;
    }
    // only partition into n_grid (random generate) 
    // int n_grid = 16;
    // if ((end-start)<mod.size()/n_grid) {
    //     for (int i=start; i<=end; i++) {
    //         int x = left + (rand() % int(right-left));
    //         int y = bottom + (rand() % int(top-bottom));
    //         _insts[mod[i]].setCenterPosition(x, y);
    //     }
    //     return;
    // }

    int left1, left2, bottom1, bottom2, right1, right2, top1, top2;
    if (right - left > top - bottom) {
        left1 = left;
        left2 = (left + right) / 2;
        right1 = (left + right) / 2;
        right2 = right;
        bottom1 = bottom;
        bottom2 = bottom;
        top1 = top;
        top2 = top;
    }
    else {
        left1 = left;
        left2 = left;
        right1 = right;
        right2 = right;
        bottom1 = bottom;
        bottom2 = (bottom + top) / 2;
        top1 = (bottom + top) / 2;
        top2 = top;
    }

    int middle = (start + end) / 2;

    // do the partition similar to KL partition
    for (int i=start; i<=middle; i++) {
        for (int j=0; j<_insts[mod[i]].numPins(); j++) {
            net_cnt_1[_insts[mod[i]]._Pins0[j]._netId]++;
        }
    }
    for (int i=middle+1; i<=end; i++) {
        for (int j=0; j<_insts[mod[i]].numPins(); j++) {
            net_cnt_2[_insts[mod[i]]._Pins0[j]._netId]++;
        }
    }

    int cnt = 0; // count for termination
    int cnt_not_update = 0;
    int cnt_0 = 0;
    int cnt_good = 0;
    int total_cut_size = 0;
    int cnt_limit = 1000;

    while (cnt < cnt_limit) {
        int rand1 = start + int(rand() % (middle-start+1));
        int rand2 = middle+1 + int(rand() % (end-(middle+1)+1));

        for (int i=0; i<_insts[mod[rand1]].numPins(); i++) {
            net_cnt_1[_insts[mod[rand1]]._Pins0[i]._netId]--;
            net_cnt_2[_insts[mod[rand1]]._Pins0[i]._netId]++;
        }
        int gain1 = 0;
        for (int j=0; j<_insts[mod[rand1]].numPins(); j++) {
            if (net_cnt_1[_insts[mod[rand1]]._Pins0[j]._netId] == 0) gain1++;
            if (net_cnt_2[_insts[mod[rand1]]._Pins0[j]._netId] == 1) gain1--;
        }
        for (int i=0; i<_insts[mod[rand2]].numPins(); i++) {
            net_cnt_2[_insts[mod[rand2]]._Pins0[i]._netId]--;
            net_cnt_1[_insts[mod[rand2]]._Pins0[i]._netId]++;
        }
        int gain2 = 0;
        for (int j=0; j<_insts[mod[rand2]].numPins(); j++) {
            if (net_cnt_2[_insts[mod[rand2]]._Pins0[j]._netId] == 0) gain2++;
            if (net_cnt_1[_insts[mod[rand2]]._Pins0[j]._netId] == 1) gain2--;
        }
        if (gain1 + gain2 <= 0) cnt_not_update++;
        else { cnt_not_update = 0; }

        int not_updata_limit = (end - start) / 1;
        if (cnt_not_update > not_updata_limit) {
            for (int i=0; i<_insts[mod[rand1]].numPins(); i++) {
                net_cnt_2[_insts[mod[rand1]]._Pins0[i]._netId]--;
                net_cnt_1[_insts[mod[rand1]]._Pins0[i]._netId]++;
            }
            for (int i=0; i<_insts[mod[rand2]].numPins(); i++) {
                net_cnt_1[_insts[mod[rand2]]._Pins0[i]._netId]--;
                net_cnt_2[_insts[mod[rand2]]._Pins0[i]._netId]++;
            }
            break;
        }
        if (gain1 + gain2 < 0) {
            cnt++;
            for (int i=0; i<_insts[mod[rand1]].numPins(); i++) {
                net_cnt_2[_insts[mod[rand1]]._Pins0[i]._netId]--;
                net_cnt_1[_insts[mod[rand1]]._Pins0[i]._netId]++;
            }
            for (int i=0; i<_insts[mod[rand2]].numPins(); i++) {
                net_cnt_1[_insts[mod[rand2]]._Pins0[i]._netId]--;
                net_cnt_2[_insts[mod[rand2]]._Pins0[i]._netId]++;
            }
            continue;
        }

        // change the module vector
        swap(mod[rand1], mod[rand2]);
        cnt++;
    }
    
    // for debug
    for (int i=start; i<=middle; i++) {
        for (int j=0; j<_insts[mod[i]].numPins(); j++) {
            net_cnt_1[_insts[mod[i]]._Pins0[j]._netId]--;
        }
    }
    for (int i=middle+1; i<=end; i++) {
        for (int j=0; j<_insts[mod[i]].numPins(); j++) {
            net_cnt_2[_insts[mod[i]]._Pins0[j]._netId]--;
        }
    }

    // reursive call
    randomPartition(start, middle, mod, left1, bottom1, right1, top1, net_cnt_1, net_cnt_2);
    randomPartition(middle+1, end, mod, left2, bottom2, right2, top2, net_cnt_1, net_cnt_2);
}

void Placement::random_SA(int swap_time, double T, double T_rate) // Similar to TimberWolf
{
    for (int i=0; i<swap_time; i++) {
        int r = rand() % 2;
        if (r == 0) { // Swap two cells
            int rand0 = rand() % numInsts();
            int rand1 = rand() % numInsts();
            int wire_length = estimate_two_inst_WL(rand0, rand1);
            int temp_x = _insts[rand0].x();
            int temp_y = _insts[rand0].y();
            _insts[rand0].setPosition(_insts[rand1].x(), _insts[rand1].y());
            _insts[rand1].setPosition(temp_x, temp_y);
            // estimate wirelength difference
            int new_wire_length = estimate_two_inst_WL(rand0, rand1);
            if ((new_wire_length > wire_length) && (exp(-(new_wire_length-wire_length) / T) < (rand() / double(RAND_MAX)) )) {
                int temp_x = _insts[rand0].x();
                int temp_y = _insts[rand0].y();
                _insts[rand0].setPosition(_insts[rand1].x(), _insts[rand1].y());
                _insts[rand1].setPosition(temp_x, temp_y);
            }
        }
        else { // Move a cell to a new location
            int rand0 = rand() % numInsts();
            int wire_length = estimate_one_inst_WL(rand0);
            int temp_x = _insts[rand0].x();
            int temp_y = _insts[rand0].y();
            int rand_x = _boundryLeft + rand() % (_boundryRight - _boundryLeft);
            int rand_y = _boundryBottom + rand() % (_boundryTop - _boundryBottom);
            _insts[rand0].setPosition(rand_x, rand_y);
            // estimate wirelength difference
            int new_wire_length = estimate_one_inst_WL(rand0);
            if ((new_wire_length > wire_length) && (exp(-(new_wire_length-wire_length) / T) < (rand() / double(RAND_MAX)) )) {
                _insts[rand0].setPosition(temp_x, temp_y);
            }
        }
        T *= T_rate;
        // if (T < 1) break;
    }
}

int Placement::estimate_one_inst_WL(int rand0)
{
    int total_wire_length = 0;
    for (int j=0; j<_insts[rand0]._nets.size(); j++) {
        int netId = _insts[rand0]._nets[j];
        int max_x = INT_MIN;
        int min_x = INT_MAX;
        int max_y = INT_MIN;
        int min_y = INT_MAX;
        for (int k=0; k<_nets[netId].numPins(); k++) {
            double pin_x = _insts[_nets[netId]._Pins0[k]._instId].x() + _nets[netId]._Pins0[k]._xOffset;
            double pin_y = _insts[_nets[netId]._Pins0[k]._instId].y() + _nets[netId]._Pins0[k]._yOffset;
            if (pin_x > max_x) max_x = pin_x;
            if (pin_x < min_x) min_x = pin_x;
            if (pin_y > max_y) max_y = pin_y;
            if (pin_y < min_y) min_y = pin_y;
        }
        total_wire_length += (max_x - min_x) + (max_y - min_y);
    }
    return total_wire_length;
}

int Placement::estimate_two_inst_WL(int rand0, int rand1)
{
    int total_wire_length = 0;
    for (int j=0; j<_insts[rand0]._nets.size(); j++) {
        int netId = _insts[rand0]._nets[j];
        int max_x = INT_MIN;
        int min_x = INT_MAX;
        int max_y = INT_MIN;
        int min_y = INT_MAX;
        for (int k=0; k<_nets[netId].numPins(); k++) {
            double pin_x = _insts[_nets[netId]._Pins0[k]._instId].x() + _nets[netId]._Pins0[k]._xOffset;
            double pin_y = _insts[_nets[netId]._Pins0[k]._instId].y() + _nets[netId]._Pins0[k]._yOffset;
            if (pin_x > max_x) max_x = pin_x;
            if (pin_x < min_x) min_x = pin_x;
            if (pin_y > max_y) max_y = pin_y;
            if (pin_y < min_y) min_y = pin_y;
        }
        total_wire_length += (max_x - min_x) + (max_y - min_y);
    }
    for (int j=0; j<_insts[rand1]._nets.size(); j++) {
        int netId = _insts[rand1]._nets[j];
        int max_x = INT_MIN;
        int min_x = INT_MAX;
        int max_y = INT_MIN;
        int min_y = INT_MAX;
        for (int k=0; k<_nets[netId].numPins(); k++) {
            double pin_x = _insts[_nets[netId]._Pins0[k]._instId].x() + _nets[netId]._Pins0[k]._xOffset;
            double pin_y = _insts[_nets[netId]._Pins0[k]._instId].y() + _nets[netId]._Pins0[k]._yOffset;
            if (pin_x > max_x) max_x = pin_x;
            if (pin_x < min_x) min_x = pin_x;
            if (pin_y > max_y) max_y = pin_y;
            if (pin_y < min_y) min_y = pin_y;
        }
        total_wire_length += (max_x - min_x) + (max_y - min_y);
    }
    return total_wire_length;
}

void Placement::bruteForcePlacement() {
    
}

void Placement::boundingTerminalInitial()
{
    for (int i=0; i<numNets(); i++) {
        bool hasTop = false;
        bool hasBottom = false;
        for (int j=0; j<_nets[i]._insts.size(); j++) {
            if (_insts[_nets[i]._insts[j]]._techId == 0) {
                hasTop = true;
            }
            else hasBottom = true;
        }
        if (hasTop && hasBottom) {
            _nets[i]._needTerminal = true;
            _nets[i].setTerminal(i, 0, 0); // initial terminal position to (0, 0)
            _terminals.push_back(_nets[i]._terminal);
        }
    }
}

// for bounding terminal (trans (grid_x, grid_y) to (x, y))
// return false if the position is illegal (over the boundary)
bool Placement::Grid2XY(int grid_x, int grid_y, int &x, int &y)
{
    x = grid_x * (_terminalSizeX + _terminalSpacing) + (_terminalSizeX / 2) + _terminalSpacing;
    y = grid_y * (_terminalSizeY + _terminalSpacing) + (_terminalSizeY / 2) + _terminalSpacing;
    if (x + (_terminalSizeX / 2) + _terminalSpacing > _boundryRight) return false;
    if (y + (_terminalSizeY / 2) + _terminalSpacing > _boundryTop) return false;
    if ((grid_x < 0 ) || (grid_y < 0)) return false;
    return true;
}

// for bounding terminal (trans (x, y) to (grid_x, grid_y))
// return false if the position is illegal (over the boundary)
bool Placement::XY2Grid(int &grid_x, int &grid_y, int x, int y)
{
    if (x + (_terminalSizeX / 2) + _terminalSpacing > _boundryRight) return false;
    if (y + (_terminalSizeY / 2) + _terminalSpacing > _boundryTop) return false;
    if (x - (_terminalSizeX / 2) - _terminalSpacing < _boundryLeft) return false;
    if (y - (_terminalSizeY / 2) - _terminalSpacing < _boundryBottom) return false;
    grid_x = (x - (_terminalSizeX / 2) - _terminalSpacing) / (_terminalSizeX + _terminalSpacing);
    grid_y = (y - (_terminalSizeY / 2) - _terminalSpacing) / (_terminalSizeY + _terminalSpacing);
    return true;
}

void Placement::boundingTerminalPlacement()
{
    // sort the net according to their current (top_max_x - top_min_x) * (top_max_y - top_min_y) + (bot_max_x - bot_min_x) * (bot_max_y - bot_min_y)
    vector< pair<long long, int> > netPriority;
    vector< pair<long long, int> > failnetPriority;
    for (int i=0; i<_terminals.size(); i++) {
        int top_max_x = INT_MIN;
        int top_max_y = INT_MIN;
        int top_min_x = INT_MAX;
        int top_min_y = INT_MAX;
        int bot_max_x = INT_MIN;
        int bot_max_y = INT_MIN;
        int bot_min_x = INT_MAX;
        int bot_min_y = INT_MAX;
        for (int j=0; j<_nets[_terminals[i]._NetId]._insts.size(); j++) {
            int pinX, pinY;
            if (_insts[_nets[_terminals[i]._NetId]._insts[j]]._techId == 0) {
                for (int k=0; k<_insts[_nets[_terminals[i]._NetId]._insts[j]]._Pins0.size(); k++) {
                    if (_insts[_nets[_terminals[i]._NetId]._insts[j]]._Pins0[k]._netId == _terminals[i]._NetId) {
                        pinX = _insts[_nets[_terminals[i]._NetId]._insts[j]].x() + _insts[_nets[_terminals[i]._NetId]._insts[j]]._Pins0[k]._xOffset;
                        pinY = _insts[_nets[_terminals[i]._NetId]._insts[j]].y() + _insts[_nets[_terminals[i]._NetId]._insts[j]]._Pins0[k]._yOffset;
                        break;
                    }
                }
            }
            else {
                for (int k=0; k<_insts[_nets[_terminals[i]._NetId]._insts[j]]._Pins1.size(); k++) {
                    if (_insts[_nets[_terminals[i]._NetId]._insts[j]]._Pins1[k]._netId == _terminals[i]._NetId) {
                        pinX = _insts[_nets[_terminals[i]._NetId]._insts[j]].x() + _insts[_nets[_terminals[i]._NetId]._insts[j]]._Pins1[k]._xOffset;
                        pinY = _insts[_nets[_terminals[i]._NetId]._insts[j]].y() + _insts[_nets[_terminals[i]._NetId]._insts[j]]._Pins1[k]._yOffset;
                        break;
                    }
                }
            }
            if ((_insts[_nets[_terminals[i]._NetId]._insts[j]]._techId == 0) && (pinX > top_max_x)) top_max_x = pinX;
            if ((_insts[_nets[_terminals[i]._NetId]._insts[j]]._techId == 1) && (pinX > bot_max_x)) bot_max_x = pinX;
            if ((_insts[_nets[_terminals[i]._NetId]._insts[j]]._techId == 0) && (pinY > top_max_y)) top_max_y = pinY;
            if ((_insts[_nets[_terminals[i]._NetId]._insts[j]]._techId == 1) && (pinY > bot_max_y)) bot_max_y = pinY;
            if ((_insts[_nets[_terminals[i]._NetId]._insts[j]]._techId == 0) && (pinX < top_min_x)) top_min_x = pinX;
            if ((_insts[_nets[_terminals[i]._NetId]._insts[j]]._techId == 1) && (pinX < bot_min_x)) bot_min_x = pinX;
            if ((_insts[_nets[_terminals[i]._NetId]._insts[j]]._techId == 0) && (pinY < top_min_y)) top_min_y = pinY;
            if ((_insts[_nets[_terminals[i]._NetId]._insts[j]]._techId == 1) && (pinY < bot_min_y)) bot_min_y = pinY;
        }
        netPriority.push_back(make_pair((top_max_x - top_min_x) * (top_max_y - top_min_y) + (bot_max_x - bot_min_x) * (bot_max_y - bot_min_y), i));
    }
    sort(netPriority.begin(), netPriority.end());
    // build a table to remember which grid point is occupied
    int num_grid_x = ((_boundryRight - _boundryLeft - (2 * _terminalSpacing) - _terminalSizeX) / (_terminalSizeX + _terminalSpacing)) + 1;
    int num_grid_y = ((_boundryTop - _boundryBottom - (2 * _terminalSpacing) - _terminalSizeY) / (_terminalSizeY + _terminalSpacing)) + 1;
    bool occupy[num_grid_x][num_grid_y] = {false};
    // try to build the terminal from the smallest net to the largest net
    int ngadd = 0;
    int rdadd = 0;
    for (int i=0; i<netPriority.size(); i++) {
        int top_max_x = INT_MIN;
        int top_max_y = INT_MIN;
        int top_min_x = INT_MAX;
        int top_min_y = INT_MAX;
        int bot_max_x = INT_MIN;
        int bot_max_y = INT_MIN;
        int bot_min_x = INT_MAX;
        int bot_min_y = INT_MAX;
        for (int j=0; j<_nets[_terminals[netPriority[i].second]._NetId]._insts.size(); j++) {
            int pinX, pinY;
            if (_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._techId == 0) {
                for (int k=0; k<_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._Pins0.size(); k++) {
                    if (_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._Pins0[k]._netId == _terminals[netPriority[i].second]._NetId) {
                        pinX = _insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]].x() + _insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._Pins0[k]._xOffset;
                        pinY = _insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]].y() + _insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._Pins0[k]._yOffset;
                        break;
                    }
                }
            }
            else {
                for (int k=0; k<_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._Pins1.size(); k++) {
                    if (_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._Pins1[k]._netId == _terminals[netPriority[i].second]._NetId) {
                        pinX = _insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]].x() + _insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._Pins1[k]._xOffset;
                        pinY = _insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]].y() + _insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._Pins1[k]._yOffset;
                        break;
                    }
                }
            }
            if ((_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._techId == 0) && (pinX > top_max_x)) top_max_x = pinX;
            if ((_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._techId == 1) && (pinX > bot_max_x)) bot_max_x = pinX;
            if ((_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._techId == 0) && (pinY > top_max_y)) top_max_y = pinY;
            if ((_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._techId == 1) && (pinY > bot_max_y)) bot_max_y = pinY;
            if ((_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._techId == 0) && (pinX < top_min_x)) top_min_x = pinX;
            if ((_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._techId == 1) && (pinX < bot_min_x)) bot_min_x = pinX;
            if ((_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._techId == 0) && (pinY < top_min_y)) top_min_y = pinY;
            if ((_insts[_nets[_terminals[netPriority[i].second]._NetId]._insts[j]]._techId == 1) && (pinY < bot_min_y)) bot_min_y = pinY;
        }
        // find the tightest bound for both top and bottom die
        int x_begin = max(top_min_x, bot_min_x);
        int x_end = min(top_max_x, bot_max_x);
        int y_begin = max(top_min_y, bot_min_y);
        int y_end = min(top_max_y, bot_max_y);
        // still space to place without any wirelength gain
        // cerr << "\nx = (" << x_begin << ", " << x_end << ")\n";
        // cerr << "y = (" << y_begin << ", " << y_end << ")\n";
        bool generate_success = false;
        for (int m=x_begin; m<x_end; m++) {
            for (int n=y_begin; n<y_end; n++) {
                int grid_x, grid_y;
                if (XY2Grid(grid_x, grid_y, m, n)) {
                    if (!occupy[grid_x][grid_y]) {
                        int x, y;
                        if (Grid2XY(grid_x, grid_y, x, y)) {
                            _terminals[netPriority[i].second]._locationX = x;
                            _terminals[netPriority[i].second]._locationY = y;
                            _nets[_terminals[netPriority[i].second]._NetId]._terminal._locationX = x;
                            _nets[_terminals[netPriority[i].second]._NetId]._terminal._locationY = y;
                            occupy[grid_x][grid_y] = true;
                            generate_success = true;
                            // cerr<<"No Gain Add   "<<i<<" / "<<netPriority.size()<<endl;
                            ngadd++;
                            break;
                        }
                    }
                }
            }
            if (generate_success) break;
        }
        if (!generate_success) failnetPriority.push_back(netPriority[i]);
    }
    // Random place for the fail nets
    vector< pair<int, int> > vec;
    for (int m=0; m<num_grid_x; m++) {
        for (int n=0; n<num_grid_y; n++) {
            if (!occupy[m][n]) {
                vec.push_back(make_pair(m, n));
            }
        }
    }
    for (int i=0; i<failnetPriority.size(); i++) {
        int top_max_x = INT_MIN;
        int top_max_y = INT_MIN;
        int top_min_x = INT_MAX;
        int top_min_y = INT_MAX;
        int bot_max_x = INT_MIN;
        int bot_max_y = INT_MIN;
        int bot_min_x = INT_MAX;
        int bot_min_y = INT_MAX;
        for (int j=0; j<_nets[_terminals[failnetPriority[i].second]._NetId]._insts.size(); j++) {
            int pinX, pinY;
            if (_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._techId == 0) {
                for (int k=0; k<_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._Pins0.size(); k++) {
                    if (_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._Pins0[k]._netId == _terminals[failnetPriority[i].second]._NetId) {
                        pinX = _insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]].x() + _insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._Pins0[k]._xOffset;
                        pinY = _insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]].y() + _insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._Pins0[k]._yOffset;
                        break;
                    }
                }
            }
            else {
                for (int k=0; k<_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._Pins1.size(); k++) {
                    if (_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._Pins1[k]._netId == _terminals[failnetPriority[i].second]._NetId) {
                        pinX = _insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]].x() + _insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._Pins1[k]._xOffset;
                        pinY = _insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]].y() + _insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._Pins1[k]._yOffset;
                        break;
                    }
                }
            }
            if ((_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._techId == 0) && (pinX > top_max_x)) top_max_x = pinX;
            if ((_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._techId == 1) && (pinX > bot_max_x)) bot_max_x = pinX;
            if ((_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._techId == 0) && (pinY > top_max_y)) top_max_y = pinY;
            if ((_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._techId == 1) && (pinY > bot_max_y)) bot_max_y = pinY;
            if ((_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._techId == 0) && (pinX < top_min_x)) top_min_x = pinX;
            if ((_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._techId == 1) && (pinX < bot_min_x)) bot_min_x = pinX;
            if ((_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._techId == 0) && (pinY < top_min_y)) top_min_y = pinY;
            if ((_insts[_nets[_terminals[failnetPriority[i].second]._NetId]._insts[j]]._techId == 1) && (pinY < bot_min_y)) bot_min_y = pinY;
        }
        int cnt = 0;
        int min_cost = INT_MAX;
        int x_set = -10000;
        int y_set = -10000;
        int min_id = -1;
        int cnt_limit = 1000; // this number will effect the runtime and wirelength
        while ((cnt < cnt_limit) && (cnt < vec.size())) {
            int id = rand() % vec.size();
            int grid_x = vec[id].first;
            int grid_y = vec[id].second;
            int x, y;
            if (Grid2XY(grid_x, grid_y, x, y)) {
                int cost_x = max(x-top_max_x, 0) + max(x-bot_max_x, 0) + max(top_min_x-x, 0) + max(bot_min_x-x, 0);
                int cost_y = max(y-top_max_y, 0) + max(y-bot_max_y, 0) + max(top_min_y-y, 0) + max(bot_min_y-y, 0);
                int cost = cost_x + cost_y;
                if (cost <= min_cost) {
                    min_id = id;
                    min_cost = cost;
                    x_set = x;
                    y_set = y;
                }
                cnt++;
            }
        }
        int grid_x, grid_y;
        if (XY2Grid(grid_x, grid_y, x_set, y_set)) {
            vec[min_id] = vec[vec.size()-1];
            vec.pop_back();
            _terminals[failnetPriority[i].second]._locationX = x_set;
            _terminals[failnetPriority[i].second]._locationY = y_set;
            _nets[_terminals[failnetPriority[i].second]._NetId]._terminal._locationX = x_set;
            _nets[_terminals[failnetPriority[i].second]._NetId]._terminal._locationY = y_set;
            occupy[grid_x][grid_y] = true;
            rdadd++;
        }
        else {
            cerr << "Error: Too many Bounding Terminals, Fail to generate legal position !!!\n";
            cerr << "Fail Bounding Terminal count = " << failnetPriority.size() - i << endl;
            return;
        }
    }
    cerr << "Count No Gain Add = " << ngadd << " / " << netPriority.size() << endl;
    cerr << "Count Random Add = " << rdadd << " / " << netPriority.size() << endl;
}

void Placement::printNetlistInformation()
{
    // Net information
    map<int, int> m;
    for (int i=0; i<_nets.size(); i++) {
        if (m.count(_nets[i]._insts.size()) != 0) m[_nets[i]._insts.size()]++;
        else m[_nets[i]._insts.size()] = 1;
    }
    cout << "\n\n=================================================" << endl;
    cout << "================ Net Information ================" << endl;
    for (map<int, int>::iterator it = m.begin(); it != m.end(); it++) {
        cout << "# " << (*it).first << "-Pins Net = " << (*it).second << "\n";
    }
    cout << "=================================================" << endl;

}

void Placement::printInformation(bool detail)
{
    cout << "#################################################\n";
    cout << "Die size = (" << _boundryLeft << "," << _boundryBottom << ")-(" << _boundryRight << "," << _boundryTop << ")\n";
    cout << "# Inst   = " << _insts.size() << endl;
    cout << "# Net    = " << _nets.size() << endl;
    cout << "HPWL     = " << computeHpwl() << endl;
    if (detail) {
        // seperate Inst on top die with Inst on bottom die
        vector<string> topName, bottomName;
        vector<int> topX, topY, bottomX, bottomY;
        for (int i=0; i<_insts.size(); i++) {
            if (_insts[i]._techId == 0) {
                topName.push_back(_insts[i]._name);
                topX.push_back(_insts[i].x());
                topY.push_back(_insts[i].y());
            }
            else {
                bottomName.push_back(_insts[i]._name);
                bottomX.push_back(_insts[i].x());
                bottomY.push_back(_insts[i].y());
            }
        }    
        // output TopDiePlacement
        cout << "\nTopDiePlacement " << topName.size() << endl;
        for (int i=0; i<topName.size(); i++) {
            cout << "Inst " << topName[i] << " " << topX[i] << " " << topY[i] << endl;
        }
        // output BottomDiePlacement
        cout << "\nBottomDiePlacement " << bottomName.size() << endl;
        for (int i=0; i<bottomName.size(); i++) {
            cout << "Inst " << bottomName[i] << " " << bottomX[i] << " " << bottomY[i] << endl;
        }
        // output Terminal
        cout << "\nNumTerminals " << _terminals.size() << endl;
        for (int i=0; i<_terminals.size(); i++) {
            cout << "Terminal " << _terminals[i]._Netname << " " << _terminals[i]._locationX << " " << _terminals[i]._locationY << endl;
        }
    }
    cout << "#################################################\n\n";
}

void Placement::outputTopDiePlotFile()
{
    ofstream output;
    output.open("module.txt");
    output << _boundryLeft << ' ' << _boundryBottom << ' ' << _boundryRight << ' ' << _boundryTop << endl;
    for (int i = 0; i < _insts.size(); ++i) {
        Inst &inst = _insts[i];
        if (inst._techId == 0) 
            output << inst._name << ' ' << inst.x() << ' ' << inst.y() << ' ' << inst.x() + inst.width() << ' ' << inst.y() + inst.height() << endl;
    }
}

void Placement::outputBottomDiePlotFile()
{
    ofstream output;
    output.open("module.txt");
    output << _boundryLeft << ' ' << _boundryBottom << ' ' << _boundryRight << ' ' << _boundryTop << endl;
    for (int i = 0; i < _insts.size(); ++i) {
        Inst &inst = _insts[i];
        if (inst._techId == 1) 
            output << inst._name << ' ' << inst.x() << ' ' << inst.y() << ' ' << inst.x() + inst.width() << ' ' << inst.y() + inst.height() << endl;
    }
}

void Placement::outputTerminalPlotFile()
{
    ofstream output;
    output.open("module.txt");
    output << _boundryLeft << ' ' << _boundryBottom << ' ' << _boundryRight << ' ' << _boundryTop << endl;
    for (int i = 0; i < _terminals.size(); ++i) {
        Terminal &term = _terminals[i];
        output << term._Netname << ' ' << term._locationX << ' ' << term._locationY << ' ' << term._locationX + _terminalSizeX << ' ' << term._locationY + _terminalSizeY << endl;
    }
}

void Placement::SetModuleLocation( const int& id, int x, int y,bool bottomdie)
{
    assert( id < (int)_insts.size() );
    
    _insts[id].setPosition(x,y);
    _insts[id].setCenterPosition(x + _insts[id].width() * 0.5, y + _insts[id].height() * 0.5); 
    //m_modules[id].rotate = rotate;
    if (bottomdie){
        for( int i=0; i<(int)_insts[id]._Pins1.size(); i++ )
        {
            CalcPinLocation( _insts[id]._Pins1[i], _insts[id].x(), _insts[id].y() );
        }
    }
    else{
        for( int i=0; i<(int)_insts[id]._Pins0.size(); i++ )
        {
            CalcPinLocation( _insts[id]._Pins0[i], _insts[id].x(), _insts[id].y() );
        }
    }
}
void Placement::CalcPinLocation( Pin &pid, int cx, int cy ) 
    {
        pid.absX = cx + pid._xOffset;
        pid.absY = cy + pid._yOffset;   
        //if (cx + pid._xOffset<0 || cy + pid._yOffset<0) cout<<"dddd";
    }
long long Placement::calculateModuleHPWL(int mid,bool bottomdie)
{
    long long wl = 0;
    assert(_insts[mid]._techId == bottomdie);
    //cout<<_insts[mid]._name<<endl;
    int numPins = _insts[mid]._techId ? _insts[mid]._Pins1.size():_insts[mid]._Pins0.size();
    if (bottomdie){
        for(int k=0;k<numPins;k++){
            int netid = _insts[mid]._Pins1[k]._netId;
            //cout<<_nets[netid]._Pins1.size()<<"  "<<_nets[netid]._Pins1.size()<<"  "<<netid<<endl;
            if( netid == -1){
                //cout<<"WTF";
                continue;
            }
            if (_nets[netid]._needTerminal==0){
                int xx = _insts[mid]._Pins1[k].absX;
                int yy = _insts[mid]._Pins1[k].absY;
                if (xx <0) cout<<xx<<" "; //neg
                int maxX = xx;
                int minX = xx;
                int maxY = yy;
                int minY = yy;
                //cout<<_nets[netid]._Pins1.size()<<endl;
                for (int o = 1;o < _nets[netid]._Pins1.size();o++)
                {
                        
                        xx = _nets[netid]._Pins1[o].absX;
                        yy = _nets[netid]._Pins1[o].absY;
                        minX = min( minX, xx );
                        maxX = max( maxX, xx );
                        minY = min( minY, yy );
                        maxY = max( maxY, yy );
                }
                //cout<<maxX<<"-"<<minX<<maxY<<"-"<<minY <<endl;
                wl+=((maxX-minX) + (maxY-minY));
            }
            else{
                int xx = _insts[mid]._Pins1[k].absX;
                int yy = _insts[mid]._Pins1[k].absY;
                int maxX = xx;
                int minX = xx;
                int maxY = yy;
                int minY = yy;
                for (int p =0 ;p<_nets[netid]._insts.size();p++){
                    if (mid == _nets[netid]._insts[p]) continue;
                    if (_insts[_nets[netid]._insts[p]]._techId != bottomdie ){
                        xx = _nets[netid]._terminal._locationX;
                        yy = _nets[netid]._terminal._locationY;
                        minX = min( minX, xx );
                        maxX = max( maxX, xx );
                        minY = min( minY, yy );
                        maxY = max( maxY, yy );
                    }
                    else{
                        for (int u =0;u<_insts[_nets[netid]._insts[p]]._Pins1.size();u++){
                            if (netid ==_insts[_nets[netid]._insts[p]]._Pins1[u]._netId){
                                xx = _insts[_nets[netid]._insts[p]]._Pins1[u].absX;
                                yy = _insts[_nets[netid]._insts[p]]._Pins1[u].absY;
                                minX = min( minX, xx );
                                maxX = max( maxX, xx );
                                minY = min( minY, yy );
                                maxY = max( maxY, yy );
                                break;
                            }
                           
                        }
                        
                    }
                    wl+=((maxX-minX) + (maxY-minY));
                }
            }
            
        }
        return wl;
    }
    else{
        for(int k=0;k<numPins;k++){
            int netid = _insts[mid]._Pins0[k]._netId;
            //cout<<_nets[netid]._Pins1.size()<<"  "<<_nets[netid]._Pins1.size()<<"  "<<netid<<endl;
            if( netid == -1){
                continue;
            }
            if (_nets[netid]._needTerminal==0){
                int xx = _insts[mid]._Pins0[k].absX;
                int yy = _insts[mid]._Pins0[k].absY;
                if (xx <0) cout<<xx<<" "; //neg
                int maxX = xx;
                int minX = xx;
                int maxY = yy;
                int minY = yy;
                //cout<<_nets[netid]._Pins1.size()<<endl;
                for (int o = 1;o < _nets[netid]._Pins0.size();o++)
                {
                        
                        xx = _nets[netid]._Pins0[o].absX;
                        yy = _nets[netid]._Pins0[o].absY;
                        minX = min( minX, xx );
                        maxX = max( maxX, xx );
                        minY = min( minY, yy );
                        maxY = max( maxY, yy );
                }
                //cout<<maxX<<"-"<<minX<<maxY<<"-"<<minY <<endl;
                wl+=((maxX-minX) + (maxY-minY));
            }
            else{
                int xx = _insts[mid]._Pins0[k].absX;
                int yy = _insts[mid]._Pins0[k].absY;
                int maxX = xx;
                int minX = xx;
                int maxY = yy;
                int minY = yy;
                for (int p =0 ;p<_nets[netid]._insts.size();p++){
                    if (mid == _nets[netid]._insts[p]) continue;
                    if (_insts[_nets[netid]._insts[p]]._techId != bottomdie ){
                        xx = _nets[netid]._terminal._locationX;
                        yy = _nets[netid]._terminal._locationY;
                        minX = min( minX, xx );
                        maxX = max( maxX, xx );
                        minY = min( minY, yy );
                        maxY = max( maxY, yy );
                    }
                    else{
                        for (int u =0;u<_insts[_nets[netid]._insts[p]]._Pins0.size();u++){
                            if (netid ==_insts[_nets[netid]._insts[p]]._Pins0[u]._netId){
                                xx = _insts[_nets[netid]._insts[p]]._Pins0[u].absX;
                                yy = _insts[_nets[netid]._insts[p]]._Pins0[u].absY;
                                minX = min( minX, xx );
                                maxX = max( maxX, xx );
                                minY = min( minY, yy );
                                maxY = max( maxY, yy );
                            }
                           
                        }
                        
                    }
                    wl+=((maxX-minX) + (maxY-minY));
                }
            }
            
        }
        return wl;
    }
}