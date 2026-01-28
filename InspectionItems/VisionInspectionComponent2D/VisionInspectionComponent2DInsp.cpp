//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionComponent2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionComponent2D.h"
#include "PassiveAlign.h"
#include "Result.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u.h>
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
template<typename _T>
void insertEnd(std::vector<_T>& s1, const std::vector<_T>& s2)
{
    s1.insert(s1.end(), s2.begin(), s2.end());
}

BOOL VisionInspectionComponent2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

void VisionInspectionComponent2D::ClearInspResult()
{
    for (long n = 0; n < (long)m_fixedInspectionSpecs.size(); n++)
    {
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[n].m_specName);
        if (pResult)
        {
            pResult->m_totalResult = NOT_MEASURED;
            pResult->Clear();
        }
    }
}

BOOL VisionInspectionComponent2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //
    VerifyFrame();

    if (m_bFrameInitalize) //TRUE라면 다 Invalid 다시 설정해
    {
        for (auto InspectionItem : m_fixedInspectionSpecs)
        {
            if (InspectionItem.m_use)
            {
                VisionInspectionResult* pResult = m_resultGroup.GetResultByName(InspectionItem.m_specName);
                if (pResult == nullptr)
                    return FALSE;

                pResult->Clear();
                pResult->SetResult(0, INVALID);
                pResult->m_totalResult = INVALID;
                pResult->SetTotalResult();
            }
        }

        return FALSE;
    }

    m_bIsComponentInsp = TRUE;
    m_bUse2ndInspection = FALSE;

    m_fCalcTime = 0.f;
    Ipvm::TimeCheck TimeCheck;

    m_nTotalResult = NOT_MEASURED;
    ClearInspResult();

    BOOL bPass = FALSE;

    ResetResult();
    m_vecrtReject.clear();

    // 08.20.20 KSY
    m_vecbUseComp2ndInsp.clear();
    m_vecstrComp2ndMatchCode.clear();

    for (long i = 0; i < m_vecsPassiveInfoDB.size(); i++)
    {
        PassiveAlignSpec::CapAlignSpec* pCapSpec = m_pPassiveAlign->GetCapAlignSpec(m_vecsPassiveInfoDB[i].strSpecName);
        if (pCapSpec == nullptr)
        {
            m_fCalcTime = CAST_FLOAT(TimeCheck.Elapsed_ms());
            return FALSE;
        }

        if (pCapSpec->m_bUse2ndInspection)
            m_bUse2ndInspection = TRUE;

        m_vecbUseComp2ndInsp.push_back(pCapSpec->m_bUse2ndInspection);
        m_vecstrComp2ndMatchCode.push_back(pCapSpec->m_str2ndInspCode);
    }

    if (DoAlign(detailSetupMode))
    {
        bPass = DoInsp(detailSetupMode);
    }

    m_fCalcTime = CAST_FLOAT(TimeCheck.Elapsed_ms());

    return TRUE;
}

BOOL VisionInspectionComponent2D::GetBodyAlignInfo(
    float& o_fAngle_rad, Ipvm::Rect32r& o_frtBody, Ipvm::Point32r2& o_fptCenter)
{
    VisionAlignResult* pEdgeAlignResult = NULL;
    long nDataNum = 0;
    void* pEdgeAlignData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pEdgeAlignData != nullptr && nDataNum > 0)
        pEdgeAlignResult = (VisionAlignResult*)pEdgeAlignData;

    //PadAlign을 이용한 Center 좌표
    //Comp는 기본적으로 PadAlign의 좌표를 따라간다.
    Ipvm::Point32r2* pfptGetCenterPosbyPadAlign = nullptr;
    long nPadDataNum(0);
    void* pPadAlignData = m_visionUnit.GetVisionDebugInfo(
        _VISION_INSP_GUID_FIDUCIAL_ALIGN, _T("Apply PAD Align to Body Center"), nPadDataNum);
    if (pPadAlignData != NULL && nPadDataNum > 0)
        pfptGetCenterPosbyPadAlign = (Ipvm::Point32r2*)pPadAlignData;

    if (pEdgeAlignResult == NULL) // && pPadAlignData == NULL)
        return FALSE;

    o_fAngle_rad = pEdgeAlignResult->m_angle_rad;
    o_frtBody = pEdgeAlignResult->getBodyRect();

    if (pPadAlignData != NULL && nPadDataNum > 0) //PadAlign Data가 존재한다면
        o_fptCenter = *(pfptGetCenterPosbyPadAlign);
    else
        o_fptCenter = pEdgeAlignResult->m_center;

    FPI_RECT sfrtBody
        = FPI_RECT(pEdgeAlignResult->fptLT, pEdgeAlignResult->fptRT, pEdgeAlignResult->fptLB, pEdgeAlignResult->fptRB);

    m_sfrtPackageBody = sfrtBody.Rotate(o_fAngle_rad);

    return TRUE;
}

void VisionInspectionComponent2D::MakePackageInfo(
    const bool detailSetupMode, float i_fBodyAngle_rad, Ipvm::Point32r2 i_fptBodyCenter)
{
    m_result->clear();
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

    long nCurPassiveindex(0);
    for (auto PassiveInfoDB : m_vecsPassiveInfoDB)
    {
        ResultPassiveItem item;
        item.m_sPassive_Index = nCurPassiveindex;
        item.m_specROI = PassiveInfoDB.sfrtSpecROI_px.GetSPI_RECT();
        m_result->m_passiveItems.push_back(item);
        nCurPassiveindex++;
    }

    SetDebugInfoItem(detailSetupMode, _T("Spec Passive ROI"), vecsrtPassive);
}

FPI_RECT VisionInspectionComponent2D::GetSpecROI_px(
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

BOOL VisionInspectionComponent2D::DoAlign(const bool detailSetupMode)
{
    if (m_pVisionInspDlg != NULL && detailSetupMode)
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();

    if (!MakePreInspInfo(detailSetupMode))
        return FALSE;

    std::vector<PI_RECT> vecsrtPackageBody(1);
    vecsrtPackageBody[0] = m_sfrtPackageBody.GetSPI_RECT();
    SetDebugInfoItem(detailSetupMode, _T("PACKAGE BODY ROI"), vecsrtPackageBody);

    m_PassiveAlignResult.clear();
    m_PassiveAlignResult.resize(m_vecsPassiveInfoDB.size());

    for (long nidx = 0; nidx < m_vecsPassiveInfoDB.size(); nidx++)
    {
        if (!DoAlign_Passive(&m_vecsPassiveInfoDB[nidx], m_result->m_passiveItems[nidx], m_PassiveAlignResult[nidx]))
            return FALSE;
    }

    SetDebugInfo(detailSetupMode);

    if (!SetMaskImage(detailSetupMode))
        return FALSE;

    return TRUE;
}

void VisionInspectionComponent2D::SetDebugInfo(const bool detailSetupMode)
{
    std::vector<FPI_RECT> Debug_RoughAlignResult;
    std::vector<FPI_RECT> Debug_vecDetailAlign_LeftSearchROI;
    std::vector<FPI_RECT> Debug_vecDetailAlign_TopSearchROI;
    std::vector<FPI_RECT> Debug_vecDetailAlign_RightSearchROI;
    std::vector<FPI_RECT> Debug_vecDetailAlign_BottomSearchROI;
    std::vector<FPI_RECT> Debug_DetailAlign_Passive_Result;
    std::vector<FPI_RECT> Debug_DetailAlign_Passive_Result_Horizontal;
    std::vector<FPI_RECT> Debug_DetailAlign_Passive_Result_Vertical;

    EDGEPOINTS Debug_vecDetailAlign_LeftSearchEdgePoints;
    EDGEPOINTS Debug_vecDetailAlign_TopSearchEdgePoints;
    EDGEPOINTS Debug_vecDetailAlign_RightSearchEdgePoints;
    EDGEPOINTS Debug_vecDetailAlign_BottomSearchEdgePoints;

    std::vector<Ipvm::Rect32s> Debug_DetailAlign_Passive_Shorten_SearchROI;

    for (auto& item : m_result->m_passiveItems)
    {
        Debug_RoughAlignResult.push_back(item.m_roughAlignResult);
        Debug_vecDetailAlign_LeftSearchROI.push_back(item.m_detailSearchROI[LEFT]);
        Debug_vecDetailAlign_TopSearchROI.push_back(item.m_detailSearchROI[UP]);
        Debug_vecDetailAlign_RightSearchROI.push_back(item.m_detailSearchROI[RIGHT]);
        Debug_vecDetailAlign_BottomSearchROI.push_back(item.m_detailSearchROI[DOWN]);
        Debug_DetailAlign_Passive_Result.push_back(item.m_detailAlignResult);
        Debug_DetailAlign_Passive_Result_Horizontal.push_back(item.m_detailAlignResult_Horizontal);
        Debug_DetailAlign_Passive_Result_Vertical.push_back(item.m_detailAlignResult_Vertical);

        insertEnd(Debug_vecDetailAlign_LeftSearchEdgePoints, item.m_debugEdgePoints[LEFT]);
        insertEnd(Debug_vecDetailAlign_TopSearchEdgePoints, item.m_debugEdgePoints[UP]);
        insertEnd(Debug_vecDetailAlign_RightSearchEdgePoints, item.m_debugEdgePoints[RIGHT]);
        insertEnd(Debug_vecDetailAlign_BottomSearchEdgePoints, item.m_debugEdgePoints[DOWN]);

        insertEnd(Debug_DetailAlign_Passive_Shorten_SearchROI, item.m_shortenSearchROIs);
    }

    SetDebugInfoItem(detailSetupMode, _T("RoughAlign_Result"), Debug_RoughAlignResult, TRUE);

    SetDebugInfoItem(
        detailSetupMode, _T("DetailAlign_Passive_Left_SearchROI"), Debug_vecDetailAlign_LeftSearchROI, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("DetailAlign_Passive_Top_SearchROI"), Debug_vecDetailAlign_TopSearchROI, TRUE);
    SetDebugInfoItem(
        detailSetupMode, _T("DetailAlign_Passive_Right_SearchROI"), Debug_vecDetailAlign_RightSearchROI, TRUE);
    SetDebugInfoItem(
        detailSetupMode, _T("DetailAlign_Passive_Bottom_SearchROI"), Debug_vecDetailAlign_BottomSearchROI, TRUE);

    SetDebugInfoItem(
        detailSetupMode, _T("DetailAlign_Passive_Left_EdgePoints"), Debug_vecDetailAlign_LeftSearchEdgePoints, TRUE);
    SetDebugInfoItem(
        detailSetupMode, _T("DetailAlign_Passive_Top_EdgePoints"), Debug_vecDetailAlign_TopSearchEdgePoints, TRUE);
    SetDebugInfoItem(
        detailSetupMode, _T("DetailAlign_Passive_Right_EdgePoints"), Debug_vecDetailAlign_RightSearchEdgePoints, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("DetailAlign_Passive_Bottom_EdgePoints"),
        Debug_vecDetailAlign_BottomSearchEdgePoints, TRUE);

    SetDebugInfoItem(detailSetupMode, _T("DetailAlign_Passive_Result"), Debug_DetailAlign_Passive_Result, TRUE);

    SetDebugInfoItem(detailSetupMode, _T("DetailAlign_Passive_Result_Horizontal"),
        Debug_DetailAlign_Passive_Result_Horizontal, TRUE);
    SetDebugInfoItem(
        detailSetupMode, _T("DetailAlign_Passive_Result_Vertical"), Debug_DetailAlign_Passive_Result_Vertical, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("DetailAlign_Passive_Shorten_SearchROI"),
        Debug_DetailAlign_Passive_Shorten_SearchROI, TRUE);
}

BOOL VisionInspectionComponent2D::SetMaskImage(const bool detailSetupMode)
{
    //{{mc_예외처리 Global Mask 생성시 Mask Image를 못찾는경우가 발생한다 //빈이미지라도 넘겨줘야한다
    Ipvm::Image8u imgCompMask;
    Ipvm::Image8u imgMeasured_PassiveMask;
    if (!getReusableMemory().GetInspByteImage(imgCompMask))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(imgMeasured_PassiveMask))
        return FALSE;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imgCompMask), 0, imgCompMask);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imgCompMask), 0, imgMeasured_PassiveMask);
    //}}

    for (long index = 0; index < (long)m_result->m_passiveItems.size(); index++)
    {
        auto& item = m_result->m_passiveItems[index];
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[item.m_sPassive_Index];

        Ipvm::Rect32s rtMaskROI(0, 0, 0, 0);
        if (item.m_bAlignSuccess == true)
            rtMaskROI = item.m_detailAlignResult.GetExtCRect();
        else if (item.m_bAlignSuccess == false)
            rtMaskROI = PassiveInfoDB->sfrtSpecROI_px.GetExtCRect();

        Ipvm::ImageProcessing::Fill(rtMaskROI, 255, imgCompMask);
        Ipvm::ImageProcessing::Fill(rtMaskROI, 255, imgMeasured_PassiveMask);

        if (PassiveInfoDB->ePassiveType
            != enumPassiveTypeDefine::enum_PassiveType_MIA) //mc_MIA는 Pad가 존재하지 않는듯하다
        {
            if (PassiveInfoDB->fPassiveAngle == PassiveAngle_Horizontal
                || PassiveInfoDB->fPassiveAngle == PassiveAngle_Horizontal_reverse)
            {
                CreateMaskInPadArea(PassiveInfoDB, PassiveInfoDB->fPassiveAngle, item.m_specROI, LEFT, imgCompMask);
                CreateMaskInPadArea(PassiveInfoDB, PassiveInfoDB->fPassiveAngle, item.m_specROI, RIGHT, imgCompMask);
            }
            else if (PassiveInfoDB->fPassiveAngle == PassiveAngle_Vertical
                || PassiveInfoDB->fPassiveAngle == PassiveAngle_Vertical_reverse)
            {
                CreateMaskInPadArea(PassiveInfoDB, PassiveInfoDB->fPassiveAngle, item.m_specROI, UP, imgCompMask);
                CreateMaskInPadArea(PassiveInfoDB, PassiveInfoDB->fPassiveAngle, item.m_specROI, DOWN, imgCompMask);
            }
        }
    }

    SetDebugInfoItem(detailSetupMode, _T("Passive Mask Image with Pad"), imgCompMask, TRUE);

    auto* OriginMask = getReusableMemory().AddSurfaceLayerMaskClass(_T("Measured - Component with Pad"));
    if (OriginMask == nullptr)
        return FALSE;

    OriginMask->Reset();
    OriginMask->Add(imgCompMask);

    auto* layerMask = getReusableMemory().AddSurfaceLayerMaskClass(
        _T("Measured - Component")); //Pad그려주는 부분제외하고 그려줘야한다
    if (layerMask == nullptr)
        return FALSE;

    layerMask->Reset();
    layerMask->Add(imgMeasured_PassiveMask);

    SetPassiveTypeMask();

    return TRUE;
}

void VisionInspectionComponent2D::SetPassiveTypeMask()
{
    std::vector<CString> vecstrPassiveTypeName = GetPassiveTypeName(m_vecsPassiveInfoDB);
    if (vecstrPassiveTypeName.size() > 0)
    {
        for (auto PassiveTypeName : vecstrPassiveTypeName)
        {
            Ipvm::Image8u PassiveTypeMask
                = GetPassiveTypeMaskImage(PassiveTypeName, m_result->m_passiveItems, m_vecsPassiveInfoDB);

            if (PassiveTypeMask.GetMem() == nullptr)
                continue;

            CString strMaskName = PassiveTypeName + _T(" Mask Image");

            auto* PassivelayerMask = getReusableMemory().AddSurfaceLayerMaskClass(strMaskName);
            if (PassivelayerMask == nullptr)
                continue;

            PassivelayerMask->Reset();

            PassivelayerMask->Add(PassiveTypeMask);

            strMaskName.Empty();
        }
    }

    for (int nIdx = 0; nIdx < vecstrPassiveTypeName.size(); nIdx++)
    {
        vecstrPassiveTypeName[nIdx].Empty();
    }
}

std::vector<CString> VisionInspectionComponent2D::GetPassiveTypeName(std::vector<sPassive_InfoDB> i_sPassiveInfoDB)
{
    std::vector<CString> PassiveTypeName;
    if (i_sPassiveInfoDB.size() < 0)
    {
        PassiveTypeName.clear();
        return PassiveTypeName;
    }

    for (auto PassiveCompTypeName : i_sPassiveInfoDB)
    {
        BOOL bCheckValue = std::find(PassiveTypeName.begin(), PassiveTypeName.end(), PassiveCompTypeName.strCompType)
            != PassiveTypeName.end();

        if (!bCheckValue)
            PassiveTypeName.push_back(PassiveCompTypeName.strCompType);
    }

    return PassiveTypeName;
}

Ipvm::Image8u VisionInspectionComponent2D::GetPassiveTypeMaskImage(CString i_strPassiveTypeName,
    std::vector<ResultPassiveItem> i_vecResultPassiveItem, std::vector<sPassive_InfoDB> i_sPassiveInfoDB)
{
    Ipvm::Image8u imgPassiveTypeMask;
    if (!getReusableMemory().GetInspByteImage(imgPassiveTypeMask))
        return imgPassiveTypeMask;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imgPassiveTypeMask), 0, imgPassiveTypeMask);

    for (long nidx = 0; nidx < (long)i_vecResultPassiveItem.size(); nidx++)
    {
        auto& item = i_vecResultPassiveItem[nidx];
        auto& SpecItem = i_sPassiveInfoDB[nidx];

        if (SpecItem.strCompType == i_strPassiveTypeName)
        {
            Ipvm::Rect32s rtMaskROI(0, 0, 0, 0);
            if (item.m_bAlignSuccess == true)
                rtMaskROI = item.m_detailAlignResult.GetExtCRect();
            else if (item.m_bAlignSuccess == false)
                rtMaskROI = SpecItem.sfrtSpecROI_px.GetExtCRect();

            Ipvm::ImageProcessing::Fill(rtMaskROI, 255, imgPassiveTypeMask);
        }
    }

    return imgPassiveTypeMask;
}

//BOOL VisionInspectionComponent2D::DoAlign_Passive(sPassive_InfoDB *i_pPassiveInfoDB, ResultPassiveItem& o_result)
//{
//	if (i_pPassiveInfoDB == NULL)
//		return FALSE;
//
//	Ipvm::Point32r2 rotateCenter = i_pPassiveInfoDB->sfrtSpecROI_px.GetFRect().CenterPoint();
//
//	if (m_pPassiveAlign->DoAlign(getScale(), i_pPassiveInfoDB, &o_result))
//	{
//		PassiveAlignResult* pPassiveAlignResut = new PassiveAlignResult;
//		*pPassiveAlignResut = m_pPassiveAlign->m_result;
//
//		i_pPassiveInfoDB->pResult = pPassiveAlignResut;
//	}
//
//	return TRUE;
//}

BOOL VisionInspectionComponent2D::DoAlign_Passive(
    sPassive_InfoDB* i_pPassiveInfoDB, ResultPassiveItem& o_result, PassiveAlignResult& o_PassiveAlignResult)
{
    if (i_pPassiveInfoDB == NULL)
        return FALSE;

    Ipvm::Point32r2 rotateCenter = i_pPassiveInfoDB->sfrtSpecROI_px.GetFRect().CenterPoint();
    if (m_pPassiveAlign->DoAlign(getScale(), i_pPassiveInfoDB, &o_result))
    {
        PassiveAlignResult* pPassiveAlignResut = &o_PassiveAlignResult;
        *pPassiveAlignResut = m_pPassiveAlign->m_result;

        i_pPassiveInfoDB->pResult = pPassiveAlignResut;
    }

    return TRUE;
}

BOOL VisionInspectionComponent2D::CreateMaskInPadArea(sPassive_InfoDB* i_pPassiveInfoDB, float i_fAngleType,
    PI_RECT i_rtSpecROI, long i_nDircetion, Ipvm::Image8u& o_MaskImage)
{
    long nOffSetWidth(0), nOffSetLength(0);
    float fOffSetWidth(0), fOffSetLength(0);
    const float fUMtoPxl = getScale().umToPixelXY();
    const float fMMtoPxl = 1000.f * fUMtoPxl;
    bool bUsePadInfo = true;
    fOffSetWidth = i_pPassiveInfoDB->fPassivePAD_Width_mm * fMMtoPxl;
    fOffSetLength = i_pPassiveInfoDB->fPassivePAD_Length_mm * fMMtoPxl;
    nOffSetWidth = long(fOffSetWidth * .5f + .5f); //kircheis_MED_Bug_20200305
    nOffSetLength = long(fOffSetLength * .5f + .5f);
    if (i_fAngleType == PassiveAngle_Horizontal || i_fAngleType == PassiveAngle_Horizontal_reverse)
    {
        if ((nOffSetWidth * nOffSetLength) <= 0
            || (i_pPassiveInfoDB->fPassivePAD_Width_mm == 0.15f && i_pPassiveInfoDB->fPassivePAD_Length_mm == 0.15f))
        {
            nOffSetWidth = long(i_rtSpecROI.GetExtRect().Height() * .1f); //10%
            nOffSetLength = long(i_rtSpecROI.GetExtRect().Width() * .25f); //25%
            bUsePadInfo = false;
        }
    }
    else if (i_fAngleType == PassiveAngle_Vertical || i_fAngleType == PassiveAngle_Vertical_reverse)
    {
        if ((nOffSetWidth * nOffSetLength) <= 0
            || (i_pPassiveInfoDB->fPassivePAD_Width_mm == 0.15f && i_pPassiveInfoDB->fPassivePAD_Length_mm == 0.15f))
        {
            nOffSetWidth = long(i_rtSpecROI.GetExtRect().Width() * .1f); //10%
            nOffSetLength = long(i_rtSpecROI.GetExtRect().Height() * .25f); //25%
            bUsePadInfo = false;
        }
    }

    //좌표들 계산해주고
    Ipvm::Point32s2 nptLT(0, 0), nptRT(0, 0), nptLB(0, 0), nptRB(0, 0);

    if (bUsePadInfo)
    {
        long nPadGap = (long)(i_pPassiveInfoDB->fPassivePAD_Gap_mm * fMMtoPxl + .5f);
        Ipvm::Point32s2 ptCenter = i_rtSpecROI.GetExtRect().CenterPoint();
        switch (i_nDircetion)
        {
            case LEFT:
            {
                nptLT = Ipvm::Point32s2(ptCenter.m_x - nPadGap - nOffSetLength, ptCenter.m_y - nOffSetWidth);
                nptRT = Ipvm::Point32s2(ptCenter.m_x - nPadGap + nOffSetLength, ptCenter.m_y - nOffSetWidth);
                nptLB = Ipvm::Point32s2(ptCenter.m_x - nPadGap - nOffSetLength, ptCenter.m_y + nOffSetWidth);
                nptRB = Ipvm::Point32s2(ptCenter.m_x - nPadGap + nOffSetLength, ptCenter.m_y + nOffSetWidth);
                break;
            }
            case UP:
            {
                nptLT = Ipvm::Point32s2(ptCenter.m_x - nOffSetWidth, ptCenter.m_y - nPadGap - nOffSetLength);
                nptRT = Ipvm::Point32s2(ptCenter.m_x + nOffSetWidth, ptCenter.m_y - nPadGap - nOffSetLength);
                nptLB = Ipvm::Point32s2(ptCenter.m_x - nOffSetWidth, ptCenter.m_y - nPadGap + nOffSetLength);
                nptRB = Ipvm::Point32s2(ptCenter.m_x + nOffSetWidth, ptCenter.m_y - nPadGap + nOffSetLength);
                break;
            }
            case RIGHT:
            {
                nptLT = Ipvm::Point32s2(ptCenter.m_x + nPadGap - nOffSetLength, ptCenter.m_y - nOffSetWidth);
                nptRT = Ipvm::Point32s2(ptCenter.m_x + nPadGap + nOffSetLength, ptCenter.m_y - nOffSetWidth);
                nptLB = Ipvm::Point32s2(ptCenter.m_x + nPadGap - nOffSetLength, ptCenter.m_y + nOffSetWidth);
                nptRB = Ipvm::Point32s2(ptCenter.m_x + nPadGap + nOffSetLength, ptCenter.m_y + nOffSetWidth);
                break;
            }
            case DOWN:
            {
                nptLT = Ipvm::Point32s2(ptCenter.m_x - nOffSetWidth, ptCenter.m_y + nPadGap - nOffSetLength);
                nptRT = Ipvm::Point32s2(ptCenter.m_x + nOffSetWidth, ptCenter.m_y + nPadGap - nOffSetLength);
                nptLB = Ipvm::Point32s2(ptCenter.m_x - nOffSetWidth, ptCenter.m_y + nPadGap + nOffSetLength);
                nptRB = Ipvm::Point32s2(ptCenter.m_x + nOffSetWidth, ptCenter.m_y + nPadGap + nOffSetLength);
                break;
            }
            default:
                break;
        }
    }
    else
    {
        switch (i_nDircetion)
        {
            case LEFT:
            {
                nptLT = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().TopLeft().m_x - nOffSetLength,
                    i_rtSpecROI.GetExtRect().TopLeft().m_y - nOffSetWidth);
                nptRT = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().TopLeft().m_x + nOffSetLength,
                    i_rtSpecROI.GetExtRect().TopLeft().m_y - nOffSetWidth);
                nptLB = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().BottomLeft().m_x - nOffSetLength,
                    i_rtSpecROI.GetExtRect().BottomLeft().m_y + nOffSetWidth);
                nptRB = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().BottomLeft().m_x + nOffSetLength,
                    i_rtSpecROI.GetExtRect().BottomLeft().m_y + nOffSetWidth);
                break;
            }
            case UP:
            {
                nptLT = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().TopLeft().m_x - nOffSetWidth,
                    i_rtSpecROI.GetExtRect().TopLeft().m_y - nOffSetLength);
                nptRT = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().TopRight().m_x + nOffSetWidth,
                    i_rtSpecROI.GetExtRect().TopLeft().m_y - nOffSetLength);
                nptLB = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().TopLeft().m_x - nOffSetWidth,
                    i_rtSpecROI.GetExtRect().TopLeft().m_y + nOffSetLength);
                nptRB = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().TopRight().m_x + nOffSetWidth,
                    i_rtSpecROI.GetExtRect().TopLeft().m_y + nOffSetLength);
                break;
            }
            case RIGHT:
            {
                nptLT = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().TopRight().m_x - nOffSetLength,
                    i_rtSpecROI.GetExtRect().TopRight().m_y - nOffSetWidth);
                nptRT = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().TopRight().m_x + nOffSetLength,
                    i_rtSpecROI.GetExtRect().TopRight().m_y - nOffSetWidth);
                nptLB = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().BottomRight().m_x - nOffSetLength,
                    i_rtSpecROI.GetExtRect().BottomRight().m_y + nOffSetWidth);
                nptRB = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().BottomRight().m_x + nOffSetLength,
                    i_rtSpecROI.GetExtRect().BottomRight().m_y + nOffSetWidth);
                break;
            }
            case DOWN:
            {
                nptLT = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().BottomLeft().m_x - nOffSetWidth,
                    i_rtSpecROI.GetExtRect().BottomLeft().m_y - nOffSetLength);
                nptRT = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().BottomRight().m_x + nOffSetWidth,
                    i_rtSpecROI.GetExtRect().BottomRight().m_y - nOffSetLength);
                nptLB = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().BottomLeft().m_x - nOffSetWidth,
                    i_rtSpecROI.GetExtRect().BottomLeft().m_y + nOffSetLength);
                nptRB = Ipvm::Point32s2(i_rtSpecROI.GetExtRect().BottomRight().m_x + nOffSetWidth,
                    i_rtSpecROI.GetExtRect().BottomRight().m_y + nOffSetLength);
                break;
            }
            default:
                break;
        }
    }

    Ipvm::Rect32s rtPadIgnoreArea(0, 0, 0, 0);
    rtPadIgnoreArea.m_left = long((nptLT.m_x + nptLB.m_x) * .5f);
    rtPadIgnoreArea.m_top = long((nptLT.m_y + nptRT.m_y) * .5f);
    rtPadIgnoreArea.m_right = long((nptRT.m_x + nptRB.m_x) * .5f);
    rtPadIgnoreArea.m_bottom = long((nptLB.m_y + nptRB.m_y) * .5f);

    //Image에 Fill해서 채워넣어서 return 해주자
    Ipvm::ImageProcessing::Fill(rtPadIgnoreArea, 255, o_MaskImage);

    return TRUE;

    //const auto& fptPixelperMM = scale.mmToPixel();
    //long nOffSetWidth(0), nOffSetLength(0);
    //if (i_fAngleType == PassiveAngle_Horizontal || i_fAngleType == PassiveAngle_Horizontal_reverse)
    //{
    //	nOffSetWidth = long(i_pPassiveInfoDB->fPassivePAD_Width_mm * fptPixelperMM.m_x);
    //	nOffSetLength = long(i_pPassiveInfoDB->fPassivePAD_Length_mm * fptPixelperMM.m_y);
    //}
    //else if (i_fAngleType == PassiveAngle_Vertical || i_fAngleType == PassiveAngle_Vertical_reverse)
    //{
    //	nOffSetWidth = long(i_pPassiveInfoDB->fPassivePAD_Length_mm * fptPixelperMM.m_y);
    //	nOffSetLength = long(i_pPassiveInfoDB->fPassivePAD_Width_mm * fptPixelperMM.m_x);
    //}

    ////좌표들 계산해주고
    //Ipvm::Point32s2 nptLT(0, 0), nptRT(0, 0), nptLB(0, 0), nptRB(0, 0);
    //switch (i_nDircetion)
    //{
    //case LEFT:
    //{
    //	nptLT = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().TopLeft().m_x - nOffSetLength, i_detailAlignResult.GetExtCRect().TopLeft().m_y - nOffSetWidth);
    //	nptRT = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().TopLeft().m_x + nOffSetLength, i_detailAlignResult.GetExtCRect().TopLeft().m_y - nOffSetWidth);
    //	nptLB = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().BottomLeft().m_x - nOffSetLength, i_detailAlignResult.GetExtCRect().BottomLeft().m_y + nOffSetWidth);
    //	nptRB = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().BottomLeft().m_x + nOffSetLength, i_detailAlignResult.GetExtCRect().BottomLeft().m_y + nOffSetWidth);
    //	break;
    //}
    //case UP:
    //{
    //	nptLT = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().TopLeft().m_x - nOffSetWidth, i_detailAlignResult.GetExtCRect().TopLeft().m_y - nOffSetLength);
    //	nptRT = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().TopRight().m_x + nOffSetWidth, i_detailAlignResult.GetExtCRect().TopLeft().m_y - nOffSetLength);
    //	nptLB = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().TopLeft().m_x - nOffSetWidth, i_detailAlignResult.GetExtCRect().TopLeft().m_y + nOffSetLength);
    //	nptRB = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().TopRight().m_x + nOffSetWidth, i_detailAlignResult.GetExtCRect().TopLeft().m_y + nOffSetLength);
    //	break;
    //}
    //case RIGHT:
    //{
    //	nptLT = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().TopRight().m_x - nOffSetLength, i_detailAlignResult.GetExtCRect().TopRight().m_y - nOffSetWidth);
    //	nptRT = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().TopRight().m_x + nOffSetLength, i_detailAlignResult.GetExtCRect().TopRight().m_y - nOffSetWidth);
    //	nptLB = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().BottomRight().m_x - nOffSetLength, i_detailAlignResult.GetExtCRect().BottomRight().m_y + nOffSetWidth);
    //	nptRB = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().BottomRight().m_x + nOffSetLength, i_detailAlignResult.GetExtCRect().BottomRight().m_y + nOffSetWidth);
    //	break;
    //}
    //case DOWN:
    //{
    //	nptLT = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().BottomLeft().m_x - nOffSetWidth, i_detailAlignResult.GetExtCRect().BottomLeft().m_y - nOffSetLength);
    //	nptRT = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().BottomRight().m_x + nOffSetWidth, i_detailAlignResult.GetExtCRect().BottomRight().m_y - nOffSetLength);
    //	nptLB = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().BottomLeft().m_x - nOffSetWidth, i_detailAlignResult.GetExtCRect().BottomLeft().m_y + nOffSetLength);
    //	nptRB = Ipvm::Point32s2(i_detailAlignResult.GetExtCRect().BottomRight().m_x + nOffSetWidth, i_detailAlignResult.GetExtCRect().BottomRight().m_y + nOffSetLength);
    //	break;
    //}
    //default:
    //	break;
    //}

    //Ipvm::Rect32s rtPadIgnoreArea(0, 0, 0, 0);
    //rtPadIgnoreArea.m_left = long((nptLT.m_x + nptLB.m_x) * .5f);
    //rtPadIgnoreArea.m_top = long((nptLT.m_y + nptRT.m_y) * .5f);
    //rtPadIgnoreArea.m_right = long((nptRT.m_x + nptRB.m_x) * .5f);
    //rtPadIgnoreArea.m_bottom = long((nptLB.m_y + nptRB.m_y) * .5f);

    ////Image에 Fill해서 채워넣어서 return 해주자
    //Ipvm::ImageProcessing::Fill(rtPadIgnoreArea, 255, o_MaskImage);

    //return TRUE;
}

BOOL VisionInspectionComponent2D::MakePreInspInfo(const bool detailSetupMode)
{
    float fBodyAngle_rad(0.f);
    Ipvm::Rect32r frtBodyAlign(0.f, 0.f, 0.f, 0.f);
    Ipvm::Point32r2 fptBodyCenter(0.f, 0.f);
    if (!GetBodyAlignInfo(fBodyAngle_rad, frtBodyAlign, fptBodyCenter))
    {
        return FALSE;
    }

    // 1. Package 정보 생성.
    MakePackageInfo(detailSetupMode, fBodyAngle_rad, fptBodyCenter);
    SetPackageInfo_SpecLink();

    if (m_pVisionInspDlg != nullptr)
    {
        if (m_pVisionInspDlg->m_pVisionInsp != nullptr)
        {
            if (m_pVisionInspDlg->IsWindowVisible())
            {
                m_pVisionInspDlg->UpdateChipList();
            }
        }
    }

    return TRUE;
}

BOOL VisionInspectionComponent2D::DoInsp(const bool detailSetupMode)
{
    BOOL bResult(TRUE);

    bResult &= Insp_Passive_Missing();
    bResult &= Insp_Passive_Rotate();
    bResult &= Insp_Passive_Shift_Length();
    bResult &= Insp_Passive_Shift_Width();
    bResult &= Insp_Passive_Tolerance_Length();
    bResult &= Insp_Passive_Tolerance_Width();

    UNREFERENCED_PARAMETER(detailSetupMode); //추후 위의 검사 함수에서 detailSetupMode를 사용할 수도 있다.

    if (m_pVisionInspDlg)
    {
        if (m_pVisionInspDlg->m_pVisionInspParaDlg)
            if (m_pVisionInspDlg->IsWindowVisible())
                m_pVisionInspDlg->UpdateChipList();
    }

    return bResult;
}

BOOL VisionInspectionComponent2D::Insp_Passive_Missing()
{
    auto* result = m_resultGroup.GetResultByName(g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_MISSING]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    long nPassiveNum = (long)m_vecsPassiveInfoDB.size();

    result->Clear();
    result->Resize(nPassiveNum);

    long nMissingNum(1);
    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        PassiveAlignResult* pAlignResult = (PassiveAlignResult*)m_vecsPassiveInfoDB[nPassive].pResult;

        if (pAlignResult == NULL)
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nPassive, PassiveInfoDB->strCompName,
                PassiveInfoDB->strCompType, PassiveInfoDB->strCompType, CAST_FLOAT(nMissingNum), *spec,
                PassiveInfoDB->fSpecPos_um.m_x, PassiveInfoDB->fSpecPos_um.m_y); //kircheis_MED2.5

            result->SetRect(nPassive, PassiveInfoDB->sfrtSpecROI_px.GetExtCRect());

            nMissingNum++;

            continue;
        }

        long nResult = result->IsValueCheck(spec, 1.f);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(nPassive, PassiveInfoDB->strCompName,
            PassiveInfoDB->strCompType, PassiveInfoDB->strCompType, 0.f, *spec, PassiveInfoDB->fSpecPos_um.m_x,
            PassiveInfoDB->fSpecPos_um.m_y); //kircheis_MED2.5

        result->SetRect(nPassive, pAlignResult->m_chip_resFinal.GetExtCRect());

        if (nResult != PASS)
            m_vecrtReject.push_back(pAlignResult->m_chip_resFinal.GetExtCRect());
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return (result->m_totalResult == PASS) || (result->m_totalResult == MARGINAL);
}

BOOL VisionInspectionComponent2D::Insp_Passive_Rotate()
{
    auto* result = m_resultGroup.GetResultByName(g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_ROTATE]);
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

        groupResults[strType] = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strCompRotateName);
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

    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        PassiveAlignResult* pAlignResult = (PassiveAlignResult*)m_vecsPassiveInfoDB[nPassive].pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Passive_Rotate(spec, result, PassiveInfoDB, pAlignResult, nPassive);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                Judg_Passive_Rotate(
                    groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult, vecGroupCount[strType]);

                vecGroupCount[strType]++;
            }
        }
    }

    BOOL nInspResult = FALSE;

    if (spec->m_use)
    {
        nInspResult = (result->m_totalResult == PASS) || (result->m_totalResult == MARGINAL);

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

        nInspResult
            |= (groupResults[strType]->m_totalResult == PASS) || (groupResults[strType]->m_totalResult == MARGINAL);
    }

    return nInspResult;
}

BOOL VisionInspectionComponent2D::Judg_Passive_Rotate(VisionInspectionSpec* spec, VisionInspectionResult* result,
    sPassive_InfoDB* chipInfo, PassiveAlignResult* chipAlign, long ObjectNum)
{
    if (chipAlign == NULL)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, Ipvm::k_noiseValue32r, *spec, chipInfo->fSpecPos_um.m_x,
            chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        return FALSE;
    }

    float fRotateValue = chipAlign->m_fRotateDegree;

    long nResult = result->IsValueCheck(spec, fRotateValue);

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fRotateValue, *spec, chipInfo->fSpecPos_um.m_x,
        chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

    result->SetRect(ObjectNum, chipAlign->m_chip_resFinal.GetExtCRect());

    if (nResult != PASS)
        m_vecrtReject.push_back(chipAlign->m_chip_resFinal.GetExtCRect());

    return TRUE;
}

BOOL VisionInspectionComponent2D::Insp_Passive_Shift_Width()
{
    auto* result = m_resultGroup.GetResultByName(g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_OFFSET_X]);
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

        groupResults[strType]
            = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strCompShiftWidthName);
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

    float fMaxOffsetValue(0.f);

    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        PassiveAlignResult* pAlignResult = (PassiveAlignResult*)m_vecsPassiveInfoDB[nPassive].pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Passive_Shift_Width(spec, result, PassiveInfoDB, pAlignResult, nPassive, &fMaxOffsetValue);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                Judg_Passive_Shift_Width(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &fMaxOffsetValue);

                vecGroupCount[strType]++;
            }
        }
    }

    BOOL nInspResult = FALSE;

    if (spec->m_use)
    {
        nInspResult = (result->m_totalResult == PASS) || (result->m_totalResult == MARGINAL);

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

        nInspResult
            |= (groupResults[strType]->m_totalResult == PASS) || (groupResults[strType]->m_totalResult == MARGINAL);
    }

    return nInspResult;
}

BOOL VisionInspectionComponent2D::Judg_Passive_Shift_Width(VisionInspectionSpec* spec, VisionInspectionResult* result,
    sPassive_InfoDB* chipInfo, PassiveAlignResult* chipAlign, long ObjectNum, float* fMaxOffsetValue)
{
    const float px2um = getScale().pixelToUmXY();

    if (chipAlign == NULL)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, Ipvm::k_noiseValue32r, *spec, chipInfo->fSpecPos_um.m_x,
            chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        return FALSE;
    }

    float fShiftInspSpecPos(0.f);
    float fShfitInspAlignPos(0.f);

    if (chipInfo->fPassiveAngle == PassiveAngle_Horizontal
        || chipInfo->fPassiveAngle == PassiveAngle_Horizontal_reverse)
    {
        fShiftInspSpecPos = chipInfo->sfrtSpecROI_px.GetCenter().m_y;
        fShfitInspAlignPos = chipAlign->m_chip_resFinal.GetCenter().m_y;
    }
    else if (chipInfo->fPassiveAngle == PassiveAngle_Vertical
        || chipInfo->fPassiveAngle == PassiveAngle_Vertical_reverse)
    {
        fShiftInspSpecPos = chipInfo->sfrtSpecROI_px.GetCenter().m_x;
        fShfitInspAlignPos = chipAlign->m_chip_resFinal.GetCenter().m_x;
    }

    float fErrorShiftWdith(0.f);
    fErrorShiftWdith = (fShfitInspAlignPos - fShiftInspSpecPos) * px2um;

    long nResult = result->IsValueCheck(spec, fErrorShiftWdith);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fErrorShiftWdith, *spec, chipInfo->fSpecPos_um.m_x,
        chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

    result->SetRect(ObjectNum, chipAlign->m_chip_resFinal.GetExtCRect());

    if (fErrorShiftWdith > *fMaxOffsetValue)
        *fMaxOffsetValue = fErrorShiftWdith;

    if (nResult != PASS)
        m_vecrtReject.push_back(chipAlign->m_chip_resFinal.GetExtCRect());

    return TRUE;
}

BOOL VisionInspectionComponent2D::Insp_Passive_Shift_Length()
{
    auto* result = m_resultGroup.GetResultByName(g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_OFFSET_Y]);
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

        groupResults[strType]
            = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strCompShiftLengthName);
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

    float fMaxOffsetValue(0.f);

    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        PassiveAlignResult* pAlignResult = (PassiveAlignResult*)m_vecsPassiveInfoDB[nPassive].pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Passive_Shift_Length(spec, result, PassiveInfoDB, pAlignResult, nPassive, &fMaxOffsetValue);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                Judg_Passive_Shift_Length(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &fMaxOffsetValue);

                vecGroupCount[strType]++;
            }
        }
    }

    BOOL nInspResult = FALSE;

    if (spec->m_use)
    {
        nInspResult = (result->m_totalResult == PASS) || (result->m_totalResult == MARGINAL);

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

        nInspResult
            |= (groupResults[strType]->m_totalResult == PASS) || (groupResults[strType]->m_totalResult == MARGINAL);
    }

    return nInspResult;
}

BOOL VisionInspectionComponent2D::Judg_Passive_Shift_Length(VisionInspectionSpec* spec, VisionInspectionResult* result,
    sPassive_InfoDB* chipInfo, PassiveAlignResult* chipAlign, long ObjectNum, float* fMaxOffsetValue)
{
    const float px2um = getScale().pixelToUmXY();

    if (chipAlign == NULL)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, Ipvm::k_noiseValue32r, *spec, chipInfo->fSpecPos_um.m_x,
            chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        return FALSE;
    }

    float fErrorShiftLength(0.f);

    float fShiftInspSpecPos(0.f);
    float fShfitInspAlignPos(0.f);

    if (chipInfo->fPassiveAngle == PassiveAngle_Horizontal
        || chipInfo->fPassiveAngle == PassiveAngle_Horizontal_reverse)
    {
        fShiftInspSpecPos = chipInfo->sfrtSpecROI_px.GetCenter().m_x;
        fShfitInspAlignPos = chipAlign->m_chip_resFinal.GetCenter().m_x;
    }
    else if (chipInfo->fPassiveAngle == PassiveAngle_Vertical
        || chipInfo->fPassiveAngle == PassiveAngle_Vertical_reverse)
    {
        fShiftInspSpecPos = chipInfo->sfrtSpecROI_px.GetCenter().m_y;
        fShfitInspAlignPos = chipAlign->m_chip_resFinal.GetCenter().m_y;
    }

    fErrorShiftLength = (fShfitInspAlignPos - fShiftInspSpecPos) * px2um;

    long nResult = result->IsValueCheck(spec, fErrorShiftLength);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fErrorShiftLength, *spec, chipInfo->fSpecPos_um.m_x,
        chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

    result->SetRect(ObjectNum, chipAlign->m_chip_resFinal.GetExtCRect());

    if (fErrorShiftLength > *fMaxOffsetValue)
        *fMaxOffsetValue = fErrorShiftLength;

    if (nResult != PASS)
        m_vecrtReject.push_back(chipAlign->m_chip_resFinal.GetExtCRect());

    return TRUE;
}

BOOL VisionInspectionComponent2D::Insp_Passive_Tolerance_Width() //Total Width
{
    auto* result = m_resultGroup.GetResultByName(g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_WIDTH]);
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

        groupResults[strType] = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strCompWidthName);
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

    long nTotalResult(NOT_MEASURED);
    std::map<CString, long> vecGroupTotalResult;

    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        PassiveAlignResult* pAlignResult = (PassiveAlignResult*)m_vecsPassiveInfoDB[nPassive].pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Passive_Tolerance_Width(spec, result, PassiveInfoDB, pAlignResult, nPassive, &nTotalResult);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                vecGroupTotalResult[strType] = NOT_MEASURED;
                Judg_Passive_Tolerance_Width(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &vecGroupTotalResult[strType]);

                vecGroupCount[strType]++;
            }
        }
    }

    BOOL nInspResult = FALSE;

    if (spec->m_use)
    {
        nInspResult = (result->m_totalResult == PASS) || (result->m_totalResult == MARGINAL);

        result->m_totalResult = nTotalResult;

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

        nInspResult
            |= (groupResults[strType]->m_totalResult == PASS) || (groupResults[strType]->m_totalResult == MARGINAL);
    }

    return nInspResult;
}

BOOL VisionInspectionComponent2D::Judg_Passive_Tolerance_Width(VisionInspectionSpec* spec,
    VisionInspectionResult* result, sPassive_InfoDB* chipInfo, PassiveAlignResult* chipAlign, long ObjectNum,
    long* totalResult)
{
    const auto& mm2px = getScale().mmToPixel();
    const float px2um = getScale().pixelToUmXY();

    if (chipAlign == NULL)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, Ipvm::k_noiseValue32r, *spec, chipInfo->fSpecPos_um.m_x,
            chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        return FALSE;
    }

    float fSpecWidth = chipInfo->fPassiveSpecWidth_mm * mm2px.m_x;
    float fAlignWidth(0.f);
    float fErrorWidth(0.f);

    if (chipInfo->fPassiveAngle == PassiveAngle_Horizontal
        || chipInfo->fPassiveAngle == PassiveAngle_Horizontal_reverse)
        fAlignWidth = chipAlign->m_chip_resFinal.GetFRect().Height();

    if (chipInfo->fPassiveAngle == PassiveAngle_Vertical || chipInfo->fPassiveAngle == PassiveAngle_Vertical_reverse)
        fAlignWidth = chipAlign->m_chip_resFinal.GetFRect().Width();

    fErrorWidth = (fAlignWidth - fSpecWidth) * px2um;

    long nResult = result->IsValueCheck(spec, fErrorWidth);
    *totalResult = max(*totalResult, nResult);

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fErrorWidth, *spec, chipInfo->fSpecPos_um.m_x,
        chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

    result->SetRect(ObjectNum, chipAlign->m_chip_resFinal.GetExtCRect());

    if (nResult != PASS)
    {
        m_vecrtReject.push_back(chipAlign->m_chip_resFinal.GetExtCRect());
        result->vecrtRejectROI.push_back(chipAlign->m_chip_resFinal.GetExtCRect());
    }

    return TRUE;
}

BOOL VisionInspectionComponent2D::Insp_Passive_Tolerance_Length()
{
    auto* result = m_resultGroup.GetResultByName(g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_LENGTH]);
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

        groupResults[strType] = m_resultGroup.GetResultByName(m_VisionPara->m_vecstrGroupInspName[i].strCompLengthName);
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

    long nTotalResult(NOT_MEASURED);
    std::map<CString, long> vecGroupTotalResult;

    for (long nPassive = 0; nPassive < nPassiveNum; nPassive++)
    {
        auto* PassiveInfoDB = &m_vecsPassiveInfoDB[nPassive];
        PassiveAlignResult* pAlignResult = (PassiveAlignResult*)m_vecsPassiveInfoDB[nPassive].pResult;
        CString strType = PassiveInfoDB->strCompSpec;

        if (spec->m_use)
            Judg_Passive_Tolerance_Length(spec, result, PassiveInfoDB, pAlignResult, nPassive, &nTotalResult);

        if (groupSpecs.size() > 0 && groupSpecs[strType] != nullptr)
        {
            if (groupSpecs[strType]->m_use)
            {
                vecGroupTotalResult[strType] = NOT_MEASURED;
                Judg_Passive_Tolerance_Length(groupSpecs[strType], groupResults[strType], PassiveInfoDB, pAlignResult,
                    vecGroupCount[strType], &vecGroupTotalResult[strType]);

                vecGroupCount[strType]++;
            }
        }
    }

    BOOL nInspResult = FALSE;

    if (spec->m_use)
    {
        nInspResult = (result->m_totalResult == PASS) || (result->m_totalResult == MARGINAL);

        result->m_totalResult = nTotalResult;

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

        nInspResult
            |= (groupResults[strType]->m_totalResult == PASS) || (groupResults[strType]->m_totalResult == MARGINAL);
    }

    return nInspResult;
}

BOOL VisionInspectionComponent2D::Judg_Passive_Tolerance_Length(VisionInspectionSpec* spec,
    VisionInspectionResult* result, sPassive_InfoDB* chipInfo, PassiveAlignResult* chipAlign, long ObjectNum,
    long* totalResult)
{
    const auto& mm2px = getScale().mmToPixel();
    const float px2um = getScale().pixelToUmXY();

    if (chipAlign == NULL)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
            chipInfo->strCompType, Ipvm::k_noiseValue32r, *spec, chipInfo->fSpecPos_um.m_x,
            chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

        result->SetRect(ObjectNum, chipInfo->sfrtSpecROI_px.GetExtCRect());

        return FALSE;
    }

    float fSpecLength = chipInfo->fPassiveSpecLength_mm * mm2px.m_y;
    float fAlignLength(0.f);
    float fErrorLength(0.f);

    if (chipInfo->fPassiveAngle == PassiveAngle_Horizontal
        || chipInfo->fPassiveAngle == PassiveAngle_Horizontal_reverse)
        fAlignLength = chipAlign->m_chip_resFinal.GetFRect().Width();

    if (chipInfo->fPassiveAngle == PassiveAngle_Vertical || chipInfo->fPassiveAngle == PassiveAngle_Vertical_reverse)
        fAlignLength = chipAlign->m_chip_resFinal.GetFRect().Height();

    fErrorLength = (fAlignLength - fSpecLength) * px2um;

    long nResult = result->IsValueCheck(spec, fErrorLength);
    *totalResult = max(*totalResult, nResult);

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(ObjectNum, chipInfo->strCompName, chipInfo->strCompSpec,
        chipInfo->strCompType, fErrorLength, *spec, chipInfo->fSpecPos_um.m_x,
        chipInfo->fSpecPos_um.m_y); //kircheis_MED2.5

    result->SetRect(ObjectNum, chipAlign->m_chip_resFinal.GetExtCRect());

    if (nResult != PASS)
    {
        m_vecrtReject.push_back(chipAlign->m_chip_resFinal.GetExtCRect());
        result->vecrtRejectROI.push_back(chipAlign->m_chip_resFinal.GetExtCRect());
    }

    return TRUE;
}

Ipvm::Point32r2 VisionInspectionComponent2D::ConvertSpecPosMMtoUM(float i_fSpecPosX_mm, float i_fSpecPosY_mm)
{
    Ipvm::Point32r2 fSpecPosXY_um(0.f, 0.f);
    float fMMtoum = 1000.f;

    fSpecPosXY_um.m_x = i_fSpecPosX_mm * fMMtoum;
    fSpecPosXY_um.m_y = i_fSpecPosY_mm * fMMtoum;

    return fSpecPosXY_um;
}