#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

#define Group_MaxSize 10

//HDR_6_________________________________ Header body
//
enum LandType
{
    LandType_Start = 0,
    LandType_Rect = LandType_Start,
    LandType_Circle,
    LandType_End,
};

class Land3DGroupInsp
{
public:
    CString LandCoplName;
    CString LandUnitCoplName;
    CString LandHeightName;
    CString LandWarpageName;
    CString LandUnitWarpageName;

public:
    void SetName(LPCTSTR group_id);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};

class Land3DPara
{
public:
    Land3DPara();
    ~Land3DPara();

    void Init();
    BOOL Copy(Land3DPara Data);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

public:
    long LandType; //0 : Rect, 1 : Circle

    //Edge Search Offset
    float m_fObjSearchX; // mm
    float m_fObjSearchY; // mm

    //Land Mask Parameter
    float m_fLandMaskErodeSize;

    //SR Mask Parameter
    float m_SR_landIgnoreSize;
    float m_SR_landDilateSize;
};

class VisionInspectionLgaBottom3DPara
{
public:
    VisionInspectionLgaBottom3DPara(void);
    ~VisionInspectionLgaBottom3DPara(void);

    void Init();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    long m_nBlobSearchImage; // 0 : grayimage, 1: v-map
    BOOL isReverseThreshold;

    float m_fSRSearchExtRatio; // %

    int m_nSR_Algorithm;
    float m_SR_topPercentAverage_Max; //kircheis_Tan;
    float m_SR_topPercentAverage_Min; //kircheis_Tan
    float m_SR_IgnoreOffsetXum;
    float m_SR_IgnoreOffsetYum;
    double m_SR_visibilityLowerBound;

    //Group별 Land Parameter 관리
    std::vector<Land3DPara> m_vecLandParameter;

    std::vector<Land3DGroupInsp> m_vecstrGroupInspName;

    //UnitCopl Group 관리
    std::vector<std::vector<long>> m_vec2UnitCoplGroup;
    std::vector<CString> m_vecstrGroupUnitCoplName;
    long m_nGroupUnitCoplCount;

    //spec 관리 변수
    std::vector<VisionInspectionSpec> m_vecVisionInspectionSpecs;
};
