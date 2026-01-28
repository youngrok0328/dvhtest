//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBottom3D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBottom3D.h"
#include "VisionInspectionBgaBottom3DPara.h"
#include "VisionInspectionBgaBottom3DResult.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/ConversionEx.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedComponent/IntelSpecific/ShapeAlgorithm.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/Coplanarity_Algorithm.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image64r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/PlaneEq64r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32r3.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/Size32s2.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>
#include <math.h>

//CPP_6_________________________________ Preprocessor macros
#define PHYSICAL_PIXEL_SIZE_MAX (2 * 11 * 11)
#define LOGICAL_PIXEL_SIZE_MAX (255)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionBgaBottom3D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}
BOOL VisionInspectionBgaBottom3D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck TimeBall3DInspTotal;
    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}
    Ipvm::TimeCheck time;

    m_visionWarpageShapeResult.Init();

    //{{//kircheis_AIBC
    m_vecstrInvalidBallInfo_BH.clear();
    m_vecstrInvalidBallInfo_Copl.clear();
    //}}

    // 패키지 스펙 변경으로 인해 볼 종류가 달라질 수 있으므로 갯수를 맞춰준다.
    long GroupCount = (long)m_packageSpec.m_originalballMap->m_ballTypes.size();
    long ParamSize = (long)m_pVisionPara->m_parameters.size();

    m_pVisionPara->m_parameters.resize(GroupCount);

    if (ParamSize < GroupCount && ParamSize != 0)
    {
        for (int i = ParamSize; i < GroupCount; i++)
        {
            m_pVisionPara->m_parameters[i].Copy(m_pVisionPara->m_parameters[0]);
        }
    }

    // 20131004 영훈 : Bug 수정
    bool edgeAvailable = CollectBodyAlignResult();
    /// Data초기화
    /// 모든 사용데이터를 Invalid 처리 해 놓고 시작한다.
    InvalidSequence();

    if (!Initialize())
    {
        m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());
        return FALSE;
    }

    if (!edgeAvailable)
    {
        m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());
        return FALSE;
    }

    if (m_pVisionInspDlg != NULL)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    BOOL ret = InspRun(detailSetupMode);

    DisplayResultOverlay();

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("InspTotalTime"), CAST_FLOAT(TimeBall3DInspTotal.Elapsed_ms()));

    return ret;
}

BOOL VisionInspectionBgaBottom3D::Initialize()
{
    if ((long)getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].size() <= 0)
        return FALSE;

    //Ipvm::Image8u& image = getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][0];

    m_result->Init();

    ResetResult();

    ResetInspItem((long)m_packageSpec.m_ballMap->m_balls.size());

    return TRUE;
}

void VisionInspectionBgaBottom3D::InvalidSequence()
{
    if (m_fixedInspectionSpecs[_3DINSP_COPL].m_use)
    {
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[_3DINSP_COPL]);
        if (pResult != nullptr)
        {
            pResult->m_totalResult = INVALID;
            for (long i = 0; i < (long)pResult->m_objectErrorValues.size(); i++)
            {
                pResult->SetResult(i, pResult->m_totalResult);
            }
            pResult->SetTotalResult();
        }
    }

    if (m_fixedInspectionSpecs[_3DINSP_UNIT_COPL].m_use)
    {
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[_3DINSP_UNIT_COPL]);
        if (pResult != nullptr)
        {
            pResult->m_totalResult = INVALID;
            for (long i = 0; i < (long)pResult->m_objectErrorValues.size(); i++)
            {
                pResult->SetResult(i, pResult->m_totalResult);
            }
            pResult->SetTotalResult();
        }
    }

    if (m_fixedInspectionSpecs[_3DINSP_HEIGHT].m_use)
    {
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[_3DINSP_HEIGHT]);
        if (pResult != nullptr)
        {
            pResult->m_totalResult = INVALID;
            for (long i = 0; i < (long)pResult->m_objectErrorValues.size(); i++)
            {
                pResult->SetResult(i, pResult->m_totalResult);
            }
            pResult->SetTotalResult();
        }
    }

    if (m_fixedInspectionSpecs[_3DINSP_WARPAGE].m_use)
    {
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[_3DINSP_WARPAGE]);
        if (pResult != nullptr)
        {
            pResult->m_totalResult = INVALID;
            for (long i = 0; i < (long)pResult->m_objectErrorValues.size(); i++)
            {
                pResult->SetResult(i, pResult->m_totalResult);
            }
            pResult->SetTotalResult();
        }
    }

    if (m_fixedInspectionSpecs[_3DINSP_UNIT_WARPAGE].m_use)
    {
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE]);
        if (pResult != nullptr)
        {
            pResult->m_totalResult = INVALID;
            for (long i = 0; i < (long)pResult->m_objectErrorValues.size(); i++)
            {
                pResult->SetResult(i, pResult->m_totalResult);
            }
            pResult->SetTotalResult();
        }
    }
}

BOOL VisionInspectionBgaBottom3D::CheckROI(Ipvm::Rect32s& rtROI, long i_width, long i_height)
{
    if (rtROI.Width() <= 0 || rtROI.Height() <= 0)
        return FALSE;
    if (rtROI.m_left < 0 || rtROI.m_top < 0 || rtROI.m_right < rtROI.m_left || rtROI.m_bottom < rtROI.m_top)
        return FALSE;
    if (rtROI.m_right > i_width || rtROI.m_bottom > i_height)
        return FALSE;

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::MakeSpecObjectROI_ByPKGSize(std::vector<Ipvm::Rect32s>& vecrtObjectSpecROI,
    std::vector<Ipvm::Rect32s>& vecrtObjectSearchROI, long ballTypeIndex)
{
    vecrtObjectSpecROI.clear();

    /// Image 좌표계에서 찾아진 4개의 꼭지점
    Ipvm::Point32r2 pfptImgBody[4] = {
        Ipvm::Point32r2(0.f, 0.f),
    };
    PI_RECT srtPackageBody(
        m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB);

    pfptImgBody[0] = Ipvm::Point32r2((float)srtPackageBody.ltX, (float)srtPackageBody.ltY);
    pfptImgBody[1] = Ipvm::Point32r2((float)srtPackageBody.rtX, (float)srtPackageBody.rtY);
    pfptImgBody[2] = Ipvm::Point32r2((float)srtPackageBody.brX, (float)srtPackageBody.brY);
    pfptImgBody[3] = Ipvm::Point32r2((float)srtPackageBody.blX, (float)srtPackageBody.blY);

    /// 실제 좌표계에서 Packge Center를 기준으로 하는 4개의 꼭지점. 원래는 Body Size 정보를 사용하였으나, 문제가 있어 Align된 Size를 사용한다.
    float sizeMinX(0.f), sizeMaxX(0.f), sizeAvrX(0.f), sizeMinY(0.f), sizeMaxY(0.f), sizeAvrY(0.f);
    CPI_Geometry::GetRectSizeX(srtPackageBody, true, sizeMinX, sizeMaxX, sizeAvrX);
    CPI_Geometry::GetRectSizeY(srtPackageBody, true, sizeMinY, sizeMaxY, sizeAvrY);

    double dszX = (double)sizeMaxX;
    double dszY = (double)sizeMaxY;

    Ipvm::Point32r2 pfptRealBody[4] = {
        Ipvm::Point32r2(0.f, 0.f),
    };
    pfptRealBody[0] = Ipvm::Point32r2(float(-dszX / 2.0f), float(-dszY / 2.0f));
    pfptRealBody[1] = Ipvm::Point32r2(float(+dszX / 2.0f), float(-dszY / 2.0f));
    pfptRealBody[2] = Ipvm::Point32r2(float(+dszX / 2.0f), float(+dszY / 2.0f));
    pfptRealBody[3] = Ipvm::Point32r2(float(-dszX / 2.0f), float(+dszY / 2.0f));

    /// 실좌표와 Image좌표간의 방정식 계수를 구한다. (Polynomial warpping 또는 Bilinear transform을 참고하라)
    double bilinearTransformCoeff[2][4] = {
        0.,
    };

    if (Ipvm::Status::e_ok
        != Ipvm::Geometry::GetBilinearTransform(pfptRealBody, pfptImgBody, 4, bilinearTransformCoeff))
    {
        return FALSE;
    }

    auto& algoParas = m_pVisionPara->m_parameters;

    const Ipvm::Rect32s roiImage(getImageLotInsp().GetImageRect());

    for (const auto& ball : m_packageSpec.m_ballMapAll->m_balls)
    {
        if (ball.m_typeIndex != ballTypeIndex && ballTypeIndex >= 0)
        {
            continue;
        }

        const float x = (float)ball.m_posX_px;
        const float y = (float)ball.m_posY_px;

        const float ballImageCenterX = (float)(bilinearTransformCoeff[0][0] * x * y + bilinearTransformCoeff[0][1] * x
            + bilinearTransformCoeff[0][2] * y + bilinearTransformCoeff[0][3]);
        const float ballImageCenterY = (float)(bilinearTransformCoeff[1][0] * x * y + bilinearTransformCoeff[1][1] * x
            + bilinearTransformCoeff[1][2] * y + bilinearTransformCoeff[1][3]);

        vecrtObjectSpecROI.emplace_back((int32_t)(ballImageCenterX - ball.m_radiusX_px + 0.5f),
            (int32_t)(ballImageCenterY - ball.m_radiusY_px + 0.5f),
            (int32_t)(ballImageCenterX + ball.m_radiusX_px + 1.5f),
            (int32_t)(ballImageCenterY + ball.m_radiusY_px + 1.5f));

        vecrtObjectSpecROI.back() &= roiImage;

        const float searchAreaExtend = (float)(max(1.f, algoParas[ball.m_typeIndex].m_fBallSearchAreaPercent / 100.f));

        vecrtObjectSearchROI.emplace_back((int32_t)(ballImageCenterX - searchAreaExtend * ball.m_radiusX_px + 0.5f),
            (int32_t)(ballImageCenterY - searchAreaExtend * ball.m_radiusY_px + 0.5f),
            (int32_t)(ballImageCenterX + searchAreaExtend * ball.m_radiusX_px + 1.5f),
            (int32_t)(ballImageCenterY + searchAreaExtend * ball.m_radiusY_px + 1.5f));

        vecrtObjectSearchROI.back() &= roiImage;
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::InspBallHeight(const std::vector<Package::Ball> vecBallMapData,
    const std::vector<Ipvm::Rect32s>& vecrtObjectROI, const std::vector<Ipvm::Point32r3>& vec3DObjectData,
    const std::vector<Ipvm::Point32r3>& vec3DSRData, const CString strInspcetionName, const CString strCompName)
{
    auto* pResult = m_resultGroup.GetResultByName(strInspcetionName);
    if (pResult == NULL)
        return FALSE;

    auto* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    pResult->Clear();

    if (!pSpec->m_use)
        return TRUE;

    // bk 2015.09.23 데이터가 없으면 계산 무의미.
    const long nDataNum = long(vecBallMapData.size());

    if (nDataNum <= 0)
    {
        return FALSE;
    }

    pResult->Resize(nDataNum);
    CString strGroupID; //kircheis_MED2.5

    long idx = 0;

    //{{ //kircheis_AIBC
    //BOOL bIsAllGroupInsp = (vecBallMapData.size() == m_packageSpec.m_ballMap->m_balls.size());
    long nBallGroupNum = (long)m_packageSpec.m_originalballMap->m_ballTypes.size();
    std::vector<long> vecnInvalidBallCnt(nBallGroupNum);
    std::vector<long> vecnInvalidResult(nBallGroupNum);
    for (auto& nCnt : vecnInvalidBallCnt)
        nCnt = 0;
    for (auto& nResult : vecnInvalidResult)
        nResult = REJECT;

    CString strInvalidBallInfo;
    for (const auto& ball : vecBallMapData)
    {
        if (vec3DObjectData[idx].m_z != Ipvm::k_noiseValue32r)
        {
            idx++;
            continue;
        }

        strInvalidBallInfo.Format(_T("Group ID : [%s] , Ball Index : [%d] , Ball ID : [%s]"), (LPCTSTR)ball.m_groupID,
            ball.m_index, (LPCTSTR)ball.m_name);
        SetInvalidBallInfo(strInvalidBallInfo, m_vecstrInvalidBallInfo_BH);

        vecnInvalidBallCnt[ball.m_typeIndex]++;
        idx++;
    }

    if (nBallGroupNum == m_pVisionPara->m_parameters.size())
    {
        for (long nCurGroup = 0; nCurGroup < nBallGroupNum; nCurGroup++)
        {
            if (vecnInvalidBallCnt[nCurGroup] <= m_pVisionPara->m_parameters[nCurGroup]
                    .m_paraBallZ[(long)enumGetherBallZType::Height]
                    .m_dAcceptableInvalidBallCount)
                vecnInvalidResult[nCurGroup] = PASS;
        }
    }

    idx = 0;
    //}}

    for (const auto& ball : vecBallMapData)
    {
        const float fSpecBallHeight_um = (float)ball.m_height_um;
        strGroupID = ball.m_groupID; //kircheis_MED2.5

        float fBallHeight_um = vec3DObjectData[idx].m_z - vec3DSRData[idx].m_z;

        if (vec3DObjectData[idx].m_z == Ipvm::k_noiseValue32r)
            fBallHeight_um = Ipvm::k_noiseValue32r;
        if (vec3DSRData[idx].m_z == Ipvm::k_noiseValue32r)
            fBallHeight_um = Ipvm::k_noiseValue32r;

        const float fBallHeightDiff_um
            = fBallHeight_um == Ipvm::k_noiseValue32r ? Ipvm::k_noiseValue32r : (fBallHeight_um - fSpecBallHeight_um);

        if (fBallHeightDiff_um == Ipvm::k_noiseValue32r) //kircheis_AIBC
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, strGroupID, strCompName,
                fBallHeightDiff_um, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um, fSpecBallHeight_um,
                vecnInvalidResult[ball.m_typeIndex]);
        else
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, strGroupID, strCompName,
                fBallHeightDiff_um, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um,
                fSpecBallHeight_um); //kircheis_MED2.5
        pResult->SetRect(
            idx, vecrtObjectROI[idx]); // SDY 3D Ball height에서 Group별 검사의 Index를 전체 index로 주는 문제 수정

        idx++;
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strGroupID.Empty();
    strInvalidBallInfo.Empty();

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::InspBallHeight(const std::vector<Ipvm::Rect32s>& vecrtObjectROI,
    const std::vector<Ipvm::Point32r3>& vec3DObjectData, const std::vector<Ipvm::Point32r3>& vec3DSRData)
{
    //All Object 구간

    CString strCompID
        = m_packageSpec.m_originalballMap
              ->m_strAllGroupName; //kircheis_MED2.5 // 이 검사에 소속된 모든 Obj의 전체 그룹 ID를 몰아 넣는다.

    if (!InspBallHeight(m_packageSpec.m_ballMap->m_balls, vecrtObjectROI, vec3DObjectData, vec3DSRData,
            g_szBGA3DInspectionName[_3DINSP_HEIGHT], strCompID)) //kircheis_MED2.5
    {
        return FALSE;
    }

    //Group별 Data 분리 구간
    long ObjectCount = (long)m_packageSpec.m_ballMap->m_balls.size();
    long GroupCount = (long)m_packageSpec.m_originalballMap->m_ballTypes.size();

    std::vector<std::vector<Package::Ball>> vecGroupBallData;
    std::vector<std::vector<Ipvm::Point32r3>> vecObjectPosition;
    std::vector<std::vector<Ipvm::Point32r3>> vecSRPosition;
    std::vector<std::vector<Ipvm::Rect32s>> vecObject;

    vecGroupBallData.resize(GroupCount);
    vecObjectPosition.resize(GroupCount);
    vecSRPosition.resize(GroupCount);
    vecObject.resize(GroupCount);

    for (int idx = 0; idx < ObjectCount; idx++)
    {
        long GroupIdx = m_packageSpec.m_ballMap->m_balls[idx].m_typeIndex;

        vecGroupBallData[GroupIdx].push_back(m_packageSpec.m_ballMap->m_balls[idx]);
        vecObjectPosition[GroupIdx].push_back(vec3DObjectData[idx]);
        vecSRPosition[GroupIdx].push_back(vec3DSRData[idx]);
        vecObject[GroupIdx].push_back(vecrtObjectROI[idx]);
        ;
    }

    //Group Object 구간
    long GroupSpecCount = (long)m_pVisionPara->m_vecstrGroupInspName.size();

    if (GroupSpecCount != 0 && GroupSpecCount != GroupCount)
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        strCompID = m_packageSpec.m_originalballMap->m_ballTypes[idx].m_groupID; //kircheis_MED2.5
        if (!InspBallHeight(vecGroupBallData[idx], vecObject[idx], vecObjectPosition[idx], vecSRPosition[idx],
                m_pVisionPara->m_vecstrGroupInspName[idx].BallHeightName, strCompID)) //kircheis_MED2.5
        {
            return FALSE;
        }
    }

    strCompID.Empty();

    return TRUE;
}

void VisionInspectionBgaBottom3D::DisplayResultOverlay()
{
    if (m_pVisionInspDlg != NULL)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_AddLine(
            m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, RGB(0, 255, 0));
        m_pVisionInspDlg->m_imageLotView->Overlay_AddLine(
            m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptRB, RGB(0, 255, 0));
        m_pVisionInspDlg->m_imageLotView->Overlay_AddLine(
            m_sEdgeAlignResult->fptRB, m_sEdgeAlignResult->fptLB, RGB(0, 255, 0));
        m_pVisionInspDlg->m_imageLotView->Overlay_AddLine(
            m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptLT, RGB(0, 255, 0));

        m_pVisionInspDlg->m_imageLotView->Overlay_Show(TRUE);
    }
}

BOOL VisionInspectionBgaBottom3D::InspRun(const bool detailSetupMode)
{
    Ipvm::TimeCheck TimeBall3DPreprocessing;
    Ipvm::Quadrangle32r srtPackageBody(ConversionEx::ToQuadrangle32r(
        m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB));

    Ipvm::Rect32s roiPackage(Ipvm::Conversion::ToRect32s(srtPackageBody));

    if ((roiPackage & Ipvm::Rect32s(getImageLotInsp().m_zmapImage)) != roiPackage)
    {
        // Package Roi가 화면을 벗어났다. 이상하다
        return FALSE;
    }

    //const long roiLeft = roiPackage.m_left;
    //const long roiTop = roiPackage.m_top;
    const int roiSizeX = roiPackage.Width();
    const int roiSizeY = roiPackage.Height();

    Ipvm::Image32r subZmap(getImageLotInsp().m_zmapImage, roiPackage);

    // 디바이스 존재 여부 체크
    Ipvm::Rect32s roiCheck{0, 0, roiSizeX, roiSizeY};
    static bool bDeflate = false;
    if (bDeflate)
        roiCheck.DeflateRect(roiSizeX / 4, roiSizeY / 4, roiSizeX / 4, roiSizeY / 4);

    long validPixelCount = 0;
    const long nCalcGap = 2;

    for (long y = roiCheck.m_top; y < roiCheck.m_bottom; y += nCalcGap)
    {
        const float* zmap = subZmap.GetMem(0, y);

        for (long x = roiCheck.m_left; x < roiCheck.m_right; x += nCalcGap)
        {
            if (zmap[x] != Ipvm::k_noiseValue32r)
            {
                validPixelCount++;
            }
        }
    }
    validPixelCount *= (nCalcGap * nCalcGap);

    static bool bIsSocketTest = false; //kircheis_SocketTest
    if (!bIsSocketTest)
    {
        if (validPixelCount < roiCheck.Width() * roiCheck.Height() / 4)
        {
            return FALSE;
        }
    }

    MakeSpecObjectROI_ByPKGSize(m_result->m_allBallSpecROIs, m_result->m_allBallSearchROIs);

    if (m_result->m_allBallSpecROIs.size() <= 0)
    {
        return FALSE;
    }

    SetDebugInfoItem(detailSetupMode, _T("Ball 3D Spec ROI"), m_result->m_allBallSpecROIs, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("Ball 3D Search ROI"), m_result->m_allBallSearchROIs);

    if (!FindBallBlob(m_result->m_allBallSearchROIs, m_result->m_allBlobPositions))
    {
        return FALSE;
    }

    SetDebugInfoItem(detailSetupMode, _T("Ball 3D Blob Positions"), m_result->m_allBlobPositions);

    BOOL alignSuccess = AlignBall(m_result->m_allBlobPositions, m_result->m_allBallPositions,
        m_result->m_validBallPositions, m_result->m_validBallROIs);

    SetDebugInfoItem(detailSetupMode, _T("Ball 3D Aligned Positions"), m_result->m_validBallPositions);
    SetDebugInfoItem(detailSetupMode, _T("Ball 3D Aligned ROI"), m_result->m_validBallROIs, TRUE);

    if (alignSuccess == FALSE)
    {
        return FALSE;
    }

    Ipvm::Image8u substrateMaskImage;
    BOOL srFindSuccess = GatherSrZ(m_result->m_allBallPositions, m_result->m_validSubstrateROIs,
        m_result->m_validSubstrate3DData, substrateMaskImage);

    for (auto& pt : m_result->m_validSubstrate3DData)
    {
        m_result->m_validSubstrateDataZ.push_back(pt.m_z);
    }

    SetDebugInfoItem(detailSetupMode, _T("Substrate Mask Image"), substrateMaskImage);
    SetDebugInfoItem(detailSetupMode, _T("Ball 3D Substrate ROI"), m_result->m_validSubstrateROIs);
    SetDebugInfoItem(detailSetupMode, _T("Substrate z values"), m_result->m_validSubstrateDataZ);

    if (srFindSuccess == FALSE)
    {
        return FALSE;
    }

    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("PreprocessingTime"), CAST_FLOAT(TimeBall3DPreprocessing.Elapsed_ms()));

    Ipvm::TimeCheck TimeBall3DGetInspData;
    for (long type = 0; type < long(enumGetherBallZType::END); type++)
    {
        Ipvm::Image8u ballLogicalPixelImage;

        GatherBallZ(m_result->m_validBallPositions, m_result->m_validSubstrate3DData, enumGetherBallZType(type),
            m_result->m_validBall3DData[type], ballLogicalPixelImage);

        std::vector<float> validBallDataZ;
        for (auto& pt : m_result->m_validBall3DData[type])
        {
            validBallDataZ.push_back(pt.m_z);
        }

        CString typeName;
        typeName.Format(_T("[%s]"), GetBallZTypeString(enumGetherBallZType(type)));

        SetDebugInfoItem(detailSetupMode, typeName + _T(" Ball Logical Pixel Image"), ballLogicalPixelImage);
        SetDebugInfoItem(detailSetupMode, typeName + _T(" Ball z values"), validBallDataZ);

        Ipvm::Image8u maskedHeightImage;
        if (!getReusableMemory().GetInspByteImage(maskedHeightImage))
            return FALSE;
        maskedHeightImage.FillZero();

        for (long y = roiPackage.m_top; y < roiPackage.m_bottom; y++)
        {
            const BYTE* srcGray = getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][0].GetMem(0, y);
            const BYTE* ballLogicalPixel = ballLogicalPixelImage.GetMem(0, y);
            const BYTE* srMask = substrateMaskImage.GetMem(0, y);
            BYTE* dstGray = maskedHeightImage.GetMem(0, y);

            for (long x = roiPackage.m_left; x < roiPackage.m_right; x++)
            {
                if (ballLogicalPixel[x] || srMask[x])
                {
                    dstGray[x] = srcGray[x];
                }
            }
        }

        SetDebugInfoItem(detailSetupMode, typeName + _T(" Masked Height Image"), maskedHeightImage);

        typeName.Empty();
    }

    long nBallTypeCount = (long)m_packageSpec.m_originalballMap->m_ballTypes.size();
    std::vector<SurfaceLayerRoi*> groupSurfaceRoi;

    groupSurfaceRoi.resize(nBallTypeCount);

    for (int i = 0; i < nBallTypeCount; i++)
    {
        CString str;

        str.Format(_T("Measured - Ball Group%s"), (LPCTSTR)m_packageSpec.m_originalballMap->m_ballTypes[i].m_groupID);

        groupSurfaceRoi[i] = getReusableMemory().AddSurfaceLayerRoiClass(str);
        groupSurfaceRoi[i]->Reset();

        str.Empty();
    }

    if (auto* surfaceRoi = getReusableMemory().AddSurfaceLayerRoiClass(_T("Measured - Ball")))
    {
        surfaceRoi->Reset();
        int idx = 0;

        for (auto& ball : m_result->m_validBallROIs)
        {
            Ipvm::EllipseEq32r ballData = Ipvm::EllipseEq32r((ball.m_left + ball.m_right) * 0.5f,
                (ball.m_top + ball.m_bottom) * 0.5f, ball.Width() * 0.5f, ball.Height() * 0.5f);

            if (nBallTypeCount > 1)
                groupSurfaceRoi[m_packageSpec.m_ballMap->m_balls[idx].m_typeIndex]->Add(ballData);

            surfaceRoi->Add(ballData);

            idx++;
        }
    }
    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("GetInspDataTime"), CAST_FLOAT(TimeBall3DGetInspData.Elapsed_ms()));

    Ipvm::TimeCheck TimeInspTime;
    //if (m_fixedInspectionSpecs[_3DINSP_COPL].m_use || m_fixedInspectionSpecs[_3DINSP_UNIT_COPL].m_use)//kircheis_AIBC
    {
        if (!InspBallCoplanarity(
                m_result->m_validBall3DData[long(enumGetherBallZType::Copl)], m_result->m_validBallROIs))
        {
            return FALSE;
        }
    }

    //if (m_fixedInspectionSpecs[_3DINSP_HEIGHT].m_use)//kircheis_AIBC
    {
        if (!InspBallHeight(m_result->m_validBallROIs, m_result->m_validBall3DData[long(enumGetherBallZType::Height)],
                m_result->m_validSubstrate3DData))
        {
            return FALSE;
        }
    }

    //if (m_fixedInspectionSpecs[_3DINSP_WARPAGE].m_use || m_fixedInspectionSpecs[_3DINSP_UNIT_WARPAGE].m_use)//kircheis_AIBC
    {
        if (!InspWarpage(m_result->m_validSubstrate3DData, m_result->m_validSubstrateROIs, detailSetupMode))
        {
            return FALSE;
        }
    }

    AddProcessingFunctionExcuteTimeLog(m_strModuleName, _T("InspTime"), CAST_FLOAT(TimeInspTime.Elapsed_ms()));

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::FindBallBlob(
    std::vector<Ipvm::Rect32s>& vecrtObjectSearchROI, std::vector<Ipvm::Point32r2>& ballBlobPositions)
{
    // Adaptive Threshold 적용을 시도한다.

    // Z-Map 을 가지고 적분 영상을 만든 후,
    // 최대 Ball 크기의 3배 영역 크기로 배경 높이를 구한 후,
    // 배경 높이를 제거해 준다.
    // 이렇게 하면 Ball 또는 높이를 가지는 녀석들만 남게 됨...

    // 이것을 최소 Ball Spec 의 최소 높이를 기준으로 255 밝기로 정규화한 이미지를 만든다..
    // 이것을 가지고 Ball 들을 검출하자.

    Ipvm::Quadrangle32r srtPackageBody(ConversionEx::ToQuadrangle32r(
        m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB));
    Ipvm::Rect32s roiPackage(Ipvm::Conversion::ToRect32s(srtPackageBody));

    //const long roiLeft = roiPackage.m_left;
    //const long roiTop = roiPackage.m_top;
    const long roiSizeX = roiPackage.Width();
    const long roiSizeY = roiPackage.Height();

    if (roiSizeX <= 0 || roiSizeY <= 0)
    {
        return FALSE;
    }

    Ipvm::Image32r pkgForeZMapImage(getImageLotInsp().m_zmapImage, roiPackage);
    Ipvm::Image16u pkgForeVMapImage(getImageLotInsp().m_vmapImage, roiPackage);
    Ipvm::Image64r pkgIntegralImage(roiSizeX, roiSizeY);
    Ipvm::Image32s pkgSumCountImage(roiSizeX, roiSizeY);

    if (Ipvm::Status::e_ok
        != Ipvm::ImageProcessing::IntegralInclusiveWithNoise(pkgForeZMapImage, Ipvm::Rect32s(pkgForeZMapImage),
            Ipvm::k_noiseValue32r, pkgIntegralImage, pkgSumCountImage))
    {
        return FALSE;
    }

    float maxBallDiameterX_px = 0.f;
    float maxBallDiameterY_px = 0.f;
    float maxBallHeight_um = -FLT_MAX;
    float minBallHeight_um = FLT_MAX;

    for (const auto& typeInfo : m_packageSpec.m_originalballMap->m_ballTypes)
    {
        const float ballDiameterX_px = (float)getScale().convert_umToPixelX(typeInfo.m_diameter_um);

        if (ballDiameterX_px > maxBallDiameterX_px)
        {
            maxBallDiameterX_px = ballDiameterX_px;
        }

        const float ballDiameterY_px = (float)getScale().convert_umToPixelY(typeInfo.m_diameter_um);

        if (ballDiameterY_px > maxBallDiameterY_px)
        {
            maxBallDiameterY_px = ballDiameterY_px;
        }

        const float ballHeight_um = (float)typeInfo.m_height_um;

        if (ballHeight_um < minBallHeight_um)
        {
            minBallHeight_um = ballHeight_um;
        }

        if (ballHeight_um > maxBallHeight_um)
        {
            maxBallHeight_um = ballHeight_um;
        }
    }

    Ipvm::Image32r pkgBackZMapImage(roiSizeX, roiSizeY);
    Ipvm::Size32s2 windowSize = {long(3 * maxBallDiameterX_px + 0.5f), long(3 * maxBallDiameterY_px + 0.5f)};

    if (Ipvm::Status::e_ok
        != Ipvm::ImageProcessing::MakeBackgroundImageWithNoise(pkgIntegralImage, pkgSumCountImage,
            Ipvm::Rect32s(pkgForeZMapImage), windowSize, Ipvm::k_noiseValue32r, pkgBackZMapImage))
    {
        return FALSE;
    }

    minBallHeight_um = CAST_FLOAT(fabs(minBallHeight_um));
    maxBallHeight_um = CAST_FLOAT(fabs(maxBallHeight_um));

    Ipvm::Image8u grayImage;
    getReusableMemory().GetInspByteImage(grayImage);
    Ipvm::Image8u pkgGrayImage(grayImage, roiPackage);

    if (maxBallHeight_um == minBallHeight_um)
    {
        for (long y = 0; y < roiSizeY; y++)
        {
            const float* zmap = pkgForeZMapImage.GetMem(0, y);
            const float* back = pkgBackZMapImage.GetMem(0, y);

            BYTE* gray = pkgGrayImage.GetMem(0, y);

            for (long x = 0; x < roiSizeX; x++)
            {
                if (zmap[x] != Ipvm::k_noiseValue32r && back[x] != Ipvm::k_noiseValue32r)
                {
                    const float diff = zmap[x] - back[x];

                    if (diff >= minBallHeight_um)
                    {
                        gray[x] = 255;
                    }
                    else if (diff <= 0.f)
                    {
                        gray[x] = 0;
                    }
                    else
                    {
                        gray[x] = (BYTE)(diff / minBallHeight_um * 255.f);
                    }
                }
                else
                {
                    gray[x] = 0;
                }
            }
        }
    }
    else
    {
        for (long y = 0; y < roiSizeY; y++)
        {
            const float* zmap = pkgForeZMapImage.GetMem(0, y);
            const float* back = pkgBackZMapImage.GetMem(0, y);

            BYTE* gray = pkgGrayImage.GetMem(0, y);

            for (long x = 0; x < roiSizeX; x++)
            {
                if (zmap[x] != Ipvm::k_noiseValue32r && back[x] != Ipvm::k_noiseValue32r)
                {
                    const float diff = zmap[x] - back[x];

                    if (diff >= maxBallHeight_um)
                    {
                        gray[x] = 0;
                    }
                    else if (diff >= minBallHeight_um)
                    {
                        gray[x] = 255;
                    }
                    else if (diff <= 0.f)
                    {
                        gray[x] = 0;
                    }
                    else
                    {
                        gray[x] = (BYTE)(diff / minBallHeight_um * 255.f);
                    }
                }
                else
                {
                    gray[x] = 0;
                }
            }
        }
    }

#ifdef _DEBUG
    Ipvm::ImageFile::SaveAsPng(pkgGrayImage, L"C:\\Temp\\BallImage_0.png");
#endif

    Ipvm::Image8u tempBuffer;
    getReusableMemory().GetInspByteImage(tempBuffer);
    Ipvm::Image8u pkgTempImage(tempBuffer, roiPackage);

    if (Ipvm::Status::e_ok
        != Ipvm::ImageProcessing::FilterMedian3x3(pkgGrayImage, Ipvm::Rect32s(pkgGrayImage), false, pkgTempImage))
    {
        return FALSE;
    }

#ifdef _DEBUG
    Ipvm::ImageFile::SaveAsPng(pkgTempImage, L"C:\\Temp\\BallImage_1.png");
#endif

    if (Ipvm::Status::e_ok
        != Ipvm::ImageProcessing::FilterMedian3x3(pkgTempImage, Ipvm::Rect32s(pkgTempImage), false, pkgGrayImage))
    {
        return FALSE;
    }

#ifdef _DEBUG
    Ipvm::ImageFile::SaveAsPng(pkgGrayImage, L"C:\\Temp\\BallImage_2.png");
#endif

    Ipvm::Image8u binFullImage;
    getReusableMemory().GetInspByteImage(binFullImage);
    Ipvm::Image8u pkgBinImage(binFullImage, roiPackage);

    //const BYTE threshold = 150;

    BYTE byLowMean = 0;
    BYTE byHighMean = 0;
    BYTE byThreshold = 0;

    Ipvm::ImageProcessing::GetThresholdOtsu(
        pkgGrayImage, Ipvm::Rect32s(pkgGrayImage), byLowMean, byHighMean, byThreshold);

    if (Ipvm::Status::e_ok
        != Ipvm::ImageProcessing::BinarizeGreater(pkgGrayImage, Ipvm::Rect32s(pkgGrayImage), byThreshold, pkgBinImage))
    {
        return FALSE;
    }

#ifdef _DEBUG
    Ipvm::ImageFile::SaveAsPng(pkgBinImage, L"C:\\Temp\\BallImage_3.png");
#endif

    const long nObjectTotalNum = (long)vecrtObjectSearchROI.size();
    const long nMaxBlobNum = 100;

    ballBlobPositions.clear();
    ballBlobPositions.reserve(nObjectTotalNum);

    std::vector<Ipvm::BlobInfo> vecObjBlob(nMaxBlobNum);

    Ipvm::Image32s labelImage;
    getReusableMemory().GetInspLongImage(labelImage);
    Ipvm::Image32s pkgLabelImage(labelImage, roiPackage);

    auto* pBlob = getReusableMemory().GetBlob();

    for (long nIdx = 0; nIdx < nObjectTotalNum; nIdx++)
    {
        Ipvm::Rect32s rtROI = vecrtObjectSearchROI[nIdx] - roiPackage.TopLeft();

        int32_t nCurBlobNum = nMaxBlobNum;

        pBlob->DoBlob(pkgBinImage, rtROI, nMaxBlobNum, pkgLabelImage, &vecObjBlob[0], nCurBlobNum);

        if (nCurBlobNum <= 0)
        {
            Ipvm::Point32r2 pt = {0.5f * (rtROI.m_left + rtROI.m_right - 1), 0.5f * (rtROI.m_top + rtROI.m_bottom - 1)};

            pt.m_x += roiPackage.m_left;
            pt.m_y += roiPackage.m_top;

            ballBlobPositions.push_back(pt);
            continue;
        }

        long maxBlobIndex = 0;
        long maxBlobArea = 0;

        for (long i = 0; i < nCurBlobNum; i++)
        {
            if (vecObjBlob[i].m_area > maxBlobArea)
            {
                maxBlobArea = vecObjBlob[i].m_area;
                maxBlobIndex = i;
            }
        }

        const auto& rtObject = vecObjBlob[maxBlobIndex].m_roi;
        const long label = vecObjBlob[maxBlobIndex].m_label;

        // Blob 의 무게 중심을 다시 구한다.
        // 이 때는 Reliability Image 를 가중치로 더 사용한다.

        float fBallCenterX_px = 0.f;
        float fBallCenterY_px = 0.f;
        float weightSum = 0;

        for (long y = rtObject.m_top; y < rtObject.m_bottom; y++)
        {
            const BYTE* gray = pkgGrayImage.GetMem(0, y);
            const WORD* relib = pkgForeVMapImage.GetMem(0, y);
            const auto* labels = pkgLabelImage.GetMem(0, y);

            for (long x = rtObject.m_left; x < rtObject.m_right; x++)
            {
                if (labels[x] == label && relib[x] > 0)
                {
                    const long intensityWeight = gray[x] - byThreshold;

                    if (intensityWeight > 0)
                    {
                        const float weight = (float)(intensityWeight * relib[x]);

                        fBallCenterX_px += weight * x;
                        fBallCenterY_px += weight * y;
                        weightSum += weight;
                    }
                }
            }
        }

        if (weightSum > 0.f)
        {
            fBallCenterX_px /= weightSum;
            fBallCenterY_px /= weightSum;
        }
        else
        {
            Ipvm::Point32r2 pt = {0.5f * (rtROI.m_left + rtROI.m_right - 1), 0.5f * (rtROI.m_top + rtROI.m_bottom - 1)};

            pt.m_x += roiPackage.m_left;
            pt.m_y += roiPackage.m_top;

            ballBlobPositions.push_back(pt);
            continue;
        }

        Ipvm::Point32r2 pt = {fBallCenterX_px + roiPackage.m_left, fBallCenterY_px + roiPackage.m_top};

        ballBlobPositions.push_back(pt);
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::AlignBall(const std::vector<Ipvm::Point32r2>& allBlobPositions,
    std::vector<Ipvm::Point32r2>& allBallPositions, std::vector<Ipvm::Point32r2>& validBallPositions,
    std::vector<Ipvm::Rect32s>& validBallROIs)
{
    // 처음에 찾은 Ball 들을 가지고 Ball Align 을 수행한다.
    // 수행한 결과는 스펙의 배치와 똑같은 배치를 가지는 볼 위치이다.

    // Spec Ball 위치에서 찾은 Blob 위치로의 변환을 구한다.

    // 먼저 Spec Ball 위치를 수집
    std::vector<Ipvm::Point32r2> specBallPositions_px;
    std::vector<float> specBallRadiusX_px;
    std::vector<float> specBallRadiusY_px;

    for (const auto& ball : m_packageSpec.m_ballMapAll->m_balls)
    {
        Ipvm::Point32r2 fptSpecBallPos((float)ball.m_posX_px, (float)ball.m_posY_px);
        specBallPositions_px.push_back(fptSpecBallPos);

        Ipvm::Point32r2 fBallRadius((float)ball.m_radiusX_px, (float)ball.m_radiusY_px);
        specBallRadiusX_px.push_back(fBallRadius.m_x);
        specBallRadiusY_px.push_back(fBallRadius.m_y);
    }

    if (specBallPositions_px.size() < 4)
    {
        return FALSE;
    }

    if (allBlobPositions.size() != specBallPositions_px.size())
    {
        return FALSE;
    }

    double affineTransformSpecToReal[2][3] = {0., 0., 0., 0., 0., 0.};

    if (Ipvm::Status::e_ok
        != Ipvm::Geometry::GetAffineTransform(
            &specBallPositions_px[0], &allBlobPositions[0], (long)allBlobPositions.size(), affineTransformSpecToReal))
    {
        return FALSE;
    }

    std::vector<float> errorScores(specBallPositions_px.size());

    for (long idx = (long)specBallPositions_px.size() - 1; idx >= 0; idx--)
    {
        const float x = (float)(specBallPositions_px[idx].m_x * affineTransformSpecToReal[0][0]
            + specBallPositions_px[idx].m_y * affineTransformSpecToReal[0][1] + affineTransformSpecToReal[0][2]);
        const float y = (float)(specBallPositions_px[idx].m_x * affineTransformSpecToReal[1][0]
            + specBallPositions_px[idx].m_y * affineTransformSpecToReal[1][1] + affineTransformSpecToReal[1][2]);

        const float dx = allBlobPositions[idx].m_x - x;
        const float dy = allBlobPositions[idx].m_y - y;

        errorScores[idx] = dx * dx + dy * dy;
    }

    Ipvm::Geometry::GetOptimalAffineTransform(&specBallPositions_px[0], &allBlobPositions[0], &errorScores[0], 0.5f,
        (long)specBallPositions_px.size(), affineTransformSpecToReal);

    // 구한 변환으로 정확한 Ball 위치를 정한다.
    allBallPositions.clear();
    allBallPositions.reserve(specBallPositions_px.size());

    validBallPositions.clear();
    validBallPositions.reserve(specBallPositions_px.size());

    validBallROIs.clear();
    validBallROIs.reserve(specBallPositions_px.size());

    const long totalBallCount = (long)specBallPositions_px.size();

    for (long idx = 0; idx < totalBallCount; idx++)
    {
        const float t_x = (float)(specBallPositions_px[idx].m_x * affineTransformSpecToReal[0][0]
            + specBallPositions_px[idx].m_y * affineTransformSpecToReal[0][1] + affineTransformSpecToReal[0][2]);
        const float t_y = (float)(specBallPositions_px[idx].m_x * affineTransformSpecToReal[1][0]
            + specBallPositions_px[idx].m_y * affineTransformSpecToReal[1][1] + affineTransformSpecToReal[1][2]);

        Ipvm::Point32r2 pt = {t_x, t_y};

        allBallPositions.push_back(pt);

        if (m_packageSpec.m_ballMapAll->m_balls[idx].m_ignored)
        {
            continue;
        }

        validBallPositions.push_back(pt);

        const float radius_x = specBallRadiusX_px[idx];
        const float radius_y = specBallRadiusY_px[idx];

        validBallROIs.emplace_back(int32_t(t_x - radius_x + 0.5f), int32_t(t_y - radius_y + 0.5f),
            int32_t(t_x + radius_x + 1.5f), int32_t(t_y + radius_y + 1.5f));
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::GatherBallZ(const std::vector<Ipvm::Point32r2>& alignedBallPositions,
    const std::vector<Ipvm::Point32r3>& srPosData, enumGetherBallZType type,
    std::vector<Ipvm::Point32r3>& vec3DObjectPos, Ipvm::Image8u& o_ballLogicalPixelImage)
{
    //const long imageSizeX = getImageLotInsp().m_zmapImage.GetSizeX();
    //const long imageSizeY = getImageLotInsp().m_zmapImage.GetSizeY();

    if (!getReusableMemory().GetInspByteImage(o_ballLogicalPixelImage))
        return FALSE;
    o_ballLogicalPixelImage.FillZero();

    vec3DObjectPos.clear();
    vec3DObjectPos.reserve(m_packageSpec.m_ballMap->m_balls.size());

    std::vector<Ipvm::Point32r2> tempData;

    Ipvm::Image8u phygicalPixelMask;
    if (!getReusableMemory().GetInspByteImage(phygicalPixelMask))
        return FALSE;

    if (Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(phygicalPixelMask), 0, phygicalPixelMask) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    auto& algoParas = m_pVisionPara->m_parameters;

    auto zmap = getImageLotInsp().m_zmapImage;
    auto vmap = getImageLotInsp().m_vmapImage;

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        auto para_ballZ = algoParas[ball.m_typeIndex].m_paraBallZ[long(type)];

        const float fBallCenterX_px = alignedBallPositions[ball.m_index].m_x;
        const float fBallCenterY_px = alignedBallPositions[ball.m_index].m_y;

        const float fBallRadiusX_px = (float)(0.01f * para_ballZ.m_calcAreaPercent * ball.m_radiusX_px);
        const float fBallRadiusY_px = (float)(0.01f * para_ballZ.m_calcAreaPercent * ball.m_radiusY_px);

        const float fHeightThreshold
            = (float)(0.01f * para_ballZ.m_validPixelHeightThreshold_Percent * ball.m_height_um);

        Ipvm::EllipseEq32r ellipse = {fBallCenterX_px, fBallCenterY_px, fBallRadiusX_px, fBallRadiusY_px};

        const Ipvm::Rect32s rtObject(int32_t(fBallCenterX_px - fBallRadiusX_px - 0.5f),
            int32_t(fBallCenterY_px - fBallRadiusY_px - 0.5f), int32_t(fBallCenterX_px + fBallRadiusX_px + 2.5f),
            int32_t(fBallCenterY_px + fBallRadiusY_px + 2.5f));

        MakeBallPhysicalPixel(alignedBallPositions[ball.m_index], fBallRadiusX_px, fBallRadiusY_px, phygicalPixelMask);

        float pixelPersent = float(min(100.f, max(0.f, para_ballZ.m_topPercentAverage)));
        long maxValidCount = GetMaxValidPointCount(ball.m_index, type);

        float fObjectZ = CalcBallZ(zmap, vmap, rtObject, pixelPersent, maxValidCount, srPosData[ball.m_index].m_z,
            fHeightThreshold, phygicalPixelMask, o_ballLogicalPixelImage);

        vec3DObjectPos.push_back(Ipvm::Point32r3(fBallCenterX_px, fBallCenterY_px, fObjectZ));
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::GatherSrZ(const std::vector<Ipvm::Point32r2>& allBallPositions,
    std::vector<Ipvm::Rect32s>& validSubstrateROIs, std::vector<Ipvm::Point32r3>& validSubstrate3DData,
    Ipvm::Image8u& o_substrateMaskImage)
{
    validSubstrateROIs.clear();
    validSubstrate3DData.clear();

    validSubstrateROIs.reserve(m_packageSpec.m_ballMap->m_balls.size());
    validSubstrate3DData.reserve(m_packageSpec.m_ballMap->m_balls.size());

    //const long imageSizeX = getImageLotInsp().m_zmapImage.GetSizeX();
    //const long imageSizeY = getImageLotInsp().m_zmapImage.GetSizeY();

    if (!getReusableMemory().GetInspByteImage(o_substrateMaskImage))
        return FALSE;
    o_substrateMaskImage.FillZero();

    // Mask 생성
    Ipvm::Quadrangle32r srtPackageBody(ConversionEx::ToQuadrangle32r(
        m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB));
    Ipvm::Rect32s roiPackage(Ipvm::Conversion::ToRect32s(srtPackageBody));

    // Z-Map 으로 유효 데이터 픽셀로 추가 마스크를 만든 다음에
    // 이 녀석들을 2 px 정도 Erosion 한 후에 원래 마스크에 AND 시키자.
    // 데이터 없는 부분 근처에서 번지는 현상 때문임. 이현민.

    float boundScale = 65535 / 255.f;
    USHORT lowerBound = (USHORT)min(USHRT_MAX, max(0, m_pVisionPara->m_visibilityLowerBound_sr * boundScale));

    for (long y = roiPackage.m_top; y < roiPackage.m_bottom; y++)
    {
        const auto* zmap = getImageLotInsp().m_zmapImage.GetMem(0, y);
        const auto* vmap = getImageLotInsp().m_vmapImage.GetMem(0, y);

        BYTE* mask = o_substrateMaskImage.GetMem(0, y);

        for (long x = roiPackage.m_left; x < roiPackage.m_right; x++)
        {
            if (zmap[x] != Ipvm::k_noiseValue32r && vmap[x] > lowerBound)
            {
                mask[x] = 255;
            }
        }
    }

    Ipvm::Rect32s roiImage(getImageLotInsp().GetImageRect());

    auto& algoParas = m_pVisionPara->m_parameters;

    // Ball 영역 실제 마스킹
    for (const auto& ball : m_packageSpec.m_ballMapAll->m_balls)
    {
        const float fBallRadiusX_px = (float)ball.m_radiusX_px;
        const float fBallRadiusY_px = (float)ball.m_radiusY_px;

        const auto& algoPara = algoParas[ball.m_typeIndex];

        const float fIgnoreRadiusX_px = (float)(0.01f * algoPara.m_fBallIgnorePercent * fBallRadiusX_px);
        const float fIgnoreRadiusY_px = (float)(0.01f * algoPara.m_fBallIgnorePercent * fBallRadiusY_px);

        const float fIgnoreExpandX_px = (float)(0.01f * algoPara.m_fBallIgnoreExpandPercent * fBallRadiusX_px);

        const float fBallCenterX_px = allBallPositions[ball.m_index].m_x;
        const float fBallCenterY_px = allBallPositions[ball.m_index].m_y;

        // Left ellipse
        const float ellipse_left_x = fBallCenterX_px - fIgnoreExpandX_px;

        Ipvm::EllipseEq32r ellipse_left = {ellipse_left_x, fBallCenterY_px, fIgnoreRadiusX_px, fIgnoreRadiusY_px};

        Ipvm::ImageProcessing::Fill(ellipse_left, 0, o_substrateMaskImage);

        // Right ellipse
        const float ellipse_right_x = fBallCenterX_px + fIgnoreExpandX_px;

        Ipvm::EllipseEq32r ellipse_right = {ellipse_right_x, fBallCenterY_px, fIgnoreRadiusX_px, fIgnoreRadiusY_px};

        Ipvm::ImageProcessing::Fill(ellipse_right, 0, o_substrateMaskImage);

        // center rectangle
        Ipvm::Rect32s centerBox{int32_t(::floor(ellipse_left_x)), int32_t(::floor(fBallCenterY_px - fIgnoreRadiusY_px)),
            int32_t(::ceil(ellipse_right_x)) + 1, int32_t(::ceil(fBallCenterY_px + fIgnoreRadiusY_px)) + 1};

        Ipvm::ImageProcessing::Fill(centerBox, 0, o_substrateMaskImage);

        if (ball.m_ignored)
        {
            continue;
        }

        // SR 계산 영역의 크기는 Ball 지름의 3 배로 해 보자.
        const float fSrRadiusX_px = (float)(0.01f * algoPara.m_fSRCalcAreaPercent * fBallRadiusX_px);
        const float fSrRadiusY_px = (float)(0.01f * algoPara.m_fSRCalcAreaPercent * fBallRadiusY_px);

        Ipvm::Rect32s roi((int)(fBallCenterX_px - fSrRadiusX_px + 0.5f), (int)(fBallCenterY_px - fSrRadiusY_px + 0.5f),
            (int)(fBallCenterX_px + fSrRadiusX_px + 1.5f), (int)(fBallCenterY_px + fSrRadiusY_px + 1.5f));

        roi &= roiImage;

        validSubstrateROIs.push_back(roi);
        validSubstrate3DData.emplace_back(fBallCenterX_px, fBallCenterY_px, Ipvm::k_noiseValue32r);
    }

    Ipvm::Image32r tempBuffer;
    getReusableMemory().GetInspFloatImage(tempBuffer);

    //{{//kircheis_IgnoreEdge
    Ipvm::Rect32s rtMaskFull;
    Ipvm::Image8u SrEdgeMaskImage;
    if (!getReusableMemory().GetInspByteImage(SrEdgeMaskImage))
        return FALSE;
    rtMaskFull = Ipvm::Rect32s(SrEdgeMaskImage);
    Ipvm::ImageProcessing::Fill(rtMaskFull, 0, SrEdgeMaskImage);
    Ipvm::Quadrangle32r qrtBody;
    GetSrIgnoreQRT(qrtBody);

    Ipvm::ImageProcessing::Fill(qrtBody, 255, SrEdgeMaskImage);
    Ipvm::ImageProcessing::BitwiseAnd(SrEdgeMaskImage, rtMaskFull, o_substrateMaskImage);
    //}}

    for (long idx = 0; idx < validSubstrateROIs.size(); idx++)
    {
        validSubstrate3DData[idx].m_z = CalcSrZ(validSubstrateROIs[idx], o_substrateMaskImage, tempBuffer.GetMem());
    }

    return TRUE;
}

void VisionInspectionBgaBottom3D::GetSrIgnoreQRT(Ipvm::Quadrangle32r& o_qrtValidSR)
{
    Ipvm::Quadrangle32r qrtBody;
    qrtBody = ConversionEx::ToQuadrangle32r(
        m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB);

    float fOffsetLeftUM = (float)m_pVisionPara->m_SR_IgnoreOffsetXum;
    float fOffsetTopUM = (float)m_pVisionPara->m_SR_IgnoreOffsetYum;
    float fOffsetRightUM = (float)m_pVisionPara->m_SR_IgnoreOffsetXum;
    float fOffsetBottomUM = (float)m_pVisionPara->m_SR_IgnoreOffsetYum;

    const float fScaleX = getScale().pixelToUm().m_x;
    const float fScaleY = getScale().pixelToUm().m_y;

    const float fOffsetLeftPXL = fOffsetLeftUM / fScaleX;
    const float fOffsetTopPXL = fOffsetTopUM / fScaleY;
    const float fOffsetRightPXL = fOffsetRightUM / fScaleX;
    const float fOffsetBottomPXL = fOffsetBottomUM / fScaleY;

    qrtBody.m_ltX += fOffsetLeftPXL;
    qrtBody.m_ltY += fOffsetTopPXL;
    qrtBody.m_rtX -= fOffsetRightPXL;
    qrtBody.m_rtY += fOffsetTopPXL;
    qrtBody.m_lbX += fOffsetLeftPXL;
    qrtBody.m_lbY -= fOffsetBottomPXL;
    qrtBody.m_rbX -= fOffsetRightPXL;
    qrtBody.m_rbY -= fOffsetBottomPXL;

    Ipvm::Rect32s roiImage(getImageLotInsp().GetImageRect());

    qrtBody.m_ltX = (float)max(0.f, (float)min((float)roiImage.m_right - 1.f, qrtBody.m_ltX));
    qrtBody.m_ltY = (float)max(0.f, (float)min((float)roiImage.m_bottom - 1.f, qrtBody.m_ltY));
    qrtBody.m_rtX = (float)max(0.f, (float)min((float)roiImage.m_right - 1.f, qrtBody.m_rtX));
    qrtBody.m_rtY = (float)max(0.f, (float)min((float)roiImage.m_bottom - 1.f, qrtBody.m_rtY));
    qrtBody.m_lbX = (float)max(0.f, (float)min((float)roiImage.m_right - 1.f, qrtBody.m_lbX));
    qrtBody.m_lbY = (float)max(0.f, (float)min((float)roiImage.m_bottom - 1.f, qrtBody.m_lbY));
    qrtBody.m_rbX = (float)max(0.f, (float)min((float)roiImage.m_right - 1.f, qrtBody.m_rbX));
    qrtBody.m_rbY = (float)max(0.f, (float)min((float)roiImage.m_bottom - 1.f, qrtBody.m_rbY));

    o_qrtValidSR = qrtBody;
}

float VisionInspectionBgaBottom3D::CalcSrZ(
    const Ipvm::Rect32s& rtROI, const Ipvm::Image8u& maskImage, float* tempBuffer)
{
    if (m_pVisionPara->m_nSR_Algorithm == 1) //kircheis_Tan
        return CalcSrZ_SortAvr(rtROI, maskImage, tempBuffer);

    const long allPixelCount = rtROI.Width() * rtROI.Height();
    const long desiredMaxPixelCount = 4000;

    // 소팅시 계산량을 줄이기 위해서 샘플링을 시도한다.
    const long samplingStep = max(1, long(sqrt((allPixelCount + desiredMaxPixelCount - 1) / desiredMaxPixelCount)));

    long dataCount = 0;

    Ipvm::Point32r3* points = (Ipvm::Point32r3*)tempBuffer;
    const float px2um_x = getScale().pixelToUm().m_x;
    const float px2um_y = getScale().pixelToUm().m_y;

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y += samplingStep)
    {
        const float* zmap = getImageLotInsp().m_zmapImage.GetMem(0, y);
        const BYTE* mask = maskImage.GetMem(0, y);

        const float y_um = y * px2um_y;

        for (long x = rtROI.m_left; x < rtROI.m_right; x += samplingStep)
        {
            // 마스크에 이미 노이즈는 고려되어 있으므로 노이즈 체크는 하지 않는다.
            if (mask[x] != 255) // && zmap[x] != Ipvm::k_noiseValue32r)
            {
                continue;
            }

            points[dataCount].m_x = x * px2um_x;
            points[dataCount].m_y = y_um;
            points[dataCount].m_z = zmap[x];
            dataCount++;
        }
    }

    if (dataCount < 3)
    {
        return Ipvm::k_noiseValue32r;
    }

    Ipvm::PlaneEq64r planeEq = {0., 0., 0., 0.};

    if (Ipvm::Status::e_ok != Ipvm::DataFitting::FitToPlane(dataCount, points, planeEq))
    {
        return Ipvm::k_noiseValue32r;
    }

    //Ipvm::Point32r3 *points_new = points + dataCount;

    //long newDataCount = 0;

    //for (long idx = 0; idx < dataCount; idx++)
    //{
    //	points_new[newDataCount] = points[idx];
    //	points_new[newDataCount].m_z = planeEq.m_a * points[idx].m_x + planeEq.m_b * points[idx].m_y + planeEq.m_c * points[idx].m_z + planeEq.m_d;
    //	newDataCount++;

    //}

    const float centerx_um = 0.5f * (rtROI.m_right - 1 + rtROI.m_left) * px2um_x;
    const float centery_um = 0.5f * (rtROI.m_bottom - 1 + rtROI.m_top) * px2um_y;

    return CAST_FLOAT(planeEq.GetPositionZ((double)centerx_um, (double)centery_um));
}

float VisionInspectionBgaBottom3D::CalcSrZ_SortAvr(
    const Ipvm::Rect32s& rtROI, const Ipvm::Image8u& maskImage, float* tempBuffer) //kircheis_Tan
{
    UNREFERENCED_PARAMETER(tempBuffer);

    std::vector<float> vecfZMapData;

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        const float* zmap = getImageLotInsp().m_zmapImage.GetMem(0, y);
        const BYTE* mask = maskImage.GetMem(0, y);

        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            // 마스크에 이미 노이즈는 고려되어 있으므로 노이즈 체크는 하지 않는다.
            if (mask[x] != 255) // && zmap[x] != Ipvm::k_noiseValue32r)
            {
                continue;
            }
            vecfZMapData.push_back(zmap[x]);
        }
    }

    if (vecfZMapData.size() < 3)
    {
        return Ipvm::k_noiseValue32r;
    }

    // TODO: std::sort() 함수는 느리므로, std::nth_element() 함수 조합으로 변경하는 것을 고려해야 함.
    std::sort(vecfZMapData.begin(), vecfZMapData.end(), std::less<float>());

    // 평균 계산
    const long calcCutoffMin = (long)max(
        0, ((float)vecfZMapData.size() * (m_pVisionPara->m_SR_topPercentAverage_Min / 100.f))); //kircheis_Tan
    const long calcCutoffMax = (long)min(vecfZMapData.size(),
        ((float)vecfZMapData.size() * (m_pVisionPara->m_SR_topPercentAverage_Max / 100.f))); //kircheis_Tan
    long calcCount = 0;

    double sum = 0.;
    const float* data = &vecfZMapData[0];

    for (long idx = calcCutoffMin; idx < calcCutoffMax; idx++)
    {
        sum += data[idx];
        calcCount++;
    }

    if (calcCount <= 5)
    {
        // Z값을 계산할 수 없다
        return Ipvm::k_noiseValue32r;
    }

    return float(sum / calcCount);
}

float VisionInspectionBgaBottom3D::CalcBallZ(const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap,
    const Ipvm::Rect32s& rtROI, const float pixelPercent, long maxValidCount, const float srZPos,
    const float heightThreshold, const Ipvm::Image8u& phygicalPixelMask, Ipvm::Image8u& o_logicalPixel)
{
    if (srZPos == Ipvm::k_noiseValue32r)
    {
        return Ipvm::k_noiseValue32r;
    }

    MakeLogicalPixelMask_Ball(rtROI, vmap, phygicalPixelMask, o_logicalPixel);

    // Top Percent Average
    struct SZInfo
    {
        float m_z;
        BYTE m_logicalPixel;

        SZInfo() = default;
        SZInfo(const float z, BYTE logicalPixel)
            : m_z(z)
            , m_logicalPixel(logicalPixel)
        {
        }
    };

    const float filteringHeight_um = srZPos + heightThreshold;

    std::vector<SZInfo> validZList;
    float neighbor[8];

    //{{//kircheis_loss3D
    const BOOL bUseInvalidateSinglePoint = (m_pVisionPara->m_nInvalidateSinglePointData == INVALIDATE_SINGLE_POINT);
    long nValidDataCount = 0;
    //}}

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        const float* zmap_my = zmap.GetMem(0, y - 1);
        const float* zmap_ny = zmap.GetMem(0, y);
        const float* zmap_py = zmap.GetMem(0, y + 1);
        auto* weight_y = o_logicalPixel.GetMem(0, y);

        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            auto& z_value = zmap_ny[x];

            if (z_value == Ipvm::k_noiseValue32r || weight_y[x] == 0)
            {
                continue;
            }

            neighbor[0] = zmap_my[x - 1];
            neighbor[1] = zmap_my[x];
            neighbor[2] = zmap_my[x + 1];
            neighbor[3] = zmap_ny[x - 1];
            neighbor[4] = zmap_ny[x + 1];
            neighbor[5] = zmap_py[x - 1];
            neighbor[6] = zmap_py[x];
            neighbor[7] = zmap_py[x + 1];

            float minDiff = FLT_MAX;
            nValidDataCount = 0; //kircheis_loss3D

            for (int i = 0; i < 8; i++)
            {
                if (neighbor[i] == Ipvm::k_noiseValue32r)
                {
                    continue;
                }

                float diff = CAST_FLOAT(fabs(z_value - neighbor[i]));
                nValidDataCount++; //kircheis_loss3D
                if (minDiff > diff)
                {
                    minDiff = diff;
                }
            }

            if ((z_value >= filteringHeight_um && minDiff < 5.f)
                || (!bUseInvalidateSinglePoint && nValidDataCount == 0)) //kircheis_loss3D
            {
                validZList.emplace_back(z_value, weight_y[x]);
            }
            else
            {
                // 참여하지 않는 영역을 확인하기 위해 Weight Factor를 제거하여
                // Debug Info에서 볼 수 있게 하자
                weight_y[x] = 0;
            }
        }
    }

    if (validZList.size() == 0)
    {
        return Ipvm::k_noiseValue32r;
    }
    else if (validZList.size() == 1) //kircheis_loss3D
    {
        return validZList[0].m_z;
    }

    std::sort(validZList.begin(), validZList.end(),
        [](const SZInfo& a, const SZInfo& b)
        {
            return a.m_z > b.m_z;
        });

    //{{//Noise Cutting //kircheis_Ball3dNoise
    const float fGroupingGap = getScale().pixelToUmZ() * 1.5f;
    const long nDataNum = (long)validZList.size();
    std::vector<SZInfo> vecSortZList(0);
    std::vector<std::vector<SZInfo>> vec2SortZList(0);

    vecSortZList.push_back(validZList[0]);

    for (long nID = 1; nID < nDataNum; nID++)
    {
        if ((validZList[nID - 1].m_z - validZList[nID].m_z) > fGroupingGap)
        {
            vec2SortZList.push_back(vecSortZList);
            vecSortZList.clear();
            vecSortZList.push_back(validZList[nID]);
        }
        else
            vecSortZList.push_back(validZList[nID]);
    }
    vec2SortZList.push_back(vecSortZList);
    vecSortZList.clear();

    if (vec2SortZList.size() > 1)
    {
        validZList.clear();
        for (long nID = 0; nID < vec2SortZList.size(); nID++)
        {
            if (validZList.size() < vec2SortZList[nID].size())
            {
                validZList.clear();
                validZList.insert(validZList.begin(), vec2SortZList[nID].begin(), vec2SortZList[nID].end());
            }
        }
    }
    vec2SortZList.clear();
    //}}

    // 전체 Logical Pixel 수를 센다.
    long totalLogicalPixelCount = 0;

    for (const auto& d : validZList)
    {
        totalLogicalPixelCount += d.m_logicalPixel;
    }

    long pixelCountLimit = min(maxValidCount * 255, long(totalLogicalPixelCount * pixelPercent / 100.f + 0.5f));

    // Logical Pixel 수를 세어서 필요한 만큼 까지만 평균을 구하려고 한다.
    long dataCount = long(validZList.size());
    long pixelCount = 0;
    long lastPixelIndex = 0;

    for (long idx = 0; idx < dataCount; idx++)
    {
        auto& d = validZList[idx];

        pixelCount += d.m_logicalPixel;

        if (pixelCount >= pixelCountLimit)
        {
            const long residue = pixelCount - pixelCountLimit;

            if (residue > 0)
            {
                d.m_logicalPixel -= (BYTE)residue;
            }

            lastPixelIndex = idx;
            break;
        }
    }

    double weightedSum = 0.;
    double weightSum = 0.;

    for (long index = 0; index < lastPixelIndex + 1; index++)
    {
        const auto weight = validZList[index].m_logicalPixel;

        weightedSum += validZList[index].m_z * weight;
        weightSum += weight;
    }

    if (weightSum > 0)
    {
        return float(weightedSum / weightSum);
    }
    else
    {
        return Ipvm::k_noiseValue32r;
    }
}

BOOL VisionInspectionBgaBottom3D::InspBallCoplanarity(const BOOL ReadDataType,
    const std::vector<Package::Ball> vecBallMapData, const std::vector<Ipvm::Point32r3>& vecObjectPos,
    const std::vector<Ipvm::Rect32s>& vecrtObject, const LPCTSTR strCoplInspName, const LPCTSTR strUnitCoplInspName,
    const CString strCompName) //kircheis_MED2.5
{
    const long objectCount = long(vecBallMapData.size());
    std::vector<float> vecfCopl_LSF(objectCount);

    const BOOL bDeadBug = m_packageSpec.m_deadBug;

    if (!Coplanarity_Algorithm::CalcReferencePlaneDeviation(vecObjectPos, getScale().pixelToUm().m_x,
            getScale().pixelToUm().m_y, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, vecfCopl_LSF, bDeadBug))
    {
        return FALSE;
    }

    if (ReadDataType)
    {
        auto* pResult = m_resultGroup.GetResultByName(strCoplInspName);
        if (pResult == NULL)
            return FALSE;
        auto* pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == NULL)
            return FALSE;
        BOOL bUseCopl = pSpec->m_use;
        pResult->Clear();
        if (bUseCopl)
            pResult->Resize(objectCount);

        auto* pUnitResult = m_resultGroup.GetResultByName(strUnitCoplInspName);
        if (pUnitResult == NULL)
            return FALSE;
        auto* pUnitSpec = GetSpecByName(pUnitResult->m_resultName);
        if (pUnitSpec == NULL)
            return FALSE;

        pUnitResult->Clear();
        BOOL bUseUnitCopl = pUnitSpec->m_use;
        if (bUseUnitCopl)
            pUnitResult->Resize(1);

        //여기 값을 기존 Copl의 Result에서도 뽑을수는 있는데 기존 Copl을 안쓰고 Unit Copl만 사용할 수도 있으니까 별도로 계산한다.
        float fMin = FLT_MAX;
        float fMax = FLT_MIN;
        float fWorst = FLT_MIN;
        float fABS = 0.f;
        Ipvm::Rect32s rtWorstPos{};
        CString strWorstID;

        long idx = 0;
        //{{ //kircheis_AIBC
        //BOOL bIsAllGroupInsp = (vecBallMapData.size() == m_packageSpec.m_ballMap->m_balls.size());
        long nBallGroupNum = (long)m_packageSpec.m_originalballMap->m_ballTypes.size();
        std::vector<long> vecnInvalidBallCnt(nBallGroupNum);
        std::vector<long> vecnInvalidResult(nBallGroupNum);
        for (auto& nCnt : vecnInvalidBallCnt)
            nCnt = 0;
        for (auto& nResult : vecnInvalidResult)
            nResult = REJECT;

        CString strInvalidBallInfo;
        for (const auto& ball : vecBallMapData)
        {
            if (vecfCopl_LSF[idx] != Ipvm::k_noiseValue32r)
            {
                idx++;
                continue;
            }

            strInvalidBallInfo.Format(_T("Group ID : [%s] , Ball Index : [%d] , Ball ID : [%s]"),
                (LPCTSTR)ball.m_groupID, ball.m_index, (LPCTSTR)ball.m_name);
            SetInvalidBallInfo(strInvalidBallInfo, m_vecstrInvalidBallInfo_Copl);
            vecnInvalidBallCnt[ball.m_typeIndex]++;
            idx++;
        }

        if (nBallGroupNum == m_pVisionPara->m_parameters.size())
        {
            for (long nCurGroup = 0; nCurGroup < nBallGroupNum; nCurGroup++)
            {
                if (vecnInvalidBallCnt[nCurGroup] <= m_pVisionPara->m_parameters[nCurGroup]
                        .m_paraBallZ[(long)enumGetherBallZType::Copl]
                        .m_dAcceptableInvalidBallCount)
                    vecnInvalidResult[nCurGroup] = PASS;
            }
        }

        long nNoiseUnitResult = 0;
        //}}

        idx = 0;

        for (const auto& ball : vecBallMapData)
        {
            float fCopl = vecfCopl_LSF[idx];

            if (bUseCopl)
            {
                if (fCopl == Ipvm::k_noiseValue32r) //kircheis_AIBC
                    pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID,
                        strCompName, fCopl, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um, 0.f,
                        vecnInvalidResult[ball.m_typeIndex]);
                else
                    pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID,
                        strCompName, fCopl, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um); //kircheis_MED2.5
                pResult->SetRect(idx, vecrtObject[idx]);
            }

            if (bUseUnitCopl) // && !(fCopl == Ipvm::k_noiseValue32r && vecnInvalidResult[ball.m_typeIndex] == PASS))//kircheis_AIBC
            {
                fMin = (float)min(fMin, fCopl);
                fMax = (float)max(fMax, fCopl);
                fABS = (float)fabs(fCopl);
                if (fABS > fWorst)
                {
                    fWorst = fABS;
                    rtWorstPos = vecrtObject[idx];
                    strWorstID = ball.m_name;
                }

                if (fCopl == Ipvm::k_noiseValue32r) //kircheis_AIBC
                {
                    nNoiseUnitResult = (long)max(nNoiseUnitResult, vecnInvalidResult[ball.m_typeIndex]);
                }
            }

            idx++;
        }

        if (bUseCopl)
        {
            pResult->UpdateTypicalValue(pSpec);
            pResult->SetTotalResult();
        }

        if (bUseUnitCopl)
        {
            if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
            {
                pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(0, strWorstID, strCompName, strCompName,
                    Ipvm::k_noiseValue32r, *pUnitSpec, 0.f, 0.f, 0.f,
                    nNoiseUnitResult); //kircheis_MED2.5//kircheis_AIBC
            }
            else
            {
                pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, fMax - fMin, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
            }
            pUnitResult->SetRect(0, rtWorstPos);
            pUnitResult->UpdateTypicalValue(pUnitSpec);
            pUnitResult->SetTotalResult();
        }

        strWorstID.Empty();
        strInvalidBallInfo.Empty();
    }
    else
    {
        auto* pUnitResult = m_resultGroup.GetResultByName(strUnitCoplInspName);
        if (pUnitResult == NULL)
            return FALSE;
        auto* pUnitSpec = GetSpecByName(pUnitResult->m_resultName);
        if (pUnitSpec == NULL)
            return FALSE;

        pUnitResult->Clear();
        BOOL bUseUnitCopl = pUnitSpec->m_use;
        if (bUseUnitCopl)
            pUnitResult->Resize(1);

        //여기 값을 기존 Copl의 Result에서도 뽑을수는 있는데 기존 Copl을 안쓰고 Unit Copl만 사용할 수도 있으니까 별도로 계산한다.
        float fMin = FLT_MAX;
        float fMax = FLT_MIN;
        float fWorst = FLT_MIN;
        float fABS = 0.f;
        Ipvm::Rect32s rtWorstPos{};
        CString strWorstID;

        long idx = 0;

        for (const auto& ball : vecBallMapData)
        {
            float fCopl = vecfCopl_LSF[idx];

            if (bUseUnitCopl)
            {
                fMin = (float)min(fMin, fCopl);
                fMax = (float)max(fMax, fCopl);
                fABS = (float)fabs(fCopl);
                if (fABS > fWorst)
                {
                    fWorst = fABS;
                    rtWorstPos = vecrtObject[idx];
                    strWorstID = ball.m_name;
                }
            }

            idx++;
        }

        if (bUseUnitCopl)
        {
            if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
            {
                pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(0, strWorstID, strCompName, strCompName,
                    Ipvm::k_noiseValue32r, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
            }
            else
            {
                pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, fMax - fMin, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
            }
            pUnitResult->SetRect(0, rtWorstPos);
            pUnitResult->UpdateTypicalValue(pUnitSpec);
            pUnitResult->SetTotalResult();
        }

        strWorstID.Empty();
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::InspBallCoplanarity(
    const std::vector<Ipvm::Point32r3>& vecObjectPos, const std::vector<Ipvm::Rect32s>& vecrtObject)
{
    //All Object 구간
    CString strCompID
        = m_packageSpec.m_originalballMap
              ->m_strAllGroupName; //kircheis_MED2.5 // 이 검사에 소속된 모든 Obj의 전체 그룹 ID를 몰아 넣는다.

    if (!InspBallCoplanarity(TRUE, m_packageSpec.m_ballMap->m_balls, vecObjectPos, vecrtObject,
            g_szBGA3DInspectionName[_3DINSP_COPL], g_szBGA3DInspectionName[_3DINSP_UNIT_COPL],
            strCompID)) //kircheis_MED2.5
    {
        return FALSE;
    }

    //Group별 Data 분리 구간
    long ObjectCount = (long)m_packageSpec.m_ballMap->m_balls.size();
    long GroupCount = (long)m_packageSpec.m_originalballMap->m_ballTypes.size();

    std::vector<std::vector<Package::Ball>> vecGroupBallData;
    std::vector<std::vector<Ipvm::Point32r3>> vecObjectPosition;
    std::vector<std::vector<Ipvm::Rect32s>> vecObject;

    vecGroupBallData.resize(GroupCount);
    vecObjectPosition.resize(GroupCount);
    vecObject.resize(GroupCount);

    for (int idx = 0; idx < ObjectCount; idx++)
    {
        long GroupIdx = m_packageSpec.m_ballMap->m_balls[idx].m_typeIndex;

        vecGroupBallData[GroupIdx].push_back(m_packageSpec.m_ballMap->m_balls[idx]);
        vecObjectPosition[GroupIdx].push_back(vecObjectPos[idx]);
        vecObject[GroupIdx].push_back(vecrtObject[idx]);
    }

    //Group Object 구간
    long GroupSpecCount = (long)m_pVisionPara->m_vecstrGroupInspName.size();

    if (GroupSpecCount != 0 && GroupSpecCount != GroupCount)
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        strCompID = m_packageSpec.m_originalballMap->m_ballTypes[idx].m_groupID; //kircheis_MED2.5
        if (!InspBallCoplanarity(TRUE, vecGroupBallData[idx], vecObjectPosition[idx], vecObject[idx],
                m_pVisionPara->m_vecstrGroupInspName[idx].BallCoplName,
                m_pVisionPara->m_vecstrGroupInspName[idx].BallUnitCoplName, strCompID)) //kircheis_MED2.5
        {
            return FALSE;
        }
    }

    //Group Copl 구간
    long SpecCount = (long)m_pVisionPara->m_vec2UnitCoplGroup.size();
    strCompID.Empty(); //kircheis_MED2.5

    for (int idx = 0; idx < SpecCount; idx++)
    {
        std::vector<Package::Ball> BallData;
        std::vector<Ipvm::Point32r3> ObjectPos;
        std::vector<Ipvm::Rect32s> ObjectRoi;

        long GroupUnitCount = (long)m_pVisionPara->m_vec2UnitCoplGroup[idx].size();

        for (int i = 0; i < GroupUnitCount; i++)
        {
            long selectedidx = m_pVisionPara->m_vec2UnitCoplGroup[idx][i];

            //만약 저장된 값보다 Layer 갯수가 작을경우 Invalid 처리한다. (Index = 5, LayerNum = 3 일경우)
            if (GroupCount <= selectedidx)
            {
                m_bInvalid = TRUE;
                return FALSE;
            }

            BallData.insert(
                BallData.begin(), vecGroupBallData[selectedidx].begin(), vecGroupBallData[selectedidx].end());
            ObjectPos.insert(
                ObjectPos.end(), vecObjectPosition[selectedidx].begin(), vecObjectPosition[selectedidx].end());
            ObjectRoi.insert(ObjectRoi.end(), vecObject[selectedidx].begin(), vecObject[selectedidx].end());

            strCompID += m_packageSpec.m_originalballMap->m_ballTypes[selectedidx].m_groupID; //kircheis_MED2.5
        }

        if (!InspBallCoplanarity(FALSE, BallData, ObjectPos, ObjectRoi, _T(""),
                m_pVisionPara->m_vecstrGroupUnitCoplName[idx], strCompID)) //kircheis_MED2.5
        {
            strCompID.Empty(); //kircheis_MED2.5
            return FALSE;
        }
        strCompID.Empty(); //kircheis_MED2.5
    }

    strCompID.Empty();

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::InspWarpage(const std::vector<Package::Ball> vecBallMapData,
    const std::vector<Ipvm::Point32r3>& vecSrPos, const std::vector<Ipvm::Rect32s>& vecrtSr,
    const LPCTSTR strWarpageInspName, const LPCTSTR strUnitWarpageInspName, const CString strCompName) //kircheis_MED2.5
{
    const long objectCount = long(vecBallMapData.size());

    std::vector<float> vecfWarpage_LSF(objectCount);

    const BOOL bDeadBug = m_packageSpec.m_deadBug;

    if (!Coplanarity_Algorithm::CalcReferencePlaneDeviation(vecSrPos, getScale().pixelToUm().m_x,
            getScale().pixelToUm().m_y, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, vecfWarpage_LSF, bDeadBug))
    {
        return FALSE;
    }

    auto* pResult = m_resultGroup.GetResultByName(strWarpageInspName);
    if (pResult == NULL)
        return FALSE;
    auto* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;
    BOOL bUseWarpage = pSpec->m_use;
    pResult->Clear();
    if (bUseWarpage)
        pResult->Resize(objectCount);

    auto* pUnitResult = m_resultGroup.GetResultByName(strUnitWarpageInspName);
    if (pUnitResult == NULL)
        return FALSE;
    auto* pUnitSpec = GetSpecByName(pUnitResult->m_resultName);
    if (pUnitSpec == NULL)
        return FALSE;
    BOOL bUseUnitWarpage = pUnitSpec->m_use;
    pUnitResult->Clear();
    if (bUseUnitWarpage)
        pUnitResult->Resize(1);

    //여기 값을 기존 Warpage의 Result에서도 뽑을수는 있는데 기존 Warpage를 안쓰고 Unit Warpage만 사용할 수도 있으니까 별도로 계산한다.
    float fMin = FLT_MAX;
    float fMax = FLT_MIN;
    float fWorst = FLT_MIN;
    float fABS = 0.f;
    Ipvm::Rect32s rtWorstPos{};
    CString strWorstID;

    long idx = 0;

    for (const auto& ball : vecBallMapData)
    {
        float fWarpage = vecfWarpage_LSF[idx];

        if (bUseWarpage)
        {
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID, strCompName,
                fWarpage, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um); //kircheis_MED2.5
            pResult->SetRect(idx, vecrtSr[idx]);
        }

        if (bUseUnitWarpage)
        {
            fMin = (float)min(fMin, fWarpage);
            fMax = (float)max(fMax, fWarpage);
            fABS = (float)fabs(fWarpage);
            if (fABS > fWorst)
            {
                fWorst = fABS;
                rtWorstPos = vecrtSr[idx];
                strWorstID = ball.m_name;
            }
        }

        idx++;
    }

    if (bUseWarpage)
    {
        pResult->UpdateTypicalValue(pSpec);
        pResult->SetTotalResult();
    }

    if (bUseUnitWarpage)
    {
        if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
        {
            pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompName, strCompName, Ipvm::k_noiseValue32r, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
        }
        else
        {
            pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompName, strCompName, fMax - fMin, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
        }

        pUnitResult->SetRect(0, rtWorstPos);
        pUnitResult->UpdateTypicalValue(pUnitSpec);
        pUnitResult->SetTotalResult();
    }

    strWorstID.Empty();

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::InspWarpage(const std::vector<Package::Ball> vecBallMapData,
    const std::vector<Ipvm::Point32r3>& vecSrPos, const std::vector<Ipvm::Rect32s>& vecrtSr,
    const LPCTSTR strWarpageInspName, const LPCTSTR strUnitWarpageInspName, const bool detailSetupMode)
{
    const long objectCount = long(vecBallMapData.size());

    std::vector<float> vecfWarpage_LSF(objectCount);

    const BOOL bDeadBug = m_packageSpec.m_deadBug;
    const float fDeadBugGain = bDeadBug ? -1.f : 1.f;

    if (!Coplanarity_Algorithm::CalcReferencePlaneDeviation(vecSrPos, getScale().pixelToUm().m_x,
            getScale().pixelToUm().m_y, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, vecfWarpage_LSF, bDeadBug))
    {
        return FALSE;
    }

    CString strCompID
        = m_packageSpec.m_originalballMap
              ->m_strAllGroupName; //kircheis_MED2.5 // 이 검사에 소속된 모든 Obj의 전체 그룹 ID를 몰아 넣는다.

    // Warpage shape
    {
        ShapeAlgorithm shapeAlgorithm;

        std::vector<Ipvm::Point64r3> dataPoints;
        long idx = 0;

        for (const auto& ball : vecBallMapData)
        {
            float fWarpage = vecfWarpage_LSF[idx];

            if (fWarpage != Ipvm::k_noiseValue32r)
            {
                dataPoints.emplace_back(ball.m_posX_um * fDeadBugGain, ball.m_posY_um, fWarpage);
            }

            idx++;
        }

        CString strSign;
        CString strShape;
        double R;
        double B4_um;
        double B5_um;
        double angle_deg;

        if (shapeAlgorithm.GetShape(dataPoints, strSign, strShape, R, B4_um, B5_um, angle_deg))
        {
            m_visionWarpageShapeResult.SetResult(strSign, strShape, R, B4_um, B5_um, angle_deg);

            auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Warpage Shape Info"));
            if (psDebugInfo != NULL)
            {
                if (psDebugInfo->pData != NULL)
                {
                    psDebugInfo->Reset();
                }
                psDebugInfo->nDataNum = 1;
                VisionWarpageShapeResult* pVisionWarpageShapeResult
                    = new VisionWarpageShapeResult[psDebugInfo->nDataNum];
                for (long i = 0; i < psDebugInfo->nDataNum; i++)
                {
                    pVisionWarpageShapeResult[i].SetResult(m_visionWarpageShapeResult);
                }
                psDebugInfo->pData = pVisionWarpageShapeResult;
            }

            strSign.Empty();
            strShape.Empty();
        }
        else
        {
            m_visionWarpageShapeResult.Init();
        }
    }

    auto* pResult = m_resultGroup.GetResultByName(strWarpageInspName);
    if (pResult == NULL)
        return FALSE;
    auto* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;
    BOOL bUseWarpage = pSpec->m_use;
    pResult->Clear();
    if (bUseWarpage)
        pResult->Resize(objectCount);

    auto* pUnitResult = m_resultGroup.GetResultByName(strUnitWarpageInspName);
    if (pUnitResult == NULL)
        return FALSE;
    auto* pUnitSpec = GetSpecByName(pUnitResult->m_resultName);
    if (pUnitSpec == NULL)
        return FALSE;
    BOOL bUseUnitWarpage = pUnitSpec->m_use;
    pUnitResult->Clear();
    if (bUseUnitWarpage)
        pUnitResult->Resize(1);

    //{{//kircheis_MED2LW
    auto* pUnitResultAX = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE_AX]);
    if (pUnitResult == NULL)
        return FALSE;
    auto* pUnitSpecAX = GetSpecByName(pUnitResultAX->m_resultName);
    if (pUnitSpecAX == NULL)
        return FALSE;
    BOOL bUseUnitWarpageAX = pUnitSpecAX->m_use;
    pUnitResultAX->Clear();
    if (bUseUnitWarpageAX)
        pUnitResultAX->Resize(1);

    auto* pUnitResultAV = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE_AV]);
    if (pUnitResultAV == NULL)
        return FALSE;
    auto* pUnitSpecAV = GetSpecByName(pUnitResultAV->m_resultName);
    if (pUnitSpecAV == NULL)
        return FALSE;
    BOOL bUseUnitWarpageAV = pUnitSpecAV->m_use;
    pUnitResultAV->Clear();
    if (bUseUnitWarpageAV)
        pUnitResultAV->Resize(1);

    auto* pUnitResultAS = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE_AS]);
    if (pUnitResultAS == NULL)
        return FALSE;
    auto* pUnitSpecAS = GetSpecByName(pUnitResultAS->m_resultName);
    if (pUnitSpecAS == NULL)
        return FALSE;
    BOOL bUseUnitWarpageAS = pUnitSpecAS->m_use;
    pUnitResultAS->Clear();
    if (bUseUnitWarpageAS)
        pUnitResultAS->Resize(1);

    auto* pUnitResultAO = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE_AO]);
    if (pUnitResultAO == NULL)
        return FALSE;
    auto* pUnitSpecAO = GetSpecByName(pUnitResultAO->m_resultName);
    if (pUnitSpecAO == NULL)
        return FALSE;
    BOOL bUseUnitWarpageAO = pUnitSpecAO->m_use;
    pUnitResultAO->Clear();
    if (bUseUnitWarpageAO)
        pUnitResultAO->Resize(1);

    BOOL bUseUnitAs
        = (bUseUnitWarpageAX && bUseUnitWarpageAV && bUseUnitWarpageAS && bUseUnitWarpageAO); //kircheis_MED2LW //
    BOOL bUseOrUnitAs
        = (bUseUnitWarpageAX || bUseUnitWarpageAV || bUseUnitWarpageAS || bUseUnitWarpageAO); //kircheis_MED2LW

    if (detailSetupMode && (!bUseUnitAs && bUseOrUnitAs))
    {
        ::SimpleMessage(_T("All items from [Unit Warpage AX] to [Unit Warpage AO] must be on."));
    }
    //}}

    //여기 값을 기존 Warpage의 Result에서도 뽑을수는 있는데 기존 Warpage를 안쓰고 Unit Warpage만 사용할 수도 있으니까 별도로 계산한다.
    float fMin = FLT_MAX;
    float fMax = FLT_MIN;
    float fWorst = FLT_MIN;
    float fABS = 0.f;
    Ipvm::Rect32s rtWorstPos{};
    CString strWorstID;

    long idx = 0;

    for (const auto& ball : vecBallMapData)
    {
        float fWarpage = vecfWarpage_LSF[idx];

        if (bUseWarpage)
        {
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID, strCompID,
                fWarpage, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um); //kircheis_MED2.5
            pResult->SetRect(ball.m_index, vecrtSr[idx]);
        }

        if (bUseUnitWarpage)
        {
            fMin = (float)min(fMin, fWarpage);
            fMax = (float)max(fMax, fWarpage);
            fABS = (float)fabs(fWarpage);
            if (fABS > fWorst)
            {
                fWorst = fABS;
                rtWorstPos = vecrtSr[idx];
                strWorstID = ball.m_name;
            }
        }

        idx++;
    }

    //{{//kircheis_MED2.5
    CString strWarpageShape;
    BOOL bIsSignPlus = (m_visionWarpageShapeResult.m_sign == _T("+"));

    if (bIsSignPlus
        && (m_visionWarpageShapeResult.m_shape == _T("Cylindrical")
            || m_visionWarpageShapeResult.m_shape == _T("Spherical"))) //AX
        strWarpageShape.Format(_T("AX"));
    else if (m_visionWarpageShapeResult.m_shape == _T("Cylindrical")
        || m_visionWarpageShapeResult.m_shape == _T("Spherical")) //AV
        strWarpageShape.Format(_T("AV"));
    else if (m_visionWarpageShapeResult.m_shape == _T("Saddle")) //AS
        strWarpageShape.Format(_T("AS"));
    else //AO
        strWarpageShape.Format(_T("AO"));
    //}}

    if (bUseWarpage)
    {
        pResult->UpdateTypicalValue(pSpec);
        pResult->SetTotalResult();
    }

    if (bUseUnitWarpage)
    {
        if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
        {
            pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(0, strWorstID, strCompID, strWarpageShape,
                Ipvm::k_noiseValue32r, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
        }
        else
        {
            pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompID, strWarpageShape, fMax - fMin, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
        }

        pUnitResult->SetRect(0, rtWorstPos);
        pUnitResult->UpdateTypicalValue(pUnitSpec);
        pUnitResult->SetTotalResult();
    }

    if (bUseUnitAs) //kircheis_MED2.5
    {
        if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
        {
            pUnitResultAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompID, strCompID, Ipvm::k_noiseValue32r, *pUnitSpecAX, 0.f, 0.f);
            pUnitResultAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompID, strCompID, Ipvm::k_noiseValue32r, *pUnitSpecAV, 0.f, 0.f);
            pUnitResultAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompID, strCompID, Ipvm::k_noiseValue32r, *pUnitSpecAS, 0.f, 0.f);
            pUnitResultAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompID, strCompID, Ipvm::k_noiseValue32r, *pUnitSpecAO, 0.f, 0.f);
        }
        else
        {
            bIsSignPlus = (m_visionWarpageShapeResult.m_sign == _T("+"));
            float fValue = (fMax - fMin) * (bIsSignPlus ? 1.f : -1.f);
            if (strWarpageShape == _T("AX")) //AX
            {
                pUnitResultAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, fValue, *pUnitSpecAX, 0.f, 0.f);
                pUnitResultAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAV, 0.f, 0.f, 0.f, PASS);
                pUnitResultAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAS, 0.f, 0.f, 0.f, PASS);
                pUnitResultAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAO, 0.f, 0.f, 0.f, PASS);
            }
            else if (strWarpageShape == _T("AV")) //AV
            {
                pUnitResultAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAX, 0.f, 0.f, 0.f, PASS);
                pUnitResultAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, fValue, *pUnitSpecAV, 0.f, 0.f);
                pUnitResultAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAS, 0.f, 0.f, 0.f, PASS);
                pUnitResultAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAO, 0.f, 0.f, 0.f, PASS);
            }
            else if (strWarpageShape == _T("AS")) //AS
            {
                pUnitResultAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAX, 0.f, 0.f, 0.f, PASS);
                pUnitResultAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAV, 0.f, 0.f, 0.f, PASS);
                pUnitResultAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, fValue, *pUnitSpecAS, 0.f, 0.f);
                pUnitResultAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAO, 0.f, 0.f, 0.f, PASS);
            }
            else //AO
            {
                pUnitResultAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAX, 0.f, 0.f, 0.f, PASS);
                pUnitResultAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAV, 0.f, 0.f, 0.f, PASS);
                pUnitResultAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, 0.f, *pUnitSpecAS, 0.f, 0.f, 0.f, PASS);
                pUnitResultAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompID, strCompID, fValue, *pUnitSpecAO, 0.f, 0.f);
            }
        }

        pUnitResultAX->SetRect(0, rtWorstPos);
        pUnitResultAV->SetRect(0, rtWorstPos);
        pUnitResultAS->SetRect(0, rtWorstPos);
        pUnitResultAO->SetRect(0, rtWorstPos);

        pUnitResultAX->UpdateTypicalValue(pUnitSpecAX);
        pUnitResultAV->UpdateTypicalValue(pUnitSpecAV);
        pUnitResultAS->UpdateTypicalValue(pUnitSpecAS);
        pUnitResultAO->UpdateTypicalValue(pUnitSpecAO);

        pUnitResultAX->SetTotalResult();
        pUnitResultAV->SetTotalResult();
        pUnitResultAS->SetTotalResult();
        pUnitResultAO->SetTotalResult();
    }

    strCompID.Empty();
    strWorstID.Empty();
    strWarpageShape.Empty();

    return TRUE;
}

BOOL VisionInspectionBgaBottom3D::InspWarpage(
    const std::vector<Ipvm::Point32r3>& vecSrPos, const std::vector<Ipvm::Rect32s>& vecrtSr, const bool detailSetupMode)
{
    //All Object 구간
    if (!InspWarpage(m_packageSpec.m_ballMap->m_balls, vecSrPos, vecrtSr, g_szBGA3DInspectionName[_3DINSP_WARPAGE],
            g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE], detailSetupMode))
    {
        return FALSE;
    }

    //Group별 Data 분리 구간
    long ObjectCount = (long)m_packageSpec.m_ballMap->m_balls.size();
    long GroupCount = (long)m_packageSpec.m_originalballMap->m_ballTypes.size();

    std::vector<std::vector<Package::Ball>> vecGroupBallData;
    std::vector<std::vector<Ipvm::Point32r3>> vecSRPosition;
    std::vector<std::vector<Ipvm::Rect32s>> vecObject;

    vecGroupBallData.resize(GroupCount);
    vecSRPosition.resize(GroupCount);
    vecObject.resize(GroupCount);

    for (int idx = 0; idx < ObjectCount; idx++)
    {
        long GroupIdx = m_packageSpec.m_ballMap->m_balls[idx].m_typeIndex;

        vecGroupBallData[GroupIdx].push_back(m_packageSpec.m_ballMap->m_balls[idx]);
        vecSRPosition[GroupIdx].push_back(vecSrPos[idx]);
        vecObject[GroupIdx].push_back(vecrtSr[idx]);
    }

    //Group Object 구간
    long GroupSpecCount = (long)m_pVisionPara->m_vecstrGroupInspName.size();

    if (GroupSpecCount != 0 && GroupSpecCount != GroupCount)
        return TRUE;

    CString strCompName; //kircheis_MED2.5
    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        strCompName = m_packageSpec.m_originalballMap->m_ballTypes[idx].m_groupID; //kircheis_MED2.5
        if (!InspWarpage(vecGroupBallData[idx], vecSRPosition[idx], vecObject[idx],
                m_pVisionPara->m_vecstrGroupInspName[idx].BallWarpageName,
                m_pVisionPara->m_vecstrGroupInspName[idx].BallUnitWarpageName, strCompName)) //kircheis_MED2.5
        {
            strCompName.Empty();
            return FALSE;
        }
    }

    strCompName.Empty();
    return TRUE;
}

bool VisionInspectionBgaBottom3D::CollectBodyAlignResult()
{
    bool available = false;
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
        available = true;
    }

    {
        pData = m_visionUnit.GetVisionDebugInfo(
            _VISION_INSP_GUID_FIDUCIAL_ALIGN, _T("Apply PAD Align to Body ROI"), nDataNum);

        FPI_RECT* psfrtBody;

        if (pData != nullptr && nDataNum > 0)
        {
            psfrtBody = (FPI_RECT*)pData;

            m_sEdgeAlignResult->fptLT = psfrtBody->fptLT;
            m_sEdgeAlignResult->fptRT = psfrtBody->fptRT;
            m_sEdgeAlignResult->fptLB = psfrtBody->fptLB;
            m_sEdgeAlignResult->fptRB = psfrtBody->fptRB;

            available = true;
        }
    }

    return available;
}

void VisionInspectionBgaBottom3D::MakeBallPhysicalPixel(
    const Ipvm::Point32r2& center, float radiusX, float radiusY, Ipvm::Image8u& mask)
{
    long imageSizeX = mask.GetSizeX();
    long imageSizeY = mask.GetSizeY();

    // Fill donut outer
    const float radiusSqr = radiusX * radiusY;

    const float xCen = center.m_x;
    const float yCen = center.m_y;

    const long yBegin = min(imageSizeY, max(0, long(yCen - radiusX)));
    const long yEnd = min(imageSizeY, max(0, long(yCen + radiusX + 2)));
    const long xBegin = min(imageSizeX, max(0, long(xCen - radiusY)));
    const long xEnd = min(imageSizeX, max(0, long(xCen + radiusY + 2)));

    for (long y = yBegin; y < yEnd; y++)
    {
        unsigned char* physicalPixelSize = mask.GetMem(0, y);

        for (long x = xBegin; x < xEnd; x++)
        {
            const float fx1 = x - 0.5f;
            const float fy1 = y - 0.5f;
            const float fx2 = x + 0.5f;
            const float fy2 = y - 0.5f;
            const float fx3 = x + 0.5f;
            const float fy3 = y + 0.5f;
            const float fx4 = x - 0.5f;
            const float fy4 = y + 0.5f;

            const float dr1 = (fx1 - xCen) * (fx1 - xCen) + (fy1 - yCen) * (fy1 - yCen);
            const float dr2 = (fx2 - xCen) * (fx2 - xCen) + (fy2 - yCen) * (fy2 - yCen);
            const float dr3 = (fx3 - xCen) * (fx3 - xCen) + (fy3 - yCen) * (fy3 - yCen);
            const float dr4 = (fx4 - xCen) * (fx4 - xCen) + (fy4 - yCen) * (fy4 - yCen);

            long nInnerPointCount = dr1 <= radiusSqr ? 1 : 0;
            nInnerPointCount += dr2 <= radiusSqr ? 1 : 0;
            nInnerPointCount += dr3 <= radiusSqr ? 1 : 0;
            nInnerPointCount += dr4 <= radiusSqr ? 1 : 0;

            if (nInnerPointCount == 4)
            {
                physicalPixelSize[x] = PHYSICAL_PIXEL_SIZE_MAX;
            }
            else if (nInnerPointCount > 0)
            {
                for (float fy = y - 0.5f; fy <= y + 0.5f; fy += 0.1f)
                {
                    for (float fx = x - 0.5f; fx <= x + 0.5f; fx += 0.1f)
                    {
                        const float value = (fx - xCen) * (fx - xCen) + (fy - yCen) * (fy - yCen);

                        if (value <= radiusSqr)
                        {
                            physicalPixelSize[x] += 2;
                        }
                    }
                }
            }
            else
            {
                physicalPixelSize[x] = 0;
            }
        }
    }
}

void VisionInspectionBgaBottom3D::MakeLogicalPixelMask_Ball(const Ipvm::Rect32s& roi, const Ipvm::Image16u& vismapImage,
    const Ipvm::Image8u& physicalPixelMask, Ipvm::Image8u& logicalPixelMask)
{
    float boundScale = 65535 / 255.f;
    USHORT lowerSBound
        = (USHORT)min(USHRT_MAX, max(0, m_pVisionPara->m_logicalPixelVisibilityLowerStartBound_ball * boundScale));
    USHORT lowerEBound
        = (USHORT)min(USHRT_MAX, max(0, m_pVisionPara->m_logicalPixelVisibilityLowerEndBound_ball * boundScale));
    USHORT upperSBound
        = (USHORT)min(USHRT_MAX, max(0, m_pVisionPara->m_logicalPixelVisibilityUpperStartBound_ball * boundScale));
    USHORT upperEBound
        = (USHORT)min(USHRT_MAX, max(0, m_pVisionPara->m_logicalPixelVisibilityUpperEndBound_ball * boundScale));

    lowerEBound = max(lowerSBound, lowerEBound);
    upperSBound = max(lowerEBound, upperSBound);
    upperEBound = max(upperSBound, upperEBound);

    const USHORT rangeL = lowerEBound - lowerSBound;
    const USHORT rangeU = upperEBound - upperSBound;

    for (long y = roi.m_top; y < roi.m_bottom; y++)
    {
        const auto* vis = vismapImage.GetMem(0, y);
        const unsigned char* physicalPixel = physicalPixelMask.GetMem(0, y);
        unsigned char* logicalPixel = logicalPixelMask.GetMem(0, y);

        for (long x = roi.m_left; x < roi.m_right; x++)
        {
            if (physicalPixel[x] == 0)
            {
                logicalPixel[x] = 0;
            }
            else if (vis[x] < lowerSBound || vis[x] > upperEBound)
            {
                logicalPixel[x] = 0;
            }
            else if (vis[x] <= lowerEBound)
            {
                logicalPixel[x] = unsigned char(LOGICAL_PIXEL_SIZE_MAX * float(physicalPixel[x])
                        / PHYSICAL_PIXEL_SIZE_MAX * (vis[x] - lowerSBound) / rangeL
                    + 0.5f);
            }
            else if (vis[x] >= upperSBound)
            {
                logicalPixel[x] = unsigned char(LOGICAL_PIXEL_SIZE_MAX * float(physicalPixel[x])
                        / PHYSICAL_PIXEL_SIZE_MAX * (vis[x] - upperSBound) / rangeU
                    + 0.5f);
            }
            else
            {
                logicalPixel[x]
                    = unsigned char(LOGICAL_PIXEL_SIZE_MAX * float(physicalPixel[x]) / PHYSICAL_PIXEL_SIZE_MAX + 0.5f);
            }
        }
    }
}

long VisionInspectionBgaBottom3D::GetMaxValidPointCount(long ballIndex, enumGetherBallZType zType)
{
    auto& algoParas = m_pVisionPara->m_parameters;
    const auto& ballInfo = m_packageSpec.m_ballMap->m_balls[ballIndex];

    auto& para_ballZ = algoParas[ballInfo.m_typeIndex].m_paraBallZ[long(zType)];

    if (para_ballZ.m_validPixelHeightThreshold_Percent <= 0.f)
        return 0;

    float persent = float(min(100.f, max(0.f, para_ballZ.m_maxValidPointsPercentOfSpecRadius))) / 100.f;
    float totalPixelCount = float(ITP_PI * ballInfo.m_radiusX_px * ballInfo.m_radiusY_px);

    return long(totalPixelCount * persent);
}

void VisionInspectionBgaBottom3D::SetInvalidBallInfo(
    CString strCurBallInfo, std::vector<CString>& vecstrInvalidBallInfo) //kircheis_AIBC
{
    long nSize = (long)vecstrInvalidBallInfo.size();

    if (nSize == 0)
    {
        vecstrInvalidBallInfo.push_back(strCurBallInfo);
        return;
    }

    for (auto& strInvalidBallInfo : vecstrInvalidBallInfo)
    {
        if (strCurBallInfo == strInvalidBallInfo)
            return;
    }
    vecstrInvalidBallInfo.push_back(strCurBallInfo);
}