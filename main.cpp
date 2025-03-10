//#define EIGEN_USE_MKL_ALL
#include <QApplication>
#include <QStringList>
#include "mainwindow.h"
#include "QNewFunLib.h"
#include "QtMyTest.cpp"
#include "readconfigurefile.h"
#include "ConfigWidget.h"


int main(int argc, char *argv[])
{   
    ConfTranIni temp_configure("ARISEN.ini");
    QString Startup_interface = temp_configure.getValue("/ARISEN/Startup_interface");

    Configure conf;

    if(Startup_interface == "true")
    {
        QApplication a(argc, argv);
        MainWindow wnd;
        wnd.show();

        return a.exec();
    }

    conf = getconfigurefromini();

    if(conf.Cflag==0)
    {
        QApplication a(argc, argv);
        MainWindow wnd;
        wnd.show();

        return a.exec();
    }
    else if(conf.OperationStrategy==1) RunPPP(conf);
    else if(conf.OperationStrategy==2) RunSSDPPP(conf);
    else if(conf.OperationStrategy==3) RunPPPAR(conf);
    else if(conf.OperationStrategy==5) PPPRunBatch(conf);
    else if(conf.OperationStrategy == 6 ||conf.OperationStrategy == 7) SSDorAR_batch(conf);
    else RunSPP(conf);

    return 0;


}


