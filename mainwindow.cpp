#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "readconfigurefile.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_ConfigWidget(parent),
    m_ConfTranIni("ARISEN.ini")
{
    ui->setupUi(this);
    // init Widget and add it
    initWindow();
    //    setAttribute(Qt::WA_DeleteOnClose, true);
}

MainWindow::~MainWindow()
{
    if(m_mutiply_data.length() > 0)
        m_mutiply_data.clear();
    delete ui;
}
void MainWindow::closeEvent(QCloseEvent *e)
{
    exit(0);// exit all app processes and threads
}

void MainWindow::AboutApp()
{
    QMessageBox::information(this, tr("About"),
                             tr("PPP-ARISEN is written by Zhang Chengfeng.\nE-mail: zhangcf@apm.ac.cn\n          guoaizhi@whigg.ac.cn"));
}


void MainWindow::initWindow()
{
    m_station_path = "/media/zcf/Rinex";
    ui->textEdit_FilePath_obs->setText(m_station_path);
    m_station_path = "/media/zcf/Product";
    ui->textEdit_FilePath_pro->setText(m_station_path);
    m_station_path = "/media/zcf/Result";
    ui->textEdit_FilePath_res->setText(m_station_path);

    m_isRuned = false;
    m_isRunedBatch = false;
    m_Display_Max_line = 99999;
    mp_qtPlot = NULL;
    // fix windows
    setFixedSize(this->width(), this->height());
    setWindowIcon(QIcon("ARISEN.ico"));
    setWindowTitle("ARISEN v1.0");
    // connect signal to slots
    // pushButon
    connect(ui->pushButton_Selectobs, SIGNAL(clicked(bool)), this, SLOT(selectFilePath()));

    connect(ui->pushButton_selectfile, SIGNAL(clicked(bool)), this, SLOT(selectcsv()));
    connect(ui->pushButton_selectkeep, SIGNAL(clicked(bool)), this, SLOT(selectpathtokeep()));

    connect(ui->pushButton_Selectpro, SIGNAL(clicked(bool)), this, SLOT(selectpath_pro()));
    connect(ui->pushButton_Selectres, SIGNAL(clicked(bool)), this, SLOT(selectpath_res()));

    ui->groupBox_6->setEnabled(false);
    ui->pushButton_selectfile->setEnabled(false);

    connect(ui->comboBox_PPPmodel, SIGNAL(currentTextChanged(QString)), this, SLOT(contral_seismology()));
    connect(ui->comboBox_initialize, SIGNAL(currentTextChanged(QString)), this, SLOT(contral_selectfile()));

    ui->comboBox_kinmodel->setEnabled(false);
    ui->lineEdit_randomwalk->setEnabled(false);

    connect(ui->comboBox_PPPmodel, SIGNAL(currentTextChanged(QString)), this, SLOT(contral_kin()));
    connect(ui->comboBox_kinmodel, SIGNAL(currentTextChanged(QString)), this, SLOT(contral_rand()));

    ui->lineEdit_ointerval->setEnabled(false);
    ui->lineEdit_used->setEnabled(false);
    connect(ui->checkBox_sampling, SIGNAL(clicked(bool)), this, SLOT(contral_sampling()));

    ui->groupBox_5->setEnabled(false);
    connect(ui->comboBox_strategy, SIGNAL(currentTextChanged(QString)), this, SLOT(contral_CFZ_IP()));

    connect(ui->pushButton_run, SIGNAL(clicked(bool)), this, SLOT(run()));

    // menuBar
    m_otherMenu = menuBar()->addMenu("&Tools");
    // about
    QAction *AboutAct = new QAction(tr("&About"),this);
    AboutAct->setStatusTip(tr("ARISEN is written by ZhangChengfeng."));
    AboutAct->setIcon(QIcon("./images/about.ico"));
    connect(AboutAct,SIGNAL(triggered()),this,SLOT(AboutApp()));
    m_otherMenu->addAction(AboutAct);

    // Configure
    QAction *configureAct = new QAction(tr("&Configure"),this);
    configureAct->setStatusTip(tr("Configure ARISEN."));
    configureAct->setIcon(QIcon("./images/config.ico"));
    connect(configureAct,SIGNAL(triggered()), this,SLOT(WriteShowConfWnd()));
    m_otherMenu->addAction(configureAct);

    // status tip
    setStatusTip("It is running.");
}

void MainWindow::WriteShowConfWnd()
{
    // get Mainwindow configure
    QString obspath = ui->textEdit_FilePath_obs->text(),
            trop_model = ui->comboBox_TropDelay->currentText(),
            CutAngle_Str = ui->lineEdit_Angle->text(),
            sats_sys = "",
            initialize = ui->comboBox_initialize->currentText(),
            keeppath = ui->textEdit_FilePath_keep->text(),
            inifilepath = ui->textEdit_FilePath_file->text();

    if(ui->checkBox_GPS->isChecked()) sats_sys = sats_sys + "G";
    if(ui->checkBox_GLONASS->isChecked()) sats_sys = sats_sys + "R";
    if(ui->checkBox_BDS->isChecked()) sats_sys = sats_sys + "C";
    if(ui->checkBox_GAlieo->isChecked()) sats_sys = sats_sys + "E";

    QString strategy = ui->comboBox_strategy->currentText(),
            filterdirection = ui->comboBox_filterdirection->currentText(),
            kinmodel = ui->comboBox_kinmodel->currentText(),
            randomwalk_str = ui->lineEdit_randomwalk->text();

    QString interval_original = ui->lineEdit_ointerval->text(),
            interval_used = ui->lineEdit_used->text(),
            OTh = ui->lineEdit_OTh->text(),
            OTm = ui->lineEdit_OTm->text(),
            OTs = ui->lineEdit_OTs->text(),
            before_OT = ui->lineEdit_epochsbefore->text(),
            after_OT = ui->lineEdit_epochsafter->text(),
            ppp_model = ui->comboBox_PPPmodel->currentText(),
            MF = ui->comboBox_MF->currentText();

    // set Mainwindow configure to m_ConfigWidget
    QStringList allConf;
    allConf.append(obspath);        allConf.append(trop_model);      allConf.append(CutAngle_Str);
    allConf.append(sats_sys);       allConf.append(initialize);      allConf.append(keeppath);
    allConf.append(inifilepath);    allConf.append(strategy);        allConf.append(filterdirection);
    allConf.append(kinmodel);       allConf.append(randomwalk_str);  allConf.append(interval_original);
    allConf.append(interval_used);  allConf.append(OTh);             allConf.append(OTm);
    allConf.append(OTs);            allConf.append(before_OT);       allConf.append(after_OT);
    allConf.append(ppp_model);      allConf.append(MF);

    m_ConfigWidget.setParamerter(allConf);
    // show m_ConfigWidget
    m_ConfigWidget.show();
}

void MainWindow::selectcsv()
{
    QFileDialog fileDialog;
    fileDialog.setFileMode(QFileDialog::Directory);

    if(ui->comboBox_initialize->currentText() == "SNX")
        m_station_path = fileDialog.getOpenFileName(this, "Open Directory","*.snx");
    if(ui->comboBox_initialize->currentText() == "CSV")
        m_station_path = fileDialog.getOpenFileName(this, "Open Directory","*.csv");


    if(m_station_path.isEmpty())
    {
        QString WarngInfo = "Failed to select file";
        WaringBox(WarngInfo);
        ui->textEdit_FilePath_file->setText("");
        autoScrollTextEdit(ui->textEdit_Display, WarngInfo);
    }
    else
    {
        QString Display = "Select file:  " + m_station_path;
        ui->textEdit_FilePath_file->setText(m_station_path);
        autoScrollTextEdit(ui->textEdit_Display, Display);
    }
}

void MainWindow::selectpathtokeep()
{
    QFileDialog fileDialog;
    fileDialog.setFileMode(QFileDialog::Directory);
    m_station_path = fileDialog.getExistingDirectory(this, "Open Directory", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(m_station_path.isEmpty())
    {
        QString WarngInfo = "Failed to select file";
        WaringBox(WarngInfo);
        ui->textEdit_FilePath_keep->setText("");
        autoScrollTextEdit(ui->textEdit_Display, WarngInfo);
    }
    else
    {
        QString Display = "Select path to save:  " + m_station_path;
        ui->textEdit_FilePath_keep->setText(m_station_path);
        autoScrollTextEdit(ui->textEdit_Display, Display);
    }
}

void MainWindow::run()
{
    QString obspath = ui->textEdit_FilePath_obs->text(),
            trop_model = ui->comboBox_TropDelay->currentText(),
            CutAngle_Str = ui->lineEdit_Angle->text(),
            sats_sys = "",
            initialize = ui->comboBox_initialize->currentText(),
            savepath = ui->textEdit_FilePath_keep->text(),
            inipath = ui->textEdit_FilePath_file->text(),
            propath = ui->textEdit_FilePath_pro->text(),
            respath = ui->textEdit_FilePath_res->text();

    if(ui->checkBox_GPS->isChecked()) sats_sys = sats_sys + "G";
    if(ui->checkBox_GLONASS->isChecked()) sats_sys = sats_sys + "R";
    if(ui->checkBox_BDS->isChecked()) sats_sys = sats_sys + "C";
    if(ui->checkBox_GAlieo->isChecked()) sats_sys = sats_sys + "E";

    QString strategy = ui->comboBox_strategy->currentText(),
            filterdirection = ui->comboBox_filterdirection->currentText(),
            kinmodel = ui->comboBox_kinmodel->currentText(),
            randomwalk_str = ui->lineEdit_randomwalk->text();

    QString interval_original = ui->lineEdit_ointerval->text(),
            interval_used = ui->lineEdit_used->text(),
            OTh = ui->lineEdit_OTh->text(),
            OTm = ui->lineEdit_OTm->text(),
            OTs = ui->lineEdit_OTs->text(),
            before_OT = ui->lineEdit_epochsbefore->text(),
            after_OT = ui->lineEdit_epochsafter->text(),
            PPPmodel = ui->comboBox_PPPmodel->currentText(),
            trop_map = ui->comboBox_MF->currentText(),
            batch = "",
            sampling = "",
            CSI_ZWDV = ui->CFZ_IP->text();

    if(ui->checkBox_batch->isChecked()) batch = "true";
    if(ui->checkBox_sampling->isChecked()) sampling = "true";

    // set Mainwindow configure to m_ConfigWidget
    QStringList allConf;
    allConf.append(obspath);        allConf.append(trop_model);      allConf.append(CutAngle_Str);
    allConf.append(sats_sys);       allConf.append(initialize);      allConf.append(savepath);
    allConf.append(inipath);        allConf.append(strategy);        allConf.append(filterdirection);
    allConf.append(kinmodel);       allConf.append(randomwalk_str);  allConf.append(interval_original);
    allConf.append(interval_used);  allConf.append(OTh);             allConf.append(OTm);
    allConf.append(OTs);            allConf.append(before_OT);       allConf.append(after_OT);
    allConf.append(PPPmodel);       allConf.append(trop_map);        allConf.append(batch);
    allConf.append(sampling);       allConf.append(CSI_ZWDV);
    allConf.append(propath);        allConf.append(respath);

    QJsonArray myconfArryJson;
    QJsonObject allConfig;

    //add conf
    allConfig.insert("Rinexdir", allConf.at(0).toUtf8().data());         allConfig.insert("Trop_model", allConf.at(1).toUtf8().data());
    allConfig.insert("Cut_angle", allConf.at(2).toUtf8().data());        allConfig.insert("Satellites_system", allConf.at(3).toUtf8().data());
    allConfig.insert("Initialize", allConf.at(4).toUtf8().data());       allConfig.insert("Path_save", allConf.at(5).toUtf8().data());
    allConfig.insert("Path_ini", allConf.at(6).toUtf8().data());         allConfig.insert("Strategy", allConf.at(7).toUtf8().data());
    allConfig.insert("Filter_direction", allConf.at(8).toUtf8().data()); allConfig.insert("Kin_model", allConf.at(9).toUtf8().data());
    allConfig.insert("Randomwalk", allConf.at(10).toUtf8().data());      allConfig.insert("Interval_original", allConf.at(11).toUtf8().data());
    allConfig.insert("Interval_used", allConf.at(12).toUtf8().data());   allConfig.insert("OTh", allConf.at(13).toUtf8().data());
    allConfig.insert("OTm", allConf.at(14).toUtf8().data());             allConfig.insert("OTs", allConf.at(15).toUtf8().data());
    allConfig.insert("Before_OT", allConf.at(16).toUtf8().data());       allConfig.insert("After_OT", allConf.at(17).toUtf8().data());
    allConfig.insert("PPP_model", allConf.at(18).toUtf8().data());       allConfig.insert("Trop_map", allConf.at(19).toUtf8().data());
    allConfig.insert("Batch", allConf.at(20).toUtf8().data());           allConfig.insert("Sampling", allConf.at(21).toUtf8().data());
    allConfig.insert("CSI_ZWDV", allConf.at(22).toUtf8().data());

    allConfig.insert("Productdir", allConf.at(23).toUtf8().data());
    allConfig.insert("Resultdir", allConf.at(24).toUtf8().data());


    // add all config to array
    myconfArryJson.append(allConfig);

    QString iniFileName = m_ConfTranIni.getFileName(), jsonFileName = "ARISEN.json";
    QStringList iniList = iniFileName.split(".");
    if(iniList.length() > 1) jsonFileName = iniList.at(0) + ".json";
    ConfTranIni::writeJson2Ini(m_ConfTranIni.getFileName(), myconfArryJson);
    ConfTranIni::writeJson2File(jsonFileName, myconfArryJson);

    //run
    Configure conf;
    conf = getconfigurefromini();

    if(conf.OperationStrategy==1) RunPPP(conf, ui->textEdit_Display);
    else if(conf.OperationStrategy==2) RunSSDPPP(conf, ui->textEdit_Display);
    else if(conf.OperationStrategy==3) RunPPPAR(conf, ui->textEdit_Display);
    else if(conf.OperationStrategy==5) PPPRunBatch(conf, ui->textEdit_Display);
    else if(conf.OperationStrategy == 6 ||conf.OperationStrategy == 7) SSDorAR_batch(conf, ui->textEdit_Display);
    else RunSPP(conf);
}

void MainWindow::contral_seismology()
{
    if(ui->comboBox_PPPmodel->currentIndex() == 2)
        ui->groupBox_6->setEnabled(true);
    else
        ui->groupBox_6->setEnabled(false);
}

void MainWindow::contral_selectfile()
{
    if(ui->comboBox_initialize->currentIndex() != 0)
        ui->pushButton_selectfile->setEnabled(true);
    else
        ui->pushButton_selectfile->setEnabled(false);
}

void MainWindow::contral_kin()
{
    if(ui->comboBox_PPPmodel->currentIndex() != 0)
    {
        ui->comboBox_kinmodel->setEnabled(true);
        if(ui->comboBox_kinmodel->currentIndex() == 1)
            ui->lineEdit_randomwalk->setEnabled(true);
    }
    else
        ui->comboBox_kinmodel->setEnabled(false), ui->lineEdit_randomwalk->setEnabled(false);
}

void MainWindow::contral_rand()
{
    if(ui->comboBox_kinmodel->currentIndex() == 1 && ui->comboBox_kinmodel->isEnabled())
        ui->lineEdit_randomwalk->setEnabled(true);
    else
        ui->lineEdit_randomwalk->setEnabled(false);
}

void MainWindow::contral_sampling()
{
    if(ui->checkBox_sampling->isChecked())
        ui->lineEdit_ointerval->setEnabled(true), ui->lineEdit_used->setEnabled(true);
    else
        ui->lineEdit_ointerval->setEnabled(false), ui->lineEdit_used->setEnabled(false);
}

void MainWindow::contral_CFZ_IP()
{
    if(ui->comboBox_strategy->currentIndex() == 2 || ui->comboBox_strategy->currentIndex() == 3)
        ui->groupBox_5->setEnabled(true);
    else
        ui->groupBox_5->setEnabled(false);
}



void MainWindow::selectpath_pro()
{
    QFileDialog fileDialog;
    fileDialog.setFileMode(QFileDialog::Directory);
    m_station_path = fileDialog.getExistingDirectory(this, "Open Directory", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(m_station_path.isEmpty())
    {
        QString WarngInfo = "Failed to select file";
        WaringBox(WarngInfo);
        ui->textEdit_FilePath_pro->setText("");
        autoScrollTextEdit(ui->textEdit_Display, WarngInfo);
    }
    else
    {
        QString Display = "Product Dir:  " + m_station_path;
        ui->textEdit_FilePath_pro->setText(m_station_path);
        autoScrollTextEdit(ui->textEdit_Display, Display);
    }
}



void MainWindow::selectpath_res()
{
    QFileDialog fileDialog;
    fileDialog.setFileMode(QFileDialog::Directory);
    m_station_path = fileDialog.getExistingDirectory(this, "Open Directory", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(m_station_path.isEmpty())
    {
        QString WarngInfo = "Failed to select file";
        WaringBox(WarngInfo);
        ui->textEdit_FilePath_res->setText("");
        autoScrollTextEdit(ui->textEdit_Display, WarngInfo);
    }
    else
    {
        QString Display = "Result Dir:  " + m_station_path;
        ui->textEdit_FilePath_res->setText(m_station_path);
        autoScrollTextEdit(ui->textEdit_Display, Display);
    }
}


void MainWindow::plotAllRes()
{
    //    if(m_mutiply_data.length() > 0 && m_isRunedBatch && ui->comboBox_plotwhat->count() > 0 )
    //    {// mutiply stations
    //        // find QComoBox select station
    //        QString plotStationName = ui->comboBox_plotwhat->currentText();
    //        int plot_flag = 0;
    //        for(int i = 0;i < m_mutiply_names.length();i++)
    //        {
    //            QString temp_station_name = m_mutiply_names.at(i);
    //            if(temp_station_name.compare(plotStationName) == 0)
    //            {
    //                plot_flag = i;
    //                break;
    //            }
    //        }
    //        // plot select station
    //        PlotGUIData plot_station_data = m_mutiply_data.at(plot_flag);
    //        plotSigleStation(plot_station_data);
    //    }
    //    else if(m_isRuned)
    //    {// single station
    //        plotSigleStation(m_single_data);
    //    }
    //    else
    //    {
    //        WaringBox("There is no data.");
    //        return ;
    //    }
}

void MainWindow::plotSigleStation(PlotGUIData &station_data)
{
    //    int revDataLen = station_data.X.length();
    //    int wnd_dev_pix = 20;// Window deviates from pixels
    //    QPoint tempPos;
    //    // juge QWrite2File have data
    //    if(revDataLen == 0) return ;
    //    // plot ENU
    //    QVector< double > xAixsData;
    //    // get axis of x and use ( station_data.X - station_data.X(end) )
    //    double true_pos[3] = {0};
    //    true_pos[0] = station_data.X.at(revDataLen-1);
    //    true_pos[1] = station_data.Y.at(revDataLen-1);
    //    true_pos[2] = station_data.Z.at(revDataLen-1);

    //    PlotGUIData station_data_copy;
    //    for(int i = 0;i < revDataLen;i++)
    //    {
    //        if(station_data.X[i] !=0 && station_data.spp_X[i] !=0)
    //        {
    //            xAixsData.append(i);
    //            station_data_copy.X.append(station_data.X[i] - true_pos[0]);
    //            station_data_copy.Y.append(station_data.Y[i] - true_pos[1]);
    //            station_data_copy.Z.append(station_data.Z[i] - true_pos[2]);
    //            station_data_copy.spp_X.append(station_data.spp_X[i] - true_pos[0]);
    //            station_data_copy.spp_Y.append(station_data.spp_Y[i] - true_pos[1]);
    //            station_data_copy.spp_Z.append(station_data.spp_Z[i] - true_pos[2]);
    //            station_data_copy.clockData.append(station_data.clockData[i]);
    //            station_data_copy.ZTD_W.append(station_data.ZTD_W[i]);
    //        }
    //    }
    //    station_data_copy.save_file_path = station_data.save_file_path;

    //    //int save_image_width = 800, save_image_hight = 495;
    //    QString save_image_folder = station_data.save_file_path + "images"+ PATHSEG;

    //    if(ui->comboBox_plotwhat->currentText().contains("pos"))
    //    {
    //        // plot PPP pos
    //        QVector< QVector< double > > XX, YY;
    //        QVector< QString > XY_Names;
    //        XX.append(xAixsData); YY.append(station_data_copy.X);
    //        XX.append(xAixsData); YY.append(station_data_copy.Y);
    //        XX.append(xAixsData); YY.append(station_data_copy.Z);
    //        XY_Names.append("filter_dX"); XY_Names.append("filter_dY"); XY_Names.append("filter_dZ");
    //        mp_qtPlot = new QtPlot();
    //        mp_qtPlot->setAttribute(Qt::WA_DeleteOnClose, true);
    //        mp_qtPlot->qtPlot2D(XX, YY, XY_Names, "Epoch", "Unit(m)");
    //        mp_qtPlot->show();
    ////        if(isDirExist(save_image_folder))
    ////        {// save image
    ////            QString file_name = "ppp_dXYZ.png";
    ////            mp_qtPlot->savePng(save_image_folder + file_name, save_image_width, save_image_hight);
    ////        }
    //        // get pos and move window
    //        tempPos = mp_qtPlot->pos();
    //        mp_qtPlot->move(tempPos.x()+0*wnd_dev_pix, tempPos.y()+0*wnd_dev_pix);
    //        mp_qtPlot->resize(this->width(), (int) (this->width() * 0.618));
    //    }



    //    if(ui->comboBox_plotwhat->currentText().contains("spp"))
    //    {
    //        // plot SPP pos
    //        QVector< QVector< double > > spp_XX, spp_YY;
    //        QVector< QString > spp_Names;
    //        spp_XX.append(xAixsData); spp_YY.append(station_data_copy.spp_X);
    //        spp_XX.append(xAixsData); spp_YY.append(station_data_copy.spp_Y);
    //        spp_XX.append(xAixsData); spp_YY.append(station_data_copy.spp_Z);
    //        spp_Names.append("spp_dX"); spp_Names.append("spp_dY"); spp_Names.append("spp_dZ");
    //        mp_qtPlot = new QtPlot();
    //        mp_qtPlot->setAttribute(Qt::WA_DeleteOnClose, true);
    //        mp_qtPlot->qtPlot2D(spp_XX, spp_YY, spp_Names, "Epoch", "Unit(m)");
    //        mp_qtPlot->show();
    //        // get pos and move window
    //        tempPos = mp_qtPlot->pos();
    //        mp_qtPlot->move(tempPos.x()+ 1*wnd_dev_pix, tempPos.y()+ 1*wnd_dev_pix);
    //        mp_qtPlot->resize(this->width(), (int) (this->width() * 0.618));
    ////        if(isDirExist(save_image_folder))
    ////        {// save image
    ////            QString file_name = "spp_dXYZ.png";
    ////            mp_qtPlot->savePng(save_image_folder + file_name, save_image_width, save_image_hight);
    ////        }
    //    }

    //    if(ui->comboBox_plotwhat->currentText().contains("clk"))
    //    {
    //        // plot Clock
    //        QVector< QVector< double > > xClock, YClock;
    //        QVector< QString > clock_Names;
    //        xClock.append(xAixsData); YClock.append(station_data_copy.clockData);
    //        clock_Names.append("Receiver_Clock");
    //        mp_qtPlot = new QtPlot();
    //        mp_qtPlot->setAttribute(Qt::WA_DeleteOnClose, true);
    //        mp_qtPlot->qtPlot2D(xClock, YClock, clock_Names, "Epoch", "Unit(m)");
    //        mp_qtPlot->show();
    //        // get pos and move window
    //        tempPos = mp_qtPlot->pos();
    //        mp_qtPlot->move(tempPos.x()+2*wnd_dev_pix, tempPos.y()+2*wnd_dev_pix);
    //        mp_qtPlot->resize(this->width(), (int) (this->width() * 0.618));
    ////        if(isDirExist(save_image_folder))
    ////        {// save image
    ////            QString file_name = "Receiver_Clock.png";
    ////            mp_qtPlot->savePng(save_image_folder + file_name, save_image_width, save_image_hight);
    ////        }
    //    }

    //    if(ui->comboBox_plotwhat->currentText().contains("ZTD"))
    //    {
    //        // plot zwd
    //        QVector< QVector< double > > xZWD, YZWD;
    //        QVector< QString > ZWD_Names;
    //        xZWD.append(xAixsData); YZWD.append(station_data_copy.ZTD_W);
    //        ZWD_Names.append("ZTD");
    //        mp_qtPlot = new QtPlot();
    //        mp_qtPlot->setAttribute(Qt::WA_DeleteOnClose, true);
    //        mp_qtPlot->qtPlot2D(xZWD, YZWD, ZWD_Names, "Epoch", "Unit(m)");
    //        mp_qtPlot->show();
    //        // get pos and move window
    //        tempPos = mp_qtPlot->pos();
    //        mp_qtPlot->move(tempPos.x()+3*wnd_dev_pix, tempPos.y()+3*wnd_dev_pix);
    //        mp_qtPlot->resize(this->width(), (int) (this->width() * 0.618));
    ////        if(isDirExist(save_image_folder))
    ////        {// save image
    ////            QString file_name = "ZTD.png";
    ////            mp_qtPlot->savePng(save_image_folder + file_name, save_image_width, save_image_hight);
    ////        }
    //    }
}

//void MainWindow::RunPPPBatch()
//{
//    m_station_path = ui->textEdit_FilePath->text();
//    // clear all station data
//    m_isRuned = false;
//    m_isRunedBatch = false;
//    clearPlotGUIData(m_single_data);// clear old data
//    m_mutiply_data.clear();
//    m_mutiply_names.clear();
//    //ui->comboBox_plotwhat->clear();
//    // QComoBox
//    QString TropDelay = ui->comboBox_TropDelay->currentText(),
//            //Method = ui->comboBox_Method->currentText(),
//            CutAngle_Str = ui->lineEdit_Angle->text(),
//            PPPModel_Str = ui->comboBox_PPPModel->currentText();
//    double CutAngle = CutAngle_Str.toDouble();
//    // QCheckBox
//    QString SatSystem = "";
//    bool Kinematic = false;
//    if(ui->checkBox_GPS->isChecked())
//        SatSystem.append("G");
//    if(ui->checkBox_GLONASS->isChecked())
//        SatSystem.append("R");
//    if(ui->checkBox_GAlieo->isChecked())
//        SatSystem.append("E");
//    if(ui->checkBox_BDS->isChecked())
//        SatSystem.append("C");
////    if(ui->checkBox_Kinematic->isChecked())
////        Kinematic = true;
////    QString Smooth_Str = ui->comboBox_PPP_SMOOTH->currentText();
////    QString m_Products = "igs";
////    if(ui->radioButton_igs->isChecked())
////        m_Products = "igs";
////    else if(ui->radioButton_cnt->isChecked())
////        m_Products = "cnt";

//    // run batch stations
//    if(!m_station_path.isEmpty())
//    {//2021.03.31 23z
//        ui->textEdit_Display->clear();// clear QTextEdit
////        bool isBackBatch = ui->checkBox_Back->isChecked();

//        QBatchProcess batchPPP(m_station_path, ui->textEdit_Display, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, isBackBatch, m_Products, PPPModel_Str);

//        if(PPPModel_Str == "IF")
//            batchPPP.Run(false);
//        if(PPPModel_Str == "SSD")
//            batchPPP.run_SSD_or_AR(6,true);
//        if(PPPModel_Str == "AR")
//            batchPPP.run_SSD_or_AR(7,true);

//        m_isRunedBatch = batchPPP.isRuned();
//        if(m_isRunedBatch)
//        {
//            //ui->comboBox_plotwhat->clear();
//            batchPPP.getStoreAllData(m_mutiply_data);// if you want store data before Run set setStoreAllData.
//            m_mutiply_names = batchPPP.getStationNames();
//            //ui->comboBox_plotwhat->addItems(m_mutiply_names);
//        }
//        else
//        {
//            //ui->comboBox_plotwhat->clear();
//            m_mutiply_data.clear();
//            m_mutiply_names.clear();
//        }
//    }
//    else
//    {
//        WaringBox("Please select obsevation floder.");
//    }
//}

//void MainWindow::RunPPP()
//{
//    m_station_path = ui->textEdit_FilePath->text();
//    // clear mutiply stations
//    m_isRuned = false;
//    m_isRunedBatch = false;
//    clearPlotGUIData(m_single_data);// clear old data
//    m_mutiply_data.clear();
//    m_mutiply_names.clear();
//    //ui->comboBox_plotwhat->clear();
//    // QComoBox
//    QString TropDelay = ui->comboBox_TropDelay->currentText(),
//            Method = ui->comboBox_Method->currentText(),
//            CutAngle_Str = ui->lineEdit_Angle->text(),
//            PPPModel_Str = ui->comboBox_PPPModel->currentText();
//    double CutAngle = CutAngle_Str.toDouble();
//    // QCheckBox
//    QString SatSystem = "";
//    bool Kinematic = false;
//    // Clock difference sequence of receiver
//    if(ui->checkBox_GPS->isChecked())
//        SatSystem.append("G");
//    if(ui->checkBox_GLONASS->isChecked())
//        SatSystem.append("R");
//    if(ui->checkBox_GAlieo->isChecked())
//        SatSystem.append("E");
//    if(ui->checkBox_BDS->isChecked())
//        SatSystem.append("C");
//    if(ui->checkBox_Kinematic->isChecked())
//        Kinematic = true;
//    QString Smooth_Str = ui->comboBox_PPP_SMOOTH->currentText();
//    QString m_Products = "igs";
//    if(ui->radioButton_igs->isChecked())
//        m_Products = "igs";
//    else if(ui->radioButton_cnt->isChecked())
//        m_Products = "cnt";
//// From Configure file
//    // Delete satellites
//    QString removeSats = m_ConfigWidget.myConfTranIni.getValue("/ARISEN/deleteSats");
//    removeSats.append(";C01;C02;C03;C04;C05");// remove GEO of BeiDou
//    // SYS/#/OBS TYPES (Set the PPP dual-frequency observation type)
//    QVector<QStringList> ObsTypeSet = getConfObsType();
//    // Set Parameters
//    QString Qw_Str = m_ConfigWidget.myConfTranIni.getValue("/ARISEN/Qw"),
//            Pk_Str = m_ConfigWidget.myConfTranIni.getValue("/ARISEN/Pk"),
//            LP_Str = m_ConfigWidget.myConfTranIni.getValue("/ARISEN/LP_precision");
//    QStringList Qw_StrList =  Qw_Str.split(";"), Pk_StrList = Pk_Str.split(";"),
//            LP_List = LP_Str.split(";");
//    QVector<QStringList> Qw_Pk;
//    Qw_Pk.append(Qw_StrList); Qw_Pk.append(Pk_StrList); Qw_Pk.append(LP_List);

//    if(!m_station_path.isEmpty())
//    {//界面中加上SSD AR 2021.03.31 23z
//        ui->textEdit_Display->clear();
//        if(ui->checkBox_Back->isChecked())
//        {
//            if(PPPModel_Str == "IF")
//            {
//                QPPPBackSmooth  myBkPPP(m_station_path, ui->textEdit_Display, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, m_Products, PPPModel_Str);
//                myBkPPP.Run(true);// true represent disply every epoch information(ENU or XYZ)
//                m_isRuned = myBkPPP.isRuned();
//                if(m_isRuned)
//                {
//                    clearPlotGUIData(m_single_data);
//                    myBkPPP.getRunResult(m_single_data);
//                }
//            }
//            if(PPPModel_Str == "SSD")
//            {
//                QPPPBackSmooth  myBkPPP(m_station_path, ui->textEdit_Display, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, m_Products, PPPModel_Str);
//                myBkPPP.SSDPPP();
//                m_isRuned = myBkPPP.isRuned();
//                if(m_isRuned)
//                {
//                    clearPlotGUIData(m_single_data);
//                    myBkPPP.getRunResult(m_single_data);
//                }
//            }
//            if(PPPModel_Str == "AR")
//            {
//                QPPPBackSmooth  myBkPPP(m_station_path, ui->textEdit_Display, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, m_Products, PPPModel_Str);
//                myBkPPP.PPPAR();
//                m_isRuned = myBkPPP.isRuned();
//                if(m_isRuned)
//                {
//                    clearPlotGUIData(m_single_data);
//                    myBkPPP.getRunResult(m_single_data);
//                }
//            }
//        }
//        else
//        {
//            if(PPPModel_Str == "IF")
//            {
//                QPPPModel myPPP(m_station_path, ui->textEdit_Display, Method, SatSystem, TropDelay, CutAngle, Kinematic,
//                                Smooth_Str, m_Products, PPPModel_Str, removeSats, ObsTypeSet, Qw_Pk);
//                myPPP.Run(true);// true represent disply every epoch information(ENU or XYZ)
//                m_isRuned = myPPP.isRuned();
//                if(m_isRuned)
//                {
//                    clearPlotGUIData(m_single_data);
//                    myPPP.getRunResult(m_single_data);
//                }
//            }
//            if(PPPModel_Str == "SSD")
//            {
//                QPPPModel myPPP(m_station_path, ui->textEdit_Display, Method, SatSystem, TropDelay, CutAngle, Kinematic,
//                                Smooth_Str, m_Products, PPPModel_Str, removeSats, ObsTypeSet, Qw_Pk);
//                myPPP.runSSDPPP();
//                m_isRuned = myPPP.isRuned();
//                if(m_isRuned)
//                {
//                    clearPlotGUIData(m_single_data);
//                    myPPP.getRunResult(m_single_data);
//                }
//            }
//            if(PPPModel_Str == "AR")
//            {
//                QPPPModel myPPP(m_station_path, ui->textEdit_Display, Method, SatSystem, TropDelay, CutAngle, Kinematic,
//                                Smooth_Str, m_Products, PPPModel_Str, removeSats, ObsTypeSet, Qw_Pk);
//                myPPP.runPPPAR();
//                m_isRuned = myPPP.isRuned();
//                if(m_isRuned)
//                {
//                    clearPlotGUIData(m_single_data);
//                    myPPP.getRunResult(m_single_data);
//                }
//            }
//        }

//    }
//    else
//    {
//        WaringBox("Please select obsevation floder.");
//    }
//}

//void MainWindow::RunSPP()
//{
//    m_station_path = ui->textEdit_FilePath->text();
//    // clear mutiply stations
//    m_isRuned = false;
//    m_isRunedBatch = false;
//    clearPlotGUIData(m_single_data);// clear old data
//    m_mutiply_names.clear();
//    //ui->comboBox_plotwhat->clear();
//    // QComoBox
//    QString TropDelay = ui->comboBox_TropDelay->currentText(),
//            Method = ui->comboBox_Method->currentText(),
//            CutAngle_Str = ui->lineEdit_Angle->text(),
//            PPPModel_Str = ui->comboBox_PPPModel->currentText();;
//    double CutAngle = CutAngle_Str.toDouble();
//    // QCheckBox
//    QString SatSystem = "";
//    bool Kinematic = false;
//    if(ui->checkBox_GPS->isChecked())
//        SatSystem.append("G");
//    if(ui->checkBox_GLONASS->isChecked())
//        SatSystem.append("R");
//    if(ui->checkBox_GAlieo->isChecked())
//        SatSystem.append("E");
//    if(ui->checkBox_BDS->isChecked())
//        SatSystem.append("C");
//    if(ui->checkBox_Kinematic->isChecked())
//        Kinematic = true;
//    // QComoBox
//    QString Smooth_Str = ui->comboBox_SPP_SMOOTH->currentText(),
//            SPP_Model = ui->comboBox_SPP_LC->currentText();

//    if(!m_station_path.isEmpty())
//    {
//        ui->textEdit_Display->clear();
//        QSPPModel mySPP(m_station_path, ui->textEdit_Display, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, SPP_Model, PPPModel_Str);
//        mySPP.Run(true);// false represent disply every epoch information(ENU or XYZ)
//        m_isRuned = mySPP.isRuned();
//        if(m_isRuned)
//        {
//            clearPlotGUIData(m_single_data);
//            mySPP.getRunResult(m_single_data);
//        }
//    }
//    else
//    {
//        WaringBox("Please select obsevation floder.");
//    }
//}

QVector<QStringList> MainWindow::getConfObsType()
{
    QVector<QStringList> tempConfObs;
    QString SatOBStype;
    QStringList Sat_List;
    // GPS
    SatOBStype = m_ConfigWidget.myConfTranIni.getValue("/ARISEN/GPS_OBS_TYPE");
    Sat_List= SatOBStype.split(";");
    Sat_List.prepend("G");
    Sat_List.removeAll(QString(""));
    if(Sat_List.length() >= 2)  tempConfObs.append(Sat_List);
    //GLONASS
    SatOBStype = m_ConfigWidget.myConfTranIni.getValue("/ARISEN/GLONASS_OBS_TYPE");
    Sat_List= SatOBStype.split(";");
    Sat_List.prepend("R");
    Sat_List.removeAll(QString(""));
    if(Sat_List.length() >= 2)  tempConfObs.append(Sat_List);
    //BDS
    SatOBStype = m_ConfigWidget.myConfTranIni.getValue("/ARISEN/BDS_OBS_TYPE");
    Sat_List= SatOBStype.split(";");
    Sat_List.prepend("C");
    Sat_List.removeAll(QString(""));
    if(Sat_List.length() >= 2)  tempConfObs.append(Sat_List);
    //Galileo
    SatOBStype = m_ConfigWidget.myConfTranIni.getValue("/ARISEN/Galileo_OBS_TYPE");
    Sat_List= SatOBStype.split(";");
    Sat_List.prepend("E");
    Sat_List.removeAll(QString(""));
    if(Sat_List.length() >= 2)  tempConfObs.append(Sat_List);

    return tempConfObs;
}

void MainWindow::selectFilePath()
{
    QFileDialog fileDialog;
    fileDialog.setFileMode(QFileDialog::Directory);

    if(ui->checkBox_batch->isChecked())
        m_station_path = fileDialog.getExistingDirectory(this, "Open Directory", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    else
        m_station_path = fileDialog.getOpenFileName(this, "Open Directory","*.*o");

    if(m_station_path.isEmpty())
    {
        QString WarngInfo = "Failed to select file";
        WaringBox(WarngInfo);
        ui->textEdit_FilePath_obs->setText("");
        autoScrollTextEdit(ui->textEdit_Display, WarngInfo);
    }
    else
    {
        QString Display = "Rinex Dir:  " + m_station_path;
        ui->textEdit_FilePath_obs->setText(m_station_path);
        autoScrollTextEdit(ui->textEdit_Display, Display);
    }
}
void MainWindow::clearPlotGUIData(PlotGUIData &station_data)
{
    station_data.X.clear();
    station_data.Y.clear();
    station_data.Z.clear();
    station_data.spp_X.clear();
    station_data.spp_Y.clear();
    station_data.spp_Z.clear();
    station_data.clockData.clear();
    station_data.ZTD_W.clear();
}

// The edit box automatically scrolls, adding one row or more lines at a time.
void MainWindow::autoScrollTextEdit(QTextEdit *textEdit,QString &add_text)
{
    //Add line character and refresh edit box.
    QString insertText = add_text + ENDLINE;
    textEdit->insertPlainText(insertText);
    //Keep the editor in the last line of the cursor.
    QTextCursor cursor=textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    textEdit->setTextCursor(cursor);
    //If you exceed a certain number of lines, empty it.
    if(textEdit->document()->lineCount() > m_Display_Max_line)
    {
        textEdit->clear();
    }
}

void MainWindow::WaringBox(QString info)
{
    QMessageBox::warning(this, "Warning", info);
}

bool MainWindow::isDirExist(QString fullPath)
{
    if(fullPath.isEmpty()) return false;

    QDir dir(fullPath);
    if(dir.exists())
    {
        return true;
    }
    else
    {
        bool ok = dir.mkpath(fullPath);//Create a multi-level directory
        return ok;
    }
}



