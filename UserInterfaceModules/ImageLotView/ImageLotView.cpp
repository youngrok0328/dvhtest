//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ImageLotView.h"

//CPP_2_________________________________ This project's headers
#include "ImageLotViewImpl.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"
#include "../ImageViewEx/ImageViewEx.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <afxmt.h>

//CPP_5_________________________________ Standard library headers
#include <map>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
std::map<ImageLotViewImpl*, BOOL> g_imageLotViews;
CCriticalSection g_imageLotViews_Lock;

void ImageLotView::PressSystemKey(WPARAM key)
{
    g_imageLotViews_Lock.Lock();
    for (auto& view : g_imageLotViews)
    {
        switch (key)
        {
            case VK_F7:
                view.first->SystemEvent_BackPane();
                break; // Back Pane
            case VK_F8:
                view.first->SystemEvent_NextPane();
                break; // Next Pane
            case VK_F9:
                view.first->SystemEvent_UpdateCurrentPaneZoom();
                break; // Pane Zoome
            case VK_F10:
                view.first->SystemEvent_ChangeOverlayMode();
                break; // Cross Line
        }
    }

    g_imageLotViews_Lock.Unlock();
}

ImageLotView::ImageLotView(const CRect& roi, VisionUnitAgent& unitItem, bool showInitialPaneArea, HWND hwndParent,
    const enSideVisionModule i_eSideVisionModule)
    : m_impl(nullptr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    auto* parentWnd = CWnd::FromHandle(hwndParent);

    m_impl = new ImageLotViewImpl(roi, unitItem.getIllumInfo2D(), unitItem.getInspectionAreaInfo(),
        unitItem.getTrayScanSpec(), unitItem.getImageLot(), unitItem.getImageLotInsp(), showInitialPaneArea, parentWnd,
        i_eSideVisionModule);

    m_impl->Create(ImageLotViewImpl::IDD, parentWnd);
    m_impl->ShowWindow(SW_SHOW);

    g_imageLotViews_Lock.Lock();
    g_imageLotViews[m_impl] = TRUE;
    g_imageLotViews_Lock.Unlock();
}

ImageLotView::ImageLotView(const CRect& roi, VisionProcessing& processingItem, bool enableLotInsp,
    bool showInitialPaneArea, HWND hwndParent, const enSideVisionModule i_eSideVisionModule)
    : m_impl(nullptr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    auto* parentWnd = CWnd::FromHandle(hwndParent);

    if (enableLotInsp)
    {
        m_impl = new ImageLotViewImpl(roi, processingItem.getIllumInfo2D(), processingItem.getInspectionAreaInfo(),
            processingItem.getTrayScanSpec(), processingItem.getImageLot(), processingItem.getImageLotInsp(),
            showInitialPaneArea, parentWnd, i_eSideVisionModule);
    }
    else
    {
        m_impl = new ImageLotViewImpl(roi, processingItem.getIllumInfo2D(), processingItem.getInspectionAreaInfo(),
            processingItem.getTrayScanSpec(), processingItem.getImageLot(), showInitialPaneArea, parentWnd,
            i_eSideVisionModule);
    }

    m_impl->Create(ImageLotViewImpl::IDD, parentWnd);
    m_impl->ShowWindow(SW_SHOW);

    g_imageLotViews_Lock.Lock();
    g_imageLotViews[m_impl] = TRUE;
    g_imageLotViews_Lock.Unlock();
}

ImageLotView::ImageLotView(
    const CRect& roi, VisionProcessing& processingItem, HWND hwndParent, const enSideVisionModule i_eSideVisionModule)
    : m_impl(nullptr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    auto* parentWnd = CWnd::FromHandle(hwndParent);

    m_impl = new ImageLotViewImpl(roi, processingItem.getIllumInfo2D(), processingItem.getInspectionAreaInfo(),
        processingItem.getTrayScanSpec(), processingItem.getScale(), processingItem.getImageLotInsp(), parentWnd,
        i_eSideVisionModule);

    m_impl->Create(ImageLotViewImpl::IDD, parentWnd);
    m_impl->ShowWindow(SW_SHOW);

    g_imageLotViews_Lock.Lock();
    g_imageLotViews[m_impl] = TRUE;
    g_imageLotViews_Lock.Unlock();
}

ImageLotView::~ImageLotView()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_impl->GetSafeHwnd())
    {
        m_impl->DestroyWindow();
    }

    g_imageLotViews_Lock.Lock();
    g_imageLotViews.erase(m_impl);
    g_imageLotViews_Lock.Unlock();

    delete m_impl;
}

HWND ImageLotView::GetSafeHwnd()
{
    if (this == nullptr)
    {
        return NULL;
    }

    if (m_impl == nullptr)
    {
        return NULL;
    }

    return m_impl->GetSafeHwnd();
}

ImageViewEx* ImageLotView::GetCoreView()
{
    if (this == nullptr)
    {
        return NULL;
    }

    if (m_impl == nullptr)
    {
        return NULL;
    }

    return m_impl->m_coreView;
}

void ImageLotView::SetMode(Mode mode)
{
    m_impl->SetMode(mode);
}

void ImageLotView::SetPaneWindowEnabled(bool enabled)
{
    m_impl->SetPaneWindowEnabled(enabled);
}

void ImageLotView::SetInspectionAreaInfo(const InspectionAreaInfo& inspectionAreaInfo)
{
    m_impl->SetInspectionAreaInfo(inspectionAreaInfo);
}

void ImageLotView::SetMessage_RoiChanged(UINT message)
{
    m_impl->SetMessage_RoiChanged(message);
}

void ImageLotView::SetMessage_SelChangedPane(UINT message)
{
    m_impl->SetMessage_SelChangedPane(message);
}

void ImageLotView::SetMessage_SelChangedRaw(UINT message)
{
    m_impl->SetMessage_SelChangedRaw(message);
}

void ImageLotView::SetMessage_SelChangedImage(UINT message)
{
    m_impl->SetMessage_SelChangedImage(message);
}

void ImageLotView::SetTarget(VisionImageLot& imageLot, VisionImageLotInsp& imageLotInsp)
{
    m_impl->SetTarget(imageLot, imageLotInsp);
}

void ImageLotView::ShowImage(const long frameIndex, const bool showRaw, const bool bIsSrcRaw)
{
    m_impl->ShowImage(frameIndex, showRaw, bIsSrcRaw);
}

void ImageLotView::ShowCurrentImage(const bool showRaw)
{
    m_impl->ShowImage(m_impl->GetCurrentFrame(), showRaw);
}

void ImageLotView::ShowPrevImage(const bool showRaw)
{
    m_impl->ShowImage(m_impl->GetCurrentFrame() - 1, showRaw);
}

void ImageLotView::ShowNextImage(const bool showRaw)
{
    m_impl->ShowImage(m_impl->GetCurrentFrame() + 1, showRaw);
}

void ImageLotView::ShowPane(const long paneIndex, const bool showRaw)
{
    m_impl->ShowPane(paneIndex, showRaw);
}

long ImageLotView::GetCurrentImageFrame() const
{
    return m_impl->m_currentImageIndex;
}

void ImageLotView::ZoomImageFit()
{
    m_impl->ZoomImageFit();
}

void ImageLotView::ZoomPaneFit(bool displayPaneROI)
{
    m_impl->ZoomPaneFit(displayPaneROI);
}

void ImageLotView::NavigateTo(const Ipvm::Rect32s& roi)
{
    m_impl->m_coreView->NavigateTo(roi);
}

void ImageLotView::SetScanIndexField(LPCTSTR text)
{
    m_impl->SetScanIndexField(text);
}

void ImageLotView::UpdateState()
{
    m_impl->UpdateState();
}

void ImageLotView::OnJobChanged()
{
    m_impl->OnJobChanged(false, false);
}

bool ImageLotView::IsShowRaw()
{
    return m_impl->IsShowRaw();
}

void ImageLotView::SetImage(const Ipvm::Image8u& image, LPCTSTR state)
{
    m_impl->m_coreView->SetImage(image);
    m_impl->m_frameText = state;
    m_impl->UpdateState();
}

void ImageLotView::SetImage(const Ipvm::Image8u3& image, LPCTSTR state)
{
    m_impl->m_coreView->SetImage(image);
    m_impl->m_frameText = state;
    m_impl->UpdateState();
}

void ImageLotView::Overlay_Show(BOOL show)
{
    if (show)
    {
        m_impl->m_coreView->ImageOverlayShow();
    }
    else
    {
        m_impl->m_coreView->ImageOverlayHide();
    }
}

void ImageLotView::Overlay_AddLine(const Ipvm::Point32s2& begin, const Ipvm::Point32s2& end, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(
        Ipvm::LineSeg32r(CAST_FLOAT(begin.m_x), CAST_FLOAT(begin.m_y), CAST_FLOAT(end.m_x), CAST_FLOAT(end.m_y)),
        color);
}

void ImageLotView::Overlay_AddLine(const Ipvm::Point32r2& begin, const Ipvm::Point32r2& end, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(Ipvm::LineSeg32r(begin.m_x, begin.m_y, end.m_x, end.m_y), color);
}

void ImageLotView::Overlay_AddLine(
    const std::vector<Ipvm::Point32r2> begin, const std::vector<Ipvm::Point32r2> end, COLORREF color)
{
    if (begin.size() != end.size())
    {
        ASSERT(!_T("??"));
        return;
    }

    for (long index = 0; index < long(begin.size()); index++)
    {
        Overlay_AddLine(begin[index], end[index], color);
    }
}

void ImageLotView::Overlay_AddRectangle(const Ipvm::Rect32r& rect, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(rect, color);
}

void ImageLotView::Overlay_AddRectangle(const Ipvm::Rect32s& rect, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(rect, color);
}

void ImageLotView::Overlay_AddRectangle(const PI_RECT& rect, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(rect, color);
}

void ImageLotView::Overlay_AddRectangle(const FPI_RECT& rect, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(rect, color);
}

void ImageLotView::Overlay_AddRectangles(const std::vector<Ipvm::Rect32s>& rects, COLORREF color)
{
    for (auto& object : rects)
    {
        m_impl->m_coreView->ImageOverlayAdd(object, color);
    }
}

void ImageLotView::Overlay_AddRectangles(const std::vector<Ipvm::Rect32r>& rects, COLORREF color)
{
    for (auto& object : rects)
    {
        m_impl->m_coreView->ImageOverlayAdd(object, color);
    }
}

void ImageLotView::Overlay_AddRectangles(const std::vector<FPI_RECT>& rects, COLORREF color)
{
    for (auto& object : rects)
    {
        m_impl->m_coreView->ImageOverlayAdd(object, color);
    }
}

void ImageLotView::Overlay_AddRectangles(const std::vector<Ipvm::Quadrangle32r>& rects, COLORREF color)
{
    for (auto& object : rects)
    {
        m_impl->m_coreView->ImageOverlayAdd(object, color);
    }
}

void ImageLotView::Overlay_AddEllipse(const Ipvm::Rect32r& rect, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(rect, color);
}

void ImageLotView::Overlay_AddEllipses(const std::vector<Ipvm::Rect32r>& rect, COLORREF color)
{
    for (auto& object : rect)
    {
        Ipvm::EllipseEq32r ellipse;
        ellipse.m_x = object.CenterPoint().m_x;
        ellipse.m_y = object.CenterPoint().m_y;
        ellipse.m_xradius = object.Width() * 0.5f;
        ellipse.m_yradius = object.Height() * 0.5f;

        m_impl->m_coreView->ImageOverlayAdd(ellipse, color);
    }
}

void ImageLotView::Overlay_AddPolygon(const Ipvm::Point32r2* pointArray, long arraySize, COLORREF color)
{
    Ipvm::Polygon32r polygon;
    polygon.SetVertexNum(arraySize);
    for (long n = 0; n < arraySize; n++)
    {
        polygon.GetVertices()[n] = pointArray[n];
    }

    m_impl->m_coreView->ImageOverlayAdd(polygon, color);
}

void ImageLotView::Overlay_AddPolygon(const std::vector<Ipvm::Point32s2> points, COLORREF color)
{
    Ipvm::Polygon32r polygon;
    polygon.SetVertexNum(long(points.size()));
    for (long n = 0; n < long(points.size()); n++)
    {
        polygon.GetVertices()[n] = Ipvm::Conversion::ToPoint32r2(points[n]);
    }

    m_impl->m_coreView->ImageOverlayAdd(polygon, color);
}

void ImageLotView::Overlay_AddPoint(const Ipvm::Point32s2& point, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(Ipvm::Conversion::ToPoint32r2(point), color);
}

void ImageLotView::Overlay_AddPoint(const Ipvm::Point32r2& point, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(point, color);
}

void ImageLotView::Overlay_AddPoints(const std::vector<Ipvm::Point32s2>& points, COLORREF color)
{
    for (auto& point : points)
    {
        m_impl->m_coreView->ImageOverlayAdd(Ipvm::Conversion::ToPoint32r2(point), color);
    }
}

void ImageLotView::Overlay_AddPoints(const std::vector<Ipvm::Point32r2>& points, COLORREF color)
{
    for (auto& point : points)
    {
        m_impl->m_coreView->ImageOverlayAdd(point, color);
    }
}

void ImageLotView::Overlay_AddXPoint(const Ipvm::Point32s2& point, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(Ipvm::Conversion::ToPoint32r2(point), color);
}

void ImageLotView::Overlay_AddXPoint(const Ipvm::Point32r2& point, COLORREF color)
{
    m_impl->m_coreView->ImageOverlayAdd(
        Ipvm::LineSeg32r(point.m_x, point.m_y - 5.f, point.m_x, point.m_y + 5.f), color, 3.f);
    m_impl->m_coreView->ImageOverlayAdd(
        Ipvm::LineSeg32r(point.m_x - 5.f, point.m_y, point.m_x + 5.f, point.m_y), color, 3.f);
}

void ImageLotView::Overlay_AddText(const Ipvm::Point32s2& position, LPCTSTR text, COLORREF color, long nFontSize)
{
    m_impl->m_coreView->ImageOverlayAdd(position, text, color, CAST_FLOAT(nFontSize));
}

void ImageLotView::Overlay_AddText(const Ipvm::Point32r2& position, LPCTSTR text, COLORREF color, long nFontSize)
{
    m_impl->m_coreView->ImageOverlayAdd(Ipvm::Conversion::ToPoint32s2(position), text, color, CAST_FLOAT(nFontSize));
}

void ImageLotView::Overlay_RemoveAll()
{
    m_impl->m_coreView->ImageOverlayClear();
}

void ImageLotView::ROI_Show(BOOL show)
{
    if (show)
    {
        m_impl->m_coreView->ROIShow();
    }
    else
    {
        m_impl->m_coreView->ROIHide();
    }
}

void ImageLotView::ROI_Add(
    LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& rect, COLORREF color, BOOL isMovable, BOOL isResizable)
{
    m_impl->m_coreView->ROISet(key, name, rect, color, isMovable, isResizable, 100);
}

void ImageLotView::ROI_Add(
    LPCTSTR key, LPCTSTR name, const PI_RECT& rect, COLORREF color, BOOL isMovable, BOOL isResizable)
{
    Ipvm::Polygon32r polygon;
    polygon.SetVertexNum(4);
    polygon.GetVertices()[0].m_x = CAST_FLOAT(rect.ltX);
    polygon.GetVertices()[0].m_y = CAST_FLOAT(rect.ltY);
    polygon.GetVertices()[1].m_x = CAST_FLOAT(rect.rtX);
    polygon.GetVertices()[1].m_y = CAST_FLOAT(rect.rtY);
    polygon.GetVertices()[2].m_x = CAST_FLOAT(rect.brX);
    polygon.GetVertices()[2].m_y = CAST_FLOAT(rect.brY);
    polygon.GetVertices()[3].m_x = CAST_FLOAT(rect.blX);
    polygon.GetVertices()[3].m_y = CAST_FLOAT(rect.blY);

    m_impl->m_coreView->ROISet(key, name, polygon, color, isMovable, isResizable);
}

void ImageLotView::ROI_Add(LPCTSTR key, LPCTSTR name, const Ipvm::Point32s2 points[], long count, COLORREF color,
    BOOL isMovable, BOOL isResizable)
{
    Ipvm::Polygon32r polygon;
    polygon.SetVertexNum(count);

    for (long n = 0; n < count; n++)
    {
        polygon.GetVertices()[n] = Ipvm::Conversion::ToPoint32r2(points[n]);
    }

    m_impl->m_coreView->ROISet(key, name, polygon, color, isMovable, isResizable);
}

void ImageLotView::ROI_Add(LPCTSTR key, LPCTSTR name, const std::vector<Ipvm::Point32s2>& vecPoints, COLORREF color,
    BOOL isMovable, BOOL isResizable)
{
    Ipvm::Polygon32r polygon;
    polygon.SetVertexNum(long(vecPoints.size()));

    for (long n = 0; n < long(vecPoints.size()); n++)
    {
        polygon.GetVertices()[n] = Ipvm::Conversion::ToPoint32r2(vecPoints[n]);
    }

    m_impl->m_coreView->ROISet(key, name, polygon, color, isMovable, isResizable);
}

bool ImageLotView::ROI_Get(LPCTSTR key, Ipvm::Rect32s& rect)
{
    return m_impl->m_coreView->ROIGet(key, rect);
}

bool ImageLotView::ROI_Get(LPCTSTR key, PI_RECT& rect)
{
    Ipvm::Polygon32r polygon{};
    if (m_impl->m_coreView->ROIGet(key, polygon))
    {
        if (polygon.GetVertexNum() != 4)
            return false;

        rect.ltX = CAST_INT32T(polygon.GetVertices()[0].m_x);
        rect.ltY = CAST_INT32T(polygon.GetVertices()[0].m_y);
        rect.rtX = CAST_INT32T(polygon.GetVertices()[1].m_x);
        rect.rtY = CAST_INT32T(polygon.GetVertices()[1].m_y);
        rect.brX = CAST_INT32T(polygon.GetVertices()[2].m_x);
        rect.brY = CAST_INT32T(polygon.GetVertices()[2].m_y);
        rect.blX = CAST_INT32T(polygon.GetVertices()[3].m_x);
        rect.blY = CAST_INT32T(polygon.GetVertices()[3].m_y);

        return true;
    }

    return false;
}

bool ImageLotView::ROI_Get(LPCTSTR key, std::vector<Ipvm::Point32s2>& points)
{
    points.clear();
    Ipvm::Polygon32r polygon;
    if (m_impl->m_coreView->ROIGet(key, polygon))
    {
        ImageViewEx::PolygonToPoints(polygon, points);
        return true;
    }

    return false;
}

void ImageLotView::ROI_RemoveAll()
{
    m_impl->m_coreView->ROIClear();
}

// SDY Profile view를 닫기 위한 함수
void ImageLotView::CloseProfileView()
{
    m_impl->m_coreView->CloseProfileView();
}

void ImageLotView::SetSideVisionModule(const enSideVisionModule i_eSideVisionModule)
{
    m_impl->SetSideVisionModule(i_eSideVisionModule);
}

void ImageLotView::SetRawImageForStitchImageCombine() // Stitch Section Image Combine
{
    // RawImage Check Box를 먼저 TRUE로 만들어준다,
    m_impl->SetRawImageCheckBox(true);

    // RawImage Check 버튼을 클릭했을 때의 이벤트를 호출
    m_impl->OnBnClickedCheckRawImage();
}