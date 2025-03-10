
#ifndef READCONFIGUREFILE_H
#define READCONFIGUREFILE_H

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

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <qmath.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCursor>
#include <QIcon>
#include <QPoint>
#include <QString>
#include <QAction>
#include <QProcess>

#include "QPPPModel.h"
#include "QPPPBackSmooth.h"
#include "QSPPModel.h"
#include "QBatchProcess.h"
#include "QtPPPGUI/qtplot.h"

typedef struct{//23Z 2020.10.16

    QString FilePath;//Station file path
    QString TropDelay;//Tropospheric model
    QString Method;//Filter model
    double CutAngle;//Cut-off angle
    QString PPPModel;//PPP model
    bool Kinematic;//
    QString Products;//final/cnt
    QString SatelliteSys;//GCRE
    QString PPPSmooth;//
    bool PPPBack;//
    QString SPPModel;//
    QString SPPSmooth;//SPP model（P_IF/PL_IF）
    int OperationStrategy;//IF/SSD/ARC/ARF/Batch

    int Cflag;//falg，Cflag=0，Configuration file error

    double randomwalk_pos = 0.0;//Random walk for kinematic filtering
    bool keepornot = false;//Whether to save the state of Kalman filter
    bool initializeornot = false;//Whether to initialize according to state
    QString path_keep;//Path to save Kalman filter state
    QString path_initialize;//Initialize the Kalman filter path according to the state
    double randomwalk_zwd = 0.0;//Random walk for zwd
    bool whitenoise_pos = false;//White noise model of kinematic filtering

    QString path_product;
    QString path_result;

} Configure;

//Read configuration file
Configure ReadConfigureFile(QString txtPath);//23Z 2020.10.16

//PPP
void RunPPP(Configure temp_con, QTextEdit *pQTextEdit = NULL);

//PPP batch
void PPPRunBatch(Configure temp_con, QTextEdit *pQTextEdit = NULL);

//SSDPPP 2020.10.22 by23z
void RunSSDPPP(Configure temp_con, QTextEdit *pQTextEdit = NULL);

//PPP-AR 2020.11.30 by23z
void RunPPPAR(Configure temp_con, QTextEdit *pQTextEdit = NULL);

//SPP batch
void RunSPP(Configure temp_con, QTextEdit *pQTextEdit = NULL);

// 2020.12.26 23z
void SSDorAR_batch(Configure temp_con, QTextEdit *pQTextEdit = NULL);

//2021.06.16 23z
Configure getconfigurefromini();

//2021.06.27 23z
QVector<QStringList> getConfObsType();


#endif // READCONFIGUREFILE_H
