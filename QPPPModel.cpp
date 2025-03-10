#include "QPPPModel.h"

QStringList QPPPModel::searchFilterFile(QString floder_path, QStringList filers)
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
QPPPModel::QPPPModel(QStringList files_path,  QTextEdit *pQTextEdit, QString Method, QString Satsystem,
                     QString TropDelay, double CutAngle, bool isKinematic, QString Smooth_Str, QString products,
                     QString pppmodel_t, QString deleteSats, QVector<QStringList> ObsTypeSet, QVector<QStringList> Qw_Pk_LPacc)
{
    //Initialize variables
    initVar();
    // Display for GUI
    mp_QTextEditforDisplay = pQTextEdit;
    m_run_floder = files_path.at(0);
    RPR_filepath = files_path;
    m_App_floder = QCoreApplication::applicationDirPath() + PATHSEG;
    // find files
    QStringList tempFilters,
            OFileNamesList, Sp3FileNamesList, ClkFileNamesList, ErpFileNamesList,
            AtxFileNamesList, BlqFileNamesList, GrdFileNamesList, BIAfilenameslist;
    QStringList DCBfilepath, shadfile;
    // !!!2020.11.09 23z
    QString temp_filepath = RPR_filepath.at(1) + PATHSEG + "dcb";
    tempFilters.clear();
    tempFilters.append("*.*DCB");
    DCBfilepath = searchFilterFile(temp_filepath, tempFilters);

    temp_filepath = RPR_filepath.at(1) + PATHSEG + "bia";
    tempFilters.clear();
    tempFilters.append("*.*bia");
    BIAfilenameslist = searchFilterFile(temp_filepath, tempFilters);

    temp_filepath = RPR_filepath.at(1) + PATHSEG + "shad";
    tempFilters.clear();
    tempFilters.append("*.*shadhist");
    shadfile = searchFilterFile(temp_filepath, tempFilters);

    temp_filepath = RPR_filepath.at(1) + PATHSEG + "bia";
    tempFilters.clear();
    tempFilters.append("*.BIA*");
    BIAfilenameslist = searchFilterFile(temp_filepath, tempFilters);

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
    temp_filepath = m_run_floder.mid(0,temp_lp);
    OFileNamesList = searchFilterFile(temp_filepath, tempFilters);

    // find sp3 files
    temp_filepath = RPR_filepath.at(1) + PATHSEG + "sp3";
    tempFilters.clear();
    tempFilters.append("*.sp3");
    Sp3FileNamesList = searchFilterFile(temp_filepath, tempFilters);
    //    if(Sp3FileNamesList.length() == 0)
    //    {//23Z
    // find eph files
    QStringList ephFileNamesList;
    tempFilters.clear();
    tempFilters.append("*.eph*");
    ephFileNamesList = searchFilterFile(temp_filepath, tempFilters);
    for(int cf_i = 0; cf_i < ephFileNamesList.length(); cf_i++)
    {
        Sp3FileNamesList.append(ephFileNamesList.at(cf_i));
    }
    //    }

    // find clk files
    temp_filepath = RPR_filepath.at(1) + PATHSEG + "clk";
    tempFilters.clear();
    tempFilters.append("*.clk*");
    ClkFileNamesList = searchFilterFile(temp_filepath, tempFilters);
    // if not find clk try clk_*
    //    if(ClkFileNamesList.isEmpty())
    //    {
    //        tempFilters.clear();
    //        tempFilters.append("*.clk_*");
    //        ClkFileNamesList = searchFilterFile(temp_filepath, tempFilters);
    //    }

    // find erp files
    temp_filepath = RPR_filepath.at(1) + PATHSEG + "erp";
    tempFilters.clear();
    tempFilters.append("*.erp*");
    ErpFileNamesList = searchFilterFile(temp_filepath, tempFilters);
    // find Atx files
    tempFilters.clear();
    tempFilters.append("*.atx");
    QStringList run_floder, app_floder;
    run_floder = searchFilterFile(m_run_floder, tempFilters);
    app_floder = searchFilterFile(m_App_floder, tempFilters);
    AtxFileNamesList.append(run_floder);
    AtxFileNamesList.append(app_floder);
    run_floder.clear();
    app_floder.clear();
    // find blq files
    tempFilters.clear();
    tempFilters.append("*.blq");
    run_floder = searchFilterFile(m_run_floder, tempFilters);
    app_floder = searchFilterFile(m_App_floder, tempFilters);
    BlqFileNamesList.append(run_floder);
    BlqFileNamesList.append(app_floder);
    run_floder.clear();
    app_floder.clear();
    // find grd files
    tempFilters.clear();
    tempFilters.append("*.grd");
    run_floder = searchFilterFile(m_run_floder, tempFilters);
    app_floder = searchFilterFile(m_App_floder, tempFilters);
    GrdFileNamesList.append(run_floder);
    GrdFileNamesList.append(app_floder);
    run_floder.clear();
    app_floder.clear();
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
    QString OfileName = "", erpFile = "", blqFile = "", atxFile = "", grdFile = "", biafile = "";
    if(temp_oflag)
    {
        OfileName = files_path.at(0);
        m_haveObsFile = true;
    }
    else
    {
        ErroTrace("QPPPModel::QPPPModel: Cant not find Obsvertion file.");
        m_haveObsFile = false;
    }
    //if(!ErpFileNamesList.isEmpty()) erpFile = ErpFileNamesList.at(0);
    if(!AtxFileNamesList.isEmpty()) atxFile = AtxFileNamesList.at(0);
    if(!BlqFileNamesList.isEmpty()) blqFile = BlqFileNamesList.at(0);

    //if(!BIAfilenameslist.isEmpty()) codbiafilepath = BIAfilenameslist.at(0);

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

    //    if(!DCBfilepath.isEmpty())
    //        GPSDCBfilepath = DCBfilepath.at(0);

    //    if(!shadfile.isEmpty())
    //        shadfilepath = shadfile.at(0);


    // get OBS file size for real-time write file
    double Threshold_ofile = global_cf::Threshold_ofile_size;
    QFileInfo obs_info(OfileName);
    double obsMB = (double)(obs_info.size())/(1024.0*1024.0);
    if(obsMB > Threshold_ofile) m_IS_MAX_OBS = true;

    // use defualt config
    setConfigure(Method, Satsystem, TropDelay, CutAngle, isKinematic, Smooth_Str, products, pppmodel_t, deleteSats,
                 ObsTypeSet, Qw_Pk_LPacc);
    // save data to QPPPModel
    initQPPPModel(OfileName, Sp3FileNamesList, ClkFileNamesList, ErpFileNamesList, BIAfilenameslist, DCBfilepath, shadfile, blqFile, atxFile, grdFile);
}
void QPPPModel::setConfigure(QString Method, QString Satsystem, QString TropDelay, double CutAngle,
                             bool isKinematic, QString Smooth_Str, QString products, QString pppmodel_t,
                             QString deleteSats, QVector<QStringList> ObsTypeSet, QVector<QStringList> Qw_Pk_LPacc)
{
    // Configure
    m_Solver_Method = Method;// m_Solver_Method value can be "SRIF" or "Kalman"
    m_CutAngle = CutAngle;// (degree)
    m_SatSystem = Satsystem;// GPS, GLONASS, BDS, and Galieo are used respectively: the letters G, R, C, E
    m_TropDelay = TropDelay;// The tropospheric model m_TropDelay can choose Sass, Hopfiled, UNB3m
    m_Product = products;
    m_PPPModel_Str = pppmodel_t;
    m_isKinematic = isKinematic;
    m_deleteSats = deleteSats;
    m_ObsTypeSet = ObsTypeSet;
    m_Qw_Pk_LPacc = Qw_Pk_LPacc;
    //Setting up the file system  SystemStr:"G"(Turn on the GPS system);"GR":(Turn on the GPS+GLONASS system);"GRCE" (Open all), etc.
    //GPS, GLONASS, BDS, and Galieo are used respectively: the letters G, R, C, E
    setSatlitSys(Satsystem);
    m_sys_str = Satsystem;
    m_sys_num = getSystemnum();
    m_ReadOFileClass.setWangObsType(ObsTypeSet);
    // set filter Model
    if(isKinematic)
    {
        m_KalmanClass.setModel(QKalmanFilter::KALMAN_MODEL::PPP_KINEMATIC);// set Kinematic model
        m_SRIFAlgorithm.setModel(SRIFAlgorithm::SRIF_MODEL::PPP_KINEMATIC);
        m_minSatFlag = 5;// Dynamic Settings 4 or 1???, Static Settings 1
    }
    else
    {
        m_KalmanClass.setModel(QKalmanFilter::KALMAN_MODEL::PPP_STATIC);// set static model
        m_SRIFAlgorithm.setModel(SRIFAlgorithm::SRIF_MODEL::PPP_STATIC);
        m_minSatFlag = 1;// Dynamic Settings 1, Static Settings 1
    }
    m_KalmanClass.setFilterParams(Qw_Pk_LPacc);// set Qw and Pk for Kalman
    m_SRIFAlgorithm.setFilterParams(Qw_Pk_LPacc);// set Qw and Pk for SRIF


    if("Smooth" == Smooth_Str)
    {
        m_KalmanClass.setSmoothRange(QKalmanFilter::KALMAN_SMOOTH_RANGE::SMOOTH);
        m_SRIFAlgorithm.setSmoothRange(SRIFAlgorithm::SRIF_SMOOTH_RANGE::SMOOTH);
        m_isSmoothRange = true;
    }
    else if("NoSmooth" == Smooth_Str)
    {
        m_KalmanClass.setSmoothRange(QKalmanFilter::KALMAN_SMOOTH_RANGE::NO_SMOOTH);
        m_SRIFAlgorithm.setSmoothRange(SRIFAlgorithm::SRIF_SMOOTH_RANGE::NO_SMOOTH);
        m_isSmoothRange = false;
    }
    if(Method == "KalmanOu")
    {
        m_KalmanClass.setFilterMode(QKalmanFilter::KALMAN_FILLTER::KALMAN_MrOu);
    }
    if(m_PPPModel_Str.contains("IF", Qt::CaseInsensitive) || m_PPPModel_Str.contains("SSD", Qt::CaseInsensitive) || m_PPPModel_Str.contains("AR", Qt::CaseInsensitive))
    {
        setPPPModel(PPP_MODEL::PPP_Combination);
        m_KalmanClass.setPPPModel(PPP_MODEL::PPP_Combination);
        m_SRIFAlgorithm.setPPPModel(PPP_MODEL::PPP_Combination);
        m_writeFileClass.setPPPModel(PPP_MODEL::PPP_Combination);
        if(m_IS_MAX_OBS) m_QRTWrite2File.setPPPModel(PPP_MODEL::PPP_Combination);
        m_qualityCtrl.setPPPModel(PPP_MODEL::PPP_Combination);
    }
    else if(m_PPPModel_Str.contains("Uncomb", Qt::CaseInsensitive))
    {
        setPPPModel(PPP_MODEL::PPP_NOCombination);
        m_KalmanClass.setPPPModel(PPP_MODEL::PPP_NOCombination);
        m_SRIFAlgorithm.setPPPModel(PPP_MODEL::PPP_NOCombination);
        m_writeFileClass.setPPPModel(PPP_MODEL::PPP_NOCombination);
        if(m_IS_MAX_OBS) m_QRTWrite2File.setPPPModel(PPP_MODEL::PPP_NOCombination);
        m_qualityCtrl.setPPPModel(PPP_MODEL::PPP_NOCombination);
    }
    else
        m_haveObsFile = false;
}

//Initialization operation
void QPPPModel::initVar()
{
    for (int i = 0;i < 3;i++)
        m_ApproxRecPos[0] = 0;
    m_OFileName = "";
    multReadOFile = 3000;
    m_leapSeconds = 0;
    m_isConnect = false;
    m_isConnectCNT = false;
    m_run_floder = "";
    m_haveObsFile = false;
    m_isRuned = false;
    m_save_images_path = "";
    m_iswritre_file = false;
    m_minSatFlag = 5;// Dynamic Settings 5 or 1, Static Settings 1 in setConfigure()
    m_isSmoothRange = false;
    m_clock_jump_type = 0;
    m_interval = -1;
    mp_QTextEditforDisplay = NULL;
    m_IS_MAX_OBS = false;
    m_deleteSats = "";
}

//Constructor
void QPPPModel::initQPPPModel(QString OFileName, QStringList Sp3FileNames, QStringList ClkFileNames, QStringList ErpFileName, QStringList biafilename, QStringList DCBfilepath, QStringList Shadfilepath, QString BlqFileName, QString AtxFileName, QString GrdFileName)
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
    QString ot_ym = QString::number(obsTime[0],10).mid(2,2) + QString::number(obsTime[1],10).sprintf("%02d",obsTime[1]);
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
        if(temp_file.contains(ot_WGPS.mid(0,4)) || temp_file.contains(ot_UTC))
        {
            s_erp = temp_file;
            break;
        }
    }
    for(int cf_i = 0; cf_i < biafilename.length(); cf_i++)
    {
        temp_file = biafilename.at(cf_i);
        if(temp_file.contains(ot_WGPS) || temp_file.contains(ot_UTC))
        {
            s_bia = temp_file;
            break;
        }
    }
    for(int cf_i = 0; cf_i < DCBfilepath.length(); cf_i++)
    {
        temp_file = DCBfilepath.at(cf_i);
        if(temp_file.contains(ot_WGPS) || temp_file.contains(ot_UTC) || temp_file.contains(ot_ym))
        {
            s_DCB = temp_file;
            break;
        }
    }
    for(int cf_i = 0; cf_i < Shadfilepath.length(); cf_i++)
    {
        temp_file = Shadfilepath.at(cf_i);
        if(temp_file.contains(ot_WGPS) || temp_file.contains(ot_UTC))
        {
            s_shad = temp_file;
            break;
        }
    }


    //Set up multi-system data
    //Initial various classes
    m_ReadSP3Class.setSP3FileNames(s_sp3);
    m_ReadClkClass.setClkFileNames(s_clk);
    m_ReadTropClass.setTropFileNames(GrdFileName,"VMF", m_TropDelay);// Default tropospheric model projection function GMF
    m_ReadAntClass.setAntFileName(AtxFileName);
    m_TideEffectClass.setTideFileName(BlqFileName,s_erp);// for OCEAN and Erp tide
    m_ReadAntClass.m_CmpClass.readRepFile(s_erp);//  for compute sun and moon position
    qCmpGpsT.readRepFile(s_erp);// safe operation

    m_ReadAntClass.setObsJD(m_ReadOFileClass.getAntType(),ObsJD);//Set the antenna effective time
    m_TideEffectClass.setStationName(m_ReadOFileClass.getMakerName());//Setting the tide requires a station name

    codbiafilepath = s_bia;
    GPSDCBfilepath = s_DCB;
    shadfilepath = s_shad;

    //Save file name
    m_OFileName = OFileName;
    m_Sp3FileNames = s_sp3;
    m_ClkFileNames = s_clk;
    m_ErpFileName = s_erp;


    if(s_sp3.isEmpty() || s_clk.isEmpty() || s_erp.isEmpty())
    {
        QString disPlayQTextEdit = "Lack of necessary precision products!";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
    }
    else if(s_sp3.length() !=3 || s_clk.length() !=3)
    {
        QString disPlayQTextEdit = "Lack of some precision products!";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
    }

    //Check analysis center 23z
    QString product_clk;
    product_clk =  s_clk.at(1).mid(RPR_filepath.at(1).length(),10);
    if(product_clk.contains("GRG") || product_clk.contains("grg")) AC_product = "GRG";
    if(product_clk.contains("COD") || product_clk.contains("cod") || product_clk.contains("COM") || product_clk.contains("com")) AC_product = "COD";


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
    //        m_ReadAntClass.m_CmpClass.readRepFile(s_erp);//  for compute sun and moon position
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
    m_interval = m_ReadOFileClass.getInterval();

    //Read the required calculation file module (time consuming)
    if(m_haveObsFile)
    {
        m_ReadAntClass.getAllData();//Read all data from satellites and receivers
        m_TideEffectClass.getAllData();//Read tide data
        m_ReadTropClass.getAllData();//Read grd files for later tropospheric calculations
        m_ReadSP3Class.getAllData();//Read the entire SP3 file
        m_ReadClkClass.getAllData();//Read the clock error file for later calculation
    }

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

    // init QRTWrite2File class
    if(m_IS_MAX_OBS)
    {
        QString outputfilepath = RPR_filepath.at(2) + PATHSEG;
        QDir tempDir(outputfilepath);
        // product name
        QString Product_name = "IGS";
        if(m_Sp3FileNames.length() > 0)
        {
            QString sp3name_path = m_Sp3FileNames.at(0);
            int index_p = sp3name_path.lastIndexOf("/");// get sp3 name
            QString sp3name = sp3name_path.mid(index_p+1);
            Product_name = sp3name.mid(0,3).toUpper();
        }
        QString floder_name = m_markname+m_ot_UTC+ "_" +Product_name+"_" + m_Solver_Method + "_" + m_PPPModel_Str + "_Static_"  + m_sys_str + PATHSEG; //+ "_1e8" + "_1e8_new"
        if(m_isKinematic)
            floder_name = m_markname+m_ot_UTC+ "_" +Product_name+"_" + m_Solver_Method + "_" + m_PPPModel_Str + "_Kinematic_" + m_sys_str +PATHSEG;
        if(global_cf::flag_seismology)//23z
            floder_name = m_markname+m_ot_UTC+ "_" +Product_name+"_" + m_Solver_Method + "_" + m_PPPModel_Str + "_Seismologic_" + m_sys_str +PATHSEG;

        QString RTwriteFloder = tempDir.absoluteFilePath(floder_name);
        QString obsinfo = m_markname+m_ot_UTC + "_";
        m_QRTWrite2File.setSaveFloder(RTwriteFloder, obsinfo);
    }



}

// get matrix B and observer L
void QPPPModel::Obtaining_equation(QVector< SatlitData > &currEpoch, double *ApproxRecPos, MatrixXd &mat_B, VectorXd &Vct_L, MatrixXd &mat_P, bool isSmoothRange)
{//SPP 2020.11.12 23z
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
        // debug by xiaogongwei 2019.04.10 is exist base system satlite clk
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
        if(!is_find_base_sat) new_hang++; // debug by xiaogongwei 2019.04.10 is exist base system satlite clk
        mat_B.resize(new_hang,new_lie);
        mat_P.resize(new_hang,new_hang);
        Vct_L.resize(new_hang);
        mat_B.setZero();
        Vct_L.setZero();
        mat_P.setIdentity();
        // debug by xiaogongwei 2019.04.10 is exist base system satlite clk
        if(!is_find_base_sat)
        {
            for(int i = 0;i < B.rows();i++)
                B(i, 3) = 0;
            mat_B(mat_B.rows() - 1, 3) = 1;// 3 is conntain [dx,dy,dz]
        }
        mat_B.block(0,0,B.rows(),B.cols()) = B;
        mat_P.block(0,0,P.rows(),P.cols()) = P;
        Vct_L.head(L.rows()) = L;
        for(int i = 1; i < sys_len.size();i++)
        {
            if(0 == sys_len[i])
            {
                mat_B(epochLenLB+flag, 3+i) = 1;// 3 is conntain [dx,dy,dz]
                flag++;
            }

        }
    }//if(no_zero > 0)
}

void QPPPModel::SimpleSPP(QVector < SatlitData > &prevEpochSatlitData, QVector < SatlitData > &epochSatlitData, double *spp_pos)
{
    double p_HEN[3] = {0};
    m_ReadOFileClass.getAntHEN(p_HEN);//Get the antenna high
    GPSPosTime epochTime;//Obtaining observation time
    if(epochSatlitData.length() > 0)
        epochTime = epochSatlitData.at(0).UTCTime;//Obtaining observation time
    else
        return ;
    Vector3d tempPos3d, diff_3d;
    tempPos3d[0] = spp_pos[0]; tempPos3d[1] = spp_pos[1]; tempPos3d[2] = spp_pos[2];
    QVector< SatlitData > store_currEpoch;
    int max_iter = 20;
    for(int iterj = 0; iterj < max_iter; iterj++)
    {
        QVector< SatlitData > currEpoch;
        // get every satilite data
        for (int i = 0;i < epochSatlitData.length();i++)
        {
            SatlitData tempSatlitData = epochSatlitData.at(i);//Store calculated corrected satellite data
            if(!isInSystem(tempSatlitData.SatType))
                continue;
            //Test whether the carrier and pseudorange are abnormal and terminate in time.
            if(!(tempSatlitData.L1&&tempSatlitData.L2&&tempSatlitData.C1&&tempSatlitData.C2))
            {
                QString errorline;
                ErrorMsg(errorline);
                tempSatlitData.badMsg.append("Lack of observations"+errorline);
                m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                continue;
            }
            //When seeking GPS
            double m_PrnGpst = qCmpGpsT.YMD2GPSTime(epochTime.Year,epochTime.Month,epochTime.Day,
                                                    epochTime.Hours,epochTime.Minutes,epochTime.Seconds);
            // Read satellite clock error from CLK file
            double stalitClock = 0;//Unit m
            // Note: Time is the signal transmission time(m_PrnGpst - tempSatlitData.C2/M_C)
            getCLKData(tempSatlitData.PRN,tempSatlitData.SatType,m_PrnGpst - tempSatlitData.C2/M_C,&stalitClock);
            tempSatlitData.StaClock = stalitClock;
            //Obtain the coordinates of the epoch satellite from the SP3 data data
            double pXYZ[3] = {0},pdXYZ[3] = {0}, sp3Clk = 0.0;//Unit m
            // Note: Time is the signal transmission time(m_PrnGpst - tempSatlitData.C2/M_C - tempSatlitData.StaClock/M_C)
            getSP3Pos(m_PrnGpst - tempSatlitData.C2/M_C - tempSatlitData.StaClock/M_C,tempSatlitData.PRN,
                      tempSatlitData.SatType,pXYZ,pdXYZ, &sp3Clk);//Obtain the precise ephemeris coordinates of the satellite launch time(Obtain the precise ephemeris coordinates of the satellite launch time  tempSatlitData.StaClock/M_C Otherwise it will cause a convergence gap of 20cm)
            tempSatlitData.X = pXYZ[0];tempSatlitData.Y = pXYZ[1];tempSatlitData.Z = pXYZ[2];
            //            tempSatlitData.StaClock = sp3Clk;// Use igu to do real-time PPP need to use sp3 clock difference replacement, the directory must have the same day clk file, but the clk file data is not used
            //Calculate the satellite's high sitting angle (as the receiver approximates the target)
            double EA[2]={0};
            getSatEA(tempSatlitData.X,tempSatlitData.Y,tempSatlitData.Z,spp_pos,EA);
            tempSatlitData.EA[0] = EA[0];tempSatlitData.EA[1] = EA[1];
            EA[0] = EA[0]*MM_PI/180;EA[1] = EA[1]*MM_PI/180;//Go to the arc to facilitate the calculation below
            tempSatlitData.SatWight = 0.01;// debug xiaogongwei 2018.11.16
            getWight(tempSatlitData);
            //Test the state of the precise ephemeris and the clock difference and whether the carrier and pseudorange are abnormal, and terminate the XYZ or the satellite with the clock difference of 0 in time.
            if (!(tempSatlitData.X&&tempSatlitData.Y&&tempSatlitData.Z&&tempSatlitData.StaClock))
            {
                QString errorline;
                ErrorMsg(errorline);
                tempSatlitData.badMsg.append("Can't calculate the orbit and clock offset"+errorline);
                m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                continue;
            }
            //Quality control (height angle pseudorange difference)
            if (qAbs(tempSatlitData.C1 - tempSatlitData.C2) > 50)
            {
                QString errorline;
                ErrorMsg(errorline);
                tempSatlitData.badMsg.append("C1-C2>50"+errorline);
                m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                continue;
            }
            if(spp_pos[0] !=0 && tempSatlitData.EA[0] < m_CutAngle)
            {
                QString errorline;
                ErrorMsg(errorline);
                tempSatlitData.badMsg.append("elevation angle is " + QString::number(tempSatlitData.EA[0],'f',2)
                        + " less " + QString::number(m_CutAngle,'f',2) +errorline);
                m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                continue;
            }
            //At the time of SPP, the five satellites with poor GEO orbit in front of Beidou are not removed.
            //          if(tempSatlitData.SatType == 'C' && tempSatlitData.PRN <=5 )
            //              continue;
            //Calculate the wavelength (mainly for multiple systems)
            double F1 = tempSatlitData.Frq[0],F2 = tempSatlitData.Frq[1];
            if(F1 == 0 || F2 == 0) continue;//Frequency cannot be 0
            //Computational relativity correction
            double relative = 0;
            if(spp_pos[0] !=0 ) relative = getRelativty(tempSatlitData.SatType, pXYZ,spp_pos,pdXYZ);
            tempSatlitData.Relativty = relative;
            //Calculate the autobiographic correction of the earth
            double earthW = 0;
            earthW = getSagnac(tempSatlitData.X,tempSatlitData.Y,tempSatlitData.Z,spp_pos);
            tempSatlitData.Sagnac = 0;
            if(spp_pos[0] !=0 ) tempSatlitData.Sagnac = earthW;
            //Calculate tropospheric dry delay!!!
            double MJD = qCmpGpsT.computeJD(epochTime.Year,epochTime.Month,epochTime.Day,
                                            epochTime.Hours,epochTime.Minutes,epochTime.Seconds) - 2400000.5;//Simplified Julian Day
            //Calculate and save the annual accumulation date
            double TDay = qCmpGpsT.YearAccDay(epochTime.Year,epochTime.Month,epochTime.Day);
            double p_BLH[3] = {0},mf = 0, TropZPD = 0;;
            qCmpGpsT.XYZ2BLH(spp_pos[0], spp_pos[1], spp_pos[2], p_BLH);
            if(spp_pos[0] !=0 ) getTropDelay(MJD,TDay,EA[0],p_BLH,&mf, NULL, &TropZPD);
            tempSatlitData.SatTrop = TropZPD;
            tempSatlitData.StaTropMap = mf;
            if(spp_pos[0] !=0 ) tempSatlitData.StaTropMap = mf;
            //Calculate antenna high offset correction  Antenna Height
            tempSatlitData.AntHeight = 0;
            if( spp_pos[0] !=0 )
                tempSatlitData.AntHeight = p_HEN[0]*qSin(EA[0]) + p_HEN[1]*qCos(EA[0])*qSin(EA[1]) + p_HEN[2]*qCos(EA[0])*qCos(EA[1]);
            //Receiver L1 L2 offset correction
            double Lamta1 = M_C/F1,Lamta2 = M_C/F2;
            double L1Offset = 0,L2Offset = 0;
            if( spp_pos[0] !=0 ) getRecvOffset(EA,tempSatlitData.SatType,L1Offset,L2Offset, tempSatlitData.wantObserType);
            tempSatlitData.L1Offset = L1Offset/Lamta1;
            tempSatlitData.L2Offset = L2Offset/Lamta2;
            //Satellite antenna phase center correction
            double SatL12Offset[2] = {0};
            if( spp_pos[0] !=0 )
                getSatlitOffset(epochTime.Year,epochTime.Month,epochTime.Day,
                                epochTime.Hours,epochTime.Minutes,epochTime.Seconds - tempSatlitData.C2/M_C,
                                tempSatlitData.PRN,tempSatlitData.SatType,pXYZ,spp_pos, SatL12Offset, tempSatlitData.wantObserType);//pXYZ saves satellite coordinates
            tempSatlitData.SatL1Offset = SatL12Offset[0]/Lamta1;
            tempSatlitData.SatL2Offset = SatL12Offset[1]/Lamta2;
            //Calculate tide correction
            tempSatlitData.TideEffect = 0;
            //Calculate antenna phase winding
            double AntWindup = 0,preAntWindup = 0;
            //Find the previous epoch. Is there a satellite present? The deposit is stored in preAntWindup or preAntWindup=0.
            if( spp_pos[0] !=0 )
            {
                preAntWindup = getPreEpochWindUp(prevEpochSatlitData,tempSatlitData.PRN,tempSatlitData.SatType);//Get the previous epoch of WindUp
                AntWindup = getWindup(epochTime.Year,epochTime.Month,epochTime.Day,
                                      epochTime.Hours,epochTime.Minutes,epochTime.Seconds - tempSatlitData.C2/M_C,
                                      pXYZ,spp_pos,preAntWindup,m_ReadAntClass.m_sunpos);
            }
            tempSatlitData.AntWindup = AntWindup;
            //Computation to eliminate ionospheric pseudorange and carrier combinations (here absorbed receiver carrier deflection and WindUp) add SatL1Offset and SatL1Offset by xiaogongwei 2019.04.12
            double alpha1 = (F1*F1)/(F1*F1 - F2*F2),alpha2 = (F2*F2)/(F1*F1 - F2*F2);
            tempSatlitData.LL1 = Lamta1*(tempSatlitData.L1 + tempSatlitData.L1Offset + tempSatlitData.SatL1Offset - tempSatlitData.AntWindup);
            tempSatlitData.LL2 = Lamta2*(tempSatlitData.L2 + tempSatlitData.L2Offset + tempSatlitData.SatL2Offset - tempSatlitData.AntWindup);
            tempSatlitData.CC1 = tempSatlitData.C1 + Lamta1*tempSatlitData.L1Offset + Lamta1*tempSatlitData.SatL1Offset;
            tempSatlitData.CC2 = tempSatlitData.C2 + Lamta2 *tempSatlitData.L2Offset + Lamta2*tempSatlitData.SatL2Offset;

            tempSatlitData.LL3 = alpha1*tempSatlitData.LL1 - alpha2*tempSatlitData.LL2;//Eliminate ionospheric carrier LL3
            tempSatlitData.PP3 = alpha1*tempSatlitData.CC1 - alpha2*tempSatlitData.CC2;//Eliminate ionospheric carrier PP3
            // save data to currEpoch
            currEpoch.append(tempSatlitData);
        }
        // judge satilite number large 4
        if(currEpoch.length() < 5)
        {
            memset(spp_pos, 0, 3*sizeof(double));// debug by xiaogongwei 2019.09.25
            epochSatlitData = currEpoch;// debug by xiaogongwei 2019.04.10
            return ;
        }
        // get equation
        MatrixXd mat_B, mat_P;
        VectorXd Vct_L, Xk;
        Vector3d XYZ_Pos;
        Obtaining_equation( currEpoch, spp_pos, mat_B, Vct_L, mat_P);// debug xiaogongwei 2018.11.16
        // slover by least square
        Xk = (mat_B.transpose()*mat_P*mat_B).inverse()*mat_B.transpose()*mat_P*Vct_L;
        XYZ_Pos[0] = tempPos3d[0] + Xk[0];
        XYZ_Pos[1] = tempPos3d[1] + Xk[1];
        XYZ_Pos[2] = tempPos3d[2] + Xk[2];
        diff_3d = XYZ_Pos - tempPos3d;
        tempPos3d = XYZ_Pos;// save slover pos
        // update spp_pos
        spp_pos[0] = XYZ_Pos[0]; spp_pos[1] = XYZ_Pos[1]; spp_pos[2] = XYZ_Pos[2];
        // debug by xiaogongwei 2018.11.17
        if(diff_3d.cwiseAbs().maxCoeff() < 1)
        {
            spp_pos[3] = Xk[3];// save base clk
            store_currEpoch = currEpoch;
            break;
        }
        if(diff_3d.cwiseAbs().maxCoeff() > 2e7 || !isnormal(diff_3d[0]) || iterj == max_iter - 1)
        {
            memset(spp_pos, 0, 4*sizeof(double));
            epochSatlitData = currEpoch;// debug by xiaogongwei 2019.09.25
            return ;
        }
    }// for(int iterj = 0; iterj < 20; iterj++)
    // add Pesudorange smoothed by xiaogongwei 2018.11.20
    MatrixXd mat_B, mat_P;
    VectorXd Vct_L, Xk_smooth;
    //Monitor satellite quality and cycle slip
    //    getGoodSatlite(prevEpochSatlitData,store_currEpoch, m_CutAngle);
    if(store_currEpoch.length() < m_minSatFlag)
    {
        memset(spp_pos, 0, 4*sizeof(double));// debug by xiaogongwei 2019.09.25
        epochSatlitData = store_currEpoch;// debug by xiaogongwei 2019.04.10
        return ;
    }

    if(m_isSmoothRange)
    {
        m_QPseudoSmooth.SmoothPesudoRange(prevEpochSatlitData, store_currEpoch);
        Obtaining_equation( store_currEpoch, spp_pos, mat_B, Vct_L, mat_P, true);// debug xiaogongwei 2018.11.16
        // slover by least square
        Xk_smooth = (mat_B.transpose()*mat_P*mat_B).inverse()*mat_B.transpose()*mat_P*Vct_L;
        // update spp_pos
        spp_pos[0] += Xk_smooth[0]; spp_pos[1] += Xk_smooth[1]; spp_pos[2] += Xk_smooth[2];
        // use spp_pos update  mat_B  Vct_L
        Obtaining_equation( store_currEpoch, spp_pos, mat_B, Vct_L, mat_P, true);// debug xiaogongwei 2019.03.28
    }
    else
    {// Safe operation
        for(int i = 0; i < store_currEpoch.length(); i++)
        {
            store_currEpoch[i].PP3_Smooth = store_currEpoch[i].PP3;
            store_currEpoch[i].PP3_Smooth_NUM = 1;
            store_currEpoch[i].PP3_Smooth_Q = 1 / store_currEpoch[i].SatWight;
        }
        Obtaining_equation( store_currEpoch, spp_pos, mat_B, Vct_L, mat_P, false);
    }
    // Qulity control. add by xiaogongwei 2019.05.06
    VectorXd delate_flag;
    //    int max_iter1 = 10;
    // Assuming that SPP has only one gross error, use if is not while, filtering uses while loop to eliminate Debug by xiaogongwei  2019.05.06
    while(m_qualityCtrl.VtPVCtrl_C(mat_B, Vct_L, mat_P, delate_flag, store_currEpoch.length()))
    {
        QVector<int> del_val;
        int sat_len = store_currEpoch.length();
        for(int i = sat_len - 1; i >= 0;i--)
        {
            if(0 != delate_flag[i])
                del_val.append(i);
        }
        //        max_iter1--;
        //        if(max_iter1 <= 0) break;
        // delete gross Errors
        int del_len = del_val.length();
        if(sat_len - del_len >= 5)
        {
            for(int i = 0; i < del_len;i++)
                store_currEpoch.remove(del_val[i]);
            sat_len = store_currEpoch.length();// update epochLenLB

            //update spp_pos
            Obtaining_equation( store_currEpoch, spp_pos, mat_B, Vct_L, mat_P, m_isSmoothRange);
            MatrixXd mat_Q = (mat_B.transpose()*mat_P*mat_B).inverse();
            VectorXd x_solver = mat_Q*mat_B.transpose()*mat_P*Vct_L;
            spp_pos[0] += x_solver[0]; spp_pos[1] += x_solver[1]; spp_pos[2] += x_solver[2];
        }
        else
        {
            memset(spp_pos, 0, 4*sizeof(double));// debug by xiaogongwei 2019.09.25
            break;
        }
    }
    // change epochSatlitData !!!!!!
    epochSatlitData = store_currEpoch;
}

//Read O files, sp3 files, clk files, and various error calculations, Kalman filtering ......................
//isDisplayEveryEpoch represent is disply every epoch information?(ENU or XYZ)
void QPPPModel::Run(bool isDisplayEveryEpoch)
{
    if(!m_haveObsFile) return ;// if not have observation file.
    QTime myTime; // set timer
    myTime.start();// start timer
    //Externally initialize fixed variables to speed up calculations
    double p_HEN[3] = {0};//Get the antenna high
    m_ReadOFileClass.getAntHEN(p_HEN);
    //Traversing data one by one epoch, reading O file data
    QString disPlayQTextEdit = "";// display for QTextEdit
    QVector < SatlitData > prevEpochSatlitData;//Store satellite data of an epoch, use cycle slip detection（Put it on top, otherwise read multReadOFile epochs, the life cycle will expire when reading）
    double spp_pos[4] = {0};// store SPP pos and clk
    memcpy(spp_pos, m_ApproxRecPos, 3*sizeof(double));
    int epoch_num = 0, continue_bad_epoch = 0;//Record the first epoch
    bool isInitSpp = false;
    if(spp_pos[0] !=0 ) isInitSpp = true;

    //2021.01.07 23Z
    m_getbia.readPRN_GPS();
    if(!shadfilepath.isEmpty())
        m_getbia.readshadfile(shadfilepath);

    while (!m_ReadOFileClass.isEnd())
    {
        QVector< QVector < SatlitData > > multepochSatlitData;//Store multiple epochs
        m_ReadOFileClass.getMultEpochData(multepochSatlitData,multReadOFile);//Read multReadOFile epochs

        //2021.04.07 23z There may be no sampling rate information in the header file
        m_interval = getinterval(multepochSatlitData);

        int flag_sampling = 1;
        if(global_cf::flag_sampling)
        {
            double interval_original, interval_used;
            interval_original = global_cf::Interval_original;
            interval_used = global_cf::Interval_used;

            int temp_test = interval_used/interval_original;
            bool flag_temp_test = true;
            if(temp_test*interval_original == interval_used) flag_temp_test = false;
            if(m_interval != interval_original || interval_used < interval_original || flag_temp_test)
            {//23z
                QString info = "There is something wrong with sampling!";
                QMessageBox::warning(mp_QTextEditforDisplay, "Warning", info);
                break;
            }
            else
                flag_sampling = temp_test;
        }

        //Delete short arc satellite   2021.01.16 23z
        //m_getbia.deletetempsats(multepochSatlitData, m_SystemStr, m_interval);

        //tese of CFZ
        //        q_zwd.resize(2880,7);
        //        q_zwd.setZero();

        //Multiple epoch cycles
        for (int epoch = 0; epoch < multepochSatlitData.length();epoch = epoch + flag_sampling)
        {
            //if(epoch < 1000) continue;

            QVector< SatlitData > epochSatlitData;//Temporary storage of uncalculated data for each epoch satellite
            QVector< SatlitData > epochResultSatlitData;// Store each epoch satellite to calculate the correction data
            epochSatlitData = multepochSatlitData.at(epoch);
            if(epochSatlitData.length() == 0) continue;
            GPSPosTime epochTime;
            if(epochSatlitData.length() > 0)
            {
                epochTime= epochSatlitData.at(0).UTCTime;//Obtain the observation time (the epoch stores the observation time for each satellite)
                epochTime.epochNum = epoch_num;
            }

            //2021.01.07 23z
            if(!shadfilepath.isEmpty())
                m_getbia.shadcorrcect(epochSatlitData);

            //Set the epoch of the satellite
            for(int i = 0;i < epochSatlitData.length();i++)
                epochSatlitData[i].UTCTime.epochNum = epoch_num;

            if(epoch_num > 2880)
            {// Debug for epoch
                //2018-12- 8 13: 4: 0.0000000
                int a = 0;
            }
            // use spp compute postion and smooth pesudorange get clk at spp_pos[3]
            // !isInitSpp || m_isKinematic
            SimpleSPP(prevEpochSatlitData, epochSatlitData, spp_pos);

            if(!isInitSpp && spp_pos[0] != 0)
                memcpy(m_ApproxRecPos, spp_pos, 3*sizeof(double));
            if(!m_isKinematic)
                memcpy(spp_pos, m_ApproxRecPos, 3*sizeof(double));
            if(!isnormal(spp_pos[0]))
                memset(spp_pos, 0, 4*sizeof(double));

            // The number of skipping satellites is less than m_minSatFlag
            // update at 2018.10.17 for less m_minSatFlag satellites at the begin observtion
            if(epochSatlitData.length() < m_minSatFlag || spp_pos[0] == 0)
            {
                if(epochSatlitData.length() == 0)
                    continue;
                if(m_isKinematic&&continue_bad_epoch++ > 8)
                {
                    prevEpochSatlitData.clear();// Exception reinitialization
                    continue_bad_epoch = 0;
                }

                for(int i = 0;i < epochSatlitData.length();i++)
                {
                    epochSatlitData[i].EpochFlag = 888;// add bad flag 888 for SPP
                    // set residual as zeros
                    epochSatlitData[i].VC1 = 0; epochSatlitData[i].VC2 = 0;
                    epochSatlitData[i].VL1 = 0; epochSatlitData[i].VL2 = 0;
                    epochSatlitData[i].VLL3 = 0; epochSatlitData[i].VPP3 = 0;
                }

                disPlayQTextEdit = "GPST: " + QString::number(epochTime.Hours) + ":" + QString::number(epochTime.Minutes)
                        + ":" + QString::number(epochTime.Seconds) ;
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                disPlayQTextEdit = "Satellite number: " + QString::number(epochSatlitData.length())
                        + '\n' + "satellites number is less than 5.";
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                // translation to ENU
                VectorXd ENU_Vct;
                double spp_vct[3] = {0};
                int param_len = 3*epochSatlitData.length() + 32;
                ENU_Vct.resize(param_len);
                ENU_Vct.fill(0);
                // debug by xiaogongwei 2020.04.24
                saveResult2Class(ENU_Vct, spp_vct, epochTime, epochSatlitData, epoch_num);
                epoch_num++;
                continue;
            }
            //An epoch cycle begins
            for (int i = 0;i < epochSatlitData.length();i++)
            {
                SatlitData tempSatlitData = epochSatlitData.at(i);//Store calculated corrected satellite data

                //2021.01.16 23z
                tempSatlitData.epochlength = epochSatlitData.length();

                if(!isInSystem(tempSatlitData.SatType))
                    continue;
                //Test whether the carrier and pseudorange are abnormal and terminate in time.
                if(!(tempSatlitData.L1&&tempSatlitData.L2&&tempSatlitData.C1&&tempSatlitData.C2))
                {
                    QString errorline;
                    ErrorMsg(errorline);
                    tempSatlitData.badMsg.append("Lack of observations"+errorline);
                    m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                    continue;
                }
                //When seeking GPS
                double m_PrnGpst = qCmpGpsT.YMD2GPSTime(epochTime.Year,epochTime.Month,epochTime.Day,
                                                        epochTime.Hours,epochTime.Minutes,epochTime.Seconds);
                // Read satellite clock error from CLK file
                double stalitClock = 0.0;
                // Note: Time is the signal transmission time(m_PrnGpst - tempSatlitData.C2/M_C)
                getCLKData(tempSatlitData.PRN,tempSatlitData.SatType,m_PrnGpst - tempSatlitData.C2/M_C,&stalitClock);
                tempSatlitData.StaClock = stalitClock;
                tempSatlitData.StaClockRate = 0;
                //Obtain the coordinates of the epoch satellite from the SP3 data data
                double pXYZ[3] = {0},pdXYZ[3] = {0}, sp3Clk = 0.0;//Unit m
                // Note: Time is the signal transmission time(m_PrnGpst - tempSatlitData.C2/M_C - tempSatlitData.StaClock/M_C)
                getSP3Pos(m_PrnGpst - tempSatlitData.C2/M_C - tempSatlitData.StaClock/M_C,tempSatlitData.PRN,
                          tempSatlitData.SatType,pXYZ,pdXYZ, &sp3Clk);//Obtain the precise ephemeris coordinates of the satellite launch time(Here we need to subtract the satellite clock error. tempSatlitData.StaClock/M_C Otherwise it will cause a convergence gap of 20cm)
                tempSatlitData.X = pXYZ[0];tempSatlitData.Y = pXYZ[1];tempSatlitData.Z = pXYZ[2];
                //                tempSatlitData.StaClock = sp3Clk;// Use igu to do real-time PPP need to use sp3 clock difference replacement, the directory must have the same day clk file, but the clk file data is not used

                //Test the state of the precise ephemeris and the clock difference and whether the carrier and pseudorange are abnormal, and terminate the XYZ or the satellite with the clock difference of 0 in time.
                if (!(tempSatlitData.X&&tempSatlitData.Y&&tempSatlitData.Z&&tempSatlitData.StaClock))
                {
                    QString errorline;
                    ErrorMsg(errorline);
                    tempSatlitData.badMsg.append("Can't calculate the orbit and clock offset"+errorline);
                    m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                    continue;
                }
                //PPP removes  satellites
                QString removeSat = QString(tempSatlitData.SatType);
                if(tempSatlitData.PRN < 10)
                    removeSat += "0" + QString::number(tempSatlitData.PRN);
                else
                    removeSat += QString::number(tempSatlitData.PRN);
                if(m_deleteSats.contains(removeSat, Qt::CaseInsensitive))
                {
                    QString errorline;
                    ErrorMsg(errorline);
                    tempSatlitData.badMsg.append("remove " + removeSat +errorline);
                    m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                    continue;
                }
                //Calculate the wavelength (mainly for multiple systems)
                double F1 = tempSatlitData.Frq[0],F2 = tempSatlitData.Frq[1];
                if(F1 == 0 || F2 == 0) continue;//Frequency cannot be 0

                //Computational relativity correction
                double relative = 0;
                relative = getRelativty(tempSatlitData.SatType, pXYZ,spp_pos,pdXYZ);
                tempSatlitData.Relativty = relative;

                //Calculate the satellite's high sitting angle (as the receiver approximates the target)
                double EA[2]={0};
                getSatEA(tempSatlitData.X,tempSatlitData.Y,tempSatlitData.Z,spp_pos,EA);
                tempSatlitData.EA[0] = EA[0];tempSatlitData.EA[1] = EA[1];
                EA[0] = EA[0]*MM_PI/180;EA[1] = EA[1]*MM_PI/180;//Go to the arc to facilitate the calculation below
                getWight(tempSatlitData);

                //Calculate the autobiographic correction of the earth
                double earthW = 0;
                earthW = getSagnac(tempSatlitData.X,tempSatlitData.Y,tempSatlitData.Z,spp_pos);
                tempSatlitData.Sagnac = earthW;

                //Calculate tropospheric dry delay
                double MJD = qCmpGpsT.computeJD(epochTime.Year,epochTime.Month,epochTime.Day,
                                                epochTime.Hours,epochTime.Minutes,epochTime.Seconds) - 2400000.5;//Simplified Julian Day
                //Calculate and save the annual accumulation date
                double TDay = qCmpGpsT.YearAccDay(epochTime.Year,epochTime.Month,epochTime.Day);
                double p_BLH[3] = {0},mf = 0, TropZHD_s = 0, store_epoch_ZHD;
                qCmpGpsT.XYZ2BLH(spp_pos[0], spp_pos[1], spp_pos[2], p_BLH);
                getTropDelay(MJD,TDay,EA[0],p_BLH,&mf, &TropZHD_s, NULL, &store_epoch_ZHD);
                tempSatlitData.SatTrop = TropZHD_s;
                tempSatlitData.StaTropMap = mf;
                tempSatlitData.UTCTime.TropZHD = store_epoch_ZHD;

                //Calculate antenna high offset correction Antenna Height
                tempSatlitData.AntHeight = p_HEN[0]*qSin(EA[0]) + p_HEN[1]*qCos(EA[0])*qSin(EA[1]) + p_HEN[2]*qCos(EA[0])*qCos(EA[1]);

                //Receiver L1 L2 offset correction
                double Lamta1 = M_C/F1,Lamta2 = M_C/F2;
                double L1Offset = 0,L2Offset = 0;
                getRecvOffset(EA,tempSatlitData.SatType,L1Offset,L2Offset, tempSatlitData.wantObserType);
                tempSatlitData.L1Offset = L1Offset/Lamta1;
                tempSatlitData.L2Offset = L2Offset/Lamta2;

                //Satellite antenna phase center correction store data to
                //(m_ReadAntClass.m_sunpos,m_ReadAntClass.m_moonpos,m_ReadAntClass.m_gmst)
                // and update sunpos and moonpos
                double SatL12Offset[2] = {0};
                getSatlitOffset(epochTime.Year,epochTime.Month,epochTime.Day,
                                epochTime.Hours,epochTime.Minutes,epochTime.Seconds - tempSatlitData.C2/M_C,
                                tempSatlitData.PRN,tempSatlitData.SatType,pXYZ,spp_pos, SatL12Offset, tempSatlitData.wantObserType);
                //pXYZ saves satellite coordinates
                tempSatlitData.SatL1Offset = SatL12Offset[0] / Lamta1;
                tempSatlitData.SatL2Offset = SatL12Offset[1] / Lamta2;

                //Calculate tide correction
                double effctDistance = 0;
                effctDistance = getTideEffect(epochTime.Year,epochTime.Month,epochTime.Day,
                                              epochTime.Hours,epochTime.Minutes,epochTime.Seconds,spp_pos,EA,
                                              m_ReadAntClass.m_sunpos,m_ReadAntClass.m_moonpos,m_ReadAntClass.m_gmst);
                tempSatlitData.TideEffect = effctDistance;

                //Calculate antenna phase winding
                double AntWindup = 0,preAntWindup = 0;
                //Find the previous epoch. Is there a satellite present?
                //The deposit is stored in preAntWindup or preAntWindup=0.
                preAntWindup = getPreEpochWindUp(prevEpochSatlitData,tempSatlitData.PRN,tempSatlitData.SatType);
                //Get the previous epoch of WindUp
                AntWindup = getWindup(epochTime.Year,epochTime.Month,epochTime.Day,
                                      epochTime.Hours,epochTime.Minutes,epochTime.Seconds - tempSatlitData.C2/M_C,
                                      pXYZ,spp_pos,preAntWindup,m_ReadAntClass.m_sunpos);
                tempSatlitData.AntWindup = AntWindup;

                //Computation to eliminate ionospheric pseudorange and carrier combinations
                //(here absorbed receiver carrier deflection and WindUp)
                //add SatL1Offset and SatL1Offset by xiaogongwei 2019.04.12
                double alpha1 = (F1*F1)/(F1*F1 - F2*F2),alpha2 = (F2*F2)/(F1*F1 - F2*F2);

                //以周为单位的模型改正改到观测量上
                tempSatlitData.LL1 = Lamta1*(tempSatlitData.L1 + tempSatlitData.L1Offset + tempSatlitData.SatL1Offset - tempSatlitData.AntWindup);
                tempSatlitData.LL2 = Lamta2*(tempSatlitData.L2 + tempSatlitData.L2Offset + tempSatlitData.SatL2Offset - tempSatlitData.AntWindup);
                tempSatlitData.CC1 = tempSatlitData.C1 + Lamta1*tempSatlitData.L1Offset + Lamta1*tempSatlitData.SatL1Offset;
                tempSatlitData.CC2 = tempSatlitData.C2 + Lamta2*tempSatlitData.L2Offset + Lamta2*tempSatlitData.SatL2Offset;

                //Eliminate ionospheric carrier LL3
                tempSatlitData.LL3 = alpha1*tempSatlitData.LL1 - alpha2*tempSatlitData.LL2;
                //Eliminate ionospheric carrier PP3
                tempSatlitData.PP3 = alpha1*tempSatlitData.CC1 - alpha2*tempSatlitData.CC2;

                //Save an epoch satellite data
                epochResultSatlitData.append(tempSatlitData);
            }//End of an epoch. for (int i = 0;i < epochSatlitData.length();i++)

            //Monitor satellite quality and cycle slip
            getGoodSatlite(prevEpochSatlitData,epochResultSatlitData, m_CutAngle);

            //Satellite number not sufficient
            if(epochResultSatlitData.length() < m_minSatFlag)
            {
                if(epochResultSatlitData.length() == 0)
                    continue;

                if(m_isKinematic&&continue_bad_epoch++ > 8)
                {
                    prevEpochSatlitData.clear();// Exception reinitialization
                    continue_bad_epoch = 0;
                }

                for(int i = 0;i < epochResultSatlitData.length();i++)
                {
                    epochResultSatlitData[i].EpochFlag = 777;// add bad flag 777 for getGoodSatlite
                    // set residual as zeros
                    epochResultSatlitData[i].VC1 = 0; epochResultSatlitData[i].VC2 = 0;
                    epochResultSatlitData[i].VL1 = 0; epochResultSatlitData[i].VL2 = 0;
                    epochResultSatlitData[i].VLL3 = 0; epochResultSatlitData[i].VPP3 = 0;
                }

                // display clock jump
                disPlayQTextEdit = "Valid sat number: " + QString::number(epochResultSatlitData.length()) + ENDLINE +
                        "Waring: *Satellite number is not sufficient";
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                // translation to ENU
                VectorXd ENU_Vct;
                double spp_vct[3] = {0};
                int param_len = 3*epochResultSatlitData.length() + 32;
                ENU_Vct.resize(param_len);
                ENU_Vct.fill(0);
                // debug by xiaogongwei 2020.04.24
                saveResult2Class(ENU_Vct, spp_vct, epochTime, epochResultSatlitData, epoch_num);
                epoch_num++;
                continue;
            }

            for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
            {
                epochResultSatlitData[cf_i].interval = m_interval;
                epochResultSatlitData[cf_i].markname = m_markname;
            }

            if(global_cf::reinitialize != 999999)
            {//Reinitialize 2021.04.22 23z
                int reiniflag = global_cf::reinitialize;
                if(epochResultSatlitData.at(0).UTCTime.epochNum%reiniflag == 0)
                {
                    prevEpochSatlitData.clear();
                    flag_epoch = false;
                }
            }

            // Choose solve method Kalman or SRIF
            MatrixXd P;
            VectorXd X;//Respectively dX, dY, dZ, dT (zenith tropospheric residual), dVt (receiver clock error), N1, N2...Nm (fuzzy)[dx,dy,dz,dTrop,dClock,N1,N2,...Nn]
            double spp_vct[3] = {0};// save spp pos
            bool is_filter_good = false;
            X.resize(5+epochResultSatlitData.length());
            X.setZero();
            // store spp position
            spp_vct[0] = spp_pos[0]; spp_vct[1] = spp_pos[1]; spp_vct[2] = spp_pos[2];
            if (!m_Solver_Method.compare("SRIF", Qt::CaseInsensitive))
                is_filter_good = m_SRIFAlgorithm.SRIFforStatic(prevEpochSatlitData,epochResultSatlitData,spp_pos,X,P);
            else
                is_filter_good = m_KalmanClass.KalmanforStatic(prevEpochSatlitData,epochResultSatlitData,spp_pos,X,P);

            //Save the last epoch satellite data
            if(is_filter_good)
            {
                m_qualityCtrl.CmpSatClkRate(prevEpochSatlitData, epochResultSatlitData);// CmpSatClkRate add by 2020.01.03
                prevEpochSatlitData = epochResultSatlitData;
                continue_bad_epoch = 0;
            }
            else
            {
                continue_bad_epoch++;
                memset(spp_pos, 0, 4*sizeof(double));
                memset(spp_vct, 0, 3*sizeof(double));
                X.setZero();
            }
            if(m_isKinematic && continue_bad_epoch++ > 8)
            {
                prevEpochSatlitData.clear();// Exception reinitialization
                continue_bad_epoch = 0;
            }
            //Output calculation result(print result)
            // display every epoch results
            if(isDisplayEveryEpoch)
            {
                int Valid_SatNumber = epochResultSatlitData.length();
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
            saveResult2Class(ENU_Vct, spp_vct, epochTime, epochResultSatlitData, epoch_num, &P);

            if(epochSatlitData.length() != epochResultSatlitData.length())
                int a = 23;

            epoch_num++;//Increase in epoch

            //            q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,0) = X(3,0);
            //            q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,1) = P(3,3);
            //            q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,2) = sqrt(P(0,0)*P(0,0) + P(1,1)*P(1,1) + P(2,2)*P(2,2));
            //            q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,3) = epoch_num;
            //            q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,4) = spp_pos[0];
            //            q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,5) = spp_pos[1];
            //            q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,6) = spp_pos[2];


        }//End of multiple epochs.  (int n = 0; n < multepochSatlitData.length();n++)



        //        QString filepath = "/home/cfz/data_23z/zcompare/test_of_CFZ/IF_K-GE/" + m_markname + ".csv";
        //        const char *path = filepath.toLatin1().data();
        //        m_matrix.writeCSV(path, q_zwd);

        // clear multepochSatlitData
        multepochSatlitData.clear();
    }//Arrive at the end of the file. while (!m_ReadOFileClass.isEnd())

    prevEpochSatlitData.clear();

    // time end
    //    float m_diffTime = myTime.elapsed() / 1000.0;
    if(isDisplayEveryEpoch)
    {
        disPlayQTextEdit = "done";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
    }
    //Write result to file

    m_PPPModel_Str = "IF";
    if(!m_IS_MAX_OBS) writeResult2File();
    m_isRuned = true;// Determine whether the operation is complete.


}

// The edit box automatically scrolls, adding one row or more lines at a time.
void QPPPModel::autoScrollTextEdit(QTextEdit *textEdit,QString &add_text)
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

bool QPPPModel::connectHost()
{
    if(m_isConnect) return true;
    QString ftp_link = "cddis.gsfc.nasa.gov", user_name = "anonymous", user_password = "";//ftp information
    int Port = 21;
    ftpClient.FtpSetUserInfor(user_name, user_password);
    ftpClient.FtpSetHostPort(ftp_link, Port);
    m_isConnect = true;
    return true;
}

bool QPPPModel::connectCNTHost()
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
QString QPPPModel::downErpFile(QString store_floder_path, int GPS_Week, int GPS_day, QString productType)
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
QStringList QPPPModel::downProducts(QString store_floder_path, int GPS_Week, int GPS_day, QString productType)
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

        if(!ftpClient.FtpGet(net_fileList.at(i), gbm_temp_local_file))
        {
            disPlayQTextEdit = "download: " + net_fileList.at(i) + " bad!";
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            is_down_gbm = false;
            if(!ftpClient.FtpGet(igs_net_fileList.at(i), igs_temp_local_file))
            {
                disPlayQTextEdit = "download: " + igs_net_fileList.at(i) + " bad!";
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            }
            else
            {
                is_down_igs = true;
                disPlayQTextEdit = "download: " + igs_net_fileList.at(i) + " success!";
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            }
        }
        else
        {
            disPlayQTextEdit = "download: " + net_fileList.at(i) + " success!";
            autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
            is_down_gbm = true;
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
    }//for(int i = 0; i < 3;i++)
    return fileList;
}

// dowload CNT products, this product update only one day
QStringList QPPPModel::downCNTProducts(QString store_floder_path, int GPS_Week, int GPS_day, QString productType)
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
QPPPModel::~QPPPModel()
{

}

//Setting up the file system. SystemStr:"G"(Turn on the GPS system);"GR":(Turn on the GPS+GLONASS system);"GRCE"(all open), etc.
//GPS, GLONASS, BDS, and Galieo are used respectively: the letters G, R, C, E
bool QPPPModel::setSatlitSys(QString SystemStr)
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
void QPPPModel::getSP3Pos(double GPST,int PRN,char SatType,double *p_XYZ,double *pdXYZ, double *pSp3Clk)
{
    m_ReadSP3Class.getPrcisePoint(PRN,SatType,GPST,p_XYZ,pdXYZ, pSp3Clk);
}

//Get clock error from CLK data
void QPPPModel::getCLKData(int PRN,char SatType,double GPST,double *pCLKT)
{
    m_ReadClkClass.getStaliteClk(PRN,SatType,GPST,pCLKT);
}

//Earth autobiography correction
double QPPPModel::getSagnac(double X,double Y,double Z,double *approxRecvXYZ)
{//Calculate the autobiographic correction of the earth
    double dltaP = M_We*((X - approxRecvXYZ[0])*Y - (Y - approxRecvXYZ[1])*X)/M_C;
    return -dltaP;//Returns the opposite number such that p = p' + dltaP; can be added directly
}

void QPPPModel::getWight(SatlitData &tempSatlitData)
{//2021.04.25 23z
    double E = 0, SatWight = 0;
    E = tempSatlitData.EA[0]*MM_PI/180;
    //1:MG(GAMIT)   2:CLS   3:Bernese   4:pride、PANDA和EPOS
    int flag_wight = 1;
    if(global_cf::Elevation_weight_function.contains("GAMIT")) flag_wight = 1;
    else if(global_cf::Elevation_weight_function.contains("CNES")) flag_wight = 2;
    else if(global_cf::Elevation_weight_function.contains("Bernese")) flag_wight = 3;
    else flag_wight = 4;

    //b_2 = M_Zgama_P_square, a and b are the accuracy of the observation type, here are given according to the pseudorange accuracy of 3m, and the carrier will be corrected later
    int a_2 = 0;
    double cf = qSin(E);
    if(flag_wight == 1)
    {//sigma_2 = a_2 + b_2/sinE_2
        SatWight = 1.0/(a_2 + M_Zgama_P_square/(qSin(E)*qSin(E)));
    }
    else if(flag_wight == 2)
    {//sigma_2 = b_2/(0.15 + 0.85sinE)_2
        SatWight = (0.15 + 0.85*qSin(E))*(0.15 + 0.85*qSin(E))/M_Zgama_P_square;
    }
    else if(flag_wight == 3)
    {//sigma_2 = a_2 + b_2*cosE_2
        SatWight = 1.0/(a_2 + qCos(E)*qCos(E)*M_Zgama_P_square);
    }
    else
    {//E>=30: a_2; E<30:a_2/(2sinE)_2
        a_2 = M_Zgama_P_square;
        if(E >= 30) SatWight = 1.0/a_2;
        else SatWight = 4*qSin(E)*qSin(E)/a_2;
    }

    double temp_G,temp_R,temp_C,temp_E;
    temp_G = global_cf::weight_G;temp_R = global_cf::weight_R;temp_C = global_cf::weight_C;temp_E = global_cf::weight_E;
    if(temp_G*temp_R*temp_C*temp_E == 0)
    {
        temp_G = 1;temp_R = 0.5;temp_C = 0.5;temp_E = 1;
    }

    switch (tempSatlitData.SatType) {
    case 'G': SatWight = temp_G*SatWight; break;
    case 'R': SatWight = temp_R*SatWight; break;
    case 'C': SatWight = temp_C*SatWight; break;
    case 'E': SatWight = temp_E*SatWight; break;
    default:
        break;
    }

    //Five satellites with poor GEO orbit in front of Beidou
    if(tempSatlitData.SatType == 'C' && tempSatlitData.PRN <= 5)
        SatWight = 0.01*SatWight;
    tempSatlitData.SatWight = SatWight;//Set the weight of the satellite  debug by xiaogongwei 2019.04.24
}

//Computational relativistic effect
double QPPPModel::getRelativty(char SatType, double *pSatXYZ,double *pRecXYZ,double *pSatdXYZ)
{
    /*double c = 299792458.0;
    double dltaP = -2*(pSatXYZ[0]*pSatdXYZ[0] + pSatdXYZ[1]*pdXYZ[1] + pSatXYZ[2]*pSatdXYZ[2]) / c;*/
    double b[3] = {0},a = 0,R = 0,Rs = 0,Rr = 0,v_light = 299792458.0,dltaP = 0;
    b[0] = pRecXYZ[0] - pSatXYZ[0];
    b[1] = pRecXYZ[1] - pSatXYZ[1];
    b[2] = pRecXYZ[2] - pSatXYZ[2];
    a = pSatXYZ[0]*pSatdXYZ[0] + pSatXYZ[1]*pSatdXYZ[1] + pSatXYZ[2]*pSatdXYZ[2];
    R=qCmpGpsT.norm(b,3);
    Rs = qCmpGpsT.norm(pSatXYZ,3);
    Rr = qCmpGpsT.norm(pRecXYZ,3);

    double oldM_GM = 3.986005e14, old_We = 7.2921151467E-5;// GPS
    switch(SatType)
    {
    case 'G':   oldM_GM = 3.986005e14;  old_We = 7.2921151467E-5;  break;
    case 'R':   oldM_GM = 3.9860044E14;  old_We = 7.292115E-5;  break;
    case 'E':   oldM_GM = 3.986004418E14; old_We = 7.2921151467E-5;   break;
    case 'C':   oldM_GM = 3.986004418E14;  old_We = 7.292115E-5;  break;
    }
    double gravity_delay = 0.0;
    gravity_delay =  -(2*oldM_GM/(v_light*v_light))*qLn((Rs+Rr+R)/(Rs+Rr-R));
    dltaP=-2*a/M_C + gravity_delay;
    return dltaP;//m
}



//Calculate EA, E: satellite elevation angle, A: azimuth
void QPPPModel::getSatEA(double X,double Y,double Z,double *approxRecvXYZ,double *EA)
{//Calculate EA
    //BUG occurs, because when calculating XYZ to BLH, L (earth longitude) is actually opposite when y < 0, x > 0.
    //L = -atan(y/x) error should be L = -atan(-y/x)
    double pSAZ[3] = {0};
    qCmpGpsT.XYZ2SAZ(X,Y,Z,pSAZ,approxRecvXYZ);//BUG occurs
    EA[0] = (MM_PI/2 - pSAZ[2])*360/(2*MM_PI);
    EA[1] = pSAZ[1]*360/(2*MM_PI);
}


//Using the Sass model There are other models and projection functions that can be used, as well as the GPT2 model.
// ZHD_s: GNSS signal direction dry delay, ZHD: Station zenith direction dry delay, ZPD: GNSS signal direction total delay
void QPPPModel::getTropDelay(double MJD,int TDay,double E,double *pBLH,double *mf, double *ZHD_s, double *ZPD, double *ZHD)
{
    //double GPT2_Trop = m_ReadTropClass.getGPT2SasstaMDelay(MJD,TDay,E,pBLH,mf);//The GPT2 model only returns the dry delay estimate and the wet delay function.
    double tropDelayH = 0, tropDelayP = 0, tropDelayZHD = 0;
    if(m_TropDelay.mid(0,1).compare("U") == 0)
    {
        tropDelayH = m_ReadTropClass.getUNB3mDelay(pBLH,TDay,E,mf, &tropDelayP, &tropDelayZHD);//Total delay of the UNB3M model
    }
    else if(m_TropDelay.contains("GPT3"))
    {
        tropDelayH =  m_ReadTropClass.GPT3saszd(MJD,TDay,E,pBLH,mf, &tropDelayP, &tropDelayZHD);
    }
    else if(m_TropDelay.mid(0,1).compare("S") == 0)
    {
        tropDelayH =  m_ReadTropClass.getGPT2SasstaMDelay(MJD,TDay,E,pBLH,mf, &tropDelayP, &tropDelayZHD);//GPT2 model total delay
    }
    else
    {
        tropDelayH = m_ReadTropClass.getGPT2HopfieldDelay(MJD,TDay,E,pBLH,mf, &tropDelayP, &tropDelayZHD);//GPT2 model total delay
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

    if(qAbs(tropDelayZHD) > 100  || qAbs(pBLH[2]) > 50000)
        tropDelayZHD = 1e-6;
    else if(!isnormal(tropDelayZHD))
        tropDelayZHD = 1e-6;
    if(ZHD) *ZHD = tropDelayZHD;
    if(ZHD_s)  *ZHD_s = tropDelayH;
    if(ZPD)  *ZPD = tropDelayP;
    return ;
}

//Calculate the receivers L1 and L2 phase center correction PCO + PCV, L1Offset and L2Offset
//represent the distance correction of the line of sight direction
bool QPPPModel::getRecvOffset(double *EA,char SatType,double &L1Offset,double &L2Offset, QVector<QString> FrqFlag)
{
    if (m_ReadAntClass.getRecvL12(EA[0],EA[1],SatType,L1Offset,L2Offset, FrqFlag))
        return true;
    else
    {
        L1Offset = 0; L2Offset = 0;
        return false;
    }
}

//Calculate the satellite PCO+PCV correction, because the satellite G1 and G2 frequencies are the same, so the two bands change exactly the same;
//StaPos and RecPos, satellite and receiver WGS84 coordinates (unit m)
//L12Offset
bool QPPPModel::getSatlitOffset(int Year,int Month,int Day,int Hours,int Minutes,double Seconds,int PRN,char SatType,double *StaPos,double *RecPos,
                                double *L12Offset, QVector<QString> FrqFlag)
{
    bool isGood = m_ReadAntClass.getSatOffSet(Year,Month,Day,Hours,Minutes,Seconds,PRN,SatType,StaPos,RecPos, L12Offset, FrqFlag);
    //pXYZ saves satellite coordinates
    if(isGood)
        return true;
    {
        L12Offset[0] = 0; L12Offset[1] = 0;
        return false;
    }

}

//Calculate the correction of the tide in the direction of the line of sight (unit m)
double QPPPModel::getTideEffect(int Year,int Month,int Day,int Hours,int Minutes,double Seconds,
                                double *pXYZ,double *EA,double *psunpos/* =NULL */, double *pmoonpos /* = NULL */,double gmst /* = 0 */,QString StationName /* = "" */)
{
    return m_TideEffectClass.getAllTideEffect(Year,Month,Day,Hours,Minutes,Seconds,pXYZ,EA,psunpos,pmoonpos,gmst,StationName);
}

//SatPos and RecPos represent the satellite and receiver WGS84 coordinates. Return to the weekly (unit week) range [-0.5 +0.5]
double QPPPModel::getWindup(int Year,int Month,int Day,int Hours,int Minutes,double Seconds,double *StaPos,double *RecPos,double &phw,double *psunpos)
{
    return m_WinUpClass.getWindUp(Year,Month,Day,Hours,Minutes,Seconds,StaPos,RecPos,phw,psunpos);
}

//Detect cycle hops: return pLP is a three-dimensional array, the first is the W-M combination (N2-N1 < 3.5) The second number ionospheric residual (<0.3) The third is (lamt2*N2-lamt1*N1 < 3.5)
bool QPPPModel::CycleSlip(const SatlitData &oneSatlitData,double *pLP)
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
double QPPPModel::getPreEpochWindUp(QVector< SatlitData > &prevEpochSatlitData,int PRN,char SatType)
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
void QPPPModel::reciveClkRapaire(QVector< SatlitData > &prevEpochSatlitData,QVector< SatlitData > &epochSatlitData)
{
    int preEpochLen = prevEpochSatlitData.length();
    int epochLen = epochSatlitData.length();
    if(preEpochLen == 0 || epochLen == 0) return ;
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
    double jump_pro = 0.0;
    if(check_sat_num != 0) jump_pro = (double)clock_num / check_sat_num;
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
void QPPPModel::getGoodSatlite(QVector< SatlitData > &prevEpochSatlitData,QVector< SatlitData > &epochSatlitData,double eleAngle)
{
    int preEpochLen = prevEpochSatlitData.length();
    int epochLen = epochSatlitData.length();
    if(epochLen == 0) return ;
    reciveClkRapaire(prevEpochSatlitData, epochSatlitData);// Rapaire recive Clk

    //Cycle slip detection
    QVector< int > CycleFlag;//Record the position of the weekly jump
    CycleFlag.resize(epochLen);
    for (int i = 0;i < epochLen;i++) CycleFlag[i] = 0;
    for (int i = 0;i < epochLen;i++)
    {
        SatlitData epochData = epochSatlitData.at(i);
        //Data is not 0
        if (!(epochData.L1&&epochData.L2&&epochData.C1&&epochData.C2)) // debug xiaogongwei 2018.11.16
        {
            QString errorline;
            ErrorMsg(errorline);
            epochData.badMsg.append("Lack of observations"+errorline);
            m_writeFileClass.allBadSatlitData.append(epochData);
            CycleFlag[i] = -1;
        }
        //The corrections are not zero
        if (!(epochData.X&&epochData.Y&&epochData.Z&&epochData.StaClock)) // debug xiaogongwei 2018.11.16
        {
            QString errorline;
            ErrorMsg(errorline);
            epochData.badMsg.append("Can't calculate the orbit and clock offset"+errorline);
            m_writeFileClass.allBadSatlitData.append(epochData);
            CycleFlag[i] = -1;
        }
        //Quality control (height angle, pseudorange difference)
        if (epochData.EA[0] < eleAngle)
        {
            QString errorline;
            ErrorMsg(errorline);
            epochData.badMsg.append("elevation angle is " + QString::number(epochData.EA[0],'f',2)
                    + " less " + QString::number(eleAngle,'f',2) +errorline);
            m_writeFileClass.allBadSatlitData.append(epochData);
            CycleFlag[i] = -1;
        }
        if(qAbs(epochData.C1 - epochData.C2) > 50)
        {
            QString errorline;
            ErrorMsg(errorline);
            epochData.badMsg.append("C1-C2>50"+errorline);
            m_writeFileClass.allBadSatlitData.append(epochData);
            CycleFlag[i] = -1;
        }
        // signal intensity
        if(epochData.SigInten >0 && epochData.SigInten < 0)
        {
            QString errorline;
            ErrorMsg(errorline);
            epochData.badMsg.append("Signal intensity is less"+errorline);
            m_writeFileClass.allBadSatlitData.append(epochData);
            CycleFlag[i] = -1;
        }
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
                    QString errorline;
                    ErrorMsg(errorline);
                    epochData.badMsg.append("Cycle slip detection, Ionospheric residual: "+ QString::number(diffLP[1],'f',4)+errorline);
                    m_writeFileClass.allBadSatlitData.append(epochData);
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


void QPPPModel::saveResult2Class(VectorXd X, double *spp_vct, GPSPosTime epochTime, QVector< SatlitData > epochResultSatlitData,
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
    if(!m_IS_MAX_OBS) m_writeFileClass.allReciverPos.append(epochRecivePos);
    //Save wet delay and receiver clock error
    double epoch_ZHD = 0.0;
    int const_num = 4 + m_sys_num;
    if(epochResultSatlitData.length() >= m_minSatFlag) epoch_ZHD = epochResultSatlitData.at(0).UTCTime.TropZHD;
    ClockData epochRecClock;
    epochRecClock.UTCTime= epochRecivePos.UTCtime;
    if(X(3) == 0)
        epochRecClock.ZTD_W = 0;
    else
        epochRecClock.ZTD_W = X(3) + epoch_ZHD;//Storage zenith wet delay + zenith dry delay
    // save clock
    memset(epochRecClock.clockData, 0, 6*sizeof(double));
    //Stores the receiver skew of the first system, and the relative offset of its other systems  GCRE
    if(epochResultSatlitData.at(0).SSDPPP)
        epochRecClock.clockData[0] = epochRecClock.clockData[1] =
                epochRecClock.clockData[2] = epochRecClock.clockData[3] = 0;
    else
    {
        for(int i = 0;i < m_sys_str.length();i++)
        {
            switch (m_sys_str.at(i).toLatin1()) {
            case 'G':
                epochRecClock.clockData[0] = X(4+i);
                break;
            case 'C':
                epochRecClock.clockData[1] = X(4+i);
                break;
            case 'R':
                epochRecClock.clockData[2] = X(4+i);
                break;
            case 'E':
                epochRecClock.clockData[3] = X(4+i);
                break;
            default:
                break;
            }
        }
    }
    if(!m_IS_MAX_OBS) m_writeFileClass.allClock.append(epochRecClock);
    if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
    {
        int sat_num = epochResultSatlitData.length();
        //Save satellite ambiguity
        Ambiguity oneSatAmb;
        for (int i = 0;i < sat_num;i++)
        {
            SatlitData oneSat = epochResultSatlitData.at(i);
            oneSatAmb.PRN = oneSat.PRN;
            oneSatAmb.SatType = oneSat.SatType;
            oneSatAmb.UTCTime = epochRecClock.UTCTime;
            oneSatAmb.isIntAmb = false;
            memcpy(oneSatAmb.EA, oneSat.EA, 2*sizeof(double));
            oneSatAmb.ionL1 = X(i+const_num);
            oneSatAmb.Amb1 = X(i+const_num+sat_num);
            oneSatAmb.Amb2 = X(i+const_num+2*sat_num);
            oneSatAmb.Amb = 0.0;
            epochResultSatlitData[i].ionL1 = X(i+const_num);
            epochResultSatlitData[i].Amb1 = X(i+const_num+sat_num);
            epochResultSatlitData[i].Amb2 = X(i+const_num+2*sat_num);
            epochResultSatlitData[i].Amb = 0.0;
            oneSatAmb.UTCTime.epochNum = epochNum;
            if(!m_IS_MAX_OBS) m_writeFileClass.allAmbiguity.append(oneSatAmb);
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

            if(!m_IS_MAX_OBS) m_writeFileClass.allAmbiguity.append(oneSatAmb);
        }
    }
    // save used satlite Information
    if(!m_IS_MAX_OBS) m_writeFileClass.allPPPSatlitData.append(epochResultSatlitData);
    // save solver X
    if(!m_IS_MAX_OBS) m_writeFileClass.allSolverX.append(X);
    // save P matrix
    if(!m_IS_MAX_OBS)
    {
        if(P)
            m_writeFileClass.allSloverQ.append(*P);
        else
            m_writeFileClass.allSloverQ.append(MatrixXd::Identity(32,32) * 1e10);
    }

    // Real time write to file add by xiaogongwei 2019.09.24
    if(m_IS_MAX_OBS)
    {
        if(P)
            m_QRTWrite2File.writeRecivePos2Txt(epochRecivePos, P);
        else
        {
            MatrixXd tempP = MatrixXd::Identity(32,32) * 1e10;
            m_QRTWrite2File.writeRecivePos2Txt(epochRecivePos, &tempP);
        }
        m_QRTWrite2File.writePPP2Txt(epochResultSatlitData);
        m_QRTWrite2File.writeClockZTDW2Txt(epochRecClock);

        QVector<SatlitData>::iterator iter;
        for (iter=m_writeFileClass.allBadSatlitData.end()-1;iter>=m_writeFileClass.allBadSatlitData.begin();iter--)
        {
            SatlitData tempBadSat = *iter;
            if(epochNum != tempBadSat.UTCTime.epochNum)
                break;
            m_QRTWrite2File.writeBadSatliteData(tempBadSat);
            if(iter == m_writeFileClass.allBadSatlitData.begin())
                break;
        }
        m_writeFileClass.allBadSatlitData.clear();
    }// end of if(m_IS_MAX_OBS)

}


void QPPPModel::writeResult2File()
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
    QString floder_name = m_markname+m_ot_UTC+ "_" +Product_name+ "_" +m_Solver_Method+ "_" +m_PPPModel_Str + "_Static_" + m_sys_str + PATHSEG;
    if(m_isKinematic)
        floder_name = m_markname+m_ot_UTC+ "_" +Product_name+ "_" +m_Solver_Method+ "_" +m_PPPModel_Str + "_Kinematic_" + m_sys_str + PATHSEG;
    if(global_cf::flag_seismology)
        floder_name = m_markname+m_ot_UTC+ "_" +Product_name+ "_" +m_Solver_Method+ "_" +m_PPPModel_Str + "_Seismologic_" + m_sys_str +PATHSEG;

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
    //    m_writeFileClass.writeRecivePos2Txt(product_path, "position.txt");
    //    m_writeFileClass.writePPP2Txt(product_path, "Satellite_info.ppp");
    //    m_writeFileClass.writeClockZTDW2Txt(product_path, "ZTD_Clock.txt");
    //    //m_writeFileClass.writeAmbiguity2Txt(ambiguit_floder);//The path is .//Ambiguity//
    //    //m_writeFileClass.writeRecivePosKML(product_path, "position.kml");// gernerate KML
    //    m_writeFileClass.writeBadSatliteData(product_path, "bad_satellites.txt");// Writing Eliminated Satellites into File
}

// Get operation results( clear QWrite2File::allPPPSatlitData Because the amount of data is too large.)
void QPPPModel::getRunResult(PlotGUIData &plotData)
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


//SSDPPP 2020.11.04 23z  refer to RUN
void QPPPModel::runSSDPPP(bool isDisplayEveryEpoch)
{
    if(!m_haveObsFile) return ;
    //Initialization parameters
    //Antenna height
    double p_HEN[3] = {0};
    m_ReadOFileClass.getAntHEN(p_HEN);
    //Store the satellite data of the previous epoch
    QVector < SatlitData > prevEpochSatlitData;
    //calculate receiver coordinates and clock offset
    double spp_pos[4] = {0};// store SPP pos and clk
    memcpy(spp_pos, m_ApproxRecPos, 3*sizeof(double));
    int epoch_num = 0, continue_bad_epoch = 0;
    bool isInitSpp = false;
    if(spp_pos[0] !=0 ) isInitSpp = true;
    QString disPlayQTextEdit = "";

    //2021.01.07 23Z
    m_getbia.readPRN_GPS();
    if(!shadfilepath.isEmpty())
        m_getbia.readshadfile(shadfilepath);

    while (!m_ReadOFileClass.isEnd())
    {
        //Obtain multi-epoch observation data
        QVector< QVector < SatlitData > > multepochSatlitData;
        m_ReadOFileClass.getMultEpochData(multepochSatlitData,multReadOFile);

        //2021.04.07 23z There may be no sampling rate information in the header file
        m_interval = getinterval(multepochSatlitData);

        int flag_sampling = 1;
        if(global_cf::flag_sampling)
        {
            double interval_original, interval_used;
            interval_original = global_cf::Interval_original;
            interval_used = global_cf::Interval_used;

            int temp_test = interval_used/interval_original;
            bool flag_temp_test = true;
            if(temp_test*interval_original == interval_used) flag_temp_test = false;
            if(m_interval != interval_original || interval_used < interval_original || flag_temp_test)
            {//23z
                QString info = "There is something wrong with sampling!";
                QMessageBox::warning(mp_QTextEditforDisplay, "Warning", info);
                break;
            }
            else
                flag_sampling = temp_test;
        }

        //Delete short arc satellite   2021.01.16 23z
        m_getbia.deletetempsats(multepochSatlitData, m_sys_str, m_interval);

        //Multi-epoch data loop processing

        //tese of CFZ
        //        q_zwd.resize(2880,7);
        //        q_zwd.setZero();

        for (int epoch = 0; epoch < multepochSatlitData.length();epoch = epoch + flag_sampling)
        {
            //Store the original observations of the current epoch
            QVector< SatlitData > epochSatlitData;
            //Store the model-corrected observations of the current epoch
            QVector< SatlitData > epochResultSatlitData;
            epochSatlitData = multepochSatlitData.at(epoch);

            //If there is no data, go to the next epoch
            if(epochSatlitData.length() == 0) continue;

            GPSPosTime epochTime;

            if(epochSatlitData.length() > 0)
            {
                //Get the time information of the current epoch
                epochTime= epochSatlitData.at(0).UTCTime;
                epochTime.epochNum = epoch_num;
            }

            //2021.01.07 23z
            if(!shadfilepath.isEmpty())
                m_getbia.shadcorrcect(epochSatlitData);

            //Set the current epoch number
            for(int i = 0;i < epochSatlitData.length();i++)
                epochSatlitData[i].UTCTime.epochNum = epoch_num;
            //Use SPP to calculate the initial value of the coordinate position and the receiver clock error
            SimpleSPP(prevEpochSatlitData, epochSatlitData, spp_pos);

            if(!isInitSpp && spp_pos[0] != 0)
                memcpy(m_ApproxRecPos, spp_pos, 3*sizeof(double));
            if(!m_isKinematic)
                memcpy(spp_pos, m_ApproxRecPos, 3*sizeof(double));
            if(!isnormal(spp_pos[0]))
                memset(spp_pos, 0, 4*sizeof(double));

            //When the observation is insufficient
            if(epochSatlitData.length() < m_minSatFlag || spp_pos[0] == 0)
            {
                if(epochSatlitData.length() == 0)
                    continue;
                if(m_isKinematic && continue_bad_epoch++ > 8)
                {
                    prevEpochSatlitData.clear();
                    continue_bad_epoch = 0;
                }

                for(int i = 0;i < epochSatlitData.length();i++)
                {
                    //Set error flag 888
                    epochSatlitData[i].EpochFlag = 888;
                    //residual = 0
                    epochSatlitData[i].VC1 = 0; epochSatlitData[i].VC2 = 0;
                    epochSatlitData[i].VL1 = 0; epochSatlitData[i].VL2 = 0;
                    epochSatlitData[i].VLL3 = 0; epochSatlitData[i].VPP3 = 0;
                }

                VectorXd ENU_Vct;
                double spp_vct[3] = {0};
                int param_len = 3*epochSatlitData.length() + 32;
                ENU_Vct.resize(param_len);
                ENU_Vct.fill(0);
                //Store the result and go to the next epoch
                saveResult2Class(ENU_Vct, spp_vct, epochTime, epochSatlitData, epoch_num);
                epoch_num++;
                continue;
            }

            for (int i = 0;i < epochSatlitData.length();i++)
            {
                //Satellite-by-satellite processing
                SatlitData tempSatlitData = epochSatlitData.at(i);

                //2021.01.16 23z
                tempSatlitData.epochlength = epochSatlitData.length();

                //Set SSD PPP processing flag 2020.11.29 23z
                tempSatlitData.SSDPPP = true;

                if(!isInSystem(tempSatlitData.SatType))
                    continue;
                //Check whether the current satellite observations are complete
                if(!(tempSatlitData.L1&&tempSatlitData.L2&&tempSatlitData.C1&&tempSatlitData.C2))
                {
                    QString errorline;
                    ErrorMsg(errorline);
                    tempSatlitData.badMsg.append("Lack of observations"+errorline);
                    m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                    continue;
                }
                //obtain GPST
                double m_PrnGpst = qCmpGpsT.YMD2GPSTime(epochTime.Year,epochTime.Month,epochTime.Day,
                                                        epochTime.Hours,epochTime.Minutes,epochTime.Seconds);
                double stalitClock = 0.0;
                //Pay attention to the effect of signal propagation time(m_PrnGpst - tempSatlitData.C2/M_C)
                getCLKData(tempSatlitData.PRN,tempSatlitData.SatType,m_PrnGpst - tempSatlitData.C2/M_C,&stalitClock);
                tempSatlitData.StaClock = stalitClock;
                tempSatlitData.StaClockRate = 0;
                //Obtain the satellite coordinates from the SP3 file (unit: meter) Coordinate\Speed\Clock offset
                double pXYZ[3] = {0},pdXYZ[3] = {0}, sp3Clk = 0.0;
                //Pay attention to the effect of signal propagation time(m_PrnGpst - tempSatlitData.C2/M_C - tempSatlitData.StaClock/M_C)
                //Obtain the precise satellite coordinates at the time when the satellite signal is transmitted. Note: You need to subtract the satellite clock error tempSatlitData.StaClock/M_C, otherwise it will cause an error of 0.2 meters
                getSP3Pos(m_PrnGpst - tempSatlitData.C2/M_C - tempSatlitData.StaClock/M_C,tempSatlitData.PRN,
                          tempSatlitData.SatType,pXYZ,pdXYZ, &sp3Clk);
                tempSatlitData.X = pXYZ[0];tempSatlitData.Y = pXYZ[1];tempSatlitData.Z = pXYZ[2];
                if (!(tempSatlitData.X && tempSatlitData.Y && tempSatlitData.Z && tempSatlitData.StaClock))
                {
                    QString errorline;
                    ErrorMsg(errorline);
                    tempSatlitData.badMsg.append("Can't calculate the orbit and clock offset"+errorline);
                    m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                    continue;
                }
                //PPP removes  satellites
                QString removeSat = QString(tempSatlitData.SatType);
                if(tempSatlitData.PRN < 10)
                    removeSat += "0" + QString::number(tempSatlitData.PRN);
                else
                    removeSat += QString::number(tempSatlitData.PRN);
                if(m_deleteSats.contains(removeSat, Qt::CaseInsensitive))
                {
                    QString errorline;
                    ErrorMsg(errorline);
                    tempSatlitData.badMsg.append("remove " + removeSat +errorline);
                    m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                    continue;
                }
                //frequency
                double F1 = tempSatlitData.Frq[0],F2 = tempSatlitData.Frq[1];
                if(F1 == 0 || F2 == 0) continue;

                //Obtain wide lane bias
                m_getbia.getgrgwlbiadata(tempSatlitData);

                //Computational relativistic effect
                double relative = 0;
                relative = getRelativty(tempSatlitData.SatType, pXYZ,spp_pos,pdXYZ);
                tempSatlitData.Relativty = relative;

                //Calculate the altitude angle of the satellite with the approximate coordinates of the receiver and determine the weight
                double EA[2]={0};
                getSatEA(tempSatlitData.X,tempSatlitData.Y,tempSatlitData.Z,spp_pos,EA);
                tempSatlitData.EA[0] = EA[0];tempSatlitData.EA[1] = EA[1];
                EA[0] = EA[0]*MM_PI/180;EA[1] = EA[1]*MM_PI/180;//Go to the arc to facilitate the calculation below
                getWight(tempSatlitData);

                //Calculate the rotation of the earth
                double earthW = 0;
                earthW = getSagnac(tempSatlitData.X,tempSatlitData.Y,tempSatlitData.Z,spp_pos);
                tempSatlitData.Sagnac = earthW;

                //Calculate the tropospheric dry delay
                //Julian day
                double MJD = qCmpGpsT.computeJD(epochTime.Year,epochTime.Month,epochTime.Day,
                                                epochTime.Hours,epochTime.Minutes,epochTime.Seconds) - 2400000.5;
                //TDay：doy day of year
                double TDay = qCmpGpsT.YearAccDay(epochTime.Year,epochTime.Month,epochTime.Day);
                double p_BLH[3] = {0},mf = 0, TropZHD_s = 0, store_epoch_ZHD;
                qCmpGpsT.XYZ2BLH(spp_pos[0], spp_pos[1], spp_pos[2], p_BLH);
                getTropDelay(MJD,TDay,EA[0],p_BLH,&mf, &TropZHD_s, NULL, &store_epoch_ZHD);
                tempSatlitData.SatTrop = TropZHD_s;
                tempSatlitData.StaTropMap = mf;
                tempSatlitData.UTCTime.TropZHD = store_epoch_ZHD;

                //Calculate antenna height
                tempSatlitData.AntHeight = p_HEN[0]*qSin(EA[0])
                        + p_HEN[1]*qCos(EA[0])*qSin(EA[1]) + p_HEN[2]*qCos(EA[0])*qCos(EA[1]);

                //Calculate the receiver antenna phase center
                double Lamta1 = M_C/F1,Lamta2 = M_C/F2;
                double L1Offset = 0,L2Offset = 0;
                getRecvOffset(EA,tempSatlitData.SatType,L1Offset,L2Offset, tempSatlitData.wantObserType);
                tempSatlitData.L1Offset = L1Offset/Lamta1;
                tempSatlitData.L2Offset = L2Offset/Lamta2;

                //Calculate the antenna phase center and update the sun and moon coordinates
                double SatL12Offset[2] = {0};
                getSatlitOffset(epochTime.Year,epochTime.Month,epochTime.Day,
                                epochTime.Hours,epochTime.Minutes,epochTime.Seconds - tempSatlitData.C2/M_C,
                                tempSatlitData.PRN,tempSatlitData.SatType,pXYZ,spp_pos, SatL12Offset, tempSatlitData.wantObserType);
                //pXYZ is the satellite coordinates
                tempSatlitData.SatL1Offset = SatL12Offset[0] / Lamta1;
                tempSatlitData.SatL2Offset = SatL12Offset[1] / Lamta2;

                //Calculate tide correction
                double effctDistance = 0;
                effctDistance = getTideEffect(epochTime.Year,epochTime.Month,epochTime.Day,
                                              epochTime.Hours,epochTime.Minutes,epochTime.Seconds,spp_pos,EA,
                                              m_ReadAntClass.m_sunpos,m_ReadAntClass.m_moonpos,m_ReadAntClass.m_gmst);
                tempSatlitData.TideEffect = effctDistance;

                //windup The correction is made on the basis of the previous epoch. When the current epoch is the first epoch, the value of the previous epoch is 0
                double AntWindup = 0,preAntWindup = 0;
                preAntWindup = getPreEpochWindUp(prevEpochSatlitData,tempSatlitData.PRN,tempSatlitData.SatType);
                AntWindup = getWindup(epochTime.Year,epochTime.Month,epochTime.Day,
                                      epochTime.Hours,epochTime.Minutes,epochTime.Seconds - tempSatlitData.C2/M_C,
                                      pXYZ,spp_pos,preAntWindup,m_ReadAntClass.m_sunpos);
                tempSatlitData.AntWindup = AntWindup;

                //Error correction in cycle
                tempSatlitData.LL1 = Lamta1*(tempSatlitData.L1 + tempSatlitData.L1Offset
                                             + tempSatlitData.SatL1Offset - tempSatlitData.AntWindup);
                tempSatlitData.LL2 = Lamta2*(tempSatlitData.L2 + tempSatlitData.L2Offset
                                             + tempSatlitData.SatL2Offset - tempSatlitData.AntWindup);
                tempSatlitData.CC1 = tempSatlitData.C1 + Lamta1*tempSatlitData.L1Offset
                        + Lamta1*tempSatlitData.SatL1Offset;
                tempSatlitData.CC2 = tempSatlitData.C2 + Lamta2 *tempSatlitData.L2Offset
                        + Lamta2*tempSatlitData.SatL2Offset;

                //IF
                double alpha1 = (F1*F1)/(F1*F1 - F2*F2),alpha2 = (F2*F2)/(F1*F1 - F2*F2);
                tempSatlitData.LL3 = alpha1*tempSatlitData.LL1 - alpha2*tempSatlitData.LL2;
                tempSatlitData.PP3 = alpha1*tempSatlitData.CC1 - alpha2*tempSatlitData.CC2;
                //Store current satellite data
                epochResultSatlitData.append(tempSatlitData);
            }//

            //Monitoring satellite status, cycle slip detection
            getGoodSatlite(prevEpochSatlitData,epochResultSatlitData, m_CutAngle);

            //Sort current observation 2021.03.22 23z
            int num_sys_temp = 0; QString sys_temp = "";
            if(m_sys_num > 1)
            {//G E R C
                QVector< SatlitData > sats_GPS;
                QVector< SatlitData > sats_GAL;
                QVector< SatlitData > sats_GLO;
                QVector< SatlitData > sats_BDS;
                QVector< SatlitData > sats_valid;
                SatlitData temp_sat;   QVector< int > sit_GERC;
                for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
                {
                    temp_sat = epochResultSatlitData.at(cf_i);
                    if(temp_sat.SatType == 'G')
                    {
                        sats_GPS.append(temp_sat);
                        sit_GERC.append(cf_i);
                    }
                    if(temp_sat.SatType == 'E')
                    {
                        sats_GAL.append(temp_sat);
                        sit_GERC.append(cf_i);
                    }
                    if(temp_sat.SatType == 'R')
                    {
                        sats_GLO.append(temp_sat);
                        sit_GERC.append(cf_i);
                    }
                    if(temp_sat.SatType == 'C')
                    {
                        sats_BDS.append(temp_sat);
                        sit_GERC.append(cf_i);
                    }
                }
                if(sats_GPS.length() > 2)
                {
                    num_sys_temp ++, sys_temp = sys_temp + 'G';
                    for(int cf_i = 0; cf_i < sats_GPS.length(); cf_i++)
                    {
                        temp_sat = sats_GPS.at(cf_i);
                        sats_valid.append(temp_sat);
                    }
                }
                if(sats_GAL.length() > 2)
                {
                    num_sys_temp ++, sys_temp = sys_temp + 'E';
                    for(int cf_i = 0; cf_i < sats_GAL.length(); cf_i++)
                    {
                        temp_sat = sats_GAL.at(cf_i);
                        sats_valid.append(temp_sat);
                    }
                }
                if(sats_GLO.length() > 2)
                {
                    num_sys_temp ++, sys_temp = sys_temp + 'R';
                    for(int cf_i = 0; cf_i < sats_GLO.length(); cf_i++)
                    {
                        temp_sat = sats_GLO.at(cf_i);
                        sats_valid.append(temp_sat);
                    }
                }
                if(sats_BDS.length() > 2)
                {
                    num_sys_temp ++, sys_temp = sys_temp + 'C';
                    for(int cf_i = 0; cf_i < sats_BDS.length(); cf_i++)
                    {
                        temp_sat = sats_BDS.at(cf_i);
                        sats_valid.append(temp_sat);
                    }
                }
                epochResultSatlitData = sats_valid;

            }
            else
            {
                num_sys_temp = 1;   sys_temp = m_sys_str;
            }

            if(!m_isKinematic)
                m_minSatFlag = 2*m_sys_num;
            else
                m_minSatFlag = 5;

            //Check whether the current number of satellites is sufficient
            if(epochResultSatlitData.length() < m_minSatFlag)
            {
                if(epochResultSatlitData.length() == 0)
                    continue;

                if(m_isKinematic&&continue_bad_epoch++ > 8)
                {
                    prevEpochSatlitData.clear();
                    continue_bad_epoch = 0;
                }

                for(int i = 0;i < epochResultSatlitData.length();i++)
                {
                    //flag 777
                    epochResultSatlitData[i].EpochFlag = 777;
                    epochResultSatlitData[i].VC1 = 0; epochResultSatlitData[i].VC2 = 0;
                    epochResultSatlitData[i].VL1 = 0; epochResultSatlitData[i].VL2 = 0;
                    epochResultSatlitData[i].VLL3 = 0; epochResultSatlitData[i].VPP3 = 0;
                }

                //
                VectorXd ENU_Vct;
                double spp_vct[3] = {0};
                int param_len = 3*epochResultSatlitData.length() + 32;
                ENU_Vct.resize(param_len);
                ENU_Vct.fill(0);
                saveResult2Class(ENU_Vct, spp_vct, epochTime, epochResultSatlitData, epoch_num);
                epoch_num++;
                continue;
            }

            //Select the reference sat according to the satellite elevation
            m_getbia.select_refsat(epochResultSatlitData, prevEpochSatlitData, m_CutAngle, sys_temp);
            m_getbia.get_refsat_sit(epochResultSatlitData, sys_temp);


            //Get the reference sat prn of the previous epoch
            for(int cf = 0; cf < epochResultSatlitData.length(); cf ++)
            {
                epochResultSatlitData[cf].interval = m_interval;
                epochResultSatlitData[cf].markname = m_markname;

                if(prevEpochSatlitData.length() == 0)
                {
                    for(int cf_j = 0; cf_j < 2*num_sys_temp; cf_j++)
                    {
                        epochResultSatlitData[cf].prn_referencesat_previous[cf_j] = 0;
                    }

                }
                else
                {
                    for(int cf_j = 0; cf_j < 2*num_sys_temp; cf_j++)
                    {
                        epochResultSatlitData[cf].prn_referencesat_previous[cf_j]
                                = prevEpochSatlitData[0].prn_referencesat[cf_j];
                    }
                }
            }

            if(global_cf::reinitialize != 999999)
            {//Reinitialize 2021.04.22 23z
                int reiniflag = global_cf::reinitialize;
                if(epochResultSatlitData.at(0).UTCTime.epochNum%reiniflag == 0)
                {
                    prevEpochSatlitData.clear();
                    flag_epoch = false;
                }
            }

            //filter
            MatrixXd P;
            VectorXd X;
            double spp_vct[3] = {0};
            bool is_filter_good = false;
            X.resize(3 + epochResultSatlitData.length());
            X.setZero();
            spp_vct[0] = spp_pos[0]; spp_vct[1] = spp_pos[1]; spp_vct[2] = spp_pos[2];

            if (!m_Solver_Method.compare("SRIF", Qt::CaseInsensitive))
                is_filter_good = m_SRIFAlgorithm.SRIFforStatic(prevEpochSatlitData,epochResultSatlitData,spp_pos,X,P);
            else
                is_filter_good = m_KalmanClass.KalmanforStatic(prevEpochSatlitData,epochResultSatlitData,spp_pos,X,P);


            //update
            if(is_filter_good)
            {
                m_qualityCtrl.CmpSatClkRate(prevEpochSatlitData, epochResultSatlitData);
                prevEpochSatlitData = epochResultSatlitData;
                continue_bad_epoch = 0;
            }
            else
            {
                continue_bad_epoch++;
                memset(spp_pos, 0, 4*sizeof(double));
                memset(spp_vct, 0, 3*sizeof(double));
                X.setZero();
            }
            if(m_isKinematic && continue_bad_epoch++ > 8)
            {
                prevEpochSatlitData.clear();
                continue_bad_epoch = 0;
            }

            //X to ENU  X = [dx,dy,dz,dTrop,dClock,N1,N2,...Nn]
            VectorXd ENU_Vct;
            ENU_Vct = X;
            ENU_Vct[0] = spp_pos[0]; ENU_Vct[1] = spp_pos[1]; ENU_Vct[2] = spp_pos[2];
            saveResult2Class(ENU_Vct, spp_vct, epochTime, epochResultSatlitData, epoch_num, &P);

            if(isDisplayEveryEpoch)
            {
                int Valid_SatNumber = epochResultSatlitData.length();
                disPlayQTextEdit = "Epoch number: " + QString::number(epoch_num);
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
                disPlayQTextEdit = "GPST: " + QString::number(epochTime.Hours) + ":" + QString::number(epochTime.Minutes)
                        + ":" + QString::number(epochTime.Seconds) + ENDLINE + "Satellite number: " + QString::number(epochSatlitData.length());
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                disPlayQTextEdit = "Valid sat number: " + QString::number(Valid_SatNumber) + ENDLINE
                        + "Estimated: " + '\n' + "X:   " + QString::number(spp_pos[0], 'f', 4)
                        + '\n' + "Y:   " + QString::number(spp_pos[1], 'f', 4)
                        + '\n' + "Z:   " + QString::number(spp_pos[2], 'f', 4) + ENDLINE;
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
            }

            //ENU_REC.append(ENU_Vct);

            epoch_num++;

            //            if(X(3,0) != 0)
            //            {
            //                q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,0) = X(3,0);
            //                q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,1) = P(3,3);
            //                q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,2) = sqrt(P(0,0)*P(0,0) + P(1,1)*P(1,1) + P(2,2)*P(2,2));
            //                q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,3) = epoch_num;
            //                q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,4) = spp_pos[0];
            //                q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,5) = spp_pos[1];
            //                q_zwd(epochResultSatlitData.at(0).UTCTime.epochNum,6) = spp_pos[2];
            //            }

        }

        //        QString filepath = "/home/cfz/data_23z/zcompare/test_of_CFZ/SSD_K-GE/" + m_markname + ".csv";
        //        const char *path = filepath.toLatin1().data();
        //        m_matrix.writeCSV(path, q_zwd);

        // clear
        multepochSatlitData.clear();

    }

    prevEpochSatlitData.clear();

    m_PPPModel_Str = "SSD";

    //write to file
    if(!m_IS_MAX_OBS) writeResult2File();

    if(isDisplayEveryEpoch)
    {
        disPlayQTextEdit = "done";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
    }
    //flag of finash
    m_isRuned = true;

}




//PPPAR 2020.11.30 23z  refer to RUNSSDPPP
void QPPPModel::runPPPAR(bool isDisplayEveryEpoch)
{
    if(!m_haveObsFile) return;
    //Initialization parameters
    //Antenna height
    double p_HEN[3] = {0};
    m_ReadOFileClass.getAntHEN(p_HEN);
    //Store the satellite data of the previous epoch
    QVector < SatlitData > prevEpochSatlitData;
    //Calculate receiver coordinates and clock offset
    double spp_pos[4] = {0};// store SPP pos and clk
    memcpy(spp_pos, m_ApproxRecPos, 3*sizeof(double));
    int epoch_num = 0, continue_bad_epoch = 0;
    bool isInitSpp = false;
    if(spp_pos[0] !=0 ) isInitSpp = true;
    QString disPlayQTextEdit = "";

    if(AC_product == "GRG")
    {
        m_getbia.readDCBfile(GPSDCBfilepath);
        //2020.11.09 23z
        //Read the wide lane bias in the grg*****.clk file of CNES
        //2021.03.29 23z
        //CNES provides a new .wsb wide lane bias product, which is included in copydata
        m_getbia.readgrgwlbiafile();
    }
    if(AC_product == "COD")
    {
        //2021.07.25 23z AR_IPC based on COD products
        m_getbia.readcodosbfile(codbiafilepath);
    }

    //2021.01.07 23Z
    m_getbia.readPRN_GPS();
    if(!shadfilepath.isEmpty())
        m_getbia.readshadfile(shadfilepath);

    while (!m_ReadOFileClass.isEnd())
    {

        //Obtain multi-epoch observation data
        QVector< QVector < SatlitData > > multepochSatlitData;
        m_ReadOFileClass.getMultEpochData(multepochSatlitData,multReadOFile);

        //2021.04.07 23z There may be no sampling rate information in the header file
        m_interval = getinterval(multepochSatlitData);

        int flag_sampling = 1;
        if(global_cf::flag_sampling)
        {
            double interval_original, interval_used;
            interval_original = global_cf::Interval_original;
            interval_used = global_cf::Interval_used;

            int temp_test = interval_used/interval_original;
            bool flag_temp_test = true;
            if(temp_test*interval_original == interval_used) flag_temp_test = false;
            if(m_interval != interval_original || interval_used < interval_original || flag_temp_test)
            {//23z
                QString info = "There is something wrong with sampling!";
                QMessageBox::warning(mp_QTextEditforDisplay, "Warning", info);
                break;
            }
            else
                flag_sampling = temp_test;
        }

        multepochSatlitData[0][0].interval = m_interval;
        //Delete short arc satellite  2021.01.16 23z
        m_getbia.deletetempsats(multepochSatlitData, m_sys_str, m_interval);
        //        //Cycle slip detection        2021.03.01 23z
        //        m_getbia.cycleslip_refsats(multepochSatlitData, m_sys_str);

        //Multi-epoch data loop processing
        for (int epoch = 0; epoch < multepochSatlitData.length();epoch = epoch + flag_sampling)
        {

            //Store the original observations of the current epoch
            QVector< SatlitData > epochSatlitData;
            //Store the model-corrected observations of the current epoch
            QVector< SatlitData > epochResultSatlitData;
            epochSatlitData = multepochSatlitData.at(epoch);

            //If there is no data, go to the next epoch
            if(epochSatlitData.length() == 0) continue;

            GPSPosTime epochTime;

            if(epochSatlitData.length() > 0)
            {
                //Get the time information of the current epoch
                epochTime= epochSatlitData.at(0).UTCTime;
                epochTime.epochNum = epoch_num;
            }

            //2021.01.07 23z
            if(!shadfilepath.isEmpty())
                m_getbia.shadcorrcect(epochSatlitData);

            //Set the current epoch number
            for(int i = 0;i < epochSatlitData.length();i++)
                epochSatlitData[i].UTCTime.epochNum = epoch_num;
            //Use SPP to calculate the initial value of the coordinate position and the receiver clock error
            SimpleSPP(prevEpochSatlitData, epochSatlitData, spp_pos);

            if(!isInitSpp && spp_pos[0] != 0)
                memcpy(m_ApproxRecPos, spp_pos, 3*sizeof(double));
            if(!m_isKinematic)
                memcpy(spp_pos, m_ApproxRecPos, 3*sizeof(double));
            if(!isnormal(spp_pos[0]))
                memset(spp_pos, 0, 4*sizeof(double));

            //When the observation is insufficient
            if(epochSatlitData.length() < m_minSatFlag || spp_pos[0] == 0)
            {
                if(epochSatlitData.length() == 0)
                    continue;

                if(m_isKinematic && continue_bad_epoch++ > 8)
                {
                    prevEpochSatlitData.clear();
                    continue_bad_epoch = 0;
                }

                for(int i = 0;i < epochSatlitData.length();i++)
                {
                    //Set error flag 888
                    epochSatlitData[i].EpochFlag = 888;
                    //residual = 0
                    epochSatlitData[i].VC1 = 0; epochSatlitData[i].VC2 = 0;
                    epochSatlitData[i].VL1 = 0; epochSatlitData[i].VL2 = 0;
                    epochSatlitData[i].VLL3 = 0; epochSatlitData[i].VPP3 = 0;
                }

                VectorXd ENU_Vct;
                double spp_vct[3] = {0};
                int param_len = 3*epochSatlitData.length() + 32;
                ENU_Vct.resize(param_len);
                ENU_Vct.fill(0);
                //Store the result and go to the next epoch
                saveResult2Class(ENU_Vct, spp_vct, epochTime, epochSatlitData, epoch_num);
                epoch_num++;
                continue;
            }

            for (int i = 0;i < epochSatlitData.length();i++)
            {
                //Satellite-by-satellite processing
                SatlitData tempSatlitData = epochSatlitData.at(i);

                //2021.01.16 23z
                tempSatlitData.epochlength = epochSatlitData.length();

                //Set SSDPPP processing flag 2020.11.29 23z
                tempSatlitData.SSDPPP = true;

                //Set PPPAR processing flag 2020.11.30 23z
                tempSatlitData.ARornot = true;

                if(!isInSystem(tempSatlitData.SatType))
                    continue;
                //Check whether the current satellite observations are complete
                if(!(tempSatlitData.L1&&tempSatlitData.L2&&tempSatlitData.C1&&tempSatlitData.C2))
                {
                    QString errorline;
                    ErrorMsg(errorline);
                    tempSatlitData.badMsg.append("Lack of observations"+errorline);
                    m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                    continue;
                }
                //GPST
                double m_PrnGpst = qCmpGpsT.YMD2GPSTime(epochTime.Year,epochTime.Month,epochTime.Day,
                                                        epochTime.Hours,epochTime.Minutes,epochTime.Seconds);
                double stalitClock = 0.0;
                //Pay attention to the effect of signal propagation time(m_PrnGpst - tempSatlitData.C2/M_C)
                getCLKData(tempSatlitData.PRN,tempSatlitData.SatType,m_PrnGpst - tempSatlitData.C2/M_C,&stalitClock);
                tempSatlitData.StaClock = stalitClock;
                tempSatlitData.StaClockRate = 0;
                //Obtain the satellite coordinates from the SP3 file (unit: meter) Coordinate\Speed\Clock offset
                double pXYZ[3] = {0},pdXYZ[3] = {0}, sp3Clk = 0.0;
                //Pay attention to the effect of signal propagation time(m_PrnGpst - tempSatlitData.C2/M_C - tempSatlitData.StaClock/M_C)
                //Obtain the precise satellite coordinates at the time when the satellite signal is transmitted. Note: You need to subtract the satellite clock error tempSatlitData.StaClock/M_C, otherwise it will cause an error of 0.2 meters
                getSP3Pos(m_PrnGpst - tempSatlitData.C2/M_C - tempSatlitData.StaClock/M_C,tempSatlitData.PRN,
                          tempSatlitData.SatType,pXYZ,pdXYZ, &sp3Clk);
                tempSatlitData.X = pXYZ[0];tempSatlitData.Y = pXYZ[1];tempSatlitData.Z = pXYZ[2];
                if (!(tempSatlitData.X && tempSatlitData.Y && tempSatlitData.Z && tempSatlitData.StaClock))
                {
                    QString errorline;
                    ErrorMsg(errorline);
                    tempSatlitData.badMsg.append("Can't calculate the orbit and clock offset"+errorline);
                    m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                    continue;
                }
                //PPP removes  satellites
                QString removeSat = QString(tempSatlitData.SatType);
                if(tempSatlitData.PRN < 10)
                    removeSat += "0" + QString::number(tempSatlitData.PRN);
                else
                    removeSat += QString::number(tempSatlitData.PRN);
                if(m_deleteSats.contains(removeSat, Qt::CaseInsensitive))
                {
                    QString errorline;
                    ErrorMsg(errorline);
                    tempSatlitData.badMsg.append("remove " + removeSat +errorline);
                    m_writeFileClass.allBadSatlitData.append(tempSatlitData);
                    continue;
                }
                //frequency
                double F1 = tempSatlitData.Frq[0],F2 = tempSatlitData.Frq[1];
                if(F1 == 0 || F2 == 0) continue;

                if(AC_product == "GRG")
                {//23z
                    //P1C1.DCB
                    m_getbia.getDCBdata(tempSatlitData);
                    //Obtain wide lane bias
                    m_getbia.getgrgwlbiadata(tempSatlitData);
                }
                if(AC_product == "COD")
                {//23z
                    m_getbia.getcodosb(tempSatlitData);
                }

                //Computational relativistic effect
                double relative = 0;
                relative = getRelativty(tempSatlitData.SatType, pXYZ,spp_pos,pdXYZ);
                tempSatlitData.Relativty = relative;

                //Calculate the altitude angle of the satellite with the approximate coordinates of the receiver and determine the weight
                double EA[2]={0};
                getSatEA(tempSatlitData.X,tempSatlitData.Y,tempSatlitData.Z,spp_pos,EA);
                tempSatlitData.EA[0] = EA[0];tempSatlitData.EA[1] = EA[1];
                EA[0] = EA[0]*MM_PI/180;EA[1] = EA[1]*MM_PI/180;//Go to the arc to facilitate the calculation below
                getWight(tempSatlitData);

                //Calculate the rotation of the earth
                double earthW = 0;
                earthW = getSagnac(tempSatlitData.X,tempSatlitData.Y,tempSatlitData.Z,spp_pos);
                tempSatlitData.Sagnac = earthW;

                //Calculate the tropospheric dry delay
                //Julian day
                double MJD = qCmpGpsT.computeJD(epochTime.Year,epochTime.Month,epochTime.Day,
                                                epochTime.Hours,epochTime.Minutes,epochTime.Seconds) - 2400000.5;
                //TDay：doy day of year
                double TDay = qCmpGpsT.YearAccDay(epochTime.Year,epochTime.Month,epochTime.Day);
                double p_BLH[3] = {0},mf = 0, TropZHD_s = 0, store_epoch_ZHD;
                qCmpGpsT.XYZ2BLH(spp_pos[0], spp_pos[1], spp_pos[2], p_BLH);
                getTropDelay(MJD,TDay,EA[0],p_BLH,&mf, &TropZHD_s, NULL, &store_epoch_ZHD);
                tempSatlitData.SatTrop = TropZHD_s;
                tempSatlitData.StaTropMap = mf;
                tempSatlitData.UTCTime.TropZHD = store_epoch_ZHD;

                //Calculate antenna height
                tempSatlitData.AntHeight = p_HEN[0]*qSin(EA[0])
                        + p_HEN[1]*qCos(EA[0])*qSin(EA[1]) + p_HEN[2]*qCos(EA[0])*qCos(EA[1]);

                //Calculate the receiver antenna phase center
                double Lamta1 = M_C/F1,Lamta2 = M_C/F2;
                double L1Offset = 0,L2Offset = 0;
                getRecvOffset(EA,tempSatlitData.SatType,L1Offset,L2Offset, tempSatlitData.wantObserType);
                tempSatlitData.L1Offset = L1Offset/Lamta1;
                tempSatlitData.L2Offset = L2Offset/Lamta2;

                //Calculate the antenna phase center and update the sun and moon coordinates
                double SatL12Offset[2] = {0};
                getSatlitOffset(epochTime.Year,epochTime.Month,epochTime.Day,
                                epochTime.Hours,epochTime.Minutes,epochTime.Seconds - tempSatlitData.C2/M_C,
                                tempSatlitData.PRN,tempSatlitData.SatType,pXYZ,spp_pos, SatL12Offset, tempSatlitData.wantObserType);
                //pXYZ is the satellite coordinates
                tempSatlitData.SatL1Offset = SatL12Offset[0] / Lamta1;
                tempSatlitData.SatL2Offset = SatL12Offset[1] / Lamta2;

                //Calculate tide correction
                double effctDistance = 0;
                effctDistance = getTideEffect(epochTime.Year,epochTime.Month,epochTime.Day,
                                              epochTime.Hours,epochTime.Minutes,epochTime.Seconds,spp_pos,EA,
                                              m_ReadAntClass.m_sunpos,m_ReadAntClass.m_moonpos,m_ReadAntClass.m_gmst);
                tempSatlitData.TideEffect = effctDistance;

                //windup The windup correction is made on the basis of the previous epoch. When the current epoch is the first epoch, the value of the previous epoch is 0
                double AntWindup = 0,preAntWindup = 0;
                preAntWindup = getPreEpochWindUp(prevEpochSatlitData,tempSatlitData.PRN,tempSatlitData.SatType);
                AntWindup = getWindup(epochTime.Year,epochTime.Month,epochTime.Day,
                                      epochTime.Hours,epochTime.Minutes,epochTime.Seconds - tempSatlitData.C2/M_C,
                                      pXYZ,spp_pos,preAntWindup,m_ReadAntClass.m_sunpos);
                tempSatlitData.AntWindup = AntWindup;

                //correction in cycle
                tempSatlitData.LL1 = Lamta1*(tempSatlitData.L1 + tempSatlitData.L1Offset
                                             + tempSatlitData.SatL1Offset - tempSatlitData.AntWindup);
                tempSatlitData.LL2 = Lamta2*(tempSatlitData.L2 + tempSatlitData.L2Offset
                                             + tempSatlitData.SatL2Offset - tempSatlitData.AntWindup);
                tempSatlitData.CC1 = tempSatlitData.C1 + Lamta1*tempSatlitData.L1Offset
                        + Lamta1*tempSatlitData.SatL1Offset;
                tempSatlitData.CC2 = tempSatlitData.C2 + Lamta2 *tempSatlitData.L2Offset
                        + Lamta2*tempSatlitData.SatL2Offset;

                //IF
                double alpha1 = (F1*F1)/(F1*F1 - F2*F2),alpha2 = (F2*F2)/(F1*F1 - F2*F2);
                tempSatlitData.LL3 = alpha1*tempSatlitData.LL1 - alpha2*tempSatlitData.LL2;
                tempSatlitData.PP3 = alpha1*tempSatlitData.CC1 - alpha2*tempSatlitData.CC2;
                //Store current satellite data
                epochResultSatlitData.append(tempSatlitData);
            }//

            //Monitoring satellite status, cycle slip detection
            getGoodSatlite(prevEpochSatlitData,epochResultSatlitData, m_CutAngle);


            //carrier gross error detection 23z
            //m_getbia.gross_L(epochResultSatlitData, spp_pos);

            //Sort current observation types 2021.03.22 23z
            int num_sys_temp = 0; QString sys_temp = "";
            if(m_SatSystem.length() > 1)
            {//G E R C
                QVector< SatlitData > sats_GPS;
                QVector< SatlitData > sats_GAL;
                QVector< SatlitData > sats_GLO;
                QVector< SatlitData > sats_BDS;
                QVector< SatlitData > sats_valid;
                SatlitData temp_sat;   QVector< int > sit_GERC;
                for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
                {
                    temp_sat = epochResultSatlitData.at(cf_i);
                    if(temp_sat.SatType == 'G')
                    {
                        sats_GPS.append(temp_sat);
                        sit_GERC.append(cf_i);
                    }
                    if(temp_sat.SatType == 'E')
                    {
                        sats_GAL.append(temp_sat);
                        sit_GERC.append(cf_i);
                    }
                    if(temp_sat.SatType == 'R')
                    {
                        sats_GLO.append(temp_sat);
                        sit_GERC.append(cf_i);
                    }
                    if(temp_sat.SatType == 'C')
                    {
                        sats_BDS.append(temp_sat);
                        sit_GERC.append(cf_i);
                    }
                }
                if(sats_GPS.length() > 3)
                {
                    num_sys_temp ++, sys_temp = sys_temp + 'G';
                    for(int cf_i = 0; cf_i < sats_GPS.length(); cf_i++)
                    {
                        temp_sat = sats_GPS.at(cf_i);
                        sats_valid.append(temp_sat);
                    }
                }
                if(sats_GAL.length() > 3)
                {
                    num_sys_temp ++, sys_temp = sys_temp + 'E';
                    for(int cf_i = 0; cf_i < sats_GAL.length(); cf_i++)
                    {
                        temp_sat = sats_GAL.at(cf_i);
                        sats_valid.append(temp_sat);
                    }
                }
                if(sats_GLO.length() > 3)
                {
                    num_sys_temp ++, sys_temp = sys_temp + 'R';
                    for(int cf_i = 0; cf_i < sats_GLO.length(); cf_i++)
                    {
                        temp_sat = sats_GLO.at(cf_i);
                        sats_valid.append(temp_sat);
                    }
                }
                if(sats_BDS.length() > 3)
                {
                    num_sys_temp ++, sys_temp = sys_temp + 'C';
                    for(int cf_i = 0; cf_i < sats_BDS.length(); cf_i++)
                    {
                        temp_sat = sats_BDS.at(cf_i);
                        sats_valid.append(temp_sat);
                    }
                }
                epochResultSatlitData = sats_valid;
                if(num_sys_temp > 0)
                {
                    m_sys_num = num_sys_temp;
                    m_sys_str = sys_temp;
                }

            }
            else
            {
                num_sys_temp = 1;   sys_temp = m_sys_str;
            }

            if(!m_isKinematic)
                m_minSatFlag = 1 + 2*num_sys_temp;
            else
                m_minSatFlag = 5;


            //Check whether the current number of satellites is sufficient
            if(epochResultSatlitData.length() < m_minSatFlag)
            {
                if(epochResultSatlitData.length() == 0)
                    continue;

                if(m_isKinematic&&continue_bad_epoch++ > 8)
                {
                    prevEpochSatlitData.clear();
                    continue_bad_epoch = 0;
                }

                for(int i = 0;i < epochResultSatlitData.length();i++)
                {
                    //Set error flag 777
                    epochResultSatlitData[i].EpochFlag = 777;
                    epochResultSatlitData[i].VC1 = 0; epochResultSatlitData[i].VC2 = 0;
                    epochResultSatlitData[i].VL1 = 0; epochResultSatlitData[i].VL2 = 0;
                    epochResultSatlitData[i].VLL3 = 0; epochResultSatlitData[i].VPP3 = 0;
                }

                VectorXd ENU_Vct;
                double spp_vct[3] = {0};
                int param_len = 3*epochResultSatlitData.length() + 32;
                ENU_Vct.resize(param_len);
                ENU_Vct.fill(0);
                saveResult2Class(ENU_Vct, spp_vct, epochTime, epochResultSatlitData, epoch_num);
                epoch_num++;
                continue;
            }

            //Select the reference sat according to the satellite elevation
            m_getbia.select_refsat(epochResultSatlitData, prevEpochSatlitData, m_CutAngle, sys_temp);
            m_getbia.get_refsat_sit(epochResultSatlitData, sys_temp);

            //Get the reference sat prn of the previous epoch
            for(int cf = 0; cf < epochResultSatlitData.length(); cf ++)
            {
                //Get the sampling interval for calculating the tropospheric randomwalk 2020.12.16 23z
                epochResultSatlitData[cf].interval = m_interval;
                epochResultSatlitData[cf].markname = m_markname;

                if(prevEpochSatlitData.length() == 0)
                {
                    for(int cf_j = 0; cf_j < 2*num_sys_temp; cf_j++)
                    {
                        epochResultSatlitData[cf].prn_referencesat_previous[cf_j] = 0;
                    }

                }
                else
                {
                    for(int cf_j = 0; cf_j < 2*num_sys_temp; cf_j++)
                    {
                        epochResultSatlitData[cf].prn_referencesat_previous[cf_j]
                                = prevEpochSatlitData[0].prn_referencesat[cf_j];
                    }
                }
            }


            //filter
            MatrixXd P;
            VectorXd X;
            double spp_vct[3] = {0};
            bool is_filter_good = false;
            X.resize(3 + epochResultSatlitData.length());
            X.setZero();
            spp_vct[0] = spp_pos[0]; spp_vct[1] = spp_pos[1]; spp_vct[2] = spp_pos[2];

            if(AC_product == "GRG")
            {
                //P1C1.DCB
                for(int cf_i = 0; cf_i < 4; cf_i++)
                {
                    QString type_obs = "";
                    double c1_corrected = 0.0;
                    type_obs = epochResultSatlitData.at(0).wantObserType[cf_i];
                    if(type_obs.contains("C1") || type_obs.contains("C1C"))
                    {
                        for(int cf_j = 0; cf_j < epochResultSatlitData.length(); cf_j++)
                        {
                            c1_corrected = epochResultSatlitData.at(cf_j).C1 +
                                    epochResultSatlitData.at(cf_j).DCB_P1C1 * M_C * 1e-9;
                            epochResultSatlitData[cf_j].C1 = c1_corrected;
                        }
                    }

                }
            }


            if(global_cf::reinitialize != 999999)
            {//Reinitialize 2021.04.22 23z
                int reiniflag = global_cf::reinitialize;
                if(epochResultSatlitData.at(0).UTCTime.epochNum%reiniflag == 0)
                {
                    prevEpochSatlitData.clear();

                    q_zwd_pre = 1000;
                    q_zwd_cur = 0.0;
                    q_zwd_slope_pre = -1.0;
                    q_zwd_slope_cur = 0.0;
                    infpoints = 0;
                    flag_epoch = false;
                }
            }


            if (!m_Solver_Method.compare("SRIF", Qt::CaseInsensitive))
                is_filter_good = m_SRIFAlgorithm.SRIFforStatic(prevEpochSatlitData,epochResultSatlitData,spp_pos,X,P);
            else
                is_filter_good = m_KalmanClass.KalmanforStatic(prevEpochSatlitData,epochResultSatlitData,spp_pos,X,P);


            //AR processing
            if(epochResultSatlitData.at(0).ARornot && prevEpochSatlitData.length() != 0)
            {
                //calculate wl amb 2020.11.30 23z
                calculatewl(prevEpochSatlitData,epochResultSatlitData);

                //calculate nl amb 2020.12.01 23z
                calculatenl(prevEpochSatlitData,epochResultSatlitData,X,P);

                if(is_filter_good)
                {
                    //Refilter the current epoch
                    QVector< SatlitData > epoch_temp;
                    epoch_temp = epochResultSatlitData;

                    int times = 0;
                    times = prevEpochSatlitData.at(0).times_hold;

                    if(times != 0)
                    {
                        for(int cf_i = 0; cf_i < epoch_temp.length(); cf_i++)
                            epoch_temp[cf_i].times_hold = times;
                    }

                    //Constrained filtering 2020.12.07 23z
                    MatrixXd temp_P;
                    bool cf_flag = false;
                    if(epochResultSatlitData.at(0).SSD_fixed_flag)
                        cf_flag = m_KalmanClass.KalmanforStatic(epoch_temp,epochResultSatlitData,spp_pos,X,temp_P);
                    if(cf_flag) P = temp_P;
                }

            }


            //update
            if(is_filter_good)
            {
                m_qualityCtrl.CmpSatClkRate(prevEpochSatlitData, epochResultSatlitData);
                prevEpochSatlitData = epochResultSatlitData;
                continue_bad_epoch = 0;
            }
            else
            {
                continue_bad_epoch++;
                memset(spp_pos, 0, 4*sizeof(double));
                memset(spp_vct, 0, 3*sizeof(double));
                X.setZero();
                //prevEpochSatlitData = epochResultSatlitData;
            }
            if(m_isKinematic && continue_bad_epoch++ > 8)
            {
                prevEpochSatlitData.clear();
                continue_bad_epoch = 0;
            }

            //X = [dx,dy,dz,dTrop,dClock,N1,N2,...Nn]
            VectorXd ENU_Vct;
            ENU_Vct = X;
            ENU_Vct[0] = spp_pos[0]; ENU_Vct[1] = spp_pos[1]; ENU_Vct[2] = spp_pos[2];
            saveResult2Class(ENU_Vct, spp_vct, epochTime, epochResultSatlitData, epoch_num, &P);

            if(isDisplayEveryEpoch)
            {
                int Valid_SatNumber = epochResultSatlitData.length();
                disPlayQTextEdit = "Epoch number: " + QString::number(epoch_num);
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
                disPlayQTextEdit = "GPST: " + QString::number(epochTime.Hours) + ":" + QString::number(epochTime.Minutes)
                        + ":" + QString::number(epochTime.Seconds) + ENDLINE + "Satellite number: " + QString::number(epochSatlitData.length());
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);// display for QTextEdit
                disPlayQTextEdit = "Valid sat number: " + QString::number(Valid_SatNumber) + ENDLINE
                        + "Estimated: " + '\n' + "X:   " + QString::number(spp_pos[0], 'f', 4)
                        + '\n' + "Y:   " + QString::number(spp_pos[1], 'f', 4)
                        + '\n' + "Z:   " + QString::number(spp_pos[2], 'f', 4) + ENDLINE;
                autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);
            }

            //ENU_REC.append(ENU_Vct);

            epoch_num++;

            if(epoch_num > 1)
            {
                int a = 23;

            }


        }//

        // clear
        multepochSatlitData.clear();

    }//

    //    m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/residual/amb.csv",amb_info);
    //    m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/residual/vamb.csv",vamb_info);

    prevEpochSatlitData.clear();

    if(global_cf::fix_and_hold) m_PPPModel_Str = "ARF(SSD)";
    else m_PPPModel_Str = "ARI(SSD)";

    if(!m_IS_MAX_OBS) writeResult2File();

    if(isDisplayEveryEpoch)
    {
        disPlayQTextEdit = "done";
        autoScrollTextEdit(mp_QTextEditforDisplay, disPlayQTextEdit);

    }
    m_isRuned = true;

}



//calculate wl amb 2020.11.30 23z
void QPPPModel::calculatewl(QVector<SatlitData> &prevEpochSatlitData, QVector<SatlitData> &epochResultSatlitData)
{
    QString sys_cur = ""; int sys_num_cur = 0;

    double f1 = 0.0, f2 = 0.0, lamda_wl = 0.0;
    double wl = 0.0, wl_smooth_pre = 0.0;
    double l1 = 0.0, l2 = 0.0, p1 = 0.0, p2 = 0.0;

    //The number of satellite systems in the current epoch
    sys_cur = epochResultSatlitData.at(0).SatType;
    sys_num_cur = 1;
    for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
    {
        if(!sys_cur.contains(epochResultSatlitData.at(cf_i).SatType))
            sys_num_cur++, sys_cur = sys_cur + epochResultSatlitData.at(cf_i).SatType;
    }

    if(prevEpochSatlitData.length() == 0)
    {//First epoch
        for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
        {
            f1 = epochResultSatlitData.at(cf_i).Frq[0];
            f2 = epochResultSatlitData.at(cf_i).Frq[1];
            lamda_wl = M_C/(f1 - f2);

            l1 = epochResultSatlitData.at(cf_i).LL1;
            l2 = epochResultSatlitData.at(cf_i).LL2;
            p1 = epochResultSatlitData.at(cf_i).CC1;
            p2 = epochResultSatlitData.at(cf_i).CC2;

            //When using CODE product for AR, grg_bia_wl = 0 CYCLE
            wl_smooth_pre = 0.0;
            wl = ((f1*l1 - f2*l2)/(f1 - f2) - (f1*p1 + f2*p2)/(f1 + f2))/lamda_wl
                    + epochResultSatlitData.at(cf_i).grg_bia_wl;
            epochResultSatlitData[cf_i].wl_smooth = wl_smooth_pre +
                    (wl - wl_smooth_pre)/1;
            //The number of consecutive occurrences of the satellite
            epochResultSatlitData[cf_i].times_wl = 1;
        }
    }
    else
    {
        for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
        {
            //Get the smooth value of the wide lane in the previous epoch
            int prn = 0, times = 0;
            prn = epochResultSatlitData.at(cf_i).PRN;
            for(int cf_j = 0; cf_j < prevEpochSatlitData.length(); cf_j++)
            {
                if(prevEpochSatlitData.at(cf_j).PRN == prn
                        && epochResultSatlitData.at(cf_i).SatType == prevEpochSatlitData.at(cf_j).SatType)
                {
                    wl_smooth_pre = prevEpochSatlitData.at(cf_j).wl_smooth;
                    times = prevEpochSatlitData.at(cf_j).times_wl;
                    break;
                }
                else
                {
                    //If the satellite did not appear in the previous epoch, re-initialize
                    wl_smooth_pre = 0.0;
                    times = 0;
                }
            }
            f1 = epochResultSatlitData.at(cf_i).Frq[0];
            f2 = epochResultSatlitData.at(cf_i).Frq[1];
            lamda_wl = M_C/(f1 - f2);
            //Calculate the ambiguity of the wide lane in the current epoch and its smooth value   CYCLE
            l1 = epochResultSatlitData.at(cf_i).LL1;
            l2 = epochResultSatlitData.at(cf_i).LL2;
            p1 = epochResultSatlitData.at(cf_i).CC1;
            p2 = epochResultSatlitData.at(cf_i).CC2;
            wl = ((f1*l1 - f2*l2)/(f1 - f2) - (f1*p1 + f2*p2)/(f1 + f2))/lamda_wl
                    + epochResultSatlitData.at(cf_i).grg_bia_wl;
            epochResultSatlitData[cf_i].wl_smooth = wl_smooth_pre +
                    (wl - wl_smooth_pre)/(times + 1);

            epochResultSatlitData[cf_i].times_wl = times + 1;

        }

    }

    for(int cf_sys = 0; cf_sys < sys_num_cur; cf_sys++)
    {
        if(sys_cur.at(cf_sys) != "G" && sys_cur.at(cf_sys) != "E") continue;

        //Single difference between the sats of wl
        int sit_ref = 0;
        double wl_smooth_ref = 0.0;
        QString sys_ref = "";
        sit_ref = epochResultSatlitData.at(0).prn_referencesat[2*cf_sys + 1];
        wl_smooth_ref = epochResultSatlitData.at(sit_ref).wl_smooth;
        sys_ref = sys_cur.at(cf_sys);
        for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
        {
            if(epochResultSatlitData.at(cf_i).SatType == sys_ref)
            {
                epochResultSatlitData[cf_i].wl_float_SSD = epochResultSatlitData.at(cf_i).wl_smooth - wl_smooth_ref;
                if(epochResultSatlitData.at(cf_i).wl_float_SSD == 0)
                    epochResultSatlitData[cf_i].wl_fixed_flag = false;
            }
        }
    }


    //When the number of consecutive epochs exceeds 10, round to the nearest integer
    int wl_fixed_SSD = 0;
    double diff = 0.0;
    for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
    {
        if(epochResultSatlitData.at(cf_i).times_wl > 10)
        {
            wl_fixed_SSD = qRound(epochResultSatlitData.at(cf_i).wl_float_SSD);
            diff = qAbs(wl_fixed_SSD - epochResultSatlitData.at(cf_i).wl_float_SSD);

            if(diff < 0.25)
            {
                epochResultSatlitData[cf_i].wl_fixed_SSD = wl_fixed_SSD;
                epochResultSatlitData[cf_i].wl_fixed_flag = true;
            }

        }
        else
            epochResultSatlitData[cf_i].wl_fixed_flag = false;
    }

    for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
    {
        if(epochResultSatlitData.at(cf_i).wl_fixed_SSD != 0)
        {
            double temp_diff = qAbs(epochResultSatlitData.at(cf_i).wl_float_SSD - epochResultSatlitData.at(cf_i).wl_fixed_SSD);
            if(temp_diff > 0.5)
            {
                epochResultSatlitData[cf_i].wl_fixed_SSD = 0;
                epochResultSatlitData[cf_i].wl_fixed_flag = false;
            }
        }
    }

    int sys_num_pre = 1;   QString sys_pre = "";
    sys_pre = prevEpochSatlitData.at(0).SatType;
    for(int cf_num = 1; cf_num < prevEpochSatlitData.length(); cf_num++)
    {
        if(!sys_pre.contains(prevEpochSatlitData.at(cf_num).SatType))
            sys_num_pre++,sys_pre = sys_pre + prevEpochSatlitData.at(cf_num).SatType;
    }

    for(int cf_sys = 0; cf_sys < sys_num_cur; cf_sys++)
    {
        if(sys_cur.at(cf_sys) != "G" && sys_cur.at(cf_sys) != "E") continue;

        //There will be an ambiguity difference when the reference sat changes
        if(prevEpochSatlitData.length() != 0)
        {
            int prn_ref_cur = 0, prn_ref_pre = 0, pre_sit = -923;
            QString sys_ref;
            sys_ref = sys_cur.at(cf_sys);

            if(sys_cur == sys_pre) pre_sit = cf_sys;
            else
            {
                for(int cf_pre = 0; cf_pre < sys_num_pre; cf_pre++)
                {
                    if(sys_pre.at(cf_pre) == sys_ref)
                    {
                        pre_sit = cf_pre;
                        break;
                    }
                }
            }

            prn_ref_cur = epochResultSatlitData.at(0).prn_referencesat[2*cf_sys];
            if(pre_sit == -923) prn_ref_pre = -923;
            else prn_ref_pre = prevEpochSatlitData.at(0).prn_referencesat[2*pre_sit];

            int diff_wl = 0;
            if(prn_ref_cur != prn_ref_pre)
            {
                //here is a change in the reference sat at this time
                //Match the wide lane ambiguity of the satellite that has also been fixed in the previous epoch
                for(int cf_j = 0; cf_j < prevEpochSatlitData.length(); cf_j++)
                {
                    int prn_pre = prevEpochSatlitData.at(cf_j).PRN;
                    if(prn_pre == prn_ref_cur && prevEpochSatlitData.at(cf_j).wl_fixed_flag && prevEpochSatlitData.at(cf_j).SatType == sys_ref)
                    {
                        diff_wl = prevEpochSatlitData.at(cf_j).wl_fixed_SSD;
                        break;
                    }
                }
                for(int cf_k = 0; cf_k < epochResultSatlitData.length(); cf_k++)
                    epochResultSatlitData[cf_k].diff_wl_refsats[cf_sys] = diff_wl;

            }
        }
    }

}




//calculate nl amb 2020.12.01 23z
void QPPPModel::calculatenl(QVector < SatlitData > &prevEpochSatlitData, QVector<SatlitData> &epochResultSatlitData, VectorXd X, MatrixXd P)
{
    if(!flag_epoch)
    {
        //        int temp_infpoints = 2;
        //        if(m_isKinematic) temp_infpoints = 3;
        int temp_ips = global_cf::CSI_ZWDV;
        double temp_product = 0.0;
        q_zwd_cur = P(3,3);
        q_zwd_slope_cur = q_zwd_cur - q_zwd_pre;
        temp_product = q_zwd_slope_cur*q_zwd_slope_pre;
        if(temp_product < 0) infpoints++;
        if(infpoints >= temp_ips) flag_epoch = true;

        q_zwd_pre = q_zwd_cur;
        q_zwd_slope_pre = q_zwd_slope_cur;
    }

    if(true)//epochResultSatlitData.at(0).times_sigam_pos >= 5)
    {
        bool fixed_from_pre_flag = false;
        QString sys_cur = ""; int sys_num_cur = 0;

        int num_fixed_sats = 0;
        bool SSD_fixed_flag = false;
        //The number of satellite systems in the current epoch
        sys_cur = epochResultSatlitData.at(0).SatType;
        sys_num_cur = 1;
        for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
        {
            if(!sys_cur.contains(epochResultSatlitData.at(cf_i).SatType))
                sys_num_cur++, sys_cur = sys_cur + epochResultSatlitData.at(cf_i).SatType;
        }

        int sys_num_pre = 1;   QString sys_pre = "";
        sys_pre = prevEpochSatlitData.at(0).SatType;
        for(int cf_num = 1; cf_num < prevEpochSatlitData.length(); cf_num++)
        {
            if(!sys_pre.contains(prevEpochSatlitData.at(cf_num).SatType))
                sys_num_pre++,sys_pre = sys_pre + prevEpochSatlitData.at(cf_num).SatType;
        }

        //Separate systems, ambiguity search does not interfere with each other
        for(int cf_sys = 0; cf_sys < sys_num_cur; cf_sys++)
        {
            if(sys_cur.at(cf_sys) != "G" && sys_cur.at(cf_sys) != "E") continue;

            int length_curepoch = 0;
            length_curepoch = epochResultSatlitData.at(0).prn_referencesat[2*cf_sys + 1] + 1;
            if(cf_sys > 0)
                length_curepoch = length_curepoch - epochResultSatlitData.at(0).prn_referencesat[2*cf_sys - 1] - 1;
            int sit_ref = epochResultSatlitData.at(0).prn_referencesat[2*cf_sys + 1];

            double f1 = 0.0, f2 = 0.0, lamda_nl = 0.0, beta2 = 0.0, lamda2 = 0.0;
            for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
            {
                if(epochResultSatlitData.at(cf_i).SatType == sys_cur.at(cf_sys))
                {
                    f1 = epochResultSatlitData.at(cf_i).Frq[0];
                    f2 = epochResultSatlitData.at(cf_i).Frq[1];
                    break;
                }
            }
            lamda_nl = M_C/(f1 + f2);
            beta2 = f1*f1/(f2*f2);
            lamda2 = M_C/f2;

            //Get the narrow lane floatingsolution of all satellites (0 : wide lane is not fixed, excluding the reference sats) 2020.12.04 23z
            VectorXd nl_float_all, nl_float_all_flag;
            nl_float_all.resize(length_curepoch);
            nl_float_all.setZero();
            nl_float_all_flag.resize(length_curepoch);
            nl_float_all_flag.setZero();

            int prn_ref_cur = 0, prn_ref_pre = 0, pre_sit = -923;
            if(sys_cur == sys_pre) pre_sit = cf_sys;
            else
            {
                for(int cf_pre = 0; cf_pre < sys_num_pre; cf_pre++)
                {
                    if(sys_pre.at(cf_pre) == sys_cur.at(cf_sys))
                    {
                        pre_sit = cf_pre;
                        break;
                    }
                }
            }

            prn_ref_cur = epochResultSatlitData.at(0).prn_referencesat[2*cf_sys];
            if(pre_sit == -923) prn_ref_pre = -923;
            else prn_ref_pre = prevEpochSatlitData.at(0).prn_referencesat[2*pre_sit];

            int fixed_useless_ref_sats = 0, fixed_sats = 0;
            //Calculate the floating solution of the narrow lane after the wide lane is successfully fixed (0 : the wide lane is not fixed)  2020.12.04 23z
            //-2：.** of nl > 0.25c  -1：ref sats  0：float wl  1：lambda  2：fixed in the preepoch
            for(int cf_i = 0, cfi = 0; cf_i < epochResultSatlitData.length(); cf_i++, cfi++)
            {
                if(epochResultSatlitData.at(cf_i).SatType == sys_cur.at(cf_sys))
                {
                    if(epochResultSatlitData.at(cf_i).wl_fixed_flag)
                    {
                        double nl_f = 0.0, diff = 0.0;
                        int nl_r = 0;
                        if(cf_i < sit_ref)
                        {
                            epochResultSatlitData[cf_i].nl_float_SSD =
                                    (X[cf_i + 4 - cf_sys] + lamda2*epochResultSatlitData.at(cf_i).wl_fixed_SSD/(beta2 - 1))/lamda_nl;
                            nl_f = nl_float_all[cfi] = epochResultSatlitData[cf_i].nl_float_SSD;
                            nl_r = qRound(nl_f);
                            diff = qAbs(nl_f - nl_r);
                            if(diff < 0.15) nl_float_all_flag[cfi] = 1;
                            else nl_float_all_flag[cfi] = -2, fixed_useless_ref_sats++;
                        }
                        else if(cf_i == sit_ref)
                        {
                            fixed_useless_ref_sats++;
                            nl_float_all_flag[cfi] = - 1;
                            epochResultSatlitData[cf_i].nl_fixed_flag = false;
                        }
                        else
                        {
                            epochResultSatlitData[cf_i].nl_float_SSD =
                                    (X[cf_i + 3 - cf_sys] + lamda2*epochResultSatlitData.at(cf_i).wl_fixed_SSD/(beta2 - 1))/lamda_nl;
                            nl_f = nl_float_all[cfi] = epochResultSatlitData[cf_i].nl_float_SSD;
                            nl_r = qRound(nl_f);
                            diff = qAbs(nl_f - nl_r);
                            if(diff < 0.15) nl_float_all_flag[cfi] = 1;
                            else nl_float_all_flag[cfi] = -2, fixed_useless_ref_sats++;
                        }
                    }
                    else
                        fixed_useless_ref_sats++;
                }
                else
                    cfi--;
            }

            //Previous epoch state 2020.12.12 23z
            if(prevEpochSatlitData.at(0).SSD_fixed_flag && global_cf::fix_and_hold && flag_epoch)
            {
                if(prn_ref_cur == prn_ref_pre)
                {//The reference sat is unchanged, and the fixed ambiguity information can be directly obtained
                    int prn = 0;
                    for(int cf_i = 0, cfi = 0; cf_i < epochResultSatlitData.length(); cf_i++, cfi++)
                    {
                        if(epochResultSatlitData.at(cf_i).SatType == sys_cur.at(cf_sys))
                        {
                            prn = epochResultSatlitData.at(cf_i).PRN;
                            if(cf_i != sit_ref)
                            {
                                for(int cf_j = 0; cf_j < prevEpochSatlitData.length(); cf_j++)
                                    if(prevEpochSatlitData.at(cf_j).PRN == prn && prevEpochSatlitData.at(cf_j).nl_fixed_flag
                                            && prevEpochSatlitData.at(cf_j).SatType == sys_cur.at(cf_sys))
                                    {
                                        fixed_from_pre_flag = true;
                                        fixed_sats++, fixed_useless_ref_sats++;
                                        nl_float_all_flag[cfi] = 2;

                                        epochResultSatlitData[cf_i].nl_fixed_SSD = prevEpochSatlitData.at(cf_j).nl_fixed_SSD;
                                        epochResultSatlitData[cf_i].nl_fixed_flag = true;
                                        epochResultSatlitData[cf_i].times_nl = prevEpochSatlitData.at(cf_j).times_nl + 1;
                                        break;
                                    }
                            }

                        }
                        else
                            cfi--;

                    }
                }
                else
                {//The reference sat is unchanged, obtain the ambiguity difference of narrow lane between refsats
                    int diff_nl_refsats = -923;
                    for(int cf_i = 0; cf_i < prevEpochSatlitData.length(); cf_i++)
                    {
                        if(prevEpochSatlitData.at(cf_i).PRN == prn_ref_cur && prevEpochSatlitData.at(cf_i).nl_fixed_flag
                                && prevEpochSatlitData.at(cf_i).SatType == sys_cur.at(cf_sys))
                            diff_nl_refsats = prevEpochSatlitData.at(cf_i).nl_fixed_SSD;
                    }

                    if(diff_nl_refsats != -923)
                    {//Successfully obtain the difference between two reference sats
                        for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
                        {
                            epochResultSatlitData[cf_i].diff_nl_refsats[cf_sys] = diff_nl_refsats;
                            epochResultSatlitData[cf_i].refsat_changed_succeed = true;
                        }

                        int prn = 0;
                        for(int cf_i = 0, cfi = 0; cf_i < epochResultSatlitData.length(); cf_i++, cfi++)
                        {
                            if(epochResultSatlitData.at(cf_i).SatType == sys_cur.at(cf_sys))
                            {
                                prn = epochResultSatlitData.at(cf_i).PRN;
                                if(epochResultSatlitData.at(cf_i).PRN != prn_ref_cur && epochResultSatlitData.at(cf_i).PRN != prn_ref_pre)
                                {//Get information about the previous epoch
                                    for(int cf_j = 0; cf_j < prevEpochSatlitData.length(); cf_j++)
                                        if(prevEpochSatlitData.at(cf_j).PRN == prn && prevEpochSatlitData.at(cf_j).nl_fixed_flag
                                                && prevEpochSatlitData.at(cf_j).SatType == sys_cur.at(cf_sys))
                                        {
                                            fixed_from_pre_flag = true;
                                            fixed_sats++, fixed_useless_ref_sats++;
                                            nl_float_all_flag[cfi] = 2;

                                            epochResultSatlitData[cf_i].nl_fixed_SSD = prevEpochSatlitData.at(cf_j).nl_fixed_SSD - diff_nl_refsats;
                                            epochResultSatlitData[cf_i].nl_fixed_flag = true;
                                            epochResultSatlitData[cf_i].times_nl = prevEpochSatlitData.at(cf_j).times_nl + 1;
                                            break;
                                        }
                                }
                                else if(epochResultSatlitData.at(cf_i).PRN == prn_ref_pre)
                                {
                                    fixed_from_pre_flag = true;
                                    fixed_sats++, fixed_useless_ref_sats++;
                                    nl_float_all_flag[cfi] = 2;

                                    epochResultSatlitData[cf_i].nl_fixed_SSD = - diff_nl_refsats;
                                    epochResultSatlitData[cf_i].nl_fixed_flag = true;
                                    epochResultSatlitData[cf_i].times_nl = 3;
                                }
                                else if(epochResultSatlitData.at(cf_i).PRN == prn_ref_cur)
                                {
                                    epochResultSatlitData[cf_i].nl_fixed_SSD = 0;
                                    epochResultSatlitData[cf_i].nl_fixed_flag = false;
                                    epochResultSatlitData[cf_i].times_nl = 0;
                                }
                                else
                                    epochResultSatlitData[cf_i].nl_fixed_flag = false;
                            }
                            else
                                cfi--;

                        }
                    }
                    //Reinitialize without success to obtain the difference between two reference sats

                }
            }

            for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
            {
                if(epochResultSatlitData.at(cf_i).nl_fixed_SSD != 0)
                {
                    double temp_diff = qAbs(epochResultSatlitData.at(cf_i).nl_fixed_SSD - epochResultSatlitData.at(cf_i).nl_float_SSD);
                    if(temp_diff > 0.5)
                    {
                        epochResultSatlitData[cf_i].nl_fixed_SSD = 0;
                        epochResultSatlitData[cf_i].nl_fixed_flag = false;
                    }
                }
                else
                    epochResultSatlitData[cf_i].nl_fixed_flag = false;
            }

            //When the number of satellites that can resolve the ambiguity of the narrow lane > 1, try to fix them 2020.12.04 23z
            if((length_curepoch - fixed_useless_ref_sats) > 1)
            {
                VectorXd nl_float_part, nl_float_part_flag;
                nl_float_part.resize(length_curepoch - fixed_useless_ref_sats);
                nl_float_part.setZero();
                nl_float_part_flag.resize(length_curepoch - fixed_useless_ref_sats);
                nl_float_part_flag.setZero();

                //Delete the ambiguity of satellites that do not need to be fixed (do not meet the fixed conditions or have been fixed)  23z hahaha 2020.12.02
                for(int cf_i = 0, cf_j = 0; cf_i < (length_curepoch - fixed_useless_ref_sats); cf_i++ , cf_j++)
                {
                    if(nl_float_all_flag[cf_j] == 1)
                    {
                        nl_float_part[cf_i] = nl_float_all[cf_j];
                        nl_float_part_flag[cf_i] = cf_j;
                    }
                    else
                        cf_i--;
                }

                MatrixXd Q_part;
                Q_part.resize((length_curepoch - fixed_useless_ref_sats),(length_curepoch - fixed_useless_ref_sats));
                Q_part.setZero();

                //Find the corresponding covariance
                for(int cf_i = 0, cf_ii = 0; cf_i < (length_curepoch - fixed_useless_ref_sats); cf_i++, cf_ii++)
                {
                    if(nl_float_all_flag[cf_ii] == 1 || cf_ii == sit_ref)
                    {
                        for(int cf_j = 0, cf_jj = 0; cf_j < (length_curepoch - fixed_useless_ref_sats); cf_j++, cf_jj++)
                        {
                            if(nl_float_all_flag[cf_jj] == 1 || cf_jj == sit_ref)
                            {
                                int temp_r = 0, temp_c = 0;
                                if(cf_sys > 0)
                                    temp_r = cf_ii + 4 + epochResultSatlitData.at(0).prn_referencesat[2*cf_sys - 1] - cf_sys,
                                            temp_c = cf_jj + 4 + epochResultSatlitData.at(0).prn_referencesat[2*cf_sys - 1] - cf_sys;
                                else
                                    temp_r = cf_ii + 4, temp_c = cf_jj + 4;
                                Q_part(cf_i,cf_j) = M_Zgama_P_square * P(temp_r,temp_c)/(lamda_nl*lamda_nl);
                            }
                            else
                                cf_j--;
                        }
                    }
                    else
                        cf_i--;
                }

                MatrixXd Q_part_corrected;
                VectorXd nl_float_part_corrected;
                //update 2020.12.14 23z
                fixed_from_pre_flag = false;
                if(fixed_from_pre_flag)
                {
                    VectorXd fixed_part_flag, fixed_part_float;
                    fixed_part_flag.resize(fixed_sats);
                    fixed_part_float.resize(fixed_sats);
                    fixed_part_flag.setZero();
                    fixed_part_float.setZero();
                    for(int cf_i = 0, cf_j = 0; cf_i < fixed_sats; cf_i++ , cf_j++)
                    {
                        if(nl_float_all_flag[cf_j] == 2)
                        {
                            fixed_part_flag[cf_i] = cf_j;
                            fixed_part_float[cf_i] = nl_float_all[cf_j];
                        }
                        else
                            cf_i--;
                    }
                    VectorXd fixed_part_fixed;
                    fixed_part_fixed.resize(fixed_sats);
                    fixed_part_fixed.setZero();
                    for(int cf_i = 0, cf_j = 0; cf_i < fixed_sats; cf_i++, cf_j++)
                    {
                        if(nl_float_all_flag[cf_j] == 2)
                            fixed_part_fixed[cf_i] = epochResultSatlitData.at(cf_j).nl_fixed_SSD;
                        else
                            cf_i--;
                    }
                    MatrixXd Q_part_fixed;
                    Q_part_fixed.resize(fixed_sats,fixed_sats);
                    Q_part_fixed.setZero();
                    for(int cf_i = 0, cf_ii = 0; cf_i < fixed_sats; cf_i++, cf_ii++)
                    {
                        if(nl_float_all_flag[cf_ii] == 2 || cf_ii == sit_ref)
                        {
                            for(int cf_j = 0, cf_jj = 0; cf_j < fixed_sats; cf_j++, cf_jj++)
                            {
                                if(nl_float_all_flag[cf_jj] == 2 || cf_jj == sit_ref)
                                {
                                    Q_part_fixed(cf_i,cf_j) = P(cf_ii + 4,cf_jj + 4)/(lamda_nl*lamda_nl);
                                }
                                else
                                    cf_j--;
                            }
                        }
                        else
                            cf_i--;
                    }
                    MatrixXd Q_right;
                    Q_right.resize(fixed_sats,(length_curepoch - fixed_useless_ref_sats));
                    Q_right.setZero();
                    for(int cf_i = 0; cf_i < fixed_sats; cf_i++)
                        for(int cf_j = 0; cf_j < (length_curepoch - fixed_useless_ref_sats); cf_j++)
                        {
                            int cf_a = fixed_part_flag[cf_i], cf_b = nl_float_part_flag[cf_j];
                            if(cf_a > sit_ref)
                                cf_a = fixed_part_flag[cf_i] - 1;
                            if(cf_b > sit_ref)
                                cf_b = nl_float_part_flag[cf_j] - 1;
                            Q_right(cf_i,cf_j) = P(cf_a + 4,cf_b + 4)/(lamda_nl*lamda_nl);
                        }
                    //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_part_fixed.csv", Q_part_fixed);
                    //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/P.csv", P);
                    //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_right.csv", Q_right);
                    //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_part.csv", Q_part);

                    nl_float_part_corrected.resize(length_curepoch - fixed_useless_ref_sats);
                    nl_float_part_corrected.setZero();
                    nl_float_part_corrected = nl_float_part - (Q_right.transpose())*(Q_part_fixed.inverse())
                            *(fixed_part_float - fixed_part_fixed);

                    Q_part_corrected.resize((length_curepoch - fixed_useless_ref_sats),(length_curepoch - fixed_useless_ref_sats));
                    Q_part_corrected.setZero();
                    Q_part_corrected = Q_part - (Q_right.transpose())*(Q_part_fixed.inverse())*Q_right;

                    //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_part.csv", Q_part);
                    //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_part_corrected.csv", Q_part_corrected);
                    int a = 23;
                }
                else
                {
                    Q_part_corrected = Q_part;
                    nl_float_part_corrected = nl_float_part;
                }

                //                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_part.csv", Q_part);
                //                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/P.csv", P);

                //Z transform Q_part 2020.12.06z
                MatrixXd Q_Z;
                lambda.calculateZ(Q_part_corrected,Q_Z);

                //                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/Q_part_corrected.csv", Q_part_corrected);
                //                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/Q_Z.csv", Q_Z);
                //                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/P.csv", P);

                if(Q_Z != Q_part)
                {
                    //Sort the ambiguities of narrow lanes and save their original position numbers for back assignment 2020.12.04 23z
                    VectorXd nl_float_part_sort, nl_float_part_sort_flag;
                    MatrixXd Q_part_sort;
                    nl_float_part_sort.resize(length_curepoch - fixed_useless_ref_sats);
                    nl_float_part_sort.setZero();
                    nl_float_part_sort_flag.resize(length_curepoch - fixed_useless_ref_sats);
                    nl_float_part_sort_flag.setZero();
                    Q_part_sort.resize((length_curepoch - fixed_useless_ref_sats),(length_curepoch - fixed_useless_ref_sats));
                    Q_part_sort.setZero();

                    //Sort in ascending order based on variance
                    double Q_element_max = Q_Z(0,0);
                    for(int cf_i = 0; cf_i < (length_curepoch - fixed_useless_ref_sats);cf_i++)
                    {
                        if(Q_Z(cf_i,cf_i) > Q_element_max)
                            Q_element_max = Q_Z(cf_i,cf_i);
                    }
                    for(int cf_i = 0; cf_i < (length_curepoch - fixed_useless_ref_sats); cf_i++)
                    {
                        double Q_element = Q_element_max;
                        for(int cf_j = 0; cf_j < (length_curepoch - fixed_useless_ref_sats); cf_j++)
                        {
                            if(cf_i == 0)
                            {//min
                                if(Q_Z(cf_j,cf_j) <= Q_element)
                                {
                                    Q_element = Q_part_sort(cf_i,cf_i) = Q_Z(cf_j,cf_j);
                                    nl_float_part_sort_flag[cf_i] = cf_j;
                                    nl_float_part_sort[cf_i] = nl_float_part_corrected[cf_j];
                                }
                            }
                            else
                            {//after that
                                if(Q_part_sort(cf_i - 1,cf_i - 1) < Q_Z(cf_j,cf_j) && Q_Z(cf_j,cf_j) <= Q_element)
                                {
                                    Q_element = Q_part_sort(cf_i,cf_i) = Q_Z(cf_j,cf_j);
                                    nl_float_part_sort_flag[cf_i] = cf_j;
                                    nl_float_part_sort[cf_i] = nl_float_part_corrected[cf_j];
                                }
                            }
                        }

                    }
                    for(int cf_i = 0; cf_i < (length_curepoch - fixed_useless_ref_sats);cf_i++)
                        for(int cf_j = 0; cf_j< (length_curepoch - fixed_useless_ref_sats);cf_j++)
                            Q_part_sort(cf_i,cf_j) = Q_part_corrected(nl_float_part_sort_flag[cf_i],nl_float_part_sort_flag[cf_j]);


                    //                    m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_part.csv", Q_part);
                    //                    m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_part_sort.csv", Q_part_sort);
                    //                    m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/P.csv", P);

                    VectorXd nl_float_selected;
                    MatrixXd Q_part_selected, nl_fixed_lambda;
                    double ratio_temp = 0.0,ratio_max = 0.0, *ratio = &ratio_temp;
                    int out_fixamb_num = 3;
                    //Fix nl ambs as much as possible 2020.12.04 23z
                    for(int cf_i = (length_curepoch - fixed_useless_ref_sats); cf_i > 1; cf_i--)
                    {
                        nl_float_selected.resize(cf_i);
                        nl_float_selected.setZero();
                        Q_part_selected.resize(cf_i,cf_i);
                        Q_part_selected.setZero();
                        for(int cf_j = 0; cf_j < cf_i; cf_j++)
                        {
                            nl_float_selected[cf_j] = nl_float_part_sort[cf_j];
                            for(int cf_k = 0; cf_k < cf_i; cf_k++)
                                Q_part_selected(cf_j,cf_k) = Q_part_sort(cf_j,cf_k);
                        }
                        //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/nl_float_selected.csv", nl_float_selected);
                        //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_part_selected.csv", Q_part_selected);
                        //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/Q_part_sort.csv", Q_part_sort);

                        lambda.QLamdaSearch(nl_float_selected,Q_part_selected,nl_fixed_lambda,ratio,out_fixamb_num);

                        //ratio_all(epochResultSatlitData.at(0).UTCTime.epochNum,13 - cf_i) = *ratio;

                        if(ratio_temp > ratio_max)
                            ratio_max = ratio_temp;

                        //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/float.csv", nl_float_selected);
                        //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/fixed.csv", nl_fixed_lambda);

                        //When the ratio value > threshold, the initial judgment is that the fixation is successful 2020.12.07 23z
                        if(*ratio > 1.5)
                        {

                            //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/p.csv", P);

                            //Assign a fixed result 2020.12.07 23z
                            for(int cfi = 0; cfi < cf_i; cfi++)
                            {
                                //Get the state of this satellite in the previous epoch, and continue
                                int times = 0, prn = 0, nl_fixed = 0;
                                prn = epochResultSatlitData[nl_float_part_flag[nl_float_part_sort_flag[cfi]]].PRN;
                                for(int cfj = 0; cfj < prevEpochSatlitData.length(); cfj++)
                                {
                                    if(prn == prevEpochSatlitData.at(cfj).PRN
                                            && prevEpochSatlitData.at(cfj).SatType == sys_cur.at(cf_sys))
                                    {
                                        times = prevEpochSatlitData.at(cfj).times_nl;
                                        nl_fixed = prevEpochSatlitData.at(cfj).nl_fixed_SSD;
                                    }

                                }

                                if(nl_fixed == nl_fixed_lambda(cfi,0))
                                {
                                    epochResultSatlitData[nl_float_part_flag[nl_float_part_sort_flag[cfi]]].times_nl
                                            = times + 1;
                                }
                                else
                                {
                                    times = 0;
                                    epochResultSatlitData[nl_float_part_flag[nl_float_part_sort_flag[cfi]]].times_nl
                                            = times + 1;
                                }
                                epochResultSatlitData[nl_float_part_flag[nl_float_part_sort_flag[cfi]]].nl_fixed_SSD
                                        = nl_fixed_lambda(cfi,0);
                            }
                            break;
                        }
                        int B = 23;
                    }

                    //ratio_all(epochResultSatlitData.at(0).UTCTime.epochNum,0) = ratio_max;
                    for(int cfm = 0; cfm < epochResultSatlitData.length(); cfm++)
                        epochResultSatlitData[cfm].ratio = ratio_max;
                }

                int A = 23;
            }

            //When the narrow lane ambiguity is successfully fixed for multiple consecutive epochs and the values are equal, you can start to try to fix the SSD ambiguity 2020.12.02 23z
            for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
            {
                if(epochResultSatlitData.at(cf_i).SatType == sys_cur.at(cf_sys))
                {
                    //When the accuracy of nl amb is high enough, round it 2020.12.29 23z
                    double nl_f = 0.0, diff = 0.0;
                    int nl_r = 0;
                    nl_f = epochResultSatlitData.at(cf_i).nl_float_SSD;
                    nl_r = qRound(nl_f);
                    diff = qAbs(nl_f - nl_r);

                    double flag_dlimt = 0.1, flag_vamblimt = 3e-6, temp_vamb;
                    if(m_isKinematic) flag_dlimt = 0.05, flag_vamblimt = 1e-6;

                    if(cf_i > epochResultSatlitData.at(0).prn_referencesat[2*cf_sys + 1])
                        temp_vamb = P(4 + cf_i - cf_sys - 1, 4 + cf_i - cf_sys - 1);
                    else if(cf_i == epochResultSatlitData.at(0).prn_referencesat[2*cf_sys + 1])
                        temp_vamb = 1;
                    else
                        temp_vamb = P(4 + cf_i - cf_sys, 4 + cf_i - cf_sys);

                    bool flag_vamb = false;
                    if(temp_vamb <= flag_vamblimt) flag_vamb = true;

                    if(diff > 0 && diff < flag_dlimt && epochResultSatlitData.at(cf_i).times_nl < 2 && flag_vamb)
                    {
                        epochResultSatlitData[cf_i].nl_fixed_SSD = nl_r;
                        epochResultSatlitData[cf_i].nl_fixed_flag = true;
                        epochResultSatlitData[cf_i].times_nl = 3;
                    }

                    if(epochResultSatlitData.at(cf_i).times_nl > 2)
                    {
                        double SSD_fixed = 0.0;
                        double SSD_float = 0.0;
                        if(cf_i < sit_ref)
                            SSD_float = X[4 + cf_i - cf_sys];
                        else if(cf_i == sit_ref)
                            SSD_float = 0;
                        else
                            SSD_float = X[3 + cf_i - cf_sys];

                        SSD_fixed = lamda_nl*epochResultSatlitData.at(cf_i).nl_fixed_SSD
                                - lamda2*epochResultSatlitData.at(cf_i).wl_fixed_SSD/(beta2 - 1);

                        //double sigam = 1000.0*epochResultSatlitData.at(0).sigam_pos/1.7;

                        if(qAbs(SSD_fixed - SSD_float) < 0.025) //0.25c * 0.1m            0.05/sqrt3 2020.12.29 23z
                        {
                            epochResultSatlitData[cf_i].SSD_fixed = SSD_fixed;
                            epochResultSatlitData[cf_i].nl_fixed_flag = true;
                            num_fixed_sats++;
                        }
                        else
                        {
                            epochResultSatlitData[cf_i].SSD_fixed = 0.0;
                            epochResultSatlitData[cf_i].nl_fixed_flag = false;
                            epochResultSatlitData[cf_i].times_nl = 0;
                        }

                        int cf =23;
                    }

                }
            }

        }//sys
        //When at least 3 satellites are fixed, it can be considered fixed
        int flag_fixednum = 3;
        if(m_isKinematic) flag_fixednum = 4;
        if(num_fixed_sats >= flag_fixednum)
            SSD_fixed_flag = true;

        if(SSD_fixed_flag)
        {
            for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
                epochResultSatlitData[cf_i].SSD_fixed_flag = true;
        }
        else if(global_cf::fix_and_hold && prevEpochSatlitData.at(0).times_hold > 5)
        {
            for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
                epochResultSatlitData[cf_i].SSD_fixed_flag = true;
        }
    }

    if(flag_epoch && epochResultSatlitData.at(0).SSD_fixed_flag)// && epochResultSatlitData.at(0).times_sigam_pos > 10)
    {
        for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
            epochResultSatlitData[cf_i].AR_succeed = true;
    }
}





int QPPPModel::getinterval(QVector<QVector<SatlitData> > multepochSatlitData)
{
    int interval_cur = 0;
    QVector < int > interval_temp;   QVector < int > count_temp;

    interval_cur = qAbs(multepochSatlitData.at(1).at(0).UTCTime.Seconds
                        - multepochSatlitData.at(0).at(0).UTCTime.Seconds);
    interval_temp.append(interval_cur);   count_temp.append(1);

    int length_mul = multepochSatlitData.length();
    if(length_mul >= 30) length_mul = 30;

    for(int cf_i = 1; cf_i < length_mul - 1; cf_i++)
    {
        interval_cur = qAbs(multepochSatlitData.at(cf_i + 1).at(0).UTCTime.Seconds
                            - multepochSatlitData.at(cf_i).at(0).UTCTime.Seconds);

        bool temp_flag = false;
        for(int cf_j = 0; cf_j < interval_temp.length(); cf_j++)
        {
            if(interval_temp.at(cf_j) == interval_cur)
            {
                count_temp[cf_j]++; temp_flag = true;
            }
        }
        if(!temp_flag)
        {
            interval_temp.append(interval_cur);   count_temp.append(1);
        }
    }

    interval_cur = interval_temp.at(0);
    int count_cur = count_temp.at(0);
    for(int cf_i = 1; cf_i < interval_temp.length(); cf_i++)
    {
        if(count_temp.at(cf_i) > count_cur)
            interval_cur = interval_temp.at(cf_i);
    }

    return interval_cur;
}


