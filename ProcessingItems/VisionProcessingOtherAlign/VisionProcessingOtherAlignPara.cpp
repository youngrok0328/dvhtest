//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingOtherAlignPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/iDataType.h"
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
CVisionProcessingOtherAlignPara::CVisionProcessingOtherAlignPara(VisionProcessing& parent)
    : m_ImageProcMangePara(parent)
{
    Init();
}

CVisionProcessingOtherAlignPara::~CVisionProcessingOtherAlignPara(void)
{
}

BOOL CVisionProcessingOtherAlignPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    CString strTemp;
    long nSize(0);
    long version = 1;

    if (!bSave)
    {
        Init();
    }

    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    // Blob Parameter
    CString strModuleName = _T("Rect Object Align");

    for (long n = 0; n < 4; n++)
    {
        strTemp.Format(_T("m_nEdgeDirection_%d"), n);
        if (!db[strTemp].Link(bSave, m_nEdgeDirection[n]))
            m_nEdgeDirection[n] = 0;

        strTemp.Format(_T("m_nSearchDirection_%d"), n);
        if (!db[strTemp].Link(bSave, m_nSearchDirection[n]))
            m_nSearchDirection[n] = 0;

        strTemp.Format(_T("m_nEdgeDetectMode_%d"), n);
        if (!db[strTemp].Link(bSave, m_nEdgeDetectMode[n]))
            m_nEdgeDetectMode[n] = 0;

        strTemp.Format(_T("m_nFirstEdgeValue_%d"), n);
        if (!db[strTemp].Link(bSave, m_nFirstEdgeValue[n]))
            m_nFirstEdgeValue[n] = 10;

        strTemp.Format(_T("m_nEdgeSearchLength_%d"), n);
        if (!db[strTemp].Link(bSave, m_nEdgeSearchLength[n]))
            m_nEdgeSearchLength[n] = 300;

        //{{//kircheis_USI_OA
        strTemp.Format(_T("m_arbUseEdgeAlignResult%d"), n);
        if (!db[strTemp].Link(bSave, m_arbUseEdgeAlignResult[n]))
            m_arbUseEdgeAlignResult[n] = FALSE;
        //}}
    }

    if (!db[_T("m_nSearchROICount")].Link(bSave, m_nSearchROICount))
        m_nSearchROICount = 1;
    m_nSearchROICount = 1; //kircheis_USI_OA 일단 무조건 한개만 하는 걸로....

    // m_vecrtLeftEdgeDetectROI 저장
    if (!bSave)
    {
        if (!db[_T("Edge_Detect_ROI_Size")].Link(bSave, nSize))
            nSize = 0;
        m_vecrtSearchROI_BCU.resize(nSize);
    }
    else
    {
        nSize = (long)(m_vecrtSearchROI_BCU.size());
        if (!db[_T("Edge_Detect_ROI_Size")].Link(bSave, nSize))
            nSize = 0;
    }

    if (version == 0)
    {
        if (bSave == FALSE)
        {
            m_bIsValidateRecipe = false; // Recipe가 유효하지 않음
            for (long i = 0; i < nSize; i++)
            {
                m_vecrtSearchROI_BCU[i] = Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f);
            }
            m_fptBodyCenterOffset_um.m_x = -100000.f;
            m_fptBodyCenterOffset_um.m_y = -100000.f;
        }
        else // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
            return FALSE;
    }
    else
    {
        for (long i = 0; i < nSize; i++)
        {
            strTemp.Format(_T("Other_Detect_ROI_%d"), i);
            if (!LinkEx(bSave, db[strTemp], m_vecrtSearchROI_BCU[i]))
            {
                m_vecrtSearchROI_BCU[i] = Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f);
            }
        }

        //{{//kircheis_USI_OA
        if (!db[_T("m_fptBodyCenterOffset.m_x")].Link(bSave, m_fptBodyCenterOffset_um.m_x))
            m_fptBodyCenterOffset_um.m_x = -100000.f;
        if (!db[_T("m_fptBodyCenterOffset.m_y")].Link(bSave, m_fptBodyCenterOffset_um.m_y))
            m_fptBodyCenterOffset_um.m_y = -100000.f;
        //}}
    }

    m_ImageProcMangePara.LinkDataBase(bSave, db[_T("{E6ABC182-09CF-49DE-965B-5927B1B8C9E1}")]);

    strTemp.Empty();
    strModuleName.Empty();

    return TRUE;
}

std::vector<CString> CVisionProcessingOtherAlignPara::ExportAlgoParaToText(
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

    //Image Combine
    std::vector<CString> vecstrImageCombineParameters(0);
    vecstrImageCombineParameters = m_ImageProcMangePara.ExportImageCombineParaToText(
        strVisionName, strInspectionModuleName, _T("Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //Module 특성 변수
    CString strDirection{};
    CString arStrDirectionName[] = {_T("LEFT"), _T("TOP"), _T("RIGHT"), _T("BOTTOM")};

    CString arStrDirectionSimpleName[] = {_T(" (L)"), _T(" (T)"), _T(" (R)"), _T(" (B)")};

    //Algorithm Parameter 정리
    strCategory.Format(_T("Algorithm Parameter"));
    strGroup.Format(_T("Edge Detect Parameter"));

    for (long nDir = 0; nDir < 4; nDir++) //L:0, T:1, R:2, B:3
    {
        strParaName = arStrDirectionName[nDir];

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Use Body Align Result"), (bool)m_arbUseEdgeAlignResult[nDir]));

        strParaNameAux.Format(_T("Search Direction%s"), (LPCTSTR)arStrDirectionSimpleName[nDir]);
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nSearchDirection[nDir]));

        strParaNameAux.Format(_T("Edge Direction%s"), (LPCTSTR)arStrDirectionSimpleName[nDir]);
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nEdgeDirection[nDir]));

        strParaNameAux.Format(_T("Edge Detect Mode%s"), (LPCTSTR)arStrDirectionSimpleName[nDir]);
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nEdgeDetectMode[nDir]));

        strParaNameAux.Format(_T("First Edge Value%s"), (LPCTSTR)arStrDirectionSimpleName[nDir]);
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nFirstEdgeValue[nDir]));

        strParaNameAux.Format(_T("Edge Search Length%s"), (LPCTSTR)arStrDirectionSimpleName[nDir]);
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nEdgeSearchLength[nDir]));
    }

    long nROI_Num = (long)m_vecrtSearchROI_BCU.size();
    if (nROI_Num > 0)
    {
        std::vector<CString> vecStrROI = CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Object Detect ROI 0"), m_vecrtSearchROI_BCU[0]);
        vecstrAlgorithmParameters.insert(vecstrAlgorithmParameters.end(), vecStrROI.begin(), vecStrROI.end());
    }

    return vecstrAlgorithmParameters;
}

void CVisionProcessingOtherAlignPara::Init()
{
    for (long n = 0; n < 4; n++)
    {
        m_nEdgeDirection[n] = PI_ED_DIR_BOTH;
        m_nSearchDirection[n] = PI_ED_DIR_OUTER;
        m_nEdgeDetectMode[n] = 0;
        m_nFirstEdgeValue[n] = 10;
        m_nEdgeSearchLength[n] = 300;
        m_arbUseEdgeAlignResult[n] = FALSE; //kircheis_USI_OA
    }

    m_nSelectDirection = 0;
    m_nSearchROICount = 1;

    m_fptBodyCenterOffset_um = Ipvm::Point32r2(-100000.f, -100000.f); //kircheis_USI_OA

    m_vecrtSearchROI_BCU.clear();
    m_ImageProcMangePara.Init();
    m_bIsValidateRecipe = true;
}
