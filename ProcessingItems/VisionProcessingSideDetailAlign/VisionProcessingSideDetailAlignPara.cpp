//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingSideDetailAlignPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SideAlignParameters::SideAlignParameters(VisionProcessing& parent)
    : m_alignImageProcManagePara(parent)
    , m_alignFrameIndex(parent, TRUE)
    , m_alignFrameIndexGlassLR(parent, TRUE)
{
    Init();
}

SideAlignParameters::~SideAlignParameters()
{
}

void SideAlignParameters::Init()
{
    m_alignImageProcManagePara.Init();
    m_alignFrameIndex.setFrameIndex(0);

    m_nSearchDirection = PI_ED_DIR_INNER;
    m_nEdgeDirection = PI_ED_DIR_RISING;
    m_nEdgeDetectMode = PI_ED_MAX_EDGE;
    m_dEdgeThreshold = 3.;
    m_nSubstrateInnerParamType = USE_GLASS_PARAM;
    m_bUseRoughAlign = FALSE;
    m_nRoughEdgeSearchWidthTB_um = 500;
    m_nRoughEdgeSearchLengthTB_um = 100;
    m_nEdgeSearchWidthTB_um = 200;
    m_nEdgeSearchLengthTB_um = 50;
    m_nEdgeSearchWidthLR_Ratio = 50;
    m_nEdgeSearchLengthLR_um = 100;

    m_alignFrameIndexGlassLR.setFrameIndex(0);
    m_nSearchDirectionGlassLR = PI_ED_DIR_INNER;
    m_nEdgeDirectionGlassLR = PI_ED_DIR_RISING;
}
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

VisionProcessingSideDetailAlignPara::VisionProcessingSideDetailAlignPara(VisionProcessing& parent)
    : m_substrateAlignParam(parent)
    , m_glassAlignParam(parent)
{
    Init();
}

VisionProcessingSideDetailAlignPara::~VisionProcessingSideDetailAlignPara(void)
{
}

BOOL VisionProcessingSideDetailAlignPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    //Common
    if (!db[_T("{2DA791B3-6FD4-4E1D-B68F-40A53DDCF9FF}")].Link(bSave, m_nEdgeCount))
        m_nEdgeCount = 7;
    if (!db[_T("{FB53F055-1232-4AFC-88F9-E8C8FE6723E0}")].Link(bSave, m_nEdgePointNoiseFilter))
        m_nEdgePointNoiseFilter = ENF_USE_RANSAC_FILTER;

    //Substrate
    m_substrateAlignParam.m_alignImageProcManagePara.LinkDataBase(
        bSave, db[_T("{92D26DDC-091E-409F-B580-F9465AD917FA}")]);
    m_substrateAlignParam.m_alignFrameIndex.LinkDataBase(bSave, db[_T("{7B28D2DA-D2EB-4895-8E20-9A81C4FC5621}")]);
    if (!db[_T("{3EC6F60D-5CFE-420E-A5C4-665D2EB1EA44}")].Link(bSave, m_substrateAlignParam.m_nSearchDirection))
        m_substrateAlignParam.m_nSearchDirection = PI_ED_DIR_INNER;
    if (!db[_T("{6B42DF1B-83C9-4CA5-8ADB-DE8FA5764DC1}")].Link(bSave, m_substrateAlignParam.m_nEdgeDirection))
        m_substrateAlignParam.m_nEdgeDirection = PI_ED_DIR_RISING;
    if (!db[_T("{69E3EFFD-DE0A-4442-AD5C-0DE3A3E9E109}")].Link(bSave, m_substrateAlignParam.m_nEdgeDetectMode))
        m_substrateAlignParam.m_nEdgeDetectMode = PI_ED_MAX_EDGE;
    if (!db[_T("{43EF5E7F-C5F1-4B5A-BB98-563A37AE94C0}")].Link(bSave, m_substrateAlignParam.m_dEdgeThreshold))
        m_substrateAlignParam.m_dEdgeThreshold = 3.;
    if (!db[_T("{1218C39B-98C8-4A1F-A22C-70BD0CD6A653}")].Link(bSave, m_substrateAlignParam.m_nSubstrateInnerParamType))
        m_substrateAlignParam.m_nSubstrateInnerParamType = USE_GLASS_PARAM;
    if (!db[_T("{325276D6-9488-42AA-A424-7AF4AAED1840}")].Link(bSave, m_substrateAlignParam.m_bUseRoughAlign))
        m_substrateAlignParam.m_bUseRoughAlign = FALSE;
    if (!db[_T("{08FA812E-49B6-4CCE-BDB8-876EB40B2EA9}")].Link(
            bSave, m_substrateAlignParam.m_nRoughEdgeSearchWidthTB_um))
        m_substrateAlignParam.m_nRoughEdgeSearchWidthTB_um = 500;
    if (!db[_T("{0F0FF525-0894-497C-A877-3498D1EC7EC5}")].Link(
            bSave, m_substrateAlignParam.m_nRoughEdgeSearchLengthTB_um))
        m_substrateAlignParam.m_nRoughEdgeSearchLengthTB_um = 100;
    if (!db[_T("{3696A5DB-8D4E-4FE8-BC50-C9DA414D55D5}")].Link(bSave, m_substrateAlignParam.m_nEdgeSearchWidthTB_um))
        m_substrateAlignParam.m_nEdgeSearchWidthTB_um = 200;
    if (!db[_T("{21676069-CBD1-44BD-B317-C1154CC9EE51}")].Link(bSave, m_substrateAlignParam.m_nEdgeSearchLengthTB_um))
        m_substrateAlignParam.m_nEdgeSearchLengthTB_um = 50;
    if (!db[_T("{98256B49-976E-4B45-B796-7169FC319755}")].Link(bSave, m_substrateAlignParam.m_nEdgeSearchWidthLR_Ratio))
        m_substrateAlignParam.m_nEdgeSearchWidthLR_Ratio = 50;
    if (!db[_T("{E7621A2F-04C8-489F-ABCA-DCCF9A3952BD}")].Link(bSave, m_substrateAlignParam.m_nEdgeSearchLengthLR_um))
        m_substrateAlignParam.m_nEdgeSearchLengthLR_um = 100;

    m_substrateAlignParam.m_bUseRoughAlign
        = FALSE; //일단 Substrate의 Rough align은 잠근다. 알고리즘에도 외곽은 고려 대상이 아니다.

    //Glass
    if (!db[_T("{B7A80E84-ED43-4291-9846-9587C937486A}")].Link(bSave, m_glassAlignParam.m_nEdgeDetectMode))
        m_glassAlignParam.m_nEdgeDetectMode = PI_ED_MAX_EDGE;
    if (!db[_T("{FC0CB597-7D78-4D77-9780-667408062992}")].Link(bSave, m_glassAlignParam.m_dEdgeThreshold))
        m_glassAlignParam.m_dEdgeThreshold = 3.;
    //m_glassAlignParam.m_alignImageProcManagePara.LinkDataBase(bSave, db[_T("{36F69DD8-D174-4B39-9A0C-822CD15A4F4C}")]);
    m_glassAlignParam.m_alignImageProcManagePara.Init();
    m_glassAlignParam.m_alignFrameIndex.LinkDataBase(bSave, db[_T("{36F63757-3AF9-42CC-9DD0-BC792C7D1DC4}")]);
    if (!db[_T("{483F1801-C63D-443D-902B-E037D34DFD42}")].Link(bSave, m_glassAlignParam.m_nSearchDirection))
        m_glassAlignParam.m_nSearchDirection = PI_ED_DIR_INNER;
    if (!db[_T("{F77CB21C-166E-4478-9465-56AB08588830}")].Link(bSave, m_glassAlignParam.m_nEdgeDirection))
        m_glassAlignParam.m_nEdgeDirection = PI_ED_DIR_RISING;
    if (!db[_T("{3399C107-A534-41F8-8474-8C5D410DC158}")].Link(bSave, m_glassAlignParam.m_bUseRoughAlign))
        m_glassAlignParam.m_bUseRoughAlign = FALSE;
    if (!db[_T("{2C60103B-A812-4C93-8548-3412737F9C12}")].Link(bSave, m_glassAlignParam.m_nRoughEdgeSearchWidthTB_um))
        m_glassAlignParam.m_nRoughEdgeSearchWidthTB_um = 500;
    if (!db[_T("{B6139F7C-F2D4-4B88-AFA3-83724FC5C2E7}")].Link(bSave, m_glassAlignParam.m_nRoughEdgeSearchLengthTB_um))
        m_glassAlignParam.m_nRoughEdgeSearchLengthTB_um = 100;
    if (!db[_T("{6D9FD8E9-53D9-4FBC-8ECF-4FAB05A04034}")].Link(bSave, m_glassAlignParam.m_nEdgeSearchWidthTB_um))
        m_glassAlignParam.m_nEdgeSearchWidthTB_um = 200;
    if (!db[_T("{F2AC0EB1-1009-473D-8E1D-C5F9BE9CFF8E}")].Link(bSave, m_glassAlignParam.m_nEdgeSearchLengthTB_um))
        m_glassAlignParam.m_nEdgeSearchLengthTB_um = 50;

    m_glassAlignParam.m_alignFrameIndexGlassLR.LinkDataBase(bSave, db[_T("{AC307592-FA51-4116-91BA-521FA212BE59}")]);
    if (!db[_T("{42201877-DB55-4B0A-B8F2-118C7B468568}")].Link(bSave, m_glassAlignParam.m_nSearchDirectionGlassLR))
        m_glassAlignParam.m_nSearchDirectionGlassLR = PI_ED_DIR_INNER;
    if (!db[_T("{7E9553D7-0FEE-48DA-87F0-0DB2CC1475BA}")].Link(bSave, m_glassAlignParam.m_nEdgeDirectionGlassLR))
        m_glassAlignParam.m_nEdgeDirectionGlassLR = PI_ED_DIR_RISING;
    if (!db[_T("{5B7E324C-AACD-49AC-BEFD-7EBBAA2819FC}")].Link(bSave, m_glassAlignParam.m_nEdgeSearchWidthLR_Ratio))
        m_glassAlignParam.m_nEdgeSearchWidthLR_Ratio = 50;
    if (!db[_T("{576DE242-B120-4A1C-81C6-F25B3F3DBE44}")].Link(bSave, m_glassAlignParam.m_nEdgeSearchLengthLR_um))
        m_glassAlignParam.m_nEdgeSearchLengthLR_um = 100;

    if (!db[_T("{965E41B6-E448-43B6-9B52-C2589F7B6336}")].Link(bSave, m_bUseGlassCompensationLeftRight))
        m_bUseGlassCompensationLeftRight = FALSE;

    return TRUE;
}

void VisionProcessingSideDetailAlignPara::Init()
{
    // 파라미터 초기화
    m_nEdgeCount = 7;
    m_nEdgePointNoiseFilter = ENF_USE_RANSAC_FILTER;
    m_bUseGlassCompensationLeftRight = FALSE;

    m_substrateAlignParam.Init();
    m_glassAlignParam.Init();
}

std::vector<CString> VisionProcessingSideDetailAlignPara::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
{
    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strInspectionModuleName);

    //개별 Parameter용 변수
    CString strCategory;
    CString strGroup;
    CString strParaName;
    CString strParaNameAux;
    CString strValue;

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    //Module 특성 변수
    //Algorithm Parameters
    strCategory.Format(_T("Algorithm Parameters"));

    //Common Parameters
    strGroup.Format(_T("Common parameters"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge Count"), _T(""), m_nEdgeCount));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge point noise filter type"), _T(""), m_nEdgePointNoiseFilter));

    //Substrate Align Parameters
    strGroup.Format(_T("Substrate Align parameters"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Search Direction"), _T(""), m_substrateAlignParam.m_nSearchDirection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge Direction"), _T(""), m_substrateAlignParam.m_nEdgeDirection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge Detection Mode"), _T(""), m_substrateAlignParam.m_nEdgeDetectMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge Threshold"), _T(""), m_substrateAlignParam.m_dEdgeThreshold));
    //vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Use Rough Align"), _T(""), m_substrateAlignParam.m_bUseRoughAlign));
    //vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Rough Edge Search Width TB(um)"), _T(""), m_substrateAlignParam.m_nRoughEdgeSearchWidthTB_um));
    //vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Rough Edge Search Length TB(um)"), _T(""), m_substrateAlignParam.m_nRoughEdgeSearchLengthTB_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Edge Search Width TB(um)"), _T(""), m_substrateAlignParam.m_nEdgeSearchWidthTB_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Edge Search Length TB(um)"), _T(""), m_substrateAlignParam.m_nEdgeSearchLengthTB_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Edge Search Width LR(%)"), _T(""), m_substrateAlignParam.m_nEdgeSearchWidthLR_Ratio));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Edge Search Length LR(um)"), _T(""), m_substrateAlignParam.m_nEdgeSearchLengthLR_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Inner Align Parameter"), _T(""), m_substrateAlignParam.m_nSubstrateInnerParamType));

    //Glass Align Parameters
    strGroup.Format(_T("Glass Align parameters"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge Detection Mode"), _T(""), m_glassAlignParam.m_nEdgeDetectMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge Threshold"), _T(""), m_glassAlignParam.m_dEdgeThreshold));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("[T/B] Search Direction"), _T(""), m_glassAlignParam.m_nSearchDirection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("[T/B] Edge Direction"), _T(""), m_glassAlignParam.m_nEdgeDirection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("[T/B] Use Rough Align"), _T(""), m_glassAlignParam.m_bUseRoughAlign));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("[T/B] Rough Edge Search Width (um)"), _T(""), m_glassAlignParam.m_nRoughEdgeSearchWidthTB_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("[T/B] Rough Edge Search Length (um)"), _T(""), m_glassAlignParam.m_nRoughEdgeSearchLengthTB_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("[T/B] Edge Search Width (um)"), _T(""), m_glassAlignParam.m_nEdgeSearchWidthTB_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("[T/B] Edge Search Length (um)"), _T(""), m_glassAlignParam.m_nEdgeSearchLengthTB_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("[L/R] Search Direction"), _T(""), m_glassAlignParam.m_nSearchDirectionGlassLR));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("[L/R] Edge Direction"), _T(""), m_glassAlignParam.m_nEdgeDirectionGlassLR));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("[L/R] Edge Search Width LR(%)"), _T(""), m_glassAlignParam.m_nEdgeSearchWidthLR_Ratio));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("[L/R] Edge Search Length LR(um)"), _T(""), m_glassAlignParam.m_nEdgeSearchLengthLR_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("[L/R] Use Compensation"), _T(""), m_bUseGlassCompensationLeftRight));

    //Image Combine
    std::vector<CString> vecstrImageCombineParameters(0);

    vecstrImageCombineParameters = m_substrateAlignParam.m_alignImageProcManagePara.ExportImageCombineParaToText(
        strVisionName, strInspectionModuleName, _T("SubstrateAlign-Image Combine"));
    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //vecstrImageCombineParameters.clear();
    //vecstrImageCombineParameters = m_glassAlignParam.m_alignImageProcManagePara.ExportImageCombineParaToText(strVisionName, strInspectionModuleName, _T("GlassAlign-Image Combine"));
    //vecstrAlgorithmParameters.insert(vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    return vecstrAlgorithmParameters;
}