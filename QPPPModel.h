
#ifndef QPPPMODEL_H
#define QPPPMODEL_H

#include "QCmpGPST.h"
#include "QReadOFile.h"
#include "QReadSP3.h"
#include "QReadClk.h"
#include "QTropDelay.h"
#include "QReadAnt.h"
#include "QWindUp.h"
#include "QTideEffect.h"
#include "QKalmanFilter.h"
#include "QWrite2File.h"
#include "SRIFAlgorithm.h"
#include "FtpClient.h"
#include "MyCompress.h"
#include "QualityCtrl.h"
#include "QPseudoSmooth.h"

// next include model is for GUI
#include <QTextEdit>
#include <QApplication>
#include <QTextCursor>
#include <QTime>
#include <QCoreApplication>
#include "QRTWrite2File.h"
#include <QMessageBox>

#include "ReadBIA.h"
#include "QLambda.h"


// use ionospheric free PPP model
class QPPPModel:public QBaseObject
{
    // function part
public:
    // Configure PPP parameters
    QPPPModel(QStringList files_path, QTextEdit *pQTextEdit = NULL, QString Method = "Kalman", QString Satsystem = "G",
              QString TropDelay = "Sass", double CutAngle = 10, bool isKinematic = false, QString Smooth_Str = "NoSmooth",
              QString products = "igs", QString pppmodel_t = "IF", QString deleteSats = "",
              QVector<QStringList> ObsTypeSet = QVector<QStringList>(),
              QVector<QStringList> Qw_Pk_LPacc = QVector<QStringList>());
    ~QPPPModel();
    void initQPPPModel(QString OFileName, QStringList Sp3FileNames, QStringList ClkFileNames, QStringList ErpFileName,
                       QStringList bialistQString, QStringList DCBlist, QStringList shadlist,
                       QString BlqFileName = "OCEAN-GOT48.blq",QString AtxFileName = "antmod.atx",
                       QString GrdFileName = "gpt2_5.grd");
    void Run(bool isDisplayEveryEpoch = true);//isDisplay Every Epoch represent is disply every epoch information?(ENU or XYZ)
    // set SystemStr:"G"(turn on GPS system); GR":(turn on GPS+GLONASS system);" GRCE"(all turned on), etc
    bool setSatlitSys(QString SystemStr);// The letters G,R,C and E are used for GPS,GLONASS,BDS and Galieo respectively
    // configure model
    void setConfigure(QString Method = "Kalman", QString Satsystem = "G", QString TropDelay = "Sass", double CutAngle = 10,
                      bool isKinematic = false, QString Smooth_Str = "NoSmooth", QString products = "igs",
                      QString pppmodel_t = "Ion_free", QString deleteSats = "",
                      QVector<QStringList> ObsTypeSet = QVector<QStringList>(),
                      QVector<QStringList> Qw_Pk = QVector<QStringList>());
    // next public function is for GUI
    // Get operation results( clear QWrite2File::allPPPSatlitData Because the amount of data is too large.)
    void getRunResult(PlotGUIData &plotData);
    bool isRuned(){return m_isRuned;}


    // SSDPPP 2020.11.04 23z
    void runSSDPPP(bool isDisplayEveryEpoch = true);

    // PPPAR 2020.11.30 23z
    void runPPPAR(bool isDisplayEveryEpoch = true);

    // calculate wl amb 2020.11.30 23z
    void calculatewl(QVector < SatlitData > &prevEpochSatlitData, QVector< SatlitData > &epochResultSatlitData);

    // calculate nl amb 2020.12.01 23z  P为协方差阵！！！
    void calculatenl(QVector < SatlitData > &prevEpochSatlitData, QVector< SatlitData > &epochResultSatlitData, VectorXd X, MatrixXd P);

    // obtain interval of obs 2021.04.08 23z
    int getinterval(QVector< QVector < SatlitData > > multepochSatlitData);

private:
    void initVar();
    bool connectHost();
    bool connectCNTHost();
    void getSP3Pos(double GPST, int PRN, char SatType, double *p_XYZ, double *pdXYZ = NULL, double *pSp3Clk = NULL);//GPST satellite p_XYZ returns WGS84 coordinates and velocity in seconds per week
    void getCLKData(int PRN,char SatType,double GPST,double *pCLKT);// obtain satellite clock error within seconds of GPST launch time
    void getSatEA(double X,double Y,double Z,double *approxRecvXYZ,double *EA);// Calculate height Angle and azimuth EA
    double getSagnac(double X,double Y,double Z,double *approxRecvXYZ);// the autobiography of the earth(m)
    double getRelativty(char SatType, double *pSatXYZ, double *pRecXYZ, double *pSatdXYZ);// Calculate relativistic effects
    void getWight(SatlitData &tempSatlitData);// obtain weights of different satellite systems
    void getTropDelay(double MJD,int TDay,double E,double *pBLH,double *mf = NULL, double *ZHD_s = NULL, double *ZPD = NULL, double *ZHD = NULL);// MJD: simplified Julian day, TDay: annual product day, E: altitude Angle (rad) pBLH: geodetic coordinate system, *mf: projection function
    bool getRecvOffset(double *EA,char SatType,double &L1Offset,double &L2Offset, QVector<QString> FrqFlag);// Calculation receiver L1 and L2 phase center correction PCO+PCV,EA: height Angle, azimuth (unit rad), L1Offset and L2Offset represent distance correction of line of sight direction (unit m)
    bool getSatlitOffset(int Year, int Month, int Day, int Hours, int Minutes, double Seconds, int PRN, char SatType, double *StaPos, double *RecPos,
                         double *L12Offset, QVector<QString> FrqFlag);// PCO+PCV correction is calculated. Since G1 and G2 have the same frequency, the correction of the two bands is the same. StaPos and RecPos, satellite and receiver WGS84 coordinates (unit m)
    double getTideEffect(int Year,int Month,int Day,int Hours,int Minutes,double Seconds,double *pXYZ,double *EA,double *psunpos=NULL,
                         double *pmoonpos = NULL,double gmst = 0,QString StationName = "");// calculate the correction of tide in line of sight (unit m) in and out of sun and moon, GMST data can reduce the repeated calculation of these data, StationName can be sent in
    double getWindup(int Year,int Month,int Day,int Hours,int Minutes,double Seconds,double *StaPos,double *RecPos,double &phw,double *psunpos = NULL);//SatPos and RecPos represent satellite and receiver WGS84 coordinate return cycle (unit cycle) range [-0.5 +0.5]
    bool CycleSlip(const SatlitData &oneSatlitData,double *pLP);// cycle slip detection. The pLP is a three-dimensional array. The first is the w-m combination (n2-n1 < 5), the second is the ionospheric residual (<0.3), and the third is (lamt2* n2-lamt1 *N1 < 5).
    double getPreEpochWindUp(QVector < SatlitData > &prevEpochSatlitData,int PRN,char SatType);// WindUp of previous epoch, does not return 0
    // the satellites with high quality are obtained, including: whether the cycle skip height Angle data are missing, c1-p2 <50 adjacent epoch WindUp < 0.3; EpochSatlitData: previous epochSatlitData: current epochSatlitData (automatically delete low-quality satellite); EleAngle: high Angle
    void getGoodSatlite(QVector < SatlitData > &prevEpochSatlitData,QVector < SatlitData > &epochSatlitData,double eleAngle = 10);
    QStringList downProducts(QString store_floder_path = "./", int GPS_Week = 0, int GPS_day = 0, QString productType = "sp3");// download the sp3, CLK file
    QStringList downCNTProducts(QString store_floder_path, int GPS_Week, int GPS_day, QString productType);
    QString downErpFile(QString store_floder_path = "./", int GPS_Week = 0, int GPS_day = 0, QString productType = "erp");// download the ERP file
    void saveResult2Class(VectorXd X, double *spp_vct, GPSPosTime epochTime, QVector< SatlitData > epochResultSatlitData, int epochNum, MatrixXd *P = NULL);// save Filter Result to m_writeFileClass (QWrite2File)
    void writeResult2File();// save the class m_writeFileClass (QWrite2File) to the file
    QStringList searchFilterFile(QString floder_path, QStringList filers);// serch files by filter
    void SimpleSPP(QVector < SatlitData > &prevEpochSatlitData, QVector < SatlitData > &epochSatlitData, double *spp_pos);// spp for Few corrections
    void Obtaining_equation( QVector< SatlitData > &currEpoch, double *ApproxRecPos, MatrixXd &mat_B, VectorXd &Vct_L, MatrixXd &mat_P, bool isSmoothRange = false);// get Observation equation B*X = L~(P)
    void reciveClkRapaire(QVector< SatlitData > &prevEpochSatlitData, QVector< SatlitData > &epochSatlitData);
    // next function is for GUI
    void autoScrollTextEdit(QTextEdit *textEdit,QString &add_text);

    ////PPP-AR 2020.11.01 23z
    //    void PPPAR(QVector< SatlitData > epochResultSatlitData,VectorXd PosofRec);

    // data section
public:
    QWrite2File m_writeFileClass;// write file class(store all epoch coordinates and corrections)
    QKalmanFilter m_KalmanClass;//kalman fillter class
    SRIFAlgorithm m_SRIFAlgorithm;// SRIF fillter class
private:
    QTextEdit *mp_QTextEditforDisplay;
    QString m_OFileName;// O file path + file name
    QString m_run_floder;// run the specified directory file
    QString m_App_floder;// executable directory
    QStringList m_Sp3FileNames;// SP3 file path + file name
    QStringList m_ClkFileNames;// CLK file path + file name
    QString m_ErpFileName;// ERP file path + file name
    QString m_Solver_Method;// m_Solver_Method value can be "SRIF" or "Kalman"
    bool m_isKinematic;// judge is Kinematic
    double m_ApproxRecPos[3];// Approximate coordinates of the station
    double m_CutAngle;// cut-off height Angle (degree)
    QString m_SatSystem;// for GPS,GLONASS,BDS, and Galieo, use the letters G,R,C, and e. to set the file system m_SatSystem:"G"(turn on the GPS system); GR":(turn on GPS+GLONASS system);" GRCE"(all turned on), etc
    QString m_TropDelay;// the tropospheric model m_TropDelay can be selected as Sass, hopfield and UNB3m
    QString m_Product;// value is "igs" or "cnt"
    QString m_PPPModel_Str;// // value is "Ion_free" or "Uncombined"
    bool m_isSmoothRange;// Whether to use phase smoothing pseudo-distance for SPP
    QString m_sys_str;// satellite system for short ('G', 'R', 'C', 'E')
    int m_sys_num;// number of satellite systems
    int multReadOFile;// each buffer O file epoch metadata (the larger the number of memory occupied, the higher the speed is relatively fast... The default 1000)
    int m_leapSeconds;// leap seconds
    double m_interval;// // Sampling rate of observation data
    bool m_isConnect, m_isConnectCNT;// determine whether the network is connected
    bool m_haveObsFile;// jugue have obsvertion file
    bool m_isRuned;// Determine whether the operation is complete.
    // various libraries are used to calculate error correction, kalman filtering and file operation
    QCmpGPST qCmpGpsT;// function library for calculating GPS time, coordinate transformation, etc
    QReadSP3 m_ReadSP3Class;// for reading and calculating satellite orbits
    QReadClk m_ReadClkClass;// read satellite clock files
    QReadOFile m_ReadOFileClass;// reads the O file class
    QTropDelay m_ReadTropClass;// reading the troposphere requires files
    QReadAnt m_ReadAntClass;// read antenna data class
    QWindUp m_WinUpClass;// phase unwinding
    QTideEffect m_TideEffectClass;// tidal effects
    FtpClient ftpClient;// ftp Dowload class
    QualityCtrl m_qualityCtrl;// Quality control class
    QPseudoSmooth m_QPseudoSmooth;// smoothed Pesudorange
    // for save plot image
    QString m_save_images_path;
    bool m_iswritre_file;
    int m_clock_jump_type;// 1 is Pseudo range jump, 2 is carrier jump
    // min flag
    int m_minSatFlag;// the minimum number of satellites required


    // add Real time write file
    QRTWrite2File m_QRTWrite2File;
    bool m_IS_MAX_OBS;// If the observed file is greater than 200MB write the file in real time
    QString m_deleteSats;
    QVector<QStringList> m_ObsTypeSet, m_Qw_Pk_LPacc;


    QString GPSDCBfilepath;
    //gbm.bia
    QString gbmbiafilepath;


    QString shadfilepath;

    QReadBIA m_getbia;

    QVector< VectorXd >ENU_REC;

    //2020.12.01 23z
    QLambda lambda;

    MyMatrix m_matrix;

    //2021.01.08 23z
    QString m_markname;

    //    MatrixXd q_zwd;

    MatrixXd obs;

    //flag
    double q_zwd_pre = 1000;
    double q_zwd_cur = 0.0;
    double q_zwd_slope_pre = -1.0;
    double q_zwd_slope_cur = 0.0;
    int infpoints = 0;
    bool flag_epoch = false;

    //analysis center
    QString AC_product;

    QString codbiafilepath;

    QStringList RPR_filepath;

    QString m_ot_UTC;

};

#endif // QPPPMODEL_H
