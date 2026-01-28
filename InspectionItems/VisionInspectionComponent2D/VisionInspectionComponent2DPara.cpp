//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionComponent2DPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
AlgorithmSpec::AlgorithmSpec()
    : strSpecName(_T("Default"))
{
}

BOOL AlgorithmSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("SpecName")].Link(bSave, strSpecName);

    return TRUE;
}

PassiveAlignSpec::PassiveAlignSpec(VisionProcessing& parent)
    : m_capAlignSpec(parent)
{
}

PassiveAlignSpec::CapAlignSpec::CapAlignSpec(VisionProcessing& parent)
    : m_Rough_align_ImageFrameIndex(parent, FALSE)
    , m_nRougAlign_SearchOffsetX_um(100)
    , m_nRougAlign_SearchOffsetY_um(100)
    , m_bRoughAlign_ElectrodeColor(BRIGHT)
    , m_bRoughAlign_OnlyUse(FALSE)

    , m_Outline_align_Electrode_ImageFrameIndex(parent, FALSE)
    , m_nOutline_align_Electrode_Search_Width_Ratio(80)
    , m_nOutline_align_Electrode_Search_Length_um(150)
    , m_nOutline_align_Electrode_ElectrodeColor(BRIGHT)
    , m_nOutline_align_Electrode_EdgeSearch_Direction(PI_ED_DIR_OUTER)
    , m_fOutline_align_Electrode_EdgeThreshold(5.)
    , m_nOutline_align_Electrode_Edge_Detect_Mode(PI_ED_MAX_EDGE)

    , m_Outline_align_Body_ImageFrameIndex(parent, FALSE)
    , m_nOutline_align_Body_Search_Width_Ratio(50)
    , m_nOutline_align_Body_Search_Length_um(200)
    , m_nOutline_align_Body_Search_Count_Gap(10)
    , m_nOutline_align_Body_ElectrodeColor(BRIGHT)
    , m_nOutline_align_Body_EdgeSearch_Direction(PI_ED_DIR_OUTER)
    , m_fOutline_align_Body_EdgeThreshold(3.)
    , m_nOutline_align_Body_Edge_Detect_Mode(PI_ED_MAX_EDGE)

    , m_padUseAlign(FALSE)
    , m_padAlignImageFrameIndex(parent, FALSE)
    , m_fPadSearchROI_Width_Ratio(40.f)
    , m_fPadSearchROI_Length_Ratio(40.f)

    , m_bUse2ndInspection(FALSE)
    , m_str2ndInspCode(_T(""))
{
}

BOOL PassiveAlignSpec::CapAlignSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    //RoughAlign_Param
    m_Rough_align_ImageFrameIndex.LinkDataBase(bSave, db[_T("Rough_Align_ImageIndex")]);
    if (!db[_T("Rough_Align_SearchOffsetX_um")].Link(bSave, m_nRougAlign_SearchOffsetX_um))
        m_nRougAlign_SearchOffsetX_um = 100;
    if (!db[_T("Rough_Align_SearchOffsetY_um")].Link(bSave, m_nRougAlign_SearchOffsetY_um))
        m_nRougAlign_SearchOffsetY_um = 100;
    if (!db[_T("Rough_Align_Electrode_Color")].Link(bSave, m_bRoughAlign_ElectrodeColor))
        m_bRoughAlign_ElectrodeColor = BRIGHT;
    if (!db[_T("Rough_Align_Only_RoughAlign_Use")].Link(bSave, m_bRoughAlign_OnlyUse))
        m_bRoughAlign_OnlyUse = FALSE;

    //Outline Align Electrode Param
    if (!m_Outline_align_Electrode_ImageFrameIndex.LinkDataBase(bSave, db[_T("Outline_Align_Electrode_ImageIndex")]))
    {
        if (!m_Outline_align_Electrode_ImageFrameIndex.LinkDataBase(
                bSave, db[_T("Outline_Align_Electorde_ImageIndex")]))
        {
        }
    }
    if (!db[_T("Outline_Align_Electorde_Search_Width_Ratio")].Link(
            bSave, m_nOutline_align_Electrode_Search_Width_Ratio))
    {
        if (!db[_T("Outline_Align_Electrode_Search_Width_Ratio")].Link(
                bSave, m_nOutline_align_Electrode_Search_Width_Ratio))
            m_nOutline_align_Electrode_Search_Width_Ratio = 80;
    }
    if (!db[_T("Outline_Align_Electorde_Search_Length_um")].Link(bSave, m_nOutline_align_Electrode_Search_Length_um))
    {
        if (!db[_T("Outline_Align_Electrode_Search_Length_um")].Link(
                bSave, m_nOutline_align_Electrode_Search_Length_um))
            m_nOutline_align_Electrode_Search_Length_um = 150;
    }
    if (!db[_T("Outline_Align_Electorde_Electrode_Color")].Link(bSave, m_nOutline_align_Electrode_ElectrodeColor))
    {
        if (!db[_T("Outline_Align_Electrode_Electrode_Color")].Link(bSave, m_nOutline_align_Electrode_ElectrodeColor))
            m_nOutline_align_Electrode_ElectrodeColor = BRIGHT;
    }
    if (!db[_T("Outline_Align_Electorde_EdgeSearch_Direction")].Link(
            bSave, m_nOutline_align_Electrode_EdgeSearch_Direction))
    {
        if (!db[_T("Outline_Align_Electrode_EdgeSearch_Direction")].Link(
                bSave, m_nOutline_align_Electrode_EdgeSearch_Direction))
            m_nOutline_align_Electrode_EdgeSearch_Direction = PI_ED_DIR_OUTER;
    }
    if (!db[_T("Outline_Align_Electorde_EdgeThreshold")].Link(bSave, m_fOutline_align_Electrode_EdgeThreshold))
    {
        if (!db[_T("Outline_Align_Electrode_EdgeThreshold")].Link(bSave, m_fOutline_align_Electrode_EdgeThreshold))
            m_fOutline_align_Electrode_EdgeThreshold = 5.;
    }
    if (!db[_T("Outline_Align_Electorde_Edge_Detect_Mode")].Link(bSave, m_nOutline_align_Electrode_Edge_Detect_Mode))
    {
        if (!db[_T("Outline_Align_Electorde_Edge_Detect_Mode")].Link(
                bSave, m_nOutline_align_Electrode_Edge_Detect_Mode))
            m_nOutline_align_Electrode_Edge_Detect_Mode = PI_ED_MAX_EDGE;
    }

    //Outline Align Body Param
    m_Outline_align_Body_ImageFrameIndex.LinkDataBase(bSave, db[_T("Outline_Align_Body_ImageIndex")]);
    if (!db[_T("Outline_Align_Body_Search_Width_Ratio")].Link(bSave, m_nOutline_align_Body_Search_Width_Ratio))
        m_nOutline_align_Body_Search_Width_Ratio = 50;
    if (!db[_T("Outline_Align_Body_Search_Length_um")].Link(bSave, m_nOutline_align_Body_Search_Length_um))
        m_nOutline_align_Body_Search_Length_um = 200;
    if (!db[_T("Outline_Align_Body_Search_Count_Gap")].Link(bSave, m_nOutline_align_Body_Search_Count_Gap))
        m_nOutline_align_Body_Search_Count_Gap = 10;
    if (!db[_T("Outline_Align_Body_Electrode_Color")].Link(bSave, m_nOutline_align_Body_ElectrodeColor))
        m_nOutline_align_Body_ElectrodeColor = BRIGHT;
    if (!db[_T("Outline_Align_Body_EdgeSearch_Direction")].Link(bSave, m_nOutline_align_Body_EdgeSearch_Direction))
        m_nOutline_align_Body_EdgeSearch_Direction = PI_ED_DIR_OUTER;
    if (!db[_T("Outline_Align_Body_EdgeThreshold")].Link(bSave, m_fOutline_align_Body_EdgeThreshold))
        m_fOutline_align_Body_EdgeThreshold = 3.;
    if (!db[_T("Outline_Align_Body_Edge_Detect_Mode")].Link(bSave, m_nOutline_align_Body_Edge_Detect_Mode))
        m_nOutline_align_Body_Edge_Detect_Mode = PI_ED_MAX_EDGE;

    if (!db[_T("PadUseAlign")].Link(bSave, m_padUseAlign))
        m_padUseAlign = FALSE;
    m_padAlignImageFrameIndex.LinkDataBase(bSave, db[_T("PadAlignImageIndex")]);
    if (!db[_T("PadAlign_Width_ROIRatio")].Link(bSave, m_fPadSearchROI_Width_Ratio))
        m_fPadSearchROI_Width_Ratio = 40.f;
    if (!db[_T("PadAlign_Length_ROIRatio")].Link(bSave, m_fPadSearchROI_Length_Ratio))
        m_fPadSearchROI_Length_Ratio = 40.f;

    if (!db[_T("bUse2ndInspection")].Link(bSave, m_bUse2ndInspection))
        m_bUse2ndInspection = FALSE;
    if (!db[_T("str2ndInspCode")].Link(bSave, m_str2ndInspCode))
        m_str2ndInspCode = _T("");

    return TRUE;
}

BOOL PassiveAlignSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    m_capAlignSpec.LinkDataBase(bSave, db[_T("{4D129AD6-A37C-4593-90A5-484A36FA0DCA}")]);

    return TRUE;
}

PassiveAlgorithmSpec::PassiveAlgorithmSpec(VisionProcessing& parent)
    : AlignSpec(parent)
{
}

BOOL PassiveAlgorithmSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    __super::LinkDataBase(bSave, db);

    AlignSpec.LinkDataBase(bSave, db[_T("{DDA64FDA-C2A5-4128-B28C-C1DD10FD63B4}")]);

    return TRUE;
}

CSpecLinkPara::CSpecLinkPara(void)
{
}

CSpecLinkPara::~CSpecLinkPara(void)
{
}

void CSpecLinkPara::Apply(const CPackageSpec& spec)
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

    for (auto& object : spec.vecDMSChip)
        erase_list[eComponentTypeDefine::tyCHIP].erase(object.strCompType);
    for (auto& object : spec.vecDMSPassive)
        erase_list[eComponentTypeDefine::tyPASSIVE].erase(object.strCompType);
    for (auto& object : spec.vecDMSArray)
        erase_list[eComponentTypeDefine::tyARRAY].erase(object.strCompType);
    for (auto& object : spec.vecDMSHeatsink)
        erase_list[eComponentTypeDefine::tyHEATSINK].erase(object.strCompType);

    for (long type = 0; type < 5; type++)
    {
        auto& info_type = m_infos[type];

        for (auto& eraseInfo : erase_list[type])
        {
            info_type.erase(eraseInfo.first);
        }
    }
}

BOOL CSpecLinkPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    for (long type = 0; type < 5; type++)
    {
        CString keyName;
        switch (type)
        {
            case eComponentTypeDefine::tyCHIP:
                keyName = _T("Chip");
                break;
            case eComponentTypeDefine::tyPASSIVE:
                keyName = _T("Passive");
                break;
            case eComponentTypeDefine::tyARRAY:
                keyName = _T("Array");
                break;
            case eComponentTypeDefine::tyHEATSINK:
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

        keyName.Empty();
    }

    return TRUE;
}

CString& CSpecLinkPara::GetSpecName(long chipType, LPCTSTR chipName)
{
    auto itFind = m_infos[chipType].find(chipName);
    if (itFind != m_infos[chipType].end())
    {
        return itFind->second;
    }

    m_infos[chipType][chipName] = _T("Default");
    return m_infos[chipType][chipName];
}

VisionInspectionComponent2DPara::VisionInspectionComponent2DPara(VisionProcessing& parent)
    : m_bAlgoDB(FALSE)
    , m_vecPassiveAlgoSpec(parent, 1)
{
    Init();
}

VisionInspectionComponent2DPara::~VisionInspectionComponent2DPara(void)
{
}

void VisionInspectionComponent2DPara::Init()
{
    m_bAlgoDB = FALSE;

    m_vecPassiveAlgoSpec.clear();
    m_vecPassiveAlgoSpec.resize(1);
    m_vecVisionInspectionSpecs.clear();

    m_vecsTextRecipePassiveInfoDB.clear(); //mc_TextRecipe
}

BOOL VisionInspectionComponent2DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        Init();
    }

    // m_vecPassiveAlgoSpec
    {
        long count = long(m_vecPassiveAlgoSpec.size());

        db[_T("{5DFFD4AA-BA00-4F84-8BBF-2A43FD92FBA4}")].Link(bSave, count);

        m_vecPassiveAlgoSpec.resize(count);

        for (long n = 0; n < count; n++)
        {
            m_vecPassiveAlgoSpec[n].LinkDataBase(
                bSave, db.GetSubDBFmt(_T("{5DFFD4AA-BA00-4F84-8BBF-2A43FD92FBA4}_%d"), n));
        }
    }

    m_specLink.LinkDataBase(bSave, db[_T("{944789AF-D717-41C8-8BA4-54581C417095}")]);

    long nSize = 0;

    if (bSave)
    {
        nSize = CAST_LONG(m_vecstrGroupInspName.size());

        db[_T("{2CFF2412-D40E-4330-829F-5AC56E098C92}")].Link(bSave, nSize);
    }
    else
    {
        db[_T("{2CFF2412-D40E-4330-829F-5AC56E098C92}")].Link(bSave, nSize);

        m_vecstrGroupInspName.resize(nSize);
    }

    for (long idx = 0; idx < nSize; idx++)
    {
        m_vecstrGroupInspName[idx].LinkDataBase(
            bSave, db.GetSubDBFmt(_T("{6E0EE7BC-EAEA-4A5D-93FB-FEC313DAA106}_%d"), idx));
    }

    return TRUE;
}

void Component2DGroupInsp::SetName(LPCTSTR compSpec)
{
    strCompSpec = compSpec;

    strCompRotateName.Format(_T("(%s) %s"), compSpec, g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_ROTATE]);
    strCompShiftWidthName.Format(_T("(%s) %s"), compSpec, g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_OFFSET_Y]);
    strCompShiftLengthName.Format(_T("(%s) %s"), compSpec, g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_OFFSET_X]);
    strCompWidthName.Format(_T("(%s) %s"), compSpec, g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_WIDTH]);
    strCompLengthName.Format(_T("(%s) %s"), compSpec, g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_LENGTH]);
}

BOOL Component2DGroupInsp::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{C30E188E-B82C-4537-98A1-D3EF008D18E1}")].Link(bSave, strCompRotateName))
        strCompRotateName = _T("Untitle");
    if (!db[_T("{E3230759-6043-4C44-9C19-773B825427F6}")].Link(bSave, strCompShiftWidthName))
        strCompShiftWidthName = _T("Untitle");
    if (!db[_T("{C7D02EF4-4597-4A34-B711-8613113B48D4}")].Link(bSave, strCompShiftLengthName))
        strCompShiftLengthName = _T("Untitle");
    if (!db[_T("{0CE50165-65BC-45E3-BF23-A314D26D086D}")].Link(bSave, strCompWidthName))
        strCompWidthName = _T("Untitle");
    if (!db[_T("{46B6D056-464F-40ED-9D3F-7360B8C8DB1A}")].Link(bSave, strCompLengthName))
        strCompLengthName = _T("Untitle");

    if (!db[_T("{93BF5023-4144-41FB-B56A-1E38CD10F1EA}")].Link(bSave, strCompSpec))
        strCompSpec = _T("Unknown Type");

    return TRUE;
}

std::vector<CString> VisionInspectionComponent2DPara::ExportAlgoParaToText(
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
    //strCategory.Format(_T("Algorithm Parameter"));
    strCategory.Format(_T("AlgorithmDB"));
    strGroup.Format(_T("2D Passive Paramter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Algorithm DB Count"), _T(""), (long)m_vecPassiveAlgoSpec.size()));

    //strCategory.Format(_T("AlgorithmDB"));
    for (long nAlgoSpecidx = 0; nAlgoSpecidx < (long)m_vecPassiveAlgoSpec.size(); nAlgoSpecidx++)
    {
        strGroup.Format(_T("%s"), (LPCTSTR)m_vecPassiveAlgoSpec[nAlgoSpecidx].strSpecName);
        auto& AlignSpec = m_vecPassiveAlgoSpec[nAlgoSpecidx].AlignSpec.m_capAlignSpec;

        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Rough Align Parameters"),
                _T("Rough Align Image"), AlignSpec.m_Rough_align_ImageFrameIndex.getFrameIndex()));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Rough Align Parameters"), _T("Search Offset X (um)"), AlignSpec.m_nRougAlign_SearchOffsetX_um));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Rough Align Parameters"), _T("Search Offset Y (um)"), AlignSpec.m_nRougAlign_SearchOffsetY_um));
        vecstrAlgorithmParameters.push_back(
            CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, _T("Rough Align Parameters"),
                _T("Electorde Color (contrast to substrate)"), AlignSpec.m_bRoughAlign_ElectrodeColor));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Rough Align Parameters"), _T("Only Use Rough Align"), (bool)AlignSpec.m_bRoughAlign_OnlyUse));

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Outline Align(Electrode) Image"),
            AlignSpec.m_Outline_align_Electrode_ImageFrameIndex.getFrameIndex()));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Search Width (Passive Width Ration %)"),
            AlignSpec.m_nOutline_align_Electrode_Search_Width_Ratio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Search Length (um)"),
            AlignSpec.m_nOutline_align_Electrode_Search_Length_um));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Electorde Color (contrast to substrate)"),
            AlignSpec.m_nOutline_align_Electrode_ElectrodeColor));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Edge Search Direction"),
            AlignSpec.m_nOutline_align_Electrode_EdgeSearch_Direction));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Edge Threshold"),
            AlignSpec.m_fOutline_align_Electrode_EdgeThreshold));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Electrode"), _T("Electrode Edge Detect Mode"),
            AlignSpec.m_nOutline_align_Electrode_Edge_Detect_Mode));

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Outline Align(Body) Image"),
            AlignSpec.m_Outline_align_Body_ImageFrameIndex.getFrameIndex()));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Search Width (Passive Width Ration %)"),
            AlignSpec.m_nOutline_align_Body_Search_Width_Ratio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Search Length (um)"),
            AlignSpec.m_nOutline_align_Body_Search_Length_um));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Search Count"),
            AlignSpec.m_nOutline_align_Body_Search_Count_Gap));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Body Color (contrast to substrate)"),
            AlignSpec.m_nOutline_align_Body_ElectrodeColor));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Edge Search Direction"),
            AlignSpec.m_nOutline_align_Body_EdgeSearch_Direction));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Edge Threshold"),
            AlignSpec.m_fOutline_align_Body_EdgeThreshold));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Outline Align Parameters for Body"), _T("Body Edge Detect Mode"),
            AlignSpec.m_nOutline_align_Body_Edge_Detect_Mode));

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("2nd Inspection Parameter (Deep Learning)"), _T("Use Inspection"), (bool)AlignSpec.m_bUse2ndInspection));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("2nd Inspection Parameter (Deep Learning)"), _T("DL Model Match Code"), AlignSpec.m_str2ndInspCode));
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

void VisionInspectionComponent2DPara::SetPassiveInfoDB(const std::vector<sPassive_InfoDB> i_vecPassiveInfoDB)
{
    m_vecsTextRecipePassiveInfoDB.clear();

    m_vecsTextRecipePassiveInfoDB.insert(
        m_vecsTextRecipePassiveInfoDB.begin(), i_vecPassiveInfoDB.begin(), i_vecPassiveInfoDB.end());
}

std::vector<sPassive_InfoDB> VisionInspectionComponent2DPara::GetPassiveInfoDB()
{
    return m_vecsTextRecipePassiveInfoDB;
}