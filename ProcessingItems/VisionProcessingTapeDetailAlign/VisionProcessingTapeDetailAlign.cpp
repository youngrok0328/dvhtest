//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingTapeDetailAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingTapeDetailAlign.h"
#include "Inspection.h"
#include "Para.h"
#include "Result.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/EdgeDetection.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionProcessingTapeDetailAlign::VisionProcessingTapeDetailAlign(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_TAPE_DETAIL_ALIGN, _T("Tape Detail Align"), visionUnit, packageSpec)
    , m_para(new Para(*this))
    , m_pVisionInspDlg(nullptr)
{
    m_inspector = new Inspection(*this);
    m_edgeDetection = new Ipvm::EdgeDetection;

    m_DebugInfoGroup.Add(_T("Spec Sprocket Hole ROIs"), enumDebugInfoType::Rect_32f);
    m_DebugInfoGroup.Add(_T("Spec Pocket ROI"), enumDebugInfoType::Rect_32f);
    m_DebugInfoGroup.Add(_T("Spec Device ROI"), enumDebugInfoType::Rect_32f);

    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_SPROCKET_HOLE_SEARCH_ROI, enumDebugInfoType::Rect_32f);
    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_SPROCKET_HOLE_SEARCH_LINE, enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_SPROCKET_HOLE_EDGE_POINT, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_SPROCKET_HOLE_CIRCLE, enumDebugInfoType::EllipseEq_32f);

    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_POCKET_SEARCH_ROI, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_POCKET_EDGE_POINT, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_POCKET_ALIGN_QRECT, enumDebugInfoType::Quadrangle_32f);
    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_POCKET_RECT, enumDebugInfoType::Rect_32f);

    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_DEVICE_SEARCH_ROI, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_DEVICE_EDGE_POINT, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_DEBUG_INFO_TDA_DEVICE_ALIGN_QRECT, enumDebugInfoType::Quadrangle_32f);
}

void VisionProcessingTapeDetailAlign::ResetSpecAndPara()
{
    m_para->Init();
}

void VisionProcessingTapeDetailAlign::ResetResult()
{
    ResetDebugInfo();

    m_inspector->ResetResult();
}

VisionProcessingTapeDetailAlign::~VisionProcessingTapeDetailAlign(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    delete m_para;
    delete m_edgeDetection;
    delete m_inspector;
}

void VisionProcessingTapeDetailAlign::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingTapeDetailAlign::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    m_visionUnit.RunInspection(m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_TAPE_DETAIL_ALIGN));

    DoInspection(true); // UI 표시를 위해 현재계산도 그냥 치뤄주자

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new DlgVisionProcessingTapeDetailAlign(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(DlgVisionProcessingTapeDetailAlign::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->SetInitParaWindow();
    //	m_pVisionInspDlg->ShowImage();

    return 0;
}

BOOL VisionProcessingTapeDetailAlign::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
    {
        return FALSE;
    }

    if (!m_para->LinkDataBase(bSave, db[_T("{167F48D4-E05C-4D5B-AE15-FE3DFD7C36B9}")]))
    {
        return FALSE;
    }

    return TRUE;
}

void VisionProcessingTapeDetailAlign::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (detailSetupMode)
    {
        overlayResult->OverlayReset();
        auto& result = m_inspector->m_result;

        //Sprocket Hole 수집
        if (result.m_sprocketHoles.size() > 0)
        {
            std::vector<Ipvm::Rect> vecHoleROI(0);
            for (const auto& hole : result.m_sprocketHoles)
            {
                vecHoleROI.push_back(hole.m_alignedHoleROI);
            }
            overlayResult->AddRectangles(vecHoleROI, RGB(0, 255, 0));
        }

        //Pocket Data 수집
        if (result.m_pocket.m_edgePointsFiltered.size() > 16)
        {
            Ipvm::Rect32r pocketROIr = result.m_pocket.m_alignedPocket.GetExtRect();
            Ipvm::Rect pocketROI = {
                LONG(pocketROIr.m_left), LONG(pocketROIr.m_top), LONG(pocketROIr.m_right),
                LONG(pocketROIr.m_bottom)};

            overlayResult->AddRectangle(pocketROI, RGB(0, 255, 0));
        }

        //Device Data 수집
        if (result.m_device.m_edgePointsFiltered.size() > 16)
        {
            Ipvm::Rect32r deviceROIr = result.m_device.m_alignedPacket.GetExtRect();
            Ipvm::Rect deviceROI = {
                LONG(deviceROIr.m_left), LONG(deviceROIr.m_top), LONG(deviceROIr.m_right), LONG(deviceROIr.m_bottom)};

            overlayResult->AddRectangle(deviceROI, RGB(0, 255, 0));
        }

    }
}

void VisionProcessingTapeDetailAlign::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);

    if (m_bInvalid)
    {
        textResult.AppendFormat(_T("  Pad Align Invalid\r\n"));
        textResult += m_errorLogText;
    }
}

std::vector<CString> VisionProcessingTapeDetailAlign::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_para->ExportAlgoParaToText(strVisionName, m_strModuleName);

    return vecStrResult;
}
