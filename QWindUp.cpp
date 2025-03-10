#include "QWindUp.h"


QWindUp::QWindUp(void)
{
}


QWindUp::~QWindUp(void)
{
}

//Calculate satellite antenna phase winding
double QWindUp::getWindUp(int Year,int Month,int Day,int Hours,int Minuts,double Seconds,double *StaPos,double *RecPos,double &phw,double *psunpos)
{
	gtime_t obsGPST;
    double ep[6] = {(double)Year, (double)Month, (double)Day, (double)Hours, (double)Minuts, (double)Seconds};
	obsGPST = m_qCmpClass.epoch2time(ep);
	windupcorr(obsGPST,StaPos,RecPos,&phw,psunpos);
	return phw;
}


/* phase windup correction -----------------------------------------------------
* phase windup correction (ref [7] 5.1.2)
* args   : gtime_t time     I   time (GPST)
*          double  *rs      I   satellite position (ecef) {x,y,z} (m)
*          double  *rr      I   receiver  position (ecef) {x,y,z} (m)
*          double  *phw     IO  phase windup correction (cycle)
* return : none
* notes  : the previous value of phase windup correction should be set to *phw
*          as an input. the function assumes windup correction has no jump more
*          than 0.5 cycle.
*-----------------------------------------------------------------------------*/
void QWindUp::windupcorr(gtime_t time, const double *rs, const double *rr,
	double *phw,double *psunpos)
{
    double ek[3],exs[3],eys[3],ezs[3],ess[3],exr[3],eyr[3],eks[3],ekr[3],E[9];
    double dr[3],ds[3],drs[3],r[3],pos[3],rsun[3],cosp,ph,erpv[5]={0};
    int i;

    //trace(4,"windupcorr: time=%s\n",time_str(time,0));

    /* sun position in ecef */
    if (psunpos)
    {
        rsun[0] = psunpos[0];rsun[1] = psunpos[1];rsun[2] = psunpos[2];
    }
    else
        m_qCmpClass.sunmoonpos(m_qCmpClass.gpst2utc(time),erpv,rsun,NULL,NULL);

    /* unit vector satellite to receiver */
    for (i=0;i<3;i++) r[i]=rr[i]-rs[i];
    if (!m_qCmpClass.normv3(r,ek)) return;

    /* unit vectors of satellite antenna */
    for (i=0;i<3;i++) r[i]=-rs[i];
    if (!m_qCmpClass.normv3(r,ezs)) return;
    for (i=0;i<3;i++) r[i]=rsun[i]-rs[i];
    if (!m_qCmpClass.normv3(r,ess)) return;
    m_qCmpClass.cross3(ezs,ess,r);
    if (!m_qCmpClass.normv3(r,eys)) return;
    m_qCmpClass.cross3(eys,ezs,exs);

    /* unit vectors of receiver antenna */
    m_qCmpClass.ecef2pos(rr,pos);
    m_qCmpClass.xyz2enu(pos,E);
    exr[0]= E[1]; exr[1]= E[4]; exr[2]= E[7]; /* x = north */
    eyr[0]=-E[0]; eyr[1]=-E[3]; eyr[2]=-E[6]; /* y = west  */

    /* phase windup effect */
    m_qCmpClass.cross3(ek,eys,eks);
    m_qCmpClass.cross3(ek,eyr,ekr);
    for (i=0;i<3;i++) {
        ds[i]=exs[i]-ek[i]*m_qCmpClass.dot(ek,exs,3)-eks[i];
        dr[i]=exr[i]-ek[i]*m_qCmpClass.dot(ek,exr,3)+ekr[i];
    }
    cosp=m_qCmpClass.dot(ds,dr,3)/m_qCmpClass.norm(ds,3)/m_qCmpClass.norm(dr,3);

    //RTKLAB
    if      (cosp<-1.0) cosp=-1.0;
    else if (cosp> 1.0) cosp= 1.0;
    ph=acos(cosp)/2.0/MM_PI;
    m_qCmpClass.cross3(ds,dr,drs);
    if (m_qCmpClass.dot(ek,drs,3)<0.0) ph=-ph;

    *phw = ph + floor(*phw - ph + 0.5); /* in cycle */

////2020.12.24 23z   rtklib
//    double exs[3],eys[3],ek[3],exr[3],eyr[3],eks[3],ekr[3],E[9];
//    double dr[3],ds[3],drs[3],r[3],pos[3],cosp,ph;
//    int i;

////    if (opt<=0) return 1; /* no phase windup */

//    if (m_qCmpClass.norm(rr,3)<=0.0) return;

//    /* satellite yaw attitude model */
//    if (!sat_yaw(time,rs,exs,eys)) return;

//    /* unit vector satellite to receiver */
//    for (i=0;i<3;i++) r[i]=rr[i]-rs[i];
//    if (!m_qCmpClass.normv3(r,ek)) return;

//    /* unit vectors of receiver antenna */
//    m_qCmpClass.ecef2pos(rr,pos);
//    m_qCmpClass.xyz2enu(pos,E);
//    exr[0]= E[1]; exr[1]= E[4]; exr[2]= E[7]; /* x = north */
//    eyr[0]=-E[0]; eyr[1]=-E[3]; eyr[2]=-E[6]; /* y = west  */

//    /* phase windup effect */
//    m_qCmpClass.cross3(ek,eys,eks);
//    m_qCmpClass.cross3(ek,eyr,ekr);
//    for (i=0;i<3;i++) {
//        ds[i]=exs[i]-ek[i]*m_qCmpClass.dot(ek,exs,3)-eks[i];
//        dr[i]=exr[i]-ek[i]*m_qCmpClass.dot(ek,exr,3)+ekr[i];
//    }
//    cosp=m_qCmpClass.dot(ds,dr,3)/m_qCmpClass.norm(ds,3)/m_qCmpClass.norm(dr,3);
//    if      (cosp<-1.0) cosp=-1.0;
//    else if (cosp> 1.0) cosp= 1.0;
//    //acos£¨-1£© invalid
//    if (fabs(fabs(cosp)-1.0)<1.0e-10) return;

//    ph=acos(cosp)/2.0/MM_PI;
//    m_qCmpClass.cross3(ds,dr,drs);
//    if (m_qCmpClass.dot(ek,drs,3)<0.0) ph=-ph;

//    *phw=ph+floor(*phw-ph+0.5); /* in cycle */

}
//2020.12.24 23z rtklib
/* nominal yaw-angle ---------------------------------------------------------*/
double QWindUp::yaw_nominal(double beta, double mu)
{
    if (fabs(beta)<1E-12&&fabs(mu)<1E-12) return MM_PI;
    return atan2(-tan(beta),sin(mu))+MM_PI;
}
/* yaw-angle of satellite ----------------------------------------------------*/
int QWindUp::yaw_angle(double beta, double mu, double *yaw)
{
    *yaw=yaw_nominal(beta,mu);
    return 1;
}
/* satellite attitude model --------------------------------------------------*/
int QWindUp::sat_yaw(gtime_t time, const double *rs, double *exs, double *eys)
{
    double rsun[3],ri[6],es[3],esun[3],n[3],p[3],en[3],ep[3],ex[3],E,beta,mu;
    double yaw,cosy,siny,erpv[5]={0};
    int i;

    m_qCmpClass.sunmoonpos(m_qCmpClass.gpst2utc(time),erpv,rsun,NULL,NULL);

    /* beta and orbit angle */
    matcpy(ri,rs,6,1);
//    if(isnan(ri[4])) ri[4] = 0;
//    if(isnan(ri[5])) ri[5] = 0;

    ri[3]-=OMGE*ri[1];
    ri[4]+=OMGE*ri[0];
    m_qCmpClass.cross3(ri,ri+3,n);
    m_qCmpClass.cross3(rsun,n,p);
    if (!m_qCmpClass.normv3(rs,es)||!m_qCmpClass.normv3(rsun,esun)||!m_qCmpClass.normv3(n,en)||
        !m_qCmpClass.normv3(p,ep)) return 0;
    beta=MM_PI/2.0-acos(m_qCmpClass.dot(esun,en,3));
    E=acos(m_qCmpClass.dot(es,ep,3));
    mu=MM_PI/2.0+(m_qCmpClass.dot(es,esun,3)<=0?-E:E);
    if      (mu<-MM_PI/2.0) mu+=2.0*MM_PI;
    else if (mu>=MM_PI/2.0) mu-=2.0*MM_PI;

    /* yaw-angle of satellite */
    if (!yaw_angle(beta,mu,&yaw)) return 0;

    /* satellite fixed x,y-vector */
    m_qCmpClass.cross3(en,es,ex);
    cosy=cos(yaw);
    siny=sin(yaw);
    for (i=0;i<3;i++) {
        exs[i]=-siny*en[i]+cosy*ex[i];
        eys[i]=-cosy*en[i]-siny*ex[i];
    }
    return 1;
}

void QWindUp::matcpy(double *A, const double *B, int n, int m)
{
    memcpy(A,B,sizeof(double)*n*m);
}
