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

// k1 * x + k2 * width (k2 should < 0)
// maybe we should only use width information for mix size design
int Placement::calculatePriority(int k1, int k2, int x, int width)
{
    return (k1 * x + k2 * width);
}

// calculate the position score (all the parameter can be change in this function to get a better result)
// use distance and density information (should run faster)
int Placement::calculatePositionScore(int dieId, int rowId, int inst_x, int inst_y, int inst_w)
{
    int a = 1; // can be tuned
    int b = 1; // can be tuned
    if (dieId == 0) {
        if (_topRow[rowId]._density + inst_w > _topRow[rowId]._rowLength) return INT_MAX;
        int score = a * (abs(inst_x - _topRow[rowId]._density) + abs(inst_y - rowId * _topRow[rowId]._rowHeight)) + b * (_topRow[rowId]._density);
        return score;
    }
    else {
        if (_bottomRow[rowId]._density + inst_w > _bottomRow[rowId]._rowLength) return INT_MAX;
        int score = a * (abs(inst_x - _bottomRow[rowId]._density) + abs(inst_y - rowId * _bottomRow[rowId]._rowHeight)) + b * (_bottomRow[rowId]._density);
        return score;
    }
}

// calculate the position score (all the parameter can be change in this function to get a better result)
// use hpwl to calculate (should have better quality but not)
int Placement::calculatePositionScore2(int rowId, Inst inst)
{
    int inst_x = inst.x();
    int inst_y = inst.y();
    int inst_w = inst.width();
    if (inst._techId == 0) {
        // calculate old position wirelength
        if (_topRow[rowId]._density + inst_w > _topRow[rowId]._rowLength) return INT_MAX;
        long long total_wl_old = 0;
        for (int i=0; i<inst._nets.size(); i++) {
            int top_max_x = INT_MIN;
            int top_max_y = INT_MIN;
            int top_min_x = INT_MAX;
            int top_min_y = INT_MAX;
            int bot_max_x = INT_MIN;
            int bot_max_y = INT_MIN;
            int bot_min_x = INT_MAX;
            int bot_min_y = INT_MAX;
            for (int j=0; j<_nets[inst._nets[i]]._insts.size(); j++) {
                int pinX, pinY;
                if (_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) {
                    for (int k=0; k<_insts[_nets[inst._nets[i]]._insts[j]]._Pins0.size(); k++) {
                        if (_insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._netId == _netName2ID[_nets[inst._nets[i]]._name]) {
                            pinX = _insts[_nets[inst._nets[i]]._insts[j]].x() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._xOffset;
                            pinY = _insts[_nets[inst._nets[i]]._insts[j]].y() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._yOffset;
                            break;
                        }
                    }
                }
                else {
                    for (int k=0; k<_insts[_nets[inst._nets[i]]._insts[j]]._Pins1.size(); k++) {
                        if (_insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._netId == _netName2ID[_nets[inst._nets[i]]._name]) {
                            pinX = _insts[_nets[inst._nets[i]]._insts[j]].x() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._xOffset;
                            pinY = _insts[_nets[inst._nets[i]]._insts[j]].y() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._yOffset;
                            break;
                        }
                    }
                }
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinX > top_max_x)) top_max_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinX > bot_max_x)) bot_max_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinY > top_max_y)) top_max_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinY > bot_max_y)) bot_max_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinX < top_min_x)) top_min_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinX < bot_min_x)) bot_min_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinY < top_min_y)) top_min_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinY < bot_min_y)) bot_min_y = pinY;
            }
            total_wl_old += (top_max_x - top_min_x) + (top_max_y - top_min_y) + (bot_max_x - bot_min_x) + (bot_max_y - bot_min_y);
        }
        // change position to the row
        int old_x = inst.x();
        int old_y = inst.y();
        inst.setPosition(_topRow[rowId]._density, _topRow[rowId]._rowHeight * rowId);
        // calculate new position wirelength
        long long total_wl_new = 0;
        for (int i=0; i<inst._nets.size(); i++) {
            int top_max_x = INT_MIN;
            int top_max_y = INT_MIN;
            int top_min_x = INT_MAX;
            int top_min_y = INT_MAX;
            int bot_max_x = INT_MIN;
            int bot_max_y = INT_MIN;
            int bot_min_x = INT_MAX;
            int bot_min_y = INT_MAX;
            for (int j=0; j<_nets[inst._nets[i]]._insts.size(); j++) {
                int pinX, pinY;
                if (_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) {
                    for (int k=0; k<_insts[_nets[inst._nets[i]]._insts[j]]._Pins0.size(); k++) {
                        if (_insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._netId == _netName2ID[_nets[inst._nets[i]]._name]) {
                            pinX = _insts[_nets[inst._nets[i]]._insts[j]].x() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._xOffset;
                            pinY = _insts[_nets[inst._nets[i]]._insts[j]].y() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._yOffset;
                            break;
                        }
                    }
                }
                else {
                    for (int k=0; k<_insts[_nets[inst._nets[i]]._insts[j]]._Pins1.size(); k++) {
                        if (_insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._netId == _netName2ID[_nets[inst._nets[i]]._name]) {
                            pinX = _insts[_nets[inst._nets[i]]._insts[j]].x() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._xOffset;
                            pinY = _insts[_nets[inst._nets[i]]._insts[j]].y() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._yOffset;
                            break;
                        }
                    }
                }
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinX > top_max_x)) top_max_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinX > bot_max_x)) bot_max_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinY > top_max_y)) top_max_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinY > bot_max_y)) bot_max_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinX < top_min_x)) top_min_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinX < bot_min_x)) bot_min_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinY < top_min_y)) top_min_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinY < bot_min_y)) bot_min_y = pinY;
            }
            total_wl_new += (top_max_x - top_min_x) + (top_max_y - top_min_y) + (bot_max_x - bot_min_x) + (bot_max_y - bot_min_y);
        }
        return (total_wl_new - total_wl_old);
    }
    else {
        if (_bottomRow[rowId]._density + inst_w > _bottomRow[rowId]._rowLength) return INT_MAX;
        long long total_wl_old = 0;
        for (int i=0; i<inst._nets.size(); i++) {
            int top_max_x = INT_MIN;
            int top_max_y = INT_MIN;
            int top_min_x = INT_MAX;
            int top_min_y = INT_MAX;
            int bot_max_x = INT_MIN;
            int bot_max_y = INT_MIN;
            int bot_min_x = INT_MAX;
            int bot_min_y = INT_MAX;
            for (int j=0; j<_nets[inst._nets[i]]._insts.size(); j++) {
                int pinX, pinY;
                if (_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) {
                    for (int k=0; k<_insts[_nets[inst._nets[i]]._insts[j]]._Pins0.size(); k++) {
                        if (_insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._netId == _netName2ID[_nets[inst._nets[i]]._name]) {
                            pinX = _insts[_nets[inst._nets[i]]._insts[j]].x() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._xOffset;
                            pinY = _insts[_nets[inst._nets[i]]._insts[j]].y() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._yOffset;
                            break;
                        }
                    }
                }
                else {
                    for (int k=0; k<_insts[_nets[inst._nets[i]]._insts[j]]._Pins1.size(); k++) {
                        if (_insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._netId == _netName2ID[_nets[inst._nets[i]]._name]) {
                            pinX = _insts[_nets[inst._nets[i]]._insts[j]].x() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._xOffset;
                            pinY = _insts[_nets[inst._nets[i]]._insts[j]].y() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._yOffset;
                            break;
                        }
                    }
                }
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinX > top_max_x)) top_max_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinX > bot_max_x)) bot_max_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinY > top_max_y)) top_max_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinY > bot_max_y)) bot_max_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinX < top_min_x)) top_min_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinX < bot_min_x)) bot_min_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinY < top_min_y)) top_min_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinY < bot_min_y)) bot_min_y = pinY;
            }
            total_wl_old += (top_max_x - top_min_x) + (top_max_y - top_min_y) + (bot_max_x - bot_min_x) + (bot_max_y - bot_min_y);
        }
        // change position to the row
        int old_x = inst.x();
        int old_y = inst.y();
        inst.setPosition(_bottomRow[rowId]._density, _bottomRow[rowId]._rowHeight * rowId);
        // calculate new position wirelength
        long long total_wl_new = 0;
        for (int i=0; i<inst._nets.size(); i++) {
            int top_max_x = INT_MIN;
            int top_max_y = INT_MIN;
            int top_min_x = INT_MAX;
            int top_min_y = INT_MAX;
            int bot_max_x = INT_MIN;
            int bot_max_y = INT_MIN;
            int bot_min_x = INT_MAX;
            int bot_min_y = INT_MAX;
            for (int j=0; j<_nets[inst._nets[i]]._insts.size(); j++) {
                int pinX, pinY;
                if (_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) {
                    for (int k=0; k<_insts[_nets[inst._nets[i]]._insts[j]]._Pins0.size(); k++) {
                        if (_insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._netId == _netName2ID[_nets[inst._nets[i]]._name]) {
                            pinX = _insts[_nets[inst._nets[i]]._insts[j]].x() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._xOffset;
                            pinY = _insts[_nets[inst._nets[i]]._insts[j]].y() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins0[k]._yOffset;
                            break;
                        }
                    }
                }
                else {
                    for (int k=0; k<_insts[_nets[inst._nets[i]]._insts[j]]._Pins1.size(); k++) {
                        if (_insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._netId == _netName2ID[_nets[inst._nets[i]]._name]) {
                            pinX = _insts[_nets[inst._nets[i]]._insts[j]].x() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._xOffset;
                            pinY = _insts[_nets[inst._nets[i]]._insts[j]].y() + _insts[_nets[inst._nets[i]]._insts[j]]._Pins1[k]._yOffset;
                            break;
                        }
                    }
                }
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinX > top_max_x)) top_max_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinX > bot_max_x)) bot_max_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinY > top_max_y)) top_max_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinY > bot_max_y)) bot_max_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinX < top_min_x)) top_min_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinX < bot_min_x)) bot_min_x = pinX;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 0) && (pinY < top_min_y)) top_min_y = pinY;
                if ((_insts[_nets[inst._nets[i]]._insts[j]]._techId == 1) && (pinY < bot_min_y)) bot_min_y = pinY;
            }
            total_wl_new += (top_max_x - top_min_x) + (top_max_y - top_min_y) + (bot_max_x - bot_min_x) + (bot_max_y - bot_min_y);
        }
        return (total_wl_new - total_wl_old);
    }
}

// the legalization process will continue until there is no overlap or reach the legalize_limits
// an evenly dispersed global placement will make it easier to legalize
// during the legalization process, the below 2 variables are considered for the priority
//     1. x coordinate of Inst (center)
//     2. Inst size (width only due to the same height)
// while placing the Insts on the rows, few things below are considered
//     1. consider distance of all the rows
//     2. consider the density of all the rows
void Placement::TetrisLikeLegalization(int legalize_limits)
{
    vector< pair<int, int> > priorityTop; // priority for topDie Inst
    vector< pair<int, int> > priorityBottom; // priority for bottomDie Inst
    // calculate for the priority
    for (int i=0; i<numInsts(); i++) {
        if (_insts[i]._techId == 0) {
            priorityTop.push_back(make_pair(calculatePriority(1, 0, _insts[i].x(), _insts[i].width()), i));
        }
        else {
            priorityBottom.push_back(make_pair(calculatePriority(1, 0, _insts[i].x(), _insts[i].width()), i));
        }
    }
    // sort the priority for legalization
    sort(priorityTop.begin(), priorityTop.end());
    sort(priorityBottom.begin(), priorityBottom.end());
    // save the original x, y
    vector< pair<int, int> > top_x_y;
    vector< pair<int, int> > bottom_x_y;
    for (int i=0; i<priorityTop.size(); i++) top_x_y.push_back(make_pair(_insts[priorityTop[i].second].x(), _insts[priorityTop[i].second].y()));
    for (int i=0; i<priorityBottom.size(); i++) bottom_x_y.push_back(make_pair(_insts[priorityBottom[i].second].x(), _insts[priorityBottom[i].second].y()));
    // ========== TopDie: ==========
    // legalization for the first time
    int a = 0;
    int fail_cnt = 0;
    top_again:
    for (int i=0; i<_topRow.size(); i++) {
        _topRow[i]._density = 0;
        _topRow[i].m_interval.clear(); // for DetailPlacement
        // _topRow[i].m_insts.clear();
    }
    bool top_success = true;
    for (int i=0; i<priorityTop.size(); i++) {
        Inst inst = _insts[priorityTop[i].second];
        int min_score = INT_MAX;
        int id = -1;
        for (int j=0; j<_topRow.size(); j++) {
            int score = calculatePositionScore(0, j, max(_topRow[id]._density, (a*_topRow[id]._density+inst.x()) / (a+1)), inst.y(), inst.width());
            // int score = calculatePositionScore2(j, inst);
            if (score < min_score) {
                min_score = score;
                id = j;
            }
        }
        if (min_score == INT_MAX) {
            cerr << "Warning: Top Die Legalization fail !!! (" << a << ")\n";
            top_success = false;
            break;
        }
        else {
        	int pos = max(_topRow[id]._density, (a*_topRow[id]._density+_insts[priorityTop[i].second].x()) / (a+1));
        	if (_insts[priorityTop[i].second].x() + _insts[priorityTop[i].second].width() > _topRow[id]._rowLength) {
        		pos = _topRow[id]._density;
        	}
            _insts[priorityTop[i].second].setPosition(pos, _topRow[id]._rowHeight * id);
            _topRow[id]._density = pos + _insts[priorityTop[i].second].width();
            _topRow[id].m_interval.push_back(pos); // build up for DetailPlacement
            // _topRow[id].m_insts.push_back(priorityTop[i].second);
        }
    }
    // first-time legalization fails (try more times)
    if (!top_success) {
        if (fail_cnt < legalize_limits) {
        	fail_cnt++;
        	a += (100 / legalize_limits);
        	for (int i=0; i<priorityTop.size(); i++) {
        		_insts[priorityTop[i].second].setPosition(top_x_y[i].first, top_x_y[i].second);
        	}
        	goto top_again;
        }
    }

    // ========== BottomDie: ==========
    // legalization for the first time
    a = 0;
    fail_cnt = 0;
    bottom_again:
    for (int i=0; i<_bottomRow.size(); i++) {
        _bottomRow[i]._density = 0;
        _bottomRow[i].m_interval.clear(); // for DetailPlacement
        // _bottomRow[i].m_insts.clear();
    }
    bool bottom_success = true;
    for (int i=0; i<priorityBottom.size(); i++) {
        Inst inst = _insts[priorityBottom[i].second];
        int min_score = INT_MAX;
        int id = -1;
        for (int j=0; j<_bottomRow.size(); j++) {
            int score = calculatePositionScore(1, j, max(_bottomRow[id]._density, (a*_bottomRow[id]._density+inst.x()) / (a+1)), inst.y(), inst.width());
            // int score = calculatePositionScore2(j, inst);
            if (score < min_score) {
                min_score = score;
                id = j;
            }
        }
        if (min_score == INT_MAX) {
            cerr << "Warning: Bottom Die Legalization fail !!! (" << a << ")\n";
            bottom_success = false;
            break;
        }
        else {
        	int pos = max(_bottomRow[id]._density, (a*_bottomRow[id]._density+_insts[priorityBottom[i].second].x()) / (a+1));
        	if (_insts[priorityBottom[i].second].x() + _insts[priorityBottom[i].second].width() > _bottomRow[id]._rowLength) {
        		pos = _bottomRow[id]._density;
        	}
            _insts[priorityBottom[i].second].setPosition(pos, _bottomRow[id]._rowHeight * id);
            _bottomRow[id]._density = pos + _insts[priorityBottom[i].second].width();
            _bottomRow[id].m_interval.push_back(pos); // build up for DetailPlacement
            // _bottomRow[id].m_insts.push_back(priorityBottom[i].second);
        }
    }
    // first-time legalization fails (try more times)
    if (!bottom_success) {
        if (fail_cnt < legalize_limits) {
        	fail_cnt++;
        	a += (100 / legalize_limits);
        	for (int i=0; i<priorityBottom.size(); i++) {
        		_insts[priorityBottom[i].second].setPosition(bottom_x_y[i].first, bottom_x_y[i].second);
        	}
        	goto bottom_again;
        }
    }
    if (top_success && bottom_success) cerr << "Legalization Success !!!\n";
}


void Placement::singleRowOptimization()
{
    // sort the row by density
    // vector< pair<int, int> > top_row_density;
    // vector< pair<int, int> > bottom_row_density;
    // for (int i=0; i<_topRow.size(); i++) {
    //     int row_density = 0;
    //     for (int j=0; j<_topRow[i].m_insts.size(); j++) {
    //         row_density += _insts[_topRow[i].m_insts[j]].width();
    //     }
    //     top_row_density.push_back(make_pair(row_density, i));
    // }
    // for (int i=0; i<_bottomRow.size(); i++) {
    //     int row_density = 0;
    //     for (int j=0; j<_bottomRow[i].m_insts.size(); j++) {
    //         row_density += _insts[_bottomRow[i].m_insts[j]].width();
    //     }
    //     bottom_row_density.push_back(make_pair(row_density, i));
    // }
    // sort(top_row_density.rbegin(), top_row_density.rend());
    // sort(bottom_row_density.rbegin(), bottom_row_density.rend());
    // // optimize top die
    // for (int i=0; i<top_row_density.size(); i++) {
    //     _topRow[top_row_density[i].second]._density = 0;
    //     for (int j=0; j<_topRow[top_row_density[i].second].m_insts.size(); j++) {
    //         int id = _topRow[top_row_density[i].second].m_insts[j];
    //         int wire_length = estimate_one_die_inst_WL(0, id);
    //         int temp_x = _insts[id].x();
    //         int temp_y = _insts[id].y();
    //         int new_x = _topRow[top_row_density[i].second]._density;
    //         _insts[id].setPosition(new_x, temp_y);
    //         // estimate wirelength difference
    //         int new_wire_length = estimate_one_die_inst_WL(0, id);
    //         if (new_wire_length > wire_length) {
    //             _insts[id].setPosition(temp_x, temp_y);
    //         }
    //         _topRow[top_row_density[i].second]._density = _insts[id].x() + _insts[id].width();
    //     }
    //     _topRow[top_row_density[i].second].m_interval.clear();
    //     for (int j=0; j<_topRow[top_row_density[i].second].m_insts.size(); j++) {
    //         _topRow[top_row_density[i].second].m_interval.push_back(_insts[_topRow[top_row_density[i].second].m_insts[j]].x());
    //     }
    // }
    // // optimize bottom die
    // for (int i=0; i<bottom_row_density.size(); i++) {
    //     _bottomRow[bottom_row_density[i].second]._density = 0;
    //     for (int j=0; j<_bottomRow[bottom_row_density[i].second].m_insts.size(); j++) {
    //         int id = _bottomRow[bottom_row_density[i].second].m_insts[j];
    //         int wire_length = estimate_one_die_inst_WL(1, id);
    //         int temp_x = _insts[id].x();
    //         int temp_y = _insts[id].y();
    //         int new_x = _bottomRow[bottom_row_density[i].second]._density;
    //         _insts[id].setPosition(new_x, temp_y);
    //         // estimate wirelength difference
    //         int new_wire_length = estimate_one_die_inst_WL(1, id);
    //         if (new_wire_length > wire_length) {
    //             _insts[id].setPosition(temp_x, temp_y);
    //         }
    //         _bottomRow[bottom_row_density[i].second]._density = _insts[id].x() + _insts[id].width();
    //     }
    //     _bottomRow[bottom_row_density[i].second].m_interval.clear();
    //     for (int j=0; j<_bottomRow[bottom_row_density[i].second].m_insts.size(); j++) {
    //         _bottomRow[bottom_row_density[i].second].m_interval.push_back(_insts[_bottomRow[bottom_row_density[i].second].m_insts[j]].x());
    //     }
    // }
}

int Placement::estimate_one_die_inst_WL(int dieId, int id)
{
    // int total_wire_length = 0;
    // for (int j=0; j<_insts[id]._nets.size(); j++) {
    //     int netId = _insts[id]._nets[j];
    //     int max_x = INT_MIN;
    //     int min_x = INT_MAX;
    //     int max_y = INT_MIN;
    //     int min_y = INT_MAX;
    //     for (int k=0; k<_nets[netId].numPins(); k++) {
    //         if (_insts[_nets[netId]._Pins0[k]._instId]._techId != dieId) continue;
    //         double pin_x = _insts[_nets[netId]._Pins0[k]._instId].x() + _nets[netId]._Pins0[k]._xOffset;
    //         double pin_y = _insts[_nets[netId]._Pins0[k]._instId].y() + _nets[netId]._Pins0[k]._yOffset;
    //         if (pin_x > max_x) max_x = pin_x;
    //         if (pin_x < min_x) min_x = pin_x;
    //         if (pin_y > max_y) max_y = pin_y;
    //         if (pin_y < min_y) min_y = pin_y;
    //     }
    //     total_wire_length += (max_x - min_x) + (max_y - min_y);
    // }
    // return total_wire_length;
}