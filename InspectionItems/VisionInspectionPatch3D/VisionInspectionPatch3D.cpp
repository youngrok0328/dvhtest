//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionPatch3D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionPatch3D.h"
#include "Result.h"
#include "VisionInspectionPatch3DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionWarpageShapeResult.h"
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
VisionInspectionPatch3D::VisionInspectionPatch3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_PATCH_3D, _VISION_INSP_NAME_PATCH_3D, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionPatch3DPara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
    , m_result(new Result)
{
    m_pVisionInspDlg = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{BF41A5D6-D3E3-4469-8FE2-E5E9227CD240}"),
        g_szPatchInspection3DName[PATCH_INSPECTION_3D_PATCH_HEIGHT], _T("POI3"), _T("P"), _T("um"),
        HostReportCategory::LID, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{46C825A6-C79F-45D2-ABFA-99E45733AEAE}"),
        g_szPatchInspection3DName[PATCH_INSPECTION_3D_UNIT_PATCH_HIGHT], _T("POI3"), _T("P"), _T("um"),
        HostReportCategory::LID, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{0C7C98ED-BDEE-4D55-AED7-76A3ADF3D4C6}"),
        g_szPatchInspection3DName[PATCH_INSPECTION_3D_UNIT_PATCH_HIGHT_ABS], _T("POI3"), _T("P"), _T("um"),
        HostReportCategory::LID, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    __super::ResetSpec();

    m_DebugInfoGroup.Add(_T("Patch Spec Height (um)"), enumDebugInfoType::Float);
    m_DebugInfoGroup.Add(_T("Patch Spec"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Patch Edge Search ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Patch Edge Point Left"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Patch Edge Point Top"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Patch Edge Point Right"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Patch Edge Point Bottom"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Detect Patch Rect"), enumDebugInfoType::Rect_32f);
    m_DebugInfoGroup.Add(_T("Detect Patch ROI"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("Interposer ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Patch Height ROI"), enumDebugInfoType::Rect);
    //m_DebugInfoGroup.Add(_T("Patch Warpage ROI"), enumDebugInfoType::Rect);
    //m_DebugInfoGroup.Add(_T("Patch Warpage Shape Info"), enumDebugInfoType::None);
}

void VisionInspectionPatch3D::ResetSpecAndPara()
{
    __super::ResetSpec();
}

VisionInspectionPatch3D::~VisionInspectionPatch3D(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    delete m_result;
    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspectionPatch3D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionPatch3D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_ALIGN_3D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_ALIGN_3D), false, GetCurVisionModule_Status());
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

    m_pVisionInspDlg = new CDlgVisionInspectionPatch3D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionPatch3D::IDD, parent);

    Ipvm::Rect32r frtBody = m_pVisionInspDlg->m_pVisionInsp->GetBodyRect();

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->SetInitParaWindow();

    return 0;
}

BOOL VisionInspectionPatch3D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    //// Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{D0353692-8A1A-4586-9CC7-6709E6481D7C}")]))
        return FALSE;

    return TRUE;
}

void VisionInspectionPatch3D::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    for (long nInsp = LID_INSPECTION_3D_START; nInsp < LID_INSPECTION_3D_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szLidInspection3DName[nInsp]);
            if (pResult == NULL)
                continue;

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
                            vrtReject.push_back(rtObj);
                    }

                    overlayResult->AddRectangles(vrtReject, RGB(255, 0, 0));
                    if (m_nOverlayMode == OverlayType_ShowAll)
                        overlayResult->AddRectangles(vrtPass, RGB(0, 255, 0));
                }
            }
        }
    }
}

void VisionInspectionPatch3D::AppendTextResult(CString& textResult)
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

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_LID_3D, _T("Lid Warpage Shape Info"), nDataNum);
    if (nDataNum > 0 && pData != nullptr && (vecbUseInsp[_3DINSP_WARPAGE] || vecbUseInsp[_3DINSP_UNIT_WARPAGE]))
    {
        VisionWarpageShapeResult* pWarpageShapeInfo = (VisionWarpageShapeResult*)pData;
        CString strLid;
        for (long nID = 0; nID < nDataNum; nID++)
        {
            strLid.Format(_T("Lid ID - %d"), nID);
            textResult.AppendFormat(_T("         %s => Sign : (%s),  Shape : (%s),  R : (%.4lf),  B4 : (%.4lf),  B5 : ")
                                    _T("(%.4lf),  Angle : (%.4lf)\r\n"),
                (LPCTSTR)strLid, (LPCTSTR)pWarpageShapeInfo[nID].m_sign, (LPCTSTR)pWarpageShapeInfo[nID].m_shape,
                pWarpageShapeInfo[nID].m_R, pWarpageShapeInfo[nID].m_B4_um, pWarpageShapeInfo[nID].m_B5_um,
                pWarpageShapeInfo[nID].m_angle_deg);
        }
        strLid.Empty();
    }

    for (int nIdx = 0; nIdx < vecstrInspShortName.size(); nIdx++)
    {
        vecstrInspShortName[nIdx].Empty();
    }
}

void VisionInspectionPatch3D::ResetResult()
{
    __super::ResetResult();

    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();
    m_result->Initialize(getScale(), *m_VisionPara);

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }
}

std::vector<CString> VisionInspectionPatch3D::ExportRecipeToText() //kircheis_TxtRecipe
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