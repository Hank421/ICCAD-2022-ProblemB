// #########################################################################################################
// this class is used to save "Terminal" for 2022 ICCAD CAD contest problem B
// remember to pass the net name
// #########################################################################################################

#ifndef TERMINAL_H
#define TERMINAL_H
#include <string>
#include<bits/stdc++.h> 

using namespace std;

class Terminal
{
public:
    Terminal(string name = "", int netid = 0, int locationX = 0, int locationY = 0) : _Netname(name), _NetId(netid), _locationX(locationX), _locationY(locationY) {}

    string _Netname;
    int _NetId;
    int _locationX;
    int _locationY;

};

#endif // TERMINAL_H
