//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingNGRVPara.h"

//CPP_2_________________________________ This project's headers

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
enum enumColorIndex
{
    INDEX_START = 0,
    INDEX_RED = INDEX_START,
    INDEX_GREEN,
    INDEX_BLUE,
    INDEX_END,
};

VisionProcessingNGRVPara::VisionProcessingNGRVPara(VisionProcessing& parent)
    : m_ImageProcMangePara(parent)
    , m_imageSizeX(0)
    , m_imageSizeY(0)
{
    Init();
}

VisionProcessingNGRVPara::~VisionProcessingNGRVPara(void)
{
}

BOOL VisionProcessingNGRVPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;

    CString strTemp;

    if (!bSave)
    {
        Init();
    }

    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    if (db[_T("{3DE48989-6BD2-470A-8D20-E39C31A49D7F}")].Link(bSave, m_defectNumber) != TRUE)
    {
        m_defectNumber = 1;
    }

    m_vecDefectName.resize(m_defectNumber);
    m_vecItemID.resize(m_defectNumber);
    m_vecDefectCode.resize(m_defectNumber);
    m_vecDisposition.resize(m_defectNumber);
    m_vecDecision.resize(m_defectNumber);
    m_vecStitchCount.resize(m_defectNumber);
    m_vecAFtype.resize(m_defectNumber);
    m_vecGrabHeightOffset_um.resize(m_defectNumber);
    m_vec2IllumUsage.resize(m_defectNumber);
    m_vecGrabPosROI.resize(m_defectNumber);

    // Defect Name
    for (long i = 0; i < m_defectNumber; i++)
    {
        strTemp.Format(_T("Inspection_Name_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecDefectName[i]))
            m_vecDefectName[i] = _T("Untitle Name");
    }

    // Item ID
    for (long i = 0; i < m_defectNumber; i++)
    {
        strTemp.Format(_T("Item_ID_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecItemID[i]))
            m_vecItemID[i] = _T("000");
    }

    // Loss Code
    for (long i = 0; i < m_defectNumber; i++)
    {
        strTemp.Format(_T("Defect_Code_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecDefectCode[i]))
            m_vecDefectCode[i] = _T("");
    }

    // Disposition
    for (long i = 0; i < m_defectNumber; i++)
    {
        strTemp.Format(_T("Disposition_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecDisposition[i]))
            m_vecDisposition[i] = DISPOSITION_NULL; // Default to PASS
    }

    // Decision
    for (long i = 0; i < m_defectNumber; i++)
    {
        strTemp.Format(_T("Decision_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecDecision[i]))
            m_vecDecision[i] = NOT_MEASURED; // Default to PASS
    }

    // Stitch CountX,Y
    for (long i = 0; i < m_defectNumber; i++)
    {
        strTemp.Format(_T("Stitch_Count_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecStitchCount[i]))
            m_vecStitchCount[i] = Ipvm::Point32s2(1, 1);
    }

    // AF Type
    for (long i = 0; i < m_defectNumber; i++)
    {
        strTemp.Format(_T("AFType_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecAFtype[i]))
            m_vecAFtype[i] = 0;
    }

    // Grab Height Offset
    for (long i = 0; i < m_defectNumber; i++)
    {
        strTemp.Format(_T("Grab_Height_Offset_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecGrabHeightOffset_um[i]))
            m_vecGrabHeightOffset_um[i] = 0.0f;
    }

    // Frame Usage
    //2차원 vector사용시에는 초기화를 주의하여야 한다
    for (long index = 0; index < m_defectNumber; index++)
    {
        m_vec2IllumUsage[index].resize(LED_ILLUM_FRAME_MAX);
    }
    for (long i = 0; i < m_defectNumber; i++)
    {
        for (long index = 0; index < (long)m_vec2IllumUsage[i].size(); index++)
        {
            //strTemp.Format(_T("Frame_Num_%d"), index + 1);
            strTemp.Format(_T("Defect_idx_%d_Frame_Num_%d"), i, index + 1);
            if (!db[strTemp].Link(bSave, m_vec2IllumUsage[i][index]))
            {
                m_vec2IllumUsage[i][index] = FALSE;
            }
        }
    }

    // Grab Position ROI
    for (long i = 0; i < m_defectNumber; i++)
    {
        strTemp.Format(_T("Position_ROI_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecGrabPosROI[i]))
            m_vecGrabPosROI[i] = Ipvm::Rect32s(0, 0, 0, 0);

        if (!bSave) //Grab Position ROI정보가 imageSize를 넘어가는 경우에 대한 예외처리
        {
            m_vecGrabPosROI[i] = ConvertGrabROI_Position_Limit_imageSize(m_vecGrabPosROI[i], m_imageSizeX, m_imageSizeY);
        }
    }

    return TRUE;
}

void VisionProcessingNGRVPara::SaveVec2_FRect(
    BOOL bSave, CString strBaseKeyName, CiDataBase& db, std::vector<std::vector<Ipvm::Rect32r>>& vecstrInspectionROI)
{
    UNREFERENCED_PARAMETER(bSave);
    UNREFERENCED_PARAMETER(db);
    UNREFERENCED_PARAMETER(vecstrInspectionROI);
    /*long nSize = 0;
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

    strKeyName.Empty();*/
}

void VisionProcessingNGRVPara::SaveVec2_String(
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

void VisionProcessingNGRVPara::SaveVec3_Long(
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

Ipvm::Rect32s VisionProcessingNGRVPara::ConvertGrabROI_Position_Limit_imageSize(
    const Ipvm::Rect32s& srcROI, const long& imageSizeX, const long& imageSizeY)
{
    if (imageSizeX <= 0 || imageSizeY <= 0) //imageSize가 0이하라면 그냥 recipe에 있는 ROI를 사용한다
        return srcROI;

    Ipvm::Rect32s dst(0, 0, 0, 0);
    if (srcROI.IsRectEmpty() == true)
        dst = Ipvm::Rect32s(0, 0, 0, 0);

    const long width = max(srcROI.m_left, srcROI.m_right) - min(srcROI.m_left, srcROI.m_right);
    const long height = max(srcROI.m_top, srcROI.m_bottom) - min(srcROI.m_top, srcROI.m_bottom);

    //유효성 검사 ImageSize를 넘어가지 않도록
    if (srcROI.m_left > imageSizeX)
        dst.m_left = min(dst.m_left, srcROI.m_left);
    else
        dst.m_left = max(dst.m_left, srcROI.m_left);

    dst.m_right = min(imageSizeX, dst.m_left + width);

    if (srcROI.m_top > imageSizeY)
        dst.m_top = min(dst.m_top, srcROI.m_top);
    else
        dst.m_top = max(dst.m_top, srcROI.m_top);

    dst.m_bottom = min(imageSizeY, dst.m_top + height);

    return dst;
}

void VisionProcessingNGRVPara::Init()
{
    m_defectNumber = 1;

    m_vecDefectName.clear();
    m_vecDefectName.resize(m_defectNumber);
    m_vecDefectName[0] = _T("Untitle Name");

    m_vecItemID.clear();
    m_vecItemID.resize(m_defectNumber);
    m_vecItemID[0] = _T("000");

    m_vecDefectCode.clear();
    m_vecDefectCode.resize(m_defectNumber);
    m_vecDefectCode[0] = _T("");

    m_vecDisposition.clear();
    m_vecDisposition.resize(m_defectNumber);
    m_vecDisposition[0] = DISPOSITION_NULL; // Default to PASS

    m_vecDecision.clear();
    m_vecDecision.resize(m_defectNumber);
    m_vecDecision[0] = NOT_MEASURED; // Default to PASS

    m_vecStitchCount.clear();
    m_vecStitchCount.resize(m_defectNumber);
    m_vecStitchCount[0].m_x = 0;
    m_vecStitchCount[0].m_y = 0;

    m_vecAFtype.clear();
    m_vecAFtype.resize(m_defectNumber);
    m_vecAFtype[0] = 0;

    m_vecGrabHeightOffset_um.clear();
    m_vecGrabHeightOffset_um.resize(m_defectNumber);
    m_vecGrabHeightOffset_um[0] = 0;

    m_vec2IllumUsage.clear();
    m_vec2IllumUsage.resize(m_defectNumber);
    for (long index = 0; index < m_defectNumber; index++)
    {
        m_vec2IllumUsage[index].resize(LED_ILLUM_FRAME_MAX);
    }

    m_vecGrabPosROI.clear();
    m_vecGrabPosROI.resize(m_defectNumber);
    for (long index = 0; index < m_defectNumber; index++)
    {
        m_vecGrabPosROI[index].SetRectEmpty();
    }

    /*m_mainImageSizeX = 0;
    m_mainImageSizeY = 0;

    m_mainImageScale = Ipvm::Point32r2(0.f, 0.f);

    for (long index = 0; index < 3; index++)
    {
        m_imageColor[index].Create(0, 0);
    }

    m_imageReference.Create(0, 0);*/

    m_ImageProcMangePara.Init();
}

void VisionProcessingNGRVPara::SetNGRV_SingleRun_RefimageSize(const long& imageSizeX, const long& imageSizeY)
{
    m_imageSizeX = imageSizeX;
    m_imageSizeY = imageSizeY;
}

//std::vector<CString> VisionProcessingNGRVPara::ExportAlgoParaToText(
//    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
//{
////본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
//CString strHeader;
//strHeader.Format(_T("%s,%s"), strVisionName, strInspectionModuleName);

////개별 Parameter용 변수
//CString strCategory;
//CString strGroup;
//CString strParaName;
//CString strParaNameAux;
//CString strValue;

////Output Parameter 생성
//std::vector<CString> vecstrAlgorithmParameters(0);

////Image Combine
//std::vector<CString> vecstrImageCombineParameters(0);
//vecstrImageCombineParameters = m_ImageProcMangePara.ExportImageCombineParaToText(
//    strVisionName, strInspectionModuleName, _T("Image Combine"));

//vecstrAlgorithmParameters.insert(
//    vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

////Module 특성 변수 //채워야 함
//for (long nCategory = 0; nCategory < (long)m_vecstrInspectionName.size(); nCategory++)
//{
//    strCategory = m_vecstrInspectionName[nCategory];

//    strGroup.Format(_T("Reference Setting Parameter"));

//    for (long nRefPara = 0; nRefPara < (long)m_vecstrinfoName_Ref[nCategory].size(); nRefPara++)
//    {
//        if ((long)m_vecstrinfoName_Ref[nCategory].size() < 1)
//        {
//            break;
//        }

//        strParaName = m_vecstrinfoName_Ref[nCategory][nRefPara];

//        // Align Data List를 어떤 것을 사용하였는지 확인 - 2022.08.31_JHB_TxtRecipe
//        if (strParaName.CompareNoCase(_T("Body Line Info")) == 0)
//        {
//            for (long nValue = 0; nValue < (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara].size(); nValue++)
//            {
//                // 여기다가 한번에 적으면 됨
//                strParaNameAux = g_szBodyLineParameter_Name[nValue];
//                double dValue = m_vecInfoParameter_0_Ref[nCategory][nRefPara][nValue];
//                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
//                    strHeader, strCategory, strGroup, strParaName, strParaNameAux, (bool)dValue));
//            }
//        }

//        if (strParaName.CompareNoCase(_T("Body Center Info")) == 0)
//        {
//            for (long nValue = 0; nValue < (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara].size(); nValue++)
//            {
//                // 여기다가 한번에 적으면 됨
//                strParaNameAux = _T("Use Data");
//                double dValue = m_vecInfoParameter_0_Ref[nCategory][nRefPara][nValue];
//                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
//                    strHeader, strCategory, strGroup, strParaName, strParaNameAux, (bool)dValue));
//            }
//        }

//        if (strParaName.CompareNoCase(_T("Find Edge Point")) == 0)
//        {
//            // 여기다가 한번에 적으면 됨
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Search_Dir],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Search_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Dir],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Type],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Type]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Thresh],
//                (float)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Thresh]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Angle],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Angle]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Detect_Area],
//                (bool)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
//        }

//        if (strParaName.CompareNoCase(_T("Find Line Data")) == 0)
//        {
//            // 여기다가 한번에 적으면 됨
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Search_Dir],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Search_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Dir],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Type],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Type]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Thresh],
//                (float)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Thresh]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Angle],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Angle]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Detect_Area],
//                (bool)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
//        }

//        if (strParaName.CompareNoCase(_T("Find Round")) == 0)
//        {
//            // 여기다가 한번에 적으면 됨
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Search_Dir],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Search_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Dir],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Type],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Type]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Thresh],
//                (float)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Thresh]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Angle],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Angle]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Detect_Area],
//                (bool)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
//        }

//        if (strParaName.CompareNoCase(_T("Find Circle")) == 0)
//        {
//            // 여기다가 한번에 적으면 됨
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Search_Dir],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Search_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Dir],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Type],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Type]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Thresh],
//                (float)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Thresh]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Angle],
//                (long)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Angle]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Detect_Area],
//                (bool)m_vecInfoParameter_0_Ref[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
//        }
//        // 이후 추가되는 Align Data List는 여기 밑으로 추가
//    }

//    strGroup.Format(_T("Target Setting Parameter"));

//    for (long nRefPara = 0; nRefPara < (long)m_vecstrinfoName_Tar[nCategory].size(); nRefPara++)
//    {
//        if ((long)m_vecstrinfoName_Tar[nCategory].size() < 1)
//        {
//            break;
//        }

//        strParaName = m_vecstrinfoName_Tar[nCategory][nRefPara];

//        // Align Data List를 어떤 것을 사용하였는지 확인 - 2022.08.31_JHB_TxtRecipe
//        if (strParaName.CompareNoCase(_T("Body Line Info")) == 0)
//        {
//            for (long nValue = 0; nValue < (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara].size(); nValue++)
//            {
//                // 여기다가 한번에 적으면 됨
//                strParaNameAux = g_szBodyLineParameter_Name[nValue];
//                bool bValue = m_vecInfoParameter_0_Tar[nCategory][nRefPara][nValue];
//                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
//                    strHeader, strCategory, strGroup, strParaName, strParaNameAux, bValue));
//            }
//        }

//        if (strParaName.CompareNoCase(_T("Body Center Info")) == 0)
//        {
//            for (long nValue = 0; nValue < (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara].size(); nValue++)
//            {
//                // 여기다가 한번에 적으면 됨
//                strParaNameAux = _T("Use Data");
//                bool bValue = m_vecInfoParameter_0_Tar[nCategory][nRefPara][nValue];
//                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
//                    strHeader, strCategory, strGroup, strParaName, strParaNameAux, bValue));
//            }
//        }

//        if (strParaName.CompareNoCase(_T("Find Edge Point")) == 0)
//        {
//            // 여기다가 한번에 적으면 됨
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Search_Dir],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Search_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Dir],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Type],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Type]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Thresh],
//                (float)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Thresh]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Angle],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Angle]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Detect_Area],
//                (bool)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
//        }

//        if (strParaName.CompareNoCase(_T("Find Line Data")) == 0)
//        {
//            // 여기다가 한번에 적으면 됨
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Search_Dir],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Search_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Dir],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Type],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Type]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Thresh],
//                (float)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Thresh]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Angle],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Angle]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName[ED_Param_Edge_Detect_Area],
//                (bool)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
//        }

//        if (strParaName.CompareNoCase(_T("Find Round")) == 0)
//        {
//            // 여기다가 한번에 적으면 됨
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Search_Dir],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Search_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Dir],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Type],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Type]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Thresh],
//                (float)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Thresh]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Angle],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Angle]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Detect_Area],
//                (bool)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
//        }

//        if (strParaName.CompareNoCase(_T("Find Circle")) == 0)
//        {
//            // 여기다가 한번에 적으면 됨
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Search_Dir],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Search_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Dir],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Dir]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Type],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Type]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Thresh],
//                (float)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Thresh]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Angle],
//                (long)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Angle]));
//            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//                strGroup, strParaName, g_szEdgeDetectParameterName_Circle[ED_Param_Edge_Detect_Area],
//                (bool)m_vecInfoParameter_0_Tar[nCategory][nRefPara][ED_Param_Edge_Detect_Area]));
//        }
//        // 이후 추가되는 Align Data List는 여기 밑으로 추가
//    }

//    strGroup.Format(_T("Inspection Parameter"));

//    if (m_vecnInspectionType[nCategory] == INSP_PARAM_TYPE_DISTANCE)
//    {
//        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
//            strHeader, strCategory, strGroup, _T("Insp Type"), _T(""), m_vecnInspectionType[nCategory]));
//        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//            strGroup, _T("Distance Result"), _T(""), m_vecnInspectionDistanceResult[nCategory]));
//        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//            strGroup, _T("Insp Spec (um)"), _T(""), (double)m_vecfReferenceSpec[nCategory]));
//    }

//    if (m_vecnInspectionType[nCategory] == INSP_PARAM_TYPE_CIRCLE)
//    {
//        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
//            strHeader, strCategory, strGroup, _T("Insp Type"), _T(""), m_vecnInspectionType[nCategory]));
//        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
//            strGroup, _T("Circle Result"), _T(""), m_vecnInspectionCircleResult[nCategory]));
//        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
//            strHeader, strCategory, strGroup, _T("Insp Spec (um)"), _T(""), m_vecfReferenceSpec[nCategory]));
//    }

//    if (m_vecnInspectionType[nCategory] == INSP_PARAM_TYPE_ANGLE)
//    {
//        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
//            strHeader, strCategory, strGroup, _T("Insp Type"), _T(""), m_vecnInspectionType[nCategory]));
//        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
//            strHeader, strCategory, strGroup, _T("Insp Spec (um)"), _T(""), m_vecfReferenceSpec[nCategory]));
//    }
//}

//return vecstrAlgorithmParameters;
//}
