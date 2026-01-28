//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ImageViewExImpl.h"

//CPP_2_________________________________ This project's headers
#include "DialogProfile.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Widget/ImageView.h>
#include <afxdialogex.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TOOLBAR_WIDTH (40)
#define BTN_SIZE (32)
#define IDC_CMB_UNIT (9000)
#define IDC_BTN_ID_COLORMAP (9002)
#define IDC_BTN_ID_ZOOMFIT (9004)
#define IDC_BTN_ID_HISTOGRAM (9006)

//CPP_7_________________________________ Implementation body
//
ImageViewExImpl::ExButton::ExButton(UINT normalID, UINT selID, UINT disableID)
    : m_isActive(TRUE)
    , m_normalID(normalID)
    , m_checkID(0)
    , m_selID(selID)
    , m_disableID(disableID)
    , m_isCheck(FALSE)
{
    LoadBitmaps(m_normalID, m_selID);
}

ImageViewExImpl::ExButton::ExButton(UINT normalID, UINT checkID, UINT selID, UINT disableID)
    : m_isActive(TRUE)
    , m_normalID(normalID)
    , m_checkID(checkID)
    , m_selID(selID)
    , m_disableID(disableID)
    , m_isCheck(FALSE)
{
    LoadBitmaps(m_normalID, m_selID);
}

void ImageViewExImpl::ExButton::SetCheck(BOOL check)
{
    if (m_isCheck == check)
        return;

    m_isCheck = check;
    UpdateUI();
}

void ImageViewExImpl::ExButton::EnableWindow(BOOL enable)
{
    if (m_isActive == enable)
        return;

    m_isActive = enable;
    if (!m_isActive)
        m_isCheck = FALSE;

    UpdateUI();
}

BOOL ImageViewExImpl::ExButton::GetCheck()
{
    return m_isCheck;
}

void ImageViewExImpl::ExButton::UpdateUI()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_isActive)
    {
        if (m_isCheck)
        {
            LoadBitmaps(m_checkID, m_selID);
        }
        else
        {
            LoadBitmaps(m_normalID, m_selID);
        }
    }
    else
    {
        LoadBitmaps(m_disableID, m_disableID);
    }

    Invalidate(FALSE);
}

IMPLEMENT_DYNAMIC(ImageViewExImpl, CDialogEx)

ImageViewExImpl::ImageViewExImpl(const Ipvm::Rect32s& rtPoition, const long controlID, CWnd* pParent /*=nullptr*/,
    const enSideVisionModule i_eSideVisionModule)
    : CDialogEx(IDD_IMAGEVIEWEX, pParent)
    , m_view(nullptr)
    , m_viewID(controlID)
    , m_window_create(rtPoition)
    , m_profileView(nullptr)
    , m_btn_colormap(IDB_BUTTON_COLORMAP_DOWN, IDB_BUTTON_COLORMAP_ING, IDB_BUTTON_COLORMAP_UP, IDB_BUTTON_EMPTY)
    , m_btn_zoomfit(IDB_BUTTON_ZOOMFIT_DOWN, IDB_BUTTON_ZOOMFIT_UP, IDB_BUTTON_EMPTY)
    , m_btn_histogram(IDB_BUTTON_HISTO_DOWN, IDB_BUTTON_HISTO_DOWN_ING, IDB_BUTTON_HISTO_UP, IDB_BUTTON_EMPTY)
    , m_firstLoadImage(true)
    , m_displayMeasurement(false)
    , m_isProfileRefresh(true)
{
    m_scale = SystemConfig::GetInstance().GetScale(i_eSideVisionModule);
}

ImageViewExImpl::~ImageViewExImpl()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    delete m_profileView;
    delete m_view;
}

void ImageViewExImpl::SetScale(const VisionScale& scale)
{
    m_scale = scale;
}

void ImageViewExImpl::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ImageViewExImpl, CDialogEx)
ON_WM_SIZE()
ON_COMMAND(IDC_BTN_ID_COLORMAP, OnClickedBtnColorMap)
ON_COMMAND(IDC_BTN_ID_ZOOMFIT, OnClickedBtnZoomfit)
ON_COMMAND(IDC_BTN_ID_HISTOGRAM, OnClickedBtnHistogram)
ON_CBN_SELCHANGE(IDC_CMB_UNIT, OnSelChangedUnit)
ON_MESSAGE(UM_PROFILEVIEW_DEAD, OnProfileViewDead)
END_MESSAGE_MAP()

// ImageViewExImpl message handlers

BOOL ImageViewExImpl::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_view = new Ipvm::ImageView(GetSafeHwnd(), Ipvm::Rect32s(0, 0, 0, 0), m_viewID);
    m_view->RegisterPixelInfoCallback(GetSafeHwnd(), this, callback_pixelInfo);
    m_view->RegisterMeasurementBeginCallback(GetSafeHwnd(), this, callback_measurement_begin);
    m_view->RegisterMeasurementEndCallback(GetSafeHwnd(), this, callback_measurement_end);
    m_view->RegisterMeasurementChangeEndCallback(GetSafeHwnd(), this, callback_measurement_change);

    m_cmb_unit.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, CRect(0, 0, 0, 0), this, IDC_CMB_UNIT);
    m_cmb_unit.AddString(_T("px"));
    m_cmb_unit.AddString(_T("um"));
    m_cmb_unit.AddString(_T("mil"));
    m_cmb_unit.SetCurSel(0);

    m_btn_colormap.Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, CRect(0, 0, 0, 0), this, IDC_BTN_ID_COLORMAP);
    m_btn_zoomfit.Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, CRect(0, 0, 0, 0), this, IDC_BTN_ID_ZOOMFIT);
    m_btn_histogram.Create(NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, CRect(0, 0, 0, 0), this, IDC_BTN_ID_HISTOGRAM);

    m_btn_colormap.EnableWindow(FALSE);
    m_btn_histogram.EnableWindow(FALSE);

    MoveWindow(Ipvm::ToMFC(m_window_create));
    arrarymentUI();
    OnSelChangedUnit();

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void ImageViewExImpl::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_view)
    {
        arrarymentUI();
    }
}

void ImageViewExImpl::OnClickedBtnColorMap()
{
    m_btn_colormap.SetCheck(!m_btn_colormap.GetCheck());

    if (m_btn_colormap.GetCheck())
    {
        m_view->SetImage(m_display_8u_C1, Ipvm::ColorMapIndex::e_rainbow, true);
    }
    else
    {
        m_view->SetImage(m_display_8u_C1);
    }
}

void ImageViewExImpl::OnClickedBtnZoomfit()
{
    m_view->ZoomImageFit();
}

void ImageViewExImpl::OnClickedBtnHistogram()
{
    if (m_btn_histogram.GetCheck())
    {
        m_btn_histogram.SetCheck(FALSE);
        m_view->HideMeasurement();
    }
    else
    {
        m_btn_histogram.SetCheck(TRUE);

        if (!m_displayMeasurement)
        {
            Ipvm::Rect32r area;
            m_view->GetDrawingArea(area);

            float lineHalfX = (area.Width()) * 0.2f;
            float lineHalfY = (area.Height()) * 0.2f;

            m_displayMeasurementLine.m_sx = area.CenterPoint().m_x - lineHalfX;
            m_displayMeasurementLine.m_sy = area.CenterPoint().m_y - lineHalfY;
            m_displayMeasurementLine.m_ex = area.CenterPoint().m_x + lineHalfX;
            m_displayMeasurementLine.m_ey = area.CenterPoint().m_y + lineHalfY;

            m_view->ShowMeasurement(m_displayMeasurementLine);
        }
        else
        {
            // 이미 측정중이므로 Profile View만 만들어 띄워주자
            if (m_profileView == nullptr)
            {
                AFX_MANAGE_STATE(AfxGetStaticModuleState());

                m_profileView = new DialogProfile(this);
                m_profileView->Create(IDD_DIALOG_PROFILE, this);
                m_profileView->ShowWindow(SW_SHOW);
                m_profileView->UpdateProfile(m_displayMeasurementLine);
            }
        }
    }
}

void ImageViewExImpl::GetImageSize(Ipvm::Size32s2& size)
{
    m_view->GetImageSize(size);
}

const Ipvm::Image8u& ImageViewExImpl::GetImage_8u_C1()
{
    return m_display_8u_C1;
}

const Ipvm::Image8u3& ImageViewExImpl::GetImage_8u_C3()
{
    return m_display_8u_C3;
}

void ImageViewExImpl::SetImage(const Ipvm::Image8u& image)
{
    m_display_8u_C3.Free();
    //delete m_profileView;
    //m_profileView = nullptr;

    m_display_8u_C1 = image;

    m_btn_colormap.EnableWindow(TRUE);
    m_btn_histogram.EnableWindow(TRUE);

    if (m_firstLoadImage)
    {
        if (SystemConfig::GetInstance().m_nVisionType == VISIONTYPE_3D_INSP)
        {
            m_btn_colormap.SetCheck(TRUE);
        }
    }

    if (m_btn_colormap.GetCheck())
    {
        m_view->SetImage(image, Ipvm::ColorMapIndex::e_rainbow, true);
    }
    else
    {
        m_view->SetImage(image);
    }

    if (m_profileView && m_isProfileRefresh)
    {
        m_profileView->UpdateProfile();
        m_isProfileRefresh = false;
    }
    else
    {
        m_isProfileRefresh = true;
    }

    m_firstLoadImage = false;
}

void ImageViewExImpl::SetImage(const Ipvm::Image8u3& image)
{
    m_display_8u_C1.Free();
    //delete m_profileView;
    //m_profileView = nullptr;

    m_display_8u_C3 = image;

    m_view->SetImage(image);

    m_btn_colormap.EnableWindow(FALSE);
    m_btn_histogram.EnableWindow(FALSE);

    if (m_profileView && m_isProfileRefresh)
    {
        m_profileView->UpdateProfile();
        m_isProfileRefresh = false;
    }
    else
    {
        m_isProfileRefresh = true;
    }

    m_firstLoadImage = false;
}

void ImageViewExImpl::SetImage(const Ipvm::Image32r& image, const float noiseValue, const Ipvm::ColorMapIndex colorMap)
{
    m_display_8u_C1.Free();
    m_display_8u_C3.Free();
    //delete m_profileView;
    //m_profileView = nullptr;

    m_view->SetImage(image, noiseValue, colorMap);

    m_btn_colormap.EnableWindow(FALSE);
    m_btn_histogram.EnableWindow(FALSE);

    if (m_profileView && m_isProfileRefresh)
    {
        m_profileView->UpdateProfile();
        m_isProfileRefresh = false;
    }
    else
    {
        m_isProfileRefresh = true;
    }

    m_firstLoadImage = false;
}

// Tugu 20190312
void ImageViewExImpl::SetRawImage(const Ipvm::Image32r& image)
{
    m_RawImage.Create(image.GetSizeX(), image.GetSizeY());
    Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), m_RawImage);
}

void ImageViewExImpl::ImageOverlayClear()
{
    m_view->ClearImageOverlay();
}

void ImageViewExImpl::ImageOverlayAdd(const Ipvm::Point32r2& object, COLORREF rgb, float fLineThickness)
{
    m_view->AddImageOverlay(object, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(const Ipvm::Rect32s& object, COLORREF rgb, float fLineThickness)
{
    m_view->AddImageOverlay(object, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(const Ipvm::Rect32r& object, COLORREF rgb, float fLineThickness)
{
    m_view->AddImageOverlay(object, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(const Ipvm::EllipseEq32r& object, COLORREF rgb, float fLineThickness)
{
    m_view->AddImageOverlay(object, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(const Ipvm::Quadrangle32r& object, COLORREF rgb, float fLineThickness)
{
    m_view->AddImageOverlay(object, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(const PI_RECT& object, COLORREF rgb, float fLineThickness)
{
    Ipvm::Quadrangle32r quad;

    quad.m_ltX = CAST_FLOAT(object.ltX);
    quad.m_ltY = CAST_FLOAT(object.ltY);
    quad.m_rtX = CAST_FLOAT(object.rtX);
    quad.m_rtY = CAST_FLOAT(object.rtY);
    quad.m_lbX = CAST_FLOAT(object.blX);
    quad.m_lbY = CAST_FLOAT(object.blY);
    quad.m_rbX = CAST_FLOAT(object.brX);
    quad.m_rbY = CAST_FLOAT(object.brY);

    m_view->AddImageOverlay(quad, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(const FPI_RECT& object, COLORREF rgb, float fLineThickness)
{
    Ipvm::Quadrangle32r quad;

    quad.m_ltX = object.fptLT.m_x;
    quad.m_ltY = object.fptLT.m_y;
    quad.m_rtX = object.fptRT.m_x;
    quad.m_rtY = object.fptRT.m_y;
    quad.m_lbX = object.fptLB.m_x;
    quad.m_lbY = object.fptLB.m_y;
    quad.m_rbX = object.fptRB.m_x;
    quad.m_rbY = object.fptRB.m_y;

    m_view->AddImageOverlay(quad, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(const Ipvm::LineSeg32r& object, COLORREF rgb, float fLineThickness)
{
    m_view->AddImageOverlay(object, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(const Ipvm::Polygon32r& object, COLORREF rgb, float fLineThickness)
{
    m_view->AddImageOverlay(object, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, float fontSize)
{
    m_view->AddImageOverlay(pos, text, rgb, CAST_INT32T(fontSize));
}

void ImageViewExImpl::ImageOverlayShow()
{
    m_view->ShowImageOverlay();
}

void ImageViewExImpl::ImageOverlayHide()
{
    m_view->HideImageOverlay();
}

void ImageViewExImpl::ImageOverlayClear(const long layer)
{
    m_view->ClearImageOverlay(layer);
}

void ImageViewExImpl::ImageOverlayAdd(const long layer, const Ipvm::Rect32s& object, COLORREF rgb, float fLineThickness)
{
    m_view->AddImageOverlay(layer, object, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(
    const long layer, const Ipvm::LineSeg32r& object, COLORREF rgb, float fLineThickness)
{
    m_view->AddImageOverlay(layer, object, rgb, fLineThickness);
}

void ImageViewExImpl::ImageOverlayAdd(
    const long layer, const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, float fontSize)
{
    m_view->AddImageOverlay(layer, pos, text, rgb, CAST_INT32T(fontSize));
}

void ImageViewExImpl::ImageOverlayShow(const long layer)
{
    m_view->ShowImageOverlay(layer);
}

void ImageViewExImpl::ImageOverlayHide(const long layer)
{
    m_view->HideImageOverlay(layer);
}

void ImageViewExImpl::NavigateTo(const Ipvm::Rect32s& roi)
{
    m_view->NavigateTo(roi);
}

void ImageViewExImpl::ROIHide()
{
    m_view->HideRoi();
}

void ImageViewExImpl::ROIShow()
{
    m_view->ShowRoi();
}

void ImageViewExImpl::ROIClear()
{
    m_view->ClearRoi();
}

void ImageViewExImpl::ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& roi, COLORREF rgb, const long fontSize)
{
    m_view->SetRoi(key, name, roi, rgb, fontSize);
}

void ImageViewExImpl::ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Rect32s& roi, COLORREF rgb, const bool isMovable,
    const bool isResizable, const long fontSize)
{
    m_view->SetRoi(key, name, roi, rgb, isMovable, isResizable, fontSize);
}

void ImageViewExImpl::ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Polygon32r& roi, COLORREF rgb, const long fontSize)
{
    m_view->SetRoi(key, name, roi, rgb, fontSize);
}

void ImageViewExImpl::ROISet(LPCTSTR key, LPCTSTR name, const Ipvm::Polygon32r& roi, COLORREF rgb, const bool isMovable,
    const bool isResizable, const long fontSize)
{
    m_view->SetRoi(key, name, roi, rgb, isMovable, isResizable, fontSize);
}

bool ImageViewExImpl::ROIGet(LPCTSTR key, Ipvm::Rect32s& roi)
{
    return m_view->GetRoi(key, roi);
}

bool ImageViewExImpl::ROIGet(LPCTSTR key, Ipvm::Polygon32r& roi)
{
    return m_view->GetRoi(key, roi);
}

void ImageViewExImpl::WindowOverlayClear()
{
    m_view->ClearWindowOverlay();
}

void ImageViewExImpl::WindowOverlayAdd(const Ipvm::Point32s2& pos, LPCTSTR text, COLORREF rgb, const long fontSize)
{
    m_view->AddWindowOverlay(pos, text, rgb, fontSize);
}

void ImageViewExImpl::WindowOverlayShow()
{
    m_view->ShowWindowOverlay();
}

void ImageViewExImpl::RegisterCallback_ROIChangeBegin(HWND owner, void* userData, Ipvm::ImageViewRoiCallback callback)
{
    m_view->RegisterRoiChangeBeginCallback(owner, userData, callback);
}

void ImageViewExImpl::RegisterCallback_ROIChanging(HWND owner, void* userData, Ipvm::ImageViewRoiCallback callback)
{
    m_view->RegisterRoiChangingCallback(owner, userData, callback);
}

void ImageViewExImpl::RegisterCallback_ROIChangeEnd(HWND owner, void* userData, Ipvm::ImageViewRoiCallback callback)
{
    m_view->RegisterRoiChangeEndCallback(owner, userData, callback);
}

void ImageViewExImpl::RegisterCallback_MouseLButtonDown(
    HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_view->RegisterMouseLButtonDownCallback(owner, userData, callback);
}

void ImageViewExImpl::RegisterCallback_MouseLButtonUp(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_view->RegisterMouseLButtonUpCallback(owner, userData, callback);
}

void ImageViewExImpl::RegisterCallback_MouseMButtonDown(
    HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_view->RegisterMouseMButtonDownCallback(owner, userData, callback);
}

void ImageViewExImpl::RegisterCallback_MouseMButtonUp(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_view->RegisterMouseMButtonUpCallback(owner, userData, callback);
}

void ImageViewExImpl::RegisterCallback_MouseRButtonDown(
    HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_view->RegisterMouseRButtonDownCallback(owner, userData, callback);
}

void ImageViewExImpl::RegisterCallback_MouseRButtonUp(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_view->RegisterMouseRButtonUpCallback(owner, userData, callback);
}

void ImageViewExImpl::RegisterCallback_MouseMove(HWND owner, void* userData, Ipvm::ImageViewMouseCallback callback)
{
    m_view->RegisterMouseMoveCallback(owner, userData, callback);
}

void ImageViewExImpl::arrarymentUI()
{
    if (m_view == nullptr)
    {
        ASSERT(!_T("??"));
        return;
    }
    CRect rectWindow;
    GetClientRect(rectWindow);

    long btnStartX = rectWindow.Width() - TOOLBAR_WIDTH + (TOOLBAR_WIDTH - BTN_SIZE) / 2;

    m_cmb_unit.MoveWindow(rectWindow.Width() - TOOLBAR_WIDTH, 0, TOOLBAR_WIDTH, 1000);
    m_btn_colormap.MoveWindow(btnStartX, 30 + (BTN_SIZE + 2) * 0, BTN_SIZE, BTN_SIZE);
    m_btn_zoomfit.MoveWindow(btnStartX, 30 + (BTN_SIZE + 2) * 1, BTN_SIZE, BTN_SIZE);
    m_btn_histogram.MoveWindow(btnStartX, 30 + (BTN_SIZE + 2) * 2, BTN_SIZE, BTN_SIZE);

    ::MoveWindow(m_view->GetSafeHwnd(), 0, 0, rectWindow.Width() - TOOLBAR_WIDTH - 5, rectWindow.Height(), TRUE);
}

void ImageViewExImpl::OnSelChangedUnit()
{
    switch (m_cmb_unit.GetCurSel())
    {
        case 0:
            m_view->SetMeasurementScale(0.f, 0.f, nullptr);
            break;

        case 1:
            m_view->SetMeasurementScale(m_scale.pixelToUm().m_x, m_scale.pixelToUm().m_y, _T("um"));
            break;

        case 2:
            m_view->SetMeasurementScale(m_scale.pixelToMil().m_x, m_scale.pixelToMil().m_y, _T("mil"));
            break;
    }
}

LRESULT ImageViewExImpl::OnProfileViewDead(WPARAM, LPARAM)
{
    delete m_profileView;
    m_profileView = nullptr;

    m_btn_histogram.SetCheck(FALSE);
    m_view->HideMeasurement();

    return 0;
}

// SDY Profile view를 닫기 위한 함수
void ImageViewExImpl::CloseProfileView()
{
    if (m_btn_histogram.GetCheck())
    {
        m_btn_histogram.SetCheck(FALSE);
        m_view->HideMeasurement();
    }
}

void ImageViewExImpl::callback_pixelInfo(void* userData, const int32_t id, const Ipvm::Point32r2& imagePoint,
    const uint32_t infoBufferLength, wchar_t* infoBuffer)
{
    ((ImageViewExImpl*)userData)->callback_pixelInfo(id, imagePoint, infoBuffer, infoBufferLength);
}

void ImageViewExImpl::callback_measurement_begin(void* userData, const int32_t id, const Ipvm::LineSeg32r& line)
{
    ((ImageViewExImpl*)userData)->callback_measurement_begin(id, line);
}

void ImageViewExImpl::callback_measurement_end(void* userData, const int32_t id, const Ipvm::LineSeg32r& line)
{
    ((ImageViewExImpl*)userData)->callback_measurement_end(id, line);
}

void ImageViewExImpl::callback_measurement_change(void* userData, const int32_t id, const Ipvm::LineSeg32r& line)
{
    ((ImageViewExImpl*)userData)->callback_measurement_change(id, line);
}

void ImageViewExImpl::callback_pixelInfo(
    const int32_t /*id*/, const Ipvm::Point32r2& pt, wchar_t* pixelInfo, const uint32_t bufferLength)
{
    if (m_display_8u_C1.GetMem() != nullptr)
    {
        Ipvm::Point32s2 ptXY(long(pt.m_x + 0.5f), long(pt.m_y + 0.5f));

        // Tugu 20190312 ZMap Raw Data 디스플레이
        float zvalue(0.);
        ;
        if (SystemConfig::GetInstance().m_nVisionType == VISIONTYPE_3D_INSP /* && m_useRawImage*/)
        {
            const long x = long(pt.m_x + 0.5f);
            const long y = long(pt.m_y + 0.5f);

            if (x >= 0 && x < m_RawImage.GetSizeX() && y >= 0 && y < m_RawImage.GetSizeY())
            {
                zvalue = m_RawImage[y][x];
            }

            if (Ipvm::Rect32s(m_display_8u_C1).PtInRect(ptXY))
            {
                if (zvalue == Ipvm::k_noiseValue32r)
                    ::_snwprintf_s(pixelInfo, bufferLength, _TRUNCATE, _T("POS[%.2f %.2f] GV [%03d] ZValue [NOISE]"),
                        pt.m_x, pt.m_y, m_display_8u_C1[ptXY.m_y][ptXY.m_x]);
                else
                    ::_snwprintf_s(pixelInfo, bufferLength, _TRUNCATE, _T("POS[%.2f %.2f] GV [%03d] ZValue [%.2lf um]"),
                        pt.m_x, pt.m_y, m_display_8u_C1[ptXY.m_y][ptXY.m_x], zvalue);
            }
            else
            {
                ::_snwprintf_s(pixelInfo, bufferLength, _TRUNCATE, _T("POS[%.2f %.2f]"), pt.m_x, pt.m_y);
            }
        }
        else
        {
            if (Ipvm::Rect32s(m_display_8u_C1).PtInRect(ptXY))
            {
                ::_snwprintf_s(pixelInfo, bufferLength, _TRUNCATE, _T("POS[%.2f %.2f] GV [%03d]"), pt.m_x, pt.m_y,
                    m_display_8u_C1[ptXY.m_y][ptXY.m_x]);
            }
            else
            {
                ::_snwprintf_s(pixelInfo, bufferLength, _TRUNCATE, _T("POS[%.2f %.2f]"), pt.m_x, pt.m_y);
            }
        }
    }
}

void ImageViewExImpl::callback_measurement_begin(const int32_t /*id*/, const Ipvm::LineSeg32r& line)
{
    if (m_btn_histogram.GetCheck())
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        delete m_profileView;
        m_profileView = new DialogProfile(this);
        m_profileView->Create(IDD_DIALOG_PROFILE, this);
        m_profileView->ShowWindow(SW_SHOW);
        m_profileView->UpdateProfile(line);
    }

    m_displayMeasurement = true;
    m_displayMeasurementLine = line;
}

void ImageViewExImpl::callback_measurement_end(const int32_t /*id*/, const Ipvm::LineSeg32r& line)
{
    UNREFERENCED_PARAMETER(line);

    if (m_profileView)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        delete m_profileView;
        m_profileView = nullptr;
    }

    m_displayMeasurement = false;
}

void ImageViewExImpl::callback_measurement_change(const int32_t /*id*/, const Ipvm::LineSeg32r& line)
{
    if (m_profileView)
    {
        m_profileView->UpdateProfile(line);
    }

    m_displayMeasurementLine = line;
}
