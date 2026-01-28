//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PropertyGrid.h"

//CPP_2_________________________________ This project's headers
#include "DlgSearchRoiPara.h"
#include "Inspection.h"
#include "Para.h"
#include "Resource.h"
#include "ResultAlign.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTapeSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/Config.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/LineEq32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 400
#define SEARCH_ROI_MAX_NUM 5

//CPP_7_________________________________ Implementation body
//
enum enumEDGEAlignParamID
{
    EDGE_ALIGN_PARAM_CARRIER_TAPE_IMAGE_COMBINE = 1,
    EDGE_ALIGN_PARAM_CARRIER_TAPE_FRAME_INDEX,
    EDGE_ALIGN_PARAM_CARRIER_TAPE_EDGE_DETECT_MODE,
    EDGE_ALIGN_PARAM_CARRIER_TAPE_FIRST_EDGE_MIN_THRESHOLD,
    EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_MICRO,
    EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_PIXEL_X,

    EDGE_ALIGN_PARAM_POCKET_IMAGE_COMBINE,
    EDGE_ALIGN_PARAM_POCKET_FRAME_INDEX,
    EDGE_ALIGN_PARAM_POCKET_EDGE_DETECT_MODE,
    EDGE_ALIGN_PARAM_POCKET_FIRST_EDGE_MIN_THRESHOLD,
    EDGE_ALIGN_PARAM_POCKET_GAP_MICRO,
    EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_X,
    EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_Y,
};

enum enumSelectRoughAlign
{
    SELECT_ROUGH_ALIGN_START = 0,
    SELECT_ROUGH_ALIGN_NOT_USE = SELECT_ROUGH_ALIGN_START,
    SELECT_ROUGH_ALIGN_PROFILE_ALIGN,
    SELECT_ROUGH_ALIGN_END,
};

enum enumSearchDirection
{
    SEARCH_DIRECTION_START = 0,
    SEARCH_DIRECTION_IN_OUT = SEARCH_DIRECTION_START,
    SEARCH_DIRECTION_OUT_IN,
    SEARCH_DIRECTION_END,
};

enum enumEdgeDirection
{
    EDGE_DIRECTION_START = 0,
    EDGE_DIRECTION_FALLING = EDGE_DIRECTION_START,
    EDGE_DIRECTION_RISING,
    EDGE_DIRECTION_END,
};

enum enumEdgeDetectMode
{
    EDGE_DETECT_MODE_START = 0,
    EDGE_DETECT_MODE_BEST = EDGE_DETECT_MODE_START,
    EDGE_DETECT_MODE_FIRST,
    EDGE_DETECT_MODE_AUTO,
    EDGE_DETECT_MODE_END,
};

PropertyGrid::PropertyGrid(VisionProcessing& processor, Inspection& inspection, ImageLotView* imageLotView,
    ProcCommonDebugInfoDlg& procDebugInfo, Para& para)
    : m_processor(&processor)
    , m_inspection(inspection)
    , m_imageLotView(imageLotView)
    , m_procDebugInfo(procDebugInfo)
    , m_para(&para)
{
    m_propertyGrid[(int32_t)CurInspectionType::e_carrierTapeAlign] = new CXTPPropertyGrid;
    m_propertyGrid[(int32_t)CurInspectionType::e_pocketAlign] = new CXTPPropertyGrid;
}

PropertyGrid::~PropertyGrid()
{
    m_inspection.GetResult().FreeMemoryForCalculation();

    for (auto* propertyGrid : m_propertyGrid)
    {
        if (propertyGrid->GetSafeHwnd())
        {
            propertyGrid->ShowWindow(SW_HIDE);
        }
        delete propertyGrid;
    }

    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        for (auto* dlg : m_edgeSearchRoiPara)
        {
            delete dlg;
        }
    }
}

long PropertyGrid::getGridCount() const
{
    return 2;
}

bool PropertyGrid::create(long gridIndex, const RECT& rect, HWND parentWnd)
{
    auto* propertyGrid = m_propertyGrid[gridIndex];

    CWnd* pParentWnd = CWnd::FromHandle(parentWnd);
    auto modifiedRect = rect;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWnd* pWnd = CWnd::FromHandle(parentWnd);

    int roiParaSizeY = (rect.bottom - rect.top) / 2;
    m_edgeSearchRoiPara[gridIndex] = new DlgSearchRoiPara(*m_para, this, gridIndex, pWnd);
    m_edgeSearchRoiPara[gridIndex]->Create(IDD_SEARCH_ROI_PARA, pWnd);
    m_edgeSearchRoiPara[gridIndex]->MoveWindow(
        rect.left, rect.top + roiParaSizeY, rect.right - rect.left, roiParaSizeY);
    modifiedRect.bottom = rect.top + roiParaSizeY;

    if (!propertyGrid->Create(modifiedRect, pParentWnd, IDC_FRAME_PROPERTY_GRID + gridIndex))
    {
        return false;
    }

    propertyGrid->ShowWindow(SW_HIDE);
    propertyGrid->SetViewDivider(0.5);
    propertyGrid->HighlightChangedItems(TRUE);

    return true;
}

LPCTSTR PropertyGrid::getName(long gridIndex) const
{
    switch ((CurInspectionType)gridIndex)
    {
        case CurInspectionType::e_carrierTapeAlign:
            return _T("Carrier Tape Align");
            break;
        case CurInspectionType::e_pocketAlign:
            return _T("Pocket Align");
            break;
    }

    return _T("");
}

void PropertyGrid::active(long gridIndex)
{
    m_processor->DoInspection(true, m_processor->GetCurVisionModule_Status());
    m_imageLotView->SetMode(ImageLotView::Mode::SpecifyImage);

    switch ((CurInspectionType)gridIndex)
    {
        case CurInspectionType::e_carrierTapeAlign:
            InitPropertyCarrierTape(gridIndex);
            m_procDebugInfo.Display(1);

            m_edgeSearchRoiPara[gridIndex]->UpdateList();
            m_edgeSearchRoiPara[gridIndex]->ShowWindow(SW_SHOW);
            break;

        case CurInspectionType::e_pocketAlign:
            InitPropertyPocket(gridIndex);
            m_procDebugInfo.Display(1);

            m_edgeSearchRoiPara[gridIndex]->UpdateList();
            m_edgeSearchRoiPara[gridIndex]->ShowWindow(SW_SHOW);
            break;
    }

    ShowDisplayImage(gridIndex);
    m_imageLotView->ZoomImageFit();
}

void PropertyGrid::deactivate(long gridIndex)
{
    m_edgeSearchRoiPara[gridIndex]->ShowWindow(SW_HIDE);
    m_propertyGrid[gridIndex]->ShowWindow(SW_HIDE);
}

bool PropertyGrid::notify(long gridIndex, WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch ((CurInspectionType)gridIndex)
        {
            case CurInspectionType::e_carrierTapeAlign:
                NotifyCarrierTape(gridIndex, item);
                break;

            case CurInspectionType::e_pocketAlign:
                NotifyPocket(gridIndex, item);
                break;
        }

        UpdateROI((CurInspectionType)gridIndex);
    }

    return false;
}
void PropertyGrid::event_afterInspection(long gridIndex)
{
    switch ((CurInspectionType)gridIndex)
    {
        case CurInspectionType::e_carrierTapeAlign:
            SetCarrierTapeROI();
            break;

        case CurInspectionType::e_pocketAlign:
            SetPocketROI();
            break;
    }

    ShowDisplayImage(gridIndex);

    auto& result = m_inspection.GetResult();
    if ((CurInspectionType)gridIndex == CurInspectionType::e_carrierTapeAlign)
    {
        // Tape이 첫번이라 모든 결과 보여주기

        m_imageLotView->Overlay_RemoveAll();
        m_imageLotView->Overlay_AddLine(result.fptLT, result.fptRT, RGB(0, 255, 0));
        m_imageLotView->Overlay_AddLine(result.fptRT, result.fptRB, RGB(0, 255, 0));
        m_imageLotView->Overlay_AddLine(result.fptLB, result.fptRB, RGB(0, 255, 0));
        m_imageLotView->Overlay_AddLine(result.fptLT, result.fptLB, RGB(0, 255, 0));
        m_imageLotView->Overlay_AddRectangle(result.m_roughPocketAlignRect, RGB(0, 255, 0));
        m_imageLotView->Overlay_Show(TRUE);
    }
    else if ((CurInspectionType)gridIndex == CurInspectionType::e_pocketAlign)
    {
        // Pocket Align 결과만 보여주기
        m_imageLotView->Overlay_RemoveAll();
        m_imageLotView->Overlay_AddRectangle(result.m_roughPocketAlignRect, RGB(0, 255, 0));
        m_imageLotView->Overlay_Show(TRUE);
    }
}

void PropertyGrid::event_changedRoi(long gridIndex)
{
    // 동작 없음
}

void PropertyGrid::event_changedPane(long gridIndex)
{
    m_processor->DoInspection(true);

    switch ((CurInspectionType)gridIndex)
    {
        case CurInspectionType::e_carrierTapeAlign:
            SetCarrierTapeROI();
            break;
        case CurInspectionType::e_pocketAlign:
            SetPocketROI();
            break;
    }

    ShowDisplayImage(gridIndex);

    m_imageLotView->ZoomImageFit();
}

void PropertyGrid::event_changedRaw(long gridIndex)
{
    ShowDisplayImage(gridIndex);
}

void PropertyGrid::event_changedImage(long gridIndex)
{
    ShowDisplayImage(gridIndex);
}

void PropertyGrid::RedrawEdgeRoi(long gridIndex)
{
    switch ((CurInspectionType)gridIndex)
    {
        case CurInspectionType::e_carrierTapeAlign:
            SetCarrierTapeROI();
            break;
        case CurInspectionType::e_pocketAlign:
            SetPocketROI();
            break;
    }
}

void PropertyGrid::UpdateROI(CurInspectionType gridIndex)
{
    switch (gridIndex)
    {
        case CurInspectionType::e_carrierTapeAlign:
            SetCarrierTapeROI();
            break;
        case CurInspectionType::e_pocketAlign:
            SetPocketROI();
            break;
    }
}

CString GetKey(long type)
{
    CString key;
    key.Format(_T("%d"), type);

    return key;
}

void PropertyGrid::SetCarrierTapeROI()
{
    if (m_imageLotView == nullptr)
    {
        return;
    }

    const auto* tapeSpec = m_processor->getTapeSpec();
    if (tapeSpec == nullptr)
    {
        return;
    }

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    auto& result = m_inspection.GetResult();
    const auto& paneCenter = result.getPaneCenter();

    auto carrierTapeWidth_um = tapeSpec->m_w0 * 1000.f;
    auto pocketSizeY_um = tapeSpec->m_a0 * 1000.f;

    const auto& scale = m_processor->getScale();

    float carrierTapeWidth_px = scale.convert_umToPixelX(carrierTapeWidth_um);
    float pocketSizeY_px = scale.convert_umToPixelY(pocketSizeY_um);

    for (auto searchDirection = 0; searchDirection < 2; searchDirection++)
    {
        auto bodyWidth_px = carrierTapeWidth_px;
        auto direction = LEFT + searchDirection;

        const auto& para = m_para->m_carrierTapeAlign.m_edgeSearchRois[searchDirection];
        CString strName = GetDirectionName(false, searchDirection);
        Ipvm::Rect32s rtROI = para.ToRect(scale, direction, paneCenter, bodyWidth_px, pocketSizeY_px);
        m_imageLotView->ROI_Add(GetKey(searchDirection), strName, rtROI, RGB(255, 0, 0), FALSE, FALSE);
    }

    m_imageLotView->ROI_Show(TRUE);
    m_imageLotView->Overlay_Show(TRUE);
}

void PropertyGrid::SetPocketROI()
{
    if (m_imageLotView == nullptr)
    {
        return;
    }

    const auto* tapeSpec = m_processor->getTapeSpec();
    if (tapeSpec == nullptr)
    {
        return;
    }

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    auto& result = m_inspection.GetResult();
    auto pocketCenter = result.getPaneCenter();

    if (result.m_roughPocketCenter_px.m_x != Ipvm::k_noiseValue32r)
    {
        // Pocket Center 위치가 이미 계산되어 있으면 그것을 사용한다
        pocketCenter.m_x = result.m_roughPocketCenter_px.m_x;
    }

    if (result.m_roughPocketCenter_px.m_y != Ipvm::k_noiseValue32r)
    {
        // Pocket Center 위치가 이미 계산되어 있으면 그것을 사용한다
        pocketCenter.m_y = result.m_roughPocketCenter_px.m_y;
    }

    auto pocketSizeX_um = tapeSpec->m_b0 * 1000.f;
    auto pocketSizeY_um = tapeSpec->m_a0 * 1000.f;

    const auto& scale = m_processor->getScale();

    float pocketSizeX_px = scale.convert_umToPixelX(pocketSizeX_um);
    float pocketSizeY_px = scale.convert_umToPixelY(pocketSizeY_um);

    for (auto searchDirection = 0; searchDirection < 4; searchDirection++)
    {
        auto direction = searchDirection;
        const auto& para = m_para->m_pocketAlign.m_edgeSearchRois[searchDirection];
        CString strName = GetDirectionName(true, searchDirection);
        Ipvm::Rect32s rtROI = para.ToRect(scale, direction, pocketCenter, pocketSizeX_px, pocketSizeY_px);
        m_imageLotView->ROI_Add(GetKey(searchDirection), strName, rtROI, RGB(255, 0, 0), FALSE, FALSE);
    }

    m_imageLotView->ROI_Show(TRUE);
    m_imageLotView->Overlay_Show(TRUE);
}

void PropertyGrid::ShowDisplayImage(long gridIndex)
{
    switch ((CurInspectionType)gridIndex)
    {
        case CurInspectionType::e_carrierTapeAlign:
            ShowCarierTapeEdgeAlignImage();
            break;

        case CurInspectionType::e_pocketAlign:
            ShowPocketEdgeAlignImage();
            break;
    }
}

void PropertyGrid::ShowCarierTapeEdgeAlignImage()
{
    const auto& result = m_inspection.GetResult();

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);
    m_imageLotView->SetImage(result.getImage(false), _T("Edge Align Image **"));
}

void PropertyGrid::ShowPocketEdgeAlignImage()
{
    const auto& result = m_inspection.GetResult();

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);
    m_imageLotView->SetImage(result.getImage(true), _T("Edge Align Image **"));
}

void PropertyGrid::ClickedButtonCarrierTapeImageCombine()
{
    Ipvm::Rect32s rtPaneROI = m_inspection.getImageSourceRoiForNoStitch();
    auto bRet = ::Combine_SetParameter(
        *m_processor,
        m_para->m_carrierTapeAlign.m_imageFrameIndex.getFrameIndex(),
        true,
        &m_para->m_carrierTapeAlign.m_imageProc, rtPaneROI)
        == IDOK; //파라미터 중 true는 원래 false 였음.. 바꾼 이유는 Align 시에는 Raw Image 기준으로 돌아야 한다.

    if (bRet == true)
    {
        m_processor->DoInspection(true);
        ShowCarierTapeEdgeAlignImage();
    }
}

void PropertyGrid::ClickedButtonPocketImageCombine()
{
    Ipvm::Rect32s rtPaneROI = m_inspection.getImageSourceRoiForNoStitch();
    auto bRet = ::Combine_SetParameter(
        *m_processor,
        m_para->m_pocketAlign.m_imageFrameIndex.getFrameIndex(),
        true,
        &m_para->m_pocketAlign.m_imageProc, rtPaneROI)
        == IDOK; //파라미터 중 true는 원래 false 였음.. 바꾼 이유는 Align 시에는 Raw Image 기준으로 돌아야 한다.

    if (bRet == true)
    {
        m_processor->DoInspection(true);
        ShowPocketEdgeAlignImage();
    }
}

void PropertyGrid::InitPropertyCarrierTape(long gridIndex)
{
    const auto& scale = m_processor->getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    auto* propertyGrid = m_propertyGrid[gridIndex];

    propertyGrid->ResetContent();

    if (auto* category = propertyGrid->AddCategory(_T("Edge Align Paramter")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(EDGE_ALIGN_PARAM_CARRIER_TAPE_IMAGE_COMBINE);
        m_para->m_carrierTapeAlign.m_imageFrameIndex.makePropertyGridItem(
            category, _T("Align Image Frame Index"), EDGE_ALIGN_PARAM_CARRIER_TAPE_FRAME_INDEX);
        category->Expand();
    }

    if (auto* category = propertyGrid->AddCategory(_T("Carrier Tape - Edge Detect Parameter")))
    {
        auto& edgeParams = m_para->m_carrierTapeAlign.m_edgeParams;
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Search Direction"), edgeParams.m_searchDirection, (int*)&edgeParams.m_searchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), SEARCH_DIRECTION_IN_OUT);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), SEARCH_DIRECTION_OUT_IN);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Direction"), edgeParams.m_edgeType, (int*)&edgeParams.m_edgeType)))
        {
            item->GetConstraints()->AddConstraint(_T("Falling"), EDGE_DIRECTION_FALLING);
            item->GetConstraints()->AddConstraint(_T("Rising"), EDGE_DIRECTION_RISING);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Detect Mode"), edgeParams.m_edgeDetectMode, (int*)&edgeParams.m_edgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), EDGE_DETECT_MODE_BEST);
            item->GetConstraints()->AddConstraint(_T("First Edge"), EDGE_DETECT_MODE_FIRST);
            item->GetConstraints()->AddConstraint(_T("Auto Detect"), EDGE_DETECT_MODE_AUTO);

            item->SetID(EDGE_ALIGN_PARAM_CARRIER_TAPE_EDGE_DETECT_MODE);
        }
        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("FirstEdge Min Value"), edgeParams.m_firstEdgeMinThreshold, _T("%.3lf GV")))
            ->SetID(EDGE_ALIGN_PARAM_CARRIER_TAPE_FIRST_EDGE_MIN_THRESHOLD);
        auto* edgeAlignGap = category->AddChildItem(new CXTPPropertyGridItemDouble(
            _T("Detected EdgeAlign Gap(um)"), edgeParams.m_edgeAlignGap_um, _T("%.3lf um")));
        edgeAlignGap->SetID(EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_MICRO);

        long nGapToPixel_X = (long)((edgeParams.m_edgeAlignGap_um / fpx2um_x) + 0.5f);
        long nGapToPixel_Y = (long)((edgeParams.m_edgeAlignGap_um / fpx2um_y) + 0.5f);

        if (nGapToPixel_X < 2 || nGapToPixel_Y < 2)
        {
            nGapToPixel_X = 2;
            nGapToPixel_Y = 2;
            edgeParams.m_edgeAlignGap_um = nGapToPixel_X * fpx2um_x;
        }
        edgeAlignGap->AddChildItem(new CXTPPropertyGridItemNumber(_T("Detected EdgeAlign Gap X(px)"), nGapToPixel_X))
            ->SetID(EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_PIXEL_X);

        category->Expand();
        if (edgeParams.m_edgeDetectMode != EDGE_DETECT_MODE_FIRST)
        {
            propertyGrid->FindItem(EDGE_ALIGN_PARAM_CARRIER_TAPE_FIRST_EDGE_MIN_THRESHOLD)->SetReadOnly(TRUE);
        }
    }

    propertyGrid->ShowWindow(SW_SHOW);
    UpdateROI((CurInspectionType)gridIndex);
}

void PropertyGrid::InitPropertyPocket(long gridIndex)
{
    const auto& scale = m_processor->getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    auto* propertyGrid = m_propertyGrid[gridIndex];

    propertyGrid->ResetContent();

    if (auto* category = propertyGrid->AddCategory(_T("Edge Align Paramter")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(EDGE_ALIGN_PARAM_POCKET_IMAGE_COMBINE);
        m_para->m_pocketAlign.m_imageFrameIndex.makePropertyGridItem(
            category, _T("Align Image Frame Index"), EDGE_ALIGN_PARAM_POCKET_FRAME_INDEX);
        category->Expand();
    }

    if (auto* category = propertyGrid->AddCategory(_T("Pocket - Edge Detect Parameter")))
    {
        auto& edgeParams = m_para->m_pocketAlign.m_edgeParams;
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Search Direction"), edgeParams.m_searchDirection, (int*)&edgeParams.m_searchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), SEARCH_DIRECTION_IN_OUT);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), SEARCH_DIRECTION_OUT_IN);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Direction"), edgeParams.m_edgeType, (int*)&edgeParams.m_edgeType)))
        {
            item->GetConstraints()->AddConstraint(_T("Falling"), EDGE_DIRECTION_FALLING);
            item->GetConstraints()->AddConstraint(_T("Rising"), EDGE_DIRECTION_RISING);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Detect Mode"), edgeParams.m_edgeDetectMode, (int*)&edgeParams.m_edgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), EDGE_DETECT_MODE_BEST);
            item->GetConstraints()->AddConstraint(_T("First Edge"), EDGE_DETECT_MODE_FIRST);
            item->GetConstraints()->AddConstraint(_T("Auto Detect"), EDGE_DETECT_MODE_AUTO);

            item->SetID(EDGE_ALIGN_PARAM_POCKET_EDGE_DETECT_MODE);
        }
        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("FirstEdge Min Value"), edgeParams.m_firstEdgeMinThreshold, _T("%.3lf GV")))
            ->SetID(EDGE_ALIGN_PARAM_POCKET_FIRST_EDGE_MIN_THRESHOLD);
        auto* edgeAlignGap = category->AddChildItem(new CXTPPropertyGridItemDouble(
            _T("Detected EdgeAlign Gap(um)"), edgeParams.m_edgeAlignGap_um, _T("%.3lf um")));
        edgeAlignGap->SetID(EDGE_ALIGN_PARAM_POCKET_GAP_MICRO);

        long nGapToPixel_X = (long)((edgeParams.m_edgeAlignGap_um / fpx2um_x) + 0.5f);
        long nGapToPixel_Y = (long)((edgeParams.m_edgeAlignGap_um / fpx2um_y) + 0.5f);

        if (nGapToPixel_X < 2 || nGapToPixel_Y < 2)
        {
            nGapToPixel_X = 2;
            nGapToPixel_Y = 2;
            edgeParams.m_edgeAlignGap_um = nGapToPixel_X * fpx2um_x;
        }
        edgeAlignGap->AddChildItem(new CXTPPropertyGridItemNumber(_T("Detected EdgeAlign Gap X(px)"), nGapToPixel_X))
            ->SetID(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_X);
        edgeAlignGap->AddChildItem(new CXTPPropertyGridItemNumber(_T("Detected EdgeAlign Gap Y(px)"), nGapToPixel_Y))
            ->SetID(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_Y);

        category->Expand();
        if (edgeParams.m_edgeDetectMode != EDGE_DETECT_MODE_FIRST)
        {
            propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_FIRST_EDGE_MIN_THRESHOLD)->SetReadOnly(TRUE);
        }
    }

    propertyGrid->ShowWindow(SW_SHOW);

    UpdateROI((CurInspectionType)gridIndex);
}

bool PropertyGrid::NotifyCommon(long gridIndex, CXTPPropertyGridItem* item)
{
    switch (item->GetID())
    {
        case EDGE_ALIGN_PARAM_CARRIER_TAPE_IMAGE_COMBINE:
            ClickedButtonCarrierTapeImageCombine();
            return true;

        case EDGE_ALIGN_PARAM_POCKET_IMAGE_COMBINE:
            ClickedButtonPocketImageCombine();
            return true;
            
        case EDGE_ALIGN_PARAM_CARRIER_TAPE_FRAME_INDEX:
            m_processor->DoInspection(true);
            ShowDisplayImage(gridIndex);
            return true;

        case EDGE_ALIGN_PARAM_POCKET_FRAME_INDEX:
            m_processor->DoInspection(true);
            ShowDisplayImage(gridIndex);
            return true;

    }

    return false;
}

void PropertyGrid::NotifyCarrierTape(long gridIndex, CXTPPropertyGridItem* item)
{
    if (NotifyCommon(gridIndex, item))
    {
        return;
    }

    const auto* propertyGrid = m_propertyGrid[gridIndex];

    CString strTemp;

    const auto& scale = m_processor->getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    long nGapToPixel_X = 0;
    long nGapToPixel_Y = 0;
    float nGapToum = 0.f;

    switch (item->GetID())
    {
        case EDGE_ALIGN_PARAM_CARRIER_TAPE_EDGE_DETECT_MODE:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                m_para->m_carrierTapeAlign.m_edgeParams.m_edgeDetectMode = value->GetEnum();
            if (m_para->m_carrierTapeAlign.m_edgeParams.m_edgeDetectMode == EDGE_DETECT_MODE_FIRST)
            {
                propertyGrid->FindItem(EDGE_ALIGN_PARAM_CARRIER_TAPE_FIRST_EDGE_MIN_THRESHOLD)->SetReadOnly(FALSE);
            }
            else
            {
                propertyGrid->FindItem(EDGE_ALIGN_PARAM_CARRIER_TAPE_FIRST_EDGE_MIN_THRESHOLD)->SetReadOnly(TRUE);
            }
            break;
        case EDGE_ALIGN_PARAM_CARRIER_TAPE_FIRST_EDGE_MIN_THRESHOLD:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
            {
                auto& threshold = m_para->m_carrierTapeAlign.m_edgeParams.m_firstEdgeMinThreshold;

                if (value->GetDouble() < 5.f || value->GetDouble() > 250.f)
                {
                    threshold = 5.f;
                    strTemp.Format(_T("%.3lf GV"), threshold);
                    item->SetValue(strTemp);
                }
                else
                    threshold = (float)value->GetDouble();
            }
            break;
        case EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_MICRO:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
            {
                auto& gap_um = m_para->m_carrierTapeAlign.m_edgeParams.m_edgeAlignGap_um;

                nGapToPixel_X = (long)((value->GetDouble() / fpx2um_x) + 0.5f);
                nGapToPixel_Y = (long)((value->GetDouble() / fpx2um_y) + 0.5f);

                if (nGapToPixel_X < 2 || nGapToPixel_Y < 2)
                {
                    nGapToPixel_X = 2;
                    nGapToPixel_Y = 2;
                    gap_um = nGapToPixel_X * fpx2um_x;

                    strTemp.Format(_T("%.3lf um"), gap_um);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_MICRO)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_X);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_PIXEL_X)->SetValue(strTemp);
                }
                else
                {
                    gap_um = (float)value->GetDouble();

                    strTemp.Format(_T("%d"), nGapToPixel_X);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_PIXEL_X)->SetValue(strTemp);
                }
            }
            break;
        case EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_PIXEL_X:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
            {
                auto& gap_um = m_para->m_carrierTapeAlign.m_edgeParams.m_edgeAlignGap_um;

                if (value->GetNumber() < 2.f)
                {
                    nGapToPixel_X = 2;
                    nGapToPixel_Y = 2;
                    nGapToum = nGapToPixel_X * fpx2um_x;

                    gap_um = nGapToum;

                    strTemp.Format(_T("%.3lf um"), gap_um);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_MICRO)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_X);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_PIXEL_X)->SetValue(strTemp);
                }
                else
                {
                    nGapToum = value->GetNumber() * fpx2um_x;
                    nGapToPixel_Y = (long)((nGapToum / fpx2um_y) + 0.5f);

                    gap_um = nGapToum;

                    strTemp.Format(_T("%.3lf um"), gap_um);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_CARRIER_TAPE_GAP_MICRO)->SetValue(strTemp);
                }
            }
            break;
    }
}

void PropertyGrid::NotifyPocket(long gridIndex, CXTPPropertyGridItem* item)
{
    if (NotifyCommon(gridIndex, item))
    {
        return;
    }

    auto* propertyGrid = m_propertyGrid[gridIndex];

    CString strTemp;

    const auto& scale = m_processor->getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    long nGapToPixel_X = 0;
    long nGapToPixel_Y = 0;
    float nGapToum = 0.f;

    switch (item->GetID())
    {
        case EDGE_ALIGN_PARAM_POCKET_EDGE_DETECT_MODE:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                m_para->m_pocketAlign.m_edgeParams.m_edgeDetectMode = value->GetEnum();
            if (m_para->m_pocketAlign.m_edgeParams.m_edgeDetectMode == EDGE_DETECT_MODE_FIRST)
            {
                propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_FIRST_EDGE_MIN_THRESHOLD)->SetReadOnly(FALSE);
            }
            else
            {
                propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_FIRST_EDGE_MIN_THRESHOLD)->SetReadOnly(TRUE);
            }
            break;
        case EDGE_ALIGN_PARAM_POCKET_FIRST_EDGE_MIN_THRESHOLD:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
            {
                auto& threshold = m_para->m_pocketAlign.m_edgeParams.m_firstEdgeMinThreshold;

                if (value->GetDouble() < 5.f || value->GetDouble() > 250.f)
                {
                    threshold = 5.f;
                    strTemp.Format(_T("%.3lf GV"), threshold);
                    item->SetValue(strTemp);
                }
                else
                    threshold = (float)value->GetDouble();
            }
            break;
        case EDGE_ALIGN_PARAM_POCKET_GAP_MICRO:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
            {
                auto& gap_um = m_para->m_pocketAlign.m_edgeParams.m_edgeAlignGap_um;

                nGapToPixel_X = (long)((value->GetDouble() / fpx2um_x) + 0.5f);
                nGapToPixel_Y = (long)((value->GetDouble() / fpx2um_y) + 0.5f);

                if (nGapToPixel_X < 2 || nGapToPixel_Y < 2)
                {
                    nGapToPixel_X = 2;
                    nGapToPixel_Y = 2;
                    gap_um = nGapToPixel_X * fpx2um_x;

                    strTemp.Format(_T("%.3lf um"), gap_um);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_MICRO)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_X);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_X)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_Y);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_Y)->SetValue(strTemp);
                }
                else
                {
                    gap_um = (float)value->GetDouble();

                    strTemp.Format(_T("%d"), nGapToPixel_X);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_X)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_Y);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_Y)->SetValue(strTemp);
                }
            }
            break;
        case EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_X:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
            {
                auto& gap_um = m_para->m_pocketAlign.m_edgeParams.m_edgeAlignGap_um;

                if (value->GetNumber() < 2.f)
                {
                    nGapToPixel_X = 2;
                    nGapToPixel_Y = 2;
                    nGapToum = nGapToPixel_X * fpx2um_x;

                    gap_um = nGapToum;

                    strTemp.Format(_T("%.3lf um"), gap_um);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_MICRO)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_X);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_X)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_Y);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_Y)->SetValue(strTemp);
                }
                else
                {
                    nGapToum = value->GetNumber() * fpx2um_x;
                    nGapToPixel_Y = (long)((nGapToum / fpx2um_y) + 0.5f);

                    gap_um = nGapToum;

                    strTemp.Format(_T("%.3lf um"), gap_um);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_MICRO)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_Y);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_Y)->SetValue(strTemp);
                }
            }
            break;

        case EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_Y:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
            {
                auto& gap_um = m_para->m_pocketAlign.m_edgeParams.m_edgeAlignGap_um;

                if (value->GetNumber() < 2.f)
                {
                    nGapToPixel_X = 2;
                    nGapToPixel_Y = 2;
                    nGapToum = nGapToPixel_Y * fpx2um_y;

                    gap_um = nGapToum;

                    strTemp.Format(_T("%.3lf um"), gap_um);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_MICRO)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_X);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_X)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_Y);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_Y)->SetValue(strTemp);
                }
                else
                {
                    nGapToum = value->GetNumber() * fpx2um_y;
                    nGapToPixel_X = (long)((nGapToum / fpx2um_x) + 0.5f);

                    gap_um = nGapToum;

                    strTemp.Format(_T("%.3lf um"), gap_um);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_MICRO)->SetValue(strTemp);
                    strTemp.Format(_T("%d"), nGapToPixel_X);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_POCKET_GAP_PIXEL_X)->SetValue(strTemp);
                }
            }
            break;
    }
}
