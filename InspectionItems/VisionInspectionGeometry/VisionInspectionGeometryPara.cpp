//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionGeometryPara.h"

//CPP_2_________________________________ This project's headers
#include "GeometryDefines.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionGeometryPara::VisionInspectionGeometryPara(VisionProcessing& parent)
    : m_ImageProcMangePara(parent)
    , m_nGeometryInspType(eGeometryInspType ::GeometryInspType_Both)
{
    Init();
}

VisionInspectionGeometryPara::~VisionInspectionGeometryPara(void)
{
}

BOOL VisionInspectionGeometryPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    CString strTemp;

    if (!bSave)
    {
        Init();
    }

    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    // Inspection Count
    if (!db[_T("Inspection_Number")].Link(bSave, m_nInspectionNumber))
        m_nInspectionNumber = 1;

    // Inspection Type
    if (!db[_T("Inspection_Type")].Link(bSave, m_nGeometryInspType))
        m_nInspectionNumber = eGeometryInspType ::GeometryInspType_Both;

    m_vecstrInspectionName.resize(m_nInspectionNumber);
    m_vecnInspectionType.resize(m_nInspectionNumber);
    m_vecnInspectionDistanceResult.resize(m_nInspectionNumber);
    m_vecnInspectionCircleResult.resize(m_nInspectionNumber);
    m_vecfReferenceSpec.resize(m_nInspectionNumber);

    // Inspection Spec
    for (long i = 0; i < m_nInspectionNumber; i++)
    {
        strTemp.Format(_T("ReferenceSpec_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecfReferenceSpec[i]))
            m_vecfReferenceSpec[i] = 0.f;
    }

    // Inspection Name
    for (long i = 0; i < m_nInspectionNumber; i++)
    {
        strTemp.Format(_T("Inspection_Name_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecstrInspectionName[i]))
            m_vecstrInspectionName[i] = _T("Untitle Name");
    }

    // Inspection Type
    for (long i = 0; i < m_nInspectionNumber; i++)
    {
        strTemp.Format(_T("Inspection_Type_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecnInspectionType[i]))
            m_vecnInspectionType[i] = Insp_Type_Shape_Distance;
    }

    // Inspection Type
    for (long i = 0; i < m_nInspectionNumber; i++)
    {
        strTemp.Format(_T("InspectionDistanceResult_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecnInspectionDistanceResult[i]))
            m_vecnInspectionDistanceResult[i] = Insp_Param_Dist_XY;
    }

    // Inspection Type
    for (long i = 0; i < m_nInspectionNumber; i++)
    {
        strTemp.Format(_T("InspectionCicleResult_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecnInspectionCircleResult[i]))
            m_vecnInspectionCircleResult[i] = Insp_Param_Circle_Radius;
    }

    // InfoParameter
    SaveVec3_Long(bSave, _T("Geo_Info_Count_Ref_0"), db, m_vecInfoParameter_0_Ref);
    SaveVec3_Long(bSave, _T("Geo_Info_Count_Tar_0"), db, m_vecInfoParameter_0_Tar);

    SaveVec3_Long(bSave, _T("Geo_Info_Count_Ref_1"), db, m_vecInfoParameter_1_Ref);
    SaveVec3_Long(bSave, _T("Geo_Info_Count_Tar_1"), db, m_vecInfoParameter_1_Tar);

    SaveVec3_Long(bSave, _T("Geo_Info_Count_Ref_2"), db, m_vecInfoParameter_2_Ref);
    SaveVec3_Long(bSave, _T("Geo_Info_Count_Tar_2"), db, m_vecInfoParameter_2_Tar);

    // Info Name
    SaveVec2_String(bSave, _T("m_vecstrinfoName_Ref_Count"), db, m_vecstrinfoName_Ref);
    SaveVec2_String(bSave, _T("m_vecstrinfoName_Tar_Count"), db, m_vecstrinfoName_Tar);

    if (version == 0)
    {
        // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
        return FALSE;
    }
    else
    {
        std::vector<std::vector<Ipvm::Rect32s>> vectorRoi;
        // Inspection ROI
        SaveVec2_FRect(bSave, _T("vecrtInspectionROI_Ref_Count"), db, m_vecrtInspectionROI_Ref_BCU);
        SaveVec2_FRect(bSave, _T("vecrtInspectionROI_Tar_Count"), db, m_vecrtInspectionROI_Tar_BCU);
    }

    m_ImageProcMangePara.LinkDataBase(bSave, db[_T("{87931126-27FE-46A3-86BC-FE8B522C3A4A}")]);

    strTemp.Empty();

    return TRUE;
}

void VisionInspectionGeometryPara::SaveVec2_FRect(
    BOOL bSave, CString strBaseKeyName, CiDataBase& db, std::vector<std::vector<Ipvm::Rect32r>>& vecstrInspectionROI)
{
    long nSize = 0;
    long nSize1 = 0;
    CString strKeyName;

    strKeyName.Format(_T("%s"), (LPCTSTR)strBaseKeyName);
    if (bSave == TRUE)
    {
        nSize = (long)vecstrInspectionROI.size();
        if (!db[strKeyName].Link(bSave, nSize))
            nSize = 0;
    }
    else
    {
        if (!db[strKeyName].Link(bSave, nSize))
            nSize = 0;
        vecstrInspectionROI.resize(nSize);
    }

    for (long n = 0; n < nSize; n++)
    {
        strKeyName.Format(_T("%s_%d"), (LPCTSTR)strBaseKeyName, n);
        if (bSave == TRUE)
        {
            nSize1 = (long)vecstrInspectionROI[n].size();
            if (!db[strKeyName].Link(bSave, nSize1))
                nSize1 = 0;
        }
        else
        {
            if (!db[strKeyName].Link(bSave, nSize1))
                nSize1 = 0;
            vecstrInspectionROI[n].resize(nSize1);
        }

        for (long m = 0; m < nSize1; m++)
        {
            strKeyName.Format(_T("%s_%d_%d"), (LPCTSTR)strBaseKeyName, n, m);
            if (!LinkEx(bSave, db[strKeyName], vecstrInspectionROI[n][m]))
            {
                vecstrInspectionROI[n][m].SetRectEmpty();
            }
        }
    }

    strKeyName.Empty();
}

void VisionInspectionGeometryPara::SaveVec2_String(
    BOOL bSave, CString strBaseKeyName, CiDataBase& db, std::vector<std::vector<CString>>& vecstrinfoName)
{
    long nSize = 0;
    long nSize1 = 0;
    CString strKeyName;

    strKeyName.Format(_T("%s"), (LPCTSTR)strBaseKeyName);
    if (bSave == TRUE)
    {
        nSize = (long)vecstrinfoName.size();
        if (!db[strKeyName].Link(bSave, nSize))
            nSize = 0;
    }
    else
    {
        if (!db[strKeyName].Link(bSave, nSize))
            nSize = 0;
        vecstrinfoName.resize(nSize);
    }

    for (long n = 0; n < nSize; n++)
    {
        strKeyName.Format(_T("%s_%d"), (LPCTSTR)strBaseKeyName, n);
        if (bSave == TRUE)
        {
            nSize1 = (long)vecstrinfoName[n].size();
            if (!db[strKeyName].Link(bSave, nSize1))
                nSize1 = 0;
        }
        else
        {
            if (!db[strKeyName].Link(bSave, nSize1))
                nSize1 = 0;
            vecstrinfoName[n].resize(nSize1);
        }

        for (long m = 0; m < nSize1; m++)
        {
            strKeyName.Format(_T("%s_%d_%d"), (LPCTSTR)strBaseKeyName, n, m);
            if (!db[strKeyName].Link(bSave, vecstrinfoName[n][m]))
                vecstrinfoName[n][m] = _T("");
        }
    }

    strKeyName.Empty();
}

void VisionInspectionGeometryPara::SaveVec3_Long(
    BOOL bSave, CString strBaseKeyName, CiDataBase& db, std::vector<std::vector<std::vector<long>>>& vecInfoParameter)
{
    long nSize = 0;
    long nSize1 = 0;
    long nSize2 = 0;
    CString strKeyName;

    strKeyName.Format(_T("%s"), (LPCTSTR)strBaseKeyName);
    if (bSave == TRUE)
    {
        nSize = (long)vecInfoParameter.size();
        if (!db[strKeyName].Link(bSave, nSize))
            nSize = 0;
    }
    else
    {
        if (!db[strKeyName].Link(bSave, nSize))
            nSize = 0;
        vecInfoParameter.resize(nSize);
    }

    for (long n = 0; n < nSize; n++)
    {
        strKeyName.Format(_T("%s_%d"), (LPCTSTR)strBaseKeyName, n);
        if (bSave == TRUE)
        {
            nSize1 = (long)vecInfoParameter[n].size();
            if (!db[strKeyName].Link(bSave, nSize1))
                nSize1 = 0;
        }
        else
        {
            if (!db[strKeyName].Link(bSave, nSize1))
                nSize1 = 0;
            vecInfoParameter[n].resize(nSize1);
        }

        for (long m = 0; m < nSize1; m++)
        {
            strKeyName.Format(_T("%s_%d_%d"), (LPCTSTR)strBaseKeyName, n, m);
            if (bSave == TRUE)
            {
                nSize2 = (long)vecInfoParameter[n][m].size();
                if (!db[strKeyName].Link(bSave, nSize2))
                    nSize2 = 0;
            }
            else
            {
                if (!db[strKeyName].Link(bSave, nSize2))
                    nSize2 = 0;
                vecInfoParameter[n][m].resize(nSize2);
            }

            for (long j = 0; j < nSize2; j++)
            {
                strKeyName.Format(_T("%s_%d_%d_%d"), (LPCTSTR)strBaseKeyName, n, m, j);
                if (!db[strKeyName].Link(bSave, vecInfoParameter[n][m][j]))
                    vecInfoParameter[n][m][j] = 0;
            }
        }
    }

    strKeyName.Empty();
}

void VisionInspectionGeometryPara::Init()
{
    m_nInspectionNumber = 1;
    m_vecfReferenceSpec.clear();

    m_vecstrInspectionName.clear();
    m_vecstrInspectionName.resize(m_nInspectionNumber);
    m_vecstrInspectionName[0] = _T("Untitle Name");

    m_vecnInspectionType.clear();
    m_vecnInspectionType.resize(m_nInspectionNumber);
    m_vecnInspectionType[0] = 0;

    m_vecnInspectionDistanceResult.clear();
    m_vecnInspectionDistanceResult.resize(m_nInspectionNumber);
    m_vecnInspectionDistanceResult[0] = 0;

    m_vecnInspectionCircleResult.clear();
    m_vecnInspectionCircleResult.resize(m_nInspectionNumber);
    m_vecnInspectionCircleResult[0] = 0;

    m_vecInfoParameter_0_Ref.clear();
    m_vecInfoParameter_0_Tar.clear();

    m_ImageProcMangePara.Init();
}

std::vector<CString> VisionInspectionGeometryPara::ExportAlgoParaToText(
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

    //Module 특성 변수 //채워야 함
    for (long nCategory = 0; nCategory < (long)m_vecstrInspectionName.size(); nCategory++)
    {
        strCategory = m_vecstrInspectionName[nCategory];

        strGroup.Format(_T("Reference Setting Parameter"));

        for (long nRefPara = 0; nRefPara < (long)m_vecstrinfoName_Ref[nCategory].size(); nRefPara++)
        {
            if ((long)m_vecstrinfoName_Ref[nCategory].size() < 1)
            {
                break;
            }

            strParaName = m_vecstrinfoName_Ref[nCategory][nRefPara];

            // Align Data List를 어떤 것을 사용하였는지 확인 - 2022.08.31_JHB_TxtRecipe
            if (strParaName.CompareNoCase(_T("Body Line Info")) == 0)
            {
                for (long nValue = 0; nValue < (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara].size(); nValue++)
                {
                    // 여기다가 한번에 적으면 됨
                    strParaNameAux = g_szBodyLineParameter_Name[nValue];
                    double dValue = m_vecInfoParameter_0_Ref[nCategory][nRefPara][nValue];
                    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                        strHeader, strCategory, strGroup, strParaName, strParaNameAux, (bool)dValue));
                }
            }

            if (strParaName.CompareNoCase(_T("Body Center Info")) == 0)
            {
                for (long nValue = 0; nValue < (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara].size(); nValue++)
                {
                    // 여기다가 한번에 적으면 됨
                    strParaNameAux = _T("Use Data");
                    double dValue = m_vecInfoParameter_0_Ref[nCategory][nRefPara][nValue];
                    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                        strHeader, strCategory, strGroup, strParaName, strParaNameAux, (bool)dValue));
                }
            }

            if (strParaName.CompareNoCase(_T("Find Edge Point")) == 0)
            {
                // 여기다가 한번에 적으면 됨
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Search_Dir],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Search_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Dir],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Type],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Type]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Thresh],
                    (float)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Thresh]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Angle],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Angle]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Detect_Area],
                    (bool)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
            }

            if (strParaName.CompareNoCase(_T("Find Line Data")) == 0)
            {
                // 여기다가 한번에 적으면 됨
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Search_Dir],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Search_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Dir],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Type],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Type]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Thresh],
                    (float)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Thresh]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Angle],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Angle]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Detect_Area],
                    (bool)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
            }

            if (strParaName.CompareNoCase(_T("Find Round")) == 0)
            {
                // 여기다가 한번에 적으면 됨
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Search_Dir],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Search_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Dir],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Type],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Type]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Thresh],
                    (float)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Thresh]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Angle],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Angle]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Detect_Area],
                    (bool)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
            }

            if (strParaName.CompareNoCase(_T("Find Circle")) == 0)
            {
                // 여기다가 한번에 적으면 됨
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Search_Dir],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Search_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Dir],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Type],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Type]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Thresh],
                    (float)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Thresh]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Angle],
                    (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Angle]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Detect_Area],
                    (bool)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
            }
            // 이후 추가되는 Align Data List는 여기 밑으로 추가
        }

        strGroup.Format(_T("Target Setting Parameter"));

        for (long nRefPara = 0; nRefPara < (long)m_vecstrinfoName_Tar[nCategory].size(); nRefPara++)
        {
            if ((long)m_vecstrinfoName_Tar[nCategory].size() < 1)
            {
                break;
            }

            strParaName = m_vecstrinfoName_Tar[nCategory][nRefPara];

            // Align Data List를 어떤 것을 사용하였는지 확인 - 2022.08.31_JHB_TxtRecipe
            if (strParaName.CompareNoCase(_T("Body Line Info")) == 0)
            {
                for (long nValue = 0; nValue < (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara].size(); nValue++)
                {
                    // 여기다가 한번에 적으면 됨
                    strParaNameAux = g_szBodyLineParameter_Name[nValue];
                    bool bValue = m_vecInfoParameter_0_Tar[nCategory][nRefPara][nValue];
                    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                        strHeader, strCategory, strGroup, strParaName, strParaNameAux, bValue));
                }
            }

            if (strParaName.CompareNoCase(_T("Body Center Info")) == 0)
            {
                for (long nValue = 0; nValue < (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara].size(); nValue++)
                {
                    // 여기다가 한번에 적으면 됨
                    strParaNameAux = _T("Use Data");
                    bool bValue = m_vecInfoParameter_0_Tar[nCategory][nRefPara][nValue];
                    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                        strHeader, strCategory, strGroup, strParaName, strParaNameAux, bValue));
                }
            }

            if (strParaName.CompareNoCase(_T("Find Edge Point")) == 0)
            {
                // 여기다가 한번에 적으면 됨
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Search_Dir],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Search_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Dir],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Type],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Type]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Thresh],
                    (float)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Thresh]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Angle],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Angle]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Detect_Area],
                    (bool)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
            }

            if (strParaName.CompareNoCase(_T("Find Line Data")) == 0)
            {
                // 여기다가 한번에 적으면 됨
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Search_Dir],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Search_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Dir],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Type],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Type]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Thresh],
                    (float)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Thresh]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Angle],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Angle]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Detect_Area],
                    (bool)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
            }

            if (strParaName.CompareNoCase(_T("Find Round")) == 0)
            {
                // 여기다가 한번에 적으면 됨
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Search_Dir],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Search_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Dir],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Type],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Type]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Thresh],
                    (float)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Thresh]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Angle],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Angle]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Detect_Area],
                    (bool)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
            }

            if (strParaName.CompareNoCase(_T("Find Circle")) == 0)
            {
                // 여기다가 한번에 적으면 됨
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Search_Dir],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Search_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Dir],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Dir]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Type],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Type]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Thresh],
                    (float)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Thresh]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Angle],
                    (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Angle]));
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Detect_Area],
                    (bool)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
            }
            // 이후 추가되는 Align Data List는 여기 밑으로 추가
        }

        strGroup.Format(_T("Inspection Parameter"));

        if (m_vecnInspectionType[nCategory] == INSP_PARAM_TYPE_DISTANCE)
        {
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, _T("Insp Type"), _T(""), m_vecnInspectionType[nCategory]));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Distance Result"), _T(""), m_vecnInspectionDistanceResult[nCategory]));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Insp Spec (um)"), _T(""), (double)m_vecfReferenceSpec[nCategory]));
        }

        if (m_vecnInspectionType[nCategory] == INSP_PARAM_TYPE_CIRCLE)
        {
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, _T("Insp Type"), _T(""), m_vecnInspectionType[nCategory]));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Circle Result"), _T(""), m_vecnInspectionCircleResult[nCategory]));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, _T("Insp Spec (um)"), _T(""), m_vecfReferenceSpec[nCategory]));
        }

        if (m_vecnInspectionType[nCategory] == INSP_PARAM_TYPE_ANGLE)
        {
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, _T("Insp Type"), _T(""), m_vecnInspectionType[nCategory]));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, _T("Insp Spec (um)"), _T(""), m_vecfReferenceSpec[nCategory]));
        }
    }

    return vecstrAlgorithmParameters;
}