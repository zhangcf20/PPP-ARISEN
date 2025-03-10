
#ifndef QBASEOBJECT_H
#define QBASEOBJECT_H

#include <QString>
#include <QDebug>
#include <ConfigWidget.h>
/*
1.  illustration:
QString tempLine = "G";
char tempSatType = '0';
tempSatType = *(tempLine.mid(0,1).toLatin1().data());//convert qstring to char

*/


class QBaseObject
{
//public functions
public:
    enum PPP_MODEL    {
        PPP_Combination = 0,
        PPP_NOCombination = 1
    };
	QBaseObject(void);
	~QBaseObject(void);
    //set satilite system SystemStr:"G"( open GPS system);"GR":(open GPS+GLONASS system);"GRCE"(open all system)
    //GPS, GLONASS, BDS and Galieo use the letters G, R, C and E respectively.
    bool setSatlitSys(QString SystemStr);
    //GPS, GLONASS, BDS and Galieo use the letters G, R, C and E respectively.
    QString getSatlitSys();
    //Systeam: G R C (representing GPS, GLONASS, BDS) determines whether the system data is needed
    bool isInSystem(char Sys);
    //Get the current settings for several systems
    int getSystemnum();
    void setPPPModel(PPP_MODEL _PPP_MODEL) {m_PPP_MODEL = _PPP_MODEL;}
    PPP_MODEL getPPPModel() {return m_PPP_MODEL;}
    QString getSaveFloderName() {return m_floder_name;}
    static ConfigWidget m_ConfigWidget;
private:
    //Initialize data (GPS is turned on by default)
    void initVar();

//Data section
protected:
    //Join System Judgment

    //Whether to add GPS (default on)
    bool IsaddGPS;
    //Whether to add GLONASS or not
    bool IsaddGLOSS;
    //Whether to join Beidou
    bool IsaddBDS;
    //Whether to join Galieo or not
    bool IsaddGalieo;
    //GPS, GLONASS, BDS and Galieo use the letters G, R, C and E respectively.
    QString m_SystemStr;
    int m_SystemNum;
    PPP_MODEL m_PPP_MODEL;
    QString m_floder_name;
private:
};

#endif
