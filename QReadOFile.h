
#ifndef QREADOFILE_H
#define QREADOFILE_H

#include "QGlobalDef.h"


// Read satellite observation files, able to read versions RINEX 2.0 and 3.0

typedef struct  _obsVarNames
{// version 3 is used
    int obsNum3ver;// version 3 is used
    QString SatType;//G R C S E: system number
    QVector< QString > obsNames3ver;// version 3 is used
}obsVarNamesVer3;

// store carrier and pseudo-range dual-frequency types and positions (which can be extended to three frequencies)
typedef struct _storeCP
{
  QString SatType;//G R C S E: system number

  QVector< QString > C1Type;
  QVector< int > C1Pos;
  QVector< QString > C2Type;
  QVector< int > C2Pos;
  QVector< QString > C3Type;
  QVector< int > C3Pos;

  QVector< QString > L1Type;
  QVector< int > L1Pos;
  QVector< QString > L2Type;
  QVector< int > L2Pos;
  QVector< QString > L3Type;
  QVector< int > L3Pos;
}CLPos;


class QReadOFile:public QBaseObject
{
// function part
public:
	QReadOFile(void);
	~QReadOFile(void);
// several important functions to read data
    //  Must be called before setObsFileName()!!! Set the PPP dual-frequency observation type.
    void setWangObsType(QVector<QStringList> wantObsTypeVct);// wantObsTypeVct = {{G, L1C, L1W}, {R, L1C, L2P}, {C, L2I, L6I}, ...}
    void setObsFileName(QString OfileName);
    void getEpochData(QVector< SatlitData > &epochData);// read a epoch metadata (extendable by version)
    void getMultEpochData(QVector< QVector< SatlitData > >&multEpochData,int epochNum = 1000);// read epochNum epoch metadata (if you go to the bottom of the file in advance, it may not be enough epochNum metadata)
    void closeFile();// Please call this function to close the file
    bool isEnd();// determine whether the end of the file is reached (end)
// Function to get header file information
    QString getComment();// get header file comment information
    void getApproXYZ(double* AppXYZ);// get approximate coordinates
    void getAntHEN(double* m_AntHEM);// get antenna HEN correction
    void getFistObsTime(int* m_YMDHM,double &Seconds);// obtain the initial observation epoch time
    QString getMakerName();// get the antenna mark name
    double getInterval();// obtain observation interval (unit s)
    QString getAntType();// get receiver antenna type
    QString getReciveType();// get receiver type
private:
    void initVar();// initializes variables
    void getHeadInf();// read header information
    void readEpochVer3(QVector< SatlitData > &epochData);// read 3.x version file (read all observation data)
    void readEpochVer2(QVector< SatlitData > &epochData); // read 2.x version file (read all observation data)
    void getObsType(SatlitData &oneSatlite);// get the type of observation based on oneSatlite.SatType
    // debug: 2018.08.03
    bool getOneSatlitData(QString &dataString,SatlitData &oneSatlite);// convert characters into data lines, parse to satellite data, obtain satellite ranging codes and carrier data
    void getWantData_2(SatlitData &oneSatlit);// reinex2.x gets the data you need (my PPP needs pseudo-distance and carrier, please change this function if you need other data)
    void getWantData_3(SatlitData &oneSatlit);// reinex3.x gets the data you need (my PPP needs pseudo-distance and carrier, change this function if you need other data). Main idea: read C2P first, read C2W if C2P is 0, and recursively
    QString getObsTypeData(SatlitData &oneSatlit, QString obsType, double &obsValue);// ObsType observation data obsValue was obtained by using version less than 3, and observation type QString(obsType: Rinex2:C1 P1 L1 L2; Rinex3: C1C P1C L1C L2C)
    //debug:2017.07.08
    void getFrequencyVer3(SatlitData &oneSatlite);// get the frequencies of L1 and L2 according to the type of satellite PRN
    void getFrequencyVer2(SatlitData &oneSatlite);// get the frequencies of L1 and L2 according to the type of satellite PRN
    void getFrequency(SatlitData &oneSatlite);// get the frequencies of L1 and L2 according to the type of satellite PRN
    void PriorityRanking();// prioritize the observation values (2018.08.02) version is greater than or equal to 3
    void ProcessCLPos(obsVarNamesVer3 epochSystem);// Get the priority order of reading observations
// data section
public:
	
private:
// internal variables
    QFile m_readOFileClass;// reads the O file class
    QString m_OfileName;// save the O file name
    bool m_IsCloseFile;
	bool isReadHead;
    QString tempLine;// one-line string buffering
    QVector< CLPos > m_allObsTypePos;
    QVector< obsVarNamesVer3 > m_obsVarNamesVer3;// Version is larger than 3 storage multiple systems SYS / # / OBS TYPES
    QVector< QString > m_ObservVarsNames_2;// Version less than 3 is used //# / TYPES OF OBSERV
    int m_TypeObservNum_2;// Version less than 3 is used //# / TYPES OF OBSERV
    QRegExp matchHead;//Version 2.x USES a matching header file (just initialize it once)
    int baseYear;//Version 2.x is used to get the baseYear (for example, 1989 baseYear = 1900; 2010 baseYear = 2000)
// header file information
	//RINEX VERSION / TYPE
	double RinexVersion;
	QChar FileIdType;
	QChar SatelliteSys;
	//PGM / RUN BY / DATE
	QString PGM;
	QString RUNBY;
	QString CreatFileDate;
	//COMMENT
	QString CommentInfo;
	//MARKER NAME
	QString MarkerName;
	//MARKER NUMBER
	QString MarkerNumber;
	//OBSERVER / AGENCY
	QString ObserverNames;
	QString Agency;
	//REC # / TYPE / VERS
	QString ReciverREC;
	QString ReciverType;
	QString ReciverVers;
	//ANT # / TYPE
	QString AntNumber;
	QString AntType;
	//APPROX POSITION XYZ
	double ApproxXYZ[3];
	//ANTENNA: DELTA H/E/N
	double AntHEN[3];
	//WAVELENGTH FACT L1/2
	int FactL12[2];

	//INTERVAL
	double IntervalSeconds;
	//TIME OF FIRST OBS
	int YMDHM[5];
	double ObsSeconds;
	QString SateSystemOTime;//GPS UTC
    QVector<QStringList> m_wantObsTypeVct;
};

#endif

