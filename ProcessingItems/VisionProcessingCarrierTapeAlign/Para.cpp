//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Para.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Para::Para(VisionProcessing& parent)
    : m_carrierTapeAlign(parent)
    , m_pocketAlign(parent)
{
    Init();
}

BOOL Para::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        Init();
    }

    m_carrierTapeAlign.LinkDataBase(bSave, db[_T("Carrier Tape")]);
    m_pocketAlign.LinkDataBase(bSave, db[_T("Pocket")]);

    return TRUE;
}

void Para::Init()
{
    m_carrierTapeAlign.Init();
    m_pocketAlign.Init();
}

std::vector<CString> Para::ExportAlgoParaToText(
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

    //Module 특성 변수 //채워야 함
    CString strDirection;

    //Algorithm Parameter 정리

    std::vector<CString> vecStrRoiInfoBuffer(0);

    /********************************************************************* Edge Align **********************************************************************/
    strCategory.Format(_T("Edge Align"));

    if (auto* para = &m_carrierTapeAlign)
    {
        //Image Combine
        strGroup.Format(_T("Combine Parameter"));
        std::vector<CString> vecstrImageCombineParameters(0);
        vecstrImageCombineParameters = para->m_imageProc.ExportImageCombineParaToText(
            strVisionName, strInspectionModuleName, _T("Carrier Tape Edge Align-Image Combine"));
        vecstrAlgorithmParameters.insert(
            vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

        strParaName.Format(_T("Carrier Tape Align Image Frame Index"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_imageFrameIndex.getFrameIndex()));

        // Edge Detect Parameter
        strGroup.Format(_T("Carrier Tape Edge Detect Parameter"));
        strParaName.Format(_T("Edge Direction"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_edgeType));
        strParaName.Format(_T("Search Direction"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_searchDirection));
        strParaName.Format(_T("Edge Detect Mode"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_edgeDetectMode));
        strParaName.Format(_T("FirstEdge Min Value"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_firstEdgeMinThreshold));
        strParaName.Format(_T("Detected EdgeAlign Gap(um)"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_edgeAlignGap_um));

        // ROI
        strGroup.Format(_T("ROI"));
        for (int32_t nDirection = 0; nDirection < 2; nDirection++)
        {
            const auto& edgeSearchRoi = para->m_edgeSearchRois[nDirection];
            strParaName = GetDirectionName(false, nDirection);
            strParaNameAux.Format(_T("S.I. (um)"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, edgeSearchRoi.m_searchLength_um));
            strParaNameAux.Format(_T("Start (%%)"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, edgeSearchRoi.m_persent_start));
            strParaNameAux.Format(_T("End (%%)"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, edgeSearchRoi.m_persent_end));
        }
    }

    if (auto* para = &m_pocketAlign)
    {
        //Image Combine
        strGroup.Format(_T("Combine Parameter"));
        std::vector<CString> vecstrImageCombineParameters(0);
        vecstrImageCombineParameters = para->m_imageProc.ExportImageCombineParaToText(
            strVisionName, strInspectionModuleName, _T("Pocket Edge Align-Image Combine"));
        vecstrAlgorithmParameters.insert(
            vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

        strParaName.Format(_T("Pocket Align Image Frame Index"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_imageFrameIndex.getFrameIndex()));

        // Edge Detect Parameter
        strGroup.Format(_T("Pocket Edge Detect Parameter"));
        strParaName.Format(_T("Edge Direction"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_edgeType));
        strParaName.Format(_T("Search Direction"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_searchDirection));
        strParaName.Format(_T("Edge Detect Mode"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_edgeDetectMode));
        strParaName.Format(_T("FirstEdge Min Value"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_firstEdgeMinThreshold));
        strParaName.Format(_T("Detected EdgeAlign Gap(um)"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, para->m_edgeParams.m_edgeAlignGap_um));

        // ROI
        strGroup.Format(_T("ROI"));
        for (int32_t nDirection = 0; nDirection < 4; nDirection++)
        {
            const auto& edgeSearchRoi = para->m_edgeSearchRois[nDirection];
            strParaName = GetDirectionName(true, nDirection);
            strParaNameAux.Format(_T("S.I. (um)"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, edgeSearchRoi.m_searchLength_um));
            strParaNameAux.Format(_T("Start (%%)"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, edgeSearchRoi.m_persent_start));
            strParaNameAux.Format(_T("End (%%)"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, edgeSearchRoi.m_persent_end));
        }
    }

    /***************************************************************************************************************************************************/
    return vecstrAlgorithmParameters;
}