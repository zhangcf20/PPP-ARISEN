
#ifndef QWINDUP_H
#define QWINDUP_H

#include "QGlobalDef.h"

#include "QCmpGPST.h"
/* Antenna phase center WindUp, encapsulated by RTKLIB code
*/


class QWindUp
{
// Functional part
public:
	QWindUp(void);
	~QWindUp(void);
    // SatPos and RecPos represent WGS84 coordinates of satellites and receivers;Phw is the phase rotation of the last epoch  [-0.5+0.5]. .
    double getWindUp(int Year,int Month,int Day,int Hours,int Minuts,double Seconds,double *StaPos,double *RecPos,double &phw,double *psunpos);

    //2020.12.24 23z rtklib
    double yaw_nominal(double beta, double mu);
    int yaw_angle(double beta, double mu, double *yaw);
    int sat_yaw(gtime_t time, const double *rs, double *exs, double *eys);
    void matcpy(double *A, const double *B, int n, int m);

    double cf(int cf);

private:
    // By introducing solar coordinates, the calculation speed can be accelerated.
    void windupcorr(gtime_t time, const double *rs, const double *rr,	double *phw,double *psunpos = NULL);
// Variable part


public:

private:
	QCmpGPST m_qCmpClass;
};

#endif

