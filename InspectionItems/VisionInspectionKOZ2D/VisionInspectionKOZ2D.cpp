//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionKOZ2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionKOZ2D.h"
#include "VisionInspectionKOZ2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
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

//CPP_7_________________________________ Implementation body
//
VisionInspectionKOZ2D::VisionInspectionKOZ2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_KOZ_2D, _VISION_INSP_NAME_KOZ_2D, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionKOZ2DPara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
{
    m_pVisionInspDlg = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(GUID_KOZ2D_INSP_ITEM_KOZWIDTH,
        g_szKOZInspection2DName[KOZ_INSPECTION_2D_WIDTH], _T("KZW"), _T("K"), _T("um"), HostReportCategory::KOZ,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(GUID_KOZ2D_INSP_ITEM_KOZCHIPPING,
        g_szKOZInspection2DName[KOZ_INSPECTION_2D_CHIPPING], _T("KZC"), _T("K"), _T("um"), HostReportCategory::KOZ,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    __super::ResetSpec();

    m_DebugInfoGroup.Add(_T("Notch Spec"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("Notch Edge Sweep Line"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("Notch Edge Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Notch Align Result"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("KOZ Notch Spec"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("KOZ Notch Edge Sweep Line"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("KOZ Notch Edge Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("KOZ Notch Align Result"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("KOZ Spec"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("KOZ Edge Search ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("KOZ Detected Edge Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("KOZ Alignment Result"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("Chipping - Detected blobs(candidate)"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Chipping - Filtered blobs(Chipping)"), enumDebugInfoType::Rect);

    UpdateSweepAngleTable(NOTCH_SWEEP_LINE_NUM, m_vecfCosForNotchAlign, m_vecfSinForNotchAlign);

    m_bIsNeedToMakeChippingMask = true;
    m_bIsNeedToMakeChippingBackground = true;
}

void VisionInspectionKOZ2D::ResetSpecAndPara()
{
    __super::ResetSpec();
}

VisionInspectionKOZ2D::~VisionInspectionKOZ2D(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    m_imagePackageAlignMask.Free();
    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspectionKOZ2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionKOZ2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    m_bIsNeedToMakeChippingMask = true;
    m_bIsNeedToMakeChippingBackground = true;

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_LGA_LAND_2D)) //최대한 Mask를 끌어 써야 한다.
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_LGA_LAND_2D), false, GetCurVisionModule_Status());
    else if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_BGA_BALL_2D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_BGA_BALL_2D), false, GetCurVisionModule_Status());
    else if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D), false, GetCurVisionModule_Status());
    else if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_2D_MATRIX))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_2D_MATRIX), false, GetCurVisionModule_Status());
    else if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_ALIGN_2D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_ALIGN_2D), false, GetCurVisionModule_Status());
    else if (m_visionUnit.IsTheUsingVisionProcessingByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_visionUnit.RunInspection(m_visionUnit.GetVisionAlignProcessing(), false, GetCurVisionModule_Status());
    else
    {
        CString strError;
        strError.Format(_T("This module need execute the [%s] in the pre-process."),
            m_visionUnit.GetVisionProcessingModuleNameByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()));
        SimpleMessage(strError);
    }

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionKOZ2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionKOZ2D::IDD, parent);

    Ipvm::Rect32r frtBody = m_pVisionInspDlg->m_pVisionInsp->GetBodyRect();

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->SetInitParaWindow();

    return 0;
}

BOOL VisionInspectionKOZ2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    //// Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[GUID_KOZ2D_MODULE_PARA_LINK_ID]))
        return FALSE;

    return TRUE;
}

void VisionInspectionKOZ2D::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    BOOL bOnlyUseAlign = TRUE;
    BOOL bDrawAlignResult = true;

    for (long nInsp = KOZ_INSPECTION_2D_START; nInsp < KOZ_INSPECTION_2D_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szKOZInspection2DName[nInsp]);
            if (pResult == NULL)
                continue;

            bOnlyUseAlign = FALSE;

            if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                //if(pResult->m_totalResult == REJECT)
                {
                    std::vector<Ipvm::Rect32s> vrtPass;
                    std::vector<Ipvm::Rect32s> vrtReject;
                    long nObjNum = (long)pResult->m_objectResults.size();
                    if (nObjNum != pResult->m_objectRects.size())
                        continue;

                    for (long nID = 0; nID < nObjNum; nID++)
                    {
                        auto rtObj = pResult->m_objectRects[nID];
                        if (pResult->m_objectResults[nID] == PASS)
                            vrtPass.push_back(rtObj);
                        else
                        {
                            vrtReject.push_back(rtObj);
                            bDrawAlignResult = false;
                        }
                    }

                    overlayResult->AddRectangles(vrtReject, RGB(255, 0, 0));
                    /*if(m_nOverlayMode == OverlayType_ShowAll)
						overlayResult->AddRectangles(vrtPass, RGB(0, 255, 0));*/
                }
            }
        }
    }

    if (bDrawAlignResult && detailSetupMode)
    {
        if (m_kozAlignResult.bAvailable)
        {
            PI_RECT prtKOZ;
            prtKOZ.SetLTPoint(Ipvm::Point32s2(
                CAST_INT32T(m_kozAlignResult.fptLT.m_x + .5f), CAST_INT32T(m_kozAlignResult.fptLT.m_y + .5f)));
            prtKOZ.SetLBPoint(Ipvm::Point32s2(
                CAST_INT32T(m_kozAlignResult.fptLB.m_x + .5f), CAST_INT32T(m_kozAlignResult.fptLB.m_y + .5f)));
            prtKOZ.SetRTPoint(Ipvm::Point32s2(
                CAST_INT32T(m_kozAlignResult.fptRT.m_x + .5f), CAST_INT32T(m_kozAlignResult.fptRT.m_y + .5f)));
            prtKOZ.SetRBPoint(Ipvm::Point32s2(
                CAST_INT32T(m_kozAlignResult.fptRB.m_x + .5f), CAST_INT32T(m_kozAlignResult.fptRB.m_y + .5f)));
            overlayResult->AddRectangle(prtKOZ, RGB(0, 255, 0));
        }
    }
}

void VisionInspectionKOZ2D::AppendTextResult(CString& textResult)
{
    long nInspNum = (long)m_fixedInspectionSpecs.size();
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
        vecpInspResult[nID] = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[nID].m_specName);
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
}

void VisionInspectionKOZ2D::ResetResult()
{
    __super::ResetResult();

    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }
}

std::vector<CString> VisionInspectionKOZ2D::ExportRecipeToText() //kircheis_TxtRecipe
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