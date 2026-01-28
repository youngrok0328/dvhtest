//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionSide2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionSide2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../InformationModule/dPI_DataBase/BodyInfo.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionSide2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionSide2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    Ipvm::TimeCheck time;

    ResetResult();

    if (!m_sEdgeAlignResult->bAvailable)
    {
        m_bInvalid = TRUE;
        return FALSE;
    }

    BOOL bInsp = DoInsp(detailSetupMode);

    // Debug Info...
    SetDebugInfo(detailSetupMode);

    m_fCalcTime = (float)time.Elapsed_ms();

    return bInsp;
}

BOOL VisionInspectionSide2D::DoInsp(const bool detailSetupMode)
{
    BOOL bModuleInsp = false;
    for (auto& spec : m_fixedInspectionSpecs)
        bModuleInsp |= spec.m_use;

    if (!bModuleInsp)
        return TRUE;

    BOOL bInspResult = TRUE;

    Ipvm::Rect32s rtPackage;
    Ipvm::Rect32s rtGlass;
    Ipvm::Rect32s rtTopSubstrate;
    Ipvm::Rect32s rtBottomSubstrate;

    m_vecptDetailAlign.clear();
    m_vecptGlassCoreAlign.clear();
    m_vecptTopSubstrateAlign.clear();
    m_vecptBottomSubstrateAlign.clear();

    GetSideAlignInfo(ALIGN_OBJECT_SIDE_TOTAL_INFO, rtPackage, m_vecptDetailAlign);
    GetSideAlignInfo(ALIGN_OBJECT_SIDE_GLASS_INFO, rtGlass, m_vecptGlassCoreAlign);
    GetSideAlignInfo(ALIGN_OBJECT_SIDE_TOP_SUBSTRATE_INFO, rtTopSubstrate, m_vecptTopSubstrateAlign);
    GetSideAlignInfo(ALIGN_OBJECT_SIDE_BOTTOM_SUBSTRATE_INFO, rtBottomSubstrate, m_vecptBottomSubstrateAlign);

    SetDebugInfo(detailSetupMode);

    bInspResult &= InspTotalThickness(detailSetupMode);
    bInspResult &= InspGlassThickness(detailSetupMode);
    bInspResult &= InspTopSubstrateThickness(detailSetupMode);
    bInspResult &= InspBottomSubstrateThickness(detailSetupMode);
    bInspResult &= InspSideWarpage(detailSetupMode);

    if (!bInspResult)
    {
        m_bInvalid = TRUE;
    }

    return bInspResult;
}

void VisionInspectionSide2D::SetDebugInfo(const bool detailSetupMode)
{
    if (detailSetupMode)
    {
        SetDebugInfoItem(detailSetupMode, _T("Side-Detail Align Result"), m_vecptDetailAlign);
        SetDebugInfoItem(detailSetupMode, _T("Side-Glass Align Result"), m_vecptGlassCoreAlign);
        SetDebugInfoItem(detailSetupMode, _T("Side-Top Substrate Align Result"), m_vecptTopSubstrateAlign);
        SetDebugInfoItem(detailSetupMode, _T("Side-Bottom Substrate Align Result"), m_vecptBottomSubstrateAlign);
    }
}

BOOL VisionInspectionSide2D::GetSideAlignInfo(
    const long& i_nObject, Ipvm::Rect32s& o_rtROI, std::vector<Ipvm::Point32r2>& o_vecPoint)
{
    o_rtROI = Ipvm::Rect32s(0, 0, 0, 0);
    o_vecPoint.clear();

    if (i_nObject < ALIGN_OBJECT_SIDE_START || i_nObject >= ALIGN_OBJECT_SIDE_END)
        return FALSE;

    CString strInfoName;
    switch (i_nObject)
    {
        case ALIGN_OBJECT_SIDE_TOTAL_INFO:
            strInfoName = DEBUGINFO_SDA_NormalUnitAlignResult; //.Format(_T("Detail Align Result"));
            break;
        case ALIGN_OBJECT_SIDE_GLASS_INFO:
            strInfoName = DEBUGINFO_SDA_GlassAlignResult; // .Format(_T("Glass Align Result"));
            break;
        case ALIGN_OBJECT_SIDE_TOP_SUBSTRATE_INFO:
            strInfoName = DEBUGINFO_SDA_TopSubstrateAlignResult; // .Format(_T("Top Substrate Align Result"));
            break;
        case ALIGN_OBJECT_SIDE_BOTTOM_SUBSTRATE_INFO:
            strInfoName = DEBUGINFO_SDA_BottomSubstrateAlignResult; // .Format(_T("Bottom Substrate Align Result"));
            break;
        default:
            return FALSE;
    }

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_SIDE_DETAIL_ALIGN, strInfoName, nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        o_vecPoint.resize(nDataNum);
        Ipvm::Point32r2* pptPoint = (Ipvm::Point32r2*)pData;

        o_rtROI = Ipvm::Rect32s(100000000, 100000000, -100000000, -100000000);

        for (long nID = 0; nID < nDataNum; nID++)
        {
            o_vecPoint[nID] = pptPoint[nID];
            o_rtROI.m_left = (int32_t)min(o_rtROI.m_left, o_vecPoint[nID].m_x);
            o_rtROI.m_top = (int32_t)min(o_rtROI.m_top, o_vecPoint[nID].m_y);
            o_rtROI.m_right = (int32_t)max(o_rtROI.m_right, o_vecPoint[nID].m_x);
            o_rtROI.m_bottom = (int32_t)max(o_rtROI.m_bottom, o_vecPoint[nID].m_y);
        }
    }

    return TRUE;
}

BOOL VisionInspectionSide2D::GetDivideToTopBottomPoint(const bool detailSetupMode,
    const std::vector<Ipvm::Point32r2>& i_vecptAlignInfo, std::vector<Ipvm::Point32r2>& o_vecptTopPoint,
    std::vector<Ipvm::Point32r2>& o_vecptBottomInfo)
{
    o_vecptTopPoint.clear();
    o_vecptBottomInfo.clear();

    const long nPointNum = (long)i_vecptAlignInfo.size();
    const long nDivideNum = nPointNum / 2;
    if (nPointNum == 0 || nPointNum % 2 == 1
        || nDivideNum < 3) //검사 대상에서 좌우 측면의 최외곽 Point는 제외할 거라 분항 Point의 개수가 3이상이어야 함
    {
        if (detailSetupMode == true)
            ::SimpleMessage(_T("Please check the result of [Side Detail Align]."));
        return FALSE;
    }

    o_vecptTopPoint.resize(nDivideNum);
    o_vecptBottomInfo.resize(nDivideNum);

    for (long nIdx = 0; nIdx < nDivideNum; nIdx++)
    {
        o_vecptTopPoint[nIdx] = i_vecptAlignInfo[nIdx];
        o_vecptBottomInfo[nIdx] = i_vecptAlignInfo[nPointNum - nIdx - 1];
    }

    return TRUE;
}

BOOL VisionInspectionSide2D::InspTotalThickness(const bool detailSetupMode)
{
    auto* result = m_resultGroup.GetResultByName(g_szSide2DInspectionName[SIDE_INSPECTION_2D_TOTAL_THICKNESS]);
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

    auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;
    const float fSpecThickness_um = bodyInfoMaster->fBodyThickness;
    if (fSpecThickness_um <= 0.f)
    {
        return FALSE;
    }

    const auto& px2um = getScale().pixelToUm();
    const float fScalePxlToUmY = px2um.m_y;

    std::vector<Ipvm::Point32r2> vecptTop(0);
    std::vector<Ipvm::Point32r2> vecptBottom(0);
    if (GetDivideToTopBottomPoint(detailSetupMode, m_vecptDetailAlign, vecptTop, vecptBottom) == FALSE)
    {
        return FALSE;
    }

    long nPointNum = (long)vecptTop.size();
    long nInspNum = nPointNum - 2; // 좌우의 최외곽 포인트는 검사 하지 않는다.
    const static long nStart = 1;
    const long nEnd = nPointNum - 1;

    long nIdx = 0;
    float fError(0.f);
    float fThickness(0.f);
    Ipvm::Rect32s rtInspROI;
    CString strObjName;

    result->Resize(nInspNum);

    for (long idx = nStart; idx < nEnd; idx++, nIdx++)
    {
        strObjName.Format(_T("Point %d"), idx);
        rtInspROI = Ipvm::Conversion::ToRect32s(
            Ipvm::Rect32r(vecptTop[idx].m_x, vecptTop[idx].m_y, vecptBottom[idx].m_x, vecptBottom[idx].m_y));
        rtInspROI.InflateRect(30, 0);

        Ipvm::Geometry::GetDistance(vecptTop[idx], vecptBottom[idx], fThickness);
        fThickness *= fScalePxlToUmY;
        fError = fThickness - fSpecThickness_um;
        result->SetRect(nIdx, rtInspROI);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nIdx, strObjName, _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecThickness_um);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionSide2D::InspGlassThickness(const bool detailSetupMode)
{
    auto* result = m_resultGroup.GetResultByName(g_szSide2DInspectionName[SIDE_INSPECTION_2D_GLASS_THICKNESS]);
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

    if (VerifyNormalUnitInspection(detailSetupMode, result->m_resultName) == FALSE)
    {
        return FALSE;
    }

    auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;
    const float fSpecGlassThickness_um = bodyInfoMaster->m_fGlassCoreThickness;
    if (bodyInfoMaster->m_bGlassCorePackage == FALSE || fSpecGlassThickness_um <= 0.f)
    {
        return FALSE;
    }

    const auto& px2um = getScale().pixelToUm();
    const float fScalePxlToUmY = px2um.m_y;

    std::vector<Ipvm::Point32r2> vecptTop(0);
    std::vector<Ipvm::Point32r2> vecptBottom(0);
    if (GetDivideToTopBottomPoint(detailSetupMode, m_vecptGlassCoreAlign, vecptTop, vecptBottom) == FALSE)
    {
        return FALSE;
    }

    long nPointNum = (long)vecptTop.size();
    long nInspNum = nPointNum - 2; // 좌우의 최외곽 포인트는 검사 하지 않는다.
    const static long nStart = 1;
    const long nEnd = nPointNum - 1;

    long nIdx = 0;
    float fError(0.f);
    float fThickness(0.f);
    Ipvm::Rect32s rtInspROI;
    CString strObjName;

    result->Resize(nInspNum);

    for (long idx = nStart; idx < nEnd; idx++, nIdx++)
    {
        strObjName.Format(_T("Point %d"), idx);
        rtInspROI = Ipvm::Conversion::ToRect32s(
            Ipvm::Rect32r(vecptTop[idx].m_x, vecptTop[idx].m_y, vecptBottom[idx].m_x, vecptBottom[idx].m_y));
        rtInspROI.InflateRect(30, 0);

        Ipvm::Geometry::GetDistance(vecptTop[idx], vecptBottom[idx], fThickness);
        fThickness *= fScalePxlToUmY;
        fError = fThickness - fSpecGlassThickness_um;
        result->SetRect(nIdx, rtInspROI);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nIdx, strObjName, _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecGlassThickness_um);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionSide2D::InspTopSubstrateThickness(const bool detailSetupMode)
{
    auto* result = m_resultGroup.GetResultByName(g_szSide2DInspectionName[SIDE_INSPECTION_2D_TOP_SUBSTRATE_THICKNESS]);
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

    if (VerifyNormalUnitInspection(detailSetupMode, result->m_resultName) == FALSE)
    {
        return FALSE;
    }

    auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;
    const float fSpecTopSubstrateThickness_um = bodyInfoMaster->m_fGlassCoreTopSubstrateThickness;
    if (bodyInfoMaster->m_bGlassCorePackage == FALSE || fSpecTopSubstrateThickness_um <= 0.f)
    {
        return FALSE;
    }

    const auto& px2um = getScale().pixelToUm();
    const float fScalePxlToUmY = px2um.m_y;

    std::vector<Ipvm::Point32r2> vecptTop(0);
    std::vector<Ipvm::Point32r2> vecptBottom(0);
    if (GetDivideToTopBottomPoint(detailSetupMode, m_vecptTopSubstrateAlign, vecptTop, vecptBottom) == FALSE)
    {
        return FALSE;
    }

    long nPointNum = (long)vecptTop.size();
    long nInspNum = nPointNum - 2; // 좌우의 최외곽 포인트는 검사 하지 않는다.
    const static long nStart = 1;
    const long nEnd = nPointNum - 1;

    long nIdx = 0;
    float fError(0.f);
    float fThickness(0.f);
    Ipvm::Rect32s rtInspROI;
    CString strObjName;

    result->Resize(nInspNum);

    for (long idx = nStart; idx < nEnd; idx++, nIdx++)
    {
        strObjName.Format(_T("Point %d"), idx);
        rtInspROI = Ipvm::Conversion::ToRect32s(
            Ipvm::Rect32r(vecptTop[idx].m_x, vecptTop[idx].m_y, vecptBottom[idx].m_x, vecptBottom[idx].m_y));
        rtInspROI.InflateRect(30, 10);

        Ipvm::Geometry::GetDistance(vecptTop[idx], vecptBottom[idx], fThickness);
        fThickness *= fScalePxlToUmY;
        fError = fThickness - fSpecTopSubstrateThickness_um;
        result->SetRect(nIdx, rtInspROI);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nIdx, strObjName, _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecTopSubstrateThickness_um);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionSide2D::InspBottomSubstrateThickness(const bool detailSetupMode)
{
    auto* result
        = m_resultGroup.GetResultByName(g_szSide2DInspectionName[SIDE_INSPECTION_2D_BOTTOM_SUBSTRATE_THICKNESS]);
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

    if (VerifyNormalUnitInspection(detailSetupMode, result->m_resultName) == FALSE)
    {
        return FALSE;
    }

    auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;
    const float fSpecBottomSubstrateThickness_um = bodyInfoMaster->m_fGlassCoreBottomSubstrateThickness;
    if (bodyInfoMaster->m_bGlassCorePackage == FALSE || fSpecBottomSubstrateThickness_um <= 0.f)
    {
        return FALSE;
    }

    const auto& px2um = getScale().pixelToUm();
    const float fScalePxlToUmY = px2um.m_y;

    std::vector<Ipvm::Point32r2> vecptTop(0);
    std::vector<Ipvm::Point32r2> vecptBottom(0);
    if (GetDivideToTopBottomPoint(detailSetupMode, m_vecptBottomSubstrateAlign, vecptTop, vecptBottom) == FALSE)
    {
        return FALSE;
    }

    long nPointNum = (long)vecptTop.size();
    long nInspNum = nPointNum - 2; // 좌우의 최외곽 포인트는 검사 하지 않는다.
    const static long nStart = 1;
    const long nEnd = nPointNum - 1;

    long nIdx = 0;
    float fError(0.f);
    float fThickness(0.f);
    Ipvm::Rect32s rtInspROI;
    CString strObjName;

    result->Resize(nInspNum);

    for (long idx = nStart; idx < nEnd; idx++, nIdx++)
    {
        strObjName.Format(_T("Point %d"), idx);
        rtInspROI = Ipvm::Conversion::ToRect32s(
            Ipvm::Rect32r(vecptTop[idx].m_x, vecptTop[idx].m_y, vecptBottom[idx].m_x, vecptBottom[idx].m_y));
        rtInspROI.InflateRect(30, 10);

        Ipvm::Geometry::GetDistance(vecptTop[idx], vecptBottom[idx], fThickness);
        fThickness *= fScalePxlToUmY;
        fError = fThickness - fSpecBottomSubstrateThickness_um;
        result->SetRect(nIdx, rtInspROI);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nIdx, strObjName, _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecBottomSubstrateThickness_um);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionSide2D::InspSideWarpage(const bool detailSetupMode)
{
    auto* result = m_resultGroup.GetResultByName(g_szSide2DInspectionName[SIDE_INSPECTION_2D_SIDE_WARPAGE]);
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

    const bool bIsGlassCoreModel = m_packageSpec.m_bodyInfoMaster->m_bGlassCorePackage;

    const auto& px2um = getScale().pixelToUm();
    const float fScalePxlToUmX = px2um.m_x;
    const float fScalePxlToUmY = px2um.m_y;

    std::vector<Ipvm::Point32r2> vecptTop(0);
    std::vector<Ipvm::Point32r2> vecptBottom(0);
    auto& vecptAlignedPoint = bIsGlassCoreModel == true ? m_vecptBottomSubstrateAlign : m_vecptDetailAlign;
    if (GetDivideToTopBottomPoint(detailSetupMode, vecptAlignedPoint, vecptTop, vecptBottom) == FALSE)
    {
        return FALSE;
    }

    long nPointNum = (long)vecptTop.size();
    std::vector<Ipvm::Point32r2> vecptPoint(nPointNum);
    std::vector<Ipvm::Point32r2> vecptPointUm(nPointNum);
    Ipvm::Point32r2 ptProjection;
    Ipvm::LineEq32r lineqCenter;
    std::vector<float> vecfDist(nPointNum);
    float fDist(0.f);
    float fMin = 1000000.f;
    Ipvm::Rect32s rtInspROI;
    CString strObjName;

    //우선 모든 좌표를 um 단위로 환산하고
    for (long nIdx = 0; nIdx < nPointNum; nIdx++)
    {
        vecptPoint[nIdx] = Ipvm::Point32r2(
            (vecptTop[nIdx].m_x + vecptBottom[nIdx].m_x) * .5f, (vecptTop[nIdx].m_y + vecptBottom[nIdx].m_y) * .5f);
        vecptPointUm[nIdx].m_x = vecptPoint[nIdx].m_x * fScalePxlToUmX;
        vecptPointUm[nIdx].m_y = vecptPoint[nIdx].m_y * fScalePxlToUmY;
    }

    //직선 피팅을 한다.
    Ipvm::DataFitting::FitToLine(nPointNum, &vecptPointUm[0], lineqCenter);

    //피팅 Line을 기준으로 직교 거리를 계산 후 (이때 Line의 상하를 명확히 한다.)
    for (long nIdx = 0; nIdx < nPointNum; nIdx++)
    {
        Ipvm::Geometry::GetDistance(lineqCenter, vecptPointUm[nIdx], fDist);
        Ipvm::Geometry::GetFootOfPerpendicular(lineqCenter, vecptPointUm[nIdx], ptProjection);
        if (ptProjection.m_y > vecptPointUm[nIdx].m_y)
            fDist *= -1.f;

        vecfDist[nIdx] = fDist;

        fMin = (float)min(fMin, fDist);
    }
    //Min Value가 0이 되게끔 모든 값에 Min Value를 뺀다(왜? Min Value는 마이너스니까 빼면 더해주는 효과다)
    result->Resize(nPointNum);

    for (long nIdx = 0; nIdx < nPointNum; nIdx++)
    {
        strObjName.Format(_T("Point %d"), nIdx + 1);
        rtInspROI = Ipvm::Conversion::ToRect32s(
            Ipvm::Rect32r(vecptTop[nIdx].m_x, vecptTop[nIdx].m_y, vecptBottom[nIdx].m_x, vecptBottom[nIdx].m_y));
        rtInspROI.InflateRect(30, 10);

        vecfDist[nIdx] -= fMin;

        result->SetRect(nIdx, rtInspROI);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nIdx, strObjName, _T(""), _T(""), vecfDist[nIdx], *spec, 0.f, 0.f);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

void VisionInspectionSide2D::ResetNormalModelInspectionItem()
{
    const bool bIsGlassCoreModel = m_packageSpec.m_bodyInfoMaster->m_bGlassCorePackage;

    if (bIsGlassCoreModel == true)
        return;

    auto* resultTopSubstrateThickness
        = m_resultGroup.GetResultByName(g_szSide2DInspectionName[SIDE_INSPECTION_2D_TOP_SUBSTRATE_THICKNESS]);
    if (resultTopSubstrateThickness != nullptr)
    {
        auto* specTopSubstrateThickness = GetSpecByName(resultTopSubstrateThickness->m_resultName);
        if (specTopSubstrateThickness != nullptr)
            specTopSubstrateThickness->m_use = FALSE;
    }

    auto* resultBottomSubstrateThickness
        = m_resultGroup.GetResultByName(g_szSide2DInspectionName[SIDE_INSPECTION_2D_BOTTOM_SUBSTRATE_THICKNESS]);
    if (resultBottomSubstrateThickness != nullptr)
    {
        auto* specBottomSubstrateThickness = GetSpecByName(resultBottomSubstrateThickness->m_resultName);
        if (specBottomSubstrateThickness != nullptr)
            specBottomSubstrateThickness->m_use = FALSE;
    }

    auto* resultGlassThickness
        = m_resultGroup.GetResultByName(g_szSide2DInspectionName[SIDE_INSPECTION_2D_GLASS_THICKNESS]);
    if (resultGlassThickness != nullptr)
    {
        auto* specGlassThickness = GetSpecByName(resultGlassThickness->m_resultName);
        if (specGlassThickness != nullptr)
            specGlassThickness->m_use = FALSE;
    }
}

BOOL VisionInspectionSide2D::VerifyNormalUnitInspection(const bool detailSetupMode, const CString strInspItemName)
{
    const auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;
    const bool bIsGlassCoreModel = bodyInfoMaster->m_bGlassCorePackage;
    if (bIsGlassCoreModel == false)
    {
        if (detailSetupMode == true)
        {
            CString strMsg;
            strMsg.Format(_T("Can not inspect [%s].\nThis unit is not [Glass Core]."), (LPCTSTR)strInspItemName);
            ::SimpleMessage(strMsg);
        }

        return FALSE;
    }

    return TRUE;
}