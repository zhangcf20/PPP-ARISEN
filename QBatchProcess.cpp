#include "QBatchProcess.h"

QBatchProcess::QBatchProcess(QStringList files_path, QTextEdit *pQTextEdit, QString Method, QString Satsystem, QString TropDelay,
                             double CutAngle, bool isKinematic, QString Smooth_Str, bool isBackBatch, QString products, QString pppmodel_t)
{
    // GNSS configure
    mp_QTextEdit = pQTextEdit;
    m_Method = Method;
    m_Satsystem = Satsystem;
    m_TropDelay = TropDelay;
    m_CutAngle = CutAngle;
    m_Product = products;
    QString pre_name = "Final_";
    if(products.contains("igs", Qt::CaseInsensitive))
        pre_name = "Final_";
    else
        pre_name = "RT_";


    // store data
    M_ObsFiles_Path = files_path.at(0);
    m_mkdir_name = pre_name + "allStations";
    m_isRuned = false;
    m_isKinematic = isKinematic;
    m_Smooth_Str = Smooth_Str;
    m_isBackBatch = isBackBatch;
    m_PPPModel_Str = pppmodel_t;

    RPR_filepath = files_path;

}
QBatchProcess::~QBatchProcess()
{

}
//isDisplayEveryEpoch represent is disply every epoch information?(ENU or XYZ)
bool QBatchProcess::Run(bool isDisplayEveryEpoch)
{
    if(m_isRuned) return false;
    // clear data
    m_AllStationsData.clear();
    // distibute multiply .O files to destin_floder
    //QString mkdir = m_mkdir_name + PATHSEG;
    QString destin_floder = M_ObsFiles_Path + PATHSEG ;//+ mkdir;

    if(!isDirExist(destin_floder))
        return false;
    QDir path_dir(destin_floder);
    QStringList m_fliterList;
    m_fliterList.clear();
    m_fliterList.append("*.*o");
    path_dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QStringList ObsFileNameList = path_dir.entryList(m_fliterList);

    QStringList floder_list;

    if(ObsFileNameList.length() == 0)
    {
        QString erro_info = "QBatchProcess::Run: there is no o file!";
        ErroTrace(erro_info);
        return false;
    }
    else
    {
        QString temp_floder, temp_markname;
        for(int cf_i = 0; cf_i < ObsFileNameList.length(); cf_i++)
        {
            temp_markname = ObsFileNameList.at(cf_i).mid(0,4);
            m_AllStations.append(temp_markname);
            temp_floder = destin_floder + ObsFileNameList.at(cf_i);
            floder_list.append(temp_floder);
        }
    }

//    if(!distribute(M_ObsFiles_Path, destin_floder))
//    {
//        QString erro_info = "QBatchProcess::Run: distribute files Bad!";
//        ErroTrace(erro_info);
//        return false;
//    }
// ppp, Multiple  sation PPP
    // get and print floders path
    QString multiple_station_floder = destin_floder;
    QString disPlayQTextEdit = "Run multiply floder: " + multiple_station_floder;
    autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit

//    // run batch PPP
//    QDir stations_floder(multiple_station_floder);
//    stations_floder.setFilter(QDir::Dirs);
//    // get floder path
//    QFileInfoList list_info = stations_floder.entryInfoList();
//    QStringList floder_list;
//    for(int i = 0; i < list_info.length(); i++)
//    {
//        QFileInfo file_info = list_info.at(i);
//        if(file_info.fileName() == "." || file_info.fileName() == "..") continue;
//        if(file_info.isDir())
//            floder_list.append(file_info.absoluteFilePath());
//    }
//    // get and save floders name
//    QStringList AllStations = stations_floder.entryList();
    disPlayQTextEdit.clear();
    for(int i = 0; i < m_AllStations.length(); i++ )
    {
//        if(AllStations.at(i) == "." || AllStations.at(i) == "..") continue;
//        m_AllStations.append(AllStations.at(i));
        disPlayQTextEdit += m_AllStations.at(i) + " ";
    }
    // display floders name
    disPlayQTextEdit = ENDLINE + "Marks: " + ENDLINE + disPlayQTextEdit + ENDLINE;
    autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit
    // juge is equal
    if(m_AllStations.length() != floder_list.length())
    {
        QString erro_info = "QBatchProcess::Run: m_AllStations.length() != floder_list.length()";
        ErroTrace(erro_info);
        return false;
    }
    // run all stations
    double allTime = 0;
    int allStations_len = floder_list.length();
    for(int i = 0;i < allStations_len; i++)
    {
        QString ppp_path = floder_list.at(i);
        QStringList rpr_path = RPR_filepath;
        rpr_path.replace(0,ppp_path);

        disPlayQTextEdit = "*** Batch: " + ENDLINE + m_AllStations.at(i) +
                + " ( " + QString::number(i+1) + "/" + QString::number(allStations_len) +  " ) " +ENDLINE;
        autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit
        // run single station
        QTime myTime;
        myTime.start();//start the timer

        PlotGUIData single_data;// get single station data
        if(m_isBackBatch)
        {
            QPPPBackSmooth myPPP(rpr_path, mp_QTextEdit, m_Method, m_Satsystem, m_TropDelay, m_CutAngle, m_isKinematic, m_Smooth_Str, m_Product, m_PPPModel_Str);
            myPPP.Run(isDisplayEveryEpoch);
//            myPPP.getRunResult(single_data);
//            m_AllStationsData.append(single_data);

        }
        else
        {
            QPPPModel myPPP(rpr_path, mp_QTextEdit, m_Method, m_Satsystem, m_TropDelay, m_CutAngle, m_isKinematic, m_Smooth_Str, m_Product, m_PPPModel_Str);
            myPPP.Run(isDisplayEveryEpoch);
//            myPPP.getRunResult(single_data);
//            m_AllStationsData.append(single_data);
        }
        float m_diffTime = myTime.elapsed() / 1000.0;

        // display time
//        disPlayQTextEdit = "Batch Process The Elapse Time: " + QString::number(m_diffTime) + "s" + ENDLINE;
//        autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit
//        allTime += m_diffTime;
    }
    allTime = allTime / 60; // transfer senconds to minutes
    disPlayQTextEdit = "done";
    autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit
    m_isRuned = true;
	return true;
}

QStringList QBatchProcess::getStationNames()
{
    QStringList temp;
    if(m_isRuned)
        return m_AllStations;
    else
        return temp;
}

void QBatchProcess::getStoreAllData(QVector< PlotGUIData > &all_SationData)
{// use pointer store for External data
     all_SationData = m_AllStationsData;
}

// The edit box automatically scrolls, adding one row or more lines at a time.
void QBatchProcess::autoScrollTextEdit(QTextEdit *textEdit,QString &add_text)
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
    //If you exceed a certain number of lines, empty it.
    if(textEdit->document()->lineCount() > m_Display_Max_line)
    {
        textEdit->clear();
    }
}

bool QBatchProcess::isDirExist(QString fullPath)
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
    return false;
}

bool QBatchProcess::isFileExist(QString fullFileName)
{
    QFileInfo fileinfo(fullFileName);
    if(fileinfo.isFile())
        return true;
    else
        return false;
}

bool QBatchProcess::distribute(QString ofile_path, QString destin_floder)
{
    if(!isDirExist(destin_floder))
        return false;
    QDir path_dir(ofile_path);
    QStringList m_fliterList;
    m_fliterList.clear();
    m_fliterList.append("*.*o");
    path_dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QStringList ObsFileNameList = path_dir.entryList(m_fliterList);

    for(int i = 0; i < ObsFileNameList.length();i++)
    {
        QString obs_file_name = ObsFileNameList.at(i),
                obs_file_path = ofile_path + PATHSEG + obs_file_name,
                floder_path = destin_floder + ObsFileNameList.at(i) + PATHSEG,
                destin_file_name = floder_path + obs_file_name;
//        if(!isDirExist(floder_path))
//        {
//            QString erro_info = "QBatchProcess::distribute: make dir error. " + PATHSEG + floder_path;
//            ErroTrace(erro_info);
//            return false;
//        }
        if(!isFileExist(destin_file_name))
        {
            if(!QFile::copy(obs_file_path, destin_file_name))
            {
                QString erro_info = "QBatchProcess::distribute: move file error." + obs_file_path;
                ErroTrace(erro_info);
                return false;
            }
        }

    }
    return true;
}



//星间单差PPP和AR的批量处理 2020.12.26 23z
bool QBatchProcess::run_SSD_or_AR(int OperationStrategy, bool isBackBatch)
{
    if(m_isRuned) return false;
    // clear data
    m_AllStationsData.clear();
    // distibute multiply .O files to destin_floder
    //QString mkdir = m_mkdir_name + PATHSEG;
    QString destin_floder = M_ObsFiles_Path + PATHSEG ;//+ mkdir;

    if(!isDirExist(destin_floder))
        return false;
    QDir path_dir(destin_floder);
    QStringList m_fliterList;
    m_fliterList.clear();
    m_fliterList.append("*.*o");
    path_dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QStringList ObsFileNameList = path_dir.entryList(m_fliterList);

    QStringList floder_list;

    if(ObsFileNameList.length() == 0)
    {
        QString erro_info = "QBatchProcess::Run: there is no o file!";
        ErroTrace(erro_info);
        return false;
    }
    else
    {
        QString temp_floder, temp_markname;
        for(int cf_i = 0; cf_i < ObsFileNameList.length(); cf_i++)
        {
            temp_markname = ObsFileNameList.at(cf_i).mid(0,4);
            m_AllStations.append(temp_markname);
            temp_floder = destin_floder + ObsFileNameList.at(cf_i);
            floder_list.append(temp_floder);
        }
    }

//    if(!distribute(M_ObsFiles_Path, destin_floder))
//    {
//        QString erro_info = "QBatchProcess::Run: distribute files Bad!";
//        ErroTrace(erro_info);
//        return false;
//    }
// ppp, Multiple  sation PPP
    // get and print floders path
    QString multiple_station_floder = destin_floder;
    QString disPlayQTextEdit = "Run multiply floder: " + multiple_station_floder;
    autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit

//    // run batch PPP
//    QDir stations_floder(multiple_station_floder);
//    stations_floder.setFilter(QDir::Dirs);
//    // get floder path
//    QFileInfoList list_info = stations_floder.entryInfoList();
//    QStringList floder_list;
//    for(int i = 0; i < list_info.length(); i++)
//    {
//        QFileInfo file_info = list_info.at(i);
//        if(file_info.fileName() == "." || file_info.fileName() == "..") continue;
//        if(file_info.isDir())
//            floder_list.append(file_info.absoluteFilePath());
//    }
//    // get and save floders name
//    QStringList AllStations = stations_floder.entryList();
    disPlayQTextEdit.clear();
    for(int i = 0; i < m_AllStations.length(); i++ )
    {
//        if(AllStations.at(i) == "." || AllStations.at(i) == "..") continue;
//        m_AllStations.append(AllStations.at(i));
        disPlayQTextEdit += m_AllStations.at(i) + " ";
    }
//    if(!distribute(M_ObsFiles_Path, destin_floder))
//    {
//        QString erro_info = "QBatchProcess::Run: distribute files Bad!";
//        ErroTrace(erro_info);
//        return false;
//    }
//// ppp, Multiple  sation PPP
//    // get and print floders path
//    QString multiple_station_floder = destin_floder;
//    QString disPlayQTextEdit = "Run multiply floder: " + multiple_station_floder;
//    autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit

//    // run batch PPP
//    QDir stations_floder(multiple_station_floder);
//    stations_floder.setFilter(QDir::Dirs);
//    // get floder path
//    QFileInfoList list_info = stations_floder.entryInfoList();
//    QStringList floder_list;
//    for(int i = 0; i < list_info.length(); i++)
//    {
//        QFileInfo file_info = list_info.at(i);
//        if(file_info.fileName() == "." || file_info.fileName() == "..") continue;
//        if(file_info.isDir())
//            floder_list.append(file_info.absoluteFilePath());
//    }
//    // get and save floders name
//    QStringList AllStations = stations_floder.entryList();
//    disPlayQTextEdit.clear();
//    for(int i = 0; i < AllStations.length(); i++ )
//    {
//        if(AllStations.at(i) == "." || AllStations.at(i) == "..") continue;
//        m_AllStations.append(AllStations.at(i));
//        disPlayQTextEdit += AllStations.at(i) + ", ";
//    }
//    // display floders name
    disPlayQTextEdit = ENDLINE + "Marks: " + ENDLINE + disPlayQTextEdit + ENDLINE;
    autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit
    // juge is equal
    if(m_AllStations.length() != floder_list.length())
    {
        QString erro_info = "QBatchProcess::Run: m_AllStations.length() != floder_list.length()";
        ErroTrace(erro_info);
        return false;
    }
    // run all stations
    double allTime = 0;
    int allStations_len = floder_list.length();
    for(int i = 0;i < allStations_len; i++)
    {
        QString ppp_path = floder_list.at(i);
        QStringList rpr_path = RPR_filepath;
        rpr_path.replace(0,ppp_path);

        disPlayQTextEdit = "*** Batch: " + ENDLINE + m_AllStations.at(i) +
                + " ( " + QString::number(i+1) + "/" + QString::number(allStations_len) +  " )" +ENDLINE;
        autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit
        // run single station
        QTime myTime;
        myTime.start();//start the timer

        PlotGUIData single_data;// get single station data
        if(m_isBackBatch)
        {
            if(OperationStrategy == 6)
            {
                QPPPBackSmooth myPPP(rpr_path, mp_QTextEdit, m_Method, m_Satsystem, m_TropDelay, m_CutAngle, m_isKinematic, m_Smooth_Str, m_Product, m_PPPModel_Str);
                myPPP.SSDPPP(false);
//                myPPP.getRunResult(single_data);
//                m_AllStationsData.append(single_data);
            }

            if(OperationStrategy == 7)
            {
                QPPPBackSmooth myPPP(rpr_path, mp_QTextEdit, m_Method, m_Satsystem, m_TropDelay, m_CutAngle, m_isKinematic, m_Smooth_Str, m_Product, m_PPPModel_Str);
                myPPP.PPPAR(false);
//                myPPP.getRunResult(single_data);
//                m_AllStationsData.append(single_data);
            }


        }
        else
        {
            if(OperationStrategy == 6)
            {
                QPPPModel myPPP(rpr_path, mp_QTextEdit, m_Method, m_Satsystem, m_TropDelay, m_CutAngle, m_isKinematic, m_Smooth_Str, m_Product, m_PPPModel_Str);
                myPPP.runSSDPPP(false);
//                myPPP.getRunResult(single_data);
//                m_AllStationsData.append(single_data);
            }

            if(OperationStrategy == 7)
            {
                QPPPModel myPPP(rpr_path, mp_QTextEdit, m_Method, m_Satsystem, m_TropDelay, m_CutAngle, m_isKinematic, m_Smooth_Str, m_Product, m_PPPModel_Str);
                myPPP.runPPPAR(false);
//                myPPP.getRunResult(single_data);
//                m_AllStationsData.append(single_data);
            }

        }
        float m_diffTime = myTime.elapsed() / 1000.0;

        // display time
//        disPlayQTextEdit = "Batch Process The Elapse Time: " + QString::number(m_diffTime) + "s" + ENDLINE;
//        autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit
//        allTime += m_diffTime;
    }
    allTime = allTime / 60; // transfer senconds to minutes
    disPlayQTextEdit = "done";
    autoScrollTextEdit(mp_QTextEdit, disPlayQTextEdit);// display for QTextEdit
    m_isRuned = true;
    return true;
}
