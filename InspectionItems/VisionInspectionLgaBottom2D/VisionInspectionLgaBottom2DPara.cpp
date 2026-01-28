//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionLgaBottom2DPara.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionLgaBottom2D.h"

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

#define RectType_MaxSize 4

//CPP_7_________________________________ Implementation body
//
VisionInspectionLgaBottom2DPara::VisionInspectionLgaBottom2DPara(VisionProcessing& parent)
{
    UNREFERENCED_PARAMETER(parent);

    Init();
}

VisionInspectionLgaBottom2DPara::~VisionInspectionLgaBottom2DPara(void)
{
}

BOOL VisionInspectionLgaBottom2DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    CString strTemp;

    if (!bSave)
    {
        Init();
    }

    if (!db[_T("m_nIntensityofLand")].Link(bSave, m_nIntensityofLand))
        m_nIntensityofLand = 0;

    long nGroupCount = 0;
    if (bSave)
        nGroupCount = (long)m_vecLandParameter.size();
    if (!db[_T("{3EFA85F0-87AB-4551-8DD7-3F898F774EAE}")].Link(bSave, nGroupCount))
        nGroupCount = 0;

    m_vecLandParameter.resize(nGroupCount);

    for (int i = 0; i < nGroupCount; i++)
    {
        m_vecLandParameter[i].LinkDataBase(bSave, db.GetSubDBFmt(_T("{7A0FA7FE-462C-4CF0-99EE-E348BB61C746}_%d"), i));
    }

    // 20140710_BGA_Align : BGA_Align에서 사용하던 파라미터를 받아오도록 한다.
    //kk Parameter Group에 저장된 것이 없을 경우 Load
    if (nGroupCount == 0)
    {
        LandPara temp;

        if (!db[_T("m_fSearchLength")].Link(bSave, temp.m_fEdgeSearchLength))
            temp.m_fEdgeSearchLength = 0.3f;
        if (!db[_T("m_fTopSearchRangeStart")].Link(bSave, temp.m_vec2fSearchRangeStart[0]))
            temp.m_vec2fSearchRangeStart[0] = 10.f;
        if (!db[_T("m_fTopSearchRangeEnd")].Link(bSave, temp.m_vec2fSearchRangeEnd[0]))
            temp.m_vec2fSearchRangeEnd[0] = 90.f;
        if (!db[_T("m_fBottomSearchRangeStart")].Link(bSave, temp.m_vec2fSearchRangeStart[1]))
            temp.m_vec2fSearchRangeStart[1] = 10.f;
        if (!db[_T("m_fBottomSearchRangeEnd")].Link(bSave, temp.m_vec2fSearchRangeEnd[1]))
            temp.m_vec2fSearchRangeEnd[1] = 90.f;

        if (!db[_T("m_fLeftSearchRangeStart")].Link(bSave, temp.m_vec2fSearchRangeStart[2]))
            temp.m_vec2fSearchRangeStart[2] = 10.f;
        if (!db[_T("m_fLeftSearchRangeEnd")].Link(bSave, temp.m_vec2fSearchRangeEnd[2]))
            temp.m_vec2fSearchRangeEnd[2] = 90.f;
        if (!db[_T("m_fRightSearchRangeStart")].Link(bSave, temp.m_vec2fSearchRangeStart[3]))
            temp.m_vec2fSearchRangeStart[3] = 10.f;
        if (!db[_T("m_fRightSearchRangeEnd")].Link(bSave, temp.m_vec2fSearchRangeEnd[3]))
            temp.m_vec2fSearchRangeEnd[3] = 90.f;

        if (m_vecLandParameter.size() <= 0)
            m_vecLandParameter.resize(1);
        m_vecLandParameter[0].Copy(temp);
    }

    //kk Group Spec 저장 부분

    long GroupSpec = 0;

    if (bSave)
        GroupSpec = (long)m_vecstrGroupInspName.size();

    if (!db[_T("{22A0A47A-8F8F-4080-A29F-10C6BFE3BECA}")].Link(bSave, GroupSpec))
        GroupSpec = 0;

    m_vecstrGroupInspName.resize(GroupSpec);

    for (int i = 0; i < GroupSpec; i++)
    {
        CString str;

        str.Format(_T("{5B74616C-D0BA-47FA-8A06-0F496611C5DB}_%d"), i);

        m_vecstrGroupInspName[i].LinkDataBase(
            bSave, db.GetSubDBFmt(_T("{A946C922-E89F-4C6E-99CF-2F2384C247D0}_%d"), i));

        str.Empty();
    }

    strTemp.Empty();

    return TRUE;
}

void VisionInspectionLgaBottom2DPara::Init()
{
    // 2D Algorithm Parameter...
    m_nIntensityofLand = 0; // 0: Bright, 1: Dark
}

std::vector<CString> VisionInspectionLgaBottom2DPara::ExportAlgoParaToText(
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
    strCategory.Format(_T("Algorithm Parameters"));

    //Main Parameters
    strGroup.Format(_T("Land Specification Parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Intensity of Land"), _T(""), m_nIntensityofLand));

    //SubDlg Parameters
    long nGroupNum = (long)m_vecLandParameter.size();
    vecstrAlgorithmParameters.push_back(
        CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, _T("Group Num"), _T(""), _T(""), nGroupNum));

    for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
    {
        strGroup.Format(_T("Group %d"), nGroup + 1);
        auto& para = m_vecLandParameter[nGroup];

        strParaName.Format(_T("Land Search Parameter"));
        if (para.LandType == LandType_Rect)
        {
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Edge Search Length (mm)"), para.m_fEdgeSearchLength));

            strParaName.Format(_T("Land Top Search Range"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Top Search Start (%)"), para.m_vec2fSearchRangeStart[0]));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Top Search End (%)"), para.m_vec2fSearchRangeEnd[0]));

            strParaName.Format(_T("Land Bottom Search Range"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Bottom Search Start (%)"), para.m_vec2fSearchRangeStart[1]));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Bottom Search End (%)"), para.m_vec2fSearchRangeEnd[1]));

            strParaName.Format(_T("Land Left Search Range"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Left Search Start (%)"), para.m_vec2fSearchRangeStart[2]));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Left Search End (%)"), para.m_vec2fSearchRangeEnd[2]));

            strParaName.Format(_T("Land Right Search Range"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Right Search Start (%)"), para.m_vec2fSearchRangeStart[3]));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Right Search End (%)"), para.m_vec2fSearchRangeEnd[3]));
        }
        else if (para.LandType == LandType_Circle)
        {
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Edge Search Length Ratio"), para.m_fEdgeSearchLength));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, _T("Edge Search Threshold"), para.m_fThresholdValue));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, _T("Edge Search Count"), para.m_nEdgeSearchCount));
        }
        else //추후 새로운 Type의 Land의 파라미터를 채울 자리
        {
        }
    }

    return vecstrAlgorithmParameters;
}

LandPara::LandPara()
{
    Init();
}
LandPara::~LandPara()
{
}

void LandPara::Init()
{
    LandType = 0;

    //rect
    m_vec2fSearchRangeStart.clear();
    m_vec2fSearchRangeEnd.clear();

    m_vec2fSearchRangeStart.resize(RectType_MaxSize);
    m_vec2fSearchRangeEnd.resize(RectType_MaxSize);

    for (int i = 0; i < RectType_MaxSize; i++)
    {
        m_vec2fSearchRangeStart[i] = 10;
        m_vec2fSearchRangeEnd[i] = 90;
    }

    //Common
    m_fEdgeSearchLength = 0.3f;

    //Circle
    m_fThresholdValue = 1.0f;
    m_nEdgeSearchCount = 16;
}

BOOL LandPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    //Common
    if (!db[_T("{E7A8B100-75FC-42D9-9B23-EB864A42AA96}")].Link(bSave, m_fEdgeSearchLength))
        m_fEdgeSearchLength = 0.3f;

    //Rect
    for (int i = 0; i < RectType_MaxSize; i++)
    {
        CString str;

        str.Format(_T("{EFC20681-3760-4F1E-81FE-E095D1B25913}_%d"), i);
        if (!db[str].Link(bSave, m_vec2fSearchRangeStart[i]))
            m_vec2fSearchRangeStart[i] = 30;

        str.Format(_T("{33EF4EE3-CD19-44CA-9C57-919C119FE3A7}_%d"), i);
        if (!db[str].Link(bSave, m_vec2fSearchRangeEnd[i]))
            m_vec2fSearchRangeEnd[i] = 70;

        str.Empty();
    }

    //Circle
    if (!db[_T("{F43D3F2B-7354-4971-9EC4-788AC28934FE}")].Link(bSave, m_fThresholdValue))
        m_fThresholdValue = 1.0;
    if (!db[_T("{85C6EE02-75DA-408A-9492-7CE5C4FD41DF}")].Link(bSave, m_nEdgeSearchCount))
        m_nEdgeSearchCount = 16;

    return TRUE;
}

BOOL LandPara::Copy(LandPara Data)
{
    LandType = Data.LandType;

    m_fEdgeSearchLength = Data.m_fEdgeSearchLength;

    for (int i = 0; i < RectType_MaxSize; i++)
    {
        m_vec2fSearchRangeStart[i] = Data.m_vec2fSearchRangeStart[i];
        m_vec2fSearchRangeEnd[i] = Data.m_vec2fSearchRangeEnd[i];
    }

    m_fThresholdValue = Data.m_fThresholdValue;
    m_nEdgeSearchCount = Data.m_nEdgeSearchCount;

    return TRUE;
}

void Land2DGroupInsp::SetName(LPCTSTR group_id)
{
    LandOffsetXName.Format(_T("(%s) %s"), group_id, g_szLandInspectionName[LAND_INSPECTION_LAND_OFFSET_X]);
    LandOffsetYName.Format(_T("(%s) %s"), group_id, g_szLandInspectionName[LAND_INSPECTION_LAND_OFFSET_Y]);
    LandOffsetRName.Format(_T("(%s) %s"), group_id, g_szLandInspectionName[LAND_INSPECTION_LAND_OFFSET_R]);
    LandWidthName.Format(_T("(%s) %s"), group_id, g_szLandInspectionName[LAND_INSPECTION_LAND_WIDTH]);
    LandLengthName.Format(_T("(%s) %s"), group_id, g_szLandInspectionName[LAND_INSPECTION_LAND_LENGTH]);
}

BOOL Land2DGroupInsp::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{3E361918-24A9-4910-BA25-9476E81AFF18}")].Link(bSave, LandOffsetXName))
        LandOffsetXName = _T("Untitle");
    if (!db[_T("{D3D83704-0BAF-4E23-81F1-6B6192A865DE}")].Link(bSave, LandOffsetYName))
        LandOffsetYName = _T("Untitle");
    if (!db[_T("{F5D85512-9A67-439D-89A2-A61DD281E1C3}")].Link(bSave, LandOffsetRName))
        LandOffsetRName = _T("Untitle");
    if (!db[_T("{2EC01097-AD14-47D1-A0B9-118E6627EA48}")].Link(bSave, LandWidthName))
        LandWidthName = _T("Untitle");
    if (!db[_T("{B5BE3C80-C81A-4ACC-9ED1-91991474E1C8}")].Link(bSave, LandLengthName))
        LandLengthName = _T("Untitle");

    return TRUE;
}