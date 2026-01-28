//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBottom3DPara.h"

//CPP_2_________________________________ This project's headers
#include "ParaDefine.h" //kircheis_TxtRecipe

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
VisionInspectionBgaBottom3DPara::VisionInspectionBgaBottom3DPara(void)
    : m_parameters(1)
{
    Init();
}

VisionInspectionBgaBottom3DPara::~VisionInspectionBgaBottom3DPara(void)
{
}

BallLevelParameter::BallLevelParameter()
{
    Init();
}

void BallLevelParameter::Init()
{
    m_fBallSearchAreaPercent = 200.;

    for (long type = 0; type < long(enumGetherBallZType::END); type++)
    {
        auto& para = m_paraBallZ[type];

        para.m_calcAreaPercent = 50.;
        para.m_validPixelHeightThreshold_Percent = 30.;
        para.m_topPercentAverage = 100.;
        para.m_maxValidPointsPercentOfSpecRadius = 100.;
        para.m_dAcceptableInvalidBallCount = 0.; //kircheis_AIBC
    }

    m_fBallIgnorePercent = 120.;
    m_fBallIgnoreExpandPercent = 50.;
    m_fSRCalcAreaPercent = 300.;
}

void BallLevelParameter::Copy(BallLevelParameter Data)
{
    m_fBallSearchAreaPercent = Data.m_fBallSearchAreaPercent;
    for (int i = 0; i < long(enumGetherBallZType::END); i++)
    {
        m_paraBallZ[i].m_calcAreaPercent = Data.m_paraBallZ[i].m_calcAreaPercent;
        m_paraBallZ[i].m_maxValidPointsPercentOfSpecRadius = Data.m_paraBallZ[i].m_maxValidPointsPercentOfSpecRadius;
        m_paraBallZ[i].m_topPercentAverage = Data.m_paraBallZ[i].m_topPercentAverage;
        m_paraBallZ[i].m_validPixelHeightThreshold_Percent = Data.m_paraBallZ[i].m_validPixelHeightThreshold_Percent;
        m_paraBallZ[i].m_dAcceptableInvalidBallCount
            = Data.m_paraBallZ[i].m_dAcceptableInvalidBallCount; //kircheis_AIBC
    }
    m_fBallIgnorePercent = m_fBallIgnorePercent;
    m_fBallIgnoreExpandPercent = Data.m_fBallIgnoreExpandPercent;
    m_fSRCalcAreaPercent = Data.m_fSRCalcAreaPercent;
}

BOOL BallLevelParameter::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("{941939FC-BD16-456B-87CE-C118221ACCA9}")].Link(bSave, version))
        version = 0;

    if (version == 0)
    {
        // Ball Z을 하나의 파라메터로 계산했을 때

        for (long type = 0; type < long(enumGetherBallZType::END); type++)
        {
            auto& para = m_paraBallZ[type];

            db[_T("{C023E227-32F4-40C3-A3BB-B4CB54A05171}")].Link(bSave, para.m_calcAreaPercent);
            db[_T("{7BCD5A3E-EA6D-485C-B0C2-63D356D91281}")].Link(bSave, para.m_validPixelHeightThreshold_Percent);
            db[_T("{10F87682-ECFF-4169-B2AB-3EF84C506493}")].Link(bSave, para.m_topPercentAverage);

            para.m_maxValidPointsPercentOfSpecRadius = 100.;
            para.m_dAcceptableInvalidBallCount = 0.; //kircheis_AIBC
        }
    }
    else
    {
        // Ball Z을 검사에 따라 다른 파라메터로 계산

        auto& para_height = m_paraBallZ[long(enumGetherBallZType::Height)];
        auto& para_copl = m_paraBallZ[long(enumGetherBallZType::Copl)];

        db[_T("{C023E227-32F4-40C3-A3BB-B4CB54A05171}")].Link(bSave, para_height.m_calcAreaPercent);
        db[_T("{7BCD5A3E-EA6D-485C-B0C2-63D356D91281}")].Link(bSave, para_height.m_validPixelHeightThreshold_Percent);
        db[_T("{10F87682-ECFF-4169-B2AB-3EF84C506493}")].Link(bSave, para_height.m_topPercentAverage);
        db[_T("{4DA6DA55-0884-4864-B90C-97170027F74E}")].Link(bSave, para_height.m_maxValidPointsPercentOfSpecRadius);
        db[_T("{560C2A02-6B26-4308-BABE-E10E1CF7960F}")].Link(
            bSave, para_height.m_dAcceptableInvalidBallCount); //kircheis_AIBC

        db[_T("{FF90F6F9-C823-49FC-8C29-DE351DC83136}")].Link(bSave, para_copl.m_calcAreaPercent);
        db[_T("{D85321A2-1B4F-405A-9FE2-B1F9ED3F05E6}")].Link(bSave, para_copl.m_validPixelHeightThreshold_Percent);
        db[_T("{E4518579-7A90-4C11-9853-010E93392107}")].Link(bSave, para_copl.m_topPercentAverage);
        db[_T("{6688DE3A-C8FB-4F71-BAB2-BABE7A4E8ECC}")].Link(bSave, para_copl.m_maxValidPointsPercentOfSpecRadius);
        db[_T("{A65C11FF-B7EC-41E9-BDC5-E74439C90F20}")].Link(
            bSave, para_copl.m_dAcceptableInvalidBallCount); //kircheis_AIBC
    }

    db[_T("{66BED331-3E9A-4455-BB2A-559D75DEE3AC}")].Link(bSave, m_fBallSearchAreaPercent);
    db[_T("{3B48EA89-EE51-4154-B84E-2DC8D9B8B9E9}")].Link(bSave, m_fBallIgnorePercent);
    db[_T("{1EF90B38-9C6D-4B0E-A99D-F5BDF3EEA577}")].Link(bSave, m_fBallIgnoreExpandPercent);
    db[_T("{1294F015-E18B-4F05-8B29-FED2BA61D88B}")].Link(bSave, m_fSRCalcAreaPercent);

    return TRUE;
}

BOOL VisionInspectionBgaBottom3DPara::LinkDataBase(BOOL bSave, CiDataBase& db, const long ballDiameterTypeCount)
{
    if (!bSave)
        m_parameters.clear();
    m_parameters.resize(ballDiameterTypeCount);

    for (long idx = 0; idx < ballDiameterTypeCount; idx++)
    {
        m_parameters[idx].LinkDataBase(bSave, db.GetSubDBFmt(_T("{FB6730F4-A3C7-47F6-BAE1-BC6E4088965E}_%d"), idx));
    }

    db[_T("{2021B47D-91C1-4605-AC62-991E5AFB89AD}")].Link(bSave, m_logicalPixelVisibilityLowerStartBound_ball);
    db[_T("{E9BC594F-FDF0-4DA2-B4A7-CD1DCB048730}")].Link(bSave, m_logicalPixelVisibilityLowerEndBound_ball);
    db[_T("{922B6CE9-9D17-4A02-AEA0-5C3E3A4EDF60}")].Link(bSave, m_logicalPixelVisibilityUpperStartBound_ball);
    db[_T("{63323892-2CE3-4926-8C76-FBAB2A6BAE36}")].Link(bSave, m_logicalPixelVisibilityUpperEndBound_ball);
    db[_T("{E5C9BAD5-210F-4E54-AF5B-ABBE7224B642}")].Link(bSave, m_visibilityLowerBound_sr);

    //kircheis_Tan
    db[_T("{86D7F127-2F49-41FC-8A1D-BCAFB40950E5}")].Link(bSave, m_nSR_Algorithm);
    db[_T("{3ABB348D-DD4E-439B-8743-28D0097EAB78}")].Link(bSave, m_SR_topPercentAverage_Min);
    db[_T("{653ACCC9-672E-4968-BEC6-FD25D68C7DD3}")].Link(bSave, m_SR_topPercentAverage_Max);

    db[_T("{7ED19601-7F1C-4B7B-A079-D2767E5D75A3}")].Link(bSave, m_SR_IgnoreOffsetXum);
    db[_T("{B1441DC5-DA90-43E1-B941-8B909CF8AA6E}")].Link(bSave, m_SR_IgnoreOffsetYum);

    if (!db[_T("{60C9729B-1092-40BD-9257-F512FE04FAE9}")].Link(bSave, m_nInvalidateSinglePointData))
        m_nInvalidateSinglePointData = INVALIDATE_SINGLE_POINT; //kircheis_loss3D

    //kk Group Unit Copl
    if (!db[_T("{80BEB202-1737-4EC1-AF57-9CDB56818F73}")].Link(bSave, m_nGroupUnitCoplCount))
        m_nGroupUnitCoplCount = 0;

    m_vec2UnitCoplGroup.resize(m_nGroupUnitCoplCount);
    m_vecstrGroupUnitCoplName.resize(m_nGroupUnitCoplCount);

    for (int i = 0; i < m_nGroupUnitCoplCount; i++)
    {
        long GroupCount = 0;
        CString str;

        str.Format(_T("{9933397C-649B-4E8F-AC91-1CF9B51B1560}_%d"), i);

        if (bSave)
            GroupCount = (long)m_vec2UnitCoplGroup[i].size();

        if (!db[str].Link(bSave, GroupCount))
            GroupCount = 0;

        m_vec2UnitCoplGroup[i].resize(GroupCount);

        for (int j = 0; j < GroupCount; j++)
        {
            CString str2;

            str2.Format(_T("{764CAD84-7E42-46B4-9989-DBF6E3DC0E31}_%d_%d"), i, j);

            if (!db[str2].Link(bSave, m_vec2UnitCoplGroup[i][j]))
                m_vec2UnitCoplGroup[i][j] = 0;

            str2.Empty();
        }

        str.Format(_T("{8AB0A4A1-026D-40C0-9C44-76F6C8C8AB61}_%d"), i);

        if (!db[str].Link(bSave, m_vecstrGroupUnitCoplName[i]))
            m_vecstrGroupUnitCoplName[i] = "Untitle";

        str.Empty();
    }

    //kk Group Spec 저장 부분
    long GroupSpec = 0;

    if (bSave)
        GroupSpec = (long)m_vecstrGroupInspName.size();

    if (!db[_T("{10D8226A-74AC-4710-9B4F-9C6D44EDF716}")].Link(bSave, GroupSpec))
        GroupSpec = 0;

    m_vecstrGroupInspName.resize(GroupSpec);

    for (int i = 0; i < GroupSpec; i++)
    {
        CString str;

        str.Format(_T("{61EA351E-02A3-4127-9354-754C25E3EACB}_%d"), i);

        m_vecstrGroupInspName[i].LinkDataBase(
            bSave, db.GetSubDBFmt(_T("{21D286C9-7E46-45B4-9C6B-354C5A950943}_%d"), i));

        str.Empty();
    }

    return TRUE;
}

void VisionInspectionBgaBottom3DPara::Init()
{
    for (auto& para : m_parameters)
    {
        para.Init();
    }

    m_logicalPixelVisibilityLowerStartBound_ball = 0.;
    m_logicalPixelVisibilityLowerEndBound_ball = 255.;
    m_logicalPixelVisibilityUpperStartBound_ball = 255.;
    m_logicalPixelVisibilityUpperEndBound_ball = 255.;
    m_visibilityLowerBound_sr = 0.;

    //kircheis_Tan
    m_nSR_Algorithm = 0;
    m_SR_topPercentAverage_Min = 50.f;
    m_SR_topPercentAverage_Max = 100.f;

    m_SR_IgnoreOffsetXum = 0.;
    m_SR_IgnoreOffsetYum = 0.;

    m_vec2UnitCoplGroup.resize(0);
    m_vecstrGroupUnitCoplName.clear();
    m_vecstrGroupUnitCoplName.resize(0);

    m_nInvalidateSinglePointData = INVALIDATE_SINGLE_POINT; //kircheis_loss3D

    m_nGroupUnitCoplCount = 0;
}

void BallGroupInsp::SetName(LPCTSTR group_id)
{
    BallCoplName.Format(_T("(%s) %s"), group_id, g_szBGA3DInspectionName[_3DINSP_COPL]);
    BallUnitCoplName.Format(_T("(%s) %s"), group_id, g_szBGA3DInspectionName[_3DINSP_UNIT_COPL]);
    BallHeightName.Format(_T("(%s) %s"), group_id, g_szBGA3DInspectionName[_3DINSP_HEIGHT]);
    BallWarpageName.Format(_T("(%s) %s"), group_id, g_szBGA3DInspectionName[_3DINSP_WARPAGE]);
    BallUnitWarpageName.Format(_T("(%s) %s"), group_id, g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE]);
}

BOOL BallGroupInsp::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{2C2E2B56-86F7-44A6-8917-2D823FC0CF09}")].Link(bSave, BallCoplName))
        BallCoplName = _T("Untitle");
    if (!db[_T("{49B56521-084B-4595-AAFE-6747A4260F6B}")].Link(bSave, BallUnitCoplName))
        BallUnitCoplName = _T("Untitle");
    if (!db[_T("{E28BA2CE-2C13-4797-B407-91E1DF2803F7}")].Link(bSave, BallHeightName))
        BallHeightName = _T("Untitle");
    if (!db[_T("{9A1F0311-F91A-4D0B-9B78-4AB70FD83F44}")].Link(bSave, BallWarpageName))
        BallWarpageName = _T("Untitle");
    if (!db[_T("{8EBB77A8-F79C-4AB7-A026-BD60507B6FB1}")].Link(bSave, BallUnitWarpageName))
        BallUnitWarpageName = _T("Untitle");

    return TRUE;
}

std::vector<CString> VisionInspectionBgaBottom3DPara::ExportAlgoParaToText(
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
    strGroup.Format(_T("Common Parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Logical Pixel Visibility Lower Start Bound(Ball)"), _T(""), m_logicalPixelVisibilityLowerStartBound_ball));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Logical Pixel Visibility Lower End Bound (Ball)"), _T(""), m_logicalPixelVisibilityLowerEndBound_ball));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Logical Pixel Visibility Upper Start Bound (Ball)"), _T(""), m_logicalPixelVisibilityUpperStartBound_ball));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Logical Pixel Visibility Upper End Bound (Ball)"), _T(""), m_logicalPixelVisibilityUpperEndBound_ball));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Visibility Lower Bound (SR)"), _T(""), m_visibilityLowerBound_sr));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Invalidate single 3D point data"), _T(""), m_nInvalidateSinglePointData));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("SR Z Algorithm"), _T(""), m_nSR_Algorithm));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("SR Top Z-Value Average Percent Min(%)"), _T(""), m_SR_topPercentAverage_Min));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("SR Top Z-Value Average Percent Max(%)"), _T(""), m_SR_topPercentAverage_Max));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("SR Ignore Offset X (um)"), _T(""), m_SR_IgnoreOffsetXum));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("SR Ignore Offset Y (um)"), _T(""), m_SR_IgnoreOffsetYum));

    //SubDlg Parameters
    long nGroupNum = (long)m_parameters.size();
    vecstrAlgorithmParameters.push_back(
        CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, _T("Group Num"), _T(""), _T(""), nGroupNum));

    for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
    {
        auto& ballParam = m_parameters[nGroup];
        strGroup.Format(_T("Group %d Parameter"), nGroup + 1);

        strParaName.Format(_T("Diameter"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("Ball search area"), ballParam.m_fBallSearchAreaPercent));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("Ball ignore area"), ballParam.m_fBallIgnorePercent));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Ball ignore expansion"), ballParam.m_fBallIgnoreExpandPercent));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("SR calculation area"), ballParam.m_fSRCalcAreaPercent));

        long nType = (long)enumGetherBallZType::Height;
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("HEIGHT: Ball calculation area"), ballParam.m_paraBallZ[nType].m_calcAreaPercent));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("HEIGHT: Ball valid pixel height threshold"),
            ballParam.m_paraBallZ[nType].m_validPixelHeightThreshold_Percent));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("HEIGHT: Ball Top Percent Average (%)"), ballParam.m_paraBallZ[nType].m_topPercentAverage));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("HEIGHT: Max. Valid Points of spec radius (%)"),
            ballParam.m_paraBallZ[nType].m_maxValidPointsPercentOfSpecRadius));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("HEIGHT: Acceptable invalid ball count"),
            ballParam.m_paraBallZ[nType].m_dAcceptableInvalidBallCount));

        nType = (long)enumGetherBallZType::Copl;
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("COPL: Ball calculation area"), ballParam.m_paraBallZ[nType].m_calcAreaPercent));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("COPL: Ball valid pixel height threshold"),
            ballParam.m_paraBallZ[nType].m_validPixelHeightThreshold_Percent));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("COPL: Ball Top Percent Average (%)"), ballParam.m_paraBallZ[nType].m_topPercentAverage));
        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, strParaName,
                _T("COPL: Acceptable invalid ball count"), ballParam.m_paraBallZ[nType].m_dAcceptableInvalidBallCount));
    }

    return vecstrAlgorithmParameters;
}