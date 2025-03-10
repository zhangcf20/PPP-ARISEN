
#ifndef QKALMANFILTER_H
#define QKALMANFILTER_H


// Kalman filter class, considering the satellite replacement, initialization of Kalman parameters

#include "QGlobalDef.h"
#include "QualityCtrl.h"
#include "MyMatrix.h"

#include "ReadBIA.h"



class QKalmanFilter:public QBaseObject
{
// function part
public:
    enum KALMAN_MODEL    {
        SPP_STATIC = 0,
        SPP_KINEMATIC = 1,
        PPP_STATIC = 2,
        PPP_KINEMATIC = 3
    };
    enum KALMAN_SMOOTH_RANGE    {
        NO_SMOOTH = 0,
        SMOOTH = 1
    };

    enum KALMAN_FILLTER {
        KALMAN_STANDARD = 0,
        KALMAN_MrOu = 1
    };

    QKalmanFilter();
	~QKalmanFilter(void);
    void initVar();// Initialize some parameters
    //F: state transition matrix, Xk_1: previous filtering value, Pk_1: previous filtering error matrix, Qk_1: previous state transition noise matrix, Bk: observation matrix,
    //Rk: observation noise matrix, Lk: observation vector
    void KalmanforStatic(MatrixXd Bk,VectorXd Lk,MatrixXd F,MatrixXd Qw,MatrixXd Rk,VectorXd &Xk_1,MatrixXd &Pk_1,QVector< SatlitData > &preEpoch,QVector< SatlitData > &currEpoch); //2020.12.07 23z
    void KalmanforStaticOu(MatrixXd Bk,VectorXd Lk,MatrixXd F,MatrixXd Qw,MatrixXd Rk,VectorXd &Xk_1,MatrixXd &Pk_1);
    bool KalmanforStatic(QVector< SatlitData > &preEpoch, QVector< SatlitData > &currEpoch, double *m_ApproxRecPos, VectorXd &Xk_1, MatrixXd &Pk_1);
    // some get data simple function
    inline VectorXd getInitXk() { return m_init_Xk; }
    inline VectorXd getXk() { return m_Xk_1; }
    inline MatrixXd getQk() {return m_Pk_1;}

    // set some configure
    void setModel(KALMAN_MODEL model_type);
    inline KALMAN_MODEL getModel() {return m_KALMAN_MODEL;}
    inline void setSmoothRange(KALMAN_SMOOTH_RANGE smooth_range) {m_KALMAN_SMOOTH_RANGE = smooth_range;}
    inline KALMAN_SMOOTH_RANGE getSmoothRange() {return m_KALMAN_SMOOTH_RANGE;}
    inline void setFilterMode(KALMAN_FILLTER filter_mode) {m_KALMAN_FILLTER = filter_mode;}
    inline KALMAN_FILLTER getFilterMode() {return m_KALMAN_FILLTER;}
    void setFilterParams(QVector<QStringList> Qw_Pk_LPacc);
private:
    void printMatrix(MatrixXd mat);// print matrix Debug
    void initKalman(QVector< SatlitData > &currEpoch,MatrixXd &B,VectorXd &L);// kalman initialization
    void initKalman_NoCombination(QVector< SatlitData > &currEpoch,MatrixXd &B,VectorXd &L);
    void changeKalmanPara(QVector< SatlitData > &preEpoch,QVector< SatlitData > &epochSatlitData,QVector< int >oldPrnFlag,MatrixXd B,VectorXd L);
    void changeKalmanPara_NoCombination(QVector< SatlitData > &epochSatlitData, QVector< int >oldPrnFlag , int preEpochLen);
    void Obtaining_equation( QVector< SatlitData > &currEpoch, double *m_ApproxRecPos, MatrixXd &mat_B, VectorXd &Vct_L,
                             MatrixXd &mat_P);// get observation equation
    void Obtaining_equation_NoCombination(QVector< SatlitData > &currEpoch, double *m_ApproxRecPos, MatrixXd &mat_B, VectorXd &Vct_L,
                                 MatrixXd &mat_P);
    void ls_solver(QVector< SatlitData > &currEpoch, double *m_ApproxRecPos);// use least square method solver B*X = L
    // the residual error after Kalman filtering is used as the gross error detection, and the circular filtering with gross error is kicked out.
    bool isSatelliteChange(QVector< SatlitData > &preEpoch,QVector< SatlitData > &currEpoch, QVector< int > &oldPrnFlag);
    void updateRk(QVector< SatlitData > &currEpoch, int B_len, QVector< int > oldPrnFlag);// update Rk(Observation Covariance)
    void updateRk_NoCombination(QVector< SatlitData > &currEpoch, int B_len);
    void filter(QVector< SatlitData > &preEpoch, QVector< SatlitData > &currEpoch, VectorXd &X, MatrixXd &P);
// variable section
public:

private:
    bool isInitPara;// determines whether the first epoch is initialized
	MatrixXd m_Fk_1,m_Pk_1,m_Qwk_1,m_Rk_1;
    VectorXd m_Xk_1, m_init_Xk;// are dX,dY,dZ,dT(zenith tropospheric residual),dVt(receiver clock difference), N1,N2... Nm(ambiguity)

    //Used to re-constrain the current epoch after the ambiguity is fixed 2020.12.10 23z
//    MatrixXd F_pre, P_pre, Q_pre, R_pre;
//    VectorXd X_pre;
    MatrixXd P_pre;
    VectorXd X_pre;

    bool m_VarChang;// marks whether the matrix changes in the next filtering period
    MyMatrix m_matrix;// print matrix to file
    double m_SPP_Pos[3];
    QualityCtrl m_qualityCtrl;
    KALMAN_MODEL m_KALMAN_MODEL;
    KALMAN_SMOOTH_RANGE m_KALMAN_SMOOTH_RANGE;
    KALMAN_FILLTER m_KALMAN_FILLTER;
    int m_const_param;// Invariant parameters in filtering
    int m_sys_num;
    QString m_sys_str;
    double m_LP_whight;// Carrier and Pseudo Range Weight Ratio
    double m_xyz_dynamic_Qw, m_zwd_Qw, m_clk_Qw, m_amb_Qw, m_ion_Qw;// Transfer of noise (Qw)
    double m_xyz_dynamic_Pk, m_zwd_Pk, m_clk_Pk, m_amb_Pk, m_ion_Pk;// Initial covariance (Pk)

    double m_amb_SSD;

    //SSD conversion matrix
    MatrixXd tran_mat;

    QReadBIA m_snx_initialpos;

public:
    //obtain SSD B，L 2021.03.23 23z
    MatrixXd getSSDBL(MatrixXd &B, VectorXd &L, MatrixXd B_part, VectorXd L_temp, int num_single_sys);

    int sys_num_cur;
    QString sys_cur;

    int interval_cur;



};

#endif
