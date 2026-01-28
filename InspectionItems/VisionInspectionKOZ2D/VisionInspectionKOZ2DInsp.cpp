//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionKOZ2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionKOZ2D.h"
#include "VisionInspectionKOZ2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../DefineModules/dA_Base/ConversionEx.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/CircleEq32r.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
bool sort_toptobottom(Ipvm::Rect32s _F, Ipvm::Rect32s _S)
{
    return _F.m_top < _S.m_top;
};
bool sort_lefttoright(Ipvm::Rect32s _F, Ipvm::Rect32s _S)
{
    return _F.m_left < _S.m_left;
};

void VisionInspectionKOZ2D::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    AbsoluteRectToImageRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionKOZ2D::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left += (long)(fptCenter.m_x + .5f);
    rtROI.m_top += (long)(fptCenter.m_y + .5f);
    rtROI.m_right += (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom += (long)(fptCenter.m_y + .5f);
}

void VisionInspectionKOZ2D::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    ImageRectToAbsoluteRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionKOZ2D::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left -= (long)(fptCenter.m_x + .5f);
    rtROI.m_top -= (long)(fptCenter.m_y + .5f);
    rtROI.m_right -= (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom -= (long)(fptCenter.m_y + .5f);
}

Ipvm::Rect32r VisionInspectionKOZ2D::GetBodyRect()
{
    Ipvm::Rect32r frtBodyRect = Ipvm::Rect32r(0.f, 0.f, 0.f, 0.f);

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
        frtBodyRect = m_sEdgeAlignResult->getBodyRect();
    }

    return frtBodyRect;
}

void VisionInspectionKOZ2D::UpdateSweepAngleTable(
    long nSectionNum, std::vector<float>& vecfCos, std::vector<float>& vecfSin)
{
    vecfCos.resize(nSectionNum);
    vecfSin.resize(nSectionNum);

    float* pfCos = &vecfCos[0];
    float* pfSin = &vecfSin[0];

    for (long nSection = 0; nSection < nSectionNum; nSection++)
    {
        float fCurAngle = float(nSection) / (float)nSectionNum * (float)ITP_TwoPI;
        pfCos[nSection] = float(cos(fCurAngle));
        pfSin[nSection] = float(sin(fCurAngle));
    }
}

BOOL VisionInspectionKOZ2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

BOOL VisionInspectionKOZ2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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

    Ipvm::TimeCheck time_Insp;

    BOOL bResult = TRUE;

    // Init...
    ResetResult();
    if (m_pVisionInspDlg != NULL && m_pVisionInspDlg->IsWindowVisible())
    {
        // Teaching
        if (detailSetupMode)
        {
            m_pVisionInspDlg->GetROI();
        }

        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    m_kozAlignResult.bAvailable = false;
    if (DoAlign(detailSetupMode) == false)
        return false;

    if (!GetKOZWidthData()) //먼저 KOZ Width가 모두 0보다 큰지 확인한다. LTRB의 KOZ Width 모두가 0이하면 아래의 검사를 할 필요가 없다.
        return true;

    MakeKOZChippingAreaMask(); //Surface에서도 쓸수 있으니까 일단 Mask는 만든다.

    //{{//Inspection
    bResult &= DoInspKOZWidth(); //얘는 함수 내부에서 Align 결과로만 거리 계산해서 판정하면 된다
    bResult &= DoInspKOZChipping(
        detailSetupMode); //얘는 함수 안에서 할거 많다. 최대한 잘게 쪼개라. 검사 영상 Combine->적분영상생성->AdaptiveThreshold->Blob->Blob분석->판정
    //}}

    m_fCalcTime = CAST_FLOAT(time_Insp.Elapsed_ms());

    return bResult;
}

BOOL VisionInspectionKOZ2D::DoAlign(const bool detailSetupMode)
{
    Ipvm::Rect32r frtBody = GetBodyRect();
    if (frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        return FALSE;

    GetPackageOutlineLineEq(m_veclinePackage);

    Ipvm::Quadrangle32r qrtPackageAlign;
    qrtPackageAlign = ConversionEx::ToQuadrangle32r(
        m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB);
    m_imagePackageAlignMask.Free();
    if (!getReusableMemory().GetInspByteImage(m_imagePackageAlignMask))
        return false;
    Ipvm::ImageProcessing::Fill(qrtPackageAlign, 255, m_imagePackageAlignMask);

    //{{//Notch Align
    if (GetNotchInfo(detailSetupMode)) //Notch 정보 가져오기 Notch가 없으면 false다
    {
        if (!DoAlignNotch(detailSetupMode))
            return false;
    }
    //}}

    //{{//KOZ Align
    //먼저 KOZ Width가 모두 0보다 큰지 확인한다. LTRB의 KOZ Width 모두가 0이하면 아래의 검사를 할 필요가 없다.
    if (!GetKOZWidthData())
        return true;

    if (!DoAlignKOZ(detailSetupMode))
        return false;
    //}}

    return true;
}

BOOL VisionInspectionKOZ2D::GetNotchInfo(const bool detailSetupMode)
{
    m_vecNotchSpecInfo.clear();
    const auto& um2px = getScale().umToPixel();

    GetPackageOutlineLineEq(m_veclinePackage);

    //Map Data의 Notch 개수를 가져와서 Notch가 없으면 return false
    std::vector<Ipvm::EllipseEq32r> vecEllipseNotchSpec_mm(0);
    //kircheis_KOZ_HardCodingTest
    vecEllipseNotchSpec_mm.push_back(Ipvm::EllipseEq32r(-38.15f, -2.5f, 0.95f, 0.95f));
    vecEllipseNotchSpec_mm.push_back(Ipvm::EllipseEq32r(-38.15f, -16.f, 0.95f, 0.95f));
    vecEllipseNotchSpec_mm.push_back(Ipvm::EllipseEq32r(38.15f, -2.5f, 0.95f, 0.95f));
    vecEllipseNotchSpec_mm.push_back(Ipvm::EllipseEq32r(38.15f, -16.f, 0.95f, 0.95f));

    long nNotchNum = (long)vecEllipseNotchSpec_mm.size();
    if (nNotchNum < 1)
        return false;

    //일단 Notch의 위치를 영상 기준으로 변환하고 (Body Center를 적용한 pxl 단위로 환산) Notch Dir및 Projection Point를 계산
    SNotchInfo sNotchInfo;
    long nDirection;
    float fShiftOffset;
    float fPackageSpecHalfSizeX_mm = (m_packageSpec.m_bodyInfoMaster->fBodySizeX * 0.5f) * 0.001f;
    float fPackageSpecHalfSizeY_mm = (m_packageSpec.m_bodyInfoMaster->fBodySizeY * 0.5f) * 0.001f;
    Ipvm::Point32r2 ptPackageCenter = m_sEdgeAlignResult->m_center;
    std::vector<Ipvm::EllipseEq32r> vecEllipseNoichForDebugInfo(0); //DebugInfo

    for (long nNotch = 0; nNotch < nNotchNum; nNotch++)
    {
        sNotchInfo.ellipseNotchSpec_pxl.m_xradius = vecEllipseNotchSpec_mm[nNotch].m_xradius * 1000.f * um2px.m_x;
        sNotchInfo.ellipseNotchSpec_pxl.m_yradius = vecEllipseNotchSpec_mm[nNotch].m_yradius * 1000.f * um2px.m_y;
        sNotchInfo.ellipseNotchSpec_pxl.m_x
            = ptPackageCenter.m_x + ((vecEllipseNotchSpec_mm[nNotch].m_x * 1000.f) * um2px.m_x);
        sNotchInfo.ellipseNotchSpec_pxl.m_y
            = ptPackageCenter.m_y + ((-vecEllipseNotchSpec_mm[nNotch].m_y * 1000.f) * um2px.m_y);

        nDirection = GetNotchDirectionAndProjectionEdgeLine(m_veclinePackage, sNotchInfo.ellipseNotchSpec_pxl);
        sNotchInfo.nNotchPositionDir = nDirection;

        switch (nDirection)
        {
            case LEFT:
                fShiftOffset
                    = ((float)fabs(vecEllipseNotchSpec_mm[nNotch].m_x) - fPackageSpecHalfSizeX_mm) * 1000.f * um2px.m_x;
                sNotchInfo.ellipseNotchSpec_pxl.m_x -= fShiftOffset;
                break;
            case UP:
                fShiftOffset
                    = ((float)fabs(vecEllipseNotchSpec_mm[nNotch].m_x) - fPackageSpecHalfSizeY_mm) * 1000.f * um2px.m_y;
                sNotchInfo.ellipseNotchSpec_pxl.m_y -= fShiftOffset;
                break;
            case RIGHT:
                fShiftOffset
                    = ((float)fabs(vecEllipseNotchSpec_mm[nNotch].m_x) - fPackageSpecHalfSizeX_mm) * 1000.f * um2px.m_x;
                sNotchInfo.ellipseNotchSpec_pxl.m_x += fShiftOffset;
                break;
            case DOWN:
                fShiftOffset
                    = ((float)fabs(vecEllipseNotchSpec_mm[nNotch].m_x) - fPackageSpecHalfSizeY_mm) * 1000.f * um2px.m_y;
                sNotchInfo.ellipseNotchSpec_pxl.m_y += fShiftOffset;
                break;
        }
        m_vecNotchSpecInfo.push_back(sNotchInfo);
        vecEllipseNoichForDebugInfo.push_back(sNotchInfo.ellipseNotchSpec_pxl);
    }

    SetDebugInfoItem(detailSetupMode, _T("Notch Spec"), vecEllipseNoichForDebugInfo);

    return true;
}

BOOL VisionInspectionKOZ2D::GetPackageOutlineLineEq(std::vector<Ipvm::LineEq32r>& o_vecLinePackage)
{
    o_vecLinePackage.clear();
    o_vecLinePackage.resize(4);

    std::vector<Ipvm::Point32r2> vecfptCorner(2);

    for (long nDir = UP; nDir <= RIGHT; nDir++)
    {
        switch (nDir)
        {
            case LEFT:
                vecfptCorner[0] = Ipvm::Point32r2(m_sEdgeAlignResult->fptLT);
                vecfptCorner[1] = Ipvm::Point32r2(m_sEdgeAlignResult->fptLB);
                break;
            case UP:
                vecfptCorner[0] = Ipvm::Point32r2(m_sEdgeAlignResult->fptLT);
                vecfptCorner[1] = Ipvm::Point32r2(m_sEdgeAlignResult->fptRT);
                break;
            case RIGHT:
                vecfptCorner[0] = Ipvm::Point32r2(m_sEdgeAlignResult->fptRT);
                vecfptCorner[1] = Ipvm::Point32r2(m_sEdgeAlignResult->fptRB);
                break;
            case DOWN:
                vecfptCorner[0] = Ipvm::Point32r2(m_sEdgeAlignResult->fptLB);
                vecfptCorner[1] = Ipvm::Point32r2(m_sEdgeAlignResult->fptRB);
                break;
            default:
                return false;
        }

        Ipvm::DataFitting::FitToLineRn(2, &vecfptCorner[0], 3.f, o_vecLinePackage[nDir]);
    }
    return true;
}

long VisionInspectionKOZ2D::GetNotchDirectionAndProjectionEdgeLine(
    std::vector<Ipvm::LineEq32r> i_vecLinePackage, Ipvm::EllipseEq32r& io_ellipseNotch_pxl)
{
    if (i_vecLinePackage.size() != 4)
        return 0;

    float fDist(0.f), fMinDist(99999.f);
    long nMinDirection{};
    Ipvm::Point32r2 ptEllipseCenter(io_ellipseNotch_pxl.m_x, io_ellipseNotch_pxl.m_y);
    Ipvm::Point32r2 ptEllipseCenterProjection;

    for (long nDir = UP; nDir <= RIGHT; nDir++)
    {
        Ipvm::Geometry::GetDistance(i_vecLinePackage[nDir], ptEllipseCenter, fDist);
        if (fDist < fMinDist)
        {
            fMinDist = fDist;
            nMinDirection = nDir;
        }
    }

    Ipvm::Geometry::GetFootOfPerpendicular(i_vecLinePackage[nMinDirection], ptEllipseCenter, ptEllipseCenterProjection);
    io_ellipseNotch_pxl.m_x = ptEllipseCenterProjection.m_x;
    io_ellipseNotch_pxl.m_y = ptEllipseCenterProjection.m_y;

    return nMinDirection;
}

BOOL VisionInspectionKOZ2D::DoAlignNotch(const bool detailSetupMode)
{
    m_vecEllipseNotchAlignResult.clear();
    m_vecnNotchDirection.clear();
    long nNotchNum = (long)m_vecNotchSpecInfo.size();
    if (nNotchNum <= 0)
        return true;

    //검사 영상 준비
    long nNotchAlignFrameIdx = m_VisionPara->m_NotchAlignFrameIndex.getFrameIndex();
    Ipvm::Image8u image = GetInspectionFrameImage(false, nNotchAlignFrameIdx);
    if (image.GetMem() == nullptr)
        return FALSE;

    Ipvm::Image8u combineImage;
    if (!getReusableMemory().GetInspByteImage(combineImage))
        return false;
    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_NotchImageProcManagePara, combineImage))
    {
        return false;
    }

    //Debug Info용 변수
    std::vector<Ipvm::LineSeg32r> vecLineNotchSweepLine(0);
    std::vector<Ipvm::LineSeg32r> vecLineNotchSweepLineForDebugInfo(0);
    std::vector<Ipvm::Point32r2> vecfptNotchEdgePointForDebugInfo(0);
    std::vector<Ipvm::EllipseEq32r> vecfptNotchAlignResultForDebugInfo(0);

    //연산용 변수
    float fNotchSearchLengthRatio((float)m_VisionPara->m_nNotchEdgeSearchLengthRatio * 0.01f);
    long nNotchDir(0);
    std::vector<Ipvm::Point32r2> vecfptEdgePoint(0);
    Ipvm::EllipseEq32r ellipseEqAlignResult;
    BOOL bIsFirstEdge = (m_VisionPara->m_nEdgeDetectMode == PI_ED_FIRST_EDGE);
    Ipvm::CircleEq32r CircleCoeff;

    //Notch On KOZ의 Mask 만들기
    Ipvm::Image8u imageMask;
    if (!getReusableMemory().GetInspByteImage(imageMask))
        return false;
    imageMask.FillZero();

    float fNotchSearchLength;

    for (long nNotch = 0; nNotch < nNotchNum; nNotch++)
    {
        const auto& notchSpecInfo = m_vecNotchSpecInfo[nNotch];
        auto notchSpecEllipse = notchSpecInfo.ellipseNotchSpec_pxl;
        nNotchDir = notchSpecInfo.nNotchPositionDir;
        fNotchSearchLength
            = ((float)max(notchSpecEllipse.m_xradius, notchSpecEllipse.m_yradius) * fNotchSearchLengthRatio);

        AlignOneNotch(combineImage, notchSpecEllipse, nNotchDir, PI_ED_DIR_RISING, 5.f, fNotchSearchLength, 0.f,
            bIsFirstEdge, vecfptEdgePoint, vecLineNotchSweepLine);

        vecLineNotchSweepLineForDebugInfo.insert(
            vecLineNotchSweepLineForDebugInfo.end(), vecLineNotchSweepLine.begin(), vecLineNotchSweepLine.end());
        vecfptNotchEdgePointForDebugInfo.insert(
            vecfptNotchEdgePointForDebugInfo.end(), vecfptEdgePoint.begin(), vecfptEdgePoint.end());

        //if (Ipvm::DataFitting::FitToEllipse(long(vecfptEdgePoint.size()), &vecfptEdgePoint[0], ellipseEqAlignResult) != Ipvm::Status::e_ok)
        if (Ipvm::DataFitting::FitToCircle(long(vecfptEdgePoint.size()), &vecfptEdgePoint[0], CircleCoeff)
            != Ipvm::Status::e_ok) //Ellipse Fitting 실패율이 높다
        {
            SetDebugInfoItem(detailSetupMode, _T("Notch Edge Sweep Line"), vecLineNotchSweepLineForDebugInfo);
            SetDebugInfoItem(detailSetupMode, _T("Notch Edge Point"), vecfptNotchEdgePointForDebugInfo);
            SetDebugInfoItem(detailSetupMode, _T("Notch Align Result"), vecfptNotchAlignResultForDebugInfo);
            return false;
        }
        ellipseEqAlignResult.m_x = CircleCoeff.m_x;
        ellipseEqAlignResult.m_y = CircleCoeff.m_y;
        ellipseEqAlignResult.m_xradius = ellipseEqAlignResult.m_yradius = CircleCoeff.m_radius;

        m_vecEllipseNotchAlignResult.push_back(ellipseEqAlignResult);
        m_vecnNotchDirection.push_back(nNotchDir);
        vecfptNotchAlignResultForDebugInfo.push_back(ellipseEqAlignResult);

        Ipvm::ImageProcessing::Fill(ellipseEqAlignResult, 255, imageMask);
    }

    SetDebugInfoItem(detailSetupMode, _T("Notch Edge Sweep Line"), vecLineNotchSweepLineForDebugInfo);
    SetDebugInfoItem(detailSetupMode, _T("Notch Edge Point"), vecfptNotchEdgePointForDebugInfo);
    SetDebugInfoItem(detailSetupMode, _T("Notch Align Result"), vecfptNotchAlignResultForDebugInfo);

    //Notch On KOZ의 Mask 만들기
    Ipvm::ImageProcessing::BitwiseAnd(m_imagePackageAlignMask, Ipvm::Rect32s(imageMask), imageMask);

    auto* alignMask = getReusableMemory().AddSurfaceLayerMaskClass(_T("Measured - Notch Hole Mask"));
    if (alignMask)
    {
        alignMask->Reset();
        alignMask->Add(imageMask);
    }

    return true;
}

BOOL VisionInspectionKOZ2D::AlignOneNotch(Ipvm::Image8u i_image, Ipvm::EllipseEq32r i_ellipseNotch_pxl,
    long i_nNotchDir, long i_nEdgeDir, float i_fEdgeThrehsold, float i_fSearchLength, float i_fIgnoreLength,
    bool bFirstEdge, std::vector<Ipvm::Point32r2>& o_vecfptEdge, std::vector<Ipvm::LineSeg32r>& o_vecLineNotchSweepLine)
{
    o_vecfptEdge.clear();
    o_vecLineNotchSweepLine.clear();

    float* pfCos = &m_vecfCosForNotchAlign[0];
    float* pfSin = &m_vecfSinForNotchAlign[0];

    //{{//Special **주의 : NOTCH_SWEEP_LINE_NUM가 24일때에 최적화 되어 있다. 24가 아닌 다른 수가 되면 여기의 값들 다시 조정해야 된다.
    std::vector<long> vecnSweepStartID(4), vecnSweepEndID(4);
    vecnSweepStartID[LEFT] = 20;
    vecnSweepEndID[LEFT] = 28; //순차적으로 늘려주기 위해 28로 정의 했다. 어차피 아래에서 %연산자로 안전장치 넣었다
    vecnSweepStartID[UP] = 14;
    vecnSweepEndID[UP] = 22;
    vecnSweepStartID[RIGHT] = 8;
    vecnSweepEndID[RIGHT] = 16;
    vecnSweepStartID[DOWN] = 2;
    vecnSweepEndID[DOWN] = 10;
    //}}

    //long nNotchSearchLength(0);
    Ipvm::Point32r2 ptNotchEdge;
    long nConvertSection(0);
    std::vector<BYTE> vbyLineValue;
    float fEdgeRadius(0.f), fEdgeValue(0.f);
    long nLineValue, nIgnoreLineValue;

    BOOL bSubPixelingDone;

    float fEdgeThresholdOld = m_pEdgeDetect->SetMininumThreshold(i_fEdgeThrehsold);

    for (long nSection = vecnSweepStartID[i_nNotchDir]; nSection <= vecnSweepEndID[i_nNotchDir]; nSection++)
    {
        nConvertSection = nSection % NOTCH_SWEEP_LINE_NUM;
        const float fOutX = i_ellipseNotch_pxl.m_x + i_fSearchLength * pfCos[nConvertSection];
        const float fOutY = i_ellipseNotch_pxl.m_y + i_fSearchLength * pfSin[nConvertSection];
        o_vecLineNotchSweepLine.emplace_back(i_ellipseNotch_pxl.m_x, i_ellipseNotch_pxl.m_y, fOutX, fOutY);
        nIgnoreLineValue = 0;

        if (i_fIgnoreLength > 0.f)
        {
            const float fIgnoreOutX = i_ellipseNotch_pxl.m_x + i_fIgnoreLength * pfCos[nConvertSection];
            const float fIgnoreOutY = i_ellipseNotch_pxl.m_y + i_fIgnoreLength * pfSin[nConvertSection];
            nIgnoreLineValue = CippModules::GetLineData(i_image, i_ellipseNotch_pxl.m_x, i_ellipseNotch_pxl.m_y,
                fIgnoreOutX, fIgnoreOutY, pfCos[nConvertSection], pfSin[nConvertSection], NULL);
        }

        nLineValue = CippModules::GetLineData(i_image, i_ellipseNotch_pxl.m_x, i_ellipseNotch_pxl.m_y, fOutX, fOutY,
            pfCos[nConvertSection], pfSin[nConvertSection], NULL);
        vbyLineValue.clear();
        if (nLineValue <= 0)
            return false;
        vbyLineValue.resize(nLineValue);
        CippModules::GetLineData(i_image, i_ellipseNotch_pxl.m_x, i_ellipseNotch_pxl.m_y, fOutX, fOutY,
            pfCos[nConvertSection], pfSin[nConvertSection], &vbyLineValue[0]);

        if (m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(i_nEdgeDir, nLineValue - nIgnoreLineValue,
                &vbyLineValue[nIgnoreLineValue], fEdgeRadius, fEdgeValue, bSubPixelingDone,
                bFirstEdge)) //FALSE ))//kircheis_ShinyTest//kircheis_NantongBO
        {
            ptNotchEdge.m_x = i_ellipseNotch_pxl.m_x + (nIgnoreLineValue + fEdgeRadius) * pfCos[nConvertSection];
            ptNotchEdge.m_y = i_ellipseNotch_pxl.m_y + (nIgnoreLineValue + fEdgeRadius) * pfSin[nConvertSection];
            o_vecfptEdge.push_back(ptNotchEdge);
        }
    }

    m_pEdgeDetect->SetMininumThreshold(fEdgeThresholdOld);
    return true;
}

void VisionInspectionKOZ2D::GetNotchEdgeSearchLine(
    std::vector<Ipvm::Point32r2>& o_vecfptLineStart, std::vector<Ipvm::Point32r2>& o_vecfptLineEnd)
{
    o_vecfptLineStart.clear();
    o_vecfptLineEnd.clear();

    if (GetNotchInfo(false) == false)
        return;

    long nNotchNum = (long)m_vecNotchSpecInfo.size();
    if (nNotchNum <= 0)
        return;

    GetPackageOutlineLineEq(m_veclinePackage);
    float* pfCos = &m_vecfCosForNotchAlign[0];
    float* pfSin = &m_vecfSinForNotchAlign[0];

    //{{//Special **주의 : NOTCH_SWEEP_LINE_NUM가 24일때에 최적화 되어 있다. 24가 아닌 다른 수가 되면 여기의 값들 다시 조정해야 된다.
    std::vector<long> vecnSweepStartID(4), vecnSweepEndID(4);
    vecnSweepStartID[LEFT] = 20;
    vecnSweepEndID[LEFT] = 28; //순차적으로 늘려주기 위해 28로 정의 했다. 어차피 아래에서 %연산자로 안전장치 넣었다
    vecnSweepStartID[UP] = 14;
    vecnSweepEndID[UP] = 22;
    vecnSweepStartID[RIGHT] = 8;
    vecnSweepEndID[RIGHT] = 16;
    vecnSweepStartID[DOWN] = 2;
    vecnSweepEndID[DOWN] = 10;
    //}}

    long nNotchDir(UP), nConvertSection(0);
    float fSearchLength;
    Ipvm::Point32r2 fptStart, fptEnd;
    float fNotchSearchLengthRatio((float)m_VisionPara->m_nNotchEdgeSearchLengthRatio * 0.01f);

    for (long nNotch = 0; nNotch < nNotchNum; nNotch++)
    {
        auto notchSpecInfo = m_vecNotchSpecInfo[nNotch];
        auto notchSpecEllipse = notchSpecInfo.ellipseNotchSpec_pxl;
        nNotchDir = notchSpecInfo.nNotchPositionDir;
        fptStart = Ipvm::Point32r2(notchSpecEllipse.m_x, notchSpecEllipse.m_y);

        fSearchLength = ((float)max(notchSpecEllipse.m_xradius, notchSpecEllipse.m_yradius) * fNotchSearchLengthRatio);

        for (long nSection = vecnSweepStartID[nNotchDir]; nSection <= vecnSweepEndID[nNotchDir]; nSection++)
        {
            nConvertSection = nSection % NOTCH_SWEEP_LINE_NUM;
            const float fOutX = notchSpecEllipse.m_x + fSearchLength * pfCos[nConvertSection];
            const float fOutY = notchSpecEllipse.m_y + fSearchLength * pfSin[nConvertSection];
            fptEnd = Ipvm::Point32r2(fOutX, fOutY);

            o_vecfptLineStart.push_back(fptStart);
            o_vecfptLineEnd.push_back(fptEnd);
        }
    }
}

BOOL VisionInspectionKOZ2D::GetKOZWidthData()
{
    const auto& um2px = getScale().umToPixel();
    m_vecfKOZSpecWidth_um.clear();
    m_vecfKOZSpecWidth_pxl.clear();
    m_vecbKOZSpecWidth_Valid.clear();

    m_vecfKOZSpecWidth_um.resize(4);
    m_vecfKOZSpecWidth_pxl.resize(4);
    m_vecbKOZSpecWidth_Valid.resize(4);

    m_vecfKOZSpecWidth_um[LEFT]
        = 400.f; // m_packageSpec.m_bodyInfoMaster->m_fKOZWidthLeft;//kirchies_KOZ_HardCodingTest
    m_vecfKOZSpecWidth_um[UP] = 400.f; //m_packageSpec.m_bodyInfoMaster->m_fKOZWidthUp;
    m_vecfKOZSpecWidth_um[RIGHT] = 400.f; //m_packageSpec.m_bodyInfoMaster->m_fKOZWidthRight;
    m_vecfKOZSpecWidth_um[DOWN] = 400.f; //m_packageSpec.m_bodyInfoMaster->m_fKOZWidthDown;

    float fum2px;
    long nValidCnt = 0;
    for (long nDir = UP; nDir <= RIGHT; nDir++)
    {
        if (nDir == UP || nDir == DOWN)
            fum2px = um2px.m_y;
        else
            fum2px = um2px.m_x;

        m_vecfKOZSpecWidth_pxl[nDir] = m_vecfKOZSpecWidth_um[nDir] * fum2px;

        if (m_vecfKOZSpecWidth_pxl[nDir] > 0.f)
        {
            m_vecbKOZSpecWidth_Valid[nDir] = true;
            nValidCnt++;
        }
        else
            m_vecbKOZSpecWidth_Valid[nDir] = false;
    }

    return (nValidCnt > 0);
}

BOOL VisionInspectionKOZ2D::DoAlignKOZ(const bool detailSetupMode)
{
    m_kozAlignResult.bAvailable = false;
    //검사 영상 준비
    long nKOZAlignFrameIdx = m_VisionPara->m_KOZAlignFrameIndex.getFrameIndex();
    Ipvm::Image8u image = GetInspectionFrameImage(false, nKOZAlignFrameIdx);
    if (image.GetMem() == nullptr)
        return FALSE;

    Ipvm::Image8u combineImage;
    if (!getReusableMemory().GetInspByteImage(combineImage))
        return false;
    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_ImageProcManagePara, combineImage))
    {
        return FALSE;
    }

    //Debug Info 용 변수 확보
    std::vector<PI_RECT> vecprtSpecROI(0);
    std::vector<Ipvm::Rect32s> vecrtSearchROI(0);
    std::vector<Ipvm::Point32r2> vecfptDebugEdge(0);
    std::vector<PI_RECT> vecprtAlignResult(0);

    //연산용 버퍼
    std::vector<Ipvm::Rect32s> vecrtEdgeSearchROI(4);
    std::vector<Ipvm::Point32r2> vecfptLeftEdge(0);
    std::vector<Ipvm::Point32r2> vecfptTopEdge(0);
    std::vector<Ipvm::Point32r2> vecfptRightEdge(0);
    std::vector<Ipvm::Point32r2> vecfptBottomEdge(0);
    std::vector<Ipvm::Point32r2> vecfptEdge(0);
    VisionAlignResult kozAlignResult;
    PI_RECT piAlignRect;
    m_veclineKOZ.clear();
    m_veclineKOZ.resize(4);

    //KOZ Spec ROI
    PI_RECT frtSpecKOZ;

    //Edge Search용 ROI를 만들고 Debug Info용 버퍼에 넣는다.
    MakeEdgeSearchROI(vecrtEdgeSearchROI, frtSpecKOZ);
    vecrtSearchROI.push_back(vecrtEdgeSearchROI[LEFT]); //Align ROI의 순서를 맞추기 위해 insert를 안쓴거임
    vecrtSearchROI.push_back(vecrtEdgeSearchROI[UP]);
    vecrtSearchROI.push_back(vecrtEdgeSearchROI[RIGHT]);
    vecrtSearchROI.push_back(vecrtEdgeSearchROI[DOWN]);
    vecprtSpecROI.push_back(frtSpecKOZ);
    SetDebugInfoItem(detailSetupMode, _T("KOZ Spec"), frtSpecKOZ);
    SetDebugInfoItem(detailSetupMode, _T("KOZ Edge Search ROI"), vecrtSearchROI);

    //KOZ의 Notch 영역 Align
    AlignNotchOnKOZ(combineImage, detailSetupMode);

    //Edge 탐색
    for (long nDir = UP; nDir <= RIGHT; nDir++)
    {
        vecfptEdge.clear();
        if (m_vecbKOZSpecWidth_Valid[nDir]) //이게 false면 KOZ Width가 0이하다.
        {
            GetKOZEdgePoint(combineImage, nDir, vecrtEdgeSearchROI[nDir], vecfptEdge);
            vecfptDebugEdge.insert(vecfptDebugEdge.end(), vecfptEdge.begin(), vecfptEdge.end());
            if (vecfptEdge.size() <= 3)
            {
                SetDebugInfoItem(detailSetupMode, _T("KOZ Detected Edge Point"),
                    vecfptDebugEdge); //어디서 잘못된건지 알아야 하니까 중간 실패 시 DebugInfo를 채운다
                return false;
            }
        }
        else //KOZ Width가 0이하면 Package Align을 쓴다.
        {
            switch (nDir)
            {
                case LEFT:
                    vecfptEdge.push_back(m_sEdgeAlignResult->fptLT);
                    vecfptEdge.push_back(m_sEdgeAlignResult->fptLB);
                    break;
                case UP:
                    vecfptEdge.push_back(m_sEdgeAlignResult->fptLT);
                    vecfptEdge.push_back(m_sEdgeAlignResult->fptRT);
                    break;
                case RIGHT:
                    vecfptEdge.push_back(m_sEdgeAlignResult->fptRT);
                    vecfptEdge.push_back(m_sEdgeAlignResult->fptRB);
                    break;
                case DOWN:
                    vecfptEdge.push_back(m_sEdgeAlignResult->fptLB);
                    vecfptEdge.push_back(m_sEdgeAlignResult->fptRB);
                    break;
            }
        }

        switch (nDir)
        {
            case LEFT:
                vecfptLeftEdge.insert(vecfptLeftEdge.end(), vecfptEdge.begin(), vecfptEdge.end());
                break;
            case UP:
                vecfptTopEdge.insert(vecfptTopEdge.end(), vecfptEdge.begin(), vecfptEdge.end());
                break;
            case RIGHT:
                vecfptRightEdge.insert(vecfptRightEdge.end(), vecfptEdge.begin(), vecfptEdge.end());
                break;
            case DOWN:
                vecfptBottomEdge.insert(vecfptBottomEdge.end(), vecfptEdge.begin(), vecfptEdge.end());
                break;
        }
    }
    SetDebugInfoItem(detailSetupMode, _T("KOZ Detected Edge Point"), vecfptDebugEdge);

    //Line Fitting and Make Cross Point
    if (CalcKOZAlignResult(vecfptLeftEdge, vecfptTopEdge, vecfptRightEdge, vecfptBottomEdge, m_veclineKOZ[LEFT],
            m_veclineKOZ[UP], m_veclineKOZ[RIGHT], m_veclineKOZ[DOWN], kozAlignResult))
    {
        m_kozAlignResult = kozAlignResult;

        piAlignRect.SetLTPoint(Ipvm::Conversion::ToPoint32s2(kozAlignResult.fptLT));
        piAlignRect.SetRTPoint(Ipvm::Conversion::ToPoint32s2(kozAlignResult.fptRT));
        piAlignRect.SetRBPoint(Ipvm::Conversion::ToPoint32s2(kozAlignResult.fptRB));
        piAlignRect.SetLBPoint(Ipvm::Conversion::ToPoint32s2(kozAlignResult.fptLB));
        vecprtAlignResult.push_back(piAlignRect);
        SetDebugInfoItem(detailSetupMode, _T("KOZ Alignment Result"), vecprtAlignResult, TRUE);
    }

    //Mask 연결
    auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("KOZ Alignment Result"));
    if (psDebugInfo != NULL)
    {
        if (psDebugInfo->pData != NULL)
        {
            psDebugInfo->Reset();
        }
        long nLayoutCount = (long)vecprtAlignResult.size();
        psDebugInfo->nDataNum = nLayoutCount;
        PI_RECT* pPoint = new PI_RECT[psDebugInfo->nDataNum];
        for (long i = 0; i < psDebugInfo->nDataNum; i++)
        {
            pPoint[i] = vecprtAlignResult[i];
        }
        psDebugInfo->pData = pPoint;
    }

    if (vecprtAlignResult.size() > 0)
    {
        Ipvm::Quadrangle32r quadRangle;
        quadRangle.m_ltX = (float)vecprtAlignResult[0].ltX;
        quadRangle.m_ltY = (float)vecprtAlignResult[0].ltY;
        quadRangle.m_rtX = (float)vecprtAlignResult[0].rtX;
        quadRangle.m_rtY = (float)vecprtAlignResult[0].rtY;
        quadRangle.m_lbX = (float)vecprtAlignResult[0].blX;
        quadRangle.m_lbY = (float)vecprtAlignResult[0].blY;
        quadRangle.m_rbX = (float)vecprtAlignResult[0].brX;
        quadRangle.m_rbY = (float)vecprtAlignResult[0].brY;

        CString name;
        name.Format(_T("Measured - KOZ"));
        if (auto* surfaceROI = getReusableMemory().AddSurfaceLayerRoiClass(name))
        {
            surfaceROI->Reset();
            surfaceROI->Add(quadRangle);
        }
    }

    m_kozAlignResult.bAvailable = true;

    //}}
    return true;
}

BOOL VisionInspectionKOZ2D::MakeEdgeSearchROI(std::vector<Ipvm::Rect32s>& o_vecrtSearchROI, PI_RECT& prtKOZ)
{
    const auto& um2px = getScale().umToPixel();

    Ipvm::Rect32r rtPackageAlign = m_sEdgeAlignResult->getBodyRect();
    Ipvm::Point32r2 fptPackageCenter = rtPackageAlign.CenterPoint();

    o_vecrtSearchROI.clear();
    o_vecrtSearchROI.resize(4);

    //const float fWidthRatio = (float)max(0.1f, min(1.f, (float)m_VisionPara->m_nEdgeSearchWidthRatio * 0.01f));
    const float fSearchLengthHalfLR_Pxl = ((float)m_VisionPara->m_nEdgeSearchLength_um * um2px.m_x * .5f);
    const float fSearchLengthHalfTB_Pxl = ((float)m_VisionPara->m_nEdgeSearchLength_um * um2px.m_y * .5f);
    const float fSearchWidthRatio = (float)(m_VisionPara->m_nEdgeSearchWidthRatio * 0.01f);

    float fKOZ_InnerSizeX_pxl = (rtPackageAlign.Width() - m_vecfKOZSpecWidth_pxl[LEFT] - m_vecfKOZSpecWidth_pxl[RIGHT]);
    float fKOZ_InnerSizeY_pxl = (rtPackageAlign.Height() - m_vecfKOZSpecWidth_pxl[UP] - m_vecfKOZSpecWidth_pxl[DOWN]);
    float fIgnoreGapLR = (fKOZ_InnerSizeX_pxl * (1.f - fSearchWidthRatio) * .5f);
    float fIgnoreGapTB = (fKOZ_InnerSizeY_pxl * (1.f - fSearchWidthRatio) * .5f);

    //{{//DebugInfo용 KOZ Spec
    prtKOZ.SetLTPoint(Ipvm::Point32s2(CAST_INT32T(m_sEdgeAlignResult->fptLT.m_x + m_vecfKOZSpecWidth_pxl[LEFT] + .5f),
        CAST_INT32T(m_sEdgeAlignResult->fptLT.m_y + m_vecfKOZSpecWidth_pxl[UP] + .5f)));
    prtKOZ.SetLBPoint(Ipvm::Point32s2(CAST_INT32T(m_sEdgeAlignResult->fptLB.m_x + m_vecfKOZSpecWidth_pxl[LEFT] + .5f),
        CAST_INT32T(m_sEdgeAlignResult->fptLB.m_y - m_vecfKOZSpecWidth_pxl[DOWN] + .5f)));
    prtKOZ.SetRTPoint(Ipvm::Point32s2(CAST_INT32T(m_sEdgeAlignResult->fptRT.m_x - m_vecfKOZSpecWidth_pxl[RIGHT] + .5f),
        CAST_INT32T(m_sEdgeAlignResult->fptRT.m_y + m_vecfKOZSpecWidth_pxl[UP] + .5f)));
    prtKOZ.SetRBPoint(Ipvm::Point32s2(CAST_INT32T(m_sEdgeAlignResult->fptRB.m_x - m_vecfKOZSpecWidth_pxl[RIGHT] + .5f),
        CAST_INT32T(m_sEdgeAlignResult->fptRB.m_y - m_vecfKOZSpecWidth_pxl[DOWN] + .5f)));
    //}}

    //UP
    if (m_vecbKOZSpecWidth_Valid[UP] == false)
        o_vecrtSearchROI[UP] = Ipvm::Rect32s{};
    else
    {
        o_vecrtSearchROI[UP].m_left = (long)(rtPackageAlign.m_left + m_vecfKOZSpecWidth_pxl[LEFT] + fIgnoreGapLR + .5f);
        o_vecrtSearchROI[UP].m_right
            = (long)(rtPackageAlign.m_right - m_vecfKOZSpecWidth_pxl[RIGHT] - fIgnoreGapLR + .5f);
        o_vecrtSearchROI[UP].m_top
            = (long)(rtPackageAlign.m_top + m_vecfKOZSpecWidth_pxl[UP] - fSearchLengthHalfTB_Pxl + .5f);
        o_vecrtSearchROI[UP].m_bottom
            = (long)(rtPackageAlign.m_top + m_vecfKOZSpecWidth_pxl[UP] + fSearchLengthHalfTB_Pxl + .5f);
    }

    //DOWN
    if (m_vecbKOZSpecWidth_Valid[DOWN] == false)
        o_vecrtSearchROI[DOWN] = Ipvm::Rect32s{};
    else
    {
        o_vecrtSearchROI[DOWN].m_left
            = (long)(rtPackageAlign.m_left + m_vecfKOZSpecWidth_pxl[LEFT] + fIgnoreGapLR + .5f);
        o_vecrtSearchROI[DOWN].m_right
            = (long)(rtPackageAlign.m_right - m_vecfKOZSpecWidth_pxl[RIGHT] - fIgnoreGapLR + .5f);
        o_vecrtSearchROI[DOWN].m_top
            = (long)(rtPackageAlign.m_bottom - m_vecfKOZSpecWidth_pxl[DOWN] - fSearchLengthHalfTB_Pxl + .5f);
        o_vecrtSearchROI[DOWN].m_bottom
            = (long)(rtPackageAlign.m_bottom - m_vecfKOZSpecWidth_pxl[DOWN] + fSearchLengthHalfTB_Pxl + .5f);
    }

    //LEFT
    if (m_vecbKOZSpecWidth_Valid[LEFT] == false)
        o_vecrtSearchROI[LEFT] = Ipvm::Rect32s{};
    else
    {
        o_vecrtSearchROI[LEFT].m_left
            = (long)(rtPackageAlign.m_left + m_vecfKOZSpecWidth_pxl[LEFT] - fSearchLengthHalfLR_Pxl + .5f);
        o_vecrtSearchROI[LEFT].m_right
            = (long)(rtPackageAlign.m_left + m_vecfKOZSpecWidth_pxl[LEFT] + fSearchLengthHalfLR_Pxl + .5f);
        o_vecrtSearchROI[LEFT].m_top = (long)(rtPackageAlign.m_top + m_vecfKOZSpecWidth_pxl[UP] + fIgnoreGapTB + .5f);
        o_vecrtSearchROI[LEFT].m_bottom
            = (long)(rtPackageAlign.m_bottom - m_vecfKOZSpecWidth_pxl[DOWN] - fIgnoreGapTB + .5f);
    }

    //RIGHT
    if (m_vecbKOZSpecWidth_Valid[RIGHT] == false)
        o_vecrtSearchROI[RIGHT] = Ipvm::Rect32s{};
    else
    {
        o_vecrtSearchROI[RIGHT].m_left
            = (long)(rtPackageAlign.m_right - m_vecfKOZSpecWidth_pxl[RIGHT] - fSearchLengthHalfLR_Pxl + .5f);
        o_vecrtSearchROI[RIGHT].m_right
            = (long)(rtPackageAlign.m_right - m_vecfKOZSpecWidth_pxl[RIGHT] + fSearchLengthHalfLR_Pxl + .5f);
        o_vecrtSearchROI[RIGHT].m_top = (long)(rtPackageAlign.m_top + m_vecfKOZSpecWidth_pxl[UP] + fIgnoreGapTB + .5f);
        o_vecrtSearchROI[RIGHT].m_bottom
            = (long)(rtPackageAlign.m_bottom - m_vecfKOZSpecWidth_pxl[DOWN] - fIgnoreGapTB + .5f);
    }

    return TRUE;
}

BOOL VisionInspectionKOZ2D::AlignNotchOnKOZ(Ipvm::Image8u i_image, const bool detailSetupMode)
{
    m_vecEllipseKOZNotchAlignResult.clear();
    long nNotchNum = (long)m_vecEllipseNotchAlignResult.size();
    if (nNotchNum <= 0)
        return true;

    //const auto& um2px = getScale().umToPixel();

    //Debug Info용 변수
    std::vector<Ipvm::LineSeg32r> vecLineNotchSweepLine(0);
    std::vector<Ipvm::LineSeg32r> vecLineNotchSweepLineForDebugInfo(0);
    std::vector<Ipvm::Point32r2> vecfptNotchEdgePointForDebugInfo(0);
    std::vector<Ipvm::EllipseEq32r> vecfptNotchAlignSpecForDebugInfo(0);
    std::vector<Ipvm::EllipseEq32r> vecfptNotchAlignResultForDebugInfo(0);

    //Notch On KOZ의 Mask 만들기
    Ipvm::Image8u imageMask;
    if (!getReusableMemory().GetInspByteImage(imageMask))
        return false;
    imageMask.FillZero();

    //연산용 변수
    float fNotchSearchLengthRatio((float)m_VisionPara->m_nNotchEdgeSearchLengthRatio * 0.01f);
    float fSearchLength, fIgnoreLength;
    float fKOZWidth_Pxl;
    long nNotchDir(0);
    std::vector<Ipvm::Point32r2> vecfptEdgePoint(0);
    Ipvm::EllipseEq32r notchOnKozAlignSpec, notchOnKozAlignResult;
    float fEdgeThreshold = (float)m_VisionPara->m_dEdgeThreshold; //GetCurrentEdgeThresholdValueKOZ();
    float fEdgeThreshold_Old = m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold);
    Ipvm::CircleEq32r CircleCoeff;

    for (long nNotch = 0; nNotch < nNotchNum; nNotch++)
    {
        nNotchDir = m_vecnNotchDirection[nNotch];
        if (m_vecbKOZSpecWidth_Valid[nNotchDir] == false)
            continue;
        auto notchAlignResult = m_vecEllipseNotchAlignResult[nNotch];
        fKOZWidth_Pxl = m_vecfKOZSpecWidth_pxl[nNotchDir];
        notchOnKozAlignSpec = notchAlignResult;
        notchOnKozAlignSpec.m_xradius += fKOZWidth_Pxl;
        notchOnKozAlignSpec.m_yradius += fKOZWidth_Pxl;
        vecfptNotchAlignSpecForDebugInfo.push_back(notchOnKozAlignSpec);

        fIgnoreLength = (float)max(notchAlignResult.m_xradius, notchAlignResult.m_yradius);
        fSearchLength = fIgnoreLength + fKOZWidth_Pxl;
        fSearchLength *= fNotchSearchLengthRatio;

        AlignOneNotch(i_image, notchAlignResult, nNotchDir, PI_ED_DIR_FALLING, fEdgeThreshold, fSearchLength,
            fIgnoreLength, true, vecfptEdgePoint, vecLineNotchSweepLine);

        vecLineNotchSweepLineForDebugInfo.insert(
            vecLineNotchSweepLineForDebugInfo.end(), vecLineNotchSweepLine.begin(), vecLineNotchSweepLine.end());
        vecfptNotchEdgePointForDebugInfo.insert(
            vecfptNotchEdgePointForDebugInfo.end(), vecfptEdgePoint.begin(), vecfptEdgePoint.end());

        if (Ipvm::DataFitting::FitToCircle(long(vecfptEdgePoint.size()), &vecfptEdgePoint[0], CircleCoeff)
            != Ipvm::Status::e_ok) //Ellipse Fitting이 좀 이상하다
        { //Edge Point가 들쭉 날쭉하면 실패한다.
            SetDebugInfoItem(detailSetupMode, _T("KOZ Notch Spec"), vecfptNotchAlignSpecForDebugInfo);
            SetDebugInfoItem(detailSetupMode, _T("KOZ Notch Edge Sweep Line"), vecLineNotchSweepLineForDebugInfo);
            SetDebugInfoItem(detailSetupMode, _T("KOZ Notch Edge Point"), vecfptNotchEdgePointForDebugInfo);
            SetDebugInfoItem(detailSetupMode, _T("KOZ Notch Align Result"), vecfptNotchAlignResultForDebugInfo);
            m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold_Old);
            return false;
        }
        notchOnKozAlignResult.m_x = CircleCoeff.m_x;
        notchOnKozAlignResult.m_y = CircleCoeff.m_y;
        notchOnKozAlignResult.m_xradius = notchOnKozAlignResult.m_yradius = CircleCoeff.m_radius;

        m_vecEllipseKOZNotchAlignResult.push_back(notchOnKozAlignResult);
        vecfptNotchAlignResultForDebugInfo.push_back(notchOnKozAlignResult);

        Ipvm::ImageProcessing::Fill(notchOnKozAlignResult, 255, imageMask);
    }
    m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold_Old);

    SetDebugInfoItem(detailSetupMode, _T("KOZ Notch Spec"), vecfptNotchAlignSpecForDebugInfo);
    SetDebugInfoItem(detailSetupMode, _T("KOZ Notch Edge Sweep Line"), vecLineNotchSweepLineForDebugInfo);
    SetDebugInfoItem(detailSetupMode, _T("KOZ Notch Edge Point"), vecfptNotchEdgePointForDebugInfo);
    SetDebugInfoItem(detailSetupMode, _T("KOZ Notch Align Result"), vecfptNotchAlignResultForDebugInfo);

    //Notch On KOZ의 Mask 만들기
    Ipvm::ImageProcessing::BitwiseAnd(m_imagePackageAlignMask, Ipvm::Rect32s(imageMask), imageMask);

    auto* alignMask = getReusableMemory().AddSurfaceLayerMaskClass(_T("Measured - Notch Hole On KOZ Mask"));
    if (alignMask)
    {
        alignMask->Reset();
        alignMask->Add(imageMask);
    }

    return true;
}

BOOL VisionInspectionKOZ2D::GetKOZEdgePoint(
    Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge)
{
    const auto& um2px = getScale().umToPixel();

    o_vecfptEdge.clear();

    long nEdgeGap(0), nSearchLength(0), nSearchWidth(0);
    long nEdgeSearchDirection = m_VisionPara->m_nSearchDirection;
    long nEdgeDirection = m_VisionPara->m_nEdgeDirection;
    long nSearchDir = nDir;
    BOOL bFirstEdge = m_VisionPara->m_nEdgeDetectMode == PI_ED_FIRST_EDGE;
    Ipvm::Point32s2 ptStart;

    if (nDir == LEFT || nDir == RIGHT) //Left/Right Edge용
    {
        nEdgeGap = (long)(m_VisionPara->m_nEdgeDetectGap_um * um2px.m_y + .5f);
        nSearchLength = (long)rtSearchROI.Width();
        if (nDir == LEFT && nEdgeSearchDirection == PI_ED_DIR_INNER)
            nSearchDir = RIGHT;
        else if (nDir == RIGHT && nEdgeSearchDirection == PI_ED_DIR_INNER)
            nSearchDir = LEFT;
    }
    else //Top/Bottom Edge용
    {
        nEdgeGap = (long)(m_VisionPara->m_nEdgeDetectGap_um * um2px.m_x + .5f);
        nSearchLength = (long)rtSearchROI.Height();
        if (nDir == UP && nEdgeSearchDirection == PI_ED_DIR_INNER)
            nSearchDir = DOWN;
        else if (nDir == DOWN && nEdgeSearchDirection == PI_ED_DIR_INNER)
            nSearchDir = UP;
    }
    nEdgeGap = (long)max(2, nEdgeGap);
    nSearchLength = (long)max(5, nSearchLength);
    nSearchWidth = nEdgeGap / 2;
    nSearchWidth = (long)max(2, nSearchWidth);

    switch (nSearchDir)
    {
        case LEFT:
            ptStart = Ipvm::Point32s2(rtSearchROI.TopRight().m_x, rtSearchROI.TopRight().m_y + nSearchWidth);
            break;
        case UP:
            ptStart = Ipvm::Point32s2(rtSearchROI.BottomLeft().m_x + nSearchWidth, rtSearchROI.BottomLeft().m_y);
            break;
        case RIGHT:
            ptStart = Ipvm::Point32s2(rtSearchROI.TopLeft().m_x, rtSearchROI.TopLeft().m_y + nSearchWidth);
            break;
        case DOWN:
            ptStart = Ipvm::Point32s2(rtSearchROI.TopLeft().m_x + nSearchWidth, rtSearchROI.TopLeft().m_y);
            break;
        default:
            return FALSE;
    }

    float fEdgeThreshold = (float)m_VisionPara->m_dEdgeThreshold; // GetCurrentEdgeThresholdValueKOZ();

    float fEdgeThreshold_Old = m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold);
    if (nDir == LEFT || nDir == RIGHT)
    {
        long nEnd = rtSearchROI.BottomRight().m_y;
        for (long nY = ptStart.m_y; nY < nEnd; nY += nEdgeGap)
        {
            ptStart.m_y = nY;

            if (IsPtInNotchArea(ptStart)) //Start Point가 Notch 영역이면 이부분은 Edge를 찾지 마라
                continue;
            Ipvm::Point32r2 bestEdge;

            if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(
                    nEdgeDirection, ptStart, nSearchDir, nSearchLength, nSearchWidth, image, bestEdge, bFirstEdge))
            {
                o_vecfptEdge.push_back(bestEdge);
            }
        }
    }
    else
    {
        long nEnd = rtSearchROI.BottomRight().m_x;
        for (long nX = ptStart.m_x; nX < nEnd; nX += nEdgeGap)
        {
            ptStart.m_x = nX;
            if (IsPtInNotchArea(ptStart)) //Start Point가 Notch 영역이면 이부분은 Edge를 찾지 마라
                continue;
            Ipvm::Point32r2 bestEdge;

            if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(
                    nEdgeDirection, ptStart, nSearchDir, nSearchLength, nSearchWidth, image, bestEdge, bFirstEdge))
            {
                o_vecfptEdge.push_back(bestEdge);
            }
        }
    }
    m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold_Old);

    return TRUE;
}

BOOL VisionInspectionKOZ2D::IsPtInNotchArea(Ipvm::Point32s2 ptStart)
{
    long nNotchAlignNum = (long)m_vecEllipseNotchAlignResult.size();
    if (nNotchAlignNum)
        return false;

    Ipvm::Rect32s rtNotch;
    float fKOZWidth;

    for (long nNotch = 0; nNotch < nNotchAlignNum; nNotch++)
    {
        auto notchAlignResult = m_vecEllipseNotchAlignResult[nNotch];
        fKOZWidth = m_vecfKOZSpecWidth_pxl[LEFT];
        rtNotch.m_left = (long)(notchAlignResult.m_x - notchAlignResult.m_xradius - fKOZWidth + .5f);
        rtNotch.m_top = (long)(notchAlignResult.m_y - notchAlignResult.m_yradius - fKOZWidth + .5f);
        rtNotch.m_right = (long)(notchAlignResult.m_x + notchAlignResult.m_xradius + fKOZWidth + .5f);
        rtNotch.m_bottom = (long)(notchAlignResult.m_y + notchAlignResult.m_yradius + fKOZWidth + .5f);

        if (rtNotch.PtInRect(ptStart))
            return true;
    }

    return false;
}

BOOL VisionInspectionKOZ2D::CalcKOZAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge,
    std::vector<Ipvm::Point32r2> vecfptTopEdge, std::vector<Ipvm::Point32r2> vecfptRightEdge,
    std::vector<Ipvm::Point32r2> vecfptBottomEdge, Ipvm::LineEq32r& o_alignLeftLine, Ipvm::LineEq32r& o_alignTopLine,
    Ipvm::LineEq32r& o_alignRightLine, Ipvm::LineEq32r& o_alignBottomLine, VisionAlignResult& o_dieAlignResult)
{
    o_dieAlignResult.bAvailable = FALSE;

    long nLeftEdgeNum = (long)vecfptLeftEdge.size();
    long nTopEdgeNum = (long)vecfptTopEdge.size();
    long nRightEdgeNum = (long)vecfptRightEdge.size();
    long nBottomEdgeNum = (long)vecfptBottomEdge.size();

    if ((nLeftEdgeNum * nTopEdgeNum * nRightEdgeNum * nBottomEdgeNum) == 0)
        return FALSE;

    std::vector<Ipvm::LineEq32r> vecsLineEq(4);

    if (Ipvm::DataFitting::FitToLineRn(nLeftEdgeNum, &vecfptLeftEdge[0], 3.f, vecsLineEq[LEFT]) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::DataFitting::FitToLineRn(nTopEdgeNum, &vecfptTopEdge[0], 3.f, vecsLineEq[UP]) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::DataFitting::FitToLineRn(nRightEdgeNum, &vecfptRightEdge[0], 3.f, vecsLineEq[RIGHT])
        != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::DataFitting::FitToLineRn(nBottomEdgeNum, &vecfptBottomEdge[0], 3.f, vecsLineEq[DOWN])
        != Ipvm::Status::e_ok)
        return FALSE;

    o_alignLeftLine = vecsLineEq[LEFT];
    o_alignTopLine = vecsLineEq[UP];
    o_alignRightLine = vecsLineEq[RIGHT];
    o_alignBottomLine = vecsLineEq[DOWN];
    // 직선 방정식으로부터 네 모서리를 계산한다.
    Ipvm::Point32r2& ptLeftTop = o_dieAlignResult.fptLT;
    Ipvm::Point32r2& ptRightTop = o_dieAlignResult.fptRT;
    Ipvm::Point32r2& ptRightBottom = o_dieAlignResult.fptRB;
    Ipvm::Point32r2& ptLeftBottom = o_dieAlignResult.fptLB;

    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[LEFT], vecsLineEq[UP], ptLeftTop) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[UP], vecsLineEq[RIGHT], ptRightTop) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[RIGHT], vecsLineEq[DOWN], ptRightBottom) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[DOWN], vecsLineEq[LEFT], ptLeftBottom) != Ipvm::Status::e_ok)
        return FALSE;

    // 네 모서리로부터 바디 센터를 얻는다.
    o_dieAlignResult.m_center.m_x = (ptLeftTop.m_x + ptRightTop.m_x + ptRightBottom.m_x + ptLeftBottom.m_x) * 0.25f;
    o_dieAlignResult.m_center.m_y = (ptLeftTop.m_y + ptRightTop.m_y + ptRightBottom.m_y + ptLeftBottom.m_y) * 0.25f;

    // 네 모서리로부터 바디 앵글을 얻는다.
    const float angleLower = (float)atan2(ptRightBottom.m_y - ptLeftBottom.m_y, ptRightBottom.m_x - ptLeftBottom.m_x);
    const float angleUpper = (float)atan2(ptRightTop.m_y - ptLeftTop.m_y, ptRightTop.m_x - ptLeftTop.m_x);

    float fTopBottomAngle = (angleLower + angleUpper) * 0.5f;

    float fAngleLeft = (float)(atan2(ptLeftBottom.m_y - ptLeftTop.m_y, ptLeftBottom.m_x - ptLeftTop.m_x)
        - (90.f * 0.017453292519943));
    float fAngleRight = (float)(atan2(ptRightBottom.m_y - ptRightTop.m_y, ptRightBottom.m_x - ptRightTop.m_x)
        - (90.f * 0.017453292519943));
    float fLeftRightAngle = (fAngleLeft + fAngleRight) * .5f;
    o_dieAlignResult.SetAngle((fTopBottomAngle + fLeftRightAngle) * 0.5f);

    o_dieAlignResult.bAvailable = TRUE;

    return TRUE;
}

BOOL VisionInspectionKOZ2D::DoInspKOZWidth()
{
    auto* result = m_resultGroup.GetResultByName(g_szKOZInspection2DName[KOZ_INSPECTION_2D_WIDTH]);
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

    const auto& pixelToUm = getScale().pixelToUm();

    result->Resize(8);

    LPCTSTR szKOZWidthPositionID[] = {
        _T("LeftTop"),
        _T("LeftBottom"),
        _T("TopLeft"),
        _T("TopRight"),
        _T("RightTop"),
        _T("RightBottom"),
        _T("BottomLeft"),
        _T("BottomRight"),
    };

    float fDistance;
    float fSpecWidth;
    float fError;

    std::vector<Ipvm::Point32r2> vecfptCorner(2);
    Ipvm::LineEq32r lineEq;
    Ipvm::Rect32s rtROI;
    long nID = 0;
    long nDir = LEFT;

    //Left
    vecfptCorner[0] = m_sEdgeAlignResult->fptLT;
    vecfptCorner[1] = m_sEdgeAlignResult->fptLB;
    Ipvm::DataFitting::FitToLineRn(2, &vecfptCorner[0], 3.f, lineEq);
    fSpecWidth = m_vecfKOZSpecWidth_um[nDir];
    //LeftTop
    rtROI.m_left
        = (long)((vecfptCorner[0].m_x + m_kozAlignResult.fptLT.m_x) * .5f - m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_top = (long)(m_kozAlignResult.fptLT.m_y + .5f);
    rtROI.m_right
        = (long)((vecfptCorner[0].m_x + m_kozAlignResult.fptLT.m_x) * .5f + m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_bottom = (long)(m_kozAlignResult.fptLT.m_y + (2.f * m_vecfKOZSpecWidth_pxl[nDir]) + .5f);
    Ipvm::Geometry::GetDistance(lineEq, m_kozAlignResult.fptLT, fDistance);
    fError = (fDistance * pixelToUm.m_x) - fSpecWidth;
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        nID, szKOZWidthPositionID[nID], _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecWidth); //kircheis_MED2.5
    result->SetRect(nID, rtROI);
    nID++;
    //LeftBottom
    rtROI.m_left
        = (long)((vecfptCorner[1].m_x + m_kozAlignResult.fptLB.m_x) * .5f - m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_top = (long)(m_kozAlignResult.fptLB.m_y - (2.f * m_vecfKOZSpecWidth_pxl[nDir]) + .5f);
    rtROI.m_right
        = (long)((vecfptCorner[1].m_x + m_kozAlignResult.fptLB.m_x) * .5f + m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_bottom = (long)(m_kozAlignResult.fptLB.m_y + .5f);
    Ipvm::Geometry::GetDistance(lineEq, m_kozAlignResult.fptLB, fDistance);
    fError = (fDistance * pixelToUm.m_x) - fSpecWidth;
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        nID, szKOZWidthPositionID[nID], _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecWidth); //kircheis_MED2.5
    result->SetRect(nID, rtROI);
    nID++;

    //Top
    nDir = UP;
    vecfptCorner[0] = m_sEdgeAlignResult->fptLT;
    vecfptCorner[1] = m_sEdgeAlignResult->fptRT;
    Ipvm::DataFitting::FitToLineRn(2, &vecfptCorner[0], 3.f, lineEq);
    fSpecWidth = m_vecfKOZSpecWidth_um[nDir];
    //TopLeft
    rtROI.m_left = (long)(m_kozAlignResult.fptLT.m_x + .5f);
    rtROI.m_top = (long)((vecfptCorner[0].m_y + m_kozAlignResult.fptLT.m_y) * .5f - m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_right = (long)(m_kozAlignResult.fptLT.m_x + (2.f * m_vecfKOZSpecWidth_pxl[nDir]) + .5f);
    rtROI.m_bottom
        = (long)((vecfptCorner[0].m_y + m_kozAlignResult.fptLT.m_y) * .5f + m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    Ipvm::Geometry::GetDistance(lineEq, m_kozAlignResult.fptLT, fDistance);
    fError = (fDistance * pixelToUm.m_x) - fSpecWidth;
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        nID, szKOZWidthPositionID[nID], _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecWidth); //kircheis_MED2.5
    result->SetRect(nID, rtROI);
    nID++;
    //TopRight
    rtROI.m_left = (long)(m_kozAlignResult.fptRT.m_x - (2.f * m_vecfKOZSpecWidth_pxl[nDir]) + .5f);
    rtROI.m_top = (long)((vecfptCorner[1].m_y + m_kozAlignResult.fptRT.m_y) * .5f - m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_right = (long)(m_kozAlignResult.fptRT.m_x + .5f);
    rtROI.m_bottom
        = (long)((vecfptCorner[1].m_y + m_kozAlignResult.fptRT.m_y) * .5f + m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    Ipvm::Geometry::GetDistance(lineEq, m_kozAlignResult.fptRT, fDistance);
    fError = (fDistance * pixelToUm.m_x) - fSpecWidth;
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        nID, szKOZWidthPositionID[nID], _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecWidth); //kircheis_MED2.5
    result->SetRect(nID, rtROI);
    nID++;

    //Right
    nDir = RIGHT;
    vecfptCorner[0] = m_sEdgeAlignResult->fptRT;
    vecfptCorner[1] = m_sEdgeAlignResult->fptRB;
    Ipvm::DataFitting::FitToLineRn(2, &vecfptCorner[0], 3.f, lineEq);
    fSpecWidth = m_vecfKOZSpecWidth_um[nDir];
    //RightTop
    rtROI.m_left
        = (long)((vecfptCorner[0].m_x + m_kozAlignResult.fptRT.m_x) * .5f - m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_top = (long)(m_kozAlignResult.fptRT.m_y + .5f);
    rtROI.m_right
        = (long)((vecfptCorner[0].m_x + m_kozAlignResult.fptRT.m_x) * .5f + m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_bottom = (long)(m_kozAlignResult.fptRT.m_y + (2.f * m_vecfKOZSpecWidth_pxl[nDir]) + .5f);
    Ipvm::Geometry::GetDistance(lineEq, m_kozAlignResult.fptRT, fDistance);
    fError = (fDistance * pixelToUm.m_x) - fSpecWidth;
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        nID, szKOZWidthPositionID[nID], _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecWidth); //kircheis_MED2.5
    result->SetRect(nID, rtROI);
    nID++;
    //RightBottom
    rtROI.m_left
        = (long)((vecfptCorner[1].m_x + m_kozAlignResult.fptRB.m_x) * .5f - m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_top = (long)(m_kozAlignResult.fptRB.m_y - (2.f * m_vecfKOZSpecWidth_pxl[nDir]) + .5f);
    rtROI.m_right
        = (long)((vecfptCorner[1].m_x + m_kozAlignResult.fptRB.m_x) * .5f + m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_bottom = (long)(m_kozAlignResult.fptRB.m_y + .5f);
    Ipvm::Geometry::GetDistance(lineEq, m_kozAlignResult.fptRB, fDistance);
    fError = (fDistance * pixelToUm.m_x) - fSpecWidth;
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        nID, szKOZWidthPositionID[nID], _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecWidth); //kircheis_MED2.5
    result->SetRect(nID, rtROI);
    nID++;

    //Bottom
    nDir = DOWN;
    vecfptCorner[0] = m_sEdgeAlignResult->fptLB;
    vecfptCorner[1] = m_sEdgeAlignResult->fptRB;
    Ipvm::DataFitting::FitToLineRn(2, &vecfptCorner[0], 3.f, lineEq);
    fSpecWidth = m_vecfKOZSpecWidth_um[nDir];
    //BottomLeft
    rtROI.m_left = (long)(m_kozAlignResult.fptLB.m_x + .5f);
    rtROI.m_top = (long)((vecfptCorner[0].m_y + m_kozAlignResult.fptLB.m_y) * .5f - m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_right = (long)(m_kozAlignResult.fptLB.m_x + (2.f * m_vecfKOZSpecWidth_pxl[nDir]) + .5f);
    rtROI.m_bottom
        = (long)((vecfptCorner[0].m_y + m_kozAlignResult.fptLB.m_y) * .5f + m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    Ipvm::Geometry::GetDistance(lineEq, m_kozAlignResult.fptLB, fDistance);
    fError = (fDistance * pixelToUm.m_x) - fSpecWidth;
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        nID, szKOZWidthPositionID[nID], _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecWidth); //kircheis_MED2.5
    result->SetRect(nID, rtROI);
    nID++;
    //BottomRight
    rtROI.m_left = (long)(m_kozAlignResult.fptRB.m_x - (2.f * m_vecfKOZSpecWidth_pxl[nDir]) + .5f);
    rtROI.m_top = (long)((vecfptCorner[1].m_y + m_kozAlignResult.fptRB.m_y) * .5f - m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    rtROI.m_right = (long)(m_kozAlignResult.fptRB.m_x + .5f);
    rtROI.m_bottom
        = (long)((vecfptCorner[1].m_y + m_kozAlignResult.fptRB.m_y) * .5f + m_vecfKOZSpecWidth_pxl[nDir] + .5f);
    Ipvm::Geometry::GetDistance(lineEq, m_kozAlignResult.fptRB, fDistance);
    fError = (fDistance * pixelToUm.m_x) - fSpecWidth;
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        nID, szKOZWidthPositionID[nID], _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecWidth); //kircheis_MED2.5
    result->SetRect(nID, rtROI);
    nID++;

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionKOZ2D::DoInspKOZChipping(const bool detailSetupMode)
{
    auto* result = m_resultGroup.GetResultByName(g_szKOZInspection2DName[KOZ_INSPECTION_2D_CHIPPING]);
    if (result == NULL)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == NULL)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    const auto& pixelToUm = getScale().pixelToUm();
    const auto& umToPxl = getScale().umToPixel();
    long nMinBlobSize = (long)((float)m_VisionPara->m_nMinimumBlobArea_um2 * umToPxl.m_x * umToPxl.m_y + .5f);
    nMinBlobSize = (long)max(4, nMinBlobSize);
    int32_t nCurBlobNum;
    Ipvm::Rect32s rtROI = m_sEdgeAlignResult->getBodyRect32s();
    std::vector<Ipvm::Rect32s> vecrtBlob(0);
    std::vector<long> vecnDirectionOfBlob(0);
    long nMaxBlobNum = BLOB_INFO_MAX_SIZE;
    float fMergePixelDistance = (float)m_VisionPara->m_nChippingMergeDistance_um * getScale().umToPixelXY();
    CString strIdx;
    float fError;

    //검사 영상 준비
    long nChippingFrameIdx = m_VisionPara->m_ChippingInspFrameIndex.getFrameIndex();
    Ipvm::Image8u image = GetInspectionFrameImage(false, nChippingFrameIdx);
    if (image.GetMem() == nullptr)
        return FALSE;

    Ipvm::Image8u combineImage;
    if (!getReusableMemory().GetInspByteImage(combineImage))
        return false;
    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_ChippingImageProcManagePara, combineImage))
    {
        return false;
    }

    //이진 영상 준비
    Ipvm::Image8u darkThresholdImage, brightThresholdImage;
    if (!getReusableMemory().GetInspByteImage(darkThresholdImage))
        return false;
    if (!getReusableMemory().GetInspByteImage(brightThresholdImage))
        return false;

    MakeKOZChippingBackgroundImage(combineImage); //Adaptvie Threshold를 위한 배경 준비
    MakeKOZChippingThresholdImage(combineImage, m_imageChippingBackground, m_imageChippingInspMask, darkThresholdImage,
        brightThresholdImage); //이진화

    if (m_VisionPara->m_nDefectColor == DEFECT_ALL) //Color 상관없이 올라오는거 다 잡으려면 Dark와 Bright를 합친다
        Ipvm::ImageProcessing::Add(brightThresholdImage, Ipvm::Rect32s(darkThresholdImage), 0, darkThresholdImage);

    auto calcImage = (m_VisionPara->m_nDefectColor == DEFECT_BRIGHT) ? brightThresholdImage : darkThresholdImage;

    //Blob
    auto* pBlob = getReusableMemory().GetBlob();
    Ipvm::Image32s labelImage;
    if (!getReusableMemory().GetInspLongImage(labelImage))
        return FALSE;
    Ipvm::ImageProcessing::Fill(rtROI, 0, labelImage);

    std::vector<Ipvm::BlobInfo> vecObjBlobCandidate(nMaxBlobNum);
    pBlob->DoBlob(calcImage, rtROI, nMaxBlobNum, labelImage, &vecObjBlobCandidate[0], nCurBlobNum, nMinBlobSize);
    pBlob->MergeBlobsByDistance(&vecObjBlobCandidate[0], nCurBlobNum, fMergePixelDistance, labelImage);

    if (nCurBlobNum == 0) //Blob을 못 잡으면 종료
    {
        result->Resize(1);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Not detected"), _T(""), _T(""), 0, *spec, 0.f, 0.f, 0, PASS);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();

        return TRUE;
    }

    for (long nBlob = 0; nBlob < nCurBlobNum; nBlob++)
        vecrtBlob.push_back(vecObjBlobCandidate[nBlob].m_roi);
    SetDebugInfoItem(detailSetupMode, _T("Chipping - Detected blobs(candidate)"), vecrtBlob);

    //Blob의 위치 확인 및 Parmeter의 Width Length 와 Package Edge Line에 붙어 있는지를 기준으로 버릴넘 버린다.
    long nFilterdBlobNum = nCurBlobNum;
    std::vector<Ipvm::BlobInfo> vecFilterdObjBlob(0);
    CheckDirectionAndExcludeSpecOutBlob(vecObjBlobCandidate, m_VisionPara->m_nMinimumBlobWidth_um,
        m_VisionPara->m_nMinimumBlobLenght_um, nFilterdBlobNum, vecFilterdObjBlob, vecnDirectionOfBlob);
    vecrtBlob.clear();
    for (long nBlob = 0; nBlob < nFilterdBlobNum; nBlob++)
        vecrtBlob.push_back(vecFilterdObjBlob[nBlob].m_roi);
    SetDebugInfoItem(detailSetupMode, _T("Chipping - Filtered blobs(Chipping)"), vecrtBlob);
    if (nFilterdBlobNum == 0) //Blob을 못 잡으면 종료
    {
        result->Resize(1);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Not detected"), _T(""), _T(""), 0, *spec, 0.f, 0.f, 0, PASS);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();

        return TRUE;
    }
    //Blob이 KOZ 경계를 얼마나 침범 했나 확인
    float fDistBlobToPKG, fDistKOZtoPKG, fNominal;
    result->Resize(nFilterdBlobNum);
    for (long nBlob = 0; nBlob < nFilterdBlobNum; nBlob++)
    {
        auto nDir = vecnDirectionOfBlob[nBlob];
        auto objBlob = vecFilterdObjBlob[nBlob];
        //fError = GetChippingDistanceOfBlob_pxl(objBlob, nDir);
        CalcChippingDistanceValue_pxl(objBlob, labelImage, nDir, fDistBlobToPKG, fDistKOZtoPKG);
        fError = fDistBlobToPKG - fDistKOZtoPKG;
        fError *= (vecnDirectionOfBlob[nBlob] == LEFT || vecnDirectionOfBlob[nBlob] == RIGHT) ? pixelToUm.m_x
                                                                                              : pixelToUm.m_y;
        fNominal = fDistKOZtoPKG;
        fNominal *= (vecnDirectionOfBlob[nBlob] == LEFT || vecnDirectionOfBlob[nBlob] == RIGHT) ? pixelToUm.m_x
                                                                                                : pixelToUm.m_y;
        strIdx.Format(_T("%d"), nBlob + 1);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nBlob, strIdx, _T(""), _T(""), fError, *spec, 0.f, 0.f, fNominal); //kircheis_MED2.5
        result->SetRect(nBlob, objBlob.m_roi);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

void VisionInspectionKOZ2D::MakeKOZChippingAreaMask()
{
    m_imageChippingInspMask.Free();
    if (!getReusableMemory().GetInspByteImage(m_imageChippingInspMask))
        return;
    m_imageChippingInspMask.FillZero();

    const auto& umToPixel = getScale().umToPixel();

    Ipvm::Image8u imageFullPackageMask;
    if (!getReusableMemory().GetInspByteImage(imageFullPackageMask))
        return;
    imageFullPackageMask.FillZero();

    Ipvm::Image8u imageAlignedKOZMask;
    if (!getReusableMemory().GetInspByteImage(imageAlignedKOZMask))
        return;
    imageAlignedKOZMask.FillZero();

    Ipvm::Image8u imagePackageMask;
    if (!getReusableMemory().GetInspByteImage(imagePackageMask))
        return;
    imagePackageMask.FillZero();

    Ipvm::Image8u imageIgnoreMask;
    if (!getReusableMemory().GetInspByteImage(imageIgnoreMask))
        return;

    float fEdgeIgnoreOffsetX = (float)(m_VisionPara->m_nChippingIgnoreOffset_um) * umToPixel.m_x;
    float fEdgeIgnoreOffsetY = (float)(m_VisionPara->m_nChippingIgnoreOffset_um) * umToPixel.m_y;
    float fExtentionOffsetX = (float)(m_VisionPara->m_nChippingInspectionExpandOffset_um) * umToPixel.m_x;
    float fExtentionOffsetY = (float)(m_VisionPara->m_nChippingInspectionExpandOffset_um) * umToPixel.m_y;

    Ipvm::Quadrangle32r qrtPackageAlign, qrtKOZAlign;
    qrtPackageAlign = ConversionEx::ToQuadrangle32r(
        m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB);
    qrtKOZAlign = ConversionEx::ToQuadrangle32r(
        m_kozAlignResult.fptLT, m_kozAlignResult.fptRT, m_kozAlignResult.fptLB, m_kozAlignResult.fptRB);

    Ipvm::ImageProcessing::Fill(qrtPackageAlign, 255, imageFullPackageMask);
    Ipvm::ImageProcessing::Fill(qrtPackageAlign, 255, imageAlignedKOZMask);
    Ipvm::ImageProcessing::Fill(qrtPackageAlign, 255, imageIgnoreMask);
    Ipvm::ImageProcessing::Fill(qrtKOZAlign, 0, imageAlignedKOZMask);

    qrtPackageAlign.m_ltX += fEdgeIgnoreOffsetX;
    qrtPackageAlign.m_ltY += fEdgeIgnoreOffsetY;
    qrtPackageAlign.m_rtX -= fEdgeIgnoreOffsetX;
    qrtPackageAlign.m_rtY += fEdgeIgnoreOffsetY;
    qrtPackageAlign.m_lbX += fEdgeIgnoreOffsetX;
    qrtPackageAlign.m_lbY -= fEdgeIgnoreOffsetY;
    qrtPackageAlign.m_rbX -= fEdgeIgnoreOffsetX;
    qrtPackageAlign.m_rbY -= fEdgeIgnoreOffsetY;
    Ipvm::ImageProcessing::Fill(qrtPackageAlign, 0, imageIgnoreMask);

    qrtKOZAlign.m_ltX += fExtentionOffsetX;
    qrtKOZAlign.m_ltY += fExtentionOffsetY;
    qrtKOZAlign.m_rtX -= fExtentionOffsetX;
    qrtKOZAlign.m_rtY += fExtentionOffsetY;
    qrtKOZAlign.m_lbX += fExtentionOffsetX;
    qrtKOZAlign.m_lbY -= fExtentionOffsetY;
    qrtKOZAlign.m_rbX -= fExtentionOffsetX;
    qrtKOZAlign.m_rbY -= fExtentionOffsetY;

    Ipvm::ImageProcessing::Fill(qrtPackageAlign, 255, imagePackageMask);
    Ipvm::ImageProcessing::Fill(qrtPackageAlign, 255, m_imageChippingInspMask);
    Ipvm::ImageProcessing::Fill(qrtKOZAlign, 0, m_imageChippingInspMask);

    long nAlignedNotchNum = (long)m_vecEllipseNotchAlignResult.size();
    long nAlignedNotchOnKozNum = (long)m_vecEllipseKOZNotchAlignResult.size();

    if (nAlignedNotchNum > 0 && nAlignedNotchOnKozNum > 0)
    {
        Ipvm::EllipseEq32r ellipseNotch;

        Ipvm::Image8u imageNotchMask;
        if (!getReusableMemory().GetInspByteImage(imageNotchMask))
            return;
        imageNotchMask.FillZero();

        for (long nNotch = 0; nNotch < nAlignedNotchOnKozNum; nNotch++)
        {
            ellipseNotch = m_vecEllipseKOZNotchAlignResult[nNotch];
            Ipvm::ImageProcessing::Fill(ellipseNotch, 255, imageAlignedKOZMask);
            ellipseNotch.m_xradius += fExtentionOffsetX;
            ellipseNotch.m_yradius += fExtentionOffsetY;
            Ipvm::ImageProcessing::Fill(ellipseNotch, 255, imageNotchMask);
        }

        for (long nNotch = 0; nNotch < nAlignedNotchNum; nNotch++)
        {
            ellipseNotch = m_vecEllipseNotchAlignResult[nNotch];
            Ipvm::ImageProcessing::Fill(ellipseNotch, 0, imageAlignedKOZMask);
            ellipseNotch.m_xradius += fEdgeIgnoreOffsetX;
            ellipseNotch.m_yradius += fEdgeIgnoreOffsetX;
            Ipvm::ImageProcessing::Fill(ellipseNotch, 0, imageNotchMask);
            Ipvm::ImageProcessing::Fill(ellipseNotch, 0, m_imageChippingInspMask);
            Ipvm::ImageProcessing::Fill(ellipseNotch, 255, imageIgnoreMask);
            Ipvm::ImageProcessing::Fill(m_vecEllipseNotchAlignResult[nNotch], 0, imageIgnoreMask);
        }
        Ipvm::ImageProcessing::BitwiseAnd(
            imageFullPackageMask, Ipvm::Rect32s(imageFullPackageMask), imageAlignedKOZMask);
        Ipvm::ImageProcessing::BitwiseAnd(imageFullPackageMask, Ipvm::Rect32s(imageFullPackageMask), imageIgnoreMask);
        Ipvm::ImageProcessing::BitwiseAnd(imagePackageMask, Ipvm::Rect32s(imageNotchMask), imageNotchMask);
        Ipvm::ImageProcessing::Add(imageNotchMask, Ipvm::Rect32s(m_imageChippingInspMask), 0, m_imageChippingInspMask);
    }

    auto* alignMaskKOZ = getReusableMemory().AddSurfaceLayerMaskClass(_T("Measured - KOZ Aligned Mask"));
    if (alignMaskKOZ)
    {
        alignMaskKOZ->Reset();
        alignMaskKOZ->Add(imageAlignedKOZMask);
    }
    auto* alignIgnoreMaskKOZ = getReusableMemory().AddSurfaceLayerMaskClass(_T("Measured - KOZ Mask for Ignore"));
    if (alignIgnoreMaskKOZ)
    {
        alignIgnoreMaskKOZ->Reset();
        alignIgnoreMaskKOZ->Add(imageIgnoreMask);
    }
    imageIgnoreMask.FillZero();

    //여기에 Pad, Ball, Land를 Ignore하는 기능을 추가해야됨
    if (m_VisionPara->m_nUseIgnorePreprocessObj == OPT_USE)
    {
        MakeIgnoreMask(m_VisionPara->m_nIgnoreDilationCount, imageIgnoreMask);
        Ipvm::ImageProcessing::Subtract(
            imageIgnoreMask, Ipvm::Rect32s(m_imageChippingInspMask), 0, m_imageChippingInspMask);
    }

    auto* alignMask = getReusableMemory().AddSurfaceLayerMaskClass(_T("Measured - KOZ Chipping Mask"));
    if (alignMask)
    {
        alignMask->Reset();
        alignMask->Add(m_imageChippingInspMask);
    }

    m_bIsNeedToMakeChippingMask = false;
}
BOOL VisionInspectionKOZ2D::MakeIgnoreMask(long nDialationCount, Ipvm::Image8u& o_imageMask)
{
    if (o_imageMask.GetMem() == nullptr)
        return FALSE;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_imageMask), 0, o_imageMask);

    Ipvm::Image8u MaskImage;
    if (!getReusableMemory().GetInspByteImage(MaskImage))
        return FALSE;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(MaskImage), 0, MaskImage);

    long nDataNum(0);
    BOOL bIsValid = false;

    void* pPadIgnoreData
        = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_PAD_ALIGN_2D, _T("Align PAD Image"), nDataNum);
    if (pPadIgnoreData != nullptr && nDataNum > 0)
    {
        Ipvm::Image8u* pPadIgnoreimg;
        pPadIgnoreimg = (Ipvm::Image8u*)pPadIgnoreData;

        Ipvm::ImageProcessing::Add(*pPadIgnoreimg, Ipvm::Rect32s(MaskImage), 0, MaskImage);
        bIsValid = true;
    }

    void* pBallIgnoreData
        = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_BGA_BALL_2D, _T("Ball Mask Image"), nDataNum);
    if (pBallIgnoreData != nullptr && nDataNum > 0)
    {
        Ipvm::Image8u* pBallIgnoreimg;
        pBallIgnoreimg = (Ipvm::Image8u*)pBallIgnoreData;

        Ipvm::ImageProcessing::Add(*pBallIgnoreimg, Ipvm::Rect32s(MaskImage), 0, MaskImage);
        bIsValid = true;
    }

    void* pLandIgnoreData
        = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_LGA_LAND_2D, _T("Land Origin Mask Image"), nDataNum);
    if (pLandIgnoreData != nullptr && nDataNum > 0)
    {
        Ipvm::Image8u* pLandIgnoreimg;
        pLandIgnoreimg = (Ipvm::Image8u*)pLandIgnoreData;

        Ipvm::ImageProcessing::Add(*pLandIgnoreimg, Ipvm::Rect32s(MaskImage), 0, MaskImage);
        bIsValid = true;
    }

    void* pCompIgnoreData
        = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_PASSIVE_2D, _T("Passive Mask Image with Pad"), nDataNum);
    if (pCompIgnoreData != nullptr && nDataNum > 0)
    {
        Ipvm::Image8u* pCompIgnoreimg;
        pCompIgnoreimg = (Ipvm::Image8u*)pCompIgnoreData;

        Ipvm::ImageProcessing::Add(*pCompIgnoreimg, Ipvm::Rect32s(MaskImage), 0, MaskImage);
        bIsValid = true;
    }

    if (bIsValid == false)
        return false;

    Ipvm::ImageProcessing::MorphDilate(
        MaskImage, Ipvm::Rect32s(MaskImage), nDialationCount, nDialationCount, true, o_imageMask);

    return true;
}

void VisionInspectionKOZ2D::MakeKOZChippingBackgroundImage(Ipvm::Image8u combinedImage)
{
    m_imageChippingBackground.Free();
    if (!getReusableMemory().GetInspByteImage(m_imageChippingBackground))
        return;
    m_imageChippingBackground.FillZero();

    const auto& umToPixel = getScale().umToPixel();

    Ipvm::Rect32s rtROI = m_sEdgeAlignResult->getBodyRect32s();

    float fWindowSizeXlimit = (m_packageSpec.m_bodyInfoMaster->fBodySizeX - getScale().pixelToUm().m_x * 5.f);
    float fWindowSizeYlimit = (m_packageSpec.m_bodyInfoMaster->fBodySizeY - getScale().pixelToUm().m_y * 5.f);
    float fWindowSizeX_pxl = (float)min(fWindowSizeXlimit, m_VisionPara->m_nWindowSizeX_um) * umToPixel.m_x;
    float fWindowSizeY_pxl = (float)min(fWindowSizeYlimit, m_VisionPara->m_nWindowSizeY_um) * umToPixel.m_y;

    long nSpeedUpScale = 4; //연산 속도를 위해 영상 Size X,Y를 각각 1/nSpeedUpScale으로 줄인다
    CippModules::MakeGray_BackgroundImageForAdaptiveThreshold(getReusableMemory(), combinedImage,
        m_imageChippingInspMask, rtROI, nSpeedUpScale, fWindowSizeX_pxl, fWindowSizeY_pxl, m_imageChippingBackground);

    m_bIsNeedToMakeChippingBackground = false;
}

void VisionInspectionKOZ2D::MakeKOZChippingThresholdImage(const Ipvm::Image8u i_combinedImage,
    const Ipvm::Image8u i_backgroundImage, const Ipvm::Image8u i_maskImage, Ipvm::Image8u& o_darkThreshImage,
    Ipvm::Image8u& o_brightThreshImage)
{
    Ipvm::ImageProcessing::Fill((Ipvm::Rect32s)o_darkThreshImage, 0, o_darkThreshImage);
    Ipvm::ImageProcessing::Fill((Ipvm::Rect32s)o_brightThreshImage, 0, o_brightThreshImage);

    Ipvm::Rect32s rtROI = m_sEdgeAlignResult->getBodyRect32s();

    CippModules::MakeGray_ThresholdedImageForAdaptiveThreshold(getReusableMemory(), i_combinedImage, i_backgroundImage,
        i_maskImage, rtROI, m_VisionPara->m_nMinDarkContrast, m_VisionPara->m_nMinBrightContrast, false,
        o_darkThreshImage, o_brightThreshImage);

    Ipvm::ImageProcessing::BitwiseAnd(i_maskImage, Ipvm::Rect32s(i_maskImage), o_darkThreshImage);
    Ipvm::ImageProcessing::BitwiseAnd(i_maskImage, Ipvm::Rect32s(i_maskImage), o_brightThreshImage);
}

void VisionInspectionKOZ2D::CheckDirectionAndExcludeSpecOutBlob(const std::vector<Ipvm::BlobInfo> i_vecObjBlobCandidate,
    long i_nMinWidth_um, long i_nMinLength_um, long& io_nBlobNum, std::vector<Ipvm::BlobInfo>& o_vecFilterdObjBlob,
    std::vector<long>& o_vecnDirectionOfBlob)
{
    o_vecFilterdObjBlob.clear();
    o_vecnDirectionOfBlob.clear();
    long nBlobNum = io_nBlobNum;
    io_nBlobNum = 0;
    if (nBlobNum <= 0)
        return;

    const auto& umToPxl = getScale().umToPixel();
    float fDist(0.f), fMinDist(9999999999.f);
    long nDirectionOfBlob{};
    long nMinWidthLR_pxl = (long)(i_nMinWidth_um * umToPxl.m_y + .5f);
    long nMinWidthTB_pxl = (long)(i_nMinWidth_um * umToPxl.m_x + .5f);
    long nMinLengthLR_pxl = (long)(i_nMinLength_um * umToPxl.m_x + .5f);
    long nMinLengthTB_pxl = (long)(i_nMinLength_um * umToPxl.m_y + .5f);
    float fDistMaskToPackageEdgeX_pxl = (float)m_VisionPara->m_nChippingIgnoreOffset_um * umToPxl.m_x;
    float fDistMaskToPackageEdgeY_pxl = (float)m_VisionPara->m_nChippingIgnoreOffset_um * umToPxl.m_y;

    Ipvm::Point32r2 fptBlobEdge;

    for (long nBlob = 0; nBlob < nBlobNum; nBlob++)
    {
        fMinDist = 9999999999.f;
        auto objBlob = i_vecObjBlobCandidate[nBlob];
        for (long nDir = UP; nDir <= RIGHT; nDir++) //Blob이 붙어 있는 가장 가까운 Package Edge Line으로 방향을 알고
        {
            Ipvm::Geometry::GetDistance(m_veclinePackage[nDir], objBlob.m_roi.CenterPoint32r(), fDist);
            if (fMinDist > fDist)
            {
                fMinDist = fDist;
                nDirectionOfBlob = nDir;
            }
        }

        if (nDirectionOfBlob == LEFT || nDirectionOfBlob == RIGHT) //좌우 측면에 위치한 Blob이면
        {
            if (objBlob.m_roi.Width() >= nMinLengthLR_pxl
                && objBlob.m_roi.Height() >= nMinWidthLR_pxl) //Blob의 크기가 지정한 크기보다 크면
            {
                fptBlobEdge = objBlob.m_roi.CenterPoint32r();
                if (nDirectionOfBlob == LEFT)
                    fptBlobEdge.m_x = (float)objBlob.m_roi.m_left;
                else
                    fptBlobEdge.m_x = (float)objBlob.m_roi.m_right;

                Ipvm::Geometry::GetDistance(m_veclinePackage[nDirectionOfBlob], fptBlobEdge, fDist);

                if (fDist < (fDistMaskToPackageEdgeX_pxl
                        + 3.f)) //Edge에 붙어 있는지 확인한다. 붙어 있어야 이 Blob이 검사할 가치가 있다고 본다
                {
                    o_vecFilterdObjBlob.push_back(objBlob);
                    o_vecnDirectionOfBlob.push_back(nDirectionOfBlob);
                    io_nBlobNum++;
                }
            }
        }
        else //상하 측면에 위치한 Blob이면
        {
            if (objBlob.m_roi.Width() >= nMinWidthTB_pxl
                && objBlob.m_roi.Height() >= nMinLengthTB_pxl) //Blob의 크기가 지정한 크기보다 크면
            {
                fptBlobEdge = objBlob.m_roi.CenterPoint32r();
                if (nDirectionOfBlob == UP)
                    fptBlobEdge.m_y = (float)objBlob.m_roi.m_top;
                else if (nDirectionOfBlob == DOWN)
                    fptBlobEdge.m_y = (float)objBlob.m_roi.m_bottom;
                else
                    continue;

                Ipvm::Geometry::GetDistance(m_veclinePackage[nDirectionOfBlob], fptBlobEdge, fDist);

                if (fDist < (fDistMaskToPackageEdgeY_pxl
                        + 3.f)) //Edge에 붙어 있는지 확인한다. 붙어 있어야 이 Blob이 검사할 가치가 있다고 본다
                {
                    o_vecFilterdObjBlob.push_back(objBlob);
                    o_vecnDirectionOfBlob.push_back(nDirectionOfBlob);
                    io_nBlobNum++;
                }
            }
        }
    }
}

float VisionInspectionKOZ2D::GetChippingDistanceOfBlob_pxl(const Ipvm::BlobInfo objBlobInfo, const long nDirection)
{
    if (nDirection < UP || nDirection > RIGHT)
        return 0.f;

    auto lineEq = m_veclineKOZ[nDirection];
    Ipvm::Point32r2 fptOnLine;
    Ipvm::Point32r2 fptEdge = objBlobInfo.m_roi.CenterPoint32r();
    float fDist;

    switch (nDirection)
    {
        case UP:
            fptEdge.m_y = (float)objBlobInfo.m_roi.m_bottom;
            break;
        case DOWN:
            fptEdge.m_y = (float)objBlobInfo.m_roi.m_top;
            break;
        case LEFT:
            fptEdge.m_x = (float)objBlobInfo.m_roi.m_right;
            break;
        case RIGHT:
            fptEdge.m_x = (float)objBlobInfo.m_roi.m_left;
            break;
    }

    Ipvm::Geometry::GetDistance(lineEq, fptEdge, fDist);
    Ipvm::Geometry::GetFootOfPerpendicular(lineEq, fptEdge, fptOnLine);

    switch (nDirection)
    {
        case UP:
        {
            if (fptEdge.m_y < fptOnLine.m_y)
                return -fDist;
        }
        break;
        case DOWN:
        {
            if (fptEdge.m_y > fptOnLine.m_y)
                return -fDist;
        }
        break;
        case LEFT:
        {
            if (fptEdge.m_x < fptOnLine.m_x)
                return -fDist;
        }
        break;
        case RIGHT:
        {
            if (fptEdge.m_x > fptOnLine.m_x)
                return -fDist;
        }
        break;
    }

    return fDist;
}

void VisionInspectionKOZ2D::CalcChippingDistanceValue_pxl(const Ipvm::BlobInfo i_objBlobInfo,
    const Ipvm::Image32s i_blobLabel, const long i_nDirection, float& o_fChippingTotalLength, float& o_fKOZWidth)
{
    o_fChippingTotalLength = o_fKOZWidth = 0.f;
    if (i_nDirection < UP || i_nDirection > RIGHT)
        return;

    auto lineEqKOZ = m_veclineKOZ[i_nDirection];
    auto lineEqPkg = m_veclinePackage[i_nDirection];

    Ipvm::Point32r2 fptEdgePoint, fptProjPkgLine, fptCrossPointToProjLineKOZ;
    std::vector<Ipvm::Point32r2> vecfptProjectionLine(2);
    Ipvm::LineEq32r lineProj;

    GetChippingBlobInnerPoint(i_objBlobInfo, i_blobLabel, i_nDirection, fptEdgePoint);

    Ipvm::Geometry::GetFootOfPerpendicular(lineEqPkg, fptEdgePoint, fptProjPkgLine);
    vecfptProjectionLine[0] = fptEdgePoint;
    vecfptProjectionLine[1] = fptProjPkgLine;
    Ipvm::DataFitting::FitToLineRn(2, &vecfptProjectionLine[0], 3.f, lineProj);
    Ipvm::Geometry::GetCrossPoint(lineProj, lineEqKOZ, fptCrossPointToProjLineKOZ);

    Ipvm::Geometry::GetDistance(fptEdgePoint, fptProjPkgLine, o_fChippingTotalLength);
    Ipvm::Geometry::GetDistance(fptCrossPointToProjLineKOZ, fptProjPkgLine, o_fKOZWidth);
}

void VisionInspectionKOZ2D::GetChippingBlobInnerPoint(const Ipvm::BlobInfo i_objBlobInfo,
    const Ipvm::Image32s i_blobLabel, const long i_nDirection, Ipvm::Point32r2& o_point)
{
    o_point = Ipvm::Point32r2(0.f, 0.f);
    if (i_nDirection < UP || i_nDirection > RIGHT)
        return;

    Ipvm::Point32s2 ptStart;
    Ipvm::Rect32s rtBlob(i_objBlobInfo.m_roi);
    long nRetryCnt = 0;

    switch (i_nDirection)
    {
        case LEFT:
            ptStart = rtBlob.TopRight();
            break;
        case UP:
            ptStart = rtBlob.BottomLeft();
            break;
        case RIGHT:
        case DOWN:
            ptStart = rtBlob.TopLeft();
            break;
        default:
            return;
    }

    if (i_nDirection == UP || i_nDirection == DOWN)
    {
        while (nRetryCnt++ < 5)
        {
            auto* label_y = i_blobLabel.GetMem(0, ptStart.m_y);
            for (long nX = ptStart.m_x; nX < rtBlob.m_right; nX++)
            {
                if (label_y[nX] == i_objBlobInfo.m_label)
                {
                    o_point.Set((float)nX, (float)ptStart.m_y);
                    return;
                }
            }
            if (i_nDirection == UP)
                ptStart.m_y--;
            else
                ptStart.m_y++;
        }
    }
    else
    {
        while (nRetryCnt++ < 5)
        {
            for (long nY = ptStart.m_y; nY < rtBlob.m_bottom; nY++)
            {
                auto* label_y = i_blobLabel.GetMem(0, nY);
                if (label_y[ptStart.m_x] == i_objBlobInfo.m_label)
                {
                    o_point.Set((float)ptStart.m_x, (float)nY);
                    return;
                }
            }
            if (i_nDirection == LEFT)
                ptStart.m_x--;
            else
                ptStart.m_x++;
        }
    }
}

float VisionInspectionKOZ2D::GetCurrentEdgeThresholdValueKOZ()
{
    switch (m_VisionPara->m_nEdgeThresholdLevel)
    {
        case EDGE_THRESHOLD_LEVEL_LOW:
            return KOZ2D_EDGE_THRESH_LEVEL_LOW;
        case EDGE_THRESHOLD_LEVEL_MIDDLE:
            return KOZ2D_EDGE_THRESH_LEVEL_MID;
        case EDGE_THRESHOLD_LEVEL_HIGH:
            return KOZ2D_EDGE_THRESH_LEVEL_HIGH;
        case EDGE_THRESHOLD_LEVEL_HIGHEST:
            return KOZ2D_EDGE_THRESH_LEVEL_HIGHEST;
    }

    return KOZ2D_EDGE_THRESH_LEVEL_LOW;
}