// #########################################################################################################
// this class is used to save "Libcell" for 2022 ICCAD CAD contest problem B
// the information of "LibCell" will be copied to "Inst" during parsing
// therefore, we can just use "Inst" to do the placement, and there is no need to modify this part
// #########################################################################################################

#ifndef LIBCELL_H
#define LIBCELL_H

#include <string>
#include <vector>
#include <map>
#include<bits/stdc++.h> 
#include "Pin.h"

using namespace std;

class LibCell
{
public:

    // construct the LibCell with name
    LibCell(string name = "" ) : _name(name) {}

    // set the size of LibCell (remember to pass techId to indicate which techonology is used)
    void setSize(int techId, int width, int height) {
        if (techId == 0) {
            _width[0] = width;
            _height[0] = height;
        }
        else {
            _width[1] = width;
            _height[1] = height;
        }
    }

    // add pins of this LibCell
    void addPin(int techId, Pin pPin) {
        if (techId == 0) _Pins0.push_back(pPin);
        else _Pins1.push_back(pPin);
    }

    // clear all the pins of this LibCell
    void clearPins() {
        _Pins0.clear();
        _Pins1.clear();
    }

    // variables from benchmark input
    string _name;
    int _width[2];
    int _height[2];

    // pins of this libcell
    map<string, int> _pinName2Id;
    vector<Pin> _Pins0; // for the first technology
    vector<Pin> _Pins1; // for the second techonology

};

#endif // LIBCELL_H
