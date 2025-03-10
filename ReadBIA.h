
#ifndef READBIA_H
#define READBIA_H

#include "QGlobalDef.h"
#include "MyMatrix.h"

//23z

typedef struct GBMBIA_line
{
    //year，day of year，second
    int Syear;
    int Sdoy;
    double Ssod;

    //PRN，obs tyoe，bias(ns)
    QString prn;
    QString obs_type;
    double bia_ns;

}GBMBIA_line;

typedef struct GBMBIAof4SYS
{
    int interval;
    QString gbmbiafilename;

    QVector< QString > GPSobs_type;
    QVector< QString > COMPASSobs_type;
    QVector< QString > GLONASSobs_type;
    QVector< QString > GALILEOobs_type;

    QVector< GBMBIA_line > GPSSYS;
    QVector< GBMBIA_line > COMPASSSYS;
    QVector< GBMBIA_line > GLONASSSYS;
    QVector< GBMBIA_line > GALILEOSYS;

}GBMBIA;


typedef struct
{
    int prn;
    double bia_ns;

}grg_wl_line;

typedef struct
{
    int year_wl;
    int month_wl;
    int day_wl;

    QVector< grg_wl_line > wl;
}grg_wl;


typedef struct
{

    int year_as;
    int month_as;
    int day_as;
    int hour_as;
    int minute_as;
    double second_as;

    QString prn;
    double clock;
    double clock_rate;

}grg_clk;

typedef struct
{
    QVector< QString > prn;
    QVector< double > DCB;

}DCB_P1C1;

typedef struct
{
    int launch_year;
    int launch_month;
    int launch_day;

    int deactiv_year;
    int deactiv_month;
    int deactiv_day;

    int GPS;
    int PRN;

    QString block;
    QString orbit;
    QString clock;
}PRN_SVN_single;

typedef struct
{
    int GPS;
    int PRN = 0;

    QString flag;

    double gpst;

    int year;
    int month;
    int day;
    int hour;
    int min;
    double sec;

}shadow;

typedef struct
{
    QString code;
    QString type;
    double value;
    double std_dev;
}snx_single;

typedef struct
{
    QString prn;
    QString type;
    double value;
    double std_dev;
}cod_osb;



class QReadBIA
{
public:

//DCB P1C1.DCB
    void readDCBfile(QString DCBfilepath);
    void getDCBdata(SatlitData &tempSatlitData);

//GFZ
    //gbm.bia
    void readGBMBIAfile(QString gbmbiafilepath);
    //gbm.bia
    void getgbmbiadata(SatlitData &tempSatlitData);

//CNES
    //grg.bia
    void readgrgwlbiafile();
    //grg.bia
    void getgrgwlbiadata(SatlitData &tempSatlitData);

//PRN_SVN  shad
    void readPRN_GPS();
    void readshadfile(QString shadfilepath);
    void shadcorrcect(QVector< SatlitData > epochSatlitData);

//.snx
    void readsnx();
    bool getinitialpos(QVector< snx_single >  &snx_temp);

//Delete short arc satellite
    void deletetempsats(QVector< QVector < SatlitData > > &multepochSatlitData, QString m_system, double m_interval);

//cycle slip detection
    void cycleslip_refsats(QVector< QVector < SatlitData > > &multepochSatlitData, QString m_system);

//Select reference sat
    void select_refsat(QVector< SatlitData > &epochResultSatlitData,QVector< SatlitData > &prevEpochSatlitData, int m_CutAngle, QString m_system);

//Get the sit of the reference sat
    void get_refsat_sit(QVector< SatlitData > &epochResultSatlitData, QString m_system);

//COD
    //osb
    void readcodosbfile(QString codbiafilepath);
    void getcodosb(SatlitData &tempSatlitData);

private:

    DCB_P1C1 DCB_GPS;
    //gbm bias
    GBMBIA gbmbia;
    //grg bia
    QVector< grg_clk > grgclk;
    QVector< grg_wl > wl_gps;
    QVector< grg_wl > wl_gal;

    QVector< PRN_SVN_single > prn_svn;
    QVector< shadow > shad;
    QVector< snx_single > snx;

    //cod
    QVector< cod_osb > cod_bia;


    MyMatrix m_matrix;

};

#endif // READBIA_H
