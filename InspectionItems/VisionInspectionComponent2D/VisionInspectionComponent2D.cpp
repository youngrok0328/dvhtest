//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionComponent2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionComponent2D.h"
#include "PassiveAlign.h"
#include "Result.h"
#include "VisionInspectionComponent2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NOT_USE_GROUP_INSPECTION 1

//CPP_7_________________________________ Implementation body
//
VisionInspectionComponent2D::VisionInspectionComponent2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_PASSIVE_2D, _VISION_INSP_NAME_PASSIVE_2D, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionComponent2DPara(*this))
    , m_bFrameInitalize(FALSE)
{
    m_result = new Result;
    m_pVisionInspDlg = NULL;

    m_nTotalResult = NOT_MEASURED;
    m_fCalcTime = 0.f;

    //{{ 검사 Item 설정. Item은 각각 Dialog와 연결시켜서 사용.
    m_pPassiveAlign = new CPassiveAlign(this);

    // Spec 를 미리 결정해서 생성자 부분에 하드코딩
    CreateSpec();

    m_DebugInfoGroup.Add(_T("PACKAGE BODY ROI"), enumDebugInfoType::PI_Rect);

    m_DebugInfoGroup.Add(_T("Spec Passive ROI"), enumDebugInfoType::FPI_Rect);

    //{{mc_DetailAlignDebug
    m_DebugInfoGroup.Add(_T("RoughAlign_Result"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Left_SearchROI"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Top_SearchROI"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Right_SearchROI"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Bottom_SearchROI"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Left_EdgePoints"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Top_EdgePoints"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Right_EdgePoints"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Bottom_EdgePoints"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Result"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Result_Horizontal"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Result_Vertical"), enumDebugInfoType::FPI_Rect);
    //m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Electrode"), enumDebugInfoType::FPI_Rect);
    //m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Body_Result"), enumDebugInfoType::FPI_Rect);

    m_DebugInfoGroup.Add(_T("DetailAlign_Passive_Shorten_SearchROI"), enumDebugInfoType::Rect);

    m_DebugInfoGroup.Add(_T("Passive Mask Image with Pad"), enumDebugInfoType::Image_8u_C1);

    //m_DebugInfoGroup.Add(_T("Ref Spec Line"), enumDebugInfoType::LineSeg_32f);
    //}}
}

void VisionInspectionComponent2D::ResetSpecAndPara()
{
    __super::ResetSpec();
}

VisionInspectionComponent2D::~VisionInspectionComponent2D(void)
{
    CloseDlg();

    if (m_pPassiveAlign)
    {
        delete m_pPassiveAlign;
        m_pPassiveAlign = NULL;
    }

    delete m_VisionPara;
    delete m_result;
}

void VisionInspectionComponent2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionComponent2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D), false, GetCurVisionModule_Status());
    else if (m_visionUnit.IsTheUsingVisionProcessingByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_visionUnit.RunInspection(m_visionUnit.GetVisionAlignProcessing(), false, GetCurVisionModule_Status());
    else
    {
        CString strError;
        strError.Format(_T("This module need execute the [%s] in the pre-process."),
            m_visionUnit.GetVisionProcessingModuleNameByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()));
        SimpleMessage(strError);
        strError.Empty();
    }

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionComponent2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionComponent2D::IDD, parent);

    //mc_이전결과의 성공여부를 떠나서 ChipList는 Update가 되어야한다.
    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->ShowImage();
    //m_pVisionInspDlg->SetInitParaWindow();
    MakePreInspInfo(true);

    return 0;
}

void VisionInspectionComponent2D::CreateSpec()
{
    m_defaultFixedInspectionSpecs.emplace_back(_T("{A144119C-B23D-4BEF-852C-C84545BB8C57}"),
        g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_MISSING], _T("PMISS"), _T("c"), _T("Num"),
        HostReportCategory::COMPONENT, II_RESULT_TYPE::COUNT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{11EBE754-EFB7-41AA-A507-2D1BF5E0DE15}"),
        g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_ROTATE], _T("PR"), _T("c"), _T("degree"),
        HostReportCategory::COMPONENT, II_RESULT_TYPE::DEGREE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{90BAFE10-E752-44AF-92B6-8030BDE5C62D}"),
        g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_OFFSET_X], _T("PdX"), _T("c"), _T("um"),
        HostReportCategory::COMPONENT, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{9A3F577D-DCA8-435B-A130-A289B7B74070}"),
        g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_OFFSET_Y], _T("PdY"), _T("c"), _T("um"),
        HostReportCategory::COMPONENT, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{EDCFEFD4-A846-4E75-805E-DB6D8C8C9D14}"),
        g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_WIDTH], _T("PTWD"), _T("c"), _T("um"),
        HostReportCategory::COMPONENT, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{4442A7FA-57E4-414F-BA4A-C4E95BF5430D}"),
        g_szPassive2DInspectionName[PASSIVE_INSPECTION_2D_LENGTH], _T("PTLN"), _T("c"), _T("um"),
        HostReportCategory::COMPONENT, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    __super::ResetSpec();

    UpdateSpec();
}

void VisionInspectionComponent2D::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    for (long nInsp = PASSIVE_INSPECTION_START; nInsp < PASSIVE_INSPECTION_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[nInsp].m_specName);
            if (pResult == nullptr)
                continue;
            if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                if (pResult->vecrtRejectROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtRejectROI, RGB(255, 0, 0));

                    if (m_fixedInspectionSpecs[nInsp].m_useMarginal && pResult->vecrtMarginalROI.size())
                    {
                        overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                    }
                }

                else if (pResult->vecrtMarginalROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                }
            }
        }
    }

    long variableSpecNum = (long)m_variableInspectionSpecs.size();
    for (long nInsp = 0; nInsp < variableSpecNum; nInsp++)
    {
        if (m_variableInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult
                = m_resultGroup.GetResultByName(m_variableInspectionSpecs[nInsp].m_specName);
            if (pResult == nullptr)
                continue;

            if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                if (pResult->vecrtRejectROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtRejectROI, RGB(255, 0, 0));

                    if (m_variableInspectionSpecs[nInsp].m_useMarginal && pResult->vecrtMarginalROI.size())
                    {
                        overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                    }
                }

                else if (pResult->vecrtMarginalROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                }
            }
        }
    }
}

void VisionInspectionComponent2D::AppendTextResult(CString& textResult)
{
    long nInspNum = (long)m_VisionPara->m_vecVisionInspectionSpecs.size();
    long nResultNum = (long)m_resultGroup.m_vecResult.size();
    if (nInspNum != nResultNum)
    {
        ASSERT(!(_T("Check the inspection item's number")));
        return;
    }

    std::vector<VisionInspectionResult*> vecpInspResult(nInspNum);
    std::vector<VisionInspectionSpec*> vecpInspSpec(nInspNum);
    std::vector<BOOL> vecbUseInsp(nInspNum);
    std::vector<CString> vecstrInspShortName(nInspNum);
    long nVisionModuleTotalResult = NOT_MEASURED;

    for (long nID = 0; nID < nInspNum; nID++)
    {
        vecpInspResult[nID] = m_resultGroup.GetResultByName(m_VisionPara->m_vecVisionInspectionSpecs[nID].m_specName);
        if (vecpInspResult[nID] == nullptr)
            ASSERT(!(_T("Check the inspection item's number")));
        vecpInspSpec[nID] = GetSpecByName(vecpInspResult[nID]->m_resultName);
        if (vecpInspSpec[nID] == nullptr)
            ASSERT(!(_T("Check the inspection item's number")));

        vecbUseInsp[nID] = -vecpInspSpec[nID]->m_use;
        vecstrInspShortName[nID] = vecpInspSpec[nID]->m_shortName;
        nVisionModuleTotalResult = (long)max(nVisionModuleTotalResult, vecpInspResult[nID]->m_totalResult);
    }

    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);
    textResult.AppendFormat(
        _T("    %s Total Result : %s\r\n"), (LPCTSTR)m_strModuleName, (LPCTSTR)Result2String(nVisionModuleTotalResult));

    for (long nID = 0; nID < nInspNum; nID++)
    {
        if (!vecbUseInsp[nID])
            continue;

        auto worstErrorValue = vecpInspResult[nID]->getObjectWorstErrorValue();

        if (worstErrorValue == Ipvm::k_noiseValue32r)
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ INV ])\r\n"),
                (LPCTSTR)vecpInspResult[nID]->m_resultName, (LPCTSTR)vecstrInspShortName[nID],
                (LPCTSTR)Result2String(vecpInspResult[nID]->m_totalResult),
                (LPCTSTR)vecpInspResult[nID]->getObjectWorstID());
        }
        else
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ %.2f ])\r\n"),
                (LPCTSTR)vecpInspResult[nID]->m_resultName, (LPCTSTR)vecstrInspShortName[nID],
                (LPCTSTR)Result2String(vecpInspResult[nID]->m_totalResult),
                (LPCTSTR)vecpInspResult[nID]->getObjectWorstID(), worstErrorValue);
        }
    }

    AppendDetailTextResult(textResult);

    for (int nIdx = 0; nIdx < vecstrInspShortName.size(); nIdx++)
    {
        vecstrInspShortName[nIdx].Empty();
    }
}

void VisionInspectionComponent2D::ResetResult()
{
    __super::ResetResult();

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();
}

BOOL VisionInspectionComponent2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{7D33B209-C81E-4C5F-A90D-115CE7343EDD}")]))
        return FALSE;

    if (!bSave)
    {
        m_VisionPara->m_specLink.Apply(m_packageSpec);
        MakePackageInfo(false, 0.f, Ipvm::Point32r2(0.f, 0.f));
        SetPackageInfo_SpecLink();
    }

    m_VisionPara->SetPassiveInfoDB(m_vecsPassiveInfoDB);

    UpdateSpec();

    return TRUE;
}

BOOL VisionInspectionComponent2D::SetPackageInfo_SpecLink()
{
    auto& specLink = m_VisionPara->m_specLink;

    // 해당 Chip에 연결된 Spec 연결.
    for (long i = 0; i < m_vecsPassiveInfoDB.size(); i++)
    {
        auto PassiveInfoDB = m_vecsPassiveInfoDB[i];

        auto& link_specName = specLink.GetSpecName(
            eComponentTypeDefine(m_vecsPassiveInfoDB[i].eComponentType), m_vecsPassiveInfoDB[i].strCompName);

        // 같은 Type의 Spec이 Job에 있어야 함. Job에 없다면 SpecDB에서 검색하여 얻어오도록 해야함.
        // 일단, SpecDB 연동은 나중에...

        bool bLinkSuccess(false);
        for (long index = 0; index < m_VisionPara->m_vecPassiveAlgoSpec.size(); index++)
        {
            auto& data = m_VisionPara->m_vecPassiveAlgoSpec[index];

            if (data.strSpecName == link_specName)
            {
                m_vecsPassiveInfoDB[i].strSpecName = link_specName;
                bLinkSuccess = true;
                break;
            }
        }

        if (bLinkSuccess == false)
            m_vecsPassiveInfoDB[i].strSpecName = _T("Default");
    }

    return TRUE;
}

void VisionInspectionComponent2D::VerifyFrame()
{
    //BOOL bPassiveDiff(FALSE);

    //Passive Algo에 등록된 Frame

    //for (long n = 0; n < m_VisionPara->m_vecPassiveAlgoSpec.size(); n++)
    //{
    //	if (m_VisionPara->m_vecPassiveAlgoSpec[n].AlignSpec.m_capAlignSpec.m_alignImageFrameIndex.getFrameIndex() < 0)
    //	{
    //		bPassiveDiff = TRUE;
    //	}

    //	if (m_VisionPara->m_vecPassiveAlgoSpec[n].AlignSpec.m_capAlignSpec.m_padAlignImageFrameIndex.getFrameIndex() < 0)
    //	{
    //		bPassiveDiff = TRUE;
    //	}
    //}

    //if (bPassiveDiff)
    //{
    //	for (long index = 0; index < m_VisionPara->m_vecPassiveAlgoSpec.size(); index++)
    //	{
    //		auto& data = m_VisionPara->m_vecPassiveAlgoSpec[index];

    //		data.AlignSpec.m_capAlignSpec.m_alignImageFrameIndex.setFrameListIndex(0);
    //		data.AlignSpec.m_capAlignSpec.m_alignSubImageFrameIndex.setFrameListIndex(0);
    //		data.AlignSpec.m_capAlignSpec.m_padAlignImageFrameIndex.setFrameListIndex(0);
    //	}

    //	::SimpleMessage(_T("Passive Algorithm Frame has been initialized Please Setup Frame"));
    //}

    //if (bPassiveDiff) //4개중 뭐하나라도 TRUE라면 모두 Invalid때린다.
    //	m_bFrameInitalize = TRUE;
    //else
    //	m_bFrameInitalize = FALSE;
}

void VisionInspectionComponent2D::UpdateSpec()
{
    //kk 이전 버전 사용 Job들을 아예 다시 셋업 하면 상관이 없는데
    //이전 버전 Job을 그대로 사용할 때, GroupName이 만들어지지 않는게 문제
    //검사 항목 마다 초기화 하는 위치가 달라 한번 더 체크하도록 수정
    //mc_무슨이유인지는 모르겠는데 m_vecstrGroupInspName에 있는 Name이랑 m_vecsPassiveTypes에 존재하는 Name이랑 다름.. 예외처리 필요

    while (m_VisionPara->m_vecstrGroupInspName.size() != m_vecsPassiveTypes.size())
    {
        if (m_VisionPara->m_vecstrGroupInspName.size() < m_vecsPassiveTypes.size())
        {
            long index = long(m_VisionPara->m_vecstrGroupInspName.size());
            Component2DGroupInsp Data;

            Data.SetName(m_vecsPassiveTypes[index]);

            m_VisionPara->m_vecstrGroupInspName.push_back(Data);
        }
        else
        {
            m_VisionPara->m_vecstrGroupInspName.erase(m_VisionPara->m_vecstrGroupInspName.end() - 1);
        }
    }

    const long nGroupInspNameNum = (long)m_VisionPara->m_vecstrGroupInspName.size();
    const long nPassiveTypeNum = (long)m_vecsPassiveTypes.size();

    std::vector<CString> vecstrExistCompSpecName(0);
    std::vector<long> vecnChangeCompSpecNameidx(0);
    std::vector<CString> vecstrChangeCompSpecName(0);
    for (long nGroupInspidx = 0; nGroupInspidx < nGroupInspNameNum; nGroupInspidx++)
    {
        auto GroupInspName = m_VisionPara->m_vecstrGroupInspName[nGroupInspidx].strCompSpec;
        vecstrExistCompSpecName.push_back(GroupInspName);

        if (std::find(m_vecsPassiveTypes.begin(), m_vecsPassiveTypes.end(), GroupInspName) != m_vecsPassiveTypes.end()
            == false)
            vecnChangeCompSpecNameidx.push_back(nGroupInspidx);
    }

    for (long nPassiveTypeidx = 0; nPassiveTypeidx < nPassiveTypeNum; nPassiveTypeidx++)
    {
        auto PassiveTypeName = m_vecsPassiveTypes[nPassiveTypeidx];

        if (std::find(vecstrExistCompSpecName.begin(), vecstrExistCompSpecName.end(), PassiveTypeName)
            != vecstrExistCompSpecName.end() == false)
            vecstrChangeCompSpecName.push_back(PassiveTypeName);
    }

    for (long nChangeCompSpecidx = 0; nChangeCompSpecidx < vecnChangeCompSpecNameidx.size(); nChangeCompSpecidx++)
    {
        auto ChangeSpecidx = vecnChangeCompSpecNameidx[nChangeCompSpecidx];
        auto ChangeSpecName = vecstrChangeCompSpecName[nChangeCompSpecidx];

        //개수는 같다
        m_VisionPara->m_vecstrGroupInspName[ChangeSpecidx].SetName(ChangeSpecName);
    }

    long SpecNum = (long)m_variableInspectionSpecs.size();
    long DefaultNum = (long)m_defaultFixedInspectionSpecs.size() - NOT_USE_GROUP_INSPECTION;
    long MakeSpecNum = ((long)m_VisionPara->m_vecstrGroupInspName.size() * DefaultNum);

    while (SpecNum < MakeSpecNum)
    {
        long Index = SpecNum;

        if (Index < 0)
            return;

        CString strSpecInital;
        strSpecInital.Format(_T("CUCP_G%d"), Index);
        m_variableInspectionSpecs.emplace_back(m_moduleGuid, ::CreateGUID(), m_strModuleName, _T(""), strSpecInital,
            _T("C"), _T("um"), HostReportCategory::COMPONENT, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f,
            FALSE, FALSE, FALSE);

        SpecNum = long(m_variableInspectionSpecs.size());
    }

    while (SpecNum > MakeSpecNum)
    {
        m_variableInspectionSpecs.erase(m_variableInspectionSpecs.end() - 1);

        SpecNum = long(m_variableInspectionSpecs.size());
    }

    std::vector<CString> InspectionName;

    if (m_vecsPassiveTypes.size() != 1)
    {
        if (m_VisionPara->m_vecstrGroupInspName.size() != 0)
        {
            for (int i = 0; i < long(m_vecsPassiveTypes.size()); i++)
            {
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].strCompRotateName);
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].strCompShiftWidthName);
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].strCompShiftLengthName);
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].strCompWidthName);
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].strCompLengthName);
            }
        }
    }
    else if (long(m_vecsPassiveTypes.size())
        <= 1) //kk Ball Type이 한개 이하일 경우 필요 없습니다. 다시 호출해서 설정합시다.
    {
        m_variableInspectionSpecs.clear();
        m_VisionPara->m_vecstrGroupInspName.clear();
        SpecNum = 0;
    }

    for (long idx = 0; idx < long(SpecNum); idx++)
    {
        CString specName;
        specName.Format(_T("CIGS_%d"), idx);

        m_variableInspectionSpecs[idx].m_specName = InspectionName[idx];
        m_variableInspectionSpecs[idx].m_shortName = specName;

        CString strUnit = _T("um");

        if (InspectionName[idx].Find(_T("ROTATE")) != -1)
        {
            m_variableInspectionSpecs[idx].m_resultType = II_RESULT_TYPE::DEGREE;
            strUnit = _T("degree");
        }

        m_variableInspectionSpecs[idx].m_unit = strUnit;

        m_variableInspectionSpecs[idx].MakeSpecFullName();
    }

    GetVisionInspectionSpecs();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->ResetSpecAndResultDlg();
    }
}

void VisionInspectionComponent2D::GetVisionInspectionSpecs()
{
    m_VisionPara->m_vecVisionInspectionSpecs.clear();

    for (auto& spec : m_fixedInspectionSpecs)
    {
        m_VisionPara->m_vecVisionInspectionSpecs.push_back(spec);
    }

    for (auto& spec : m_variableInspectionSpecs)
    {
        m_VisionPara->m_vecVisionInspectionSpecs.push_back(spec);
    }
}

void VisionInspectionComponent2D::SetVisionInspectionSpecs()
{
    for (auto& spec : m_VisionPara->m_vecVisionInspectionSpecs)
    {
        for (auto& item : m_fixedInspectionSpecs)
        {
            if (item.m_specGuid == spec.m_specGuid)
            {
                item = spec;
            }
        }

        for (auto& item : m_variableInspectionSpecs)
        {
            if (item.m_specGuid == spec.m_specGuid)
            {
                item = spec;
            }
        }
    }
}

std::vector<CString> VisionInspectionComponent2D::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);
    std::vector<CString> vecstrInspectionSpec = ExportInspectionSpecToText(strVisionName);

    vecStrResult.insert(vecStrResult.end(), vecstrInspectionSpec.begin(), vecstrInspectionSpec.end());

    return vecStrResult;
}