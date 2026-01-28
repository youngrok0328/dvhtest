//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Property_Stitch.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingAlign3DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 200

//CPP_7_________________________________ Implementation body
//
enum enumImageMerge3DImageParaID
{
    MERGE_3D_PARAM_START,
    STITCH_3D_PARAM_RUN_ONLY_STITCH,
    MERGE_3D_PARAM_PANE_EXTENSION_RATIO,
    MERGE_3D_PARAM_PATTERN_SIZE_X,
    MERGE_3D_PARAM_PATTERN_SIZE_Y,
    MERGE_3D_PARAM_MOVING_ERROR_X,
    MERGE_3D_PARAM_MOVING_ERROR_Y,
    MERGE_3D_PARAM_MATCHING_SIZE_Y,
    MERGE_3D_PARAM_MATCHING_REDUCE_SIZE_X,

    //MERGE_3D_PARAM_MATCHING_POS_OFFSET,

    MERGE_3D_PARAM_PLANE_COMPENSATION,

    MERGE_3D_PARAM_TOP_STITCH_ROI_AMOUNT_X,
    MERGE_3D_PARAM_TOP_STITCH_ROI_AMOUNT_Y,
    MERGE_3D_PARAM_TOP_STITCH_ROI_RADIUS_X,
    MERGE_3D_PARAM_TOP_STITCH_ROI_RADIUS_Y,

    MERGE_3D_PARAM_END,
};

enum enumPlaneCompesation
{
    PLANE_COMPENSATION_START,
    PLANE_COMPENSATION_OFFSET_Z = PLANE_COMPENSATION_START,
    PLANE_COMPENSATION_ROTATE_3D,
    PLANE_COMPENSATION_END,
};

static LPCTSTR g_szImageMerge3DParaName[] = {
    _T("Pane Extension Ratio"), _T("Pattern Size X"), _T("Pattern Size Y"), _T("Moving Error X"), _T("Moving Error Y"),
    _T("Matching Size Y"), // Matching할 크기
    _T("Matching Reduce Size X"), // Y방향으로 줄이는 크기
    //	_T("Matching Position Offset (mm)"), // Matching할 위치 Offset
    //	_T("Ball Search Extension Ratio (%)"),

    // 사용자가 지정된 크기와 개수만큼 측정 영역 설정.
    //	_T("Match Plane Overlap Offset-X Ratio (%)"),
    //	_T("Match Plane Overlap Offset-Y Ratio (%)"),
    //	_T("Plane Data Num X"),
    //	_T("Plane Data Num Y"),

    //	_T("Measure ROI Size (mm)"),
    //	_T("Measure ROI Offset (mm)"),
    _T("Plane Compensation"),
    //	_T("Calc Z-Point"),
};

static LPCTSTR g_szPlaneCompesationParaName[] = {
    _T("Offset Z"),
    _T("Rotate 3D"),
};

static LPCTSTR g_szTopStitchParaName[] = {
    _T("Top Stitch Algorithm Enable"),
    _T("Top Stitch ROI Amount X"),
    _T("Top Stitch ROI Amount Y"),
    _T("Top Stitch ROI Radius X (um)"),
    _T("Top Stitch ROI Radius Y (um)"),
};

enum enum3DStitching_MultiROI_KeyType //Edge과 3D Stitching의 Key값이 동일하면 발생함으로, 분기하여 사용 할 수있도록 하자..
{
    MultiROIKeyType_LEFT = 4,
    MultiROIKeyType_TOP,
    MultiROIKeyType_RIGHT,
    MultiROIKeyType_BOTTOM,
};

Property_Stitch::Property_Stitch(VisionProcessing& proccsor, ImageLotView* imageLotView,
    ProcCommonDebugInfoDlg& procDebugInfo, CVisionProcessingAlign3DPara& para)
    : m_proccsor(proccsor)
    , m_imageLotView(imageLotView)
    , m_procDebugInfo(procDebugInfo)
    , m_para(para)
    , m_grid(new CXTPPropertyGrid)
{
}

Property_Stitch::~Property_Stitch()
{
    delete m_grid;
}

bool Property_Stitch::create(long /*gridIndex*/, const RECT& rect, HWND parentWnd)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWnd* pParentWnd = CWnd::FromHandle(parentWnd);

    if (!m_grid->Create(rect, pParentWnd, IDC_FRAME_PROPERTY_GRID))
    {
        return false;
    }

    m_grid->ShowWindow(SW_HIDE);

    return true;
}

LPCTSTR Property_Stitch::getName(long /*gridIndex*/) const
{
    return _T("Stitch");
}

void Property_Stitch::active(long /*gridIndex*/)
{
    m_procDebugInfo.Display(0);
    m_imageLotView->SetMode(ImageLotView::Mode::LotRawAndInspImage);

    ShowImage();

    m_grid->ResetContent();

    if (auto* category = m_grid->AddCategory(_T("Run")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Run only stitch"), TRUE, FALSE))
            ->SetID(STITCH_3D_PARAM_RUN_ONLY_STITCH);

        category->Expand();
    }

    if (auto* trayInfo = m_grid->AddCategory(_T("Stitch information")))
    {
        if (auto* item = new CXTPPropertyGridItemEnum(
                _T("Rough align"), (long)m_para.m_eRoughAlignUsingDefine, (int*)&m_para.m_eRoughAlignUsingDefine))
        {
            item->GetConstraints()->AddConstraint(
                _T("Not Use"), (long)eRoughAlignUsingDefine::eRoughAlignUsingDefine_NOT_USE);
            item->GetConstraints()->AddConstraint(
                _T("H-Map"), (long)eRoughAlignUsingDefine::eRoughAlignUsingDefine_H_Map);
            item->GetConstraints()->AddConstraint(
                _T("V-Map"), (long)eRoughAlignUsingDefine::eRoughAlignUsingDefine_V_Map);
            trayInfo->AddChildItem(item);
        }

        trayInfo
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                g_szImageMerge3DParaName[0], m_para.m_fPaneExtensionRatio, _T("%.3lf %%")))
            ->SetID(MERGE_3D_PARAM_PANE_EXTENSION_RATIO);
        trayInfo
            ->AddChildItem(
                new CXTPPropertyGridItemDouble(g_szImageMerge3DParaName[1], m_para.m_fPatternSizeX, _T("%.3lf mm")))
            ->SetID(MERGE_3D_PARAM_PATTERN_SIZE_X);
        trayInfo
            ->AddChildItem(
                new CXTPPropertyGridItemDouble(g_szImageMerge3DParaName[2], m_para.m_fPatternSizeY, _T("%.3lf mm")))
            ->SetID(MERGE_3D_PARAM_PATTERN_SIZE_Y);
        trayInfo
            ->AddChildItem(
                new CXTPPropertyGridItemDouble(g_szImageMerge3DParaName[3], m_para.m_fMovingErrorX, _T("%.3lf mm")))
            ->SetID(MERGE_3D_PARAM_MOVING_ERROR_X);
        trayInfo
            ->AddChildItem(
                new CXTPPropertyGridItemDouble(g_szImageMerge3DParaName[4], m_para.m_fMovingErrorY, _T("%.3lf mm")))
            ->SetID(MERGE_3D_PARAM_MOVING_ERROR_Y);
        trayInfo
            ->AddChildItem(
                new CXTPPropertyGridItemDouble(g_szImageMerge3DParaName[5], m_para.m_fMatchingSize, _T("%.3lf mm")))
            ->SetID(MERGE_3D_PARAM_MATCHING_SIZE_Y);
        trayInfo
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                g_szImageMerge3DParaName[6], m_para.m_fMatchingReduceSizeX, _T("%.3lf mm")))
            ->SetID(MERGE_3D_PARAM_MATCHING_REDUCE_SIZE_X);

        trayInfo->Expand();
    }

    if (auto* TopInfo = m_grid->AddCategory(_T("Top Stitching Algorithm information")))
    {
        TopInfo->AddChildItem(new CXTPPropertyGridItemNumber(g_szTopStitchParaName[1], m_para.m_nTopStichRoiAmountX))
            ->SetID(MERGE_3D_PARAM_TOP_STITCH_ROI_AMOUNT_X);
        TopInfo->AddChildItem(new CXTPPropertyGridItemNumber(g_szTopStitchParaName[2], m_para.m_nTopStichRoiAmountY))
            ->SetID(MERGE_3D_PARAM_TOP_STITCH_ROI_AMOUNT_Y);
        TopInfo
            ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(g_szTopStitchParaName[3],
                m_para.m_topStichRoiRadiusX_um, _T("%.3f um"), m_para.m_topStichRoiRadiusX_um, 0.f, 50000.f, 0.1f))
            ->SetID(MERGE_3D_PARAM_TOP_STITCH_ROI_RADIUS_X);
        TopInfo
            ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(g_szTopStitchParaName[4],
                m_para.m_topStichRoiRadiusY_um, _T("%.3f um"), m_para.m_topStichRoiRadiusY_um, 0.f, 50000.f, 0.1f))
            ->SetID(MERGE_3D_PARAM_TOP_STITCH_ROI_RADIUS_Y);
        TopInfo->Expand();
    }

    m_grid->SetViewDivider(0.33);
    m_grid->HighlightChangedItems(TRUE);
    m_grid->ShowWindow(SW_SHOW);

    SetROI();
}

void Property_Stitch::deactivate(long /*gridIndex*/)
{
    m_grid->ShowWindow(SW_HIDE);
}

bool Property_Stitch::notify(long /*gridIndex*/, WPARAM wparam, LPARAM lparam)
{
    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;
        CString strTemp("");

        switch (item->GetID())
        {
            case STITCH_3D_PARAM_RUN_ONLY_STITCH:
                m_para.m_skipEdgeAlign = true;

                m_proccsor.OnInspection();

                m_para.m_skipEdgeAlign = false;
                break;

            case MERGE_3D_PARAM_PANE_EXTENSION_RATIO:
            {
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_para.m_fPaneExtensionRatio = (float)value->GetDouble();
            }
            break;
            case MERGE_3D_PARAM_PATTERN_SIZE_X:
            {
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_para.m_fPatternSizeX = (float)value->GetDouble();
            }
            break;
            case MERGE_3D_PARAM_PATTERN_SIZE_Y:
            {
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_para.m_fPatternSizeY = (float)value->GetDouble();
            }
            break;
            case MERGE_3D_PARAM_MOVING_ERROR_X:
            {
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_para.m_fMovingErrorX = (float)value->GetDouble();
            }
            break;
            case MERGE_3D_PARAM_MOVING_ERROR_Y:
            {
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_para.m_fMovingErrorY = (float)value->GetDouble();
            }
            break;
            case MERGE_3D_PARAM_MATCHING_SIZE_Y:
            {
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_para.m_fMatchingSize = (float)value->GetDouble();
            }
            break;
            case MERGE_3D_PARAM_MATCHING_REDUCE_SIZE_X:
            {
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_para.m_fMatchingReduceSizeX = (float)value->GetDouble();
            }
            break;
            case MERGE_3D_PARAM_TOP_STITCH_ROI_AMOUNT_X:
            {
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                    m_para.m_nTopStichRoiAmountX = value->GetNumber();
                break;
            }
            case MERGE_3D_PARAM_TOP_STITCH_ROI_AMOUNT_Y:
            {
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                    m_para.m_nTopStichRoiAmountY = value->GetNumber();
                break;
            }
        }
    }

    return false;
}

void Property_Stitch::SetROI()
{
    if (m_imageLotView == nullptr)
        return;

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    m_imageLotView->ROI_Show(TRUE);
    m_imageLotView->Overlay_Show(TRUE);
}

CString Property_Stitch::GetKey(long type, long index)
{
    CString key;
    key.Format(_T("%d_%d"), type, index);

    return key;
}

void Property_Stitch::event_afterInspection(long /*gridIndex*/)
{
    m_imageLotView->ShowImage(m_proccsor.GetImageFrameIndex(0));
    m_imageLotView->ZoomPaneFit(true);
    m_imageLotView->ROI_Show(m_imageLotView->IsShowRaw());
}

void Property_Stitch::event_changedRoi(long /*gridIndex*/)
{
}

void Property_Stitch::event_changedPane(long /*gridIndex*/)
{
    SetROI();
}

void Property_Stitch::event_changedRaw(long /*gridIndex*/)
{
    m_imageLotView->ROI_Show(m_imageLotView->IsShowRaw());
}

void Property_Stitch::ShowImage()
{
    m_imageLotView->ShowImage(m_proccsor.GetImageFrameIndex(0), true);
    m_imageLotView->ZoomPaneFit(true);
}
