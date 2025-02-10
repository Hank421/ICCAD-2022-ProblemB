// #########################################################################################################
// this class is used to save "Pin" for 2022 ICCAD CAD contest problem B
// we can find the information of Pin from "Inst" and "Net"
// we can reach to the connected net by the information of "_netId"
// #########################################################################################################

#ifndef PIN_H
#define PIN_H
#include <string>
#include<bits/stdc++.h> 

using namespace std;

class Pin
{
public:
    Pin(string name = "", int xOffset = 0, int yOffset = 0) : _name(name), _xOffset(xOffset), _yOffset(yOffset) {
        _netId = -1; // indicate not connect
    }

    string _name;
    int _xOffset, _yOffset; // offsets from the left bottom of the instance
    int _netId; // save the net ID connect this pin
    int _instId; // save the inst ID of this pin
    int absX, absY;//add by lee for DP

};

#endif // PIN_H
