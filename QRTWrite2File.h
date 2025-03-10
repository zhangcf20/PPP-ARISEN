
#ifndef QRTWRITE2FILE_H
#define QRTWRITE2FILE_H



#include "QCmpGPST.h"

#define MAX_FILE_WRITE 10

class QRTWrite2File:public QBaseObject
{
// Functional part
public:
    QRTWrite2File(void);
    ~QRTWrite2File(void);
    bool setSaveFloder(QString fload_path, QString obsinfo);
    void closeSaveFile();
    bool writeRecivePos2Txt(const RecivePos oneRecivePos, MatrixXd *Qmat=NULL);// The result of calculating ENU direction is written to TXT
    bool writePPP2Txt(const QVector<SatlitData> epochSatlite);// Corrections to errors are written to. PPP file
    bool writeClockZTDW2Txt(const ClockData epochZTDWClock);// Write zenith wet delay and clock difference to TXT first column wet delay second column clock difference
    bool writeAmbiguity2Txt(QString fload_path);// Write the satellite stored in all Ambiguity variable into TXT file named "G32.txt", "C02.txt", "R08.txt" and so on. The first column is ambiguity.
    bool WriteEpochPRN(QString fload_path, QString tempfileName);// Write the satellite stored in variable allAmbiguity to the file, the first column epoch, the second column satellite number
    bool writeRecivePosKML(QString fload_path, QString tempfileName);// generate KML format
    bool writeBadSatliteData(const SatlitData oneSatAmb);// Write the bad satellite stored in variable allBadSatlitData to the file
private:
    bool isDirExist(QString fullPath);
    bool WriteAmbPRN(QString temp_floder, int PRN,char SatType);// Fuzziness of Writing to PRN Satellite
    int findFilenameFlag(QString Filename);

    QFile m_QFileVct[MAX_FILE_WRITE];
    QTextStream m_QTextStreamVct[MAX_FILE_WRITE];
    QString  m_QFileNames[MAX_FILE_WRITE];
    int m_save_filenum;
    bool m_ZTDW_Clock_head;

    QString m_ofn;


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
