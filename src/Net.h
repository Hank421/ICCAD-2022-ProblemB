// #########################################################################################################
// this class is used to save "Net" for 2022 ICCAD CAD contest problem B
// we can trace each "Inst" connect by the "Net"
// #########################################################################################################

#ifndef NET_H
#define NET_H

#include <string>
#include <vector>
#include<bits/stdc++.h> 
#include "Pin.h"
#include "Terminal.h"

using namespace std;

class Net {
public:
    Net(string name = "") : _name(name) {}

    int numPins() { return _Pins0.size();}

    void setNumPins(int numPins) { 
        _Pins0.resize(numPins);
        _Pins1.resize(numPins);
    }
    
    // add pins of this LibCell
    void addPin(int techId, Pin pPin) {
        if (techId == 0) _Pins0.push_back(pPin);
        else _Pins1.push_back(pPin);
    }

    // clear all the pins of this Net
    void clearPins() {
        _Pins0.clear();
        _Pins1.clear();
    }

    // set the terminal when needed (remember to add to the vector<Terminal> _terminals in placement.h)
    void setTerminal(int netId, int X, int Y) {
        _terminal._Netname = _name;
        _terminal._NetId = netId;
        _terminal._locationX = X;
        _terminal._locationY = Y;
        _needTerminal = true;
    }

    // delete the terminal when there is no need for this net
    void deleteTerminal() { _needTerminal = false; }

    // variables from benchmark input
    string _name;
    vector<int> _insts; // this vector saves Inst ID

    // pins of this Net
    vector<Pin> _Pins0;
    vector<Pin> _Pins1;

    // terminal
    bool _needTerminal; // this can be an indicator to determine whether terminal is used or not
    Terminal _terminal;
};

#endif // NET_H
