#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "SpecBase.h"
#include "SpecTemplate.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspFrameIndex.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class CPackageSpec;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
enum ePassiveEdgeSearchDefinition
{
    DARK,
    BRIGHT,
    BOTH,
};

class AlgorithmSpec : public SpecBase
{
public:
    AlgorithmSpec();
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    CString strSpecName;
};

// {{ Passve Algo Spec
class PassiveAlignSpec : public SpecBase
{
public:
    PassiveAlignSpec(VisionProcessing& parent);
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    class CapAlignSpec : public SpecBase
    {
    public:
        CapAlignSpec(VisionProcessing& parent);
        virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

        //RoughAlign_Param
        VisionInspFrameIndex m_Rough_align_ImageFrameIndex;
        long m_nRougAlign_SearchOffsetX_um;
        long m_nRougAlign_SearchOffsetY_um;
        BOOL m_bRoughAlign_ElectrodeColor;
        BOOL m_bRoughAlign_OnlyUse;

        //Outline Align Electrode Param
        VisionInspFrameIndex m_Outline_align_Electrode_ImageFrameIndex;
        long m_nOutline_align_Electrode_Search_Width_Ratio;
        long m_nOutline_align_Electrode_Search_Length_um;
        int m_nOutline_align_Electrode_ElectrodeColor;
        int m_nOutline_align_Electrode_EdgeSearch_Direction;
        double m_fOutline_align_Electrode_EdgeThreshold;
        int m_nOutline_align_Electrode_Edge_Detect_Mode;

        //Outline Align Body Param
        VisionInspFrameIndex m_Outline_align_Body_ImageFrameIndex;
        long m_nOutline_align_Body_Search_Width_Ratio;
        long m_nOutline_align_Body_Search_Length_um;
        long m_nOutline_align_Body_Search_Count_Gap;
        int m_nOutline_align_Body_ElectrodeColor;
        int m_nOutline_align_Body_EdgeSearch_Direction;
        double m_fOutline_align_Body_EdgeThreshold;
        int m_nOutline_align_Body_Edge_Detect_Mode;

        //{{Pad
        BOOL m_padUseAlign;
        VisionInspFrameIndex m_padAlignImageFrameIndex;
        float m_fPadSearchROI_Width_Ratio;
        float m_fPadSearchROI_Length_Ratio;
        //}}

        //Deep Learning Param
        BOOL m_bUse2ndInspection;
        CString m_str2ndInspCode;
    };

    CapAlignSpec m_capAlignSpec;
};

class PassiveAlgorithmSpec : public AlgorithmSpec
{
public:
    PassiveAlgorithmSpec(VisionProcessing& parent);
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    PassiveAlignSpec AlignSpec;
};

class Component2DGroupInsp
{
public:
    CString strCompRotateName;
    CString strCompShiftLengthName;
    CString strCompShiftWidthName;
    CString strCompWidthName;
    CString strCompLengthName;

    CString strCompSpec;

public:
    void SetName(LPCTSTR compSpec);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};

//}}

/////////////////////////////////////////////////////////////////////////////////////////
// Result Parameter

class CSpecLinkPara
{
public:
    CSpecLinkPara(void);
    ~CSpecLinkPara(void);

    void Apply(const CPackageSpec& spec);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    CString& GetSpecName(long chipType, LPCTSTR chipName);

private:
    std::map<CString, CString> m_infos[5];
};

class VisionInspectionComponent2DPara
{
public:
    VisionInspectionComponent2DPara(VisionProcessing& parent);
    ~VisionInspectionComponent2DPara(void);

    void Init();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    BOOL m_bAlgoDB;

    // 알고리즘 SpecDB에서 선택하여 검사, SpecDB에서 선택한 수 만큼 나뉘어서 검사한다.
    SpecTemplate<PassiveAlgorithmSpec> m_vecPassiveAlgoSpec;
    CSpecLinkPara m_specLink;

    //kk Groupping Comp Spec
    std::vector<Component2DGroupInsp> m_vecstrGroupInspName;

    //spec 관리 변수
    std::vector<VisionInspectionSpec> m_vecVisionInspectionSpecs;

    void SetPassiveInfoDB(const std::vector<sPassive_InfoDB> i_vecPassiveInfoDB);
    std::vector<sPassive_InfoDB> GetPassiveInfoDB();

private:
    std::vector<sPassive_InfoDB> m_vecsTextRecipePassiveInfoDB;
};
