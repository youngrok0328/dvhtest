//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "StitchPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
StitchPara::StitchPara(VisionProcessing& parent)
    : m_type(enumAlgorithmType::Matching)
    , m_boundaryInterpolation(1)
    , m_saveTemplateDebugImage(0)
    , m_calcFrameIndex(parent, TRUE)
    , m_stitchImageProcManagePara(parent) // Stitch Section Image Combine
    , m_skipEdgeAlign(false)
    , m_fMoveDistanceXbetweenFOV(0.f)
    , m_fMoveDistanceYbetweenFOV(0.f)
{
    Init();
}

StitchPara::~StitchPara(void)
{
}

BOOL StitchPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        Init();
    }

    long nVersion = 3;
    if (!db[_T("Version")].Link(bSave, nVersion))
        nVersion = 0;

    if (nVersion < 3)
    {
        m_type = enumAlgorithmType::Matching;

        m_paraBasedOnMatching.LinkDataBase(bSave, nVersion, db);
        m_paraBasedOnBodySize.Init();
    }
    else
    {
        db[_T("Based On Matching")].Link(bSave, (int&)m_type);
        m_paraBasedOnMatching.LinkDataBase(bSave, nVersion, db[_T("Based On Matching")]);
        m_paraBasedOnBodySize.LinkDataBase(bSave, nVersion, db[_T("Based On Body Size")]);
    }

    m_stitchImageProcManagePara.LinkDataBase(
        bSave, db[_T("{CA5B5E4D-3204-4092-8F95-2582EC9A232C}")]); // Stitch Section Image Combine

    if (!m_calcFrameIndex.LinkDataBase(bSave, db[_T("MatchingFrameID")]))
        m_calcFrameIndex.setFrameIndex(0);
    if (!db[_T("bTemplateBoundaryInterpolation")].Link(bSave, m_boundaryInterpolation))
        m_boundaryInterpolation = 1;

    return TRUE;
}

void StitchPara::Init()
{
    m_calcFrameIndex.setFrameIndex(0);
    m_stitchImageProcManagePara.Init(); // Stitch Section Image Combine

    m_type = enumAlgorithmType::Matching;
    m_boundaryInterpolation = 1;
    m_saveTemplateDebugImage = 0;

    m_paraBasedOnMatching.Init();
    m_paraBasedOnBodySize.Init();
}

std::vector<CString> StitchPara::ExportAlgoParaToText(
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
    CString strDirection{};

    CString arStrDirectionName[] = {_T("LEFT"), _T("TOP"), _T("RIGHT"), _T("BOTTOM")};

    //Algorithm Parameter 정리

    /********************************************************************* Stitch **********************************************************************/
    strCategory.Format(_T("Stitch"));

    // Common parameter
    strGroup.Format(_T("Common Parameter"));

    strParaName.Format(_T("Select Frame ID"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_calcFrameIndex.getFrameIndex()));
    strParaName.Format(_T("Algorithm"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_type));
    strParaName.Format(_T("Move distance X between FOVs (mm)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fMoveDistanceXbetweenFOV));
    strParaName.Format(_T("Move distance Y between FOVs (mm)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fMoveDistanceYbetweenFOV));
    strParaName.Format(_T("Boundary Interpolation"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_boundaryInterpolation));
    strParaName.Format(_T("Save Template Debug Image"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_saveTemplateDebugImage));

    // Run
    strGroup.Format(_T("Run"));

    // Alogrithm Parameter (Matching)
    strGroup.Format(_T("Algorithm Parameter"));

    strParaName.Format(_T("Template Search Offset (um)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, strParaNameAux, m_paraBasedOnMatching.m_templateSearchOffset_um));
    strParaName.Format(_T("Template ROI Count"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_paraBasedOnMatching.m_nTemplateROICount));

    // Algorithm parameter (BodySize)
    strGroup.Format(_T("Algorithm Parameter"));

    strParaName.Format(_T("Edge Direction"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_paraBasedOnBodySize.m_edgeDirection));
    strParaName.Format(_T("Search Direction"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_paraBasedOnBodySize.m_searchDirection));
    strParaName.Format(_T("Edge Detect Mode"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_paraBasedOnBodySize.m_edgeDetectMode));
    strParaName.Format(_T("Edge Num"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_paraBasedOnBodySize.m_edgeNum));
    strParaName.Format(_T("First Edge Min Threshold"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_paraBasedOnBodySize.m_firstEdgeMinThreshold));

    // Stitch ROI
    strGroup.Format(_T("ROI"));

    std::vector<CString> vecStrRoiInfoBuffer(0);
    CString strBodySizeStitchInfoBuffer = _T("");

    long nStitchROISize = (long)m_paraBasedOnMatching.m_vecStitchROI.size();

    for (long nIdx = 0; nIdx < nStitchROISize; nIdx++)
    {
        if (m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtHorRef_BCU[0].Height() > 0.f
            && m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtHorRef_BCU[0].Width() > 0.f)
        {
            strParaName.Format(_T("Stitch%d_HorROI1"), nIdx + 1);
            //m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtHorRef_BCU[0] 저장
            vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
                strParaName, m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtHorRef_BCU[0]);
            vecstrAlgorithmParameters.insert(
                vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());

            strParaName.Format(_T("Stitch%d_HorROI2"), nIdx + 1);
            //m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtHorRef_BCU[1] 저장
            vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
                strParaName, m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtHorRef_BCU[1]);
            vecstrAlgorithmParameters.insert(
                vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());
        }
        else
        {
            strParaName.Format(_T("Stitch%d_VerROI1"), nIdx + 1);
            //m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtVerRef_BCU[0] 저장
            vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
                strParaName, m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtVerRef_BCU[0]);
            vecstrAlgorithmParameters.insert(
                vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());

            strParaName.Format(_T("Stitch%d_VerROI2"), nIdx + 1);
            //m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtVerRef_BCU[1] 저장
            vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
                strParaName, m_paraBasedOnMatching.m_vecStitchROI[nIdx].m_rtVerRef_BCU[1]);
            vecstrAlgorithmParameters.insert(
                vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());
        }
    }

    //Bodysize 추후 추가
    long nBasedOnBodySizeLength = CAST_LONG(m_paraBasedOnBodySize.m_stitchPara.size());

    for (long nIndex = 0; nIndex < nBasedOnBodySizeLength; nIndex++)
    {
        //	Based on body size Stitching의 Vertical, Horizonal ROI 표기 순서는 Vertical -> Horizontal 순서로 되어 있음, 위의 Matching과는 반대로 표기하였음 - JHB_2022.10.20
        strParaName.Format(_T("Stitch%d_VerROI"), nIndex + 1);

        strParaNameAux.Format(_T("Search Length(um)"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_paraBasedOnBodySize.m_stitchPara[nIndex].m_searchLength_um[0]));

        strParaNameAux.Format(_T("Start(%%)"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_paraBasedOnBodySize.m_stitchPara[nIndex].m_persent_start[0]));

        strParaNameAux.Format(_T("End(%%)"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_paraBasedOnBodySize.m_stitchPara[nIndex].m_persent_end[0]));

        strParaName.Format(_T("Stitch%d_HorROI"), nIndex + 1);

        strParaNameAux.Format(_T("Search Length(um)"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_paraBasedOnBodySize.m_stitchPara[nIndex].m_searchLength_um[1]));

        strParaNameAux.Format(_T("Start(%%)"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_paraBasedOnBodySize.m_stitchPara[nIndex].m_persent_start[1]));

        strParaNameAux.Format(_T("End(%%)"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_paraBasedOnBodySize.m_stitchPara[nIndex].m_persent_end[1]));
    }
    /*for (long y = 0; y < m_nStitchCountY; y++)
	{
		for (long x = 0; x < m_nStitchCountX; x++)
		{
			long stitchIndex = y * m_nStitchCountX + x;
			for (long i = 0; i < 2; i++)
			{
				strParaName.Format(_T("S%d_H%d"), stitchIndex + 1, i + 1);
				vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, strParaName, m_paraBasedOnMatching.m_vecStitchROI[stitchIndex].m_rtHorRef_BCU[i]);
				vecstrAlgorithmParameters.insert(vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());

				strParaName.Format(_T("S%d_V%d"), stitchIndex + 1, i + 1);
				vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, strParaName, m_paraBasedOnMatching.m_vecStitchROI[stitchIndex].m_rtVerRef_BCU[i]);
				vecstrAlgorithmParameters.insert(vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());
			}
		}
	}*/
    /***************************************************************************************************************************************************/
    return vecstrAlgorithmParameters;
}