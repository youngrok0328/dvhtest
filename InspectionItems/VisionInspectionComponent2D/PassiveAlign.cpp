//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PassiveAlign.h"

//CPP_2_________________________________ This project's headers
#include "Passive2DAlgorithm.h"
#include "Result.h"
#include "VisionInspectionComponent2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ErrorMax 2.f
#define ErrorMin 0.4f

//CPP_7_________________________________ Implementation body
//
CPassiveAlign::CPassiveAlign(VisionInspectionComponent2D* pChipAlign)
{
    m_pChipVision = pChipAlign;
}

CPassiveAlign::~CPassiveAlign(void)
{
}

BOOL CPassiveAlign::DoAlign(const VisionScale& scale, sPassive_InfoDB* i_pPassiveInfoDB, ResultPassiveItem* out_result)
{
    m_outResult = out_result;
    m_result.Clear();

    m_outResult = out_result;
    m_result.Clear();

    if (i_pPassiveInfoDB == NULL || i_pPassiveInfoDB->eComponentType != eComponentTypeDefine::tyPASSIVE)
        return FALSE;

    PassiveAlignSpec::CapAlignSpec* pCapSpec = GetCapAlignSpec(i_pPassiveInfoDB->strSpecName);
    if (pCapSpec == NULL)
        return FALSE;

    //Create Image Buff And Set
    const Ipvm::Image8u imgOrg_Rough_Align_Image = pCapSpec->m_Rough_align_ImageFrameIndex.getImage(false);
    const Ipvm::Image8u imgOrg_Outline_Align_ElectrodeImage
        = pCapSpec->m_Outline_align_Electrode_ImageFrameIndex.getImage(false);
    const Ipvm::Image8u imgOrg_Outline_Align_BodyImage = pCapSpec->m_Outline_align_Body_ImageFrameIndex.getImage(false);

    if (imgOrg_Rough_Align_Image.GetMem() == nullptr || imgOrg_Outline_Align_ElectrodeImage.GetMem() == nullptr
        || imgOrg_Outline_Align_BodyImage.GetMem() == nullptr)
        return FALSE;

    Ipvm::Image8u img_Rough_Align = imgOrg_Rough_Align_Image;
    Ipvm::Image8u img_Outline_Align_Electrode = imgOrg_Outline_Align_ElectrodeImage;
    Ipvm::Image8u img_Outline_Align_Body = imgOrg_Outline_Align_BodyImage;

    if (!MakeSpecROIs(i_pPassiveInfoDB))
        return FALSE;

    CPassive2DAlgorithm PassiveAlgorithm(
        i_pPassiveInfoDB, pCapSpec, m_pChipVision->m_pEdgeDetect, m_outResult, m_result);
    if (i_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_Capacitor
        || i_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_Register)
    {
        m_result.bAlignSuccess = PassiveAlgorithm.DoCapacitor_Align(
            scale, imgOrg_Rough_Align_Image, imgOrg_Outline_Align_ElectrodeImage, imgOrg_Outline_Align_BodyImage);
    }
    else if (i_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        m_result.bAlignSuccess
            = PassiveAlgorithm.DoMIA_Align(scale, imgOrg_Rough_Align_Image, imgOrg_Outline_Align_BodyImage);
    }

    return m_result.bAlignSuccess;
}

BOOL CPassiveAlign::RotateImage(
    const Ipvm::Image8u& imgOrg, Ipvm::Image8u& o_imgRotate, sPassive_InfoDB* i_pPassiveInfoDB)
{
    Ipvm::Rect32r frtChip = i_pPassiveInfoDB->sfrtSpecROI_px.GetFRect();
    long nInflateX = static_cast<long>(frtChip.Width() + .5f);
    long nInflateY = static_cast<long>(frtChip.Height() + .5f);
    Ipvm::Rect32s rtChip = Ipvm::Conversion::ToRect32s(frtChip);
    rtChip.InflateRect(nInflateX, nInflateY);

    if (Ipvm::ImageProcessing::RotateLinearInterpolation(imgOrg, rtChip, frtChip.CenterPoint(),
            -i_pPassiveInfoDB->fPassiveAngle, Ipvm::Point32r2(0.f, 0.f), o_imgRotate)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CPassiveAlign::MakeSpecROIs(sPassive_InfoDB* i_pPassiveInfoDB)
{
    const auto& scale = m_pChipVision->getScale();
    Ipvm::Point32r2 mm2px = scale.mmToPixel();

    // 패드는 거버가 있을 경우와 없을 경우에 대해서 처리해야 한다.
    m_result.m_elect_spec.resize(2);
    m_result.m_pad_spec.resize(2);

    Ipvm::Rect32r rtSpecChip;

    Ipvm::Point32r2 fptCenter = i_pPassiveInfoDB->sfrtSpecROI_px.GetCenter();
    if (i_pPassiveInfoDB->fPassiveAngle == 90.f) // Vertical
    {
        float fChipWidth = i_pPassiveInfoDB->fPassiveSpecWidth_mm * mm2px.m_x;
        float fChipLength = i_pPassiveInfoDB->fPassiveSpecLength_mm * mm2px.m_y;

        rtSpecChip.m_left = (float)fptCenter.m_x - fChipWidth / 2.f;
        rtSpecChip.m_right = (float)fptCenter.m_x + fChipWidth / 2.f;
        rtSpecChip.m_top = (float)fptCenter.m_y - fChipLength / 2.f;
        rtSpecChip.m_bottom = (float)fptCenter.m_y + fChipLength / 2.f;
        m_result.m_chip_spec = rtSpecChip;

        float fElectWidth = i_pPassiveInfoDB->fPassiveElectrodeWidth_mm * mm2px.m_y;
        m_result.m_elect_spec[0] = m_result.m_elect_spec[1] = Ipvm::Conversion::ToRect32s(rtSpecChip);
        m_result.m_elect_spec[0].m_bottom = (long)((float)rtSpecChip.m_top + fElectWidth + .5f);
        m_result.m_elect_spec[1].m_top = (long)((float)rtSpecChip.m_bottom - fElectWidth + .5f);
        m_result.rtSpecBody = Ipvm::Conversion::ToRect32s(rtSpecChip);
        m_result.rtSpecBody.m_top = m_result.m_elect_spec[0].m_bottom;
        m_result.rtSpecBody.m_bottom = m_result.m_elect_spec[1].m_top;

        // 패드 영역은 거버 이용시에만 확인용으로 .... 검사에서 쓸일은 없을 듯...
        float fChipWidthhwithPAD = i_pPassiveInfoDB->fPassivePAD_Width_mm * mm2px.m_x;
        float fChipLengthwithPAD = i_pPassiveInfoDB->fPassivePAD_Length_mm * mm2px.m_y;
        Ipvm::Rect32r rtTotalPAD = rtSpecChip;
        rtTotalPAD.m_top = fptCenter.m_y - fChipLengthwithPAD / 2.f;
        rtTotalPAD.m_bottom = fptCenter.m_y + fChipLengthwithPAD / 2.f;
        rtTotalPAD.m_left = fptCenter.m_x - fChipWidthhwithPAD / 2.f;
        rtTotalPAD.m_right = fptCenter.m_x + fChipWidthhwithPAD / 2.f;

        long nPADWidth = (long)((fChipLengthwithPAD - (float)rtSpecChip.Height()) / 2.f + .5f);
        m_result.m_pad_spec[0] = m_result.m_pad_spec[1] = rtTotalPAD;
        m_result.m_pad_spec[0].m_bottom = rtTotalPAD.m_top + nPADWidth;
        m_result.m_pad_spec[1].m_top = rtTotalPAD.m_bottom - nPADWidth;
    }
    else
    {
        // 회전된 영상에 영역을 맞춘다.
        float fRadian = (float)(-i_pPassiveInfoDB->fPassiveAngle * DEF_DEG_TO_RAD);
        rtSpecChip = i_pPassiveInfoDB->sfrtSpecROI_px.Rotate(fRadian).GetFRect();
        m_result.m_chip_spec = rtSpecChip;

        float fElectWidth = i_pPassiveInfoDB->fPassiveElectrodeWidth_mm * mm2px.m_x;
        m_result.m_elect_spec[0] = m_result.m_elect_spec[1] = Ipvm::Conversion::ToRect32s(rtSpecChip);
        m_result.m_elect_spec[0].m_right = (long)((float)rtSpecChip.m_left + fElectWidth + .5f);
        m_result.m_elect_spec[1].m_left = (long)((float)rtSpecChip.m_right - fElectWidth + .5f);
        m_result.rtSpecBody = Ipvm::Conversion::ToRect32s(rtSpecChip);
        m_result.rtSpecBody.m_left = m_result.m_elect_spec[0].m_right;
        m_result.rtSpecBody.m_right = m_result.m_elect_spec[1].m_left;

        float fChipWidthhwithPAD = i_pPassiveInfoDB->fPassivePAD_Width_mm * mm2px.m_y;
        float fChipLengthwithPAD = i_pPassiveInfoDB->fPassivePAD_Length_mm * mm2px.m_x;
        Ipvm::Rect32r rtTotalPAD = rtSpecChip;
        rtTotalPAD.m_left = fptCenter.m_x - fChipLengthwithPAD / 2.f;
        rtTotalPAD.m_right = fptCenter.m_x + fChipLengthwithPAD / 2.f;
        rtTotalPAD.m_top = fptCenter.m_y - fChipWidthhwithPAD / 2.f;
        rtTotalPAD.m_bottom = fptCenter.m_y + fChipWidthhwithPAD / 2.f;

        long nPADWidth = (long)((fChipLengthwithPAD - (float)rtSpecChip.Width()) / 2.f + .5f);
        m_result.m_pad_spec[0] = m_result.m_pad_spec[1] = rtTotalPAD;
        m_result.m_pad_spec[0].m_right = rtTotalPAD.m_left + nPADWidth;
        m_result.m_pad_spec[1].m_left = rtTotalPAD.m_right - nPADWidth;
    }

    m_result.rtChipSearch = Ipvm::Conversion::ToRect32s(rtSpecChip);

    return TRUE;
}

PassiveAlignSpec::CapAlignSpec* CPassiveAlign::GetCapAlignSpec(CString i_strSelectSpecName)
{
    PassiveAlignSpec::CapAlignSpec* pSpec = NULL;
    if (i_strSelectSpecName.IsEmpty())
        return pSpec;

    long nPassiveAlgoSpecNum = (long)m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec.size();
    for (long nAlgo = 0; nAlgo < nPassiveAlgoSpecNum; nAlgo++)
    {
        if (i_strSelectSpecName.Compare(m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec[nAlgo].strSpecName) == 0)
        {
            pSpec = &m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec[nAlgo].AlignSpec.m_capAlignSpec;
            break;
        }
    }

    return pSpec;
}