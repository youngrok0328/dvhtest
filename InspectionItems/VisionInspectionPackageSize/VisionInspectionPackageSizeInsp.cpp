//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionPackageSize.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionPackageSize.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionPackageSize::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionPackageSize::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    return bInsp;
}

BOOL VisionInspectionPackageSize::DoInsp(const bool detailSetupMode)
{
    BOOL bModuleInsp = false;
    for (auto& spec : m_fixedInspectionSpecs)
        bModuleInsp |= spec.m_use;

    if (!bModuleInsp)
        return TRUE;

    BOOL bInspResult = TRUE;

    int32_t nleft = (int32_t)(min(m_sEdgeAlignResult->fptLT.m_x, m_sEdgeAlignResult->fptLB.m_x));
    int32_t ntop = (int32_t)(min(m_sEdgeAlignResult->fptLT.m_y, m_sEdgeAlignResult->fptRT.m_y));
    int32_t nright = (int32_t)(max(m_sEdgeAlignResult->fptRT.m_x, m_sEdgeAlignResult->fptRB.m_x));
    int32_t nbottom = (int32_t)(max(m_sEdgeAlignResult->fptLB.m_y, m_sEdgeAlignResult->fptRB.m_y));

    Ipvm::Rect32s rtObject(nleft, ntop, nright, nbottom);

    bInspResult &= InspBodySizeX(rtObject);
    bInspResult &= InspBodySizeY(rtObject);
    bInspResult &= InspParallelism(rtObject);
    bInspResult &= InspOrthogonality(rtObject);

    UNREFERENCED_PARAMETER(detailSetupMode);

    if (!bInspResult)
    {
        m_bInvalid = TRUE;
    }

    return bInspResult;
}

void VisionInspectionPackageSize::SetDebugInfo(const bool detailSetupMode)
{
    if (detailSetupMode)
    {
    }
}

BOOL VisionInspectionPackageSize::InspBodySizeX(const Ipvm::Rect32s& i_rtObject)
{
    auto* result = m_resultGroup.GetResultByName(g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_BODYSIZE_X]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionPackageSize::InspBodySizeX() - result is nullptr"));
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionPackageSize::InspBodySizeX() - spec is nullptr"));
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    Ipvm::Point32r2 leftPoint;
    Ipvm::Point32r2 rightPoint;

    leftPoint.m_x = (m_sEdgeAlignResult->fptLT.m_x + m_sEdgeAlignResult->fptLB.m_x) * 0.5f;
    leftPoint.m_y = (m_sEdgeAlignResult->fptLT.m_y + m_sEdgeAlignResult->fptLB.m_y) * 0.5f;

    rightPoint.m_x = (m_sEdgeAlignResult->fptRT.m_x + m_sEdgeAlignResult->fptRB.m_x) * 0.5f;
    rightPoint.m_y = (m_sEdgeAlignResult->fptRT.m_y + m_sEdgeAlignResult->fptRB.m_y) * 0.5f;

    float fSpecBodySizeX = m_packageSpec.m_bodyInfoMaster->fBodySizeX;

    float distance = 0.f;
    Ipvm::Geometry::GetDistance(leftPoint, rightPoint, distance);

    float scale = (getScale().pixelToUm().m_x + getScale().pixelToUm().m_y) * 0.5f;
    float fError = (distance * scale) - fSpecBodySizeX;

    result->Resize(1);
    result->SetRect(0, i_rtObject);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("0"), _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecBodySizeX);
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionPackageSize::InspBodySizeY(const Ipvm::Rect32s& i_rtObject)
{
    auto* result = m_resultGroup.GetResultByName(g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_BODYSIZE_Y]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionPackageSize::InspBodySizeY() - result is nullptr"));
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionPackageSize::InspBodySizeY() - spec is nullptr"));
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    Ipvm::Point32r2 topPoint;
    Ipvm::Point32r2 bottomPoint;

    topPoint.m_x = (m_sEdgeAlignResult->fptLT.m_x + m_sEdgeAlignResult->fptRT.m_x) * 0.5f;
    topPoint.m_y = (m_sEdgeAlignResult->fptLT.m_y + m_sEdgeAlignResult->fptRT.m_y) * 0.5f;

    bottomPoint.m_x = (m_sEdgeAlignResult->fptLB.m_x + m_sEdgeAlignResult->fptRB.m_x) * 0.5f;
    bottomPoint.m_y = (m_sEdgeAlignResult->fptLB.m_y + m_sEdgeAlignResult->fptRB.m_y) * 0.5f;

    float fSpecBodySizeY = m_packageSpec.m_bodyInfoMaster->fBodySizeY;

    float distance = 0.f;
    Ipvm::Geometry::GetDistance(topPoint, bottomPoint, distance);

    float scale = (getScale().pixelToUm().m_x + getScale().pixelToUm().m_y) * 0.5f;
    float fError = (distance * scale) - fSpecBodySizeY;

    result->Resize(1);
    result->SetRect(0, i_rtObject);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("0"), _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecBodySizeY);
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionPackageSize::InspParallelism(const Ipvm::Rect32s& i_rtObject)
{
    auto* result = m_resultGroup.GetResultByName(g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_PARALLELISM]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionPackageSize::InspParallelism() - result is nullptr"));
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionPackageSize::InspParallelism() - spec is nullptr"));
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    const auto& vecfParallelismDistLeft = m_sEdgeAlignResult->vecfParallelismDistLeft;
    const auto& vecfParallelismDistRight = m_sEdgeAlignResult->vecfParallelismDistRight;
    const auto& vecfParallelismDistTop = m_sEdgeAlignResult->vecfParallelismDistTop;
    const auto& vecfParallelismDistBottom = m_sEdgeAlignResult->vecfParallelismDistBottom;

    float fWorst = 0.f;

    // LEFT
    for (auto& dist : vecfParallelismDistLeft)
    {
        fWorst = fabs(dist) > fabs(fWorst) ? dist : fWorst;
    }

    // RIGHT
    for (auto& dist : vecfParallelismDistRight)
    {
        fWorst = fabs(dist) > fabs(fWorst) ? dist : fWorst;
    }

    // TOP
    for (auto& dist : vecfParallelismDistTop)
    {
        fWorst = fabs(dist) > fabs(fWorst) ? dist : fWorst;
    }

    // BOTTOM
    for (auto& dist : vecfParallelismDistBottom)
    {
        fWorst = fabs(dist) > fabs(fWorst) ? dist : fWorst;
    }

    float scale = (getScale().pixelToUm().m_x + getScale().pixelToUm().m_y) * 0.5f;
    float fError = fWorst * scale;

    result->Resize(1);
    result->SetRect(0, i_rtObject);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), fError, *spec, 0.f, 0.f);
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionPackageSize::InspOrthogonality(const Ipvm::Rect32s& i_rtObject)
{
    auto* result = m_resultGroup.GetResultByName(g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_ORTHOGONALITY]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionPackageSize::InspOrthogonality() - result is nullptr"));
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionPackageSize::InspOrthogonality() - spec is nullptr"));
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    Ipvm::Point32r2 ptLeftTop = m_sEdgeAlignResult->fptLT;
    Ipvm::Point32r2 ptLeftBottom = m_sEdgeAlignResult->fptLB;
    Ipvm::Point32r2 ptRightTop = m_sEdgeAlignResult->fptRT;
    Ipvm::Point32r2 ptRightBottom = m_sEdgeAlignResult->fptRB;
    Ipvm::Point32r2 ptLowtopLT = m_sEdgeAlignResult->fptLowtopLT;
    Ipvm::Point32r2 ptLowtopRT = m_sEdgeAlignResult->fptLowtopRT;

    float fError = 0.f;

    float fBodyAngleLT = CPI_Geometry::GetAngleBetween2Lines(ptLeftBottom, ptLeftTop, ptRightTop);
    float fBodyAngleRT = CPI_Geometry::GetAngleBetween2Lines(ptLeftTop, ptRightTop, ptRightBottom);
    float fBodyAngleLB = CPI_Geometry::GetAngleBetween2Lines(ptRightBottom, ptLeftBottom, ptLeftTop);
    float fBodyAngleRB = CPI_Geometry::GetAngleBetween2Lines(ptRightTop, ptRightBottom, ptLeftBottom);

    if (m_packageSpec.m_bLowtopEdge)
    {
        fBodyAngleLT = CPI_Geometry::GetAngleBetween2Lines(ptLeftBottom, ptLowtopLT, ptLowtopRT);
    }

    float fWorst = 0.f;
    fWorst = fabs(fBodyAngleLT - 90.f) > fabs(fWorst) ? fBodyAngleLT - 90.f : fWorst;
    fWorst = fabs(fBodyAngleRT - 90.f) > fabs(fWorst) ? fBodyAngleRT - 90.f : fWorst;
    fWorst = fabs(fBodyAngleLB - 90.f) > fabs(fWorst) ? fBodyAngleLB - 90.f : fWorst;
    fWorst = fabs(fBodyAngleRB - 90.f) > fabs(fWorst) ? fBodyAngleRB - 90.f : fWorst;

    // 각도에 왜 스케일값을 곱함? 지움. 2018.10.11
    //float scale = (getScale().pixelToUm().m_x + getScale().pixelToUm().m_y) * 0.5f;
    fError = fWorst; // *scale;

    result->Resize(1);
    result->SetRect(0, i_rtObject);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), fError, *spec, 0.f, 0.f);
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}
