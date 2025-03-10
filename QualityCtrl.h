
#ifndef QUALITYCTRL_H
#define QUALITYCTRL_H
#include "QGlobalDef.h"
#include "QCmpGPST.h"
#include "MyMatrix.h"


using namespace Eigen;

class QualityCtrl: public QBaseObject
{
public:
    QualityCtrl();
    // use clk detect gross error
    bool VtPVCtrl_CLK(QVector < SatlitData > &epochSatlitData, double *predict_pos, VectorXd &del_flag);
    bool VtPVCtrl_CLKA(QVector < SatlitData > &epochSatlitData, double *predict_pos);
    // mat_B * X = mat_L; mat_P; del_flag store delete erro flag
    bool VtPVCtrl_Filter_LC(MatrixXd mat_B, VectorXd vec_L, VectorXd vec_X, VectorXd &del_flag, int sat_len, QVector< double > LP_threshold);
    bool VtPVCtrl_Filter_LC_NoCombination(MatrixXd mat_B, VectorXd vec_L, VectorXd vec_X, VectorXd &del_flag , int sat_len, double *L12P12_threshold=NULL);
    bool VtPVCtrl_Filter_C(MatrixXd mat_B, VectorXd vec_L, VectorXd vec_X, VectorXd &del_flag, int sat_len);
    bool VtPVCtrl_C(MatrixXd mat_B, VectorXd vec_L, MatrixXd mat_P, VectorXd &del_flag, int sat_len);
    bool VtPVCtrlA_C(MatrixXd mat_B, VectorXd vec_L, MatrixXd mat_P, VectorXd &del_flag, int sat_len);
    bool solver_LS(MatrixXd mat_B, VectorXd vec_L, MatrixXd mat_P, VectorXd del_flag, VectorXd &vec_X);
    bool deleteMat(MatrixXd &mat_B, VectorXd del_cols, VectorXd del_rows);// delete Matrix Rows
    bool addZeroMat(MatrixXd &mat_B, int add_row_index, int add_col_index);
    bool VtPVCtrl_Filter_newIGG(MatrixXd mat_B, VectorXd vec_L, VectorXd vec_X, VectorXd &del_flag , int sat_len);
    QVector<int> QCSatClk(QVector<SatlitData> prevEpochSatlitData, QVector<SatlitData> epochSatlitData);
    void CmpSatClkRate(const QVector<SatlitData> &prevEpochSatlitData, QVector<SatlitData> &epochSatlitData);// Calculating the rate of change of satellite clock difference
    MatrixXd GenerateData();

    //IGG3
    void IGG3Algorithm(MatrixXd mat_B, VectorXd vec_L, VectorXd vec_X, MatrixXd &mat_P, int sat_len);

private:
    void Obtaining_equation(QVector< SatlitData > &currEpoch, double *ApproxRecPos, MatrixXd &mat_B, VectorXd &Vct_L, MatrixXd &mat_P, bool isSmoothRange = false);
    void sort_vec(const VectorXd& vec, VectorXd& sorted_vec,  VectorXi& ind);
    QCmpGPST m_QCmpGPST;// function library for calculating GPS time, coordinate transformation, etc

    MyMatrix m_matrix;
};

#endif // QUALITYCTRL_H
