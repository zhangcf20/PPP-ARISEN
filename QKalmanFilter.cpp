#include "QKalmanFilter.h"


QKalmanFilter::QKalmanFilter()
{
	initVar();
}


QKalmanFilter::~QKalmanFilter(void)
{
}

void QKalmanFilter::initVar()
{
	isInitPara = false;//The first epoch is only initialized once
	m_VarChang = false;
    m_KALMAN_MODEL = KALMAN_MODEL::PPP_STATIC;
    m_KALMAN_SMOOTH_RANGE = KALMAN_SMOOTH_RANGE::NO_SMOOTH;
    m_KALMAN_FILLTER = KALMAN_FILLTER::KALMAN_STANDARD;
    m_SPP_Pos[0] = 0;m_SPP_Pos[1] = 0; m_SPP_Pos[2] = 0;
    m_Xk_1.resize(32);// XiaoGongWei Update:2018.10.26
    m_init_Xk.resize(32);// XiaoGongWei Update:2018.10.26
    m_Xk_1.setZero();// XiaoGongWei Update:2018.10.26
    m_init_Xk.setZero();// XiaoGongWei Update:2018.10.26
    m_const_param = 4;// [dx,dy,dz,mf,clki]
    m_sys_num = 1;
    m_sys_str = "G";
    m_LP_whight = 1e6;
    m_xyz_dynamic_Qw = 1e6; m_zwd_Qw = 3e-8; m_clk_Qw = 1e6; m_amb_Qw = 1e-16; m_ion_Qw = 1e6;
    m_xyz_dynamic_Pk = 1e6; m_zwd_Pk = 10; m_clk_Pk = 1e6; m_amb_Pk = 1e6; m_ion_Pk = 10;

    m_amb_SSD = 9.0/36*1e-10;

    //randomwalk model：v(m/sqrt(s)) q(m^2)=v^2(m^2/s) * interval(s) 23z


    //2020.12.16 23z
    double randomwalk_zwd = global_cf::randomwalk_zwd;
    if(randomwalk_zwd < 0) m_zwd_Qw = 9.0/36*1e-8;
    else m_zwd_Qw = randomwalk_zwd*randomwalk_zwd/36*1e-8;

    double randomwalk_pos = global_cf::randomwalk_pos/100;
    if(global_cf::whitenoise_pos) m_xyz_dynamic_Qw = 100;
    else if(randomwalk_pos < 0) m_xyz_dynamic_Qw = 100;
    else m_xyz_dynamic_Qw = randomwalk_pos*randomwalk_pos/1.7321;


}

void QKalmanFilter::setFilterParams(QVector<QStringList> Qw_Pk_LPacc)
{
    if(Qw_Pk_LPacc.length() >= 2){
        QStringList Qw_StrList =  Qw_Pk_LPacc.at(0),Pk_StrList = Qw_Pk_LPacc.at(1);
        if(Qw_StrList.length() < 5) return ; if(Pk_StrList.length() < 5) return ;
        // set Qw
        double q_pos, q_zwd, q_clk, q_amb, q_ion;
        q_pos = Qw_StrList.at(0).toDouble();     q_zwd = Qw_StrList.at(1).toDouble();
        q_clk = Qw_StrList.at(2).toDouble();     q_amb = Qw_StrList.at(3).toDouble();
        q_ion = Qw_StrList.at(4).toDouble();
        if(q_pos > 0) m_xyz_dynamic_Qw = q_pos * q_pos/1.7321;
        if(q_zwd > 0) m_zwd_Qw = q_zwd;
        if(q_clk > 0) m_clk_Qw = q_clk;
        if(q_amb > 0) m_amb_Qw = q_amb;
        if(q_ion > 0) m_ion_Qw = q_ion;

        // set Pk
        double p_pos, p_zwd, p_clk, p_amb, p_ion;
        p_pos = Qw_StrList.at(0).toDouble();     p_zwd = Qw_StrList.at(1).toDouble();
        p_clk = Qw_StrList.at(2).toDouble();     p_amb = Qw_StrList.at(3).toDouble();
        p_ion = Qw_StrList.at(4).toDouble();
        if(p_pos > 0) m_xyz_dynamic_Pk = q_pos * q_pos/1.7321;
        if(p_zwd > 0) m_zwd_Pk = p_zwd;
        if(p_clk > 0) m_clk_Pk = p_clk;
        if(p_amb > 0) m_amb_Pk = p_amb;
        if(p_ion > 0) m_ion_Pk = p_ion;

    }
    if(Qw_Pk_LPacc.length() >= 3){
        QStringList LP_StrList = Qw_Pk_LPacc.at(2);
        double LP_ratio = 1e3;
        if(LP_StrList.length() == 2 && LP_StrList.at(0).toDouble() != 0 )
            LP_ratio = LP_StrList.at(1).toDouble() / LP_StrList.at(0).toDouble();
        m_LP_whight = LP_ratio * LP_ratio;// set m_LP_whight
    }
}

// set KALMAN_MODEL
void QKalmanFilter::setModel(KALMAN_MODEL model_type)
{
    m_KALMAN_MODEL = model_type;
    m_sys_num = getSystemnum();
    m_sys_str = getSatlitSys(); 
    switch (model_type)
    {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        m_const_param = 3 + m_sys_num;//[dx,dy,dz,clki]
        break;
    case KALMAN_MODEL::PPP_KINEMATIC:
    case KALMAN_MODEL::PPP_STATIC:
        m_const_param = 4 + m_sys_num;//[dx,dy,dz,mf,clki]
        break;
    default:
        m_const_param = 4+1;
        break;
    }
}

//Print matrix for Debug
void QKalmanFilter::printMatrix(MatrixXd mat)
{
    qDebug()<<"Print Matrix......";
    for (int i = 0; i < mat.rows();i++)
    {
        for (int j = 0;j< mat.cols();j++)
        {
            cout <<mat(i,j)<<",";
        }
        cout << endl;
    }
    cout<<"___________________";
}

//Initialize Kalman
void QKalmanFilter::initKalman(QVector< SatlitData > &currEpoch,MatrixXd &B,VectorXd &L)
{
  //2020.11.10 23z  + SSD Function model
	int epochLenLB = currEpoch.length();

	//Fk_1 initialization
    switch (m_KALMAN_MODEL) {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        m_Fk_1.resize(m_const_param, m_const_param);
        m_Fk_1.setIdentity(m_const_param, m_const_param);
        break;
    case KALMAN_MODEL::PPP_KINEMATIC:
    case KALMAN_MODEL::PPP_STATIC:
        if(currEpoch.at(0).SSDPPP)
        {
            m_Fk_1.resize(4 + epochLenLB - sys_num_cur,4 + epochLenLB - sys_num_cur);
            m_Fk_1.setIdentity();
            break;
        }
        else
        {
            m_Fk_1.resize(m_const_param+epochLenLB,m_const_param+epochLenLB);
            m_Fk_1.setIdentity(m_const_param+epochLenLB,m_const_param+epochLenLB);
            break;
        }

    default:
        break;
    }
    //Xk_1 pesodurange init  Initialization, least squares initialization
    switch (m_KALMAN_MODEL) {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        m_Xk_1.resize(m_const_param);
        m_Xk_1.setZero();
        break;
    case KALMAN_MODEL::PPP_KINEMATIC:
    case KALMAN_MODEL::PPP_STATIC:
        if(currEpoch.at(0).SSDPPP)
        {
            m_Xk_1.resize(4 + epochLenLB - sys_num_cur);
            m_Xk_1.setZero();
            break;
        }
        else
        {
            m_Xk_1.resize(epochLenLB+m_const_param);
            m_Xk_1.setZero();
            break;
        }

    default:
        ErroTrace("QKalmanFilter::initKalman Bad.");
        break;
    }

    int length_L = L.rows();

// 2021.02.05

    MatrixXd temp_p;
    temp_p.resize(length_L,length_L);
    temp_p.setZero();

    MatrixXd temp_p_all;
    temp_p_all.resize(currEpoch.length()*2,currEpoch.length()*2);
    temp_p_all.setZero();

    if(currEpoch.at(0).SSDPPP)
    {
        for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
        {
            temp_p_all(cf_i,cf_i) = currEpoch.at(cf_i).SatWight * m_LP_whight;
            temp_p_all(cf_i + currEpoch.length(),cf_i + currEpoch.length()) = currEpoch.at(cf_i).SatWight;
        }
        temp_p = tran_mat*temp_p_all*(tran_mat.transpose());
    }
    else
    {
        for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
        {
            temp_p(cf_i,cf_i) = currEpoch.at(cf_i).SatWight * m_LP_whight;
            temp_p(cf_i + currEpoch.length(),cf_i + currEpoch.length()) = currEpoch.at(cf_i).SatWight;
        }
        for(int cf_i = 1; cf_i < m_sys_num; cf_i++)
        {//Avoid rank deficiency
            if(temp_p(length_L - cf_i,length_L - cf_i) == 0)
                temp_p(length_L - cf_i,length_L - cf_i) = 1;
        }
    }

    m_Xk_1 = (B.transpose()*temp_p*B).inverse()*B.transpose()*temp_p*L;
    m_init_Xk = m_Xk_1;

    //2021.01.09 23z Adjustment
    m_Pk_1 = 3*(B.transpose()*temp_p*B).inverse();
    if(KALMAN_MODEL::PPP_KINEMATIC) m_Pk_1 = (B.transpose()*temp_p*B).inverse();

	//Qk_1 system noise initialization
    switch (m_KALMAN_MODEL) {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        m_Qwk_1.resize(m_const_param, m_const_param);
        m_Qwk_1.setZero();
        for(int i = 3; i < m_const_param;i++) m_Qwk_1(i,i) = m_clk_Qw;// for clock
        break;
    case KALMAN_MODEL::PPP_KINEMATIC:
    case KALMAN_MODEL::PPP_STATIC:
        if(currEpoch.at(0).SSDPPP)
        {
            m_Qwk_1.resize(4 + epochLenLB - sys_num_cur,4 + epochLenLB - sys_num_cur);
            m_Qwk_1.setZero();
            m_Qwk_1(3,3) = m_zwd_Qw * interval_cur;

            for(int cf_i = 0; cf_i < (epochLenLB - sys_num_cur); cf_i++)
                m_Qwk_1((cf_i + 4),(cf_i + 4)) = m_amb_SSD * interval_cur;

            break;
        }
        else
        {
            m_Qwk_1.resize(m_const_param+epochLenLB,m_const_param+epochLenLB);
            m_Qwk_1.setZero();
            m_Qwk_1(3,3) = m_zwd_Qw * interval_cur;//Zenith tropospheric residual variance
            for(int i = 4; i < m_const_param;i++) m_Qwk_1(i,i) = m_clk_Qw; // for clock
            break;
        }

    default:
        ErroTrace("QKalmanFilter::initKalman Bad.");
        break;
    }
    if(m_KALMAN_MODEL == KALMAN_MODEL::SPP_KINEMATIC || m_KALMAN_MODEL == KALMAN_MODEL::PPP_KINEMATIC)
    {
        m_Qwk_1(0,0) = m_xyz_dynamic_Qw * interval_cur * interval_cur;
        m_Qwk_1(1,1) = m_xyz_dynamic_Qw * interval_cur * interval_cur;
        m_Qwk_1(2,2) = m_xyz_dynamic_Qw * interval_cur * interval_cur;
    }

	//Rk_1 initialization is in place to determine that there is no change in the number of satellites
	isInitPara = true;//No longer initialized after

//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/B.csv", B);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/TEMPP.csv", temp_p);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/L.csv", L);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/P.csv", m_Pk_1);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/Q.csv", m_Qwk_1);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/TEMPPA.csv", temp_p_all);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/TRAN.csv", tran_mat);

//    //2021.01.08 23z
    if(global_cf::ini_snx)
    {
        m_snx_initialpos.readsnx();

        bool snx_flag = false;
        QVector< snx_single > snx_pos;
        snx_single snx_temp;

        snx_temp.code = currEpoch.at(0).markname;
        snx_temp.type = "STAX";
        snx_pos.append(snx_temp);
        snx_temp.type = "STAY";
        snx_pos.append(snx_temp);
        snx_temp.type = "STAZ";
        snx_pos.append(snx_temp);

        snx_flag = m_snx_initialpos.getinitialpos(snx_pos);

        if(snx_flag)
        {
            double x_snx = 0.0, y_snx = 0.0, z_snx = 0.0,
                   p_x_snx = 0.0, p_y_snx = 0.0, p_z_snx = 0.0;
            x_snx = snx_pos.at(0).value - m_SPP_Pos[0];
            y_snx = snx_pos.at(1).value - m_SPP_Pos[1];
            z_snx = snx_pos.at(2).value - m_SPP_Pos[2];

            p_x_snx = snx_pos.at(0).std_dev*snx_pos.at(0).std_dev;
            p_y_snx = snx_pos.at(1).std_dev*snx_pos.at(1).std_dev;
            p_z_snx = snx_pos.at(2).std_dev*snx_pos.at(2).std_dev;

            int row_cf = B.rows() + 3, col_cf = B.cols();
            MatrixXd b_snx, p_snx, l_snx;
            b_snx.resize(row_cf,col_cf);   p_snx.resize(row_cf,row_cf);   l_snx.resize(row_cf,1);
            b_snx.setZero();               p_snx.setZero();               l_snx.setZero();
            for(int cf_i = 0; cf_i < length_L; cf_i++)
            {
                l_snx(cf_i,0) = L(cf_i);

                p_snx(cf_i,cf_i) = temp_p(cf_i,cf_i);

                for(int cf_j = 0; cf_j < col_cf; cf_j++)
                {
                    b_snx(cf_i,cf_j) = B(cf_i,cf_j);  
                }
            }
            l_snx(length_L + 0,0) = x_snx;
            l_snx(length_L + 1,0) = y_snx;
            l_snx(length_L + 2,0) = z_snx;
            p_snx(length_L + 0,length_L + 0) = 1.0/p_x_snx;
            p_snx(length_L + 1,length_L + 1) = 1.0/p_y_snx;
            p_snx(length_L + 2,length_L + 2) = 1.0/p_z_snx;
            b_snx(length_L + 0,0) = 1;
            b_snx(length_L + 1,1) = 1;
            b_snx(length_L + 2,2) = 1;

//            bool cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/l_snx.csv", l_snx);
//            cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/b_snx.csv", b_snx);
//            cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/p_snx.csv", p_snx);
//            cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/B.csv", B);
//            cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/L.csv", L);
//            cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/temp_p.csv", temp_p);

            m_Xk_1 = (b_snx.transpose()*p_snx*b_snx).inverse()*b_snx.transpose()*p_snx*l_snx;
            m_init_Xk = m_Xk_1;

            m_Pk_1 = (b_snx.transpose()*p_snx*b_snx).inverse();
        }
    }

    if(global_cf::ini_csv)
    {
        double x_para[4],p_x_para[4];
        int location[4];
        QString filepath = global_cf::filepath_ini;

        QFile file(filepath);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            int cf_i = 0;
            while(!file.atEnd())
            {
                QString temp_para = file.readLine();
                int n = temp_para.length();

                for(int i=0,k=0;i<n;i++)
                {
                    if(temp_para.at(i)==',')
                        location[k]=i,k++;
                    if(k == 4)
                        break;
                }

                x_para[cf_i] = temp_para.mid(location[3]+1,n-2).toDouble();
                if(cf_i == 0)
                    p_x_para[cf_i] = temp_para.mid(0,location[0]).toDouble();
                else if(cf_i == 1)
                    p_x_para[cf_i] = temp_para.mid(location[0]+1,location[1]-location[0]-1).toDouble();
                else
                    p_x_para[cf_i] = temp_para.mid(location[1]+1,location[2]-location[1]-1).toDouble();

                cf_i++;
                if(cf_i == 4)
                    break;
            }
            file.close();
        }

        int row_cf = B.rows() + 3, col_cf = B.cols();
        MatrixXd b_para, p_para, l_para;
        b_para.resize(row_cf,col_cf);   p_para.resize(row_cf,row_cf);   l_para.resize(row_cf,1);
        b_para.setZero();               p_para.setZero();               l_para.setZero();
        for(int cf_i = 0; cf_i < length_L; cf_i++)
        {
            l_para(cf_i,0) = L(cf_i);

            p_para(cf_i,cf_i) = temp_p(cf_i,cf_i);

            for(int cf_j = 0; cf_j < col_cf; cf_j++)
            {
                b_para(cf_i,cf_j) = B(cf_i,cf_j);
            }
        }
        for(int cf_i = 0; cf_i < 3; cf_i++)
        {
            l_para(length_L + cf_i,0) = x_para[cf_i] - m_SPP_Pos[cf_i];
            p_para(length_L + cf_i,length_L + cf_i) = 1.0/p_x_para[cf_i];
            b_para(length_L + cf_i,cf_i) = 1;
        }

//        bool cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/l_para.csv", l_para);
//        cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/b_para.csv", b_para);
//        cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/p_para.csv", p_para);
//        cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/B.csv", B);
//        cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/L.csv", L);
//        cf = m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/temp_p.csv", temp_p);

        m_Xk_1 = (b_para.transpose()*p_para*b_para).inverse()*b_para.transpose()*p_para*l_para;
        m_init_Xk = m_Xk_1;

        m_Pk_1 = (b_para.transpose()*p_para*b_para).inverse();

    }

//USELESS
//    double temp_q = 0.0;
//    int length_epoch = 0;
//    if(currEpoch.at(0).SSDPPP)
//    {
//        length_epoch = currEpoch.length() - sys_num_cur;
//        for(int cf_i = 0; cf_i < length_epoch; cf_i++)
//            temp_q = temp_q + m_Pk_1(cf_i + 4,cf_i + 4);
//    }
//    else
//    {
//        length_epoch = currEpoch.length();
//        for(int cf_i = 0; cf_i < length_epoch; cf_i++)
//            temp_q = temp_q + m_Pk_1(cf_i + 5,cf_i + 5);

//    }
//    m_amb_Pk = temp_q/length_epoch;



//    int a = 23;
}

//Initialize Kalman
void QKalmanFilter::initKalman_NoCombination(QVector< SatlitData > &currEpoch,MatrixXd &B,VectorXd &L)
{
    int epochLenLB = currEpoch.length();

    //Fk_1 initialization
    switch (m_KALMAN_MODEL) {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        m_Fk_1.resize(m_const_param+epochLenLB, m_const_param+epochLenLB);
        m_Fk_1.setIdentity();
        break;
    case KALMAN_MODEL::PPP_KINEMATIC:
    case KALMAN_MODEL::PPP_STATIC:
        m_Fk_1.resize(m_const_param+3*epochLenLB,m_const_param+3*epochLenLB);
        m_Fk_1.setIdentity();
        break;
    default:
        break;
    }
    //Xk_1 pesodurange init  Initialization, least squares initialization
    switch (m_KALMAN_MODEL) {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        m_Xk_1.resize(m_const_param+epochLenLB);
        m_Xk_1.setZero();
        break;
    case KALMAN_MODEL::PPP_KINEMATIC:
    case KALMAN_MODEL::PPP_STATIC:
        m_Xk_1.resize(3*epochLenLB+m_const_param);
        m_Xk_1.setZero();
        break;
    default:
        ErroTrace("QKalmanFilter::initKalman Bad.");
        break;
    }
    m_Xk_1 = (B.transpose()*B).inverse()*B.transpose()*L;
    m_init_Xk = m_Xk_1;
    //Initialization state covariance matrix Pk_1 initialization
    switch (m_KALMAN_MODEL) {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        m_Pk_1.resize(m_const_param+epochLenLB, m_const_param+epochLenLB);
        m_Pk_1.setZero();
        m_Pk_1(0,0) = m_xyz_dynamic_Pk;m_Pk_1(1,1) = m_xyz_dynamic_Pk;m_Pk_1(2,2) = m_xyz_dynamic_Pk;
        for(int i = 3; i < m_const_param;i++) m_Pk_1(i,i) = m_clk_Pk;// for clock
        for(int i = m_const_param; i < epochLenLB+m_const_param;i++) m_Pk_1(i,i) = m_ion_Pk;// for ION
        break;
    case KALMAN_MODEL::PPP_STATIC:
    case KALMAN_MODEL::PPP_KINEMATIC:
        m_Pk_1.resize(m_const_param+3*epochLenLB,m_const_param+3*epochLenLB);
        m_Pk_1.setZero();
        m_Pk_1(0,0) = m_xyz_dynamic_Pk;m_Pk_1(1,1) = m_xyz_dynamic_Pk;m_Pk_1(2,2) = m_xyz_dynamic_Pk;
        m_Pk_1(3,3) = m_zwd_Pk;
        for(int i = 4; i < m_const_param;i++) m_Pk_1(i,i) = m_clk_Pk; // for clock
        for(int i = m_const_param; i < m_const_param+epochLenLB;i++) m_Pk_1(i,i) = m_ion_Pk; // for ION
        for (int i = m_const_param+epochLenLB;i < m_const_param+3*epochLenLB;i++)	m_Pk_1(i,i) = m_amb_Pk;// for Ambiguity
        break;
    default:
        ErroTrace("QKalmanFilter::initKalman Bad.");
        break;
    }

    //Qk_1 system noise initialization
    switch (m_KALMAN_MODEL) {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        m_Qwk_1.resize(m_const_param+epochLenLB, m_const_param+epochLenLB);
        m_Qwk_1.setZero();
        for(int i = 3; i < m_const_param;i++) m_Qwk_1(i,i) = m_clk_Qw;// for clock
        for(int i = m_const_param; i < epochLenLB+m_const_param;i++) m_Pk_1(i,i) = m_ion_Qw;// for ION
        break;
    case KALMAN_MODEL::PPP_KINEMATIC:
    case KALMAN_MODEL::PPP_STATIC:
        m_Qwk_1.resize(m_const_param+3*epochLenLB,m_const_param+3*epochLenLB);
        m_Qwk_1.setZero();
        m_Qwk_1(3,3) = m_zwd_Qw * interval_cur;//Zenith tropospheric residual variance
        for(int i = 4; i < m_const_param;i++) m_Qwk_1(i,i) = m_clk_Qw; // for clock
        for(int i = m_const_param; i < m_const_param+epochLenLB;i++) m_Qwk_1(i,i) = m_ion_Qw; // for ION
        break;
    default:
        ErroTrace("QKalmanFilter::initKalman Bad.");
        break;
    }

    if(m_KALMAN_MODEL == KALMAN_MODEL::SPP_KINEMATIC || m_KALMAN_MODEL == KALMAN_MODEL::PPP_KINEMATIC)
    {
        m_Qwk_1(0,0) = m_xyz_dynamic_Qw;
        m_Qwk_1(1,1) = m_xyz_dynamic_Qw;
        m_Qwk_1(2,2) = m_xyz_dynamic_Qw;
    }

    //Rk_1 initialization is in place to determine that there is no change in the number of satellites
    isInitPara = true;//No longer initialized after
}

//Change the Kalman parameter size (only PPP can change paramater)
void QKalmanFilter::changeKalmanPara(QVector< SatlitData > &preEpoch, QVector< SatlitData > &epochSatlitData,QVector< int >oldPrnFlag,MatrixXd B,VectorXd L)
{//Adding SSD parameter modification 2020.11.11 23z

	int epochLenLB = epochSatlitData.length();
    VectorXd tempXk_1;
    MatrixXd tempPk_1;

    tempXk_1 = m_Xk_1;
    tempPk_1 = m_Pk_1;

    if(epochSatlitData.at(0).SSDPPP)
    {
        //public
        m_Fk_1.resize(4 + epochLenLB - sys_num_cur,4 + epochLenLB - sys_num_cur);
        m_Fk_1.setZero();
        m_Fk_1.setIdentity();

        m_Xk_1.resize(4 + epochLenLB - sys_num_cur);
        m_Xk_1.setZero();
        for (int i = 0;i < 4;i++)
            m_Xk_1(i) = tempXk_1(i);

        m_Qwk_1.resize(4 + epochLenLB - sys_num_cur,4 + epochLenLB - sys_num_cur);
        m_Qwk_1.setZero();
        m_Qwk_1(3,3) = m_zwd_Qw * interval_cur;
        for(int cf_i = 0; cf_i < (epochLenLB - sys_num_cur); cf_i++)
            m_Qwk_1((cf_i + 4),(cf_i + 4)) = m_amb_SSD * interval_cur;
        if(m_KALMAN_MODEL == KALMAN_MODEL::PPP_KINEMATIC)
        {
            m_Qwk_1(0,0) = m_xyz_dynamic_Qw * interval_cur * interval_cur;
            m_Qwk_1(1,1) = m_xyz_dynamic_Qw * interval_cur * interval_cur;
            m_Qwk_1(2,2) = m_xyz_dynamic_Qw * interval_cur * interval_cur;
        }

        m_Pk_1.resize(4 + epochLenLB - sys_num_cur,4 + epochLenLB - sys_num_cur);
        m_Pk_1.setZero();
        for (int i = 0;i < 4;i++)
            for (int j = 0;j < 4;j++)
                m_Pk_1(i,j) = tempPk_1(i,j);

        int sys_num_pre = 1;   QString sys_pre = "";
        sys_pre = preEpoch.at(0).SatType;
        for(int cf_num = 1; cf_num < preEpoch.length(); cf_num++)
        {
            if(!sys_pre.contains(preEpoch.at(cf_num).SatType))
                sys_num_pre++,sys_pre = sys_pre + preEpoch.at(cf_num).SatType;
        }

        //check sys number and recover sits
        int num_single_sys[4] = {0};
        for(int cf_j = sys_cur.length() - 1; cf_j >= 0; cf_j--)
        {
            num_single_sys[cf_j] = epochSatlitData.at(0).prn_referencesat[2*cf_j + 1] + 1;
            //refsats_cur
            oldPrnFlag.remove(epochSatlitData.at(0).prn_referencesat[2*cf_j + 1]);

            //refsats_pre
            int prn_sit_pre = 0, prn_temp = 0;
            prn_sit_pre = preEpoch.at(0).prn_referencesat[2*cf_j + 1];
            prn_temp = preEpoch.at(0).prn_referencesat[2*cf_j + 3];

            for(int cf_i = 0; cf_i < oldPrnFlag.length(); cf_i++)
            {
                int temp_flag = oldPrnFlag.at(cf_i);
                if(temp_flag > prn_sit_pre)
                    oldPrnFlag[cf_i] = temp_flag - 1;

                if(temp_flag == prn_sit_pre || (temp_flag == prn_temp && prn_temp != 0))
                    oldPrnFlag[cf_i] = -1;
            }
        }

        bool ref_changed;
        for(int cf_sys = 0; cf_sys < sys_num_cur; cf_sys++)
        {
            int add_temp = 0, prn_ref_pre = 0, pre_sit = -923;
            if(cf_sys > 0)
                add_temp = num_single_sys[cf_sys - 1] - cf_sys;
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
            if(pre_sit == -923) prn_ref_pre = -923;
            else prn_ref_pre = preEpoch.at(0).prn_referencesat[2*pre_sit];
            //is resat changed
            ref_changed = false;
            if(epochSatlitData.at(0).prn_referencesat[2*cf_sys] != prn_ref_pre)
                ref_changed = true;

            if(ref_changed)
            {
                double diff_refsats = 0.0;
                int ref_cur_prn = 0, ref_cur_sit_pre = 0, ref_cur_sit_cur = 0;
                ref_cur_prn = epochSatlitData.at(0).prn_referencesat[2*cf_sys];
                ref_cur_sit_cur = epochSatlitData.at(0).prn_referencesat[2*cf_sys + 1];
                for(int cf_i = 0; cf_i < preEpoch.length(); cf_i++)
                {
                    if(ref_cur_prn == preEpoch.at(cf_i).PRN
                            && epochSatlitData.at(ref_cur_sit_cur).SatType == preEpoch.at(cf_i).SatType)
                    {
                        ref_cur_sit_pre = cf_i;
                        break;
                    }
                }
                if(pre_sit == -923)
                {
                    for(int cf_i = 0; cf_i < oldPrnFlag.length(); cf_i++)
                    {
                        oldPrnFlag[cf_i] = -1;
                    }
                }
                else
                    diff_refsats = tempXk_1(4 + ref_cur_sit_pre - pre_sit);

                //m_x
                for(int i = add_temp;i < (num_single_sys[cf_sys] - 1 - cf_sys);i++)
                {
                    if(oldPrnFlag.at(i) != -1)//Save the old satellite ambiguity
                        m_Xk_1(4 + i) = tempXk_1(oldPrnFlag.at(i) + 4) - diff_refsats;
                    else
                    {//New satellite ambiguity calculation
                        int numofref = epochSatlitData.at(0).prn_referencesat[2*cf_sys + 1];
                        SatlitData oneStalit = epochSatlitData.at(i);
                        SatlitData onestalit_ref = epochSatlitData.at(numofref);
                        m_Xk_1(4 + i) = (oneStalit.PP3 - oneStalit.LL3)/M_GetLamta3(oneStalit.Frq[0],oneStalit.Frq[1])
                                - (onestalit_ref.PP3 - onestalit_ref.LL3)/M_GetLamta3(onestalit_ref.Frq[0],onestalit_ref.Frq[1]);
                    }
                }
                //m_p
                for (int n = add_temp; n < (num_single_sys[cf_sys] - 1 - cf_sys);n++)
                {
                    int flag = oldPrnFlag.at(n);
                    if (flag != -1)
                    {
                        flag+=4;
                        for (int i = 0;i < tempPk_1.cols();i++)
                        {
                            if (i < 4)
                            {
                                m_Pk_1(n + 4,i) = tempPk_1(flag,i);
                                m_Pk_1(i,n + 4) = tempPk_1(i,flag);
                            }
                            else
                            {
                                int findCols = i - 4,saveFlag = -1;
                                for (int m = 0;m < oldPrnFlag.length();m++)
                                {
                                    if (findCols == oldPrnFlag.at(m))
                                    {
                                        saveFlag = m;
                                        break;
                                    }
                                }
                                if (saveFlag!=-1)
                                {
                                    m_Pk_1(n + 4,saveFlag + 4) = tempPk_1(flag,i);
                                }

                            }
                        }

                    }
                    else
                    {
                        m_Pk_1(n + 4,n + 4) = m_amb_Pk;
                    }
                }
            }
//            else if(ref_changed)
//            {
//                //refsat changed
//                //m_x
//                for (int i = add_temp;i < (num_single_sys[cf_sys] - 1 - cf_sys);i++)
//                {
//                    int numofref = epochSatlitData.at(0).prn_referencesat[2*cf_sys + 1];
//                    SatlitData oneStalit = epochSatlitData.at(i);
//                    SatlitData onestalit_ref = epochSatlitData.at(numofref);
//                    if(i == numofref) continue;

//                    m_Xk_1(4 + i) = (oneStalit.PP3 - oneStalit.LL3)/M_GetLamta3(oneStalit.Frq[0],oneStalit.Frq[1])
//                            - (onestalit_ref.PP3 - onestalit_ref.LL3)/M_GetLamta3(onestalit_ref.Frq[0],onestalit_ref.Frq[1]);
//                }
//                //m_p
//                for (int n = add_temp; n < (num_single_sys[cf_sys] - 1 - cf_sys);n++)
//                    m_Pk_1(n + 4,n + 4) = m_amb_Pk;
//            }
            else
            {
                //m_x
                for (int i = add_temp;i < (num_single_sys[cf_sys] - 1 - cf_sys);i++)
                {
                    if(oldPrnFlag.at(i) != -1)//Save the old satellite ambiguity
                        m_Xk_1(4 + i) = tempXk_1(oldPrnFlag.at(i) + 4);
                    else
                    {//New satellite ambiguity calculation
                        int numofref = epochSatlitData.at(0).prn_referencesat[2*cf_sys + 1];
                        SatlitData oneStalit = epochSatlitData.at(i);
                        SatlitData onestalit_ref = epochSatlitData.at(numofref);
                        m_Xk_1(4 + i) = (oneStalit.PP3 - oneStalit.LL3)/M_GetLamta3(oneStalit.Frq[0],oneStalit.Frq[1])
                                - (onestalit_ref.PP3 - onestalit_ref.LL3)/M_GetLamta3(onestalit_ref.Frq[0],onestalit_ref.Frq[1]);
                    }
                }
                //m_p
                for (int n = add_temp; n < (num_single_sys[cf_sys] - 1 - cf_sys);n++)
                {
                    int flag = oldPrnFlag.at(n);
                    if (flag != -1)
                    {
                        flag+=4;
                        for (int i = 0;i < tempPk_1.cols();i++)
                        {
                            if (i < 4)
                            {
                                m_Pk_1(n + 4,i) = tempPk_1(flag,i);
                                m_Pk_1(i,n + 4) = tempPk_1(i,flag);
                            }
                            else
                            {
                                int findCols = i - 4,saveFlag = -1;
                                for (int m = 0;m < oldPrnFlag.length();m++)
                                {
                                    if (findCols == oldPrnFlag.at(m))
                                    {
                                        saveFlag = m;
                                        break;
                                    }
                                }
                                if (saveFlag!=-1)
                                {
                                    m_Pk_1(n + 4,saveFlag + 4) = tempPk_1(flag,i);
                                }

                            }//if (i < 5)
                        }//for (int i = 0;i < tempPk_1.cols();i++)
                    }
                    else
                        m_Pk_1(n + 4,n + 4) = m_amb_Pk;
                }

            }

//            m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/P.csv", m_Pk_1);
//            m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/tempP.csv", tempPk_1);
//            m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/X.csv", m_Xk_1);
//            m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/tempX.csv", tempXk_1);
            int cf = 23;

        }//for

    }
    else
    {
        //Fk_1(4,4) = 0;//Static PPP has only a clock difference of 0
        m_Fk_1.resize(m_const_param+epochLenLB,m_const_param+epochLenLB);
        m_Fk_1.setZero();
        m_Fk_1.setIdentity(m_const_param+epochLenLB,m_const_param+epochLenLB);

        m_Xk_1.resize(epochLenLB+m_const_param);
        m_Xk_1.setZero();
        for (int i = 0;i < m_const_param;i++)
            m_Xk_1(i) = tempXk_1(i);
        //Xk.resize(epochLenLB+5);

        for (int i = 0;i<epochLenLB;i++)
        {
            if (oldPrnFlag.at(i)!=-1)//Save the old satellite ambiguity
                m_Xk_1(m_const_param+i) = tempXk_1(oldPrnFlag.at(i)+m_const_param);
            else
            {//New satellite ambiguity calculation
                SatlitData oneStalit = epochSatlitData.at(i);
                m_Xk_1(m_const_param+i) = (oneStalit.PP3 - oneStalit.LL3)/M_GetLamta3(oneStalit.Frq[0],oneStalit.Frq[1]);
            }
        }

        //Qk_1 system noise will not be updated, system noise is not measurable
        m_Qwk_1.resize(m_const_param+epochLenLB,m_const_param+epochLenLB);
        m_Qwk_1.setZero();
        if(m_KALMAN_MODEL == KALMAN_MODEL::PPP_KINEMATIC)
        {
            m_Qwk_1(0,0) = m_xyz_dynamic_Qw * interval_cur * interval_cur;
            m_Qwk_1(1,1) = m_xyz_dynamic_Qw * interval_cur * interval_cur;
            m_Qwk_1(2,2) = m_xyz_dynamic_Qw * interval_cur * interval_cur;
        }
        m_Qwk_1(3,3) = m_zwd_Qw * interval_cur;//Zenith tropospheric residual variance
        for(int i = 4; i < m_const_param;i++) m_Qwk_1(i,i) = m_clk_Qw; // for clock

        //Reset Rk_1 observation noise matrix (reset on the outside, no need to repeat reset here)
        //The saved state covariance matrix Pk_1 is increased or decreased (here is more complicated, the main idea is to take out old satellite data, and initialize the new satellite data)

        m_Pk_1.resize(m_const_param+epochLenLB, m_const_param+epochLenLB);
        m_Pk_1.setZero();
        //If the number of satellites ces
        for (int i = 0;i < m_const_param;i++)
            for (int j = 0;j < m_const_param;j++)
                m_Pk_1(i,j) = tempPk_1(i,j);

        for (int n = 0; n < epochLenLB;n++)
        {
            int flag = oldPrnFlag.at(n);
            if ( flag != -1)//Description: The previous epoch contains this satellite data and needs to be taken from tempPk_1
            {
                flag+=m_const_param;//The number of rows of this satellite in the original data tempPk_1
                for (int i = 0;i < tempPk_1.cols();i++)
                {//Take out from tempPk_1 and skip the data with oldPrnFlag -1
                    if (i < m_const_param)
                    {
                        m_Pk_1(n+m_const_param,i) = tempPk_1(flag,i);
                        m_Pk_1(i,n+m_const_param) = tempPk_1(i,flag);
                    }
                    else
                    {
                        int findCols = i - m_const_param,saveFlag = -1;
                        //Find if the data exists in the old linked list and where it will be saved
                        for (int m = 0;m < oldPrnFlag.length();m++)
                        {
                            if (findCols == oldPrnFlag.at(m))
                            {
                                saveFlag = m;
                                break;
                            }
                        }
                        if (saveFlag!=-1)
                        {
                            m_Pk_1(n+m_const_param,saveFlag+m_const_param) = tempPk_1(flag,i);
                            //Pk_1(saveFlag+5,n+5) = tempPk_1(i,flag);
                        }

                    }//if (i < 5)
                }//for (int i = 0;i < tempPk_1.cols();i++)

            }
            else
            {
                m_Pk_1(n+m_const_param,n+m_const_param) = m_amb_Pk;
            }
        }//Pk_1 saves the data
    }

    m_VarChang = true;
}


//Ce the Kalman parameter size (only PPP can ce paramater)
void QKalmanFilter::changeKalmanPara_NoCombination( QVector< SatlitData > &epochSatlitData,QVector< int >oldPrnFlag, int preEpochLen)
{
    int epochLenLB = epochSatlitData.length();

    m_Fk_1.resize(m_const_param+3*epochLenLB,m_const_param+3*epochLenLB);
    m_Fk_1.setZero();
    m_Fk_1.setIdentity();
    //Fk_1(4,4) = 0;//Static PPP has only a clock difference of 0
    //Xk_1 ce
    VectorXd tempXk_1 = m_Xk_1;
    m_Xk_1.resize(3*epochLenLB+m_const_param);
    m_Xk_1.setZero();
    for (int i = 0;i < m_const_param;i++)
        m_Xk_1(i) = tempXk_1(i);
    for (int i = 0;i<epochLenLB;i++)
    {
        if (oldPrnFlag.at(i)!=-1)//Save the old satellite ION and L1 L2 ambiguity
        {
            m_Xk_1(m_const_param+i) = tempXk_1(oldPrnFlag.at(i)+m_const_param);// for ION
            m_Xk_1(m_const_param+epochLenLB+i) = tempXk_1(oldPrnFlag.at(i)+preEpochLen+m_const_param);// for L1 ambiguity
            m_Xk_1(m_const_param+2*epochLenLB+i) = tempXk_1(oldPrnFlag.at(i)+2*preEpochLen+m_const_param);// for L1 ambiguity
        }
        else
        {//New satellite ambiguity calculation
            SatlitData oneStalit = epochSatlitData.at(i);
            m_Xk_1(m_const_param+epochLenLB+i) = (oneStalit.C1 - oneStalit.L1)/oneStalit.Frq[0];// new L1 ambiguity
            m_Xk_1(m_const_param+2*epochLenLB+i) = (oneStalit.C2 - oneStalit.L2)/oneStalit.Frq[1];// new L2 ambiguity
        }
    }
//    m_matrix.writeCSV("newXk.csv", m_Xk_1);
//    m_matrix.writeCSV("oldXk.csv", tempXk_1);
    //Qk_1 system noise will not be updated, system noise is not measurable
    m_Qwk_1.resize(m_const_param+3*epochLenLB,m_const_param+3*epochLenLB);
    m_Qwk_1.setZero();
    if(m_KALMAN_MODEL == KALMAN_MODEL::PPP_KINEMATIC)
    {
        m_Qwk_1(0,0) = m_xyz_dynamic_Qw;
        m_Qwk_1(1,1) = m_xyz_dynamic_Qw;
        m_Qwk_1(2,2) = m_xyz_dynamic_Qw;
    }
    m_Qwk_1(3,3) = m_zwd_Qw * interval_cur;//Zenith tropospheric residual variance
    for(int i = 4; i < m_const_param;i++) m_Qwk_1(i,i) = m_clk_Qw; // for clock
    for(int i = m_const_param; i < m_const_param+epochLenLB;i++) m_Qwk_1(i,i) = m_ion_Qw; // for ION
    //Reset Rk_1 observation noise matrix (reset on the outside, no need to repeat reset here)
    //The saved state covariance matrix Pk_1 is increased or decreased (here is more complicated, the main idea is to take out old satellite data, and initialize the new satellite data)
    MatrixXd tempPk_1 = m_Pk_1;
    m_Pk_1.resize(m_const_param+3*epochLenLB, m_const_param+3*epochLenLB);
    m_Pk_1.setZero();
    //If the number of satellites ces
    for (int i = 0;i < m_const_param;i++)
        for (int j = 0;j < m_const_param;j++)
            m_Pk_1(i,j) = tempPk_1(i,j);

    for (int n = 0; n < epochLenLB;n++)
    {
        int flag = oldPrnFlag.at(n);
        if ( flag != -1)//Description: The previous epoch contains this satellite data and needs to be taken from tempPk_1
        {
            flag+=m_const_param;//The number of rows of this satellite in the original data tempPk_1
            for (int i = 0;i < tempPk_1.cols();i++)
            {//Take out from tempPk_1 and skip the data with oldPrnFlag -1
                if (i < m_const_param)
                {
                    // for ION
                    m_Pk_1(n+m_const_param,i) = tempPk_1(flag,i);
                    m_Pk_1(i,n+m_const_param) = tempPk_1(i,flag);
                    // for L1 AMB
                    m_Pk_1(n+m_const_param+epochLenLB,i) = tempPk_1(flag+preEpochLen,i);
                    m_Pk_1(i,n+m_const_param+epochLenLB) = tempPk_1(i,flag+preEpochLen);
                    // for L2 AMB
                    m_Pk_1(n+m_const_param+2*epochLenLB,i) = tempPk_1(flag+2*preEpochLen,i);
                    m_Pk_1(i,n+m_const_param+2*epochLenLB) = tempPk_1(i,flag+2*preEpochLen);
                }
                else
                {
                    int findCols = i - m_const_param,saveFlag = -1;
                    //Find if the data exists in the old linked list and where it will be saved
                    for (int m = 0;m < oldPrnFlag.length();m++)
                    {
                        if (findCols == oldPrnFlag.at(m))
                        {
                            saveFlag = m;
                            break;
                        }
                    }
                    if (saveFlag!=-1)
                    {
//                        qDebug() <<"(" << flag << "," << i << ") -> " <<" (" << n+m_const_param << "," << saveFlag+m_const_param << ")";
                        m_Pk_1(n+m_const_param,saveFlag+m_const_param) = tempPk_1(flag,i);// for ION
                        m_Pk_1(n+m_const_param,saveFlag+m_const_param+epochLenLB) = tempPk_1(flag,i+preEpochLen);
                        m_Pk_1(n+m_const_param,saveFlag+m_const_param+2*epochLenLB) = tempPk_1(flag,i+2*preEpochLen);

                        m_Pk_1(n+m_const_param+epochLenLB,saveFlag+m_const_param) = tempPk_1(flag+preEpochLen,i);// for L1 AMB
                        m_Pk_1(n+m_const_param+epochLenLB,saveFlag+m_const_param+epochLenLB) = tempPk_1(flag+preEpochLen,i+preEpochLen);// for L1 AMB
                        m_Pk_1(n+m_const_param+epochLenLB,saveFlag+m_const_param+2*epochLenLB) = tempPk_1(flag+preEpochLen,i+2*preEpochLen);// for L1 AMB

                        m_Pk_1(n+m_const_param+2*epochLenLB,saveFlag+m_const_param) = tempPk_1(flag+2*preEpochLen,i);// for L2 AMB
                        m_Pk_1(n+m_const_param+2*epochLenLB,saveFlag+m_const_param+epochLenLB) = tempPk_1(flag+2*preEpochLen,i+preEpochLen);// for L2 AMB
                        m_Pk_1(n+m_const_param+2*epochLenLB,saveFlag+m_const_param+2*epochLenLB) = tempPk_1(flag+2*preEpochLen,i+2*preEpochLen);// for L2 AMB
                    }

                }//if (i < 5)
            }//for (int i = 0;i < tempPk_1.cols();i++)

        }
        else
        {
            m_Pk_1(n+m_const_param,n+m_const_param) = m_ion_Pk;// for ION
            m_Pk_1(n+m_const_param+epochLenLB,n+m_const_param+epochLenLB) = m_amb_Pk;// for L1 amb
            m_Pk_1(n+m_const_param+2*epochLenLB,n+m_const_param+2*epochLenLB) = m_amb_Pk;// for L2 amb

        }
    }//Pk_1 saves the data

//    m_matrix.writeCSV("newPk.csv", m_Pk_1);
//    m_matrix.writeCSV("oldPk.csv", tempPk_1);
    m_VarChang = true;
}


//First version MrOu Kalman
// PBk is Weight matrix
void QKalmanFilter::KalmanforStaticOu(MatrixXd Bk,VectorXd Lk,MatrixXd F,MatrixXd Qwk,MatrixXd PBk,VectorXd &tXk_1,MatrixXd &tPk_1)
{
    //Time update
    VectorXd Xkk_1 = F*tXk_1,LVk;
    MatrixXd Pkk_1 = F*tPk_1*F.transpose() + Qwk;
    LVk = Lk - Bk*Xkk_1;

    //Filter update
    MatrixXd Mk, Nk , Pkk_1_inv;
    Nk = Bk.transpose()*PBk*Bk;
    Pkk_1_inv = Pkk_1.inverse();
    Mk = Nk + Pkk_1_inv;// Symmetry is not guaranteed
//    Mk = (Nk + Nk.transpose()) / 2 + (Pkk_1_inv + Pkk_1_inv.transpose()) / 2;// Symmetry is not guaranteed
    tPk_1 = Mk.inverse();
    VectorXd delatXkd = tPk_1*(Bk.transpose())*PBk*LVk;
    tXk_1 = Xkk_1 + delatXkd;
}

//Third version use to ce Kalman
void QKalmanFilter::KalmanforStatic(MatrixXd Bk,VectorXd Lk,MatrixXd F,MatrixXd Qwk,MatrixXd Rk,VectorXd &tXk_1,
                                    MatrixXd &tPk_1,QVector< SatlitData > &preEpoch,QVector< SatlitData > &currEpoch)
{
    //2020.12.07 23z
    //Update filter with fixed ambiguity
    if(currEpoch.at(0).AR_succeed)
    {
        QVector< SatlitData > temp_epoch;
        for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
        {
            bool temp_flag = true;
            for(int cf_j = 0; cf_j < sys_num_cur; cf_j++)
            {
                if(cf_i == currEpoch.at(0).prn_referencesat[2*cf_j + 1])
                {
                    temp_flag = false;
                    break;
                }
            }
            if(temp_flag)
                temp_epoch.append(currEpoch.at(cf_i));
        }

        //get fixed information
        int num_fixedsats = 0,length_epoch = 0;
        length_epoch = temp_epoch.length();
        //flag，0：float  1：fixed
        VectorXd fixed_flag, fixedSSD_value;
        fixed_flag.resize(length_epoch), fixedSSD_value.resize(length_epoch);
        fixed_flag.setZero(), fixedSSD_value.setZero();

        bool temp_fix_and_hold = false;
        for(int cf_i = 0; cf_i < length_epoch; cf_i++)
        {
            if(temp_epoch.at(cf_i).nl_fixed_flag)
            {
                if(temp_epoch.at(cf_i).SSD_fixed == 0)
                {
                    temp_epoch[cf_i].nl_fixed_flag = false;
                    temp_epoch[cf_i].times_nl = 0;
                }
                else
                {
                    num_fixedsats++;
                    fixedSSD_value[cf_i] = temp_epoch.at(cf_i).SSD_fixed;
                    fixed_flag[cf_i] = 1;
                    temp_fix_and_hold = true;
                }

            }
        }

        for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
            currEpoch[cf_i].num_fixedsats = num_fixedsats;

        if(temp_fix_and_hold)
        {
            VectorXd fixed_value, fixed_flag_sit;
            fixed_value.resize(num_fixedsats);       fixed_value.setZero();
            fixed_flag_sit.resize(num_fixedsats);   fixed_flag_sit.setZero();
            for(int cf_i = 0, cf_j = 0; cf_i < num_fixedsats; cf_i++, cf_j++)
            {//For the convenience of assignment, a vector with all fixed solutions is obtained 2020.12.07 23z
                if(fixedSSD_value[cf_j] != 0)
                {
                    fixed_value[cf_i] = fixedSSD_value[cf_j];
                    fixed_flag_sit[cf_i] = cf_j;
                }
                else
                    cf_i--;
            }

            //Save the original filter parameters
            MatrixXd temp_B = Bk, temp_R = Rk;
            VectorXd temp_L = Lk;

            //Add virtual observations
            int rows_a = 2*length_epoch + num_fixedsats, rows_b = length_epoch + 4;
            Bk.resize(rows_a,rows_b);   Bk.setZero();
            Rk.resize(rows_a,rows_a);   Rk.setZero();
            Lk.resize(rows_a);          Lk.setZero();

            for(int cf_i = 0; cf_i < rows_a; cf_i++)
            {
                //L
                if(cf_i < (rows_a - num_fixedsats))
                    Lk[cf_i] = temp_L[cf_i];
                else
                    Lk[cf_i] = fixed_value[cf_i - 2*length_epoch];


                //B
                for(int cf_j = 0; cf_j < rows_b; cf_j++)
                    if(cf_i < (rows_a - num_fixedsats))
                        Bk(cf_i,cf_j) = temp_B(cf_i,cf_j);

                //R
                for(int cf_j = 0; cf_j < rows_a; cf_j++)
                    if(cf_i < (rows_a - num_fixedsats) && cf_j < (rows_a - num_fixedsats))
                        Rk(cf_i,cf_j) = temp_R(cf_i,cf_j);

            }

            //B
            for(int cf_i = 0, cf_j = 0; cf_i < length_epoch; cf_i++)
                if(fixed_flag[cf_i] == 1)
                {
                    Bk((2*length_epoch + cf_j),(4 + cf_i)) = 1;
                    cf_j++;
                }

            //R
            int temp_deno = 1;

            for(int cf_i = 0, cf_j = 0; cf_j < length_epoch; cf_i++, cf_j++)
            {
                double p = 0.0;
                if(currEpoch.at(cf_i).SatType =='G' && global_cf::fix_and_hold) temp_deno = 5;
                else if(global_cf::fix_and_hold) temp_deno = 2;
                else if(currEpoch.at(cf_i).SatType =='G') temp_deno = 2;
                else temp_deno = 1;

                if(fixed_flag[cf_j] == 1)
                {
                    p = temp_R(cf_j,cf_j);
                    Rk((2*length_epoch + cf_i),(2*length_epoch + cf_i)) = p/temp_deno;
                }
                else
                    cf_i--;
            }
//            for(int cf_i = 0; cf_i < num_fixedsats; cf_i++)
//                Rk((2*length_epoch + cf_i),(2*length_epoch + cf_i)) = (1.0/30)*1e-6;

//

//            m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/R.csv", Rk);
//            m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/B.csv", Bk);
//            m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/L.csv", Lk);

            if(global_cf::fix_and_hold)
            {
                int times = 0;
                if(preEpoch.length() == 0)
                    times = currEpoch.at(0).times_hold;
                else
                    times = preEpoch.at(0).times_hold;
                for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                {
                    currEpoch[cf_i].times_hold = times + 1;
                }
            }
            int cfz = 23;
        }

    }

//Kalman
    //Time update
    VectorXd Xkk_1 = F*tXk_1,Vk;
    MatrixXd Pkk_1 = F*tPk_1*F.transpose() + Qwk/M_Zgama_P_square,I,tempKB,Kk;
    //Calculated gain matrix
    Kk = (Pkk_1*Bk.transpose())*((Bk*Pkk_1*Bk.transpose() + Rk).inverse());

//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/x.csv", tXk_1);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/p.csv", tPk_1);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/f.csv", F);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/Q.csv", Qwk);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/R.csv", Rk);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/B.csv", Bk);
//    m_matrix.writeCSV("/home/cfz/data_23z/zcompare/kalman/L.csv", Lk);

    //Filter update
    Vk = Lk - Bk*Xkk_1;
    //Update X
    tXk_1 = Xkk_1 + Kk*Vk;
    //Filtered residual, normal download wave is very small
    VectorXd Vk_temp = Lk - Bk*tXk_1;

    tempKB = Kk*Bk;
    I.resize(tempKB.rows(),tempKB.cols());
    I.setIdentity();
    //Update P (Case I) (this update is extremely unstable)
    tPk_1 = (I - tempKB)*Pkk_1;

    //2020.12.18 23z Prevent the variance matrix from being symmetrical
    tPk_1 = (tPk_1 + tPk_1.transpose())/2;

    //m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/R.csv", Rk);

    //2021.03.09 23z Save filter state（x y z zwd，its variance and covariance）
    if(global_cf::keepornot)
    {
        MatrixXd para;
        para.resize(4,5);
        for(int cf_i = 0; cf_i < 4; cf_i++)
        {
            for(int cf_j = 0; cf_j < 4; cf_j++)
                para(cf_i,cf_j) = tPk_1(cf_i,cf_j);

            if(cf_i < 3)
                para(cf_i,4) = tXk_1(cf_i) + m_SPP_Pos[cf_i];
            else
                para(cf_i,4) = tXk_1(cf_i);

        }

        QString month_para;
        if(currEpoch.at(0).UTCTime.Month < 10)
            month_para = '0' + QString::number(currEpoch.at(0).UTCTime.Month);
        else
            month_para = QString::number(currEpoch.at(0).UTCTime.Month);
        QString day_para;
        if(currEpoch.at(0).UTCTime.Day < 10)
            day_para = '0' + QString::number(currEpoch.at(0).UTCTime.Day);
        else
            day_para = QString::number(currEpoch.at(0).UTCTime.Day);
        QString markname = currEpoch.at(0).markname + QString::number(currEpoch.at(0).UTCTime.Year)
                + month_para + day_para;

        QString temp_filepath_keep = global_cf::filepath_keep + '/' + markname +".csv";
        const char *path_keep = temp_filepath_keep.toLatin1().data();
        m_matrix.writeCSV(path_keep, para);
    }

//    m_matrix.writeCSV("B.csv", Bk);
//    m_matrix.writeCSV("L.csv", Lk);

    //Update P(Case II)
//    MatrixXd Mk_1 = Pkk_1.inverse() + Bk.transpose()*Rk.inverse()*Bk;
//    tPk_1 =Mk_1.inverse();
//    MatrixXd newPk =  0.5*(tPk_1 + tPk_1.transpose());
//    tPk_1 = newPk;
    //printMatrix(tPk_1);
//    tPk_1 = 0.5*(tPk_1 + tPk_1.transpose());	//(In theory, it should be added but added or beating. Ced the original covariance data)
    //printMatrix(tPk_1);


}


////Third version  use to change Kalman
//void QKalmanFilter::KalmanforStatic(MatrixXd Bk,VectorXd Lk,MatrixXd F,MatrixXd Qwk,
//                                    MatrixXd Rk,VectorXd &tXk_1,MatrixXd &tPk_1)
//{
//    //
////    int keepnum = -1;
//    int keepnum = 9;
//    m_matrix.keepMatPricision(Bk,keepnum);
//    m_matrix.keepMatPricision(Lk,keepnum);
//    m_matrix.keepMatPricision(F,keepnum);
//    m_matrix.keepMatPricision(Qwk,keepnum);

//    m_matrix.keepMatPricision(Rk,keepnum);
//    m_matrix.keepMatPricision(tXk_1,keepnum);
//    m_matrix.keepMatPricision(tPk_1,keepnum);
//    //Time update
//    VectorXd Xkk_1 = F*tXk_1,Vk;
//    MatrixXd Pkk_1 = F*tPk_1*F.transpose() + Qwk,I,tempKB,Kk;

//    m_matrix.keepMatPricision(Xkk_1,keepnum);
//    m_matrix.keepMatPricision(Pkk_1,keepnum);

//    //Calculated gain matrix
//    Kk = (Pkk_1*Bk.transpose())*((Bk*Pkk_1*Bk.transpose() + Rk).inverse());
//    m_matrix.keepMatPricision(Kk,keepnum);
//    //Filter update
//    Vk = Lk - Bk*Xkk_1;
//    m_matrix.keepMatPricision(Vk,keepnum);
//    //Update X
//    tXk_1 = Xkk_1 + Kk*Vk;
//    m_matrix.keepMatPricision(tXk_1,keepnum);
//    //Filtered residual, normal download wave is very small
//    VectorXd Vk_temp = Lk - Bk*tXk_1;

//    tempKB = Kk*Bk;
//    I.resize(tempKB.rows(),tempKB.cols());
//    I.setIdentity();
//    m_matrix.keepMatPricision(tempKB,keepnum);
//    //Update P (Case I) (this update is extremely unstable)
//    tPk_1 = (I - tempKB)*Pkk_1;
//    //Update P(Case II)
////    MatrixXd Mk_1 = Pkk_1.inverse() + Bk.transpose()*Rk.inverse()*Bk;
////    tPk_1 =Mk_1.inverse();
//    //printMatrix(tPk_1);

//    m_matrix.keepMatPricision(tPk_1,keepnum);

////    tPk_1 = 0.5*(tPk_1 + tPk_1.transpose());	//(In theory, it should be added but added or beating. Changed the original covariance data)
//    //printMatrix(tPk_1);
//}


// get matrix B and observer L for Combination
void QKalmanFilter::Obtaining_equation(QVector< SatlitData > &currEpoch, double *m_ApproxRecPos, MatrixXd &mat_B, VectorXd &Vct_L,
                             MatrixXd &mat_P)
{
 //2020.11.10 23z Adding single difference function model between satellites
    int epochLenLB = currEpoch.length(), const_num = 3;
    MatrixXd B, P;
    VectorXd L, sys_len;
    sys_len.resize(sys_cur.length());
    sys_len.setZero();
    switch(m_KALMAN_MODEL)
    {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        B.resize(epochLenLB,m_const_param);
        P.resize(epochLenLB,epochLenLB);
        L.resize(epochLenLB);
        const_num = 3;// 3 is conntain [dx,dy,dz]
        break;
    case KALMAN_MODEL::PPP_KINEMATIC:
    case KALMAN_MODEL::PPP_STATIC:
        B.resize(2*epochLenLB,epochLenLB+m_const_param);
        P.resize(2*epochLenLB,2*epochLenLB);
        L.resize(2*epochLenLB);
        const_num = 4;// 4 is conntain [dx,dy,dz,mf]
        break;
    default:
        ErroTrace("QKalmanFilter::Obtaining_equation you should use setModel().");
        break;
    }
    // init matrix
    B.setZero();
    L.setZero();
    P.setIdentity();

    //B_part is the first four columns of B（dlta_ex,dlta_ey,dlta_ez,-dlta_mf）
    MatrixXd B_part, Q;
    B_part.resize(2*epochLenLB,4);
    B_part.setZero();
    Q.resize(2*epochLenLB,2*epochLenLB);
    Q.setZero();

    VectorXd L_temp;
    L_temp.resize(2*epochLenLB);


    bool is_find_base_sat = false;
    for (int i = 0; i < epochLenLB;i++)
    {
        SatlitData oneSatlit = currEpoch.at(i);  
        double li = 0,mi = 0,ni = 0,p0 = 0,dltaX = 0,dltaY = 0,dltaZ = 0;
        dltaX = oneSatlit.X - m_ApproxRecPos[0];
        dltaY = oneSatlit.Y - m_ApproxRecPos[1];
        dltaZ = oneSatlit.Z - m_ApproxRecPos[2];
        p0 = qSqrt(dltaX*dltaX+dltaY*dltaY+dltaZ*dltaZ);
        // compute li mi ni
        li = dltaX/p0;mi = dltaY/p0;ni = dltaZ/p0;
        //Correction of each
        double dlta_code = 0.0, dlta_phase = 0.0;
        dlta_code =  - oneSatlit.StaClock + oneSatlit.SatTrop - oneSatlit.Relativty -
                      oneSatlit.Sagnac - oneSatlit.TideEffect - oneSatlit.AntHeight;
    //2020.11.10 23z
//        double cnes_clk_phase = M_C*oneSatlit.clock_grg_phase;
//        dlta_phase = - cnes_clk_phase + oneSatlit.SatTrop - oneSatlit.Relativty -
//                      oneSatlit.Sagnac - oneSatlit.TideEffect - oneSatlit.AntHeight;


        //Add the corrections to the observations 2020.11.19 23z
        double L1 = 0.0, L2 = 0.0, P1 = 0.0, P2 = 0.0;
        L1 = currEpoch.at(i).LL1 - dlta_code;
        L2 = currEpoch.at(i).LL2 - dlta_code;
        P1 = currEpoch.at(i).CC1 - dlta_code;
        P2 = currEpoch.at(i).CC2 - dlta_code;
        currEpoch[i].LL1 = L1;
        currEpoch[i].LL2 = L2;
        currEpoch[i].CC1 = P1;
        currEpoch[i].CC2 = P2;

        //double cf = M_C*oneSatlit.clock_grg_phase;
        // set B L P
        double LP_whight  = m_LP_whight;
        switch(m_KALMAN_MODEL)
        {
        case KALMAN_MODEL::SPP_STATIC:
        case KALMAN_MODEL::SPP_KINEMATIC:
            //Computational B matrix
            //L3 carrier matrix
            B(i,0) = li;B(i,1) = mi;B(i,2) = ni;B(i,3) = -1;
            // debug by xiaogongwei 2019.04.03 for ISB
            for(int k = 1; k < m_sys_str.length();k++)
            {
                if(m_sys_str[k] == oneSatlit.SatType)
                {
                    B(i,3+k) = -1;B(i,3) = 0;
                    sys_len[k] = 1;//good no zeros cloumn in B,sys_lenmybe 0 1 1 0(debug by xiaogongwei 2019.04.09 for ISB)
                }
            }

            //Pseudorange code L
            if(KALMAN_SMOOTH_RANGE::SMOOTH == m_KALMAN_SMOOTH_RANGE)
            {
                L(i) = p0 - oneSatlit.PP3_Smooth + dlta_code;
                // Computing weight matrix PP3
                P(i, i) = 1 / oneSatlit.PP3_Smooth_Q;// Pseudo-range right
            }
            else
            {
                L(i) = p0 - oneSatlit.PP3 + dlta_code;
                // Computing weight matrix P
                P(i, i) = oneSatlit.SatWight;// Pseudo-range right
            }
            break;
        case KALMAN_MODEL::PPP_KINEMATIC:
        case KALMAN_MODEL::PPP_STATIC:
            //Computational B matrix
            //L3 carrier matrix

            if(oneSatlit.SSDPPP)
            {
                B_part(i,0) = li; B_part(i,1) = mi; B_part(i,2) = ni; B_part(i,3) = -oneSatlit.StaTropMap;
                B_part(i + epochLenLB,0) = li; B_part(i + epochLenLB,1) = mi; B_part(i + epochLenLB,2) = ni;
                B_part(i + epochLenLB,3) = -oneSatlit.StaTropMap;

                L_temp(i) = p0 - oneSatlit.LL3 + dlta_code;
                L_temp(i + epochLenLB) = p0 - oneSatlit.PP3 + dlta_code;

                P(i,i) = oneSatlit.SatWight * LP_whight;
                P(i + epochLenLB, i + epochLenLB) = oneSatlit.SatWight;

                Q(i,i) = 1/P(i,i);
                Q(i + epochLenLB, i + epochLenLB) = 1/P(i + epochLenLB,i + epochLenLB);

            }
            else
            {
                B(i,0) = li;B(i,1) = mi;B(i,2) = ni;B(i,3) = -oneSatlit.StaTropMap;B(i,4) = -1;
                for (int n = 0;n < epochLenLB;n++)//The diagonal part of the rear part initializes the wavelength of Lamta3, and the rest is 0.
                    if (i == n)
                        B(i,m_const_param+n) = M_GetLamta3(oneSatlit.Frq[0],oneSatlit.Frq[1]);//LL3 wavelength
                //P3 pseudorange code matrix
                B(i+epochLenLB,0) = li;B(i+epochLenLB,1) = mi;B(i+epochLenLB,2) = ni;B(i+epochLenLB,3) = -oneSatlit.StaTropMap;B(i+epochLenLB,4) = -1;
                // debug by xiaogongwei 2019.04.03 for ISB
                for(int k = 1; k < m_sys_str.length();k++)
                {
                    if(m_sys_str[k] == oneSatlit.SatType)
                    {
                        B(i,4+k) = -1;            B(i,4) = 0;
                        B(i+epochLenLB,4+k) = -1; B(i+epochLenLB,4) = 0;
                        sys_len[k] = 1;//good no zeros cloumn in B,sys_lenmybe 0 1 1 0(debug by xiaogongwei 2019.04.09 for ISB)
                    }
                }

                //Carrier L  pseudorange code L
                L(i) = p0 - oneSatlit.LL3 + dlta_code;
                L(i+epochLenLB) = p0 - oneSatlit.PP3 + dlta_code;
                // Computing weight matrix P
                //            if(oneSatlit.UTCTime.epochNum <= 100) LP_whight = 1e6;// for convergence
                P(i, i) = oneSatlit.SatWight * LP_whight;// Carrier weight
                P(i + epochLenLB, i + epochLenLB) = oneSatlit.SatWight;// Pseudo-range right
                break;
            }


        }//switch(m_KALMAN_MODEL)

    }//B, L is calculated
    // save data to mat_B

    if(currEpoch.at(0).SSDPPP)
    {
//        m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/B_part.csv",B_part);
//        m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/L_temp.csv",L_temp);
        tran_mat.resize(2*(epochLenLB - sys_num_cur),2*epochLenLB);
        tran_mat.setZero();

        if(sys_num_cur == 1)
        {
            MatrixXd B_dlta;
            tran_mat = getSSDBL(B_dlta,L,B_part,L_temp,epochLenLB);

            B.resize(2*(epochLenLB - 1),(epochLenLB + 3));
            B.setZero();
            for(int cf = 0; cf < 2*(currEpoch.length() - 1); cf++)
            {
                B(cf,0) = B_dlta(cf,0);
                B(cf,1) = B_dlta(cf,1);
                B(cf,2) = B_dlta(cf,2);
                B(cf,3) = B_dlta(cf,3);

                if(cf < (epochLenLB - 1))
                    B(cf,(cf + 4)) = 1;
            }
//            m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/B.csv",B);
            int cf = 23;
        }
        else
        {
            MatrixXd tran_mat_temp;
            B.resize(2*(epochLenLB - sys_num_cur),(epochLenLB + 4 - sys_num_cur));   B.setZero();
            L.resize(2*(epochLenLB - sys_num_cur),1);                                 L.setZero();
            //check sys number
            int num_single_sys[4] = {0}; int add_temp = 0;
            for(int cf_i = 0; cf_i < epochLenLB; cf_i++)
            {
                for(int cf_j = 0; cf_j < sys_cur.length(); cf_j++)
                    if(currEpoch.at(cf_i).SatType == sys_cur.at(cf_j))
                        num_single_sys[cf_j]++;
            }
            //single difference between sats in respective systems
            for(int cf_i = 0; cf_i < sys_num_cur; cf_i++)
            {
                MatrixXd B_part_sys, B_dlta; VectorXd L_part_sys, L_dlta;
                B_part_sys.resize(2*num_single_sys[cf_i],4);   B_part_sys.setZero();
                L_part_sys.resize(2*num_single_sys[cf_i],1);   L_part_sys.setZero();
                if(cf_i == 0)
                {
                    for(int cf_j = 0; cf_j < num_single_sys[cf_i]; cf_j++)
                    {
                        L_part_sys(cf_j) = L_temp(cf_j);
                        L_part_sys(cf_j + num_single_sys[cf_i]) = L_temp(cf_j + epochLenLB);
                        for(int cf_k = 0; cf_k < 4; cf_k++)
                        {
                            B_part_sys(cf_j,cf_k) = B_part(cf_j,cf_k);
                            B_part_sys(cf_j + num_single_sys[cf_i],cf_k) = B_part(cf_j + epochLenLB,cf_k);
                        }
                    }

                }
                else
                {
                    for(int cf_j = 0; cf_j < num_single_sys[cf_i]; cf_j++)
                    {
                        L_part_sys(cf_j) = L_temp(cf_j + num_single_sys[cf_i - 1]);
                        L_part_sys(cf_j + num_single_sys[cf_i]) = L_temp(cf_j + epochLenLB + num_single_sys[cf_i - 1]);
                        for(int cf_k = 0; cf_k < 4; cf_k++)
                        {
                            B_part_sys(cf_j,cf_k) = B_part(cf_j + num_single_sys[cf_i - 1],cf_k);
                            B_part_sys(cf_j + num_single_sys[cf_i],cf_k) = B_part(cf_j + epochLenLB + num_single_sys[cf_i - 1],cf_k);
                        }
                    }
                }
                tran_mat_temp = getSSDBL(B_dlta,L_dlta,B_part_sys,L_part_sys,num_single_sys[cf_i]);
                //get transformation matrix
                if(cf_i > 0) add_temp = add_temp + num_single_sys[cf_i - 1] - 1;
                for(int cf_j = 0; cf_j < num_single_sys[cf_i] - 1; cf_j++)
                {
                    L(cf_j + add_temp) = L_dlta(cf_j);
                    L(cf_j + epochLenLB - sys_num_cur + add_temp) = L_dlta(cf_j + num_single_sys[cf_i] - 1);
                    for(int cf_k = 0; cf_k < 4 + epochLenLB - sys_num_cur; cf_k++)
                    {
                        if(cf_k < 4)
                        {
                            B(cf_j + add_temp,cf_k) = B_dlta(cf_j,cf_k);
                            B(cf_j + epochLenLB - sys_num_cur + add_temp,cf_k) = B_dlta(cf_j + num_single_sys[cf_i] - 1,cf_k);
                        }
                        else
                            B(cf_k - 4,cf_k) = 1;

                    }
                    for(int cf_k = 0; cf_k < num_single_sys[cf_i]; cf_k++)
                    {
                        if(cf_i == 0)
                        {
                            tran_mat(cf_j + add_temp,cf_k + add_temp) = tran_mat_temp(cf_j,cf_k);
                            tran_mat(cf_j + add_temp + epochLenLB - sys_num_cur,cf_k + epochLenLB)
                                    = tran_mat_temp(cf_j + num_single_sys[cf_i] - 1,cf_k + num_single_sys[cf_i]);
                        }
                        else
                        {
                            tran_mat(cf_j + add_temp,cf_k + add_temp + cf_i) = tran_mat_temp(cf_j,cf_k);
                            tran_mat(cf_j + add_temp + epochLenLB - sys_num_cur,cf_k + epochLenLB + add_temp + cf_i)
                                    = tran_mat_temp(cf_j + num_single_sys[cf_i] - 1,cf_k + num_single_sys[cf_i]);
                        }

                    }
                }

//                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/B_part_sys.csv",B_part_sys);
//                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/L_part_sys.csv",L_part_sys);
//                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/B_part.csv",B_part);
//                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/L_temp.csv",L_temp);
//                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/B.csv",B);
//                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/L.csv",L);
//                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/T.csv",tran_mat);
//                m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/t.csv",tran_mat_temp);
                int cf = 23;

            }
        }

    }



    mat_B = B;
    Vct_L = L;
    mat_P = P;
//    m_matrix.writeCSV("./csv/mat_B.csv", mat_B);
//    m_matrix.writeCSV("./csv/mat_P.csv", mat_P);
    // debug by xiaogongwei 2019.04.04
    int no_zero = sys_len.size() - 1 - sys_len.sum();
    if(no_zero > 0 && !currEpoch.at(0).SSDPPP)
    {
        int new_hang = B.rows() + no_zero, new_lie = B.cols(), flag = 0;

        mat_B.resize(new_hang,new_lie);
        mat_P.resize(new_hang,new_hang);
        Vct_L.resize(new_hang);

        mat_B.setZero();
        Vct_L.setZero();
        mat_P.setIdentity();

        mat_B.block(0,0,B.rows(),B.cols()) = B;
        mat_P.block(0,0,P.rows(),P.cols()) = P;
        Vct_L.head(L.rows()) = L;

        for(int i = 1; i < sys_len.size();i++)
        {
            if(0 == sys_len[i])
            {
                mat_B(B.rows()+flag, const_num+i) = 1;
                flag++;
            }
        }
    }//if(no_zero > 0)
//    m_matrix.writeCSV("./csv/mat_B1.csv", mat_B);
//    m_matrix.writeCSV("./csv/mat_P1.csv", mat_P);
}

//get SSD'B，SSD'L 2021.03.23 23z
MatrixXd QKalmanFilter::getSSDBL(MatrixXd &B_dlta, VectorXd &L, MatrixXd B_part, VectorXd L_temp, int num_single_sys)
{
    //tran_mat
    MatrixXd tran_mat_temp;
    tran_mat_temp.resize(2*(num_single_sys - 1),2*num_single_sys);
    tran_mat_temp.setZero();

    for(int tran = 0; tran < num_single_sys - 1; tran++)
    {
        //refsats column -1
        tran_mat_temp(tran,num_single_sys - 1) = -1;
        tran_mat_temp(tran + num_single_sys - 1,num_single_sys - 1 + num_single_sys) = -1;
        //diagonal 1
        if(tran < num_single_sys - 1)
            tran_mat_temp(tran,tran) = 1, tran_mat_temp(tran + num_single_sys - 1,tran + num_single_sys) = 1;
        else
            tran_mat_temp(tran,tran + 1) = 1, tran_mat_temp(tran + num_single_sys -1,tran + num_single_sys + 1) = 1;

    }

    B_dlta = tran_mat_temp*B_part;
    L = tran_mat_temp*L_temp;
    return tran_mat_temp;

//    m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/T.csv",tran_mat);
//    m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/B_dlta.csv",B_dlta);
//    m_matrix.writeCSV("/home/zcf/Documents/Balabala/data/zcompare/SSD/L.csv",L);
}

// get matrix B and observer L for No Combination
void QKalmanFilter::Obtaining_equation_NoCombination(QVector< SatlitData > &currEpoch, double *m_ApproxRecPos, MatrixXd &mat_B, VectorXd &Vct_L,
                             MatrixXd &mat_P)
{
    int epochLenLB = currEpoch.length(), const_num = 3;
    MatrixXd B, P;
    VectorXd L, sys_len;
    sys_len.resize(m_sys_str.length());
    sys_len.setZero();
    switch(m_KALMAN_MODEL)
    {
    case KALMAN_MODEL::SPP_STATIC:
    case KALMAN_MODEL::SPP_KINEMATIC:
        B.resize(2*epochLenLB,m_const_param+epochLenLB);
        P.resize(2*epochLenLB,2*epochLenLB);
        L.resize(2*epochLenLB);
        const_num = 3;// 3 is conntain [dx,dy,dz]
        break;
    case KALMAN_MODEL::PPP_KINEMATIC:
    case KALMAN_MODEL::PPP_STATIC:
        B.resize(4*epochLenLB,3*epochLenLB+m_const_param);
        P.resize(4*epochLenLB,4*epochLenLB);
        L.resize(4*epochLenLB);
        const_num = 4;// 4 is conntain [dx,dy,dz,mf]
        break;
    default:
        ErroTrace("QKalmanFilter::Obtaining_equation you should use setModel().");
        break;
    }
    // init matrix
    B.setZero();
    L.setZero();
    P.setIdentity();

    bool is_find_base_sat = false;
    for (int i = 0; i < epochLenLB;i++)
    {
        SatlitData oneSatlit = currEpoch.at(i);
        double li = 0,mi = 0,ni = 0,p0 = 0,dltaX = 0,dltaY = 0,dltaZ = 0;
        dltaX = oneSatlit.X - m_ApproxRecPos[0];
        dltaY = oneSatlit.Y - m_ApproxRecPos[1];
        dltaZ = oneSatlit.Z - m_ApproxRecPos[2];
        p0 = qSqrt(dltaX*dltaX+dltaY*dltaY+dltaZ*dltaZ);
        // compute li mi ni
        li = dltaX/p0;mi = dltaY/p0;ni = dltaZ/p0;
        //Correction of each
        double dlta = 0;
        dlta =  - oneSatlit.StaClock + oneSatlit.SatTrop - oneSatlit.Relativty -
            oneSatlit.Sagnac - oneSatlit.TideEffect - oneSatlit.AntHeight;
        // set B L P
        double LP_whight  = m_LP_whight;
        double F1 = oneSatlit.Frq[0], F2 = oneSatlit.Frq[1];
        double lamda1 = M_C/F1, lamda2 = M_C/F2;
        switch(m_KALMAN_MODEL)
        {
        case KALMAN_MODEL::SPP_STATIC:
        case KALMAN_MODEL::SPP_KINEMATIC:
            //Computational B matrix
            //L3 carrier matrix
            B(2*i,0) = li;B(2*i,1) = mi;B(2*i,2) = ni;B(2*i,3) = -1;
            B(2*i+1,0) = li;B(2*i+1,1) = mi;B(2*i+1,2) = ni;B(2*i+1,3) = -1;
            B(2*i,i+m_const_param) = -1;// ION for P1
            B(2*i+1,i+m_const_param) = -(F1*F1)/(F2*F2);// ION for P2
            // debug by xiaogongwei 2019.04.03 for ISB
            for(int k = 1; k < m_sys_str.length();k++)
            {
                if(m_sys_str[k] == oneSatlit.SatType)
                {
                    B(2*i,3+k) = -1;
                    B(2*i+1,3+k) = -1;
                    sys_len[k] = 1;//good no zeros cloumn in B,sys_lenmybe 0 1 1 0(debug by xiaogongwei 2019.04.09 for ISB)
                }
            }
            // debug by xiaogongwei 2019.04.10 is exist base system satlite clk
            if(m_sys_str[0] == oneSatlit.SatType)
                is_find_base_sat = true;
            //Pseudorange code not use KALMAN_SMOOTH_RANGE::SMOOTH
            //Pseudorange code L
            if(KALMAN_SMOOTH_RANGE::SMOOTH == m_KALMAN_SMOOTH_RANGE)
            {
                L(2*i) = p0 - oneSatlit.CC1_Smooth + dlta;
                L(2*i+1) = p0 - oneSatlit.CC2_Smooth + dlta;
                P(2*i, 2*i) = 1 / oneSatlit.CC1_Smooth_Q;// Pseudo-range Wight
                P(2*i+1, 2*i+1) = 1 / oneSatlit.CC2_Smooth_Q;// Pseudo-range Wight
            }
            else
            {
                L(2*i) = p0 - oneSatlit.C1 + dlta;
                L(2*i+1) = p0 - oneSatlit.C2 + dlta;
                P(2*i, 2*i) = oneSatlit.SatWight;
                P(2*i+1, 2*i+1) = oneSatlit.SatWight;
            }
            break;
        case KALMAN_MODEL::PPP_KINEMATIC:
        case KALMAN_MODEL::PPP_STATIC:
            //Computational B matrix
            //L carrier matrix
            B(2*i,0) = li;B(2*i,1) = mi;B(2*i,2) = ni;B(2*i,3) = -oneSatlit.StaTropMap;B(2*i,4) = -1; // L1
            B(2*i+1,0) = li;B(2*i+1,1) = mi;B(2*i+1,2) = ni;B(2*i+1,3) = -oneSatlit.StaTropMap;B(2*i+1,4) = -1; // L2
            B(2*i,i+m_const_param) = +1;// ION for L1
            B(2*i+1,i+m_const_param) = +(F1*F1)/(F2*F2);// ION for L2
            B(2*i,i+m_const_param+epochLenLB) = lamda1;// N1 for L1
            B(2*i+1,i+m_const_param+2*epochLenLB) = lamda2;// N2 for L2

            //P pseudorange code matrix
            B(2*i+2*epochLenLB,0) = li;B(2*i+2*epochLenLB,1) = mi;B(2*i+2*epochLenLB,2) = ni;B(2*i+2*epochLenLB,3) = -oneSatlit.StaTropMap;B(2*i+2*epochLenLB,4) = -1;
            B(2*i+2*epochLenLB+1,0) = li;B(2*i+2*epochLenLB+1,1) = mi;B(2*i+2*epochLenLB+1,2) = ni;B(2*i+2*epochLenLB+1,3) = -oneSatlit.StaTropMap;B(2*i+2*epochLenLB+1,4) = -1;
            B(2*i+2*epochLenLB,i+m_const_param) = -1;// ION for P1
            B(2*i+2*epochLenLB+1,i+m_const_param) = -(F1*F1)/(F2*F2);// ION for P2

            // debug by xiaogongwei 2019.04.03 for ISB
            for(int k = 1; k < m_sys_str.length();k++)
            {
                if(m_sys_str[k] == oneSatlit.SatType)
                {
                    B(2*i,const_num+k) = -1;
                    B(2*i+1,const_num+k) = -1;
                    B(2*i+2*epochLenLB,const_num+k) = -1;
                    B(2*i+2*epochLenLB+1,const_num+k) = -1;
                    sys_len[k] = 1;//good no zeros cloumn in B,sys_lenmybe 0 1 1 0(debug by xiaogongwei 2019.04.09 for ISB)
                }
            }
            // debug by xiaogongwei 2019.04.10 is exist base system satlite clk
            if(m_sys_str[0] == oneSatlit.SatType)
                is_find_base_sat = true;
            //Carrier L  pseudorange code L
            L(2*i) = p0 - oneSatlit.LL1 + dlta;
            L(2*i+1) = p0 - oneSatlit.LL2 + dlta;
            L(2*i+2*epochLenLB) = p0 - oneSatlit.CC1 + dlta;
            L(2*i+2*epochLenLB+1) = p0 - oneSatlit.CC2 + dlta;
            // Computing weight matrix P
            P(2*i, 2*i) = oneSatlit.SatWight * LP_whight;// Carrier L1 weight
            P(2*i+1, 2*i+1) = oneSatlit.SatWight * LP_whight;// Carrier L2 weight
            P(2*i + epochLenLB, 2*i + epochLenLB) = oneSatlit.SatWight;// Pseudo-range C1 weight
            P(2*i + epochLenLB+1, 2*i + epochLenLB+1) = oneSatlit.SatWight;// Pseudo-range C2 weight
            break;
        default:
            ErroTrace("QKalmanFilter::Obtaining_equation you should use setModel().");
            break;
        }//switch(m_KALMAN_MODEL)

    }//B, L is calculated
    // save data to mat_B
    mat_B = B;
    Vct_L = L;
    mat_P = P;
//    m_matrix.writeCSV("./csv/mat_B.csv", mat_B);
//    m_matrix.writeCSV("./csv/mat_P.csv", mat_P);
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
                B(i, const_num) = 0;
            mat_B(mat_B.rows() - 1, const_num) = 1;
        }
        mat_B.block(0,0,B.rows(),B.cols()) = B;
        mat_P.block(0,0,P.rows(),P.cols()) = P;
        Vct_L.head(L.rows()) = L;

        for(int i = 1; i < sys_len.size();i++)
        {
            if(0 == sys_len[i])
            {
                mat_B(B.rows()+flag, const_num+i) = 1;
                flag++;
            }
        }
    }//if(no_zero > 0)
//    m_matrix.writeCSV("./csv/mat_B1.csv", mat_B);
//    m_matrix.writeCSV("./csv/mat_P1.csv", mat_P);
}

//Second version
bool QKalmanFilter::KalmanforStatic(QVector< SatlitData > &preEpoch,QVector< SatlitData > &currEpoch,double *m_ApproxRecPos,
                                    VectorXd &X,MatrixXd &P)
{
    sys_cur = ""; sys_num_cur = 0;
    if(currEpoch.at(0).SSDPPP)
    {
        sys_cur = currEpoch.at(0).SatType;
        sys_num_cur = 1;
        //check sys number
        for(int cf_i = 1; cf_i < currEpoch.length(); cf_i++)
        {
            if(!sys_cur.contains(currEpoch.at(cf_i).SatType))
                sys_num_cur++, sys_cur = sys_cur + currEpoch.at(cf_i).SatType;
        }
    }
    else
    {
        sys_cur = m_sys_str;   sys_num_cur = m_sys_num;
    }

    interval_cur = currEpoch.at(0).interval;

    if (!isInitPara)
    {
        m_SPP_Pos[0] = m_ApproxRecPos[0];
        m_SPP_Pos[1] = m_ApproxRecPos[1];
        m_SPP_Pos[2] = m_ApproxRecPos[2];
    }

    if(m_KALMAN_MODEL == KALMAN_MODEL::SPP_KINEMATIC)
    {
        // we solver five parameter[dx,dy,dz,dTrop,dClock],so epochLenLB > 4
        m_SPP_Pos[0] = m_ApproxRecPos[0];
        m_SPP_Pos[1] = m_ApproxRecPos[1];
        m_SPP_Pos[2] = m_ApproxRecPos[2];
        // must set zero of [dx,dy,dy] int Kinematic
        m_Xk_1(0) = 0; m_Xk_1(1) = 0; m_Xk_1(2) = 0;
    }

    //save filter sate for Quality Control
    MatrixXd temp_Fk_1 = m_Fk_1, temp_Qwk_1 = m_Qwk_1,
            temp_Rk_1 = m_Rk_1, temp_Pk_1 = m_Pk_1;
    VectorXd temp_Xk_1 = m_Xk_1;

    P_pre = m_Pk_1;
    X_pre = m_Xk_1;

    double temp_SPP_POS[3] = {0};
    memcpy(temp_SPP_POS, m_SPP_Pos, 3*sizeof(double));

    // use spp clk as priori value for base clk
    QKalmanFilter::KALMAN_MODEL kalman_model = getModel();
    if(kalman_model == QKalmanFilter::KALMAN_MODEL::SPP_STATIC || kalman_model == QKalmanFilter::KALMAN_MODEL::SPP_KINEMATIC)
        m_Xk_1(3) =  m_ApproxRecPos[3];
    else if(currEpoch.at(0).SSDPPP)    // !!!!!
    {
        //there is no r_clk in SSD  2020.11.10 23z
    }
    else
        m_Xk_1(4) =  m_ApproxRecPos[3];

// filter for the first time
    filter(preEpoch, currEpoch, X, P);

    // Quality Control
    bool gross_LC = true;// false
    int minSatNum = 5, max_iter = 10;

    if(m_KALMAN_MODEL == KALMAN_MODEL::SPP_STATIC || m_KALMAN_MODEL == KALMAN_MODEL::PPP_STATIC)
        minSatNum = 2*sys_num_cur;
    else
        minSatNum = 5;

    while(gross_LC)
    {
        if(preEpoch.length() != 0)
        {
            if(currEpoch.at(0).SSD_fixed_flag)
            {
                gross_LC = false;
                break;
            }
//BackSmooth
//            if(currEpoch.at(0).ARornot && currEpoch.at(0).UTCTime.epochNum < preEpoch.at(0).UTCTime.epochNum && m_KALMAN_MODEL == KALMAN_MODEL::PPP_STATIC)
//            {
//                gross_LC = false;
//                break;
//            }
        }

        if(currEpoch.at(0).UTCTime.epochNum > 0 && preEpoch.length() == 0)
        {
            gross_LC = false;
            break;
        }

        // get B, wightP ,L
        MatrixXd B, wightP;
        VectorXd L, delate_LC;
        if(getPPPModel() == PPP_MODEL::PPP_Combination)
            Obtaining_equation(currEpoch, m_SPP_Pos, B, L, wightP);
        else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
            Obtaining_equation_NoCombination(currEpoch, m_SPP_Pos, B, L, wightP);

        // detect gross error
        int sat_len = 0;
        if(currEpoch.at(0).SSDPPP)
            sat_len = currEpoch.length() - sys_num_cur;
        else
            sat_len = currEpoch.length();


        if(m_KALMAN_MODEL == KALMAN_MODEL::SPP_STATIC || m_KALMAN_MODEL == KALMAN_MODEL::SPP_KINEMATIC)
        {
            gross_LC = m_qualityCtrl.VtPVCtrl_Filter_C(B, L, m_Xk_1, delate_LC, sat_len);// QC pesoderange
        }
        else
        {
            if(getPPPModel() == PPP_MODEL::PPP_Combination)
            {
                if(getModel() == KALMAN_MODEL::PPP_STATIC)
                {
                    QVector< double > LP_threshold;// {0.06 10};
                    for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                    {
                        if(currEpoch.at(cf_i).EA[0] > 40)
                            LP_threshold.append(0.035);
                        else
                        {
                            double elevation = currEpoch.at(cf_i).EA[0];
                            LP_threshold.append((40 - elevation)*0.002 + 0.035);
                        }
                        //LP_threshold.append(100);
                    }

                    if(currEpoch.at(0).SSDPPP)
                    {
                        double q_ref = 0.0,q_temp = 0.0;
                        for(int cf_i = 0; cf_i < LP_threshold.length(); cf_i++)
                        {
                            if(cf_i < currEpoch.at(0).prn_referencesat[1])
                            {
                                q_ref = LP_threshold.at(currEpoch.at(0).prn_referencesat[1]);
                                q_temp = LP_threshold[cf_i];
                                LP_threshold[cf_i] = sqrt(q_ref*q_ref + q_temp*q_temp);
                            }
                            if(currEpoch.at(0).prn_referencesat[1] < cf_i && cf_i < currEpoch.at(0).prn_referencesat[3])
                            {
                                q_ref = LP_threshold.at(currEpoch.at(0).prn_referencesat[3]);
                                q_temp = LP_threshold[cf_i];
                                LP_threshold[cf_i] = sqrt(q_ref*q_ref + q_temp*q_temp);
                            }
                            if(currEpoch.at(0).prn_referencesat[3] < cf_i && cf_i < currEpoch.at(0).prn_referencesat[5])
                            {
                                q_ref = LP_threshold.at(currEpoch.at(0).prn_referencesat[5]);
                                q_temp = LP_threshold[cf_i];
                                LP_threshold[cf_i] = sqrt(q_ref*q_ref + q_temp*q_temp);
                            }
                            if(currEpoch.at(0).prn_referencesat[5] < cf_i && cf_i < currEpoch.at(0).prn_referencesat[7])
                            {
                                q_ref = LP_threshold.at(currEpoch.at(0).prn_referencesat[7]);
                                q_temp = LP_threshold[cf_i];
                                LP_threshold[cf_i] = sqrt(q_ref*q_ref + q_temp*q_temp);
                            }
                        }
                        for(int cf_sys = sys_num_cur - 1; cf_sys >= 0; cf_sys--)
                        {
                            LP_threshold.remove(currEpoch.at(0).prn_referencesat[2*cf_sys + 1]);
                        }
                    }

                    gross_LC = m_qualityCtrl.VtPVCtrl_Filter_LC(B, L, m_Xk_1, delate_LC, sat_len, LP_threshold);// QC for carrire and pesoderange
                }
                else
                {
                    QVector< double > LP_threshold;//{0.1, 10.0};
                    for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                    {
                        if(currEpoch.at(cf_i).EA[0] > 40)
                            LP_threshold.append(0.04);
                        else
                        {
                            double elevation = currEpoch.at(cf_i).EA[0];
                            LP_threshold.append((40 - elevation)*0.002 + 0.04);
                        }
                        //LP_threshold.append(100);
                    }

                    if(currEpoch.at(0).SSDPPP)
                    {
                        double q_ref = 0.0,q_temp = 0.0;
                        for(int cf_i = 0; cf_i < LP_threshold.length(); cf_i++)
                        {
                            if(cf_i < currEpoch.at(0).prn_referencesat[1])
                            {
                                q_ref = LP_threshold.at(currEpoch.at(0).prn_referencesat[1]);
                                q_temp = LP_threshold[cf_i];
                                LP_threshold[cf_i] = sqrt(q_ref*q_ref + q_temp*q_temp);
                            }
                            if(currEpoch.at(0).prn_referencesat[1] < cf_i && cf_i < currEpoch.at(0).prn_referencesat[3])
                            {
                                q_ref = LP_threshold.at(currEpoch.at(0).prn_referencesat[3]);
                                q_temp = LP_threshold[cf_i];
                                LP_threshold[cf_i] = sqrt(q_ref*q_ref + q_temp*q_temp);
                            }
                            if(currEpoch.at(0).prn_referencesat[3] < cf_i && cf_i < currEpoch.at(0).prn_referencesat[5])
                            {
                                q_ref = LP_threshold.at(currEpoch.at(0).prn_referencesat[5]);
                                q_temp = LP_threshold[cf_i];
                                LP_threshold[cf_i] = sqrt(q_ref*q_ref + q_temp*q_temp);
                            }
                            if(currEpoch.at(0).prn_referencesat[5] < cf_i && cf_i < currEpoch.at(0).prn_referencesat[7])
                            {
                                q_ref = LP_threshold.at(currEpoch.at(0).prn_referencesat[7]);
                                q_temp = LP_threshold[cf_i];
                                LP_threshold[cf_i] = sqrt(q_ref*q_ref + q_temp*q_temp);
                            }
                        }
                        for(int cf_sys = sys_num_cur - 1; cf_sys >= 0; cf_sys--)
                        {
                            LP_threshold.remove(currEpoch.at(0).prn_referencesat[2*cf_sys + 1]);
                        }
                    }
                    gross_LC = m_qualityCtrl.VtPVCtrl_Filter_LC(B, L, m_Xk_1, delate_LC, sat_len, LP_threshold);// QC for carrire and pesoderange
                }
            }
            else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
            {
                if(getModel() == KALMAN_MODEL::PPP_STATIC)
                {
                    double L12P12_threshold[4] = {0.05, 0.05, 5.0, 5.0};
                    gross_LC = m_qualityCtrl.VtPVCtrl_Filter_LC_NoCombination(B, L, m_Xk_1, delate_LC, sat_len, L12P12_threshold);// QC for carrire and pesoderange
                }
                else
                {
                    double L12P12_threshold[4] = {0.05, 0.05, 5.0, 5.0};
                    gross_LC = m_qualityCtrl.VtPVCtrl_Filter_LC_NoCombination(B, L, m_Xk_1, delate_LC, sat_len, L12P12_threshold);// QC for carrire and pesoderange
                }
            }
        }
        max_iter--;
        if(gross_LC == false || max_iter <= 0) break;
        // delate gross Errors Satlites form end for start.
        QVector<int> del_flag;
        for(int i = sat_len - 1; i >= 0;i--)
        {
            if(0 != delate_LC[i])
                del_flag.append(i);
        }

//        int refsats_G, refsats_R, refsats_E;
//        refsats_G = currEpoch.at(0).prn_referencesat[1];
//        refsats_R = currEpoch.at(0).prn_referencesat[3];
//        refsats_E = currEpoch.at(0).prn_referencesat[5];

        // delete gross Errors
        int del_len = del_flag.length();

        if(del_len == 0)
        {
            gross_LC = false;
            break;
        }

        if(currEpoch.at(0).SSDPPP)
        {//consider the sit of refsats
            int ref_sit = 0;
            for(int cf_j = 1; cf_j < sys_num_cur; cf_j++)
            {
               ref_sit = currEpoch.at(0).prn_referencesat[2*cf_j - 1];
               for(int cf_i = 0; cf_i < del_len; cf_i++)
               {
                   if(del_flag.at(cf_i) >= ref_sit)
                       del_flag[cf_i]++;
               }

            }
        }

        if(currEpoch.length() - del_len >= minSatNum)
        {
        //delete
            for(int i = 0; i < del_len;i++)
                currEpoch.remove(del_flag[i]);

            QString sys_str_temp = sys_cur;

            if(currEpoch.at(0).SSDPPP)
            {//update sits of refsats
                int num_sys_temp = 0; QString sys_temp = "";
                if(sys_num_cur > 1)
                {//G E R C
                    QVector< SatlitData > sats_GPS;
                    QVector< SatlitData > sats_GAL;
                    QVector< SatlitData > sats_GLO;
                    QVector< SatlitData > sats_BDS;
                    QVector< SatlitData > sats_valid;
                    SatlitData temp_sat;   QVector< int > sit_GERC;
                    for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                    {
                        temp_sat = currEpoch.at(cf_i);
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
                    int threshold_satnum = 2;
                    //if(m_KALMAN_MODEL == KALMAN_MODEL::PPP_KINEMATIC) threshold_satnum = 4;

                    if(sats_GPS.length() >= threshold_satnum)
                    {
                        num_sys_temp ++, sys_temp = sys_temp + 'G';
                        for(int cf_i = 0; cf_i < sats_GPS.length(); cf_i++)
                        {
                            temp_sat = sats_GPS.at(cf_i);
                            sats_valid.append(temp_sat);
                        }
                    }
                    if(sats_GAL.length() >= threshold_satnum)
                    {
                        num_sys_temp ++, sys_temp = sys_temp + 'E';
                        for(int cf_i = 0; cf_i < sats_GAL.length(); cf_i++)
                        {
                            temp_sat = sats_GAL.at(cf_i);
                            sats_valid.append(temp_sat);
                        }
                    }
                    if(sats_GLO.length() >= threshold_satnum)
                    {
                        num_sys_temp ++, sys_temp = sys_temp + 'R';
                        for(int cf_i = 0; cf_i < sats_GLO.length(); cf_i++)
                        {
                            temp_sat = sats_GLO.at(cf_i);
                            sats_valid.append(temp_sat);
                        }
                    }
                    if(sats_BDS.length() >= threshold_satnum)
                    {
                        num_sys_temp ++, sys_temp = sys_temp + 'C';
                        for(int cf_i = 0; cf_i < sats_BDS.length(); cf_i++)
                        {
                            temp_sat = sats_BDS.at(cf_i);
                            sats_valid.append(temp_sat);
                        }
                    }
                    currEpoch = sats_valid;
                    if(num_sys_temp > 0)
                    {
                        sys_num_cur = num_sys_temp;
                        sys_cur = sys_temp;
                    }

                }

                //update
                for(int cf_sys = 0; cf_sys < sys_str_temp.length(); cf_sys++)
                {
                    int prn = currEpoch.at(0).prn_referencesat[2*cf_sys], ref_sit = 0;
                    for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                    {
                        if(currEpoch.at(cf_i).PRN == prn && currEpoch.at(cf_i).SatType == sys_str_temp.at(cf_sys))
                        {
                            ref_sit = cf_i;
                            break;
                        }
                    }
                    for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                        currEpoch[cf_i].prn_referencesat[2*cf_sys + 1] = ref_sit;
                }

                if(sys_cur != sys_str_temp)
                {
                    int ref_G, sit_G, ref_R, sit_R, ref_C, sit_C, ref_E, sit_E, cumulation_sys;

                    cumulation_sys = 0;
                    if(sys_str_temp.contains("G"))
                    {
                        ref_G = currEpoch.at(0).prn_referencesat[2*cumulation_sys];
                        sit_G = currEpoch.at(0).prn_referencesat[2*cumulation_sys+1];
                        cumulation_sys++;
                    }
                    if(sys_str_temp.contains("E"))
                    {
                        ref_E = currEpoch.at(0).prn_referencesat[2*cumulation_sys];
                        sit_E = currEpoch.at(0).prn_referencesat[2*cumulation_sys+1];
                        cumulation_sys++;
                    }
                    if(sys_str_temp.contains("R"))
                    {
                        ref_R = currEpoch.at(0).prn_referencesat[2*cumulation_sys];
                        sit_R = currEpoch.at(0).prn_referencesat[2*cumulation_sys+1];
                        cumulation_sys++;
                    }
                    if(sys_str_temp.contains("C"))
                    {
                        ref_C = currEpoch.at(0).prn_referencesat[2*cumulation_sys];
                        sit_C = currEpoch.at(0).prn_referencesat[2*cumulation_sys+1];
                        cumulation_sys++;
                    }

                    cumulation_sys = 0;
                    if(sys_cur.contains("G"))
                    {
                        for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                        {
                            currEpoch[cf_i].prn_referencesat[2*cumulation_sys] = ref_G;
                            currEpoch[cf_i].prn_referencesat[2*cumulation_sys+1] = sit_G;
                        }
                        cumulation_sys++;
                    }
                    if(sys_cur.contains("E"))
                    {
                        for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                        {
                            currEpoch[cf_i].prn_referencesat[2*cumulation_sys] = ref_E;
                            currEpoch[cf_i].prn_referencesat[2*cumulation_sys+1] = sit_E;
                        }
                        cumulation_sys++;
                    }
                    if(sys_cur.contains("R"))
                    {
                        for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                        {
                            currEpoch[cf_i].prn_referencesat[2*cumulation_sys] = ref_R;
                            currEpoch[cf_i].prn_referencesat[2*cumulation_sys+1] = sit_R;
                        }
                        cumulation_sys++;
                    }
                    if(sys_cur.contains("C"))
                    {
                        for(int cf_i = 0; cf_i < currEpoch.length(); cf_i++)
                        {
                            currEpoch[cf_i].prn_referencesat[2*cumulation_sys] = ref_C;
                            currEpoch[cf_i].prn_referencesat[2*cumulation_sys+1] = sit_C;
                        }
                        cumulation_sys++;
                    }

                    for(int cf_i = sys_num_cur; cf_i < m_sys_num; cf_i++)
                        for(int cf_j = 0; cf_j < currEpoch.length(); cf_j++)
                            currEpoch[cf_j].prn_referencesat[2*cf_i] = 0,
                                    currEpoch[cf_j].prn_referencesat[2*cf_i + 1] = 0;
                }

            }

            // restore filter state
            m_Fk_1 = temp_Fk_1; m_Qwk_1 = temp_Qwk_1; m_Rk_1 = temp_Rk_1;
            m_Pk_1 = temp_Pk_1; m_Xk_1 = temp_Xk_1;
            memcpy(m_SPP_Pos, temp_SPP_POS, 3*sizeof(double));
        //filter again
            filter(preEpoch, currEpoch, X, P);
        }
        else
        {
            gross_LC = true;
            break;
        }
    }

    // Calculate the filtered residuals and save them in the satellite structure
    // get B, wightP ,L
    MatrixXd B, wightP;
    VectorXd L, Vk;
    int sat_len = 0;
    if(currEpoch.at(0).SSDPPP)
        sat_len = currEpoch.length() - sys_num_cur;
    else
        sat_len = currEpoch.length();
    if(getPPPModel() == PPP_MODEL::PPP_Combination)
        Obtaining_equation(currEpoch, m_SPP_Pos, B, L, wightP);
    else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
        Obtaining_equation_NoCombination(currEpoch, m_SPP_Pos, B, L, wightP);

//IGGIII 23z
    if(false){
        // for IGGIII
        MatrixXd wightPk = m_Rk_1.inverse();
        int SizeofP = wightPk.rows();
        m_qualityCtrl.IGG3Algorithm(B,L,m_Xk_1,wightPk,SizeofP);// will change wightRk!!!
        //Version Kalman filter
        MatrixXd newRk = wightPk.inverse();// new Rk
        if(KALMAN_FILLTER::KALMAN_STANDARD ==  m_KALMAN_FILLTER)
            KalmanforStatic(B,L,m_Fk_1,m_Qwk_1,newRk,m_Xk_1,m_Pk_1,preEpoch,currEpoch);
        else if(KALMAN_FILLTER::KALMAN_MrOu ==  m_KALMAN_FILLTER)
            KalmanforStaticOu(B,L,m_Fk_1,m_Qwk_1,wightPk,m_Xk_1,m_Pk_1);
    }


    Vk = B*m_Xk_1 - L;

    if(m_KALMAN_MODEL == KALMAN_MODEL::SPP_STATIC || m_KALMAN_MODEL == KALMAN_MODEL::SPP_KINEMATIC)
    {
        if(getPPPModel() == PPP_MODEL::PPP_Combination)
        {
            for(int i = 0; i < sat_len;i++)
            {
                currEpoch[i].VLL3 = 0;
                currEpoch[i].VPP3 = Vk[i];
            }
        }
        else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
        {
            for(int i = 0; i < sat_len;i++)
            {
                currEpoch[i].VL1 = 0; currEpoch[i].VL2 = 0;
                currEpoch[i].VC1 = Vk[2*i]; currEpoch[i].VC2 = Vk[2*i+1];
            }
        }
    }
    else
    {
        if(getPPPModel() == PPP_MODEL::PPP_Combination)
        {
            if(currEpoch.at(0).SSDPPP)
            {
                for(int i = 0; i < sat_len;i++)
                {
                    if(i < currEpoch.at(0).prn_referencesat[1] - 0)
                    {
                        currEpoch[i].VLL3 = Vk[i];
                        currEpoch[i].VPP3 = Vk[i+sat_len];
                    }
                    else if(currEpoch.at(0).prn_referencesat[1] - 0 <= i && i < currEpoch.at(0).prn_referencesat[3] - 1)
                    {
                        currEpoch[i + 1].VLL3 = Vk[i];
                        currEpoch[i + 1].VPP3 = Vk[i+sat_len];
                    }
                    else if(currEpoch.at(0).prn_referencesat[3] - 1 <= i && i < currEpoch.at(0).prn_referencesat[5] - 2)
                    {
                        currEpoch[i + 2].VLL3 = Vk[i];
                        currEpoch[i + 2].VPP3 = Vk[i+sat_len];
                    }
                    else if(currEpoch.at(0).prn_referencesat[5] - 2 <= i && currEpoch.at(0).prn_referencesat[5] != 0)
                    {
                        currEpoch[i + 3].VLL3 = Vk[i];
                        currEpoch[i + 3].VPP3 = Vk[i+sat_len];
                    }
                    else
                    {
                        currEpoch[i].VLL3 = 0;
                        currEpoch[i].VPP3 = 0;
                    }
                }
            }
            else
            {
                for(int i = 0; i < sat_len;i++)
                {
                    currEpoch[i].VLL3 = Vk[i];
                    currEpoch[i].VPP3 = Vk[i+sat_len];
                }
            }
        }
        else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
        {
            for(int i = 0; i < sat_len;i++)
            {
                currEpoch[i].VL1 = Vk[2*i]; currEpoch[i].VL2 = Vk[2*i+1];
                currEpoch[i].VC1 = Vk[2*i+2*sat_len]; currEpoch[i].VC2 = Vk[2*i+2*sat_len+1];
            }
        }

    }

    bool flag_nan = false;
    int size_x = m_Xk_1.rows();
    for(int cf_i = 0; cf_i < size_x; cf_i++)
    {
        flag_nan = isnan(m_Xk_1(cf_i));
        if(flag_nan)
        {
            gross_LC = true;
            break;
        }
    }

    //Save the results of this epoch (does not contain initialization data)
    X = m_Xk_1;
    P = m_Pk_1;
    if(gross_LC)
    {
        // restore filter state
        m_Fk_1 = temp_Fk_1; m_Qwk_1 = temp_Qwk_1; m_Rk_1 = temp_Rk_1;
        m_Pk_1 = temp_Pk_1; m_Xk_1 = temp_Xk_1;
        memcpy(m_SPP_Pos, temp_SPP_POS, 3*sizeof(double));
        X.setZero();
        P.setIdentity();
        P = P * 1e10;
        // add bad flag 999 for filter bad
        for(int i = 0;i < currEpoch.length();i++)
            currEpoch[i].EpochFlag = 999;
    }
    else
    {
        // update m_ApproxRecPos use kalman
        m_ApproxRecPos[0] = m_SPP_Pos[0] + m_Xk_1(0);
        m_ApproxRecPos[1] = m_SPP_Pos[1] + m_Xk_1(1);
        m_ApproxRecPos[2] = m_SPP_Pos[2] + m_Xk_1(2);
    }

    //calculate sigma_pos 2020.12.08 23z
    if(!gross_LC)
    {
        double Q_pos[3] = {0.0}, sigam_pos = 0.0;
        for(int cf_i = 0; cf_i < 3; cf_i++)
            Q_pos[cf_i] = m_Pk_1(cf_i,cf_i);
        sigam_pos = sqrt(abs(Q_pos[0]) + abs(Q_pos[1]) + abs(Q_pos[2]));

        int times = 0;
        if(preEpoch.length() != 0)
            times = preEpoch.at(0).times_sigam_pos;

        for(int cf_i = 0;cf_i < currEpoch.length(); cf_i++)
        {
            currEpoch[cf_i].sigam_pos = sigam_pos;

            if(sigam_pos < 0.5)
                currEpoch[cf_i].times_sigam_pos = times + 1;
        }

        int cf = 23;
    }

    return (!gross_LC);
}

void QKalmanFilter::filter(QVector< SatlitData > &preEpoch,QVector< SatlitData > &currEpoch, VectorXd &X,MatrixXd &P)
{
    int preEpochLen = preEpoch.length();
    int epochLenLB = currEpoch.length();
    // get B, wightP ,L
    MatrixXd B, wightP;
    VectorXd L;
    if(getPPPModel() == PPP_MODEL::PPP_Combination)
        Obtaining_equation(currEpoch, m_SPP_Pos, B, L, wightP);
    else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
        Obtaining_equation_NoCombination(currEpoch, m_SPP_Pos, B, L, wightP);

    //First epoch initialization  Filter init
    if (0 == preEpochLen)
    {
        if(getPPPModel() == PPP_MODEL::PPP_Combination)
            initKalman(currEpoch,B,L);
        else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
            initKalman_NoCombination(currEpoch,B,L);
    }

    //Determine whether the number of satellites has changed (comparison of two epochs before and after)
    QVector< int > oldPrnFlag;//Compared with the location of the same satellite in the previous epoch, it is not found with -1
    bool isNewSatlite = false;
    isNewSatlite = isSatelliteChange(preEpoch, currEpoch, oldPrnFlag);

    // if have P matrix use P. This is for back smooth
    if(P.rows() > 1)
    {
        m_Xk_1 = X;
        m_Pk_1 = P;
        //isNewSatlite = true;
    }

    //Update Rk_1（There is no change in the number of satellites） -1 for new
    if(getPPPModel() == PPP_MODEL::PPP_Combination)
        updateRk(currEpoch, B.rows(), oldPrnFlag);
    else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
        updateRk_NoCombination(currEpoch, B.rows());

//When the reference star changes, the filter parameters are obtained again    2020.11.17 23z
    bool referencesat_changed = false;
    if(currEpoch.at(0).SSDPPP && preEpoch.length() != 0)
    {
        int sys_num_pre = 1;   QString sys_pre = "";
        sys_pre = preEpoch.at(0).SatType;
        for(int cf_num = 1; cf_num < preEpoch.length(); cf_num++)
        {
            if(!sys_pre.contains(preEpoch.at(cf_num).SatType))
                sys_num_pre++,sys_pre = sys_pre + preEpoch.at(cf_num).SatType;
        }

        if(sys_cur == sys_pre)
        {
            for(int cf_i = 0; cf_i < sys_num_cur; cf_i++)
            {
                if(currEpoch.at(0).prn_referencesat[2*cf_i] != preEpoch.at(0).prn_referencesat[2*cf_i])
                {
                    referencesat_changed = true;
                    break;
                }
            }
        }
        else
        {
            int pre_sit = -923, prn_ref_pre = 0;
            for(int cf_i = 0; cf_i < sys_num_cur; cf_i++)
            {
                for(int cf_j = 0; cf_j < sys_num_pre; cf_j++)
                {
                    if(sys_pre.at(cf_j) == sys_cur.at(cf_i))
                    {
                        pre_sit = cf_j;
                        break;
                    }
                }
                if(pre_sit == -923) prn_ref_pre = -923;
                else prn_ref_pre = preEpoch.at(0).prn_referencesat[2*pre_sit];
                if(currEpoch.at(0).prn_referencesat[2*cf_i] != prn_ref_pre)
                {
                    referencesat_changed = true;
                    break;
                }
            }
        }


    }

    //Change filter parameters
    if(KALMAN_MODEL::PPP_KINEMATIC == m_KALMAN_MODEL ||  KALMAN_MODEL::PPP_STATIC == m_KALMAN_MODEL)
    {
        //Increase or decrease n satellites
        if (((preEpochLen != epochLenLB) || isNewSatlite || referencesat_changed) && preEpochLen != 0)
        {
            if(getPPPModel() == PPP_MODEL::PPP_Combination)
                changeKalmanPara(preEpoch,currEpoch,oldPrnFlag,B,L);//Update all kalman parameter data sizes
            else if(getPPPModel() == PPP_MODEL::PPP_NOCombination)
                changeKalmanPara_NoCombination(currEpoch,oldPrnFlag, preEpochLen);//Update all kalman parameter data sizes
        }
    }

    //tesrt 2021.04.13 23z
    if(global_cf::randomwalk_zwd == 923)
    {
        double zwd_q = 0;
        MatrixXd temp_p_zwd;
        temp_p_zwd = (B.transpose()*B).inverse();
        zwd_q = m_Qwk_1(3,3) = temp_p_zwd(3,3)*1e-3/interval_cur;
    }

    if(global_cf::flag_seismology)
    {//the difference between GPST and UTC is ignored
        double epoch_obs = 0;
        epoch_obs = (currEpoch.at(0).UTCTime.Hours*3600 +
                currEpoch.at(0).UTCTime.Minutes*60 + currEpoch.at(0).UTCTime.Seconds)/interval_cur;

        double OTh, OTm, OTs, AOT, BOT;
        OTh = global_cf::OTh;   OTm = global_cf::OTm;   OTs = global_cf::OTs;
        AOT = global_cf::After_OT;   BOT = global_cf::Before_OT;

        int OTepoch, AOTepoch, BOTepoch;
        OTepoch= (OTh*3600 + OTm*60 + OTs)/interval_cur;
        AOTepoch = OTepoch + AOT;
        BOTepoch = OTepoch - BOT;

        if(epoch_obs < BOTepoch || AOTepoch < epoch_obs)
        {
            for(int cf_i = 0; cf_i < 3; cf_i++) m_Qwk_1(cf_i,cf_i) = 0;
        }
        if(epoch_obs == BOTepoch)
            for(int cf_i = 0; cf_i < 3; cf_i++) m_Xk_1(cf_i) = 0;

    }

    //Version Kalman filter
    if(KALMAN_FILLTER::KALMAN_STANDARD ==  m_KALMAN_FILLTER)
        KalmanforStatic(B,L,m_Fk_1,m_Qwk_1,m_Rk_1,m_Xk_1,m_Pk_1,preEpoch,currEpoch);
    else if(KALMAN_FILLTER::KALMAN_MrOu ==  m_KALMAN_FILLTER)
        KalmanforStaticOu(B,L,m_Fk_1,m_Qwk_1,wightP,m_Xk_1,m_Pk_1);
}



//Determine whether the number of satellites has changed (comparison of two epochs before and after)   debug by xiaogongwei 2019.04.29
bool QKalmanFilter::isSatelliteChange(QVector< SatlitData > &preEpoch,QVector< SatlitData > &currEpoch, QVector< int > &oldPrnFlag)
{
    int preEpochLen = preEpoch.length();
    int epochLenLB = currEpoch.length();
    //Determine whether the number of satellites has changed (comparison of two epochs before and after)
    int oldSatLen = 0;
    bool isNewSatlite = false;
    for (int i = 0;i < epochLenLB;i++)
    {//Whether the satellite inspections before and after the cycle are completely equal
        SatlitData epochSatlit = currEpoch.at(i);
        bool Isfind = false;//Whether the tag finds the last epoch
        for (int j = 0;j < preEpochLen;j++)
        {
            SatlitData preEpochSatlit = preEpoch.at(j);
            if (epochSatlit.PRN == preEpochSatlit.PRN&&epochSatlit.SatType == preEpochSatlit.SatType)
            {
                oldPrnFlag.append(j);//Compared with the location of the same satellite in the previous epoch, it is not found with -1

                Isfind = true;
                oldSatLen++;
                break;
            }
        }
        if (!Isfind)
        {
            oldPrnFlag.append(-1);//Compared with the location of the same satellite in the previous epoch, it is not found with -1
            isNewSatlite = true;
        }
    }
    return isNewSatlite;
}

// update Rk(Observation Covariance)
void QKalmanFilter::updateRk(QVector< SatlitData > &currEpoch, int B_len, QVector< int > oldPrnFlag)
{
    int epochLenLB = currEpoch.length();
    if(KALMAN_MODEL::SPP_STATIC == m_KALMAN_MODEL || KALMAN_MODEL::SPP_KINEMATIC == m_KALMAN_MODEL)
    {
        m_Rk_1.resize(B_len, B_len);// this m_Rk_1 is for ISB
        m_Rk_1.setIdentity();// this m_Rk_1 is for ISB
        for (int i = 0;i < epochLenLB;i++)
        {
            SatlitData oneSatlit = currEpoch.at(i);
            if(KALMAN_SMOOTH_RANGE::SMOOTH == m_KALMAN_SMOOTH_RANGE)
                m_Rk_1(i, i) = oneSatlit.PP3_Smooth_Q;//Covariance of pseudorange equations Reciprocal (noise)
            else
                m_Rk_1(i, i) = 1 / oneSatlit.SatWight;//Covariance of pseudorange equations Reciprocal (noise)
        }
    }
    else
    {
        if(currEpoch.at(0).SSDPPP)
        {//2020.11.11 23z
            m_Rk_1.resize(2*epochLenLB, 2*epochLenLB);
            m_Rk_1.setIdentity();
            for (int i = 0;i < epochLenLB;i++)
            {
                SatlitData oneSatlit = currEpoch.at(i);
                double Q_LP_whight  = 1.0/m_LP_whight;
//                if(oldPrnFlag.at(i) == -1)
//                {
//                    //new sats, lower weight 2020.11.29 23z
//                    m_Rk_1(i,i) = Q_LP_whight/oneSatlit.SatWight;
//                    m_Rk_1(i + epochLenLB,i + epochLenLB) = 1.0/oneSatlit.SatWight;
//                }
//                else
//                {
                    m_Rk_1(i,i) = Q_LP_whight/oneSatlit.SatWight;//Covariance of carrier equation Reciprocal (small noise)// 1/25000 =4e-4
                    m_Rk_1(i + epochLenLB,i + epochLenLB) = 1.0/oneSatlit.SatWight;//Covariance of pseudorange equations Reciprocal (noise)
//                }


            }
        }
        else
        {
            m_Rk_1.resize(B_len, B_len);// this m_Rk_1 is for ISB
            m_Rk_1.setIdentity();// this m_Rk_1 is for ISB
            for (int i = 0;i < epochLenLB;i++)
            {
                SatlitData oneSatlit = currEpoch.at(i);
                double Q_LP_whight  = 1 / m_LP_whight;// Contrast in paper 2019.05.11 by xiaogongwei
    //            if(oneSatlit.UTCTime.epochNum <= 100) LP_whight = 1e-6;// for convergence
                m_Rk_1(i,i) = Q_LP_whight / oneSatlit.SatWight;//Covariance of carrier equation Reciprocal (small noise)// 1/25000 =4e-4
                m_Rk_1(i + epochLenLB,i + epochLenLB) = 1 /oneSatlit.SatWight;//Covariance of pseudorange equations Reciprocal (noise)

            }
        }

    }
  //m_Rk_1 for SSD
    if(currEpoch.at(0).SSDPPP)
    {
//        double q_ref_l = 0.0, q_ref_p = 0.0, q_temp = 0.0;

//        QVector< double > qlist_temp;
//        for(int cf_i = 0; cf_i < m_Rk_1.rows(); cf_i++)
//            q_temp = m_Rk_1(cf_i,cf_i), qlist_temp.append(q_temp);

//        for(int cf_i = sys_num_cur - 1; cf_i >=0; cf_i--)
//            qlist_temp.remove(currEpoch.at(0).prn_referencesat[2*cf_i + 1]);

//        MatrixXd temp_R = m_Rk_1;
//        m_Rk_1.resize(2*(epochLenLB - sys_num_cur),2*(epochLenLB - sys_num_cur));
//        m_Rk_1.setIdentity();
//        //m_Rk_1 = tran_mat*temp_R*(tran_mat.transpose());

//        for(int cf_i = 0; cf_i < (epochLenLB - sys_num_cur); cf_i++)
//        {
//            if(cf_i < currEpoch.at(0).prn_referencesat[1])
//            {
//                q_ref_l = temp_R(currEpoch.at(0).prn_referencesat[1],currEpoch.at(0).prn_referencesat[1]);
//                q_ref_p = temp_R(currEpoch.at(0).prn_referencesat[1] + epochLenLB,currEpoch.at(0).prn_referencesat[1] + epochLenLB);
//            }
//            else if(currEpoch.at(0).prn_referencesat[1] < cf_i + 1 && cf_i + 1 < currEpoch.at(0).prn_referencesat[3])
//            {
//                q_ref_l = temp_R(currEpoch.at(0).prn_referencesat[3],currEpoch.at(0).prn_referencesat[3]);
//                q_ref_p = temp_R(currEpoch.at(0).prn_referencesat[3] + epochLenLB,currEpoch.at(0).prn_referencesat[3] + epochLenLB);
//            }
//            else if(currEpoch.at(0).prn_referencesat[3] < cf_i + 2 && cf_i + 2 < currEpoch.at(0).prn_referencesat[5])
//            {
//                q_ref_l = temp_R(currEpoch.at(0).prn_referencesat[5],currEpoch.at(0).prn_referencesat[5]);
//                q_ref_p = temp_R(currEpoch.at(0).prn_referencesat[5] + epochLenLB,currEpoch.at(0).prn_referencesat[5] + epochLenLB);
//            }
//            else if(currEpoch.at(0).prn_referencesat[5] < cf_i + 3 && cf_i + 3 < currEpoch.at(0).prn_referencesat[7])
//            {
//                q_ref_l = temp_R(currEpoch.at(0).prn_referencesat[7],currEpoch.at(0).prn_referencesat[7]);
//                q_ref_p = temp_R(currEpoch.at(0).prn_referencesat[7] + epochLenLB,currEpoch.at(0).prn_referencesat[7] + epochLenLB);
//            }

//            m_Rk_1(cf_i,cf_i) = sqrt(q_ref_l*q_ref_l + qlist_temp.at(cf_i)*qlist_temp.at(cf_i));
//            m_Rk_1(cf_i + epochLenLB - sys_num_cur,cf_i + epochLenLB - sys_num_cur)
//     = sqrt(q_ref_p*q_ref_p + qlist_temp.at(cf_i + epochLenLB - sys_num_cur)*qlist_temp.at(cf_i + epochLenLB - sys_num_cur));

//        }
        MatrixXd temp_R = m_Rk_1;
        m_Rk_1 = tran_mat*temp_R*(tran_mat.transpose());

    }
}

// update Rk(Observation Covariance)
void QKalmanFilter::updateRk_NoCombination(QVector< SatlitData > &currEpoch, int B_len)
{
    int epochLenLB = currEpoch.length();
    if(KALMAN_MODEL::SPP_STATIC == m_KALMAN_MODEL || KALMAN_MODEL::SPP_KINEMATIC == m_KALMAN_MODEL)
    {
        m_Rk_1.resize(B_len, B_len);// this m_Rk_1 is for ISB
        m_Rk_1.setIdentity();// this m_Rk_1 is for ISB
        for (int i = 0;i < epochLenLB;i++)
        {
            SatlitData oneSatlit = currEpoch.at(i);
            if(KALMAN_SMOOTH_RANGE::SMOOTH == m_KALMAN_SMOOTH_RANGE)
            {
                m_Rk_1(2*i, 2*i) = oneSatlit.CC1_Smooth_Q;//Covariance of C1 pseudorange equations Reciprocal (noise)
                m_Rk_1(2*i+1, 2*i+1) = oneSatlit.CC2_Smooth_Q;//Covariance C2 of pseudorange equations Reciprocal (noise)
            }
            else
            {
                m_Rk_1(2*i, 2*i) = 1 / oneSatlit.SatWight;//Covariance of C1  pseudorange equations Reciprocal (noise)
                m_Rk_1(2*i+1, 2*i+1) = 1 / oneSatlit.SatWight;//Covariance of C2 pseudorange equations Reciprocal (noise)
            }
        }
    }
    else
    {
        m_Rk_1.resize(B_len, B_len);// this m_Rk_1 is for ISB
        m_Rk_1.setIdentity();// this m_Rk_1 is for ISB
        for (int i = 0;i < epochLenLB;i++)
        {
            SatlitData oneSatlit = currEpoch.at(i);
            double Q_LP_whight  = 1 / m_LP_whight;// Contrast in paper 2019.05.11 by xiaogongwei
            m_Rk_1(2*i,2*i) = Q_LP_whight / oneSatlit.SatWight;//Covariance of carrier equation Reciprocal (small noise)// 1/25000 =4e-4
            m_Rk_1(2*i+1,2*i+1) = Q_LP_whight / oneSatlit.SatWight;//Covariance of carrier equation Reciprocal (small noise)// 1/25000 =4e-4
            m_Rk_1(2*i+epochLenLB,2*i+epochLenLB) = 1 /oneSatlit.SatWight;//Covariance of pseudorange equations Reciprocal (noise)
            m_Rk_1(2*i+epochLenLB+1,2*i+epochLenLB+1) = 1 /oneSatlit.SatWight;//Covariance of pseudorange equations Reciprocal (noise)
        }
    }
}
