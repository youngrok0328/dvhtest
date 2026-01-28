#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "VisionInspectionComponent3DPara.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionInspectionComponent3D;
class VisionScale;
struct SComponentAlignSpec;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
class SComponentAlignResult : public BaseAlignResult
{
public:
    //{{ Spec 영역
    Ipvm::Rect32s rtSpecChip;
    std::vector<Ipvm::Rect32s> vecrtSpecElect;
    Ipvm::Rect32s rtSpecBody;
    //}}

    //{{ ExtComponet에 의해 스펙으로 설정.
    Ipvm::Rect32s rtResChip;
    std::vector<Ipvm::Rect32s> vecrtResElect;
    Ipvm::Rect32s rtResBody;
    //}}

    //{{ Align 완료된 영역
    Ipvm::Rect32s rtValidComponent;
    std::vector<Ipvm::Rect32s> vecrtValidElect;
    Ipvm::Rect32s rtValidBody;
    //}}

    std::vector<Ipvm::Rect32s> vecrtSubstrateArea;

    //{{Measure ItemData
    //Passive
    float fCompHeight;
    float fCompHeightRatio;
    float fValidCompRatio;

    //Electrode
    std::vector<float> vecfElectrodeHeight;
    float fElectTiltHeight;

    //Body
    std::vector<Ipvm::Rect32s> vecrtSplitBody;
    std::vector<float> vecfSplitBody_Height;
    float fBodyHeight;
    float fBodyTiltHeight;

    //}}

    //Debug Item
    FPI_RECT Debug_frtShiftSpecROI;
    Ipvm::Rect32s Debug_rtRoughAlign_SearchROI;
    std::vector<Ipvm::Rect32s> Debug_vecrtDetailPassiveSearchROI;
    std::vector<Ipvm::Point32r2> Debug_vecfptPassiveEdge;

    //Passive Height
    float Debug_fAlignAreaPassiveHeight;
    std::vector<float> Debug_vecfPassive_Calc_SubstrateArea_Height;
    std::vector<float> Debug_vecfPassive_Calc_SubstrateArea_Height_Ratio;
    //Electrode
    std::vector<float> Debug_vecfElectrode_Calc_SubstrateArea_Height;
    std::vector<float> Debug_vecfElectrode_Area_Height;
    float Debug_fSubstrate_Avg_Height_Electrode;

    //Body
    std::vector<float> Debug_vecfBody_Calc_SubstrateArea_Height;
    std::vector<float> Debug_vecfValidSplitBodyRatio;
    std::vector<float> Debug_vecfBody_Calc_SubstrateArea_Height_Ratio;

    void Clear()
    {
        rtSpecChip = Ipvm::Rect32s(0, 0, 0, 0);
        vecrtSpecElect.clear();
        rtSpecBody = Ipvm::Rect32s(0, 0, 0, 0);

        rtResChip = Ipvm::Rect32s(0, 0, 0, 0);
        vecrtResElect.clear();
        rtResBody = Ipvm::Rect32s(0, 0, 0, 0);

        rtValidComponent = Ipvm::Rect32s(0, 0, 0, 0);
        vecrtValidElect.clear();
        rtValidBody = Ipvm::Rect32s(0, 0, 0, 0);

        fCompHeight = 0.f;
        fCompHeightRatio = 0.f;
        fValidCompRatio = 0.f;

        vecfElectrodeHeight.clear();
        fElectTiltHeight = 0.f;

        fBodyHeight = 0.f;
        fBodyTiltHeight = 0.f;

        vecrtSplitBody.clear();
        vecfSplitBody_Height.clear();

        vecrtSubstrateArea.clear();

        Debug_frtShiftSpecROI = FPI_RECT(
            Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f));
        Debug_rtRoughAlign_SearchROI = Ipvm::Rect32s(0, 0, 0, 0);
        Debug_fAlignAreaPassiveHeight = 0.f;
        Debug_fSubstrate_Avg_Height_Electrode = 0.f;
        Debug_vecfPassive_Calc_SubstrateArea_Height.clear();
        Debug_vecrtDetailPassiveSearchROI.clear();
        Debug_vecfptPassiveEdge.clear();
        Debug_vecfElectrode_Calc_SubstrateArea_Height.clear();
        Debug_vecfElectrode_Area_Height.clear();
        Debug_vecfBody_Calc_SubstrateArea_Height.clear();
        Debug_vecfValidSplitBodyRatio.clear();
        Debug_vecfBody_Calc_SubstrateArea_Height_Ratio.clear();
        Debug_vecfPassive_Calc_SubstrateArea_Height_Ratio.clear();
    }

    SComponentAlignResult()
    {
        Clear();
    }

    virtual ~SComponentAlignResult()
    {
    }
};

struct ComponentDebugResult
{
    Ipvm::Image32r m_zmap;
    Ipvm::Image16u m_wIntensity;

    Ipvm::Image8u m_imgHmap;
    Ipvm::Image8u m_imgInten;
};

class CComponentAlign
{
public:
    CComponentAlign(VisionInspectionComponent3D* pChipAlign);
    ~CComponentAlign(void);

    VisionInspectionComponent3D* m_pChipVision;
    Ipvm::Point32r2 m_fPixelperMM;

    SComponentAlignSpec* GetAlignSpec(CString i_strSelectSpecName);
    SComponentAlignResult m_result;

    //BOOL DoAlign(
    //    const VisionScale& scale, sPassive_InfoDB* InfoDB, ComponentDebugResult* debugResult = nullptr);

    BOOL DoAlign(const Ipvm::Image32r& Zmap, const Ipvm::Image16u& WIntensity, const Ipvm::Image8u& HMap,
        const Ipvm::Image8u& ByInten, Ipvm::Image8u& AlignimageBuf, const VisionScale& scale, sPassive_InfoDB* InfoDB,
        ComponentDebugResult* debugResult = nullptr);

    BOOL RotateComponentImage(const Ipvm::Image32r& i_image, Ipvm::Image32r& o_image, float i_fAngle,
        Ipvm::Rect32s i_rtImage, double i_fCenterX, double i_fCenterY);
    BOOL RotateComponentWORDImage(const Ipvm::Image16u& i_image, Ipvm::Image16u& o_image, float i_fAngle,
        Ipvm::Rect32s i_rtImage, double i_fCenterX, double i_fCenterY);
    void ConvertFloattoByteData(
        const Ipvm::Image32r& i_image, float fMultiplexData, Ipvm::Rect32s rtProc, Ipvm::Image8u& o_image);
    void ConvertWORDtoByteData(
        const Ipvm::Image16u& i_image, float fMultiplexData, Ipvm::Rect32s rtProc, Ipvm::Image8u& o_image);

private:
    bool GetGrayScaleFactor(const float& srcLowerBound, const float& srcUpperBound, const uint8_t& dstNoiseValue,
        const uint8_t& dstLowerBound, const uint8_t& dstUpperBound, float& gain, float& offset);

private:
    float m_fROIConvertScale;
    float m_fROIConvertScaleInten;
};
