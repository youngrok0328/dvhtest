//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionLgaBottom2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionLgaBottom2D.h"
#include "LandInspectionResult.h"
#include "VisionInspectionLgaBottom2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Algorithm/TinyMatrix.h>
#include <Ipvm/Base/CircleEq32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionLgaBottom2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionLgaBottom2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    //{{ //kircheis_3DEmpty
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    Ipvm::TimeCheck Time;

    ResetResult();

    // 임시
    BOOL bInsp = DoInsp(detailSetupMode);

    m_fCalcTime = CAST_FLOAT(Time.Elapsed_ms());

    return bInsp;
}

BOOL VisionInspectionLgaBottom2D::DoInsp(const bool detailSetupMode)
{
    Ipvm::Image8u imageThreshold;
    Ipvm::Image8u imageFM;

    if (!getReusableMemory().GetInspByteImage(imageThreshold))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(imageFM))
        return FALSE;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageThreshold), 0, imageThreshold);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageFM), 0, imageFM);

    //BOOL bSurfaceRotateCoord = TRUE; // Mask나 영역 Surface연동 시 Rotate하여 연동. // 영상을 상위에서 미리 Rotate하는 것이 속도면이나 검사면이나 다 유리한데...
    // 좌표 Rotate시 Package의 틀어짐과 Land의 각을 같이 고려해야 한다.
    BOOL bSurfaceRotateCoord = FALSE; // Surface로 넘겨봐야 할 수 있는게 없네...

    Ipvm::Image8u image = GetInspectionFrameImage();

    if (image.GetMem() == NULL)
    {
        return FALSE;
    }

    BOOL bInspResult = TRUE;
    if (!ResetInspItem())
    {
        return FALSE;
    }

    m_result->sfrtPackageBody = FPI_RECT(
        m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB);

    Ipvm::Rect32s rtInspectionROI = m_result->sfrtPackageBody.GetExtCRect();
    rtInspectionROI.InflateRect(10, 10, 10, 10);

    Ipvm::Point32r2 fptSpecGravityCenterbyOrigin;
    Ipvm::Point32r2 fptSpecGravityCenter;
    if (!GetSpecLandCenterByBody(m_sEdgeAlignResult->m_angle_rad, m_result->vec2fptSpecLandbyOrigin,
            m_result->vec2fptSpecLand, fptSpecGravityCenterbyOrigin, fptSpecGravityCenter))
        return FALSE;

    std::vector<Ipvm::Point32r2> vecDebugfptSpecLandbyOrigin;
    for (long n = 0; n < (long)m_result->vec2fptSpecLandbyOrigin.size(); n++)
        vecDebugfptSpecLandbyOrigin.insert(vecDebugfptSpecLandbyOrigin.end(),
            m_result->vec2fptSpecLandbyOrigin[n].begin(), m_result->vec2fptSpecLandbyOrigin[n].end());
    SetDebugInfoItem(detailSetupMode, _T("Spec Land Raw Point by Origin"), vecDebugfptSpecLandbyOrigin);

    std::vector<Ipvm::Point32r2> vecDebugfptSpecLand;
    for (long n = 0; n < (long)m_result->vec2fptSpecLand.size(); n++)
        vecDebugfptSpecLand.insert(
            vecDebugfptSpecLand.end(), m_result->vec2fptSpecLand[n].begin(), m_result->vec2fptSpecLand[n].end());
    SetDebugInfoItem(detailSetupMode, _T("Spec Land Point"), vecDebugfptSpecLand);

    // Object Spec ROI
    std::vector<std::vector<PI_RECT>> vec2srtSpecLand;
    std::vector<std::vector<std::vector<PI_RECT>>> vec3srtSearchLand;
    MakeLandROI(m_vec2LandInfoPerLayer, m_result->vec2fptSpecLand, vec2srtSpecLand, m_result->vec2sfrtSpecLand,
        vec3srtSearchLand, m_result->vec3sfrtSearchLand);

    std::vector<Ipvm::Image8u> vecLandMask;

    for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
    {
        Ipvm::Rect32s rtLandMask;
        if (m_result->vec2sfrtSpecLand[nLayer].size())
        {
            rtLandMask = m_result->vec2sfrtSpecLand[nLayer][0].GetExtCRect();
        }

        float fExtRatio = 0.3f;
        float fExtX = (float)rtLandMask.Width() / 2.f * fExtRatio;
        float fExtY = (float)rtLandMask.Height() / 2.f * fExtRatio;
        rtLandMask.InflateRect(long(fExtX + .5f), long(fExtY + .5f));

        Ipvm::Image8u imgLandMask;
        if (!getReusableMemory().GetByteImage(imgLandMask, rtLandMask.Width(), rtLandMask.Height()))
        {
            return FALSE;
        }
        vecLandMask.push_back(imgLandMask);
    }

    // {{ Rough Align - Using Blob
    // 혹시나 속도에 우선순위를 둔다면 Blob만으로 모든 치수 데이터를 뽑자..
    std::vector<std::vector<Ipvm::Point32r2>> vec2fptBlobCenter;
    std::vector<std::vector<Ipvm::Point32r2>> vec2fptBlobMassCenter;
    Ipvm::Point32r2 fptBlobGravityCenter = Ipvm::Point32r2(0.f, 0.f);
    Ipvm::Point32r2 fptBlobMassGravityCenter = Ipvm::Point32r2(0.f, 0.f);
    if (!RoughAlignLand(image, imageThreshold, m_result->vec2sfrtSpecLand, m_result->vec2rtBlobObj, vec2fptBlobCenter,
            vec2fptBlobMassCenter, fptBlobGravityCenter, fptBlobMassGravityCenter, vecLandMask))
    {
        return FALSE;
    }

    // Blob 중심으로 Spec위치 이동: 외곽 정보보다 더 잘 맞겠지..
    std::vector<std::vector<Ipvm::Point32r2>> vecfptShiftSpecLand((long)m_result->vec2fptSpecLand.size());
    Ipvm::Point32r2 fptBlobShiftGravity = Ipvm::Point32r2(0.f, 0.f); //fptBlobGravityCenter - fptSpecGravityCenter;
    for (long nLayer = 0; nLayer < (long)m_result->vec2fptSpecLand.size(); nLayer++)
    {
        long nLandNum = (long)m_result->vec2fptSpecLand[nLayer].size();
        vecfptShiftSpecLand[nLayer].resize(nLandNum);
        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            vecfptShiftSpecLand[nLayer][nLand] = m_result->vec2fptSpecLand[nLayer][nLand] + fptBlobShiftGravity;
        }
    }
    MakeLandROI(m_vec2LandInfoPerLayer, vecfptShiftSpecLand, vec2srtSpecLand, m_result->vec2sfrtSpecLand,
        vec3srtSearchLand, m_result->vec3sfrtSearchLand);
    //}}

    std::vector<PI_RECT> vecDebugsrtSpecLand;
    for (long n = 0; n < (long)vec2srtSpecLand.size(); n++)
        vecDebugsrtSpecLand.insert(vecDebugsrtSpecLand.end(), vec2srtSpecLand[n].begin(), vec2srtSpecLand[n].end());
    SetDebugInfoItem(detailSetupMode, _T("Spec Land ROI"), vecDebugsrtSpecLand);
    std::vector<PI_RECT> vecDebugsrtSearchLand;
    for (long i = 0; i < (long)vec3srtSearchLand.size(); i++)
    {
        for (long j = 0; j < (long)vec3srtSearchLand[i].size(); j++)
            vecDebugsrtSearchLand.insert(
                vecDebugsrtSearchLand.end(), vec3srtSearchLand[i][j].begin(), vec3srtSearchLand[i][j].end());
    }
    SetDebugInfoItem(detailSetupMode, _T("Land Search ROI"), vecDebugsrtSearchLand);

    Ipvm::Point32r2 fptGravityCenter;
    if (!AlignLand(detailSetupMode, image, m_result->vec2fptSpecLand, m_result->vec3sfrtSearchLand,
            m_result->vec2sfrtLand, m_result->vec2fptLandCenter, m_result->vec2fWidth, m_result->vec2fLength,
            m_result->vec2bAlignResult, fptGravityCenter)) //kircheis_LandMissing
    {
        return FALSE;
    }

    // Align된 좌표에 Spec의 위치를 맞춘다.
    std::vector<Ipvm::Point32r2> vecDebugfptShiftSpecLand;
    Ipvm::Point32r2 fptShiftGravity = fptGravityCenter - fptSpecGravityCenter;

    for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
    {
        long nLandNum = (long)m_result->vec2fptSpecLand[nLayer].size();
        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            Ipvm::Point32r2 fptShiftSpec = m_result->vec2fptSpecLand[nLayer][nLand] + fptShiftGravity;
            m_result->vec2fptSpecLand[nLayer][nLand] = fptShiftSpec;
            vecDebugfptShiftSpecLand.push_back(fptShiftSpec);
        }
    }
    SetDebugInfoItem(detailSetupMode, _T("Spec Land Point"), vecDebugfptShiftSpecLand);

    // 외곽 Align 오차 및 Package 크기 오차로 인해 기준점을 다시 계산.
    std::vector<Ipvm::Point32r2> vecDebugfptShiftLandbyOrigin;
    std::vector<Ipvm::Point32r2> vecDebugfptShiftSpecLandbyOrigin;
    ModifyLandPoints(fptSpecGravityCenterbyOrigin, m_result->vec2fptLandCenter, m_result->vec2fptLandbyOrigin,
        m_result->vec2fptSpecLandbyOrigin);

    for (long n = 0; n < (long)m_result->vec2fptLandbyOrigin.size(); n++)
        vecDebugfptShiftLandbyOrigin.insert(vecDebugfptShiftLandbyOrigin.end(),
            m_result->vec2fptLandbyOrigin[n].begin(), m_result->vec2fptLandbyOrigin[n].end());
    for (long n = 0; n < (long)m_result->vec2fptSpecLandbyOrigin.size(); n++)
        vecDebugfptShiftSpecLandbyOrigin.insert(vecDebugfptShiftSpecLandbyOrigin.end(),
            m_result->vec2fptSpecLandbyOrigin[n].begin(), m_result->vec2fptSpecLandbyOrigin[n].end());
    SetDebugInfoItem(detailSetupMode, _T("Spec Land Point by Origin"), vecDebugfptShiftSpecLandbyOrigin);
    SetDebugInfoItem(detailSetupMode, _T("Land Point by Origin"), vecDebugfptShiftLandbyOrigin);

    if (bSurfaceRotateCoord) // Surface 검사 기준 좌표로 변환한다.
    {
        std::vector<std::vector<PI_RECT>> vec2srtLand;
        std::vector<std::vector<FPI_RECT>> vec2sfrtLand;
        RotateLandROI(m_vec2LandInfoPerLayer, m_result->vec2fptLandbyOrigin, vec2srtLand, vec2sfrtLand);

        std::vector<PI_RECT> vecsrtTransferLand;
        for (long n = 0; n < (long)vec2srtLand.size(); n++)
            vecsrtTransferLand.insert(vecsrtTransferLand.end(), vec2srtLand[n].begin(), vec2srtLand[n].end());
        SetDebugInfoItem(detailSetupMode, _T("Land Measured ROI"), vecsrtTransferLand);

        // {{ Mask Image 생성.
        // Blob의 중심점과 실제 Land의 중심점의 차 계산.

        std::vector<Ipvm::Point32r2> vecfptDiffBlobCenter(m_group_id_list.size());
        std::vector<std::vector<Ipvm::Point32r2>> vec2fptDiffBlobCenter((long)vec2fptBlobCenter.size());

        Ipvm::Point32r2 bodyCenter = m_sEdgeAlignResult->m_center;

        for (long nLayer = 0; nLayer < (long)vec2fptBlobCenter.size(); nLayer++)
        {
            vecfptDiffBlobCenter[nLayer] = Ipvm::Point32r2(0.f, 0.f);
            vec2fptDiffBlobCenter[nLayer].resize((long)vec2fptBlobCenter[nLayer].size());
            for (long nLand = 0; nLand < (long)vec2fptBlobCenter[nLayer].size(); nLand++)
            {
                CPI_Geometry::RotatePoint(
                    vec2fptBlobCenter[nLayer][nLand], -m_sEdgeAlignResult->m_angle_rad, bodyCenter);

                vec2fptDiffBlobCenter[nLayer][nLand]
                    = vec2fptBlobCenter[nLayer][nLand] - m_result->vec2fptLandbyOrigin[nLayer][nLand];
                vecfptDiffBlobCenter[nLayer].m_x += vec2fptDiffBlobCenter[nLayer][nLand].m_x;
                vecfptDiffBlobCenter[nLayer].m_y += vec2fptDiffBlobCenter[nLayer][nLand].m_y;
            }
            vecfptDiffBlobCenter[nLayer].m_x /= (float)vec2fptBlobCenter[nLayer].size();
            vecfptDiffBlobCenter[nLayer].m_y /= (float)vec2fptBlobCenter[nLayer].size();
        }

        Ipvm::ImageProcessing::Fill(rtInspectionROI, 0, imageThreshold);

        for (long nLayer = 0; nLayer < (long)m_result->vec2fptLandbyOrigin.size(); nLayer++)
        {
            Ipvm::Image8u& LandMask = vecLandMask[nLayer];

            long nLandNum = (long)m_result->vec2fptLandbyOrigin[nLayer].size();
            for (long nLand = 0; nLand < nLandNum; nLand++)
            {
                Ipvm::Point32r2 fptLandCenter
                    = m_result->vec2fptLandbyOrigin[nLayer][nLand] + vecfptDiffBlobCenter[nLayer];
                Ipvm::Rect32s rtDestLand;
                rtDestLand.m_left = long(fptLandCenter.m_x - LandMask.GetSizeX() / 2.f + .5f);
                rtDestLand.m_top = long(fptLandCenter.m_y - LandMask.GetSizeY() / 2.f + .5f);
                rtDestLand.m_right = rtDestLand.m_left + LandMask.GetSizeX();
                rtDestLand.m_bottom = rtDestLand.m_top + LandMask.GetSizeY();

                Ipvm::ImageProcessing::Copy(LandMask, Ipvm::Rect32s(LandMask), rtDestLand, imageThreshold);
            }
        }

        SetDebugInfoItem(detailSetupMode, _T("Land Origin Mask Image"), imageThreshold, true);
    }
    else
    {
        std::vector<PI_RECT> vecsrtTransferLand;
        for (long n = 0; n < (long)m_result->vec2sfrtLand.size(); n++)
        {
            for (long n2 = 0; n2 < (long)m_result->vec2sfrtLand[n].size(); n2++)
            {
                vecsrtTransferLand.push_back(m_result->vec2sfrtLand[n][n2].GetSPI_RECT());
            }
        }
        SetDebugInfoItem(detailSetupMode, _T("Land Measured ROI"), vecsrtTransferLand);

        // {{ Mask Image 생성.
        // Blob의 중심점과 실제 Land의 중심점의 차 계산.
        std::vector<Ipvm::Point32r2> vecfptDiffBlobCenter(m_group_id_list.size());
        std::vector<std::vector<Ipvm::Point32r2>> vec2fptDiffBlobCenter((long)vec2fptBlobCenter.size());
        for (long nLayer = 0; nLayer < (long)vec2fptBlobCenter.size(); nLayer++)
        {
            vecfptDiffBlobCenter[nLayer] = Ipvm::Point32r2(0.f, 0.f);
            vec2fptDiffBlobCenter[nLayer].resize((long)vec2fptBlobCenter[nLayer].size());
            for (long nLand = 0; nLand < (long)vec2fptBlobCenter[nLayer].size(); nLand++)
            {
                vec2fptDiffBlobCenter[nLayer][nLand]
                    = vec2fptBlobCenter[nLayer][nLand] - m_result->vec2fptLandCenter[nLayer][nLand];
                vecfptDiffBlobCenter[nLayer].m_x += vec2fptDiffBlobCenter[nLayer][nLand].m_x;
                vecfptDiffBlobCenter[nLayer].m_y += vec2fptDiffBlobCenter[nLayer][nLand].m_y;
            }
            vecfptDiffBlobCenter[nLayer].m_x /= (float)vec2fptBlobCenter[nLayer].size();
            vecfptDiffBlobCenter[nLayer].m_y /= (float)vec2fptBlobCenter[nLayer].size();
        }

        //0으로 검사영역 초기화
        Ipvm::ImageProcessing::Fill(rtInspectionROI, 0, imageThreshold);
        //resize
        //m_result->vec2fAverage.resize((long)m_result->vec2sfrtLand.size());
        m_result->vec2fFMAreaRatio.resize((long)m_result->vec2sfrtLand.size());
        for (long nLayer = 0; nLayer < (long)m_result->vec2sfrtLand.size(); nLayer++)
        {
            //Layer를 하나 받아와 정보를 imageLandMask에게 저장
            Ipvm::Image8u& LandMask = vecLandMask[nLayer];

            //resize
            long nLandNum = (long)m_result->vec2sfrtLand[nLayer].size();
            //m_result->vec2fAverage[nLayer].resize(nLandNum);
            m_result->vec2fFMAreaRatio[nLayer].resize(nLandNum);
            for (long nLand = 0; nLand < nLandNum; nLand++)
            {
                //검사할 Land 중심 좌표에 Blob 중심점과 실제 중심점 간 차의 평균을 더한 후 검사 영역 위치 계산
                Ipvm::Point32r2 fptLandCenter
                    = m_result->vec2sfrtLand[nLayer][nLand].GetCenter() + vecfptDiffBlobCenter[nLayer];
                //그런데 이렇게하면 사각형상태이기만 해서 Land 전체를 검사 할 수 있나?
                Ipvm::Rect32s rtDestLand;
                rtDestLand.m_left = long(fptLandCenter.m_x - LandMask.GetSizeX() / 2.f + .5f);
                rtDestLand.m_top = long(fptLandCenter.m_y - LandMask.GetSizeY() / 2.f + .5f);
                rtDestLand.m_right = rtDestLand.m_left + LandMask.GetSizeX();
                rtDestLand.m_bottom = rtDestLand.m_top + LandMask.GetSizeY();

                //검사영역 복사
                Ipvm::ImageProcessing::Copy(LandMask, Ipvm::Rect32s(LandMask), rtDestLand, imageThreshold);
            }
        }

        SetDebugInfoItem(detailSetupMode, _T("Land Origin Mask Image"), imageThreshold, true);

        auto* alignMask = getReusableMemory().AddSurfaceLayerMaskClass(_T("Land Aligned Mask Image"));
        if (alignMask)
        {
            alignMask->Reset();
            alignMask->Add(imageThreshold);
        }

        long nGroupSize = (long)m_group_id_list.size();

        for (long groupIndex = 0; groupIndex < nGroupSize; groupIndex++)
        {
            Ipvm::Image8u layerMask;
            Ipvm::Image8u tempImage;
            if (!getReusableMemory().GetInspByteImage(layerMask))
                return FALSE;
            if (!getReusableMemory().GetInspByteImage(tempImage))
                return FALSE;

            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(tempImage), 0, tempImage);

            long nLandNum = (long)m_result->vec2sfrtLand[groupIndex].size();
            for (long nLand = 0; nLand < nLandNum; nLand++)
            {
                auto roi = m_result->vec2sfrtLand[groupIndex][nLand].GetSPI_RECT();
                roi.Offset(1, 1, FALSE);
                FillArbitraryRect(roi, 255, tempImage);
            }

            if (Ipvm::ImageProcessing::BitwiseAnd(imageThreshold, tempImage, Ipvm::Rect32s(imageThreshold), layerMask)
                != Ipvm::Status::e_ok)
            {
                return FALSE;
            }

            CString layerName;
            CString strGroupID = m_group_id_list[groupIndex];
            if (strGroupID.GetAt(0) == 'G')
                strGroupID.Delete(0);
            layerName.Format(_T("Measured - Land Group%s"), LPCTSTR(strGroupID));
            auto* surfaceLayerMask = getReusableMemory().AddSurfaceLayerMaskClass(layerName);
            surfaceLayerMask->Reset();
            surfaceLayerMask->Add(layerMask);

            layerName.Empty();
            strGroupID.Empty();
        }
    }

    if (!GetBallCenterByLand())
        return FALSE;

    bInspResult &= DoLandAlign(); //kircheis_LandMissing
    bInspResult &= DoLandOffsetX();
    bInspResult &= DoLandOffsetY();
    bInspResult &= DoLandOffsetR();
    bInspResult &= DoLandWidth();
    bInspResult &= DoLandLength();

    return bInspResult;
}

BOOL VisionInspectionLgaBottom2D::ModifyLandPoints(Ipvm::Point32r2 fptSpecGravityCenterbyOrigin,
    std::vector<std::vector<Ipvm::Point32r2>> vec2fptLandCenter,
    std::vector<std::vector<Ipvm::Point32r2>>& vec2fptLandbyOrigin,
    std::vector<std::vector<Ipvm::Point32r2>>& vec2fptSpecLandbyOrigin)
{
    long nLayerNum = (long)m_vec2LandInfoPerLayer.size();

    // Align된 좌표에 Spec의 위치를 맞춘다. (원점 기준)
    long nTotalLandNum = 0;
    vec2fptLandbyOrigin.resize(nLayerNum);
    Ipvm::Point32r2 fptGravityCenterbyOrigin = Ipvm::Point32r2(0.f, 0.f);
    Ipvm::Point32r2 bodyCenter = m_sEdgeAlignResult->m_center;

    for (long nLayer = 0; nLayer < nLayerNum; nLayer++)
    {
        long nLandNum = (long)vec2fptLandCenter[nLayer].size();
        nTotalLandNum += nLandNum;
        vec2fptLandbyOrigin[nLayer].resize(nLandNum);
        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            CPI_Geometry::RotatePoint(vec2fptLandbyOrigin[nLayer][nLand], -m_sEdgeAlignResult->m_angle_rad, bodyCenter);

            fptGravityCenterbyOrigin += vec2fptLandbyOrigin[nLayer][nLand];
        }
    }

    fptGravityCenterbyOrigin.m_x /= (float)nTotalLandNum;
    fptGravityCenterbyOrigin.m_y /= (float)nTotalLandNum;

    Ipvm::Point32r2 fptShiftGravitybyOrigin = fptGravityCenterbyOrigin - fptSpecGravityCenterbyOrigin;
    for (long nLayer = 0; nLayer < nLayerNum; nLayer++)
    {
        long nLandNum = (long)vec2fptSpecLandbyOrigin[nLayer].size();
        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            Ipvm::Point32r2 fptShiftSpec = vec2fptSpecLandbyOrigin[nLayer][nLand] + fptShiftGravitybyOrigin;
            vec2fptSpecLandbyOrigin[nLayer][nLand] = fptShiftSpec;
        }
    }

    return TRUE;
}

void VisionInspectionLgaBottom2D::SetDefaultLandROI(const Ipvm::Point32r2 i_SpecCenter, const float fLandHalfWidth,
    const float fLandHalfLength, const float fCurLayerAngle, Ipvm::Point32r2& o_fptLB, Ipvm::Point32r2& o_fptRB,
    Ipvm::Point32r2& o_fptLT, Ipvm::Point32r2& o_fptRT) //kircheis_LandMissing
{
    Ipvm::Rect32r frtLand;
    frtLand.m_left = (i_SpecCenter.m_x - (fLandHalfWidth));
    frtLand.m_top = (i_SpecCenter.m_y - (fLandHalfLength));
    frtLand.m_right = (i_SpecCenter.m_x + (fLandHalfWidth));
    frtLand.m_bottom = (i_SpecCenter.m_y + (fLandHalfLength));
    FPI_RECT sfrtLand
        = FPI_RECT(Ipvm::Point32r2(frtLand.m_left, frtLand.m_top), Ipvm::Point32r2(frtLand.m_right, frtLand.m_top),
            Ipvm::Point32r2(frtLand.m_left, frtLand.m_bottom), Ipvm::Point32r2(frtLand.m_right, frtLand.m_bottom));

    sfrtLand = sfrtLand.Rotate(fCurLayerAngle);

    o_fptLT = sfrtLand.fptLT;
    o_fptRT = sfrtLand.fptRT;
    o_fptLB = sfrtLand.fptLB;
    o_fptRB = sfrtLand.fptRB;
}

BOOL VisionInspectionLgaBottom2D::AlignLand(const bool detailSetupMode, const Ipvm::Image8u& image,
    std::vector<std::vector<Ipvm::Point32r2>> vec2fptSpecLand,
    std::vector<std::vector<std::vector<FPI_RECT>>> vec3sfrtSearchLand,
    std::vector<std::vector<FPI_RECT>>& o_vec2sfrtLand, std::vector<std::vector<Ipvm::Point32r2>>& o_vec2fptLandCenter,
    std::vector<std::vector<float>>& o_vec2fWidth, std::vector<std::vector<float>>& o_vec2fLength,
    std::vector<std::vector<BOOL>>& o_vec2bAlignResult,
    Ipvm::Point32r2& o_fptGravityCenter) //kircheis_LandMissing
{
    o_vec2sfrtLand.clear();
    o_vec2fptLandCenter.clear();
    o_vec2bAlignResult.clear(); //kircheis_LandMissing
    o_vec2fWidth.clear();
    o_vec2fLength.clear();
    o_fptGravityCenter = Ipvm::Point32r2(0.f, 0.f);

    m_rough_vecSweepLine.clear(); //kircheis_LandShape
    m_vecSweepLine.clear(); //kircheis_LandShape

    Ipvm::Point32r2 mm2Px = getScale().mmToPixel();
    float fPixelperMM = (mm2Px.m_x + mm2Px.m_y) / 2.f;
    float fTolerance = 0.03f * fPixelperMM;

    o_vec2sfrtLand.resize(m_group_id_list.size());
    o_vec2fptLandCenter.resize(m_group_id_list.size());
    o_vec2fWidth.resize(m_group_id_list.size());
    o_vec2fLength.resize(m_group_id_list.size());
    o_vec2bAlignResult.resize(m_group_id_list.size()); //kircheis_LandMissing

    std::vector<Ipvm::Point32r2> vecDebugfptLandEdgePoint;
    std::vector<Ipvm::Point32r2> vecDebugfptLandPoint;
    std::vector<PI_RECT> vecDebugsfrtLand;
    std::vector<Ipvm::EllipseEq32r> vecsCircleLand(0); //kircheis_LandShape
    Ipvm::EllipseEq32r ellipseCircleLand; //kircheis_LandShape

    long nTotalLandNum = 0;
    BOOL bRisingEdge = m_VisionPara->m_nIntensityofLand;
    for (long nLayer = 0; nLayer < (long)vec2fptSpecLand.size(); nLayer++)
    {
        long nLandNum = (long)vec2fptSpecLand[nLayer].size();
        nTotalLandNum += nLandNum;

        o_vec2sfrtLand[nLayer].resize(nLandNum);
        o_vec2fptLandCenter[nLayer].resize(nLandNum);
        o_vec2fWidth[nLayer].resize(nLandNum);
        o_vec2fLength[nLayer].resize(nLandNum);
        o_vec2bAlignResult[nLayer].resize(nLandNum); //kircheis_LandMissing
        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            float fLandHalfWidth = (m_vec2LandInfoPerLayer[nLayer][nLand].fWidth) * fPixelperMM / 2.f;
            float fLandHalfLength = (m_vec2LandInfoPerLayer[nLayer][nLand].fLength) * fPixelperMM / 2.f;
            float fLandAngle = (float)m_vec2LandInfoPerLayer[nLayer][nLand].nAngle;
            float fCurLayerAngle = fLandAngle * ITP_DEG_TO_RAD;
            Ipvm::Point32r2 fptSpecLandCenter = vec2fptSpecLand[nLayer][nLand];
            BOOL bAlignResult = true; //kircheis_LandMissing
            o_vec2bAlignResult[nLayer][nLand] = TRUE; //kircheis_LandMissing

            if (m_vec2LandInfoPerLayer[nLayer][nLand].nLandShapeType == Shape_Base_Circle) //kircheis_LandShape
            {
                //1.Edge 추출 방향 추가해야함
                //2.Rect Algorithm 사용 중인데 그것 분기해야함
                //3.Circle Parameter 추가 해야함

                Ipvm::CircleEq32r landAlignResult;
                Ipvm::Point32r2 fptAlignCenter;
                std::vector<Ipvm::Point32r2> vecfptFitEdge(0);

                if (!AlignLandForCircleShape(
                        image, fptSpecLandCenter, fLandHalfWidth, nLayer, vecfptFitEdge, landAlignResult))
                {
                    bAlignResult = false;
                    Ipvm::Point32r2 fptOffsetX(fLandHalfWidth, 0), fptOffsetY(0, fLandHalfWidth);
                    Ipvm::Point32r2 fptOffsetPlus(fLandHalfWidth, fLandHalfWidth),
                        fptOffsetCrossX(fLandHalfWidth, -fLandHalfWidth);
                    o_vec2fLength[nLayer][nLand] = o_vec2fWidth[nLayer][nLand] = fLandHalfWidth * 2;
                    o_vec2fptLandCenter[nLayer][nLand] = fptSpecLandCenter;
                    o_vec2sfrtLand[nLayer][nLand]
                        = FPI_RECT(fptSpecLandCenter - fptOffsetPlus, fptSpecLandCenter + fptOffsetCrossX,
                            fptSpecLandCenter - fptOffsetCrossX, fptSpecLandCenter + fptOffsetPlus);
                    o_vec2bAlignResult[nLayer][nLand] = FALSE;
                    continue;
                }
                //kk Parameter Offset
                Ipvm::Point32r2 fptOffsetX(landAlignResult.m_radius, 0), fptOffsetY(0, landAlignResult.m_radius);
                Ipvm::Point32r2 fptOffsetPlus(landAlignResult.m_radius, landAlignResult.m_radius),
                    fptOffsetCrossX(landAlignResult.m_radius, -landAlignResult.m_radius);

                ellipseCircleLand = Ipvm::EllipseEq32r(landAlignResult.m_x, landAlignResult.m_y,
                    landAlignResult.m_radius, landAlignResult.m_radius); //kircheis_LandShape
                vecsCircleLand.push_back(ellipseCircleLand); //kircheis_LandShape

                o_vec2fLength[nLayer][nLand] = o_vec2fWidth[nLayer][nLand] = landAlignResult.m_radius * 2;

                o_vec2fptLandCenter[nLayer][nLand] = fptAlignCenter
                    = Ipvm::Point32r2(landAlignResult.m_x, landAlignResult.m_y);
                o_fptGravityCenter.m_x += fptAlignCenter.m_x;
                o_fptGravityCenter.m_y += fptAlignCenter.m_y;
                vecDebugfptLandPoint.push_back(fptAlignCenter);

                o_vec2sfrtLand[nLayer][nLand] = FPI_RECT(fptAlignCenter - fptOffsetPlus,
                    fptAlignCenter + fptOffsetCrossX, fptAlignCenter - fptOffsetCrossX, fptAlignCenter + fptOffsetPlus);
                vecDebugsfrtLand.push_back(o_vec2sfrtLand[nLayer][nLand].GetSPI_RECT());

                vecDebugfptLandEdgePoint.insert(
                    vecDebugfptLandEdgePoint.end(), vecfptFitEdge.begin(), vecfptFitEdge.end());

                continue;
            }

            Ipvm::LineEq32r lines[4];
            std::vector<Ipvm::Point32r2> vecfptFitEdge[4];
            for (long nDir = 0; nDir < 4; nDir++)
            {
                Ipvm::LineSeg32r sLineStart, sLineEnd;
                GetSearchLine(fptSpecLandCenter, vec3sfrtSearchLand[nLayer][nLand][nDir], nDir, sLineStart, sLineEnd);

                std::vector<Ipvm::Point32r2> vecfptEdge;
                if (!FindLineEdge(image, sLineStart, sLineEnd, bRisingEdge, FALSE, vecfptEdge))
                    continue;

                if (vecfptEdge.size() <= 3) //kircheis_LandMissing
                {
                    bAlignResult = false;
                    break;
                }
                if (RemoveNoisebyLine(vecfptEdge, vecfptFitEdge[nDir], lines[nDir], fTolerance)
                    == false) //kircheis_LandMissing
                    bAlignResult = false;

                vecDebugfptLandEdgePoint.insert(
                    vecDebugfptLandEdgePoint.end(), vecfptFitEdge[nDir].begin(), vecfptFitEdge[nDir].end());
            }

            Ipvm::Point32r2 fptLB, fptRB, fptLT, fptRT;
            if (bAlignResult == false) //kircheis_LandMissing
            {
                SetDefaultLandROI(m_result->vec2fptSpecLand[nLayer][nLand], fLandHalfWidth, fLandHalfLength,
                    fCurLayerAngle, fptLB, fptRB, fptLT, fptRT); //kircheis_LandMissing
                o_vec2bAlignResult[nLayer][nLand] = FALSE;
            }
            else
            {
                if (Ipvm::Geometry::GetCrossPoint(lines[LEFT], lines[UP], fptLT) != Ipvm::Status::e_ok)
                    continue;
                if (Ipvm::Geometry::GetCrossPoint(lines[RIGHT], lines[UP], fptRT) != Ipvm::Status::e_ok)
                    continue;
                if (Ipvm::Geometry::GetCrossPoint(lines[LEFT], lines[DOWN], fptLB) != Ipvm::Status::e_ok)
                    continue;
                if (Ipvm::Geometry::GetCrossPoint(lines[RIGHT], lines[DOWN], fptRB) != Ipvm::Status::e_ok)
                    continue;

                //{{//kircheis_LandMissing
                float fAngleLT = CPI_Geometry::GetAngleBetween2Lines(fptLB, fptLT, fptRT);
                CPI_Geometry::VerifyPolygonInnerAngle(fAngleLT);
                float fAngleRT = CPI_Geometry::GetAngleBetween2Lines(fptLT, fptRT, fptRB);
                CPI_Geometry::VerifyPolygonInnerAngle(fAngleRT);
                float fAngleRB = CPI_Geometry::GetAngleBetween2Lines(fptRT, fptRB, fptLB);
                CPI_Geometry::VerifyPolygonInnerAngle(fAngleRB);
                float fAngleLB = CPI_Geometry::GetAngleBetween2Lines(fptRB, fptLB, fptLT);
                CPI_Geometry::VerifyPolygonInnerAngle(fAngleLB);

                float fMaxAngle = (float)max(max(max(fAngleLT, fAngleRT), fAngleRB), fAngleLB);
                float fMinAngle = (float)min(min(min(fAngleLT, fAngleRT), fAngleRB), fAngleLB);

                float fAngleSum = fAngleLT + fAngleRT + fAngleRB + fAngleLB;
                float fAngleGap = fMaxAngle - fMinAngle;
                if ((fAngleSum < 355.f || fAngleSum > 365.f) || fAngleGap > 65.f)
                {
                    SetDefaultLandROI(m_result->vec2fptSpecLand[nLayer][nLand], fLandHalfWidth, fLandHalfLength,
                        fCurLayerAngle, fptLB, fptRB, fptLT, fptRT); //kircheis_LandMissing
                    o_vec2bAlignResult[nLayer][nLand] = FALSE;
                }
                //}}
            }

            float fCenterLx = (fptLT.m_x + fptLB.m_x) / 2.f;
            float fCenterLy = (fptLT.m_y + fptLB.m_y) / 2.f;
            float fCenterRx = (fptRT.m_x + fptRB.m_x) / 2.f;
            float fCenterRy = (fptRT.m_y + fptRB.m_y) / 2.f;

            float fDistLtoR, fDistRtoL;
            Ipvm::Geometry::GetDistance(lines[RIGHT], Ipvm::Point32r2(fCenterLx, fCenterLy), fDistLtoR);
            Ipvm::Geometry::GetDistance(lines[LEFT], Ipvm::Point32r2(fCenterRx, fCenterRy), fDistRtoL);

            float fCenterTx = (fptLT.m_x + fptRT.m_x) / 2.f;
            float fCenterTy = (fptLT.m_y + fptRT.m_y) / 2.f;
            float fCenterBx = (fptLB.m_x + fptRB.m_x) / 2.f;
            float fCenterBy = (fptLB.m_y + fptRB.m_y) / 2.f;

            float fDistTtoB, fDistBtoT;
            Ipvm::Geometry::GetDistance(lines[DOWN], Ipvm::Point32r2(fCenterTx, fCenterTy), fDistTtoB);
            Ipvm::Geometry::GetDistance(lines[UP], Ipvm::Point32r2(fCenterBx, fCenterBy), fDistBtoT);

            //float fWidth = 0.f;
            //float fLength = 0.f;
            //if(fDistLR < fDistTB)
            //{
            //	fWidth = fDistLR;
            //	fLength = fDistTB;
            //}
            //else
            //{
            //	fWidth = fDistTB;
            //	fLength = fDistLR;
            //}
            float fWidth = (fDistLtoR + fDistRtoL) * .5f; // fDistLR;
            float fLength = (fDistTtoB + fDistBtoT) * .5f; //fDistTB;

            o_vec2fWidth[nLayer][nLand] = fWidth;
            o_vec2fLength[nLayer][nLand] = fLength;

            Ipvm::Rect32r frtLand;
            Ipvm::Point32r2 fptLandCenter = FPI_RECT(fptLT, fptRT, fptLB, fptRB).GetCenter();
            frtLand.m_left = (fptLandCenter.m_x - fLandHalfWidth);
            frtLand.m_top = (fptLandCenter.m_y - fLandHalfLength);
            frtLand.m_right = (fptLandCenter.m_x + fLandHalfWidth);
            frtLand.m_bottom = (fptLandCenter.m_y + fLandHalfLength);
            FPI_RECT sfrtLand = FPI_RECT(Ipvm::Point32r2(frtLand.m_left, frtLand.m_top),
                Ipvm::Point32r2(frtLand.m_right, frtLand.m_top), Ipvm::Point32r2(frtLand.m_left, frtLand.m_bottom),
                Ipvm::Point32r2(frtLand.m_right, frtLand.m_bottom));
            if (fLandAngle != 0)
            {
                sfrtLand = sfrtLand.Rotate(fCurLayerAngle);
            }

            o_vec2fptLandCenter[nLayer][nLand] = fptLandCenter;
            o_fptGravityCenter.m_x += fptLandCenter.m_x;
            o_fptGravityCenter.m_y += fptLandCenter.m_y;
            vecDebugfptLandPoint.push_back(fptLandCenter);

            o_vec2sfrtLand[nLayer][nLand] = sfrtLand;
            vecDebugsfrtLand.push_back(sfrtLand.GetSPI_RECT());
        }
    }

    o_fptGravityCenter.m_x /= (float)nTotalLandNum;
    o_fptGravityCenter.m_y /= (float)nTotalLandNum;

    SetDebugInfoItem(detailSetupMode, _T("Land Edge Point"), vecDebugfptLandEdgePoint);
    SetDebugInfoItem(detailSetupMode, _T("Land ROI"), vecDebugsfrtLand);
    SetDebugInfoItem(detailSetupMode, _T("Land Point"), vecDebugfptLandPoint);

    if (vecsCircleLand.size() > 0) //kircheis_LandShape
    {
        SetDebugInfoItem(detailSetupMode, _T("Circle Land Measured Circle"), vecsCircleLand);
        vecsCircleLand.clear();

        if (m_rough_vecSweepLine.size() > 0)
        {
            SetDebugInfoItem(detailSetupMode, _T("Circle Land Edge Sweep Line(Rough)"), m_rough_vecSweepLine);
            m_rough_vecSweepLine.clear();
        }
        if (m_vecSweepLine.size() > 0)
        {
            SetDebugInfoItem(detailSetupMode, _T("Circle Land Edge Sweep Line(Final)"), m_vecSweepLine);
            m_vecSweepLine.clear();
        }
    }

    return TRUE;
}

BOOL VisionInspectionLgaBottom2D::AlignLandForCircleShape(const Ipvm::Image8u& image,
    Ipvm::Point32r2 i_fptSpecLandCenter, float i_fSpecWidthPXL, long i_nLayer,
    std::vector<Ipvm::Point32r2>& o_vecptFitEdge, Ipvm::CircleEq32r& o_AlignResult) //kircheis_LandShape
{
    o_vecptFitEdge.clear();
    float fSetEdgeThreshold = m_VisionPara->m_vecLandParameter[i_nLayer].m_fThresholdValue;
    float fCurEdgeThresh = m_pEdgeDetect->SetMininumThreshold(fSetEdgeThreshold); //kircheis_ShinyTest
    BOOL bFirstEdge = (fSetEdgeThreshold == 5.f); //kk 이 코드가 있어야하는가..

    //{{//kircheis_NantongBO
    BOOL bRoughFirstEdge = TRUE; // (m_VisionPara->m_nBallWidthMethod == enumGetMaxValue);
    long nRoughLineNum;
    float fEdgeThreshOld = 0.f;
    //}}

    long nSectionNumber = m_VisionPara->m_vecLandParameter[i_nLayer].m_nEdgeSearchCount;

    UpdateSweepAngleTable(nSectionNumber, m_vecfCosForBallCenter, m_vecfSinForBallCenter); //kircheis_LandShape

    std::vector<Ipvm::Point32r2> vfCirclePointXY;

    std::vector<BYTE> vbyLineValue;
    std::vector<BYTE> vbyLineValue_Revers;

    Ipvm::CircleEq32r CircleCoeff;

    float fDistX, fDistY;
    float fEdgeRadius;
    float fSpec_Temp;
    float fRadiusSpec_Min;
    float fRadiusSpec_Max;
    float fEdgeValue;

    BOOL bSubPixelingDone;

    std::vector<float> vecfBallEdgeDist(0);
    float* pfCos = &m_vecfCosForBallCenter[0];
    float* pfSin = &m_vecfSinForBallCenter[0];
    float fSearchOffset = 1 + m_VisionPara->m_vecLandParameter[i_nLayer].m_fEdgeSearchLength;
    const float fSweepLength = float(i_fSpecWidthPXL * fSearchOffset);

    // 무게중심을 구할 영역의 볼 센터
    float fInX = i_fptSpecLandCenter.m_x;
    float fInY = i_fptSpecLandCenter.m_y;

    if (fInX <= 0 || fInY <= 0)
    {
        m_pEdgeDetect->SetMininumThreshold(fCurEdgeThresh);

        return FALSE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////러프 센터를 먼저 찾는다/////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CPoint ptStart((long)(i_fptSpecLandCenter.m_x + 0.5f), (long)(i_fptSpecLandCenter.m_y + 0.5f));

    vfCirclePointXY.clear();
    vfCirclePointXY.resize(nSectionNumber, Ipvm::Point32r2(0.f, 0.f));

    for (long nSection = 0; nSection < nSectionNumber; nSection++)
    {
        const float fOutX = fInX + fSweepLength * pfCos[nSection];
        const float fOutY = fInY + fSweepLength * pfSin[nSection];

        // 스윕 라인 상의 인센서티 샘플링
        long nLineValue
            = CippModules::GetLineData(image, fOutX, fOutY, fInX, fInY, pfCos[nSection], pfSin[nSection], NULL);
        nRoughLineNum = bRoughFirstEdge ? nLineValue / 2 : nLineValue; //kircheis_NantongBO
        vbyLineValue.clear();
        vbyLineValue.resize(nLineValue);
        CippModules::GetLineData(image, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);

        vbyLineValue_Revers = vbyLineValue;
        for (long n = 0; n < nLineValue; n++)
        {
            vbyLineValue_Revers[nLineValue - n - 1] = vbyLineValue[n];
        }
        // 스윕 라인 상의 상승 에지 구하기.
        //{{//kircheis_NantongBO
        float fEdgeT = bRoughFirstEdge ? 3.f : fCurEdgeThresh;
        fEdgeThreshOld = m_pEdgeDetect->SetMininumThreshold(fEdgeT);
        //}}
        m_rough_vecSweepLine.push_back(Ipvm::LineSeg32r(fInX, fInY, fOutX, fOutY)); //kircheis_LandShape

        if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nRoughLineNum, &vbyLineValue_Revers[0],
                fEdgeRadius, fEdgeValue, bSubPixelingDone,
                bRoughFirstEdge)) //FALSE ))//kircheis_ShinyTest//kircheis_NantongBO
        {
            m_pEdgeDetect->SetMininumThreshold(fEdgeThreshOld); //kircheis_NantongBO
            continue;
        }
        m_pEdgeDetect->SetMininumThreshold(fEdgeThreshOld); //kircheis_NantongBO

        fEdgeRadius = nLineValue - fEdgeRadius;

        vfCirclePointXY[nSection].m_x = fInX + fEdgeRadius * pfCos[nSection];
        vfCirclePointXY[nSection].m_y = fInY + fEdgeRadius * pfSin[nSection];
        //m_result->m_rough_vecfptBallWidthEdgeTotalPoint.push_back(vfCirclePointXY[nSection]);
    }

    // 영훈 20131119 : 0의 값은 버리도록 하자.
    for (long n = (long)vfCirclePointXY.size() - 1; n >= 0; n--)
    {
        if (vfCirclePointXY[n].m_x <= 0.f || vfCirclePointXY[n].m_y <= 0.f)
        {
            vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
        }
    }

    long nValidEdgeCount = (long)vfCirclePointXY.size();
    if (nValidEdgeCount < EDGE_COUNT_MIN)
    {
        m_pEdgeDetect->SetMininumThreshold(fCurEdgeThresh);

        return FALSE;
    }

    Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

    if (fabs(CircleCoeff.m_x - fInX) < 10.f && fabs(CircleCoeff.m_y - fInY) < 10.f
        && fabs((CircleCoeff.m_radius) - i_fSpecWidthPXL) < 10)
    {
        fInX = CircleCoeff.m_x;
        fInY = CircleCoeff.m_y;
    }

    const long nSweepLineIgnoreDist = (long)(i_fSpecWidthPXL * 0.5f + 0.5f);

    vfCirclePointXY.clear();
    vfCirclePointXY.resize(nSectionNumber, Ipvm::Point32r2(0.f, 0.f));

    for (long nSection = 0; nSection < nSectionNumber; nSection++)
    {
        const float fOutX = fInX + fSweepLength * pfCos[nSection];
        const float fOutY = fInY + fSweepLength * pfSin[nSection];

        // 스윕 라인 상의 인센서티 샘플링
        long nLineValue
            = CippModules::GetLineData(image, fOutX, fOutY, fInX, fInY, pfCos[nSection], pfSin[nSection], NULL);
        vbyLineValue.clear();
        vbyLineValue.resize(nLineValue);
        CippModules::GetLineData(image, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);

        for (long nIgnoreSOP = 0; nIgnoreSOP < nSweepLineIgnoreDist; nIgnoreSOP++)
        {
            vbyLineValue[nIgnoreSOP] = 0; // // 20140616 영훈 : 여긴 어둡게 한다.
        }

        m_vecSweepLine.push_back(Ipvm::LineSeg32r(fInX, fInY, fOutX, fOutY)); //kircheis_LandShape

        // 스윕 라인 상의 상승 에지 구하기.
        if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_FALLING, nLineValue, &vbyLineValue[0], fEdgeRadius,
                fEdgeValue, bSubPixelingDone, bFirstEdge)) //FALSE ))//kircheis_ShinyTest
        {
            continue;
        }

        vfCirclePointXY[nSection].m_x = fInX + fEdgeRadius * pfCos[nSection];
        vfCirclePointXY[nSection].m_y = fInY + fEdgeRadius * pfSin[nSection];
    }

    std::vector<Ipvm::Point32r2> vecfptBallEdgePoint(
        0); //kircheis_20160524 //일종의 치트. 불량이 의심되면 상하좌우의 Max 길이를 구한다.
    // 영훈 20131119 : 0의 값은 버리도록 하자.
    for (long n = (long)vfCirclePointXY.size() - 1; n >= 0; n--)
    {
        vecfptBallEdgePoint.push_back(
            vfCirclePointXY[n]); //kircheis_20160524 //일종의 치트. 불량이 의심되면 상하좌우의 Max 길이를 구한다.

        if (vfCirclePointXY[n].m_x <= 0.f || vfCirclePointXY[n].m_y <= 0.f)
        {
            vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
        }
    }

    if ((long)vfCirclePointXY.size() <= 0)
    {
        m_pEdgeDetect->SetMininumThreshold(fCurEdgeThresh);

        return FALSE;
    }

    nValidEdgeCount = (long)vfCirclePointXY.size();
    Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

    // 구한 원의 원주에 대해서 일정 거리 이상 떨어진 놈들을 없애고 다시 피팅
    long nNewValidEdgeCount = 0;
    for (long nSection = 0; nSection < nValidEdgeCount; nSection++)
    {
        fDistX = vfCirclePointXY[nSection].m_x - CircleCoeff.m_x;
        fDistY = vfCirclePointXY[nSection].m_y - CircleCoeff.m_y;
        fEdgeRadius = CAST_FLOAT(sqrt(fDistX * fDistX + fDistY * fDistY));

        // 영훈 20140616 : 찾은 Ball의 반지름이 입력한 값보다 크게 튀는 Edge는 필터링하여 다시 피팅한다.
        fSpec_Temp = EDGE_FILTERING * CircleCoeff.m_radius;
        fRadiusSpec_Min = CircleCoeff.m_radius - fSpec_Temp;
        fRadiusSpec_Max = CircleCoeff.m_radius + fSpec_Temp;

        if (fEdgeRadius > fRadiusSpec_Min && fEdgeRadius < fRadiusSpec_Max)
        {
            vfCirclePointXY[nNewValidEdgeCount] = vfCirclePointXY[nSection];
            o_vecptFitEdge.push_back(vfCirclePointXY[nSection]);

            nNewValidEdgeCount++;
        }
    }

    if (nNewValidEdgeCount == 0 || nNewValidEdgeCount < EDGE_COUNT_MIN)
    {
        m_pEdgeDetect->SetMininumThreshold(fCurEdgeThresh);

        return FALSE;
    }

    // 새로 정한 무게중심 포인트를 이용해 원 피팅을 실시.
    Ipvm::DataFitting::FitToCircle(nNewValidEdgeCount, &vfCirclePointXY[0], o_AlignResult);

    m_pEdgeDetect->SetMininumThreshold(fCurEdgeThresh);

    Ipvm::Rect32s rtSpecROI((int32_t)(i_fptSpecLandCenter.m_x - i_fSpecWidthPXL),
        (int32_t)(i_fptSpecLandCenter.m_y - i_fSpecWidthPXL), (int32_t)(i_fptSpecLandCenter.m_x + i_fSpecWidthPXL),
        (int32_t)(i_fptSpecLandCenter.m_y + i_fSpecWidthPXL));
    Ipvm::Point32s2 ptAlignCenter((int32_t)o_AlignResult.m_x, (int32_t)o_AlignResult.m_y);
    if (!rtSpecROI.PtInRect(ptAlignCenter))
        return false;

    return TRUE;
}

void VisionInspectionLgaBottom2D::UpdateSweepAngleTable(
    long nSectionNum, std::vector<float>& vecfCos, std::vector<float>& vecfSin) //kircheis_LandShape
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

bool VisionInspectionLgaBottom2D::RoughAlignLand(const Ipvm::Image8u& image, Ipvm::Image8u& imageBin,
    std::vector<std::vector<FPI_RECT>> vec2sfrtSpecLand, std::vector<std::vector<Ipvm::Rect32s>>& o_vec2rtObj,
    std::vector<std::vector<Ipvm::Point32r2>>& o_vec2fptObjCenter,
    std::vector<std::vector<Ipvm::Point32r2>>& o_vec2fptObjMassCenter, Ipvm::Point32r2& o_fptBlobGravityCenter,
    Ipvm::Point32r2& o_fptBlobMassGravityCenter, std::vector<Ipvm::Image8u>& vecMask)
{
    o_vec2rtObj.clear();
    o_vec2fptObjCenter.clear();
    o_vec2fptObjMassCenter.clear();

    o_fptBlobGravityCenter = Ipvm::Point32r2(0.f, 0.f);
    o_fptBlobMassGravityCenter = Ipvm::Point32r2(0.f, 0.f);

    float fExtRatio = 0.2f;
    const long nMaxBlobNum = 100;

    long nPointNum = 0;
    o_vec2rtObj.resize(m_group_id_list.size());
    o_vec2fptObjCenter.resize(m_group_id_list.size());
    o_vec2fptObjMassCenter.resize(m_group_id_list.size());

    for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
    {
        long nLandNum = (long)m_vec2LandInfoPerLayer[nLayer].size();

        if (nLandNum > USHORT_MAX)
        {
            ASSERT(
                !_T("수집 Mask를 Unsigned short로 만들었으므로 그이상의 Land 가 존재한다면 Mask 타입을 변경해야 한다"));
            return false;
        }

        int nMaskWidth = vecMask[nLayer].GetSizeX();
        int nMaskHeight = vecMask[nLayer].GetSizeY();

        Ipvm::Image16u maskAcc(nMaskWidth, nMaskHeight);
        maskAcc.FillZero();

        float fAverageBlobArea = 0.f;
        float fAverageBlobWidth = 0.f;
        float fAverageBlobHeight = 0.f;

        nPointNum += nLandNum;
        o_vec2rtObj[nLayer].resize(nLandNum);
        o_vec2fptObjCenter[nLayer].resize(nLandNum);
        o_vec2fptObjMassCenter[nLayer].resize(nLandNum);
        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            Ipvm::Rect32s rtSpecObj = vec2sfrtSpecLand[nLayer][nLand].GetExtCRect();

            float fExtX = (float)rtSpecObj.Width() / 2.f * fExtRatio;
            float fExtY = (float)rtSpecObj.Height() / 2.f * fExtRatio;
            Ipvm::Rect32s rtSearchObj = rtSpecObj;
            rtSearchObj.InflateRect(long(fExtX + .5f), long(fExtY + .5f));

            BYTE lowMean = 0;
            BYTE highMean = 0;
            BYTE nAutoThresh = 0;

            if (Ipvm::ImageProcessing::GetThresholdOtsu(image, rtSearchObj, lowMean, highMean, nAutoThresh)
                != Ipvm::Status::e_ok)
            {
                return false;
            }

            if (!CippModules::Binarize(image, imageBin, rtSearchObj, nAutoThresh, FALSE))
                return false;

            std::vector<Ipvm::BlobInfo> vecObjBlob(nMaxBlobNum);
            int32_t nCurBlobNum = nMaxBlobNum;

            Ipvm::Image32s imageLabel;

            getReusableMemory().GetInspLongImage(imageLabel);

            if (m_pBlob->DoBlob(imageBin, rtSearchObj, nMaxBlobNum, imageLabel, &vecObjBlob[0], nCurBlobNum) != 0)
            {
                continue;
            }

            if (nCurBlobNum <= 0)
            {
                o_vec2rtObj[nLayer][nLand] = rtSpecObj;
                o_vec2fptObjCenter[nLayer][nLand]
                    = Ipvm::Point32r2((float)rtSpecObj.CenterPoint().m_x, (float)rtSpecObj.CenterPoint().m_y);
                o_vec2fptObjMassCenter[nLayer][nLand] = o_vec2fptObjCenter[nLayer][nLand];
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

            const Ipvm::BlobInfo& sMaxBlob = vecObjBlob[maxBlobIndex];
            const auto& rtObject = sMaxBlob.m_roi;
            const long label = sMaxBlob.m_label;

            long nObjWidth = rtObject.Width();
            long nObjHeight = rtObject.Height();
            fAverageBlobWidth += (float)nObjWidth;
            fAverageBlobHeight += (float)nObjHeight;
            fAverageBlobArea += (float)sMaxBlob.m_area;

            Ipvm::Point32r2 fptBlobMassCenter = Ipvm::Point32r2(0.f, 0.f);
            long nObjY = 0;
            for (long y = rtObject.m_top; y < rtObject.m_bottom; y++)
            {
                long nObjX = 0;
                auto* label_y = imageLabel.GetMem(0, y);
                if (nMaskHeight <= nObjY)
                    continue; //kircheis_LandShape
                auto* mask_count_y = maskAcc.GetMem(0, nObjY);

                for (long x = rtObject.m_left; x < rtObject.m_right; x++)
                {
                    if (label_y[x] == label)
                    {
                        fptBlobMassCenter.m_x += x;
                        fptBlobMassCenter.m_y += y;
                        mask_count_y[nObjX]++;
                    }

                    nObjX++;
                }
                nObjY++;
            }
            fptBlobMassCenter.m_x /= (float)maxBlobArea;
            fptBlobMassCenter.m_y /= (float)maxBlobArea;

            o_vec2rtObj[nLayer][nLand] = rtObject;
            o_vec2fptObjCenter[nLayer][nLand]
                = Ipvm::Point32r2((float)rtObject.CenterPoint().m_x, (float)rtObject.CenterPoint().m_y);
            o_vec2fptObjMassCenter[nLayer][nLand] = fptBlobMassCenter;

            o_fptBlobGravityCenter.m_x += o_vec2fptObjCenter[nLayer][nLand].m_x;
            o_fptBlobGravityCenter.m_y += o_vec2fptObjCenter[nLayer][nLand].m_y;

            o_fptBlobMassGravityCenter.m_x += o_vec2fptObjMassCenter[nLayer][nLand].m_x;
            o_fptBlobMassGravityCenter.m_y += o_vec2fptObjMassCenter[nLayer][nLand].m_y;
        }

        fAverageBlobWidth /= (float)nLandNum;
        fAverageBlobHeight /= (float)nLandNum;
        fAverageBlobArea /= (float)nLandNum;
        int nBinMaskWidth = long(fAverageBlobWidth + .5f);
        int nBinMaskHeight = long(fAverageBlobHeight + .5f);

        USHORT maxValue = 0;

        Ipvm::ImageProcessing::GetMax(maskAcc, Ipvm::Rect32s(maskAcc), maxValue);

        float fConvertRatio = 255.f / (float)maxValue;
        float fAverageMask = 0.f;
        float fValidAverageMask = 0.f;
        long nValidValue = 0;

        for (long y = 0; y < nMaskHeight; y++)
        {
            auto* mask_y = vecMask[nLayer].GetMem(0, y);
            auto* mask_acc_y = maskAcc.GetMem(0, y);

            for (long x = 0; x < nMaskWidth; x++)
            {
                BYTE byValue = min(255, (BYTE)(mask_acc_y[x] * fConvertRatio));

                mask_y[x] = byValue;
                if (byValue > 0)
                {
                    fValidAverageMask += (float)byValue;
                    nValidValue++;
                }
                fAverageMask += (float)byValue;
            }
        }

        fValidAverageMask /= (float)nValidValue;
        fAverageMask /= (float)(nMaskWidth * nMaskHeight);

        Ipvm::Rect32s rtMask = Ipvm::Rect32s(0, 0, nMaskWidth, nMaskHeight);
        long nMaskThresh = static_cast<long>(fValidAverageMask - .5f);

        Ipvm::Image8u maskTemp;
        if (!getReusableMemory().GetByteImage(maskTemp, nMaskWidth, nMaskHeight))
        {
            return false;
        }

        if (!CippModules::Binarize(vecMask[nLayer], maskTemp, rtMask, nMaskThresh, FALSE))
        {
            return false;
        }

        if (!getReusableMemory().GetByteImage(vecMask[nLayer], nBinMaskWidth, nBinMaskHeight))
        {
            return false;
        }

        Ipvm::Rect32s rtBinMask = Ipvm::Rect32s(0, 0, nBinMaskWidth, nBinMaskHeight);
        Ipvm::ImageProcessing::Copy(maskTemp, rtBinMask, vecMask[nLayer]);
    }

    o_fptBlobGravityCenter.m_x /= (float)nPointNum;
    o_fptBlobGravityCenter.m_y /= (float)nPointNum;

    o_fptBlobMassGravityCenter.m_x /= (float)nPointNum;
    o_fptBlobMassGravityCenter.m_y /= (float)nPointNum;

    return true;
}

BOOL VisionInspectionLgaBottom2D::GetSearchLine(Ipvm::Point32r2 fptLandCenter, const FPI_RECT& sfrtSearch, long nDir,
    Ipvm::LineSeg32r& o_sLineStart, Ipvm::LineSeg32r& o_sLineEnd)
{
    if (fptLandCenter.m_x < 0.f || fptLandCenter.m_y < 0.f)
        return FALSE;

    Ipvm::Point32r2 fptLT = sfrtSearch.fptLT;
    Ipvm::Point32r2 fptRT = sfrtSearch.fptRT;
    Ipvm::Point32r2 fptRB = sfrtSearch.fptRB;
    Ipvm::Point32r2 fptLB = sfrtSearch.fptLB;

    switch (nDir)
    {
        case UP:
            o_sLineStart = Ipvm::LineSeg32r(fptLB.m_x, fptLB.m_y, fptRB.m_x, fptRB.m_y);
            o_sLineEnd = Ipvm::LineSeg32r(fptLT.m_x, fptLT.m_y, fptRT.m_x, fptRT.m_y);
            break;
        case DOWN:
            o_sLineStart = Ipvm::LineSeg32r(fptLT.m_x, fptLT.m_y, fptRT.m_x, fptRT.m_y);
            o_sLineEnd = Ipvm::LineSeg32r(fptLB.m_x, fptLB.m_y, fptRB.m_x, fptRB.m_y);
            break;
        case LEFT:
            o_sLineStart = Ipvm::LineSeg32r(fptRT.m_x, fptRT.m_y, fptRB.m_x, fptRB.m_y);
            o_sLineEnd = Ipvm::LineSeg32r(fptLT.m_x, fptLT.m_y, fptLB.m_x, fptLB.m_y);
            break;
        case RIGHT:
            o_sLineStart = Ipvm::LineSeg32r(fptLT.m_x, fptLT.m_y, fptLB.m_x, fptLB.m_y);
            o_sLineEnd = Ipvm::LineSeg32r(fptRT.m_x, fptRT.m_y, fptRB.m_x, fptRB.m_y);
            break;
        default:
            return FALSE;
    }

    //if(nDir == UP || nDir == DOWN)
    //{
    //	sLine1.Set(fptLT.m_x, fptLT.m_y, fptRT.m_x, fptRT.m_y);
    //	sLine2.Set(fptLB.m_x, fptLB.m_y, fptRB.m_x, fptRB.m_y);
    //}
    //else
    //{
    //	sLine1.Set(fptLT.m_x, fptLT.m_y, fptLB.m_x, fptLB.m_y);
    //	sLine2.Set(fptRT.m_x, fptRT.m_y, fptRB.m_x, fptRB.m_y);
    //}

    //sLine1.Set(fptLT.m_x, fptLT.m_y, fptRT.m_x, fptRT.m_y);
    //sLine2.Set(fptLB.m_x, fptLB.m_y, fptRB.m_x, fptRB.m_y);
    //float fMinDist = FLT_MAX;
    //float fDX = fptLT.m_x - fptLandCenter.m_x;
    //float fDY = fptLT.m_y - fptLandCenter.m_y;
    //float fDist_lt = sqrt(fDX*fDX + fDY*fDY);
    //if(fDist_lt < fMinDist)
    //{
    //	fMinDist = fDist_lt;
    //	o_sLineStart = sLine1;
    //	o_sLineEnd = sLine2;
    //}

    //fDX = fptRT.m_x - fptLandCenter.m_x;
    //fDY = fptRT.m_y - fptLandCenter.m_y;
    //double lfDist_rt = sqrt(fDX*fDX + fDY*fDY);
    //if(lfDist_rt < fMinDist)
    //{
    //	fMinDist = lfDist_rt;
    //	o_sLineStart = sLine1;
    //	o_sLineEnd = sLine2;
    //}

    //fDX = fptRB.m_x - fptLandCenter.m_x;
    //fDY = fptRB.m_y - fptLandCenter.m_y;
    //double lfDist_rb = sqrt(fDX*fDX + fDY*fDY);
    //if(lfDist_rb < fMinDist)
    //{
    //	lfDist_rb = fDist_lt;
    //	o_sLineStart = sLine2;
    //	o_sLineEnd = sLine1;
    //}

    //fDX = fptLB.m_x - fptLandCenter.m_x;
    //fDY = fptLB.m_y - fptLandCenter.m_y;
    //double lfDist_lb = sqrt(fDX*fDX + fDY*fDY);
    //if(lfDist_lb < fMinDist)
    //{
    //	fMinDist = lfDist_lb;
    //	o_sLineStart = sLine2;
    //	o_sLineEnd = sLine1;
    //}

    return TRUE;
}

BOOL VisionInspectionLgaBottom2D::FindLineEdge(const Ipvm::Image8u& image, Ipvm::LineSeg32r sLineStart,
    Ipvm::LineSeg32r sLineEnd, BOOL bRisingEdge, BOOL bFirstEdge, std::vector<Ipvm::Point32r2>& o_vecfptEdge)
{
    o_vecfptEdge.clear();

    Ipvm::Point32r2 fptSearchStart = Ipvm::Point32r2(sLineStart.m_sx, sLineStart.m_sy);
    Ipvm::Point32r2 fptSearchRangeEnd = Ipvm::Point32r2(sLineStart.m_ex, sLineStart.m_ey);
    float fRangeDX = fptSearchRangeEnd.m_x - fptSearchStart.m_x;
    float fRangeDY = fptSearchRangeEnd.m_y - fptSearchStart.m_y;
    float fRangeRadian = CAST_FLOAT(atan2(fRangeDY, fRangeDX));
    float fRangeDist = CAST_FLOAT(sqrt(fRangeDX * fRangeDX + fRangeDY * fRangeDY));
    float fRangeCos = (float)cos(fRangeRadian);
    float fRangeSin = (float)sin(fRangeRadian);

    float fDX = sLineEnd.m_sx - fptSearchStart.m_x;
    float fDY = sLineEnd.m_sy - fptSearchStart.m_y;
    float fRadian1 = CAST_FLOAT(atan2(fDY, fDX));
    float fDist1 = CAST_FLOAT(sqrt(fDX * fDX + fDY * fDY));

    fDX = sLineEnd.m_ex - fptSearchStart.m_x;
    fDY = sLineEnd.m_ey - fptSearchStart.m_y;
    float fRadian2 = CAST_FLOAT(atan2(fDY, fDX));
    float fDist2 = CAST_FLOAT(sqrt(fDX * fDX + fDY * fDY));

    float fSearchLengthDist;
    float fSearchLengthRadian;
    Ipvm::Point32r2 fptSearchLengthEnd;
    if (fDist1 < fDist2)
    {
        fSearchLengthDist = fDist1;
        fSearchLengthRadian = fRadian1;
        fptSearchLengthEnd = Ipvm::Point32r2(sLineEnd.m_sx, sLineEnd.m_sy);
    }
    else
    {
        fSearchLengthDist = fDist2;
        fSearchLengthRadian = fRadian2;
        fptSearchLengthEnd = Ipvm::Point32r2(sLineEnd.m_ex, sLineEnd.m_ey);
    }

    long nEdgeDir = PI_ED_DIR_FALLING;
    if (bRisingEdge)
        nEdgeDir = PI_ED_DIR_RISING;

    Ipvm::Image8u tempImage;
    if (!getReusableMemory().GetInspByteImage(tempImage))
    {
        return FALSE;
    }

    BYTE* tempData = tempImage.GetMem();

    long nSearchLengthDist = (long)(fSearchLengthDist + .5f);
    float fSearchLengthCos = (float)cos(fSearchLengthRadian);
    float fSearchLengthSin = (float)sin(fSearchLengthRadian);

    long nRangeDist = static_cast<long>(fRangeDist + .5f);
    for (long nRange = 0; nRange < nRangeDist; nRange++)
    {
        Ipvm::Point32r2 fptSt;
        fptSt.m_x = (float)nRange * fRangeCos + fptSearchStart.m_x;
        fptSt.m_y = (float)nRange * fRangeSin + fptSearchStart.m_y;

        for (long n = 0; n < nSearchLengthDist; n++)
        {
            long x = (long)((float)n * fSearchLengthCos + fptSt.m_x + .5f);
            long y = (long)((float)n * fSearchLengthSin + fptSt.m_y + .5f);
            tempData[n] = image[y][x];
        }

        Ipvm::Point32r2 fptEdge;
        float fEdge, fEdgeVal;
        BOOL bSubPixelingDone;
        if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(
                nEdgeDir, nSearchLengthDist, tempData, fEdge, fEdgeVal, bSubPixelingDone, bFirstEdge))
            continue;
        fptEdge.m_x = fEdge * fSearchLengthCos + fptSt.m_x;
        fptEdge.m_y = fEdge * fSearchLengthSin + fptSt.m_y;
        o_vecfptEdge.push_back(fptEdge);
    }

    return TRUE;
}

BOOL VisionInspectionLgaBottom2D::RemoveNoisebyLine(std::vector<Ipvm::Point32r2> i_vecptEdge,
    std::vector<Ipvm::Point32r2>& o_vecptFitEdge, Ipvm::LineEq32r& o_line, float fTolerence)
{
    if (i_vecptEdge.size() < 2)
    {
        return FALSE;
    }
    else if (i_vecptEdge.size() == 2)
    {
        o_vecptFitEdge = i_vecptEdge;

        return (0
            == CPI_Geometry::mMakeLineByTwoPoints(o_line, (float)i_vecptEdge[0].m_x, (float)i_vecptEdge[0].m_y,
                (float)i_vecptEdge[1].m_x, (float)i_vecptEdge[1].m_y));
    }

    Ipvm::LineEq32r line;
    if (!CPI_Geometry::LineFitting_RemoveNoise(i_vecptEdge, o_vecptFitEdge, line))
    {
        return FALSE;
    }

    long nEdgeOrig = (long)i_vecptEdge.size();

    o_vecptFitEdge.clear();
    o_vecptFitEdge.reserve(nEdgeOrig);

    // 피팅라인에서 벗어난 넘 Filtering
    for (long i = 0; i < (long)i_vecptEdge.size(); i++)
    {
        float det = CAST_FLOAT(sqrt(line.m_a * line.m_a + line.m_b * line.m_b));
        if (det < FLT_MIN)
            return FALSE;

        float fDist = CAST_FLOAT(fabs(line.m_a * i_vecptEdge[i].m_x + line.m_b * i_vecptEdge[i].m_y + line.m_c)) / det;
        if (fabs(fDist) < fTolerence)
            o_vecptFitEdge.push_back(i_vecptEdge[i]);
    }

    if (o_vecptFitEdge.size() < 4)
        return FALSE;
    if (o_vecptFitEdge.size() < nEdgeOrig)
    {
        i_vecptEdge = o_vecptFitEdge;
        if (!CPI_Geometry::LineFitting_RemoveNoise(i_vecptEdge, o_vecptFitEdge, o_line))
        {
            return FALSE;
        }
    }
    else
    {
        o_line = line;
    }

    return TRUE;
}

BOOL VisionInspectionLgaBottom2D::ResetInspItem()
{
    VisionInspectionResult* pResult;
    VisionInspectionSpec* pSpec;

    long nCount(0);
    for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
    {
        nCount += (long)m_vec2LandInfoPerLayer[nLayer].size();
    }

    for (long nInsp = LAND_INSPECTION_START; nInsp < LAND_INSPECTION_END; nInsp++)
    {
        pResult = m_resultGroup.GetResultByName(g_szLandInspectionName[nInsp]);
        if (pResult == nullptr)
            continue;

        pResult->Clear();

        pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == nullptr || !pSpec->m_use)
            continue;

        pResult->Resize(nCount);
    }

    if (nCount <= 0)
        return FALSE;

    return TRUE;
}

BOOL VisionInspectionLgaBottom2D::MakeLandROI(const std::vector<std::vector<Package::Land>>& i_vec2LandMapData,
    const std::vector<std::vector<Ipvm::Point32r2>>& i_vec2fptLand, std::vector<std::vector<PI_RECT>>& o_vec2srtLand,
    std::vector<std::vector<FPI_RECT>>& o_vec2sfrtLand,
    std::vector<std::vector<std::vector<PI_RECT>>>& o_vec3srtSearchLand,
    std::vector<std::vector<std::vector<FPI_RECT>>>& o_vec3sfrtSearchLand)
{
    o_vec2srtLand.clear();
    o_vec2sfrtLand.clear();
    o_vec3srtSearchLand.clear();
    o_vec3sfrtSearchLand.clear();

    //{{mc_Land 수정

    o_vec2srtLand.resize(m_group_id_list.size());
    o_vec2sfrtLand.resize(m_group_id_list.size());
    o_vec3srtSearchLand.resize(m_group_id_list.size());
    o_vec3sfrtSearchLand.resize(m_group_id_list.size());

    for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
    {
        auto& para = m_VisionPara->m_vecLandParameter[nLayer];

        if (para.LandType == LandType_Rect)
        {
            MakeLandROI_Rect(i_vec2LandMapData[nLayer], i_vec2fptLand[nLayer], para, o_vec2srtLand[nLayer],
                o_vec2sfrtLand[nLayer], o_vec3srtSearchLand[nLayer], o_vec3sfrtSearchLand[nLayer]);
        }
        else if (para.LandType == LandType_Circle)
        {
            MakeLandROI_Circle(
                i_vec2LandMapData[nLayer], i_vec2fptLand[nLayer], para, o_vec2srtLand[nLayer], o_vec2sfrtLand[nLayer]);
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

void VisionInspectionLgaBottom2D::MakeLandROI_Rect(const std::vector<Package::Land>& i_vecLandMapData,
    const std::vector<Ipvm::Point32r2>& i_vecfptLand, const LandPara& i_landpara, std::vector<PI_RECT>& o_vecsrtLand,
    std::vector<FPI_RECT>& o_vecsfrtLand, std::vector<std::vector<PI_RECT>>& o_vec2srtSearchLand,
    std::vector<std::vector<FPI_RECT>>& o_vec2sfrtSearchLand)
{
    Ipvm::Point32r2 mm2Px = getScale().mmToPixel();
    float fPixelperMM = (mm2Px.m_x + mm2Px.m_y) / 2.f;

    float fSearchLength = i_landpara.m_fEdgeSearchLength * fPixelperMM;
    float fTopRangeSt = i_landpara.m_vec2fSearchRangeStart[Rect_Top] / 100.f;
    float fTopRangeEd = i_landpara.m_vec2fSearchRangeEnd[Rect_Top] / 100.f;
    float fBottomRangeSt = i_landpara.m_vec2fSearchRangeStart[Rect_Bottom] / 100.f;
    float fBottomRangeEd = i_landpara.m_vec2fSearchRangeEnd[Rect_Bottom] / 100.f;
    float fLeftRangeSt = i_landpara.m_vec2fSearchRangeStart[Rect_Left] / 100.f;
    float fLeftRangeEd = i_landpara.m_vec2fSearchRangeEnd[Rect_Left] / 100.f;
    float fRightRangeSt = i_landpara.m_vec2fSearchRangeStart[Rect_Right] / 100.f;
    float fRightRangeEd = i_landpara.m_vec2fSearchRangeEnd[Rect_Right] / 100.f;

    long nLandNum = (long)i_vecLandMapData.size();
    o_vecsrtLand.resize(nLandNum);
    o_vecsfrtLand.resize(nLandNum);
    o_vec2srtSearchLand.resize(nLandNum);
    o_vec2sfrtSearchLand.resize(nLandNum);

    for (long nLand = 0; nLand < nLandNum; nLand++)
    {
        o_vec2srtSearchLand[nLand].resize(4);
        o_vec2sfrtSearchLand[nLand].resize(4);

        float fLandHalfWidth = (i_vecLandMapData[nLand].fWidth) * fPixelperMM / 2.f;
        float fLandHalfLength = (i_vecLandMapData[nLand].fLength) * fPixelperMM / 2.f;
        float fLandAngle = (float)i_vecLandMapData[nLand].nAngle;

        FPI_RECT sfrtLand;
        Ipvm::Point32r2 fCenter = i_vecfptLand[nLand];
        Ipvm::Rect32r frtROI;
        frtROI.m_left = (fCenter.m_x - fLandHalfWidth);
        frtROI.m_top = (fCenter.m_y - fLandHalfLength);
        frtROI.m_right = (fCenter.m_x + fLandHalfWidth);
        frtROI.m_bottom = (fCenter.m_y + fLandHalfLength);

        //Top
        Ipvm::Rect32r frtSearchTop;
        frtSearchTop.m_top = frtROI.m_top - fSearchLength / 2.f - .5f;
        frtSearchTop.m_bottom = frtROI.m_top + fSearchLength / 2.f - .5f;
        frtSearchTop.m_left = frtROI.m_left + frtROI.Width() * fTopRangeSt;
        frtSearchTop.m_right = frtROI.m_left + frtROI.Width() * fTopRangeEd;

        //Bottom
        Ipvm::Rect32r frtSearchBottom;
        frtSearchBottom.m_top = frtROI.m_bottom - fSearchLength / 2.f - .5f;
        frtSearchBottom.m_bottom = frtROI.m_bottom + fSearchLength / 2.f - .5f;
        frtSearchBottom.m_left = frtROI.m_left + frtROI.Width() * fBottomRangeSt;
        frtSearchBottom.m_right = frtROI.m_left + frtROI.Width() * fBottomRangeEd;

        //Left
        Ipvm::Rect32r frtSearchLeft;
        frtSearchLeft.m_left = frtROI.m_left - fSearchLength / 2.f - .5f;
        frtSearchLeft.m_right = frtROI.m_left + fSearchLength / 2.f - .5f;
        frtSearchLeft.m_top = frtROI.m_top + frtROI.Height() * fLeftRangeSt;
        frtSearchLeft.m_bottom = frtROI.m_top + frtROI.Height() * fLeftRangeEd;

        //Right
        Ipvm::Rect32r frtSearchRight;
        frtSearchRight.m_left = frtROI.m_right - fSearchLength / 2.f - .5f;
        frtSearchRight.m_right = frtROI.m_right + fSearchLength / 2.f - .5f;
        frtSearchRight.m_top = frtROI.m_top + frtROI.Height() * fRightRangeSt;
        frtSearchRight.m_bottom = frtROI.m_top + frtROI.Height() * fRightRangeEd;

        sfrtLand = FPI_RECT(Ipvm::Point32r2(frtROI.m_left, frtROI.m_top), Ipvm::Point32r2(frtROI.m_right, frtROI.m_top),
            Ipvm::Point32r2(frtROI.m_left, frtROI.m_bottom), Ipvm::Point32r2(frtROI.m_right, frtROI.m_bottom));

        Ipvm::Point32r2 fptRotateCenter = sfrtLand.GetCenter();

        FPI_RECT sfrtSearchTop = FPI_RECT(Ipvm::Point32r2(frtSearchTop.m_left, frtSearchTop.m_top),
            Ipvm::Point32r2(frtSearchTop.m_right, frtSearchTop.m_top),
            Ipvm::Point32r2(frtSearchTop.m_left, frtSearchTop.m_bottom),
            Ipvm::Point32r2(frtSearchTop.m_right, frtSearchTop.m_bottom));
        FPI_RECT sfrtSearchBottom = FPI_RECT(Ipvm::Point32r2(frtSearchBottom.m_left, frtSearchBottom.m_top),
            Ipvm::Point32r2(frtSearchBottom.m_right, frtSearchBottom.m_top),
            Ipvm::Point32r2(frtSearchBottom.m_left, frtSearchBottom.m_bottom),
            Ipvm::Point32r2(frtSearchBottom.m_right, frtSearchBottom.m_bottom));
        FPI_RECT sfrtSearchLeft = FPI_RECT(Ipvm::Point32r2(frtSearchLeft.m_left, frtSearchLeft.m_top),
            Ipvm::Point32r2(frtSearchLeft.m_right, frtSearchLeft.m_top),
            Ipvm::Point32r2(frtSearchLeft.m_left, frtSearchLeft.m_bottom),
            Ipvm::Point32r2(frtSearchLeft.m_right, frtSearchLeft.m_bottom));
        FPI_RECT sfrtSearchRight = FPI_RECT(Ipvm::Point32r2(frtSearchRight.m_left, frtSearchRight.m_top),
            Ipvm::Point32r2(frtSearchRight.m_right, frtSearchRight.m_top),
            Ipvm::Point32r2(frtSearchRight.m_left, frtSearchRight.m_bottom),
            Ipvm::Point32r2(frtSearchRight.m_right, frtSearchRight.m_bottom));

        if (fLandAngle != 0)
        {
            float fAngle = fLandAngle * ITP_DEG_TO_RAD;

            // 시계 반대 방향이 +
            sfrtLand = sfrtLand.Rotate(fAngle);
            sfrtSearchTop = sfrtSearchTop.Rotate(fAngle, fptRotateCenter);
            sfrtSearchBottom = sfrtSearchBottom.Rotate(fAngle, fptRotateCenter);
            sfrtSearchLeft = sfrtSearchLeft.Rotate(fAngle, fptRotateCenter);
            sfrtSearchRight = sfrtSearchRight.Rotate(fAngle, fptRotateCenter);
        }

        o_vecsrtLand[nLand] = sfrtLand.GetSPI_RECT();
        o_vecsfrtLand[nLand] = sfrtLand;

        o_vec2sfrtSearchLand[nLand][UP] = sfrtSearchTop;
        o_vec2sfrtSearchLand[nLand][DOWN] = sfrtSearchBottom;
        o_vec2sfrtSearchLand[nLand][LEFT] = sfrtSearchLeft;
        o_vec2sfrtSearchLand[nLand][RIGHT] = sfrtSearchRight;

        o_vec2srtSearchLand[nLand][UP] = sfrtSearchTop.GetSPI_RECT();
        o_vec2srtSearchLand[nLand][DOWN] = sfrtSearchBottom.GetSPI_RECT();
        o_vec2srtSearchLand[nLand][LEFT] = sfrtSearchLeft.GetSPI_RECT();
        o_vec2srtSearchLand[nLand][RIGHT] = sfrtSearchRight.GetSPI_RECT();
    }
}

void VisionInspectionLgaBottom2D::MakeLandROI_Circle(const std::vector<Package::Land>& i_vecLandMapData,
    const std::vector<Ipvm::Point32r2>& i_vecfptLand, const LandPara& i_landpara, std::vector<PI_RECT>& o_vecsrtLand,
    std::vector<FPI_RECT>& o_vecsfrtLand)
{
    UNREFERENCED_PARAMETER(i_landpara);

    Ipvm::Point32r2 mm2Px = getScale().mmToPixel();
    float fPixelperMM = (mm2Px.m_x + mm2Px.m_y) / 2.f;

    long nLandNum = (long)i_vecLandMapData.size();
    o_vecsrtLand.resize(nLandNum);
    o_vecsfrtLand.resize(nLandNum);

    for (long nLand = 0; nLand < nLandNum; nLand++)
    {
        float fLandHalfWidth = (i_vecLandMapData[nLand].fWidth) * fPixelperMM / 2.f;
        float fLandHalfLength = (i_vecLandMapData[nLand].fLength) * fPixelperMM / 2.f;
        float fLandAngle = (float)i_vecLandMapData[nLand].nAngle;

        FPI_RECT sfrtLand;
        Ipvm::Point32r2 fCenter = i_vecfptLand[nLand];
        Ipvm::Rect32r frtROI;
        frtROI.m_left = (fCenter.m_x - fLandHalfWidth);
        frtROI.m_top = (fCenter.m_y - fLandHalfLength);
        frtROI.m_right = (fCenter.m_x + fLandHalfWidth);
        frtROI.m_bottom = (fCenter.m_y + fLandHalfLength);

        sfrtLand = FPI_RECT(Ipvm::Point32r2(frtROI.m_left, frtROI.m_top), Ipvm::Point32r2(frtROI.m_right, frtROI.m_top),
            Ipvm::Point32r2(frtROI.m_left, frtROI.m_bottom), Ipvm::Point32r2(frtROI.m_right, frtROI.m_bottom));

        Ipvm::Point32r2 fptRotateCenter = sfrtLand.GetCenter();

        if (fLandAngle != 0)
        {
            float fAngle = fLandAngle * ITP_DEG_TO_RAD;

            // 시계 반대 방향이 +
            sfrtLand = sfrtLand.Rotate(fAngle);
        }

        o_vecsrtLand[nLand] = sfrtLand.GetSPI_RECT();
        o_vecsfrtLand[nLand] = sfrtLand;
    }
}

void VisionInspectionLgaBottom2D::RotateLandROI(const std::vector<std::vector<Package::Land>>& i_vec2LandMapData,
    std::vector<std::vector<Ipvm::Point32r2>> vec2fptLand, std::vector<std::vector<PI_RECT>>& o_vec2srtLand,
    std::vector<std::vector<FPI_RECT>>& o_vec2sfrtLand)
{
    o_vec2srtLand.clear();
    o_vec2sfrtLand.clear();

    Ipvm::Point32r2 mm2Px = getScale().mmToPixel();
    float fPixelperMM = (mm2Px.m_x + mm2Px.m_y) / 2.f;

    long nLayerNum = (long)i_vec2LandMapData.size();
    o_vec2srtLand.resize(nLayerNum);
    o_vec2sfrtLand.resize(nLayerNum);
    for (long nLayer = 0; nLayer < nLayerNum; nLayer++)
    {
        long nLandNum = (long)i_vec2LandMapData[nLayer].size();
        o_vec2srtLand[nLayer].resize(nLandNum);
        o_vec2sfrtLand[nLayer].resize(nLandNum);
        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            float fLandHalfWidth = (i_vec2LandMapData[nLayer][nLand].fWidth) * fPixelperMM / 2.f;
            float fLandHalfLength = (i_vec2LandMapData[nLayer][nLand].fLength) * fPixelperMM / 2.f;
            float fLandAngle = float(i_vec2LandMapData[nLayer][nLand].nAngle);

            Ipvm::Point32r2 fCenter = vec2fptLand[nLayer][nLand];
            Ipvm::Rect32r frtROI;
            frtROI.m_left = (fCenter.m_x - fLandHalfWidth);
            frtROI.m_top = (fCenter.m_y - fLandHalfLength);
            frtROI.m_right = (fCenter.m_x + fLandHalfWidth);
            frtROI.m_bottom = (fCenter.m_y + fLandHalfLength);

            FPI_RECT sfrtLand
                = FPI_RECT(Ipvm::Point32r2(frtROI.m_left, frtROI.m_top), Ipvm::Point32r2(frtROI.m_right, frtROI.m_top),
                    Ipvm::Point32r2(frtROI.m_left, frtROI.m_bottom), Ipvm::Point32r2(frtROI.m_right, frtROI.m_bottom));
            if (fLandAngle != 0)
            {
                float fAngle = fLandAngle * ITP_DEG_TO_RAD;

                // 시계 반대 방향이 +
                sfrtLand = sfrtLand.Rotate(fAngle);
            }

            o_vec2srtLand[nLayer][nLand] = sfrtLand.GetSPI_RECT();
            o_vec2sfrtLand[nLayer][nLand] = sfrtLand;
        }
    }
}

BOOL VisionInspectionLgaBottom2D::GetBallCenterByLand()
{
    std::vector<Ipvm::Point32r2> specLandPos_um;
    std::vector<Ipvm::Point32r2> realLandPos_um;

    specLandPos_um.reserve(m_result->vec2sfrtLand.size());
    realLandPos_um.reserve(m_result->vec2sfrtLand.size());
    const auto bodyCenter = m_sEdgeAlignResult->m_center;
    const auto& px2um = getScale().pixelToUm();

    for (int idx = 0; idx < m_result->vec2sfrtLand.size(); idx++)
    {
        for (const auto& SpecLand : m_result->vec2fptSpecLand[idx])
        {
            specLandPos_um.emplace_back(
                (SpecLand.m_x - bodyCenter.m_x) * px2um.m_x, -(SpecLand.m_y - bodyCenter.m_y) * px2um.m_y);
        }

        for (const auto& RealLand : m_result->vec2fptLandCenter[idx])
        {
            //if (Land.m_ignored)
            //{
            //	continue;
            //}

            realLandPos_um.emplace_back(
                (RealLand.m_x - bodyCenter.m_x) * px2um.m_x, -(RealLand.m_y - bodyCenter.m_y) * px2um.m_y);
        }
    }

    if (specLandPos_um.size() == 0)
    {
        return TRUE;
    }

    if (Ipvm::Status::e_ok
        != Ipvm::Geometry::GetAffineTransform(
            &specLandPos_um[0], &realLandPos_um[0], (long)specLandPos_um.size(), m_affineTransformSpecToReal))
    {
        return TRUE;
    }

    std::vector<float> errorScores(specLandPos_um.size());

    for (long idx = (long)specLandPos_um.size() - 1; idx >= 0; idx--)
    {
        const float x = (float)(specLandPos_um[idx].m_x * m_affineTransformSpecToReal[0][0]
            + specLandPos_um[idx].m_y * m_affineTransformSpecToReal[0][1] + m_affineTransformSpecToReal[0][2]);
        const float y = (float)(specLandPos_um[idx].m_x * m_affineTransformSpecToReal[1][0]
            + specLandPos_um[idx].m_y * m_affineTransformSpecToReal[1][1] + m_affineTransformSpecToReal[1][2]);

        const float dx = realLandPos_um[idx].m_x - x;
        const float dy = realLandPos_um[idx].m_y - y;

        errorScores[idx] = dx * dx + dy * dy;
    }

    Ipvm::Geometry::GetOptimalAffineTransform(&specLandPos_um[0], &realLandPos_um[0], &errorScores[0], 0.5f,
        (long)specLandPos_um.size(), m_affineTransformSpecToReal);

#ifdef _DEBUG
    for (long idx = (long)specLandPos_um.size() - 1; idx >= 0; idx--)
    {
        const float x = (float)(specLandPos_um[idx].m_x * m_affineTransformSpecToReal[0][0]
            + specLandPos_um[idx].m_y * m_affineTransformSpecToReal[0][1] + m_affineTransformSpecToReal[0][2]);
        const float y = (float)(specLandPos_um[idx].m_x * m_affineTransformSpecToReal[1][0]
            + specLandPos_um[idx].m_y * m_affineTransformSpecToReal[1][1] + m_affineTransformSpecToReal[1][2]);

        const float dx = realLandPos_um[idx].m_x - x;
        const float dy = realLandPos_um[idx].m_y - y;

        errorScores[idx] = dx * dx + dy * dy;
    }
#endif

    Ipvm::TinyMatrix<3, 3> matSpecToReal;

    matSpecToReal[0][0] = m_affineTransformSpecToReal[0][0];
    matSpecToReal[0][1] = m_affineTransformSpecToReal[0][1];
    matSpecToReal[0][2] = m_affineTransformSpecToReal[0][2];
    matSpecToReal[1][0] = m_affineTransformSpecToReal[1][0];
    matSpecToReal[1][1] = m_affineTransformSpecToReal[1][1];
    matSpecToReal[1][2] = m_affineTransformSpecToReal[1][2];
    matSpecToReal[2][0] = 0.;
    matSpecToReal[2][1] = 0.;
    matSpecToReal[2][2] = 1.;

    Ipvm::TinyMatrix<3, 3> matRealToSpec = !matSpecToReal;

    m_affineTransformRealToSpec[0][0] = matRealToSpec[0][0];
    m_affineTransformRealToSpec[0][1] = matRealToSpec[0][1];
    m_affineTransformRealToSpec[0][2] = matRealToSpec[0][2];
    m_affineTransformRealToSpec[1][0] = matRealToSpec[1][0];
    m_affineTransformRealToSpec[1][1] = matRealToSpec[1][1];
    m_affineTransformRealToSpec[1][2] = matRealToSpec[1][2];

#ifdef _DEBUG
    for (long idx = (long)specLandPos_um.size() - 1; idx >= 0; idx--)
    {
        const float x = (float)(realLandPos_um[idx].m_x * m_affineTransformRealToSpec[0][0]
            + realLandPos_um[idx].m_y * m_affineTransformRealToSpec[0][1] + m_affineTransformRealToSpec[0][2]);
        const float y = (float)(realLandPos_um[idx].m_x * m_affineTransformRealToSpec[1][0]
            + realLandPos_um[idx].m_y * m_affineTransformRealToSpec[1][1] + m_affineTransformRealToSpec[1][2]);

        const float dx = specLandPos_um[idx].m_x - x;
        const float dy = specLandPos_um[idx].m_y - y;

        errorScores[idx] = dx * dx + dy * dy;
    }
#endif

    return TRUE;
}

//nGroupIndex : -1 일 경우 = All data Calc. , -1 이 아닐경우 = 해당 Group Data Calc.
BOOL VisionInspectionLgaBottom2D::DoLandWidth(const CString strLandWidthName, const long nGroupIndex)
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strLandWidthName);
    if (pResult == NULL)
        return FALSE;

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    if (pSpec->m_use == FALSE)
        return TRUE;

    float px2um = getScale().pixelToUmXY();

    CString strCompID = m_packageSpec.m_OriginLandData->m_strAllGroupName; //kircheis_MED2.5

    if (nGroupIndex > -1 && nGroupIndex < long(m_group_id_list.size()))
    {
        long nLandNum = (long)m_vec2LandInfoPerLayer[nGroupIndex].size();

        pResult->Resize(nLandNum);

        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            auto& landInfo = m_vec2LandInfoPerLayer[nGroupIndex][nLand];
            float fSpecWidth = m_vec2LandInfoPerLayer[nGroupIndex][nLand].fWidth * 1000.f;
            float fWidth = m_result->vec2fWidth[nGroupIndex][nLand] * px2um;
            float fError = fWidth - fSpecWidth;

            strCompID = landInfo.m_groupID; //kircheis_MED2.5

            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(nLand, landInfo.strLandID, strCompID, strCompID,
                fError, *pSpec, landInfo.fOffsetX, landInfo.fOffsetY); //kircheis_MED2.5
            pResult->SetRect(nLand, m_result->vec2sfrtLand[nGroupIndex][nLand].GetCRect());
        }
    }
    else
    {
        for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
        {
            long nLandNum = (long)m_vec2LandInfoPerLayer[nLayer].size();

            for (long nLand = 0; nLand < nLandNum; nLand++)
            {
                auto& landInfo = m_vec2LandInfoPerLayer[nLayer][nLand];
                float fSpecWidth = m_vec2LandInfoPerLayer[nLayer][nLand].fWidth * 1000.f;
                float fWidth = m_result->vec2fWidth[nLayer][nLand] * px2um;
                float fError = fWidth - fSpecWidth;

                pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(landInfo.nIndex, landInfo.strLandID,
                    landInfo.m_groupID, strCompID, fError, *pSpec, landInfo.fOffsetX,
                    landInfo.fOffsetY); //kircheis_MED2.5
                pResult->SetRect(landInfo.nIndex, m_result->vec2sfrtLand[nLayer][nLand].GetCRect());
            }
        }
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::DoLandWidth()
{
    DoLandWidth(g_szLandInspectionName[LAND_INSPECTION_LAND_WIDTH]);

    //Group Object 구간
    long GroupSpecCount = long(m_VisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != long(m_group_id_list.size()))
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoLandWidth(m_VisionPara->m_vecstrGroupInspName[idx].LandWidthName, idx))
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::DoLandLength(const CString strLandWidthName, const long nGroupIndex)
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strLandWidthName);
    if (pResult == NULL)
        return FALSE;

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    if (pSpec->m_use == FALSE)
        return TRUE;

    float px2um = getScale().pixelToUmXY();

    CString strCompID = m_packageSpec.m_OriginLandData->m_strAllGroupName; //kircheis_MED2.5

    if (nGroupIndex > -1 && nGroupIndex < long(m_group_id_list.size()))
    {
        long nLandNum = (long)m_vec2LandInfoPerLayer[nGroupIndex].size();

        pResult->Resize(nLandNum);

        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            auto& landInfo = m_vec2LandInfoPerLayer[nGroupIndex][nLand];
            float fSpecLength = landInfo.fLength * 1000.f;
            float fLength = m_result->vec2fLength[nGroupIndex][nLand] * px2um;
            float fError = fLength - fSpecLength;

            strCompID = landInfo.m_groupID; //kircheis_MED2.5

            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(nLand, landInfo.strLandID, strCompID, strCompID,
                fError, *pSpec, landInfo.fOffsetX, landInfo.fOffsetY); //kircheis_MED2.5
            pResult->SetRect(nLand, m_result->vec2sfrtLand[nGroupIndex][nLand].GetCRect());
        }
    }
    else
    {
        for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
        {
            long nLandNum = (long)m_vec2LandInfoPerLayer[nLayer].size();
            for (long nLand = 0; nLand < nLandNum; nLand++)
            {
                auto& landInfo = m_vec2LandInfoPerLayer[nLayer][nLand];
                float fSpecLength = landInfo.fLength * 1000.f;
                float fLength = m_result->vec2fLength[nLayer][nLand] * px2um;
                float fError = fLength - fSpecLength;

                pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(landInfo.nIndex, landInfo.strLandID,
                    landInfo.m_groupID, strCompID, fError, *pSpec, landInfo.fOffsetX,
                    landInfo.fOffsetY); //kircheis_MED2.5
                pResult->SetRect(landInfo.nIndex, m_result->vec2sfrtLand[nLayer][nLand].GetCRect());
            }
        }
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::DoLandLength()
{
    DoLandLength(g_szLandInspectionName[LAND_INSPECTION_LAND_LENGTH]);

    //Group Object 구간
    long GroupSpecCount = long(m_VisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != long(m_group_id_list.size()))
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoLandLength(m_VisionPara->m_vecstrGroupInspName[idx].LandLengthName, idx))
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::DoLandOffsetX(const CString strLandWidthName, const long nGroupIndex)
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strLandWidthName);
    if (pResult == NULL)
        return FALSE;

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    if (pSpec->m_use == FALSE)
        return TRUE;

    const auto bodyCenter = m_sEdgeAlignResult->m_center;
    const auto& px2um = getScale().pixelToUm();

    CString strCompID = m_packageSpec.m_OriginLandData->m_strAllGroupName; //kircheis_MED2.5

    if (nGroupIndex > -1 && nGroupIndex < long(m_group_id_list.size()))
    {
        long nLandNum = (long)m_vec2LandInfoPerLayer[nGroupIndex].size();

        pResult->Resize(nLandNum);

        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            auto& landInfo = m_vec2LandInfoPerLayer[nGroupIndex][nLand];
            Ipvm::Point32r2 fptLandCenter = m_result->vec2fptLandCenter[nGroupIndex][nLand];
            Ipvm::Point32r2 fptSpecLandCenter = m_result->vec2fptSpecLand[nGroupIndex][nLand];

            const float curX_um = (fptLandCenter.m_x - bodyCenter.m_x) * px2um.m_x;
            const float curY_um = -(fptLandCenter.m_y - bodyCenter.m_y) * px2um.m_y;

            const float transformedX_um = (float)(curX_um * m_affineTransformRealToSpec[0][0]
                + curY_um * m_affineTransformRealToSpec[0][1] + m_affineTransformRealToSpec[0][2]);

            float fError = transformedX_um - ((fptSpecLandCenter.m_x - bodyCenter.m_x) * px2um.m_x);

            strCompID = landInfo.m_groupID; //kircheis_MED2.5

            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(nLand, landInfo.strLandID, strCompID, strCompID,
                fError, *pSpec, landInfo.fOffsetX, landInfo.fOffsetY); //kircheis_MED2.5
            pResult->SetRect(nLand, m_result->vec2sfrtLand[nGroupIndex][nLand].GetCRect());
        }
    }
    else
    {
        for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
        {
            long nLandNum = (long)m_vec2LandInfoPerLayer[nLayer].size();
            for (long nLand = 0; nLand < nLandNum; nLand++)
            {
                auto& landInfo = m_vec2LandInfoPerLayer[nLayer][nLand];
                Ipvm::Point32r2 fptLandCenter = m_result->vec2fptLandCenter[nLayer][nLand];
                Ipvm::Point32r2 fptSpecLandCenter = m_result->vec2fptSpecLand[nLayer][nLand];

                const float curX_um = (fptLandCenter.m_x - bodyCenter.m_x) * px2um.m_x;
                const float curY_um = -(fptLandCenter.m_y - bodyCenter.m_y) * px2um.m_y;

                const float transformedX_um = (float)(curX_um * m_affineTransformRealToSpec[0][0]
                    + curY_um * m_affineTransformRealToSpec[0][1] + m_affineTransformRealToSpec[0][2]);

                float fError = transformedX_um - ((fptSpecLandCenter.m_x - bodyCenter.m_x) * px2um.m_x);

                pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(landInfo.nIndex, landInfo.strLandID,
                    landInfo.m_groupID, strCompID, fError, *pSpec, landInfo.fOffsetX,
                    landInfo.fOffsetY); //kircheis_MED2.5
                pResult->SetRect(landInfo.nIndex, m_result->vec2sfrtLand[nLayer][nLand].GetCRect());
            }
        }
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strCompID.Empty();
    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::DoLandAlign() //kircheis_LandMissing
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szLandInspectionName[LAND_INSPECTION_LAND_ALIGN]);
    if (pResult == NULL)
        return FALSE;

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    if (pSpec->m_use == FALSE)
        return TRUE;

    long nTotalLandNum = 0;
    long nLayerNum = (long)m_result->vec2bAlignResult.size();
    std::vector<long> vecnLandNum(nLayerNum);
    for (long nLayer = 0; nLayer < nLayerNum; nLayer++)
    {
        vecnLandNum[nLayer] = (long)m_result->vec2bAlignResult[nLayer].size();
        nTotalLandNum += vecnLandNum[nLayer];
    }

    pResult->Resize(nTotalLandNum);

    long nMissingNum(0), nIdx(0), nResult(0);
    CString strCompID;
    float fError = 0;
    for (long nLayer = 0; nLayer < nLayerNum; nLayer++)
    {
        for (long nLand = 0; nLand < vecnLandNum[nLayer]; nLand++)
        {
            auto& landInfo = m_vec2LandInfoPerLayer[nLayer][nLand];
            strCompID = landInfo.m_groupID;
            nResult = PASS;
            if (m_result->vec2bAlignResult[nLayer][nLand] == TRUE)
                fError = 0.f;
            else
            {
                nMissingNum++;
                fError = (float)nMissingNum;
                nResult = REJECT;
            }
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(nIdx, landInfo.strLandID, strCompID, strCompID,
                fError, *pSpec, landInfo.fOffsetX, landInfo.fOffsetY, 0.f, nResult); //kircheis_MED2.5
            pResult->SetRect(nIdx, m_result->vec2sfrtLand[nLayer][nLand].GetCRect());
            nIdx++;
        }
    }
    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strCompID.Empty();

    return TRUE;
}

BOOL VisionInspectionLgaBottom2D::DoLandOffsetX()
{
    DoLandOffsetX(g_szLandInspectionName[LAND_INSPECTION_LAND_OFFSET_X]);

    //Group Object 구간
    long GroupSpecCount = long(m_VisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != long(m_group_id_list.size()))
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoLandOffsetX(m_VisionPara->m_vecstrGroupInspName[idx].LandOffsetXName, idx))
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::DoLandOffsetY(const CString strLandWidthName, const long nGroupIndex)
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strLandWidthName);
    if (pResult == NULL)
        return FALSE;

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    if (pSpec->m_use == FALSE)
        return TRUE;

    const auto bodyCenter = m_sEdgeAlignResult->m_center;
    const auto& px2um = getScale().pixelToUm();

    CString strCompID = m_packageSpec.m_OriginLandData->m_strAllGroupName; //kircheis_MED2.5

    if (nGroupIndex > -1 && nGroupIndex < long(m_group_id_list.size()))
    {
        long nLandNum = (long)m_vec2LandInfoPerLayer[nGroupIndex].size();

        pResult->Resize(nLandNum);

        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            auto& landInfo = m_vec2LandInfoPerLayer[nGroupIndex][nLand];
            Ipvm::Point32r2 fptLandCenter = m_result->vec2fptLandCenter[nGroupIndex][nLand];
            Ipvm::Point32r2 fptSpecLandCenter = m_result->vec2fptSpecLand[nGroupIndex][nLand];

            const float curX_um = (fptLandCenter.m_x - bodyCenter.m_x) * px2um.m_x;
            const float curY_um = -(fptLandCenter.m_y - bodyCenter.m_y) * px2um.m_y;

            const float transformedY_um = (float)(curX_um * m_affineTransformRealToSpec[1][0]
                + curY_um * m_affineTransformRealToSpec[1][1] + m_affineTransformRealToSpec[1][2]);

            float fError = transformedY_um - (-(fptSpecLandCenter.m_y - bodyCenter.m_y) * px2um.m_y);

            strCompID = landInfo.m_groupID; //kircheis_MED2.5

            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(nLand, landInfo.strLandID, strCompID, strCompID,
                fError, *pSpec, landInfo.fOffsetX, landInfo.fOffsetY); //kircheis_MED2.5
            pResult->SetRect(nLand, m_result->vec2sfrtLand[nGroupIndex][nLand].GetCRect());
        }
    }
    else
    {
        for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
        {
            long nLandNum = (long)m_vec2LandInfoPerLayer[nLayer].size();
            for (long nLand = 0; nLand < nLandNum; nLand++)
            {
                auto& landInfo = m_vec2LandInfoPerLayer[nLayer][nLand];
                Ipvm::Point32r2 fptLandCenter = m_result->vec2fptLandCenter[nLayer][nLand];
                Ipvm::Point32r2 fptSpecLandCenter = m_result->vec2fptSpecLand[nLayer][nLand];

                const float curX_um = (fptLandCenter.m_x - bodyCenter.m_x) * px2um.m_x;
                const float curY_um = -(fptLandCenter.m_y - bodyCenter.m_y) * px2um.m_y;

                const float transformedY_um = (float)(curX_um * m_affineTransformRealToSpec[1][0]
                    + curY_um * m_affineTransformRealToSpec[1][1] + m_affineTransformRealToSpec[1][2]);

                float fError = transformedY_um - (-(fptSpecLandCenter.m_y - bodyCenter.m_y) * px2um.m_y);

                pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(landInfo.nIndex, landInfo.strLandID,
                    landInfo.m_groupID, strCompID, fError, *pSpec, landInfo.fOffsetX,
                    landInfo.fOffsetY); //kircheis_MED2.5
                pResult->SetRect(landInfo.nIndex, m_result->vec2sfrtLand[nLayer][nLand].GetCRect());
            }
        }
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::DoLandOffsetY()
{
    DoLandOffsetY(g_szLandInspectionName[LAND_INSPECTION_LAND_OFFSET_Y]);

    //Group Object 구간
    long GroupSpecCount = long(m_VisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != long(m_group_id_list.size()))
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoLandOffsetY(m_VisionPara->m_vecstrGroupInspName[idx].LandOffsetYName, idx))
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::DoLandOffsetR(const CString strLandWidthName, const long nGroupIndex)
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strLandWidthName);
    if (pResult == NULL)
        return FALSE;

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    if (pSpec->m_use == FALSE)
        return TRUE;

    const auto bodyCenter = m_sEdgeAlignResult->m_center;
    const auto& px2um = getScale().pixelToUm();

    CString strCompID = m_packageSpec.m_OriginLandData->m_strAllGroupName; //kircheis_MED2.5

    if (nGroupIndex > -1 && nGroupIndex < long(m_group_id_list.size()))
    {
        long nLandNum = (long)m_vec2LandInfoPerLayer[nGroupIndex].size();

        pResult->Resize(nLandNum);

        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            auto& landInfo = m_vec2LandInfoPerLayer[nGroupIndex][nLand];
            Ipvm::Point32r2 fptLandCenter = m_result->vec2fptLandCenter[nGroupIndex][nLand];
            Ipvm::Point32r2 fptSpecLandCenter = m_result->vec2fptSpecLand[nGroupIndex][nLand];

            const float curX_um = (fptLandCenter.m_x - bodyCenter.m_x) * px2um.m_x;
            const float curY_um = -(fptLandCenter.m_y - bodyCenter.m_y) * px2um.m_y;

            const float transformedX_um = (float)(curX_um * m_affineTransformRealToSpec[0][0]
                + curY_um * m_affineTransformRealToSpec[0][1] + m_affineTransformRealToSpec[0][2]);
            const float transformedY_um = (float)(curX_um * m_affineTransformRealToSpec[1][0]
                + curY_um * m_affineTransformRealToSpec[1][1] + m_affineTransformRealToSpec[1][2]);

            float fErrorX = transformedX_um - ((fptSpecLandCenter.m_x - bodyCenter.m_x) * px2um.m_x);
            float fErrorY = transformedY_um - (-(fptSpecLandCenter.m_y - bodyCenter.m_y) * px2um.m_y);
            float fError = CAST_FLOAT(sqrt(fErrorX * fErrorX + fErrorY * fErrorY));

            strCompID = landInfo.m_groupID; //kircheis_MED2.5

            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(nLand, landInfo.strLandID, strCompID, strCompID,
                fError, *pSpec, landInfo.fOffsetX, landInfo.fOffsetY); //kircheis_MED2.5
            pResult->SetRect(nLand, m_result->vec2sfrtLand[nGroupIndex][nLand].GetCRect());
        }
    }
    else
    {
        for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
        {
            long nLandNum = (long)m_vec2LandInfoPerLayer[nLayer].size();
            for (long nLand = 0; nLand < nLandNum; nLand++)
            {
                auto& landInfo = m_vec2LandInfoPerLayer[nLayer][nLand];
                Ipvm::Point32r2 fptLandCenter = m_result->vec2fptLandCenter[nLayer][nLand];
                Ipvm::Point32r2 fptSpecLandCenter = m_result->vec2fptSpecLand[nLayer][nLand];

                const float curX_um = (fptLandCenter.m_x - bodyCenter.m_x) * px2um.m_x;
                const float curY_um = -(fptLandCenter.m_y - bodyCenter.m_y) * px2um.m_y;

                const float transformedX_um = (float)(curX_um * m_affineTransformRealToSpec[0][0]
                    + curY_um * m_affineTransformRealToSpec[0][1] + m_affineTransformRealToSpec[0][2]);
                const float transformedY_um = (float)(curX_um * m_affineTransformRealToSpec[1][0]
                    + curY_um * m_affineTransformRealToSpec[1][1] + m_affineTransformRealToSpec[1][2]);

                float fErrorX = transformedX_um - ((fptSpecLandCenter.m_x - bodyCenter.m_x) * px2um.m_x);
                float fErrorY = transformedY_um - (-(fptSpecLandCenter.m_y - bodyCenter.m_y) * px2um.m_y);
                float fError = CAST_FLOAT(sqrt(fErrorX * fErrorX + fErrorY * fErrorY));

                pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(landInfo.nIndex, landInfo.strLandID,
                    landInfo.m_groupID, strCompID, fError, *pSpec, landInfo.fOffsetX,
                    landInfo.fOffsetY); //kircheis_MED2.5
                pResult->SetRect(landInfo.nIndex, m_result->vec2sfrtLand[nLayer][nLand].GetCRect());
            }
        }
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::DoLandOffsetR()
{
    DoLandOffsetR(g_szLandInspectionName[LAND_INSPECTION_LAND_OFFSET_R]);

    //Group Object 구간
    long GroupSpecCount = long(m_VisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != long(m_group_id_list.size()))
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoLandOffsetR(m_VisionPara->m_vecstrGroupInspName[idx].LandOffsetRName, idx))
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionLgaBottom2D::GetSpecLandCenterByBody(float fAngle_rad,
    std::vector<std::vector<Ipvm::Point32r2>>& o_vec2ptLandCenter,
    std::vector<std::vector<Ipvm::Point32r2>>& o_vec2ptRotateLandCenter, Ipvm::Point32r2& o_fptGravityCenter,
    Ipvm::Point32r2& o_fptRotateGravityCenter)
{
    o_vec2ptLandCenter.clear();
    o_vec2ptRotateLandCenter.clear();

    o_fptGravityCenter = Ipvm::Point32r2(0.f, 0.f);
    o_fptRotateGravityCenter = Ipvm::Point32r2(0.f, 0.f);

    const auto& mm2px = getScale().mmToPixel();

    //{{mc_Land 수정
    long nPointNum(0);
    o_vec2ptLandCenter.resize(m_group_id_list.size());
    o_vec2ptRotateLandCenter.resize(m_group_id_list.size());

    Ipvm::Point32r2 center(0.f, 0.f);
    if (m_pfptGetCenterPosbyPadAlign != NULL)
    {
        center = *m_pfptGetCenterPosbyPadAlign;
    }
    else if (m_sEdgeAlignResult != NULL)
    {
        center = m_result->sfrtPackageBody.GetCenter();
    }

    for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
    {
        o_vec2ptLandCenter[nLayer].clear();
        o_vec2ptRotateLandCenter[nLayer].clear();

        for (auto LandInfo : m_packageSpec.m_LandMapManager->vecLandData)
        {
            if (LandInfo.m_groupID == m_group_id_list[nLayer])
            {
                Ipvm::Point32r2 pos(LandInfo.fOffsetX * mm2px.m_x, (LandInfo.fOffsetY * mm2px.m_y) * -1.f);
                pos += center;

                o_fptGravityCenter += pos;
                o_vec2ptLandCenter[nLayer].push_back(pos);

                CPI_Geometry::RotatePoint(pos, fAngle_rad, center);
                o_fptRotateGravityCenter += pos;
                o_vec2ptRotateLandCenter[nLayer].push_back(pos);
            }
        }
    }
    nPointNum = (long)m_packageSpec.m_LandMapManager->vecLandData.size();

    //}}

    o_fptGravityCenter.m_x /= (float)nPointNum;
    o_fptGravityCenter.m_y /= (float)nPointNum;

    o_fptRotateGravityCenter.m_x /= (float)nPointNum;
    o_fptRotateGravityCenter.m_y /= (float)nPointNum;

    return TRUE;
}

BOOL VisionInspectionLgaBottom2D::FillArbitraryRect(const PI_RECT& srtFillRect, BYTE byVal, Ipvm::Image8u& io_image)
{
    Ipvm::Quadrangle32r quadRangle;
    quadRangle.m_ltX = float(srtFillRect.ltX);
    quadRangle.m_ltY = float(srtFillRect.ltY);
    quadRangle.m_rtX = float(srtFillRect.rtX);
    quadRangle.m_rtY = float(srtFillRect.rtY);
    quadRangle.m_lbX = float(srtFillRect.blX);
    quadRangle.m_lbY = float(srtFillRect.blY);
    quadRangle.m_rbX = float(srtFillRect.brX);
    quadRangle.m_rbY = float(srtFillRect.brY);

    if (Ipvm::ImageProcessing::Fill(quadRangle, byVal, io_image) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

void VisionInspectionLgaBottom2D::getGroupInfo(
    std::vector<Package::Land> i_vecLandMap, std::vector<CString>& o_group_id_list)
{
    o_group_id_list.clear();

    for (auto LandInfoGroupID : i_vecLandMap)
    {
        BOOL bCheckValue = std::find(o_group_id_list.begin(), o_group_id_list.end(), LandInfoGroupID.m_groupID)
            != o_group_id_list.end();

        if (!bCheckValue)
            o_group_id_list.push_back(LandInfoGroupID.m_groupID);
    }

    std::sort(o_group_id_list.begin(), o_group_id_list.end());
}
