// #########################################################################################################
// this class is used to save "Inst" for 2022 ICCAD CAD contest problem B
// we can trace each "Net" connect by the "Inst"
// remember to initialize the Inst before use (void initialize(...))
// use the function in public to get and set the values
//      all the set function will only work on the current "_techId"
//      all the get function will only get the value for the current "_techId"
// #########################################################################################################

#ifndef INST_H
#define INST_H

#include <string>
#include <vector>
#include <map>
#include<bits/stdc++.h> 
#include "Pin.h"
#include "LibCell.h"

using namespace std;

class Inst 
{
public:

    Inst(string name = "") : _name(name) {}

    int x() { return ((_techId == 0) ? _x[0] : _x[1]); }
    int y() { return ((_techId == 0) ? _y[0] : _y[1]); }
    int width() { return ((_techId == 0) ? _width[0] : _width[1]); }
    int height() { return ((_techId == 0) ? _height[0] : _height[1]); }
    int centerX() { return ((_techId == 0) ? _x[0] + _width[0]/2 : _x[1] + _width[1]/2); }
    int centerY() { return ((_techId == 0) ? _y[0] + _height[0]/2 : _y[1] + _height[1]/2); }
    int area() { return ((_techId == 0) ? _width[0] * _height[0] : _width[1] * _height[1]); }

    void setPosition(int x, int y) {
        if (_techId == 0) {
            _x[0] = x;
            _y[0] = y;
        }
        else {
            _x[1] = x;
            _y[1] = y;
        }
    }

    void setCenterPosition(int x, int y) {
        if (_techId == 0) {
            _x[0] = x - _width[0]/2;
            _y[0] = y - _height[0]/2;
        }
        else {
            _x[1] = x - _width[1]/2;
            _y[1] = y - _height[1]/2;
        }
    }
    
    void setTechId(int techId) {
        if (techId >= 2) {
            cout << "Error: There would be at most 2 technologies (ID = 0, 1).\n";
            exit(-1);
        }
        _techId = techId;
    }

    void switchTechId() {
        if (_techId == 0) _techId = 1;
        if (_techId == 1) _techId = 0;
    }

    int numPins() { return _Pins0.size(); }

    // set (for pins of this Inst)
    void setNumPins(int numPins) {
        _Pins0.resize(numPins);
        _Pins1.resize(numPins);
    }

    void addPin(Pin pPin) {
        if (_techId == 0) _Pins0.push_back(pPin);
        else _Pins1.push_back(pPin);
    }

    void clearPins() {
        _Pins0.clear();
        _Pins1.clear();
    }

    // initialize the position and techID for the instance
    void initialize(int x0, int x1, int y0, int y1, int tech) {
        _x[0] = x0;
        _x[1] = x1;
        _y[0] = y0;
        _y[1] = y1;
        _techId = tech;
    }

    // copy the information from libcell to inst (need to consider the technology)
    void cloneFromlibcell(LibCell lib, int topTech, int bottomTech) {
        _libname = lib._name;
        _width[0] = lib._width[topTech];
        _width[1] = lib._width[bottomTech];
        _height[0] = lib._height[topTech];
        _height[1] = lib._height[bottomTech];
        for (int i=0; i<lib._Pins0.size(); i++) {
            (topTech == 0) ? _Pins0.push_back(lib._Pins0[i]) : _Pins0.push_back(lib._Pins1[i]);
            (bottomTech == 0) ? _Pins1.push_back(lib._Pins0[i]) : _Pins1.push_back(lib._Pins1[i]);
        }
    }

    int _libCellId; // this inst is build from this libcell ID

    vector<int> _nets; // this vector saves Net ID

    // pins of the Inst
    vector<Pin> _Pins0; // pin for top die
    vector<Pin> _Pins1; // pin for bottom die

    // variables from benchmark input
    string _name;
    string _libname;
    int _x[2]; // low x
    int _y[2]; // low y
    int _width[2];
    int _height[2];
    int _techId; // represent which layer (top or bottom) this Inst is

};

#endif // INST_H
