//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionMarkSpec.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionMarkSpec::VisionInspectionMarkSpec(void)
    : m_rtMarkTeachROI_BCU(Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f))
    , m_bTeachDone(FALSE)
    , m_bSaveTeachImage(FALSE)
    , m_nMarkThreshold(100)
    , m_nCharNum(0)
    , m_nCharLineNum(0)
    , m_fLocAngleForBody(0.f)
    , m_ptLocCenterforScreen(Ipvm::Point32r2(0.f, 0.f))
    , m_plTeachCharROI(NULL)
    , m_plTeachCharArea(NULL)
    , m_ptTeachRoiLtFromPackCenter(Ipvm::Point32s2(0, 0)) //kircheis_AutoTeach
{
    //init();
    for (long i = 0; i < NUM_OF_LOCATOR; i++)
    {
        m_pnLocID[i] = 0;
        m_prtLocaterTeachingBox[i] = Ipvm::Rect32s(0, 0, 0, 0);
        m_rtLocatorTargetROI[i] = Ipvm::Rect32s(-50, -50, 50, 50);
        m_pptLocaterTargetRoiLtFromPackCenter[i] = Ipvm::Point32s2(0, 0); //kircheis_AutoTeach
    }
}

VisionInspectionMarkSpec::~VisionInspectionMarkSpec(void)
{
}

void VisionInspectionMarkSpec::Init()
{
    m_rtMarkTeachROI_BCU = Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f);
    m_ptTeachRoiLtFromPackCenter = Ipvm::Point32s2(0, 0); //kircheis_AutoTeach
    m_nMarkThreshold = 100;
    m_bTeachDone = FALSE;
    m_nCharNum = 0;
    m_nCharLineNum = 0;
    m_fLocAngleForBody = 0.f;
    m_ptLocCenterforScreen = Ipvm::Point32r2(0.f, 0.f);

    for (long i = 0; i < NUM_OF_LOCATOR; i++)
    {
        m_pnLocID[i] = 0;
        m_prtLocaterTeachingBox[i] = Ipvm::Rect32s(0, 0, 0, 0);
        m_rtLocatorTargetROI[i] = Ipvm::Rect32s(-50, -50, 50, 50);
        m_pptLocaterTargetRoiLtFromPackCenter[i] = Ipvm::Point32s2(0, 0); //kircheis_AutoTeach
    }

    if (m_plTeachCharROI != NULL)
    {
        delete[] m_plTeachCharROI;
        m_plTeachCharROI = NULL;
    }
    if (m_plTeachCharArea != NULL)
    {
        delete[] m_plTeachCharArea;
        m_plTeachCharArea = NULL;
    }

    m_bSaveTeachImage = FALSE;
    m_bTeachDone = FALSE;

    m_veclTeachCharNumPerLine.clear();
    m_veclTeachStartCharIDPerLine.clear();
}

BOOL VisionInspectionMarkSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;

    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    if (version == 0)
    {
        // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
        return FALSE;
    }
    else
    {
        if (!LinkEx(bSave, db[_T("Mark_Teach_ROI")], m_rtMarkTeachROI_BCU))
        {
            m_rtMarkTeachROI_BCU = Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f);
        }
    }

    if (!db[_T("Auto_Mark_Teach_ROI_Offset")].Link(bSave, m_ptTeachRoiLtFromPackCenter))
        m_ptTeachRoiLtFromPackCenter = Ipvm::Point32s2(0, 0); //kircheis_AutoTeach

    if (!db[_T("Mark_Teach")].Link(bSave, m_bTeachDone))
        m_bTeachDone = FALSE;

    if (!db[_T("Mark_Threshold")].Link(bSave, m_nMarkThreshold))
        m_nMarkThreshold = 100;

    if (!db[_T("Char_Num")].Link(bSave, m_nCharNum))
        m_nCharNum = 0;
    if (!db[_T("Char_Line_Num")].Link(bSave, m_nCharLineNum))
        m_nCharLineNum = 0;
    if (!db[_T("Loc_Angle_For_Body")].Link(bSave, m_fLocAngleForBody))
        m_fLocAngleForBody = 0.f;

    if (!db[_T("Loc_Center_for_Screen")][_T("X")].Link(bSave, m_ptLocCenterforScreen.m_x))
        m_ptLocCenterforScreen.m_x = 0.f;
    if (!db[_T("Loc_Center_for_Screen")][_T("Y")].Link(bSave, m_ptLocCenterforScreen.m_y))
        m_ptLocCenterforScreen.m_y = 0.f;

    if (!db[_T("Loc_ID_0")].Link(bSave, m_pnLocID[0]))
        m_pnLocID[0] = 0;
    if (!db[_T("Loc_ID_1")].Link(bSave, m_pnLocID[1]))
        m_pnLocID[1] = 999;
    if (!db[_T("Locater_Search_Box_0")].Link(bSave, m_prtLocaterTeachingBox[0]))
        m_prtLocaterTeachingBox[0] = Ipvm::Rect32s(100, 200, 200, 300);
    if (!db[_T("Locater_Search_Box_1")].Link(bSave, m_prtLocaterTeachingBox[1]))
        m_prtLocaterTeachingBox[1] = Ipvm::Rect32s(100, 200, 200, 300);
    if (!db[_T("Locator_Target_ROI_0")].Link(bSave, m_rtLocatorTargetROI[0]))
        m_rtLocatorTargetROI[0] = Ipvm::Rect32s(-50, -50, 50, 50);
    if (!db[_T("Locator_Target_ROI_1")].Link(bSave, m_rtLocatorTargetROI[1]))
        m_rtLocatorTargetROI[1] = Ipvm::Rect32s(-50, -50, 50, 50);

    //{{ //kircheis_AutoTeach
    if (!db[_T("Locator_Target_ROI_Offset_0")].Link(bSave, m_pptLocaterTargetRoiLtFromPackCenter[0]))
        m_pptLocaterTargetRoiLtFromPackCenter[0] = Ipvm::Point32s2(0, 0);
    if (!db[_T("Locator_Target_ROI_Offset_1")].Link(bSave, m_pptLocaterTargetRoiLtFromPackCenter[1]))
        m_pptLocaterTargetRoiLtFromPackCenter[1] = Ipvm::Point32s2(0, 0);
    //}}

    if (m_rtLocatorTargetROI[0].Width() <= 0 || m_rtLocatorTargetROI[0].Height() <= 0)
        m_rtLocatorTargetROI[0] = Ipvm::Rect32s(-50, -50, 50, 50);
    if (m_rtLocatorTargetROI[1].Width() <= 0 || m_rtLocatorTargetROI[1].Height() <= 0)
        m_rtLocatorTargetROI[1] = Ipvm::Rect32s(-50, -50, 50, 50);

    if (!bSave)
    {
        AllocateMemory(m_nCharNum);
    }

    CString strTemp;
    long nSize(0);

    m_nCharMinArea = LONG_MAX; //kircheis_MarkAngle
    for (long i = 0; i < m_nCharNum; i++)
    {
        strTemp.Format(_T("ROI_Spec_%d"), i);
        if (!db[strTemp].Link(bSave, m_plTeachCharROI[i]))
        {
            m_plTeachCharROI[i] = Ipvm::Rect32s(0, 0, 0, 0);
        }

        strTemp.Format(_T("Area_Spec_%d"), i);
        if (!db[strTemp].Link(bSave, m_plTeachCharArea[i]))
            m_plTeachCharArea[i] = 0;

        strTemp.Format(_T("Char_ROI_%d"), i);

        if (!db[strTemp][_T("X")].Link(bSave, m_teachCharOffset[i].m_x))
            m_teachCharOffset[i].m_x = 0.f;
        if (!db[strTemp][_T("Y")].Link(bSave, m_teachCharOffset[i].m_y))
            m_teachCharOffset[i].m_y = 0.f;

        m_nCharMinArea = (long)min(m_nCharMinArea, m_plTeachCharArea[i]); //kircheis_MarkAngle
    }

    if (!bSave)
    {
        if (!db[_T("Teach_Char_Num_Per_Line_Size")].Link(bSave, nSize))
            nSize = 0;
        m_veclTeachCharNumPerLine.resize(nSize);
    }
    else
    {
        nSize = (long)(m_veclTeachCharNumPerLine.size());
        if (!db[_T("Teach_Char_Num_Per_Line_Size")].Link(bSave, nSize))
            nSize = 0;
    }
    for (long i = 0; i < nSize; i++)
    {
        strTemp.Format(_T("Teach_Char_Num_Per_Line_%d"), i);
        if (!db[strTemp].Link(bSave, m_veclTeachCharNumPerLine[i]))
            m_veclTeachCharNumPerLine[i] = 0;
    }

    // m_veclTeachStartCharIDPerLine 저장
    if (!bSave)
    {
        if (!db[_T("Teach_Start_Char_ID_Per_Line_Size")].Link(bSave, nSize))
            nSize = 0;
        m_veclTeachStartCharIDPerLine.resize(nSize);
    }
    else
    {
        nSize = (long)(m_veclTeachStartCharIDPerLine.size());
        if (!db[_T("Teach_Start_Char_ID_Per_Line_Size")].Link(bSave, nSize))
            nSize = 0;
    }
    for (long i = 0; i < nSize; i++)
    {
        strTemp.Format(_T("Teach_Start_Char_ID_Per_Line_%d"), i);
        if (!db[strTemp].Link(bSave, m_veclTeachStartCharIDPerLine[i]))
            m_veclTeachStartCharIDPerLine[i] = 0;
    }

    strTemp.Empty();

    return TRUE;
}
VisionInspectionMarkSpec& VisionInspectionMarkSpec::operator=(const VisionInspectionMarkSpec& Src)
{
    m_rtMarkTeachROI_BCU = Src.m_rtMarkTeachROI_BCU;
    m_ptTeachRoiLtFromPackCenter = Src.m_ptTeachRoiLtFromPackCenter; //kircheis_AutoTeach
    m_bTeachDone = Src.m_bTeachDone;
    m_nMarkThreshold = Src.m_nMarkThreshold;

    m_specImage = Src.m_specImage;
    m_nCharNum = Src.m_nCharNum;
    m_nCharLineNum = Src.m_nCharLineNum;
    m_fLocAngleForBody = Src.m_fLocAngleForBody;
    m_ptLocCenterforScreen = Src.m_ptLocCenterforScreen;
    m_nCharMinArea = Src.m_nCharMinArea; //kircheis_MarkAngle

    m_pnLocID[0] = Src.m_pnLocID[0];
    m_pnLocID[1] = Src.m_pnLocID[1];
    m_prtLocaterTeachingBox[0] = Src.m_prtLocaterTeachingBox[0];
    m_prtLocaterTeachingBox[1] = Src.m_prtLocaterTeachingBox[1];
    m_rtLocatorTargetROI[0] = Src.m_rtLocatorTargetROI[0];
    m_rtLocatorTargetROI[1] = Src.m_rtLocatorTargetROI[1];

    //{{ //kircheis_AutoTeach
    m_pptLocaterTargetRoiLtFromPackCenter[0] = Src.m_pptLocaterTargetRoiLtFromPackCenter[0];
    m_pptLocaterTargetRoiLtFromPackCenter[1] = Src.m_pptLocaterTargetRoiLtFromPackCenter[1];
    //}}

    m_plTeachCharROI = Src.m_plTeachCharROI;
    m_plTeachCharArea = Src.m_plTeachCharArea;
    m_teachCharOffset = Src.m_teachCharOffset;

    m_veclTeachCharNumPerLine = Src.m_veclTeachCharNumPerLine;
    m_veclTeachStartCharIDPerLine = Src.m_veclTeachStartCharIDPerLine;

    return *this;
}

void VisionInspectionMarkSpec::AllocateMemory(int i_nCharNum)
{
    m_nCharNum = i_nCharNum;
    if (m_nCharNum <= 0)
        return;

    if (m_plTeachCharArea != NULL)
        delete[] m_plTeachCharArea;
    if (m_plTeachCharROI != NULL)
        delete[] m_plTeachCharROI;

    m_plTeachCharArea = new long[m_nCharNum];
    m_plTeachCharROI = new Ipvm::Rect32s[m_nCharNum];

    m_teachCharOffset.clear();
    m_teachCharOffset.resize(m_nCharNum, Ipvm::Point32r2(0.f, 0.f));

    for (long idx = 0; idx < m_nCharNum; idx++)
    {
        m_plTeachCharROI[idx] = Ipvm::Rect32s(0, 0, 0, 0);
        m_plTeachCharArea[idx] = 0;
    }
}

std::vector<CString> VisionInspectionMarkSpec::ExportAlgoMarkSpecToText(
    CString strHeader, CString strCategory, CString strGroup) //kircheis_TxtRecipe
{
    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)

    //개별 Parameter용 변수
    CString strParaName;
    CString strParaNameAux;
    CString strValue;

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    //Module 특성 변수
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Mark Threshold"), _T(""), m_nMarkThreshold));
    std::vector<CString> vecStrROI = CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Mark Teach ROI"), m_rtMarkTeachROI_BCU);
    vecstrAlgorithmParameters.insert(vecstrAlgorithmParameters.end(), vecStrROI.begin(), vecStrROI.end());

    vecStrROI.clear();
    vecStrROI = CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Locator Target 1(ROI)"), m_rtLocatorTargetROI[0]);
    vecstrAlgorithmParameters.insert(vecstrAlgorithmParameters.end(), vecStrROI.begin(), vecStrROI.end());

    vecStrROI.clear();
    vecStrROI = CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Locator Target 2(ROI)"), m_rtLocatorTargetROI[1]);
    vecstrAlgorithmParameters.insert(vecstrAlgorithmParameters.end(), vecStrROI.begin(), vecStrROI.end());

    return vecstrAlgorithmParameters;
}
