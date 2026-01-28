//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionComponent3D.h"

//CPP_2_________________________________ This project's headers
#include "ComponentAlign.h"
#include "DlgVisionInspectionComponent3D.h"
#include "VisionInspectionComponent3DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r3.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define _PI_ 3.1415926535897932384626433832795
#define _2PI_ (2 * _PI_)
#define PI_ED_DIR_OUTER 0 // 안에서 밖으로
#define PI_ED_DIR_INNER 1 // 밖에서 안으로

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionComponent3D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

BOOL VisionInspectionComponent3D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck TimeInspTotalTime;
    Ipvm::TimeCheck time;

    BOOL bPass = FALSE;

    ResetResult();
    m_vecrtReject.clear();

    Ipvm::TimeCheck TimeAlign;
    if (DoAlign(detailSetupMode))
    {
        AddProcessingFunctionExcuteTimeLog(m_strModuleName, _T("AlignTime"), CAST_FLOAT(TimeAlign.Elapsed_ms()));

        Ipvm::TimeCheck TimeInspection;
        bPass = DoInsp(detailSetupMode);
        AddProcessingFunctionExcuteTimeLog(m_strModuleName, _T("InspTime"), CAST_FLOAT(TimeInspection.Elapsed_ms()));
    }

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("InspTotalTime"), CAST_FLOAT(TimeInspTotalTime.Elapsed_ms()));

    return TRUE;
}

void VisionInspectionComponent3D::GetBallInfo(std::vector<Ipvm::Point32r3>& vec3DObjectPos)
{
    auto visionDebugInfos = m_visionUnit.GetVisionDebugInfos();

    std::vector<Ipvm::Point32r3> vec3DInfo;

    for (auto* duebInfo : visionDebugInfos)
    {
        if (duebInfo->pData == nullptr)
            continue;

        else if (duebInfo->strDebugInfoName == "Object Info")
        {
            long nDataNum = duebInfo->nDataNum;

            vec3DInfo.resize(nDataNum);
            vec3DObjectPos.resize(nDataNum);

            Ipvm::Point32r3* p3DInfo = &vec3DInfo[0];

            p3DInfo = (Ipvm::Point32r3*)duebInfo->pData;
            for (long i = 0; i < nDataNum; i++)
            {
                vec3DObjectPos[i] = p3DInfo[i];
            }
        }
    }
}

BOOL VisionInspectionComponent3D::GetBodyAlignInfo(
    float& o_fAngle, Ipvm::Rect32r& o_frtBody, Ipvm::Point32r2& o_fptCenter)
{
    FPI_RECT* psfrtBody = NULL;
    VisionAlignResult* pEdgeAlignResult = NULL;
    long nDataNum = 0;
    //void* pData = m_visionUnit.GetVisionDebugInfo(m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EdgeBody Rect"), nDataNum);
    void* pData = m_visionUnit.GetVisionDebugInfo(
        _VISION_INSP_GUID_FIDUCIAL_ALIGN, _T("Apply PAD Align to Body ROI"), nDataNum);

    if (pData != nullptr && nDataNum > 0)
        psfrtBody = (FPI_RECT*)pData;

    {
        nDataNum = 0;
        pData = m_visionUnit.GetVisionDebugInfo(
            m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
        if (pData != nullptr && nDataNum > 0)
            pEdgeAlignResult = (VisionAlignResult*)pData;
        else
            return FALSE;
    }

    FPI_RECT sfrtBody = (psfrtBody != NULL)
        ? *psfrtBody
        : FPI_RECT(pEdgeAlignResult->fptLT, pEdgeAlignResult->fptRT, pEdgeAlignResult->fptLB, pEdgeAlignResult->fptRB);

    o_frtBody = sfrtBody.GetFRect();
    o_fptCenter = o_frtBody.CenterPoint();

    const float angleLower = (float)atan2(pEdgeAlignResult->fptRB.m_y - pEdgeAlignResult->fptLB.m_y,
        pEdgeAlignResult->fptRB.m_x - pEdgeAlignResult->fptLB.m_x);
    const float angleUpper = (float)atan2(pEdgeAlignResult->fptRT.m_y - pEdgeAlignResult->fptLT.m_y,
        pEdgeAlignResult->fptRT.m_x - pEdgeAlignResult->fptLT.m_x);
    o_fAngle = (angleLower + angleUpper) / 2.f;

    m_sfrtPackageBody = sfrtBody.Rotate(o_fAngle);

    return TRUE;
}

void VisionInspectionComponent3D::MakePackageInfo(
    const bool detailSetupMode, float i_fBodyAngle_rad, Ipvm::Point32r2 i_fptBodyCenter)
{
    m_vecsPassiveInfoDB.clear();
    m_vecsPassiveTypes.clear();
    m_mapPassiveTypesCount.clear();

    std::vector<FPI_RECT> vecsrtPassive(0);
    for (auto DMS_PassiveData : m_packageSpec.vecDMSPassive)
    {
        sPassive_InfoDB sCurPassive_InfoDB;

        sCurPassive_InfoDB.ePassiveType = enumPassiveTypeDefine(DMS_PassiveData.nPassiveType);
        sCurPassive_InfoDB.eComponentType = eComponentTypeDefine(DMS_PassiveData.nComponentType);
        sCurPassive_InfoDB.strCompName = DMS_PassiveData.strCompID;
        sCurPassive_InfoDB.strCompSpec = DMS_PassiveData.strCompSpec;
        sCurPassive_InfoDB.strCompType = DMS_PassiveData.strCompType;
        sCurPassive_InfoDB.fSpecPos_mm = Ipvm::Point32r2(DMS_PassiveData.fChipOffsetX, DMS_PassiveData.fChipOffsetY);
        sCurPassive_InfoDB.fPassiveSpecWidth_mm = DMS_PassiveData.fChipWidth;
        sCurPassive_InfoDB.fPassiveSpecLength_mm = DMS_PassiveData.fChipLength;
        sCurPassive_InfoDB.fPassiveAngle = DMS_PassiveData.fChipAngle;
        sCurPassive_InfoDB.fPassiveHeight_mm = DMS_PassiveData.fChipThickness;
        sCurPassive_InfoDB.fPassivePAD_Gap_mm = DMS_PassiveData.fChipPAD_Gap;
        sCurPassive_InfoDB.fPassivePAD_Width_mm = DMS_PassiveData.fChipPAD_Width;
        sCurPassive_InfoDB.fPassivePAD_Length_mm = DMS_PassiveData.fChipPAD_Length;
        sCurPassive_InfoDB.fPassiveElectrodeWidth_mm = DMS_PassiveData.fElectWidth;
        sCurPassive_InfoDB.fPassiveElectrodeHeight_mm = DMS_PassiveData.fElectThickness;

        sCurPassive_InfoDB.sfrtSpecROI_mm = DMS_PassiveData.sfrtChip;
        sCurPassive_InfoDB.sfrtSpecROI_px
            = GetSpecROI_px(sCurPassive_InfoDB.sfrtSpecROI_mm, i_fBodyAngle_rad, i_fptBodyCenter);

        sCurPassive_InfoDB.fSpecPos_px = Ipvm::Point32r2(
            sCurPassive_InfoDB.sfrtSpecROI_px.GetCenter().m_x, sCurPassive_InfoDB.sfrtSpecROI_px.GetCenter().m_y);
        sCurPassive_InfoDB.fSpecPos_um
            = ConvertSpecPosMMtoUM(DMS_PassiveData.fChipOffsetX, DMS_PassiveData.fChipOffsetY);

        sCurPassive_InfoDB.pResult = NULL;

        vecsrtPassive.push_back(sCurPassive_InfoDB.sfrtSpecROI_px);

        m_vecsPassiveInfoDB.push_back(sCurPassive_InfoDB);

        std::vector<CString>::iterator itr
            = find(m_vecsPassiveTypes.begin(), m_vecsPassiveTypes.end(), DMS_PassiveData.strCompSpec);

        if ((long)m_vecsPassiveTypes.size() == 0 || itr == m_vecsPassiveTypes.end())
        {
            m_vecsPassiveTypes.push_back(DMS_PassiveData.strCompSpec);
        }

        m_mapPassiveTypesCount[DMS_PassiveData.strCompSpec] += 1;
    }

    SetDebugInfoItem(detailSetupMode, _T("Spec Passive ROI"), vecsrtPassive);
}

FPI_RECT VisionInspectionComponent3D::GetSpecROI_px(
    FPI_RECT i_sfrtSpecROI_mm, float i_fBodyAngle_rad, Ipvm::Point32r2 i_fptBodyCenter)
{
    const auto& mm2px = getScale().mmToPixel();

    FPI_RECT sfrtSpecROI_px;

    sfrtSpecROI_px.fptLT.m_x = i_fptBodyCenter.m_x + (i_sfrtSpecROI_mm.fptLT.m_x * mm2px.m_x);
    sfrtSpecROI_px.fptLT.m_y = i_fptBodyCenter.m_y - (i_sfrtSpecROI_mm.fptLT.m_y * mm2px.m_y);
    sfrtSpecROI_px.fptRT.m_x = i_fptBodyCenter.m_x + (i_sfrtSpecROI_mm.fptRT.m_x * mm2px.m_x);
    sfrtSpecROI_px.fptRT.m_y = i_fptBodyCenter.m_y - (i_sfrtSpecROI_mm.fptRT.m_y * mm2px.m_y);
    sfrtSpecROI_px.fptLB.m_x = i_fptBodyCenter.m_x + (i_sfrtSpecROI_mm.fptLB.m_x * mm2px.m_x);
    sfrtSpecROI_px.fptLB.m_y = i_fptBodyCenter.m_y - (i_sfrtSpecROI_mm.fptLB.m_y * mm2px.m_y);
    sfrtSpecROI_px.fptRB.m_x = i_fptBodyCenter.m_x + (i_sfrtSpecROI_mm.fptRB.m_x * mm2px.m_x);
    sfrtSpecROI_px.fptRB.m_y = i_fptBodyCenter.m_y - (i_sfrtSpecROI_mm.fptRB.m_y * mm2px.m_y);

    Ipvm::Point32r2 ObjCenter_px = sfrtSpecROI_px.GetCenter();
    Ipvm::Point32r2 ObjCenterOrgin_px = ObjCenter_px;
    CPI_Geometry::RotatePoint(ObjCenter_px, i_fBodyAngle_rad, i_fptBodyCenter);
    float fOffsetX = ObjCenter_px.m_x - ObjCenterOrgin_px.m_x;
    float fOffsetY = ObjCenter_px.m_y - ObjCenterOrgin_px.m_y;

    sfrtSpecROI_px.fptLT.m_x += fOffsetX;
    sfrtSpecROI_px.fptLT.m_y += fOffsetY;
    sfrtSpecROI_px.fptRT.m_x += fOffsetX;
    sfrtSpecROI_px.fptRT.m_y += fOffsetY;
    sfrtSpecROI_px.fptLB.m_x += fOffsetX;
    sfrtSpecROI_px.fptLB.m_y += fOffsetY;
    sfrtSpecROI_px.fptRB.m_x += fOffsetX;
    sfrtSpecROI_px.fptRB.m_y += fOffsetY;

    return sfrtSpecROI_px;
}

BOOL VisionInspectionComponent3D::DoAlign(const bool detailSetupMode)
{
    if (m_pVisionInspDlg != NULL && detailSetupMode)
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();

    if (!MakePreInspInfo(detailSetupMode))
        return FALSE;

    std::vector<PI_RECT> vecsrtPackageBody(1);
    vecsrtPackageBody[0] = m_sfrtPackageBody.GetSPI_RECT();
    SetDebugInfoItem(detailSetupMode, _T("PACKAGE BODY ROI"), vecsrtPackageBody);

    // Passive Debug Info
    std::vector<PI_RECT> debug_vecsrtPassiveChip;
    std::vector<PI_RECT> debug_vecsrtPassiveElect;
    std::vector<PI_RECT> debug_vecsrtPassiveBody;

    std::vector<PI_RECT> debug_vecsrtPassiveValidElect;
    std::vector<PI_RECT> debug_vecsrtPassiveValidBody;

    // Land Debug Info
    std::vector<PI_RECT> debug_vecsrtLand;

    // Component Debug Info
    std::vector<PI_RECT> debug_vecsrtComponent;

    m_vecsCompAlignResult.clear();
    m_vecsCompAlignResult.resize(m_vecsPassiveInfoDB.size());

    Ipvm::Image32r curZmap = getImageLotInsp().m_zmapImage;
    Ipvm::Image16u curWIntensity = getImageLotInsp().m_vmapImage;
    Ipvm::Image8u curHMap = getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][0];
    Ipvm::Image8u curByInten = getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][1];

    Ipvm::Image8u AlignimageBuffer;
    if (getReusableMemory().GetInspByteImage(AlignimageBuffer) == false)
        return FALSE;

    AlignimageBuffer.FillZero();

    for (long nPassive = 0; nPassive < m_vecsPassiveInfoDB.size(); nPassive++)
    {
        if (m_pCompAlign->DoAlign(curZmap, curWIntensity, curHMap, curByInten, AlignimageBuffer, getScale(),
                &m_vecsPassiveInfoDB[nPassive]))
        {
            auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];

            SComponentAlignResult* pCompAlignResut
                = &m_vecsCompAlignResult[nPassive]; // 기존 사용하던 new로 지속적으로 할당하는 부분을 vecotr로 수정
            //SComponentAlignResult* pCompAlignResut = new SComponentAlignResult;

            *pCompAlignResut = m_pCompAlign->m_result;
            PassiveInfoDB->pResult = pCompAlignResut;

            MakeDebugROIs(PassiveInfoDB, &debug_vecsrtPassiveChip, &debug_vecsrtPassiveElect, &debug_vecsrtPassiveBody);
            MakeDebugCalcROIs(
                PassiveInfoDB, &debug_vecsrtComponent, &debug_vecsrtPassiveValidElect, &debug_vecsrtPassiveValidBody);

            std::vector<Ipvm::Rect32s> vecrtLand = pCompAlignResut->vecrtSubstrateArea;
            long nLandNum = (long)vecrtLand.size();
            if (PassiveInfoDB->fPassiveAngle == 0.f || PassiveInfoDB->fPassiveAngle == 90.f)
            {
                for (long nLand = 0; nLand < nLandNum; nLand++)
                {
                    FPI_RECT sfrtLand
                        = FPI_RECT(Ipvm::Point32r2((float)vecrtLand[nLand].m_left, (float)vecrtLand[nLand].m_top),
                            Ipvm::Point32r2((float)vecrtLand[nLand].m_right, (float)vecrtLand[nLand].m_top),
                            Ipvm::Point32r2((float)vecrtLand[nLand].m_left, (float)vecrtLand[nLand].m_bottom),
                            Ipvm::Point32r2((float)vecrtLand[nLand].m_right, (float)vecrtLand[nLand].m_bottom));

                    debug_vecsrtLand.push_back(sfrtLand.GetSPI_RECT());
                }
            }
            else
            {
                Ipvm::Point32r2 fptRotateCenter = PassiveInfoDB->sfrtSpecROI_px.GetFRect().CenterPoint();

                std::vector<FPI_RECT> vecfsrtLand(nLandNum);
                for (long nLand = 0; nLand < nLandNum; nLand++)
                {
                    FPI_RECT sfrtLand
                        = FPI_RECT(Ipvm::Point32r2((float)vecrtLand[nLand].m_left, (float)vecrtLand[nLand].m_top),
                            Ipvm::Point32r2((float)vecrtLand[nLand].m_right, (float)vecrtLand[nLand].m_top),
                            Ipvm::Point32r2((float)vecrtLand[nLand].m_left, (float)vecrtLand[nLand].m_bottom),
                            Ipvm::Point32r2((float)vecrtLand[nLand].m_right, (float)vecrtLand[nLand].m_bottom));
                    vecfsrtLand[nLand] = sfrtLand;
                }
                for (long nLand = 0; nLand < nLandNum; nLand++)
                {
                    FPI_RECT sfrtElectRotate
                        = vecfsrtLand[nLand].Rotate(PassiveInfoDB->fPassiveAngle * DEF_DEG_TO_RAD, fptRotateCenter);
                    debug_vecsrtLand.push_back(sfrtElectRotate.GetSPI_RECT());
                }
            }
        }
    }

    SetDebugInfoItem(detailSetupMode, _T("Passive Chip ROI"), debug_vecsrtPassiveChip, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("Passive Electrode ROI"), debug_vecsrtPassiveElect);
    SetDebugInfoItem(detailSetupMode, _T("Passive Body ROI"), debug_vecsrtPassiveBody);

    SetDebugInfoItem(detailSetupMode, _T("Substrate ROI"), debug_vecsrtLand);

    // Valid Rect
    SetDebugInfoItem(detailSetupMode, _T("Valid Component ROI"), debug_vecsrtComponent, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("Passive Valid Electrode ROI"), debug_vecsrtPassiveValidElect);
    SetDebugInfoItem(detailSetupMode, _T("Passive Valid Body ROI"), debug_vecsrtPassiveValidBody);

    return TRUE;
}

BOOL VisionInspectionComponent3D::DoInsp(const bool /*detailSetupMode*/)
{
    BOOL bResult = TRUE;
    long nPassiveNum = (long)m_vecsPassiveInfoDB.size();

    std::vector<bool> missingCheck;
    missingCheck.resize(nPassiveNum, false);

    // {{Basic Inspection
    bResult &= Insp_CompMissing(missingCheck);
    bResult &= Insp_CompHeight(missingCheck);
    bResult &= Insp_CompQuality(missingCheck);
    //}}

    // {{Detail Inspection
    bResult &= Insp_ElectrodeHeight(missingCheck);
    bResult &= Insp_ElectrodeTilt(missingCheck);

    bResult &= Insp_BodyHeight(missingCheck);
    bResult &= Insp_BodyTilt(missingCheck);

    if (m_pVisionInspDlg)
    {
        if (m_pVisionInspDlg->IsWindowVisible())
            m_pVisionInspDlg->UpdateChipList();
    }

    return bResult;
}

BOOL VisionInspectionComponent3D::MakePreInspInfo(const bool detailSetupMode)
{
    float fBodyAngle_rad(0.f);
    Ipvm::Rect32r frtBody(0.f, 0.f, 0.f, 0.f);
    Ipvm::Point32r2 fptBodyCenter(0.f, 0.f);
    if (!GetBodyAlignInfo(fBodyAngle_rad, frtBody, fptBodyCenter))
    {
        return FALSE;
    }

    // Package 정보 생성.
    MakePackageInfo(detailSetupMode, fBodyAngle_rad, fptBodyCenter);
    SetPackageInfo_SpecLink();

    if (m_pVisionInspDlg && m_pVisionInspDlg->IsWindowVisible())
    {
        m_pVisionInspDlg->UpdateChipList();
    }

    return TRUE;
}

BOOL VisionInspectionComponent3D::Insp_CompMissing(std::vector<bool>& o_missingCheck)
{
    auto* result = m_resultGroup.GetResultByName(g_szCompInspection3DName[COMP_INSPECTION_3D_COMP_MISSING]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    long nPassiveNum = (long)m_vecsPassiveInfoDB.size();

    result->Clear();
    result->Resize(nPassiveNum);

    long nTotalResult = NOT_MEASURED;
    long nMissingCount(0);
    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        SComponentAlignResult* pAlignResult = (SComponentAlignResult*)PassiveInfoDB->pResult;

        if (pAlignResult == NULL)
        {
            // Not Mearsure일 경우 Reject 처리..
            long nResult = REJECT;
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nPassive, PassiveInfoDB->strCompName,
                PassiveInfoDB->strCompSpec, PassiveInfoDB->strCompType, CAST_FLOAT(nMissingCount), *spec,
                PassiveInfoDB->fSpecPos_um.m_x, PassiveInfoDB->fSpecPos_um.m_y, 0.f,
                nResult); //kircheis_VSV//kircheis_MED2.5

            result->SetRect(nPassive, PassiveInfoDB->sfrtSpecROI_px.GetExtCRect());

            if (nResult != PASS)
            {
                o_missingCheck[nPassive] = true;
                m_vecrtReject.push_back(PassiveInfoDB->sfrtSpecROI_px.GetExtCRect());
            }

            nMissingCount++;
            continue;
        }

        float fValue = pAlignResult->fCompHeight;
        long nResult = PASS;

        float fMinHeight
            = (PassiveInfoDB->fPassiveHeight_mm * 1000.f) * m_VisionPara->m_fCompMissing_Height_Ratio / 100;

        if (fValue < fMinHeight)
        {
            nResult = REJECT;
            nMissingCount++;
        }

        if (nResult != PASS)
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nPassive, PassiveInfoDB->strCompName,
                PassiveInfoDB->strCompSpec, PassiveInfoDB->strCompType, CAST_FLOAT(nMissingCount), *spec,
                PassiveInfoDB->fSpecPos_um.m_x, PassiveInfoDB->fSpecPos_um.m_y, 0.f, false); //kircheis_MED2.5

            result->SetRect(nPassive, PassiveInfoDB->sfrtSpecROI_px.GetExtCRect());

            o_missingCheck[nPassive] = true;
            m_vecrtReject.push_back(PassiveInfoDB->sfrtSpecROI_px.GetExtCRect());
        }
        else
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nPassive, PassiveInfoDB->strCompName,
                PassiveInfoDB->strCompType, PassiveInfoDB->strCompType, 0.f, *spec, PassiveInfoDB->fSpecPos_um.m_x,
                PassiveInfoDB->fSpecPos_um.m_y, 0.f, false); //kirchies_MED2.5

            result->SetRect(nPassive, PassiveInfoDB->sfrtSpecROI_px.GetExtCRect());
        }

        nTotalResult = max(nTotalResult, nResult);
    }

    result->m_totalResult = nTotalResult;

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionComponent3D::Insp_CompHeight(const std::vector<bool>& /*i_missingCheck*/)
{
    auto* result = m_resultGroup.GetResultByName(g_szCompInspection3DName[COMP_INSPECTION_3D_COMP_HEIGHT]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    //kk Type별 검사 진행
    std::map<CString, VisionInspectionResult*> groupResults;
    std::map<CString, VisionInspectionSpec*> groupSpecs;

    long typeNum = (long)m_VisionPara->m_vecstrGroupInspName.size();

    BOOL use_Inspection = spec->m_use;

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        //if (strType.IsEmpty() == true) //CodeSonar 용 Test Code
        //    return FALSE;

        groupResults[strType] = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strCompHeightName);
        if (groupResults[strType] == nullptr)
        {
            return FALSE;
        }
        groupSpecs[strType] = GetSpecByName(groupResults[strType]->m_resultName);
        if (groupSpecs[strType] == nullptr)
        {
            return FALSE;
        }

        use_Inspection |= groupSpecs[strType]->m_use;
    }

    if (use_Inspection == FALSE)
        return TRUE;

    long nPassiveNum = (long)m_vecsPassiveInfoDB.size();
    std::map<CString, long> vecGroupCount;

    if (spec->m_use)
    {
        result->Clear();
        result->Resize(nPassiveNum);
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->Clear();
        groupResults[strType]->Resize(m_mapPassiveTypesCount[strType]);

        vecGroupCount[strType] = 0;
    }

    long nTotalResult = NOT_MEASURED;
    std::map<CString, long> groupTotalResult;
    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        SComponentAlignResult* pAlignResult = (SComponentAlignResult*)PassiveInfoDB->pResult;

        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Comp_Height(spec, result, PassiveInfoDB, pAlignResult, nPassive, &nTotalResult);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                Judg_Comp_Height(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &groupTotalResult[strType]);

                vecGroupCount[strType]++;
            }
        }
    }

    if (spec->m_use)
    {
        result->m_totalResult = nTotalResult;

        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->m_totalResult = groupTotalResult[strType];

        groupResults[strType]->UpdateTypicalValue(groupSpecs[strType]);
        groupResults[strType]->SetTotalResult();
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionComponent3D::Judg_Comp_Height(VisionInspectionSpec* spec, VisionInspectionResult* result,
    sPassive_InfoDB* chipInfo, SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult)
{
    if (chipAlign == NULL)
    {
        // Not Mearsure일 경우 Reject 처리..
        long nResult = REJECT;
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, 0.f, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y, 0.f,
            nResult); //kircheis_VSV //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        if (nResult != PASS)
            m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetCRect());

        return FALSE;
    }

    const float fSpecThickness = chipInfo->fPassiveHeight_mm * 1000.f;
    float fValue(0.f);
    if (chipAlign->fCompHeight != Ipvm::k_noiseValue32r)
    {
        fValue = (chipAlign->fCompHeight / fSpecThickness) * 100.f;
    }

    long nResult = result->IsValueCheck(spec, fValue);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fValue, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

    result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

    if (nResult != PASS)
        m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

    *totalResult = max(*totalResult, nResult);

    return TRUE;
}

BOOL VisionInspectionComponent3D::Insp_CompQuality(const std::vector<bool>& /*i_missingCheck*/)
{
    auto* result = m_resultGroup.GetResultByName(g_szCompInspection3DName[COMP_INSPECTION_3D_COMP_QUALITY]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    //kk Type별 검사 진행
    std::map<CString, VisionInspectionResult*> groupResults;
    std::map<CString, VisionInspectionSpec*> groupSpecs;

    long typeNum = (long)m_VisionPara->m_vecstrGroupInspName.size();

    BOOL use_Inspection = spec->m_use;

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        groupResults[strType] = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strCompQualName);
        if (groupResults[strType] == nullptr)
        {
            return FALSE;
        }
        groupSpecs[strType] = GetSpecByName(groupResults[strType]->m_resultName);
        if (groupSpecs[strType] == nullptr)
        {
            return FALSE;
        }

        use_Inspection |= groupSpecs[strType]->m_use;
    }

    if (use_Inspection == FALSE)
        return TRUE;

    long nPassiveNum = (long)m_vecsPassiveInfoDB.size();
    std::map<CString, long> vecGroupCount;

    if (spec->m_use)
    {
        result->Clear();
        result->Resize(nPassiveNum);
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->Clear();
        groupResults[strType]->Resize(m_mapPassiveTypesCount[strType]);

        vecGroupCount[strType] = 0;
    }

    long nTotalResult = NOT_MEASURED;
    std::map<CString, long> groupTotalResult;
    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        SComponentAlignResult* pAlignResult = (SComponentAlignResult*)PassiveInfoDB->pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Comp_Quality(spec, result, PassiveInfoDB, pAlignResult, nPassive, &nTotalResult);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                Judg_Comp_Quality(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &groupTotalResult[strType]);

                vecGroupCount[strType]++;
            }
        }
    }

    if (spec->m_use)
    {
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->UpdateTypicalValue(groupSpecs[strType]);
        groupResults[strType]->SetTotalResult();
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionComponent3D::Judg_Comp_Quality(VisionInspectionSpec* spec, VisionInspectionResult* result,
    sPassive_InfoDB* chipInfo, SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult)
{
    if (chipAlign == NULL)
    {
        // Not Mearsure일 경우 Reject 처리..
        long nResult = REJECT;
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, 0.f, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y, 0.f,
            nResult); //kircheis_VSV //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        if (nResult != PASS)
            m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

        return FALSE;
    }

    float fValue = chipAlign->fValidCompRatio;
    long nResult = result->IsValueCheck(spec, fValue);

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fValue, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y); //kirchies_MED2.5

    result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

    if (nResult != PASS)
        m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

    *totalResult = max(*totalResult, nResult);

    return TRUE;
}

BOOL VisionInspectionComponent3D::Insp_ElectrodeHeight(const std::vector<bool>& /*i_missingCheck*/)
{
    auto* result = m_resultGroup.GetResultByName(g_szCompInspection3DName[COMP_INSPECTION_3D_ELECTRODE_HEIGHT]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    //kk Type별 검사 진행
    std::map<CString, VisionInspectionResult*> groupResults;
    std::map<CString, VisionInspectionSpec*> groupSpecs;

    long typeNum = (long)m_VisionPara->m_vecstrGroupInspName.size();

    BOOL use_Inspection = spec->m_use;

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        groupResults[strType] = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strElecHeightName);
        if (groupResults[strType] == nullptr)
        {
            return FALSE;
        }
        groupSpecs[strType] = GetSpecByName(groupResults[strType]->m_resultName);
        if (groupSpecs[strType] == nullptr)
        {
            return FALSE;
        }

        use_Inspection |= groupSpecs[strType]->m_use;
    }

    if (use_Inspection == FALSE)
        return TRUE;

    long nPassiveNum = (long)m_vecsPassiveInfoDB.size();
    std::map<CString, long> vecGroupCount;

    if (spec->m_use)
    {
        result->Clear();
        result->Resize(nPassiveNum);
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->Clear();
        groupResults[strType]->Resize(m_mapPassiveTypesCount[strType]);

        vecGroupCount[strType] = 0;
    }

    float fMaxHeight = FLT_MIN;

    long nTotalResult = NOT_MEASURED;
    std::map<CString, long> groupTotalResult;
    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        SComponentAlignResult* pAlignResult = (SComponentAlignResult*)PassiveInfoDB->pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Electrode_Height(spec, result, PassiveInfoDB, pAlignResult, nPassive, &nTotalResult, &fMaxHeight);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                Judg_Electrode_Height(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &groupTotalResult[strType], &fMaxHeight);

                vecGroupCount[strType]++;
            }
        }
    }

    if (spec->m_use)
    {
        result->m_totalResult = nTotalResult;

        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->m_totalResult = groupTotalResult[strType];

        groupResults[strType]->UpdateTypicalValue(groupSpecs[strType]);
        groupResults[strType]->SetTotalResult();
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionComponent3D::Judg_Electrode_Height(VisionInspectionSpec* spec, VisionInspectionResult* result,
    sPassive_InfoDB* chipInfo, SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult, float* maxHeight)
{
    if (chipAlign == NULL)
    {
        // Not Mearsure일 경우 Reject 처리..
        long nResult = REJECT;
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, 0.f, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y, 0.f,
            nResult); //kircheis_VSV //kirchies_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        if (nResult != PASS)
            m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

        return FALSE;
    }

    const float fSpecElectThickness = chipInfo->fPassiveElectrodeHeight_mm * 1000.f;

    float fValue = Ipvm::k_noiseValue32r;
    float fElectrodeHeight = Ipvm::k_noiseValue32r;

    if (chipInfo->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_Capacitor
        || chipInfo->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_Register)
    {
        for (long nElect = 0; nElect < chipAlign->vecfElectrodeHeight.size(); nElect++)
        {
            float fElectHeight = chipAlign->vecfElectrodeHeight[nElect];
            if (fElectHeight > fElectrodeHeight)
            {
                fElectrodeHeight = fElectHeight;
            }
        }

        if (fElectrodeHeight != Ipvm::k_noiseValue32r)
            fValue = (fElectrodeHeight / fSpecElectThickness) * 100.f;

        if (fValue > *maxHeight)
            *maxHeight = fValue;
    }

    //long nResult = result->IsValueCheck(spec, fValue);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fValue, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

    long nResult = SkipMIATypeResult(ObjectNum, chipInfo->ePassiveType, spec, fValue, *result);

    result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

    if (nResult != PASS)
    {
        m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());
        result->vecrtRejectROI.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());
    }

    *totalResult = max(*totalResult, nResult);

    return TRUE;
}

BOOL VisionInspectionComponent3D::Insp_ElectrodeTilt(const std::vector<bool>& /*i_missingCheck*/)
{
    auto* result = m_resultGroup.GetResultByName(g_szCompInspection3DName[COMP_INSPECTION_3D_ELECTRODE_TILT]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    //kk Type별 검사 진행
    std::map<CString, VisionInspectionResult*> groupResults;
    std::map<CString, VisionInspectionSpec*> groupSpecs;

    long typeNum = (long)m_VisionPara->m_vecstrGroupInspName.size();

    BOOL use_Inspection = spec->m_use;

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        groupResults[strType] = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strElecTiltName);
        if (groupResults[strType] == nullptr)
        {
            return FALSE;
        }
        groupSpecs[strType] = GetSpecByName(groupResults[strType]->m_resultName);
        if (groupSpecs[strType] == nullptr)
        {
            return FALSE;
        }

        use_Inspection |= groupSpecs[strType]->m_use;
    }

    if (use_Inspection == FALSE)
        return TRUE;

    long nPassiveNum = (long)m_vecsPassiveInfoDB.size();
    std::map<CString, long> vecGroupCount;

    if (spec->m_use)
    {
        result->Clear();
        result->Resize(nPassiveNum);
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->Clear();
        groupResults[strType]->Resize(m_mapPassiveTypesCount[strType]);

        vecGroupCount[strType] = 0;
    }

    long nTotalResult = NOT_MEASURED;
    std::map<CString, long> groupTotalResult;
    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        SComponentAlignResult* pAlignResult = (SComponentAlignResult*)PassiveInfoDB->pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Electrode_Tilt(spec, result, PassiveInfoDB, pAlignResult, nPassive, &nTotalResult);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                Judg_Electrode_Tilt(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &groupTotalResult[strType]);

                vecGroupCount[strType]++;
            }
        }
    }

    if (spec->m_use)
    {
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->UpdateTypicalValue(groupSpecs[strType]);
        groupResults[strType]->SetTotalResult();
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionComponent3D::Judg_Electrode_Tilt(VisionInspectionSpec* spec, VisionInspectionResult* result,
    sPassive_InfoDB* chipInfo, SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult)
{
    if (chipAlign == NULL)
    {
        // Not Mearsure일 경우 Reject 처리..
        long nResult = REJECT;
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, 0.f, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y, 0.f,
            nResult); //kircheis_VSV //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        if (nResult != PASS)
            m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

        return FALSE;
    }

    float fValue = chipAlign->fElectTiltHeight;

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fValue, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

    long nResult = SkipMIATypeResult(ObjectNum, chipInfo->ePassiveType, spec, fValue, *result);

    result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

    if (nResult != PASS)
        m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

    *totalResult = max(*totalResult, nResult);

    return TRUE;
}

BOOL VisionInspectionComponent3D::Insp_BodyHeight(const std::vector<bool>& /*i_missingCheck*/)
{
    auto* result = m_resultGroup.GetResultByName(g_szCompInspection3DName[COMP_INSPECTION_3D_BODY_HEIGHT]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    //kk Type별 검사 진행
    std::map<CString, VisionInspectionResult*> groupResults;
    std::map<CString, VisionInspectionSpec*> groupSpecs;

    long typeNum = (long)m_VisionPara->m_vecstrGroupInspName.size();

    BOOL use_Inspection = spec->m_use;

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        groupResults[strType] = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strBodyHeightName);
        if (groupResults[strType] == nullptr)
        {
            return FALSE;
        }
        groupSpecs[strType] = GetSpecByName(groupResults[strType]->m_resultName);
        if (groupSpecs[strType] == nullptr)
        {
            return FALSE;
        }

        use_Inspection |= groupSpecs[strType]->m_use;
    }

    if (use_Inspection == FALSE)
        return TRUE;

    long nPassiveNum = (long)m_vecsPassiveInfoDB.size();
    std::map<CString, long> vecGroupCount;

    if (spec->m_use)
    {
        result->Clear();
        result->Resize(nPassiveNum);
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->Clear();
        groupResults[strType]->Resize(m_mapPassiveTypesCount[strType]);

        vecGroupCount[strType] = 0;
    }

    long nTotalResult = NOT_MEASURED;
    std::map<CString, long> groupTotalResult;
    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        SComponentAlignResult* pAlignResult = (SComponentAlignResult*)PassiveInfoDB->pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Body_Height(spec, result, PassiveInfoDB, pAlignResult, nPassive, &nTotalResult);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                Judg_Body_Height(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &groupTotalResult[strType]);

                vecGroupCount[strType]++;
            }
        }
    }

    if (spec->m_use)
    {
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->UpdateTypicalValue(groupSpecs[strType]);
        groupResults[strType]->SetTotalResult();
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionComponent3D::Judg_Body_Height(VisionInspectionSpec* spec, VisionInspectionResult* result,
    sPassive_InfoDB* chipInfo, SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult)
{
    if (chipAlign == NULL)
    {
        // Not Mearsure일 경우 Reject 처리..
        long nResult = REJECT;
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, 0.f, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y, 0.f,
            nResult); //kircheis_VSV //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        if (nResult != PASS)
            m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

        *totalResult = max(*totalResult, nResult);

        return FALSE;
    }

    float fSpecThickness = chipInfo->fPassiveHeight_mm * 1000.f;
    float fValue = Ipvm::k_noiseValue32r;
    if (chipAlign->fBodyHeight != Ipvm::k_noiseValue32r)
    {
        fValue = chipAlign->fBodyHeight / fSpecThickness * 100.f;
    }

    //long nResult = result->IsValueCheck(spec, fValue);

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fValue, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

    long nResult = SkipMIATypeResult(ObjectNum, chipInfo->ePassiveType, spec, fValue, *result);

    result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

    if (nResult != PASS)
        m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

    *totalResult = max(*totalResult, nResult);

    return TRUE;
}

BOOL VisionInspectionComponent3D::Insp_BodyTilt(const std::vector<bool>& /*i_missingCheck*/)
{
    auto* result = m_resultGroup.GetResultByName(g_szCompInspection3DName[COMP_INSPECTION_3D_BOTY_TILT]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    //kk Type별 검사 진행
    std::map<CString, VisionInspectionResult*> groupResults;
    std::map<CString, VisionInspectionSpec*> groupSpecs;

    long typeNum = (long)m_VisionPara->m_vecstrGroupInspName.size();

    BOOL use_Inspection = spec->m_use;

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        groupResults[strType] = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strBodyTiltName);
        if (groupResults[strType] == nullptr)
        {
            return FALSE;
        }
        groupSpecs[strType] = GetSpecByName(groupResults[strType]->m_resultName);
        if (groupSpecs[strType] == nullptr)
        {
            return FALSE;
        }

        use_Inspection |= groupSpecs[strType]->m_use;
    }

    if (use_Inspection == FALSE)
        return TRUE;

    long nPassiveNum = (long)m_vecsPassiveInfoDB.size();
    std::map<CString, long> vecGroupCount;

    if (spec->m_use)
    {
        result->Clear();
        result->Resize(nPassiveNum);
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->Clear();
        groupResults[strType]->Resize(m_mapPassiveTypesCount[strType]);

        vecGroupCount[strType] = 0;
    }

    long nTotalResult = NOT_MEASURED;
    std::map<CString, long> groupTotalResult;
    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        SComponentAlignResult* pAlignResult = (SComponentAlignResult*)PassiveInfoDB->pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Body_Tilt(spec, result, PassiveInfoDB, pAlignResult, nPassive, &nTotalResult);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                Judg_Body_Tilt(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &groupTotalResult[strType]);

                vecGroupCount[strType]++;
            }
        }
    }

    if (spec->m_use)
    {
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }

    for (long i = 0; i < typeNum; i++)
    {
        CString strType = m_VisionPara->m_vecstrGroupInspName[i].strCompSpec;

        if (!groupSpecs[strType]->m_use)
            continue;

        groupResults[strType]->UpdateTypicalValue(groupSpecs[strType]);
        groupResults[strType]->SetTotalResult();
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionComponent3D::Judg_Body_Tilt(VisionInspectionSpec* spec, VisionInspectionResult* result,
    sPassive_InfoDB* chipInfo, SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult)
{
    if (chipAlign == NULL)
    {
        // Not Mearsure일 경우 Reject 처리..
        long nResult = REJECT;
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, 0.f, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y, 0.f,
            nResult); //kircheis_VSV //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        if (nResult != PASS)
            m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

        return FALSE;
    }

    float fValue = chipAlign->fBodyTiltHeight;
    //long nResult = result->IsValueCheck(spec, fValue);

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fValue, *spec, chipInfo->fSpecPos_um.m_x, chipInfo->fSpecPos_um.m_y,
        0.f); //kircheis_MED2.5

    long nResult = SkipMIATypeResult(ObjectNum, chipInfo->ePassiveType, spec, fValue, *result);

    result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

    if (nResult != PASS)
        m_vecrtReject.push_back(chipInfo->sfrtSpecROI_px.GetExtCRect());

    *totalResult = max(*totalResult, nResult);

    return TRUE;
}

long VisionInspectionComponent3D::SkipMIATypeResult(long i_nPassiveidx, enumPassiveTypeDefine i_ePassiveType,
    VisionInspectionSpec* i_pSpec, float i_fValue, VisionInspectionResult& o_InsepctionResult)
{
    CString strElectrodeTilt_Guid
        = _T("{6609EE3A-1C06-48BE-BE82-F4799E994632}"); //해당항목에 대해서는 Nominal 값이 0이다
    CString strBodyTilt_Guid = _T("{4D351F4B-6673-4C79-AD64-64917B65E68A}"); //해당항목에 대해서는 Nominal 값이 0이다
    long nResult(NOT_MEASURED);
    if (i_ePassiveType == enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        if (o_InsepctionResult.m_specGuid == strElectrodeTilt_Guid || o_InsepctionResult.m_specGuid == strBodyTilt_Guid)
            o_InsepctionResult.SetValue(i_nPassiveidx, 0.f);
        else
            o_InsepctionResult.SetValue(i_nPassiveidx, 100.f);

        o_InsepctionResult.SetResult(i_nPassiveidx, PASS);
        nResult = PASS;
    }
    else
        nResult = o_InsepctionResult.IsValueCheck(i_pSpec, i_fValue);

    return nResult;
}

BOOL VisionInspectionComponent3D::MakeDebugROIs(sPassive_InfoDB* i_pPassiveInfoDB, std::vector<PI_RECT>* pvecsrtChip,
    std::vector<PI_RECT>* pvecsrtElectrode, std::vector<PI_RECT>* pvecsrtBody)
{
    if (i_pPassiveInfoDB == NULL || i_pPassiveInfoDB->pResult == NULL)
        return FALSE;

    SComponentAlignResult* pAlignResult = (SComponentAlignResult*)i_pPassiveInfoDB->pResult;

    if (i_pPassiveInfoDB->fPassiveAngle == 0.f || i_pPassiveInfoDB->fPassiveAngle == 90.f)
    {
        Ipvm::Rect32s rtResChip = pAlignResult->rtResChip;
        if (pvecsrtChip)
        {
            FPI_RECT sfrtChip = FPI_RECT(Ipvm::Point32r2((float)rtResChip.m_left, (float)rtResChip.m_top),
                Ipvm::Point32r2((float)rtResChip.m_right, (float)rtResChip.m_top),
                Ipvm::Point32r2((float)rtResChip.m_left, (float)rtResChip.m_bottom),
                Ipvm::Point32r2((float)rtResChip.m_right, (float)rtResChip.m_bottom));
            pvecsrtChip->push_back(sfrtChip.GetSPI_RECT());
        }

        if (pvecsrtElectrode)
        {
            std::vector<Ipvm::Rect32s> vecrtResElect = pAlignResult->vecrtResElect;
            for (long nElect = 0; nElect < (long)vecrtResElect.size(); nElect++)
            {
                FPI_RECT sfrtElect
                    = FPI_RECT(Ipvm::Point32r2((float)vecrtResElect[nElect].m_left, (float)vecrtResElect[nElect].m_top),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_right, (float)vecrtResElect[nElect].m_top),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_left, (float)vecrtResElect[nElect].m_bottom),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_right, (float)vecrtResElect[nElect].m_bottom));

                pvecsrtElectrode->push_back(sfrtElect.GetSPI_RECT());
            }
        }

        if (pvecsrtBody)
        {
            Ipvm::Rect32s rtResBody = pAlignResult->rtResBody;
            FPI_RECT sfrtBody = FPI_RECT(Ipvm::Point32r2((float)rtResBody.m_left, (float)rtResBody.m_top),
                Ipvm::Point32r2((float)rtResBody.m_right, (float)rtResBody.m_top),
                Ipvm::Point32r2((float)rtResBody.m_left, (float)rtResBody.m_bottom),
                Ipvm::Point32r2((float)rtResBody.m_right, (float)rtResBody.m_bottom));
            pvecsrtBody->push_back(sfrtBody.GetSPI_RECT());
        }
    }
    else
    {
        SComponentAlignSpec* pCompSpec = m_pCompAlign->GetAlignSpec(i_pPassiveInfoDB->strSpecName);

        FPI_RECT sfrtMoveChip = i_pPassiveInfoDB->sfrtSpecROI_px;

        float fShiftXPos
            = pCompSpec == NULL ? 0.f : CAST_FLOAT(pCompSpec->m_fSpecROI_ShiftXPos / 1000.f * m_fptPixelperMM.m_x);
        float fShiftYPos
            = pCompSpec == NULL ? 0.f : CAST_FLOAT(pCompSpec->m_fSpecROI_ShiftYPos / 1000.f * m_fptPixelperMM.m_y);
        sfrtMoveChip.Move(fShiftXPos, fShiftYPos);
        Ipvm::Point32r2 fptRotateCenter = sfrtMoveChip.GetFRect().CenterPoint();

        if (pvecsrtChip)
        {
            Ipvm::Rect32s rtResChip = pAlignResult->rtResChip;
            FPI_RECT sfrtChip = FPI_RECT(Ipvm::Point32r2((float)rtResChip.m_left, (float)rtResChip.m_top),
                Ipvm::Point32r2((float)rtResChip.m_right, (float)rtResChip.m_top),
                Ipvm::Point32r2((float)rtResChip.m_left, (float)rtResChip.m_bottom),
                Ipvm::Point32r2((float)rtResChip.m_right, (float)rtResChip.m_bottom));
            FPI_RECT sfrtChipRotate
                = sfrtChip.Rotate(i_pPassiveInfoDB->fPassiveAngle * DEF_DEG_TO_RAD, fptRotateCenter);
            pvecsrtChip->push_back(sfrtChipRotate.GetSPI_RECT());
        }

        if (pvecsrtElectrode)
        {
            std::vector<Ipvm::Rect32s> vecrtResElect = pAlignResult->vecrtResElect;
            long nElectNum = (long)vecrtResElect.size();
            std::vector<FPI_RECT> vecfsrtResElect(nElectNum);
            for (long nElect = 0; nElect < nElectNum; nElect++)
            {
                FPI_RECT sfrtElect
                    = FPI_RECT(Ipvm::Point32r2((float)vecrtResElect[nElect].m_left, (float)vecrtResElect[nElect].m_top),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_right, (float)vecrtResElect[nElect].m_top),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_left, (float)vecrtResElect[nElect].m_bottom),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_right, (float)vecrtResElect[nElect].m_bottom));
                vecfsrtResElect[nElect] = sfrtElect;
            }
            for (long nElect = 0; nElect < nElectNum; nElect++)
            {
                FPI_RECT sfrtElectRotate
                    = vecfsrtResElect[nElect].Rotate(i_pPassiveInfoDB->fPassiveAngle * DEF_DEG_TO_RAD, fptRotateCenter);
                pvecsrtElectrode->push_back(sfrtElectRotate.GetSPI_RECT());
            }
        }

        if (pvecsrtBody)
        {
            Ipvm::Rect32s rtResBody = pAlignResult->rtResBody;
            FPI_RECT sfrtBody = FPI_RECT(Ipvm::Point32r2((float)rtResBody.m_left, (float)rtResBody.m_top),
                Ipvm::Point32r2((float)rtResBody.m_right, (float)rtResBody.m_top),
                Ipvm::Point32r2((float)rtResBody.m_left, (float)rtResBody.m_bottom),
                Ipvm::Point32r2((float)rtResBody.m_right, (float)rtResBody.m_bottom));
            FPI_RECT sfrtBodyRotate
                = sfrtBody.Rotate(i_pPassiveInfoDB->fPassiveAngle * DEF_DEG_TO_RAD, fptRotateCenter);
            pvecsrtBody->push_back(sfrtBodyRotate.GetSPI_RECT());
        }
    }

    return TRUE;
}

BOOL VisionInspectionComponent3D::MakeDebugCalcROIs(sPassive_InfoDB* i_pPassiveInfoDB,
    std::vector<PI_RECT>* pvecsrtComponent, std::vector<PI_RECT>* pvecsrtValidElectrode,
    std::vector<PI_RECT>* pvecsrtValidBody)
{
    if (i_pPassiveInfoDB == NULL)
        return FALSE;

    SComponentAlignResult* pAlignResult = (SComponentAlignResult*)i_pPassiveInfoDB->pResult;
    if (pAlignResult == NULL)
        return FALSE;

    if (i_pPassiveInfoDB->fPassiveAngle == 0.f || i_pPassiveInfoDB->fPassiveAngle == 90.f)
    {
        Ipvm::Rect32s rtValidComponent = pAlignResult->rtValidComponent;
        if (pvecsrtComponent)
        {
            FPI_RECT sfrtComp = FPI_RECT(Ipvm::Point32r2((float)rtValidComponent.m_left, (float)rtValidComponent.m_top),
                Ipvm::Point32r2((float)rtValidComponent.m_right, (float)rtValidComponent.m_top),
                Ipvm::Point32r2((float)rtValidComponent.m_left, (float)rtValidComponent.m_bottom),
                Ipvm::Point32r2((float)rtValidComponent.m_right, (float)rtValidComponent.m_bottom));
            pvecsrtComponent->push_back(sfrtComp.GetSPI_RECT());
        }

        if (pvecsrtValidElectrode)
        {
            std::vector<Ipvm::Rect32s> vecrtResElect = pAlignResult->vecrtValidElect;
            for (long nElect = 0; nElect < (long)vecrtResElect.size(); nElect++)
            {
                FPI_RECT sfrtElect
                    = FPI_RECT(Ipvm::Point32r2((float)vecrtResElect[nElect].m_left, (float)vecrtResElect[nElect].m_top),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_right, (float)vecrtResElect[nElect].m_top),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_left, (float)vecrtResElect[nElect].m_bottom),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_right, (float)vecrtResElect[nElect].m_bottom));

                pvecsrtValidElectrode->push_back(sfrtElect.GetSPI_RECT());
            }
        }

        if (pvecsrtValidBody)
        {
            Ipvm::Rect32s rtValidBody = pAlignResult->rtValidBody;
            FPI_RECT sfrtBody = FPI_RECT(Ipvm::Point32r2((float)rtValidBody.m_left, (float)rtValidBody.m_top),
                Ipvm::Point32r2((float)rtValidBody.m_right, (float)rtValidBody.m_top),
                Ipvm::Point32r2((float)rtValidBody.m_left, (float)rtValidBody.m_bottom),
                Ipvm::Point32r2((float)rtValidBody.m_right, (float)rtValidBody.m_bottom));
            pvecsrtValidBody->push_back(sfrtBody.GetSPI_RECT());
        }
    }
    else
    {
        SComponentAlignSpec* pCompSpec = m_pCompAlign->GetAlignSpec(i_pPassiveInfoDB->strSpecName);

        FPI_RECT sfrtMoveChip = i_pPassiveInfoDB->sfrtSpecROI_px;

        float fShiftXPos
            = pCompSpec == NULL ? 0.f : CAST_FLOAT(pCompSpec->m_fSpecROI_ShiftXPos / 1000.f * m_fptPixelperMM.m_x);
        float fShiftYPos
            = pCompSpec == NULL ? 0.f : CAST_FLOAT(pCompSpec->m_fSpecROI_ShiftYPos / 1000.f * m_fptPixelperMM.m_y);
        sfrtMoveChip.Move(fShiftXPos, fShiftYPos);
        Ipvm::Point32r2 fptRotateCenter = sfrtMoveChip.GetFRect().CenterPoint();

        if (pvecsrtComponent)
        {
            Ipvm::Rect32s rtValidComponent = pAlignResult->rtValidComponent;
            FPI_RECT sfrtComp = FPI_RECT(Ipvm::Point32r2((float)rtValidComponent.m_left, (float)rtValidComponent.m_top),
                Ipvm::Point32r2((float)rtValidComponent.m_right, (float)rtValidComponent.m_top),
                Ipvm::Point32r2((float)rtValidComponent.m_left, (float)rtValidComponent.m_bottom),
                Ipvm::Point32r2((float)rtValidComponent.m_right, (float)rtValidComponent.m_bottom));
            FPI_RECT sfrtCompRotate
                = sfrtComp.Rotate(i_pPassiveInfoDB->fPassiveAngle * DEF_DEG_TO_RAD, fptRotateCenter);
            pvecsrtComponent->push_back(sfrtCompRotate.GetSPI_RECT());
        }

        if (pvecsrtValidElectrode)
        {
            std::vector<Ipvm::Rect32s> vecrtResElect = pAlignResult->vecrtValidElect;
            long nElectNum = (long)vecrtResElect.size();
            std::vector<FPI_RECT> vecfsrtResElect(nElectNum);
            for (long nElect = 0; nElect < nElectNum; nElect++)
            {
                FPI_RECT sfrtElect
                    = FPI_RECT(Ipvm::Point32r2((float)vecrtResElect[nElect].m_left, (float)vecrtResElect[nElect].m_top),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_right, (float)vecrtResElect[nElect].m_top),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_left, (float)vecrtResElect[nElect].m_bottom),
                        Ipvm::Point32r2((float)vecrtResElect[nElect].m_right, (float)vecrtResElect[nElect].m_bottom));
                vecfsrtResElect[nElect] = sfrtElect;
            }
            for (long nElect = 0; nElect < nElectNum; nElect++)
            {
                FPI_RECT sfrtElectRotate
                    = vecfsrtResElect[nElect].Rotate(i_pPassiveInfoDB->fPassiveAngle * DEF_DEG_TO_RAD, fptRotateCenter);
                pvecsrtValidElectrode->push_back(sfrtElectRotate.GetSPI_RECT());
            }
        }

        if (pvecsrtValidBody)
        {
            Ipvm::Rect32s rtValidBody = pAlignResult->rtValidBody;
            FPI_RECT sfrtBody = FPI_RECT(Ipvm::Point32r2((float)rtValidBody.m_left, (float)rtValidBody.m_top),
                Ipvm::Point32r2((float)rtValidBody.m_right, (float)rtValidBody.m_top),
                Ipvm::Point32r2((float)rtValidBody.m_left, (float)rtValidBody.m_bottom),
                Ipvm::Point32r2((float)rtValidBody.m_right, (float)rtValidBody.m_bottom));
            FPI_RECT sfrtBodyRotate
                = sfrtBody.Rotate(i_pPassiveInfoDB->fPassiveAngle * DEF_DEG_TO_RAD, fptRotateCenter);
            pvecsrtValidBody->push_back(sfrtBodyRotate.GetSPI_RECT());
        }
    }

    return TRUE;
}

float VisionInspectionComponent3D::GetZValue(float fHmapData, float fConvertScale)
{
    return fHmapData / fConvertScale;
}

Ipvm::Point32r2 VisionInspectionComponent3D::ConvertSpecPosMMtoUM(float i_fSpecPosX_mm, float i_fSpecPosY_mm)
{
    Ipvm::Point32r2 fSpecPosXY_um(0.f, 0.f);
    float fMMtoum = 1000.f;

    fSpecPosXY_um.m_x = i_fSpecPosX_mm * fMMtoum;
    fSpecPosXY_um.m_y = i_fSpecPosY_mm * fMMtoum;

    return fSpecPosXY_um;
}
