#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class DPI_DATABASE_API SlitBeam3DParameters
{
public:
    SlitBeam3DParameters(void);
    ~SlitBeam3DParameters(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    long GetPhysicalScanLength_px() const;
    long GetLogicalScanLength_px() const;

    // Illumination parameter
    double m_illuminationTime_ms;
    BYTE m_illuminationValue;

    // Scanning parameter
    BOOL m_cameraVerticalBinning;
    BYTE m_scanStep_um;
    double m_scanDepth_um;
    double m_scanLength_mm;

    // Algorithm parameter
    double m_noiseThreshold; //Slit Detecting을 위한 Threshold Value
    double m_noiseThreshold_FinalFiltering; //kircheis_SDK127
    BYTE m_beamThickness_um;

    double m_illuminationTimeDefused_ms;

    int m_cameraGrabMode;

    double m_noiseThreshold_Defused1;
    double m_noiseThreshold_FinalFiltering_Defused1;
    BYTE m_beamThickness_Defused1_um;

    double m_noiseThreshold_Defused2;
    double m_noiseThreshold_FinalFiltering_Defused2;
    BYTE m_beamThickness_Defused2_um;

};
