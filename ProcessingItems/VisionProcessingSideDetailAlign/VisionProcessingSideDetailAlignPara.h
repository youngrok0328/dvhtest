#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
enum enumSubstrateInnerSel
{
    USE_GLASS_PARAM = 0,
    USE_SUBSTRATE_PARAM,
};

enum enumEdgeNoiseFilter
{
    ENF_NOT_USE = 0,
    ENF_USE_GENERAL_FILTER,
    ENF_USE_RANSAC_FILTER, //kircheis : 개인적으로 RANSAC이 싫지만, Noise제거에는 FitToLineRN보다 낫다. 단, 이는 Noise Point가 정상 Point 대비 일부에 불과할 경우이며, Noise Point가 전체의 절반 근처 수준을 넘어설 경우엔 쓰레기가 된다.  https://gnaseel.tistory.com/m/33
};

class SideAlignParameters
{
public:
    SideAlignParameters(VisionProcessing& parent);
    ~SideAlignParameters(void);

    VisionInspFrameIndex m_alignFrameIndex;
    ImageProcPara m_alignImageProcManagePara; //Btn
    long m_nSearchDirection;
    long m_nEdgeDirection;
    long m_nEdgeDetectMode;
    double m_dEdgeThreshold;
    long
        m_nSubstrateInnerParamType; //Substrate 전용 Parameter. Glass와의 경계면에 사용할 Paramter를 Substrate걸 사용할 건지, Glass걸 사용할 건지 선택.
    BOOL m_bUseRoughAlign; //Substrate가 있는 경우 Galss에만 사용
    long m_nRoughEdgeSearchWidthTB_um; //Substrate가 있는 경우 Galss에만 사용
    long m_nRoughEdgeSearchLengthTB_um; //Substrate가 있는 경우 Galss에만 사용
    long m_nEdgeSearchWidthTB_um;
    long m_nEdgeSearchLengthTB_um;
    long m_nEdgeSearchWidthLR_Ratio;
    long m_nEdgeSearchLengthLR_um;

    //Glass 전용
    VisionInspFrameIndex m_alignFrameIndexGlassLR;
    long m_nSearchDirectionGlassLR;
    long m_nEdgeDirectionGlassLR;

    void Init(void);
};

class VisionProcessingSideDetailAlignPara
{
public:
    VisionProcessingSideDetailAlignPara(VisionProcessing& parent);
    ~VisionProcessingSideDetailAlignPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void Init();

    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    //Common Parameters
    long m_nEdgeCount;
    long m_nEdgePointNoiseFilter;

    //Substrate Parameters
    SideAlignParameters m_substrateAlignParam;

    //Glass Parameters
    SideAlignParameters m_glassAlignParam;
    BOOL m_bUseGlassCompensationLeftRight; //오직 Glass 전용 옵션이라 SideAlignParameters에 안 넣는다.
};