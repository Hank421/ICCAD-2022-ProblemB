// #########################################################################################################
// this class is used to save "Row" for 2022 ICCAD CAD contest problem B
// this is only a structure to save the information of row
// #########################################################################################################

#ifndef ROW_H
#define ROW_H

#include <map>
#include <vector>
#include<bits/stdc++.h> 

using namespace std;

class Row
{
public:
    Row(int startX = 0, int startY = 0, int rowLength = 0, int rowHeight = 0) : _startX(startX), _startY(startY), _rowLength(rowLength), _rowHeight(rowHeight) {
        _density = 0;
        m_interval.clear();
        // m_insts.clear();
    }

    int _startX;
    int _startY;
    int _rowLength;
    int _rowHeight;
    int _density; // record the total instance width place in this row
    vector<int> m_interval; // x-coordinate
    // vector<int> m_insts; // inst id in the row
    map<int, int> m_rowmodule; // x-coordinate, module ID // will be init in detailP
    map<int, int> m_empties; // x-coordinate, length // will be init in detailP
};

#endif // ROW_H
