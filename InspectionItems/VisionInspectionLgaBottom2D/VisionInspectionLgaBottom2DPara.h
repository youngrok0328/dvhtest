#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspFrameIndex.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
enum DirectionType
{
    DirectionType_Start = 0,
    DirectionType_Top = DirectionType_Start,
    DirectionType_Bottom,
    DirectionType_Left,
    DirectionType_Right,
    DirectionType_End,
};

enum LandType
{
    LandType_Start = 0,
    LandType_Rect = LandType_Start,
    LandType_Circle,
    LandType_End,
};

enum Rect_Direction
{
    Rect_Start = 0,
    Rect_Top = Rect_Start,
    Rect_Bottom,
    Rect_Left,
    Rect_Right,
    Rect_End,
};

class Land2DGroupInsp
{
public:
    CString LandOffsetXName;
    CString LandOffsetYName;
    CString LandOffsetRName;
    CString LandWidthName;
    CString LandLengthName;

public:
    void SetName(LPCTSTR group_id);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};

class LandPara
{
public:
    LandPara();
    ~LandPara();

    void Init();
    BOOL Copy(LandPara Data);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

public:
    long LandType; //0 : Rect, 1 : Circle

    //Common Parameter
    float m_fEdgeSearchLength;

    //Rect Parameter
    //m_vec2fSearchRangeStart[DirectionType] 0 : TOP, 1 : BOTTOM, 2 : LEFT, 3 : RIGHT
    std::vector<float> m_vec2fSearchRangeStart;
    //m_vec2fSearchRangeEnd[DirectionType] 0 : TOP, 1 : BOTTOM, 2 : LEFT, 3 : RIGHT
    std::vector<float> m_vec2fSearchRangeEnd;

    //Circle Parameter
    float m_fThresholdValue;
    long m_nEdgeSearchCount;
};

class VisionInspectionLgaBottom2DPara
{
public:
    VisionInspectionLgaBottom2DPara(VisionProcessing& parent);
    ~VisionInspectionLgaBottom2DPara(void);

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe
    //long m_nThresholdValue;

    long m_nIntensityofLand; // 0: Bright, 1: Dark

    //float m_fSearchLength;
    //float m_fTopSearchRangeStart;
    //float m_fTopSearchRangeEnd;
    //float m_fBottomSearchRangeStart;
    //float m_fBottomSearchRangeEnd;
    //float m_fLeftSearchRangeStart;
    //float m_fLeftSearchRangeEnd;
    //float m_fRightSearchRangeStart;
    //float m_fRightSearchRangeEnd;

    //kk 최대 저장 개수 10개 Fix
    std::vector<LandPara> m_vecLandParameter;

    std::vector<Land2DGroupInsp> m_vecstrGroupInspName;

    //spec 관리 변수
    std::vector<VisionInspectionSpec> m_vecVisionInspectionSpecs;
};
