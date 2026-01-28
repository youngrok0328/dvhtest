//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingFiducialAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingFiducialAlign.h"
#include "Result.h"
#include "VisionProcessingFiducialAlignPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/Config.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionProcessingFiducialAlign::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionProcessingFiducialAlign::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck time;
    m_errorLogText.Empty();

    ResetResult();
    checkJobParameter();

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    BOOL success = FALSE;
    if (align(detailSetupMode))
    {
        success = TRUE;
    }
    else
    {
        m_bInvalid = TRUE;
        success = FALSE;
    }

    if (!collect_debugResult(detailSetupMode))
    {
        success = FALSE;
    }

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    return success;
}

void VisionProcessingFiducialAlign::checkJobParameter()
{
    const auto& padInfos = m_packageSpec.m_PadMapManager;

    for (long padIndex = 0; padIndex < padInfos->GetCount(); padIndex++)
    {
        auto& padInfo = padInfos->vecPadData[padIndex];

        m_VisionPara->SetAlignPAD(padInfo.strPadName);
    }
}

bool VisionProcessingFiducialAlign::align(const bool detailSetupMode)
{
    //1. 선택되어있는 Data의 Spec값을 찾는다
    const auto& padInfos = m_packageSpec.m_PadMapManager;

    Ipvm::Point32r2 bodyCenter;
    float bodyAngle;

    if (!getBodyCenter(bodyCenter))
        return false;
    if (!getBodyAngle(bodyAngle))
        return false;

    m_result->m_bodyCenterBefore = bodyCenter;
    m_result->m_applyAlignTobodyCenter = bodyCenter;
    make_pad_imageCoordinates(bodyCenter, bodyAngle, *padInfos);

    //2. Spec값에 지정한 Offset만큼 Search ROI를 생성한다.
    long nSize = (long)m_VisionPara->m_use_pad_names.size();

    long searchOffsetX = max(0, long(getScale().convert_umToPixelX(m_VisionPara->m_SearchOffset_um)));
    long searchOffsetY = max(0, long(getScale().convert_umToPixelY(m_VisionPara->m_SearchOffset_um)));

    Ipvm::Point32r2 ptSearchOffset(CAST_FLOAT(searchOffsetX), CAST_FLOAT(searchOffsetY));
    std::vector<Ipvm::Point32r2> vecptGabList;
    Ipvm::Image8u binaryImage;

    if (!getReusableMemory().GetByteImage(
            binaryImage, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY()))
        return false;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(binaryImage), 0, binaryImage);

    for (long nidx = 0; nidx < nSize; nidx++)
    {
        long nPadIndex = m_VisionPara->FindAlignPAD_Index(m_VisionPara->m_use_pad_names[nidx]);

        if (nPadIndex == -1)
            return false;

        Ipvm::Rect32s SpecROI = m_result->m_debugResult_spec_pad_rois[nPadIndex].GetCRect();
        Ipvm::Rect32s SearchROI(0, 0, 0, 0);
        Ipvm::Image8u InspImage;

        if (!get_insp_image_and_searchROI(SpecROI, ptSearchOffset, InspImage, SearchROI))
        {
            return false;
        }

        //InspImage.SaveBmp(_T("D:\\InspImage.bmp"));

        m_result->m_debugResult_select_pad_rois.push_back(SpecROI);

        //3. Search ROI를 가지고 Object를 Blob 후 Gab을 연산한다 (이 이후 Edge를 따야 할 수도 있다.)
        Ipvm::Point32r2 ptGabSize(0, 0);
        Ipvm::Rect32s rtBlobROI(0, 0, 0, 0);
        if (!CalcGabOfObjectCenter(InspImage, SearchROI, ptGabSize, rtBlobROI, binaryImage))
        {
            return false;
        }

        //binaryImage.SaveBmp(_T("D:\\binaryImage.bmp"));

        m_result->m_debugResult_search_pad_rois.push_back(SearchROI);
        m_result->m_debugResult_final_pad_rois.push_back(rtBlobROI);

        vecptGabList.push_back(ptGabSize);
        //ptSearchOffset = ptGabSize;
    }

    SetDebugInfoItem(detailSetupMode, _T("Pad Blob Image"), binaryImage, true);

    //6. 두 Gab Data의 평균을 Edga Align의 Center 값에 연산한다.
    //{{//kircheis_POI
    float fMeanGabX = 0;
    float fMeanGabY = 0;
    long nCount = (long)vecptGabList.size();

    if (nCount <= 0)
        return false;

    for (long nidx = 0; nidx < nCount; nidx++)
    {
        fMeanGabX += vecptGabList[nidx].m_x;
        fMeanGabY += vecptGabList[nidx].m_y;
    }

    fMeanGabX /= (float)nCount;
    fMeanGabY /= (float)nCount;

    m_result->m_applyAlignTobodyCenter.m_x -= fMeanGabX;
    m_result->m_applyAlignTobodyCenter.m_y -= fMeanGabY;
    //}}

    FPI_RECT rtPackageRoi;

    if (!MakeBodyROI(rtPackageRoi))
        return false;

    rtPackageRoi.fptLT.m_x -= fMeanGabX;
    rtPackageRoi.fptLT.m_y -= fMeanGabY;
    rtPackageRoi.fptRT.m_x -= fMeanGabX;
    rtPackageRoi.fptRT.m_y -= fMeanGabY;
    rtPackageRoi.fptLB.m_x -= fMeanGabX;
    rtPackageRoi.fptLB.m_y -= fMeanGabY;
    rtPackageRoi.fptRB.m_x -= fMeanGabX;
    rtPackageRoi.fptRB.m_y -= fMeanGabY;

    m_result->m_applyAlignTobodyRoi = rtPackageRoi;

    return true;
}

bool VisionProcessingFiducialAlign::collect_debugResult(const bool detailSetupMode)
{
    SetDebugInfoItem(detailSetupMode, _T("Body Center"), m_result->m_bodyCenterBefore);
    SetDebugInfoItem(detailSetupMode, _T("Spec PAD ROIs"), m_result->m_debugResult_select_pad_rois);
    SetDebugInfoItem(detailSetupMode, _T("PAD Search ROIs"), m_result->m_debugResult_search_pad_rois);
    SetDebugInfoItem(detailSetupMode, _T("Final Align Result ROIs"), m_result->m_debugResult_final_pad_rois);
    SetDebugInfoItem(detailSetupMode, _T("Apply PAD Align to Body Center"), m_result->m_applyAlignTobodyCenter, true);
    SetDebugInfoItem(detailSetupMode, _T("Apply PAD Align to Body ROI"), m_result->m_applyAlignTobodyRoi, true);
    return true;
}

void VisionProcessingFiducialAlign::make_pad_imageCoordinates(
    const Ipvm::Point32r2& i_bodyCenter, const float i_bodyAngle, const Package::PadCollection& i_padInfo)
{
    m_result->m_pads.resize(i_padInfo.vecPadData.size());
    m_result->m_debugResult_align_Center.resize(i_padInfo.vecPadData.size());

    for (long padIndex = 0; padIndex < long(i_padInfo.vecPadData.size()); padIndex++)
    {
        auto& padInfo = i_padInfo.vecPadData[padIndex];

        make_pad_imageCoordinate(i_bodyCenter, i_bodyAngle, padInfo, m_result->m_pads[padIndex]);

        m_result->m_debugResult_spec_pad_rois.push_back(m_result->m_pads[padIndex].m_spec_px);
        m_result->m_debugResult_align_Center[padIndex] = m_result->m_pads[padIndex].m_spec_px.GetCenter();
    }
}

void VisionProcessingFiducialAlign::make_pad_imageCoordinate(
    const Ipvm::Point32r2& i_bodyCenter, const float i_bodyAngle, const Package::Pad& i_padSpec, Result_Pad& o_padPos)
{
    const auto& mmToPixel = getScale().mmToPixel();

    FPI_RECT specROI;
    float halfSizeX = i_padSpec.fWidth * 0.5f;
    float halfSizeY = i_padSpec.fLength * 0.5f;

    if (i_padSpec.GetType() == PAD_TYPE::_typeCircle)
    {
        // Circle은 Width만 존재한다
        halfSizeY = halfSizeX;
    }

    auto& padCenter = i_padSpec.GetCenter();

    specROI.fptLT.m_x = padCenter.m_x - halfSizeX;
    specROI.fptLT.m_y = padCenter.m_y - halfSizeY;
    specROI.fptRT.m_x = padCenter.m_x + halfSizeX;
    specROI.fptRT.m_y = padCenter.m_y - halfSizeY;
    specROI.fptLB.m_x = padCenter.m_x - halfSizeX;
    specROI.fptLB.m_y = padCenter.m_y + halfSizeY;
    specROI.fptRB.m_x = padCenter.m_x + halfSizeX;
    specROI.fptRB.m_y = padCenter.m_y + halfSizeY;

    specROI.Rotate(i_padSpec.nAngle * DEF_DEG_TO_RAD, Ipvm::Point32r2(0.f, 0.f));

    o_padPos.m_enable = i_padSpec.bIgnore ? false : true;
    o_padPos.m_name = i_padSpec.strPadName;
    o_padPos.m_type = (PAD_TYPE)i_padSpec.GetType();
    o_padPos.m_spec_px = get_gerberRoi_to_imageRoi(mmToPixel, i_bodyCenter, i_bodyAngle, specROI);
    o_padPos.m_align_px = o_padPos.m_spec_px;
}

FPI_RECT VisionProcessingFiducialAlign::get_gerberRoi_to_imageRoi(const Ipvm::Point32r2& pixelperMM,
    const Ipvm::Point32r2& body_center_px, const float body_angle, const FPI_RECT& gerberPos)
{
    FPI_RECT retValue;
    retValue.fptLT = get_gerberRoi_to_imageRoi(pixelperMM, body_center_px, body_angle, gerberPos.fptLT);
    retValue.fptRT = get_gerberRoi_to_imageRoi(pixelperMM, body_center_px, body_angle, gerberPos.fptRT);
    retValue.fptLB = get_gerberRoi_to_imageRoi(pixelperMM, body_center_px, body_angle, gerberPos.fptLB);
    retValue.fptRB = get_gerberRoi_to_imageRoi(pixelperMM, body_center_px, body_angle, gerberPos.fptRB);

    return retValue;
}

Ipvm::Point32r2 VisionProcessingFiducialAlign::get_gerberRoi_to_imageRoi(const Ipvm::Point32r2& pixelperMM,
    const Ipvm::Point32r2& body_center_px, const float body_angle, const Ipvm::Point32r2& gerberPos)
{
    float co = (float)cos(-body_angle);
    float si = (float)sin(-body_angle);

    Ipvm::Point32r2 retValue;
    retValue.m_x = (gerberPos.m_x * co - gerberPos.m_y * si) * pixelperMM.m_x + body_center_px.m_x;
    retValue.m_y = body_center_px.m_y - (gerberPos.m_x * si + gerberPos.m_y * co) * pixelperMM.m_y;

    return retValue;
}

bool VisionProcessingFiducialAlign::get_insp_image_and_searchROI(const Ipvm::Rect32s& i_targetPAD,
    Ipvm::Point32r2 i_SearchOffset, Ipvm::Image8u& o_inspImage, Ipvm::Rect32s& o_searchROI)
{
    //--------------------------------------------------------------------------------------------------------------------------------
    // PAD 검사에 사용될 Image와 Search ROI 를 리턴한다
    //--------------------------------------------------------------------------------------------------------------------------------
    if (!getReusableMemory().GetByteImage(
            o_inspImage, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY()))
        return false;

    long FrameIndex = 0;

    if (Config::getVisionType() == VISIONTYPE_2D_INSP)
    {
        FrameIndex = GetImageFrameIndex(m_VisionPara->m_select_use_image);
    }
    else
    {
        FrameIndex = m_VisionPara->m_select_use_image;
    }

    Ipvm::Image8u image1
        = GetInspectionFrameImage(FALSE, FrameIndex); // GetImageFrameIndex(m_VisionPara->m_select_use_image));
    //Ipvm::Image8u image2 = GetInspectionFrameImage(FALSE, m_VisionPara->m_imageProc.m_aux1FrameID.getFrameIndex());
    //Ipvm::Image8u image3 = GetInspectionFrameImage(FALSE, m_VisionPara->m_imageProc.m_aux2FrameID.getFrameIndex());

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_inspImage), 0, o_inspImage);

    o_searchROI = i_targetPAD;
    o_searchROI.InflateRect(CAST_INT32T(i_SearchOffset.m_x + .5f), CAST_INT32T(i_SearchOffset.m_y + .5f));
    o_searchROI &= Ipvm::Rect32s(o_inspImage);

    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image1, FALSE, o_searchROI, m_VisionPara->m_imageProc, o_inspImage))
    {
        return false;
    }

    return true;
}

bool VisionProcessingFiducialAlign::CalcGabOfObjectCenter(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& searchROI,
    Ipvm::Point32r2& o_ptGabSize, Ipvm::Rect32s& o_rtBlobROI, Ipvm::Image8u& o_binaryImage)
{
    //--------------------------------------------------------------------------------------------------------------------------------
    // 영역 내에서 Threshold 후 Blob하여 가장 큰 BlobInfo을 선택 후 해당 블럽의 Boundary을 얻는다
    //--------------------------------------------------------------------------------------------------------------------------------

    Ipvm::Image32s labelImage;

    if (!getReusableMemory().GetLongImage(
            labelImage, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY()))
        return false;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(labelImage), 0, labelImage);

    BYTE threshold = 200;

    /*Ipvm::Image32s histogram;
	if (Ipvm::ImageProcessing::MakeHistogram(i_image, searchROI, false, histogram) != Ipvm::Status::e_ok)
	{
		return false;
	}
	
	// Histogram 20% 위치를 Threshold로 지정해 봤다
	ULONGLONG intensitySum = 0;
	for (long index = 0; index < 256; index++)
	{
		intensitySum += histogram.GetMem()[index];
	}

	long checkSum = 0;
	for (long index = 255; index >= 0; index--)
	{
		checkSum += histogram.GetMem()[index];
		if (checkSum > intensitySum * 0.2f)
		{
			threshold = (BYTE)index;
			break;
		}
	}
	*/

    // Otsu를 Threshold로 지정해 봤다
    BYTE lowMean, highMean;
    if (Ipvm::ImageProcessing::GetThresholdOtsu(i_image, searchROI, lowMean, highMean, threshold) != Ipvm::Status::e_ok)
    {
        return false;
    }

    /*if (Ipvm::ImageProcessing::BinarizeGreater(i_image, searchROI, threshold, binaryImage) != Ipvm::Status::e_ok)
	{
		return false;
	}*/

    bool bReverseThreshold = FALSE;

    if (m_VisionPara->m_select_use_image == use_vmap)
        bReverseThreshold = TRUE;

    Ipvm::Rect32s BinarizeArea = Ipvm::Rect32s(searchROI);
    BinarizeArea.InflateRect(CAST_INT32T(-(searchROI.Width() * 0.1)), CAST_INT32T(-(searchROI.Height() * 0.1)));

    CippModules::Binarize(i_image, o_binaryImage, BinarizeArea, threshold, bReverseThreshold);

    int32_t blobCount = 0;
    if (m_pBlob->DoBlob(o_binaryImage, searchROI, 100, labelImage, m_psBlobInfo, blobCount) < 0)
        return false;
    if (blobCount <= 0)
        return false;

    // 가장 큰 Blob 찾기
    long blobIndex = 0;
    for (long index = 0; index < blobCount; index++)
    {
        if (m_psBlobInfo[index].m_area > m_psBlobInfo[blobIndex].m_area)
            blobIndex = index;
    }

    auto& blob = m_psBlobInfo[blobIndex];

    //{{//kircheis_POI
    long nSumX(0), nSumY(0);
    long nIdxY(0), nIdx(0);
    auto* pLabel = labelImage.GetMem();
    long nSizeX = labelImage.GetSizeX();
    for (long nY = blob.m_roi.m_top; nY < blob.m_roi.m_bottom; nY++)
    {
        nIdxY = nSizeX * nY;
        for (long nX = blob.m_roi.m_left; nX < blob.m_roi.m_right; nX++)
        {
            if (pLabel[nIdxY + nX] == blob.m_label)
            {
                nIdx++;
                nSumX += nX;
                nSumY += nY;
            }
        }
    }

    float fCenterX = (float)nSumX / (float)nIdx;
    float fCenterY = (float)nSumY / (float)nIdx;
    o_rtBlobROI = blob.m_roi;
    o_ptGabSize.m_x = (float)searchROI.CenterPoint().m_x - fCenterX; // blob.m_roi.CenterPoint().m_x;
    o_ptGabSize.m_y = (float)searchROI.CenterPoint().m_y - fCenterY; //blob.m_roi.CenterPoint().m_y;
    //}}

    return true;
}

bool VisionProcessingFiducialAlign::getBodyCenter(Ipvm::Point32r2& o_center)
{
    VisionAlignResult* pEdgeAlignResult = NULL;

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        pEdgeAlignResult = (VisionAlignResult*)pData;

    if (pEdgeAlignResult == NULL)
        return false;

    o_center = pEdgeAlignResult->m_center;

    return true;
}

bool VisionProcessingFiducialAlign::getBodyAngle(float& o_angle_rad)
{
    VisionAlignResult* pEdgeAlignResult = NULL;

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        pEdgeAlignResult = (VisionAlignResult*)pData;

    if (pEdgeAlignResult == NULL)
        return false;

    o_angle_rad = pEdgeAlignResult->m_angle_rad;

    return true;
}

bool VisionProcessingFiducialAlign::MakeBodyROI(FPI_RECT& o_rtPackageROI)
{
    VisionAlignResult* pEdgeAlignResult = NULL;

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        pEdgeAlignResult = (VisionAlignResult*)pData;

    if (pEdgeAlignResult == NULL)
        return false;

    o_rtPackageROI.fptLT = pEdgeAlignResult->fptLT;
    o_rtPackageROI.fptRT = pEdgeAlignResult->fptRT;
    o_rtPackageROI.fptLB = pEdgeAlignResult->fptLB;
    o_rtPackageROI.fptRB = pEdgeAlignResult->fptRB;

    return true;
}
