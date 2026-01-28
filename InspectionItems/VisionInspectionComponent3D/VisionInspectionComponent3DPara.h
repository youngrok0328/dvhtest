#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>

//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class CPackageSpec;

//HDR_6_________________________________ Header body
//
struct SComponentAlignSpec_OriginDataFloatToDouble
{
    SComponentAlignSpec_OriginDataFloatToDouble();

    BOOL LinkDataBase(CiDataBase& db);

    float fSearchRatio;
    float fMoveXPos;
    float fMoveYPos;

    float fLandSize;

    float fLandDistW;
    float fLandDistL;

    float m_fLandShiftW;
    float m_fLandShiftL;
    float m_fLandVisibilityLowerBound;

    float fBodyOuterW;
    float fBodyOuterL;

    float fZCalcRange;

    float fElectSearchOffset;
    float fElectSearchOuterOffset;

    float fValidElectWidthRatio;
    float fValidElectLengthRatio;
};

// {{ Cap 3D Algo Spec
struct SComponentAlignSpec
{
    SComponentAlignSpec();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void SetConvertingData(SComponentAlignSpec_OriginDataFloatToDouble i_OriginData);

    BOOL bOriginConvertData; //mc_MED3 기준의 변수로 변환하기 위한 변수
    long m_nExistVersionNumber; //mc_Search Length 및 Width Ratio관련하여 Convert이 필요
    long
        m_nConvertedAlignSearchParams; //수정완:1, 미수정:-1.//kircheis_debug20220721//바로 윗줄의 변수가 이름과 용법이 맞지 않는것 같아 이 파라미터로 바꾼다.

    BOOL bMeasureComp;
    BOOL bMeasureBody;
    BOOL bMeasureElect;

    double m_fRoughAlign_Search_ROI_Extension_Ratio;
    double m_fSpecROI_ShiftXPos;
    double m_fSpecROI_ShiftYPos;
    BOOL m_bReCalc_UseSpecROI;
    double m_fValidZRange;
    double m_fVisibilityLowerBound;

    //mc_MED3
    //Outline Align Electrode Param
    long m_nOutline_align_Electrode_Search_Width_Ratio;
    long m_nOutline_align_Electrode_Search_Length_um;
    double m_fOutline_align_Electrode_EdgeThreshold;

    //Outline Align Body Param
    long m_nOutline_align_Body_Search_Width_Ratio;
    long m_nOutline_align_Body_Search_Length_um;
    double m_fOutline_align_Body_EdgeThreshold;

    //Calc. Param
    //Electrode
    long m_nCalc_Electrode_Area_Width_Ratio;
    long m_nCalc_Electrode_Area_Length_Ratio;

    //Body
    long m_nCalc_Body_Area_Width_Ratio;
    long m_nCalc_Body_Area_Length_Ratio;
    long m_nCalc_Body_Area_GapX_um;
    long m_nCalc_Body_Area_GapY_um;

    //Substrate
    long m_nCalc_Substrate_Area_Size_um;
    long m_nCalc_Substrate_Area_GapX_Body_um;
    long m_nCalc_Substrate_Area_GapY_Body_um;
    long m_nCalc_Substrate_Area_GapX_um;
    long m_nCalc_Substrate_Area_GapY_um;
};

struct SComp3DAlgorithmSpec
{
    SComp3DAlgorithmSpec();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    CString strSpecName;

    SComponentAlignSpec CompAlign;
};
// }}

class CSpecLinkPara3D // SDY 3D component 관련 사항을 저장하는 클래스가 2D와 중복됨 -> 이름 변경
{
public:
    CSpecLinkPara3D(void);
    ~CSpecLinkPara3D(void);

    void Apply(const CPackageSpec& spec);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    CString& GetSpecName(long chipType, LPCTSTR chipName);

private:
    std::map<CString, CString> m_infos[5];
};

class Component3DGroupInsp
{
public:
    CString strCompHeightName;
    CString strCompQualName;
    CString strElecHeightName;
    CString strElecTiltName;
    CString strBodyHeightName;
    CString strBodyTiltName;

    CString strCompSpec;

public:
    void SetName(LPCTSTR compSpec);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};

class VisionInspectionComponent3DPara
{
public:
    VisionInspectionComponent3DPara(void);
    ~VisionInspectionComponent3DPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    float m_fCompMissing_Height_Ratio;

    // 알고리즘 SpecDB에서 선택하여 검사, SpecDB에서 선택한 수 만큼 나뉘어서 검사한다.
    std::vector<SComp3DAlgorithmSpec> m_vecComp3DAlgoSpec;
    // 알고리즘 SpecDB를 LinkDatabase에 연결
    CSpecLinkPara3D m_specLink;

    //kk Groupping Comp Spec
    std::vector<Component3DGroupInsp> m_vecstrGroupInspName;

    //spec 관리 변수
    std::vector<VisionInspectionSpec> m_vecVisionInspectionSpecs;

    void SetPassiveInfoDB(const std::vector<sPassive_InfoDB> i_vecPassiveInfoDB);
    std::vector<sPassive_InfoDB> GetPassiveInfoDB();

private:
    std::vector<sPassive_InfoDB> m_vecsTextRecipePassiveInfoDB;
};
