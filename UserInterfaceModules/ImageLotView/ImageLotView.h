#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class IllumInfo2D;
class InspectionAreaInfo;
class ImageViewEx;
class ImageLotViewImpl;
class VisionImageLot;
class VisionImageLotInsp;
class VisionProcessing;
class VisionScale;
class VisionUnitAgent;
struct PI_RECT;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
class DPI_IMAGE_LOT_VIEW_API ImageLotView
{
public:
    enum class Mode
    {
        LotRawAndInspImage,
        LotRawImage,
        LotInspImage,
        SpecifyImage,
    };

    static void PressSystemKey(WPARAM key);

    ImageLotView() = delete;
    ImageLotView(const ImageLotView& rhs) = delete;

    ImageLotView(const CRect& roi, VisionUnitAgent& unitItem, bool showInitialPaneArea, HWND hwndParent,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    ImageLotView(const CRect& roi, VisionProcessing& processingItem, bool enableLotInsp, bool showInitialPaneArea,
        HWND hwndParent, const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    // VisionImageLotInsp을 사용하여 보여주기 위한 생성자
    ImageLotView(const CRect& roi, VisionProcessing& processingItem, HWND hwndParent,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    virtual ~ImageLotView();

    HWND GetSafeHwnd();
    ImageViewEx* GetCoreView();

    void SetMode(Mode mode);
    void SetPaneWindowEnabled(bool enabled); // Pane Window 를 UI에서 건들지 못하게 한다
    void SetInspectionAreaInfo(const InspectionAreaInfo& inspectionAreaInfo);

    void SetMessage_RoiChanged(UINT message);
    void SetMessage_SelChangedPane(UINT message);
    void SetMessage_SelChangedRaw(UINT message);
    void SetMessage_SelChangedImage(UINT message);
    void SetTarget(VisionImageLot& imageLot, VisionImageLotInsp& imageLotInsp);
    void ShowImage(const long frameIndex, const bool showRaw = false, const bool bIsSrcRaw = false);
    void ShowCurrentImage(const bool showRaw = false);
    void ShowPrevImage(const bool showRaw = false);
    void ShowNextImage(const bool showRaw = false);

    void ShowPane(const long paneIndex, const bool showRaw = false);

    long GetCurrentImageFrame() const;

    void ZoomImageFit();
    void ZoomPaneFit(bool displayPaneROI);
    void NavigateTo(const Ipvm::Rect32s& roi);
    void SetScanIndexField(LPCTSTR text);
    void UpdateState();

    void OnJobChanged();
    bool IsShowRaw();

public: // Image handling
    void SetImage(const Ipvm::Image8u& image, LPCTSTR state = _T(""));
    void SetImage(const Ipvm::Image8u3& image, LPCTSTR state = _T(""));

public:
    // Overlay Show & Hide
    void Overlay_Show(BOOL show);

    // Overlay Append
    void Overlay_AddLine(const Ipvm::Point32s2& begin, const Ipvm::Point32s2& end, COLORREF color);
    void Overlay_AddLine(const Ipvm::Point32r2& begin, const Ipvm::Point32r2& end, COLORREF color);
    void Overlay_AddLine(
        const std::vector<Ipvm::Point32r2> begin, const std::vector<Ipvm::Point32r2> end, COLORREF color);
    void Overlay_AddRectangle(const Ipvm::Rect32r& rect, COLORREF color);
    void Overlay_AddRectangle(const Ipvm::Rect32s& rect, COLORREF color);
    void Overlay_AddRectangle(const PI_RECT& rect, COLORREF color);
    void Overlay_AddRectangle(const FPI_RECT& rect, COLORREF color);
    void Overlay_AddRectangles(const std::vector<Ipvm::Rect32s>& rects, COLORREF color);
    void Overlay_AddRectangles(const std::vector<Ipvm::Rect32r>& rects, COLORREF color);
    void Overlay_AddRectangles(const std::vector<FPI_RECT>& rects, COLORREF color);
    void Overlay_AddRectangles(const std::vector<Ipvm::Quadrangle32r>& rects, COLORREF color);
    void Overlay_AddEllipse(const Ipvm::Rect32r& rect, COLORREF color);
    void Overlay_AddEllipses(const std::vector<Ipvm::Rect32r>& rect, COLORREF color);
    void Overlay_AddPolygon(const Ipvm::Point32r2* pointArray, long arraySize, COLORREF color);
    void Overlay_AddPolygon(const std::vector<Ipvm::Point32s2> points, COLORREF color);
    void Overlay_AddPoint(const Ipvm::Point32s2& point, COLORREF color);
    void Overlay_AddPoint(const Ipvm::Point32r2& point, COLORREF color);
    void Overlay_AddPoints(const std::vector<Ipvm::Point32s2>& points, COLORREF color);
    void Overlay_AddPoints(const std::vector<Ipvm::Point32r2>& points, COLORREF color);
    void Overlay_AddXPoint(const Ipvm::Point32s2& point, COLORREF color);
    void Overlay_AddXPoint(const Ipvm::Point32r2& point, COLORREF color);
    void Overlay_AddText(const Ipvm::Point32s2& position, LPCTSTR text, COLORREF color, long nFontSize = 10);
    void Overlay_AddText(const Ipvm::Point32r2& position, LPCTSTR text, COLORREF color, long nFontSize = 10);

    // Overlay Clear
    void Overlay_RemoveAll();

public:
    // ROI Show & Hide
    void ROI_Show(BOOL show);

    // ROI Append
    void ROI_Add(
        LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& rect, COLORREF color, BOOL isMovable, BOOL isResizable);
    void ROI_Add(LPCTSTR key, LPCTSTR name, const PI_RECT& rect, COLORREF color, BOOL isMovable, BOOL isResizable);
    void ROI_Add(LPCTSTR key, LPCTSTR name, const Ipvm::Point32s2 points[], long count, COLORREF color, BOOL isMovable,
        BOOL isResizable);
    void ROI_Add(LPCTSTR key, LPCTSTR name, const std::vector<Ipvm::Point32s2>& vecPoints, COLORREF color,
        BOOL isMovable, BOOL isResizable);

    bool ROI_Get(LPCTSTR key, Ipvm::Rect32s& rect);
    bool ROI_Get(LPCTSTR key, PI_RECT& rect);
    bool ROI_Get(LPCTSTR key, std::vector<Ipvm::Point32s2>& points);

    // ROI Clear
    void ROI_RemoveAll();

    void CloseProfileView(); //SDY ProfileView를 닫기 위한 함수

    void SetSideVisionModule(const enSideVisionModule i_eSideVisionModule);

    void SetRawImageForStitchImageCombine(); // Stitch Section Image Combine

private:
    ImageLotViewImpl* m_impl;
};