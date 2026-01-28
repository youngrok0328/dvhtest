//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingOtherAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingOtherAlign.h"
#include "VisionProcessingOtherAlignPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
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
VisionProcessingOtherAlign::VisionProcessingOtherAlign(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_OTHER_ALIGN, _T("Rect Object Align"), visionUnit, packageSpec)
    , m_VisionPara(new CVisionProcessingOtherAlignPara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
{
    m_pVisionInspDlg = NULL;

    m_DebugInfoGroup.Add(_T("Edge Search ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Edge Point Left"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Edge Point Top"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Edge Point Right"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Edge Point Bottom"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Detect Rect"), enumDebugInfoType::Rect_32f);
    m_DebugInfoGroup.Add(_T("Detect ROI"), enumDebugInfoType::PI_Rect);
}

void VisionProcessingOtherAlign::ResetSpecAndPara()
{
    m_VisionPara->Init();
}

void VisionProcessingOtherAlign::ResetResult()
{
    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();
    m_pBlob = getReusableMemory().GetBlob();

    m_vecptLeftTop.clear();
    m_vecptRightTop.clear();
    m_vecptRightBottom.clear();
    m_vecptLeftBottom.clear();

    vecLeftEdgeAlignPoints.clear();
    vecTopEdgeAlignPoints.clear();
    vecRightEdgeAlignPoints.clear();
    vecBottomEdgeAlignPoints.clear();

    vecLeftEdgeAlignPoints_Total.clear();
    vecTopEdgeAlignPoints_Total.clear();
    vecRightEdgeAlignPoints_Total.clear();
    vecBottomEdgeAlignPoints_Total.clear();

    m_vecfrtAlignROI.clear();
    m_vecSearchROI.clear();
    m_vecSearchROI.resize(4);
}

VisionProcessingOtherAlign::~VisionProcessingOtherAlign(void)
{
    CloseDlg();

    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionProcessingOtherAlign::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingOtherAlign::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_visionUnit.RunInspection(m_visionUnit.GetVisionAlignProcessing());
    else
    {
        CString strError;
        strError.Format(_T("This module need execute the [%s] in the pre-process."),
            m_visionUnit.GetVisionProcessingModuleNameByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()));
        SimpleMessage(strError);

        strError.Empty();
    }

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionProcessingOtherAlign(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionProcessingOtherAlign::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionProcessingOtherAlign::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{B3CC00DF-9157-4583-8203-0B9A9A0F7AD2}")]))
        return FALSE;

    if (bSave == false && m_VisionPara->m_bIsValidateRecipe == FALSE)
    {
        SetEnabled(FALSE);
    }

    return TRUE;
}

std::vector<CString> VisionProcessingOtherAlign::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;
    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;

    return m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);
}

void VisionProcessingOtherAlign::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_3D_INSP)
    {
        if (m_nOverlayMode == OverlayType_ShowAll)
        {
            for (auto& searchROI_um : m_VisionPara->m_vecrtSearchROI_BCU)
            {
                Ipvm::Rect32s roi = getScale().convert_BCUToPixel(searchROI_um, imageCenter);

                overlayResult->AddRectangle(roi, RGB(0, 255, 0));
            }
        }
    }
}

void VisionProcessingOtherAlign::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);
}
