#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Widget/Callback.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageViewExImpl;
class VisionScale;
struct PI_RECT;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
class DPI_IMAGE_LOT_VIEW_API ImageViewEx
{
public:
    ImageViewEx(HWND hwndParent, const Ipvm::Rect32s& rtPoition, const long controlID = 0,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    ~ImageViewEx();

    void SetScale(const VisionScale& scale);

    static void RectToEllipse(const Ipvm::Rect32s& source, Ipvm::EllipseEq32r& dst);
    static void RectToEllipse(const Ipvm::Rect32r& source, Ipvm::EllipseEq32r& dst);
    static void PointsToPolygon(const std::vector<Ipvm::Point32r2>& source, Ipvm::Polygon32r& dst);
    static void PointsToPolygon(const std::vector<Ipvm::Point32s2>& source, Ipvm::Polygon32r& dst);
    static void PolygonToPoints(const Ipvm::Polygon32r& source, std::vector<Ipvm::Point32r2>& dst);
    static void PolygonToPoints(const Ipvm::Polygon32r& source, std::vector<Ipvm::Point32s2>& dst);

    HWND GetSafeHwnd();
    void GetImageSize(Ipvm::Size32s2& size);
    const Ipvm::Image8u& GetImage_8u_C1();
    const Ipvm::Image8u3& GetImage_8u_C3();

    void SetImage(const Ipvm::Image8u& image);
    void SetImage(const Ipvm::Image8u3& image);
    void SetImage(const Ipvm::Image32r& image, const float noiseValue, const Ipvm::ColorMapIndex colorMap);
    void SetRawImage(const Ipvm::Image32r& image); // Tugu 20190312 - 실제 Height를 디스플레이용 ZMap 전달

    void CloseProfileView(); //SDY ProfileView를 닫기 위한 함수

    void ImageOverlayClear();
    void ImageOverlayAdd(const Ipvm::Point32r2& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const Ipvm::Rect32s& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const Ipvm::Rect32r& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const Ipvm::EllipseEq32r& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const PI_RECT& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const FPI_RECT& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const Ipvm::Quadrangle32r& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const Ipvm::LineSeg32r& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const Ipvm::Polygon32r& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, float fontSize = 8);
    void ImageOverlayShow();
    void ImageOverlayHide();

    void ImageOverlayClear(const long layer);
    void ImageOverlayAdd(const long layer, const Ipvm::Rect32s& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const long layer, const Ipvm::LineSeg32r& object, COLORREF rgb, float fLineThickness = 1.f);
    void ImageOverlayAdd(const long layer, const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, float fontSize = 8);
    void ImageOverlayShow(const long layer);
    void ImageOverlayHide(const long layer);

    void NavigateTo(const Ipvm::Rect32s& roi);

    void ROIHide();
    void ROIShow();
    void ROIClear();
    void ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& roi, COLORREF rgb, const long fontSize = 10);
    void ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& roi, COLORREF rgb, const bool isMovable,
        const bool isResizable, const long fontSize = 10);
    void ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Polygon32r& roi, COLORREF rgb, const long fontSize = 10);
    void ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Polygon32r& roi, COLORREF rgb, const bool isMovable,
        const bool isResizable, const long fontSize = 10);
    bool ROIGet(LPCTSTR key, Ipvm::Rect32s& roi);
    bool ROIGet(LPCTSTR key, Ipvm::Polygon32r& roi);

    void WindowOverlayClear();
    void WindowOverlayAdd(const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, const long fontSize = 8);
    void WindowOverlayShow();

    void RegisterCallback_ROIChangeBegin(HWND owner, void* userData, Ipvm::ImageViewRoiCallback callback);
    void RegisterCallback_ROIChanging(HWND owner, void* userData, Ipvm::ImageViewRoiCallback callback);
    void RegisterCallback_ROIChangeEnd(HWND owner, void* userData, Ipvm::ImageViewRoiCallback callback);

    void RegisterCallback_MouseLButtonDown(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback);
    void RegisterCallback_MouseLButtonUp(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback);
    void RegisterCallback_MouseMButtonDown(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback);
    void RegisterCallback_MouseMButtonUp(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback);
    void RegisterCallback_MouseRButtonDown(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback);
    void RegisterCallback_MouseRButtonUp(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback);
    void RegisterCallback_MouseMove(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback);

private:
    ImageViewExImpl* m_impl;
};
