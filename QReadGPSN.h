
#ifndef QREADGPSN_H
#define QREADGPSN_H

#include "QGlobalDef.h"



class QReadGPSN: public::QBaseObject
{
// function part
public:
	QReadGPSN(void);
	~QReadGPSN(void);
    QReadGPSN(QString NFileName);
    void setFileName(QString NFileName);
    QVector< BrdData > getAllData();// read all broadcast ephemeris data to allBrdData
    //PRN: satellite, SatType: satellite type (G,C,R,E), UTC (BDS and GLONASS function internal automatic conversion), CA: pseudo-range observation value
    void getSatPos(int PRN, char SatType, double signal_transmission_time, int Year, int Month, int Day, int Hours,
                   int Minutes, double Seconds, double *StaClock, double *pXYZ, double *pdXYZ);
private:
    void initVar();// initializes variables
    void getHeadInf();// read header information
    void readNFileVer2(QVector< BrdData > &allBrdData);// read Rinex 2.x broadcast ephemeris data
    void readNFileVer3(QVector< BrdData > &allBrdData);// read Rinex 2.x broadcast ephemeris data
    int SearchNFile(int PRN,char SatType,double GPSOTime);// search for recent navigation data
    double YMD2GPSTime(int Year, int Month, int Day, int Hours, int Minutes, double Seconds, int *WeekN = NULL);//YMD Change to GPST //GPSTimeArray[4]=GPSWeek GPS_N
    double getLeapSecond(int Year,int Month,int Day,int Hours=0,int Minutes=0,double Seconds = 0.0);// gey leap seconds
	double computeJD(int Year,int Month,int Day,int HoursInt,int Minutes = 0,double Seconds = 0.0);
    double computeSatClock(double *A, double t, double t0);
	Vector3d GlonassFun(Vector3d Xt,Vector3d dXt,Vector3d ddX0);
	Vector3d RungeKuttaforGlonass(const BrdData &epochBrdData,double tk,double t0,Vector3d &dX);
// data section
public:

private:
    QFile m_readGPSNFile;// read the broadcast ephemeris file
    QString m_NfileName;// save the name of the broadcast ephemeris file

    int m_BaseYear;// basic year is defined as 2000
    double m_leapSec;// save the skip second
	QVector< BrdData > m_allBrdData;
    bool isReadHead;// determines whether to read the header file
    bool isReadAllData;// determine whether to read all data
    QString tempLine;// cache one line of string
    int m_epochDataNum_Ver2;// store one data segment (28 7 rows for GPS and BDS and 12 3 rows for GLONASS)
// The following is the header data section
	//RINEX VERSION / TYPE
	double RinexVersion;
    char FileIdType;//G,C and R represent GPS,BDS and GLONASS systems respectively
	//PGM / RUN BY / DATE
	QString PGM;
	QString RUNBY;
	QString CreatFileDate;
	//COMMENT
	QString CommentInfo;
	//ION ALPHA
	double IonAlpha[4];
	double IonBeta[4];
	//DELTA-UTC: A0,A1,T,W
	double DeltaA01[2];
	int DeltaTW[2];
	//	IsReadHeadInfo
	bool IsReadHeadInfo;
};
#endif
