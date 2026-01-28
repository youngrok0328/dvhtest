//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspection2DMatrix.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspection2DMatrix.h"
#include "VisionInspection2DMatrixPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Easy_MainHeader.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspection2DMatrix::VisionInspection2DMatrix(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_2D_MATRIX, _VISION_INSP_NAME_2D_MATRIX, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspection2DMatrixPara)
    , m_sEdgeAlignResult(new VisionAlignResult)
    , m_bUseBypassMode(false) //kircheis_NGRV Bypass
{
    m_pVisionInspDlg = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{EC829C15-7304-4752-9133-4E44BD77C63B}"),
        g_sz2DMatrixInspName[MATRIX_INSPECTION_2D_MATRIX], _T("MATX"), _T("M"), _T(""), HostReportCategory::CODE2D,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{703FC02B-ABD6-4B83-9CA1-140AE93E2918}"),
        g_sz2DMatrixInspName[MATRIX_INSPECTION_STRING_MATCH], _T("MATX"), _T("M"), _T(""), HostReportCategory::CODE2D,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE); //kircheis_MED5_13

    __super::ResetSpec();

    m_DebugInfoGroup.Add(_T("2D Matrix ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("2D Matrix Result"), enumDebugInfoType::CSTRING);
    m_DebugInfoGroup.Add(_T("2D Matrix Major Result"), enumDebugInfoType::CSTRING);
    m_DebugInfoGroup.Add(_T("2D Matrix Sub Result"), enumDebugInfoType::CSTRING);
    m_DebugInfoGroup.Add(_T("2D Matrix Baby Result"), enumDebugInfoType::CSTRING);

    //{{//kircheis_MED5_13
    m_vecstr2DID.clear();
    m_vecstr2DID.resize(TYPE_2DID_END);
    m_vecrtROI.clear();
    m_vecrtROI.resize(TYPE_2DID_END);
    //}}

    //{{//아무 의미 없지만 필요한 Code. 동글키가 있어도 Euresys를 쓰면 처음 한번은 어마무시한 예외(꽤 긴 시간 소요)가 발생한다. 이를 미리 처리해주는 Code.
    try
    {
        Euresys::Open_eVision::EImageBW8 eImageBW;
    }
    catch (...)
    {
    }
    //}}
}

void VisionInspection2DMatrix::ResetSpecAndPara()
{
    __super::ResetSpec();
}

VisionInspection2DMatrix::~VisionInspection2DMatrix(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    m_vecstr2DID.clear(); //kircheis_MED5_13

    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspection2DMatrix::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspection2DMatrix::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    //if (m_visionUnit.IsTheUsingVisionProcessingByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()))
    //m_visionUnit.RunInspection(false, m_visionUnit.GetVisionAlignProcessing());
    //else
    if (m_visionUnit.RunInspection(m_visionUnit.GetPreviousVisionProcessing(this), false, GetCurVisionModule_Status())
        == false)
    {
        CString strError;
        strError.Format(_T("This module need execute the [%s] in the pre-process."),
            m_visionUnit.GetVisionProcessingModuleNameByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()));
        SimpleMessage(strError);
        strError.Empty();
    }

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspection2DMatrix(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspection2DMatrix::IDD, parent);

    Ipvm::Rect32r frtBody = m_pVisionInspDlg->m_pVisionInsp->GetBodyRect();

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->SetInitParaWindow();

    return 0;
}

BOOL VisionInspection2DMatrix::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    //// Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{77CDAD94-47CF-48EC-B2E7-F66AB39C9C4F}")]))
        return FALSE;

    return TRUE;
}

void VisionInspection2DMatrix::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    for (long nInsp = MATRIX_INSPECTION_START; nInsp < MATRIX_INSPECTION_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_sz2DMatrixInspName[nInsp]);
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

void VisionInspection2DMatrix::AppendTextResult(CString& textResult)
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
    void* pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_2D_MATRIX, _T("2D Matrix Result"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        CString strTemp;
        CString* pStr2DID = (CString*)pData;
        for (long nID = 0; nID < nDataNum; nID++)
        {
            textResult.AppendFormat(_T("          %s\r\n"), (LPCTSTR)pStr2DID[nID]);
        }
    }

    for (int nIdx = 0; nIdx < vecstrInspShortName.size(); nIdx++)
    {
        vecstrInspShortName[nIdx].Empty();
    }
}

void VisionInspection2DMatrix::ResetResult()
{
    __super::ResetResult();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }
}

std::vector<CString> VisionInspection2DMatrix::ExportRecipeToText() //kircheis_TxtRecipe
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
