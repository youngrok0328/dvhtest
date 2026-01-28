#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DPI_SYNCCONTROLLERDEF_API IlluminationTable
{
public:
    IlluminationTable();
    ~IlluminationTable();

    enum fixColumn
    {
        FX_TIME_MS,
        FX_CH_1,
        FX_CH_2,
        FX_CH_3,
        FX_CH_4,
        FX_CH_5,
        FX_CH_6,
        FX_CH_7,
        FX_CH_8,
        FX_CH_9,
        FX_CH_10,
        FX_CH_11,
        FX_CH_12,
        FX_CH_13,
        FX_CH_14,
        FX_CH_15,
        FX_CH_16,
        FX_END,
    };

    bool Load(LPCTSTR pathName, CString& errorDesc);

    void CalcIlluminationGain(const IlluminationTable& refTable, double gains[LED_ILLUM_CHANNEL_MAX]);
    void CalcIlluminationGainRear(const IlluminationTable& refTable, double gains[LED_ILLUM_CHANNEL_MAX]);

    //SDY_Side Vision Calibration
    bool CalcCurveFitting(std::vector<Ipvm::LineEq32r>& o_vecIllumFitting,
        std::vector<Curve2DEq>& o_vecIllumCurveFitting, std::vector<Curve2DEq>& o_vecIllumCurveFittingReverse,
        std::vector<BOOL> vecbvalidillumination, double timeToIntensity[LED_ILLUM_CHANNEL_MAX], CString& o_errorDesc);
    bool CalcMirrorFitting(std::vector<Ipvm::LineEq32r>& vecIllumFitting, std::vector<BOOL> vecbvalidillumination,
        double timeToIntensity[LED_ILLUM_CHANNEL_MAX], CString& errorDesc);

    double GetTimeToIntensity(long nCh);
    double GetTimeToIntensity_R(long nCh);

    std::vector<double> m_illuminationTimes_ms;
    std::vector<double> m_intensities_ms[LED_ILLUM_CHANNEL_MAX];

private: //kircheis_ImproveIllumCal
    double m_timeToIntensity[LED_ILLUM_CHANNEL_MAX];

    double m_timeToIntensity_R[LED_ILLUM_CHANNEL_MAX]; //SDY_Side Vision Calibration
};
