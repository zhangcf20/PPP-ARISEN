
#ifndef QPSEUDOSMOOTH_H
#define QPSEUDOSMOOTH_H
#include "QGlobalDef.h"

// add by xiaogongwei 2018.11.20
class QPseudoSmooth
{
public:
    enum SmoothMooth
    {
        Hatch = 0
    };
    QPseudoSmooth();
    bool SmoothPesudoRange(QVector < SatlitData > &prevEpochSatlitData, QVector < SatlitData > &epochSatlitData);
    bool isSatChanged(const QVector<SatlitData> &preEpoch, const QVector<SatlitData> &currEpoch, QVector< int > &changePrnFlag);
private:
    double m_wa, m_wb;// soomth paramter
    SmoothMooth m_method;
};

#endif // QPSEUDOSMOOTH_H
