
#ifndef QWRITE2FILE_H
#define QWRITE2FILE_H



#include "QCmpGPST.h"


class QWrite2File:public QBaseObject
{
// Functional part
public:
	QWrite2File(void);
	~QWrite2File(void);
    bool writeRecivePos2Txt(QString fload_path, QString tempfileName);// The result of calculating ENU direction is written to TXT
    bool writePPP2Txt(QString fload_path, QString tempfileName);// Corrections to errors are written to. PPP file
    bool writeClockZTDW2Txt(QString fload_path, QString tempfileName);// Write zenith wet delay and clock difference to TXT first column wet delay second column clock difference
    bool writeAmbiguity2Txt(QString fload_path);// Write the satellite stored in all Ambiguity variable into TXT file named "G32.txt", "C02.txt", "R08.txt" and so on. The first column is ambiguity.
    bool WriteEpochPRN(QString fload_path, QString tempfileName);// Write the satellite stored in variable allAmbiguity to the file, the first column epoch, the second column satellite number
    bool writeRecivePosKML(QString fload_path, QString tempfileName);// generate KML format
    bool writeBadSatliteData(QString fload_path, QString tempfileName);// Write the bad satellite stored in variable allBadSatlitData to the file
private:
    bool isDirExist(QString fullPath);
    bool WriteAmbPRN(QString temp_floder, int PRN,char SatType);// Fuzziness of Writing to PRN Satellite
// Variable part
public:
    QVector< RecivePos > allReciverPos;// Save the position result to TXT KML
    QVector< QVector < SatlitData > > allPPPSatlitData;// Save the data calculated by PPP to write to. PPP file
    QVector< Ambiguity> allAmbiguity;// Ambiguity of Storage Satellite
    QVector< ClockData > allClock;// Store Clock Data
    QVector< VectorXd > allSolverX;// Storage Solution X
    QVector< MatrixXd > allSloverQ;// Storage Covariance Matrix
    QVector< SatlitData > allBadSatlitData;// Storage Elimination Satellites
private:
    QCmpGPST m_qcmpClass;// Function Computing Library Class
};

#endif

