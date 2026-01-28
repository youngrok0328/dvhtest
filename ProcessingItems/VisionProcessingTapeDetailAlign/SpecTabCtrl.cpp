//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SpecTabCtrl.h"

//CPP_2_________________________________ This project's headers
#include "Para.h"
#include "VisionProcessingTapeDetailAlign.h"

//CPP_4_________________________________ External library headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomNumber.h"

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum GridChildItemID
{
    EDGE_ALIGN_PARAM_SPROCKET_HOLE_EDGE_DETECT_MODE = 1,

    EDGE_ALIGN_PARAM_SET_IMAGE_COMBINE,
    EDGE_ALIGN_PARAM_EDGE_DETECT_MODE,
    EDGE_ALIGN_PARAM_GAP_MICRO,
    EDGE_ALIGN_PARAM_GAP_PIXEL_X,
    EDGE_ALIGN_PARAM_GAP_PIXEL_Y,
};

SpecTabCtrl::SpecTabCtrl(VisionProcessingTapeDetailAlign& processor)
    : m_processor(processor)
{
}

bool SpecTabCtrl::Create(const RECT& rect, CWnd* parent)
{
    if (!CTabCtrl::Create(WS_CHILD | WS_VISIBLE, rect, parent, IDC_STATIC))
    {
        return false;
    }

    // 탭 컨트롤 내부에 PropertyGrid 컨트롤 생성
    CRect tabRect;
    GetClientRect(tabRect);
    tabRect.DeflateRect(10, 30, 10, 10);
    m_propertySprocketHole.Create(tabRect, this, IDC_STATIC);
    m_propertyPocket.Create(tabRect, this, IDC_STATIC);
    m_propertyDevice.Create(tabRect, this, IDC_STATIC);

    // 탭 컨트롤에 탭 항목 추가
    InsertItem(0, _T("Sprocket Hole"));
    InsertItem(1, _T("Pocket"));
    InsertItem(2, _T("Device"));
    OnTcnSelchange(nullptr, nullptr);

    return true;
}
BEGIN_MESSAGE_MAP(SpecTabCtrl, CTabCtrl)
ON_WM_DESTROY()
ON_NOTIFY_REFLECT(TCN_SELCHANGE, &SpecTabCtrl::OnTcnSelchange)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

void SpecTabCtrl::CallImageCombine(VisionInspFrameIndex& imageFrameIndex, ImageProcPara& imageProcPara)
{
    auto* para = m_processor.m_para;

    ::Combine_SetParameter(m_processor, imageFrameIndex.getFrameIndex(), false, &imageProcPara);
}

void SpecTabCtrl::UpdatePropertySprocketHole(CXTPPropertyGrid& propertyGrid)
{
    const auto& scale = m_processor.getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    propertyGrid.ResetContent();
    auto& para = m_processor.m_para->m_sprocketHoleAlign;

    if (auto* category = propertyGrid.AddCategory(_T("Edge Detect Parameter")))
    {
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Edge Direction"), para.m_edgeType, (int*)&para.m_edgeType)))
        {
            item->GetConstraints()->AddConstraint(_T("Falling"), 0);
            item->GetConstraints()->AddConstraint(_T("Rising"), 1);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Detect Mode"), para.m_edgeDetectMode, (int*)&para.m_edgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), EdgeDetectMode_BestEdge);
            item->GetConstraints()->AddConstraint(_T("First Edge"), EdgeDetectMode_FirstEdge);
            item->GetConstraints()->AddConstraint(_T("Auto Detect"), EdgeDetectMode_AutoDetect);

            item->SetID(EDGE_ALIGN_PARAM_SPROCKET_HOLE_EDGE_DETECT_MODE);
        }

        category
            ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("FirstEdge Min Value"),
            para.m_firstEdgeMinThreshold, _T("%.3lf GV"), para.m_firstEdgeMinThreshold, para.m_firstEdgeMinThreshold,
            para.m_firstEdgeMinThreshold, 0.01f));

        category->Expand();
    }

    if (auto* category = propertyGrid.AddCategory(_T("Search Parameter")))
    {
        category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
            _T("Start (%)"), para.m_persent_start, _T("%.3lf %%"), para.m_persent_start, 0.f, 300.f, 0.1f));
        category->AddChildItem(
            new XTPPropertyGridItemCustomFloat<float>(_T("End (%)"), para.m_persent_end, _T("%.3lf %%"), para.m_persent_end, 0.f, 300.f, 0.1f));

        category->AddChildItem(new XTPPropertyGridItemCustomNumber<long>(
            _T("Edge Count"), para.m_edgeCount, _T("%d"), para.m_edgeCount, 0, 500));

        category->Expand();
    }

    if (auto* category = propertyGrid.AddCategory(_T("ETC")))
    {
        para.m_imageFrameIndex.makePropertyGridItem(category, _T("Align Image Frame Index"), 0);
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(EDGE_ALIGN_PARAM_SET_IMAGE_COMBINE);
        category->Expand();
    }

    propertyGrid.SetViewDivider(0.6);
    propertyGrid.HighlightChangedItems(TRUE);
}

void SpecTabCtrl::UpdatePropertyPocket(CXTPPropertyGrid& propertyGrid)
{
    const auto& scale = m_processor.getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    propertyGrid.ResetContent();
    auto& para = m_processor.m_para->m_pocketAlign;

    if (auto* category = propertyGrid.AddCategory(_T("Edge Detect Parameter")))
    {
        auto& edgeParams = para.m_edgeParams;

        category->AddChildItem(new CXTPPropertyGridItemBool(_T("Skip Align"), para.m_skipAlign, &para.m_skipAlign));

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Search Direction"), edgeParams.m_searchDirection, (int*)&edgeParams.m_searchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), 0);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), 1);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Direction"), edgeParams.m_edgeType, (int*)&edgeParams.m_edgeType)))
        {
            item->GetConstraints()->AddConstraint(_T("Falling"), 0);
            item->GetConstraints()->AddConstraint(_T("Rising"), 1);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Detect Mode"), edgeParams.m_edgeDetectMode, (int*)&edgeParams.m_edgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), EdgeDetectMode_BestEdge);
            item->GetConstraints()->AddConstraint(_T("First Edge"), EdgeDetectMode_FirstEdge);
            item->GetConstraints()->AddConstraint(_T("Auto Detect"), EdgeDetectMode_AutoDetect);

            item->SetID(EDGE_ALIGN_PARAM_EDGE_DETECT_MODE);
        }

        category->AddChildItem(
            new XTPPropertyGridItemCustomFloat<float>(_T("FirstEdge Min Value"), edgeParams.m_firstEdgeMinThreshold, _T("%.3lf GV"), edgeParams.m_firstEdgeMinThreshold,
            edgeParams.m_firstEdgeMinThreshold, edgeParams.m_firstEdgeMinThreshold, 0.01f));
        auto* edgeAlignGap = category->AddChildItem(new CXTPPropertyGridItemDouble(
            _T("Detected EdgeAlign Gap(um)"), edgeParams.m_edgeAlignGap_um, _T("%.3lf um")));
        edgeAlignGap->SetID(EDGE_ALIGN_PARAM_GAP_MICRO);

        long nGapToPixel_X = (long)((edgeParams.m_edgeAlignGap_um / fpx2um_x) + 0.5f);
        long nGapToPixel_Y = (long)((edgeParams.m_edgeAlignGap_um / fpx2um_y) + 0.5f);

        if (nGapToPixel_X < 2 || nGapToPixel_Y < 2)
        {
            nGapToPixel_X = 2;
            nGapToPixel_Y = 2;
            edgeParams.m_edgeAlignGap_um = nGapToPixel_X * fpx2um_x;
        }
        edgeAlignGap->AddChildItem(new CXTPPropertyGridItemNumber(_T("Detected EdgeAlign Gap X(px)"), nGapToPixel_X))->SetID(EDGE_ALIGN_PARAM_GAP_PIXEL_X);
        edgeAlignGap->AddChildItem(new CXTPPropertyGridItemNumber(_T("Detected EdgeAlign Gap Y(px)"), nGapToPixel_X))->SetID(EDGE_ALIGN_PARAM_GAP_PIXEL_Y);

        category->Expand();
    }

    for (auto index = 0; index < 4; index++)
    {
        auto& searchPara = para.m_edgeSearchRois[index];
        CString categoryName;
        switch (index)
        {
        case LEFT:  categoryName = _T("Search Parameter (Left)"); break;
        case RIGHT: categoryName = _T("Search Parameter (Right)"); break;
        case UP:    categoryName = _T("Search Parameter (Top)"); break;
        case DOWN:  categoryName = _T("Search Parameter (Bottom)"); break;
        }

        if (auto* category = propertyGrid.AddCategory(categoryName))
        {
            category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Search Length (um)"),
                searchPara.m_searchLength_um, _T("%.3lf um"), searchPara.m_searchLength_um, 0.f, 5000.f, 0.1f));

            category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Start (%)"),
                searchPara.m_persent_start, _T("%.3lf %%"), searchPara.m_persent_start, 0.f, 300.f, 0.1f));
            category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
                _T("End (%)"), searchPara.m_persent_end, _T("%.3lf %%"), searchPara.m_persent_end, 0.f, 300.f, 0.1f));

            category->Expand();
        }
    }

    if (auto* category = propertyGrid.AddCategory(_T("ETC")))
    {
        para.m_imageFrameIndex.makePropertyGridItem(category, _T("Align Image Frame Index"), 0);
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(EDGE_ALIGN_PARAM_SET_IMAGE_COMBINE);
        category->Expand();
    }

    propertyGrid.SetViewDivider(0.6);
    propertyGrid.HighlightChangedItems(TRUE);
}

void SpecTabCtrl::UpdatePropertyDevice(CXTPPropertyGrid& propertyGrid)
{
    const auto& scale = m_processor.getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    propertyGrid.ResetContent();
    auto& para = m_processor.m_para->m_deviceAlign;

    if (auto* category = propertyGrid.AddCategory(_T("Edge Detect Parameter")))
    {
        auto& edgeParams = para.m_edgeParams;
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Search Direction"), edgeParams.m_searchDirection, (int*)&edgeParams.m_searchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), 0);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), 1);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Direction"), edgeParams.m_edgeType, (int*)&edgeParams.m_edgeType)))
        {
            item->GetConstraints()->AddConstraint(_T("Falling"), 0);
            item->GetConstraints()->AddConstraint(_T("Rising"), 1);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Edge Detect Mode"), edgeParams.m_edgeDetectMode, (int*)&edgeParams.m_edgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), EdgeDetectMode_BestEdge);
            item->GetConstraints()->AddConstraint(_T("First Edge"), EdgeDetectMode_FirstEdge);
            item->GetConstraints()->AddConstraint(_T("Auto Detect"), EdgeDetectMode_AutoDetect);

            item->SetID(EDGE_ALIGN_PARAM_EDGE_DETECT_MODE);
        }

        category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("FirstEdge Min Value"), edgeParams.m_firstEdgeMinThreshold, _T("%.3lf GV"), edgeParams.m_firstEdgeMinThreshold,
            edgeParams.m_firstEdgeMinThreshold, edgeParams.m_firstEdgeMinThreshold, 0.01f));
        auto* edgeAlignGap = category->AddChildItem(new CXTPPropertyGridItemDouble(
            _T("Detected EdgeAlign Gap(um)"), edgeParams.m_edgeAlignGap_um, _T("%.3lf um")));
        edgeAlignGap->SetID(EDGE_ALIGN_PARAM_GAP_MICRO);

        long nGapToPixel_X = (long)((edgeParams.m_edgeAlignGap_um / fpx2um_x) + 0.5f);
        long nGapToPixel_Y = (long)((edgeParams.m_edgeAlignGap_um / fpx2um_y) + 0.5f);

        if (nGapToPixel_X < 2 || nGapToPixel_Y < 2)
        {
            nGapToPixel_X = 2;
            nGapToPixel_Y = 2;
            edgeParams.m_edgeAlignGap_um = nGapToPixel_X * fpx2um_x;
        }
        edgeAlignGap->AddChildItem(new CXTPPropertyGridItemNumber(_T("Detected EdgeAlign Gap X(px)"), nGapToPixel_X))->SetID(EDGE_ALIGN_PARAM_GAP_PIXEL_X);
        edgeAlignGap->AddChildItem(new CXTPPropertyGridItemNumber(_T("Detected EdgeAlign Gap Y(px)"), nGapToPixel_Y))->SetID(EDGE_ALIGN_PARAM_GAP_PIXEL_Y);

        category->Expand();
    }

    for (auto index = 0; index < 4; index++)
    {
        auto& searchPara = para.m_edgeSearchRois[index];
        CString categoryName;
        switch (index)
        {
        case LEFT:  categoryName = _T("Search Parameter (Left)"); break;
        case RIGHT: categoryName = _T("Search Parameter (Right)"); break;
        case UP:    categoryName = _T("Search Parameter (Top)"); break;
        case DOWN:  categoryName = _T("Search Parameter (Bottom)"); break;
        }

        if (auto* category = propertyGrid.AddCategory(categoryName))
        {
            category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Search Length (um)"),
                searchPara.m_searchLength_um, _T("%.3lf um"), searchPara.m_searchLength_um, 0.f, 5000.f, 0.1f));

            category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Start (%)"),
                searchPara.m_persent_start, _T("%.3lf %%"), searchPara.m_persent_start, 0.f, 300.f, 0.1f));
            category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
                _T("End (%)"), searchPara.m_persent_end, _T("%.3lf %%"), searchPara.m_persent_end, 0.f, 300.f, 0.1f));

            category->Expand();
        }
    }

    if (auto* category = propertyGrid.AddCategory(_T("ETC")))
    {
        para.m_imageFrameIndex.makePropertyGridItem(category, _T("Align Image Frame Index"), 0);
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(EDGE_ALIGN_PARAM_SET_IMAGE_COMBINE);
        category->Expand();
    }

    propertyGrid.SetViewDivider(0.6);
    propertyGrid.HighlightChangedItems(TRUE);
}

void SpecTabCtrl::UpdatePropertyGrid()
{
    UpdatePropertySprocketHole(m_propertySprocketHole);
    UpdatePropertyPocket(m_propertyPocket);
    UpdatePropertyDevice(m_propertyDevice);
}

void SpecTabCtrl::OnDestroy()
{
    CTabCtrl::OnDestroy();

    m_propertySprocketHole.DestroyWindow();
    m_propertyPocket.DestroyWindow();
    m_propertyDevice.DestroyWindow();
}

void SpecTabCtrl::OnTcnSelchange(NMHDR* pNMHDR, LRESULT* pResult)
{
    auto selectedIndex = GetCurSel();
    m_propertySprocketHole.ShowWindow(selectedIndex == 0 ? SW_SHOW : SW_HIDE);
    m_propertyPocket.ShowWindow(selectedIndex == 1 ? SW_SHOW : SW_HIDE);
    m_propertyDevice.ShowWindow(selectedIndex == 2 ? SW_SHOW : SW_HIDE);

    if (pResult != nullptr)
    {
        *pResult = 0;
    }
}

LRESULT SpecTabCtrl::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    const auto& scale = m_processor.getScale();
    float fpx2um_x = scale.pixelToUm().m_x;
    float fpx2um_y = scale.pixelToUm().m_y;

    auto* para = m_processor.m_para;
    auto* propertyGrid = item->GetGrid()->GetPropertyGrid();

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case EDGE_ALIGN_PARAM_SET_IMAGE_COMBINE:
                if (propertyGrid == &m_propertySprocketHole)
                {
                    CallImageCombine(
                        para->m_sprocketHoleAlign.m_imageFrameIndex, para->m_sprocketHoleAlign.m_imageProc);
                }
                else if (propertyGrid == &m_propertyPocket)
                {
                    CallImageCombine(para->m_pocketAlign.m_imageFrameIndex, para->m_pocketAlign.m_imageProc);
                }
                else if (propertyGrid == &m_propertyDevice)
                {
                    CallImageCombine(para->m_deviceAlign.m_imageFrameIndex, para->m_deviceAlign.m_imageProc);
                }
                break;
        }

        value->SetBool(FALSE);
        return 0;
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
    {
        double data = value->GetDouble();

        switch (item->GetID())
        {
            case EDGE_ALIGN_PARAM_GAP_MICRO:
            {
                auto* itemGapPixelX = dynamic_cast<CXTPPropertyGridItemNumber*>(item->GetChilds()->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_X));
                auto* itemGapPixelY = dynamic_cast<CXTPPropertyGridItemNumber*>(item->GetChilds()->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_Y));
                auto& gap_um = (propertyGrid == &m_propertyPocket) ? para->m_pocketAlign.m_edgeParams.m_edgeAlignGap_um
                                                                   : para->m_deviceAlign.m_edgeParams.m_edgeAlignGap_um;

                auto nGapToPixel_X = (long)((value->GetDouble() / fpx2um_x) + 0.5f);
                auto nGapToPixel_Y = (long)((value->GetDouble() / fpx2um_y) + 0.5f);

                if (nGapToPixel_X < 2 || nGapToPixel_Y < 2)
                {
                    nGapToPixel_X = 2;
                    nGapToPixel_Y = 2;
                    gap_um = nGapToPixel_X * fpx2um_x;

                    value->SetDouble(gap_um);
                    itemGapPixelX->SetNumber(nGapToPixel_X);
                    itemGapPixelY->SetNumber(nGapToPixel_Y);
                }
                else
                {
                    gap_um = (float)value->GetDouble();

                    itemGapPixelX->SetNumber(nGapToPixel_X);
                    itemGapPixelY->SetNumber(nGapToPixel_Y);
                }
            }
            break;
        }
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
    {
        // Sub Item중 Gap Pixel X, Y가 있으면 미리 얻어놓기
        switch (item->GetID())
        {
            case EDGE_ALIGN_PARAM_GAP_PIXEL_X:
                {
                    auto* itemGap = dynamic_cast<CXTPPropertyGridItemDouble*>(item->GetParentItem());
                    auto* itemGapPixelY = dynamic_cast<CXTPPropertyGridItemNumber*>(itemGap->GetChilds()->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_Y));

                    auto& gap_um = (propertyGrid == &m_propertyPocket) ?
                        para->m_pocketAlign.m_edgeParams.m_edgeAlignGap_um :
                        para->m_deviceAlign.m_edgeParams.m_edgeAlignGap_um;

                    if (value->GetNumber() < 2)
                    {
                        auto nGapToPixel_X = 2;
                        auto nGapToPixel_Y = 2;
                        auto nGapToum = nGapToPixel_X * fpx2um_x;

                        gap_um = nGapToum;

                        value->SetNumber(nGapToPixel_X);
                        itemGap->SetDouble(gap_um);
                        itemGapPixelY->SetNumber(nGapToPixel_Y);
                    }
                    else
                    {
                        auto nGapToum = static_cast<float>(value->GetNumber()) * fpx2um_x;
                        auto nGapToPixel_Y = (long)((nGapToum / fpx2um_y) + 0.5f);

                        gap_um = nGapToum;

                        itemGap->SetDouble(gap_um);
                        itemGapPixelY->SetNumber(nGapToPixel_Y);
                    }
                }
                break;

            case EDGE_ALIGN_PARAM_GAP_PIXEL_Y:
                {
                    auto* itemGap = dynamic_cast<CXTPPropertyGridItemDouble*>(item->GetParentItem());
                    auto* itemGapPixelX = dynamic_cast<CXTPPropertyGridItemNumber*>(itemGap->GetChilds()->FindItem(EDGE_ALIGN_PARAM_GAP_PIXEL_X));

                    auto& gap_um = (propertyGrid == &m_propertyPocket) ?
                        para->m_pocketAlign.m_edgeParams.m_edgeAlignGap_um :
                        para->m_deviceAlign.m_edgeParams.m_edgeAlignGap_um;

                    if (value->GetNumber() < 2)
                    {
                        auto nGapToPixel_X = 2;
                        auto nGapToPixel_Y = 2;
                        auto nGapToum = nGapToPixel_Y * fpx2um_y;

                        gap_um = nGapToum;

                        value->SetNumber(nGapToPixel_Y);
                        itemGap->SetDouble(gap_um);
                        itemGapPixelX->SetNumber(nGapToPixel_X);
                    }
                    else
                    {
                        auto nGapToum = static_cast<float>(value->GetNumber()) * fpx2um_y;
                        auto nGapToPixel_X = (long)((nGapToum / fpx2um_x) + 0.5f);

                        gap_um = nGapToum;

                        itemGap->SetDouble(gap_um);
                        itemGapPixelX->SetNumber(nGapToPixel_X);
                    }
                }
                break;
        }
    }

    return 0;
}
