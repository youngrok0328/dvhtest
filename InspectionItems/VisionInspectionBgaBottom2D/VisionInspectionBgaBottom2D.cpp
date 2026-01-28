//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBottom2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBottom2D.h"
#include "Result.h"
#include "VisionInspectionBgaBottom2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
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
VisionInspectionBgaBottom2D::VisionInspectionBgaBottom2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_BGA_BALL_2D, _VISION_INSP_NAME_BGA_BALL_2D, visionUnit, packageSpec)
    , m_pVisionPara(new VisionInspectionBgaBottom2DPara(*this))
    , m_sBallAlignResult(new VisionAlignResult)
    , m_sEdgeAlignResult(new VisionAlignResult)
    , m_pfptGetCenterPosbyPadAlign(nullptr)
{
    m_result = new Result;
    m_pVisionInspDlg = NULL;

    m_pEdgeDetect = NULL;

    m_f2DScale = 0.0f;

    long GroupCount = long(m_packageSpec.m_originalballMap->m_ballTypes.size());

    m_nWidthSweepLineCount.resize(GroupCount);

    for (int i = 0; i < GroupCount; i++)
    {
        m_nWidthSweepLineCount[i] = FAST_WIDTH_SWEEP_LINE_COUNT; //kircheis_BKShiny
    }

    m_defaultFixedInspectionSpecs.emplace_back(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_MISSING], _T("bMIS"), _T("B"), _T("Num"), HostReportCategory::BALL,
        II_RESULT_TYPE::COUNT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{9E0D268F-255F-4426-A23A-4BA30868F3BF}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_OFFSET_X], _T("bdX"), _T("B"), _T("um"), HostReportCategory::BALL,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{2D65C8A7-1262-4B8D-898F-EAAAF36AE74C}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_OFFSET_Y], _T("bdY"), _T("B"), _T("um"), HostReportCategory::BALL,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{54A8FCCC-E2D4-4CA1-AC73-529E2B0ADC11}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_OFFSET_R], _T("bR"), _T("B"), _T("um"), HostReportCategory::BALL,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{BF685BFE-6B33-413F-AA20-0C6293918094}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_GRID_OFFSET_X], _T("gDX"), _T("B"), _T("um"),
        HostReportCategory::BALL, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{3DE9EEC1-80AE-4377-803F-34F98DA31929}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_GRID_OFFSET_Y], _T("gDY"), _T("B"), _T("um"),
        HostReportCategory::BALL, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{C85EC851-C1E7-4E52-B4A2-D31D7BA555D5}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_WIDTH], _T("bWIT"), _T("B"), _T("um"), HostReportCategory::BALL,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{247146DD-08C9-4884-A72F-40CF6A43F0C1}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_QUALITY], _T("bQUA"), _T("B"), _T("%"), HostReportCategory::BALL,
        II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{60ABB038-2A63-4503-87C0-A5D648D189BC}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_CONTRAST], _T("bCON"), _T("B"), _T("GV"), HostReportCategory::BALL,
        II_RESULT_TYPE::GRAY_VALUE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{C8F9D04D-8B80-406A-9414-7A4875DF3BBC}"),
        g_szBallInspectionName[BALL_INSPECTION_BALL_ELLIPTICITY], _T("bELP"), _T("B"), _T("%"),
        HostReportCategory::BALL, II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 5.f, 0.f, FALSE, FALSE,
        FALSE); //kircheis_TMI

    __super::ResetSpec();

    UpdateSpec();

    m_DebugInfoGroup.Add(_T("Ball Spec Point"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("Ball Missing ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Ball Contrast Value"), enumDebugInfoType::Float);
    m_DebugInfoGroup.Add(_T("Ball Rough Width Edge Sweep Line"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("Ball Rough Width Edge Total Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Ball Rough Align Circle"), enumDebugInfoType::EllipseEq_32f); //kircheis_MED3
    m_DebugInfoGroup.Add(_T("Ball: Rough Ball Peak Search Circle"), enumDebugInfoType::EllipseEq_32f); //kircheis_LKF
    m_DebugInfoGroup.Add(_T("Ball: Rough Ball Peak Threshold Image"), enumDebugInfoType::Image_8u_C1); //kircheis_LKF
    m_DebugInfoGroup.Add(_T("Ball: Rough Ball Peak Center"), enumDebugInfoType::Point_32f_C2); //kircheis_LKF
    m_DebugInfoGroup.Add(_T("Ball Width Edge Sweep Line"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("Ball Width Edge Total Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Ball Width Edge Valid Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Ball Width Re-Search Area"), enumDebugInfoType::EllipseEq_32f); //kircheis_BallFlux
    m_DebugInfoGroup.Add(_T("Ball Width Re-Edge Total Point"), enumDebugInfoType::Point_32f_C2); //kircheis_BallFlux
    m_DebugInfoGroup.Add(_T("Ball Width Re-Edge Valid Point"), enumDebugInfoType::Point_32f_C2); //kircheis_BallFlux
    m_DebugInfoGroup.Add(_T("Ball Width ROI"), enumDebugInfoType::Rect_32f);
    m_DebugInfoGroup.Add(_T("Ball Width Circle"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("Ball Mask Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Ball Blob ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Ball Quality Sweep Line"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("Ball Quality Check Pair Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Ball Coaxial Blob ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Ball Squash Edge Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Ball Align Result by Ball Insp"), enumDebugInfoType::None);
    m_DebugInfoGroup.Add(_T("Ball Ellipticity Sweep Line"), enumDebugInfoType::LineSeg_32f); //kircheis_TMI
    m_DebugInfoGroup.Add(_T("Ball Ellipticity Edge Point"), enumDebugInfoType::Point_32f_C2); //kircheis_TMI
    m_DebugInfoGroup.Add(_T("Ball Ellipticity Edge Point Circle"), enumDebugInfoType::EllipseEq_32f); //kircheis_TMI
}

void VisionInspectionBgaBottom2D::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_pVisionPara->Init();
}

VisionInspectionBgaBottom2D::~VisionInspectionBgaBottom2D(void)
{
    CloseDlg();

    delete m_sEdgeAlignResult;
    delete m_sBallAlignResult;
    delete m_pVisionPara;
    delete m_result;
}

void VisionInspectionBgaBottom2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionBgaBottom2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
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

    // 티칭에서 사용할거라서 필요함.
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionBgaBottom2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionBgaBottom2D::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionInspectionBgaBottom2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_pVisionPara->LinkDataBase(bSave, db[_T("{920B73DF-28FB-4253-84AA-BF5F7AC368AB}")]))
        return FALSE;

    //kk 이 코드를 호출할 위치가 마땅치 않아 일단 여기에서 호출
    //Package Layout에서 MapData 수정 후 Land 3D 검사 들어갈 시 이전 MapData의 정보들을 들고 들어가는 문제로 인해
    //LinkDataBase에서 Data를 초기화 해주는 역할을 넣어놓음
    long nLayerNum = long(m_packageSpec.m_originalballMap->m_ballTypes.size());
    long SpecNum = long(m_pVisionPara->m_vecstrGroupInspName.size());

    if (nLayerNum < 2)
    {
        m_pVisionPara->m_vecstrGroupInspName.resize(0);
    }
    else
    {
        if (SpecNum != nLayerNum)
        {
            m_pVisionPara->m_vecstrGroupInspName.resize(nLayerNum);

            for (int i = 0; i < nLayerNum; i++)
            {
                auto group_id = m_packageSpec.m_originalballMap->m_ballTypes[i].m_groupID;
                m_pVisionPara->m_vecstrGroupInspName[i].SetName(group_id);
            }
        }
    }

    UpdateSpec();

    return TRUE;
}

std::vector<CString> VisionInspectionBgaBottom2D::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_pVisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);
    std::vector<CString> vecstrInspectionSpec = ExportInspectionSpecToText(strVisionName);

    vecStrResult.insert(vecStrResult.end(), vecstrInspectionSpec.begin(), vecstrInspectionSpec.end());

    return vecStrResult;
}

// 영훈 20141218_BallOverlay : 검사 시 ball 불량 시 화면에 표시해주도록 한다.
void VisionInspectionBgaBottom2D::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    for (long nInsp = BALL_INSPECTION_START; nInsp < BALL_INSPECTION_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            auto* pResult = m_resultGroup.GetResultByName(g_szBallInspectionName[nInsp]);
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

    long variableNum = (long)m_variableInspectionSpecs.size();
    for (long nInsp = 0; nInsp < variableNum; nInsp++)
    {
        if (m_variableInspectionSpecs[nInsp].m_use)
        {
            auto* pResult = m_resultGroup.GetResultByName(m_variableInspectionSpecs[nInsp].m_specName);
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

void VisionInspectionBgaBottom2D::AppendTextResult(CString& textResult)
{
    long fixedNum = (long)m_fixedInspectionSpecs.size();
    long variableNum = (long)m_variableInspectionSpecs.size();
    long nInspNum = fixedNum + variableNum;
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
        if (nID < fixedNum)
            vecpInspResult[nID] = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[nID].m_specName);
        else
            vecpInspResult[nID] = m_resultGroup.GetResultByName(m_variableInspectionSpecs[nID - fixedNum].m_specName);

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

    long nObjectTotalNum = (long)m_packageSpec.m_ballMap->m_balls.size(); //Parameters for this module only.

    AppendDetailTextResult(textResult, nObjectTotalNum);

    for (int nIdx = 0; nIdx < vecstrInspShortName.size(); nIdx++)
    {
        vecstrInspShortName[nIdx].Empty();
    }
}

void VisionInspectionBgaBottom2D::ResetResult()
{
    __super::ResetResult();

    m_bInvalid = FALSE;

    m_vecfCosForQuality.clear();
    m_vecfSinForQuality.clear();
    m_vecfCosForBallCenter.clear();
    m_vecfSinForBallCenter.clear();

    // Debug Info에 필요한 변수들 초기화...
    m_vecBlobBallPoint.clear();
    m_vecfptQualityCheckPairPoint.clear();
    m_vecDebugInfoValue.clear();
    m_vecrtDebugMissingBallROI.clear();
    m_vecQualitySweepLine.clear();

    m_result->reset();

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    m_sBallAlignResult->Init(Ipvm::Point32r2(0.f, 0.f));
    m_sBallAlignResult->m_ballConstrasts.clear();
    m_sBallAlignResult->m_ballConstrasts.resize(m_packageSpec.m_ballMap->m_balls.size(), 0.f);
    m_sBallAlignResult->m_ballConstrastROIs.clear();
    m_sBallAlignResult->m_ballConstrastROIs.resize(m_packageSpec.m_ballMap->m_balls.size(), Ipvm::Rect32s(0, 0, 0, 0));
    m_sBallAlignResult->m_ballWidths_px.clear();
    m_sBallAlignResult->m_ballWidths_px.resize(m_packageSpec.m_ballMap->m_balls.size(), 0.f);
    m_sBallAlignResult->m_ballWidthCenterPos_px.clear();
    m_sBallAlignResult->m_ballWidthCenterPos_px.resize(
        m_packageSpec.m_ballMap->m_balls.size(), Ipvm::Point32r2(0.f, 0.f));

    m_f2DScale = (getScale().pixelToUm().m_x + getScale().pixelToUm().m_y) * 0.5f;

    // 퀄리티 검사를 위한 스윕 각도의 삼각함수 테이블
    UpdateSweepAngleTable(SPEC_QUALITY_SWEEP_LINE_COUNT, m_vecfCosForQuality, m_vecfSinForQuality);

    // 20131004 영훈 : Bug 수정
    long nEdgeAlignDataNum(0);
    void* pEdgeAlignData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nEdgeAlignDataNum);
    if (pEdgeAlignData != nullptr && nEdgeAlignDataNum > 0)
        *m_sEdgeAlignResult = *(VisionAlignResult*)pEdgeAlignData;

    //PadAlign을 이용한 Center 좌표
    long nPadDataNum(0);
    m_pfptGetCenterPosbyPadAlign = nullptr;
    void* pPadAlignData = m_visionUnit.GetVisionDebugInfo(
        _VISION_INSP_GUID_FIDUCIAL_ALIGN, _T("Apply PAD Align to Body Center"), nPadDataNum);
    if (pPadAlignData != nullptr && nPadDataNum > 0)
        m_pfptGetCenterPosbyPadAlign = (Ipvm::Point32r2*)pPadAlignData;
    //

    // 볼 센터 검색을 위한 스윕 각도의 삼각함수 테이블
    long GroupCount = long(m_packageSpec.m_originalballMap->m_ballTypes.size());
    long ParamSize = long(m_pVisionPara->m_parameters.size());

    m_nWidthSweepLineCount.resize(GroupCount);
    m_vecfCosForBallCenter.resize(GroupCount);
    m_vecfSinForBallCenter.resize(GroupCount);
    m_pVisionPara->m_parameters.resize(GroupCount);

    if (ParamSize < GroupCount && ParamSize != 0)
    {
        for (int i = ParamSize; i < GroupCount; i++)
        {
            m_pVisionPara->m_parameters[i].Copy(m_pVisionPara->m_parameters[0]);
        }
    }

    for (int i = 0; i < GroupCount; i++)
    {
        auto& GroupParas = m_pVisionPara->m_parameters[i];

        long nBallSearchCount = GroupParas.m_nBallEdgeCountMode == enumBallEdgeFastMode
            ? SPEC_WIDTH_SWEEP_LINE_COUNT_FAST
            : SPEC_WIDTH_SWEEP_LINE_COUNT_DETAIL;
        UpdateSweepAngleTable(nBallSearchCount, m_vecfCosForBallCenter[i], m_vecfSinForBallCenter[i]);

        m_nWidthSweepLineCount[i] = GroupParas.m_nBallEdgeCountMode == enumBallEdgeFastMode
            ? SPEC_WIDTH_SWEEP_LINE_COUNT_FAST
            : SPEC_WIDTH_SWEEP_LINE_COUNT_DETAIL; //kircheis_BallFlux
        //m_nWidthSweepLineCount = SPEC_WIDTH_SWEEP_LINE_COUNT;//(m_VisionPara->nBallAlignMode == VisionInspectionBgaBottom2DPara::AlignMode_Fast) ? FAST_WIDTH_SWEEP_LINE_COUNT : SPEC_WIDTH_SWEEP_LINE_COUNT ; //kircheis_BKShiny
    }
    m_f2DScale = (getScale().pixelToUm().m_x + getScale().pixelToUm().m_y) * 0.5f;

    m_affineTransformSpecToReal[0][0] = 1.;
    m_affineTransformSpecToReal[0][1] = 0.;
    m_affineTransformSpecToReal[0][2] = 0.;
    m_affineTransformSpecToReal[1][0] = 0.;
    m_affineTransformSpecToReal[1][1] = 1.;
    m_affineTransformSpecToReal[1][2] = 0.;

    m_affineTransformRealToSpec[0][0] = 1.;
    m_affineTransformRealToSpec[0][1] = 0.;
    m_affineTransformRealToSpec[0][2] = 0.;
    m_affineTransformRealToSpec[1][0] = 0.;
    m_affineTransformRealToSpec[1][1] = 1.;
    m_affineTransformRealToSpec[1][2] = 0.;

    for (auto& result : m_resultGroup.m_vecResult)
    {
        result.Clear();
    }
}

void VisionInspectionBgaBottom2D::UpdateSpec()
{
    //kk 이전 버전 사용 Job들을 아예 다시 셋업 하면 상관이 없는데
    //이전 버전 Job을 그대로 사용할 때, GroupName이 만들어지지 않는게 문제
    //검사 항목 마다 초기화 하는 위치가 달라 한번 더 체크하도록 수정
    long nLayerNum = long(m_packageSpec.m_originalballMap->m_ballTypes.size());

    while (m_pVisionPara->m_vecstrGroupInspName.size() != nLayerNum)
    {
        if (m_pVisionPara->m_vecstrGroupInspName.size() < nLayerNum)
        {
            long index = long(m_pVisionPara->m_vecstrGroupInspName.size());
            Ball2DGroupInsp Data;

            Data.SetName(m_packageSpec.m_originalballMap->m_ballTypes[index].m_groupID);

            m_pVisionPara->m_vecstrGroupInspName.push_back(Data);
        }
        else
        {
            m_pVisionPara->m_vecstrGroupInspName.erase(m_pVisionPara->m_vecstrGroupInspName.end() - 1);
        }
    }

    long SpecNum = long(m_variableInspectionSpecs.size());
    long DefaultNum = long(m_defaultFixedInspectionSpecs.size()) - NOT_USE_GROUP_INSPECTION; //kk Ball Missing 제외
    long MakeSpecNum = (long(m_pVisionPara->m_vecstrGroupInspName.size()) * DefaultNum);

    while (SpecNum < MakeSpecNum)
    {
        long Index = SpecNum;

        if (Index < 0)
            return;

        CString strSpecInital;
        strSpecInital.Format(_T("BIGS_G%d"), Index);
        m_variableInspectionSpecs.emplace_back(m_moduleGuid, ::CreateGUID(), m_strModuleName, _T(""), strSpecInital,
            _T("C"), _T("um"), HostReportCategory::LAND, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE,
            FALSE, FALSE);

        SpecNum = long(m_variableInspectionSpecs.size());
        strSpecInital.Empty();
    }

    while (SpecNum > MakeSpecNum)
    {
        m_variableInspectionSpecs.erase(m_variableInspectionSpecs.end() - 1);

        SpecNum = long(m_variableInspectionSpecs.size());
    }

    std::vector<CString> InspectionName;
    long ballCount = long(m_packageSpec.m_originalballMap->m_ballTypes.size());

    if (ballCount != 1)
    {
        if (m_pVisionPara->m_vecstrGroupInspName.size() != 0)
        {
            for (int i = 0; i < ballCount; i++)
            {
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallOffsetXName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallOffsetYName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallOffsetRName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallGridOffsetXName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallGridOffsetYName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallWidthName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallQualityName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallContrastName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallEllipticityName);
            }
        }
    }
    else if (ballCount <= 1) //kk Ball Type이 한개 이하일 경우 필요 없습니다.
    {
        m_variableInspectionSpecs.clear();
        m_pVisionPara->m_vecstrGroupInspName.clear();
        SpecNum = 0;
    }

    for (long idx = 0; idx < long(SpecNum); idx++)
    {
        CString specName;
        specName.Format(_T("BIGS_%d"), idx);

        m_variableInspectionSpecs[idx].m_specName = InspectionName[idx];
        m_variableInspectionSpecs[idx].m_shortName = specName;

        if (InspectionName[idx].Find(_T("Quality")) != -1)
        {
            m_variableInspectionSpecs[idx].m_unit = _T("%");
        }
        else if (InspectionName[idx].Find(_T("Contrast")) != -1)
        {
            m_variableInspectionSpecs[idx].m_unit = _T("GV");
        }
        else
        {
            m_variableInspectionSpecs[idx].m_unit = _T("um");
        }

        m_variableInspectionSpecs[idx].MakeSpecFullName();

        specName.Empty();
    }

    GetVisionInspectionSpecs();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->ResetSpecAndResultDlg();
    }

    for (int nIdx = 0; nIdx < InspectionName.size(); nIdx++)
    {
        InspectionName[nIdx].Empty();
    }
}

void VisionInspectionBgaBottom2D::GetVisionInspectionSpecs()
{
    m_pVisionPara->m_vecVisionInspectionSpecs.clear();

    for (auto& spec : m_fixedInspectionSpecs)
    {
        m_pVisionPara->m_vecVisionInspectionSpecs.push_back(spec);
    }

    for (auto& spec : m_variableInspectionSpecs)
    {
        m_pVisionPara->m_vecVisionInspectionSpecs.push_back(spec);
    }
}

void VisionInspectionBgaBottom2D::SetVisionInspectionSpecs()
{
    for (auto& spec : m_pVisionPara->m_vecVisionInspectionSpecs)
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
