//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Property_Stitch.h"

//CPP_2_________________________________ This project's headers
#include "DlgBodySizeROIPara.h"
#include "DlgVisionProcessingAlign2D.h" // Stitch Section Image Combine
#include "StitchPara.h"
#include "VisionProcessingAlign2D.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h" // Stitch Section Image Combine
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h" // Stitch Section Image Combine
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h" // Stitch Section Image Combine
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Size32s2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 200

//CPP_7_________________________________ Implementation body
//
enum GridChildItemID
{
    ITEM_ID_RUN_ONLY_STITCH = 1,

    ITEM_ID_MATCHING_SELECT_FRAME_ID,
    ITEM_ID_STITCH_IMAGE_COMBINE, // Stitch Section Image Combine
    ITEM_ID_STITCH_BASED_ON_TYPE,
    ITEM_ID_MOVE_DISTANCE_X_BETWEEN_FOVS,
    ITEM_ID_MOVE_DISTANCE_Y_BETWEEN_FOVS,
    ITEM_ID_TEMPLATE_BOUNDARY_INTERPOLATION,
    ITEM_ID_SAVE_TEMPLATE_DEBUG_IMAGE,

    ITEM_ID_MATCHING_TEMPLATE_SEARCH_OFFSET,
    ITEM_ID_MATCHING_TEMPLATE_ROI_COUNT,
    ITEM_ID_MATCHING_OPTION_DEFAULT_TEMPLATE_ROI,

    ITEM_ID_BODYSIZE_SEARCH_DIRECTION,
    ITEM_ID_BODYSIZE_EDGE_DIRECTION,
    ITEM_ID_BODYSIZE_EDGE_DETECT_MODE,
    ITEM_ID_BODYSIZE_EDGE_NUM,
    ITEM_ID_BODYSIZE_FIRST_EDGE_MIN_THRESHOLD,

    ITEM_ID_DEBUG_SAVE_IMAGE,
    ITEM_ID_DEBUG_OFFSET1_X,
    ITEM_ID_DEBUG_OFFSET1_Y,
    ITEM_ID_DEBUG_OFFSET2_X,
    ITEM_ID_DEBUG_OFFSET2_Y,
    ITEM_ID_DEBUG_OFFSET3_X,
    ITEM_ID_DEBUG_OFFSET3_Y,
    ITEM_ID_DEBUG_OFFSET4_X,
    ITEM_ID_DEBUG_OFFSET4_Y,
    ITEM_ID_DEBUG_ANGLE1,
    ITEM_ID_DEBUG_ANGLE2,
    ITEM_ID_DEBUG_ANGLE3,
    ITEM_ID_DEBUG_ANGLE4,
};

Property_Stitch::Property_Stitch(VisionProcessingAlign2D& proccsor, ImageLotView* imageLotView,
    ProcCommonDebugInfoDlg& procDebugInfo, StitchPara& para)
    : m_processor(proccsor)
    , m_imageLotView(imageLotView)
    , m_procDebugInfo(procDebugInfo)
    , m_para(para)
    , m_grid(new CXTPPropertyGrid)
    , m_subDlgBodyROIs(nullptr)
{
}

// Side Vision용 Stitch
Property_Stitch::Property_Stitch(VisionProcessingAlign2D& proccsor, ImageLotView* imageLotView,
    ImageLotView* imageLotViewRearSide, ProcCommonDebugInfoDlg& procDebugInfo, StitchPara& para)
    : m_processor(proccsor)
    , m_imageLotView(imageLotView)
    , m_imageLotViewRearSide(imageLotViewRearSide)
    , m_procDebugInfo(procDebugInfo)
    , m_para(para)
    , m_grid(new CXTPPropertyGrid)
    , m_subDlgBodyROIs(nullptr)
{
}

Property_Stitch::~Property_Stitch()
{
    delete m_subDlgBodyROIs;
    delete m_grid;
}

void Property_Stitch::redrawOverlayAndROI()
{
    UpdateROI();
}

void Property_Stitch::ClickedButtonStitchImageCombine() // Stitch Section Image Combine
{
    const auto nSideSection = m_processor.GetCurVisionModule_Status();
    long nFrameIndex = m_para.m_calcFrameIndex.getFrameIndex();

    if ((::Combine_SetParameter(m_processor, nFrameIndex, true, &m_para.m_stitchImageProcManagePara) == IDOK))
    {
        // 1. 현재 Stitching에 사용되는 Frame의 영상을 얻어온다
        m_para.m_calcFrameIndex.setFrameIndex(nFrameIndex);
        const auto& image = m_processor.getImageLot().GetImageFrame(nFrameIndex, nSideSection);
        if (image.GetMem() == nullptr)
        {
            return;
        }

        // Set Raw Check on SEQ
        m_processor.m_pVisionInspDlg->m_imageLotView->SetRawImageForStitchImageCombine();
        ///////////////////////

        Ipvm::Image8u combineImage; // Combine된 영상 Buffer

        // 2. Combine 영상 Buffer의 크기를 Stitch되기 전 원본 Full 영상 Copy
        if (!m_processor.getReusableMemory().GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
            return;

        // 3. Combine 하려는 이미지에 원본 영상을 복사
        Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), combineImage);
        Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);
        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());

        // 4. Image Combine Processing
        if (CippModules::GrayImageProcessingManage(m_processor.getReusableMemory(), &image, true, rtProcessingROI,
                m_para.m_stitchImageProcManagePara, combineImage)
            == false)
        {
            return;
        }
        // 5. MainUI에 이미지 표시 - 2024.05.29_JHB
        if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
        {
            m_imageLotView->SetImage(combineImage, _T("Stitch Combined Image **"));
        }
        else
        {
            if (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
            {
                m_imageLotViewRearSide->SetImage(combineImage, _T("Stitch Combined Side Rear Image **"));
            }
            else
            {
                m_imageLotView->SetImage(combineImage, _T("Stitch Combined Side Front Image **"));
            }
        }
    }
}

bool Property_Stitch::create(long /*gridIndex */, const RECT& rect, HWND parentWnd)
{
    m_region = rect;

    CWnd* pParentWnd = CWnd::FromHandle(parentWnd);
    if (!m_grid->Create(rect, pParentWnd, IDC_FRAME_PROPERTY_GRID))
    {
        return false;
    }

    m_grid->ShowWindow(SW_HIDE);
    if (m_subDlgBodyROIs == nullptr)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CWnd* pWnd = CWnd::FromHandle(parentWnd);

        m_subDlgBodyROIs = new DlgBodySizeROIPara(m_para, this, pWnd);
        m_subDlgBodyROIs->Create(IDD_BODYSIZE_ROI_PARA, pWnd);
    }

    return true;
}

LPCTSTR Property_Stitch::getName(long /*gridIndex */) const
{
    return _T("Stitch");
}

void Property_Stitch::active(long /*gridIndex */)
{
    m_procDebugInfo.Display(0);

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_imageLotViewRearSide->SetMode(ImageLotView::Mode::LotRawAndInspImage);
    }
    else
    {
        m_imageLotView->SetMode(ImageLotView::Mode::LotRawAndInspImage);
    }

    m_grid->ResetContent();

    if (auto* category = m_grid->AddCategory(_T("Common Parameter")))
    {
        m_para.m_calcFrameIndex.makePropertyGridItem(category, _T("Select Frame ID"), ITEM_ID_MATCHING_SELECT_FRAME_ID);
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(ITEM_ID_STITCH_IMAGE_COMBINE); // Stitch Section Image Combine

        CString moveDistanceX;
        CString moveDistanceY;
        moveDistanceX.Format(_T("%.3f"), m_processor.GetMoveDistanceXbetweenFOVs_mm());
        moveDistanceY.Format(_T("%.3f"), m_processor.GetMoveDistanceYbetweenFOVs_mm());

        // SDY txtRecipe 를 위한 변수 저장
        m_para.m_fMoveDistanceXbetweenFOV = m_processor.GetMoveDistanceXbetweenFOVs_mm();
        m_para.m_fMoveDistanceYbetweenFOV = m_processor.GetMoveDistanceYbetweenFOVs_mm();

        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Algorithm"), (int)m_para.m_type, &(int&)m_para.m_type)))
        {
            item->GetConstraints()->AddConstraint(_T("Stitch based on Matching"), 0);
            item->GetConstraints()->AddConstraint(_T("Stitch based on Body Size"), 1);
            item->SetID(ITEM_ID_STITCH_BASED_ON_TYPE);
        }

        auto* itemMoveDistanceX
            = category->AddChildItem(new CXTPPropertyGridItem(_T("Move distance X between FOVs (mm)"), moveDistanceX));
        auto* itemMoveDistanceY
            = category->AddChildItem(new CXTPPropertyGridItem(_T("Move distance Y between FOVs (mm)"), moveDistanceY));

        itemMoveDistanceX->SetID(ITEM_ID_MOVE_DISTANCE_X_BETWEEN_FOVS);
        itemMoveDistanceY->SetID(ITEM_ID_MOVE_DISTANCE_Y_BETWEEN_FOVS);
        itemMoveDistanceX->SetReadOnly(TRUE);
        itemMoveDistanceY->SetReadOnly(TRUE);

        auto* itemInterpolation = category->AddChildItem(
            new CXTPPropertyGridItemEnum(_T("Boundary Interpolation"), m_para.m_boundaryInterpolation));
        itemInterpolation->GetConstraints()->AddConstraint(_T("Off"), 0);
        itemInterpolation->GetConstraints()->AddConstraint(_T("On"), 1);
        itemInterpolation->SetID(ITEM_ID_TEMPLATE_BOUNDARY_INTERPOLATION);

        auto* itemSaveDebugImage = category->AddChildItem(
            new CXTPPropertyGridItemEnum(_T("Save Template Debug Image"), m_para.m_saveTemplateDebugImage));
        itemSaveDebugImage->GetConstraints()->AddConstraint(_T("Off"), 0);
        itemSaveDebugImage->GetConstraints()->AddConstraint(_T("On"), 1);
        itemSaveDebugImage->SetID(ITEM_ID_SAVE_TEMPLATE_DEBUG_IMAGE);

        category->Expand();
    }

    if (auto* category = m_grid->AddCategory(_T("Run")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Run only stitch"), TRUE, FALSE))
            ->SetID(ITEM_ID_RUN_ONLY_STITCH);

        category->Expand();
    }

    SetGrid_BaseOn();

#ifdef DEBUG
    if (auto* category = m_grid->AddCategory(_T("Debug")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Save Image"), TRUE, FALSE))->SetID(ITEM_ID_DEBUG_SAVE_IMAGE);
        for (long n = 0; n < 4; n++)
        {
            CString part;
            part.Format(_T("S%d "), n + 1);
            category
                ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(part + _T("Offset X (px)"),
                    m_debug.m_offsetX[n], _T("%.0f"), m_debug.m_offsetX[n], -5000.f, 5000.f, 1.f))
                ->SetID(ITEM_ID_DEBUG_OFFSET1_X);
            category
                ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(part + _T("Offset Y (px)"),
                    m_debug.m_offsetY[n], _T("%.0f"), m_debug.m_offsetY[n], -5000.f, 5000.f, 1.f))
                ->SetID(ITEM_ID_DEBUG_OFFSET1_Y);
            category
                ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
                    part + _T("Angle (Deg)"), m_debug.m_angle[n], _T("%.0f"), m_debug.m_angle[n], 0.f, 360.f, 1.f))
                ->SetID(ITEM_ID_DEBUG_ANGLE1);
        }
    }
#endif

    m_grid->SetViewDivider(0.60);
    m_grid->HighlightChangedItems(TRUE);
    m_grid->ShowWindow(SW_SHOW);

    ShowImage();
    SetROI();

    ShowOverlayROI();
}

void Property_Stitch::deactivate(long /*gridIndex */)
{
    m_grid->ShowWindow(SW_HIDE);
    m_subDlgBodyROIs->ShowWindow(SW_HIDE);
}

bool Property_Stitch::notify(long gridIndex, WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return false;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    auto moveDistanceXbetweenFOVs_mm = m_processor.GetMoveDistanceXbetweenFOVs_mm();
    auto moveDistanceYbetweenFOVs_mm = m_processor.GetMoveDistanceYbetweenFOVs_mm();

    // SDY txtRecipe 를 위한 변수 저장
    m_para.m_fMoveDistanceXbetweenFOV = moveDistanceXbetweenFOVs_mm;
    m_para.m_fMoveDistanceYbetweenFOV = moveDistanceYbetweenFOVs_mm;

    bool checked = false;

    if (auto* ValueButton = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_RUN_ONLY_STITCH:
                m_para.m_skipEdgeAlign = true;
                m_processor.OnInspection();

                m_para.m_skipEdgeAlign = false;
                checked = true;
                break;

            case ITEM_ID_STITCH_IMAGE_COMBINE: // Stitch Section Image Combine
                ClickedButtonStitchImageCombine();
                checked = true;
                break;

            case ITEM_ID_DEBUG_SAVE_IMAGE:
                if (true)
                {
                    auto& sourceImage1 = m_processor.getImageLot();

                    m_processor.OnInspection();

                    CFileDialog Dlg(FALSE, _T("bmp"), NULL, OFN_HIDEREADONLY,
                        _T("Windows bitmap image file format (*.bmp)|*.bmp||"));
                    if (Dlg.DoModal() != IDOK)
                        break;

                    auto& sourceImage2 = m_processor.getImageLotInsp();

                    long imageCenterX = sourceImage2.GetImageSizeX() / 2;
                    long imageCenterY = sourceImage2.GetImageSizeY() / 2;
                    long stitchCountX = m_processor.getInspectionAreaInfo().m_stichCountX;
                    long stitchCountY = m_processor.getInspectionAreaInfo().m_stichCountY;

                    VisionImageLot imageLot;
                    imageLot.Setup2D(sourceImage1.GetSensorSizeX(), sourceImage1.GetSensorSizeY(),
                        Ipvm::Size32s2(sourceImage1.GetImageSizeX(), sourceImage1.GetImageSizeY()),
                        sourceImage1.GetImageFrameCount(), m_processor.GetCurVisionModule_Status());

                    float movePxX = m_processor.getScale().convert_mmToPixelX(moveDistanceXbetweenFOVs_mm);
                    float movePxY = m_processor.getScale().convert_mmToPixelY(moveDistanceYbetweenFOVs_mm);

                    // SDY txtRecipe 를 위한 변수 저장
                    m_para.m_fMoveDistanceXbetweenFOV = movePxX;
                    m_para.m_fMoveDistanceYbetweenFOV = movePxY;

                    for (long frameIndex = 0; frameIndex < sourceImage1.GetImageFrameCount(); frameIndex++)
                    {
                        auto& sourceImage = *sourceImage2.GetSafeImagePtr(frameIndex);
                        auto& targetImage = imageLot.GetImageFrame(frameIndex, m_processor.GetCurVisionModule_Status());

                        long stepY = CAST_LONG(movePxY);
                        long offsetY = CAST_LONG(-movePxY / 2);

                        for (int32_t y = 0; y < stitchCountY; y++)
                        {
                            long stepX = CAST_LONG(movePxX);
                            long offsetX = CAST_LONG(-movePxX / 2);

                            for (int32_t x = 0; x < stitchCountX; x++)
                            {
                                long index = y * stitchCountX + x;
                                int32_t sourceL = imageCenterX - sourceImage1.GetSensorSizeX() / 2;
                                int32_t sourceT = imageCenterY - sourceImage1.GetSensorSizeY() / 2;
                                Ipvm::Rect32s rtSource = Ipvm::Rect32s((int32_t)sourceL, (int32_t)sourceT,
                                    (int32_t)sourceL + sourceImage1.GetSensorSizeX(),
                                    (int32_t)sourceT + sourceImage1.GetSensorSizeY());

                                rtSource.OffsetRect(CAST_INT32T(offsetX + m_debug.m_offsetX[index]),
                                    CAST_INT32T(offsetY + m_debug.m_offsetY[index]));
                                offsetX += stepX;

                                Ipvm::Rect32s rtTarget = Ipvm::Rect32s((int32_t)sourceImage1.GetSensorSizeX() * x,
                                    (int32_t)sourceImage1.GetSensorSizeY() * y,
                                    (int32_t)sourceImage1.GetSensorSizeX() * (x + 1),
                                    (int32_t)sourceImage1.GetSensorSizeY() * (y + 1));

                                Ipvm::Point32s2 ptTarget;
                                ptTarget.m_x = x * sourceImage1.GetSensorSizeX();
                                ptTarget.m_y = y * sourceImage1.GetSensorSizeY();

                                Ipvm::ImageProcessing::RotateLinearInterpolation(sourceImage,
                                    Ipvm::Point32r2((rtSource.m_left + rtSource.m_right) * 0.5f,
                                        (rtSource.m_top + rtSource.m_bottom) * 0.5f),
                                    m_debug.m_angle[index],
                                    Ipvm::Point32r2(CAST_FLOAT(rtTarget.m_left - rtSource.m_left),
                                        CAST_FLOAT(rtTarget.m_top - rtSource.m_top)),
                                    rtTarget, targetImage);
                            }

                            offsetY += stepY;
                        }
                    }

                    imageLot.Save(Dlg.GetPathName());
                }
                break;
        }
    }

    if (auto* nIndexValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = nIndexValue->GetEnum();
        switch (item->GetID())
        {
            case ITEM_ID_MATCHING_SELECT_FRAME_ID:
                ShowImage(); // 영상을 보여준다.
                break;

            case ITEM_ID_STITCH_BASED_ON_TYPE:
                SetGrid_BaseOn();
                checked = true;
                break;
            case ITEM_ID_TEMPLATE_BOUNDARY_INTERPOLATION:
                m_para.m_boundaryInterpolation = data ? TRUE : FALSE;
                checked = true;
                break;

            case ITEM_ID_SAVE_TEMPLATE_DEBUG_IMAGE:
                m_para.m_saveTemplateDebugImage = data ? TRUE : FALSE;
                checked = true;
                break;
        }
    }

    if (!checked)
    {
        switch (m_para.m_type)
        {
            case enumAlgorithmType::Matching:
                return Nodifiy_BaseOnMatching(gridIndex, wparam, lparam);
            case enumAlgorithmType::BodySize:
                return Nodifiy_BaseOnBodySize(gridIndex, wparam, lparam);
        }
    }

    SetROI();
    ShowOverlayROI();

    return false;
}

void Property_Stitch::event_afterInspection(long /*gridIndex*/)
{
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_imageLotViewRearSide->ShowImage(m_para.m_calcFrameIndex.getFrameIndex());
        m_imageLotViewRearSide->ZoomPaneFit(true);
    }
    else
    {
        m_imageLotView->ShowImage(m_para.m_calcFrameIndex.getFrameIndex());
        m_imageLotView->ZoomPaneFit(true);
    }

    SelChangedRaw();
}

void Property_Stitch::event_changedRoi(long /*gridIndex */)
{
    GetROI();
    // ROI 가 바뀔때마다, Pattern Matching을 수행해서 점수를 Overlay에 표시한다.
    m_processor.Do2DStitching_GetStitchInfo(true);

    ShowOverlayROI();
}

void Property_Stitch::event_changedPane(long /*gridIndex */)
{
    SetROI();
}

void Property_Stitch::event_changedRaw(long /*gridIndex */)
{
    SelChangedRaw();
}

void Property_Stitch::SetGrid_BaseOn()
{
    auto* category = m_grid->GetCategories()->FindItem(_T("Algorithm Parameter"));

    if (category == nullptr)
    {
        category = m_grid->AddCategory(_T("Algorithm Parameter"));
    }

    category->GetChilds()->Clear();

    switch (m_para.m_type)
    {
        case enumAlgorithmType::Matching:
            m_grid->MoveWindow(m_region);
            m_subDlgBodyROIs->ShowWindow(SW_HIDE);
            SetGrid_BaseOnMatching(category);
            break;
        case enumAlgorithmType::BodySize:
            m_grid->MoveWindow(m_region.left, m_region.top, m_region.Width(), m_region.Height() / 2);
            m_subDlgBodyROIs->ShowWindow(SW_SHOW);
            m_subDlgBodyROIs->MoveWindow(
                m_region.left, m_region.top + m_region.Height() / 2, m_region.Width(), m_region.Height() / 2);
            SetGrid_BaseOnBodySize(category);
            m_subDlgBodyROIs->UpdateList();
            break;
    }

    category->Expand();
}

void Property_Stitch::SetGrid_BaseOnMatching(CXTPPropertyGridItem* category)
{
    auto& para = m_para.m_paraBasedOnMatching;
    para.m_nTemplateROICount = min(2, max(1, para.m_nTemplateROICount));

    category
        ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Template Search Offset (um)"),
            para.m_templateSearchOffset_um, _T("%.0f"), para.m_templateSearchOffset_um, 1.f, 5000.f, 1.f))
        ->SetID(ITEM_ID_MATCHING_TEMPLATE_SEARCH_OFFSET);

    auto* itemTemplateCount
        = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Template ROI Count"), para.m_nTemplateROICount));
    itemTemplateCount->GetConstraints()->AddConstraint(_T("1"), 1);
    itemTemplateCount->GetConstraints()->AddConstraint(_T("2"), 2);
    itemTemplateCount->SetID(ITEM_ID_MATCHING_TEMPLATE_ROI_COUNT);

    category->AddChildItem(new CCustomItemButton(_T("Default Template ROI"), TRUE, FALSE))
        ->SetID(ITEM_ID_MATCHING_OPTION_DEFAULT_TEMPLATE_ROI);
}

void Property_Stitch::SetGrid_BaseOnBodySize(CXTPPropertyGridItem* category)
{
    auto& para = m_para.m_paraBasedOnBodySize;

    if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
            _T("Search Direction"), para.m_searchDirection, (int*)&para.m_searchDirection)))
    {
        item->GetConstraints()->AddConstraint(_T("In -> Out"), PI_ED_DIR_OUTER);
        item->GetConstraints()->AddConstraint(_T("Out -> In"), PI_ED_DIR_INNER);

        item->SetID(ITEM_ID_BODYSIZE_SEARCH_DIRECTION);
    }
    if (auto* item = category->AddChildItem(
            new CXTPPropertyGridItemEnum(_T("Edge Direction"), para.m_edgeDirection, (int*)&para.m_edgeDirection)))
    {
        item->GetConstraints()->AddConstraint(_T("Falling"), PI_ED_DIR_FALLING);
        item->GetConstraints()->AddConstraint(_T("Rising"), PI_ED_DIR_RISING);

        item->SetID(ITEM_ID_BODYSIZE_EDGE_DIRECTION);
    }
    if (auto* item = category->AddChildItem(
            new CXTPPropertyGridItemEnum(_T("Edge Detect Mode"), para.m_edgeDetectMode, (int*)&para.m_edgeDetectMode)))
    {
        item->GetConstraints()->AddConstraint(_T("Best Edge"), EdgeDetectMode_BestEdge);
        item->GetConstraints()->AddConstraint(_T("First Edge"), EdgeDetectMode_FirstEdge);

        item->SetID(ITEM_ID_BODYSIZE_EDGE_DETECT_MODE);
    }
    if (auto* item = category->AddChildItem(
            new CXTPPropertyGridItemNumber(_T("Edge Detect Num"), para.m_edgeNum, (long*)&para.m_edgeNum)))
    {
        item->SetID(ITEM_ID_BODYSIZE_EDGE_NUM);
    }

    category
        ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("FirstEdge Min Value"),
            para.m_firstEdgeMinThreshold, _T("%.3lf GV"), para.m_firstEdgeMinThreshold, 0.f, 100.f, 0.001f))
        ->SetID(ITEM_ID_BODYSIZE_FIRST_EDGE_MIN_THRESHOLD);
}

bool Property_Stitch::Nodifiy_BaseOnMatching(long /*gridIndex*/, WPARAM /*wparam*/, LPARAM lparam)
{
    auto& para = m_para.m_paraBasedOnMatching;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* ValueButton = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_MATCHING_OPTION_DEFAULT_TEMPLATE_ROI:
                buttonClick_DefaultTemplateRoi();
                break;
        }

        ValueButton->SetBool(FALSE);
    }
    else if (auto* nIndexValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = nIndexValue->GetEnum();
        switch (item->GetID())
        {
            case ITEM_ID_MATCHING_TEMPLATE_ROI_COUNT:
                para.m_nTemplateROICount = min(2, max(1, data));
                buttonClick_DefaultTemplateRoi();
                break;
        }
    }

    return false;
}

bool Property_Stitch::Nodifiy_BaseOnBodySize(long /*gridIndex*/, WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //auto& para = m_para.m_paraBasedOnBodySize;

    return true;
}

BOOL Property_Stitch::SplitCopyImage(
    std::vector<Ipvm::Image8u>& vecSrcImage, std::vector<Ipvm::Image8u>& vecDstImage, Ipvm::Rect32s rtDestROI)
{
    long nFrameNum = (long)vecSrcImage.size();
    for (long nFrameID = 0; nFrameID < nFrameNum; nFrameID++)
    {
        auto& SingleImage = vecSrcImage[nFrameID];
        auto& MainImage = vecDstImage[nFrameID];

        Ipvm::ImageProcessing::Copy(SingleImage, Ipvm::Rect32s(SingleImage), rtDestROI, MainImage);
    }

    return TRUE;
}

void Property_Stitch::ShowImage()
{
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_imageLotViewRearSide->ShowImage(m_para.m_calcFrameIndex.getFrameIndex(), true);
        m_imageLotViewRearSide->ZoomPaneFit(true);
    }
    else
    {
        m_imageLotView->ShowImage(m_para.m_calcFrameIndex.getFrameIndex(), true);
        m_imageLotView->ZoomPaneFit(true);
    }

    ShowOverlayROI();
}

bool Property_Stitch::SetROI()
{
    bool bRearSide = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR);

    if (bRearSide == TRUE)
    {
        m_imageLotViewRearSide->ROI_RemoveAll();
    }
    else
    {
        m_imageLotView->ROI_RemoveAll();
    }

    long stitchCountX = m_processor.getInspectionAreaInfo().m_stichCountX;
    long stitchCountY = m_processor.getInspectionAreaInfo().m_stichCountY;

    if (stitchCountX == 1 && stitchCountY == 1)
    {
        return true;
    }

    bool returnValue = true;

    if (!m_processor.GetConstants())
    {
        returnValue = false;
    }
    else
    {
        if (!m_processor.SetROIsInRaw())
        {
            returnValue = false;
        }
        else
        {
            m_processor.FindAllStitchInfo_InRaw(true);
        }
    }

    if (bRearSide == TRUE)
    {
        if (m_imageLotViewRearSide->IsShowRaw())
        {
            switch (m_para.m_type)
            {
                case enumAlgorithmType::Matching:
                    SetROI_BaseOnMatching();
                    break;
                case enumAlgorithmType::BodySize:
                    SetROI_BaseOnBodySize();
                    break;
            }
        }

        m_imageLotViewRearSide->ROI_Show(m_imageLotViewRearSide->IsShowRaw());
    }
    else
    {
        if (m_imageLotView->IsShowRaw())
        {
            switch (m_para.m_type)
            {
                case enumAlgorithmType::Matching:
                    SetROI_BaseOnMatching();
                    break;
                case enumAlgorithmType::BodySize:
                    SetROI_BaseOnBodySize();
                    break;
            }
        }

        m_imageLotView->ROI_Show(m_imageLotView->IsShowRaw());
    }

    return returnValue;
}

void Property_Stitch::SetROI_BaseOnMatching()
{
    bool bRearSide = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR);

    auto& para = m_para.m_paraBasedOnMatching;

    Ipvm::Point32r2 imageCenter(
        m_processor.getImageLot().GetImageSizeX() * 0.5f, m_processor.getImageLot().GetImageSizeY() * 0.5f);

    long sitchCountX = m_processor.getInspectionAreaInfo().m_stichCountX;
    long sitchCountY = m_processor.getInspectionAreaInfo().m_stichCountY;

    if (sitchCountX * sitchCountY == (long)para.m_vecStitchROI.size())
    {
        for (long y = 0; y < sitchCountY; y++)
        {
            for (long x = 0; x < sitchCountX; x++)
            {
                long stitchIndex = y * sitchCountX + x;

                auto& curROI = para.m_vecStitchROI[stitchIndex];
                for (long j = 0; j < 2; j++)
                {
                    if (x != 0)
                    {
                        CString key;
                        key.Format(_T("S%d-H%d"), stitchIndex, j);
                        bRearSide == true
                            ? m_imageLotViewRearSide->ROI_Add(key, _T(""),
                                  m_processor.getScale().convert_BCUToPixel(curROI.m_rtHorRef_BCU[j], imageCenter),
                                  RGB(255, 0, 0), TRUE, TRUE)
                            : m_imageLotView->ROI_Add(key, _T(""),
                                  m_processor.getScale().convert_BCUToPixel(curROI.m_rtHorRef_BCU[j], imageCenter),
                                  RGB(255, 0, 0), TRUE, TRUE);
                    }

                    if (y != 0)
                    {
                        CString key;
                        key.Format(_T("S%d-V%d"), stitchIndex, j);
                        bRearSide == true
                            ? m_imageLotViewRearSide->ROI_Add(key, _T(""),
                                  m_processor.getScale().convert_BCUToPixel(curROI.m_rtVerRef_BCU[j], imageCenter),
                                  RGB(255, 0, 0), TRUE, TRUE)
                            : m_imageLotView->ROI_Add(key, _T(""),
                                  m_processor.getScale().convert_BCUToPixel(curROI.m_rtVerRef_BCU[j], imageCenter),
                                  RGB(255, 0, 0), TRUE, TRUE);
                    }
                }
            }
        }
    }
}

void Property_Stitch::SetROI_BaseOnBodySize()
{
}

void Property_Stitch::GetROI()
{
    bool bRearSide = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR);

    if (bRearSide == TRUE)
    {
        if (!m_imageLotViewRearSide->IsShowRaw())
            return;
    }
    else
    {
        if (!m_imageLotView->IsShowRaw())
            return;
    }

    switch (m_para.m_type)
    {
        case enumAlgorithmType::Matching:
            return GetROI_BaseOnMatching();
        case enumAlgorithmType::BodySize:
            return GetROI_BaseOnBodySize();
    }
}

void Property_Stitch::GetROI_BaseOnMatching()
{
    bool bRearSide = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR);

    const auto& scale = m_processor.getScale();
    auto& para = m_para.m_paraBasedOnMatching;

    Ipvm::Point32r2 imageCenter(
        m_processor.getImageLot().GetImageSizeX() * 0.5f, m_processor.getImageLot().GetImageSizeY() * 0.5f);

    auto backupRoi = para.m_vecStitchROI;
    CString key;
    for (long i = 0; i < (long)para.m_vecStitchROI.size(); i++)
    {
        auto& curROI = para.m_vecStitchROI[i];
        for (long j = 0; j < 2; j++)
        {
            Ipvm::Rect32s roi;

            if (bRearSide == TRUE)
            {
                key.Format(_T("S%d-H%d"), i, j);
                if (m_imageLotViewRearSide->ROI_Get(key, roi))
                {
                    curROI.m_rtHorRef_BCU[j] = scale.convert_PixelToBCU(roi, imageCenter);
                }

                key.Format(_T("S%d-V%d"), i, j);
                if (m_imageLotViewRearSide->ROI_Get(key, roi))
                {
                    curROI.m_rtVerRef_BCU[j] = scale.convert_PixelToBCU(roi, imageCenter);
                }
            }
            else
            {
                key.Format(_T("S%d-H%d"), i, j);
                if (m_imageLotView->ROI_Get(key, roi))
                {
                    curROI.m_rtHorRef_BCU[j] = scale.convert_PixelToBCU(roi, imageCenter);
                }

                key.Format(_T("S%d-V%d"), i, j);
                if (m_imageLotView->ROI_Get(key, roi))
                {
                    curROI.m_rtVerRef_BCU[j] = scale.convert_PixelToBCU(roi, imageCenter);
                }
            }
        }
    }

    // SearchROI, Overlap ROI를 갱신한다.
    if (!SetROI() && GetAsyncKeyState(VK_CONTROL) >= 0)
    {
        MessageBox(
            m_grid->GetSafeHwnd(), _T("Error! Current Search ROI is out of overlapped area!"), _T("Message"), MB_OK);
        para.m_vecStitchROI = backupRoi;
        SetROI();
        ShowOverlayROI();
    }
}

void Property_Stitch::GetROI_BaseOnBodySize()
{
}

void Property_Stitch::UpdateROI()
{
    GetROI();
    SetROI();

    ShowOverlayROI();
}

void Property_Stitch::ShowOverlayROI()
{
    bool bRearSide = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR);

    bRearSide == TRUE ? m_imageLotViewRearSide->Overlay_RemoveAll() : m_imageLotView->Overlay_RemoveAll();

    long stitchCountX = m_processor.getInspectionAreaInfo().m_stichCountX;
    long stitchCountY = m_processor.getInspectionAreaInfo().m_stichCountY;

    if (stitchCountX == 1 && stitchCountY == 1)
    {
        bRearSide == TRUE ? m_imageLotViewRearSide->Overlay_Show(TRUE) : m_imageLotView->Overlay_Show(TRUE);
        return;
    }

    if (bRearSide == TRUE)
    {
        if (m_imageLotViewRearSide->IsShowRaw())
        {
            switch (m_para.m_type)
            {
                case enumAlgorithmType::Matching:
                    ShowOverlayROI_BaseOnMatching();
                    break;
                case enumAlgorithmType::BodySize:
                    ShowOverlayROI_BaseOnBodySize();
                    break;
            }
        }
    }
    else
    {
        if (m_imageLotView->IsShowRaw())
        {
            switch (m_para.m_type)
            {
                case enumAlgorithmType::Matching:
                    ShowOverlayROI_BaseOnMatching();
                    break;
                case enumAlgorithmType::BodySize:
                    ShowOverlayROI_BaseOnBodySize();
                    break;
            }
        }
    }

    bRearSide == TRUE ? m_imageLotViewRearSide->Overlay_Show(TRUE) : m_imageLotView->Overlay_Show(TRUE);
}

void Property_Stitch::ShowOverlayROI_BaseOnMatching()
{
    bool bRearSide = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR);

    long stitchCountX = m_processor.getInspectionAreaInfo().m_stichCountX;
    long stitchCountY = m_processor.getInspectionAreaInfo().m_stichCountY;

    const auto& constants = m_processor.m_constants;

    auto& para = m_para.m_paraBasedOnMatching;
    auto& result = m_processor.m_result;

    CPoint tempPoint;

    if (para.m_vecStitchROI.size() != stitchCountX * stitchCountY)
    {
        m_processor.MakeDefaultTemplateROI_BaseOnMatching();
    }

    for (long y = 0; y < stitchCountY; y++)
    {
        for (long x = 0; x < stitchCountX; x++)
        {
            long stitchIndex = y * stitchCountX + x;
            auto& para_curStitchROI = para.m_vecStitchROI[stitchIndex];

            if (stitchIndex < result.m_stitchRois.size())
            {
                auto& result_curStitchROI = result.m_stitchRois[stitchIndex];

                const auto& verPreOverlap = constants.GetVerPreOverlapROI(stitchIndex);
                const auto& verCurOverlap = constants.GetVerCurOverlapROI(stitchIndex);
                const auto& horPreOverlap = constants.GetHorPreOverlapROI(stitchIndex);
                const auto& horCurOverlap = constants.GetHorCurOverlapROI(stitchIndex);

                if (bRearSide == TRUE)
                {
                    m_imageLotViewRearSide->Overlay_AddRectangle(verPreOverlap, RGB(0, 0, 255));
                    m_imageLotViewRearSide->Overlay_AddRectangle(verCurOverlap, RGB(0, 255, 0));
                    m_imageLotViewRearSide->Overlay_AddRectangle(horPreOverlap, RGB(0, 0, 255));
                    m_imageLotViewRearSide->Overlay_AddRectangle(horCurOverlap, RGB(0, 255, 0));
                }
                else
                {
                    m_imageLotView->Overlay_AddRectangle(verPreOverlap, RGB(0, 0, 255));
                    m_imageLotView->Overlay_AddRectangle(verCurOverlap, RGB(0, 255, 0));
                    m_imageLotView->Overlay_AddRectangle(horPreOverlap, RGB(0, 0, 255));
                    m_imageLotView->Overlay_AddRectangle(horCurOverlap, RGB(0, 255, 0));
                }

                for (long i = 0; i < 2; i++)
                {
                    const auto& frtHorMatched = result_curStitchROI.m_horMatchingInfo.m_rtMatched[i];
                    const auto& frtVerMatched = result_curStitchROI.m_verMatchingInfo.m_rtMatched[i];

                    if (bRearSide == TRUE)
                    {
                        m_imageLotViewRearSide->Overlay_AddRectangle(frtHorMatched, RGB(255, 0, 255));
                        m_imageLotViewRearSide->Overlay_AddRectangle(frtVerMatched, RGB(0, 255, 255));
                    }
                    else
                    {
                        m_imageLotView->Overlay_AddRectangle(frtHorMatched, RGB(255, 0, 255));
                        m_imageLotView->Overlay_AddRectangle(frtVerMatched, RGB(0, 255, 255));
                    }
                }
            }

            const auto& rtHorSearch = para_curStitchROI.m_rtHorSearch;
            const auto& rtVerSearch = para_curStitchROI.m_rtVerSearch;

            for (long i = 0; i < 2; i++)
            {
                CString textH;
                CString textV;
                // textH.Format(_T("S%d_H%d::%.f\n X:%.1f, Y:%.1f"), stitchIndex + 1, i + 1, curStitchInfo.m_horMatchingInfo.m_fMatchScore[i], curStitchInfo.m_horMatchingInfo.m_ptShift[i].m_x, curStitchInfo.m_horMatchingInfo.m_ptShift[i].m_y);
                // textV.Format(_T("S%d_V%d::%.f\n X:%.1f, Y:%.1f"), stitchIndex + 1, i + 1, curStitchInfo.m_verMatchingInfo.m_fMatchScore[i], curStitchInfo.m_verMatchingInfo.m_ptShift[i].m_x, curStitchInfo.m_verMatchingInfo.m_ptShift[i].m_y);
                textH.Format(_T("S%d_H%d"), stitchIndex + 1, i + 1);
                textV.Format(_T("S%d_V%d"), stitchIndex + 1, i + 1);
                if (!rtHorSearch[i].IsRectEmpty())
                {
                    bRearSide == TRUE
                        ? m_imageLotViewRearSide->Overlay_AddText(
                              rtHorSearch[i].TopLeft(), textH, RGB(255, 0, 255), 50L)
                        : m_imageLotView->Overlay_AddText(rtHorSearch[i].TopLeft(), textH, RGB(255, 0, 255), 50L);
                }
                if (!rtVerSearch[i].IsRectEmpty())
                {
                    bRearSide == TRUE
                        ? m_imageLotViewRearSide->Overlay_AddText(rtVerSearch[i].TopLeft(), textV, RGB(255, 0, 0), 50L)
                        : m_imageLotView->Overlay_AddText(rtVerSearch[i].TopLeft(), textV, RGB(255, 0, 0), 50L);
                }

                if (bRearSide == TRUE)
                {
                    m_imageLotViewRearSide->Overlay_AddRectangle(rtHorSearch[i], RGB(255, 0, 255));
                    m_imageLotViewRearSide->Overlay_AddRectangle(rtVerSearch[i], RGB(0, 255, 255));
                }
                else
                {
                    m_imageLotView->Overlay_AddRectangle(rtHorSearch[i], RGB(255, 0, 255));
                    m_imageLotView->Overlay_AddRectangle(rtVerSearch[i], RGB(0, 255, 255));
                }
            }
        }
    }
}

void Property_Stitch::ShowOverlayROI_BaseOnBodySize()
{
    bool bRearSide = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR);

    auto& para = m_para.m_paraBasedOnBodySize;

    for (auto& curROI : para.m_stitchPara)
    {
        for (auto& result : curROI.m_resultRois)
        {
            bRearSide == TRUE ? m_imageLotViewRearSide->Overlay_AddRectangle(result.m_roi, RGB(0, 255, 0))
                              : m_imageLotView->Overlay_AddRectangle(result.m_roi, RGB(0, 255, 0));
            for (auto& edge : result.m_edges)
            {
                bRearSide == TRUE ? m_imageLotViewRearSide->Overlay_AddXPoint(edge, RGB(255, 0, 0))
                                  : m_imageLotView->Overlay_AddXPoint(edge, RGB(255, 0, 0));
            }
        }

        if (bRearSide == TRUE)
        {
            m_imageLotViewRearSide->Overlay_AddXPoint(curROI.m_resultCornerPoint, RGB(0, 255, 0));
            m_imageLotViewRearSide->Overlay_AddXPoint(curROI.m_resultRefCornerPoint, RGB(0, 128, 0));
        }
        else
        {
            m_imageLotView->Overlay_AddXPoint(curROI.m_resultCornerPoint, RGB(0, 255, 0));
            m_imageLotView->Overlay_AddXPoint(curROI.m_resultRefCornerPoint, RGB(0, 128, 0));
        }
    }
}

void Property_Stitch::SelChangedRaw()
{
    SetROI();
    ShowOverlayROI();
}

void Property_Stitch::buttonClick_DefaultTemplateRoi()
{
    bool bRearSide = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
        && m_processor.GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_REAR);

    bRearSide == TRUE ? m_imageLotViewRearSide->ROI_RemoveAll() : m_imageLotView->ROI_RemoveAll();
    m_processor.MakeDefaultTemplateROI_BaseOnMatching();

    SetROI();
    ShowOverlayROI();
}
