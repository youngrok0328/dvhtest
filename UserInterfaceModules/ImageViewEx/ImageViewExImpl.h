#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Widget/Callback.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class DialogProfile;
struct PI_RECT;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
class ImageViewExImpl : public CDialogEx
{
    DECLARE_DYNAMIC(ImageViewExImpl)

public:
    ImageViewExImpl(const Ipvm::Rect32s& rtPoition, const long controlID, CWnd* pParent,
        const enSideVisionModule i_eSideVisionModule
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // standard constructor
    virtual ~ImageViewExImpl();

    void SetScale(const VisionScale& scale);

    void GetImageSize(Ipvm::Size32s2& size);
    const Ipvm::Image8u& GetImage_8u_C1();
    const Ipvm::Image8u3& GetImage_8u_C3();

    void SetImage(const Ipvm::Image8u& image);
    void SetImage(const Ipvm::Image8u3& image);
    void SetImage(const Ipvm::Image32r& image, const float noiseValue, const Ipvm::ColorMapIndex colorMap);
    void SetRawImage(const Ipvm::Image32r& image); // Tugu 20190312 ZMap 데이터를 디스플레이

    void ImageOverlayClear();
    void ImageOverlayAdd(const Ipvm::Point32r2& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const Ipvm::Rect32s& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const Ipvm::Rect32r& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const Ipvm::EllipseEq32r& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const PI_RECT& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const FPI_RECT& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const Ipvm::Quadrangle32r& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const Ipvm::LineSeg32r& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const Ipvm::Polygon32r& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, float fontSize);
    void ImageOverlayShow();
    void ImageOverlayHide();

    void ImageOverlayClear(const long layer);
    void ImageOverlayAdd(const long layer, const Ipvm::Rect32s& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const long layer, const Ipvm::LineSeg32r& object, COLORREF rgb, float fLineThickness);
    void ImageOverlayAdd(const long layer, const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, float fontSize);
    void ImageOverlayShow(const long layer);
    void ImageOverlayHide(const long layer);

    void NavigateTo(const Ipvm::Rect32s& roi);

    void ROIHide();
    void ROIShow();
    void ROIClear();
    void ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& roi, COLORREF rgb, const long fontSize = 10);
    void ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& roi, COLORREF rgb, const bool isMovable,
        const bool isResizable, const long fontSize = 10);
    void ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Polygon32r& roi, COLORREF rgb, const long fontSize);
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

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_IMAGEVIEWEX
    };
#endif

protected:
    VisionScale m_scale;

    class ExButton : public CBitmapButton
    {
    public:
        ExButton(UINT normalID, UINT selID, UINT disableID);
        ExButton(UINT normalID, UINT checkID, UINT selID, UINT disableID);

        void SetCheck(BOOL check);
        void EnableWindow(BOOL enable);
        BOOL GetCheck();

    private:
        UINT m_normalID;
        UINT m_checkID;
        UINT m_selID;
        UINT m_disableID;
        BOOL m_isActive;
        BOOL m_isCheck;

        void UpdateUI();
    };

    CComboBox m_cmb_unit;
    ExButton m_btn_colormap;
    ExButton m_btn_zoomfit;
    ExButton m_btn_histogram;
    long m_viewID;
    bool m_firstLoadImage;
    bool m_isProfileRefresh;
    Ipvm::Rect32s m_window_create;
    DialogProfile* m_profileView;

    void arrarymentUI();

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    bool m_displayMeasurement;
    Ipvm::LineSeg32r m_displayMeasurementLine;
    Ipvm::ImageView* m_view;
    Ipvm::Image8u m_display_8u_C1;
    Ipvm::Image32r m_RawImage; //Tugu 20190312 ZMap height 데이터 디스플레이
    Ipvm::Image8u3 m_display_8u_C3;

    virtual BOOL OnInitDialog();
    afx_msg void OnOK()
    {
    }
    afx_msg void OnCancel()
    {
    }
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnClickedBtnColorMap();
    afx_msg void OnClickedBtnZoomfit();
    afx_msg void OnClickedBtnHistogram();
    afx_msg void OnSelChangedUnit();
    afx_msg LRESULT OnProfileViewDead(WPARAM, LPARAM);

    void CloseProfileView(); //SDY ProfileView를 닫기 위한 함수

    static void callback_pixelInfo(void* userData, const int32_t id, const Ipvm::Point32r2& imagePoint,
        const uint32_t infoBufferLength, wchar_t* infoBuffer);
    static void callback_measurement_begin(void* userData, const int32_t id, const Ipvm::LineSeg32r& line);
    static void callback_measurement_end(void* userData, const int32_t id, const Ipvm::LineSeg32r& line);
    static void callback_measurement_change(void* userData, const int32_t id, const Ipvm::LineSeg32r& line);

    void callback_pixelInfo(
        const int32_t id, const Ipvm::Point32r2& pt, wchar_t* pixelInfo, const uint32_t bufferLength);
    void callback_measurement_begin(const int32_t id, const Ipvm::LineSeg32r& line);
    void callback_measurement_end(const int32_t id, const Ipvm::LineSeg32r& line);
    void callback_measurement_change(const int32_t id, const Ipvm::LineSeg32r& line);
};
