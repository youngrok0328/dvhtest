//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionComponent3DPara.h"

//CPP_2_________________________________ This project's headers
#include "ComponentAlign.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../InformationModule/dPI_DataBase/ChipInfo.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SComponentAlignSpec::SComponentAlignSpec()
    : bMeasureComp(TRUE)
    , bMeasureBody(TRUE)
    , bMeasureElect(TRUE)
    , m_fRoughAlign_Search_ROI_Extension_Ratio(32.)
    , m_fSpecROI_ShiftXPos(0.)
    , m_fSpecROI_ShiftYPos(0.)
    , m_bReCalc_UseSpecROI(FALSE)
    , m_fValidZRange(200.) // 60um Z 탐색.

    //MED3
    , m_nOutline_align_Electrode_Search_Width_Ratio(100)
    , m_nOutline_align_Electrode_Search_Length_um(150)
    , m_fOutline_align_Electrode_EdgeThreshold(3.)
    , m_nOutline_align_Body_Search_Width_Ratio(100)
    , m_nOutline_align_Body_Search_Length_um(200)
    , m_fOutline_align_Body_EdgeThreshold(3.)
    , m_nCalc_Electrode_Area_Width_Ratio(95)
    , m_nCalc_Electrode_Area_Length_Ratio(95)
    , m_nCalc_Body_Area_Width_Ratio(95)
    , m_nCalc_Body_Area_Length_Ratio(95)
    , m_nCalc_Body_Area_GapX_um(0)
    , m_nCalc_Body_Area_GapY_um(0)
    , m_nCalc_Substrate_Area_Size_um(100)
    , m_nCalc_Substrate_Area_GapX_Body_um(60)
    , m_nCalc_Substrate_Area_GapY_Body_um(60)
    , m_nCalc_Substrate_Area_GapX_um(0)
    , m_nCalc_Substrate_Area_GapY_um(0)
    , m_fVisibilityLowerBound(0.f)
    , bOriginConvertData(TRUE)
    , m_nExistVersionNumber(-1)
    , m_nConvertedAlignSearchParams(-1) //kircheis_debug20220721//일단 [아니오]로 초기화
{
}

SComponentAlignSpec_OriginDataFloatToDouble::SComponentAlignSpec_OriginDataFloatToDouble()
    : fSearchRatio(32.f)
    , fMoveXPos(0.f)
    , fMoveYPos(0.f)
    , fLandSize(0.2f)
    , fLandDistW(0.2f)
    , fLandDistL(0.2f)
    , m_fLandShiftW(0.f)
    , m_fLandShiftL(0.f)
    , m_fLandVisibilityLowerBound(0.f)
    , fBodyOuterW(-0.10f)
    , fBodyOuterL(-0.05f)
    , fZCalcRange(200.f) // 60um Z 탐색.
    , fElectSearchOffset(150.f)
    , fElectSearchOuterOffset(100.f)
    , fValidElectWidthRatio(90.f)
    , fValidElectLengthRatio(90.f)
{
}

BOOL SComponentAlignSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    SComponentAlignSpec_OriginDataFloatToDouble sComponentAlignSpec_OriginDataConvert;
    if (!bSave)
    {
        *this = SComponentAlignSpec();
        sComponentAlignSpec_OriginDataConvert.LinkDataBase(db);
    }

    if (bSave)
        bOriginConvertData = FALSE; //Save면 이미 Data Converting을 수행한 이후이기 떄문에 그대로 저장한다

    db[_T("Passive3D_OriginConvertData")].Link(bSave, bOriginConvertData);

    db[_T("Passive3D_MeasureComp")].Link(bSave, bMeasureComp);
    db[_T("Passive3D_MeasureBody")].Link(bSave, bMeasureBody);
    db[_T("Passive3D_MeasureElect")].Link(bSave, bMeasureElect);

    //MED3
    db[_T("Passive3D_RoughAlign_Search_ROI_Extension_Ratio")].Link(bSave, m_fRoughAlign_Search_ROI_Extension_Ratio);
    db[_T("Passive3D_SpecROI_ShiftXPos")].Link(bSave, m_fSpecROI_ShiftXPos);
    db[_T("Passive3D_SpecROI_ShiftYPos")].Link(bSave, m_fSpecROI_ShiftYPos);
    db[_T("Passive3D_ReCalc_UseSpecROI")].Link(bSave, m_bReCalc_UseSpecROI);
    db[_T("Passive3D_VisibilityLowerBound")].Link(bSave, m_fVisibilityLowerBound);
    db[_T("Passive3D_ValidZRange")].Link(bSave, m_fValidZRange);

    db[_T("Passive3D_Outline_Align_Electrode_Search_Width_Ratio")].Link(
        bSave, m_nOutline_align_Electrode_Search_Width_Ratio);
    db[_T("Passive3D_Outline_Align_Electrode_Search_Length_um")].Link(
        bSave, m_nOutline_align_Electrode_Search_Length_um);
    db[_T("Passive3D_Outline_Align_Electrode_EdgeThreshold")].Link(bSave, m_fOutline_align_Electrode_EdgeThreshold);

    db[_T("Passive3D_Outline_Align_Body_Search_Width_Ratio")].Link(bSave, m_nOutline_align_Body_Search_Width_Ratio);
    db[_T("Passive3D_Outline_Align_Body_Search_Length_um")].Link(bSave, m_nOutline_align_Body_Search_Length_um);
    db[_T("Passive3D_Outline_Align_Body_Search_EdgeThreshold")].Link(bSave, m_fOutline_align_Body_EdgeThreshold);

    db[_T("Passive3D_Calc_Electrode_Area_Width_Ratio")].Link(bSave, m_nCalc_Electrode_Area_Width_Ratio);
    db[_T("Passive3D_Calc_Electrode_Area_Length_Ratio")].Link(bSave, m_nCalc_Electrode_Area_Length_Ratio);

    db[_T("Passive3D_Calc_Body_Area_Width_Ratio")].Link(bSave, m_nCalc_Body_Area_Width_Ratio);
    db[_T("Passive3D_Calc_Body_Area_Length_Ratio")].Link(bSave, m_nCalc_Body_Area_Length_Ratio);
    if (!db[_T("Passive3D_Calc_Body_Area_GapX_um")].Link(bSave, m_nCalc_Body_Area_GapX_um))
    {
        if (!db[_T("Passive3D_Calc_Body_Area_GapX_px")].Link(bSave, m_nCalc_Body_Area_GapX_um))
            m_nCalc_Body_Area_GapX_um = 0;
    }

    if (!db[_T("Passive3D_Calc_Body_Area_GapY_um")].Link(bSave, m_nCalc_Body_Area_GapY_um))
    {
        if (!db[_T("Passive3D_Calc_Body_Area_GapX_px")].Link(bSave, m_nCalc_Body_Area_GapY_um))
            m_nCalc_Body_Area_GapY_um = 0;
    }

    db[_T("Passive3D_Calc_Substrate_Area_Size_um")].Link(bSave, m_nCalc_Substrate_Area_Size_um);
    db[_T("Passive3D_Calc_Substrate_Area_GapX_Body_um")].Link(bSave, m_nCalc_Substrate_Area_GapX_Body_um);
    db[_T("Passive3D_Calc_Substrate_Area_GapY_Body_um")].Link(bSave, m_nCalc_Substrate_Area_GapY_Body_um);
    db[_T("Passive3D_Calc_Substrate_Area_GapX_um")].Link(bSave, m_nCalc_Substrate_Area_GapX_um);
    db[_T("Passive3D_Calc_Substrate_Area_GapY_um")].Link(bSave, m_nCalc_Substrate_Area_GapY_um);

    if (bOriginConvertData)
        SetConvertingData(sComponentAlignSpec_OriginDataConvert);

    //if (bSave == TRUE) //원래 추가가 되었어야할 Code (154에는 있음) 그런데 좀 위험한 방식으로 보인다.//kircheis_debug20220721
    //m_nExistVersionNumber = 1;

    //db[_T("{8E56C5EB-2F10-4CE1-AC15-8045F231DC09}")].Link(bSave, m_nExistVersionNumber);//기존 초기화 Code
    //if (m_nExistVersionNumber < 0)
    //{
    //	m_nOutline_align_Electrode_Search_Width_Ratio = 100;
    //	m_nOutline_align_Body_Search_Width_Ratio = 100;
    //	m_nOutline_align_Electrode_Search_Length_um = 150;
    //	m_nOutline_align_Body_Search_Length_um = 200;
    //}

    //{{ //kircheis_debug20220721 //위 두 단락의 Code를 아래와 같이 수정해얄듯
    if (!db[_T("{8E56C5EB-2F10-4CE1-AC15-8045F231DC09}")].Link(bSave,
            m_nConvertedAlignSearchParams)) //Recipe를 읽거나 쓰는데 실패하면 m_nConvertedAlignSearchParams를 초기화
        m_nConvertedAlignSearchParams
            = -1; //그런데 읽다가는 실패해도(해당 값이 없을수 있으니까) 쓰다가 실패할리는 없겠쥬?

    if (bSave == FALSE && m_nConvertedAlignSearchParams == -1) //Recipe Open을 했는데 예전 파라미터인 Recipe이면
    { //현 알고리즘에 맞는 초기값을 쓰고 m_nConvertedAlignSearchParams를 1로 바꿔 초기화 완료되었음을 저장.
        m_nOutline_align_Electrode_Search_Width_Ratio = 100;
        m_nOutline_align_Body_Search_Width_Ratio = 100;
        m_nOutline_align_Electrode_Search_Length_um = 150;
        m_nOutline_align_Body_Search_Length_um = 200;
        m_nConvertedAlignSearchParams = 1;
    }
    //}}

    //if (bSave == FALSE) //mc_TD issue : 기존 Recipe에 저장된 값을 초과하는경우가 있으므로, Maximum을 준다
    //{
    //	long nMaximumWidthRatioValue = 150;
    //	if (m_nOutline_align_Electrode_Search_Width_Ratio > nMaximumWidthRatioValue)
    //		m_nOutline_align_Electrode_Search_Width_Ratio = nMaximumWidthRatioValue;
    //	if (m_nOutline_align_Body_Search_Width_Ratio > nMaximumWidthRatioValue)
    //		m_nOutline_align_Body_Search_Width_Ratio = nMaximumWidthRatioValue;
    //}
    //else if (bSave == TRUE)
    //{
    //	long nMaximumWidthRatioValue = 150;
    //	if (m_nOutline_align_Electrode_Search_Width_Ratio > nMaximumWidthRatioValue)
    //		m_nOutline_align_Electrode_Search_Width_Ratio = nMaximumWidthRatioValue;
    //	if (m_nOutline_align_Body_Search_Width_Ratio > nMaximumWidthRatioValue)
    //		m_nOutline_align_Body_Search_Width_Ratio = nMaximumWidthRatioValue;

    //	db[_T("Passive3D_Outline_Align_Electrode_Search_Width_Ratio")].Link(bSave, m_nOutline_align_Electrode_Search_Width_Ratio);
    //	db[_T("Passive3D_Outline_Align_Body_Search_Width_Ratio")].Link(bSave, m_nOutline_align_Body_Search_Width_Ratio);
    //}

    return TRUE;
}

BOOL SComponentAlignSpec_OriginDataFloatToDouble::LinkDataBase(CiDataBase& db)
{
    db[_T("SearchRatio")].Link(FALSE, fSearchRatio);
    db[_T("MoveXPos")].Link(FALSE, fMoveXPos);
    db[_T("MoveYPos")].Link(FALSE, fMoveYPos);
    db[_T("LandSize")].Link(FALSE, fLandSize);
    db[_T("LandDistW")].Link(FALSE, fLandDistW);
    db[_T("LandDistL")].Link(FALSE, fLandDistL);
    db[_T("LandShiftW")].Link(FALSE, m_fLandShiftW);
    db[_T("LandShiftL")].Link(FALSE, m_fLandShiftL);
    db[_T("LandVisibilityLowerBound")].Link(FALSE, m_fLandVisibilityLowerBound);
    db[_T("BodyOuterW")].Link(FALSE, fBodyOuterW);
    db[_T("BodyOuterL")].Link(FALSE, fBodyOuterL);
    db[_T("ZCalcRange")].Link(FALSE, fZCalcRange);
    db[_T("ElectSearchOffset")].Link(FALSE, fElectSearchOffset);
    db[_T("ElectSearchOuterOffset")].Link(FALSE, fElectSearchOuterOffset);
    db[_T("ValidElectWidthRatio")].Link(FALSE, fValidElectWidthRatio);
    db[_T("ValidElectLengthRatio")].Link(FALSE, fValidElectLengthRatio);

    return TRUE;
}

void SComponentAlignSpec::SetConvertingData(SComponentAlignSpec_OriginDataFloatToDouble i_OriginData)
{
    // VisionScale visionScale();

    m_fRoughAlign_Search_ROI_Extension_Ratio = i_OriginData.fSearchRatio;
    m_fSpecROI_ShiftXPos = i_OriginData.fMoveXPos;
    m_fSpecROI_ShiftYPos = i_OriginData.fMoveYPos;
    m_fValidZRange = i_OriginData.fZCalcRange;
    m_fVisibilityLowerBound = i_OriginData.m_fLandVisibilityLowerBound;

    m_nOutline_align_Electrode_Search_Width_Ratio = (long)i_OriginData.fElectSearchOffset;

    m_nOutline_align_Body_Search_Width_Ratio = (long)i_OriginData.fElectSearchOuterOffset;

    m_nCalc_Electrode_Area_Width_Ratio = (long)i_OriginData.fValidElectWidthRatio;
    m_nCalc_Electrode_Area_Length_Ratio = (long)i_OriginData.fValidElectLengthRatio;

    m_nCalc_Body_Area_GapX_um = (long)((i_OriginData.fBodyOuterW
                                           * SystemConfig::GetInstance()
                                               .GetScale(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
                                               .pixelToUm()
                                               .m_x)
        + .5f);
    m_nCalc_Body_Area_GapY_um = (long)((i_OriginData.fBodyOuterL
                                           * SystemConfig::GetInstance()
                                               .GetScale(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
                                               .pixelToUm()
                                               .m_y)
        + .5f);

    m_nCalc_Substrate_Area_Size_um = long((i_OriginData.fLandSize * 1000.f) + .5f); //mm->um
    m_nCalc_Substrate_Area_GapX_Body_um = long((i_OriginData.fLandDistW * 1000.f) + .5f); //mm->um
    m_nCalc_Substrate_Area_GapY_Body_um = long((i_OriginData.fLandDistL * 1000.f) + .5f); //mm->um
    m_nCalc_Substrate_Area_GapX_um = long((i_OriginData.m_fLandShiftW * 1000.f) + .5f); //mm->um
    m_nCalc_Substrate_Area_GapY_um = long((i_OriginData.m_fLandShiftL * 1000.f) + .5f); //mm->um
}

SComp3DAlgorithmSpec::SComp3DAlgorithmSpec()
    : strSpecName(_T("Default"))
{
}

BOOL SComp3DAlgorithmSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        *this = SComp3DAlgorithmSpec();
    }

    db[_T("SpecName")].Link(bSave, strSpecName);

    CompAlign.LinkDataBase(bSave, db[_T("{B04581A2-23EC-41E7-BE20-6F01F879490A}")]);

    return TRUE;
}

CSpecLinkPara3D::CSpecLinkPara3D(void)
{
}

CSpecLinkPara3D::~CSpecLinkPara3D(void)
{
}

void CSpecLinkPara3D::Apply(const CPackageSpec& spec)
{
    // Package에 없는 Spec은 없애자
    // 현재 리스트를 모두 추가해 놓고, 있는 것은 지우자
    // 남은 것은 필요 없는 것이므로 그 후에 남은 것을 지운다

    std::map<CString, bool> erase_list[5];

    for (long type = 0; type < 5; type++)
    {
        auto& list = erase_list[type];
        for (auto& info : erase_list[type])
        {
            list[info.first] = true;
        }
    }

    for (auto& object : spec.m_CompMapManager->vecCompData)
        erase_list[Chip::ChipInfo::tyCHIP].erase(object.strCompType);
    for (auto& object : spec.m_CompMapManager->vecCompData)
        erase_list[Chip::ChipInfo::tyPASSIVE].erase(object.strCompType);
    for (auto& object : spec.m_CompMapManager->vecCompData)
        erase_list[Chip::ChipInfo::tyARRAY].erase(object.strCompType);
    for (auto& object : spec.m_CompMapManager->vecCompData)
        erase_list[Chip::ChipInfo::tyHEATSINK].erase(object.strCompType);

    for (long type = 0; type < 5; type++)
    {
        auto& info_type = m_infos[type];

        for (auto& eraseInfo : erase_list[type])
        {
            info_type.erase(eraseInfo.first);
        }
    }
}

BOOL CSpecLinkPara3D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    for (long type = 0; type < 5; type++)
    {
        CString keyName;
        switch (type)
        {
            case Chip::ChipInfo::tyCHIP:
                keyName = _T("Chip");
                break;
            case Chip::ChipInfo::tyPASSIVE:
                keyName = _T("Passive");
                break;
            case Chip::ChipInfo::tyARRAY:
                keyName = _T("Array");
                break;
            case Chip::ChipInfo::tyHEATSINK:
                keyName = _T("Heatsink");
                break;
        }

        if (keyName.IsEmpty())
            continue;

        auto& obj_db = db[keyName];
        auto& info_type = m_infos[type];

        long count = (long)info_type.size();
        if (!obj_db[_T("Count")].Link(bSave, count))
            count = 0;

        std::vector<CString> names;
        for (auto& info : info_type)
            names.push_back(info.first);

        if (!bSave)
        {
            names.resize(count);
        }

        for (long index = 0; index < count; index++)
        {
            auto& sub_db = obj_db.GetSubDBFmt(_T("Data%d"), index);
            sub_db[_T("Name")].Link(bSave, names[index]);

            auto& specName = info_type[names[index]];

            sub_db[_T("Spec Name")].Link(bSave, specName);
        }

        for (int nIdx = 0; nIdx < names.size(); nIdx++)
        {
            names[nIdx].Empty();
        }

        keyName.Empty();
    }

    return TRUE;
}

CString& CSpecLinkPara3D::GetSpecName(long chipType, LPCTSTR chipName)
{
    auto itFind = m_infos[chipType].find(chipName);
    if (itFind != m_infos[chipType].end())
    {
        return itFind->second;
    }

    m_infos[chipType][chipName] = _T("Default");
    return m_infos[chipType][chipName];
}

VisionInspectionComponent3DPara::VisionInspectionComponent3DPara(void)
    : m_vecComp3DAlgoSpec(1)
    , m_fCompMissing_Height_Ratio(10.)
{
    m_vecsTextRecipePassiveInfoDB.clear(); //mc_TextRecipe
}

VisionInspectionComponent3DPara::~VisionInspectionComponent3DPara(void)
{
}

BOOL VisionInspectionComponent3DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        *this = VisionInspectionComponent3DPara();
    }

    // m_vecComp3DAlgoSpec
    {
        db[_T("CompMissingHeightRatio")].Link(bSave, m_fCompMissing_Height_Ratio);

        long count = long(m_vecComp3DAlgoSpec.size());

        db[_T("{B6FEB6BD-E40B-4409-9CBA-A3086CF6B7C8}")].Link(bSave, count);

        m_vecComp3DAlgoSpec.resize(count);

        for (long n = 0; n < count; n++)
        {
            m_vecComp3DAlgoSpec[n].LinkDataBase(
                bSave, db.GetSubDBFmt(_T("{B6FEB6BD-E40B-4409-9CBA-A3086CF6B7C8}_%d"), n));
        }
    }
    m_specLink.LinkDataBase(bSave, db[_T("{3D2E75E5-89F3-454E-9D9E-9DD11455E87F}")]);

    long nSize = 0;

    if (bSave)
    {
        nSize = CAST_LONG(m_vecstrGroupInspName.size());

        db[_T("{01A9AF2F-12FC-4B36-B4F2-1855D8B98743}")].Link(bSave, nSize);
    }
    else
    {
        db[_T("{01A9AF2F-12FC-4B36-B4F2-1855D8B98743}")].Link(bSave, nSize);

        m_vecstrGroupInspName.resize(nSize);
    }

    for (long idx = 0; idx < nSize; idx++)
    {
        m_vecstrGroupInspName[idx].LinkDataBase(
            bSave, db.GetSubDBFmt(_T("{3094E101-313F-48D7-AB1B-5FA57F943011}_%d"), idx));
    }

    m_vecsTextRecipePassiveInfoDB.clear(); //mc_TextRecipe

    return TRUE;
}

void Component3DGroupInsp::SetName(LPCTSTR compSpec)
{
    strCompSpec = compSpec;

    strCompHeightName.Format(_T("(%s) %s"), compSpec, g_szCompInspection3DName[COMP_INSPECTION_3D_COMP_HEIGHT]);
    strCompQualName.Format(_T("(%s) %s"), compSpec, g_szCompInspection3DName[COMP_INSPECTION_3D_COMP_QUALITY]);
    strElecHeightName.Format(_T("(%s) %s"), compSpec, g_szCompInspection3DName[COMP_INSPECTION_3D_ELECTRODE_HEIGHT]);
    strElecTiltName.Format(_T("(%s) %s"), compSpec, g_szCompInspection3DName[COMP_INSPECTION_3D_ELECTRODE_TILT]);
    strBodyHeightName.Format(_T("(%s) %s"), compSpec, g_szCompInspection3DName[COMP_INSPECTION_3D_BODY_HEIGHT]);
    strBodyTiltName.Format(_T("(%s) %s"), compSpec, g_szCompInspection3DName[COMP_INSPECTION_3D_BOTY_TILT]);
}

BOOL Component3DGroupInsp::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{F6A756FA-CCB0-4DD0-B630-5D50CF6642A3}")].Link(bSave, strCompHeightName))
        strCompHeightName = _T("Untitle");
    if (!db[_T("{68D425AD-CF6D-4DB6-9160-216162A88C94}")].Link(bSave, strCompQualName))
        strCompQualName = _T("Untitle");
    if (!db[_T("{086E5551-24CF-404C-934B-613C0CB7476E}")].Link(bSave, strElecHeightName))
        strElecHeightName = _T("Untitle");
    if (!db[_T("{C33553E6-C98B-4C3E-831B-201903A35062}")].Link(bSave, strElecTiltName))
        strElecTiltName = _T("Untitle");
    if (!db[_T("{927D7268-4125-4F88-B18C-788DA2A1AAB9}")].Link(bSave, strBodyHeightName))
        strBodyHeightName = _T("Untitle");
    if (!db[_T("{BD4946A3-0F08-439E-9F1B-A23427962208}")].Link(bSave, strBodyTiltName))
        strBodyTiltName = _T("Untitle");

    if (!db[_T("{20EBE631-9F61-4761-B81B-B83DB3B79FA5}")].Link(bSave, strCompSpec))
        strCompSpec = _T("Unknown Type");

    return TRUE;
}

std::vector<CString> VisionInspectionComponent3DPara::ExportAlgoParaToText(
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
    //Algorithm Spec DB

    strCategory.Format(_T("AlgorithmDB"));
    strGroup.Format(_T("3D Passive Paramter"));

    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("3D Component Parameter"), _T("Comp Missing. Component height ratio"), m_fCompMissing_Height_Ratio));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Algorithm DB Count"), _T(""), (long)m_vecComp3DAlgoSpec.size()));

    for (long nAlgoSpecidx = 0; nAlgoSpecidx < (long)m_vecComp3DAlgoSpec.size(); nAlgoSpecidx++)
    {
        strGroup.Format(_T("%s"), (LPCTSTR)m_vecComp3DAlgoSpec[nAlgoSpecidx].strSpecName);
        auto& AlignSpec = m_vecComp3DAlgoSpec[nAlgoSpecidx].CompAlign;

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Search Ratio"), _T("Extension Search Ratio"), AlignSpec.m_fRoughAlign_Search_ROI_Extension_Ratio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Search Ratio"), _T("Spec ROI Offset X"), AlignSpec.m_fSpecROI_ShiftXPos));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Search Ratio"), _T("Spec ROI Offset Y"), AlignSpec.m_fSpecROI_ShiftYPos));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Search Ratio"), _T("Valid Z Range)"), AlignSpec.m_fValidZRange));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Search Ratio"), _T("ReCalc. Use SpecROI"), (bool)AlignSpec.m_bReCalc_UseSpecROI));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Search Ratio"), _T("Visibility Lower Bound (0~255)"), AlignSpec.m_fVisibilityLowerBound));

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Search Width (Passive Width Ration %)"),
            AlignSpec.m_nOutline_align_Electrode_Search_Width_Ratio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Search Length (um)"),
            AlignSpec.m_nOutline_align_Electrode_Search_Length_um));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Edge Threshold"),
            AlignSpec.m_fOutline_align_Electrode_EdgeThreshold));

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Search Width (Passive Width Ration %)"),
            AlignSpec.m_nOutline_align_Body_Search_Width_Ratio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Search Length (um)"),
            AlignSpec.m_nOutline_align_Body_Search_Length_um));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Edge Threshold"),
            AlignSpec.m_fOutline_align_Body_EdgeThreshold));

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Measurement"), _T("Component"), (bool)AlignSpec.bMeasureComp));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Measurement"), _T("Body"), (bool)AlignSpec.bMeasureBody));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Measurement"), _T("Electrode"), (bool)AlignSpec.bMeasureElect));

        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Electrode Cal. Param"),
                _T("Electrode Cal. Area Width Size (Ration %)"), AlignSpec.m_nCalc_Electrode_Area_Width_Ratio));
        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Electrode Cal. Param"),
                _T("Electrode Cal. Area Length Size (Ration %)"), AlignSpec.m_nCalc_Electrode_Area_Length_Ratio));

        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Body Cal. Param"),
                _T("Body Cal. Area Width Size (Ration %)"), AlignSpec.m_nCalc_Body_Area_Width_Ratio));
        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Body Cal. Param"),
                _T("Body Cal. Area Length Size (Ration %)"), AlignSpec.m_nCalc_Body_Area_Length_Ratio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Body Cal. Param"), _T("Body Cal. Area Offset X (um)"), AlignSpec.m_nCalc_Body_Area_GapX_um));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Body Cal. Param"), _T("Body Cal. Area Offset Y (um)"), AlignSpec.m_nCalc_Body_Area_GapY_um));

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Substrate Cal. Param"), _T("Substrate Cal. Area Size (um)"), AlignSpec.m_nCalc_Substrate_Area_Size_um));
        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Substrate Cal. Param"),
                _T("Substrate Cal. Area Offset X for Body (um)"), AlignSpec.m_nCalc_Substrate_Area_GapX_Body_um));
        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Substrate Cal. Param"),
                _T("Substrate Cal. Area Offset Y for Body (um)"), AlignSpec.m_nCalc_Substrate_Area_GapY_Body_um));
        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Substrate Cal. Param"),
                _T("Substrate Cal. Area Shift X (um)"), AlignSpec.m_nCalc_Substrate_Area_GapX_um));
        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Substrate Cal. Param"),
                _T("Substrate Cal. Area Shift Y (um)"), AlignSpec.m_nCalc_Substrate_Area_GapY_um));
    }

    strCategory.Format(_T("Align Spec"));
    std::vector<sPassive_InfoDB> vecPassiveInfoDB = GetPassiveInfoDB();
    for (long nPassiveidx = 0; nPassiveidx < (long)vecPassiveInfoDB.size(); nPassiveidx++)
    {
        strGroup.Format(_T("%d"), nPassiveidx + 1);
        auto CurPassiveInfoDB = vecPassiveInfoDB[nPassiveidx];

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, CurPassiveInfoDB.strCompName, _T(""), CurPassiveInfoDB.strSpecName));
    }

    return vecstrAlgorithmParameters;
}

void VisionInspectionComponent3DPara::SetPassiveInfoDB(const std::vector<sPassive_InfoDB> i_vecPassiveInfoDB)
{
    m_vecsTextRecipePassiveInfoDB.clear();

    m_vecsTextRecipePassiveInfoDB.insert(
        m_vecsTextRecipePassiveInfoDB.begin(), i_vecPassiveInfoDB.begin(), i_vecPassiveInfoDB.end());
}

std::vector<sPassive_InfoDB> VisionInspectionComponent3DPara::GetPassiveInfoDB()
{
    return m_vecsTextRecipePassiveInfoDB;
}