//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "AutoHidePopupEngine.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
AutoHidePopupEngine::AutoHidePopupEngine()
    : m_firstCall(true)
{
}

AutoHidePopupEngine::~AutoHidePopupEngine()
{
    for (auto* window : m_windows)
    {
        delete window;
    }
}

void AutoHidePopupEngine::add(AutoHidePopupType type, LPCTSTR message)
{
    CSingleLock lock(&m_csFunction, TRUE);

    if (m_firstCall)
    {
        m_className = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW),
            (HBRUSH)GetStockObject(WHITE_BRUSH), ::LoadIcon(NULL, IDI_APPLICATION));

        m_firstCall = false;
    }

    long cenX = GetSystemMetrics(SM_CXFULLSCREEN) / 2;
    long cenY = GetSystemMetrics(SM_CYFULLSCREEN) / 2;
    long halfX = 400;
    long halfY = 25;
    long sizeX = halfX * 2;
    long sizeY = halfY * 2;

    auto* new_window = new AutoHidePopupWindow(this, type, message);
    new_window->create(m_className, CRect(cenX - halfX, cenY - halfY, cenX + halfX, cenY + halfY));
    m_windows.push_back(new_window);

    long ltX = cenX - halfX;
    long ltY = cenY - long(m_windows.size()) * halfY;

    for (long index = 0; index < long(m_windows.size()); index++)
    {
        auto* window = m_windows[index];
        window->MoveWindow(ltX, ltY, sizeX, sizeY);

        ltY += sizeY;
    }
}

void AutoHidePopupEngine::del(AutoHidePopupWindow* window)
{
    CSingleLock lock(&m_csFunction, TRUE);

    for (long index = 0; index < long(m_windows.size()); index++)
    {
        if (m_windows[index] == window)
        {
            delete window;
            m_windows.erase(m_windows.begin() + index);
            return;
        }
    }
}

//=======================================================
// AutoHidePopupWindow
//=======================================================

IMPLEMENT_DYNAMIC(AutoHidePopupWindow, CWnd)

AutoHidePopupWindow::AutoHidePopupWindow(AutoHidePopupEngine* parent, AutoHidePopupType type, LPCTSTR message)
    : m_parent(parent)
    , m_type(type)
    , m_message(message)
    , m_alpha(255)
{
    m_timer.Reset();
}

AutoHidePopupWindow::~AutoHidePopupWindow()
{
}

bool AutoHidePopupWindow::create(LPCTSTR className, const CRect& rect)
{
    if (!CWnd::CreateEx(0, className, _T("AutoHidePopupWindow"), WS_POPUP | WS_VISIBLE, rect, nullptr, 0))
    {
        ASSERT(!_T("??"));
        return false;
    }

    SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    ModifyStyleEx(0, WS_EX_TRANSPARENT | WS_EX_LAYERED);
    SetTimer(999, 50, nullptr);

    return true;
}

BEGIN_MESSAGE_MAP(AutoHidePopupWindow, CWnd)
ON_WM_PAINT()
ON_WM_SIZE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_TIMER()
ON_WM_CLOSE()
END_MESSAGE_MAP()

// CustomDrawWindow message handlers

void AutoHidePopupWindow::OnPaint()
{
    CRect clientRegion;
    GetClientRect(clientRegion);
    long cx = clientRegion.Width();
    long cy = clientRegion.Height();

    CPaintDC dc(this);

    CDC memDC;
    CBitmap memBitmap;
    memDC.CreateCompatibleDC(&dc);
    memBitmap.CreateCompatibleBitmap(&dc, cx, cy);
    auto* oldMemBitmap = memDC.SelectObject(&memBitmap);

    memDC.SetBkMode(TRANSPARENT);

    LOGFONT logFont;
    if (1)
    {
        CFont* gui_font = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
        gui_font->GetLogFont(&logFont);
    }

    logFont.lfHeight = clientRegion.Height() - 20;

    CFont font;
    font.CreateFontIndirect(&logFont);

    COLORREF fontColor = RGB(255, 255, 255);
    COLORREF bgColor = RGB(51, 144, 255);
    if (m_type == AutoHidePopupType::Error)
    {
        bgColor = RGB(255, 70, 15);
    }

    memDC.SetTextColor(fontColor);

    auto oldFont = memDC.SelectObject(&font);

    memDC.FillSolidRect(clientRegion, bgColor);

    memDC.DrawText(m_message, clientRegion, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    oldFont = memDC.SelectObject(oldFont);

    dc.BitBlt(0, 0, cx, cy, &memDC, 0, 0, SRCCOPY);
    memDC.SelectObject(oldMemBitmap);
}

void AutoHidePopupWindow::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    Invalidate(FALSE);
}

void AutoHidePopupWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
    CWnd::OnLButtonDown(nFlags, point);
}

void AutoHidePopupWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
    CWnd::OnLButtonUp(nFlags, point);
}

void AutoHidePopupWindow::OnMouseMove(UINT nFlags, CPoint point)
{
    CWnd::OnMouseMove(nFlags, point);
}

void AutoHidePopupWindow::OnTimer(UINT_PTR nIDEvent)
{
    SetLayeredWindowAttributes(0, m_alpha, LWA_ALPHA);

    if (m_timer.Elapsed_ms() >= 1000.f)
    {
        // 1초후 부터 페이트 아웃
        if (m_alpha < 10)
        {
            m_alpha = 0;
        }
        else
        {
            m_alpha -= 10;
        }

        if (m_alpha == 0)
        {
            CWnd::OnTimer(nIDEvent);
            KillTimer(nIDEvent);

            m_parent->del(this);
            return;
        }
    }

    CWnd::OnTimer(nIDEvent);
}

void AutoHidePopupWindow::OnClose()
{
    //CWnd::OnClose();
}
