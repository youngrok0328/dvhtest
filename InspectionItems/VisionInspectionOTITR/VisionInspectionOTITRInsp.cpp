//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionOTITR.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionOTITR.h"
#include "VisionInspectionOTITRPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/SimpleFunction.h"
#include "../../DefineModules/dA_Base/SprocketHoleInfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTapeSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/EdgeDetection.h>
#include <Ipvm/Algorithm/EdgeDetectionFilter.h>
#include <Ipvm/Algorithm/EdgeDetectionPara.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/TimeCheck.h>
#include <Ipvm/Base/LineSeg32r.h>

//CPP_5_________________________________ Standard library headers
#include <math.h>
 
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionOTITR::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionOTITR::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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

BOOL VisionInspectionOTITR::DoInsp(const bool detailSetupMode)
{
    BOOL bModuleInsp = false;
    for (auto& spec : m_fixedInspectionSpecs)
        bModuleInsp |= spec.m_use;

    if (!bModuleInsp)
        return TRUE;

    auto processor = m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_OTI_TR);
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

    const bool needCoverTapeAlign = GetCheckCoverTapeAlign(); //Cover Tape 검사와 Sealing 검사 중 하나라도 하면 Align을 해야 한다.
    const bool needSealingAlign = GetCheckSealingAlign(); //Sealing 검사 중 하나라도 하면 Align을 해야 한다.

    //Sprocket Hole의 Center 정보를 구한다. Align에 필요하다.
    Ipvm::Point32r2 leftHolesCenter;
    Ipvm::Point32r2 rightHolesCenter;
    if ((needCoverTapeAlign || needSealingAlign) && GetSprocketHoleCenter(leftHolesCenter, rightHolesCenter) == false)
        return FALSE;

    //Cover Tape Align
    Ipvm::LineEq32r coverTapeLeftLine{};
    Ipvm::LineEq32r coverTapeRightLine{};
    Ipvm::Quadrangle32r qrCoverTape{};
    Ipvm::Rect32s coverTapeMergedSearchROI{};

    bool succeedCoverTapeAlign = false;
    if (needCoverTapeAlign)
    {
        succeedCoverTapeAlign = AlignCoverTape(detailSetupMode, leftHolesCenter, rightHolesCenter, coverTapeLeftLine,
            coverTapeRightLine, qrCoverTape, coverTapeMergedSearchROI);
    }

    //Sealing Align
    std::vector<Ipvm::Point32r2> vecLeftSealingLeft(0);
    std::vector<Ipvm::Point32r2> vecLeftSealingRight(0);
    std::vector<Ipvm::Point32r2> vecRightSealingLeft(0);
    std::vector<Ipvm::Point32r2> vecRightSealingRight(0);
    std::vector<Ipvm::LineEq32r> leftSealingLine(0);
    std::vector<Ipvm::LineEq32r> rightSealingLine(0);
    std::vector<Ipvm::Quadrangle32r> sealingArea(0);
    Ipvm::Rect32s sealingMergedSearchROI{};

    bool succeedSealingAlign = false;
    if (needSealingAlign)
    {
        succeedSealingAlign
            = AlignSealing(detailSetupMode, leftHolesCenter, rightHolesCenter, 
                vecLeftSealingLeft, vecLeftSealingRight, vecRightSealingLeft, vecRightSealingRight, leftSealingLine, rightSealingLine,
                sealingArea, sealingMergedSearchROI);
    }

    bool bInspResult = TRUE;       

    bInspResult &= InspOTI_EmptyStatus(detailSetupMode, m_rtPocket);

    bInspResult &= InspOTI_CoverTapeShift(detailSetupMode, succeedCoverTapeAlign, 
        coverTapeLeftLine, coverTapeRightLine, qrCoverTape, coverTapeMergedSearchROI);

    bInspResult &= InspOTI_SealingShift(detailSetupMode, succeedSealingAlign, sealingMergedSearchROI, 
        leftSealingLine, rightSealingLine);

    bInspResult &= InspOTI_SealingWidth(detailSetupMode, succeedSealingAlign, sealingMergedSearchROI,
        vecLeftSealingLeft, vecLeftSealingRight, vecRightSealingLeft, vecRightSealingRight, 
        leftSealingLine, rightSealingLine);

    bInspResult &= InspOTI_SealingSpan(
        detailSetupMode, succeedSealingAlign, sealingMergedSearchROI, leftSealingLine, rightSealingLine);

    //bInspResult &= InspOTI_SealingCutting(detailSetupMode, succeedSealingAlign, sealingMergedSearchROI);

    bInspResult &= InspOTI_SealingParallelism(
        detailSetupMode, succeedSealingAlign, sealingMergedSearchROI, sealingArea);

    if (!bInspResult)
    {
        m_bInvalid = TRUE;
    }

    return bInspResult;
}

void VisionInspectionOTITR::SetDebugInfo(const bool detailSetupMode)
{
    if (detailSetupMode)
    {
    }
}

bool VisionInspectionOTITR::InspOTI_EmptyStatus(const bool detailSetupMode, const Ipvm::Rect32s& i_rtPocket)
{
    auto* result = m_resultGroup.GetResultByName(g_szOTIInspectionName[OTI_INSPECTION_EMPTY_STATUS]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_EmptyPocket() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_EmptyPocket() - spec is nullptr"));
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

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

bool VisionInspectionOTITR::InspOTI_CoverTapeShift(const bool detailSetupMode, const bool& succeedCoverTapeAlign,
    const Ipvm::LineEq32r& coverTapeLeftLine, const Ipvm::LineEq32r& coverTapeRightLine,
    const Ipvm::Quadrangle32r& inspectionArea, const Ipvm::Rect32s& invalidRejROI)
{
    auto* result = m_resultGroup.GetResultByName(g_szOTIInspectionName[OTI_INSPECTION_COVER_TAPE_SHIFT]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspBodySizeY() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspBodySizeY() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    if (succeedCoverTapeAlign == false) //Align이 실패했으면 불량 처리한다.
    {
        result->Resize(1);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Align Failed"), _T(""), _T(""), -9999.f, *spec, 0.f, 0.f, 0.f);
        result->SetResult(0, REJECT);
        result->SetRect(0, invalidRejROI); 
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
        return true;
    }

    const VisionTapeSpec* tapeSpec = m_visionUnit.getTapeSpec();
    const float specDistanceSprocketHoleToCoverTape = tapeSpec->m_f1 * 1000.f; //um 단위
    const long specSprocketHoleHalfRad
        = static_cast<long>((tapeSpec->m_d1 * 1000.f * 0.25f) / getScale().pixelToUm().m_y + .5f);//RejROI를 만들기 위함이다.
    const float scaleX = getScale().pixelToUm().m_x;

    long leftSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole.size());
    long rightSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole.size());
    if (leftSprocketHoleNum < 1 && rightSprocketHoleNum < 1)
        return true;

    const long inspectionRangeStartY = static_cast<long>(inspectionArea.m_ltY + .5f);
    const long inspectionRangeEndY = static_cast<long>(inspectionArea.m_lbY + .5f);

    long resultNumLeft = 0;
    float fDist = 0.f;
    std::vector<float> vecDist(0);
    std::vector<Ipvm::Rect32s> vecrtROI(0);
    std::vector<CString> vecIndex(0);

    for (long idx = 0; idx < leftSprocketHoleNum; idx++)
    {
        const auto& circle = m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole[idx];
        if (circle.m_y < inspectionRangeStartY || circle.m_y > inspectionRangeEndY)
            continue;
        resultNumLeft++;

        Ipvm::Geometry::GetDistance(coverTapeLeftLine, Ipvm::Point32r2(circle.m_x, circle.m_y), fDist);
        vecDist.push_back(fDist * scaleX);
        vecrtROI.push_back(
            Ipvm::Rect32s(static_cast<long>(circle.m_x), static_cast<long>(circle.m_y - specSprocketHoleHalfRad),
            static_cast<long>(circle.m_x + fDist), static_cast<long>(circle.m_y + specSprocketHoleHalfRad)));
        vecIndex.push_back(_T("L") + SimpleFunction::GetStringCount(resultNumLeft + 1));
    }

    long resultNumRight = 0;
    for (long idx = 0; idx < rightSprocketHoleNum; idx++)
    {
        const auto& circle = m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole[idx];
        if (circle.m_y < inspectionRangeStartY || circle.m_y > inspectionRangeEndY)
            continue;
        resultNumRight++;
        Ipvm::Geometry::GetDistance(coverTapeRightLine, Ipvm::Point32r2(circle.m_x, circle.m_y), fDist);
        vecDist.push_back(fDist * scaleX);
        vecrtROI.push_back(Ipvm::Rect32s(static_cast<long>(circle.m_x - fDist),
            static_cast<long>(circle.m_y - specSprocketHoleHalfRad), static_cast<long>(circle.m_x),
            static_cast<long>(circle.m_y + specSprocketHoleHalfRad)));
        vecIndex.push_back(_T("R") + SimpleFunction::GetStringCount(resultNumRight + 1));
    }

    float fError = 0.f;
    long resultNum = resultNumLeft + resultNumRight;
    result->Resize(resultNum);

    long totalIdx = 0;
    for (long idx = 0; idx < resultNumLeft; idx++)
    {
        fError = vecDist[totalIdx] - specDistanceSprocketHoleToCoverTape;
        result->SetRect(totalIdx, vecrtROI[totalIdx]);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            totalIdx, vecIndex[totalIdx], _T(""), _T(""), fError, *spec, 0.f, 0.f, specDistanceSprocketHoleToCoverTape);
        totalIdx++;
    }
    for (long idx = 0; idx < resultNumRight; idx++)
    {
        fError = vecDist[totalIdx] - specDistanceSprocketHoleToCoverTape;
        result->SetRect(totalIdx, vecrtROI[totalIdx]);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            totalIdx, vecIndex[totalIdx], _T(""), _T(""), fError, *spec, 0.f, 0.f, specDistanceSprocketHoleToCoverTape);
        totalIdx++;
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

bool VisionInspectionOTITR::InspOTI_SealingShift(const bool detailSetupMode, const bool& succeedSealingAlign,
    const Ipvm::Rect32s& invalidRejROI, const std::vector<Ipvm::LineEq32r>& leftSealingLine,
    const std::vector<Ipvm::LineEq32r>& rightSealingLine)
{
    auto* result = m_resultGroup.GetResultByName(g_szOTIInspectionName[OTI_INSPECTION_SEALING_SHIFT]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingShift() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingShift() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    if (succeedSealingAlign == false) //Align이 실패했으면 불량 처리한다.
    {
        result->Resize(1);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Align Failed"), _T(""), _T(""), -9999.f, *spec, 0.f, 0.f, 0.f);
        result->SetResult(0, REJECT);
        result->SetRect(0, invalidRejROI);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
        return true;
    }

    const long leftSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole.size());
    const long rightSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole.size());
    const bool existLeftSprocketHole = leftSprocketHoleNum > 0 ? true : false;
    const bool existRightSprocketHole = rightSprocketHoleNum > 0 ? true : false;
    const long totalInspPoint = leftSprocketHoleNum + rightSprocketHoleNum;

    //각각의 좌우 실링 라인의 중심 Line을 구한다.
    Ipvm::LineEq32r leftSealingCenterLine{};
    Ipvm::LineEq32r rightSealingCenterLine{};

    Ipvm::Geometry::GetBisection(leftSealingLine[0], leftSealingLine[1], leftSealingCenterLine);
    Ipvm::Geometry::GetBisection(rightSealingLine[0], rightSealingLine[1], rightSealingCenterLine);

    //각 Sprocket Hole의 Center에서 좌우 실링 라인까지의 거리를 구한다.
    result->Resize(totalInspPoint);
    const VisionTapeSpec* tapeSpec = m_visionUnit.getTapeSpec();
    const float specDistanceSprocketHoleToSealing = tapeSpec->m_f2 * 1000.f; //um 단위
    const float scaleX = getScale().pixelToUm().m_x;

    long inspIdx = 0;
    float fDistPxl = 0.f;
    float fDist = 0.f;
    float fError = 0.f;
    Ipvm::Rect32s rtROI;

    //좌측 실링 검사하고
    for (long idx = 0; idx < leftSprocketHoleNum; idx++)
    {
        const auto& circle = m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole[idx];
        Ipvm::Geometry::GetDistance(leftSealingCenterLine, Ipvm::Point32r2(circle.m_x, circle.m_y), fDistPxl);
        fDist = fDistPxl * scaleX;
        fError = fDist - specDistanceSprocketHoleToSealing;
        rtROI = Ipvm::Rect32s(static_cast<long>(circle.m_x), static_cast<long>(circle.m_y - 5),
            static_cast<long>(circle.m_x + fDistPxl), static_cast<long>(circle.m_y + 5));
        result->SetRect(inspIdx, rtROI);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(inspIdx,
            _T("L") + SimpleFunction::GetStringCount(idx + 1), _T(""), _T(""), fError, *spec, 0.f, 0.f,
            specDistanceSprocketHoleToSealing);
        inspIdx++;
    }

    //우측 실링 검사한다.
    for (long idx = 0; idx < rightSprocketHoleNum; idx++)
    {
        const auto& circle = m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole[idx];
        Ipvm::Geometry::GetDistance(rightSealingCenterLine, Ipvm::Point32r2(circle.m_x, circle.m_y), fDistPxl);
        fDist = fDistPxl * scaleX;
        fError = fDist - specDistanceSprocketHoleToSealing;
        rtROI = Ipvm::Rect32s(static_cast<long>(circle.m_x - fDistPxl), static_cast<long>(circle.m_y - 5),
            static_cast<long>(circle.m_x), static_cast<long>(circle.m_y + 5));
        result->SetRect(inspIdx, rtROI);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(inspIdx,
            _T("R") + SimpleFunction::GetStringCount(idx + 1), _T(""), _T(""), fError, *spec, 0.f, 0.f,
            specDistanceSprocketHoleToSealing);
        inspIdx++;
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

bool VisionInspectionOTITR::InspOTI_SealingWidth(const bool detailSetupMode, const bool& succeedSealingAlign,
    const Ipvm::Rect32s& invalidRejROI, 
    const std::vector<Ipvm::Point32r2>& vecLeftSealingLeft,
    const std::vector<Ipvm::Point32r2>& vecLeftSealingRight, 
    const std::vector<Ipvm::Point32r2>& vecRightSealingLeft,
    const std::vector<Ipvm::Point32r2>& vecRightSealingRight, 
    const std::vector<Ipvm::LineEq32r>& leftSealingLine,
    const std::vector<Ipvm::LineEq32r>& rightSealingLine)
{
    auto* result = m_resultGroup.GetResultByName(g_szOTIInspectionName[OTI_INSPECTION_SEALING_WIDTH]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingWidth() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingWidth() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    const long leftSealingLeftPointNum = static_cast<long>(vecLeftSealingLeft.size());
    const long leftSealingRightPointNum = static_cast<long>(vecLeftSealingRight.size());
    const long rightSealingLeftPointNum = static_cast<long>(vecRightSealingLeft.size());
    const long rightSealingRightPointNum = static_cast<long>(vecRightSealingRight.size());

    //각 Point의 갯수가 같은지 확인
    bool validPointNum = true;
    if (leftSealingLeftPointNum != leftSealingRightPointNum || rightSealingLeftPointNum != rightSealingRightPointNum
        || leftSealingLeftPointNum != rightSealingRightPointNum)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingWidth() - Sprocket Hole Point Num is not same"));
        validPointNum = false;
    }
    if (succeedSealingAlign == false || validPointNum == false) //Align이 실패했거나 Point 개수가 유효하지 않으면 불량 처리한다.
    {
        result->Resize(1);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Align Failed"), _T(""), _T(""), -9999.f, *spec, 0.f, 0.f, 0.f);
        result->SetResult(0, REJECT);
        result->SetRect(0, invalidRejROI);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
        return true;
    }

    //Seaing Width는 Spec값이 없어 실측값을 그대로 결과에 사용
    const float scaleX = getScale().pixelToUm().m_x;
    long numInspPoint = leftSealingLeftPointNum + rightSealingLeftPointNum;
    float leftX = 0;
    float rightX = 0;
    float dist{};
    Ipvm::Rect32s rtROI{};
    Ipvm::LineEq32r rejectPositionLine{};
    std::vector<Ipvm::Point32r2> ptInv(2);

    result->Resize(numInspPoint);
    long inspIdx = 0;
    float nominal = (spec->m_passMin + spec->m_passMax) * .5f;

    //좌 Sealing 부터 하는데 좌우 포인트중 하나라도 x가 0 이하면 불량 처리 해야한다.
    for (long idx = 0; idx < leftSealingLeftPointNum; idx++)
    {
        if (vecLeftSealingLeft[idx].m_x <= 0.f || vecLeftSealingRight[idx].m_x <= 0.f)//좌우 Point 중 하나라도 검출에 실패한 경우
        {
            ptInv[0] = Ipvm::Point32r2(0, vecLeftSealingLeft[idx].m_y);
            ptInv[1] = Ipvm::Point32r2(10, vecLeftSealingRight[idx].m_y);
            Ipvm::DataFitting::FitToLine(2, &ptInv[0], rejectPositionLine);
            Ipvm::Geometry::GetCrossPoint(rejectPositionLine, leftSealingLine[0], ptInv[0]);
            Ipvm::Geometry::GetCrossPoint(rejectPositionLine, leftSealingLine[1], ptInv[1]);
            rtROI = Ipvm::Rect32s(static_cast<long>(ptInv[0].m_x), static_cast<long>(ptInv[0].m_y - 5),
                static_cast<long>(ptInv[1].m_x), static_cast<long>(ptInv[1].m_y + 5));

            result->SetRect(idx, rtROI);
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(idx,
                _T("L") + SimpleFunction::GetStringCount(idx + 1), _T(""), _T(""), -999.f, *spec, 0.f, 0.f, nominal);
            result->SetResult(idx, REJECT);

            inspIdx++;
            continue;
        }
        //정상적인 경우에는 좌우 Point의 거리를 Width로 사용한다.
        dist = static_cast<float>(fabs( (vecLeftSealingRight[idx].m_x - vecLeftSealingLeft[idx].m_x) * scaleX ));

        rtROI = Ipvm::Rect32s(static_cast<long>(vecLeftSealingLeft[idx].m_x),
            static_cast<long>(vecLeftSealingLeft[idx].m_y - 5), static_cast<long>(vecLeftSealingRight[idx].m_x),
            static_cast<long>(vecLeftSealingRight[idx].m_y + 5));
        result->SetRect(inspIdx, rtROI);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            inspIdx, _T("L") + SimpleFunction::GetStringCount(idx + 1), _T(""), _T(""), dist, *spec, 0.f, 0.f, nominal);
        inspIdx++;

    }
    
    //우 Sealing 검사
    for (long idx = 0; idx < rightSealingLeftPointNum; idx++)
    {
        if (vecRightSealingLeft[idx].m_x <= 0.f
            || vecRightSealingRight[idx].m_x <= 0.f) //좌우 Point 중 하나라도 검출에 실패한 경우
        {
            ptInv[0] = Ipvm::Point32r2(0, vecRightSealingLeft[idx].m_y);
            ptInv[1] = Ipvm::Point32r2(10, vecRightSealingRight[idx].m_y);
            Ipvm::DataFitting::FitToLine(2, &ptInv[0], rejectPositionLine);
            Ipvm::Geometry::GetCrossPoint(rejectPositionLine, rightSealingLine[0], ptInv[0]);
            Ipvm::Geometry::GetCrossPoint(rejectPositionLine, rightSealingLine[1], ptInv[1]);
            rtROI = Ipvm::Rect32s(static_cast<long>(ptInv[0].m_x), static_cast<long>(ptInv[0].m_y - 5),
                static_cast<long>(ptInv[1].m_x), static_cast<long>(ptInv[1].m_y + 5));
            result->SetRect(inspIdx, rtROI);
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(inspIdx,
                _T("R") + SimpleFunction::GetStringCount(idx + 1), _T(""), _T(""), -999.f, *spec, 0.f, 0.f, nominal);
            result->SetResult(inspIdx, REJECT);
            inspIdx++;
            continue;
        }
        //정상적인 경우에는 좌우 Point의 거리를 Width로 사용한다.
        dist = static_cast<float>(fabs((vecRightSealingRight[idx].m_x - vecRightSealingLeft[idx].m_x) * scaleX));
        rtROI = Ipvm::Rect32s(static_cast<long>(vecRightSealingLeft[idx].m_x),
            static_cast<long>(vecRightSealingLeft[idx].m_y - 5), static_cast<long>(vecRightSealingRight[idx].m_x),
            static_cast<long>(vecRightSealingRight[idx].m_y + 5));
        result->SetRect(inspIdx, rtROI);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            inspIdx, _T("R") + SimpleFunction::GetStringCount(idx + 1), _T(""), _T(""), dist, *spec, 0.f, 0.f, nominal);
        inspIdx++;
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

bool VisionInspectionOTITR::InspOTI_SealingSpan(
    const bool detailSetupMode, const bool& succeedSealingAlign, const Ipvm::Rect32s& invalidRejROI, 
    const std::vector<Ipvm::LineEq32r>& leftSealingLine, const std::vector<Ipvm::LineEq32r>& rightSealingLine)
{
    auto* result = m_resultGroup.GetResultByName(g_szOTIInspectionName[OTI_INSPECTION_SEALING_SPAN]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingSpan() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingSpan() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    const long leftSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole.size());
    const long rightSprocketHoleNum = static_cast<long>(m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole.size());
    const bool existLeftSprocketHole = leftSprocketHoleNum > 0 ? true : false;
    const bool existRightSprocketHole = rightSprocketHoleNum > 0 ? true : false;
    const bool notExistSprocketHole
        = (existLeftSprocketHole == false && existRightSprocketHole == false) ? true : false;
    const bool invlidSprocketHoleCount
        = (existLeftSprocketHole == true && existRightSprocketHole == true && leftSprocketHoleNum != rightSprocketHoleNum) ? true : false;

    if (succeedSealingAlign == false || notExistSprocketHole == true
        || invlidSprocketHoleCount == true) //Align이 실패했으면 불량 처리한다.
    {
        result->Resize(1);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("0"), _T(""), _T(""), -9999.f, *spec, 0.f, 0.f, 0.f);
        result->SetResult(0, REJECT);
        result->SetRect(0, invalidRejROI);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
        return true;
    }

    //좌우 실링 라인의 중간 Line을 구한다.
    Ipvm::LineEq32r leftCenterSealingLine{};
    Ipvm::LineEq32r rightCenterSealingLine{};
    Ipvm::Geometry::GetBisection(leftSealingLine[0], leftSealingLine[1], leftCenterSealingLine);
    Ipvm::Geometry::GetBisection(rightSealingLine[0], rightSealingLine[1], rightCenterSealingLine);

    //Sprocket Hole의 위치에서 실링 센터라인 간의 거리를 구한다.
    float fDist = 0.f;
    std::vector<float> vecDist(0);
    std::vector<Ipvm::Rect32s> vecrtROI(0);
    std::vector<CString> vecIndex(0);
    std::vector<Ipvm::Point32r2> ptCross(2);
    Ipvm::LineEq32r crossLine{};

    const VisionTapeSpec* tapeSpec = m_visionUnit.getTapeSpec();
    const float specSealingSpan = tapeSpec->m_s2 * 1000.f; //um 단위
    const float scaleX = getScale().pixelToUm().m_x;

    const long sprocketHoleNum = existLeftSprocketHole == true ? leftSprocketHoleNum : rightSprocketHoleNum;
    for (long idx = 0; idx < sprocketHoleNum; idx++)
    {
        GetCrossPointForSealingSpan(idx, existLeftSprocketHole, existRightSprocketHole, ptCross);
        Ipvm::DataFitting::FitToLine(2, &ptCross[0], crossLine);
        Ipvm::Geometry::GetCrossPoint(crossLine, leftCenterSealingLine, ptCross[0]);
        Ipvm::Geometry::GetCrossPoint(crossLine, rightCenterSealingLine, ptCross[1]);

        Ipvm::Geometry::GetDistance(ptCross[0], ptCross[1], fDist);
        vecDist.push_back(fDist * scaleX);
        vecrtROI.push_back(Ipvm::Rect32s(static_cast<long>(ptCross[0].m_x), static_cast<long>(ptCross[0].m_y - 5),
            static_cast<long>(ptCross[1].m_x), static_cast<long>(ptCross[1].m_y + 5)));
        vecIndex.push_back(SimpleFunction::GetStringCount(idx + 1));            
    }
  
    long resultNum = static_cast<long>(vecDist.size());
    result->Resize(resultNum);
    float fError = 0.f;
    for (long idx = 0; idx < resultNum; idx++)
    {
        result->SetRect(idx, vecrtROI[idx]);
        fError = vecDist[idx] - specSealingSpan;
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            idx, vecIndex[idx], _T(""), _T(""), fError, *spec, 0.f, 0.f, specSealingSpan);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true;
}

void VisionInspectionOTITR::GetCrossPointForSealingSpan(const long& sprocketHoleID, 
    const bool& existLeftHole, const bool& existRightHole,
    std::vector<Ipvm::Point32r2>& crossPoints)
{
    if (existLeftHole == true && existRightHole == true)
    {
        const auto& circleLeft = m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole[sprocketHoleID];
        const auto& circleRight = m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole[sprocketHoleID];
        crossPoints[0] = (Ipvm::Point32r2(circleLeft.m_x, circleLeft.m_y));
        crossPoints[1] = (Ipvm::Point32r2(circleRight.m_x, circleRight.m_y));
        return;
    }

    const bool isLeft = existLeftHole == true ? true : false;
    const auto& circle = isLeft == true ? m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole[sprocketHoleID]
                                        : m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole[sprocketHoleID];
    crossPoints[0] = (Ipvm::Point32r2(circle.m_x, circle.m_y));
    crossPoints[1] = (Ipvm::Point32r2(circle.m_x + 100.f, circle.m_y));
}

    //bool VisionInspectionOTITR::InspOTI_SealingCutting(
//    const bool detailSetupMode, const bool& succeedSealingAlign, const Ipvm::Rect32s& invalidRejROI)
//{
//    auto* result = m_resultGroup.GetResultByName(g_szOTIInspectionName[OTI_INSPECTION_SEALING_CUTTING]);
//    if (result == nullptr)
//    {
//        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingCutting() - result is nullptr"));
//        return false;
//    }
//    auto* spec = GetSpecByName(result->m_resultName);
//    if (spec == nullptr)
//    {
//        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingCutting() - spec is nullptr"));
//        return false;
//    }
//
//    if (spec->m_use == FALSE)
//        return true;
//
//    return true;
//}

bool VisionInspectionOTITR::InspOTI_SealingParallelism(
    const bool detailSetupMode, const bool& succeedSealingAlign, const Ipvm::Rect32s& invalidRejROI, 
    const std::vector<Ipvm::Quadrangle32r>& sealingAlignQrt)
{
    auto* result = m_resultGroup.GetResultByName(g_szOTIInspectionName[OIT_INSPECTION_SEALING_PARALLELISM]);
    if (result == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingParallelism() - result is nullptr"));
        return false;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::InspOTI_SealingParallelism() - spec is nullptr"));
        return false;
    }

    if (spec->m_use == FALSE)
        return true;

    long sealingNum = static_cast<long>(sealingAlignQrt.size());

    if (succeedSealingAlign == false || sealingNum != 2) //Align이 실패했으면 불량 처리한다.
    {
        result->Resize(1);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("0"), _T(""), _T(""), -9999.f, *spec, 0.f, 0.f, 0.f);
        result->SetResult(0, REJECT);
        result->SetRect(0, invalidRejROI);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
        return true;
    }

    //각도는 직관적이지 않아 좌우 실링 영역의 상단돠 하단 중심 사이의 거리를 구한 후 그 차이를 구한다.
    Ipvm::Point32r2 ptLeftTopCenter = Ipvm::Point32r2((sealingAlignQrt[0].m_ltX + sealingAlignQrt[0].m_rtX) * .5f,
        (sealingAlignQrt[0].m_ltY + sealingAlignQrt[0].m_rtY) * .5f);
    Ipvm::Point32r2 ptLeftBottomCenter = Ipvm::Point32r2((sealingAlignQrt[0].m_lbX + sealingAlignQrt[0].m_rbX) * .5f,
        (sealingAlignQrt[0].m_lbY + sealingAlignQrt[0].m_rbY) * .5f);
    Ipvm::Point32r2 ptRightTopCenter = Ipvm::Point32r2((sealingAlignQrt[1].m_ltX + sealingAlignQrt[1].m_rtX) * .5f,
        (sealingAlignQrt[1].m_ltY + sealingAlignQrt[1].m_rtY) * .5f);
    Ipvm::Point32r2 ptRightBottomCenter = Ipvm::Point32r2((sealingAlignQrt[1].m_lbX + sealingAlignQrt[1].m_rbX) * .5f,
        (sealingAlignQrt[1].m_lbY + sealingAlignQrt[1].m_rbY) * .5f);
    float distTop = 0.f;
    float distBottom = 0.f;
    Ipvm::Geometry::GetDistance(ptLeftTopCenter, ptRightTopCenter, distTop);
    Ipvm::Geometry::GetDistance(ptLeftBottomCenter, ptRightBottomCenter, distBottom);
    float fError = fabs(distTop - distBottom) * getScale().pixelToUm().m_x;
    Ipvm::Rect32s rtROI
        = Ipvm::Rect32s(static_cast<long>(ptLeftTopCenter.m_x), static_cast<long>(ptLeftTopCenter.m_y - 5),
            static_cast<long>(ptRightTopCenter.m_x), static_cast<long>(ptRightTopCenter.m_y + 5));
    result->Resize(1);
    result->SetRect(0, rtROI);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("Diff"), _T(""), _T(""), fError, *spec, 0.f, 0.f, 0.f);

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return true;
}
    
void VisionInspectionOTITR::SetReceivedFillPocketStatus(const bool& receivedFillPocket)
{
    m_receivedFillPocket = receivedFillPocket;
}

bool VisionInspectionOTITR::GetReceivedFillPocketStatus() const
{
    return m_receivedFillPocket;
}

bool VisionInspectionOTITR::CollectAlignInfoInDebugInfo(const Ipvm::Point32r2& pointImageCenter)
{
    if (m_sprocketHoleInfo == nullptr)
    {
        m_sprocketHoleInfo = new SprocketHoleInfo;
    }

    Ipvm::EllipseEq32r* pEllipseEqAll = nullptr;
    long circleNum = 0;
    Ipvm::Point32r2* pEdgePointAll = nullptr;
    long pointNum = 0;

    m_rtInspection.SetRect(0, 0, 0, 0);
    m_rtPocket.SetRect(0, 0, 0, 0);
    m_qrDevice = Ipvm::Quadrangle32r{-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f};
    m_alignedFillPocket = false; // Detail Tape Align에서 확인한 Fill Pocket 여부 초기화

    const long needDataNum = 5; // Pocket Rect, Device QuadRect //[중요]-필요한 정보를 다 읽었는지 확인하기 위한 변수다,
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
        if (debugInfo->moduleGuid == _VISION_INSP_GUID_CARRIER_TAPE_ALIGN)
        {
            if (debugInfo->strDebugInfoName == _DEBUG_INFO_CTA_CARRIER_TAPE_ALIGN_RECT)
            {
                if (debugInfo->pData != nullptr && debugInfo->nDataNum == 1)
                {
                    m_rtInspection = *(Ipvm::Rect32s*)debugInfo->pData;
                    collectedDataNum++;
                }
            }
        }
        else if (debugInfo->moduleGuid == _VISION_INSP_GUID_TAPE_DETAIL_ALIGN)
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
                    m_rtPocket
                        = Ipvm::Rect(static_cast<long>(rtPocket.m_left + .5f), static_cast<long>(rtPocket.m_top + .5f),
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

void VisionInspectionOTITR::SetFillPocketInfoInImageLot(bool receivedFillPocket)
{
    static const bool isHWExist = SystemConfig::GetInstance().IsHardwareExist();
    if (isHWExist == true)
        return;

    // Image Lot에 Fill Pocket 정보를 설정한다.
    auto& imageLot = getImageLot();
    imageLot.SetFillPocketTR(receivedFillPocket);
}

bool VisionInspectionOTITR::GetCheckCoverTapeAlign()
{
    for (long idx = OTI_INSPECTION_COVER_TAPE_START; idx < OTI_INSPECTION_COVER_TAPE_END; idx++)
    {
        auto* result = m_resultGroup.GetResultByName(g_szOTIInspectionName[idx]);
        if (result == nullptr)
        {
            continue;
        }
        auto* spec = GetSpecByName(result->m_resultName);
        if (spec == nullptr)
        {
            continue;
        }

        if(spec->m_use == TRUE)
            return true;
    }

    return false;
}

bool VisionInspectionOTITR::GetCheckSealingAlign()
{
    for (long idx = OTI_INSPECTION_SEALING_START; idx < OTI_INSPECTION_SEALING_END; idx++)
    {
        auto* result = m_resultGroup.GetResultByName(g_szOTIInspectionName[idx]);
        if (result == nullptr)
        {
            continue;
        }
        auto* spec = GetSpecByName(result->m_resultName);
        if (spec == nullptr)
        {
            continue;
        }

        if (spec->m_use == TRUE)
            return true;
    }

    return false;
}
bool VisionInspectionOTITR::GetSprocketHoleCenter(Ipvm::Point32r2& leftHolesCenter,
    Ipvm::Point32r2& rightHolesCenter)
{
    // Carrier Tape의 Sprocket Hole이 좌우의 존재 여부를 확인한다.
    bool existLeftSprocketHole = false;
    bool existRightSprocketHole = false;
    if (m_sprocketHoleInfo != nullptr)
    {
        existLeftSprocketHole = m_sprocketHoleInfo->m_isExistLeftSprocketHole;
        existRightSprocketHole = m_sprocketHoleInfo->m_isExistLeftSprocketHole;
    }

    if (existLeftSprocketHole == false && existRightSprocketHole == false)
    {
        //좌우 Sprocket Hole이 모두 존재하지 않으면 Align 실패로 간주한다.
        return false;
    }

    //Sprocket Hole이 존재하는 쪽의 Hole의 Center 좌표를 가져와 평균 좌표를 구한다.
    Ipvm::Point32r2 ptLeftCenter(-1.f, -1.f);
    Ipvm::Point32r2 ptRightCenter(-1.f, -1.f);
    long leftSprocketHoleNum = 0;
    long rightSprocketHoleNum = 0;
    if (existLeftSprocketHole)
    {
        leftSprocketHoleNum = (long)m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole.size();
        for (long i = 0; i < leftSprocketHoleNum; i++)
        {
            ptLeftCenter.m_x += m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole[i].m_x;
            ptLeftCenter.m_y += m_sprocketHoleInfo->m_vecCircleEqLeftSprockeHole[i].m_y;
        }
        ptLeftCenter.m_x /= (float)leftSprocketHoleNum;
        ptLeftCenter.m_y /= (float)leftSprocketHoleNum;

        leftHolesCenter = ptLeftCenter;
    }
    if (existRightSprocketHole)
    {
        rightSprocketHoleNum = (long)m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole.size();
        for (long i = 0; i < rightSprocketHoleNum; i++)
        {
            ptRightCenter.m_x += m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole[i].m_x;
            ptRightCenter.m_y += m_sprocketHoleInfo->m_vecCircleEqRightSprockeHole[i].m_y;
        }
        ptRightCenter.m_x /= (float)rightSprocketHoleNum;
        ptRightCenter.m_y /= (float)rightSprocketHoleNum;

        rightHolesCenter = ptRightCenter;
    }

    return true;
}

bool VisionInspectionOTITR::DetectVerticalAxisEdgePointaInROI(const Ipvm::Image8u& image,
    const Ipvm::Rect32s& searchROI,
    const float& edgeThreshold, const long& edgeSearchDirection, const long& edgeDetectMode, const long& edgeSearchWidth, 
    const float& edgeJumpStep, const bool& isFirstEdge,const bool& needDetectAllPoint, const bool& skipNoDetectedAllPoint,
    std::vector<Ipvm::Point32r2>& edgePoints)
{
    edgePoints.clear();
    if (m_pEdgeDetect == nullptr)
    {
        return false;
    }

    const bool permitNotDetected = (needDetectAllPoint == true && skipNoDetectedAllPoint == true) ? true : false;

    float edgeThresholdOld = m_pEdgeDetect->SetMininumThreshold(edgeThreshold);

    Ipvm::Point32r2 edgePoint{};
    Ipvm::Point32s2 startPoint = (edgeSearchDirection == LEFT) ? searchROI.TopRight() : searchROI.TopLeft();
    long edgeSearchLength = static_cast<long>(searchROI.Width());
    long tryNum = 0;
    long detectedNum = 0;
    long halfEdgeJumpStep = static_cast<long>(edgeJumpStep / 2.f + .5f);
    long stepStartY = startPoint.m_y + halfEdgeJumpStep;
    long stepEndY = searchROI.m_bottom - halfEdgeJumpStep;
    long stepY = stepStartY;
    //for (long y = searchROI.m_top + edgeJumpStep; y <= searchROI.m_bottom - edgeJumpStep; y += edgeJumpStep)
    while (stepY < stepEndY)
    {
        tryNum++;
        startPoint.m_y = stepY;
        if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(edgeDetectMode, startPoint, edgeSearchDirection,
                edgeSearchLength, edgeSearchWidth, image, edgePoint, isFirstEdge)
            == TRUE)
        {
            edgePoints.push_back(edgePoint);
            detectedNum++;
        }
        else if (permitNotDetected == true)
        {
            edgePoints.push_back(Ipvm::Point32r2(
                -1.f, static_cast<float>(stepY))); //검출되지 않은 위치에 대한 처리를 위해 (-1,y) 좌표를 넣어준다.
        }
        stepY = stepStartY + static_cast<long>(edgeJumpStep * tryNum);
    }

    m_pEdgeDetect->SetMininumThreshold(edgeThresholdOld);

    if (needDetectAllPoint == true && skipNoDetectedAllPoint == false && detectedNum != tryNum)
        return false;
    else if (detectedNum <= tryNum / 2)
        return false;

    return true;
}

bool VisionInspectionOTITR::AlignCoverTape(const bool detailSetupMode, const Ipvm::Point32r2& leftHolesCenter,
    const Ipvm::Point32r2& rightHolesCenter, Ipvm::LineEq32r& leftCoverTapeLine, Ipvm::LineEq32r& rightCoverTapeLine,
    Ipvm::Quadrangle32r& coverTapeArea, Ipvm::Rect32s& mergedSearchROI)
{
    Ipvm::Rect32s leftCoverTapeEdgeSearchROI;
    Ipvm::Rect32s rightCoverTapeEdgeSearchROI;
    Ipvm::Quadrangle32r qrCoverTapeAlignResult;

    //ROI 생성하고
    if (GetCoverTapeEdgeSearchROI(detailSetupMode, leftHolesCenter, rightHolesCenter, leftCoverTapeEdgeSearchROI, rightCoverTapeEdgeSearchROI) == false)
        return false;    

    mergedSearchROI = leftCoverTapeEdgeSearchROI;
    mergedSearchROI.UnionRect(rightCoverTapeEdgeSearchROI);

    //Edge Point를 찾는다.
    std::vector<Ipvm::Point32r2> leftCoverTapeEdgePoint(0);
    std::vector<Ipvm::Point32r2> rightCoverTapeEdgePoint(0);
    if (GetObjectEdgePoints(detailSetupMode, enumEdgeObjectType_CoverTape, 
            leftCoverTapeEdgeSearchROI, rightCoverTapeEdgeSearchROI,
            leftCoverTapeEdgePoint, rightCoverTapeEdgePoint)
        == false)
        return false;

    //Align을 위해 상하 경계의 FittingLine을 만든다.
    Ipvm::LineEq32r topBoundaryLine{};
    Ipvm::LineEq32r bottomBoundaryLine{};
    if (GetTopBottomBoundaryFittingLine(leftCoverTapeEdgeSearchROI, topBoundaryLine, bottomBoundaryLine) == false)
        return false;

    //찾은 EdgePoint로 좌우 각각 Line을 Fit 한다.
    Ipvm::LineSeg32r leftEdgeLine;
    Ipvm::LineSeg32r rightEdgeLine;
    
    if (GetObjectLineFitting(
            leftCoverTapeEdgePoint, topBoundaryLine, bottomBoundaryLine, leftCoverTapeLine, leftEdgeLine) == false)
        return false;
       
    SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_OTI_COVER_TAPE_EDGE_LEFT_LINE, leftEdgeLine); //true: append

    if (GetObjectLineFitting(
            rightCoverTapeEdgePoint, topBoundaryLine, bottomBoundaryLine, rightCoverTapeLine, rightEdgeLine) == false)
        return false;

    SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_OTI_COVER_TAPE_EDGE_RIGHT_LINE, rightEdgeLine);
    
    qrCoverTapeAlignResult = Ipvm::Quadrangle32r(leftEdgeLine.m_sx, leftEdgeLine.m_sy, rightEdgeLine.m_sx,
        rightEdgeLine.m_sy, leftEdgeLine.m_ex, leftEdgeLine.m_ey, rightEdgeLine.m_ex, rightEdgeLine.m_ey);
    SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_OTI_COVER_TAPE_ALIGN_QRECT, qrCoverTapeAlignResult);

    coverTapeArea = qrCoverTapeAlignResult;

    auto& memory = getReusableMemory();
    if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Cover Tape Align")))
    {
        surfaceRoi->Reset();
        surfaceRoi->Add(coverTapeArea);
    }

    return true;
}

bool VisionInspectionOTITR::GetTopBottomBoundaryFittingLine(
    const Ipvm::Rect32s& referenceROI,
    Ipvm::LineEq32r& topFittingLine, Ipvm::LineEq32r& bottomFittingLine)
{
    //Align을 위해 상하 경계의 FittingLine을 만든다.
    std::vector<Ipvm::Point32r2> boundaryPoint(2);
    boundaryPoint[0] = Ipvm::Point32r2((float)referenceROI.m_left, (float)referenceROI.m_top);
    boundaryPoint[1] = Ipvm::Point32r2((float)referenceROI.m_right, (float)referenceROI.m_top);
    if (Ipvm::DataFitting::FitToLineRn(2, &boundaryPoint[0], 3.f, topFittingLine) != Ipvm::Status::e_ok)
        return false;

    boundaryPoint[0].m_y = (float)referenceROI.m_bottom;
    boundaryPoint[1].m_y = (float)referenceROI.m_bottom;
    if (Ipvm::DataFitting::FitToLineRn(2, &boundaryPoint[0], 3.f, bottomFittingLine) != Ipvm::Status::e_ok)
        return false;

    return true;
}
    
bool VisionInspectionOTITR::GetCoverTapeEdgeSearchROI(const bool detailSetupMode, const Ipvm::Point32r2& leftHolesCenter,
    const Ipvm::Point32r2& rightHolesCenter, Ipvm::Rect32s& leftCoverTapeEdgeSearchROI, Ipvm::Rect32s& rightCoverTapeEdgeSearchROI)
{
    leftCoverTapeEdgeSearchROI.SetRect(-1, -1, -1, -1);
    rightCoverTapeEdgeSearchROI.SetRect(-1, -1, -1, -1);

    const VisionTapeSpec* tapeSpec = m_visionUnit.getTapeSpec();
    if (tapeSpec == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::AlignCoverTape() - tapeSpec is nullptr"));
        return false;
    }

    //좌우 홀의 존재 여부를 확인한다.
    const bool existLeftHoles = (leftHolesCenter.m_x >= 0.f && leftHolesCenter.m_y >= 0.f) ? true : false;
    const bool existRightHoles = (rightHolesCenter.m_x >= 0.f && rightHolesCenter.m_y >= 0.f) ? true : false;

    const float scaleX = getScale().pixelToUm().m_x;
    const float scaleY = getScale().pixelToUm().m_y;
    const float fSprocketToCoverTapePxl = (tapeSpec->m_f1 * 1000.f) / scaleX;
    const float fPocketPitchPxl = (tapeSpec->m_p1 * 1000.f) / scaleY;
    const float pocketSizeY = (tapeSpec->m_a0 * 1000.f) / scaleY;
    const float pocketGapY = fPocketPitchPxl - pocketSizeY;
    const float searchRoiHalfHeight = (pocketSizeY * 0.5f) + (pocketGapY * 0.75f);
    const float searchRoiHalfWidth =  (m_VisionPara->m_coverTapeAlignSearchLength_um  * 0.5f) / scaleX;
    const long coverTapeWidthPxl = static_cast<long>((tapeSpec->m_s1 * 1000.f) / scaleX + .5f);

    //좌측 연산
    if (existLeftHoles == true)
    {
        //좌측 센터를 기준으로 Align ROI를 구성한다.
        leftCoverTapeEdgeSearchROI.m_left
            = static_cast<long>(leftHolesCenter.m_x + fSprocketToCoverTapePxl  - searchRoiHalfWidth + .5f);
        leftCoverTapeEdgeSearchROI.m_right
            = static_cast<long>(leftHolesCenter.m_x + fSprocketToCoverTapePxl + searchRoiHalfWidth + .5f);
        leftCoverTapeEdgeSearchROI.m_top = static_cast<long>(leftHolesCenter.m_y - searchRoiHalfHeight + .5f);
        leftCoverTapeEdgeSearchROI.m_bottom = static_cast<long>(leftHolesCenter.m_y + searchRoiHalfHeight + .5f);

        if (existRightHoles == false)
        {
            //좌측 홀만 존재하는 경우 우측 ROI는 좌측 ROI와 동일하게 설정 후 coverTapeWidthPxl 만큼 우로 밀어 준다.
            rightCoverTapeEdgeSearchROI = leftCoverTapeEdgeSearchROI;
            rightCoverTapeEdgeSearchROI.m_left += coverTapeWidthPxl;
            rightCoverTapeEdgeSearchROI.m_right += coverTapeWidthPxl;
        }
    }
    //우측 연산
    if (existRightHoles == true)
    {
        //우측 센터를 기준으로 Align ROI를 구성한다.
        rightCoverTapeEdgeSearchROI.m_left
            = static_cast<long>(rightHolesCenter.m_x + fSprocketToCoverTapePxl - searchRoiHalfWidth + .5f);
        rightCoverTapeEdgeSearchROI.m_right
            = static_cast<long>(rightHolesCenter.m_x + fSprocketToCoverTapePxl + searchRoiHalfWidth + .5f);
        rightCoverTapeEdgeSearchROI.m_top = static_cast<long>(rightHolesCenter.m_y - searchRoiHalfHeight + .5f);
        rightCoverTapeEdgeSearchROI.m_bottom = static_cast<long>(rightHolesCenter.m_y + searchRoiHalfHeight + .5f);

        if (existLeftHoles == false)
        {
            //우측 홀만 존재하는 경우 좌측 ROI는 우측 ROI와 동일하게 설정 후 coverTapeWidthPxl 만큼 좌로 밀어 준다.
            leftCoverTapeEdgeSearchROI = rightCoverTapeEdgeSearchROI;
            leftCoverTapeEdgeSearchROI.m_left -= coverTapeWidthPxl;
            leftCoverTapeEdgeSearchROI.m_right -= coverTapeWidthPxl;
        }
    }

    //생성한 ROI는 Debug Info에 저장한다.
    std::vector<Ipvm::Rect32s> vecCoverTapeEdgeSearchROI(0);
    if (leftCoverTapeEdgeSearchROI.m_left >= -1)
        vecCoverTapeEdgeSearchROI.push_back(leftCoverTapeEdgeSearchROI);
    if (rightCoverTapeEdgeSearchROI.m_left >= -1)
        vecCoverTapeEdgeSearchROI.push_back(rightCoverTapeEdgeSearchROI);
    SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_OTI_COVER_TAPE_SEARCH_ROI, vecCoverTapeEdgeSearchROI);

    return true;
}

bool VisionInspectionOTITR::GetObjectEdgePoints(const bool detailSetupMode, const long& edgeObject,
    const Ipvm::Rect32s& leftEdgeSearchROI, const Ipvm::Rect32s& rightEdgeSearchROI,
    std::vector<Ipvm::Point32r2>& leftEdgePoint, std::vector<Ipvm::Point32r2>& rightEdgePoint)
{
    leftEdgePoint.clear();
    rightEdgePoint.clear();

    //Edge 추출 용 영상 만들기
    const auto& frameIdx = m_VisionPara->m_coverTapeImageFrameIndex;
    auto& imageProcPara = m_VisionPara->m_coverTapeImageProc;

    Ipvm::Image8u image = frameIdx.getImage(false);
    if (image.GetMem() == nullptr)
        return FALSE;

    Ipvm::Image8u combineImage;
    if (!getReusableMemory().GetInspByteImage(combineImage))
        return FALSE;

    if (!CippModules::GrayImageProcessingManage(getReusableMemory(), &image, false, imageProcPara, combineImage))
        return FALSE;

    //공통 Parameter 변수 선언
    const bool isCoverTapeEdge = edgeObject == enumEdgeObjectType_CoverTape ? true : false;
    const bool isLeftSealingEdge = edgeObject == enumEdgeObjectType_LeftSealing ? true : false;
    const bool isRightSealingEdge = edgeObject == enumEdgeObjectType_RightSealing ? true : false;
    if (edgeObject < enumEdgeObjectType_Start || edgeObject >= enumEdgeObjectType_End)
        return false;

    const float edgeThreshold = GetParameterEdgeThreshold(edgeObject);
    const long edgeDetectMode = GetParameterEdgeDetectMode(edgeObject);
    const float edgeJumpStep = (GetParameterEdgeDetectGap_um(edgeObject) / getScale().pixelToUm().m_y);    
    const long edgeSearchWidth = static_cast<long>(edgeJumpStep + .5f);
    const bool isFirstEdge = GetParameterIsFirstEdge(edgeObject);  
    const bool needDetectAllPoint = edgeObject != enumEdgeObjectType_CoverTape ? true : false;
    const bool skipNoDetectedAllPoint = needDetectAllPoint; //우선은 모든 Point를 검출 못해도 진도는 나가게 한다. Seaing Cutting을 검사하기 위해서다.

    LPCTSTR debugNameLeftEdge = GetDebugInfoName(edgeObject, true);
    LPCTSTR debugNameRightEdge = GetDebugInfoName(edgeObject, false);
    
    //좌측 Edge 추출
    long edgeSearchDirection = RIGHT;
    bool res = DetectVerticalAxisEdgePointaInROI(combineImage, leftEdgeSearchROI, edgeThreshold, edgeSearchDirection,
        edgeDetectMode, edgeSearchWidth, edgeJumpStep, isFirstEdge, needDetectAllPoint, skipNoDetectedAllPoint,
        leftEdgePoint);
    SetDebugInfoItem(detailSetupMode, debugNameLeftEdge, leftEdgePoint);

    if (res == false)
        return false;

    //우측 Edge 추출
    edgeSearchDirection = LEFT;
    res = DetectVerticalAxisEdgePointaInROI(combineImage, rightEdgeSearchROI, edgeThreshold, edgeSearchDirection,
        edgeDetectMode, edgeSearchWidth, edgeJumpStep, isFirstEdge, needDetectAllPoint, skipNoDetectedAllPoint,
        rightEdgePoint);
    SetDebugInfoItem(detailSetupMode, debugNameRightEdge, rightEdgePoint);

    if (res == false)
        return false;

    return true;
}  

float VisionInspectionOTITR::GetParameterEdgeThreshold(const long& edgeObject)
{
    return static_cast<float>(edgeObject == enumEdgeObjectType_CoverTape ? 
        m_VisionPara->m_coverTapeEdgeThreshold : m_VisionPara->m_sealingEdgeThreshold);
}

long VisionInspectionOTITR::GetParameterEdgeDetectMode(const long& edgeObject)
{
    if (edgeObject == enumEdgeObjectType_CoverTape)
        return PI_ED_DIR_RISING;
    
    if (m_VisionPara->m_sealingBoundaryType == enumSealingBoundaryType_Inner)
        return PI_ED_DIR_FALLING;

    return PI_ED_DIR_RISING;
}

long VisionInspectionOTITR::GetParameterEdgeDetectGap_um(const long& edgeObject)
{
    if (edgeObject == enumEdgeObjectType_CoverTape)
        return m_VisionPara->m_coverTapeDetectGap_um;

    return m_VisionPara->m_sealingDetectGap_um;
}

bool VisionInspectionOTITR::GetParameterIsFirstEdge(const long& edgeObject)
{
    if (edgeObject == enumEdgeObjectType_CoverTape)
        return m_VisionPara->m_coverTapeEdgeDetectMode == EdgeDetectMode_FirstEdge ? true : false;

    return m_VisionPara->m_sealingEdgeDetectMode == EdgeDetectMode_FirstEdge ? true : false;
}

LPCTSTR VisionInspectionOTITR::GetDebugInfoName(const long& edgeObject, const bool& isLeftEdge)
{
    if (edgeObject == enumEdgeObjectType_CoverTape)
        return isLeftEdge ? _DEBUG_INFO_OTI_COVER_TAPE_LEFT_EDGE_POINT : _DEBUG_INFO_OTI_COVER_TAPE_RIGHT_EDGE_POINT;

    if (edgeObject == enumEdgeObjectType_LeftSealing)
        return isLeftEdge ? _DEBUG_INFO_OTI_LEFT_SEALING_LEFT_EDGE_POINT : _DEBUG_INFO_OTI_LEFT_SEALING_RIGHT_EDGE_POINT;
    
    if (edgeObject == enumEdgeObjectType_RightSealing)
        return isLeftEdge ? _DEBUG_INFO_OTI_RIGHT_SEALING_LEFT_EDGE_POINT : _DEBUG_INFO_OTI_RIGHT_SEALING_RIGHT_EDGE_POINT;

    return _T("");
}

bool VisionInspectionOTITR::GetObjectLineFitting(const std::vector<Ipvm::Point32r2> vecPoint,
    const Ipvm::LineEq32r& topFittingLine, const Ipvm::LineEq32r& bottomFittingLine, 
    Ipvm::LineEq32r& fittingLine, Ipvm::LineSeg32r& fittingLineSeg)
{
    if (vecPoint.size() < 2)
        return false;

    static const bool useNoiseFilter = true;

    if (useNoiseFilter == true)
    {
        long pointNum = (long)vecPoint.size();
        std::vector<Ipvm::Point32r2> filteredPoint(0);
        for (long idx = 0; idx < pointNum; idx++)
        {
            if (vecPoint[idx].m_x >= 0.f)
                filteredPoint.push_back(vecPoint[idx]);
        }
        if (filteredPoint.size() < 2)
            return false;

        if (Ipvm::DataFitting::FitToLineRn(filteredPoint.size(), &filteredPoint[0], 3.f, fittingLine)
            != Ipvm::Status::e_ok)
            return false;
    }
    else
    {
        if (Ipvm::DataFitting::FitToLineRn(vecPoint.size(), &vecPoint[0], 3.f, fittingLine) != Ipvm::Status::e_ok)
            return false;
    }
    
    Ipvm::Point32r2 crossPoint{};
    Ipvm::Geometry::GetCrossPoint(topFittingLine, fittingLine, crossPoint);
    fittingLineSeg.m_sx = crossPoint.m_x;
    fittingLineSeg.m_sy = crossPoint.m_y;
    Ipvm::Geometry::GetCrossPoint(bottomFittingLine, fittingLine, crossPoint);
    fittingLineSeg.m_ex = crossPoint.m_x;
    fittingLineSeg.m_ey = crossPoint.m_y;

    return true;
}    
    
bool VisionInspectionOTITR::AlignSealing(
    const bool detailSetupMode, const Ipvm::Point32r2& leftHolesCenter, const Ipvm::Point32r2& rightHolesCenter, 
    std::vector<Ipvm::Point32r2>& vecLeftSealingLeft, std::vector<Ipvm::Point32r2>& vecLeftSealingRight, 
    std::vector<Ipvm::Point32r2>& vecRightSealingLeft, std::vector<Ipvm::Point32r2>& vecRightSealingRight, 
    std::vector<Ipvm::LineEq32r>& leftSealingLine, std::vector<Ipvm::LineEq32r>& rightSealingLine, 
    std::vector<Ipvm::Quadrangle32r>& sealingArea, Ipvm::Rect32s& mergedSearchROI)
{
    vecLeftSealingLeft.clear();//몇개가 나올지 모르나 아래의 3개의 벡터 변수와 카운트는 같아야 함.
    vecLeftSealingRight.clear();//몇개가 나올지 모름
    vecRightSealingLeft.clear();//몇개가 나올지 모름
    vecRightSealingRight.clear();//몇개가 나올지 모름
    leftSealingLine.clear();
    leftSealingLine.resize(2);
    rightSealingLine.clear();
    rightSealingLine.resize(2);
    sealingArea.clear();
    sealingArea.resize(2);

    Ipvm::Rect32s leftSealingEdgeLeftSearchROI;
    Ipvm::Rect32s leftSealingEdgerightSearchROI;
    Ipvm::Rect32s rightSealingEdgeLeftSearchROI;
    Ipvm::Rect32s rightSealingEdgerightSearchROI;
    
    //ROI 생성하고
    if (GetSealingEdgeSearchROI(detailSetupMode, leftHolesCenter, rightHolesCenter, leftSealingEdgeLeftSearchROI,
            leftSealingEdgerightSearchROI, rightSealingEdgeLeftSearchROI, rightSealingEdgerightSearchROI)
        == false)
        return false;

    mergedSearchROI = leftSealingEdgeLeftSearchROI;
    mergedSearchROI.UnionRect(leftSealingEdgerightSearchROI);
    mergedSearchROI.UnionRect(rightSealingEdgeLeftSearchROI);
    mergedSearchROI.UnionRect(rightSealingEdgerightSearchROI);

    //좌측 Sealing의 Edge Point를 찾는다.
    if (GetObjectEdgePoints(detailSetupMode, enumEdgeObjectType_LeftSealing, 
            leftSealingEdgeLeftSearchROI, leftSealingEdgerightSearchROI,
            vecLeftSealingLeft, vecLeftSealingRight)
        == false)
        return false;

    //우측 Sealing의 Edge Point를 찾는다.
    if (GetObjectEdgePoints(detailSetupMode, enumEdgeObjectType_RightSealing, 
            rightSealingEdgeLeftSearchROI, rightSealingEdgerightSearchROI, 
            vecRightSealingLeft, vecRightSealingRight)
        == false)
        return false;

     //Align을 위해 상하 경계의 FittingLine을 만든다.
    Ipvm::LineEq32r topBoundaryLine{};
    Ipvm::LineEq32r bottomBoundaryLine{};
    if (GetTopBottomBoundaryFittingLine(leftSealingEdgeLeftSearchROI, topBoundaryLine, bottomBoundaryLine) == false)
        return false;

    //좌측 Sealing Edge Point로 좌우 각각 Line을 Fit 한다
    std::vector<Ipvm::LineSeg32r> vecSealingLine(0); //Debug Info용
    Ipvm::LineSeg32r sealingLineSeg{};

    if (GetObjectLineFitting(
            vecLeftSealingLeft, topBoundaryLine, bottomBoundaryLine, leftSealingLine[0], sealingLineSeg) == false)
        return false;

    vecSealingLine.push_back(sealingLineSeg);

    if (GetObjectLineFitting(
            vecLeftSealingRight, topBoundaryLine, bottomBoundaryLine, leftSealingLine[1], sealingLineSeg)
        == false)
        return false;
    vecSealingLine.push_back(sealingLineSeg);

    //우측 Sealing Edge Point로 좌우 각각 Line을 Fit 한다
    if (GetObjectLineFitting(
            vecRightSealingLeft, topBoundaryLine, bottomBoundaryLine, rightSealingLine[0], sealingLineSeg)
        == false)
        return false;
    vecSealingLine.push_back(sealingLineSeg);

    if (GetObjectLineFitting(
            vecRightSealingRight, topBoundaryLine, bottomBoundaryLine, rightSealingLine[1], sealingLineSeg)
        == false)
        return false;
    vecSealingLine.push_back(sealingLineSeg);

    SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_OTI_SEALING_LINE, vecSealingLine); //true: append

    //찾은 Line으로 Sealing Area를 구한다.
    long idx1st = 0;
    long idx2nd = 1;

    for (long i = 0; i < 2; i++)
    {
        idx1st = i * 2;
        idx2nd = idx1st + 1;

        sealingArea[i] = Ipvm::Quadrangle32r(vecSealingLine[idx1st].m_sx, vecSealingLine[idx1st].m_sy,
            vecSealingLine[idx2nd].m_sx, vecSealingLine[idx2nd].m_sy, vecSealingLine[idx1st].m_ex,
            vecSealingLine[idx1st].m_ey, vecSealingLine[idx2nd].m_ex, vecSealingLine[idx2nd].m_ey);
    }
    SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_OTI_SEALING_QRECT, sealingArea);

    auto& memory = getReusableMemory();
    if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Sealing Align")))
    {
        surfaceRoi->Reset();
        for (const auto& area : sealingArea)
            surfaceRoi->Add(area);
    }

    return true;
}

bool VisionInspectionOTITR::GetSealingEdgeSearchROI(const bool detailSetupMode, const Ipvm::Point32r2& leftHolesCenter,
    const Ipvm::Point32r2& rightHolesCenter,
    Ipvm::Rect32s& leftSealingLeftSearchROI, Ipvm::Rect32s& leftSealingRightSearchROI,
    Ipvm::Rect32s& rightSealingLeftSearchROI, Ipvm::Rect32s& rightSealingRightSearchROI)
{
    leftSealingLeftSearchROI.SetRect(-1, -1, -1, -1);
    leftSealingRightSearchROI.SetRect(-1, -1, -1, -1);
    rightSealingLeftSearchROI.SetRect(-1, -1, -1, -1);
    rightSealingRightSearchROI.SetRect(-1, -1, -1, -1);

    const VisionTapeSpec* tapeSpec = m_visionUnit.getTapeSpec();
    if (tapeSpec == nullptr)
    {
        ASSERT(_T("VisionInspectionOTITR::AlignSealing() - tapeSpec is nullptr"));
        return false;
    }
    //좌우 홀의 존재 여부를 확인한다.
    const bool existLeftHoles = (leftHolesCenter.m_x >= 0.f && leftHolesCenter.m_y >= 0.f) ? true : false;
    const bool existRightHoles = (rightHolesCenter.m_x >= 0.f && rightHolesCenter.m_y >= 0.f) ? true : false;
    if (existLeftHoles == false && existRightHoles == false)
    {
        //좌우 홀 모두 존재하지 않으면 Align 실패로 간주한다.
        return false;
    }
    const float scaleX = getScale().pixelToUm().m_x;
    const float scaleY = getScale().pixelToUm().m_y;
    const float fSprocketToSealingPxl = (tapeSpec->m_f2 * 1000.f) / scaleX;
    const float fPocketPitchPxl = (tapeSpec->m_p1 * 1000.f) / scaleY;
    const float pocketSizeY = (tapeSpec->m_a0 * 1000.f) / scaleY;
    const float pocketGapY = fPocketPitchPxl - pocketSizeY;
    const float searchRoiHalfHeight = (pocketSizeY * 0.5f) + (pocketGapY * 0.75f);
    const long searchRoiWidth = static_cast<long>( (m_VisionPara->m_sealingAlignSearchLength_um * 0.5f) / scaleX + .5f);
    const long sealingSpanPxl = static_cast<long>((tapeSpec->m_s2 * 1000.f) / scaleX + .5f);

    //좌측 Hole 기준 좌측 Sealing의 ROI 연산
    if (existLeftHoles == true)
    {
        leftSealingLeftSearchROI.m_left
            = static_cast<long>(leftHolesCenter.m_x + fSprocketToSealingPxl - searchRoiWidth + .5f);
        leftSealingLeftSearchROI.m_right
            = static_cast<long>(leftHolesCenter.m_x + fSprocketToSealingPxl + .5f);
        leftSealingLeftSearchROI.m_top = static_cast<long>(leftHolesCenter.m_y - searchRoiHalfHeight + .5f);
        leftSealingLeftSearchROI.m_bottom = static_cast<long>(leftHolesCenter.m_y + searchRoiHalfHeight + .5f);
        leftSealingRightSearchROI = leftSealingLeftSearchROI;
        leftSealingRightSearchROI.m_left += searchRoiWidth;
        leftSealingRightSearchROI.m_right += searchRoiWidth;

        if (existRightHoles == false)
        {
            //좌측 홀만 존재하는 경우 우측 Sealing ROI는 좌측 ROI와 동일하게 설정 후 sealingSpanPxl 만큼 우로 밀어 준다.
            rightSealingLeftSearchROI = leftSealingLeftSearchROI;
            rightSealingLeftSearchROI.m_left += sealingSpanPxl;
            rightSealingLeftSearchROI.m_right += sealingSpanPxl;
            rightSealingRightSearchROI = leftSealingRightSearchROI;
            rightSealingRightSearchROI.m_left += sealingSpanPxl;
            rightSealingRightSearchROI.m_right += sealingSpanPxl;
        }
    }
    //우측 Hole 기준 우측 Sealing의 ROI 연산
    if (existRightHoles == true)
    {
        rightSealingRightSearchROI.m_left = static_cast<long>(rightHolesCenter.m_x + fSprocketToSealingPxl + .5f);
        rightSealingRightSearchROI.m_right
            = static_cast<long>(rightHolesCenter.m_x + fSprocketToSealingPxl + searchRoiWidth + .5f);
        rightSealingRightSearchROI.m_top = static_cast<long>(rightHolesCenter.m_y - searchRoiHalfHeight + .5f);
        rightSealingRightSearchROI.m_bottom = static_cast<long>(rightHolesCenter.m_y + searchRoiHalfHeight + .5f);
        rightSealingLeftSearchROI = rightSealingRightSearchROI;
        rightSealingLeftSearchROI.m_left -= searchRoiWidth;
        rightSealingLeftSearchROI.m_right -= searchRoiWidth;
        if (existLeftHoles == false)
        {
            //우측 홀만 존재하는 경우 좌측 Sealing ROI는 우측 ROI와 동일하게 설정 후 sealingSpanPxl 만큼 좌로 밀어 준다.
            leftSealingLeftSearchROI = rightSealingLeftSearchROI;
            leftSealingLeftSearchROI.m_left -= sealingSpanPxl;
            leftSealingLeftSearchROI.m_right -= sealingSpanPxl;
            leftSealingRightSearchROI = rightSealingRightSearchROI;
            leftSealingRightSearchROI.m_left -= sealingSpanPxl;
            leftSealingRightSearchROI.m_right -= sealingSpanPxl;
        }
    }

    std::vector<Ipvm::Rect32s> vecSealingEdgeSearchROI(0);
    if (leftSealingLeftSearchROI.m_left >= -1)
        vecSealingEdgeSearchROI.push_back(leftSealingLeftSearchROI);
    if (leftSealingRightSearchROI.m_left >= -1)
        vecSealingEdgeSearchROI.push_back(leftSealingRightSearchROI);
    if (rightSealingLeftSearchROI.m_left >= -1)
        vecSealingEdgeSearchROI.push_back(rightSealingLeftSearchROI);
    if (rightSealingRightSearchROI.m_left >= -1)
        vecSealingEdgeSearchROI.push_back(rightSealingRightSearchROI);

    SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_OTI_SEALING_SEARCH_ROI, vecSealingEdgeSearchROI);

    return true;
}

void VisionInspectionOTITR::GetCoverTapeSearchROI(std::vector<Ipvm::Rect32s>& searchROIs)
{
    searchROIs.clear();
    searchROIs.resize(2);

    searchROIs[0].SetRect(-1, -1, -1, -1);
    searchROIs[1].SetRect(-1, -1, -1, -1);
    
    Ipvm::Point32r2 leftHolesCenter;
    Ipvm::Point32r2 rightHolesCenter;
    if (GetSprocketHoleCenter(leftHolesCenter, rightHolesCenter) == false)
        return;

    GetCoverTapeEdgeSearchROI(false, leftHolesCenter, rightHolesCenter, searchROIs[0], searchROIs[1]);
}

void VisionInspectionOTITR::GetSealingSearchROI(std::vector<Ipvm::Rect32s>& searchROIs)
{
    searchROIs.clear();
    searchROIs.resize(4);

    for (long idx = 0; idx < 4; idx++)
        searchROIs[idx].SetRect(-1, -1, -1, -1);
    
    Ipvm::Point32r2 leftHolesCenter;
    Ipvm::Point32r2 rightHolesCenter;
    if (GetSprocketHoleCenter(leftHolesCenter, rightHolesCenter) == false)
        return;

    GetSealingEdgeSearchROI(
        false, leftHolesCenter, rightHolesCenter, searchROIs[0], searchROIs[1], searchROIs[2], searchROIs[3]);
}

void VisionInspectionOTITR::GetCoverTapeSearchLines(std::vector<Ipvm::LineSeg32r>& searchLines)
{
    searchLines.clear();
    std::vector<Ipvm::Rect32s> searchROIs(0);

    GetCoverTapeSearchROI(searchROIs);
    if (searchROIs.size() != 2 || searchROIs[0].m_left < 0 || searchROIs[1].m_left < 0)
        return;

    const float edgeJumpStep
        = (GetParameterEdgeDetectGap_um(enumEdgeObjectType_CoverTape) / getScale().pixelToUm().m_y);
    const long halfEdgeJumpStep = static_cast<long>(edgeJumpStep / 2.f + .5f);

    Ipvm::LineSeg32r lineSeg{};

    for (long roiID = 0; roiID < 2 ; roiID++)
    {
        const long startY = searchROIs[roiID].m_top + halfEdgeJumpStep;
        const long endY = searchROIs[roiID].m_bottom - halfEdgeJumpStep;
        long stepY = startY;
        long idx = 0;
        while (stepY <= endY)
        {
            lineSeg.m_sx = static_cast<float>(searchROIs[roiID].m_left);
            lineSeg.m_sy = static_cast<float>(stepY);
            lineSeg.m_ex = static_cast<float>(searchROIs[roiID].m_right);
            lineSeg.m_ey = static_cast<float>(stepY);
            searchLines.push_back(lineSeg);
            
            idx++;
            stepY = startY + (long)(edgeJumpStep * idx + .5f);
        }
    }

}

void VisionInspectionOTITR::GetSealingSearchLines(std::vector<Ipvm::LineSeg32r>& searchLines)
{
    searchLines.clear();
    std::vector<Ipvm::Rect32s> searchROIs(0);
    GetSealingSearchROI(searchROIs);
    if (searchROIs.size() != 4)
        return;

    const float edgeJumpStep
        = (GetParameterEdgeDetectGap_um(enumEdgeObjectType_LeftSealing) / getScale().pixelToUm().m_y);
    const long halfEdgeJumpStep = static_cast<long>(edgeJumpStep / 2.f + .5f);

    Ipvm::LineSeg32r lineSeg{};

    for (long roiID = 0; roiID < 4; roiID++)
    {
        if (searchROIs[roiID].m_left < 0)
            continue;
        const long startY = searchROIs[roiID].m_top + halfEdgeJumpStep;
        const long endY = searchROIs[roiID].m_bottom - halfEdgeJumpStep;
        long stepY = startY;
        long idx = 0;
        while (stepY <= endY)
        {
            lineSeg.m_sx = static_cast<float>(searchROIs[roiID].m_left);
            lineSeg.m_sy = static_cast<float>(stepY);
            lineSeg.m_ex = static_cast<float>(searchROIs[roiID].m_right);
            lineSeg.m_ey = static_cast<float>(stepY);
            searchLines.push_back(lineSeg);
            idx++;
            stepY = startY + (long)(edgeJumpStep * idx + .5f);
        }
    }
}