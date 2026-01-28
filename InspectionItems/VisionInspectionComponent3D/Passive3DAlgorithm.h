#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ComponentAlign.h"

//HDR_2_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionScale;

//HDR_6_________________________________ Header body
//
class CPassive3DAlgorithm
{
public:
    CPassive3DAlgorithm(sPassive_InfoDB* i_pPassiveInfoDB, SComponentAlignSpec* i_pPassiveAlignParam,
        VisionReusableMemory* i_pVisionReusableMemory, CPI_EdgeDetect* i_pEdgeDetect, CPI_Blob* i_pBlob,
        const float& ConvertGV_Gain, const float& ConvertGV_Offset, SComponentAlignResult& o_sPassiveAlignResult);
    virtual ~CPassive3DAlgorithm(void);

private:
    sPassive_InfoDB* m_pPassiveInfoDB;
    SComponentAlignSpec* m_pPassiveAlignParam;
    VisionReusableMemory* m_pVisionReusableMemory;
    CPI_Blob* m_pBlob;
    CPI_EdgeDetect* m_pEdgeDetect;
    SComponentAlignResult* m_pPassiveAlignResult;

    struct sEdgeSearchNeedInfo
    {
        Ipvm::Rect32s rtSearchROI;
        long nSearchLength;
        long nSearchWidth;
        long nROISize;
        long nSearchDirection;
        long nSearchType;
        Ipvm::Point32s2 ptSearchStartPos;
    };

    float m_ConvertGV_Gain;
    float m_ConvertGV_Offset;

public: //Main에서 호출할 함수들만 사용할것
    bool DoCapacitor_DetailAlign(const VisionScale& scale, const Ipvm::Image32r& i_ZMap,
        const Ipvm::Image16u& i_WIntensityMap, const Ipvm::Image8u& i_HMap, Ipvm::Image8u& AlignimageBuf);
    bool DoMIA_DetailAlign(const VisionScale& scale, const Ipvm::Image32r& i_ZMap,
        const Ipvm::Image16u& i_WIntensityMap, const Ipvm::Image8u& i_HMap);

private:
    bool DoRoughAlign(const VisionScale& scale, const Ipvm::Image8u& i_HMap);

private:
    bool GetAlignimage(const VisionScale& scale, const Ipvm::Rect32s& SearchROI, const Ipvm::Image8u& HMap,
        Ipvm::Image8u& Alignimage, const long& Substrate_Area_Size_um, const long& Substrate_Gap_Body_X_um,
        const long& Substrate_Gap_Body_Y_um, const long& Substrate_Area_GapX_um, const long& Substrate_Area_GapY_um,
        const float& ConvertGV_Gain, const float& ConvertGV_Offset);

private:
    bool DoAlignbySpecROI(const VisionScale& scale);

private:
    FPI_RECT GetShiftSpecROI(const VisionScale& scale, FPI_RECT i_frtSpecROI);
    bool CreateSpecROI(const VisionScale& scale, FPI_RECT i_frtShift_SpecROI, Ipvm::Rect32s& o_SpecROI,
        Ipvm::Rect32s& o_SpecROI_Body, std::vector<Ipvm::Rect32s>& o_vecSpecElectrodeROI);
    bool CreatePassiveElectrodeROI_bySpec(
        const VisionScale& scale, Ipvm::Rect32s i_rtSpecROI, std::vector<Ipvm::Rect32s>& o_vecSpecElectrodeROI);

    Ipvm::Rect32s GetPassive_Rough_SearchROI(Ipvm::Rect32s i_rtSpecROI);

    bool Passive_RoughAlign_byBlobCneterGravity(const VisionScale& scale, const Ipvm::Image8u& i_imgHMap,
        Ipvm::Rect32s i_rtRoughSearchROI, Ipvm::Rect32s& o_RoughAlignResult);
    Ipvm::Point32r2 GetCenterGravity(const Ipvm::Image32s& i_image, Ipvm::Rect32s i_rtCalcROI);
    bool MakeDetailAlignROI_byRoughAlign(const VisionScale& scale, const Ipvm::Rect32s& i_rtRoughAlignROI,
        std::vector<std::vector<FPI_RECT>>& o_vec2frtChip_SearchROI);
    float CheckOverPosbyCenterPos(const float i_fCenterPos, const float i_fSearchROIPos, const long i_nROIDirection);
    bool Passive_Align_byEdge(const Ipvm::Image8u& i_imageHMap,
        std::vector<std::vector<FPI_RECT>> i_vec2frtChip_SearchROI, Ipvm::Rect32s& o_rtAlignReulst);
    bool GetfrtROI_byCrosspoint(std::vector<Ipvm::LineEq32r> i_vecLine, FPI_RECT& o_frtResultROI);
    bool Passive_Align_byEdge(const VisionScale& scale, const Ipvm::Image8u& i_imageHMap, Ipvm::Rect32s i_rtRoughAlign,
        Ipvm::Rect32s i_rtRoughAlign_Body, Ipvm::Rect32s& o_rtAlignReulst,
        std::vector<std::vector<FPI_RECT>> i_vec2frtChip_SearchROI);
    bool LineEdgeSearch(const Ipvm::Image8u& image, Ipvm::Rect32s i_rtROI, int i_nDir, long nEdgeType,
        float i_fAtLeastEdgeVal, float i_fCompareSizeX, float i_fCompareSizeY, std::vector<Ipvm::Point32r2>& vecptEdge,
        long i_nEdgeNum, BOOL bFirstEdge = FALSE);
    float GetFitEdge(const std::vector<Ipvm::Point32r2>& Edges, SComponentAlignResult* PassiveAlignResult,
        const float& Tolerence, const long& EdgeDir);
    bool GetEdgeSearchNeedInfo(const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI,
        std::vector<sEdgeSearchNeedInfo>& o_vecsEdgeSearchNeedInfo);

    Ipvm::Point32s2 GetEdgeSearchSize(long i_nCurDirection, Ipvm::Rect32s i_rtSearchROI);
    Ipvm::Point32s2 GetEdgeSearch_StartPoint(long i_SearchDir, Ipvm::Rect32s i_rtSearchROI);
    float GetEdgeThresholdValue(enumPassiveTypeDefine i_enumPassiveType, long i_nDir, float i_fPassiveAngle);
    bool RemoveNoisebyLine(const std::vector<Ipvm::Point32r2>& i_vecptEdge,
        std::vector<Ipvm::Point32r2>& o_vecptFitEdge, Ipvm::LineEq32r& o_line, float fTolerence);
    bool SetElectrodeArea(const VisionScale& scale, const Ipvm::Rect32s& i_rtAlignResult,
        std::vector<Ipvm::Rect32s>& o_vecrtElectrodeResult);
    std::vector<Ipvm::Rect32s> GetSubstrateMesureROI(const VisionScale& scale, const Ipvm::Rect32s& i_rtAlignResult,
        const long& Area_Size_um, const long& Area_GapX_From_Body_um, const long& Area_GapY_From_Body_um,
        const long& Area_GapX_um, const long& Area_GapY_um);

    bool CalcPassiveData(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_Zmap, const Ipvm::Image16u& i_wInten,
        const bool i_bisMIAType = false);
    bool GetPassive_Height(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_Zmap, const Ipvm::Image16u& i_wInten,
        float i_fMinimumVailidRange, float i_fMaximumVailidRange, const bool i_bisMIAType = false);
    float GetSubstrateAvgHeight(const Ipvm::Image8u& i_byZmap, const Ipvm::Image32r& i_Zmap,
        const Ipvm::Image16u& i_wInten, float i_fMinimumVailidRange, float i_fMaximumVailidRange);
    bool GetPassiveElectrode_Height(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_Zmap,
        const Ipvm::Image16u& i_wInten, float i_fMinimumVailidRange, float i_fMaximumVailidRange);
    bool GetPassive_Body_Height(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_Zmap,
        const Ipvm::Image16u& i_wInten, float i_fMinimumVailidRange, float i_fMaximumVailidRange);
    void GetSplitBodyROI(long i_nSpitNumH, long i_nSplitNumY, Ipvm::Rect32s i_rtValidBody,
        std::vector<Ipvm::Rect32s>& o_vecrtValidSplitBody);
    int GetROISize(Ipvm::Rect32s i_rtSplitROI, float i_fAngle, long i_nDirection);
    float CalcZForSubstrate(const Ipvm::Image32r& i_zmap, Ipvm::Rect32s i_rtArea, float i_fMinZ, float i_fMaxZ,
        long& o_nValidDataNum, const Ipvm::Image16u* i_pwInten);
    float CalcZData(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_zmap, Ipvm::Rect32s i_rtArea, float i_fMinZ,
        float i_fMaxZ, long& o_nValidDataNum, const Ipvm::Image16u* i_pwInten, const bool i_bisMIAType = false,
        const bool i_bUseFilterData = false);
    bool GetMeanZValue(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_zmap, Ipvm::Rect32s i_rtArea,
        std::vector<Ipvm::Point32s2>& o_vecfptValidArea, double& o_MeanZ_Value, const bool i_bisMIAType = false);
    bool GetZValueVailidArea(
        const Ipvm::Image8u& i_Hmap, const Ipvm::Rect32s& i_rtArea, std::vector<Ipvm::Point32s2>& o_vecfptValidPoint);
    bool GetThresholdValue_HistogramPeak(const Ipvm::Image8u& i_Hmap, const Ipvm::Rect32s& i_rtArea,
        long& o_nLowThresholdValue, long& o_nHighThresholdValue);
    long GetAvgGrayValue(const Ipvm::Image8u& i_Hmap, std::vector<Ipvm::Rect32s> i_vecrtSubstrateROI);
};