#include "ReadBIA.h"

void QReadBIA::readDCBfile(QString DCBfilepath)
{
    QFile file(DCBfilepath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString temp;
        while (!file.atEnd())
        {
            //line
            temp = file.readLine();

            QString prn = "";
            prn = temp.mid(0,3);

            if(prn.contains("G"))
            {
                QString prn = " ";
                double DCB = 0;
                prn = temp.mid(0,3);
                DCB = temp.mid(25,12).toDouble();
                DCB_GPS.prn.append(prn);
                DCB_GPS.DCB.append(DCB);
            }

        }

        file.close();
    }

}





void QReadBIA::getDCBdata(SatlitData &tempSatlitData)
{//GPS
    if(tempSatlitData.SatType == 'G')
    {
        int prn = 0;
        prn = tempSatlitData.PRN;
        QString prn_num = " ", prn_string = " ";
        prn_num = QString::number(prn);
        if(prn < 10)
            prn_string = "G0" + prn_num;
        else
            prn_string = "G" + prn_num;
        double DCB = 0.0;

        for(int cf_i = 0; cf_i < DCB_GPS.prn.length(); cf_i++)
        {
            if(DCB_GPS.prn.at(cf_i).contains(prn_string))
            {
                DCB = DCB_GPS.DCB.at(cf_i);
                tempSatlitData.DCB_P1C1 = DCB;
                break;
            }

        }
    }

    int a = 23;
}




void QReadBIA::readGBMBIAfile(QString gbmbiafilepath)
{

    QFile file(gbmbiafilepath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString temp;
        GBMBIA_line temp_gbmbia;

        while (!file.atEnd())
        {
            //line
            temp = file.readLine();

            if(temp.contains("PARAMETER_SPACING"))
            {
                int interval = 0;
                interval = temp.mid(50,5).toInt();
                gbmbia.interval = interval;
                continue;
            }

            //GPS obs_type
            if(temp.contains("DESCRIPTION        GPS"))
            {
                QString obs_type = "";
                obs_type = temp.mid(28,3);
                gbmbia.GPSobs_type.append(obs_type);
                continue;
            }

            //GLO obs_type
            if(temp.contains("DESCRIPTION        GLONASS"))
            {
                QString obs_type = "";
                obs_type = temp.mid(28,3);
                gbmbia.GLONASSobs_type.append(obs_type);
                continue;
            }


            //GAL obs_type
            if(temp.contains("DESCRIPTION        GALILEO"))
            {
                QString obs_type = "";
                obs_type = temp.mid(28,3);
                gbmbia.GALILEOobs_type.append(obs_type);
                continue;
            }

            //COM obs_type
            if(temp.contains("DESCRIPTION        BEIDOU"))
            {
                QString obs_type = "";
                obs_type = temp.mid(28,3);
                gbmbia.COMPASSobs_type.append(obs_type);
                continue;
            }

            if(temp.contains(" OSB       G"))
            {
                QString prn = "", typeofobs = "";
                int Syear = 0, Sdoy = 0;
                double Ssod = 0.0, bia = 0.0;

                prn = temp.mid(11,3);
                typeofobs = temp.mid(25,3);
                Syear = temp.mid(35,4).toInt();
                Sdoy = temp.mid(40,3).toInt();
                Ssod = temp.mid(44,5).toDouble();
                bia = temp.mid(81,10).toDouble();

                temp_gbmbia.prn = prn;
                temp_gbmbia.obs_type = typeofobs;
                temp_gbmbia.Syear = Syear;
                temp_gbmbia.Sdoy = Sdoy;
                temp_gbmbia.Ssod = Ssod;
                temp_gbmbia.bia_ns = bia;

                gbmbia.GPSSYS.append(temp_gbmbia);
            }

            if(temp.contains(" OSB       R"))
            {
                QString prn = "", typeofobs = "";
                int Syear = 0, Sdoy = 0;
                double Ssod = 0.0, bia = 0.0;

                prn = temp.mid(11,3);
                typeofobs = temp.mid(25,3);
                Syear = temp.mid(35,4).toInt();
                Sdoy = temp.mid(40,3).toInt();
                Ssod = temp.mid(44,5).toDouble();
                bia = temp.mid(81,10).toDouble();

                temp_gbmbia.prn = prn;
                temp_gbmbia.obs_type = typeofobs;
                temp_gbmbia.Syear = Syear;
                temp_gbmbia.Sdoy = Sdoy;
                temp_gbmbia.Ssod = Ssod;
                temp_gbmbia.bia_ns = bia;

                gbmbia.GLONASSSYS.append(temp_gbmbia);


            }

            if(temp.contains(" OSB       E"))
            {
                QString prn = "", typeofobs = "";
                int Syear = 0, Sdoy = 0;
                double Ssod = 0.0, bia = 0.0;

                prn = temp.mid(11,3);
                typeofobs = temp.mid(25,3);
                Syear = temp.mid(35,4).toInt();
                Sdoy = temp.mid(40,3).toInt();
                Ssod = temp.mid(44,5).toDouble();
                bia = temp.mid(81,10).toDouble();

                temp_gbmbia.prn = prn;
                temp_gbmbia.obs_type = typeofobs;
                temp_gbmbia.Syear = Syear;
                temp_gbmbia.Sdoy = Sdoy;
                temp_gbmbia.Ssod = Ssod;
                temp_gbmbia.bia_ns = bia;

                gbmbia.GALILEOSYS.append(temp_gbmbia);


            }

            if(temp.contains(" OSB       C"))
            {
                QString prn = "", typeofobs = "";
                int Syear = 0, Sdoy = 0;
                double Ssod = 0.0, bia = 0.0;

                prn = temp.mid(11,3);
                typeofobs = temp.mid(25,3);
                Syear = temp.mid(35,4).toInt();
                Sdoy = temp.mid(40,3).toInt();
                Ssod = temp.mid(44,5).toDouble();
                bia = temp.mid(81,10).toDouble();

                temp_gbmbia.prn = prn;
                temp_gbmbia.obs_type = typeofobs;
                temp_gbmbia.Syear = Syear;
                temp_gbmbia.Sdoy = Sdoy;
                temp_gbmbia.Ssod = Ssod;
                temp_gbmbia.bia_ns = bia;

                gbmbia.COMPASSSYS.append(temp_gbmbia);


            }

        }

        file.close();
    }


}




void QReadBIA::getgbmbiadata(SatlitData &tempSatlitData)
{

    //2020.11.03 23z

    QString typeofobs_p1 = "", typeofobs_p2 = "", typeofobs_l1 = "", typeofobs_l2 = "";
    typeofobs_p1 = tempSatlitData.wantObserType.at(0);
    typeofobs_l1 = tempSatlitData.wantObserType.at(1);
    typeofobs_p2 = tempSatlitData.wantObserType.at(2);
    typeofobs_l2 = tempSatlitData.wantObserType.at(3);

    //epoch time
    int year = 0, month = 0, day = 0, hour = 0, minute = 0;
    double second = 0.0;
    year = tempSatlitData.UTCTime.Year;
    month = tempSatlitData.UTCTime.Month - 1;
    day = tempSatlitData.UTCTime.Day;
    hour = tempSatlitData.UTCTime.Hours;
    minute = tempSatlitData.UTCTime.Minutes;
    second = tempSatlitData.UTCTime.Seconds;
    //day of year   how many days of this month
    int doy= 0, dom = 0;
    double Ssecond = 0.0;
    if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
        dom = 31;
    else if(month == 2)
    {
        int a = 0, b = 0;
        a = year%4; b = year%100;
        if(a == 0 && b != 0)
            dom = 29;
        else
            dom = 28;
    }
    else
        dom = 30;

    doy = month*dom + day;
    Ssecond = hour*3600 + minute*60 + second;

    //sat info
    QString typeofsat = "", prnofsat = "";
    int prn = 0;
    typeofsat = tempSatlitData.SatType;
    prn = tempSatlitData.PRN;

    if(prn < 10)
        prnofsat = "G0" + QString::number(prn);
    else
        prnofsat = QString::number(prn);

    //match
    double biaofp1 = 0.0, biaofp2 = 0.0, biaofl1 = 0.0, biaofl2 = 0.0;
    int lenofGPSbia = 0, lenofGLObia = 0, lenofGALbia = 0, lenofCAMbia = 0;
    lenofGPSbia = gbmbia.GPSSYS.length();
    lenofCAMbia = gbmbia.COMPASSSYS.length();
    lenofGLObia = gbmbia.GLONASSSYS.length();
    lenofGALbia = gbmbia.GALILEOSYS.length();
    if(typeofsat.contains("G"))
    {
        for(int j = 0;j < lenofGPSbia;j++)
        {
            QString prninbiafile = "";
            prninbiafile = gbmbia.GPSSYS.at(j).prn;
            //prn
            if(prninbiafile.contains(prnofsat))
            {
                //time
                if(gbmbia.GPSSYS.at(j).Syear == year && gbmbia.GPSSYS.at(j).Sdoy == doy)
                {
                    double sod1 = 0.0, sod2 = 0.0;
                    int interval = 0;
                    sod1 = gbmbia.GPSSYS.at(j).Ssod;
                    interval = gbmbia.interval;
                    sod2 = sod1 + interval;
                    if(sod1 <= Ssecond && Ssecond < sod2)
                    {
                        //obs
                        if(gbmbia.GPSSYS.at(j).obs_type == typeofobs_p1)
                            biaofp1 = gbmbia.GPSSYS.at(j).bia_ns;
                        if(gbmbia.GPSSYS.at(j).obs_type == typeofobs_p2)
                            biaofp2 = gbmbia.GPSSYS.at(j).bia_ns;
                        if(gbmbia.GPSSYS.at(j).obs_type == typeofobs_l1)
                            biaofl1 = gbmbia.GPSSYS.at(j).bia_ns;
                        if(gbmbia.GPSSYS.at(j).obs_type == typeofobs_l2)
                            biaofl2 = gbmbia.GPSSYS.at(j).bia_ns;

                        double flag = 0.0;
                        flag = biaofp1*biaofp2*biaofl1*biaofl2;
                        if(flag != 0) break;


                    }
                }
            }

        }
    }

    if(typeofsat.contains("C"))
    {

    }

    if(typeofsat.contains("R"))
    {

    }

    if(typeofsat.contains("E"))
    {

    }

    tempSatlitData.biaofobs[0] = biaofp1;
    tempSatlitData.biaofobs[1] = biaofl1;
    tempSatlitData.biaofobs[2] = biaofp2;
    tempSatlitData.biaofobs[3] = biaofl2;


}



//grg read
void QReadBIA::readgrgwlbiafile()
{
    QString GPS_WSB = "Wide_lane_GPS_satellite_biais.wsb";
    QString GAL_WSB = "Wide_lane_GAL_satellite_biais.wsb";

    QFile file_gps(GPS_WSB);
    if (file_gps.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        while (!file_gps.atEnd())
        {
            //line
            QString temp = file_gps.readLine();
            grg_wl_line temp_grg_wl_one;
            grg_wl temp_grg_wl_line;

            //GPS wl
            if(!temp.contains("#"))
            {
                int year = 0, month = 0, day = 0, prn = 0;
                double bia_wl =0.0;

                year = temp.mid(1,4).toInt();
                month = temp.mid(6,2).toInt();
                day = temp.mid(9,2).toInt();

                for(int cf_i = 1; cf_i < 41; cf_i++)
                {
                    prn = cf_i;
                    bia_wl = temp.mid(16 + (cf_i - 1)*7,7).toDouble();

                    temp_grg_wl_one.prn = prn;
                    temp_grg_wl_one.bia_ns = bia_wl;
                    temp_grg_wl_line.year_wl = year;
                    temp_grg_wl_line.month_wl = month;
                    temp_grg_wl_line.day_wl = day;
                    temp_grg_wl_line.wl.append(temp_grg_wl_one);
                }
                wl_gps.append(temp_grg_wl_line);
            }


        }

        file_gps.close();
    }

    QFile file_gal(GAL_WSB);
    if (file_gal.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!file_gal.atEnd())
        {
            //line
            QString temp = file_gal.readLine();

            grg_wl_line temp_grg_wl_one;
            grg_wl temp_grg_wl_line;
            //GAL wl
            if(!temp.contains("#"))
            {
                int year = 0, month = 0, day = 0, prn = 0;
                double bia_wl =0.0;

                year = temp.mid(1,4).toInt();
                month = temp.mid(6,2).toInt();
                day = temp.mid(9,2).toInt();

                for(int cf_i = 1; cf_i < 41; cf_i++)
                {
                    prn = cf_i;
                    bia_wl = temp.mid(16 + (cf_i - 1)*7,7).toDouble();

                    temp_grg_wl_one.prn = prn;
                    temp_grg_wl_one.bia_ns = bia_wl;
                    temp_grg_wl_line.year_wl = year;
                    temp_grg_wl_line.month_wl = month;
                    temp_grg_wl_line.day_wl = day;
                    temp_grg_wl_line.wl.append(temp_grg_wl_one);
                }
                wl_gal.append(temp_grg_wl_line);
            }


        }

        file_gal.close();
    }


}



//grg match
void QReadBIA::getgrgwlbiadata(SatlitData &tempSatlitData)
{
    //wl
    int year_cur = 0, month_cur = 0, day_cur = 0, prn = 0, doy_cf_cur = 0;
    year_cur = tempSatlitData.UTCTime.Year;
    month_cur = tempSatlitData.UTCTime.Month;
    day_cur = tempSatlitData.UTCTime.Day;
    prn = tempSatlitData.PRN;
    //doy
    doy_cf_cur = (month_cur - 1)*30 + day_cur;
    if(tempSatlitData.SatType == 'G')
    {
        int doy_cf = 0;
        for(int cf_i = 0; cf_i < wl_gps.length(); cf_i++)
        {
            if(wl_gps.at(cf_i).year_wl == year_cur)
            {
                doy_cf = (wl_gps.at(cf_i).month_wl - 1)*30 + wl_gps.at(cf_i).day_wl;
                if(doy_cf_cur <= doy_cf)
                {
                    for(int cf_j = 0; cf_j < wl_gps.at(cf_i).wl.length(); cf_j++)
                    {
                        if(prn == wl_gps.at(cf_i).wl.at(cf_j).prn)
                        {
                            tempSatlitData.grg_bia_wl = wl_gps.at(cf_i).wl.at(cf_j).bia_ns;
                            break;
                        }
                    }
                }
            }
            if(wl_gps.at(cf_i).year_wl > year_cur)
            {
                for(int cf_j = 0; cf_j < wl_gps.at(cf_i).wl.length(); cf_j++)
                {
                    if(prn == wl_gps.at(cf_i).wl.at(cf_j).prn)
                    {
                        tempSatlitData.grg_bia_wl = wl_gps.at(cf_i).wl.at(cf_j).bia_ns;
                        break;
                    }
                }
            }
            if(tempSatlitData.grg_bia_wl != 0)
                break;
        }
    }
    if(tempSatlitData.SatType == 'E')
    {
        int doy_cf = 0;
        for(int cf_i = 0; cf_i < wl_gal.length(); cf_i++)
        {
            if(wl_gal.at(cf_i).year_wl == year_cur)
            {
                doy_cf = (wl_gal.at(cf_i).month_wl - 1)*30 + wl_gal.at(cf_i).day_wl;
                if(doy_cf_cur <= doy_cf)
                {
                    for(int cf_j = 0; cf_j < wl_gal.at(cf_i).wl.length(); cf_j++)
                    {
                        if(prn == wl_gal.at(cf_i).wl.at(cf_j).prn)
                        {
                            tempSatlitData.grg_bia_wl = wl_gal.at(cf_i).wl.at(cf_j).bia_ns;
                            break;
                        }
                    }
                }
            }
            if(wl_gal.at(cf_i).year_wl > year_cur)
            {
                for(int cf_j = 0; cf_j < wl_gal.at(cf_i).wl.length(); cf_j++)
                {
                    if(prn == wl_gal.at(cf_i).wl.at(cf_j).prn)
                    {
                        tempSatlitData.grg_bia_wl = wl_gal.at(cf_i).wl.at(cf_j).bia_ns;
                        break;
                    }
                }
            }
            if(tempSatlitData.grg_bia_wl != 0)
                break;
        }
    }

    int a = 0;

}

//PRN_SVN 2021.01.07 23z
void QReadBIA::readPRN_GPS()
{
    QFile file("PRN_GPS");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString temp;
        PRN_SVN_single ps;

        while (!file.atEnd())
        {
            //line
            temp = file.readLine();
            int length = temp.length();

            if(!temp.contains("Launch"))
            {
                ps.launch_year = temp.mid(2,4).toInt();
                ps.launch_month = temp.mid(7,2).toInt();
                ps.launch_day = temp.mid(10,2).toInt();

                ps.deactiv_year = temp.mid(18,4).toInt();
                ps.deactiv_month = temp.mid(23,2).toInt();
                ps.deactiv_day = temp.mid(26,2).toInt();

                ps.GPS = temp.mid(33,2).toInt();
                ps.PRN = temp.mid(40,2).toInt();

                ps.block = temp.mid(47,7);
                ps.orbit = temp.mid(55,5);
                ps.clock = temp.mid((length - 7),6);

                prn_svn.append(ps);
            }

        }

        int a = 23;

        file.close();
    }

    int length_svn = prn_svn.length();
    for(int cf_i = 0; cf_i < length_svn; cf_i++)
    {
        if(prn_svn.at(cf_i).deactiv_year == 0)
        {
            prn_svn[cf_i].deactiv_year = 9999;
            prn_svn[cf_i].deactiv_month = 99;
            prn_svn[cf_i].deactiv_day = 99;
        }
    }

    int a = 23;
}

//2021.01.07 23z
void QReadBIA::readshadfile(QString shadfilepath)
{
    QFile file(shadfilepath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString temp;
        shadow temp_shad;

        while (!file.atEnd())
        {
            //line
            temp = file.readLine();

            temp_shad.GPS = temp.mid(3,2).toInt();
            temp_shad.gpst = temp.mid(36,17).toDouble();
            temp_shad.year = temp.mid(55,4).toInt();
            temp_shad.month = temp.mid(60,2).toInt();
            temp_shad.day = temp.mid(63,2).toInt();
            temp_shad.hour = temp.mid(66,2).toInt();
            temp_shad.min = temp.mid(69,2).toInt();
            temp_shad.sec = temp.mid(72,9).toDouble();
            if(temp.contains("Earth Clear To Earth Penumbra"))      temp_shad.flag = "c2p";
            else if(temp.contains("Earth Penumbra To Earth Umbra")) temp_shad.flag = "p2u";
            else if(temp.contains("Earth Umbra To Earth Penumbra")) temp_shad.flag = "u2p";
            else if(temp.contains("Earth Penumbra To Earth Clear")) temp_shad.flag = "p2c";
            else if(temp.contains("Moon Clear To Moon Penumbra")) temp_shad.flag = "c2p_m";
            else if(temp.contains("Moon Penumbra To Moon Umbra")) temp_shad.flag = "p2u_m";
            else if(temp.contains("Moon Umbra To Moon Penumbra")) temp_shad.flag = "u2p_m";
            else if(temp.contains("Moon Penumbra To Moon Clear")) temp_shad.flag = "p2c_m";
            else temp_shad.flag = "false";

            shad.append(temp_shad);
        }

        int a = 23;

        file.close();
    }

    //PRN
    int length_shad = shad.length();
    int length_svn = prn_svn.length();

    for(int cf_i = 0; cf_i < length_shad; cf_i++)
    {
        int GPS, year, month, day, apr;
        GPS = shad.at(cf_i).GPS;
        year = shad.at(cf_i).year;
        month = shad.at(cf_i).month;
        day = shad.at(cf_i).day;
        //time
        apr = (year - 1970)*360 + (month - 1)*30 + day;

        for(int cf_j = 0; cf_j < length_svn; cf_j++)
        {
            if(prn_svn.at(cf_j).GPS == GPS)
            {
                int launch_year, launch_month, launch_day, launch_apr;
                launch_year = prn_svn.at(cf_j).launch_year;
                launch_month = prn_svn.at(cf_j).launch_month;
                launch_day = prn_svn.at(cf_j).launch_day;
                launch_apr = (launch_year - 1970)*360 + (launch_month - 1)*30 + launch_day;

                int dea_year, dea_month, dea_day, dea_apr;
                dea_year = prn_svn.at(cf_j).deactiv_year;
                dea_month = prn_svn.at(cf_j).deactiv_month;
                dea_day = prn_svn.at(cf_j).deactiv_day;
                dea_apr = (dea_year - 1970)*360 + (dea_month - 1)*30 + dea_day;

                if(apr >= launch_apr && apr <= dea_apr)
                {
                    shad[cf_i].PRN = prn_svn.at(cf_j).PRN;
                    break;
                }
            }
        }
    }

    int a = 23;

}

//2021.01.07 23z
void QReadBIA::shadcorrcect(QVector< SatlitData > epochSatlitData)
{
    int prn = 0, length_epoch = 0, length_shad = 0;
    length_epoch = epochSatlitData.length();
    length_shad = shad.length();

    VectorXd flag;                VectorXd flag_del;
    flag.resize(length_shad);     flag_del.resize(length_epoch);
    flag.setZero();               flag_del.setZero();
    for(int cf_i = 0; cf_i < length_shad; cf_i++)
        if(shad.at(cf_i).flag.contains("p2c"))
            flag[cf_i] = 1;

    for(int cf_i = 0; cf_i < length_epoch; cf_i++)
    {
        if(epochSatlitData.at(cf_i).SatType == 'G')
        {
            prn = epochSatlitData.at(cf_i).PRN;
            int year, month, day, hour, minute, second, apr;
            year = epochSatlitData.at(cf_i).UTCTime.Year;      month = epochSatlitData.at(cf_i).UTCTime.Month;
            day = epochSatlitData.at(cf_i).UTCTime.Day;        hour = epochSatlitData.at(cf_i).UTCTime.Hours;
            minute = epochSatlitData.at(cf_i).UTCTime.Minutes; second = epochSatlitData.at(cf_i).UTCTime.Seconds;
            apr = ((year - 1970)*360 + (month - 1)*30 + day)*24*3600 + hour*3600 + minute*60 + second;

            for(int cf_j = 0; cf_j < length_shad; cf_j++)
            {
                if(shad.at(cf_j).PRN == prn && flag[cf_j] != 1)
                {
                    int begin_year, begin_month, begin_day, begin_hour, begin_min;
                    double begin_sec, begin_apr;
                    begin_year = shad.at(cf_j).year;   begin_month = shad.at(cf_j).month;
                    begin_day = shad.at(cf_j).day;     begin_hour = shad.at(cf_j).hour;
                    begin_min = shad.at(cf_j).min;     begin_sec = shad.at(cf_j).sec;
                    begin_apr = ((begin_year - 1970)*360 + (begin_month - 1)*30 + begin_day)*24*3600
                            + begin_hour*3600 + begin_min*60 + begin_sec;

                    //flag of end
                    int end = 0;
                    for(int cf_k = cf_j; cf_k < (length_shad - cf_j); cf_k++)
                        if(flag[cf_k] == 1)
                        {
                            end = cf_k;
                            break;
                        }

                    int end_year, end_month, end_day, end_hour, end_min;
                    double end_sec, end_apr;
                    if(end != 0)
                    {
                        end_year = shad.at(end).year;   end_month = shad.at(end).month;
                        end_day = shad.at(end).day;     end_hour = shad.at(end).hour;
                        end_min = shad.at(end).min;     end_sec = shad.at(end).sec;
                        end_apr = ((end_year - 1970)*360 + (end_month - 1)*30 + end_day)*24*3600
                                + end_hour*3600 + end_min*60 + end_sec + 30*60;
                    }
                    else
                        break;

                    if(apr > begin_apr && apr < end_apr)
                        flag_del[cf_i] = -1;

                    int a = 23;

                }
            }

        }
    }

    for(int cf_i = (length_epoch - 1); cf_i > -1; cf_i--)
        if(flag_del[cf_i] == -1)
            epochSatlitData.remove(cf_i);

    int a = 23;

}

//2021.01.08 23z
void QReadBIA::readsnx()
{
    QString filepath = global_cf::filepath_ini;
    QFile file(filepath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        bool estimate_showup = false;
        while (!file.atEnd())
        {
            //line
            QString temp = file.readLine();
            snx_single snx_temp;

            if(temp.contains("+SOLUTION/ESTIMATE")) { estimate_showup = true;  continue; }
            if(temp.contains("-SOLUTION/ESTIMATE")) { estimate_showup = false; break;    }

            if(estimate_showup && !temp.contains("*INDEX"))
            {
                snx_temp.type = temp.mid(7,4);
                snx_temp.code = temp.mid(14,4);
                snx_temp.value = temp.mid(47,21).toDouble();
                snx_temp.std_dev = temp.mid(69,11).toDouble();

                snx.append(snx_temp);
            }

        }

        int a = 23;

        file.close();
    }
}

//2021.01.08 23z
bool QReadBIA::getinitialpos(QVector< snx_single > &snx_temp)
{
    bool flag = false;

    QString code_cur;
    code_cur = snx_temp.at(0).code;

    int length_snx = 0;
    length_snx = snx.length();

    for(int cf_i = 0; cf_i < length_snx; cf_i++)
    {
        if(snx.at(cf_i).code == code_cur)
        {
            snx_temp[0].value = snx.at(cf_i).value;
            snx_temp[0].std_dev = snx.at(cf_i).std_dev;

            snx_temp[1].value = snx.at(cf_i + 1).value;
            snx_temp[1].std_dev = snx.at(cf_i + 1).std_dev;

            snx_temp[2].value = snx.at(cf_i + 2).value;
            snx_temp[2].std_dev = snx.at(cf_i + 2).std_dev;

            flag = true;
            break;

        }
    }

    int a = 23;

    if(flag) return true;
    else     return false;

}


//Delete short arc satellite 2021.01.15 23z
void QReadBIA::deletetempsats(QVector< QVector< SatlitData > > &multepochSatlitData, QString m_system, double m_interval)
{
    for(int num_sats = 0; num_sats <m_system.length(); num_sats++)
    {
        QString single_sys = m_system.at(num_sats);

        int prn_times_cur[40] = {0}, prn_times_pre[40] = {0};
        QVector< SatlitData > temp_epoch;

        for(int cf_i = 0; cf_i < multepochSatlitData.length(); cf_i++)
        {
            temp_epoch = multepochSatlitData.at(cf_i);
            for(int cf_j = 0; cf_j < temp_epoch.length(); cf_j++)
            {
                if(temp_epoch.at(cf_j).SatType == single_sys)
                    prn_times_cur[temp_epoch.at(cf_j).PRN - 1]++;
            }

            if(cf_i != 0)
            {
                for(int cf_j = 0; cf_j < 40; cf_j++)
                {
                    if(prn_times_cur[cf_j] == prn_times_pre[cf_j] && prn_times_cur[cf_j] != 0)
                    {//arc
                        double arc_length = 0.0;
                        arc_length = 1.0*prn_times_cur[cf_j]*m_interval/60;

                        if(arc_length < 15)
                        {// arc < 15
                            int prn = cf_j + 1;
                            for(int cf_k = 1; cf_k <= prn_times_cur[cf_j]; cf_k++)
                            {
                                temp_epoch = multepochSatlitData.at(cf_i - cf_k);
                                //int l_b = temp_epoch.length();
                                for(int cf_z = 0; cf_z < temp_epoch.length(); cf_z++)
                                {
                                    if(temp_epoch.at(cf_z).PRN == prn)
                                    {
                                        temp_epoch.remove(cf_z);
                                        //int l_f = temp_epoch.length();
                                        multepochSatlitData[cf_i - cf_k] = temp_epoch;
                                        break;
                                    }

                                }
                            }

                            int a = 23;
                        }

                        prn_times_cur[cf_j] = 0;

                    }
                }
            }

            for(int cf_j = 0; cf_j < 40; cf_j++)
            {
                prn_times_pre[cf_j] = prn_times_cur[cf_j];
            }

        }
    }

}


// 2021.03.01 23z
void QReadBIA::cycleslip_refsats(QVector< QVector<SatlitData> > &multepochSatlitData, QString m_system)
{
    for(int cf_sys = 0; cf_sys < m_system.length(); cf_sys++)
    {
        int epoch_num = 0;
        epoch_num = multepochSatlitData.length();

        QVector<SatlitData> curepoch;
        QVector<SatlitData> preepoch;

        int prn_times_cur[40] = {0}, prn_times_pre[40] = {0};

        for(int cf_i = 1; cf_i < epoch_num; cf_i++)
        {
            curepoch = multepochSatlitData.at(cf_i);
            preepoch = multepochSatlitData.at(cf_i - 1);

            for(int cf_j = 0; cf_j < curepoch.length(); cf_j++)
            {
                if(curepoch.at(cf_j).SatType == m_system.at(cf_sys))
                    prn_times_cur[curepoch.at(cf_j).PRN - 1] ++;
            }

            for(int cf_j = 0; cf_j < curepoch.length(); cf_j++)
            {
                if(curepoch.at(cf_j).L1*curepoch.at(cf_j).L2*curepoch.at(cf_j).C1*curepoch.at(cf_j).C2 == 0)
                {
                    int prn = curepoch.at(cf_j).PRN;
                    QVector< SatlitData > temp_epoch;
                    for(int cf_r = 0; cf_r <= prn_times_cur[curepoch.at(cf_j).PRN - 1]; cf_r++)
                    {
                        temp_epoch = multepochSatlitData.at(cf_i - cf_r);
                        for(int cf_z = 0; cf_z < temp_epoch.length(); cf_z++)
                        {
                            if(temp_epoch.at(cf_z).PRN == prn)
                            {
                                temp_epoch[cf_z].reference_flag = false;
                                multepochSatlitData[cf_i - cf_r] = temp_epoch;
                                break;
                            }

                        }
                    }
                }
                else
                {
                    double f1 = 0.0, f2 = 0.0, lamda1 = 0.0, lamda2 = 0.0, lamdaMW = 0.0;
                    f1 = curepoch.at(cf_j).Frq[0];   f2 = curepoch.at(cf_j).Frq[1];
                    lamda1 = M_C/f1;   lamda2 = M_C/f2;   lamdaMW = M_C/(f1 - f2);

                    double amb_cur = 0.0, ion_cur = 0.0;
                    amb_cur = (curepoch.at(cf_j).L1 - curepoch.at(cf_j).L2)
                            - (f1*curepoch.at(cf_j).C1 + f2*curepoch.at(cf_j).C2)/((f1 + f2)*lamdaMW);
                    ion_cur = lamda1*curepoch.at(cf_j).L1 - lamda2*curepoch.at(cf_j).L2;

                    int prn = curepoch.at(cf_j).PRN;
                    for(int cf_k = 0; cf_k < preepoch.length(); cf_k++)
                    {
                        if(preepoch.at(cf_k).PRN == prn)
                        {
                            double amb_pre = 0.0, ion_pre = 0.0;
                            amb_pre = (preepoch.at(cf_k).L1 - preepoch.at(cf_k).L2)
                                    - (f1*preepoch.at(cf_k).C1 + f2*preepoch.at(cf_k).C2)/((f1 + f2)*lamdaMW);
                            ion_pre = lamda1*preepoch.at(cf_k).L1 - lamda2*preepoch.at(cf_k).L2;

                            if(qAbs(amb_cur - amb_pre) > 5 || qAbs(ion_cur - ion_pre) > M_IR)
                            {//When this satellite has a cycle slip, the satellite cannot be selected as a reference satellite
                                QVector< SatlitData > temp_epoch;
                                for(int cf_r = 0; cf_r <= prn_times_cur[curepoch.at(cf_j).PRN - 1]; cf_r++)
                                {
                                    temp_epoch = multepochSatlitData.at(cf_i - cf_r);
                                    for(int cf_z = 0; cf_z < temp_epoch.length(); cf_z++)
                                    {
                                        if(temp_epoch.at(cf_z).PRN == prn)
                                        {
                                            temp_epoch[cf_z].reference_flag = false;
                                            multepochSatlitData[cf_i - cf_r] = temp_epoch;
                                            break;
                                        }

                                    }
                                }
                            }
                            break;
                        }
                    }
                }

            }



            for(int cf_j = 0; cf_j < 40; cf_j++)
            {
                if(prn_times_pre[cf_j] == prn_times_cur[cf_j] && prn_times_cur[cf_j] != 0)
                {
                    prn_times_cur[cf_j] = 0;
                }
            }

            for(int cf_j = 0; cf_j < 40; cf_j++)
            {
                prn_times_pre[cf_j] = prn_times_cur[cf_j];
            }

        }

    }

}





void QReadBIA::select_refsat(QVector<SatlitData> &epochResultSatlitData, QVector<SatlitData> &prevEpochSatlitData, int m_CutAngle, QString m_system)
{
    int num_single_sys[4] = {0};
    //Number of satellites in each system
    for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
    {
        for(int cf_j = 0; cf_j < m_system.length(); cf_j++)
            if(epochResultSatlitData.at(cf_i).SatType == m_system.at(cf_j))
                num_single_sys[cf_j]++;
    }
    for(int cf_i = 1; cf_i < 4; cf_i++)
    {
        if(num_single_sys[cf_i] == 0) break;
        num_single_sys[cf_i] = num_single_sys[cf_i] + num_single_sys[cf_i - 1];
    }

    //Select reference sat 2020.11.05 23z
    //elevation
    for(int num_sys = 0; num_sys < m_system.length(); num_sys++)
    {
        if(prevEpochSatlitData.length() == 0)
        {
            double E_sat = 0.0;
            int prn_refsat = 0, num_refsat = 0;
            E_sat = 0;
            for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
                if(epochResultSatlitData.at(cf_i).EA[0] >= E_sat && epochResultSatlitData.at(cf_i).SatType == m_system.at(num_sys))
                {
                    E_sat = epochResultSatlitData.at(cf_i).EA[0];
                    prn_refsat = epochResultSatlitData.at(cf_i).PRN;
                    num_refsat = cf_i;
                }

            //Put the reference star at the end of the type of satellite
            SatlitData sat_temp = epochResultSatlitData.at(num_refsat);
            epochResultSatlitData.remove(num_refsat);
            epochResultSatlitData.insert(num_single_sys[num_sys] - 1,sat_temp);

            for(int get_prn = 0; get_prn < epochResultSatlitData.length(); get_prn++)
                epochResultSatlitData[get_prn].prn_referencesat[2*num_sys] = prn_refsat;
        }

        //elevation of refsat < (m_CutAngle + 5)?
        if(prevEpochSatlitData.length() != 0)
        {
            //Find the reference sat of the previous epoch,
            int prnofrefsat = prevEpochSatlitData.at(0).prn_referencesat[2*num_sys];
            double elevation = 0.0;
            int numofrefsat = 0;
            for(int cf_i = 0; cf_i < epochResultSatlitData.length() ; cf_i++)
                if(epochResultSatlitData.at(cf_i).PRN == prnofrefsat && epochResultSatlitData.at(cf_i).SatType == m_system.at(num_sys))
                {
                    elevation = epochResultSatlitData.at(cf_i).EA[0];
                    numofrefsat = cf_i;
                    break;
                }

            //elevation of refsat < (m_CutAngle + 5)
            if(elevation < (m_CutAngle + 5))
            {
                double E_sat = 0.0;
                int prn_refsat = 0, num_refsat = 0;
                E_sat = 0;
                //Find the satellite with the highest elevation
                for(int cf_i = 0; cf_i < epochResultSatlitData.length(); cf_i++)
                    if(epochResultSatlitData.at(cf_i).EA[0] >= E_sat && epochResultSatlitData.at(cf_i).SatType == m_system.at(num_sys))
                    {
                        E_sat = epochResultSatlitData.at(cf_i).EA[0];
                        prn_refsat = epochResultSatlitData.at(cf_i).PRN;
                        num_refsat = cf_i;
                    }

                //Put the reference star at the end of the type of satellite
                SatlitData sat_temp = epochResultSatlitData.at(num_refsat);
                epochResultSatlitData.remove(num_refsat);
                epochResultSatlitData.insert(num_single_sys[num_sys] - 1,sat_temp);
                //update
                for(int get_prn = 0; get_prn < epochResultSatlitData.length(); get_prn++)
                    epochResultSatlitData[get_prn].prn_referencesat[2*num_sys] = prn_refsat;
            }
            else
            {
                //Put the reference star at the end of the type of satellite
                SatlitData sat_temp = epochResultSatlitData.at(numofrefsat);
                epochResultSatlitData.remove(numofrefsat);
                epochResultSatlitData.insert(num_single_sys[num_sys] - 1,sat_temp);

                //update
                for(int get_prn = 0; get_prn < epochResultSatlitData.length(); get_prn++)
                    epochResultSatlitData[get_prn].prn_referencesat[2*num_sys] = prnofrefsat;
            }

        }
    }


}




void QReadBIA::get_refsat_sit(QVector<SatlitData> &epochResultSatlitData, QString m_system)
{
    for(int cf_i = 0; cf_i < m_system.length(); cf_i++)
    {
        int prn = 0, sit = 0;
        prn = epochResultSatlitData.at(0).prn_referencesat[2*cf_i];
        if(prn != 0)
        {
            for(int cf_j = 0; cf_j < epochResultSatlitData.length(); cf_j++)
            {
                if(epochResultSatlitData.at(cf_j).PRN == prn && epochResultSatlitData.at(cf_j).SatType == m_system.at(cf_i))
                {
                    sit = cf_j;
                    break;
                }
            }
            for(int cf_j = 0; cf_j < epochResultSatlitData.length(); cf_j++)
                epochResultSatlitData[cf_j].prn_referencesat[2*cf_i + 1] = sit;

        }

    }
}




//cod
void QReadBIA::readcodosbfile(QString codbiafilepath)
{
//    QString COD_BIA = "CODE_MONTHLY.BIA";

//    QFile file_cod(COD_BIA);
//    if (file_cod.open(QIODevice::ReadOnly | QIODevice::Text))
//    {
//        QString temp;
//        cod_osb temp_cod_line;

//        bool match_flag = false;
//        bool finsh_flag = false;

//        while (!file_cod.atEnd())
//        {
//            //line
//            temp = file_cod.readLine();

//            if(temp.contains("*YEAR-MONTH"))
//            {
//                int year = 0, month = 0;

//                year = temp.mid(12,4).toInt();
//                month = temp.mid(17,2).toInt();

//                if(year == temp_time.Year && month == temp_time.Month)
//                    match_flag = true;
//                else
//                    match_flag = false;

//                if(year == temp_time.Year && month == (temp_time.Month + 1))
//                    finsh_flag = true;
//                else
//                    finsh_flag = false;

//                continue;
//            }

//            if(match_flag)
//            {
//                QString prn, type_obs;
//                double bias, bias_std;

//                prn = temp.mid(11,3);
//                type_obs = temp.mid(25,3);
//                bias = temp.mid(80,12).toDouble();
//                bias_std = temp.mid(95,8).toDouble();

//                temp_cod_line.prn = prn;
//                temp_cod_line.type = type_obs;
//                temp_cod_line.value = bias;
//                temp_cod_line.std_dev = bias_std;

//                cod_bia.append(temp_cod_line);
//            }

//            if(finsh_flag) break;

//        }

//        file_cod.close();
//    }

    QFile file_cod(codbiafilepath);
    if (file_cod.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString temp;
        cod_osb temp_cod_line;
        while (!file_cod.atEnd())
        {
            //line
            temp = file_cod.readLine();
            if(temp.mid(1,3) == "OSB" && temp.mid(16,4) == "    ")
            {
                temp_cod_line.prn = temp.mid(11,3);;
                temp_cod_line.type = temp.mid(25,3);
                temp_cod_line.value = temp.mid(80,12).toDouble();
                temp_cod_line.std_dev = temp.mid(95,8).toDouble();

                cod_bia.append(temp_cod_line);
            }

        }
    }

    file_cod.close();

}





void QReadBIA::getcodosb(SatlitData &tempSatlitData)
{/* TIME_SYSTEM                             G
    SATELLITE_CLOCK_REFERENCE_OBSERVABLES   G  C1W  C2W
    SATELLITE_CLOCK_REFERENCE_OBSERVABLES   R  C1P  C2P
    SATELLITE_CLOCK_REFERENCE_OBSERVABLES   E  C1C  C5Q
GPS C1C observations have to be corrected according to
    Obs_C1C - Bias_C1C + Bias_C1W (GPS P1-C1 DCB = Bias_C1W - Bias_C1C.)  23Z*/

    QString type_obs = "", prn;
    double obs_corrected = 0.0;
    double bias1 = 0.0, bias2 = 0.0;
    bool flag1, flag2;
    double Lamta1 = M_C/tempSatlitData.Frq[0],Lamta2 = M_C/tempSatlitData.Frq[1];

    for(int cf_i = 0; cf_i < 4; cf_i++)
    {
        flag1 = false;   flag2 = false;
        bias1 = 0.0;     bias2 = 0.0;

        type_obs = tempSatlitData.wantObserType[cf_i];
        if(tempSatlitData.SatType == 'G')
        {
            if(tempSatlitData.PRN < 10)
                prn = "G0" + QString::number(tempSatlitData.PRN);
            else
                prn = "G" + QString::number(tempSatlitData.PRN);

            if(type_obs == "C1" || type_obs == "C1C")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C1C")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C1W")
                        bias2 = cod_bia.at(cf_j).value, flag2 = true;

                    if(flag2) break;
                }
                obs_corrected = tempSatlitData.C1 - (bias1-bias2) * M_C * 1e-9;
                tempSatlitData.C1 = obs_corrected;
            }
            if(type_obs == "C2" || type_obs == "C2C")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C2C")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C2W")
                        bias2 = cod_bia.at(cf_j).value, flag2 = true;

                    if(flag2) break;
                }
                obs_corrected = tempSatlitData.C2 - (bias1-bias2) * M_C * 1e-9;
                tempSatlitData.C2 = obs_corrected;
            }
            if(type_obs == "L1" || type_obs == "L1C")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L1C")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L1 - bias1 * M_C * 1e-9/Lamta1;
                tempSatlitData.L1 = obs_corrected;
            }
            if(type_obs == "L1W")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L1W")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L1 - bias1 * M_C * 1e-9/Lamta1;
                tempSatlitData.L1 = obs_corrected;
            }
            if(type_obs == "L2" || type_obs == "L2C")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L2C")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L2 - bias1 * M_C * 1e-9/Lamta2;
                tempSatlitData.L2 = obs_corrected;
            }
            if(type_obs == "L2W")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L2W")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L2 - bias1 * M_C * 1e-9/Lamta2;
                tempSatlitData.L2 = obs_corrected;
            }
            if(type_obs == "L2X")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L2X")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L2 - bias1 * M_C * 1e-9/Lamta2;
                tempSatlitData.L2 = obs_corrected;
            }
        }

        if(tempSatlitData.SatType == 'E')
        {
            if(tempSatlitData.PRN < 10)
                prn = "E0" + QString::number(tempSatlitData.PRN);
            else
                prn = "E" + QString::number(tempSatlitData.PRN);

            if(type_obs == "C1X")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C1X")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C1C")
                        bias2 = cod_bia.at(cf_j).value, flag2 = true;

                    if(flag2) break;
                }
                obs_corrected = tempSatlitData.C1 - (bias1-bias2) * M_C * 1e-9;
                tempSatlitData.C1 = obs_corrected;
            }
            if(type_obs == "C5X")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C5X")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C2C")
                        bias2 = cod_bia.at(cf_j).value, flag2 = true;

                    if(flag2) break;
                }
                obs_corrected = tempSatlitData.C2 - (bias1-bias2) * M_C * 1e-9;
                tempSatlitData.C2 = obs_corrected;
            }
            if(type_obs == "C5Q")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C5Q")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="C2C")
                        bias2 = cod_bia.at(cf_j).value, flag2 = true;

                    if(flag2) break;
                }
                obs_corrected = tempSatlitData.C2 - (bias1-bias2) * M_C * 1e-9;
                tempSatlitData.C2 = obs_corrected;
            }
            if(type_obs == "L1C")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L1C")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L1 - bias1 * M_C * 1e-9/Lamta1;
                tempSatlitData.L1 = obs_corrected;
            }
            if(type_obs == "L1X")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L1X")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L1 - bias1 * M_C * 1e-9/Lamta1;
                tempSatlitData.L1 = obs_corrected;
            }
            if(type_obs == "L5C")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L5C")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L2 - bias1 * M_C * 1e-9/Lamta2;
                tempSatlitData.L2 = obs_corrected;
            }
            if(type_obs == "L5X")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L5X")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L2 - bias1 * M_C * 1e-9/Lamta2;
                tempSatlitData.L2 = obs_corrected;
            }
            if(type_obs == "L5Q")
            {
                for(int cf_j = 0; cf_j < cod_bia.length(); cf_j++)
                {
                    if(cod_bia.at(cf_j).prn == prn && cod_bia.at(cf_j).type =="L5Q")
                        bias1 = cod_bia.at(cf_j).value, flag1 = true;

                    if(flag1) break;
                }
                obs_corrected = tempSatlitData.L2 - bias1 * M_C * 1e-9/Lamta2;
                tempSatlitData.L2 = obs_corrected;
            }

        }
    }

}



