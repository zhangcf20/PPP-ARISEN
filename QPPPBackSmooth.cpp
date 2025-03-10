#include "QPPPBackSmooth.h"

QStringList QPPPBackSmooth::searchFilterFile(QString floder_path, QStringList filers)
{
    QDir floder_Dir(floder_path);
    QStringList all_file_path;
    floder_Dir.setFilter(QDir::Files | QDir::NoSymLinks);
    floder_Dir.setNameFilters(filers);
    QFileInfoList file_infos = floder_Dir.entryInfoList();
    for(int i = 0;i < file_infos.length();i++)
    {
        QFileInfo file_info = file_infos.at(i);
        if(file_info.fileName() != "." || file_info.fileName() != "..")
            all_file_path.append(file_info.absoluteFilePath());
    }
    return all_file_path;
}

//Run the specified directory file
QPPPBackSmooth::QPPPBackSmooth(QStringList files_path,  QTextEdit *pQTextEdit, QString Method, QString Satsystem,
                     QString TropDelay, double CutAngle, bool isKinematic, QString Smooth_Str, QString products, QString pppmodel_t)
{
    // Display for GUI
    mp_QTextEditforDisplay = pQTextEdit;
    //Initialize variables
    initVar();
    m_run_floder = files_path.at(0);
    RPR_filepath = files_path;

    m_App_floder = QCoreApplication::applicationDirPath() + PATHSEG;
    // find files
    QStringList tempFilters,
            OFileNamesList, Sp3FileNamesList, ClkFileNamesList, ErpFileNamesList,
            AtxFileNamesList, BlqFileNamesList, GrdFileNamesList;
    // find obs files
    tempFilters.clear();
    tempFilters.append("*.*o");
    QString temp_pathseg = PATHSEG;
    int temp_lp = 0;
    for(int cf_i = m_run_floder.length()-1; cf_i > 0; cf_i--)
    {
        if(m_run_floder.at(cf_i) == temp_pathseg)
        {
            temp_lp = cf_i;
            break;
        }
    }
    QString temp_filepath = m_run_floder.mid(0,temp_lp);
    OFileNamesList = searchFilterFile(temp_filepath, tempFilters);

    temp_filepath = RPR_filepath.at(1) + PATHSEG + "sp3";
    // find sp3 files
    tempFilters.clear();
    tempFilters.append("*.sp3");
    Sp3FileNamesList = searchFilterFile(temp_filepath, tempFilters);
    if(Sp3FileNamesList.length() == 0)
    {
        // find eph files
        tempFilters.clear();
        tempFilters.append("*.eph*");
        Sp3FileNamesList = searchFilterFile(temp_filepath, tempFilters);
    }
    temp_filepath = RPR_filepath.at(1) + PATHSEG + "clk";
    // find clk files
    tempFilters.clear();
    tempFilters.append("*.clk");
    ClkFileNamesList = searchFilterFile(temp_filepath, tempFilters);
    // if not find clk try clk_*
    if(ClkFileNamesList.isEmpty())
    {
        tempFilters.clear();
        tempFilters.append("*.clk_*");
        ClkFileNamesList = searchFilterFile(temp_filepath, tempFilters);
    }
    temp_filepath = RPR_filepath.at(1) + PATHSEG + "erp";
    // find erp files
    tempFilters.clear();
    tempFilters.append("*.erp*");
    ErpFileNamesList = searchFilterFile(temp_filepath, tempFilters);
    // find Atx files
    tempFilters.clear();
    tempFilters.append("*.atx");
    AtxFileNamesList = searchFilterFile(m_App_floder, tempFilters);
    // find blq files
    tempFilters.clear();
    tempFilters.append("*.blq");
    BlqFileNamesList = searchFilterFile(m_App_floder, tempFilters);
    // find grd files
    tempFilters.clear();
    tempFilters.append("*.grd");
    GrdFileNamesList = searchFilterFile(m_App_floder, tempFilters);
    // get want file
    // o file
    bool temp_oflag = false;
    for(int cf_i = 0; cf_i < OFileNamesList.length(); cf_i++)
    {
        if(OFileNamesList.at(cf_i) == m_run_floder)
        {
            temp_oflag = true;
            break;
        }
    }
    QString OfileName = "", erpFile = "", blqFile = "", atxFile = "", grdFile = "";
    if(temp_oflag)
    {
        OfileName = files_path.at(0);
        m_haveObsFile = true;
    }
    else
    {
        ErroTrace("QPPPBackSmooth::QPPPBackSmooth: Cant not find Obsvertion file.");
        m_haveObsFile = false;
    }
    //if(!ErpFileNamesList.isEmpty()) erpFile = ErpFileNamesList.at(0);
    if(!AtxFileNamesList.isEmpty()) atxFile = AtxFileNamesList.at(0);
    if(!BlqFileNamesList.isEmpty()) blqFile = BlqFileNamesList.at(0);

    QString temp_grd = "gpt2";
    if(global_cf::isGPT3) temp_grd = "gpt3";

    int selected = 0;
    for(int cf_i = 0; cf_i < GrdFileNamesList.length(); cf_i++)
    {
        if(GrdFileNamesList.at(cf_i).contains(temp_grd))
        {
            selected = cf_i;
            break;
        }
    }
    if(!GrdFileNamesList.isEmpty()) grdFile = GrdFileNamesList.at(selected);

    //if(!GrdFileNamesList.isEmpty()) grdFile = GrdFileNamesList.at(0);

    // use defualt config
    setConfigure(Method, Satsystem, TropDelay, CutAngle, isKinematic, Smooth_Str, products, pppmodel_t);
    // save data to QPPPBackSmooth
    initQPPPBackSmooth(OfileName, Sp3FileNamesList, ClkFileNamesList, ErpFileNamesList, blqFile, atxFile, grdFile);
}
void QPPPBackSmooth::setConfigure(QString Method, QString Satsystem, QString TropDelay, double CutAngle,
                                  bool isKinematic, QString Smooth_Str, QString products, QString pppmodel_t)
{
    // Configure
    m_Solver_Method = Method;// m_Solver_Method value can be "SRIF" or "Kalman"
    m_CutAngle = CutAngle;// (degree)
    m_SatSystem = Satsystem;// GPS, GLONASS, BDS, and Galieo are used respectively: the letters G, R, C, E
    m_TropDelay = TropDelay;// The tropospheric model m_TropDelay can choose Sass, Hopfiled, UNB3m
    m_Smooth_Str = Smooth_Str;
    m_Product = products;
    m_isKinematic = isKinematic;
    m_PPPModel_Str = pppmodel_t;

    //Setting up the file system. SystemStr:"G"(Turn on the GPS system);"GR":(Turn on the GPS+GLONASS system);"GRCE"(Open all)et al
    //GPS, GLONASS, BDS, and Galieo are used respectively: the letters G, R, C, E
    setSatlitSys(Satsystem);
    m_sys_str = Satsystem;
    m_sys_num = getSystemnum();

   if(isKinematic)
   {
       m_KalmanClass.setModel(QKalmanFilter::KALMAN_MODEL::PPP_KINEMATIC);// set Kinematic model
       m_SRIFAlgorithm.setModel(SRIFAlgorithm::SRIF_MODEL::PPP_KINEMATIC);
       m_minSatFlag = 5;// Dynamic Settings 5, Static Settings 1 in setConfigure()
   }
   else
   {
       m_KalmanClass.setModel(QKalmanFilter::KALMAN_MODEL::PPP_STATIC);// set static model
       m_SRIFAlgorithm.setModel(SRIFAlgorithm::SRIF_MODEL::PPP_STATIC);
       m_minSatFlag = 1;// Dynamic Settings 5, Static Settings 1 in setConfigure()
   }


   if("Smooth" == Smooth_Str)
   {
       m_KalmanClass.setSmoothRange(QKalmanFilter::KALMAN_SMOOTH_RANGE::SMOOTH);
       m_SRIFAlgorithm.setSmoothRange(SRIFAlgorithm::SRIF_SMOOTH_RANGE::SMOOTH);
       m_isSmoothRange = true;// Whether to use phase smoothing pseudorange for SPP
   }
   else if("NoSmooth" == Smooth_Str)
   {
       m_KalmanClass.setSmoothRange(QKalmanFilter::KALMAN_SMOOTH_RANGE::NO_SMOOTH);
       m_SRIFAlgorithm.setSmoothRange(SRIFAlgorithm::SRIF_SMOOTH_RANGE::NO_SMOOTH);
       m_isSmoothRange = false;// Whether to use phase smoothing pseudorange for SPP
   }

   if(Method == "KalmanOu")
   {
       m_KalmanClass.setFilterMode(QKalmanFilter::KALMAN_FILLTER::KALMAN_MrOu);
   }
   if(m_PPPModel_Str.contains("IF", Qt::CaseInsensitive) || m_PPPModel_Str.contains("SSD", Qt::CaseInsensitive) || m_PPPModel_Str.contains("AR", Qt::CaseInsensitive))
   {
       setPPPModel(PPP_MODEL::PPP_Combination);
       m_KalmanClass.setPPPModel(PPP_MODEL::PPP_Combination);
       m_writeFileClass.setPPPModel(PPP_MODEL::PPP_Combination);
       m_SRIFAlgorithm.setPPPModel(PPP_MODEL::PPP_Combination);
   }
   else if(m_PPPModel_Str.contains("Uncomb", Qt::CaseInsensitive))
   {
       setPPPModel(PPP_MODEL::PPP_NOCombination);
       m_KalmanClass.setPPPModel(PPP_MODEL::PPP_NOCombination);
       m_writeFileClass.setPPPModel(PPP_MODEL::PPP_NOCombination);
       m_SRIFAlgorithm.setPPPModel(PPP_MODEL::PPP_NOCombination);
   }
   else
       m_haveObsFile = false;

}

//Initialization operation
void QPPPBackSmooth::initVar()
{
    for (int i = 0;i < 3;i++)
        m_ApproxRecPos[0] = 0;
    m_OFileName = "";
    multReadOFile = 99999999999;// read all epoch in obeservation data
    m_leapSeconds = 0;
    m_isConnect = false;
    m_isConnectCNT = false;
    m_run_floder = "";
    m_haveObsFile = false;
    m_isRuned = false;
    m_isInitSPP = false;
    m_save_images_path = "";
    m_iswritre_file = false;
    m_minSatFlag = 5;// Dynamic Settings 5, Static Settings 1 in setConfigure()
    m_isSmoothRange = false;// Whether to use phase smoothing pseudorange for SPP
}

//Constructor
void QPPPBackSmooth::initQPPPBackSmooth(QString OFileName,QStringList Sp3FileNames,QStringList ClkFileNames,QStringList ErpFileName,QString BlqFileName,QString AtxFileName,QString GrdFileName)
{
    if(!m_haveObsFile) return ;// if not have observation file.

    //23z
    m_ReadOFileClass.setObsFileName(OFileName);
    //Various class settings
    int obsTime[5] = {0};
    double Seconds = 0,ObsJD = 0;
    m_ReadOFileClass.getApproXYZ(m_ApproxRecPos);//Obtain the approximate coordinates of the O file
    m_ReadOFileClass.getFistObsTime(obsTime,Seconds);//Get the initial observation time
    ObsJD = qCmpGpsT.computeJD(obsTime[0],obsTime[1],obsTime[2],obsTime[3],obsTime[4],Seconds);

    //Search products and download
    int GPS_Week = 0, GPS_Day = 0;
    qCmpGpsT.YMD2GPSTime(obsTime[0],obsTime[1],obsTime[2],obsTime[3],obsTime[4],Seconds, &GPS_Week, &GPS_Day);

    QString ot_WGPS = QString::number(GPS_Week,10) + QString::number(GPS_Day,10);
    QString ot_WGPSb = QString::number(GPS_Week,10) + QString::number(GPS_Day - 1,10);
    QString ot_WGPSa = QString::number(GPS_Week,10) + QString::number(GPS_Day + 1,10);
    if(GPS_Day == 0)
    {
        ot_WGPSb = QString::number(GPS_Week - 1,10) + QString::number(6,10);
    }
    if(GPS_Day == 6)
    {
        ot_WGPSa = QString::number(GPS_Week + 1,10) + QString::number(0,10);
    }

    int DOY = qCmpGpsT.YearAccDay(obsTime[0],obsTime[1],obsTime[2]);
    QString ot_UTC = QString::number(obsTime[0],10) + QString::number(DOY,10).sprintf("%03d",DOY);
    QString ot_UTCb = QString::number(obsTime[0],10) + QString::number(DOY - 1,10).sprintf("%03d",DOY - 1);
    QString ot_UTCa = QString::number(obsTime[0],10) + QString::number(DOY + 1,10).sprintf("%03d",DOY + 1);
    m_ot_UTC = ot_UTC;

    if(DOY == 1)
    {
        int temp_doy;
        if(qCmpGpsT.leapyear_d(obsTime[0] - 1))
            temp_doy = 366;
        else
            temp_doy = 365;
        ot_UTCb = QString::number(obsTime[0] - 1,10) + QString::number(temp_doy,10).sprintf("%03d",temp_doy);
    }
    if((qCmpGpsT.leapyear_d(obsTime[0]) && DOY == 366) || (!qCmpGpsT.leapyear_d(obsTime[0]) && DOY == 365))
    {
        int temp_doy = 001;
        ot_UTCa = QString::number(obsTime[0] + 1,10) + QString::number(temp_doy,10).sprintf("%03d",temp_doy);
    }

    QStringList s_sp3, s_clk;
    QString s_erp, s_bia, s_DCB, s_shad;
    QString temp_file;
    for(int cf_i = 0; cf_i < Sp3FileNames.length(); cf_i++)
    {
        temp_file = Sp3FileNames.at(cf_i);
        if(temp_file.contains(ot_WGPSb) || temp_file.contains(ot_WGPS) || temp_file.contains(ot_WGPSa))
        {
            s_sp3.append(temp_file);
        }
        if(temp_file.contains(ot_UTCb) || temp_file.contains(ot_UTC) || temp_file.contains(ot_UTCa))
        {
            s_sp3.append(temp_file);
        }
        if(s_sp3.length() == 3)
            break;
    }
    for(int cf_i = 0; cf_i < ClkFileNames.length(); cf_i++)
    {
        temp_file = ClkFileNames.at(cf_i);
        if(temp_file.contains(ot_WGPSb) || temp_file.contains(ot_WGPS) || temp_file.contains(ot_WGPSa))
        {
            s_clk.append(temp_file);
        }
        if(temp_file.contains(ot_UTCb) || temp_file.contains(ot_UTC) || temp_file.contains(ot_UTCa))
        {
            s_clk.append(temp_file);
        }
        if(s_clk.length() == 3)
            break;
    }
    for(int cf_i = 0; cf_i < ErpFileName.length(); cf_i++)
    {
        temp_file = ErpFileName.at(cf_i);
        if(temp_file.contains(ot_WGPS) || temp_file.contains(ot_UTC))
        {
            s_erp = temp_file;
            break;
        }
    }

//Set up multi-system data
//Initial various classes
    m_ReadSP3Class.setSP3FileNames(s_sp3);
    m_ReadClkClass.setClkFileNames(s_clk);
    m_ReadOFileClass.setObsFileName(OFileName);
    m_ReadTropClass.setTropFileNames(GrdFileName,"VMF", m_TropDelay);// Default tropospheric model projection function GMF
    m_ReadAntClass.setAntFileName(AtxFileName);

    m_TideEffectClass.setTideFileName(BlqFileName,s_erp);

    m_ReadAntClass.setObsJD(m_ReadOFileClass.getAntType(),ObsJD);//Set the antenna effective time
    m_TideEffectClass.setStationName(m_ReadOFileClass.getMakerName());//Setting the tide requires a station name

//Save file name
    m_OFileName = OFileName;
    m_Sp3FileNames = s_sp3;
    m_ClkFileNames = s_clk;
////Various class settings
//    int obsTime[5] = {0};
//    double Seconds = 0,ObsJD = 0;
//    m_ReadOFileClass.getApproXYZ(m_ApproxRecPos);//Obtain the approximate coordinates of the O file
//    m_ReadOFileClass.getFistObsTime(obsTime,Seconds);//Get the initial observation time
//    ObsJD = qCmpGpsT.computeJD(obsTime[0],obsTime[1],obsTime[2],obsTime[3],obsTime[4],Seconds);
//    m_ReadAntClass.setObsJD(m_ReadOFileClass.getAntType(),ObsJD);//Set the antenna effective time
//    m_TideEffectClass.setStationName(m_ReadOFileClass.getMakerName());//Setting the tide requires a station name
////Search products and download
//    int GPS_Week = 0, GPS_Day = 0;
//    qCmpGpsT.YMD2GPSTime(obsTime[0],obsTime[1],obsTime[2],obsTime[3],obsTime[4],Seconds, &GPS_Week, &GPS_Day);
    if(Sp3FileNames.isEmpty() || ClkFileNames.isEmpty() || ErpFileName.isEmpty())
    {
        QString disPlayQTextEdit = "Lack of necessary precision products!";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
    }
//    else if(Sp3FileNames.length() !=3 || ClkFileNames.length() !=3)
//    {
//        QString disPlayQTextEdit = "Lack of some precision products!";
//        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
//    }

    //If there is no product, it is necessary to make up such final products.
//    if(Sp3FileNames.isEmpty() || ClkFileNames.isEmpty() || ErpFileName.isEmpty())   connectHost();
//    if(m_Product.contains("igs", Qt::CaseInsensitive))
//    {
//        if(Sp3FileNames.isEmpty())  m_Sp3FileNames = downProducts(m_run_floder, GPS_Week, GPS_Day, "sp3");
//        if(ClkFileNames.isEmpty())  m_ClkFileNames = downProducts(m_run_floder, GPS_Week, GPS_Day, "clk");
//    }
//    // download erp
//    if(ErpFileName.isEmpty())
//    {
//        m_ErpFileName = downErpFile(m_run_floder, GPS_Week, GPS_Day, "erp");
//        m_TideEffectClass.setTideFileName(BlqFileName,m_ErpFileName);
//        m_ReadAntClass.m_CmpClass.readRepFile(ErpFileName);//  for compute sun and moon position
//        qCmpGpsT.readRepFile(m_ErpFileName);
//    }
//    // if download final products bad, it will download real-time cnt products
//    if(m_Product.contains("cnt", Qt::CaseInsensitive))
//    {
//        if(m_Sp3FileNames.isEmpty() || m_ClkFileNames.isEmpty())   connectCNTHost();
//        if(m_Sp3FileNames.isEmpty())  m_Sp3FileNames = downCNTProducts(m_run_floder, GPS_Week, GPS_Day, "sp3");
//        if(m_ClkFileNames.isEmpty())  m_ClkFileNames = downCNTProducts(m_run_floder, GPS_Week, GPS_Day, "clk");
//    }

//    // set download file name
//    if(!m_Sp3FileNames.isEmpty())   m_ReadSP3Class.setSP3FileNames(m_Sp3FileNames);
//    if(!m_ClkFileNames.isEmpty())   m_ReadClkClass.setClkFileNames(m_ClkFileNames);

    // end this PPP
    if(m_Sp3FileNames.isEmpty() || m_ClkFileNames.isEmpty())    m_haveObsFile = false;

//Get skip seconds
    m_leapSeconds = qCmpGpsT.getLeapSecond(obsTime[0],obsTime[1],obsTime[2],obsTime[3],obsTime[4],Seconds);

//Read the required calculation file module (time consuming)
    m_ReadAntClass.getAllData();//Read all data from satellites and receivers
    m_TideEffectClass.getAllData();//Read tide data
    m_ReadTropClass.getAllData();//Read grd files for later tropospheric calculations
    m_ReadSP3Class.getAllData();//Read the entire SP3 file
    m_ReadClkClass.getAllData();//Read the clock error file for later calculation

    m_markname = m_ReadOFileClass.getMakerName();

    if(m_markname.length() == 0)
    {
        int temp_lp = 0;
        QString temp_pathseg = PATHSEG;
        for(int cf_i = m_run_floder.length()-1; cf_i > 0; cf_i--)
        {
            if(m_run_floder.at(cf_i) == temp_pathseg)
            {
                temp_lp = cf_i;
                break;
            }
        }
        m_markname = m_run_floder.mid(temp_lp + 1,4);
    }

    if(m_markname.length() > 4)
        m_markname = m_markname.mid(0,4);
}

// get matrix B and observer L(QPPPBackSmooth::Obtaining_equation() is same function as QPPPModel::Obtaining_equation() )
void QPPPBackSmooth::Obtaining_equation(QVector< SatlitData > &currEpoch, double *ApproxRecPos, MatrixXd &mat_B, VectorXd &Vct_L, MatrixXd &mat_P, bool isSmoothRange)
{
    int epochLenLB = currEpoch.length();
    MatrixXd B, P;
    VectorXd L, sys_len;

    sys_len.resize(m_sys_str.length());
    B.resize(epochLenLB,3 + m_sys_num);
    P.resize(epochLenLB,epochLenLB);
    L.resize(epochLenLB);

    sys_len.setZero();
    B.setZero();
    L.setZero();
    P.setIdentity();
    bool is_find_base_sat = false;

    for (int i = 0; i < epochLenLB;i++)
    {
        SatlitData oneSatlit = currEpoch.at(i);
        double li = 0,mi = 0,ni = 0,p0 = 0,dltaX = 0,dltaY = 0,dltaZ = 0;
        dltaX = oneSatlit.X - ApproxRecPos[0];
        dltaY = oneSatlit.Y - ApproxRecPos[1];
        dltaZ = oneSatlit.Z - ApproxRecPos[2];
        p0 = qSqrt(dltaX*dltaX+dltaY*dltaY+dltaZ*dltaZ);
        li = dltaX/p0;mi = dltaY/p0;ni = dltaZ/p0;
        //Computational B matrix
        //P3 pseudorange code matrix
        B(i, 0) = li;B(i, 1) = mi;B(i, 2) = ni; B(i, 3) = -1;// base system satlite clk must exist
        // debug by xiaogongwei 2019.04.03 for ISB
        for(int k = 1; k < m_sys_str.length();k++)
        {
            if(m_sys_str[k] == oneSatlit.SatType)
            {
                B(i,3+k) = -1;
                sys_len[k] = 1;//good no zeros cloumn in B,sys_lenmybe 0 1 1 0
            }
        }
        // is exist base system satlite clk
        if(m_sys_str[0] == oneSatlit.SatType)
            is_find_base_sat = true;

        //Calculating the L matrix
        double dlta = 0;//Correction of each
        dlta =  - oneSatlit.StaClock + oneSatlit.SatTrop - oneSatlit.Relativty -
            oneSatlit.Sagnac - oneSatlit.TideEffect - oneSatlit.AntHeight;
        //Pseudorange code PP3
        if(isSmoothRange)
        {// add by xiaogongwei 2018.11.20
            L(i) = p0 - oneSatlit.PP3_Smooth + dlta;
            // Computing weight matrix P
//            if(oneSatlit.UTCTime.epochNum > 30 && oneSatlit.PP3_Smooth_NUM < 30 )//
//                P(i, i) = 0.0001;
//            else
                P(i, i) = 1 / oneSatlit.PP3_Smooth_Q;//Smooth pseudorange????
        }
        else
        {
            L(i) = p0 - oneSatlit.PP3 + dlta;
            // Computing weight matrix P
            P(i, i) = oneSatlit.SatWight;//Pseudo-range right
        }

    }//B, L is calculated
    // save data to mat_B
    mat_B = B;
    Vct_L = L;
    mat_P = P;
    // debug by xiaogongwei 2019.04.04
    int no_zero = sys_len.size() - 1 - sys_len.sum();
    if(no_zero > 0 || !is_find_base_sat)
    {
        int new_hang = B.rows() + no_zero, new_lie = B.cols(), flag = 0;
        if(!is_find_base_sat) new_hang++; // check base system satlite clk is exist
        mat_B.resize(new_hang,new_lie);
        mat_P.resize(new_hang,new_hang);
        Vct_L.resize(new_hang);
        mat_B.setZero();
        Vct_L.setZero();
        mat_P.setIdentity();
        // check base system satlite clk is exist
        if(!is_find_base_sat)
        {
            for(int i = 0;i < B.rows();i++)
                B(i, 3) = 0;
            mat_B(mat_B.rows() - 1, 3) = 1;
        }
        mat_B.block(0,0,B.rows(),B.cols()) = B;
        mat_P.block(0,0,P.rows(),P.cols()) = P;
        Vct_L.head(L.rows()) = L;
        for(int i = 1; i < sys_len.size();i++)
        {
            if(0 == sys_len[i])
            {
                mat_B(epochLenLB+flag, 3+i) = 1;// 3 is conntain [dx,dy,dz,mf]
                flag++;
            }

        }
    }//if(no_zero > 0)
}

//Read O files, sp3 files, clk files, and various error calculations, Kalman filtering ......................
//isDisplayEveryEpoch represent is disply every epoch information?(ENU or XYZ)
void QPPPBackSmooth::Run(bool isDisplayEveryEpoch)
{
    // next is use to back smooth fillter
    if(!m_haveObsFile) return ;// if not have observation file.
    QTime myTime; // set timer
    myTime.start();// start timer

    // Use PPP forward filtter
    QPPPModel *myPPP = new QPPPModel(RPR_filepath, mp_QTextEditforDisplay, m_Solver_Method, m_SatSystem,
                    m_TropDelay, m_CutAngle, m_isKinematic, m_Smooth_Str, m_Product, m_PPPModel_Str);
    myPPP->Run(isDisplayEveryEpoch);// false represent no disply every epoch information(ENU or XYZ)
    m_isRuned = myPPP->isRuned();
    if(!m_isRuned) return;
    // get forward fillter information
    int forWard_epoch_len = myPPP->m_writeFileClass.allSolverX.length();
    if(forWard_epoch_len == 0) return ;
    QVector< RecivePos > last_allReciverPos = myPPP->m_writeFileClass.allReciverPos;
    QVector< VectorXd > last_allSolverX = myPPP->m_writeFileClass.allSolverX;
    QVector< MatrixXd > last_allSolverQ = myPPP->m_writeFileClass.allSloverQ;
    VectorXd last_fillter_X;
    MatrixXd last_fillter_Q;
    for(int i = forWard_epoch_len - 1; i >=0; i--)
    {
        last_fillter_X = last_allSolverX.at(i);
        if(last_fillter_X[0] != 0)
        {
            last_fillter_X = last_allSolverX.at(i);
            last_fillter_Q = last_allSolverQ.at(i);
            break;
        }
    }
    QVector< QVector < SatlitData > > last_allPPPSatlitData = myPPP->m_writeFileClass.allPPPSatlitData;
    // delete myPPP pointer
    delete myPPP;
    //not use accelerate in search sp3 and clk
    m_ReadSP3Class.ACCELERATE = 0;
    m_ReadClkClass.ACCELERATE = 0;
    //Externally initialize fixed variables to speed up calculations
    double p_HEN[3] = {0};//Get the antenna high
    m_ReadOFileClass.getAntHEN(p_HEN);
    //Traversing data one by one epoch, reading O file data
    QString disPlayQTextEdit = "";// display for QTextEdit
    QVector < SatlitData > prevEpochSatlitData;//Store satellite data of an epoch, use cycle slip detection（Put it on top or read multReadOFile epochs, the life cycle will expire when reading）
    double spp_pos[4] = {last_fillter_X[0], last_fillter_X[1], last_fillter_X[2], last_fillter_X[4]};// store back smooth pos and clk
    QVector< QVector < SatlitData > > multepochSatlitData;//Store multiple epochs
    multepochSatlitData = last_allPPPSatlitData;// get all epoch data
    int all_epoch_len = multepochSatlitData.length();
    double continue_bad_epoch = 0;
//Back smooth Multiple epoch cycles
        for (int epoch_num = all_epoch_len - 1; epoch_num >= 0;epoch_num--)
        {
            if(epoch_num == 2576)
            {
                int a = 0;
            }
            QVector< SatlitData > epochSatlitData;//Temporary storage of uncalculated data for each epoch satellite
            epochSatlitData = multepochSatlitData.at(epoch_num);

            GPSPosTime epochTime;
            if(epochSatlitData.length() != 0)
            {
                epochTime = epochSatlitData.at(0).UTCTime;//Obtaining observation time（Each satellite in the epoch stores the observation time）
                epochTime.epochNum = epoch_num;
                //Set the epoch of the satellite
                for(int i = 0;i < epochSatlitData.length();i++)
                    epochSatlitData[i].UTCTime.epochNum = epoch_num;
            }
            else
            {
                epochTime = last_allReciverPos.at(epoch_num).UTCtime;
            }

            // use spp compute postion and smooth pesudorange
            double temp_spp_pos[3] = {last_allReciverPos.at(epoch_num).spp_pos[0],
                                      last_allReciverPos.at(epoch_num).spp_pos[1],
                                      last_allReciverPos.at(epoch_num).spp_pos[2]};
            memcpy(spp_pos, temp_spp_pos, 3*sizeof(double));
            last_fillter_X = last_allSolverX.at(epoch_num);
            spp_pos[3] = last_fillter_X[4];// use prior Precise base clk as constraint 2019.12.30
            //Monitor satellite quality and cycle slip
            getGoodSatlite(prevEpochSatlitData,epochSatlitData, m_CutAngle);

            // The number of skipping satellites is less than m_minSatFlag
            // update at 2018.10.17 for less m_minSatFlag satellites at the begain observtion
            if(epochSatlitData.length() < m_minSatFlag || temp_spp_pos[0] == 0)
            {
                if(m_isKinematic&&continue_bad_epoch++ > 8)
                {
                    prevEpochSatlitData.clear();// Exception reinitialization
                    continue_bad_epoch = 0;
                }
                // add bad flag 777 for getGoodSatlite
                for(int i = 0;i < epochSatlitData.length();i++)
                    epochSatlitData[i].EpochFlag = 777;

//                prevEpochSatlitData.clear();
                disPlayQTextEdit = "GPST: " + QString::number(epochTime.Hours) + ":" + QString::number(epochTime.Minutes)
                        + ":" + QString::number(epochTime.Seconds) ;
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                disPlayQTextEdit = "Satellite number: " + QString::number(epochSatlitData.length())
                                        + "satellites number is less than 5.";
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                // translation to ENU
                VectorXd ENU_Vct;
                double spp_vct[3] = {0};
                ENU_Vct.resize(32+epochSatlitData.length());
                ENU_Vct.fill(0);
                saveResult2Class(ENU_Vct, spp_vct, epochTime, epochSatlitData, epoch_num);
                continue;
            }

            // Choose solve method Kalman or SRIF
            MatrixXd P;
            VectorXd X;//dX,dY,dZ,dT(Zenith tropospheric residual),dVt(Receiver clock)，N1,N2...Nm(Ambiguity)[dx,dy,dz,dTrop,dClock,N1,N2,...Nn]
            double spp_vct[3] = {0};// save pos before fillter
            bool is_filter_good = false;
            X.resize(5+epochSatlitData.length());
            X.setZero();
            // use prior information
            if(epoch_num == all_epoch_len - 1)
            {
                X = last_fillter_X;
                P = last_fillter_Q;
                // use diff pos of X = PPP - SPP
                for(int i = 0;i < 3;i++) X[i] = last_fillter_X[i] - temp_spp_pos[i];
            }
            // store spp position
            spp_vct[0] = temp_spp_pos[0]; spp_vct[1] = temp_spp_pos[1]; spp_vct[2] = temp_spp_pos[2];
            if (!m_Solver_Method.compare("SRIF", Qt::CaseInsensitive))
                is_filter_good = m_SRIFAlgorithm.SRIFforStatic(prevEpochSatlitData,epochSatlitData,spp_pos,X,P);
            else
                is_filter_good = m_KalmanClass.KalmanforStatic(prevEpochSatlitData,epochSatlitData,spp_pos,X,P);
            //Save the last epoch satellite data
            if(is_filter_good)
            {
                prevEpochSatlitData = epochSatlitData;
                continue_bad_epoch = 0;
            }
            else
            {
                epochSatlitData.clear();
                memset(spp_pos, 0, 3*sizeof(double));
                memset(spp_vct, 0, 3*sizeof(double));
                X.setZero();
            }
//Output calculation result(print result)
            // display every epoch results
            if(isDisplayEveryEpoch)
            {
                int Valid_SatNumber = epochSatlitData.length();
                // display epoch number
                disPlayQTextEdit = "Epoch number: " + QString::number(epoch_num);
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                //
                disPlayQTextEdit = "GPST: " + QString::number(epochTime.Hours) + ":" + QString::number(epochTime.Minutes)
                        + ":" + QString::number(epochTime.Seconds) + ENDLINE + "Satellite number: " + QString::number(epochSatlitData.length());
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                // display ENU or XYZ
                disPlayQTextEdit = "Valid sat number: " + QString::number(Valid_SatNumber) + ENDLINE
                        + "Estimated: " + '\n' + "X:   " + QString::number(spp_pos[0], 'f', 4)
                                        + '\n' + "Y:   " + QString::number(spp_pos[1], 'f', 4)
                                        + '\n' + "Z:   " + QString::number(spp_pos[2], 'f', 4) + ENDLINE;
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            }
//Save each epoch X data to prepare for writing a file
            // translation to ENU
            VectorXd ENU_Vct;
            ENU_Vct = X;// [dx,dy,dz,dTrop,dClock,N1,N2,...Nn]
            //ENU_Vct[0] = P(1,1); ENU_Vct[1] = P(2,2); ENU_Vct[2] = P(3,3);
            ENU_Vct[0] = spp_pos[0]; ENU_Vct[1] = spp_pos[1]; ENU_Vct[2] = spp_pos[2];
            saveResult2Class(ENU_Vct, spp_vct, epochTime, epochSatlitData, epoch_num, &P);
        }//End of multiple epochs    for (int n = 0; n < multepochSatlitData.length();n++)

        // clear multepochSatlitData
        multepochSatlitData.clear();

//Write result to file
    m_PPPModel_Str = "IF";
    reverseResult();// before writeResult2File() shhould reverse save result
    writeResult2File();
    m_isRuned = true;// Determine whether the operation is complete.
    // time end
    if(isDisplayEveryEpoch)
    {
        disPlayQTextEdit = "done";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
    }
}


// The edit box automatically scrolls, adding one row or more lines at a time.
void QPPPBackSmooth::autoScrollTextEdit(QTextEdit *textEdit,QString &add_text)
{
    if(textEdit == NULL) return ;
    int m_Display_Max_line = 99999;
    //Add line character and refresh edit box.
    QString insertText = add_text + ENDLINE;
    textEdit->insertPlainText(insertText);
    //Keep the editor in the last line of the cursor.
    QTextCursor cursor=textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    textEdit->setTextCursor(cursor);
    textEdit->repaint();
    QApplication::processEvents();
    //If you exceed a certain number of lines, empty it.
    if(textEdit->document()->lineCount() > m_Display_Max_line)
    {
        textEdit->clear();
    }
}

bool QPPPBackSmooth::connectHost()
{
    if(m_isConnect) return true;
    QString ftp_link = "cddis.gsfc.nasa.gov", user_name = "anonymous", user_password = "";//ftp information
    int Port = 21;
    ftpClient.FtpSetUserInfor(user_name, user_password);
    ftpClient.FtpSetHostPort(ftp_link, Port);
    m_isConnect = true;
    return true;
}

bool QPPPBackSmooth::connectCNTHost()
{
    if(m_isConnectCNT) return true;
    QString ftp_link = "ppp-wizard.net", user_name = "anonymous", user_password = "";//ftp information
    int Port = 21;
    ftpClient.FtpSetUserInfor(user_name, user_password);
    ftpClient.FtpSetHostPort(ftp_link, Port);
    m_isConnectCNT = true;
    return true;
}


//download Get erp file
// productType: "erp"
// GPS_Week: is GPS week.
QString QPPPBackSmooth::downErpFile(QString store_floder_path, int GPS_Week, int GPS_day, QString productType)
{
    QString erp_path_name = "";
    if(!m_isConnect)
        return erp_path_name;
    QString igs_short_path = "/pub/gps/products/", igs_productCompany = "igs";// set path of products
    QString gbm_short_path = "/pub/gps/products/mgex/", gbm_productCompany = "gbm";// set path of products
    QString igs_path_string = "", igs_erp_name = "";
    //net file path
    igs_path_string.append(igs_short_path);
    igs_path_string.append(QString::number(GPS_Week));
    igs_path_string.append("/");
    // file name
    igs_erp_name.append(igs_productCompany);
    igs_erp_name.append(QString::number(GPS_Week));
    igs_erp_name.append(QString::number(7));
    igs_erp_name.append(".erp.Z");
    // down load begin
    bool is_down_igs = false;
    QString disPlayQTextEdit = "download start!";
    autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
    QString igs_erp_path = igs_path_string + igs_erp_name,
            igs_temp_local_file = store_floder_path + igs_erp_name;
    if(!ftpClient.FtpGet(igs_erp_path, igs_temp_local_file))
    {
        disPlayQTextEdit = "download: " + igs_erp_path + " bad!";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
        is_down_igs = false;
    }
    else
    {
        disPlayQTextEdit = "download: " + igs_erp_path + " success!";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
        is_down_igs = true;
    }
    // save products file to fileList
    QString temp_local_file;
    if(is_down_igs)
        temp_local_file = igs_temp_local_file;

    // umcompress ".Z"
    QFileInfo fileinfo(temp_local_file);
    if(0 != fileinfo.size())
    {
        MyCompress compress;
        compress.UnCompress(temp_local_file, store_floder_path);
    }
    // return file path + name
    int extend_name = temp_local_file.lastIndexOf(".");
    erp_path_name = temp_local_file.mid(0, extend_name);
    if(!is_down_igs) erp_path_name = "";
    return erp_path_name;
}

//download Get sp3, clk file
// productType: "sp3", "clk"
// GPS_Week, GPS_Day: is GPS week and day.
// fileList: if fileList least 3. down new product and change fileList
QStringList QPPPBackSmooth::downProducts(QString store_floder_path, int GPS_Week, int GPS_day, QString productType)
{
    QString igs_short_path = "/pub/gps/products/", igs_productCompany = "igs";// set path of products
    QString gbm_short_path = "/pub/gps/products/mgex/", gbm_productCompany = "gbm";// set path of products
    QStringList fileList;
    fileList.clear();
    if(!m_isConnect) return fileList;
    //get fileList
    QStringList net_fileList, igs_net_fileList;
    net_fileList.clear();
    igs_net_fileList.clear();
    QString tempStr = "", fileName = "", disPlayQTextEdit = "";
    QStringList three_fileNames, igs_three_fileNames;

    //download  "sp3", "clk"
    for(int i = -1;i < 2; i++)
    {
        int temp_week = GPS_Week, temp_day = GPS_day + i;
        if(temp_day < 0)
        {
            temp_week--;
            temp_day = 6;
        }
        else if(temp_day > 6)
        {
            temp_week++;
            temp_day = 0;
        }
        // get gbm products
        tempStr.append(gbm_short_path);
        tempStr.append(QString::number(temp_week));
        tempStr.append("/");

        fileName.append(gbm_productCompany);
        fileName.append(QString::number(temp_week));
        fileName.append(QString::number(temp_day));

        if (productType.contains("sp3", Qt::CaseInsensitive))
            fileName.append(".sp3.Z");
        else if (productType.contains("clk", Qt::CaseInsensitive))
            fileName.append(".clk.Z");
        else
            return fileList;
        tempStr.append(fileName); // save filename and ftp path
        three_fileNames.append(fileName);
        net_fileList.append(tempStr);
        // clear temp data
        tempStr = ""; fileName = "";

        // get igs products
        tempStr.append(igs_short_path);
        tempStr.append(QString::number(temp_week));
        tempStr.append("/");

        fileName.append(igs_productCompany);
        fileName.append(QString::number(temp_week));
        fileName.append(QString::number(temp_day));

        if (productType.contains("sp3", Qt::CaseInsensitive))
            fileName.append(".sp3.Z");
        else if (productType.contains("clk", Qt::CaseInsensitive))
            fileName.append(".clk_30s.Z");//.clk_30s.Z or .clk.Z
        else
            return fileList;
        tempStr.append(fileName); // save filename and ftp path
        igs_three_fileNames.append(fileName);
        igs_net_fileList.append(tempStr);
        // clear temp data
        tempStr = ""; fileName = "";

    }
    // down three files
    disPlayQTextEdit = "download start!";
    autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
    for(int i = 0; i < 3;i++)
    {
        bool is_down_gbm = false, is_down_igs = false;
        QString gbm_temp_local_file = store_floder_path, igs_temp_local_file = store_floder_path;
        gbm_temp_local_file.append(three_fileNames.at(i));
        igs_temp_local_file.append(igs_three_fileNames.at(i));

        if(!ftpClient.FtpGet(igs_net_fileList.at(i), igs_temp_local_file))
        {
            disPlayQTextEdit = "download: " + igs_net_fileList.at(i) + " bad!";
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            if(!ftpClient.FtpGet(net_fileList.at(i), gbm_temp_local_file))
            {
                disPlayQTextEdit = "download: " + net_fileList.at(i) + " bad!";
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                is_down_gbm = false;
            }
            else
            {
                disPlayQTextEdit = "download: " + net_fileList.at(i) + " success!";
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                is_down_gbm = true;
            }

        }
        else
        {
            is_down_igs = true;
            disPlayQTextEdit = "download: " + igs_net_fileList.at(i) + " success!";
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
        }

        // save products file to fileList
        QString temp_local_file;
        if(is_down_gbm)
            temp_local_file = gbm_temp_local_file;
        else if(is_down_igs)
            temp_local_file = igs_temp_local_file;
        // If the download fail
        if(!is_down_gbm && !is_down_igs)
        {
            disPlayQTextEdit = "download: gbm or igs products bad, Procedure will terminate!";
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
        }
        int extend_name = temp_local_file.lastIndexOf(".");
        QString product_name = temp_local_file.mid(0, extend_name);
        // umcompress ".Z"
        QFileInfo fileinfo(temp_local_file);
        if(0 != fileinfo.size())
        {
            MyCompress compress;
            if(compress.UnCompress(temp_local_file, store_floder_path))
                fileList.append(product_name);
            else
                fileList.clear();
        }
    }
    return fileList;
}

// dowload CNT products, this product update only one day
QStringList QPPPBackSmooth::downCNTProducts(QString store_floder_path, int GPS_Week, int GPS_day, QString productType)
{
    QString cnt_short_path = "/PRODUCTS/REAL_TIME/", cnt_productCompany = "cnt";// set path of products
    QStringList fileList;
    fileList.clear();
    if(!m_isConnectCNT) return fileList;
    //get fileList
    QString tempStr = "", fileName = "", disPlayQTextEdit = "", net_path = "", local_path = "";
    bool is_down_cnt = false;

    // get cnt products
    tempStr.append(cnt_short_path);
    fileName.append(cnt_productCompany);
    fileName.append(QString::number(GPS_Week));
    fileName.append(QString::number(GPS_day));
    if (productType.contains("sp3", Qt::CaseInsensitive))
        fileName.append(".sp3.gz");
    else if (productType.contains("clk", Qt::CaseInsensitive))
        fileName.append(".clk.gz");
    else
        return fileList;
    tempStr.append(fileName); // save filename and ftp path
    net_path = tempStr;
    local_path = store_floder_path + fileName;
    if(!ftpClient.FtpGet(net_path, local_path))
    {
        is_down_cnt = false;
        disPlayQTextEdit = "download: " + net_path + " bad!";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
    }
    else
    {
        is_down_cnt = true;
        disPlayQTextEdit = "download: " + net_path + " success!";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
    }

    if(!is_down_cnt)
    {
        disPlayQTextEdit = "download: cnt products bad, Procedure will terminate!, Please wait 24 hours!";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
    }
    else
    {
        int extend_name = local_path.lastIndexOf(".");
        QString product_name = local_path.mid(0, extend_name);

        // umcompress ".Z"
        QFileInfo fileinfo(local_path);
        if(0 != fileinfo.size())
        {
            MyCompress compress;
            if(compress.UnCompress(local_path, store_floder_path))
                fileList.append(product_name);
            else
                fileList.clear();
        }
    }
    return fileList;

}


//Destructor
QPPPBackSmooth::~QPPPBackSmooth()
{
}

//Setting up the file system SystemStr:"G"(Turn on the GPS system);"GR":(Turn on the GPS+GLONASS system);"GRCE"(Open all)et al
//GPS, GLONASS, BDS, and Galieo are used respectively: the letters G, R, C, E
bool QPPPBackSmooth::setSatlitSys(QString SystemStr)
{
	bool IsGood = QBaseObject::setSatlitSys(SystemStr);
	//Set to read O files
	m_ReadOFileClass.setSatlitSys(SystemStr);
	//Set to read Sp3
	m_ReadSP3Class.setSatlitSys(SystemStr);
	//Set to read Clk files
	m_ReadClkClass.setSatlitSys(SystemStr);
	//Setting up the receiver satellite antenna system
	m_ReadAntClass.setSatlitSys(SystemStr);
	//Set kalman filtering system
	m_KalmanClass.setSatlitSys(SystemStr);
    //Set the system of m_writeFileClass
    m_writeFileClass.setSatlitSys(SystemStr);
    //Set the system of m_SRIFAlgorithm
    m_SRIFAlgorithm.setSatlitSys(SystemStr);
	return IsGood;
}

//Obtain the coordinates of the epoch satellite from the SP3 data data
void QPPPBackSmooth::getSP3Pos(double GPST,int PRN,char SatType,double *p_XYZ,double *pdXYZ)
{
	m_ReadSP3Class.getPrcisePoint(PRN,SatType,GPST,p_XYZ,pdXYZ);
}

//Get clock error from CLK data
void QPPPBackSmooth::getCLKData(int PRN,char SatType,double GPST,double *pCLKT)
{
	m_ReadClkClass.getStaliteClk(PRN,SatType,GPST,pCLKT);
}

//Earth autobiography correction
double QPPPBackSmooth::getSagnac(double X,double Y,double Z,double *approxRecvXYZ)
{//Calculate the autobiographic correction of the earth
	double dltaP = M_We*((X - approxRecvXYZ[0])*Y - (Y - approxRecvXYZ[1])*X)/M_C;
	return -dltaP;//Returns the opposite number such that p = p' + dltaP; can be added directly
}

void QPPPBackSmooth::getWight(SatlitData &tempSatlitData)
{
    double E = tempSatlitData.EA[0]*MM_PI/180;
    double SatWight = qSin(E)*qSin(E) / M_Zgama_P_square;//Set the weight of the satellite
    switch (tempSatlitData.SatType) {
    case 'R': SatWight = 0.75*SatWight; break;
    case 'C': case 'E': SatWight = 0.3*SatWight; break;
    default:
        break;
    }
    //Five satellites with poor GEO orbit in front of Beidou
    if(tempSatlitData.SatType == 'C' && tempSatlitData.PRN <= 5)
        SatWight = 0.01*SatWight;
    tempSatlitData.SatWight = SatWight;//Set the weight of the satellite    debug by xiaogongwei 2019.04.24
}

//Computational relativistic effect
double QPPPBackSmooth::getRelativty(double *pSatXYZ,double *pRecXYZ,double *pSatdXYZ)
{
	/*double c = 299792458.0;
	double dltaP = -2*(pSatXYZ[0]*pSatdXYZ[0] + pSatdXYZ[1]*pdXYZ[1] + pSatXYZ[2]*pSatdXYZ[2]) / c;*/
	double b[3] = {0},a = 0,R = 0,Rs = 0,Rr = 0,v_light = 299792458.0,GM=3.9860047e14,dltaP = 0;
	b[0] = pRecXYZ[0] - pSatXYZ[0];
	b[1] = pRecXYZ[1] - pSatXYZ[1];
	b[2] = pRecXYZ[2] - pSatXYZ[2];
	a = pSatXYZ[0]*pSatdXYZ[0] + pSatXYZ[1]*pSatdXYZ[1] + pSatXYZ[2]*pSatdXYZ[2];
	R=qCmpGpsT.norm(b,3);
	Rs = qCmpGpsT.norm(pSatXYZ,3);
	Rr = qCmpGpsT.norm(pRecXYZ,3);
	dltaP=-2*a/M_C + (2*M_GM/qPow(M_C,2))*qLn((Rs+Rr+R)/(Rs+Rr-R));
	return dltaP;//m
}

//Calculate EA, E: satellite elevation angle, A: azimuth
void QPPPBackSmooth::getSatEA(double X,double Y,double Z,double *approxRecvXYZ,double *EA)
{//Calculate EA//appear BUG Since XYZ to BLH is calculated, L (earth longitude) is actually opposite when y < 0, x > 0.L = -atan(y/x) error should be L = -atan(-y/x)
	double pSAZ[3] = {0};
	qCmpGpsT.XYZ2SAZ(X,Y,Z,pSAZ,approxRecvXYZ);//appear Bug
	EA[0] = (MM_PI/2 - pSAZ[2])*360/(2*MM_PI);
	EA[1] = pSAZ[1]*360/(2*MM_PI);
}


//Using the Sass model There are other models and projection functions that can be used, as well as the GPT2 model.
// ZHD_s: GNSS signal direction dry delay, ZHD: Station zenith direction dry delay
void QPPPBackSmooth::getTropDelay(double MJD,int TDay,double E,double *pBLH,double *mf, double *ZHD_s, double *ZPD)
{
    //double GPT2_Trop = m_ReadTropClass.getGPT2SasstaMDelay(MJD,TDay,E,pBLH,mf);//The GPT2 model only returns the dry delay estimate and the wet delay function.
    double tropDelayH = 0, tropDelayP = 0;
    if(m_TropDelay.mid(0,1).compare("U") == 0)
    {
        if(ZPD)     m_ReadTropClass.getUNB3mDelay(pBLH,TDay,E,mf, &tropDelayP);//Total delay of the UNB3M model
        if(ZHD_s)     tropDelayH = m_ReadTropClass.getUNB3mDelay(pBLH,TDay,E,mf);//UNB3M model only returns dry delay
    }
    else if(m_TropDelay.mid(0,1).compare("S") == 0)
    {
        if(ZPD)     m_ReadTropClass.getGPT2SasstaMDelay(MJD,TDay,E,pBLH,mf, &tropDelayP);//GPT2 model total delay
        if(ZHD_s)     tropDelayH = m_ReadTropClass.getGPT2SasstaMDelay(MJD,TDay,E,pBLH,mf);//GPT2 model only returns dry delay
    }
    else
    {
        if(ZPD)     m_ReadTropClass.getGPT2HopfieldDelay(MJD,TDay,E,pBLH,mf, &tropDelayP);//GPT2 model total delay
        if(ZHD_s)     tropDelayH = m_ReadTropClass.getGPT2HopfieldDelay(MJD,TDay,E,pBLH,mf);//GPT2 model only returns dry delay
    }

    // juge tropDely is nan or no Meaningless
    if(qAbs(tropDelayH) > 100  || qAbs(pBLH[2]) > 50000)
        tropDelayH = 1e-6;
    else if(!isnormal(tropDelayH))
        tropDelayH = 1e-6;
    if(qAbs(tropDelayP) > 100  || qAbs(pBLH[2]) > 50000)
        tropDelayP = 1e-6;
    else if(!isnormal(tropDelayP))
        tropDelayP = 1e-6;

    if(ZHD_s)  *ZHD_s = tropDelayH;
    if(ZPD)  *ZPD = tropDelayP;
    return ;
}


//Calculate the receivers L1 and L2 phase center correction PCO + PCV, L1Offset and L2Offset represent the distance correction of the line of sight direction
bool QPPPBackSmooth::getRecvOffset(double *EA,char SatType,double &L1Offset,double &L2Offset, QVector<QString> FrqFlag)
{
    if (m_ReadAntClass.getRecvL12(EA[0],EA[1],SatType,L1Offset,L2Offset, FrqFlag))
        return true;
    else
    {
        L1Offset = 0; L2Offset = 0;
        return false;
    }
}

//Calculate the satellite PCO+PCV correction, because the satellite G1 and G2 frequencies are the same, so the two bands change exactly the same; StaPos and RecPos, satellite and receiver WGS84 coordinates (unit m)
//L12Offset
bool QPPPBackSmooth::getSatlitOffset(int Year,int Month,int Day,int Hours,int Minutes,double Seconds,int PRN,char SatType,double *StaPos,double *RecPos,
                                  double *L12Offset, QVector<QString> FrqFlag)
{
    bool isGood = m_ReadAntClass.getSatOffSet(Year,Month,Day,Hours,Minutes,Seconds,PRN,SatType,StaPos,RecPos, L12Offset, FrqFlag);//pXYZ saves satellite coordinates
    if(isGood)
        return true;
    {
        L12Offset[0] = 0; L12Offset[1] = 0;
        return false;
    }

}

//Calculate the correction of the tide in the direction of the line of sight (unit m)
double QPPPBackSmooth::getTideEffect(int Year,int Month,int Day,int Hours,int Minutes,double Seconds,
                                double *pXYZ,double *EA,double *psunpos/* =NULL */, double *pmoonpos /* = NULL */,double gmst /* = 0 */,QString StationName /* = "" */)
{
	return m_TideEffectClass.getAllTideEffect(Year,Month,Day,Hours,Minutes,Seconds,pXYZ,EA,psunpos,pmoonpos,gmst,StationName);
}

//SatPos and RecPos represent the satellite and receiver WGS84 coordinates. Return to the weekly (unit week) range [-0.5 +0.5]
double QPPPBackSmooth::getWindup(int Year,int Month,int Day,int Hours,int Minutes,double Seconds,double *StaPos,double *RecPos,double &phw,double *psunpos)
{
	return m_WinUpClass.getWindUp(Year,Month,Day,Hours,Minutes,Seconds,StaPos,RecPos,phw,psunpos);
}


//Detect cycle hops: return pLP is a three-dimensional array, the first is the W-M combination (N2-N1 < 3.5) The second number ionospheric residual (<0.3) The third is (lamt2*N2-lamt1*N1 < 3.5)
bool QPPPBackSmooth::CycleSlip(const SatlitData &oneSatlitData,double *pLP)
{//
    if (oneSatlitData.L1*oneSatlitData.L2*oneSatlitData.C1*oneSatlitData.C2 == 0)//Determine whether it is dual-frequency data
        return false;
    double F1 = oneSatlitData.Frq[0],F2 = oneSatlitData.Frq[1];//Get the frequency of this satellite
    double Lamta1 = M_C/F1,Lamta2 = M_C/F2;
    //MW Combination(1)
//    double NL12 = ((F1-F2)/(F1+F2))*(oneSatlitData.C1/Lamta1 + oneSatlitData.C2/Lamta2) - (oneSatlitData.L1 - oneSatlitData.L2);
    //MW Combination(2)
    double lamdaMW = M_C/(F1 - F2);
    double NL12 = (oneSatlitData.L1 - oneSatlitData.L2) - (F1*oneSatlitData.C1 + F2*oneSatlitData.C2)/((F1+F2)*lamdaMW);
    double IocL = Lamta1*oneSatlitData.L1 - Lamta2*oneSatlitData.L2;//Ionospheric delayed residual
    double IocLP = IocL+(oneSatlitData.C1 - oneSatlitData.C2);
    pLP[0] = NL12;
    pLP[1] =IocL;
    pLP[2] = IocLP;
    return true;
}

//Save the phase entanglement of the previous epoch
double QPPPBackSmooth::getPreEpochWindUp(QVector< SatlitData > &prevEpochSatlitData,int PRN,char SatType)
{
	int preEopchLen = prevEpochSatlitData.length();
	if (0 == preEopchLen) return 0;

	for (int i = 0;i < preEopchLen;i++)
	{
		SatlitData oneSatalite = prevEpochSatlitData.at(i);
		if (PRN == oneSatalite.PRN&&oneSatalite.SatType == SatType)
			return oneSatalite.AntWindup;
	}
	return 0;
}


//Repair receiver clock jump
void QPPPBackSmooth::reciveClkRapaire(QVector< SatlitData > &prevEpochSatlitData,QVector< SatlitData > &epochSatlitData)
{
    int preEpochLen = prevEpochSatlitData.length();
    int epochLen = epochSatlitData.length();
    // clock jump repair Debug by xiaogongwei 2019.03.30
    double sum_S = 0.0, k1 = 0.01*M_C, M = 0.0, jump = 0.0;// for repair clock
    int clock_num = 0, check_sat_num = 0;// for repair clock
    for (int i = 0;i < epochLen;i++)
    {
        SatlitData epochData = epochSatlitData.at(i);
        for (int j = 0;j < preEpochLen;j++)
        {
            SatlitData preEpochData = prevEpochSatlitData.at(j);
            if (epochData.PRN == preEpochData.PRN&&epochData.SatType == preEpochData.SatType)
            {
                check_sat_num++;
                double Lamta1 = M_C / preEpochData.Frq[0], Lamta2 = M_C / preEpochData.Frq[1];
                double IocL1 = Lamta1*preEpochData.L1 - Lamta2*preEpochData.L2,
                        IocL2 = Lamta1*epochData.L1 - Lamta2*epochData.L2;//Ionospheric delayed residual
                if(qAbs(IocL2 - IocL1) < M_IR)
                {
                    double dP3 = epochData.PP3 - preEpochData.PP3, dL3 = epochData.LL3 - preEpochData.LL3;
                    double Si = (dP3 - dL3);// (m)
                    // judge clock jump type
                    if(qAbs(dP3) >= 0.001*M_C && m_clock_jump_type == 0)
                        m_clock_jump_type = 1;
                    if(qAbs(dL3) >= 0.001*M_C && m_clock_jump_type == 0)
                        m_clock_jump_type = 2;
                    if(qAbs(Si) >= 0.001*M_C)
                    {
                        sum_S += Si;
                        clock_num++;
                    }
                }
            }
        }

    }
    // repair clock
    double jump_pro = (double)clock_num / check_sat_num;
    if(jump_pro > 0.8)
    {
        M = 1e3*sum_S / (M_C*clock_num);// unit:ms
        if(qAbs(qRound(M) - M) <= 1e-5)
            jump = qRound(M);
        else
            jump = 0;
    }
    if(jump != 0)
    {
        for (int i = 0;i < epochLen;i++)
        {
            SatlitData tempSatlitData = epochSatlitData[i];
            double F1 = tempSatlitData.Frq[0],F2 = tempSatlitData.Frq[1];
            double Lamta1 = M_C/F1,Lamta2 = M_C/F2;
            double alpha1 = (F1*F1)/(F1*F1 - F2*F2),alpha2 = (F2*F2)/(F1*F1 - F2*F2);
            if(m_clock_jump_type == 1)
            {// clock jump type I
                tempSatlitData.L1 = tempSatlitData.L1 + 1e-3*jump*M_C / Lamta1;
                tempSatlitData.L2 = tempSatlitData.L2 + 1e-3*jump*M_C / Lamta2;
                tempSatlitData.LL3 = alpha1*(tempSatlitData.L1 + tempSatlitData.L1Offset + tempSatlitData.SatL1Offset - tempSatlitData.AntWindup)*Lamta1
                        - alpha2*(tempSatlitData.L2 + tempSatlitData.L2Offset + tempSatlitData.SatL2Offset - tempSatlitData.AntWindup)*Lamta2;//Eliminate ionospheric carrier LL3

            }
            else if(m_clock_jump_type == 2)
            {// clock jump type II
                tempSatlitData.C1 = tempSatlitData.C1 - 1e-3*jump*M_C / Lamta1;
                tempSatlitData.C2 = tempSatlitData.C2 - 1e-3*jump*M_C / Lamta2;
                tempSatlitData.PP3 = alpha1*(tempSatlitData.C1 + Lamta1*tempSatlitData.L1Offset + Lamta1*tempSatlitData.SatL1Offset)
                        - alpha2*(tempSatlitData.C2 + Lamta2 *tempSatlitData.L2Offset + Lamta2*tempSatlitData.SatL2Offset);//Eliminate ionospheric carrier PP3
            }
            epochSatlitData[i] = tempSatlitData;
        }
    }
}

//Screening satellites that do not have missing data and detect cycle slips, high quality (height angles, ranging codes, etc.)
void QPPPBackSmooth::getGoodSatlite(QVector< SatlitData > &prevEpochSatlitData,QVector< SatlitData > &epochSatlitData,double eleAngle)
{
    int preEpochLen = prevEpochSatlitData.length();
    int epochLen = epochSatlitData.length();

    reciveClkRapaire(prevEpochSatlitData, epochSatlitData);

    //Cycle slip detection
    QVector< int > CycleFlag;//Record the position of the weekly jump
    CycleFlag.resize(epochLen);
    for (int i = 0;i < epochLen;i++) CycleFlag[i] = 0;
    for (int i = 0;i < epochLen;i++)
    {
        SatlitData epochData = epochSatlitData.at(i);
        //Data is not 0
        if (!(epochData.L1&&epochData.L2&&epochData.C1&&epochData.C2)) // debug xiaogongwei 2018.11.16
            CycleFlag[i] = -1;
        //The corrections are not zero
        if (!(epochData.X&&epochData.Y&&epochData.Z&&epochData.StaClock)) // debug xiaogongwei 2018.11.16
            CycleFlag[i] = -1;
        //Quality control (height angle, pseudorange difference)
        if (epochData.EA[0] < eleAngle || qAbs(epochData.C1 - epochData.C2) > 50)
            CycleFlag[i] = -1;
        // signal intensity
        if(epochData.SigInten >0 && epochData.SigInten < 0)
            CycleFlag[i] = -1;
        //Cycle slip detection
        for (int j = 0;j < preEpochLen;j++)//(jump == 0) not happen clock jump  && (jump == 0)
        {
            SatlitData preEpochData = prevEpochSatlitData.at(j);
            if (epochData.PRN == preEpochData.PRN&&epochData.SatType == preEpochData.SatType)
            {//Need to judge the system
                double epochLP[3]={0},preEpochLP[3]={0},diffLP[3]={0};
                CycleSlip(epochData,epochLP);
                CycleSlip(preEpochData,preEpochLP);
                for (int n = 0;n < 3;n++)
                    diffLP[n] = qAbs(epochLP[n] - preEpochLP[n]);
                //Determine the weekly jump threshold based on experience.
                // diffLP[2] No longer use 2011.08.24,  diffLP[1] The ionospheric residual threshold is set to 0.1m
                if (diffLP[0] > 5 ||diffLP[1] > M_IR||diffLP[2] > 99999 || qAbs(epochData.AntWindup - preEpochData.AntWindup) > 0.3)
                {//Weekly jump
                    CycleFlag[i] = -1;//Save the weekly jump satellite logo
                }
                break;
            }
            else
            {
                continue;
            }
        }
    }
    //Remove low quality and weekly hop satellites
    QVector< SatlitData > tempEpochSatlitData;
    for (int i = 0;i < epochLen;i++)
    {
        if (CycleFlag.at(i) != -1)
        {
            tempEpochSatlitData.append(epochSatlitData.at(i));
        }
    }
    epochSatlitData = tempEpochSatlitData;

}

void QPPPBackSmooth::reverseResult()
{
    reverse(m_writeFileClass.allReciverPos.begin(), m_writeFileClass.allReciverPos.end());
    reverse(m_writeFileClass.allClock.begin(), m_writeFileClass.allClock.end());
    reverse(m_writeFileClass.allAmbiguity.begin(), m_writeFileClass.allAmbiguity.end());
    reverse(m_writeFileClass.allPPPSatlitData.begin(), m_writeFileClass.allPPPSatlitData.end());
    reverse(m_writeFileClass.allSolverX.begin(), m_writeFileClass.allSolverX.end());
    reverse(m_writeFileClass.allSloverQ.begin(), m_writeFileClass.allSloverQ.end());
}

//void QPPPBackSmooth::saveResult2Class(VectorXd X, double *spp_vct, GPSPosTime epochTime, QVector< SatlitData > epochResultSatlitData,
//                                      int epochNum, MatrixXd *P)
//{
//    //Store coordinate data
//    RecivePos epochRecivePos;
//    epochRecivePos.Year = epochTime.Year;epochRecivePos.Month = epochTime.Month;
//    epochRecivePos.Day = epochTime.Day;epochRecivePos.Hours = epochTime.Hours;
//    epochRecivePos.Minutes = epochTime.Minutes;epochRecivePos.Seconds = epochTime.Seconds;

//    epochRecivePos.totolEpochStalitNum = epochResultSatlitData.length();
//    epochRecivePos.dX = X[0];
//    epochRecivePos.dY = X[1];
//    epochRecivePos.dZ = X[2];
//    epochRecivePos.spp_pos[0] = spp_vct[0];
//    epochRecivePos.spp_pos[1] = spp_vct[1];
//    epochRecivePos.spp_pos[2] = spp_vct[2];
//    m_writeFileClass.allReciverPos.append(epochRecivePos);
//    //Save wet delay and receiver clock error
//    double epoch_ZHD = 0.0;
//    int const_num = 4 + m_sys_num;
//    if(epochResultSatlitData.length() >= m_minSatFlag) epoch_ZHD = epochResultSatlitData.at(0).UTCTime.TropZHD;
//    ClockData epochRecClock;
//    epochRecClock.UTCTime.epochNum = epochNum;
//    epochRecClock.UTCTime.Year = epochRecivePos.Year;epochRecClock.UTCTime.Month = epochRecivePos.Month;epochRecClock.UTCTime.Day = epochRecivePos.Day;
//    epochRecClock.UTCTime.Hours = epochRecivePos.Hours;epochRecClock.UTCTime.Minutes = epochRecivePos.Minutes;epochRecClock.UTCTime.Seconds = epochRecivePos.Seconds;
//    if(X(3) == 0)
//        epochRecClock.ZTD_W = 0;
//    else
//        epochRecClock.ZTD_W = X(3) + epoch_ZHD;//Storage zenith wet delay + zenith dry delay
//    // save clock
//    memset(epochRecClock.clockData, 0, 6*sizeof(double));
//    //Store the receiver skew of the first system, and the relative offset of other systems. GCRE
//    for(int i = 0;i < m_sys_str.length();i++)
//    {
//        switch (m_sys_str.at(i).toLatin1()) {
//        case 'G':
//            epochRecClock.clockData[0] = X(4+i);
//            break;
//        case 'C':
//            epochRecClock.clockData[1] = X(4+i);
//            break;
//        case 'R':
//            epochRecClock.clockData[2] = X(4+i);
//            break;
//        case 'E':
//            epochRecClock.clockData[3] = X(4+i);
//            break;
//        default:
//            break;
//        }
//    }
//    m_writeFileClass.allClock.append(epochRecClock);
//    //Save satellite ambiguity
//    Ambiguity oneSatAmb;
//    for (int i = 0;i < epochResultSatlitData.length();i++)
//    {
//        SatlitData oneSat = epochResultSatlitData.at(i);
//        oneSatAmb.PRN = oneSat.PRN;
//        oneSatAmb.SatType = oneSat.SatType;
//        oneSatAmb.UTCTime = epochRecClock.UTCTime;
//        oneSatAmb.isIntAmb = false;
//        oneSatAmb.Amb = X(i+const_num);
//        oneSatAmb.UTCTime.epochNum = epochNum;
//        m_writeFileClass.allAmbiguity.append(oneSatAmb);
//    }
//    // save used satlite Information
//    m_writeFileClass.allPPPSatlitData.append(epochResultSatlitData);
//    // save solver X
//    m_writeFileClass.allSolverX.append(X);
//    // save P matrix
//    if(P)
//        m_writeFileClass.allSloverQ.append(*P);
////    else
////        m_writeFileClass.allSloverQ.prepend(MatrixXd::Identity(10,10));
//}


void QPPPBackSmooth::saveResult2Class(VectorXd X, double *spp_vct, GPSPosTime epochTime, QVector< SatlitData > epochResultSatlitData,
                                 int epochNum, MatrixXd *P)
{
    //Store coordinate data
    RecivePos epochRecivePos;
    epochTime.epochNum = epochNum;
    epochRecivePos.UTCtime = epochTime;

    epochRecivePos.totolEpochStalitNum = epochResultSatlitData.length();
    epochRecivePos.dX = X[0];
    epochRecivePos.dY = X[1];
    epochRecivePos.dZ = X[2];
    epochRecivePos.spp_pos[0] = spp_vct[0];
    epochRecivePos.spp_pos[1] = spp_vct[1];
    epochRecivePos.spp_pos[2] = spp_vct[2];
    m_writeFileClass.allReciverPos.append(epochRecivePos);
    //Save wet delay and receiver clock error
    double epoch_ZHD = 0.0;
    int const_num = 4 + m_sys_num;

    if(epochResultSatlitData.length() >= m_minSatFlag) epoch_ZHD = epochResultSatlitData.at(0).UTCTime.TropZHD;
    ClockData epochRecClock;
    epochRecClock.UTCTime = epochRecivePos.UTCtime;
    if(X(3) == 0)
        epochRecClock.ZTD_W = 0;
    else
        epochRecClock.ZTD_W = X(3) + epoch_ZHD;//Storage zenith wet delay + zenith dry delay
    // save clock
    memset(epochRecClock.clockData, 0, 6*sizeof(double));
    //Stores the receiver skew of the first system, and the relative offset of its other systems  GCRE
//    if(!epochResultSatlitData.at(0).SSDPPP && epochResultSatlitData.length() != 0)
//    {
//        for(int i = 0;i < m_sys_str.length();i++)
//        {
//            switch (m_sys_str.at(i).toLatin1()) {
//            case 'G':
//                epochRecClock.clockData[0] = X(4+i);
//                break;
//            case 'C':
//                epochRecClock.clockData[1] = X(4+i);
//                break;
//            case 'R':
//                epochRecClock.clockData[2] = X(4+i);
//                break;
//            case 'E':
//                epochRecClock.clockData[3] = X(4+i);
//                break;
//            default:
//                break;
//            }
//        }
//    }

    m_writeFileClass.allClock.append(epochRecClock);
    if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
    {
        int sat_num = epochResultSatlitData.length();
        if(epochResultSatlitData.at(0).SSDPPP)
            sat_num = epochResultSatlitData.length() - 1;
        //Save satellite ambiguity
        Ambiguity oneSatAmb;
        for (int i = 0;i < sat_num;i++)
        {    
            SatlitData oneSat = epochResultSatlitData.at(i);
            oneSatAmb.PRN = oneSat.PRN;
            oneSatAmb.SatType = oneSat.SatType;
            oneSatAmb.UTCTime = epochRecClock.UTCTime;
            oneSatAmb.isIntAmb = false;
            epochResultSatlitData[i].ionL1 = X(i+const_num);
            oneSatAmb.ionL1 = X(i+const_num);
            memcpy(oneSatAmb.EA, oneSat.EA, 2*sizeof(double));
            oneSatAmb.Amb1 = X(i+const_num+sat_num);
            oneSatAmb.Amb2 = X(i+const_num+2*sat_num);
            oneSatAmb.Amb = 0.0;
            oneSatAmb.UTCTime.epochNum = epochNum;
            m_writeFileClass.allAmbiguity.append(oneSatAmb);
        }
    }
    else if(getPPPModel() == PPP_MODEL::PPP_Combination)
    {
        //Save satellite ambiguity
        Ambiguity oneSatAmb;
        for (int i = 0;i < epochResultSatlitData.length();i++)
        {
            SatlitData oneSat = epochResultSatlitData.at(i);

            if(oneSat.SSDPPP)
            {
                oneSatAmb.PRN = oneSat.PRN;
                oneSatAmb.SatType = oneSat.SatType;
                oneSatAmb.UTCTime = epochRecClock.UTCTime;
                oneSatAmb.isIntAmb = false;
                memcpy(oneSatAmb.EA, oneSat.EA, 2*sizeof(double));
                oneSatAmb.ionL1 = 0.0;
                oneSatAmb.Amb1 = 0.0;
                oneSatAmb.Amb2 = 0.0;
                epochResultSatlitData[i].ionL1 = 0.0;
                epochResultSatlitData[i].Amb1 = 0.0;
                epochResultSatlitData[i].Amb2 = 0.0;
                oneSatAmb.UTCTime.epochNum = epochNum;
                if(i < oneSat.prn_referencesat[1])
                {
                    oneSatAmb.amb_diff_sats = X(i + 4);
                    epochResultSatlitData[i].amb_SSD_sats = X(i + 4);
                }
                else if(oneSat.prn_referencesat[1] < i && i < oneSat.prn_referencesat[3])
                {
                    oneSatAmb.amb_diff_sats = X(i + 3);
                    epochResultSatlitData[i].amb_SSD_sats = X(i + 3);
                }
                else if(oneSat.prn_referencesat[3] < i && i < oneSat.prn_referencesat[5])
                {
                    oneSatAmb.amb_diff_sats = X(i + 2);
                    epochResultSatlitData[i].amb_SSD_sats = X(i + 2);
                }
                else if(oneSat.prn_referencesat[5] < i && oneSat.prn_referencesat[5] != 0)
                {
                    oneSatAmb.amb_diff_sats = X(i + 1);
                    epochResultSatlitData[i].amb_SSD_sats = X(i + 1);
                }
                else
                {
                    oneSatAmb.amb_diff_sats = 0;
                    epochResultSatlitData[i].amb_SSD_sats = 0;
                }
            }
            else
            {
                oneSatAmb.PRN = oneSat.PRN;
                oneSatAmb.SatType = oneSat.SatType;
                oneSatAmb.UTCTime = epochRecClock.UTCTime;
                oneSatAmb.isIntAmb = false;
                memcpy(oneSatAmb.EA, oneSat.EA, 2*sizeof(double));
                oneSatAmb.ionL1 = 0.0;
                oneSatAmb.Amb1 = 0.0;
                oneSatAmb.Amb2 = 0.0;
                oneSatAmb.Amb = X(i+const_num);
                epochResultSatlitData[i].ionL1 = 0.0;
                epochResultSatlitData[i].Amb1 = 0.0;
                epochResultSatlitData[i].Amb2 = 0.0;
                epochResultSatlitData[i].Amb = X(i+const_num);
                oneSatAmb.UTCTime.epochNum = epochNum;
            }

            m_writeFileClass.allAmbiguity.append(oneSatAmb);
        }
    }
    // save used satlite Information
    m_writeFileClass.allPPPSatlitData.append(epochResultSatlitData);
    // save solver X
    m_writeFileClass.allSolverX.append(X);
    // save P matrix
    if(P)
        m_writeFileClass.allSloverQ.append(*P);
    else
        m_writeFileClass.allSloverQ.append(MatrixXd::Identity(32,32) * 1e10);
}

void QPPPBackSmooth::writeResult2File()
{
    QString product_path = RPR_filepath.at(2) + PATHSEG, ambiguit_floder;
    // product name
    QString Product_name = "IGS";
    if(m_Sp3FileNames.length() > 0)
    {
        QString sp3name_path = m_Sp3FileNames.at(0);
        int index_p = sp3name_path.lastIndexOf("/");// get sp3 name
        QString sp3name = sp3name_path.mid(index_p+1);
        Product_name = sp3name.mid(0,3).toUpper();
    }
    QString floder_name = m_markname+m_ot_UTC+ "_" +Product_name+ "_Backsmooth_" +m_Solver_Method+ "_" +m_PPPModel_Str + "_Static_" + m_sys_str + PATHSEG;
    if(m_isKinematic)
        floder_name = m_markname+m_ot_UTC+ "_" +Product_name+ "_Backsmooth_" +m_Solver_Method+ "_" +m_PPPModel_Str + "_Kinematic_" + m_sys_str + PATHSEG;
    if(global_cf::flag_seismology)
        floder_name = m_markname+m_ot_UTC+ "_" +Product_name+ "_Backsmooth_" +m_Solver_Method+ "_" +m_PPPModel_Str + "_Seismologic_" + m_sys_str +PATHSEG;

    product_path.append(floder_name);
    m_floder_name = floder_name;
    // save images path
    m_save_images_path = product_path;
    ambiguit_floder = product_path + QString("Ambiguity") + PATHSEG;
    //m_writeFileClass.WriteEpochPRN(product_path, "Epoch_PRN.txt");
    QString outputfile = m_markname+m_ot_UTC+"_position.txt";
    m_writeFileClass.writeRecivePos2Txt(product_path, outputfile);
    outputfile = m_markname+m_ot_UTC+"_modelling.txt";
    m_writeFileClass.writePPP2Txt(product_path, outputfile);
    outputfile = m_markname+m_ot_UTC+"_ZTD_Clock.txt";
    m_writeFileClass.writeClockZTDW2Txt(product_path, outputfile);
    outputfile = m_markname+m_ot_UTC+"_bad_satellites.txt";
    m_writeFileClass.writeBadSatliteData(product_path, outputfile);
    //m_writeFileClass.writeAmbiguity2Txt(ambiguit_floder);//The path is .//Ambiguity//
    //m_writeFileClass.writeRecivePosKML(product_path, "position.kml");// gernerate KML
}

// Get operation results( clear QWrite2File::allPPPSatlitData Because the amount of data is too large.)
void QPPPBackSmooth::getRunResult(PlotGUIData &plotData)
{
    int dataLen = m_writeFileClass.allReciverPos.length();
    if(dataLen == 0 || !m_isRuned) return ;
    if(!m_save_images_path.isEmpty())  plotData.save_file_path = m_save_images_path;

    for(int i = 0;i < dataLen;i++)
    {
        plotData.X.append(m_writeFileClass.allReciverPos.at(i).dX);
        plotData.Y.append(m_writeFileClass.allReciverPos.at(i).dY);
        plotData.Z.append(m_writeFileClass.allReciverPos.at(i).dZ);
        plotData.spp_X.append(m_writeFileClass.allReciverPos.at(i).spp_pos[0]);
        plotData.spp_Y.append(m_writeFileClass.allReciverPos.at(i).spp_pos[1]);
        plotData.spp_Z.append(m_writeFileClass.allReciverPos.at(i).spp_pos[2]);
        plotData.clockData.append(m_writeFileClass.allClock.at(i).clockData[0]);// GPS clock
        plotData.clockData_bias_1.append(m_writeFileClass.allClock.at(i).clockData[1]);//clock bias maybe is zero
        plotData.clockData_bias_2.append(m_writeFileClass.allClock.at(i).clockData[2]);//clock bias maybe is zero
        plotData.clockData_bias_3.append(m_writeFileClass.allClock.at(i).clockData[3]);//clock bias maybe is zero
        plotData.ZTD_W.append(m_writeFileClass.allClock.at(i).ZTD_W);
    }
}

//SSD反向平滑 2021.03.11 23z
void QPPPBackSmooth::SSDPPP(bool isdisplay)
{
    if(!m_haveObsFile) return ;

    //前向滤波
    QPPPModel *myPPP = new QPPPModel(RPR_filepath, mp_QTextEditforDisplay, m_Solver_Method, m_SatSystem,
                        m_TropDelay, m_CutAngle, m_isKinematic, m_Smooth_Str, m_Product, m_PPPModel_Str);
    myPPP->runSSDPPP(isdisplay);
    m_isRuned = myPPP->isRuned();
    if(!m_isRuned) return;

    //获取前向滤波参数
    int forWard_epoch_len = myPPP->m_writeFileClass.allSolverX.length();
    if(forWard_epoch_len == 0) return ;
    QVector< RecivePos > last_allReciverPos = myPPP->m_writeFileClass.allReciverPos;
    QVector< VectorXd > last_allSolverX = myPPP->m_writeFileClass.allSolverX;
    QVector< MatrixXd > last_allSolverQ = myPPP->m_writeFileClass.allSloverQ;
    VectorXd last_fillter_X;
    MatrixXd last_fillter_Q;
    for(int i = forWard_epoch_len - 1; i >=0; i--)
    {
        last_fillter_X = last_allSolverX.at(i);
        if(last_fillter_X[0] != 0)
        {
            last_fillter_X = last_allSolverX.at(i);
            last_fillter_Q = last_allSolverQ.at(i);
            break;
        }
    }
    QVector< QVector < SatlitData > > last_allPPPSatlitData = myPPP->m_writeFileClass.allPPPSatlitData;

    delete myPPP;
    m_ReadSP3Class.ACCELERATE = 0;
    m_ReadClkClass.ACCELERATE = 0;

    double spp_pos[3] = {last_fillter_X[0], last_fillter_X[1], last_fillter_X[2]};
    QVector< QVector < SatlitData > > multepochSatlitData;
    multepochSatlitData = last_allPPPSatlitData;
    int all_epoch_len = multepochSatlitData.length();
    double continue_bad_epoch = 0;
    QVector < SatlitData > prevEpochSatlitData;

    for (int epoch_num = all_epoch_len - 1; epoch_num >= 0;epoch_num--)
    {
        QVector< SatlitData > epochSatlitData;
        epochSatlitData = multepochSatlitData.at(epoch_num);

        GPSPosTime epochTime;
        if(epochSatlitData.length() != 0)
        {
            epochTime = epochSatlitData.at(0).UTCTime;
            for(int i = 0;i < epochSatlitData.length();i++)
                epochSatlitData[i].UTCTime.epochNum = epoch_num;
        }
        else
        {
            epochTime = last_allReciverPos.at(epoch_num).UTCtime;
        }

        double temp_spp_pos[3] = {last_allReciverPos.at(epoch_num).spp_pos[0],
                                  last_allReciverPos.at(epoch_num).spp_pos[1],
                                  last_allReciverPos.at(epoch_num).spp_pos[2]};
        memcpy(spp_pos, temp_spp_pos, 3*sizeof(double));
        last_fillter_X = last_allSolverX.at(epoch_num);

        if(epochSatlitData.length() < m_minSatFlag || temp_spp_pos[0] == 0)
        {
            if(m_isKinematic&&continue_bad_epoch++ > 8)
            {
                prevEpochSatlitData.clear();
                continue_bad_epoch = 0;
            }
            for(int i = 0;i < epochSatlitData.length();i++)
                epochSatlitData[i].EpochFlag = 777;
            VectorXd ENU_Vct;
            double spp_vct[3] = {0};
            ENU_Vct.resize(32+epochSatlitData.length());
            ENU_Vct.fill(0);
            saveResult2Class(ENU_Vct, spp_vct, epochTime, epochSatlitData, epoch_num);
            continue;
        }

        QString sys_cur = ""; int sys_num_cur = 0;
        sys_cur = epochSatlitData.at(0).SatType;
        sys_num_cur = 1;
        //The number of satellite systems in the current epoch
        for(int cf_i = 1; cf_i < epochSatlitData.length(); cf_i++)
        {
            if(!sys_cur.contains(epochSatlitData.at(cf_i).SatType))
                sys_num_cur++, sys_cur = sys_cur + epochSatlitData.at(cf_i).SatType;
        }

        if(prevEpochSatlitData.length() != 0)
        {
            //Select the reference sat according to the satellite elevation
            m_getbia.select_refsat(epochSatlitData, prevEpochSatlitData, m_CutAngle, sys_cur);
            m_getbia.get_refsat_sit(epochSatlitData, sys_cur);
        }

        //Get the reference sat prn of the previous epoch
        for(int cf = 0; cf < epochSatlitData.length(); cf ++)
        {
            if(prevEpochSatlitData.length() == 0)
            {
                for(int cf_j = 0; cf_j < 2*sys_num_cur; cf_j++)
                {
                    epochSatlitData[cf].prn_referencesat_previous[cf_j] = 0;
                }

            }
            else
            {
                for(int cf_j = 0; cf_j < 2*sys_num_cur; cf_j++)
                {
                    epochSatlitData[cf].prn_referencesat_previous[cf_j]
                            = prevEpochSatlitData[0].prn_referencesat[cf_j];
                }
            }
        }


        MatrixXd P;
        VectorXd X;
        double spp_vct[3] = {0};
        bool is_filter_good = false;
        X.resize(3 + epochSatlitData.length());
        X.setZero();

        if(epoch_num == all_epoch_len - 1)
        {
            X = last_fillter_X;
            P = last_fillter_Q;
            for(int i = 0;i < 3;i++) X[i] = last_fillter_X[i] - temp_spp_pos[i];
            //prevEpochSatlitData = multepochSatlitData.at(epoch_num - 1);
        }

        spp_vct[0] = temp_spp_pos[0]; spp_vct[1] = temp_spp_pos[1]; spp_vct[2] = temp_spp_pos[2];
        if (!m_Solver_Method.compare("SRIF", Qt::CaseInsensitive))
            is_filter_good = m_SRIFAlgorithm.SRIFforStatic(prevEpochSatlitData,epochSatlitData,spp_pos,X,P);
        else
            is_filter_good = m_KalmanClass.KalmanforStatic(prevEpochSatlitData,epochSatlitData,spp_pos,X,P);

        if(is_filter_good)
        {
            prevEpochSatlitData = epochSatlitData;
            continue_bad_epoch = 0;
        }
        else
        {
            epochSatlitData.clear();
            memset(spp_pos, 0, 3*sizeof(double));
            memset(spp_vct, 0, 3*sizeof(double));
            X.setZero();
        }

        VectorXd ENU_Vct;
        ENU_Vct = X;
        ENU_Vct[0] = spp_pos[0]; ENU_Vct[1] = spp_pos[1]; ENU_Vct[2] = spp_pos[2];
        saveResult2Class(ENU_Vct, spp_vct, epochTime, epochSatlitData, epoch_num, &P);

        if(isdisplay)
        {
            QString disPlayQTextEdit = "";
            int Valid_SatNumber = epochSatlitData.length();
            // display epoch number
            disPlayQTextEdit = "Epoch number: " + QString::number(epoch_num);
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            //
            disPlayQTextEdit = "GPST: " + QString::number(epochTime.Hours) + ":" + QString::number(epochTime.Minutes)
                    + ":" + QString::number(epochTime.Seconds) + ENDLINE + "Satellite number: " + QString::number(epochSatlitData.length());
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            // display ENU or XYZ
            disPlayQTextEdit = "Valid sat number: " + QString::number(Valid_SatNumber) + ENDLINE
                    + "Estimated: " + '\n' + "X:   " + QString::number(spp_pos[0], 'f', 4)
                                    + '\n' + "Y:   " + QString::number(spp_pos[1], 'f', 4)
                                    + '\n' + "Z:   " + QString::number(spp_pos[2], 'f', 4) + ENDLINE;
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
        }


    }

    multepochSatlitData.clear();



    m_PPPModel_Str = "SSD";
    reverseResult();
    writeResult2File();
    m_isRuned = true;

    if(isdisplay)
    {
        QString disPlayQTextEdit = "done";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
    }

}

//AR 2021.03.11 23z
void QPPPBackSmooth::PPPAR(bool isdisplay)
{
    if(!m_haveObsFile) return ;

    //forward
    QPPPModel *myPPP = new QPPPModel(RPR_filepath, mp_QTextEditforDisplay, m_Solver_Method, m_SatSystem,
                        m_TropDelay, m_CutAngle, m_isKinematic, m_Smooth_Str, m_Product, m_PPPModel_Str);
    myPPP->runPPPAR(isdisplay);
    m_isRuned = myPPP->isRuned();
    if(!m_isRuned) return;

    int forWard_epoch_len = myPPP->m_writeFileClass.allSolverX.length();
    if(forWard_epoch_len == 0) return ;
    QVector< RecivePos > last_allReciverPos = myPPP->m_writeFileClass.allReciverPos;
    QVector< VectorXd > last_allSolverX = myPPP->m_writeFileClass.allSolverX;
    QVector< MatrixXd > last_allSolverQ = myPPP->m_writeFileClass.allSloverQ;
    VectorXd last_fillter_X;
    MatrixXd last_fillter_Q;
    for(int i = forWard_epoch_len - 1; i >=0; i--)
    {
        last_fillter_X = last_allSolverX.at(i);
        if(last_fillter_X[0] != 0)
        {
            last_fillter_X = last_allSolverX.at(i);
            last_fillter_Q = last_allSolverQ.at(i);
            break;
        }
    }
    QVector< QVector < SatlitData > > last_allPPPSatlitData = myPPP->m_writeFileClass.allPPPSatlitData;

    m_ReadSP3Class.ACCELERATE = 0;
    m_ReadClkClass.ACCELERATE = 0;

    double spp_pos[3] = {last_fillter_X[0], last_fillter_X[1], last_fillter_X[2]};
    QVector< QVector < SatlitData > > multepochSatlitData;
    multepochSatlitData = last_allPPPSatlitData;
    int all_epoch_len = multepochSatlitData.length();
    double continue_bad_epoch = 0;
    QVector < SatlitData > prevEpochSatlitData;

    for (int epoch_num = all_epoch_len - 1; epoch_num >= 0;epoch_num--)
    {
        QVector< SatlitData > epochSatlitData;
        epochSatlitData = multepochSatlitData.at(epoch_num);

        GPSPosTime epochTime;
        if(epochSatlitData.length() != 0)
        {
            epochTime = epochSatlitData.at(0).UTCTime;
            for(int i = 0;i < epochSatlitData.length();i++)
                epochSatlitData[i].UTCTime.epochNum = epoch_num;
        }
        else
        {
            epochTime = last_allReciverPos.at(epoch_num).UTCtime;
        }

        double temp_spp_pos[3] = {last_allReciverPos.at(epoch_num).spp_pos[0],
                                  last_allReciverPos.at(epoch_num).spp_pos[1],
                                  last_allReciverPos.at(epoch_num).spp_pos[2]};
        memcpy(spp_pos, temp_spp_pos, 3*sizeof(double));
        last_fillter_X = last_allSolverX.at(epoch_num);

        if(epochSatlitData.length() < m_minSatFlag || temp_spp_pos[0] == 0)
        {
            if(m_isKinematic&&continue_bad_epoch++ > 8)
            {
                prevEpochSatlitData.clear();
                continue_bad_epoch = 0;
            }
            for(int i = 0;i < epochSatlitData.length();i++)
                epochSatlitData[i].EpochFlag = 777;
            VectorXd ENU_Vct;
            double spp_vct[3] = {0};
            ENU_Vct.resize(32+epochSatlitData.length());
            ENU_Vct.fill(0);
            saveResult2Class(ENU_Vct, spp_vct, epochTime, epochSatlitData, epoch_num);
            continue;
        }

        QString sys_cur = ""; int sys_num_cur = 0;
        sys_cur = epochSatlitData.at(0).SatType;
        sys_num_cur = 1;
        //The number of satellite systems in the current epoch
        for(int cf_i = 1; cf_i < epochSatlitData.length(); cf_i++)
        {
            if(!sys_cur.contains(epochSatlitData.at(cf_i).SatType))
                sys_num_cur++, sys_cur = sys_cur + epochSatlitData.at(cf_i).SatType;
        }
        if(prevEpochSatlitData.length() != 0 && m_isKinematic)
        {
            //Select the reference sat according to the satellite elecvation
            m_getbia.select_refsat(epochSatlitData, prevEpochSatlitData, m_CutAngle, sys_cur);
            m_getbia.get_refsat_sit(epochSatlitData, sys_cur);
        }

        //Get the reference sat prn of the previous epoch
        if(epoch_num != all_epoch_len - 1)
        {
            for(int cf = 0; cf < epochSatlitData.length(); cf ++)
            {
                epochSatlitData[cf].SSD_fixed_flag = false;
                epochSatlitData[cf].SSD_fixed = 0.0;
                epochSatlitData[cf].times_nl = 0;

                if(prevEpochSatlitData.length() == 0)
                {
                    for(int cf_j = 0; cf_j < 2*sys_num_cur; cf_j++)
                    {
                        epochSatlitData[cf].prn_referencesat_previous[cf_j] = 0;
                    }

                }
                else
                {
                    for(int cf_j = 0; cf_j < 2*sys_num_cur; cf_j++)
                    {
                        epochSatlitData[cf].prn_referencesat_previous[cf_j]
                                = prevEpochSatlitData[0].prn_referencesat[cf_j];
                    }
                }
            }
        }

        MatrixXd P;
        VectorXd X;
        double spp_vct[3] = {0};
        bool is_filter_good = false;
        X.resize(3 + epochSatlitData.length());
        X.setZero();

        if(epoch_num == all_epoch_len - 1)
        {
            X = last_fillter_X;
            P = last_fillter_Q;
            for(int i = 0;i < 3;i++) X[i] = last_fillter_X[i] - temp_spp_pos[i];
        }
        if(epoch_num == 1873)
            int cf = 23;
        spp_vct[0] = temp_spp_pos[0]; spp_vct[1] = temp_spp_pos[1]; spp_vct[2] = temp_spp_pos[2];
        if (!m_Solver_Method.compare("SRIF", Qt::CaseInsensitive))
            is_filter_good = m_SRIFAlgorithm.SRIFforStatic(prevEpochSatlitData,epochSatlitData,spp_pos,X,P);
        else
            is_filter_good = m_KalmanClass.KalmanforStatic(prevEpochSatlitData,epochSatlitData,spp_pos,X,P);

        //AR
        if(epochSatlitData.at(0).ARornot && prevEpochSatlitData.length() != 0)
        {
            //wl 2020.11.30 23z
            myPPP->calculatewl(prevEpochSatlitData,epochSatlitData);

            //nl 2020.12.01 23z
            myPPP->calculatenl(prevEpochSatlitData,epochSatlitData,X,P);

            if(is_filter_good)
            {
                //filter again
                QVector< SatlitData > epoch_temp;
                epoch_temp = epochSatlitData;

                int times = 0;
                times = prevEpochSatlitData.at(0).times_hold;

                if(times != 0)
                {
                    for(int cf_i = 0; cf_i < epoch_temp.length(); cf_i++)
                        epoch_temp[cf_i].times_hold = times;
                }

                //fix and hold 2020.12.07 23z
                MatrixXd temp_P;
                bool cf_flag = false;
                if(epochSatlitData.at(0).SSD_fixed_flag)
                     cf_flag = m_KalmanClass.KalmanforStatic(epoch_temp,epochSatlitData,spp_pos,X,temp_P);
                if(cf_flag) P = temp_P;
            }

        }

        if(is_filter_good)
        {
            prevEpochSatlitData = epochSatlitData;
            continue_bad_epoch = 0;
        }
        else
        {
            epochSatlitData.clear();
            memset(spp_pos, 0, 3*sizeof(double));
            memset(spp_vct, 0, 3*sizeof(double));
            X.setZero();
        }

        VectorXd ENU_Vct;
        ENU_Vct = X;
        ENU_Vct[0] = spp_pos[0]; ENU_Vct[1] = spp_pos[1]; ENU_Vct[2] = spp_pos[2];
        saveResult2Class(ENU_Vct, spp_vct, epochTime, epochSatlitData, epoch_num, &P);

        if(isdisplay)
        {
            QString disPlayQTextEdit = "";
            int Valid_SatNumber = epochSatlitData.length();
            // display epoch number
            disPlayQTextEdit = "Epoch number: " + QString::number(epoch_num);
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            //
            disPlayQTextEdit = "GPST: " + QString::number(epochTime.Hours) + ":" + QString::number(epochTime.Minutes)
                    + ":" + QString::number(epochTime.Seconds) + ENDLINE + "Satellite number: " + QString::number(epochSatlitData.length());
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            // display ENU or XYZ
            disPlayQTextEdit = "Valid sat number: " + QString::number(Valid_SatNumber) + ENDLINE
                    + "Estimated: " + '\n' + "X:   " + QString::number(spp_pos[0], 'f', 4)
                                    + '\n' + "Y:   " + QString::number(spp_pos[1], 'f', 4)
                                    + '\n' + "Z:   " + QString::number(spp_pos[2], 'f', 4) + ENDLINE;
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
        }
    }

    delete myPPP;
    multepochSatlitData.clear();

    if(global_cf::fix_and_hold) m_PPPModel_Str = "ARF(SSD)";
    else m_PPPModel_Str = "ARI(SSD)";


    reverseResult();
    writeResult2File();
    m_isRuned = true;

    if(isdisplay)
    {
        QString disPlayQTextEdit = "done";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
    }
}

