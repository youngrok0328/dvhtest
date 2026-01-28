#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ TrayScanSpec
{
public:
    TrayScanSpec();
    ~TrayScanSpec();

    struct IncludeUnitInfo // 각 FOV 가 갖는 y방향 unit에 대한 정보
    {
        int m_unitID;
        Ipvm::Rect32r m_unitInFOV; // FOV 내에서의 위치

        BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    };

    struct FOVInfo
    {
        Ipvm::Point32r2 m_fovCenter; // Handler의 구동위치
        std::vector<IncludeUnitInfo> m_vecIncludeUnitInfo; // FOV가 포함하는 unit들

        BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    };

    struct UnitInfo
    {
        Ipvm::Rect32r m_position; // Unit의 위치 (debugging 용으로 사용가능, 화면에 표시)
        std::vector<int> m_fovList; // 해당 Unit 이 포함하는 FOV Index
        std::vector<std::vector<int>> m_stitchFovList; // 해당 Unit 이 포함하는 FOV Index (Y * X)

        BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    };

    std::vector<UnitInfo> m_vecUnits; // Unit이 포함하는 FOV들
    std::vector<FOVInfo> m_vecFovInfo; // 생성된 FOV들에 대한 정보

    // Tray 정보
    long m_pocketNumX; // Tray Pocket 카운트 X
    long m_pocketNumY; // Tray Pocket 카운트 Y
    float m_pocketPitchX_mm; // Tray Pocket 간의 거리 X
    float m_pocketPitchY_mm; // Tray Pocket 간의 거리 Y

    void reset();
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};

__VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const TrayScanSpec::IncludeUnitInfo& obj);
__VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, TrayScanSpec::IncludeUnitInfo& obj);
__VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const TrayScanSpec::FOVInfo& obj);
__VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, TrayScanSpec::FOVInfo& obj);
__VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const TrayScanSpec::UnitInfo& obj);
__VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, TrayScanSpec::UnitInfo& obj);
__VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const TrayScanSpec& obj);
__VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, TrayScanSpec& obj);
__VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const Ipvm::Rect32r& obj);
__VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, Ipvm::Rect32r& obj);
