//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PropertyGrid_Impl.h"

//CPP_2_________________________________ This project's headers
#include "CDlgAutoFocusImageSet.h"
#include "DlgSearchRoiPara.h"
#include "Inspection.h"
#include "Para.h"
#include "Resource.h"
#include "Result_2DEmpty.h"
#include "Result_EdgeAlign.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/Config.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../VisionCommon/VisionAlignResult.h"
#include "../VisionCommon/VisionImageLot.h"
#include "../VisionCommon/VisionProcessing.h"

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
    EDGE_ALIGN_PARAM_IMAGE_COMBINE = 1,
    EDGE_ALIGN_PARAM_FRAME_INDEX,

    EDGE_ALIGN_PARAM_INSP_EMPTY,
    EDGE_ALIGN_PARAM_INSP_DOUBLE,
    EDGE_ALIGN_PARAM_INSP_EMPTY_CHECK_RATIO,
    EDGE_ALIGN_PARAM_HEIGHTOFTRAYWALLFROMMOLD,
    EDGE_ALIGN_PARAM_ROUGH_TAP,

    EDGE_ALIGN_PARAM_SEARCH_DIRECTION,
    EDGE_ALIGN_PARAM_EDGE_DIRECTION,
    EDGE_ALIGN_PARAM_EDGE_DETECT_MODE,
    EDGE_ALIGN_PARAM_FIRST_EDGE_MIN_THRESHOLD,
    EDGE_ALIGN_PARAM_GAP_MICRO,
    EDGE_ALIGN_PARAM_GAP_PIXEL_X,
    EDGE_ALIGN_PARAM_GAP_PIXEL_Y,

    EMPTYCHECK_2D_PARAM_FRAME_INDEX,
    EMPTYCHECK_2D_ALGO_PARAM_THRESHOLD,
    EMPTYCHECK_2D_ALGO_PARAM_IGNORE_ROI,
    EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_UM,
    EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_PX,
    EMPTYCHECK_2D_ALGO_PARAM_BLOB_COUNT,
    EMPTYCHECK_2D_ALGO_PARAM_REVERSE_THRESHOLD,
    EMPTYCHECK_2D_ALGO_PARAM_USE_BLOB_FILTER,
    EMPTYCHECK_2D_RESULT_TEXT,
    EMPTYCHECK_2D_RESULT_BLOB_COUNT,

    EDGE_ALIGN_PARAM_SearhPara_Left,
    EDGE_ALIGN_PARAM_SearhPara_Top,
    EDGE_ALIGN_PARAM_SearhPara_Right,
    EDGE_ALIGN_PARAM_SearhPara_Bottom,
    EDGE_ALIGN_PARAM_SearhPara_LowTop,

    EDGE_ALIGN_PARAM_AUTO_FOCUS_IMAGE_SET, // JHB_AutoFocusImageSet
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

using namespace VisionEdgeAlign;

PropertyGrid_Impl::PropertyGrid_Impl(VisionProcessing& processor, Inspection& inspection, ImageLotView* imageLotView,
    ProcCommonDebugInfoDlg& procDebugInfo, VisionEdgeAlign::Para& para)
    : m_processor(&processor)
    , m_inspection(inspection)
    , m_imageLotView(imageLotView)
    , m_procDebugInfo(procDebugInfo)
    , m_para(&para)
    , m_edgeSearchRoiPara(nullptr)
{
    m_propertyGrid[0] = new CXTPPropertyGrid;
    m_propertyGrid[1] = new CXTPPropertyGrid;

    if (Config::getVisionType() == VISIONTYPE_3D_INSP)
    {
        m_propertyIndexs[0] = 0;
    }
    else
    {
        m_propertyIndexs[0] = 1;
        m_propertyIndexs[1] = 0;
    }
}

PropertyGrid_Impl::~PropertyGrid_Impl()
{
    m_inspection.getResult_EdgeAlign()->FreeMemoryForCalculation();
    m_inspection.getResult_2DEmpty()->FreeMemoryForCalculation();

    if (m_propertyGrid[0]->GetSafeHwnd())
        m_propertyGrid[0]->DestroyWindow();
    if (m_propertyGrid[1]->GetSafeHwnd())
        m_propertyGrid[1]->DestroyWindow();

    delete m_propertyGrid[0];
    delete m_propertyGrid[1];

    if (true)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        delete m_edgeSearchRoiPara;
    }
}

void PropertyGrid_Impl::redrawEdgeRoi()
{
    SetROI_edgeAlign();
}

long PropertyGrid_Impl::getGridCount() const
{
    if (Config::getVisionType() == VISIONTYPE_3D_INSP)
    {
        return 1;
    }

    return 2;
}

bool PropertyGrid_Impl::create(long gridIndex, RECT rect, HWND parentWnd)
{
    auto* propertyGrid = m_propertyGrid[m_propertyIndexs[gridIndex]];

    CWnd* pParentWnd = CWnd::FromHandle(parentWnd);

    if (m_propertyIndexs[gridIndex] == 0)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        CWnd* pWnd = CWnd::FromHandle(parentWnd);

        int roiParaSizeY = (rect.bottom - rect.top) / 2;
        m_edgeSearchRoiPara = new DlgSearchRoiPara(*m_para, this, pWnd);
        m_edgeSearchRoiPara->Create(IDD_SEARCH_ROI_PARA, pWnd);
        m_edgeSearchRoiPara->MoveWindow(rect.left, rect.top + roiParaSizeY, rect.right - rect.left, roiParaSizeY);
        rect.bottom = rect.top + roiParaSizeY;
    }

    if (!propertyGrid->Create(rect, pParentWnd, IDC_FRAME_PROPERTY_GRID + gridIndex))
    {
        return false;
    }

    propertyGrid->ShowWindow(SW_HIDE);
    propertyGrid->SetViewDivider(0.5);
    propertyGrid->HighlightChangedItems(TRUE);

    return true;
}

bool PropertyGrid_Impl::notify(long gridIndex, WPARAM wparam, LPARAM lparam)
{
    switch (m_propertyIndexs[gridIndex])
    {
        case 0:
            return notify_edgeAlign(gridIndex, wparam, lparam);
        case 1:
            return notify_empty2D(gridIndex, wparam, lparam);
            ;
    }

    return false;
}

LPCTSTR PropertyGrid_Impl::getName(long gridIndex) const
{
    switch (m_propertyIndexs[gridIndex])
    {
        case Type_EdgeAlign:
            return _T("Edge Align");
            break;
        case Type_EmptyChecker:
            return _T("Empty");
            break;
    }

    return _T("");
}

void PropertyGrid_Impl::active(long gridIndex)
{
    m_processor->DoInspection(true, m_processor->GetCurVisionModule_Status());
    m_imageLotView->SetMode(ImageLotView::Mode::SpecifyImage);

    switch (m_propertyIndexs[gridIndex])
    {
        case Type_EdgeAlign:
            active_edgeAlign(gridIndex);
            m_procDebugInfo.Display(1);

            m_edgeSearchRoiPara->UpdateList();
            m_edgeSearchRoiPara->ShowWindow(SW_SHOW);
            break;
        case Type_EmptyChecker:
            active_empty2D(gridIndex);
            m_procDebugInfo.Display(2);
            break;
    }

    ShowDisplayImage(gridIndex);
    m_imageLotView->ZoomImageFit();
}

void PropertyGrid_Impl::deactivate(long gridIndex)
{
    switch (m_propertyIndexs[gridIndex])
    {
        case 0:
            GetROI_edgeAlign();
            break;
        case 1:
            GetROI_empty2D();
            break;
    }

    m_edgeSearchRoiPara->ShowWindow(SW_HIDE);
    m_propertyGrid[m_propertyIndexs[gridIndex]]->ShowWindow(SW_HIDE);
}

void PropertyGrid_Impl::event_afterInspection(long gridIndex)
{
    switch (m_propertyIndexs[gridIndex])
    {
        case Type_EdgeAlign:
            SetROI_edgeAlign();

            break;
        case Type_EmptyChecker:
            SetROI_empty2D();

            m_resultEmpty_Text = m_processor->m_bEmpty ? _T("EMPTY") : _T("PASS");
            m_resultEmpty_BlobCount.Format(_T("%d"), m_inspection.getResult_2DEmpty()->getBlobCount());
            m_propertyGrid[m_propertyIndexs[gridIndex]]
                ->FindItem(EMPTYCHECK_2D_RESULT_TEXT)
                ->SetValue(m_resultEmpty_Text);
            m_propertyGrid[m_propertyIndexs[gridIndex]]
                ->FindItem(EMPTYCHECK_2D_RESULT_BLOB_COUNT)
                ->SetValue(m_resultEmpty_BlobCount);
            break;
    }

    ShowDisplayImage(gridIndex);

    if (m_propertyIndexs[gridIndex] == 0)
    {
        // Body Align 결과 보여주기
        auto* result = m_inspection.getResult_EdgeAlign();

        m_imageLotView->Overlay_RemoveAll();
        m_imageLotView->Overlay_AddLine(result->fptLT, result->fptRT, RGB(0, 255, 0));
        m_imageLotView->Overlay_AddLine(result->fptRT, result->fptRB, RGB(0, 255, 0));
        m_imageLotView->Overlay_AddLine(result->fptLB, result->fptRB, RGB(0, 255, 0));
        m_imageLotView->Overlay_AddLine(result->fptLT, result->fptLB, RGB(0, 255, 0));
        m_imageLotView->Overlay_Show(TRUE);
    }
    else
    {
        auto* result = m_inspection.getResult_2DEmpty();

        m_imageLotView->Overlay_RemoveAll();

        std::vector<Ipvm::Rect32r> blobRois;
        result->getBlobRois(blobRois);

        for (auto& roi : blobRois)
        {
            m_imageLotView->Overlay_AddRectangle(roi, m_processor->m_bEmpty ? RGB(255, 0, 0) : RGB(0, 255, 0));
        }

        m_imageLotView->Overlay_Show(TRUE);
    }
}

void PropertyGrid_Impl::event_changedRoi(long gridIndex)
{
    switch (m_propertyIndexs[gridIndex])
    {
        case 0:
            GetROI_edgeAlign();
            break;
        case 1:
            GetROI_empty2D();
            break;
    }
}

void PropertyGrid_Impl::event_changedPane(long gridIndex)
{
    m_processor->DoInspection(true);

    switch (m_propertyIndexs[gridIndex])
    {
        case 0:
            SetROI_edgeAlign();
            break;
        case 1:
            SetROI_empty2D();
            break;
    }

    ShowDisplayImage(gridIndex);

    m_imageLotView->ZoomImageFit();
}

void PropertyGrid_Impl::event_changedRaw(long gridIndex)
{
    ShowDisplayImage(gridIndex);
}

void PropertyGrid_Impl::event_changedImage(long gridIndex)
{
    ShowDisplayImage(gridIndex);
}

void PropertyGrid_Impl::UpdateROI_edgeAlign()
{
    GetROI_edgeAlign();

    long nCountLowTop = m_para->m_nLowTopCount;
    long nCountLowTopOld = (long)m_para->m_vecrtLowTopSearchROI_BCU.size();

    m_para->m_vecrtLowTopSearchROI_BCU.resize(nCountLowTop);

    if (nCountLowTopOld < nCountLowTop)
    {
        m_para->m_vecrtLowTopSearchROI_BCU.resize(nCountLowTop);

        for (long nCount = nCountLowTopOld; nCount < nCountLowTop; nCount++)
        {
            m_para->m_vecrtLowTopSearchROI_BCU[nCount].SetRect(-5000.f, -5000.f, 5000.f, 5000.f);
        }
    }

    SetROI_edgeAlign();
}

CString GetKey(long type, long index)
{
    CString key;
    key.Format(_T("%d_%d"), type, index);

    return key;
}

void PropertyGrid_Impl::GetROI_edgeAlign()
{
    if (m_imageLotView == nullptr)
        return;

    auto* result = m_inspection.getResult_EdgeAlign();
    const auto& paneCenter = result->getPaneCenter();
    const auto& scale = m_processor->getScale();

    for (long nIndex = 0; nIndex < (long)m_para->m_vecrtLowTopSearchROI_BCU.size(); nIndex++)
    {
        Ipvm::Rect32s roi;
        if (!m_imageLotView->ROI_Get(GetKey(4, nIndex), roi))
            continue;
        m_para->m_vecrtLowTopSearchROI_BCU[nIndex] = scale.convert_PixelToBCU(roi, paneCenter);
    }
}

void PropertyGrid_Impl::GetROI_empty2D()
{
    auto* result = m_inspection.getResult_2DEmpty();
    const auto& image = result->getImage(false);
    const auto& scale = m_processor->getScale();

    Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);

    Ipvm::Rect32s rtROI;

    if (m_imageLotView->ROI_Get(_T("Blob ROI"), rtROI))
    {
        m_para->m_2D_emptyBlobROI_BCU = scale.convert_PixelToBCU(rtROI, imageCenter);
    }

    for (long n = 0; n < (long)m_para->m_2D_emptyIgnoreROIs_BCU.size(); n++)
    {
        CString key;
        key.Format(_T("I%d"), n);

        if (m_imageLotView->ROI_Get(key, rtROI))
        {
            m_para->m_2D_emptyIgnoreROIs_BCU[n] = scale.convert_PixelToBCU(rtROI, imageCenter);
        }

        key.Empty();
    }

    ShowEmpty2D(true);
}

void PropertyGrid_Impl::SetROI_edgeAlign()
{
    if (m_imageLotView == nullptr)
        return;

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    auto* result = m_inspection.getResult_EdgeAlign();
    const auto& paneCenter = result->getPaneCenter();

    static const bool bIsSideVision = m_processor->IsSideVision();
    static const long nSideVisionNumber = m_processor->GetSideVisionNumber();
    static const bool bIsSideVision2 = (bIsSideVision && nSideVisionNumber == enSideVisionNumber::SIDE_VISIONNUMBER_2);
    const float fBodySizeX = bIsSideVision2 ? m_processor->m_packageSpec.m_bodyInfoMaster->GetBodySizeY()
                                            : m_processor->m_packageSpec.m_bodyInfoMaster->GetBodySizeX();
    const float fBodySizeY = bIsSideVision ? m_processor->m_packageSpec.m_bodyInfoMaster->fBodyThickness
                                           : m_processor->m_packageSpec.m_bodyInfoMaster->GetBodySizeY();

    const auto& scale = m_processor->getScale();

    float bodySizeX_px
        = scale.convert_umToPixelX(fBodySizeX); // m_processor->m_packageSpec.m_bodyInfoMaster->GetBodySizeX());
    float bodySizeY_px
        = scale.convert_umToPixelY(fBodySizeY); // m_processor->m_packageSpec.m_bodyInfoMaster->GetBodySizeY());

    Ipvm::Rect32s rtROITemp = Ipvm::Rect32s(0, 0, 100, 100);

    for (long direction = 0; direction < 4; direction++)
    {
        auto& paras = m_para->m_searchPara[direction];
        for (long nIndex = 0; nIndex < (long)paras.size(); nIndex++)
        {
            CString strName;
            strName.Format(_T("%s_%d"), GetDirectionName(direction), nIndex + 1);
            Ipvm::Rect32s rtROI = paras[nIndex].ToRect(scale, direction, paneCenter, bodySizeX_px, bodySizeY_px);
            m_imageLotView->ROI_Add(GetKey(direction, nIndex), strName, rtROI, RGB(255, 0, 0), FALSE, FALSE);

            strName.Empty();
        }
    }

    for (long nIndex = 0; nIndex < (long)m_para->m_vecrtLowTopSearchROI_BCU.size(); nIndex++)
    {
        CString strName;
        strName.Format(_T("LowTop_ROI%d"), nIndex);
        Ipvm::Rect32s rtROI
            = m_processor->getScale().convert_BCUToPixel(m_para->m_vecrtLowTopSearchROI_BCU[nIndex], paneCenter);
        m_imageLotView->ROI_Add(GetKey(4, nIndex), strName, rtROI, RGB(255, 0, 0), TRUE, TRUE);

        strName.Empty();
    }

    m_imageLotView->ROI_Show(TRUE);
    m_imageLotView->Overlay_Show(TRUE);
}

void PropertyGrid_Impl::SetROI_empty2D()
{
    if (m_imageLotView == nullptr)
        return;

    auto* result = m_inspection.getResult_2DEmpty();
    const auto& image = result->getImage(false);

    Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    Ipvm::Rect32s rtROI = m_processor->getScale().convert_BCUToPixel(m_para->m_2D_emptyBlobROI_BCU, imageCenter);
    rtROI &= Ipvm::Rect32s(image);

    if (rtROI.IsRectEmpty())
    {
        rtROI = m_processor->getScale().convert_BCUToPixel(Ipvm::Rect32r(-150.f, -150.f, 150.f, 150.f), imageCenter);
    }

    m_imageLotView->ROI_Add(_T("Blob ROI"), _T("Empty Check Blob ROI"), rtROI, RGB(255, 0, 0), TRUE, TRUE);

    CString strROIName;
    if (m_para->m_2D_emptyIgnoreROINum > (long)m_para->m_2D_emptyIgnoreROIs_BCU.size())
    {
        for (long n = (long)m_para->m_2D_emptyIgnoreROIs_BCU.size(); n < m_para->m_2D_emptyIgnoreROINum; n++)
        {
            m_para->m_2D_emptyIgnoreROIs_BCU.push_back(Ipvm::Rect32r(-1500.f, -1500.f, 1500.f, 1500.f));
        }
    }

    m_para->m_2D_emptyIgnoreROIs_BCU.resize(max(0, m_para->m_2D_emptyIgnoreROINum));

    for (long n = 0; n < m_para->m_2D_emptyIgnoreROINum; n++)
    {
        CString key;
        key.Format(_T("I%d"), n);

        rtROI = m_processor->getScale().convert_BCUToPixel(m_para->m_2D_emptyIgnoreROIs_BCU[n], imageCenter);
        rtROI &= Ipvm::Rect32s(image);

        strROIName.Format(_T("Ignore ROI %d"), n);
        m_imageLotView->ROI_Add(key, strROIName, rtROI, RGB(255, 255, 0), TRUE, TRUE);

        key.Empty();
    }

    m_imageLotView->ROI_Show(TRUE);
    m_imageLotView->Overlay_Show(TRUE);

    strROIName.Empty();
}

void PropertyGrid_Impl::ShowDisplayImage(long gridIndex)
{
    switch (m_propertyIndexs[gridIndex])
    {
        case Type_EdgeAlign:
            ShowEdgeAlignImage();
            break;
        case Type_EmptyChecker:
            ShowEmpty2D(false);
            break;
    }
}

void PropertyGrid_Impl::ShowEdgeAlignImage()
{
    auto* result = m_inspection.getResult_EdgeAlign();

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);
    m_imageLotView->SetImage(result->getImage(), _T("Edge Align Image **"));
}

void PropertyGrid_Impl::ShowEmpty2D(bool thresholdDisplay)
{
    auto& memory = m_processor->getReusableMemory();
    auto* result = m_inspection.getResult_2DEmpty();

    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    const auto& image = result->getImage(false);
    if (image.GetMem() == nullptr)
    {
        return;
    }

    Ipvm::Image8u thresholdImage;
    Ipvm::Image8u bufferImage;

    if (!memory.GetByteImage(thresholdImage, image.GetSizeX(), image.GetSizeY()))
        return;
    if (!memory.GetByteImage(bufferImage, image.GetSizeX(), image.GetSizeY()))
        return;

    Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), thresholdImage);

    Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);

    Ipvm::Rect32s rtROI = m_processor->getScale().convert_BCUToPixel(m_para->m_2D_emptyBlobROI_BCU, imageCenter);

    //Src --> Buf Copy
    Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), bufferImage);

    if (thresholdDisplay)
    {
        BYTE threshold = (BYTE)min(255, max(0, m_para->m_2D_emptyThresholdValue));
        Ipvm::ImageProcessing::BinarizeGreaterEqual(bufferImage, rtROI, threshold, thresholdImage);

        if (m_para->m_2D_emptyReverseThreshold)
        {
            Ipvm::ImageProcessing::BitwiseNot(rtROI, thresholdImage);
        }
    }

    for (long n = 0; n < (long)m_para->m_2D_emptyIgnoreROIs_BCU.size(); n++)
    {
        Ipvm::Rect32s rtCopyROI
            = m_processor->getScale().convert_BCUToPixel(m_para->m_2D_emptyIgnoreROIs_BCU[n], imageCenter);
        Ipvm::ImageProcessing::Copy(bufferImage, rtCopyROI, thresholdImage);
    }

    m_imageLotView->SetImage(thresholdImage, _T("Threshold Image **"));
}

void PropertyGrid_Impl::ClickedButtonImageCombine()
{
    const auto& inspectionAreaInfo = m_processor->getInspectionAreaInfo();

    bool bRet(false);

    if (inspectionAreaInfo.m_stichCountX == 1 && inspectionAreaInfo.m_stichCountY == 1)
    {
        Ipvm::Rect32s rtPaneROI = m_inspection.getImageSourceRoiForNoStitch();
        bRet = ::Combine_SetParameter(*m_processor, m_para->m_alignImageFrameIndex.getFrameIndex(), true,
                   &m_para->m_ImageProcMangePara, rtPaneROI)
            == IDOK; //파라미터 중 true는 원래 false 였음.. 바꾼 이유는 Align 시에는 Raw Image 기준으로 돌아야 한다.
    }
    else
    {
        if (m_processor->m_bInvalid == TRUE || m_processor->m_bEmpty == TRUE)
            ::SimpleMessage(_T("If using stitching, this function can only be used properly if alignment is ")
                            _T("successful.\nIt is acceptable even if result is not accurate."));
        bRet = ::Combine_SetParameter(
                   *m_processor, m_para->m_alignImageFrameIndex.getFrameIndex(), false, &m_para->m_ImageProcMangePara)
            == IDOK; //Stitching Package는 방법이 없다. false로 써야 한다. 스티칭을 하는 자재의 경우 스티칭된 Raw 영상을 보여줘야하는데 이 영상은 휘발성으로 중간 과정에서 계산 후 버려 진다.
    }

    if (bRet == true)
    {
        m_processor->DoInspection(true);
        ShowEdgeAlignImage();
    }
}

void PropertyGrid_Impl::active_edgeAlign(long gridIndex)
{
    const auto& scale = m_processor->getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    auto* propertyGrid = m_propertyGrid[m_propertyIndexs[gridIndex]];

    propertyGrid->ResetContent();

    if (Config::getVisionType() == VISIONTYPE_2D_INSP || Config::getVisionType() == VISIONTYPE_SIDE_INSP
        || Config::getVisionType() == VISIONTYPE_SWIR_INSP) //kircheis_SWIR
    {
        if (auto* category = propertyGrid->AddCategory(_T("Edge Align Paramter")))
        {
            category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
                ->SetID(EDGE_ALIGN_PARAM_IMAGE_COMBINE);
            m_para->m_alignImageFrameIndex.makePropertyGridItem(
                category, _T("Align Image Frame Index"), EDGE_ALIGN_PARAM_FRAME_INDEX);
            category->Expand();
        }
    }

    if (Config::getVisionType() == VISIONTYPE_3D_INSP)
    {
        if (auto* category = propertyGrid->AddCategory(_T("3D Empty / Double")))
        {
            if (auto* item
                = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Empty Check"), m_para->m_emptyInsp)))
            {
                item->SetID(EDGE_ALIGN_PARAM_INSP_EMPTY);
                (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            }

            if (auto* item
                = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Double Check"), m_para->m_3D_doubleInsp)))
            {
                item->SetID(EDGE_ALIGN_PARAM_INSP_DOUBLE);
                (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            }

            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Empty Check Ratio"), m_para->m_3D_emptyCheckMoldAreaRatioSpec * 100.f, _T("%.3lf %%")))
                ->SetID(EDGE_ALIGN_PARAM_INSP_EMPTY_CHECK_RATIO);
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Height of tray wall from mold"), m_para->m_3D_heightOfTrayWallFromMold, _T("%.3lf um")))
                ->SetID(EDGE_ALIGN_PARAM_HEIGHTOFTRAYWALLFROMMOLD);

            category->Expand();
        }
    }

    if (auto* category = propertyGrid->AddCategory(_T("Edge Detect Parameter")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Search Direction"), m_para->m_nSearchDirection, (int*)&m_para->m_nSearchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), SEARCH_DIRECTION_IN_OUT);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), SEARCH_DIRECTION_OUT_IN);

            item->SetID(EDGE_ALIGN_PARAM_SEARCH_DIRECTION);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Direction"), m_para->m_nEdgeDirection, (int*)&m_para->m_nEdgeDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("Falling"), EDGE_DIRECTION_FALLING);
            item->GetConstraints()->AddConstraint(_T("Rising"), EDGE_DIRECTION_RISING);

            item->SetID(EDGE_ALIGN_PARAM_EDGE_DIRECTION);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Detect Mode"), m_para->m_nEdgeDetectMode, (int*)&m_para->m_nEdgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), EDGE_DETECT_MODE_BEST);
            item->GetConstraints()->AddConstraint(_T("First Edge"), EDGE_DETECT_MODE_FIRST);
            item->GetConstraints()->AddConstraint(_T("Auto Detect"), EDGE_DETECT_MODE_AUTO);

            item->SetID(EDGE_ALIGN_PARAM_EDGE_DETECT_MODE);
        }
        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("FirstEdge Min Value"), m_para->m_fFirstEdgeMinThreshold, _T("%.3lf GV")))
            ->SetID(EDGE_ALIGN_PARAM_FIRST_EDGE_MIN_THRESHOLD);
        auto* edgeAlignGap = category->AddChildItem(new CXTPPropertyGridItemDouble(
            _T("Detected EdgeAlign Gap(um)"), m_para->m_fEdgeAlignGap_um, _T("%.3lf um")));
        edgeAlignGap->SetID(EDGE_ALIGN_PARAM_GAP_MICRO);

        long nGapToPixel_X = (long)((m_para->m_fEdgeAlignGap_um / fpx2um_x) + 0.5f);
        long nGapToPixel_Y = (long)((m_para->m_fEdgeAlignGap_um / fpx2um_y) + 0.5f);

        if (nGapToPixel_X < 2 || nGapToPixel_Y < 2)
        {
            nGapToPixel_X = 2;
            nGapToPixel_Y = 2;
            m_para->m_fEdgeAlignGap_um = nGapToPixel_X * fpx2um_x;
        }
        edgeAlignGap->AddChildItem(new CXTPPropertyGridItemNumber(_T("Detected EdgeAlign Gap X(px)"), nGapToPixel_X))
            ->SetID(EDGE_ALIGN_PARAM_GAP_PIXEL_X);
        edgeAlignGap->AddChildItem(new CXTPPropertyGridItemNumber(_T("Detected EdgeAlign Gap Y(px)"), nGapToPixel_Y))
            ->SetID(EDGE_ALIGN_PARAM_GAP_PIXEL_Y);

        category->Expand();
    }

    if (auto* category = propertyGrid->AddCategory(_T("Edge Detect ROI Parameter")))
    {
        for (long direction = 0; direction < 4; direction++)
        {
            UINT parentID = 0;
            CString searchName = GetDirectionName(direction);
            switch (direction)
            {
                case UP:
                    parentID = EDGE_ALIGN_PARAM_SearhPara_Top;
                    break;
                case DOWN:
                    parentID = EDGE_ALIGN_PARAM_SearhPara_Bottom;
                    break;
                case LEFT:
                    parentID = EDGE_ALIGN_PARAM_SearhPara_Left;
                    break;
                case RIGHT:
                    parentID = EDGE_ALIGN_PARAM_SearhPara_Right;
                    break;
            }

            auto& paras = m_para->m_searchPara[direction];

            auto* countPara = category->AddChildItem(new CXTPPropertyGridItemNumber(searchName, (long)paras.size()));
            countPara->SetID(parentID);

            searchName.Empty();
        }

        if (Config::getVisionType()
            == VISIONTYPE_2D_INSP) // JHB_AutoFocusImageSet : 2D Inspection 일 때만 Category 추가 - 2021.09.07
        {
            if (auto* categoryAF = propertyGrid->AddCategory(_T("Auto Focus Image Set")))
            {
                auto* result = m_inspection.getResult_EdgeAlign();

                if (!result
                        ->m_success) // Edge Align이 성공한 경우에만 Auto Focus Image Setting을 할 수 있도록 버튼을 활성화 시킴
                {
                    categoryAF->AddChildItem(new CCustomItemButton(_T("Align Failed"), TRUE, FALSE))
                        ->SetID(EDGE_ALIGN_PARAM_AUTO_FOCUS_IMAGE_SET);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_AUTO_FOCUS_IMAGE_SET)->SetReadOnly(TRUE);
                }
                else
                {
                    categoryAF
                        ->AddChildItem(new CCustomItemButton(_T("Set AF Ref. ROI Position For NGRV"), TRUE, FALSE))
                        ->SetID(EDGE_ALIGN_PARAM_AUTO_FOCUS_IMAGE_SET);
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_AUTO_FOCUS_IMAGE_SET)->SetReadOnly(FALSE);
                }

                categoryAF->Expand();
            }
        }

        category->AddChildItem(new CXTPPropertyGridItemNumber(_T("LowTop"), m_para->m_nLowTopCount))
            ->SetID(EDGE_ALIGN_PARAM_SearhPara_LowTop);

        category->Expand();
    }

    if (m_para->m_nEdgeDetectMode != EDGE_DETECT_MODE_FIRST)
    {
        propertyGrid->FindItem(EDGE_ALIGN_PARAM_FIRST_EDGE_MIN_THRESHOLD)->SetReadOnly(TRUE);
    }

    propertyGrid->ShowWindow(SW_SHOW);

    SetROI_edgeAlign();
}

void PropertyGrid_Impl::active_empty2D(long gridIndex)
{
    const auto& scale = m_processor->getScale();
    auto* propertyGrid = m_propertyGrid[m_propertyIndexs[gridIndex]];

    propertyGrid->ResetContent();

    if (auto* category = propertyGrid->AddCategory(_T("Empty")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Empty Check"), m_para->m_emptyInsp)))
        {
            item->SetID(EDGE_ALIGN_PARAM_INSP_EMPTY);
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
        }

        m_para->m_emptyFrameIndex.makePropertyGridItem(
            category, _T("Empty Image Frame Index"), EMPTYCHECK_2D_PARAM_FRAME_INDEX);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Image Threshold"), m_para->m_2D_emptyThresholdValue, &m_para->m_2D_emptyThresholdValue)))
        {
            if (auto* slider = item->AddSliderControl())
            {
                slider->SetMin(0);
                slider->SetMax(256);
            }

            if (auto* spin = item->AddSpinButton())
            {
                spin->SetMin(0);
                spin->SetMax(256);
            }
            item->SetID(EMPTYCHECK_2D_ALGO_PARAM_THRESHOLD);
        }
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Ignore ROI"), m_para->m_2D_emptyIgnoreROINum, &m_para->m_2D_emptyIgnoreROINum))
            ->SetID(EMPTYCHECK_2D_ALGO_PARAM_IGNORE_ROI);
        auto* ctl_minBlobSize_um = category->AddChildItem(
            new XTPPropertyGridItemCustomFloat<float>(_T("Min Blob Size (um^2)"), m_para->m_2D_emptyMinBlobSize_umSqure,
                _T("%.3f"), m_para->m_2D_emptyMinBlobSize_umSqure, 0.f, 1000000.f, 0.1f));
        ctl_minBlobSize_um->SetID(EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_UM);

        m_empty_minBlobSize_px = m_para->m_2D_emptyMinBlobSize_umSqure * scale.umToPixel().m_x * scale.umToPixel().m_y;
        auto* ctl_minBlobSize_px
            = ctl_minBlobSize_um->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Min Blob Size (px^2)"),
                m_empty_minBlobSize_px, _T("%.3f"), m_empty_minBlobSize_px, 0.f, 10000000000.f, 0.1f));
        ctl_minBlobSize_px->SetID(EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_PX);

        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Min Blob Count"), m_para->m_2D_emptyMinBlobCount, &m_para->m_2D_emptyMinBlobCount))
            ->SetID(EMPTYCHECK_2D_ALGO_PARAM_BLOB_COUNT);
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(
                _T("Reverse Threshold"), m_para->m_2D_emptyReverseThreshold, &m_para->m_2D_emptyReverseThreshold)))
        {
            item->SetID(EMPTYCHECK_2D_ALGO_PARAM_REVERSE_THRESHOLD);
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Use Blob Filter"),
                m_para->m_2D_emptyUseFilterBlobFullROI, &m_para->m_2D_emptyUseFilterBlobFullROI)))
        {
            item->SetID(EMPTYCHECK_2D_ALGO_PARAM_USE_BLOB_FILTER);
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
        }

        category->Expand();
    }

    if (auto* category = propertyGrid->AddCategory(_T("Empty Result")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItem(_T("Result Text"), m_resultEmpty_Text)))
        {
            item->SetID(EMPTYCHECK_2D_RESULT_TEXT);
            item->SetReadOnly(TRUE);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItem(_T("Blob Count"), m_resultEmpty_BlobCount)))
        {
            item->SetID(EMPTYCHECK_2D_RESULT_BLOB_COUNT);
            item->SetReadOnly(TRUE);
        }

        category->Expand();
    }

    propertyGrid->ShowWindow(SW_SHOW);

    SetROI_empty2D();
    ShowEmpty2D(true);
}

bool PropertyGrid_Impl::notify_edgeAlign(long gridIndex, WPARAM wparam, LPARAM lparam)
{
    auto* propertyGrid = m_propertyGrid[m_propertyIndexs[gridIndex]];
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    CString strTemp;

    const auto& scale = m_processor->getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    long nGapToPixel_X = 0;
    long nGapToPixel_Y = 0;
    float nGapToum = 0.f;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (item->GetID())
        {
            case EDGE_ALIGN_PARAM_IMAGE_COMBINE:
                ClickedButtonImageCombine();
                break;
            case EDGE_ALIGN_PARAM_FRAME_INDEX:
            {
                m_processor->DoInspection(true);
                ShowDisplayImage(gridIndex);

                break;
            }
            case EDGE_ALIGN_PARAM_INSP_EMPTY:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemBool*>(item))
                    m_para->m_emptyInsp = value->GetBool();
                break;
            case EDGE_ALIGN_PARAM_INSP_DOUBLE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemBool*>(item))
                    m_para->m_3D_doubleInsp = value->GetBool();
                break;
            case EDGE_ALIGN_PARAM_INSP_EMPTY_CHECK_RATIO:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    if (value->GetDouble() < 0.f || value->GetDouble() > 100.f)
                    {
                        strTemp.Format(_T("'%s' Range 0 ~ 100"), LPCTSTR(value->GetCaption()));
                        AfxMessageBox(strTemp, MB_OK | MB_ICONSTOP);
                        strTemp.Format(_T("%d"), 50);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_INSP_EMPTY_CHECK_RATIO)->SetValue(strTemp);
                        m_para->m_3D_emptyCheckMoldAreaRatioSpec = 0.5f;
                    }
                    else
                    {
                        m_para->m_3D_emptyCheckMoldAreaRatioSpec = float(value->GetDouble()) / 100.f;
                    }
                break;
            case EDGE_ALIGN_PARAM_HEIGHTOFTRAYWALLFROMMOLD:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_para->m_3D_heightOfTrayWallFromMold = (float)value->GetDouble();
                break;
            case EDGE_ALIGN_PARAM_SEARCH_DIRECTION:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_para->m_nSearchDirection = value->GetEnum();
                break;
            case EDGE_ALIGN_PARAM_EDGE_DIRECTION:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_para->m_nEdgeDirection = value->GetEnum();
                break;
            case EDGE_ALIGN_PARAM_EDGE_DETECT_MODE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_para->m_nEdgeDetectMode = value->GetEnum();
                if (m_para->m_nEdgeDetectMode == EDGE_DETECT_MODE_FIRST)
                {
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_FIRST_EDGE_MIN_THRESHOLD)->SetReadOnly(FALSE);
                }
                else
                {
                    propertyGrid->FindItem(EDGE_ALIGN_PARAM_FIRST_EDGE_MIN_THRESHOLD)->SetReadOnly(TRUE);
                }
                break;
            case EDGE_ALIGN_PARAM_FIRST_EDGE_MIN_THRESHOLD:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                {
                    if (value->GetDouble() < 5.f || value->GetDouble() > 250.f)
                    {
                        m_para->m_fFirstEdgeMinThreshold = 5.f;
                        strTemp.Format(_T("%.3lf GV"), m_para->m_fFirstEdgeMinThreshold);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_FIRST_EDGE_MIN_THRESHOLD)->SetValue(strTemp);
                    }
                    else
                        m_para->m_fFirstEdgeMinThreshold = (float)value->GetDouble();
                }
                break;
            case EDGE_ALIGN_PARAM_GAP_MICRO:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                {
                    nGapToPixel_X = (long)((value->GetDouble() / fpx2um_x) + 0.5f);
                    nGapToPixel_Y = (long)((value->GetDouble() / fpx2um_y) + 0.5f);

                    if (nGapToPixel_X < 2 || nGapToPixel_Y < 2)
                    {
                        nGapToPixel_X = 2;
                        nGapToPixel_Y = 2;
                        m_para->m_fEdgeAlignGap_um = nGapToPixel_X * fpx2um_x;

                        strTemp.Format(_T("%.3lf um"), m_para->m_fEdgeAlignGap_um);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_MICRO)->SetValue(strTemp);
                        strTemp.Format(_T("%d"), nGapToPixel_X);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_X)->SetValue(strTemp);
                        strTemp.Format(_T("%d"), nGapToPixel_Y);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_Y)->SetValue(strTemp);
                    }
                    else
                    {
                        m_para->m_fEdgeAlignGap_um = (float)value->GetDouble();

                        strTemp.Format(_T("%d"), nGapToPixel_X);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_X)->SetValue(strTemp);
                        strTemp.Format(_T("%d"), nGapToPixel_Y);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_Y)->SetValue(strTemp);
                    }
                }
                break;
            case EDGE_ALIGN_PARAM_GAP_PIXEL_X:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    if (value->GetNumber() < 2.f)
                    {
                        nGapToPixel_X = 2;
                        nGapToPixel_Y = 2;
                        nGapToum = nGapToPixel_X * fpx2um_x;

                        m_para->m_fEdgeAlignGap_um = nGapToum;

                        strTemp.Format(_T("%.3lf um"), m_para->m_fEdgeAlignGap_um);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_MICRO)->SetValue(strTemp);
                        strTemp.Format(_T("%d"), nGapToPixel_X);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_X)->SetValue(strTemp);
                        strTemp.Format(_T("%d"), nGapToPixel_Y);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_Y)->SetValue(strTemp);
                    }
                    else
                    {
                        nGapToum = value->GetNumber() * fpx2um_x;
                        nGapToPixel_Y = (long)((nGapToum / fpx2um_y) + 0.5f);

                        m_para->m_fEdgeAlignGap_um = nGapToum;

                        strTemp.Format(_T("%.3lf um"), m_para->m_fEdgeAlignGap_um);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_MICRO)->SetValue(strTemp);
                        strTemp.Format(_T("%d"), nGapToPixel_Y);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_Y)->SetValue(strTemp);
                    }
                }
                break;
            case EDGE_ALIGN_PARAM_GAP_PIXEL_Y:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    if (value->GetNumber() < 2.f)
                    {
                        nGapToPixel_X = 2;
                        nGapToPixel_Y = 2;
                        nGapToum = nGapToPixel_Y * fpx2um_y;

                        m_para->m_fEdgeAlignGap_um = nGapToum;

                        strTemp.Format(_T("%.3lf um"), m_para->m_fEdgeAlignGap_um);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_MICRO)->SetValue(strTemp);
                        strTemp.Format(_T("%d"), nGapToPixel_X);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_X)->SetValue(strTemp);
                        strTemp.Format(_T("%d"), nGapToPixel_Y);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_Y)->SetValue(strTemp);
                    }
                    else
                    {
                        nGapToum = value->GetNumber() * fpx2um_y;
                        nGapToPixel_X = (long)((nGapToum / fpx2um_x) + 0.5f);

                        m_para->m_fEdgeAlignGap_um = nGapToum;

                        strTemp.Format(_T("%.3lf um"), m_para->m_fEdgeAlignGap_um);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_MICRO)->SetValue(strTemp);
                        strTemp.Format(_T("%d"), nGapToPixel_X);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_X)->SetValue(strTemp);
                    }
                }
                break;
            case EDGE_ALIGN_PARAM_SearhPara_Left:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    if (value->GetNumber() <= 0
                        || value->GetNumber()
                            > SEARCH_ROI_MAX_NUM) // 영훈 [ EdgeBodyAlignROI ] 20130815 : (예외처리) SearchROI 개수는 5개 이상으로 하지 않는다.
                    {
                        m_para->m_searchPara[LEFT].resize(1);
                        strTemp.Format(_T("%d"), m_para->m_searchPara[LEFT].size());
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_SearhPara_Left)->SetValue(strTemp);
                    }
                    else
                    {
                        m_para->m_searchPara[LEFT].resize(value->GetNumber());
                    }

                    m_edgeSearchRoiPara->UpdateList();
                }
                break;
            case EDGE_ALIGN_PARAM_SearhPara_Top:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    if (value->GetNumber() < 0 || value->GetNumber() > SEARCH_ROI_MAX_NUM)
                    {
                        m_para->m_searchPara[UP].resize(1);
                        strTemp.Format(_T("%d"), m_para->m_searchPara[UP].size());
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_SearhPara_Top)->SetValue(strTemp);
                    }
                    else
                    {
                        m_para->m_searchPara[UP].resize(value->GetNumber());
                    }

                    m_edgeSearchRoiPara->UpdateList();
                }
                break;
            case EDGE_ALIGN_PARAM_SearhPara_Right:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    if (value->GetNumber() < 0
                        || value->GetNumber()
                            > SEARCH_ROI_MAX_NUM) // 영훈 [ EdgeBodyAlignROI ] 20130815 : (예외처리) SearchROI 개수는 5개 이상으로 하지 않는다.
                    {
                        m_para->m_searchPara[RIGHT].resize(1);
                        strTemp.Format(_T("%d"), m_para->m_searchPara[RIGHT].size());
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_SearhPara_Right)->SetValue(strTemp);
                    }
                    else
                    {
                        m_para->m_searchPara[RIGHT].resize(value->GetNumber());
                    }

                    m_edgeSearchRoiPara->UpdateList();
                }
                break;
            case EDGE_ALIGN_PARAM_SearhPara_Bottom:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    if (value->GetNumber() < 0
                        || value->GetNumber()
                            > SEARCH_ROI_MAX_NUM) // 영훈 [ EdgeBodyAlignROI ] 20130815 : (예외처리) SearchROI 개수는 5개 이상으로 하지 않는다.
                    {
                        m_para->m_searchPara[DOWN].resize(1);
                        strTemp.Format(_T("%d"), m_para->m_searchPara[DOWN].size());
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_SearhPara_Bottom)->SetValue(strTemp);
                    }
                    else
                    {
                        m_para->m_searchPara[DOWN].resize(value->GetNumber());
                    }

                    m_edgeSearchRoiPara->UpdateList();
                }
                break;
            case EDGE_ALIGN_PARAM_SearhPara_LowTop:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                    if (value->GetNumber() < 0
                        || value->GetNumber()
                            > SEARCH_ROI_MAX_NUM) // 영훈 [ EdgeBodyAlignROI ] 20130815 : (예외처리) SearchROI 개수는 5개 이상으로 하지 않는다.
                    {
                        m_para->m_nLowTopCount = 0;
                        strTemp.Format(_T("%d"), m_para->m_nLowTopCount);
                        propertyGrid->FindItem(EDGE_ALIGN_PARAM_SearhPara_LowTop)->SetValue(strTemp);
                    }
                    else
                        m_para->m_nLowTopCount = value->GetNumber();
                break;
            case EDGE_ALIGN_PARAM_AUTO_FOCUS_IMAGE_SET:
                ClickedButtonSetAutoFocusImage();
                break;
        }

        UpdateROI_edgeAlign();
    }

    strTemp.Empty();

    return false;
}

bool PropertyGrid_Impl::notify_empty2D(long gridIndex, WPARAM wparam, LPARAM lparam)
{
    //auto* propertyGrid = m_propertyGrid[m_propertyIndexs[gridIndex]];
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    const auto& scale = m_processor->getScale();

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (item->GetID())
        {
            case EDGE_ALIGN_PARAM_INSP_EMPTY:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemBool*>(item))
                    m_para->m_emptyInsp = value->GetBool();
                break;

            case EMPTYCHECK_2D_PARAM_FRAME_INDEX:
            {
                m_processor->DoInspection(true);
                ShowDisplayImage(gridIndex);
                break;
            }

            case EMPTYCHECK_2D_ALGO_PARAM_THRESHOLD:
                ShowEmpty2D(true);
                break;

            case EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_UM:
                m_empty_minBlobSize_px
                    = m_para->m_2D_emptyMinBlobSize_umSqure * scale.umToPixel().m_x * scale.umToPixel().m_y;
                ((XTPPropertyGridItemCustomFloat<float>*)m_propertyGrid[1]->FindItem(
                     EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_PX))
                    ->SetValue(m_empty_minBlobSize_px);
                break;
            case EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_PX:
                m_para->m_2D_emptyMinBlobSize_umSqure
                    = m_empty_minBlobSize_px * scale.pixelToUm().m_x * scale.pixelToUm().m_y;
                ((XTPPropertyGridItemCustomFloat<float>*)m_propertyGrid[1]->FindItem(
                     EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_UM))
                    ->SetValue(m_para->m_2D_emptyMinBlobSize_umSqure);
                break;
            default:
                break;
        }

        if (item->GetID() == EMPTYCHECK_2D_ALGO_PARAM_IGNORE_ROI
            || item->GetID() == EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_UM
            || item->GetID() == EMPTYCHECK_2D_ALGO_PARAM_BLOB_SIZE_PX
            || item->GetID() == EMPTYCHECK_2D_ALGO_PARAM_BLOB_COUNT
            || item->GetID() == EMPTYCHECK_2D_ALGO_PARAM_REVERSE_THRESHOLD)
        {
            GetROI_empty2D();
            SetROI_empty2D();

            ShowEmpty2D(true);
        }
    }
    else if (wparam == XTP_PGN_SELECTION_CHANGED && item->GetID() == EMPTYCHECK_2D_ALGO_PARAM_THRESHOLD)
    {
        if (auto* control = item->GetInplaceControls()->GetAt(0))
        {
            auto* slider = dynamic_cast<CXTPPropertyGridInplaceSlider*>(control);

            if (slider)
            {
                slider->GetSliderCtrl()->SetPageSize(8);
            }
        }

        GetROI_empty2D();
        SetROI_empty2D();

        ShowEmpty2D(true);
    }

    return false;
}

void PropertyGrid_Impl::ClickedButtonSetAutoFocusImage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //CDlgAutoFocusImageSet dlg(*m_para, this, *m_processor, m_para->m_nNGRVAF_FrameID, false);
    CDlgAutoFocusImageSet dlg(*m_para, this, *m_processor, m_para->m_nNGRVAF_FrameID, nullptr);

    dlg.DoModal();
}
