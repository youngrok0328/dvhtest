//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingOtherAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingOtherAlign.h"
#include "VisionProcessingOtherAlignPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionProcessingOtherAlign::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionProcessingOtherAlign::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck fTime;

    ResetResult();

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    // Init...
    GetEdgeAlignResult(); //kircheis_USI_OA

    BOOL bAlign = FALSE;
    if (!DoAlign(detailSetupMode))
    {
        bAlign = TRUE;
    }

    SetDebugInfo();

    m_fCalcTime = CAST_FLOAT(fTime.Elapsed_ms());

    return bAlign;
}

long VisionProcessingOtherAlign::DoAlign(const bool detailSetupMode)
{
    Ipvm::Image8u combineImage;

    if (!getReusableMemory().GetInspByteImage(combineImage))
        return -1;

    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return -1;

    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_ImageProcMangePara, combineImage))
    {
        return -1;
    }

    const auto& vecrtSearchROI_BCU = m_VisionPara->m_vecrtSearchROI_BCU;
    long nSearchROICount = (long)vecrtSearchROI_BCU.size();
    Ipvm::Rect32s rtSearchROI;

    m_vecptLeftTop.resize(nSearchROICount);
    m_vecptRightTop.resize(nSearchROICount);
    m_vecptRightBottom.resize(nSearchROICount);
    m_vecptLeftBottom.resize(nSearchROICount);
    m_vecfrtAlignROI.resize(nSearchROICount);

    Ipvm::Point32r2 fptBodyCenter = m_sEdgeAlignResult->m_center;
    Ipvm::Point32r2 fptBodyCenterOffset_um = m_VisionPara->m_fptBodyCenterOffset_um;
    Ipvm::Point32s2 ptCenterOffsetOfROI;

    const auto& scale = getScale();

    for (long nCount = 0; nCount < nSearchROICount; nCount++)
    {
        rtSearchROI = getScale().convert_BCUToPixel(vecrtSearchROI_BCU[nCount], fptBodyCenter);

        //kircheis_USI_OA 일단 rtSearchROI를 Align 좌표와 Align Angle을 기준으로 보상하는 알고리즘이 필요하다
        if (fptBodyCenterOffset_um.m_x > -90000.f && fptBodyCenterOffset_um.m_y > -90000.f
            && m_sEdgeAlignResult->bAvailable)
        {
            ptCenterOffsetOfROI.m_x = rtSearchROI.CenterPoint().m_x
                - (long)(fptBodyCenter.m_x - scale.convert_umToPixelX(fptBodyCenterOffset_um.m_x) + .5f);
            ptCenterOffsetOfROI.m_y = rtSearchROI.CenterPoint().m_y
                - (long)(fptBodyCenter.m_y - scale.convert_umToPixelY(fptBodyCenterOffset_um.m_y) + .5f);
            rtSearchROI.OffsetRect(-ptCenterOffsetOfROI.m_x, -ptCenterOffsetOfROI.m_y);
        }

        if (OtherEdgeDetect_Left(combineImage, rtSearchROI))
        {
            SetDebugInfoItem(detailSetupMode, _T("Edge Search ROI"), m_vecSearchROI);
            SetDebugInfoItem(detailSetupMode, _T("Edge Point Left"), vecLeftEdgeAlignPoints_Total);
            return -1;
        }
        if (OtherEdgeDetect_Top(combineImage, rtSearchROI))
        {
            SetDebugInfoItem(detailSetupMode, _T("Edge Search ROI"), m_vecSearchROI);
            SetDebugInfoItem(detailSetupMode, _T("Edge Point Top"), vecTopEdgeAlignPoints_Total);
            return -1;
        }
        if (OtherEdgeDetect_Right(combineImage, rtSearchROI))
        {
            SetDebugInfoItem(detailSetupMode, _T("Edge Search ROI"), m_vecSearchROI);
            SetDebugInfoItem(detailSetupMode, _T("Edge Point Right"), vecRightEdgeAlignPoints_Total);
            return -1;
        }
        if (OtherEdgeDetect_Bottom(combineImage, rtSearchROI))
        {
            SetDebugInfoItem(detailSetupMode, _T("Edge Search ROI"), m_vecSearchROI);
            SetDebugInfoItem(detailSetupMode, _T("Edge Point Bottom"), vecBottomEdgeAlignPoints_Total);
            return -1;
        }

        // 영훈 예외처리 : 엣지를 찾지 못할 경우 그냥 빠져나가도록 한다.
        long nSize = (long)vecLeftEdgeAlignPoints.size();
        if (nSize < 2)
            return -1;
        nSize = (long)vecTopEdgeAlignPoints.size();
        if (nSize < 2)
            return -1;
        nSize = (long)vecRightEdgeAlignPoints.size();
        if (nSize < 2)
            return -1;
        nSize = (long)vecBottomEdgeAlignPoints.size();
        if (nSize < 2)
            return -1;

        SetResult(nCount);
    }

    SetDebugInfoItem(detailSetupMode, _T("Edge Search ROI"), m_vecSearchROI);
    SetDebugInfoItem(detailSetupMode, _T("Edge Point Left"), vecLeftEdgeAlignPoints_Total);
    SetDebugInfoItem(detailSetupMode, _T("Edge Point Top"), vecTopEdgeAlignPoints_Total);
    SetDebugInfoItem(detailSetupMode, _T("Edge Point Right"), vecRightEdgeAlignPoints_Total);
    SetDebugInfoItem(detailSetupMode, _T("Edge Point Bottom"), vecBottomEdgeAlignPoints_Total);
    SetDebugInfoItem(detailSetupMode, _T("Detect Rect"), m_vecfrtAlignROI, TRUE);

    //{{//kircheis_USI_OA
    auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Detect ROI")); //kircheis_580 ChipLayerInsp
    if (psDebugInfo != NULL)
    {
        if (psDebugInfo->pData != NULL)
        {
            psDebugInfo->Reset();
        }
        long nLayoutCount = (long)m_vecptLeftTop.size();
        psDebugInfo->nDataNum = nLayoutCount;
        PI_RECT* pPoint = new PI_RECT[psDebugInfo->nDataNum];
        for (long i = 0; i < psDebugInfo->nDataNum; i++)
        {
            pPoint[i].SetLTPoint(Ipvm::Conversion::ToPoint32s2(m_vecptLeftTop[i]));
            pPoint[i].SetLBPoint(Ipvm::Conversion::ToPoint32s2(m_vecptLeftBottom[i]));
            pPoint[i].SetRTPoint(Ipvm::Conversion::ToPoint32s2(m_vecptRightTop[i]));
            pPoint[i].SetRBPoint(Ipvm::Conversion::ToPoint32s2(m_vecptRightBottom[i]));
        }
        psDebugInfo->pData = pPoint;
    }

    for (long i = 0; i < long(m_vecptLeftTop.size()); i++)
    {
        Ipvm::Quadrangle32r quadRangle;
        quadRangle.m_ltX = m_vecptLeftTop[i].m_x;
        quadRangle.m_ltY = m_vecptLeftTop[i].m_y;
        quadRangle.m_rtX = m_vecptRightTop[i].m_x;
        quadRangle.m_rtY = m_vecptRightTop[i].m_y;
        quadRangle.m_lbX = m_vecptLeftBottom[i].m_x;
        quadRangle.m_lbY = m_vecptLeftBottom[i].m_y;
        quadRangle.m_rbX = m_vecptRightBottom[i].m_x;
        quadRangle.m_rbY = m_vecptRightBottom[i].m_y;

        CString name;
        name.Format(_T("Rect Object Align Result %d"), i);

        if (auto* surfaceROI = getReusableMemory().AddSurfaceLayerRoiClass(name))
        {
            surfaceROI->Reset();
            surfaceROI->Add(quadRangle);
        }

        name.Empty();
    }

    //}}
    return 0;
}

long VisionProcessingOtherAlign::OtherEdgeDetect_Left(const Ipvm::Image8u& image, Ipvm::Rect32s rtSearchROI)
{
    static long nEdgeSearchWidth = 3;
    Ipvm::Point32s2 ptStart(0, 0);
    long nSearchDirection;
    const float fScale = getScale().pixelToUm().m_x;

    long nEdgeSearchLength = (long)((float)m_VisionPara->m_nEdgeSearchLength[enum_Left] / fScale + .5f);
    long nEdgeSearchLengthHalf = (long)((float)nEdgeSearchLength * .5f + .5f); //kircheis_USI_OA
    long nSideIgnoreLength = (long)((float)rtSearchROI.Height() * 0.1f + .5f); //kircheis_USI_OA
    Ipvm::Rect32s rtMakeSearchROI;
    rtMakeSearchROI.m_left = rtSearchROI.m_left - nEdgeSearchLengthHalf;
    rtMakeSearchROI.m_top = rtSearchROI.m_top + nSideIgnoreLength;
    rtMakeSearchROI.m_right = rtSearchROI.m_left + nEdgeSearchLengthHalf;
    rtMakeSearchROI.m_bottom = rtSearchROI.m_bottom - nSideIgnoreLength;
    m_vecSearchROI[enum_Left] = rtMakeSearchROI;

    if (m_VisionPara->m_arbUseEdgeAlignResult[enum_Left]) //kircheis_USI_OA
    {
        if (!m_sEdgeAlignResult->bAvailable)
            return -1;
        vecLeftEdgeAlignPoints.clear();
        vecLeftEdgeAlignPoints_Total.clear();

        Ipvm::Point32r2 fptPoint = m_sEdgeAlignResult->fptLT;
        vecLeftEdgeAlignPoints.push_back(fptPoint);
        vecLeftEdgeAlignPoints_Total.push_back(fptPoint);

        fptPoint = m_sEdgeAlignResult->fptLB;
        vecLeftEdgeAlignPoints.push_back(fptPoint);
        vecLeftEdgeAlignPoints_Total.push_back(fptPoint);

        //	if(RoughAlign_LineAlign(enum_Left))
        //		return -1;

        return 0;
    }

    long nEdgeDirection = m_VisionPara->m_nEdgeDirection[enum_Left]; //? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    BOOL bFirstEdgeMode = m_VisionPara->m_nEdgeDetectMode[enum_Left] == EdgeDetectMode_FirstEdge;

    float fOldMinEdgeThreshold(0.f);
    if (bFirstEdgeMode)
    {
        fOldMinEdgeThreshold = m_pEdgeDetect->SetMininumThreshold((float)m_VisionPara->m_nFirstEdgeValue[enum_Left]);
    }

    vecLeftEdgeAlignPoints.clear();
    for (long nHeight = rtMakeSearchROI.m_top; nHeight < rtMakeSearchROI.m_bottom; nHeight += nEdgeSearchWidth)
    {
        if (m_VisionPara->m_nSearchDirection[enum_Left] == PI_ED_DIR_OUTER) //In->Out)
        {
            nSearchDirection = LEFT;
            ptStart = Ipvm::Point32s2(rtMakeSearchROI.m_right, nHeight);
        }
        else
        {
            nSearchDirection = RIGHT;
            ptStart = Ipvm::Point32s2(rtMakeSearchROI.m_left, nHeight);
        }

        Ipvm::Point32r2 bestEdge;

        if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection, nEdgeSearchLength,
                nEdgeSearchWidth, image, bestEdge, bFirstEdgeMode))
        {
            vecLeftEdgeAlignPoints.push_back(bestEdge);
            vecLeftEdgeAlignPoints_Total.push_back(bestEdge);
        }
    }

    if (bFirstEdgeMode)
    {
        fOldMinEdgeThreshold = m_pEdgeDetect->SetMininumThreshold(fOldMinEdgeThreshold);
    }

    if (vecLeftEdgeAlignPoints.size() <= 0)
        return -1;

    if (RoughAlign_LineAlign(enum_Left))
        return -1;

    return 0;
}

long VisionProcessingOtherAlign::OtherEdgeDetect_Top(const Ipvm::Image8u& image, Ipvm::Rect32s rtSearchROI)
{
    static long nEdgeSearchWidth = 3;
    Ipvm::Point32s2 ptStart(0, 0);
    long nSearchDirection;
    const float fScale = getScale().pixelToUm().m_y;

    long nEdgeSearchLength = (long)((float)m_VisionPara->m_nEdgeSearchLength[enum_Top] / fScale + .5f);
    long nEdgeSearchLengthHalf = (long)((float)nEdgeSearchLength * .5f + .5f); //kircheis_USI_OA
    long nSideIgnoreLength = (long)((float)rtSearchROI.Width() * 0.1f + .5f); //kircheis_USI_OA
    Ipvm::Rect32s rtMakeSearchROI;
    rtMakeSearchROI.m_left = rtSearchROI.m_left + nSideIgnoreLength;
    rtMakeSearchROI.m_top = rtSearchROI.m_top - nEdgeSearchLengthHalf;
    rtMakeSearchROI.m_right = rtSearchROI.m_right - nSideIgnoreLength;
    rtMakeSearchROI.m_bottom = rtSearchROI.m_top + nEdgeSearchLengthHalf;
    m_vecSearchROI[enum_Top] = rtMakeSearchROI;

    if (m_VisionPara->m_arbUseEdgeAlignResult[enum_Top]) //kircheis_USI_OA
    {
        if (!m_sEdgeAlignResult->bAvailable)
            return -1;
        vecTopEdgeAlignPoints.clear();
        vecTopEdgeAlignPoints_Total.clear();

        Ipvm::Point32r2 fptPoint = m_sEdgeAlignResult->fptLT;
        vecTopEdgeAlignPoints.push_back(fptPoint);
        vecTopEdgeAlignPoints_Total.push_back(fptPoint);

        fptPoint = m_sEdgeAlignResult->fptRT;
        vecTopEdgeAlignPoints.push_back(fptPoint);
        vecTopEdgeAlignPoints_Total.push_back(fptPoint);

        //	if(RoughAlign_LineAlign(enum_Top))
        //		return -1;

        return 0;
    }

    long nEdgeDirection = m_VisionPara->m_nEdgeDirection[enum_Top]; //? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    BOOL bFirstEdgeMode = m_VisionPara->m_nEdgeDetectMode[enum_Top] == EdgeDetectMode_FirstEdge;

    float fOldMinEdgeThreshold(0.f);
    if (bFirstEdgeMode)
    {
        fOldMinEdgeThreshold = m_pEdgeDetect->SetMininumThreshold((float)m_VisionPara->m_nFirstEdgeValue[enum_Top]);
    }

    vecTopEdgeAlignPoints.clear();
    for (long nWidth = rtMakeSearchROI.m_left; nWidth < rtMakeSearchROI.m_right; nWidth += nEdgeSearchWidth)
    {
        if (m_VisionPara->m_nSearchDirection[enum_Top] == PI_ED_DIR_OUTER) //In->Out)
        {
            nSearchDirection = UP;
            ptStart = Ipvm::Point32s2(nWidth, rtMakeSearchROI.m_bottom);
        }
        else
        {
            nSearchDirection = DOWN;
            ptStart = Ipvm::Point32s2(nWidth, rtMakeSearchROI.m_top);
        }

        Ipvm::Point32r2 bestEdge;

        if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection, nEdgeSearchLength,
                nEdgeSearchWidth, image, bestEdge, bFirstEdgeMode))
        {
            vecTopEdgeAlignPoints.push_back(bestEdge);
            vecTopEdgeAlignPoints_Total.push_back(bestEdge);
        }
    }

    if (bFirstEdgeMode)
    {
        fOldMinEdgeThreshold = m_pEdgeDetect->SetMininumThreshold(fOldMinEdgeThreshold);
    }

    if (vecTopEdgeAlignPoints.size() <= 0)
        return -1;

    if (RoughAlign_LineAlign(enum_Top))
        return -1;

    return 0;
}

long VisionProcessingOtherAlign::OtherEdgeDetect_Right(const Ipvm::Image8u& image, Ipvm::Rect32s rtSearchROI)
{
    static long nEdgeSearchWidth = 3;
    Ipvm::Point32s2 ptStart(0, 0);
    long nSearchDirection;
    const float fScale = getScale().pixelToUm().m_x;

    long nEdgeSearchLength = (long)((float)m_VisionPara->m_nEdgeSearchLength[enum_Right] / fScale + .5f);
    long nEdgeSearchLengthHalf = (long)((float)nEdgeSearchLength * .5f + .5f); //kircheis_USI_OA
    long nSideIgnoreLength = (long)((float)rtSearchROI.Height() * 0.1f + .5f); //kircheis_USI_OA
    Ipvm::Rect32s rtMakeSearchROI;
    rtMakeSearchROI.m_left = rtSearchROI.m_right - nEdgeSearchLengthHalf;
    rtMakeSearchROI.m_top = rtSearchROI.m_top + nSideIgnoreLength;
    rtMakeSearchROI.m_right = rtSearchROI.m_right + nEdgeSearchLengthHalf;
    rtMakeSearchROI.m_bottom = rtSearchROI.m_bottom - nSideIgnoreLength;
    m_vecSearchROI[enum_Right] = rtMakeSearchROI;

    if (m_VisionPara->m_arbUseEdgeAlignResult[enum_Right]) //kircheis_USI_OA
    {
        if (!m_sEdgeAlignResult->bAvailable)
            return -1;
        vecRightEdgeAlignPoints.clear();
        vecRightEdgeAlignPoints_Total.clear();

        Ipvm::Point32r2 fptPoint = m_sEdgeAlignResult->fptRT;
        vecRightEdgeAlignPoints.push_back(fptPoint);
        vecRightEdgeAlignPoints_Total.push_back(fptPoint);

        fptPoint = m_sEdgeAlignResult->fptRB;
        vecRightEdgeAlignPoints.push_back(fptPoint);
        vecRightEdgeAlignPoints_Total.push_back(fptPoint);

        //	if(RoughAlign_LineAlign(enum_Right))
        //		return -1;

        return 0;
    }

    long nEdgeDirection = m_VisionPara->m_nEdgeDirection[enum_Right]; //? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    BOOL bFirstEdgeMode = m_VisionPara->m_nEdgeDetectMode[enum_Right] == EdgeDetectMode_FirstEdge;

    float fOldMinEdgeThreshold(0.f);
    if (bFirstEdgeMode)
    {
        fOldMinEdgeThreshold = m_pEdgeDetect->SetMininumThreshold((float)m_VisionPara->m_nFirstEdgeValue[enum_Right]);
    }

    vecRightEdgeAlignPoints.clear();
    for (long nHeight = rtMakeSearchROI.m_top; nHeight < rtMakeSearchROI.m_bottom; nHeight += nEdgeSearchWidth)
    {
        if (m_VisionPara->m_nSearchDirection[enum_Right] == PI_ED_DIR_OUTER) //In->Out)
        {
            nSearchDirection = RIGHT;
            ptStart = Ipvm::Point32s2(rtMakeSearchROI.m_left, nHeight);
        }
        else
        {
            nSearchDirection = LEFT;
            ptStart = Ipvm::Point32s2(rtMakeSearchROI.m_right, nHeight);
        }

        Ipvm::Point32r2 bestEdge;

        if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection, nEdgeSearchLength,
                nEdgeSearchWidth, image, bestEdge, bFirstEdgeMode))
        {
            vecRightEdgeAlignPoints.push_back(bestEdge);
            vecRightEdgeAlignPoints_Total.push_back(bestEdge);
        }
    }

    if (bFirstEdgeMode)
    {
        fOldMinEdgeThreshold = m_pEdgeDetect->SetMininumThreshold(fOldMinEdgeThreshold);
    }

    if (vecRightEdgeAlignPoints.size() <= 0)
        return -1;

    if (RoughAlign_LineAlign(enum_Right))
        return -1;

    return 0;
}

long VisionProcessingOtherAlign::OtherEdgeDetect_Bottom(const Ipvm::Image8u& image, Ipvm::Rect32s rtSearchROI)
{
    static long nEdgeSearchWidth = 3;
    Ipvm::Point32s2 ptStart(0, 0);
    long nSearchDirection;
    const float fScale = getScale().pixelToUm().m_y;

    long nEdgeSearchLength = (long)((float)m_VisionPara->m_nEdgeSearchLength[enum_Bottom] / fScale + .5f);
    long nEdgeSearchLengthHalf = (long)((float)nEdgeSearchLength * .5f + .5f); //kircheis_USI_OA
    long nSideIgnoreLength = (long)((float)rtSearchROI.Width() * 0.1f + .5f); //kircheis_USI_OA
    Ipvm::Rect32s rtMakeSearchROI;
    rtMakeSearchROI.m_left = rtSearchROI.m_left + nSideIgnoreLength;
    rtMakeSearchROI.m_top = rtSearchROI.m_bottom - nEdgeSearchLengthHalf;
    rtMakeSearchROI.m_right = rtSearchROI.m_right - nSideIgnoreLength;
    rtMakeSearchROI.m_bottom = rtSearchROI.m_bottom + nEdgeSearchLengthHalf;
    m_vecSearchROI[enum_Bottom] = rtMakeSearchROI;

    if (m_VisionPara->m_arbUseEdgeAlignResult[enum_Bottom]) //kircheis_USI_OA
    {
        if (!m_sEdgeAlignResult->bAvailable)
            return -1;
        vecBottomEdgeAlignPoints.clear();
        vecBottomEdgeAlignPoints_Total.clear();

        Ipvm::Point32r2 fptPoint = m_sEdgeAlignResult->fptLB;
        vecBottomEdgeAlignPoints.push_back(fptPoint);
        vecBottomEdgeAlignPoints_Total.push_back(fptPoint);

        fptPoint = m_sEdgeAlignResult->fptRB;
        vecBottomEdgeAlignPoints.push_back(fptPoint);
        vecBottomEdgeAlignPoints_Total.push_back(fptPoint);

        //	if(RoughAlign_LineAlign(enum_Bottom))
        //		return -1;

        return 0;
    }

    long nEdgeDirection = m_VisionPara->m_nEdgeDirection[enum_Bottom]; //? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    BOOL bFirstEdgeMode = m_VisionPara->m_nEdgeDetectMode[enum_Bottom] == EdgeDetectMode_FirstEdge;

    float fOldMinEdgeThreshold(0.f);
    if (bFirstEdgeMode)
    {
        fOldMinEdgeThreshold = m_pEdgeDetect->SetMininumThreshold((float)m_VisionPara->m_nFirstEdgeValue[enum_Bottom]);
    }

    vecBottomEdgeAlignPoints.clear();
    for (long nWidth = rtMakeSearchROI.m_left; nWidth < rtMakeSearchROI.m_right; nWidth += nEdgeSearchWidth)
    {
        if (m_VisionPara->m_nSearchDirection[enum_Bottom] == PI_ED_DIR_OUTER) //In->Out)
        {
            nSearchDirection = DOWN;
            ptStart = Ipvm::Point32s2(nWidth, rtMakeSearchROI.m_top);
        }
        else
        {
            nSearchDirection = UP;
            ptStart = Ipvm::Point32s2(nWidth, rtMakeSearchROI.m_bottom);
        }

        Ipvm::Point32r2 bestEdge;

        if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(
                nEdgeDirection, ptStart, nSearchDirection, nEdgeSearchLength, nEdgeSearchWidth, image, bestEdge, FALSE))
        {
            vecBottomEdgeAlignPoints.push_back(bestEdge);
            vecBottomEdgeAlignPoints_Total.push_back(bestEdge);
        }
    }

    if (bFirstEdgeMode)
    {
        fOldMinEdgeThreshold = m_pEdgeDetect->SetMininumThreshold(fOldMinEdgeThreshold);
    }

    if (vecBottomEdgeAlignPoints.size() <= 0)
        return -1;

    if (RoughAlign_LineAlign(enum_Bottom))
        return -1;

    return 0;
}

long VisionProcessingOtherAlign::RoughAlign_LineAlign(long nDirection)
{
    std::vector<Ipvm::Point32r2> vecfpTempEdge;

    if (nDirection == enum_Left)
    {
        vecfpTempEdge = vecLeftEdgeAlignPoints;
        vecLeftEdgeAlignPoints.clear();
    }
    else if (nDirection == enum_Top)
    {
        vecfpTempEdge = vecTopEdgeAlignPoints;
        vecTopEdgeAlignPoints.clear();
    }
    else if (nDirection == enum_Right)
    {
        vecfpTempEdge = vecRightEdgeAlignPoints;
        vecRightEdgeAlignPoints.clear();
    }
    else if (nDirection == enum_Bottom)
    {
        vecfpTempEdge = vecBottomEdgeAlignPoints;
        vecBottomEdgeAlignPoints.clear();
    }

    long nPointSize = 0;

    std::vector<Ipvm::Point32r2> vecfTotalXY;

    nPointSize = (long)(vecfpTempEdge.size());

    if (nPointSize <= 0)
        return -1;

    for (long i = 0; i < (long)(vecfpTempEdge.size()); i++)
    {
        vecfTotalXY.push_back(vecfpTempEdge[i]);
    }

    std::vector<Ipvm::Point32r2> vecfFilteredXY;
    Ipvm::LineEq32r line;

    if (!CPI_Geometry::RoughLineFitting(nPointSize, &vecfTotalXY[0], line, 5, 1.f, vecfFilteredXY))
    {
        return -1;
    }

    nPointSize = (long)(vecfFilteredXY.size());

    if (nPointSize <= 0)
        return -1;

    for (long i = 0; i < nPointSize; i++)
    {
        if (nDirection == enum_Left)
            vecLeftEdgeAlignPoints.push_back(vecfFilteredXY[i]);
        else if (nDirection == enum_Top)
            vecTopEdgeAlignPoints.push_back(vecfFilteredXY[i]);
        else if (nDirection == enum_Right)
            vecRightEdgeAlignPoints.push_back(vecfFilteredXY[i]);
        else if (nDirection == enum_Bottom)
            vecBottomEdgeAlignPoints.push_back(vecfFilteredXY[i]);
    }

    return 0;
}

long VisionProcessingOtherAlign::SetResult(long nSearchROIID)
{
    // 처음에 네 직선 방정식을 구한다.
    std::vector<Ipvm::LineEq32r> vecsRefLine;
    vecsRefLine.resize(LOWTOP);

    if (vecLeftEdgeAlignPoints.size() < 2)
        return -1;
    if (vecTopEdgeAlignPoints.size() < 2)
        return -1;
    if (vecRightEdgeAlignPoints.size() < 2)
        return -1;
    if (vecBottomEdgeAlignPoints.size() < 2)
        return -1;

    if (Ipvm::DataFitting::FitToLineRn(
            long(vecLeftEdgeAlignPoints.size()), &vecLeftEdgeAlignPoints[0], 3.f, vecsRefLine[LEFT])
            != Ipvm::Status::e_ok
        || Ipvm::DataFitting::FitToLineRn(
               long(vecTopEdgeAlignPoints.size()), &vecTopEdgeAlignPoints[0], 3.f, vecsRefLine[UP])
            != Ipvm::Status::e_ok
        || Ipvm::DataFitting::FitToLineRn(
               long(vecRightEdgeAlignPoints.size()), &vecRightEdgeAlignPoints[0], 3.f, vecsRefLine[RIGHT])
            != Ipvm::Status::e_ok
        || Ipvm::DataFitting::FitToLineRn(
               long(vecBottomEdgeAlignPoints.size()), &vecBottomEdgeAlignPoints[0], 3.f, vecsRefLine[DOWN])
            != Ipvm::Status::e_ok)
    {
        return -1;
    }

    // 직선 방정식으로부터 네 모서리를 계산한다.
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[LEFT], vecsRefLine[UP], m_vecptLeftTop[nSearchROIID])
        != Ipvm::Status::e_ok)
    {
        return -1;
    }
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[UP], vecsRefLine[RIGHT], m_vecptRightTop[nSearchROIID])
        != Ipvm::Status::e_ok)
    {
        return -1;
    }

    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[RIGHT], vecsRefLine[DOWN], m_vecptRightBottom[nSearchROIID])
        != Ipvm::Status::e_ok)
    {
        return -1;
    }

    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[DOWN], vecsRefLine[LEFT], m_vecptLeftBottom[nSearchROIID])
        != Ipvm::Status::e_ok)
    {
        return -1;
    }

    m_vecfrtAlignROI[nSearchROIID].m_left
        = (m_vecptLeftTop[nSearchROIID].m_x + m_vecptLeftBottom[nSearchROIID].m_x) * 0.5f;
    m_vecfrtAlignROI[nSearchROIID].m_top
        = (m_vecptLeftTop[nSearchROIID].m_y + m_vecptRightTop[nSearchROIID].m_y) * 0.5f;
    m_vecfrtAlignROI[nSearchROIID].m_right
        = (m_vecptRightTop[nSearchROIID].m_x + m_vecptRightBottom[nSearchROIID].m_x) * 0.5f;
    m_vecfrtAlignROI[nSearchROIID].m_bottom
        = (m_vecptLeftBottom[nSearchROIID].m_y + m_vecptRightBottom[nSearchROIID].m_y) * 0.5f;

    return 0;
}

void VisionProcessingOtherAlign::SetDebugInfo()
{
}

void VisionProcessingOtherAlign::GetEdgeAlignResult() //kircheis_USI_OA
{
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
}
