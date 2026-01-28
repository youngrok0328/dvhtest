//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "EngineBasedOnMatching.h"

//CPP_2_________________________________ This project's headers
#include "Constants.h"
#include "StitchResult.h"
#include "VisionProcessingAlign2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/ImageFile.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
EngineBasedOnMatching::EngineBasedOnMatching(VisionProcessingAlign2D* processor, const Constants& constants)
    : m_processor(processor)
    , m_constants(constants)
    , m_packageSpec(processor->m_packageSpec)
    , m_parentPara(processor->m_VisionPara)
    , m_para(processor->m_VisionPara.m_paraBasedOnMatching)
    , m_result(processor->m_result)
{
}

EngineBasedOnMatching::~EngineBasedOnMatching()
{
}

bool EngineBasedOnMatching::GetConstants()
{
    const auto& scale = m_processor->getScale();
    float um2Pixel = scale.umToPixelXY();
    m_searchOffsetPixel = (long)(m_para.m_templateSearchOffset_um * um2Pixel + 0.5f); // 50;

    long stitchCount = m_constants.m_stitchCountX * m_constants.m_stitchCountY;
    m_para.m_vecStitchROI.resize(stitchCount);

    return true;
}

bool EngineBasedOnMatching::MakeDefaultTemplateROI()
{
    //m_VisionPara.m_vecrtHorTemplateROI.clear();
    //m_VisionPara.m_vecrtVerTemplateROI.clear();

    float templateSize_um = 5000.f; // 5 mm 를 기본 template 크기로 한다.
    // 만일 Overlap 크기가 작으면 template 크기는 ( overlap 크기 - searchoffset ) 보다 작아야 한다.
    float minTemplateX = m_constants.m_foverlapUmX - m_para.m_templateSearchOffset_um;
    float minTemplateY = m_constants.m_foverlapUmY - m_para.m_templateSearchOffset_um;

    templateSize_um = min(templateSize_um, minTemplateX);
    templateSize_um = min(templateSize_um, minTemplateY);

    if (templateSize_um
        < 500) // template 크기가 500 um 도 안 된다면, search offset 이 너무 크거나 overlap 크기가 너무 작은 것이다. Invalid 상황
    {
        return false;
    }

    const auto& scale = m_processor->getScale();

    // Stitching후에 Device가 시작되는 예상 pixel 위치를 계산한다. (default matching Roi 계산시 사용된다)
    const long eStitchedSizeX
        = m_constants.m_rawImageSizeX - m_constants.m_overlapPixelX * (m_constants.m_stitchCountX - 1);
    const long eStitchedSizeY
        = m_constants.m_rawImageSizeY - m_constants.m_overlapPixelY * (m_constants.m_stitchCountY - 1);
    const float eStitchedSizeX_um = scale.convert_pixelToUmX((float)eStitchedSizeX);
    const float eStitchedSizeY_um = scale.convert_pixelToUmY((float)eStitchedSizeY);

    float fPackageSizeX = 0.f;
    float fPackageSizeY = 0.f;
    m_processor->GetPackageSize(false, fPackageSizeX, fPackageSizeY);

    const float eDeviceOffsetX_um = CAST_FLOAT((eStitchedSizeX_um - fPackageSizeX) * 0.5);
    const float eDeviceOffsetY_um = CAST_FLOAT((eStitchedSizeY_um - fPackageSizeY) * 0.5);

    for (long y = 0; y < m_constants.m_stitchCountY; y++)
    {
        for (long x = 0; x < m_constants.m_stitchCountX; x++)
        {
            long stitchIndex = y * m_constants.m_stitchCountX + x;
            auto& curStitchROI = m_para.m_vecStitchROI[stitchIndex];

            auto& rtVerRef_BCU = curStitchROI.m_rtVerRef_BCU;
            auto& rtHorRef_BCU = curStitchROI.m_rtHorRef_BCU;

            rtHorRef_BCU[0].SetRectEmpty();
            rtHorRef_BCU[1].SetRectEmpty();

            rtVerRef_BCU[0].SetRectEmpty();
            rtVerRef_BCU[1].SetRectEmpty();

            //{{ Top, Bottom ROI
            if (y != 0)
            {
                // verPreROIOverlap, imageOrigin 기준
                Ipvm::Rect32s verPreROIOverlap_px;
                verPreROIOverlap_px.m_left = x * m_constants.m_sensorSizeX;
                verPreROIOverlap_px.m_right = (x + 1) * m_constants.m_sensorSizeX;
                verPreROIOverlap_px.m_top = y * m_constants.m_sensorSizeY - m_constants.m_overlapPixelY;
                verPreROIOverlap_px.m_bottom = y * m_constants.m_sensorSizeY;

                auto verPreROIOverlap_BCU = scale.convert_PixelToBCU(verPreROIOverlap_px, m_constants.m_rawImageCenter);

                if (m_para.m_nTemplateROICount == 2)
                {
                    // rtVerRef[0]

                    rtVerRef_BCU[0] = verPreROIOverlap_BCU;
                    rtVerRef_BCU[0].m_right = rtVerRef_BCU[0].CenterPoint().m_x;
                    Ipvm::Point32r2 tempPoint = rtVerRef_BCU[0].CenterPoint();
                    rtVerRef_BCU[0].m_left = tempPoint.m_x - templateSize_um / 2;
                    rtVerRef_BCU[0].m_right = tempPoint.m_x + templateSize_um / 2;
                    rtVerRef_BCU[0].m_top = tempPoint.m_y - templateSize_um / 2;
                    rtVerRef_BCU[0].m_bottom = tempPoint.m_y + templateSize_um / 2;

                    // rtVerRef[1]
                    rtVerRef_BCU[1] = verPreROIOverlap_BCU;
                    rtVerRef_BCU[1].m_left = rtVerRef_BCU[1].CenterPoint().m_x;
                    tempPoint = rtVerRef_BCU[1].CenterPoint();
                    rtVerRef_BCU[1].m_left = tempPoint.m_x - templateSize_um / 2;
                    rtVerRef_BCU[1].m_right = tempPoint.m_x + templateSize_um / 2;
                    rtVerRef_BCU[1].m_top = tempPoint.m_y - templateSize_um / 2;
                    rtVerRef_BCU[1].m_bottom = tempPoint.m_y + templateSize_um / 2;

                    if (x == 0)
                    {
                        rtVerRef_BCU[0] = rtVerRef_BCU[0] + Ipvm::Point32r2(eDeviceOffsetX_um, 0);
                    }
                    else if (x == m_constants.m_stitchCountX - 1)
                    {
                        rtVerRef_BCU[1] = rtVerRef_BCU[1] - Ipvm::Point32r2(eDeviceOffsetX_um, 0);
                    }
                }
                else if (m_para.m_nTemplateROICount == 1)
                {
                    // verPreROIOverlap
                    auto tempPoint = verPreROIOverlap_BCU.CenterPoint();
                    verPreROIOverlap_BCU.m_left = tempPoint.m_x - templateSize_um / 2;
                    verPreROIOverlap_BCU.m_right = tempPoint.m_x + templateSize_um / 2;
                    verPreROIOverlap_BCU.m_top = tempPoint.m_y - templateSize_um / 2;
                    verPreROIOverlap_BCU.m_bottom = tempPoint.m_y + templateSize_um / 2;

                    rtVerRef_BCU[0] = verPreROIOverlap_BCU;
                }
            }
            //}} Top, Bottom ROI

            //{{ Left, Right ROI
            if (x != 0)
            {
                Ipvm::Rect32s horPreROIOverlap_px;

                // horPreROIOverlap, imageOrigin 기준
                horPreROIOverlap_px.m_left = x * m_constants.m_sensorSizeX - m_constants.m_overlapPixelX;
                horPreROIOverlap_px.m_right = x * m_constants.m_sensorSizeX;
                horPreROIOverlap_px.m_top = y * m_constants.m_sensorSizeY;
                horPreROIOverlap_px.m_bottom = (y + 1) * m_constants.m_sensorSizeY;

                auto horPreROIOverlap_BCU = scale.convert_PixelToBCU(horPreROIOverlap_px, m_constants.m_rawImageCenter);

                if (m_para.m_nTemplateROICount == 2)
                {
                    // rtHorRef[0]
                    rtHorRef_BCU[0] = horPreROIOverlap_BCU;
                    rtHorRef_BCU[0].m_bottom = rtHorRef_BCU[0].CenterPoint().m_y;
                    auto tempPoint = rtHorRef_BCU[0].CenterPoint();
                    rtHorRef_BCU[0].m_left = tempPoint.m_x - templateSize_um / 2;
                    rtHorRef_BCU[0].m_right = tempPoint.m_x + templateSize_um / 2;
                    rtHorRef_BCU[0].m_top = tempPoint.m_y - templateSize_um / 2;
                    rtHorRef_BCU[0].m_bottom = tempPoint.m_y + templateSize_um / 2;

                    // rtHorRef[1]
                    rtHorRef_BCU[1] = horPreROIOverlap_BCU;
                    rtHorRef_BCU[1].m_top = rtHorRef_BCU[1].CenterPoint().m_y;
                    tempPoint = rtHorRef_BCU[1].CenterPoint();
                    rtHorRef_BCU[1].m_left = tempPoint.m_x - templateSize_um / 2;
                    rtHorRef_BCU[1].m_right = tempPoint.m_x + templateSize_um / 2;
                    rtHorRef_BCU[1].m_top = tempPoint.m_y - templateSize_um / 2;
                    rtHorRef_BCU[1].m_bottom = tempPoint.m_y + templateSize_um / 2;

                    if (y == 0)
                    {
                        rtHorRef_BCU[0] = rtHorRef_BCU[0] + Ipvm::Point32r2(0, eDeviceOffsetY_um);
                    }
                    else if (y == m_constants.m_stitchCountY - 1)
                    {
                        rtHorRef_BCU[1] = rtHorRef_BCU[1] - Ipvm::Point32r2(0, eDeviceOffsetY_um);
                    }
                }
                else if (m_para.m_nTemplateROICount == 1)
                {
                    // horPreROIOverlap
                    auto tempPoint = horPreROIOverlap_BCU.CenterPoint();
                    horPreROIOverlap_BCU.m_left = tempPoint.m_x - templateSize_um / 2;
                    horPreROIOverlap_BCU.m_right = tempPoint.m_x + templateSize_um / 2;
                    horPreROIOverlap_BCU.m_top = tempPoint.m_y - templateSize_um / 2;
                    horPreROIOverlap_BCU.m_bottom = tempPoint.m_y + templateSize_um / 2;

                    //m_VisionPara.m_vecrtHorTemplateROI.push_back(horPreROIOverlap);
                    rtHorRef_BCU[0] = horPreROIOverlap_BCU;
                }
            }
            //}} Left, Right ROI
        }
    }

    SetROIsInRaw();

    return true;
}

bool EngineBasedOnMatching::SetROIsInRaw()
{
    bool returnValue = true;

    for (long y = 0; y < m_constants.m_stitchCountY; y++)
    {
        for (long x = 0; x < m_constants.m_stitchCountX; x++)
        {
            long stitchIndex = y * m_constants.m_stitchCountX + x;
            auto& para_stitchROI = m_para.m_vecStitchROI[stitchIndex];

            if (stitchIndex >= m_result.m_stitchRois.size())
            {
                returnValue = false;
                continue;
            }

            auto& verSearch = para_stitchROI.m_rtVerSearch;
            auto& horSearch = para_stitchROI.m_rtHorSearch;

            // 모두 초기화 해준다.
            verSearch[0].SetRectEmpty();
            verSearch[1].SetRectEmpty();
            horSearch[0].SetRectEmpty();
            horSearch[1].SetRectEmpty();

            auto& verRef_BCU = para_stitchROI.m_rtVerRef_BCU;
            auto& horRef_BCU = para_stitchROI.m_rtHorRef_BCU;

            if (y != 0)
            {
                for (int i = 0; i < 2; i++)
                {
                    verSearch[i]
                        = m_processor->getScale().convert_BCUToPixel(verRef_BCU[i], m_constants.m_rawImageCenter)
                        + Ipvm::Point32s2(0, m_constants.m_overlapPixelY);
                    verSearch[i].InflateRect(m_searchOffsetPixel, m_searchOffsetPixel);
                    if (verSearch[i] != (verSearch[i] & m_constants.GetVerCurOverlapROI(stitchIndex)))
                    {
                        // 온전히 포함되지 않으면, 실패
                        verSearch[i] = (verSearch[i] & m_constants.GetVerCurOverlapROI(stitchIndex));
                        returnValue = false;
                    }
                }
            }

            if (x != 0)
            {
                for (int i = 0; i < 2; i++)
                {
                    horSearch[i]
                        = m_processor->getScale().convert_BCUToPixel(horRef_BCU[i], m_constants.m_rawImageCenter)
                        + Ipvm::Point32s2(m_constants.m_overlapPixelX, 0);
                    horSearch[i].InflateRect(m_searchOffsetPixel, m_searchOffsetPixel);
                    if (horSearch[i] != (horSearch[i] & m_constants.GetHorCurOverlapROI(stitchIndex)))
                    {
                        // 온전히 포함되지 않으면, 실패
                        horSearch[i] = (horSearch[i] & m_constants.GetHorCurOverlapROI(stitchIndex));
                        returnValue = false;
                    }
                }
            }
        }
    }

    return returnValue;
}

bool EngineBasedOnMatching::FindAllStitchInfo_InRaw(const bool detailSetupMode)
{
    UNREFERENCED_PARAMETER(detailSetupMode);

    SetROIsInRaw();

    for (long y = 0; y < m_constants.m_stitchCountY; y++)
    {
        for (long x = 0; x < m_constants.m_stitchCountX; x++)
        {
            long stitchIndex = y * m_constants.m_stitchCountX + x;

            if (stitchIndex == 0) // x==y==0 이면 계산하지 않는다.
                continue;

            auto& para_curStitchROI = m_para.m_vecStitchROI[stitchIndex];
            auto& result_curStitchROI = m_result.m_stitchRois[stitchIndex];

            const auto& verPreROIOverlap = m_constants.GetVerPreOverlapROI(stitchIndex);
            const auto& verCurROIOverlap = m_constants.GetVerCurOverlapROI(stitchIndex);
            const auto& horPreROIOverlap = m_constants.GetHorPreOverlapROI(stitchIndex);
            const auto& horCurROIOverlap = m_constants.GetHorCurOverlapROI(stitchIndex);

            const auto& verSearch = para_curStitchROI.m_rtVerSearch;
            const auto& horSearch = para_curStitchROI.m_rtHorSearch;

            const auto& verRef_BCU = para_curStitchROI.m_rtVerRef_BCU;
            const auto& horRef_BCU = para_curStitchROI.m_rtHorRef_BCU;

            auto& horMatchingInfo = result_curStitchROI.m_horMatchingInfo;
            auto& verMatchingInfo = result_curStitchROI.m_verMatchingInfo;

            if (x != 0)
            {
                findStitchInfo_InRaw(horRef_BCU, horSearch, horPreROIOverlap, horCurROIOverlap, horMatchingInfo);
            }

            if (y != 0)
            {
                findStitchInfo_InRaw(verRef_BCU, verSearch, verPreROIOverlap, verCurROIOverlap, verMatchingInfo);
            }

            if (x == 0)
            {
                horMatchingInfo.m_angle_diff = verMatchingInfo.m_angle_diff;
                horMatchingInfo.m_shiftX = verMatchingInfo.m_shiftX;
                horMatchingInfo.m_shiftY = verMatchingInfo.m_shiftY;
                horMatchingInfo.m_rotate_origin_x = verMatchingInfo.m_rotate_origin_x;
                horMatchingInfo.m_rotate_origin_y = verMatchingInfo.m_rotate_origin_y;
            }

            if (y == 0)
            {
                verMatchingInfo.m_angle_diff = horMatchingInfo.m_angle_diff;
                verMatchingInfo.m_shiftX = horMatchingInfo.m_shiftX;
                verMatchingInfo.m_shiftY = horMatchingInfo.m_shiftY;
                verMatchingInfo.m_rotate_origin_x = horMatchingInfo.m_rotate_origin_x;
                verMatchingInfo.m_rotate_origin_y = horMatchingInfo.m_rotate_origin_y;
            }

            // Update Accumulate Matching Info
            auto& preHorStitchInfo = m_result.m_stitchRois[stitchIndex - 1];

            if (x == 0)
            {
                auto& preVerStitchInfo = m_result.m_stitchRois[stitchIndex - m_constants.m_stitchCountX];
                result_curStitchROI.UpdateMatchingInfo();
                result_curStitchROI.UpdateAccumulateMatchingInfoForMatching(preVerStitchInfo, preVerStitchInfo);
            }
            else if (y == 0)
            {
                result_curStitchROI.UpdateMatchingInfo();
                result_curStitchROI.UpdateAccumulateMatchingInfoForMatching(preHorStitchInfo, preHorStitchInfo);
            }
            else
            {
                auto& preVerStitchInfo = m_result.m_stitchRois[stitchIndex - m_constants.m_stitchCountX];
                result_curStitchROI.UpdateMatchingInfo();
                result_curStitchROI.UpdateAccumulateMatchingInfoForMatching(preVerStitchInfo, preHorStitchInfo);
            }
        }
    }

    return true;
}

// Stitching을 하지 않으면서, Raw 영상만으로 Matching 정보만 계산하는데 사용된다.
bool EngineBasedOnMatching::findStitchInfo_InRaw(const Ipvm::Rect32r* ppreRefROI_BCU,
    const Ipvm::Rect32s* pcurSearchROI, const Ipvm::Rect32s& preROIOverlap, const Ipvm::Rect32s& curROIOverlap,
    StitchMatchingInfo& o_MatchingInfo)
{
    const auto& scale = m_processor->getScale();

    auto& imageRaw = m_processor->getImageLot().GetImageFrame(
        m_constants.m_frameIndexForStitchInfo, m_processor->GetCurVisionModule_Status());

    // Stitch Section Image Combine
    // Combine Image를 사용할 경우를 위한 Buffer 생성(안할 경우에도 기본 이미지를 복사해서 쓰기때문에 생성이 필요함)
    // Stitching을 하게 되면 Raw Image의 전체 크기를 가져와야 하기 때문에 Buffer Image에 크기를 할당 해줌(안해주면 Image Combine 실행 시, 프로그램 죽음)
    Ipvm::Image8u combineImage;
    if (!m_processor->getReusableMemory().GetByteImage(combineImage, imageRaw.GetSizeX(), imageRaw.GetSizeY()))
    {
        return false;
    }
    //  Image Combine 적용 - Stitch Section Image Combine
    if (!CippModules::GrayImageProcessingManage(m_processor->getReusableMemory(), &imageRaw, true,
            m_processor->m_VisionPara.m_stitchImageProcManagePara, combineImage))
    {
        return false;
    }

    Ipvm::Point32r2 refPoint[2];
    Ipvm::Point32r2 curMatchPoint[2];

    for (long matchIndex = 0; matchIndex < 2; matchIndex++)
    {
        Ipvm::Rect32s rtRef
            = m_processor->getScale().convert_BCUToPixel(ppreRefROI_BCU[matchIndex], m_constants.m_rawImageCenter);
        // Combine 적용된 Image로 Stitching
        bool matchSuccess
            = ImageMatch(combineImage, combineImage, rtRef, pcurSearchROI[matchIndex], refPoint[matchIndex],
                curMatchPoint[matchIndex], o_MatchingInfo.m_fMatchScore[matchIndex]); // Stitch Section Image Combine

        if (!matchSuccess)
        {
            return FALSE;
        }

        // Matching으로 찾은 위치를 보기 위해 저장한다.
        float fHalfRefWidth = (float)(rtRef.Width() / 2.f);
        float fHalfRefHeight = (float)(rtRef.Height() / 2.f);
        Ipvm::Point32r2 fShift
            = (refPoint[matchIndex] - preROIOverlap.TopLeft()) - (curMatchPoint[matchIndex] - curROIOverlap.TopLeft());
        o_MatchingInfo.m_ptShift[matchIndex] = scale.convert_pixelToUm(fShift);

        o_MatchingInfo.m_rtMatched[matchIndex]
            = Ipvm::Rect32r(-fHalfRefWidth, -fHalfRefHeight, fHalfRefWidth, fHalfRefHeight) + curMatchPoint[matchIndex];
    }

    //{{ Angle 및 Shift 계산
    double RadToDeg = 180 / ITP_PI;

    double refCenter_x = (refPoint[0].m_x + refPoint[1].m_x) * 0.5;
    double refCenter_y = (refPoint[0].m_y + refPoint[1].m_y) * 0.5;
    double refAngle = atan2(refPoint[1].m_y - refPoint[0].m_y, refPoint[1].m_x - refPoint[0].m_x) * RadToDeg;
    o_MatchingInfo.m_curCenX = (curMatchPoint[0].m_x + curMatchPoint[1].m_x) * 0.5;
    o_MatchingInfo.m_curCenY = (curMatchPoint[0].m_y + curMatchPoint[1].m_y) * 0.5;
    o_MatchingInfo.m_rotate_origin_x = o_MatchingInfo.m_curCenX - curROIOverlap.m_left;
    o_MatchingInfo.m_rotate_origin_y = o_MatchingInfo.m_curCenY - curROIOverlap.m_top;
    double curAngle
        = atan2(curMatchPoint[1].m_y - curMatchPoint[0].m_y, curMatchPoint[1].m_x - curMatchPoint[0].m_x) * RadToDeg;

    float angle_diff1 = CAST_FLOAT(refAngle - curAngle);
    float angle_diff2 = 360.f - angle_diff1;

    // Stitch가 위아래로 겹치는 경우 위치에 있는 경우, 위쪽과 왼쪽의 Angle을 함께 고려하여 계산하는데
    // 이 때 위쪽이 Angle 0, 왼쪽의 Angle이 360도에 가깝게 나오면 중간값을 취해 180도가 나옴으로서
    // 의도하지 않게 이미지가 180도 회전에 버립니다. 이에 Angle을 비슷한 값으로 일단 치완할 수 있게 수정하였습니다.
    // Example 359도 => -1도

    o_MatchingInfo.m_angle_diff = fabs(angle_diff1) > fabs(angle_diff2) ? angle_diff2 : angle_diff1;
    o_MatchingInfo.m_shiftX = (refCenter_x - preROIOverlap.m_left) - (o_MatchingInfo.m_curCenX - curROIOverlap.m_left);
    o_MatchingInfo.m_shiftY = (refCenter_y - preROIOverlap.m_top) - (o_MatchingInfo.m_curCenY - curROIOverlap.m_top);
    //}} Angle 및 Shift 계산

    return true;
}

bool EngineBasedOnMatching::ImageMatch(const Ipvm::Image8u& imageOrigin, const Ipvm::Image8u& imageStitch,
    Ipvm::Rect32s sourceROI, Ipvm::Rect32s targetSearchROI, Ipvm::Point32r2& sourcePoint, Ipvm::Point32r2& targetPoint,
    float& fMatchRate)
{
    sourcePoint.m_x = (float)sourceROI.m_left;
    sourcePoint.m_y = (float)sourceROI.m_top;

    if (!ImageMatchSampling(imageOrigin, imageStitch, sourceROI, targetSearchROI, targetPoint, fMatchRate))
    {
        return false;
    }

    // Save Template Debug Image
    BOOL bSaveTemplateDebugImage = m_parentPara.m_saveTemplateDebugImage;

    if (bSaveTemplateDebugImage)
    {
        long value1, value2;

        long offsetx = long(targetPoint.m_x + 0.5);
        long offsety = long(targetPoint.m_y + 0.5);

        Ipvm::Image8u saveImage;
        if (!m_processor->getReusableMemory().GetByteImage(saveImage, sourceROI.Width(), sourceROI.Height()))
            return FALSE;

        const long nStitchWidthBytes = imageStitch.GetWidthBytes();
        for (long y = 0; y < sourceROI.Height(); y++)
        {
            auto* image_y = saveImage.GetMem(0, y);
            for (long x = 0; x < sourceROI.Width(); x++)
            {
                long idx = (x + sourceROI.m_left) + (y + sourceROI.m_top) * nStitchWidthBytes;
                value1 = imageStitch.GetMem()[idx];
                image_y[x] = (BYTE)value1;
            }
        }

        CString strMatch1;
        strMatch1.Format(_T("D:\\2D_Stitching\\Match\\%d_match1.bmp"), m_result.m_nSaveTemplateImageIndex);

        Ipvm::ImageFile::SaveAsBmp(saveImage, strMatch1);

        strMatch1.Empty();

        const long nOriginWidthBytes = imageOrigin.GetWidthBytes();

        for (long y = 0; y < sourceROI.Height(); y++)
        {
            auto* image_y = saveImage.GetMem(0, y);

            for (long x = 0; x < sourceROI.Width(); x++)
            {
                long idx = (x + offsetx) + (y + offsety) * nOriginWidthBytes;
                value2 = imageOrigin.GetMem()[idx];
                image_y[x] = (BYTE)value2;
            }
        }

        CString strMatch2;
        strMatch2.Format(_T("D:\\2D_Stitching\\Match\\%d_match2.bmp"), m_result.m_nSaveTemplateImageIndex);

        Ipvm::ImageFile::SaveAsBmp(saveImage, strMatch2);

        strMatch2.Empty();

        for (long y = 0; y < sourceROI.Height(); y++)
        {
            auto* image_y = saveImage.GetMem(0, y);

            for (long x = 0; x < sourceROI.Width(); x++)
            {
                long idx1 = (x + sourceROI.m_left) + (y + sourceROI.m_top) * nStitchWidthBytes;
                long idx2 = (x + offsetx) + (y + offsety) * nOriginWidthBytes;
                value1 = imageStitch.GetMem()[idx1];
                value2 = imageOrigin.GetMem()[idx2];

                image_y[x] = (BYTE)max(value1 - value2, 0);
            }
        }

        CString strDiff1;
        strDiff1.Format(_T("D:\\2D_Stitching\\Match\\%d_diff1.bmp"), m_result.m_nSaveTemplateImageIndex);

        Ipvm::ImageFile::SaveAsBmp(saveImage, strDiff1);

        strDiff1.Empty();

        for (long y = 0; y < sourceROI.Height(); y++)
        {
            auto* image_y = saveImage.GetMem(0, y);

            for (long x = 0; x < sourceROI.Width(); x++)
            {
                long idx1 = (x + sourceROI.m_left) + (y + sourceROI.m_top) * nStitchWidthBytes;
                long idx2 = (x + offsetx) + (y + offsety) * nOriginWidthBytes;
                value1 = imageStitch.GetMem()[idx1];
                value2 = imageOrigin.GetMem()[idx2];

                image_y[x] = (BYTE)max(value2 - value1, 0);
            }
        }

        CString strDiff2;
        strDiff2.Format(_T("D:\\2D_Stitching\\Match\\%d_diff2.bmp"), m_result.m_nSaveTemplateImageIndex);

        Ipvm::ImageFile::SaveAsBmp(saveImage, strDiff2);

        strDiff2.Empty();

        m_result.m_nSaveTemplateImageIndex++;
    }
    // Save Template Debug Image

    // Rect 의 left top 을 point 로 넘기지 않고 Rect 의 center point 를 넘김
    sourcePoint.m_x += sourceROI.Width() * 0.5f;
    sourcePoint.m_y += sourceROI.Height() * 0.5f;
    targetPoint.m_x += sourceROI.Width() * 0.5f;
    targetPoint.m_y += sourceROI.Height() * 0.5f;

    return true;
}

bool EngineBasedOnMatching::ImageMatchSampling(const Ipvm::Image8u& imageOrigin, const Ipvm::Image8u& imageStitch,
    Ipvm::Rect32s sourceROI, Ipvm::Rect32s targetSearchROI, Ipvm::Point32r2& targetPoint, float& fMatchRate)
{
    // sourceRect는 imageStitch 에 있고,
    // targetRect를 imageOrigin 에서 찾아야 한다.
    Ipvm::Rect32s sourceRect = Ipvm::Rect32s(sourceROI.m_left, sourceROI.m_top, sourceROI.m_right, sourceROI.m_bottom);
    Ipvm::Rect32s targetRect = Ipvm::Rect32s(
        targetSearchROI.m_left, targetSearchROI.m_top, targetSearchROI.m_right, targetSearchROI.m_bottom);

    BOOL bSaveImage = m_parentPara.m_saveTemplateDebugImage;

    if (bSaveImage)
    {
        CString strTemp;
        strTemp.Format(_T("D:\\2D_Stitching\\Match\\%d_template.bmp"), m_result.m_nSaveTemplateImageIndex);

        Ipvm::ImageFile::SaveAsBmp(Ipvm::Image8u(imageStitch, sourceRect), strTemp);

        strTemp.Format(_T("D:\\2D_Stitching\\Match\\\\%d_target.bmp"), m_result.m_nSaveTemplateImageIndex);

        Ipvm::ImageFile::SaveAsBmp(Ipvm::Image8u(imageOrigin, targetRect), strTemp);
    }

    Ipvm::Image8u imageSourceSmallResize;
    Ipvm::Image8u imageTargetSmallResize;

    // sourceRect는 imageStitch 에 있고,
    // targetRect를 imageOrigin 에서 찾아야 한다.
    imageSourceSmallResize = Ipvm::Image8u(imageStitch, sourceRect); //
    imageTargetSmallResize = Ipvm::Image8u(imageOrigin, targetRect);

    float fDstMinScore;
    float fGapScore;

    Ipvm::Image32r floatImageBuffer;
    m_processor->getReusableMemory().GetFloatImage(
        floatImageBuffer, m_processor->getImageLot().GetImageSizeX(), m_processor->getImageLot().GetImageSizeY());

    fMatchRate = -1.f;
    if (!CippModules::TemplateMatching_SurfacePeak(imageTargetSmallResize, Ipvm::Rect32s(imageTargetSmallResize),
            floatImageBuffer, imageSourceSmallResize, targetPoint, fDstMinScore, fGapScore))
    {
        return false;
    }

    fMatchRate = (float)max(100.f - (fDstMinScore * 1000.f), 0.f) * fGapScore;
    if (fMatchRate >= 100.f)
        fMatchRate = 0.f;

    Ipvm::Rect32s rtNewSearch_Sampling((int32_t)(targetPoint.m_x), (int32_t)(targetPoint.m_y),
        (int32_t)(targetPoint.m_x) + imageTargetSmallResize.GetSizeX(),
        (int32_t)(targetPoint.m_y) + imageTargetSmallResize.GetSizeY());
    rtNewSearch_Sampling.InflateRect(10, 10, 10, 10);

    targetPoint.m_x += targetSearchROI.m_left;
    targetPoint.m_y += targetSearchROI.m_top;

    return true;
}
