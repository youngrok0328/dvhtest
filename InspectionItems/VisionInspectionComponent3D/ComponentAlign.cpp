//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ComponentAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionComponent3D.h"
#include "Passive3DAlgorithm.h"
#include "VisionInspectionComponent3D.h"
#include "VisionInspectionComponent3DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NC_SUB(x, y) ((x == Ipvm::k_noiseValue32r || y == Ipvm::k_noiseValue32r) ? Ipvm::k_noiseValue32r : x - y)
#define NC_MUL(x, y) ((x == Ipvm::k_noiseValue32r || y == Ipvm::k_noiseValue32r) ? Ipvm::k_noiseValue32r : x * y)
#define NC_DIV(x, y) ((x == Ipvm::k_noiseValue32r || y == Ipvm::k_noiseValue32r) ? Ipvm::k_noiseValue32r : x / y)

//CPP_7_________________________________ Implementation body
//
CComponentAlign::CComponentAlign(VisionInspectionComponent3D* pChipAlign)
    : m_pChipVision(pChipAlign)
    , m_fPixelperMM(Ipvm::Point32r2(0.f, 0.f))
    , m_fROIConvertScale(0.f)
    , m_fROIConvertScaleInten(0.f)
{
    m_result.Clear();
}

CComponentAlign::~CComponentAlign(void)
{
}

//BOOL CComponentAlign::DoAlign(const VisionScale& scale, sPassive_InfoDB* InfoDB, ComponentDebugResult* debugResult)
BOOL CComponentAlign::DoAlign(const Ipvm::Image32r& Zmap, const Ipvm::Image16u& WIntensity, const Ipvm::Image8u& HMap,
    const Ipvm::Image8u& ByInten, Ipvm::Image8u& AlignimageBuf, const VisionScale& scale, sPassive_InfoDB* InfoDB,
    ComponentDebugResult* debugResult)
{
    ComponentDebugResult debugTempResult;
    bool useDebug = true;
    if (debugResult == nullptr)
    {
        debugResult = &debugTempResult;
        useDebug = false;
    }

    m_result.Clear();
    m_fPixelperMM = Ipvm::Point32r2(0.f, 0.f);
    m_fPixelperMM = m_pChipVision->m_fptPixelperMM;

    if (Zmap.GetMem() == nullptr || WIntensity.GetMem() == nullptr || HMap.GetMem() == nullptr
        || ByInten.GetMem() == nullptr || AlignimageBuf.GetMem() == nullptr)
        return FALSE;

    Ipvm::Image32r calcZmap = Zmap;
    Ipvm::Image16u calcWIntenisty = WIntensity;
    Ipvm::Image8u calcHMap = HMap;
    Ipvm::Image8u calcByInten = ByInten;

    m_fROIConvertScale = m_pChipVision->m_fConvertScale;
    m_fROIConvertScaleInten = m_pChipVision->m_fConvertScaleInten;

    float ConvertGV_Gain(-1.f), ConvertGV_Offset(-1.f);
    if (GetGrayScaleFactor(m_pChipVision->getImageLotInsp().m_heightRangeMin,
            m_pChipVision->getImageLotInsp().m_heightRangeMax, 0, 1, 255, ConvertGV_Gain, ConvertGV_Offset)
        == false)
    {
        return FALSE;
    }

    if (InfoDB == NULL || InfoDB->eComponentType != eComponentTypeDefine::tyPASSIVE)
        return FALSE;

    SComponentAlignSpec* pSpec = GetAlignSpec(InfoDB->strSpecName);
    if (pSpec == NULL)
        return FALSE;

    // 소자 X, Y방향 이동.
    FPI_RECT sfrtMoveChip = InfoDB->sfrtSpecROI_px;

    float fShiftXPos = CAST_FLOAT(pSpec->m_fSpecROI_ShiftXPos) / 1000.f * m_pChipVision->m_fptPixelperMM.m_x;
    float fShiftYPos = CAST_FLOAT(pSpec->m_fSpecROI_ShiftYPos) / 1000.f * m_pChipVision->m_fptPixelperMM.m_y;
    sfrtMoveChip.Move(fShiftXPos, fShiftYPos);

    // 검사 영역 설정.
    Ipvm::Rect32r frtChip = sfrtMoveChip.GetExtFRect();
    long nInflateX = static_cast<long>(frtChip.Width() + .5f);
    long nInflateY = static_cast<long>(frtChip.Height() + .5f);
    Ipvm::Rect32s rtProcImage = Ipvm::Conversion::ToRect32s(frtChip);
    rtProcImage.InflateRect((int)(nInflateX * 1.5f), (int)(nInflateY * 1.5f));
    Ipvm::Point32r2 fptGbrCenter = frtChip.CenterPoint();

    BOOL bRotateImage = !(InfoDB->fPassiveAngle == 0.f || InfoDB->fPassiveAngle == 90.f);

    if (bRotateImage)
    {
        if (!m_pChipVision->getReusableMemory().GetInspFloatImage(calcZmap))
            return FALSE;
        if (!m_pChipVision->getReusableMemory().GetInspWordImage(calcWIntenisty))
            return FALSE;
        if (!m_pChipVision->getReusableMemory().GetInspByteImage(calcHMap))
            return FALSE;
        if (!m_pChipVision->getReusableMemory().GetInspByteImage(calcByInten))
            return FALSE;

        if (!RotateComponentImage(
                Zmap, calcZmap, InfoDB->fPassiveAngle, rtProcImage, (double)fptGbrCenter.m_x, (double)fptGbrCenter.m_y))
            return FALSE;
        if (!RotateComponentWORDImage(WIntensity, calcWIntenisty, InfoDB->fPassiveAngle, rtProcImage,
                (double)fptGbrCenter.m_x, (double)fptGbrCenter.m_y))
            return FALSE;

        float fScaleConvert = 255.f
            / (m_pChipVision->getImageLotInsp().m_heightRangeMax - m_pChipVision->getImageLotInsp().m_heightRangeMin);
        m_fROIConvertScale = fScaleConvert;

        CippModules::ConvertFloattoByteData(calcZmap, fScaleConvert, rtProcImage, calcHMap);

        m_fROIConvertScaleInten = m_pChipVision->m_fConvertScaleInten;

        ConvertWORDtoByteData(calcWIntenisty, m_fROIConvertScaleInten, rtProcImage, calcByInten);
    }

    CPassive3DAlgorithm Passive3DAlignAlgorithm(InfoDB, pSpec, &m_pChipVision->getReusableMemory(),
        m_pChipVision->m_pEdgeDetect, m_pChipVision->m_pBlob, ConvertGV_Gain, ConvertGV_Offset, m_result);

    if (InfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_Capacitor
        || InfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_Register)
    {
        Passive3DAlignAlgorithm.DoCapacitor_DetailAlign(scale, calcZmap, calcWIntenisty, calcHMap, AlignimageBuf);
    }
    else if (InfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        Passive3DAlignAlgorithm.DoMIA_DetailAlign(scale, calcZmap, calcWIntenisty, calcHMap);
    }

    debugResult->m_zmap = calcZmap;
    debugResult->m_wIntensity = calcWIntenisty;
    debugResult->m_imgHmap = calcHMap;
    debugResult->m_imgInten = calcByInten;

    return TRUE;
}

BOOL CComponentAlign::RotateComponentImage(const Ipvm::Image32r& i_image, Ipvm::Image32r& o_image, float fAngle,
    Ipvm::Rect32s rtImage, double fCenterX, double fCenterY)
{
    if (Ipvm::ImageProcessing::RotateNearestInterpolation(i_image, rtImage,
            Ipvm::Point32r2(CAST_FLOAT(fCenterX), CAST_FLOAT(fCenterY)), fAngle, Ipvm::Point32r2(0.f, 0.f), o_image)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL CComponentAlign::RotateComponentWORDImage(const Ipvm::Image16u& i_image, Ipvm::Image16u& o_image, float i_fAngle,
    Ipvm::Rect32s i_rtImage, double i_fCenterX, double i_fCenterY)
{
    float fAngle = -i_fAngle;

    if (Ipvm::ImageProcessing::RotateNearestInterpolation(i_image, i_rtImage,
            Ipvm::Point32r2((float)i_fCenterX, (float)i_fCenterY), -fAngle, Ipvm::Point32r2(0.f, 0.f), o_image)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

void CComponentAlign::ConvertFloattoByteData(
    const Ipvm::Image32r& i_image, float fMultiplexData, Ipvm::Rect32s rtProc, Ipvm::Image8u& o_image)
{
    for (long y = rtProc.m_top; y < rtProc.m_bottom; y++)
    {
        auto* src_y = i_image.GetMem(0, y);
        auto* dst_y = o_image.GetMem(0, y);
        for (long x = rtProc.m_left; x < rtProc.m_right; x++)
        {
            float fVal = src_y[x] * fMultiplexData;

            if (fVal > 255.f || fVal < 0.f)
                fVal = 0.f;
            dst_y[x] = (BYTE)fVal;
        }
    }
}

void CComponentAlign::ConvertWORDtoByteData(
    const Ipvm::Image16u& i_image, float fMultiplexData, Ipvm::Rect32s rtProc, Ipvm::Image8u& o_image)
{
    for (long y = rtProc.m_top; y < rtProc.m_bottom; y++)
    {
        auto* dst_y = o_image.GetMem(0, y);
        auto* image_y = i_image.GetMem(0, y);
        for (long x = rtProc.m_left; x < rtProc.m_right; x++)
        {
            float fVal = (float)image_y[x] * fMultiplexData;

            if (fVal > 255.f)
                fVal = 255.f;
            if (fVal < 0.f)
                fVal = 0.f;
            dst_y[x] = (BYTE)fVal;
        }
    }
}

SComponentAlignSpec* CComponentAlign::GetAlignSpec(CString i_strSelectSpecName)
{
    SComponentAlignSpec* pSpec = NULL;
    if (i_strSelectSpecName.IsEmpty())
        return pSpec;

    long nChipAlgoSpecNum = (long)m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec.size();
    for (long nAlgo = 0; nAlgo < nChipAlgoSpecNum; nAlgo++)
    {
        if (i_strSelectSpecName.Compare(m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nAlgo].strSpecName) == 0)
        {
            pSpec = &m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nAlgo].CompAlign;
            break;
        }
    }

    return pSpec;
}

bool CComponentAlign::GetGrayScaleFactor(const float& srcLowerBound, const float& srcUpperBound,
    const uint8_t& dstNoiseValue, const uint8_t& dstLowerBound, const uint8_t& dstUpperBound, float& gain,
    float& offset)
{
    UNREFERENCED_PARAMETER(dstNoiseValue);

    const float validSrcLowerBound = min(srcLowerBound, srcUpperBound);
    const float validSrcUpperBound = max(srcLowerBound, srcUpperBound);

    if (validSrcLowerBound == validSrcUpperBound)
        return false;

    const uint8_t validDstLowerBound = min(dstLowerBound, dstUpperBound);
    const uint8_t validDstUpperBound = max(dstLowerBound, dstUpperBound);

    gain = (validDstUpperBound - validDstLowerBound) / (validSrcUpperBound - validSrcLowerBound);
    offset = -validSrcLowerBound * gain + validDstLowerBound + 0.5f; // 반올림 고려

    return true;
}