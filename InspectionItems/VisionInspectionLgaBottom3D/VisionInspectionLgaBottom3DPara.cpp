//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionLgaBottom3DPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionLgaBottom3DPara::VisionInspectionLgaBottom3DPara(void)
{
    Init();
}

VisionInspectionLgaBottom3DPara::~VisionInspectionLgaBottom3DPara(void)
{
}

void VisionInspectionLgaBottom3DPara::Init()
{
    m_nBlobSearchImage = 0;

    m_fSRSearchExtRatio = 100.f;

    m_nSR_Algorithm = 1;
    m_SR_topPercentAverage_Min = 50.f; //kircheis_Tan
    m_SR_topPercentAverage_Max = 100.f; //kircheis_Tan
    m_SR_IgnoreOffsetXum = 0.f;
    m_SR_IgnoreOffsetYum = 0.f;
    m_SR_visibilityLowerBound = 100.f;

    isReverseThreshold = FALSE;

    m_vecLandParameter.resize(0);
    m_vec2UnitCoplGroup.resize(0);
    m_vecstrGroupUnitCoplName.clear();
    m_vecstrGroupUnitCoplName.resize(0);

    m_nGroupUnitCoplCount = 0;
}

BOOL VisionInspectionLgaBottom3DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
        Init();

    if (!db[_T("{D23DE919-A139-47F0-8431-8E6464DEDD70}")].Link(bSave, m_nBlobSearchImage))
        m_nBlobSearchImage = 0;

    if (!db[_T("fSRSearchExtRatio")].Link(bSave, m_fSRSearchExtRatio))
        m_fSRSearchExtRatio = 100.f;

    if (!db[_T("{9D3EA6AE-A972-41EF-9204-B05515942E4B}")].Link(bSave, m_nSR_Algorithm))
        m_nSR_Algorithm = 1;
    if (!db[_T("{18739754-92A9-4422-AB42-32B1E1563015}")].Link(bSave, m_SR_topPercentAverage_Min))
        m_SR_topPercentAverage_Min = 50.f; //kircheis_Tan
    if (!db[_T("{D162B695-B693-4489-8DAD-ACA3895CE74B}")].Link(bSave, m_SR_topPercentAverage_Max))
        m_SR_topPercentAverage_Max = 100.f; //kircheis_Tan

    if (!db[_T("{08EFD296-7E80-4A60-BE66-D533BBE3F0FE}")].Link(bSave, m_SR_IgnoreOffsetXum))
        m_SR_IgnoreOffsetXum = 0.f;
    if (!db[_T("{3811C2C2-8234-4AEA-BCD9-057D9AA6CBAB}")].Link(bSave, m_SR_IgnoreOffsetYum))
        m_SR_IgnoreOffsetYum = 0.f;

    if (!db[_T("{736E15CC-6796-428D-AFB8-C3B595023DFA}")].Link(bSave, m_SR_visibilityLowerBound))
        m_SR_visibilityLowerBound = 100.f;
    if (!db[_T("{8BBDCD83-A9A7-4990-9EBA-5C3DD12CB39B}")].Link(bSave, isReverseThreshold))
        isReverseThreshold = FALSE;

    long nGroupCount = 0;
    if (bSave)
        nGroupCount = (long)m_vecLandParameter.size();
    if (!db[_T("{06943EBC-0F0B-48B4-8C68-248D4D29D4CD}")].Link(bSave, nGroupCount))
        nGroupCount = 0;

    m_vecLandParameter.resize(nGroupCount);

    for (int i = 0; i < nGroupCount; i++)
    {
        m_vecLandParameter[i].LinkDataBase(bSave, db.GetSubDBFmt(_T("{43D89952-224C-4F1F-86FA-6EB7918FFFE2}_%d"), i));
    }

    //kk Parameter Group에 저장된 것이 없을 경우 Load
    if (nGroupCount == 0)
    {
        Land3DPara temp;

        if (!db[_T("fObjSearchX")].Link(bSave, temp.m_fObjSearchX))
            temp.m_fObjSearchX = 0.2f;
        if (!db[_T("fObjSearchY")].Link(bSave, temp.m_fObjSearchY))
            temp.m_fObjSearchY = 0.2f;
        if (!db[_T("fLandMaskErodeSize")].Link(bSave, temp.m_fLandMaskErodeSize))
            temp.m_fLandMaskErodeSize = 100.f;
        if (!db[_T("{ADA013E0-5B5D-47FD-A391-2FFB1ED730D0}")].Link(bSave, temp.m_SR_landIgnoreSize))
            temp.m_SR_landIgnoreSize = 40.f;
        if (!db[_T("{C8448BD1-5952-4723-A174-A60F19C7B85E}")].Link(bSave, temp.m_SR_landDilateSize))
            temp.m_SR_landDilateSize = 150.f;

        if (m_vecLandParameter.size() <= 0)
            m_vecLandParameter.resize(1);
        m_vecLandParameter[0].Copy(temp);
    }

    //Land Group Unit Copl 저장부분

    if (!db[_T("{021C1AA2-E95C-4BDB-A356-0170ECC3ED31}")].Link(bSave, m_nGroupUnitCoplCount))
        m_nGroupUnitCoplCount = 0;

    m_vec2UnitCoplGroup.resize(m_nGroupUnitCoplCount);
    m_vecstrGroupUnitCoplName.resize(m_nGroupUnitCoplCount);

    BOOL Check = FALSE;

    for (int i = 0; i < m_nGroupUnitCoplCount; i++)
    {
        long GroupCount = 0;
        CString str;

        str.Format(_T("{D9D86D07-AD6C-4F73-BB8F-719184FDCA6D}_%d"), i);

        if (bSave)
            GroupCount = (long)m_vec2UnitCoplGroup[i].size();

        if (!db[str].Link(bSave, GroupCount))
            GroupCount = 0;

        m_vec2UnitCoplGroup[i].resize(GroupCount);

        for (int j = 0; j < GroupCount; j++)
        {
            CString str2;

            str2.Format(_T("{DE62C54A-37BF-4C2A-8FF2-7DD24AEE2133}_%d_%d"), i, j);

            if (!db[str2].Link(bSave, m_vec2UnitCoplGroup[i][j]))
                m_vec2UnitCoplGroup[i][j] = 0;

            if (m_vec2UnitCoplGroup[i][j] >= nGroupCount && !Check)
            {
                Check = TRUE;

                CString strMessage = _T("An issue has occurred that does not match the group information among the ")
                                     _T("Land Group Unit Copl values set.");

                ::SimpleMessage(strMessage);
                strMessage.Empty();
            }
            str2.Empty();
        }

        str.Format(_T("{7509529E-5C96-4180-90FB-577B600DFC8B}_%d"), i);

        if (!db[str].Link(bSave, m_vecstrGroupUnitCoplName[i]))
            m_vecstrGroupUnitCoplName[i] = "Untitle";

        str.Empty();
    }

    //kk Group Spec 저장 부분

    long GroupSpec = 0;

    if (bSave)
        GroupSpec = (long)m_vecstrGroupInspName.size();

    if (!db[_T("{DCDDF5D3-87C3-4736-84BA-77C8A56424F1}")].Link(bSave, GroupSpec))
        GroupSpec = 0;

    m_vecstrGroupInspName.resize(GroupSpec);

    for (int i = 0; i < GroupSpec; i++)
    {
        CString str;

        str.Format(_T("{243A6EA0-283E-4047-B646-31DDD98640F3}_%d"), i);

        m_vecstrGroupInspName[i].LinkDataBase(
            bSave, db.GetSubDBFmt(_T("{6DB64CE0-89AE-4E3A-A615-D58E58B49FAC}_%d"), i));

        str.Empty();
    }

    return TRUE;
}

std::vector<CString> VisionInspectionLgaBottom3DPara::ExportAlgoParaToText(
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
    strGroup.Format(_T("Land 3D Align Parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Land Center Align Image"), _T(""), m_nBlobSearchImage));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Use Reverse Threshold"), _T(""), (bool)isReverseThreshold));

    strGroup.Format(_T("Land 3D SR Parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("SR Visibility Lower Bound (GV)"), _T(""), m_SR_visibilityLowerBound));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("SR Search Extension Area (%)"), _T(""), m_fSRSearchExtRatio));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("SR Ignore Offset X (um)"), _T(""), m_SR_IgnoreOffsetXum));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("SR Ignore Offset Y (um)"), _T(""), m_SR_IgnoreOffsetYum));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("SR Z Algorithm"), _T(""), m_nSR_Algorithm));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("SR Top Z-Value Average Percent Min(%)"), _T(""), m_SR_topPercentAverage_Min));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("SR Top Z-Value Average Percent Max(%)"), _T(""), m_SR_topPercentAverage_Max));

    //SubDlg Parameters
    long nGroupNum = (long)m_vecLandParameter.size();
    vecstrAlgorithmParameters.push_back(
        CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, _T("Group Num"), _T(""), _T(""), nGroupNum));

    for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
    {
        strGroup.Format(_T("Group %d"), nGroup + 1);
        auto& para = m_vecLandParameter[nGroup];

        strParaName.Format(_T("Land Search Parameter"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("Edge Search Length X (mm)"), para.m_fObjSearchX));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("Edge Search Length Y (mm)"), para.m_fObjSearchY));

        strParaName.Format(_T("Land Mask Parameter"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Land Cal. Area from Edge (Erode Size: um)"), para.m_fLandMaskErodeSize));

        strParaName.Format(_T("SR Mask Parameter"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Land Ignore Area from Edge (Dilate Size: um)"), para.m_SR_landIgnoreSize));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("SR Cal. Area from Land Edge (um)"), para.m_SR_landDilateSize));
    }

    return vecstrAlgorithmParameters;
}

Land3DPara::Land3DPara()
{
    Init();
}
Land3DPara::~Land3DPara()
{
}

void Land3DPara::Init()
{
    LandType = 0;

    m_fObjSearchX = 0.2f;
    m_fObjSearchY = 0.2f;

    m_fLandMaskErodeSize = 100.f;

    m_SR_landIgnoreSize = 40.f;
    m_SR_landDilateSize = 150.f;
}

BOOL Land3DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{66501D56-E5E0-461C-9E94-DF6034E2C14E}")].Link(bSave, m_fObjSearchX))
        m_fObjSearchX = 0.2f;
    if (!db[_T("{6FE48A80-ABF2-4626-8E9E-155F2B7935DB}")].Link(bSave, m_fObjSearchY))
        m_fObjSearchY = 0.2f;
    if (!db[_T("{3D3FEE7F-35F0-48ED-B21C-F62C819418C3}")].Link(bSave, m_fLandMaskErodeSize))
        m_fLandMaskErodeSize = 100.f;
    if (!db[_T("{C4432EAA-A6E2-49D4-90A5-A67C5CEB90B9}")].Link(bSave, m_SR_landIgnoreSize))
        m_SR_landIgnoreSize = 40.f;
    if (!db[_T("{B4676995-246F-4CE0-9218-5AF8546C96BC}")].Link(bSave, m_SR_landDilateSize))
        m_SR_landDilateSize = 150.f;

    return TRUE;
}

BOOL Land3DPara::Copy(Land3DPara Data)
{
    LandType = Data.LandType;

    m_fObjSearchX = Data.m_fObjSearchX;
    m_fObjSearchY = Data.m_fObjSearchY;

    m_fLandMaskErodeSize = Data.m_fLandMaskErodeSize;

    m_SR_landIgnoreSize = Data.m_SR_landIgnoreSize;
    m_SR_landDilateSize = Data.m_SR_landDilateSize;

    return TRUE;
}

void Land3DGroupInsp::SetName(LPCTSTR group_id)
{
    LandCoplName.Format(_T("(%s) %s"), group_id, g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_COPL]);
    LandUnitCoplName.Format(_T("(%s) %s"), group_id, g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_COPL]);
    LandHeightName.Format(_T("(%s) %s"), group_id, g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_HEIGHT]);
    LandWarpageName.Format(_T("(%s) %s"), group_id, g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_WARPAGE]);
    LandUnitWarpageName.Format(_T("(%s) %s"), group_id, g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE]);
}

BOOL Land3DGroupInsp::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{0B518EE4-2BC7-452C-B2F1-1F6CD9C60AAF}")].Link(bSave, LandCoplName))
        LandCoplName = _T("Untitle");
    if (!db[_T("{628FF32D-91C2-47F0-8A11-4AE1B3BADA00}")].Link(bSave, LandUnitCoplName))
        LandUnitCoplName = _T("Untitle");
    if (!db[_T("{3380B13A-9229-4C7C-AE58-82E483B35B1C}")].Link(bSave, LandHeightName))
        LandHeightName = _T("Untitle");
    if (!db[_T("{DF072C08-F06F-4D27-ADD9-1D040F9706E2}")].Link(bSave, LandWarpageName))
        LandWarpageName = _T("Untitle");
    if (!db[_T("{C5E5EC22-D005-4CAB-AB96-66C19DB6A306}")].Link(bSave, LandUnitWarpageName))
        LandUnitWarpageName = _T("Untitle");

    return TRUE;
}
