
#ifndef _QGLOBALDEF
#define _QGLOBALDEF

//Necessary header file
#include <QVector>
#include <QString>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <qmath.h>
#include <QTime>
#include <QFileInfo>
#include <QMessageBox>
#include <iostream>
using namespace std;
//Eigen libraries (some classes are independent without Eigen)

#include <Eigen/Dense>
using namespace Eigen;
// C standard library function
#include <time.h>
#include <math.h>
#include <stdio.h>
// Custom base class
#include "QBaseObject.h"

class global_cf : public QObject
{
public:
    //2021.03.09 23z
    static QString filepath_keep;//Path to save Kalman filter state
    static QString filepath_ini;//The path to get the Kalman filter state
    static double randomwalk_pos;//Random walk of kinematic PPP
    static bool keepornot;//Whether to save the final Kalman filter state
    static bool ini_csv;//Whether to initialize based on state information
    static double randomwalk_zwd;//Random walk of zwd
    static bool whitenoise_pos;//White noise model of kinematic PPP
    static bool fix_and_hold;//hold or not
    static int reinitialize;
    static bool ini_snx;//Whether to use snx file to initialize

    static bool isGPT3;//Whether to use GPT3

    static QString Elevation_weight_function;
    static QString Trop_map;

    static bool flag_sampling;
    static double Interval_original;
    static double Interval_used;

    static bool flag_seismology;
    static double After_OT;
    static double Before_OT;
    static double OTh;
    static double OTm;
    static double OTs;

    static int CSI_ZWDV;
    static double Threshold_ofile_size;

    static double weight_G;
    static double weight_R;
    static double weight_C;
    static double weight_E;

};



#define OMGE        7.2921151467E-5     /* earth angular velocity (IS-GPS) (rad/s) */ //2020.12.24 23z rtklib

// Mathematical Character Definition
#define MM_PI 3.1415926535897932385// Radius of circumference
#define M_C 299792458.0 // The speed of light M
#define M_GM 3.986005e14 // Earth's gravitational constant (m)(old value:3.986005e14)
#define M_GMK 398600.5 // The Earth's Gravitational Constant (km)
#define M_GMK_GLO 398600.44 // The Earth's Gravitational Constant (km)
#define M_We 0.000072921151467 // Earth rotation angular velocity
#define M_We_OLD 0.00007292115 // Earth rotation angular velocity
#define M_Re 6378137 // Reference ellipse length radius (m)
#define M_ReK 6378.137 // Reference ellipse length radius (km)
#define M_C20 -0.001082657 // The Second Harmonic Coefficient of Gravity Potential
#define M_nan 1e-15
#define M_Zgama_P_square  (3*3) // Deionospheric Composite P Code Variance
#define M_IR  0.1 // ionospheric residual M


// GPS system parameters
#define M_F1   (1.57542e9)//F1
#define M_F2   (1.2276e9)//F2
#define M_F5   (1.17645e9)//F2
#define M_Lamta1   (0.190293672798365)//L1 wavelength
#define M_Lamta2   (0.244210213424568)//L2 wavelength
#define M_Lamta5	  (0.254828048790854)//L5 wavelength
static double g_GPSFrq[6] = {0, 1.57542e9, 1.2276e9, 0, 0, 1.17645e9};//debug:2017.07.08


//GLONASS system parameters
static int g_GlonassK[24] = {1,-4,5,6,1,-4,5,6,-2,-7,0,-1,-2,-7,0,-1,4,-3,3,2,4,-3,3,2};//Frequency Number Corresponding to Glonass PRN
#define M_GLONASSF1(k) (1602 + k*0.5625)*1e6
#define M_GLONASSF2(k) (1246 + k*0.4375)*1e6
#define M_GLOSSLamta1k(k) (M_C/((1602 + k*0.5625)*1e6))// GLONASS L1 Wavelength Corresponding to Frequency Number
#define M_GLOSSLamta2k(k) (M_C/((1246 + k*0.4375)*1e6))// GLONASS L1 Wavelength Corresponding to Frequency Number

//BDS system parameters
static double g_BDSFrq[8] = {0, 1.561098e9, 1.561098e9, 0, 0, 0, 1.26852e9, 1.20714e9};//Storage Beidou Frequency
#define M_BDSLamtak(k) (M_C/g_BDSFrq[k])// Beidou wavelength

//Galieo system parameters
static double g_GalieoFrq[9] = {0, 1.57542e9, 0, 0, 0, 1.17645e9, 1.27875e9, 1.20714e9, 1.191795};//Galieo Band 1 5 6 7 8 Frequency
#define M_GalieoLamtak(k) (M_C/g_GalieoFrq[k])// Galieo wavelength

//PPP eliminates ionospheric combination wavelengths
//GPS wavelength
#define  M_alpha1 (M_F1*M_F1/(M_F1*M_F1 - M_F2*M_F2))
#define  M_alpha2 (M_F2*M_F2/(M_F1*M_F1 - M_F2*M_F2))
#define  M_GPSLamta3 (M_alpha1*M_Lamta1 - M_alpha2*M_Lamta2)//0.106953378142147

//Calculate LL3
#define M_GetLamta3(F1,F2) ( (F1*M_C)/(F1*F1 - F2*F2) - (F2*M_C)/(F1*F1 - F2*F2) )

//RTKLIB structure
#ifndef RTKLIB_H
//Define the RTKlib variable
#define D2R         (MM_PI/180.0)          /* deg to rad */
#define AU          149597870691.0      /* 1 AU (m) */
#define AS2R        (D2R/3600.0)        /* arc sec to radian */
#define RE_WGS84    6378137.0           /* earth semimajor axis (WGS84) (m) */
#define FE_WGS84    (1.0/298.257223563) /* earth flattening (WGS84) */

// valid integer and decimal times from 1970-1-1 to 2038-1-18
typedef struct {        /* time struct */
    time_t time;        /* time (s) expressed by standard time_t */
    double sec;         /* fraction of second under 1 s */
} gtime_t;

typedef struct {        /* earth rotation parameter data type */
    double mjd;         /* mjd (days) */
    double xp,yp;       /* pole offset (rad) */
    double xpr,ypr;     /* pole offset rate (rad/day) */
    double ut1_utc;     /* ut1-utc (s) */
    double lod;         /* length of day (s/day) */
} erpd_t;

typedef struct {        /* earth rotation parameter type */
    int n,nmax;         /* number and max number of data */
    erpd_t *data;       /* earth rotation parameter data */
} erp_t;

typedef struct {        /* GLONASS broadcast ephemeris type */
    int sat;            /* satellite number */
    int iode;           /* IODE (0-6 bit of tb field) */
    int frq;            /* satellite frequency number */
    int svh,sva,age;    /* satellite health, accuracy, age of operation */
    gtime_t toe;        /* epoch of epherides (gpst) */
    gtime_t tof;        /* message frame time (gpst) */
    double pos[3];      /* satellite position (ecef) (m) */
    double vel[3];      /* satellite velocity (ecef) (m/s) */
    double acc[3];      /* satellite acceleration (ecef) (m/s^2) */
    double taun,gamn;   /* SV clock bias (s)/relative freq bias */
    double dtaun;       /* delay between L1 and L2 (s) */
} geph_t;

typedef struct {        /* GPS/QZS/GAL broadcast ephemeris type */
    int sat;            /* satellite number */
    int iode,iodc;      /* IODE,IODC */
    int sva;            /* SV accuracy (URA index) */
    int svh;            /* SV health (0:ok) */
    int week;           /* GPS/QZS: gps week, GAL: galileo week */
    int code;           /* GPS/QZS: code on L2 */
                        /* GAL: data source defined as rinex 3.03 */
                        /* BDS: data source (0:unknown,1:B1I,2:B1Q,3:B2I,4:B2Q,5:B3I,6:B3Q) */
    int flag;           /* GPS/QZS: L2 P data flag */
                        /* BDS: nav type (0:unknown,1:IGSO/MEO,2:GEO) */
    gtime_t toe,toc,ttr; /* Toe,Toc,T_trans */
                        /* SV orbit parameters */
    double A,e,i0,OMG0,omg,M0,deln,OMGd,idot;
    double crc,crs,cuc,cus,cic,cis;
    double toes;        /* Toe (s) in week */
    double fit;         /* fit interval (h) */
    double f0,f1,f2;    /* SV clock parameters (af0,af1,af2) */
    double tgd[4];      /* group delay parameters */
                        /* GPS/QZS:tgd[0]=TGD */
                        /* GAL    :tgd[0]=BGD E5a/E1,tgd[1]=BGD E5b/E1 */
                        /* CMP    :tgd[0]=BGD1,tgd[1]=BGD2 */
    double Adot,ndot;   /* Adot,ndot for CNAV */
} eph_t;

#endif
// customize the structure
typedef struct _StationInfo
{
    QString AntType;
    double AntCorr[3];// Antenna Correction: according deltype(Usually you can't accept HENs that need to look at O files.)
    double satpos[3];
    int deltype;/* Antenna delta type (0:HEN,1:XYZ) */
    QString locationName;// station location
    QString RecverType;// reciver type
}StationInfo;

typedef struct _GPSPosTime
{
    int Year;//year
    int Month;//moth
    int Day;//day
    int Hours;//hour
    int Minutes;//minutes
    double Seconds;//seconds
    double TropZHD;//Current epoch ZHD all Satlite have same ZHD in epoch
    int epochNum;//The number of epochs to which it belongs
}GPSPosTime;

typedef struct _RecivePos
{//Receiver Approximate Coordinate Correction by Single Point Positioning
    double dX;//X or E
    double dY;//Y or N
    double dZ;//Z or U
    double spp_pos[3];
    int totolEpochStalitNum;//Number of satellites in an epoch
    GPSPosTime UTCtime;
}RecivePos;

typedef struct _MWLLP
{//Parameters used to detect cycle slips
    double MW;
    double dL;
    double LP;
}CyclySlipNum;


//Broadcast ephemeris data format for each ephemeris
typedef struct _brdeph
{
    char SatType;//Satellite type
    int PRN;//Satellite Number
    GPSPosTime UTCTime;//UTC time
    double TimeDiv;//Clock deviation
    double TimeMove;//Clock drift
    double TimeMoveSpeed;//Clock drift acceleration
    int GLONASS_IODE;
    QVector< double > epochNData;//Store a data segment (GPS, BDS and Galieo are 28 7 rows GLONASS is 12 3 rows)
    geph_t temp_geph_t;
    eph_t temp_eph_t;
}BrdData;

// PPP saves the calculated data
// Preservation of satellite data and various corrections
typedef struct _SatlitData
{
    int PRN;// Satellite PRN
    char SatType;// Satellite type
    GPSPosTime UTCTime;// Time stored in minutes and seconds, month, day, year, not necessarily UTC time
    int EpochFlag;// Each epoch state (generally not 0:OK 1: power error of the first and second epoch.
                  // 777: getGoodSatlite() bad!, 888: SPP bad, 999 filter bad)
    int LL1_LOCK;// (rarely or hardly used: satellite signal out of lock or cycle-skip signal 0: OK or unknow. > : maybe ambiguity/Slip)
    int SigInten;// Satellite status signal (generally not used; -1:unknow, Signal intensity range:1-9. If EpochFlag>0 and EpochFlag < 3 can't use)
    double L1;// (cycle)
    double L2;// (cycle)
    double L3;// (cycle)
    double C1;// Store P1 or C1(m)
    double C2;// Store P2 or C2(m)
    double C3;// Store P3 or C3(m)
    // uncombination variable
    double LL1;// unEliminate ionospheric carrier (m)!!!!
    double CC1;// unElimination of ionospheric pseudo-distance (m)
    double LL2;// unEliminate ionospheric carrier (m)!!!!
    double CC2;// unElimination of ionospheric pseudo-distance (m)
    double VL1;// Filtering residue of LL1 (m)
    double VC1;// Filtering residue of CC1 (m)
    double VL2;// Filtering residue of LL2 (m)
    double VC2;// Filtering residue of CC2 (m)
    double CC1_Smooth;// Phase smoothing pseudo-distance (m)
    double CC2_Smooth;// Phase smoothing pseudo-distance (m)
    double CC1_Smooth_Q; // The covariance of PP3_Smooth
    double CC2_Smooth_Q; // The covariance of PP3_Smooth; Note:smooth number use PP3_Smooth_NUM
    double ionL1;// Calculated L1 ionospheric delay (m)

    // combination variable
    double LL3;// Eliminate ionospheric carrier (m)!!!!
    double PP3;// Elimination of ionospheric pseudo-distance (m)
    double VLL3 = 0;// Filtering residue of LL3 (m)
    double VPP3 = 0;// Filtering residue of PP3 (m)
    double PP3_Smooth;// Phase smoothing pseudo-distance (m)
    double PP3_Smooth_NUM;// Phase smoothing pseudo-distance number
    double PP3_Smooth_Q; // The covariance of PP3_Smooth

    // Ambiguity and ion
    double Amb1;// Ambiguity (stored in Amb for both integer and floating point Numbers of L1)
    double Amb2;// Ambiguity (stored in Amb for both integer and floating point Numbers of L2)
    double Amb;// Ambiguity (stored in Amb for both integer and floating point Numbers Of Ion_free LL3)
    // others correction

    double DCB_P1C1 = 0.0;

    //Store grgbia data 2020.11.03 23z
    QVector< double > biaofobs;

    //Store Reference sats[prn,sit] 2020.11.05 23z
    int prn_referencesat[8] = {0};
    int prn_referencesat_previous[8] = {0};

    double grg_bia_wl = 0.0;

    // 2020.11.28 23z
    bool SSDPPP = false;

    // 2020.11.30 23z
    bool ARornot = false;

    //wl 2020.11.30 23z
    double wl_smooth = 0.0;
    double wl_float_SSD = 0.0;
    int wl_fixed_SSD = 0;
    int times_wl = 0;
    int diff_wl_refsats[4] = {0};
    //Fixed sign of wide lane
    bool wl_fixed_flag = false;

    //NL 2020.12.01 23z
    double nl_float_SSD = 0.0;
    int nl_fixed_SSD = 0;
    int times_nl = 0;
    bool nl_fixed_flag = false;
    int diff_nl_refsats[4] = {0};

    //SSD fixed solution 2020.12.07 23z
    double SSD_fixed = 0.0;
    bool SSD_fixed_flag = false;

    //Coordinate variance 2020.12.08 23z
    double sigam_pos = 0.0;
    int times_sigam_pos = 0;

    //Get the difference between the reference stars (narrow lane) 20202.12.12 23z
    bool refsat_changed_succeed = false;

    //2020.12.15 23z
    int times_hold = 0;

    int num_fixedsats = 0;
    //The default sampling interval is 30s 2020.12.16 23z
    int interval = 30;

    double amb_SSD_sats = 0.0;

    //flag of refsat candidates 2020.12.14 23z
    bool reference_flag = true;

    double ratio = 0.0;

    //2021.01.08 23z
    QString markname;

    int epochlength = 0;

    //2021.04.17 23z
    bool AR_succeed = false;

    QVector< QString > wantObserType;// Store the observed value type corresponding to the read dual-frequency (store order as C1, L1, C2, L2)
    QVector< double > obserValue;// Observation data corresponding to obserType
    QVector< QString > obserType;// Observation types (Rinex_2: C1, L1, D1, S1, C2, L2, etc. Rinex_3: C1C, L1C, S1C, C2W, L2W, etc.)
    QVector<QString> badMsg;// Storage Elimination Satellites message
    double Frq[3];// Record the frequencies of L1, L2, and L3
    double X;// Wgs-84 satellite coordinates need to be converted from file (m)
    double Y;//(m)
    double Z;//(m)
    double EA[2];// Satellite altitude Angle and azimuth (degrees)
    //Model correction
    double Relativty;// Relativistic correction (m)
    double Sagnac;// Correction of earth autobiography (m)
    double StaClock;// Satellite clock difference (m)
    double StaClockRate;// Satellite clock difference (m/s)
    double SatTrop;// Signal direction tropospheric dry delay correction(ZPD) (m)
    double StaTropMap;// The wet projection function of the troposphere(mf)
    double AntHeight;// Correction of Antenna Height (m)
    double L1Offset;// L1 receiver antenna number correction (cycle)
    double L2Offset;// L2 receiver antenna number correction (cycle)
    double SatL1Offset;// L1 satellite antenna weeks correction (weeks)
    double SatL2Offset;// L2 satellite antenna weeks change by xiaogongwei 2019.04.12
    double TideEffect;// Tidal correction (m)
    double AntWindup;// Phase windup correction (cycle)
    double SatWight;// SatWight = sin(e)^2 / M_Zgama_P_square
} SatlitData;

// define ambiguity
typedef struct _Ambiguity
{
    int PRN;// Satellite PRN
    char SatType;// Satellite type
    double ionL1;// Calculated L1 ionospheric delay (m)
    double EA[2];// Satellite altitude Angle and azimuth (degrees)
    double Amb1;// Ambiguity (stored in Amb for both integer and floating point Numbers of L1)
    double Amb2;// Ambiguity (stored in Amb for both integer and floating point Numbers of L2)
    double Amb;// Ambiguity (stored in Amb for both integer and floating point Numbers Of Ion_free LL3)
    bool isIntAmb;// Is integer ambiguity
    GPSPosTime UTCTime;// Time stored in minutes and seconds, month, day, year, not necessarily UTC time

    //星间单差模糊度 2020.11.11 23z
    double amb_diff_sats;
}Ambiguity;

// define storage clock difference and zenith wet delay
typedef struct _Clock
{
    double clockData[6];// X system receiver clock error,Xi system relative to X system receiver clock deviation
    double ZTD_W;// Zenith wet delay
    GPSPosTime UTCTime;// UTC year, month, day, hour, minute and second
}ClockData;

//typedef struct
//{//2020.11.18 23z 一个观测量的模糊度参数
//    int prn;
//    QString typeofsat;

//    //当前卫星连续出现的次数
//    int times;

//    double mw_wl_smooth;
//    double grg_wl;

//    //星间单差消电离层组合的浮点模糊度和其固定解
//    double SSD_IF_float;
//    int SSD_IF_fixed;

//    double wl_float;
//    int wl_fixed;

//    double nl_float;
//    int nl_fixed;

//}abm;

//typedef struct
//{//2020.11.18 23z 一个历元的模糊度参数
//    //时间信息
//    GPSPosTime UTCTime;
//    //模糊度信息
//    QVector< abm >ar;
//    //nl_fixed的协方差
//    MatrixXd Q_nl_fixed;

//}AR;

// print error message
//qDebug()<<erroInfo<<endl;
#define ErroTrace(erroInfo) {QMessageBox::critical(NULL, "Error", erroInfo, QMessageBox::Yes);}
#define ErrorMsg(myerromsg) \
{char buff[256]; sprintf(buff, "%s, line:%d, fuction:%s", __FILE__, __LINE__, __func__); myerromsg = "----"+QString(buff)+"----";}
// defines system newline characters
#if defined(_WIN32)
// Windows system
static QString ENDLINE = "\r\n";
static QString PATHSEG = "/";
static bool SYSTEMTYPE = 0;
#else
// Linux or MAC
static QString ENDLINE = "\n";
static QString PATHSEG = "/";
static bool SYSTEMTYPE = 1;
#endif

// struct for GUI
typedef struct _PlotGUI
{
    QVector< double > X, Y, Z;// Store PPP coordinates
    QVector< double > spp_X, spp_Y, spp_Z;// Store SPP coordinates
    QVector< double > clockData;// Receiver difference
    QVector< double > clockData_bias_1;// Receiver clock difference system deviation 1
    QVector< double > clockData_bias_2;// Receiver clock difference system deviation 2
    QVector< double > clockData_bias_3;// Receiver clock error system deviation 3
    QVector< double > ZTD_W;// Zenith wet delay
    QString save_file_path;// Save path for drawing results

}PlotGUIData;


#endif // _QGLOBALDEF

// Annotated example
/*
 * Purpose: use SRIF Factorization Matrix Update Time
 * ------------------------------------------------------------
 * Example:
 * | Rwk_1            0         0 | QR ->   | Rwk Rwx Zw |
 * | -Rp*Phi_Inv*G  Rp*Phi_Inv  Zp|         | 0   Rp  Zp |
 * [Rd Zd; 0 ed] stored in AL. AL as input,meanwhile as output
 *-------------------------------------------------------------
 * Input:
 * 		Rp: 	a priori square root information (SRI) matrix (an n * n upper triangular matrix)
 * 		Zp: 	a priori SRIF state vector, of length n*1 (state is X, Zp = Rp*X).
 * 		Phi:    transition matrix, an n * n matrix.
 *  	G :     The n by ns matrix associated with process noise.
 *  			The process noise covariance is G*Q*transpose(G) where inverse(Q)
 *  			is transpose(Rw)*Rw. G is destroyed on output.
 *  	Rwk_1:  a priori square root information matrix for the process noise, an ns by ns upper triangular matrix
 * 		Zw :    a priori 'state' associated with the process noise, a vector with ns elements.  Usually set to zero by
 *				the calling routine (for unbiased process noise).
 * 		Rw:     An ns by n matrix which is set to zero by this routine, but is used for output.
 * -------------------------------------------------------------
 * output:
 * 		Rp: 	updated matrix (upper triangular, dimension N*N)
 * 		Zp: 	updated vector (length N)
 * 		Rwk_1:  a posteriori square root information matrix for the process noise, an ns by ns upper triangular matrix
 * 		Rwx:
 * 		Zw :
 * 		[Rwk_1 Rwx Zw] use to SRIF smoothing data
 * 	where Rp*x = Zp
 * -------------------------------------------------------------
 * Authors: XiaoGongWei; Email: xiaogongwei10@163.com;
 * Date: 2018-10-17; github: https://github.com/xiaogongwei
 * -------------------------------------------------------------
 * reference: Bierman, G.J. "Factorization Methods for Discrete Sequential
 * 				Estimation," Academic Press, 1977.
 */
