//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionTrayScanSpec.h"

//CPP_2_________________________________ This project's headers
#include "OldScanSpec.h"
#include "PackageSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
InspectionAreaInfo::InspectionAreaInfo(VisionTrayScanSpec* parent, long id)
    : m_parent(parent)
    , m_id(id)
    , m_stichCountX(1)
    , m_stichCountY(1)
{
}

long InspectionAreaInfo::getStitchIndex(long fovIndex) const
{
    for (long stitchIndex = 0; stitchIndex < (long)m_fovList.size(); stitchIndex++)
    {
        if (m_fovList[stitchIndex] == fovIndex)
        {
            return stitchIndex;
        }
    }

    return -1;
}

void InspectionAreaInfo::CopyFrom(const InspectionAreaInfo& object)
{
    m_pocketArrayX = object.m_pocketArrayX;
    m_pocketArrayY = object.m_pocketArrayY;
    m_fovList = object.m_fovList;
    m_stichCountX = object.m_stichCountX;
    m_stichCountY = object.m_stichCountY;
    m_unitIndexList = object.m_unitIndexList;
}

//==================================================================================================

VisionTrayScanSpec::VisionTrayScanSpec()
    : m_horPaneOffset_um(0.)
    , m_verPaneOffset_um(0.)
    , m_currentPaneIndex(0)
    , m_batchKey(-1)
{
}

VisionTrayScanSpec::VisionTrayScanSpec(const VisionTrayScanSpec& object)
{
    *this = object;
}

VisionTrayScanSpec::~VisionTrayScanSpec()
{
}

VisionTrayScanSpec& VisionTrayScanSpec::operator=(const VisionTrayScanSpec& object)
{
    m_horPaneOffset_um = object.m_horPaneOffset_um;
    m_verPaneOffset_um = object.m_verPaneOffset_um;
    m_currentPaneIndex = object.m_currentPaneIndex;
    m_batchKey = object.m_batchKey;
    m_batchBarcode = object.m_batchBarcode;

    m_infoForInspection.clear();
    m_infoForInspection.resize(object.m_infoForInspection.size());

    for (long index = 0; index < (long)object.m_infoForInspection.size(); index++)
    {
        m_infoForInspection[index] = std::make_shared<InspectionAreaInfo>(this, index);
        m_infoForInspection[index]->CopyFrom(*object.m_infoForInspection[index]);
    }

    *((TrayScanSpec*)this) = (const TrayScanSpec&)object;

    return *this;
}

long VisionTrayScanSpec::GetInspectionItemCount() const
{
    return (long)m_infoForInspection.size();
}

const InspectionAreaInfo& VisionTrayScanSpec::GetInspectionItem(long inspectionID) const
{
    return *m_infoForInspection[inspectionID];
}

const long VisionTrayScanSpec::GetMaxStitchCountX() const
{
    long retValue = 0;

    for (auto& info : m_infoForInspection)
    {
        retValue = max(retValue, info->m_stichCountX);
    }

    return retValue;
}

const long VisionTrayScanSpec::GetMaxStitchCountY() const
{
    long retValue = 0;

    for (auto& info : m_infoForInspection)
    {
        retValue = max(retValue, info->m_stichCountY);
    }

    return retValue;
}

const Ipvm::Point32r2 VisionTrayScanSpec::GetFovCenter(long fovIndex) const
{
    return GetFovCenter(fovIndex, m_horPaneOffset_um, m_verPaneOffset_um);
}

const Ipvm::Point32r2 VisionTrayScanSpec::GetFovCenter(
    long fovIndex, double paneOffsetX_um, double paneOffsetY_um) const
{
    Ipvm::Point32r2 empty_point(Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r);
    if (fovIndex < 0 || fovIndex >= (long)m_vecFovInfo.size())
        return empty_point;

    const auto& fovInfo = m_vecFovInfo[fovIndex];
    const auto& fovCenter = fovInfo.m_fovCenter;

    return fovCenter - Ipvm::Point32r2((float)paneOffsetX_um * 0.001f, (float)paneOffsetY_um * 0.001f);
}

const Ipvm::Point32r2 VisionTrayScanSpec::GetUnitCenterInFOV(long fovIndex, long unitID) const
{
    Ipvm::Point32r2 empty_point(Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r);
    if (fovIndex < 0 || fovIndex >= (long)m_vecFovInfo.size())
        return empty_point;

    auto fovCenter = GetFovCenter(fovIndex);
    const auto& fovInfo = m_vecFovInfo[fovIndex];

    for (auto& unitInfo : fovInfo.m_vecIncludeUnitInfo)
    {
        if (unitInfo.m_unitID == unitID)
        {
            // Fov 내에 해당 Unit이 존재한다

            return m_vecUnits[unitID].m_position.CenterPoint() - fovCenter;
        }
    }

    return empty_point;
}

const Ipvm::Rect32r VisionTrayScanSpec::GetUnitValidRegionInFOV(long fovIndex, long unitID) const
{
    Ipvm::Rect32r empty_rect(0.f, 0.f, 0.f, 0.f);
    if (fovIndex < 0 || fovIndex >= (long)m_vecFovInfo.size())
        return empty_rect;

    const auto& fovInfo = m_vecFovInfo[fovIndex];

    for (auto& info : fovInfo.m_vecIncludeUnitInfo)
    {
        if (info.m_unitID == unitID)
        {
            return info.m_unitInFOV;
        }
    }

    return empty_rect;
}

const Ipvm::Rect32r VisionTrayScanSpec::GetUnitRegionInFOV(long fovIndex, long unitID) const
{
    Ipvm::Rect32r empty_rect(0.f, 0.f, 0.f, 0.f);
    if (fovIndex < 0 || fovIndex >= (long)m_vecFovInfo.size())
        return empty_rect;

    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    static const bool bIsSideVisionNumber2 = (bIsSideVision == true
        && SystemConfig::GetInstance().GetSideVisionNumber() == enSideVisionNumber::SIDE_VISIONNUMBER_2);

    auto fovCenter = GetFovCenter(fovIndex);
    const auto& fovInfo = m_vecFovInfo[fovIndex];

    for (auto& unitInfo : fovInfo.m_vecIncludeUnitInfo)
    {
        if (unitInfo.m_unitID == unitID)
        {
            // Fov 내에 해당 Unit이 존재한다
            Ipvm::Rect32r frtUnit = m_vecUnits[unitID].m_position;
            Ipvm::Point32r2 ptUnitCenter = frtUnit.CenterPoint();
            /*if (bIsSideVisionNumber2)
			{
				float fUnitSizeHalfY = m_vecUnits[unitID].m_position.Height() * 0.5f;
				frtUnit.m_left = ptUnitCenter.m_x - fUnitSizeHalfY;
				frtUnit.m_right = ptUnitCenter.m_x + fUnitSizeHalfY;
				frtUnit.m_top = ptUnitCenter.m_y - 5.f;
				frtUnit.m_bottom = ptUnitCenter.m_y + 5.f;
			}
			else */
            if (bIsSideVision)
            {
                frtUnit.m_top = ptUnitCenter.m_y - 5.f;
                frtUnit.m_bottom = ptUnitCenter.m_y + 5.f;
            }

            return frtUnit - fovCenter;
        }
    }

    return empty_rect;
}

const Ipvm::Rect32r VisionTrayScanSpec::GetUnitPocketRegionInFOV(long fovIndex, long unitID) const
{
    Ipvm::Rect32r empty_rect(0.f, 0.f, -1.f, -1.f);
    if (fovIndex < 0 || fovIndex >= (long)m_vecFovInfo.size())
        return empty_rect;

    auto fovCenter = GetFovCenter(fovIndex);
    const auto& fovInfo = m_vecFovInfo[fovIndex];

    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    static const bool bIsSideVisionNumber2 = (bIsSideVision == true
        && SystemConfig::GetInstance().GetSideVisionNumber() == enSideVisionNumber::SIDE_VISIONNUMBER_2);

    for (auto& unitInfo : fovInfo.m_vecIncludeUnitInfo)
    {
        if (unitInfo.m_unitID == unitID)
        {
            // Fov 내에 해당 Unit이 존재한다
            Ipvm::Point32r2 unitCenter = m_vecUnits[unitID].m_position.CenterPoint() - fovCenter;

            /*if (bIsSideVisionNumber2)
			{
				return Ipvm::Rect32r(
					unitCenter.m_x - m_pocketPitchY_mm * 0.5f,
					unitCenter.m_y - 5.1f,
					unitCenter.m_x + m_pocketPitchY_mm * 0.5f,
					unitCenter.m_y + 5.1f);
			}
			else */
            if (bIsSideVision)
            {
                return Ipvm::Rect32r(unitCenter.m_x - m_pocketPitchX_mm * 0.5f, unitCenter.m_y - 5.1f,
                    unitCenter.m_x + m_pocketPitchX_mm * 0.5f, unitCenter.m_y + 5.1f);
            }
            return Ipvm::Rect32r(unitCenter.m_x - m_pocketPitchX_mm * 0.5f, unitCenter.m_y - m_pocketPitchY_mm * 0.5f,
                unitCenter.m_x + m_pocketPitchX_mm * 0.5f, unitCenter.m_y + m_pocketPitchY_mm * 0.5f);
        }
    }

    return empty_rect;
}

BOOL VisionTrayScanSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    BOOL retValue = TrayScanSpec::LinkDataBase(bSave, db);

    if (!db[_T("{D678C657-8616-47A6-AED7-FB333BA59B3D}")].Link(bSave, m_horPaneOffset_um))
        m_horPaneOffset_um = 0.f;
    if (!db[_T("{C9902F24-6E03-4146-BE20-CE013883D1EE}")].Link(bSave, m_verPaneOffset_um))
        m_verPaneOffset_um = 0.f;
    if (!db[_T("{647AD61D-D6D1-4336-B251-021E5A5D036B}")].Link(bSave, m_currentPaneIndex))
        m_currentPaneIndex = 0;

    if (!bSave)
    {
        MakeInspectionInfo();
    }

    return retValue;
}

void VisionTrayScanSpec::MakeInspectionInfo()
{
    m_infoForInspection.clear();

    std::vector<std::vector<int>> fovListForInspectionItem;
    std::vector<long> unitToinspectionIndex;

    // Unit 별로 필요한 FOV Index를 수집한다
    // Unit 별로 필요한 FOV Index가 같은 놈들은 Inspection ID를 같게 만든다

    for (long unitIndex = 0; unitIndex < (long)m_vecUnits.size(); unitIndex++)
    {
        bool find = false;
        for (long inspctIndex = 0; inspctIndex < (long)fovListForInspectionItem.size(); inspctIndex++)
        {
            if (m_vecUnits[unitIndex].m_fovList == fovListForInspectionItem[inspctIndex])
            {
                unitToinspectionIndex.push_back(inspctIndex);
                find = true;
                break;
            }
        }

        if (!find)
        {
            unitToinspectionIndex.push_back((long)fovListForInspectionItem.size());
            fovListForInspectionItem.push_back(m_vecUnits[unitIndex].m_fovList);
        }
    }

    // Inspection ID별 필요한 FOV Index를 수집한다
    for (long inspectionID = 0; inspectionID < (long)fovListForInspectionItem.size(); inspectionID++)
    {
        auto inspInfo = std::make_shared<InspectionAreaInfo>(this, inspectionID);
        inspInfo->m_fovList = fovListForInspectionItem[inspectionID];
        m_infoForInspection.push_back(inspInfo);
    }

    // Inspection ID별 필요한 Unit Index를 수집한다 - sidestitch
    // m_pocketNumX = X * Y, m_pocketNumY = 1 고정
    // m_pocketNum과 inspInfo->m_pocketArray의 상관관계 파악필요

    for (long unitID = 0; unitID < (long)unitToinspectionIndex.size(); unitID++)
    {
        auto inspInfo = m_infoForInspection[unitToinspectionIndex[unitID]];
        inspInfo->m_unitIndexList.push_back(unitID);

        long pocketX = unitID % m_pocketNumX;
        long pocketY = unitID / m_pocketNumX;

        // 받은 Pocket Array에 현재 pick된 pocket 위치와 같은 위치 값이 있는지 확인
        AddNonOverlappingValueToList(inspInfo->m_pocketArrayX, pocketX);
        AddNonOverlappingValueToList(inspInfo->m_pocketArrayY, pocketY);

        auto& unitStitchFovList = m_vecUnits[unitID].m_stitchFovList;
        auto& inspectStitchCountX = inspInfo->m_stichCountX;
        auto& inspectStitchCountY = inspInfo->m_stichCountY;

        // Side일 경우 StitchCountY는 항상 1로 고정해야함 : Side는 Y 방향으로 Stitching이 필요 없음 - sidestitch
        if (false /*SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP*/)
        {
            inspectStitchCountX = max(inspectStitchCountX, (long)unitStitchFovList[0].size());
            inspectStitchCountY = 1;
        }
        else
        {
            inspectStitchCountX = max(inspectStitchCountX, (long)unitStitchFovList[0].size());
            inspectStitchCountY = max(inspectStitchCountY, (long)unitStitchFovList.size());
        }
    }
}

void VisionTrayScanSpec::RebuildParaForSide(const long i_nSideVisionNumber)
{
    UNREFERENCED_PARAMETER(i_nSideVisionNumber);

    return;

    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    // Side Vision일 경우에만 들어옴
    if (bIsSideVision == false)
        return;

    //long nRealStitchCount = (long)m_vecUnits[0].m_stitchFovList[0].size();

    // 1. 필요한 Revuild Parameter
    // ScanID, StitchIndex, StitchCountX/Y, Total Pocket Number
    //const long nTotalPocketNum = m_pocketNumX * m_pocketNumY;
    //const long nTotalGrabNum = nTotalPocketNum * nRealStitchCount;

    // m_vecUnits 내부의 m_fovList에 0, 1, 2 ... 순서대로(직렬) 넣어줘야 함
    for (long nIndex = 0; nIndex < (long)m_vecUnits.size(); nIndex++)
    {
        for (long nFovIndex = 0; nFovIndex < m_vecUnits[nIndex].m_fovList.size(); nFovIndex++)
        {
            m_vecUnits[nIndex].m_fovList[nFovIndex] = nFovIndex;
        }
    }
}

void VisionTrayScanSpec::SetFromOldRecipe(
    float fovSizeX_mm, float fovSizeY_mm, const CPackageSpec& packageSpec, const OldScanSpec& oldScanSpec)
{
    reset();

    m_horPaneOffset_um = oldScanSpec.m_horPaneOffset_um;
    m_verPaneOffset_um = oldScanSpec.m_verPaneOffset_um;
    m_currentPaneIndex = oldScanSpec.m_currentPaneIndex;

    m_pocketNumX = oldScanSpec.m_trayPocketCountX;
    m_pocketNumY = oldScanSpec.m_trayPocketCountY;
    m_pocketPitchX_mm = static_cast<float>(oldScanSpec.m_fovPanePitchX_um * 0.001f);
    m_pocketPitchY_mm = static_cast<float>(oldScanSpec.m_fovPanePitchY_um * 0.001f);

    float deviceX_mm = packageSpec.m_bodyInfoMaster->fBodySizeX / 1000.f;
    float deviceY_mm = packageSpec.m_bodyInfoMaster->fBodySizeY / 1000.f;
    float offsetX_mm = deviceX_mm * 0.5f;
    float offsetY_mm = deviceY_mm * 0.5f;

    long pocketCount = m_pocketNumX * m_pocketNumY;
    long paneCountX = oldScanSpec.m_fovPaneCountX;
    long paneCountY = oldScanSpec.m_fovPaneCountY;

    m_vecUnits.resize(pocketCount);

    for (long y = 0; y < m_pocketNumY; y++)
    {
        for (long x = 0; x < m_pocketNumX; x++)
        {
            long pocketIndex = y * m_pocketNumX + x;
            m_vecUnits[pocketIndex].m_position
                = GetUnitLocation(offsetX_mm, offsetY_mm, x, y, packageSpec, oldScanSpec);
        }
    }

    long inspectionID = 0;

    for (long sy = 0; sy < m_pocketNumY; sy += paneCountY)
    {
        for (long sx = 0; sx < m_pocketNumX; sx += paneCountX)
        {
            for (long stitchY = 0; stitchY < oldScanSpec.m_stitchCountY; stitchY++)
            {
                for (long stitchX = 0; stitchX < oldScanSpec.m_stitchCountX; stitchX++)
                {
                    AddFOV(offsetX_mm, offsetY_mm, fovSizeX_mm, fovSizeY_mm, inspectionID, sx, sy, stitchX, stitchY,
                        packageSpec, oldScanSpec);
                }
            }

            inspectionID++;
        }
    }

    MakeInspectionInfo();
}

long VisionTrayScanSpec::GetCurrentPaneID() const
{
    return max(0, m_currentPaneIndex);
}

long VisionTrayScanSpec::GetCurrentPaneID(long paneCount) const
{
    return min(paneCount - 1, max(0, m_currentPaneIndex));
}

bool VisionTrayScanSpec::SetCurrentPaneID(long nID)
{
    if (m_currentPaneIndex == nID)
    {
        return false;
    }

    m_currentPaneIndex = nID;
    return true;
}

double VisionTrayScanSpec::GetHorPaneOffset_um() const
{
    return m_horPaneOffset_um;
}

double VisionTrayScanSpec::GetVerPaneOffset_um() const
{
    return m_verPaneOffset_um;
}

void VisionTrayScanSpec::SetHorPaneOffset(const double horPaneOffset_um)
{
    m_horPaneOffset_um = horPaneOffset_um;
}

void VisionTrayScanSpec::SetVerPaneOffset(const double verPaneOffset_um)
{
    m_verPaneOffset_um = verPaneOffset_um;
}

void VisionTrayScanSpec::SetBatchKey(long key, LPCTSTR batchBarcode)
{
    m_batchKey = key;
    m_batchBarcode = batchBarcode;
}

long VisionTrayScanSpec::GetBatchKey() const
{
    return m_batchKey;
}

LPCTSTR VisionTrayScanSpec::GetBatchBarcode() const
{
    return m_batchBarcode;
}

void VisionTrayScanSpec::AddFOV(float offsetX_mm, float offsetY_mm, float fovSizeX_mm, float fovSizeY_mm,
    long inspectionID, long sx, long sy, long stitchX, long stitchY, const CPackageSpec& packageSpec,
    const OldScanSpec& oldScanSpec)
{
    UNREFERENCED_PARAMETER(inspectionID);

    TrayScanSpec::FOVInfo fovInfo;

    float deviceX_mm = 0;
    float deviceY_mm = 0;

    long paneCountX = oldScanSpec.m_fovPaneCountX;
    long paneCountY = oldScanSpec.m_fovPaneCountY;

    long stitchCountX = 0;
    long stitchCountY = 0;

    long fovIndex = 0;
    float fovOffsetX = 0;
    float fovOffsetY = 0;

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
    {
        deviceY_mm = 10.f;
        stitchCountY = 1;
        fovOffsetY = 0;

        fovInfo.m_fovCenter.m_y = offsetY_mm + m_pocketPitchY_mm * (sy + (paneCountY - 1) * 0.5f) + fovOffsetY;

        if (SystemConfig::GetInstance().GetSideVisionNumber() == SIDE_VISIONNUMBER_1)
        {
            deviceX_mm = packageSpec.m_bodyInfoMaster->fBodySizeY / 1000.f;
            stitchCountX = max(1,
                oldScanSpec
                    .m_stitchCountX); // X축으로 Stitching을 하므로 해당 데이터는 Host에서 받아오는 방식으로 수정한다.
            fovIndex = (long)m_vecFovInfo.size();
            fovOffsetX = (stitchX - (stitchCountX - 1) * 0.5f) * oldScanSpec.m_moveDistanceYbetweenFOVs_mm;
            //fovInfo.m_fovCenter.m_x = offsetX_mm + m_pocketPitchY_mm * (sx + (paneCountX - 1) * 0.5f) + fovOffsetX;
            fovInfo.m_fovCenter.m_x = offsetX_mm + m_pocketPitchX_mm * (sx + (paneCountX - 1) * 0.5f) + fovOffsetX;
        }
        else if (SystemConfig::GetInstance().GetSideVisionNumber() == SIDE_VISIONNUMBER_2)
        {
            deviceX_mm = packageSpec.m_bodyInfoMaster->fBodySizeX / 1000.f;
            stitchCountX = max(1, oldScanSpec.m_stitchCountX);
            fovIndex = (long)m_vecFovInfo.size();
            fovOffsetX = (stitchX - (stitchCountX - 1) * 0.5f) * oldScanSpec.m_moveDistanceXbetweenFOVs_mm;
            fovInfo.m_fovCenter.m_x = offsetX_mm + m_pocketPitchX_mm * (sx + (paneCountX - 1) * 0.5f) + fovOffsetX;
        }
        else
        {
            // Side Vision number가 미설정된 경우 Number1으로 설정한다.
            deviceX_mm = packageSpec.m_bodyInfoMaster->fBodySizeY / 1000.f;
            stitchCountX = max(1, oldScanSpec.m_stitchCountX);
            fovIndex = (long)m_vecFovInfo.size();
            fovOffsetX = (stitchX - (stitchCountX - 1) * 0.5f) * oldScanSpec.m_moveDistanceYbetweenFOVs_mm;
            fovInfo.m_fovCenter.m_x = offsetX_mm + m_pocketPitchY_mm * (sx + (paneCountX - 1) * 0.5f) + fovOffsetX;
        }
    }
    else
    {
        deviceX_mm = packageSpec.m_bodyInfoMaster->fBodySizeX / 1000.f;
        deviceY_mm = packageSpec.m_bodyInfoMaster->fBodySizeY / 1000.f;

        stitchCountX = max(1, oldScanSpec.m_stitchCountX);
        stitchCountY = max(1, oldScanSpec.m_stitchCountY);
        fovIndex = (long)m_vecFovInfo.size();

        fovOffsetX = (stitchX - (stitchCountX - 1) * 0.5f) * oldScanSpec.m_moveDistanceXbetweenFOVs_mm;
        fovOffsetY = (stitchY - (stitchCountY - 1) * 0.5f) * oldScanSpec.m_moveDistanceYbetweenFOVs_mm;

        fovInfo.m_fovCenter.m_x = offsetX_mm + m_pocketPitchX_mm * (sx + (paneCountX - 1) * 0.5f) + fovOffsetX;
        fovInfo.m_fovCenter.m_y = offsetY_mm + m_pocketPitchY_mm * (sy + (paneCountY - 1) * 0.5f) + fovOffsetY;
    }

    Ipvm::Rect32r validFov(-fovSizeX_mm * 0.5f, -fovSizeY_mm * 0.5f, +fovSizeX_mm * 0.5f, +fovSizeY_mm * 0.5f);

    for (long y = 0; y < paneCountY; y++)
    {
        long pocketY = sy + y;

        for (long x = 0; x < paneCountX; x++)
        {
            long pocketX = sx + x;
            long unitID = pocketY * oldScanSpec.m_trayPocketCountX + pocketX;

            Ipvm::Point32r2 paneCenter = m_vecUnits[unitID].m_position.CenterPoint() - fovInfo.m_fovCenter;

            if (m_vecUnits.size() <= unitID)
                continue;

            TrayScanSpec::IncludeUnitInfo unitInfo;

            unitInfo.m_unitID = unitID;
            unitInfo.m_unitInFOV.m_top = -deviceY_mm * 0.5f + paneCenter.m_y;
            unitInfo.m_unitInFOV.m_bottom = +deviceY_mm * 0.5f + paneCenter.m_y;
            unitInfo.m_unitInFOV.m_left = -deviceX_mm * 0.5f + paneCenter.m_x;
            unitInfo.m_unitInFOV.m_right = +deviceX_mm * 0.5f + paneCenter.m_x;

            unitInfo.m_unitInFOV &= validFov;

            fovInfo.m_vecIncludeUnitInfo.push_back(unitInfo);
            m_vecUnits[unitID].m_fovList.push_back(fovIndex);
            AddStitchList(m_vecUnits[unitID].m_stitchFovList, stitchX, stitchY, fovIndex);
        }
    }

    m_vecFovInfo.push_back(fovInfo);
}

Ipvm::Rect32r VisionTrayScanSpec::GetUnitLocation(float offsetX_mm, float offsetY_mm, long pocketX, long pocketY,
    const CPackageSpec& packageSpec, const OldScanSpec& oldScanSpec)
{
    float deviceX_mm = packageSpec.m_bodyInfoMaster->fBodySizeX / 1000.f;
    float deviceY_mm = packageSpec.m_bodyInfoMaster->fBodySizeY / 1000.f;

    float pitchX_mm = static_cast<float>(oldScanSpec.m_fovPanePitchX_um / 1000.f);
    float pitchY_mm = static_cast<float>(oldScanSpec.m_fovPanePitchY_um / 1000.f);

    Ipvm::Rect32r roi;

    float centerX = pitchX_mm * pocketX + offsetX_mm;
    float centerY = pitchY_mm * pocketY + offsetY_mm;

    roi.m_left = centerX - deviceX_mm * 0.5f;
    roi.m_top = centerY - deviceY_mm * 0.5f;
    roi.m_right = centerX + deviceX_mm * 0.5f;
    roi.m_bottom = centerY + deviceY_mm * 0.5f;

    return roi;
}

void VisionTrayScanSpec::AddNonOverlappingValueToList(std::vector<long>& dataList, long value)
{
    for (auto& listValue : dataList)
    {
        if (listValue == value)
        {
            // 이미 존재한다
            return;
        }
    }

    dataList.push_back(value);
}

void VisionTrayScanSpec::AddStitchList(
    std::vector<std::vector<int>>& stitchList, long stitchX, long stitchY, long fovIndex)
{
    if (stitchList.size() <= stitchY)
    {
        stitchList.resize(stitchY + 1);
    }

    if (stitchList[stitchY].size() <= stitchX)
    {
        stitchList[stitchY].resize(stitchX + 1, -1);
    }

    stitchList[stitchY][stitchX] = fovIndex;
}
