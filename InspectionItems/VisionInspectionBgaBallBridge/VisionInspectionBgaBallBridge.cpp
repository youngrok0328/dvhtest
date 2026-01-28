//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBallBridge.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBallBridge.h"
#include "VisionInspectionBgaBallBridgePara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
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

VisionInspectionBgaBallBridge::VisionInspectionBgaBallBridge(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_BGA_BALL_BRIDGE, _VISION_INSP_NAME_BGA_BALL_BRIDGE, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionBgaBallBridgePara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
    , m_imageMaskInspROI(new Ipvm::Image8u)
{
    m_pVisionInspDlg = NULL;

    m_pEdgeDetect = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{CF6F103D-8007-4827-9677-A89CEBC69BF0}"), m_strModuleName,
        _T("BABR"), _T("B"), _T("%"), HostReportCategory::MEASURE, II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f,
        FALSE, FALSE, FALSE);

    m_DebugInfoGroup.Add(_T("Ball Link Map"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("Ball Reference Circle"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("Ball Ignore Circle"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("Ball Bridge Insp ROI"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("Ball Bridge Raw ROI (for developer)"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Ball Bridge Insp Mask ROI"), enumDebugInfoType::PI_Rect); //kircheis_MED3
    m_DebugInfoGroup.Add(_T("Ball Bridge Insp Mask"), enumDebugInfoType::Image_8u_C1); //kircheis_MED3
    m_DebugInfoGroup.Add(_T("Ball Bridge Insp Mask [Rect]"), enumDebugInfoType::Image_8u_C1); //kircheis_MED3

    __super::ResetSpec();
}

void VisionInspectionBgaBallBridge::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_VisionPara->Init();
}

VisionInspectionBgaBallBridge::~VisionInspectionBgaBallBridge(void)
{
    CloseDlg();

    ResetBufferResult();

    delete m_sEdgeAlignResult;
    delete m_VisionPara;
    delete m_imageMaskInspROI;
}

void VisionInspectionBgaBallBridge::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionBgaBallBridge::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_BGA_BALL_2D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_BGA_BALL_2D), false, GetCurVisionModule_Status());
    else
    {
        CString strError;
        strError.Format(_T("This module need execute the [%s] in the pre-process."),
            m_visionUnit.GetVisionProcessingModuleNameByGuid(_VISION_INSP_GUID_BGA_BALL_2D));
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

    m_pVisionInspDlg = new CDlgVisionInspectionBgaBallBridge(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionBgaBallBridge::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();
    m_imageMaskInspROI->Create(imageSizeX, imageSizeY);
    RefreshPackageSpecBallLink();
    MakeRefData(true);

    return 0;
}

BOOL VisionInspectionBgaBallBridge::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{692ADB0A-A028-4BB6-AB4D-01FB523AACDB}")]))
        return FALSE;
    if (!bSave)
    {
        RefreshPackageSpecBallLink();
        MakeRefData(false);
    }

    return TRUE;
}

// 영훈 20141218_BallOverlay : 검사 시 ball 불량 시 화면에 표시해주도록 한다.
void VisionInspectionBgaBallBridge::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (spec.m_use)
        {
            auto* pResult = m_resultGroup.GetResultByName(spec.m_specName);

            if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                if (pResult->vecrtRejectROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtRejectROI, RGB(255, 0, 0));

                    if (spec.m_useMarginal && pResult->vecrtMarginalROI.size())
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

void VisionInspectionBgaBallBridge::AppendTextResult(CString& textResult)
{
    if (m_packageSpec.nDeviceType != enDeviceType::PACKAGE_BALL) // || !m_packageSpec.bDeadBug)//kircheis_BGABS
        return;

    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);

    long nTotalResult(NOT_MEASURED);

    // Total Result...
    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (spec.m_use)
        {
            auto* result = m_resultGroup.GetResultByName(spec.m_specName);
            if (result != nullptr && result->m_totalResult > nTotalResult)
            {
                nTotalResult = result->m_totalResult;
            }
        }
    }

    textResult.AppendFormat(
        _T("    %s Total Result : %s\r\n"), (LPCTSTR)m_strModuleName, (LPCTSTR)Result2String(nTotalResult));

    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (!spec.m_use)
            continue;

        auto* result = m_resultGroup.GetResultByName(spec.m_specName);
        if (result == nullptr)
        {
            CString strAssert;
            strAssert.Format(_T("VisionInspectionBgaBallBridge::AppendTextResult() - %s result is nullptr"),
                LPCTSTR(spec.m_specName));
            ASSERT(LPCTSTR(strAssert));
            continue;
        }

        auto worstErrorValue = result->getObjectWorstErrorValue();

        if (worstErrorValue == Ipvm::k_noiseValue32r)
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ INV ])\r\n"),
                (LPCTSTR)result->m_resultName, (LPCTSTR)spec.m_shortName, (LPCTSTR)Result2String(result->m_totalResult),
                (LPCTSTR)result->getObjectWorstID());
        }
        else
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ %.2f ])\r\n"),
                (LPCTSTR)result->m_resultName, (LPCTSTR)spec.m_shortName, (LPCTSTR)Result2String(result->m_totalResult),
                (LPCTSTR)result->getObjectWorstID(), worstErrorValue);
        }
    }

    long nObjectTotalNum = (long)m_packageSpec.m_ballMap->m_balls.size(); //Parameters for this module only.

    AppendDetailTextResult(textResult, nObjectTotalNum);
}

void VisionInspectionBgaBallBridge::ResetResult()
{
    __super::ResetResult();

    m_bInvalid = FALSE;

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
    }

    for (auto& result : m_resultGroup.m_vecResult)
    {
        result.Clear();
    }
}

void VisionInspectionBgaBallBridge::RefreshPackageSpecBallLink()
{
    float fAvrBallPitch = m_packageSpec.m_ballMap->GetAvrBallDist();
    m_packageSpec.m_ballMap->SetBallLink(fAvrBallPitch, (float)(m_VisionPara->m_fBallLinkRangeRatio * 0.01f));
}

std::vector<CString> VisionInspectionBgaBallBridge::ExportRecipeToText() //kircheis_TxtRecipe
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
