
#ifndef QREADCLK_H
#define QREADCLK_H


#include "QGlobalDef.h"

/*

1. Description: Lagrange interpolation is adopted to obtain satellite clock error; 22-point interpolation is automatically adopted for 30s clock difference; and 8-point interpolation is adopted for other time intervals
If the complete data cannot be obtained, the clock error is 0

2. Examples are as follows:
(1) class initialization
QReadClk readClk("D:\\igs15786.clk");//The path must be specified

(2) it needs to read data once before calling the class, and call getAllData() if it wants to read data; A longer time
readClk.getAllData();

(3) then call getStaliteClk(int PRN,double GPST,double *pCLKT);
double GPST = 518400,SatClock = 0;// define GPST within seconds of the GPS cycle and the clock difference to be saved
readClk.getStaliteClk(32,GPST,&SatClock);// get satellite clock delay saved to SatClock

3. Note:
(1) this class can read RINEX VERSION/TYPE: 3.00 / C (currently only read GPS satellite; if necessary, you can modify readFileData2Vec and readAllData functions by yourself.)
(2) it is suggested to call getStaliteClk in accordance with GPS time (GPST) to "improve the running speed", otherwise it is not obvious. Change ACCELERATE to 0 if you don't want to call by time

*/


typedef struct _CLKData
{//Clk file data format
	int GPSWeek;
    double GPSTime;// GPS week within seconds
    MatrixXd MatrixDataGPS;// the ith column of the GPS system stores PRN X,Y and Z respectively
	MatrixXd MatrixDataGlonass;//Glonass
	MatrixXd MatrixDataBDS;//BDS
	MatrixXd MatrixDataGalieo;//Galieo
}CLKData;

class QReadClk:public QBaseObject
{
// function part
public:
	QReadClk();
	QReadClk(QStringList ClkFileNames);
    void setClkFileNames(QStringList ClkFileNames);
	~QReadClk(void);
    QVector< CLKData > getAllData();// get all the data
    bool setSatlitSys(QString SystemStr);// Where, GPS,GLONASS,BDS,Galieo are respectively used: letters G,R,C,E (override parent class)
    void releaseAllData();// use up the memory release in time
    void getStaliteClk(int PRN,char SatType,double GPST,double *pCLKT);// obtain satellite clock error within seconds of GPST launch time
private:
    void initVar();// initializes variables
    void InitStruct();// initialize the internal matrix of the structure to be used (to save memory)
    void readAllHead();// read header information
    void readFileData2Vec(QStringList ClkFileNames);// read multiple file data to m_allEpochData
    void get8Point(int PRN,char SatType,double *pCLKT,double *pGPST,double GPST);//pCLKT: coordinates of 8 points; PGPST :8 GPS points per second; GPST satellite launch time within weeks seconds
    void lagrangeMethod(int PRN,char SatType,double GPST,double *pCLKT);// the seventh-order Lagrangian interpolation was performed at 8 points before and after the launch time of GPST
    double YMD2GPSTime(int Year,int Month,int Day,int Hours,int Minutes,double Seconds,int *GPSWeek = NULL);//YMD Change to GPST //GPSTimeArray[4]=GPSWeek GPS_N
// variable section
public:
    bool ACCELERATE;// 1: accelerating program (time from small to large) 0: non-accelerating program (time out of order)

private:
    QFile m_readCLKFileClass;// read the.clk file class
	QVector< CLKData > m_allEpochData;
	CLKData epochData;
	QString m_ClkFileName;
	QStringList m_ClkFileNames;
	QString tempLine;
    int m_WeekOrder;// save the number of cross-week files
    bool IsSigalFile;// whether multiple files or a single file
    bool isReadHead;// whether to read header file
    bool isReadAllData;// whether to read the entire file data
    int lagrangeFact;// simple lagrangeFact = 2 can be used to judge whether the CLK file is 30s or 5min 30s; 5 min lagrangeFact = 8
// variables used to optimize looking for clock differences prevent looking all the way from 0
    int m_lastGPSTimeFlag;// save the first GPS time data obtained by get8Point last time
    double m_lastCmpGPSTime;// save the last calculated GPST
    int m_EndHeadNum;// the first and last clock difference between the number of read blocks
    MatrixXd *pSysChMat;// select the system pointer
	
};

#endif

