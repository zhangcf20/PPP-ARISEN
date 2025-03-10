#include "qgetde405.h"

QGetDE405::QGetDE405()
{

}
QGetDE405::~QGetDE405()
{
    if(!m_isCloseFile)
        closeDe405();
}

// init variables
void QGetDE405::initVar()
{
    m_isRead = false;
    m_isCloseFile = true;
    au = 0.0;
    kmps = 0.0;
    for(int i = 0;i < JPL_MAX_N_CONSTANTS;i ++)
    {
        for(int j = 0; j < 6; j++)
        {
            nams[i][j] = 0;
        }
        vals[i] = 0;
    }
}

// read DE405 files
bool QGetDE405::readDE405(char *de405path)
{
    char *eph_names = de405path;
    ephem = jpl_init_ephemeris( eph_names, nams, vals);
    if(ephem)
    {
        au= jpl_get_double( ephem, JPL_EPHEM_AU_IN_KM);
        kmps = au/86400.0;//convert au/day to km/s
        m_isRead = true;
        m_isCloseFile = false;
        return true;
    }
    else
    {
        m_isRead = false;
        m_isCloseFile = true;
        return false;
    }
}
// close de405 file
bool QGetDE405::closeDe405()
{
    if(!m_isCloseFile)
        jpl_close_ephemeris(ephem);
    return true;
}

/*****************************************************************************
**    INPUT sequence parameters:                                          **
**                                                                          **
**      JD = (double) julian ephemeris date at which interpolation          **
**           is wanted.                                                     **
**                                                                          **
**    ntarg = integer number of 'target' point.                             **
**                                                                          **
**    ncent = integer number of center point.                               **
**                                                                          **
**    The numbering convention for 'ntarg' and 'ncent' is:                  **
**                                                                          **
**            1 = mercury           8 = neptune                             **
**            2 = venus             9 = pluto                               **
**            3 = earth            10 = moon                                **
**            4 = mars             11 = sun                                 **
**            5 = jupiter          12 = solar-system barycenter             **
**            6 = saturn           13 = earth-moon barycenter               **
**            7 = uranus           14 = nutations (longitude and obliq)     **
**                                 15 = librations, if on eph. file         **
**                                 16 = lunar mantle omega_x,omega_y,omega_z**
**                                 17 = TT-TDB, if on eph. file             **
**                                                                          **
**            (If nutations are wanted, set ntarg = 14.                     **
**             For librations, set ntarg = 15. set ncent= 0.                **
**             For TT-TDB,  set ntarg = 17.  I've not actually              **
**             seen an ntarg = 16 case yet.)                                **
**                                                                          **
**                                                                          **
**     calc_velocity = integer flag;  if nonzero,  velocities will be       **
**           computed,  otherwise not.                                      **
**                                                                          **
**    OUTPUT:
**    vector<double> = store positions or velocity in J2000.0 coord system
*****************************************************************************/
vector<double> QGetDE405::getPosition(double JD, int ntarg, int nctr, int calc_velocity)
{
    vector<double> getPos_Vel;
    if(!m_isRead || m_isCloseFile)
    {
        for(int i = 0; i < 3;i++) getPos_Vel.push_back(0.0);
        return getPos_Vel;
    }
    double rrd[6] = {0};
    int len_rrd = 3;
    if(calc_velocity !=0)
        len_rrd = 6;
    jpl_pleph(ephem, JD, ntarg, nctr, rrd, calc_velocity);//the output in au  and au/day
    // covert rrd to position or velocity
    rrd[0] = rrd[0] * au; rrd[1] = rrd[1] * au; rrd[2] = rrd[2] * au;
    rrd[3] = rrd[3] * kmps; rrd[4] = rrd[4] * kmps; rrd[5] = rrd[5] * kmps;
    for(int i = 0; i < len_rrd; i++)
        getPos_Vel.push_back(rrd[i]*1000);
    return getPos_Vel;
}
