#include "QWrite2File.h"


QWrite2File::QWrite2File(void)
{
}


QWrite2File::~QWrite2File(void)
{
}

bool QWrite2File::writeRecivePosKML(QString fload_path, QString tempfileName)
{
    if (allReciverPos.length() == 0)//If no data is read
        return false;
    if(!isDirExist(fload_path))
    {
        QString infor = "can not construct floder: " + fload_path;
        ErroTrace(infor);
    }
    QString fileName = fload_path + tempfileName;
    QFile saveFile(fileName);//Saved file
    if (!saveFile.open(QFile::WriteOnly|QFile::Text))
    {
        QString erroInfo = "Open " + fileName + " File Error!";
        ErroTrace(erroInfo);
        return false;
    }
    QTextStream saveFileOut(&saveFile);
    // write head information
    saveFileOut << "<\?xml version=\"1.0\" encoding=\"UTF-8\"\?>" << endl;
    saveFileOut << "<kml xmlns=\"http://earth.google.com/kml/2.1\">" << endl;
    saveFileOut << "<Document>" << endl;
    saveFileOut << "<Style id=\"yellowLineGreenPoly\">" << endl;
    saveFileOut << "<LineStyle>" << endl;
    saveFileOut << "<color>ff0400ff</color>" << endl;
    saveFileOut << "<width>1.4</width>" << endl;
    saveFileOut << "</LineStyle>" << endl;
    saveFileOut << "<PolyStyle>" << endl;
    saveFileOut << "<color>7f00ff00</color>" << endl;
    saveFileOut << "</PolyStyle>" << endl;
    saveFileOut << "</Style>" << endl;
    saveFileOut << "<Placemark>" << endl;
    saveFileOut << "<name>" << endl;
    saveFileOut << "      david_kml" << endl;
    saveFileOut << "</name>" << endl;
    saveFileOut << "<description>" << endl;
    saveFileOut << "      this is create by david_xiao.Email:xiaogongwei10@163.com" << endl;
    saveFileOut << "</description>" << endl;
    saveFileOut << "<styleUrl>#yellowLineGreenPoly</styleUrl>" << endl;
    saveFileOut << "<LineString>" << endl;
    saveFileOut << "<coordinates>" << endl;

    int lenRecivePos = allReciverPos.length();
    double temp_PI = 3.1415926535897932385;
    for (int i = 0;i <lenRecivePos;i++ )
    {
        RecivePos oneRecivePos = allReciverPos.at(i);
        if(oneRecivePos.dX == 0) continue;
        double pXYZ[3] = {oneRecivePos.dX, oneRecivePos.dY, oneRecivePos.dZ},
                pBLH[3] = {0};
        m_qcmpClass.XYZ2BLH(pXYZ, pBLH);
        saveFileOut << QString::number( pBLH[1]*180 / temp_PI, 'f', 12 ) << ","
                << QString::number( pBLH[0]*180 / temp_PI, 'f', 12 ) << ","
                << QString::number( pBLH[2], 'f', 12 )
                << endl;
    }
    saveFileOut << "</coordinates>" << endl;
    saveFileOut << "</LineString>" << endl;
    saveFileOut << "</Placemark>" << endl;
    saveFileOut << "</Document>" << endl;
    saveFileOut << "</kml>" << endl;
    saveFileOut.flush();
    saveFile.close();
    return true;
}

////The calculation result is written to txt
bool QWrite2File::writeRecivePos2Txt(QString fload_path, QString tempfileName)
{
    if (allReciverPos.length() == 0)//If no data is read
        return false;

    if(!isDirExist(fload_path))
    {
        QString infor = "can not construct floder: " + fload_path;
        ErroTrace(infor);
    }
    QString fileName = fload_path + tempfileName;
    QFile saveFile(fileName);//Saved file
    if (!saveFile.open(QFile::WriteOnly|QFile::Text))
    {
        QString erroInfo = "Open " + fileName + " File Error!";
        ErroTrace(erroInfo);
        return false;
    }

    QTextStream saveFileOut(&saveFile);

    saveFileOut << "     epoch" << "             Observation time" << "    num_of_sats"
        << "         spp_x(m)" << "         spp_y(m)" << "         spp_z(m)"
        << "         ppp_x(m)" << "         ppp_y(m)" << "         ppp_z(m)"
        << "       sigam_x(m)" << "       sigam_y(m)" << "       sigam_z(m)" << endl;

    int lenRecivePos = allReciverPos.length();

    for (int i = 0;i <lenRecivePos;i++ )
    {
        RecivePos oneRecivePos = allReciverPos.at(i);
        MatrixXd Qmat = allSloverQ.at(i);
        saveFileOut.setFieldWidth(10);
        saveFileOut<<i;
        saveFileOut.setFieldWidth(2);
        saveFileOut<<": ";
        //Output year, month, day, hour, minute, second
        saveFileOut.setFieldWidth(4);
        saveFileOut<<QString::number(oneRecivePos.UTCtime.Year);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<"-";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(oneRecivePos.UTCtime.Month);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<"-";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(oneRecivePos.UTCtime.Day);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<" ";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(oneRecivePos.UTCtime.Hours);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<":";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(oneRecivePos.UTCtime.Minutes);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<":";
        saveFileOut.setFieldWidth(10);
        saveFileOut<<QString::number(oneRecivePos.UTCtime.Seconds,'f',7);

        //Calculate dE dN dU
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(oneRecivePos.totolEpochStalitNum);
        // cout SPP Pos
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(oneRecivePos.spp_pos[0],'f',4);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(oneRecivePos.spp_pos[1],'f',4);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(oneRecivePos.spp_pos[2],'f',4);
        saveFileOut.setFieldWidth(2);

        // cout PPP pos
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(oneRecivePos.dX,'f',4);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(oneRecivePos.dY,'f',4);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(oneRecivePos.dZ,'f',4);
        saveFileOut.setFieldWidth(2);

        // write pos zigama = sqrt(Q(i,i)), i=1,2,3
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(sqrt(abs(Qmat(0,0))),'f',8);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(sqrt(abs(Qmat(1,1))),'f',8);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(15);
        saveFileOut<<QString::number(sqrt(abs(Qmat(2,2))),'f',8);
        saveFileOut.setFieldWidth(2);

        saveFileOut<<endl;
    }
    saveFileOut.flush();
    saveFile.close();
    return true;
}

//Various error corrections are written to the .ppp file
bool QWrite2File::writePPP2Txt(QString fload_path, QString tempfileName)
{

    if(!isDirExist(fload_path))
    {
        QString infor = "can not construct floder: " + fload_path;
        ErroTrace(infor);
    }
    QString fileName = fload_path + tempfileName;

    QFile saveFile(fileName);//Saved file
    if (!saveFile.open(QFile::WriteOnly|QFile::Text))
    {
        QString erroInfo = "Open " + fileName + " File Error!";
        ErroTrace(erroInfo);
        return false;
    }
    QTextStream saveFileOut(&saveFile);

    int all_epoch_ppp_len = allPPPSatlitData.length();
    if (0 == all_epoch_ppp_len) return false;
    for (int i = 0;i < all_epoch_ppp_len;i++)
    {
        QVector < SatlitData > epochSatlite = allPPPSatlitData.at(i);
        if(epochSatlite.length() == 0) continue;
        RecivePos recvPos = allReciverPos.at(i);
        int StallitNumbers = epochSatlite.length();
        //Output the first epoch, and start tag">"
        saveFileOut.setFieldWidth(0);
        saveFileOut << ">epoch_num:";
        saveFileOut.setFieldWidth(5);
        saveFileOut << i;
        saveFileOut << "(";
        saveFileOut.setFieldWidth(3);
        saveFileOut<<QString::number(epochSatlite.at(0).epochlength);
        saveFileOut << ")";
        saveFileOut << endl;
        //Output header information
        saveFileOut<<"Satellite Number:";
        saveFileOut.setFieldWidth(3);
        saveFileOut<<StallitNumbers;
        saveFileOut<<",(yyyy-mm-dd-hh-mm-ss):";
        saveFileOut.setFieldWidth(4);
        saveFileOut<<QString::number(recvPos.UTCtime.Year);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<"-";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(recvPos.UTCtime.Month);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<"-";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(recvPos.UTCtime.Day);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<" ";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(recvPos.UTCtime.Hours);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<":";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(recvPos.UTCtime.Minutes);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<":";
        saveFileOut.setFieldWidth(10);
        saveFileOut<<QString::number(recvPos.UTCtime.Seconds, 'f', 7);
        saveFileOut.setFieldWidth(6);
        saveFileOut<<",ZHD:";
        // write ZTD and Epoch Flag
        SatlitData oneStallit_t = epochSatlite.at(0);
        saveFileOut.setFieldWidth(7);
        saveFileOut<<QString::number(oneStallit_t.UTCTime.TropZHD,'f',4);
        saveFileOut.setFieldWidth(12);
        saveFileOut<<", BadFlag: ";
        saveFileOut.setFieldWidth(3);
        saveFileOut<<QString::number(oneStallit_t.EpochFlag);
        saveFileOut<<", reference_sat: ";
        saveFileOut.setFieldWidth(3);
        saveFileOut<<QString::number(oneStallit_t.prn_referencesat[0]);
        saveFileOut.setFieldWidth(3);
        saveFileOut<<QString::number(oneStallit_t.prn_referencesat[2]);
        saveFileOut<<", diff_wl_refsats: ";
        saveFileOut.setFieldWidth(4);
        saveFileOut<<QString::number(oneStallit_t.diff_wl_refsats[0]);
        saveFileOut.setFieldWidth(4);
        saveFileOut<<QString::number(oneStallit_t.diff_wl_refsats[1]);
        saveFileOut<<", diff_nl_refsats: ";
        saveFileOut.setFieldWidth(4);
        saveFileOut<<QString::number(oneStallit_t.diff_nl_refsats[0]);
        saveFileOut.setFieldWidth(4);
        saveFileOut<<QString::number(oneStallit_t.diff_nl_refsats[1]);
        saveFileOut<<", sigam_pos: ";
        saveFileOut.setFieldWidth(12);
        saveFileOut<<QString::number(oneStallit_t.sigam_pos);
        saveFileOut<<", hold: ";
        saveFileOut.setFieldWidth(5);
        if(oneStallit_t.times_hold > 1)
            saveFileOut<<"true";
        else
            saveFileOut<<"false";
        saveFileOut<<", fixednum: ";
        saveFileOut.setFieldWidth(3);
        saveFileOut<<QString::number(oneStallit_t.num_fixedsats);
        saveFileOut<<", ratio: ";
        saveFileOut.setFieldWidth(3);
        saveFileOut<<QString::number(oneStallit_t.ratio);
        saveFileOut<<endl;

        saveFileOut <<      "prn" << "       v_L_IF(m)" << "       v_P_IF(m)"
            << "    pos_sat_x(m)" << "    pos_sat_y(m)" << "    pos_sat_z(m)"
            << "     wl_float(c)" << "     wl_fixed(c)" << "        times_wl"
            << "     nl_float(c)" << "     nl_fixed(c)" << "        times_nl"
            << "    SSD_float(m)" << "    SSD_fixed(m)"
            << "      clk_sat(m)" << "       elevation" << "         azimuth"
            << "      sattrop(m)" << "   sattropmap(m)" << "    relativty(m)"
            << "       sagnac(m)" << "   tideeffect(m)" << "    antheight(m)"
            << "  satL1offset(c)" << "  satL2offset(c)" << "  recL1offset(c)"
            << "  recL2offset(c)" << "       windup(c)" << endl;

        //Output coordinate information
        for (int j = 0;j < StallitNumbers;j++)
        {
            SatlitData oneStallit= epochSatlite.at(j);
            saveFileOut.setFieldWidth(1);
            saveFileOut<<oneStallit.SatType;
            saveFileOut.setFieldWidth(2);
            if (oneStallit.PRN < 10)
                saveFileOut<<"0"+QString::number(oneStallit.PRN);
            else
                saveFileOut<<oneStallit.PRN;

            saveFileOut.setFieldWidth(2);
            saveFileOut<<": ";

            if(getPPPModel() == PPP_MODEL::PPP_Combination)
            {
                // LL3
                saveFileOut.setFieldWidth(14);
                saveFileOut<<QString::number(oneStallit.VLL3,'f',8);
                saveFileOut.setFieldWidth(2);
                saveFileOut<<", ";
                // PP3
                saveFileOut.setFieldWidth(14);
                saveFileOut<<QString::number(oneStallit.VPP3,'f',8);
            }
            else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
            {
                // L1 L2
                saveFileOut.setFieldWidth(14);
                saveFileOut<<QString::number(oneStallit.VL1,'f',8);
                saveFileOut.setFieldWidth(2);
                saveFileOut<<", ";
                saveFileOut.setFieldWidth(14);
                saveFileOut<<QString::number(oneStallit.VL2,'f',8);
                saveFileOut.setFieldWidth(2);
                saveFileOut<<", ";
                // C1 C2
                saveFileOut.setFieldWidth(14);
                saveFileOut<<QString::number(oneStallit.VC1,'f',8);
                saveFileOut.setFieldWidth(2);
                saveFileOut<<", ";
                saveFileOut.setFieldWidth(14);
                saveFileOut<<QString::number(oneStallit.VC2,'f',8);
            }

            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.X,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.Y,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.Z,'f',4);

            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.wl_float_SSD,'f',5);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.wl_fixed_SSD,'f',5);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.times_wl,'f',5);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.nl_float_SSD,'f',5);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.nl_fixed_SSD,'f',5);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.times_nl,'f',5);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.amb_SSD_sats,'f',5);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.SSD_fixed,'f',5);

            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.StaClock,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.EA[0],'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.EA[1],'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.SatTrop,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.StaTropMap,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.Relativty,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.Sagnac,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.TideEffect,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.AntHeight,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.SatL1Offset,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.SatL2Offset,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.L1Offset,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.L2Offset,'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<", ";
            saveFileOut.setFieldWidth(14);
            saveFileOut<<QString::number(oneStallit.AntWindup,'f',4);
            //QString::number(oneStallit.pos[2],'f',4);

            saveFileOut<<endl;//Output coordinates of a satellite in the epoch
        }
    }
    saveFileOut.flush();
    saveFile.close();
    return true;
}

//Write the zenith wet delay and clock difference to txt, the first column wet delay second column clock difference
bool QWrite2File::writeClockZTDW2Txt(QString fload_path, QString tempfileName)
{
    if (allClock.length() == 0)//If no data is read
        return false;

    if(!isDirExist(fload_path))
    {
        QString infor = "can not construct floder: " + fload_path;
        ErroTrace(infor);
    }
    QString fileName = fload_path + tempfileName;

    QFile saveFile(fileName);//Saved file
    if (!saveFile.open(QFile::WriteOnly|QFile::Text))
    {
        QString erroInfo = "Open " + fileName + " File Error!";
        ErroTrace(erroInfo);
        return false;
    }

    QTextStream saveFileOut(&saveFile);
    // write head information
//    QString head_string[10] = {"epoch", "Observation time", "ZTD(m)", "diff_clk_G(m)",
//                             "diff_clk_C(m)", "diff_clk_R(m)", "diff_clk_E(m)"};
//    QString sys_str = getSatlitSys(), base_sys = "G";
//    if(!sys_str.isEmpty()) base_sys = sys_str.at(0);
//    switch (base_sys[0].toLatin1()) {
//    case 'G':
//        head_string[3] = "base_clk_G(m)";
//        break;
//    case 'C':
//        head_string[4] = "base_clk_C(m)";
//        break;
//    case 'R':
//        head_string[5] = "base_clk_R(m)";
//        break;
//    case 'E':
//        head_string[6] = "base_clk_E(m)";
//        break;
//    default:
//        break;
//    }
//    for(int i = 0;i < 7;i++)
//    {
//        saveFileOut.setFieldWidth(12);
//        if(i == 1) saveFileOut.setFieldWidth(26);
//        saveFileOut << head_string[i]+"|";
//    }
//    saveFileOut << endl;
      saveFileOut << "     epoch" << "         Observation time" << "         ZTD(m)" << "    clk_G(m)"
                << "    clk_C(m)" << "    clk_R(m)" << "    clk_E(m)" << endl;


    int lenClock = allClock.length();

    for (int i = 0;i <lenClock;i++ )
    {
        ClockData epochZTDWClock = allClock.at(i);
        saveFileOut.setFieldWidth(10);
        saveFileOut<<i;
        saveFileOut.setFieldWidth(2);
        saveFileOut<<": ";
        //Output year, month, day, hour, minute, second
        saveFileOut.setFieldWidth(4);
        saveFileOut<<QString::number(epochZTDWClock.UTCTime.Year);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<"-";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(epochZTDWClock.UTCTime.Month);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<"-";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(epochZTDWClock.UTCTime.Day);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<" ";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(epochZTDWClock.UTCTime.Hours);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<":";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(epochZTDWClock.UTCTime.Minutes);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<":";
        saveFileOut.setFieldWidth(10);
        saveFileOut<<QString::number(epochZTDWClock.UTCTime.Seconds,'f',7);
        //Output X system receiver clock error, Xi system relative X system receiver clock offset
        saveFileOut.setFieldWidth(10);
        saveFileOut<<QString::number(epochZTDWClock.ZTD_W,'f',4);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        for(int k = 0; k < 4;k++)
        {
            saveFileOut.setFieldWidth(10);
            saveFileOut<<QString::number(epochZTDWClock.clockData[k],'f',4);
            saveFileOut.setFieldWidth(2);
            saveFileOut<<"  ";
        }
        saveFileOut<<endl;
    }
    saveFileOut.flush();
    saveFile.close();

    return true;
}

//All satellites write txt files named "G32.txt", "C02.txt", "R08.txt", etc., the first column of ambiguity
bool QWrite2File::writeAmbiguity2Txt(QString fload_path)
{
    if(!isDirExist(fload_path))
    {
        QString infor = "can not construct floder: " + fload_path;
        ErroTrace(infor);
    }

    //Find the name of the satellite that observes all systems
    int lenSatAbm = allAmbiguity.length();
    QVector< char > store_SatType;
    QVector< int > store_SatPRN;
    for (int i = 0;i < lenSatAbm;i++)
    {
        Ambiguity oneSatAbm = allAmbiguity.at(i);
        char tSatType = '0';
        int tSatPRN = -1;
        bool isFind = false;
        for (int j = 0;j < store_SatPRN.length();j++)
        {
            tSatPRN = store_SatPRN.at(j);
            tSatType = store_SatType.at(j);
            if (tSatPRN == oneSatAbm.PRN && tSatType == oneSatAbm.SatType)
            {
                isFind = true;
                break;
            }
        }
        if (!isFind)
        {
            store_SatType.append(oneSatAbm.SatType);
            store_SatPRN.append(oneSatAbm.PRN);
        }
    }
    //Write all satellite names to the file
    for (int i = 0;i < store_SatPRN.length();i++)
    {
        char tSatType = '0';
        int tSatPRN = -1;
        tSatType = store_SatType.at(i);
        tSatPRN = store_SatPRN.at(i);
        WriteAmbPRN(fload_path, tSatPRN,tSatType);
    }
    return true;
}

//The ambiguity of writing to the PRN satellite
bool QWrite2File::WriteAmbPRN(QString temp_floder,int PRN,char SatType)
{
    if (allAmbiguity.length() == 0)//If no data is read
        return false;
    QString fileName,strPRN;
    strPRN = QString::number(PRN);
    if(PRN < 10)
        strPRN = QString::number(0) + strPRN;
    fileName = QString(SatType) + strPRN + QString(".txt");
    fileName = temp_floder + fileName; //Path of Current exec file
    QFile saveFile(fileName);//Saved file
    if (!saveFile.open(QFile::WriteOnly|QFile::Text))
    {
        QString erroInfo = "Open " + fileName + " File Error!";
        ErroTrace(erroInfo);
        return false;
    }

    QTextStream saveFileOut(&saveFile);
    saveFileOut << "epochNumber | GPS Week second | fields | Ion_free amb (cycle) | Uncombined amb1 (cycle) | Uncombined amb2 (cycle) | L1 ion (m) | Elevation angle (degree) | Azimuth angle (degree)|";
    saveFileOut << endl;
    int lenAbm = allAmbiguity.length();
    for (int i = 0;i <lenAbm;i++ )
    {
        Ambiguity oneSatAmb = allAmbiguity.at(i);
        if (oneSatAmb.PRN != PRN || oneSatAmb.SatType != SatType)
        {
            continue;
        }
        saveFileOut.setFieldWidth(10);
        saveFileOut<<(oneSatAmb.UTCTime.epochNum);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<": ";
        //Output GPS week seconds
        saveFileOut.setFieldWidth(16);
        double GPSSecond = m_qcmpClass.YMD2GPSTime(oneSatAmb.UTCTime.Year,oneSatAmb.UTCTime.Month,oneSatAmb.UTCTime.Day,oneSatAmb.UTCTime.Hours,oneSatAmb.UTCTime.Minutes,oneSatAmb.UTCTime.Seconds);
        saveFileOut<<QString::number(GPSSecond,'f',6);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";

        //Write ambiguity
        saveFileOut.setFieldWidth(4);
        saveFileOut<<QString::number(6);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";
        saveFileOut.setFieldWidth(16);
        saveFileOut<<QString::number(oneSatAmb.Amb,'f',4);
        saveFileOut<<QString::number(oneSatAmb.Amb1,'f',4);
        saveFileOut<<QString::number(oneSatAmb.Amb2,'f',4);
        saveFileOut<<QString::number(oneSatAmb.ionL1,'f',4);
        saveFileOut<<QString::number(oneSatAmb.EA[0],'f',4);
        saveFileOut<<QString::number(oneSatAmb.EA[1],'f',4);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";

        saveFileOut<<endl;
    }
    saveFileOut.flush();
    saveFile.close();

    return true;
}


//Write the satellite stored in the variable allAmbiguity to the file
bool QWrite2File::WriteEpochPRN(QString fload_path, QString tempfileName)
{
    if (allAmbiguity.length() == 0)//If no data is read
        return false;
    if(!isDirExist(fload_path))
    {
        QString infor = "can not construct floder: " + fload_path;
        ErroTrace(infor);
    }
    QString fileName = fload_path + tempfileName;

    QFile saveFile(fileName);//Saved file
    if (!saveFile.open(QFile::WriteOnly|QFile::Text))
    {
        QString erroInfo = "Open " + fileName + " File Error!";
        ErroTrace(erroInfo);
        return false;
    }
    QTextStream saveFileOut(&saveFile);
    //Write data only
    int lengAllSat = allAmbiguity.length();
    QString tQstrPRN = "";
    for (int i = 0; i < lengAllSat;i++)
    {
        Ambiguity oneSatAmb = allAmbiguity.at(i);
        //Write epoch
        saveFileOut.setFieldWidth(10);
        saveFileOut<<(oneSatAmb.UTCTime.epochNum);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<": ";
        //Write satellite type and number, such as G24, C01
        saveFileOut.setFieldWidth(1);
        saveFileOut<<QString(oneSatAmb.SatType);
        saveFileOut.setFieldWidth(2);
        if (oneSatAmb.PRN < 10)
            tQstrPRN = QString::number(0) + QString::number(oneSatAmb.PRN);
        else
            tQstrPRN = QString::number(oneSatAmb.PRN);
        saveFileOut<<tQstrPRN;
        saveFileOut<<endl;
    }
    saveFileOut.flush();
    saveFile.close();

    return true;
}

bool QWrite2File::writeBadSatliteData(QString fload_path, QString tempfileName)
{
    if (allBadSatlitData.length() == 0)//If no data is read
        return false;
    if(!isDirExist(fload_path))
    {
        QString infor = "can not construct floder: " + fload_path;
        ErroTrace(infor);
    }
    QString fileName = fload_path + tempfileName;

    QFile saveFile(fileName);//Saved file
    if (!saveFile.open(QFile::WriteOnly|QFile::Text))
    {
        QString erroInfo = "Open " + fileName + " File Error!";
        ErroTrace(erroInfo);
        return false;
    }
    QTextStream saveFileOut(&saveFile);
    //Write data only
    int lengAllSat = allBadSatlitData.length();
    QString tQstrPRN = "";
    for (int i = 0; i < lengAllSat;i++)
    {
        SatlitData oneSatAmb = allBadSatlitData.at(i);
        //Write epoch
        saveFileOut.setFieldAlignment(QTextStream::AlignRight);
        saveFileOut.setFieldWidth(10);
        saveFileOut<<(oneSatAmb.UTCTime.epochNum);
        saveFileOut.setFieldWidth(2);
        saveFileOut<<": ";
        //Write satellite type and number, such as G24, C01
        saveFileOut.setFieldWidth(1);
        saveFileOut<<QString(oneSatAmb.SatType);
        saveFileOut.setFieldWidth(2);
        if (oneSatAmb.PRN < 10)
            tQstrPRN = QString::number(0) + QString::number(oneSatAmb.PRN);
        else
            tQstrPRN = QString::number(oneSatAmb.PRN);
        saveFileOut<<tQstrPRN;
        saveFileOut.setFieldWidth(2);
        saveFileOut<<"  ";

        //Output year, month, day, hour, minute, second
        saveFileOut.setFieldWidth(4);
        saveFileOut<<QString::number(oneSatAmb.UTCTime.Year);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<"-";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(oneSatAmb.UTCTime.Month);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<"-";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(oneSatAmb.UTCTime.Day);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<" ";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(oneSatAmb.UTCTime.Hours);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<":";
        saveFileOut.setFieldWidth(2);
        saveFileOut<<QString::number(oneSatAmb.UTCTime.Minutes);
        saveFileOut.setFieldWidth(1);
        saveFileOut<<":";
        saveFileOut.setFieldWidth(10);
        saveFileOut<<QString::number(oneSatAmb.UTCTime.Seconds,'f',7);

        saveFileOut.setFieldWidth(18);
        saveFileOut<<"Eliminate info: ";
        for(int n = 0;n < oneSatAmb.badMsg.length();n++)
        {
            saveFileOut.setFieldAlignment(QTextStream::AlignLeft);
            saveFileOut.setFieldWidth(132);
            saveFileOut<<oneSatAmb.badMsg.at(n);
            saveFileOut.setFieldWidth(3);
            saveFileOut<< " | ";
        }
        saveFileOut.setFieldWidth(1);
        saveFileOut<<endl;
    }
    saveFileOut.flush();
    saveFile.close();
    return true;
}

bool QWrite2File::isDirExist(QString fullPath)
{
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
