#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class InspectionAreaInfo;
class VisionScale;

//HDR_6_________________________________ Header body
//
enum eRoughAlignUsingDefine
{
    eRoughAlignUsingDefine_NOT_USE,
    eRoughAlignUsingDefine_H_Map,
    eRoughAlignUsingDefine_V_Map,
};

class CVisionProcessingAlign3DPara
{
public:
    CVisionProcessingAlign3DPara(void);
    ~CVisionProcessingAlign3DPara(void);

    //------------------------------------------------------------------
    // 저장되지 않고 Detail Setup을 위해 존재하는 값들
    //------------------------------------------------------------------
    bool m_skipEdgeAlign;

    //------------------------------------------------------------------

    BOOL LinkDataBase(BOOL bSave, float packageSizeX_um, float packageSizeY_um, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    float m_fPaneExtensionRatio;

    float m_fPatternSizeX;
    float m_fPatternSizeY;
    float m_fMovingErrorX;
    float m_fMovingErrorY;
    float m_fMatchingSize;
    float m_fMatchingReduceSizeX;
    float m_fMatchingPosOffset;

    //k 18.02.23 탑면 Stitch를 위한 para 추가
    long m_nTopStichRoiAmountX; // X축 ROI 갯수 설정
    long m_nTopStichRoiAmountY; // Y축 ROI 갯수 설정
    float m_topStichRoiRadiusX_um; // X축 ROI Radius
    float m_topStichRoiRadiusY_um; // Y축 ROI Radius

    eRoughAlignUsingDefine m_eRoughAlignUsingDefine; // 자동으로 Pane Offset X 찾기 기능 포함 및 Align image 선택
};
