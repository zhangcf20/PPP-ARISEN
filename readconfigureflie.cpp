#include"readconfigurefile.h"

bool global_cf::ini_csv = false;
bool global_cf::keepornot = false;
QString global_cf::filepath_ini = "";
QString global_cf::filepath_keep = "";
double global_cf::randomwalk_pos = 0.0;
double global_cf::randomwalk_zwd = 0.0;
bool global_cf::whitenoise_pos = false;
bool global_cf::fix_and_hold = false;
int global_cf::reinitialize = 999999;
bool global_cf::ini_snx = false;

bool global_cf::isGPT3 = false;

QString global_cf::Elevation_weight_function = "";
QString global_cf::Trop_map = "";

bool global_cf::flag_sampling = false;
double global_cf::Interval_original = 0.0;
double global_cf::Interval_used = 0.0;

bool global_cf::flag_seismology = false;
double global_cf::After_OT = 0.0;
double global_cf::Before_OT = 0.0;
double global_cf::OTh = 0.0;
double global_cf::OTm = 0.0;
double global_cf::OTs = 0.0;

int global_cf::CSI_ZWDV = 2;
double global_cf::Threshold_ofile_size = 80;

double global_cf::weight_G = 1;
double global_cf::weight_R = 1;
double global_cf::weight_C = 1;
double global_cf::weight_E = 1;

//2020.10.22 by23z
Configure ReadConfigureFile(QString txtPath)//23Z 2020.10.16
{
    Configure temp_conf;
    int location[23];//23


    QFile file(txtPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        QByteArray temp = file.readAll();
        QString str(temp);
        int n=temp.length();


        for(int i=0,k=0;i<n;i++)
        {
            if(temp.at(i)=='\n')
                location[k]=i,k++;
            if(k==22)
                break;
        }

        int loca2=location[2];
        temp_conf.FilePath=str.mid(0+9,location[0]-9);
        temp_conf.path_keep=str.mid(location[20]+11,location[21]-location[20]-11);
        temp_conf.path_initialize=str.mid(location[21]+17,location[22]-location[21]-17);

        //location[0]=str.mid(location[1]-1,1).toInt();
        location[1]=str.mid(location[1]-1,1).toInt();
        location[2]=str.mid(location[2]-1,1).toInt();
        //location[3]=str.mid(loca2+10,location[3]-(loca2+10)).toInt();
        location[4]=str.mid(location[4]-1,1).toInt();
        location[5]=str.mid(location[5]-1,1).toInt();
        location[6]=str.mid(location[6]-1,1).toInt();
        location[7]=str.mid(location[7]-4,4).toInt();
        location[8]=str.mid(location[8]-1,1).toInt();
        location[9]=str.mid(location[9]-1,1).toInt();
        location[10]=str.mid(location[10]-1,1).toInt();
        location[11]=str.mid(location[11]-1,1).toInt();
        location[12]=str.mid(location[12]-1,1).toInt();
        location[13]=str.mid(location[13]-1,1).toInt();
        location[14]=str.mid(location[14]-1,1).toInt();
        //location[15]=str.mid(location[14]-1,1).toInt();
        //location[16]=str.mid(location[16]-1,1).toInt();
        location[17]=str.mid(location[17]-1,1).toInt();
        location[18]=str.mid(location[18]-1,1).toInt();
        location[19]=str.mid(location[19]-6,6).toInt();
        location[20]=str.mid(location[20]-1,1).toInt();
        //location[21]=str.mid(location[21]-1,1).toInt();
        //location[22]=str.mid(location[22]-1,1).toInt();

        temp_conf.CutAngle=str.mid(loca2+10,location[3]-(loca2+10)).toDouble();
        temp_conf.OperationStrategy=location[12];
        temp_conf.randomwalk_zwd = str.mid(location[15]-6,6).toDouble();
        temp_conf.randomwalk_pos = str.mid(location[16]-6,6).toDouble();

        //根据a判断配置是否正确，a=0说明配置出错
        temp_conf.Cflag=location[1]*location[2]*location[4]*location[5]*location[6]*location[7]*location[8]
                *location[9]*location[10]*location[11]*location[12]*location[13]*location[14]
                *location[17]*location[18]*location[19]*location[20];
        if(temp_conf.FilePath.isEmpty())
        {
            temp_conf.Cflag=0;
            return temp_conf;
        }

        file.close();
    }

    //编号解读
    //电离层模型
    if(location[1]==1) temp_conf.TropDelay="UNB3m";
    else if(location[1]==2) temp_conf.TropDelay="Sasstam(GPT2)";
    else if(location[1]==3) temp_conf.TropDelay="Hopfield(GPT2)";
    else if(location[1]==4) global_cf::isGPT3 = true, temp_conf.TropDelay="GPT3";
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //滤波模型
    if(location[2]==1) temp_conf.Method="Kalman";
    else if(location[2]==2) temp_conf.Method="SRIF";
    else if(location[2]==3) temp_conf.Method="KalmanOU";
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //PPP组合方式
    if(location[4]==1) temp_conf.PPPModel="IF";
    else if(location[4]==2) temp_conf.PPPModel="SSD";
    else if(location[4]==3) temp_conf.PPPModel="AR";
    else if(location[4]==4) temp_conf.PPPModel="Uncombined";
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //是否是动态处理
    if(location[5]==1) temp_conf.Kinematic=false;
    else if(location[5]==2) temp_conf.Kinematic=true;
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //产品类型
    if(location[6]==1) temp_conf.Products="igs";
    else if(location[6]==2) temp_conf.Products="cnt";
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //卫星类型设置
    QString temp_sys = QString::number(location[7]);
    if(temp_sys.contains('1')) temp_conf.SatelliteSys=temp_conf.SatelliteSys + "G";
    if(temp_sys.contains('4')) temp_conf.SatelliteSys=temp_conf.SatelliteSys + "E";
    if(temp_sys.contains('3')) temp_conf.SatelliteSys=temp_conf.SatelliteSys + "R";
    if(temp_sys.contains('2')) temp_conf.SatelliteSys=temp_conf.SatelliteSys + "C";
    if(temp_conf.SatelliteSys == "")
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //PPP是否平滑
    if(location[8]==1) temp_conf.PPPSmooth="NoSmooth";
    else if(location[8]==2) temp_conf.PPPSmooth="Smooth";
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //PPP是否反向平滑
    if(location[9] == 1) temp_conf.PPPBack = false;
    else if(location[9] == 2) temp_conf.PPPBack = true;
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //SPP模型
    if(location[10]==1) temp_conf.SPPModel="P_IF";
    else if(location[10]==2) temp_conf.SPPModel="PL_IF";
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //SPP是否平滑
    if(location[11]==1) temp_conf.SPPSmooth="NoSmooth";
    else if(location[11]==2) temp_conf.SPPSmooth="Smooth";
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //判断处理选项的编号是否合法
    if(location[12] !=1 && location[12] !=2 && location[12] !=3 && location[12] !=4
            && location[12] !=5 && location[12] !=6 && location[12] !=7)
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //是否保存卡尔曼滤波最后一历元的状态参数
    if(location[13]==1) temp_conf.keepornot=false;
    else if(location[13]==2) temp_conf.keepornot=true;
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //是否自定义初始化卡尔曼滤波参数
    if(location[14]==1) temp_conf.initializeornot=false;
    else if(location[14]==2) temp_conf.initializeornot=true;
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //保存参数位置
    if(temp_conf.keepornot && temp_conf.path_keep.isEmpty())
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //读取参数的位置
    if(temp_conf.initializeornot && temp_conf.path_initialize.isEmpty())
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //白噪声模型
    if(location[17]==1) temp_conf.whitenoise_pos=false;
    else if(location[17]==2) temp_conf.whitenoise_pos=true;
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    //是否hold
    if(location[18]==1) global_cf::fix_and_hold = false;
    else if(location[18]==2) global_cf::fix_and_hold = true;
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    if(location[20]==1) global_cf::ini_snx = false;
    else if(location[20]==2) global_cf::ini_snx = true;
    else
    {
        temp_conf.Cflag=0;
        return temp_conf;
    }
    global_cf::ini_csv = temp_conf.initializeornot;
    global_cf::keepornot = temp_conf.keepornot;
    global_cf::filepath_ini = temp_conf.path_initialize;
    global_cf::filepath_keep = temp_conf.path_keep;
    global_cf::randomwalk_pos = temp_conf.randomwalk_pos;
    global_cf::randomwalk_zwd = temp_conf.randomwalk_zwd;
    global_cf::reinitialize = location[19];

    if(temp_conf.Kinematic) global_cf::whitenoise_pos = temp_conf.whitenoise_pos;

    return temp_conf;


}

Configure getconfigurefromini()
{
    Configure temp_conf;
    ConfTranIni temp_configure("ARISEN.ini");

    temp_conf.FilePath = temp_configure.getValue("/ARISEN/Rinexdir");
    temp_conf.TropDelay = temp_configure.getValue("/ARISEN/Trop_model");
    temp_conf.Method = "Kalman";
    temp_conf.PPPModel = temp_configure.getValue("/ARISEN/Strategy");
    temp_conf.Products = "igs";
    temp_conf.SatelliteSys = temp_configure.getValue("/ARISEN/Satellites_system");
    temp_conf.PPPSmooth = "NoSmooth";
    temp_conf.CutAngle = temp_configure.getValue("/ARISEN/Cut_angle").toDouble();

    temp_conf.path_product = temp_configure.getValue("/ARISEN/Productdir");
    temp_conf.path_result = temp_configure.getValue("/ARISEN/Resultdir");

    QString Elevation_weight_function = temp_configure.getValue("/ARISEN/Elevation_weight_function"),
            Initialize = temp_configure.getValue("ARISEN/Initialize"),
            Path_ini = temp_configure.getValue("ARISEN/Path_ini"),
            Path_save = temp_configure.getValue("ARISEN/Path_save"),
            Trop_map = temp_configure.getValue("ARISEN/Trop_map");
    double Interval_original = temp_configure.getValue("ARISEN/Interval_original").toDouble(),
           Interval_used = temp_configure.getValue("ARISEN/Interval_used").toDouble(),
           After_OT = temp_configure.getValue("ARISEN/After_OT").toDouble(),
           Before_OT = temp_configure.getValue("ARISEN/Before_OT").toDouble(),
           OTh = temp_configure.getValue("ARISEN/OTh").toDouble(),
           OTm = temp_configure.getValue("ARISEN/OTm").toDouble(),
           OTs = temp_configure.getValue("ARISEN/OTs").toDouble();

    global_cf::Threshold_ofile_size = temp_configure.getValue("ARISEN/Threshold_for_o_file_size").toDouble();

    QString WGNSS = temp_configure.getValue("/ARISEN/Weight_GNSS");
    QStringList WGNSS_StrList =  WGNSS.split(";");
    WGNSS_StrList.removeAll(QString(""));
    if(WGNSS_StrList.length() == 4)
    {
        global_cf::weight_G = WGNSS_StrList.at(0).toDouble();
        global_cf::weight_R = WGNSS_StrList.at(1).toDouble();
        global_cf::weight_C = WGNSS_StrList.at(2).toDouble();
        global_cf::weight_E = WGNSS_StrList.at(3).toDouble();
    }

    if(temp_configure.getValue("ARISEN/Strategy").contains("AR"))
        global_cf::CSI_ZWDV = temp_configure.getValue("ARISEN/CSI_ZWDV").toInt();

    if(Initialize.contains("CSV")) global_cf::ini_csv = true,global_cf::filepath_ini = Path_ini;
    else global_cf::ini_csv = false;

    if(Initialize.contains("SNX")) global_cf::ini_snx = true,global_cf::filepath_ini = Path_ini;
    else global_cf::ini_snx = false;

    if(Path_save != "") global_cf::keepornot = true,global_cf::filepath_keep = Path_save;
    else global_cf::keepornot = false;

    if(temp_conf.TropDelay.contains("GPT3")) global_cf::isGPT3 = true;
    else global_cf::isGPT3 = false;

    global_cf::Elevation_weight_function = Elevation_weight_function;
    global_cf::Trop_map = Trop_map;
    global_cf::Interval_original = Interval_original;
    global_cf::Interval_used = Interval_used;
    if(temp_configure.getValue("/ARISEN/PPP_model") == "Seismology")
        global_cf::OTh = OTh, global_cf::OTm = OTm, global_cf::OTs = OTs, global_cf::After_OT = After_OT, global_cf::Before_OT = Before_OT;
    global_cf::randomwalk_zwd = 3;

    if(temp_configure.getValue("/ARISEN/PPP_model") != "Static")
        temp_conf.Kinematic = true;
    else
        temp_conf.Kinematic = false;

    if(temp_configure.getValue("/ARISEN/Filter_direction") != "Forward")
        temp_conf.PPPBack = true;
    else
        temp_conf.PPPBack = false;

    if(temp_conf.Kinematic)
    {
        if(temp_configure.getValue("/ARISEN/Kin_model") == "Whitenoise")
            global_cf::whitenoise_pos = true;
        else
            global_cf::randomwalk_pos = temp_configure.getValue("/ARISEN/Randomwalk").toDouble();
    }

    if(temp_configure.getValue("/ARISEN/Batch") != "true")
    {
        if(temp_conf.PPPModel == "IF" || temp_conf.PPPModel.contains("Uncom")) temp_conf.OperationStrategy = 1;
        if(temp_conf.PPPModel == "SSD(IF)") temp_conf.OperationStrategy = 2;
        if(temp_conf.PPPModel.contains("AR")) temp_conf.OperationStrategy = 3;
    }
    else
    {
        if(temp_conf.PPPModel == "IF"|| temp_conf.PPPModel.contains("Uncom")) temp_conf.OperationStrategy = 5;
        if(temp_conf.PPPModel == "SSD(IF)") temp_conf.OperationStrategy = 6;
        if(temp_conf.PPPModel.contains("AR")) temp_conf.OperationStrategy = 7;
    }

    if(temp_configure.getValue("ARISEN/Strategy").contains("ARF")) global_cf::fix_and_hold = true;
    else global_cf::fix_and_hold = false;

    if(temp_configure.getValue("ARISEN/PPP_model").contains("Seis"))
    {
        global_cf::flag_seismology = true;
        global_cf::OTh = temp_configure.getValue("ARISEN/OTh").toDouble();
        global_cf::OTm = temp_configure.getValue("ARISEN/OTm").toDouble();
        global_cf::OTs = temp_configure.getValue("ARISEN/OTs").toDouble();

        global_cf::Before_OT = temp_configure.getValue("ARISEN/Before_OT").toDouble();
        global_cf::After_OT = temp_configure.getValue("ARISEN/After_OT").toDouble();

    }
    else
        global_cf::flag_seismology = false;

    if(temp_configure.getValue("/ARISEN/Sampling") == "true")
    {
        global_cf::flag_sampling = true;
        global_cf::Interval_original = temp_configure.getValue("ARISEN/Interval_original").toDouble();
        global_cf::Interval_used = temp_configure.getValue("ARISEN/Interval_used").toDouble();
    }
    else
        global_cf::flag_sampling = false;


    return temp_conf;
}

//PPP 2020.10.22 by23z
void RunPPP(Configure temp_conf, QTextEdit *pQTextEdit)
{
    QString rin_path = temp_conf.FilePath,
            pro_path = temp_conf.path_product,
            res_path = temp_conf.path_result,
            TropDelay = temp_conf.TropDelay,
            Method = temp_conf.Method,
            PPPModel_Str = temp_conf.PPPModel;
    double CutAngle = temp_conf.CutAngle;
    QString SatSystem =temp_conf.SatelliteSys;
    bool Kinematic = temp_conf.Kinematic;
    QString Smooth_Str = temp_conf.PPPSmooth;
    QString m_Products = temp_conf.Products;

    ConfTranIni temp_con("ARISEN.ini");
    QString removeSats = temp_con.getValue("/ARISEN/DeleteSats");
    removeSats.append(";C01;C02;C03;C04;C05");// remove GEO of BeiDou

    QVector<QStringList> ObsTypeSet = getConfObsType();

    QString Qw_Str = temp_con.getValue("/ARISEN/Qw"),
            Pk_Str = temp_con.getValue("/ARISEN/Pk"),
            LP_Str = temp_con.getValue("/ARISEN/LP_precision");
    QStringList Qw_StrList =  Qw_Str.split(";"), Pk_StrList = Pk_Str.split(";"),
            LP_List = LP_Str.split(";");
    QVector<QStringList> Qw_Pk;
    Qw_Pk.append(Qw_StrList); Qw_Pk.append(Pk_StrList); Qw_Pk.append(LP_List);

    QStringList rpr_path;
    rpr_path.append(rin_path); rpr_path.append(pro_path); rpr_path.append(res_path);

//PPP
    if(temp_conf.PPPBack)
    {
         QPPPBackSmooth myBkPPP(rpr_path, pQTextEdit, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, m_Products, PPPModel_Str);
         myBkPPP.Run(true);
         bool RunorNot = myBkPPP.isRuned();
         PlotGUIData m_singledata;
         if(RunorNot) myBkPPP.getRunResult(m_singledata);

    }
    else
    {       
        QPPPModel myPPP(rpr_path, pQTextEdit, Method, SatSystem, TropDelay, CutAngle, Kinematic,
                        Smooth_Str, m_Products, PPPModel_Str, removeSats, ObsTypeSet, Qw_Pk);
        myPPP.Run(true);
        bool RunorNot = myPPP.isRuned();
        PlotGUIData m_singledata;
        if(RunorNot) myPPP.getRunResult(m_singledata);

        //int a=0;2020.10.22by23z

    }


}



//PPP batch 2020.10.22 by23z
void PPPRunBatch(Configure temp_conf, QTextEdit *pQTextEdit)
{
    QString rin_path = temp_conf.FilePath,
            pro_path = temp_conf.path_product,
            res_path = temp_conf.path_result,
            TropDelay = temp_conf.TropDelay,
            Method = temp_conf.Method,
            PPPModel_Str = temp_conf.PPPModel;
    double CutAngle = temp_conf.CutAngle;
    QString SatSystem =temp_conf.SatelliteSys;
    bool Kinematic = temp_conf.Kinematic;
    QString Smooth_Str = temp_conf.PPPSmooth;
    QString m_Products = temp_conf.Products;
    bool isBackBatch=temp_conf.PPPBack;

    QStringList rpr_path;
    rpr_path.append(rin_path); rpr_path.append(pro_path); rpr_path.append(res_path);

//PPPbatch
    QBatchProcess batchPPP(rpr_path, pQTextEdit, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, isBackBatch, m_Products, PPPModel_Str);
    batchPPP.Run(false);
    bool RunorNot = batchPPP.isRuned();
    if(RunorNot)
    {
        QVector< PlotGUIData > m_mutiplydata;
        QStringList m_mutiplynames;
        batchPPP.getStoreAllData(m_mutiplydata);
        m_mutiplynames = batchPPP.getStationNames();
    }




}


//SPP  2020.10.22 by23z
void RunSPP(Configure temp_conf, QTextEdit *pQTextEdit)
{

    QString m_station_path = temp_conf.FilePath,
            TropDelay = temp_conf.TropDelay,
            Method = temp_conf.Method,
            PPPModel_Str = temp_conf.PPPModel;
    double CutAngle = temp_conf.CutAngle;
    QString SatSystem =temp_conf.SatelliteSys;
    bool Kinematic = temp_conf.Kinematic;
    QString Smooth_Str = temp_conf.SPPSmooth;
    QString SPP_Model = temp_conf.SPPModel;

//SPP
    QSPPModel mySPP(m_station_path, pQTextEdit, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, SPP_Model, PPPModel_Str);
    mySPP.Run(true);
    bool RunorNot = mySPP.isRuned();
    PlotGUIData m_singledata;
    if(RunorNot) mySPP.getRunResult(m_singledata);

}



//SSDPPP 2020.11.05 by23z
void RunSSDPPP(Configure temp_conf, QTextEdit *pQTextEdit)
{
    QString rin_path = temp_conf.FilePath,
            pro_path = temp_conf.path_product,
            res_path = temp_conf.path_result,
            TropDelay = temp_conf.TropDelay,
            Method = temp_conf.Method,
            PPPModel_Str = temp_conf.PPPModel;
    double CutAngle = temp_conf.CutAngle;
    QString SatSystem =temp_conf.SatelliteSys;
    bool Kinematic = temp_conf.Kinematic;
    QString Smooth_Str = temp_conf.PPPSmooth;
    QString m_Products = temp_conf.Products;

    //bool pppback = temp_conf.PPPBack;

    ConfTranIni temp_con("ARISEN.ini");
    QString removeSats = temp_con.getValue("/ARISEN/DeleteSats");
    removeSats.append(";C01;C02;C03;C04;C05");// remove GEO of BeiDou

    QVector<QStringList> ObsTypeSet = getConfObsType();

    QString Qw_Str = temp_con.getValue("/ARISEN/Qw"),
            Pk_Str = temp_con.getValue("/ARISEN/Pk"),
            LP_Str = temp_con.getValue("/ARISEN/LP_precision");
    QStringList Qw_StrList =  Qw_Str.split(";"), Pk_StrList = Pk_Str.split(";"),
            LP_List = LP_Str.split(";");
    QVector<QStringList> Qw_Pk;
    Qw_Pk.append(Qw_StrList); Qw_Pk.append(Pk_StrList); Qw_Pk.append(LP_List);

    QStringList rpr_path;
    rpr_path.append(rin_path); rpr_path.append(pro_path); rpr_path.append(res_path);

    if(temp_conf.PPPBack)
    {
        QPPPBackSmooth myBkPPP(rpr_path, pQTextEdit, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, m_Products, PPPModel_Str);
        myBkPPP.SSDPPP(true);
        bool RunorNot = myBkPPP.isRuned();
        PlotGUIData m_singledata;
        if(RunorNot) myBkPPP.getRunResult(m_singledata);

    }
    else
    {
        QPPPModel myPPP(rpr_path, pQTextEdit, Method, SatSystem, TropDelay, CutAngle, Kinematic,
                        Smooth_Str, m_Products, PPPModel_Str, removeSats, ObsTypeSet, Qw_Pk);
        myPPP.runSSDPPP(true);
        bool RunorNot = myPPP.isRuned();
        PlotGUIData m_singledata;
        if(RunorNot) myPPP.getRunResult(m_singledata);
    }



}


//PPP-AR 2020.11.30 by23z
void RunPPPAR(Configure temp_conf, QTextEdit *pQTextEdit)
{
    QString rin_path = temp_conf.FilePath,
            pro_path = temp_conf.path_product,
            res_path = temp_conf.path_result,
            TropDelay = temp_conf.TropDelay,
            Method = temp_conf.Method,
            PPPModel_Str = temp_conf.PPPModel;
    double CutAngle = temp_conf.CutAngle;
    QString SatSystem =temp_conf.SatelliteSys;
    bool Kinematic = temp_conf.Kinematic;
    QString Smooth_Str = temp_conf.PPPSmooth;
    QString m_Products = temp_conf.Products;

    ConfTranIni temp_con("ARISEN.ini");
    QString removeSats = temp_con.getValue("/ARISEN/DeleteSats");
    removeSats.append(";C01;C02;C03;C04;C05");// remove GEO of BeiDou

    QVector<QStringList> ObsTypeSet = getConfObsType();

    QString Qw_Str = temp_con.getValue("/ARISEN/Qw"),
            Pk_Str = temp_con.getValue("/ARISEN/Pk"),
            LP_Str = temp_con.getValue("/ARISEN/LP_precision");
    QStringList Qw_StrList =  Qw_Str.split(";"), Pk_StrList = Pk_Str.split(";"),
            LP_List = LP_Str.split(";");
    QVector<QStringList> Qw_Pk;
    Qw_Pk.append(Qw_StrList); Qw_Pk.append(Pk_StrList); Qw_Pk.append(LP_List);

    QStringList rpr_path;
    rpr_path.append(rin_path); rpr_path.append(pro_path); rpr_path.append(res_path);

    if(temp_conf.PPPBack)
    {
        QPPPBackSmooth myBkPPP(rpr_path, pQTextEdit, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, m_Products, PPPModel_Str);
        myBkPPP.PPPAR(true);
        bool RunorNot = myBkPPP.isRuned();
        PlotGUIData m_singledata;
        if(RunorNot) myBkPPP.getRunResult(m_singledata);

    }
    else
    {
        QPPPModel myPPP(rpr_path, pQTextEdit, Method, SatSystem, TropDelay, CutAngle, Kinematic,
                        Smooth_Str, m_Products, PPPModel_Str, removeSats, ObsTypeSet, Qw_Pk);
        myPPP.runPPPAR(true);
        bool RunorNot = myPPP.isRuned();
        PlotGUIData m_singledata;
        if(RunorNot) myPPP.getRunResult(m_singledata);
    }



}


//batch 2020.12.26 by23z
void SSDorAR_batch(Configure temp_conf, QTextEdit *pQTextEdit)
{
    QString rin_path = temp_conf.FilePath,
            pro_path = temp_conf.path_product,
            res_path = temp_conf.path_result,
            TropDelay = temp_conf.TropDelay,
            Method = temp_conf.Method,
            PPPModel_Str = temp_conf.PPPModel;
    double CutAngle = temp_conf.CutAngle;
    QString SatSystem =temp_conf.SatelliteSys;
    bool Kinematic = temp_conf.Kinematic;
    QString Smooth_Str = temp_conf.PPPSmooth;
    QString m_Products = temp_conf.Products;
    bool isBackBatch=temp_conf.PPPBack;

    int OperationStrategy = temp_conf.OperationStrategy;

    QStringList rpr_path;
    rpr_path.append(rin_path); rpr_path.append(pro_path); rpr_path.append(res_path);

    QBatchProcess batch(rpr_path, pQTextEdit, Method, SatSystem, TropDelay, CutAngle, Kinematic, Smooth_Str, isBackBatch, m_Products, PPPModel_Str);
    batch.run_SSD_or_AR(OperationStrategy,isBackBatch);
    bool RunorNot = batch.isRuned();
    if(RunorNot)
    {
        QVector< PlotGUIData > m_mutiplydata;
        QStringList m_mutiplynames;
        batch.getStoreAllData(m_mutiplydata);
        m_mutiplynames = batch.getStationNames();
    }

}





QVector<QStringList> getConfObsType()
{
    ConfTranIni temp_con("ARISEN.ini");
    QVector<QStringList> tempConfObs;
    QString SatOBStype;
    QStringList Sat_List;
    // GPS
    SatOBStype = temp_con.getValue("/ARISEN/GPS_OBS_TYPE");
    Sat_List= SatOBStype.split(";");
    Sat_List.prepend("G");
    Sat_List.removeAll(QString(""));
    if(Sat_List.length() >= 2)  tempConfObs.append(Sat_List);
    //GLONASS
    SatOBStype = temp_con.getValue("/ARISEN/GLONASS_OBS_TYPE");
    Sat_List= SatOBStype.split(";");
    Sat_List.prepend("R");
    Sat_List.removeAll(QString(""));
    if(Sat_List.length() >= 2)  tempConfObs.append(Sat_List);
    //BDS
    SatOBStype = temp_con.getValue("/ARISEN/BDS_OBS_TYPE");
    Sat_List= SatOBStype.split(";");
    Sat_List.prepend("C");
    Sat_List.removeAll(QString(""));
    if(Sat_List.length() >= 2)  tempConfObs.append(Sat_List);
    //Galileo
    SatOBStype = temp_con.getValue("/ARISEN/Galileo_OBS_TYPE");
    Sat_List= SatOBStype.split(";");
    Sat_List.prepend("E");
    Sat_List.removeAll(QString(""));
    if(Sat_List.length() >= 2)  tempConfObs.append(Sat_List);

    return tempConfObs;
}
