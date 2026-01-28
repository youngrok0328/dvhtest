//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ImageViewEx.h"

//CPP_2_________________________________ This project's headers
#include "ImageViewExImpl.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ImageViewEx::ImageViewEx(
    HWND hwndParent, const Ipvm::Rect32s& rtPoition, const long controlID, const enSideVisionModule i_eSideVisionModule)
    : m_impl(nullptr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_impl = new ImageViewExImpl(rtPoition, controlID, CWnd::FromHandle(hwndParent), i_eSideVisionModule);
    m_impl->Create(IDD_IMAGEVIEWEX, CWnd::FromHandle(hwndParent));
}

ImageViewEx::~ImageViewEx()
{
    delete m_impl;
}

void ImageViewEx::SetScale(const VisionScale& scale)
{
    m_impl->SetScale(scale);
}

void ImageViewEx::RectToEllipse(const Ipvm::Rect32s& source, Ipvm::EllipseEq32r& dst)
{
    dst.m_x = (source.m_left + source.m_right) * 0.5f;
    dst.m_y = (source.m_top + source.m_bottom) * 0.5f;
    dst.m_xradius = source.Width() * 0.5f;
    dst.m_yradius = source.Height() * 0.5f;
}

void ImageViewEx::RectToEllipse(const Ipvm::Rect32r& source, Ipvm::EllipseEq32r& dst)
{
    dst.m_x = (source.m_left + source.m_right) * 0.5f;
    dst.m_y = (source.m_top + source.m_bottom) * 0.5f;
    dst.m_xradius = source.Width() * 0.5f;
    dst.m_yradius = source.Height() * 0.5f;
}

void ImageViewEx::PointsToPolygon(const std::vector<Ipvm::Point32r2>& source, Ipvm::Polygon32r& dst)
{
    dst.SetVertexNum(long(source.size()));
    for (long n = 0; n < long(source.size()); n++)
    {
        dst.GetVertices()[n] = source[n];
    }
}

void ImageViewEx::PointsToPolygon(const std::vector<Ipvm::Point32s2>& source, Ipvm::Polygon32r& dst)
{
    dst.SetVertexNum(long(source.size()));
    for (long n = 0; n < long(source.size()); n++)
    {
        dst.GetVertices()[n] = Ipvm::Conversion::ToPoint32r2(source[n]);
    }
}

void ImageViewEx::PolygonToPoints(const Ipvm::Polygon32r& source, std::vector<Ipvm::Point32r2>& dst)
{
    dst.resize(source.GetVertexNum());
    for (long n = 0; n < long(dst.size()); n++)
    {
        dst[n] = source.GetVertices()[n];
    }
}

void ImageViewEx::PolygonToPoints(const Ipvm::Polygon32r& source, std::vector<Ipvm::Point32s2>& dst)
{
    dst.resize(source.GetVertexNum());
    for (long n = 0; n < long(dst.size()); n++)
    {
        dst[n] = Ipvm::Conversion::ToPoint32s2(source.GetVertices()[n]);
    }
}

HWND ImageViewEx::GetSafeHwnd()
{
    return m_impl->GetSafeHwnd();
}

void ImageViewEx::GetImageSize(Ipvm::Size32s2& size)
{
    m_impl->GetImageSize(size);
}

const Ipvm::Image8u& ImageViewEx::GetImage_8u_C1()
{
    return m_impl->GetImage_8u_C1();
}

const Ipvm::Image8u3& ImageViewEx::GetImage_8u_C3()
{
    return m_impl->GetImage_8u_C3();
}

void ImageViewEx::SetImage(const Ipvm::Image8u& image)
{
    m_impl->SetImage(image);
}

void ImageViewEx::SetImage(const Ipvm::Image8u3& image)
{
    m_impl->SetImage(image);
}

void ImageViewEx::SetImage(const Ipvm::Image32r& image, const float noiseValue, const Ipvm::ColorMapIndex colorMap)
{
    m_impl->SetImage(image, noiseValue, colorMap);
}

void ImageViewEx::SetRawImage(const Ipvm::Image32r& image)
{
    m_impl->SetRawImage(image);
}

// SDY Profile view를 닫기 위한 함수
void ImageViewEx::CloseProfileView()
{
    m_impl->CloseProfileView();
}

void ImageViewEx::ImageOverlayClear()
{
    m_impl->ImageOverlayClear();
}

void ImageViewEx::ImageOverlayAdd(const Ipvm::Rect32s& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const Ipvm::Rect32r& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const Ipvm::EllipseEq32r& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const PI_RECT& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const FPI_RECT& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const Ipvm::Quadrangle32r& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const Ipvm::Point32r2& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const Ipvm::LineSeg32r& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const Ipvm::Polygon32r& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, float fontSize)
{
    m_impl->ImageOverlayAdd(pos, text, rgb, fontSize);
}

void ImageViewEx::ImageOverlayShow()
{
    m_impl->ImageOverlayShow();
}

void ImageViewEx::ImageOverlayHide()
{
    m_impl->ImageOverlayHide();
}

void ImageViewEx::ImageOverlayClear(const long layer)
{
    m_impl->ImageOverlayClear(layer);
}

void ImageViewEx::ImageOverlayAdd(const long layer, const Ipvm::Rect32s& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(layer, object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(const long layer, const Ipvm::LineSeg32r& object, COLORREF rgb, float fLineThickness)
{
    m_impl->ImageOverlayAdd(layer, object, rgb, fLineThickness);
}

void ImageViewEx::ImageOverlayAdd(
    const long layer, const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, float fontSize)
{
    m_impl->ImageOverlayAdd(layer, pos, text, rgb, fontSize);
}

void ImageViewEx::ImageOverlayShow(const long layer)
{
    m_impl->ImageOverlayShow(layer);
}

void ImageViewEx::ImageOverlayHide(const long layer)
{
    m_impl->ImageOverlayHide(layer);
}

void ImageViewEx::NavigateTo(const Ipvm::Rect32s& roi)
{
    m_impl->NavigateTo(roi);
}

void ImageViewEx::ROIHide()
{
    m_impl->ROIHide();
}

void ImageViewEx::ROIShow()
{
    m_impl->ROIShow();
}

void ImageViewEx::ROIClear()
{
    m_impl->ROIClear();
}

void ImageViewEx::ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& roi, COLORREF rgb, const long fontSize)
{
    m_impl->ROISet(key, name, roi, rgb, fontSize);
}

void ImageViewEx::ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& roi, COLORREF rgb, const bool isMovable,
    const bool isResizable, const long fontSize)
{
    m_impl->ROISet(key, name, roi, rgb, isMovable, isResizable, fontSize);
}

void ImageViewEx::ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Polygon32r& roi, COLORREF rgb, const long fontSize)
{
    m_impl->ROISet(key, name, roi, rgb, fontSize);
}

void ImageViewEx::ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Polygon32r& roi, COLORREF rgb, const bool isMovable,
    const bool isResizable, const long fontSize)
{
    m_impl->ROISet(key, name, roi, rgb, isMovable, isResizable, fontSize);
}

bool ImageViewEx::ROIGet(LPCTSTR key, Ipvm::Rect32s& roi)
{
    return m_impl->ROIGet(key, roi);
}

bool ImageViewEx::ROIGet(LPCTSTR key, Ipvm::Polygon32r& roi)
{
    return m_impl->ROIGet(key, roi);
}

void ImageViewEx::WindowOverlayClear()
{
    m_impl->WindowOverlayClear();
}

void ImageViewEx::WindowOverlayAdd(const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, const long fontSize)
{
    m_impl->WindowOverlayAdd(pos, text, rgb, fontSize);
}

void ImageViewEx::WindowOverlayShow()
{
    m_impl->WindowOverlayShow();
}

void ImageViewEx::RegisterCallback_ROIChangeBegin(HWND owner, void* userData, Ipvm::ImageViewRoiCallback callback)
{
    m_impl->RegisterCallback_ROIChangeBegin(owner, userData, callback);
}

void ImageViewEx::RegisterCallback_ROIChanging(HWND owner, void* userData, Ipvm::ImageViewRoiCallback callback)
{
    m_impl->RegisterCallback_ROIChanging(owner, userData, callback);
}

void ImageViewEx::RegisterCallback_ROIChangeEnd(HWND owner, void* userData, Ipvm::ImageViewRoiCallback callback)
{
    m_impl->RegisterCallback_ROIChangeEnd(owner, userData, callback);
}

void ImageViewEx::RegisterCallback_MouseLButtonDown(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_impl->RegisterCallback_MouseLButtonDown(owner, userData, callback);
}

void ImageViewEx::RegisterCallback_MouseLButtonUp(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_impl->RegisterCallback_MouseLButtonUp(owner, userData, callback);
}

void ImageViewEx::RegisterCallback_MouseMButtonDown(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_impl->RegisterCallback_MouseMButtonDown(owner, userData, callback);
}

void ImageViewEx::RegisterCallback_MouseMButtonUp(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_impl->RegisterCallback_MouseMButtonUp(owner, userData, callback);
}

void ImageViewEx::RegisterCallback_MouseRButtonDown(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_impl->RegisterCallback_MouseRButtonDown(owner, userData, callback);
}

void ImageViewEx::RegisterCallback_MouseRButtonUp(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_impl->RegisterCallback_MouseRButtonUp(owner, userData, callback);
}

void ImageViewEx::RegisterCallback_MouseMove(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_impl->RegisterCallback_MouseMove(owner, userData, callback);
}
