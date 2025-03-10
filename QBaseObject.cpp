#include "QBaseObject.h"


//Initialization data
void QBaseObject::initVar()
{
	IsaddGPS = false;
	IsaddGLOSS = false;
	IsaddBDS = false;
	IsaddGalieo = false;
	m_SystemNum = 0;
    m_PPP_MODEL = PPP_MODEL::PPP_Combination;
    m_floder_name = "";
}

//
QBaseObject::QBaseObject(void)
{
    initVar();
}

//
QBaseObject::~QBaseObject(void)
{
}

//Setting up the file system. SystemStr:"G"(Turn on the GPS system);"GR":(Turn on the GPS+GLONASS system);"GRCE"(Open all)et al
//GPS, GLONASS, BDS, and Galieo are used respectively: the letters G, R, C, E 23z
bool QBaseObject::setSatlitSys(QString SystemStr)
{
	if (!(SystemStr.contains("G")||SystemStr.contains("R")||SystemStr.contains("C")||SystemStr.contains("E")))
		return	false;
    m_SystemStr = SystemStr;// save systeam sat
	//
	if (SystemStr.contains("G"))
	{
		IsaddGPS = true;
		m_SystemNum++;
	}
	else
		IsaddGPS = false;
	//
	if (SystemStr.contains("R"))
	{
		IsaddGLOSS = true;
		m_SystemNum++;
	}
	else
		IsaddGLOSS = false;
	//
	if (SystemStr.contains("C"))
	{
		IsaddBDS = true;
		m_SystemNum++;
	}
	else
		IsaddBDS = false;
	//
	if (SystemStr.contains("E"))
	{
		IsaddGalieo = true;
		m_SystemNum++;
	}
	else
		IsaddGalieo = false;
	return true;
}


//Sys = G R C E(Representing GPS, GLONASS, BDS, Galieo systems)Determine if the system data is needed
bool QBaseObject::isInSystem(char Sys)
{
	if (IsaddGPS&&Sys == 'G')
		return true;
	else if (IsaddGLOSS&&Sys == 'R')
		return true;
	else if (IsaddBDS&&Sys == 'C')
		return true;
	else if (IsaddGalieo&&Sys == 'E')
		return true;
	return false;
}

//Get the current face set several systems
int QBaseObject::getSystemnum()
{
	return m_SystemNum;
}


//Get the current face set several systems
QString QBaseObject::getSatlitSys()
{
    //Let the satellite types strictly follow this order 2021.03.24 23z
    QString sys_temp="";
    if(m_SystemStr.contains('G')) sys_temp = sys_temp + 'G';
    if(m_SystemStr.contains('E')) sys_temp = sys_temp + 'E';
    if(m_SystemStr.contains('R')) sys_temp = sys_temp + 'R';
    if(m_SystemStr.contains('C')) sys_temp = sys_temp + 'C';
    m_SystemStr = sys_temp;
    return m_SystemStr;
}
