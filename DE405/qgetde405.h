// testeph.cpp: verify a JPL ephemeris

// Copyright (C) 2011, Project Pluto,get from the github:
// https://github.com/Bill-Gray/jpl_eph
// date:      2018-10-22
// author:   Modified by AizhiGuo:guoaizhi@asch.whigg.ac.cn
// OS:        Windows7-32bit, this class maybe can run in Qt
// purpose: get the planets' position using the JPL's DExxx products
// Here is mainly for the DE405 used by GNSS PPP
// Dr XiaoGongWei pack the subroutines into a Class
//-----------------------------------------------------------------------------------------
/*
Example:
    QGetDE405 qde405;
    QVector<double> pos_vel;
    qde405.readDE405("/home/david/MySoft/jpleph.405");
    pos_vel = qde405.getPosition(2456596.5,4,3,1);
    qDebug() << pos_vel;
*/


#ifndef QGETDE405_H
#define QGETDE405_H

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <vector>
#include "jpleph.h"

using namespace std;

/* At least at present,  there's no provision for storing more than 1018 */
/* constants in a binary JPL ephemeris.  See 'asc2eph.cpp' for details. */
#define JPL_MAX_N_CONSTANTS 1018

class QGetDE405
{
public:
    QGetDE405();
    ~QGetDE405();
    bool readDE405(char* de405path = "jpleph.405");
    bool closeDe405();
    // return positions or velocity in J2000.0 coord system
    vector<double> getPosition(double JD, int ntarg = 11, int nctr = 3, int calc_velocity = 0); // JD is julian ephemeris date at which interpolation

private:
    void initVar();

// variables
private:
    char nams[JPL_MAX_N_CONSTANTS][6];
    double vals[JPL_MAX_N_CONSTANTS];
    void *ephem;
    char* m_DE405_Path; // DE405 file PATH
    double au;
    double kmps;//convert au/day to km/s

    bool m_isRead;// juge is read DE405
    bool m_isCloseFile;// juge is close DE405

};

#endif // QGETDE405_H
