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
//=====================================================================================
// Host로부터 Vision이 Tray 정보를 관리하던 클래스이다
// 내부적으로 구버전 호환을 위해 로드만 하고 쓰지 않는 클래스 이다
//=====================================================================================
class DPI_DATABASE_API OldScanSpec
{
public:
    OldScanSpec();
    ~OldScanSpec();

    long m_trayPocketCountX;
    long m_trayPocketCountY;

    long m_fovPaneCountX;
    long m_fovPaneCountY;

    double m_fovPanePitchX_um;
    double m_fovPanePitchY_um;

    double m_horPaneOffset_um;
    double m_verPaneOffset_um;

    long m_currentPaneIndex;

    BOOL LinkAreaImageInfo(BOOL bSave, CiDataBase& db);

    long m_stitchCountX;
    long m_stitchCountY;
    float m_moveDistanceXbetweenFOVs_mm;
    float m_moveDistanceYbetweenFOVs_mm;

    double m_scanLength_mm; // 3D 전용
};
