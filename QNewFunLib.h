
#ifndef QNEWFUNLIB_H
#define QNEWFUNLIB_H

// You can add some new functions here
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include<Eigen/Dense>
using namespace Eigen;

class QNewFunLib
{
// function part
public:
	QNewFunLib(void);
	~QNewFunLib(void);
	void computeCrossPoint(Vector3d Recv1Pos,Vector3d Recv2Pos,Vector3d SatPos,Vector3d *crossPoint,Vector3d *talpha = NULL);//计算SatPos在1,2号接收机直线上的投影点
    static bool deleteDirectory(const QString &path);
private:

// data section
public:

private:
};
#endif
