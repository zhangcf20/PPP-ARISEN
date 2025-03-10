
#ifndef QBATCHPROCESS_H
#define QBATCHPROCESS_H

#include "QPPPModel.h"
#include "QPPPBackSmooth.h"


/*
 *illustrate:
 * ObsFiles_Path store multiply obsevation data. We will make " ObsFiles_Path " dir in ObsFiles_Path.
 *
 * Example:
 * QString ObsFiles_Path = "C:\\OFiles\\";
 * QBatchProcess process_stations;
 * process_stations.Run(ObsFiles_Path);
 *
 */

class QBatchProcess
{
public:
    QBatchProcess(QStringList files_path, QTextEdit *pQTextEdit = NULL, QString Method = "Kalman", QString Satsystem = "G", QString TropDelay = "Sass",
                  double CutAngle = 10, bool isKinematic = false, QString Smooth_Str = "NoSmooth", bool isBackBatch = false,
                  QString products = "igs", QString pppmodel_t = "Ion_free");
    ~QBatchProcess();
    //isDisplayEveryEpoch represent is disply every epoch information?(ENU or XYZ)
    bool Run(bool isDisplayEveryEpoch = false);
    void getStoreAllData(QVector<PlotGUIData> &all_SationData);
    QStringList getStationNames();
    bool isRuned(){ return m_isRuned; }

    //星间单差PPP和AR的批量处理 2020.12.26 23z
    bool run_SSD_or_AR(int OperationStrategy, bool isBackBatch);

private:
    bool isDirExist(QString fullPath);
    bool distribute(QString ofile_path, QString destin_floder);
    bool isFileExist(QString fullFileName);
    void autoScrollTextEdit(QTextEdit *textEdit,QString &add_text);
private:
    QString m_mkdir_name;
    QTextEdit *mp_QTextEdit;
    QString M_ObsFiles_Path;// M_ObsFiles_Path is a floder which store multiply .O files
    QStringList m_AllStations;// all Stations names
    QVector< PlotGUIData > m_AllStationsData;// use pointer store for External data
    bool m_isRuned;
    // configure flag
    QString m_Method;
    QString m_TropDelay;
    QString m_Satsystem;
    QString m_Smooth_Str;
    QString m_Product;// value is "igs" or "cnt"
    QString m_PPPModel_Str;// // value is "Ion_free" or "Uncombined"
    bool m_isKinematic;
    bool m_isBackBatch;
    double m_CutAngle;

    QStringList RPR_filepath;

};

#endif // QBATCHPROCESS_H
