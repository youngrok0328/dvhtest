//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionInPocketTR.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionInPocketTR.h"
#include "VisionInspectionInPocketTRPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/SimpleFunction.h"
#include "../../DefineModules/dA_Base/SprocketHoleInfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTapeSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionInPocketTR::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionInPocketTR::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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

    BOOL bInsp = DoInsp(detailSetupMode);

    // Debug Info...
    SetDebugInfo(detailSetupMode);

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    return bInsp;
}

BOOL VisionInspectionInPocketTR::DoInsp(const bool detailSetupMode)
{
    BOOL bModuleInsp = false;
    for (auto& spec : m_fixedInspectionSpecs)
        bModuleInsp |= spec.m_use;

    if (!bModuleInsp)
        return TRUE;

    auto processor = m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_IN_POCKET_TR);
    if (processor == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::DoInsp() - processor is nullptr"));
        return FALSE;
    }

    //실제 사용할 영상은 아니고 영상의 Center를 구하기 위한 용도이다.
    Ipvm::Image8u image = GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return FALSE;

    //검사에 사용할 각종 정보를 DebugInfo에서 가져온다
    if (CollectAlignInfoInDebugInfo(Ipvm::Point32r2((float)image.GetSizeX() * 0.5f, (float)image.GetSizeY() * 0.5f))
        == false)
        return FALSE;

    //Device의 QRect를 Rect로 변환 후 Orientation Reject ROI로 사용한다.
    Ipvm::Rect32s rtDevice = Ipvm::Conversion::ToRect32s(m_qrDevice);

    bool bInspResult = true;
    bInspResult &= InspInPocketEmptyStatus(detailSetupMode, m_rtPocket);
    bInspResult &= InspInPocketSprocketHoleWidth(detailSetupMode);
    bInspResult &= InspInPocketSprocketHoleDamage(detailSetupMode);
    bInspResult &= InspInPocketMisplace(detailSetupMode, m_rtPocket);
    bInspResult &= InspInPocketMisplaceAngle(detailSetupMode, rtDevice);
    bInspResult &= InspInPocketDeviceOrientation(detailSetupMode, rtDevice);

    if (!bInspResult)
    {
        m_bInvalid = TRUE;
    }

    return bInspResult;
}

void VisionInspectionInPocketTR::SetDebugInfo(const bool detailSetupMode)
{
    if (detailSetupMode)
    {
    }
}

bool VisionInspectionInPocketTR::InspInPocketEmptyStatus(const bool detailSetupMode, const Ipvm::Rect32s& i_rtPocket)
{
    auto* result = m_resultGroup.GetResultByName(g_szInPocketInspectionName[INPOCKET_INSPECTION_EMPTY_STATUS]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketEmptyPocket() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketEmptyPocket() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    const bool isReceivedFillPocket = m_receivedFillPocket; //Handler로부터 받은 Fill Pocket 여부
    const bool isAlignedFillPocket = m_alignedFillPocket; //Edge Align 결과로부터 받은 Fill Pocket 여부

    float fError = 0.f;
    long inspResult = NOT_MEASURED;

    if (isReceivedFillPocket == false && isAlignedFillPocket == false)
    { //Handler로부터 받은 Fill Pocket이 False이고 Edge Align 결과로도 False인 경우
        fError = 0.0f;
        inspResult = PASS; //비어 있다고 불량이 아니다.
    }
    else if (isReceivedFillPocket == false && isAlignedFillPocket == true)
    { //Handler로부터 받은 Fill Pocket이 False인데 Edge Align 결과로는 True인 경우
        fError = 0.1f;
        inspResult = REJECT;
    }
    else if (isReceivedFillPocket == true && isAlignedFillPocket == false)
    { //Handler로부터 받은 Fill Pocket이 True인데 Edge Align 결과로는 False인 경우
        fError = 1.0f;
        inspResult = REJECT;
    }
    else
    { //Handler로부터 받은 Fill Pocket이 True이고 Edge Align 결과로도 True인 경우
        fError = 1.1f; //둘 다 True인 경우
        inspResult = PASS;
    }

    result->Resize(1);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), fError, *spec, 0.f, 0.f, 0.f);
    result->SetResult(0, inspResult);
    result->SetRect(0, i_rtPocket);

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

bool VisionInspectionInPocketTR::InspInPocketSprocketHoleWidth(const bool detailSetupMode)
{ //Align된 Sprocket Hole의 Width를 검사한다.
    auto* result = m_resultGroup.GetResultByName(g_szInPocketInspectionName[INPOCKET_INSPECTION_SPROCKET_HOLE_WIDTH]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketSprocketHoleWidth() - result is nullptr"));
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketSprocketHoleWidth() - spec is nullptr"));
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return true;

    const VisionTapeSpec* tapeSpec = m_visionUnit.getTapeSpec();
    if (tapeSpec == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketSprocketHoleWidth() - tapeSpec is nullptr"));
        return false;
    }

    const float fSprocketHoleWidthUM = tapeSpec->m_d1 * 1000.f;

    const long leftSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole.size());
    const long rightSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole.size());

    const long nResultSize = leftSprocketHoleNum + rightSprocketHoleNum;
    result->Resize(nResultSize);

    const float scale = (getScale().pixelToUm().m_x + getScale().pixelToUm().m_y) * 0.5f;
    float radiusUM = 0.f;
    float error = 0.f;
    long resultIdx = 0;
    CString strIndex;
    Ipvm::Rect32s rtROI;

    for (long nLeft = 0; nLeft < leftSprocketHoleNum; nLeft++)
    {
        const auto& circle = m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole[nLeft];
        rtROI = Ipvm::Conversion::ToRect32s(circle);
        strIndex.Format(_T("L%s"), (LPCTSTR)SimpleFunction::GetStringCount(nLeft + 1));

        radiusUM = circle.m_radius * 2.f * scale;
        error = fSprocketHoleWidthUM - radiusUM;

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nLeft, strIndex, _T(""), _T(""), error, *spec, 0.f, 0.f, fSprocketHoleWidthUM);
        result->SetRect(nLeft, rtROI);
    }

    for (long nRight = 0; nRight < rightSprocketHoleNum; nRight++)
    {
        const auto& circle = m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole[nRight];
        rtROI = Ipvm::Conversion::ToRect32s(circle);
        strIndex.Format(_T("R%s"), (LPCTSTR)SimpleFunction::GetStringCount(nRight + 1));
        resultIdx = leftSprocketHoleNum + nRight;

        radiusUM = circle.m_radius * 2.f * scale;
        error = fSprocketHoleWidthUM - radiusUM;

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            resultIdx, strIndex, _T(""), _T(""), error, *spec, 0.f, 0.f, fSprocketHoleWidthUM);
        result->SetRect(resultIdx, rtROI);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

bool VisionInspectionInPocketTR::InspInPocketSprocketHoleDamage(const bool detailSetupMode)
{ //Aling 정보 중 Edge Point와 Circle의 Center 간의 거리를 확인하여 Sprocket Hole의 손상을 검사한다.
    auto* result = m_resultGroup.GetResultByName(g_szInPocketInspectionName[INPOCKET_INSPECTION_SPROCKET_HOLE_DAMAGE]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketSprocketHoleDamage() - result is nullptr"));
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketSprocketHoleDamage() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    const VisionTapeSpec* tapeSpec = m_visionUnit.getTapeSpec();
    if (tapeSpec == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketSprocketHoleWidth() - tapeSpec is nullptr"));
        return false;
    }

    const float fSprocketHoleRadiusUM = tapeSpec->m_d1 * .5f * 1000.f;

    const long leftSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole.size());
    const long rightSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole.size());

    const long nResultSize = leftSprocketHoleNum + rightSprocketHoleNum;
    result->Resize(nResultSize);

    const float scale = (getScale().pixelToUm().m_x + getScale().pixelToUm().m_y) * 0.5f;
    float radiusUM = 0.f;
    float fDistance = 0.f;
    float fMaxDistance = 0.f;
    float error = 0.f;
    long nMaxIdx = 0;
    long resultIdx = 0;
    long nEdgePointNum = 0;
    CString strIndex;
    Ipvm::Rect32s rtROI;

    for (long nLeft = 0; nLeft < leftSprocketHoleNum; nLeft++)
    {
        const auto& circle = m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole[nLeft];
        strIndex.Format(_T("L%s"), (LPCTSTR)SimpleFunction::GetStringCount(nLeft + 1));

        nEdgePointNum = static_cast<long>(m_sprocketHoleInfo->m_vec2PointLeftSprocketHole[nLeft].size());
        fMaxDistance = 0.f;
        for (long nEdge = 0; nEdge < nEdgePointNum; nEdge++)
        {
            const auto& edgePoint = m_sprocketHoleInfo->m_vec2PointLeftSprocketHole[nLeft][nEdge];
            fDistance = 0.f;
            Ipvm::Geometry::GetDistance(edgePoint, Ipvm::Point32r2(circle.m_x, circle.m_y), fDistance);

            if (fDistance > fMaxDistance)
            {
                fMaxDistance = fDistance;
                nMaxIdx = nEdge;
            }
        }

        const auto& edgePoint = m_sprocketHoleInfo->m_vec2PointLeftSprocketHole[nLeft][nMaxIdx];
        rtROI.SetRect(static_cast<long>(edgePoint.m_x), static_cast<long>(edgePoint.m_y), static_cast<long>(circle.m_x),
            static_cast<long>(circle.m_y));
        rtROI.NormalizeRect();
        radiusUM = fMaxDistance * scale;
        error = radiusUM - fSprocketHoleRadiusUM;
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nLeft, strIndex, _T(""), _T(""), error, *spec, 0.f, 0.f, fSprocketHoleRadiusUM);
        result->SetRect(nLeft, rtROI);
    }

    for (long nRight = 0; nRight < rightSprocketHoleNum; nRight++)
    {
        const auto& circle = m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole[nRight];
        strIndex.Format(_T("R%s"), (LPCTSTR)SimpleFunction::GetStringCount(nRight + 1));
        nEdgePointNum = static_cast<long>(m_sprocketHoleInfo->m_vec2PointRightSprocketHole[nRight].size());
        fMaxDistance = 0.f;

        for (long nEdge = 0; nEdge < nEdgePointNum; nEdge++)
        {
            const auto& edgePoint = m_sprocketHoleInfo->m_vec2PointRightSprocketHole[nRight][nEdge];
            fDistance = 0.f;
            Ipvm::Geometry::GetDistance(edgePoint, Ipvm::Point32r2(circle.m_x, circle.m_y), fDistance);
            if (fDistance > fMaxDistance)
            {
                fMaxDistance = fDistance;
                nMaxIdx = nEdge;
            }
        }
        const auto& edgePoint = m_sprocketHoleInfo->m_vec2PointRightSprocketHole[nRight][nMaxIdx];
        rtROI.SetRect(static_cast<long>(edgePoint.m_x), static_cast<long>(edgePoint.m_y), static_cast<long>(circle.m_x),
            static_cast<long>(circle.m_y));
        rtROI.NormalizeRect();
        radiusUM = fMaxDistance * scale;
        error = radiusUM - fSprocketHoleRadiusUM;
        resultIdx = leftSprocketHoleNum + nRight;
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            resultIdx, strIndex, _T(""), _T(""), error, *spec, 0.f, 0.f, fSprocketHoleRadiusUM);
        result->SetRect(resultIdx, rtROI);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

bool VisionInspectionInPocketTR::InspInPocketMisplace(const bool detailSetupMode, const Ipvm::Rect32s& i_rtPocket)
{
    auto* result = m_resultGroup.GetResultByName(g_szInPocketInspectionName[INPOCKET_INSPECTION_MISPLACE]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketMisplace() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketMisplace() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    result->Resize(1);

    if (m_alignedFillPocket == false)
    {
        // Device Align 결과가 없으면(Empty) 검사하지 않는다.
        result->SetRect(0, i_rtPocket);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), 0.f, *spec, 0.f, 0.f);
        result->SetResult(0, PASS);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
        return true;
    }

    // 검사에 사용할 영상 준비
    auto& memory = getReusableMemory();
    const auto& misplaceFrameIdx = m_VisionPara->m_calcFrameIndexMisPlace;
    auto& misplaceImageProcPara = m_VisionPara->m_ImageProcManageParaMisPlace;

    Ipvm::Image8u image = misplaceFrameIdx.getImage(false);
    Ipvm::Image8u combineImage;
    if (!memory.GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
        return false;
    if (combineImage.GetMem() == nullptr)
        return false;

    // imagecombine 해서 이미지를 불러온다.
    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, true, m_VisionPara->m_ImageProcManageParaMisPlace, combineImage))
    {
        return false;
    }

    //검사 알고리즘 부

    //검사 결과 수집부
    float fError = 0.f;

    result->SetRect(0, i_rtPocket);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), fError, *spec, 0.f, 0.f);
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

bool VisionInspectionInPocketTR::InspInPocketMisplaceAngle(const bool detailSetupMode, const Ipvm::Rect32s& i_rtPocket)
{
    auto* result = m_resultGroup.GetResultByName(g_szInPocketInspectionName[INPOCKET_INSPECTION_MISPLACE_ANGLE]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketMisplace() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketMisplace() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    result->Resize(1);

    if (m_alignedFillPocket == false)
    {
        // Device Align 결과가 없으면(Empty) 검사하지 않는다.
        result->SetRect(0, i_rtPocket);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), 0.f, *spec, 0.f, 0.f);
        result->SetResult(0, PASS);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
        return true;
    }

    //검사 알고리즘 부분. m_qrDevice 상하 2변의 평균 각도를 구해서 불량 여부를 판단한다.
    float fError = 0.f;
    std::vector<float> vecAngles(2);

    CPI_Geometry::Get1LineAngle(m_qrDevice.m_ltX, m_qrDevice.m_ltY, m_qrDevice.m_lbX, m_qrDevice.m_lbY, vecAngles[0]);
    CPI_Geometry::Get1LineAngle(m_qrDevice.m_rtX, m_qrDevice.m_rtY, m_qrDevice.m_rbX, m_qrDevice.m_rbY, vecAngles[1]);

    fError = (vecAngles[0] + vecAngles[1]) * 0.5f; //상하 2변의 평균 각도를 구한다.

    result->SetRect(0, i_rtPocket);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), fError, *spec, 0.f, 0.f);
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

bool VisionInspectionInPocketTR::InspInPocketDeviceOrientation(
    const bool detailSetupMode, const Ipvm::Rect32s& i_rtDevice)
{
    auto* result = m_resultGroup.GetResultByName(g_szInPocketInspectionName[INPOCKET_INSPECTION_DEVICE_ORIENTATION]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketDeviceOrientation() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionInPocketTR::InspInPocketDeviceOrientation() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    if (m_alignedFillPocket == false)
    {
        // Device Align 결과가 없으면(Empty) 검사하지 않는다.
        result->Resize(1);
        result->SetRect(0, i_rtDevice);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), 0.f, *spec, 0.f, 0.f);
        result->SetResult(0, PASS);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
        return true;
    }

    // 검사에 사용할 영상 준비
    auto& memory = getReusableMemory();
    const auto& misplaceFrameIdx = m_VisionPara->m_calcFrameIndexMisPlace;
    auto& misplaceImageProcPara = m_VisionPara->m_ImageProcManageParaMisPlace;

    Ipvm::Image8u image = misplaceFrameIdx.getImage(false);
    Ipvm::Image8u combineImage;
    if (!memory.GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
        return false;
    if (combineImage.GetMem() == nullptr)
        return false;

    // imagecombine 해서 이미지를 불러온다.
    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, true, m_VisionPara->m_ImageProcManageParaMisPlace, combineImage))
    {
        return false;
    }

    //검사 알고리즘 부

    //검사 결과 수집부
    float fError = 0.f;

    result->SetRect(0, i_rtDevice);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), fError, *spec, 0.f, 0.f);
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

void VisionInspectionInPocketTR::SetReceivedFillPocketStatus(const bool& receivedFillPocket)
{
    m_receivedFillPocket = receivedFillPocket;
}

bool VisionInspectionInPocketTR::GetReceivedFillPocketStatus() const
{
    return m_receivedFillPocket;
}

bool VisionInspectionInPocketTR::CollectAlignInfoInDebugInfo(const Ipvm::Point32r2& pointImageCenter)
{
    if (m_sprocketHoleInfo == nullptr)
    {
        m_sprocketHoleInfo = new SprocketHoleInfo;
    }

    Ipvm::EllipseEq32r* pEllipseEqAll = nullptr;
    long circleNum = 0;
    Ipvm::Point32r2* pEdgePointAll = nullptr;
    long pointNum = 0;
    m_rtPocket.SetRect(0, 0, 0, 0);
    m_qrDevice = Ipvm::Quadrangle32r{-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};
    m_alignedFillPocket = false; // Detail Tape Align에서 확인한 Fill Pocket 여부 초기화

    const long needDataNum
        = 4; // Sprocket Hole Circle, Edge Point, Pocket Rect, Device QuadRect //[중요]-필요한 정보를 다 읽었는지 확인하기 위한 변수다,
    long collectedDataNum = 0;

    auto visionDebugInfos = m_visionUnit.GetVisionDebugInfos();
    long nDebugInfoSize = (long)visionDebugInfos.size();
    long nDataNum = 0;
    for (long i = 0; i < nDebugInfoSize; i++)
    {
        auto* debugInfo = visionDebugInfos[i];
        if (debugInfo == nullptr)
            continue;
        if (debugInfo->pData == nullptr)
            continue;
        if (debugInfo->nDataNum <= 0)
            continue;

        nDataNum = 0;

        if (debugInfo->moduleGuid == _VISION_INSP_GUID_TAPE_DETAIL_ALIGN)
        {
            if (debugInfo->strDebugInfoName == _DEBUG_INFO_TDA_SPROCKET_HOLE_CIRCLE)
            {
                circleNum = (long)debugInfo->nDataNum;
                if (circleNum <= 0 || debugInfo->pData == nullptr)
                    continue;

                pEllipseEqAll = (Ipvm::EllipseEq32r*)debugInfo->pData;
                collectedDataNum++;
            }
            else if (debugInfo->strDebugInfoName == _DEBUG_INFO_TDA_SPROCKET_HOLE_EDGE_POINT)
            {
                pointNum = (long)debugInfo->nDataNum;
                if (pointNum <= 0 || debugInfo->pData == nullptr)
                    continue;

                pEdgePointAll = (Ipvm::Point32r2*)debugInfo->pData;
                collectedDataNum++;
            }
            else if (debugInfo->strDebugInfoName == _DEBUG_INFO_TDA_POCKET_RECT)
            {
                if (debugInfo->pData != nullptr && debugInfo->nDataNum == 1)
                {
                    Ipvm::Rect32r rtPocket = *(Ipvm::Rect32r*)debugInfo->pData;
                    m_rtPocket = Ipvm::Rect(
                        static_cast<long>(rtPocket.m_left + .5f), static_cast<long>(rtPocket.m_top + .5f), 
                        static_cast<long>(rtPocket.m_right + .5f), static_cast<long>(rtPocket.m_bottom + .5f));
                    collectedDataNum++;
                }
            }
            else if (debugInfo->strDebugInfoName == _DEBUG_INFO_TDA_DEVICE_ALIGN_QRECT)
            {
                if (debugInfo->pData != nullptr && debugInfo->nDataNum == 1)
                {
                    m_qrDevice = *(Ipvm::Quadrangle32r*)debugInfo->pData;
                    collectedDataNum++;
                    m_alignedFillPocket = true; //Detail Tape Align에서 Fill Pocket이 Align되었음을 확인한다.
                }
            }

            if (collectedDataNum == needDataNum)
            {
                //모든 정보가 수집되면 break 한다.
                break;
            }
        }
    }

    bool res = m_sprocketHoleInfo->CollectSprocketHoleInfo(
        pEllipseEqAll, circleNum, pEdgePointAll, pointNum, pointImageCenter, getScale().pixelToUm());

    if (collectedDataNum != needDataNum)
    {
        //필요한 정보가 모두 수집되지 않았다면 초기화 한다. 단 Device Align은 제외
        if (collectedDataNum + 1 == needDataNum && m_alignedFillPocket == false)
        {
        } //Device Align 정보가 수집되지 않았지만 Fill Pocket 정보는 수신되었으므로 true를 반환한다.
        else
        {
            m_sprocketHoleInfo->Init();
            res = false;
            m_rtPocket.SetRect(0, 0, 0, 0); // Pocket Rect 초기화
            m_qrDevice = Ipvm::Quadrangle32r{-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f}; // Device QuadRect 초기화
            m_alignedFillPocket = false; // Detail Tape Align에서 확인한 Fill Pocket 여부 초기화
        }
    }

    return res;
}

void VisionInspectionInPocketTR::SetFillPocketInfoInImageLot(bool receivedFillPocket)
{
    static const bool isHWExist = SystemConfig::GetInstance().IsHardwareExist();
    if (isHWExist == true)
        return;

    // Image Lot에 Fill Pocket 정보를 설정한다.
    auto& imageLot = getImageLot();
    imageLot.SetFillPocketTR(receivedFillPocket);
}