//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NewLabel.h"

//CPP_2_________________________________ This project's headers
#include "ControlToolTip.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CNewLabel::CNewLabel()
    : m_cControlToolTip(new CControlToolTip)
{
    m_clrBackColor = ::GetSysColor(COLOR_3DFACE);
    m_clrTextBackColor = ::GetSysColor(COLOR_3DFACE);
    m_clrTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
    m_clrSaveTextColor = m_clrTextColor;
    m_clrHightLight = ::GetSysColor(COLOR_HIGHLIGHT);
    m_hBackBrush = ::CreateSolidBrush(m_clrBackColor);
    m_bForceNoOwnerDraw = FALSE;
    m_bUseDefaultBackColor = TRUE;
    m_bUseDefaultTextBackColor = TRUE;
    m_bUseDefaultTextColor = TRUE;
    m_bLink = FALSE;
    m_bUseBevelLine = FALSE;
    m_bUseHighLighting = FALSE;
    m_bVerticalText = FALSE;
    m_bInside = FALSE;
    m_bTextFlash = FALSE;
    m_bBackFlash = FALSE;
    m_bFlashTextState = FALSE;
    m_bFlashBackState = FALSE;
    m_bUseToolTip = FALSE;
    m_bUseHighLightingOnlyWithToolTip = FALSE;
    m_bBlockMessage = FALSE;
    m_bLowerCase = FALSE;
    m_bUpperCase = FALSE;
    m_bStretchBitmap = FALSE;
    m_hCursor = NULL;
    m_hBackImage = NULL;
    m_uiAlignment = HORZ_LEFT | VERT_TOP;
    m_uiAngle = 0; // 0?

    ::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(m_stLF), &m_stLF);

    m_cFont.CreateFontIndirect(&m_stLF);
}

// --------------------------------------------------------------------------

CNewLabel::~CNewLabel()
{
    delete m_cControlToolTip;

    if (m_hBackImage)
        DeleteObject(m_hBackImage);

    m_cFont.DeleteObject();

    DeleteObject(m_hBackBrush);
}

// --------------------------------------------------------------------------
// PROTECTED MEMBER FUNCTIONS
// --------------------------------------------------------------------------

void CNewLabel::DrawAlignedText(CDC* pDC, LPCTSTR lpszText, RECT stRect, UINT uiAlignment)
{
    UINT uiFormat = 0;

    if ((uiAlignment & HORZ_LEFT) == HORZ_LEFT)
        uiFormat |= DT_LEFT;
    if ((uiAlignment & HORZ_CENTER) == HORZ_CENTER)
        uiFormat |= DT_CENTER;
    if ((uiAlignment & HORZ_RIGHT) == HORZ_RIGHT)
        uiFormat |= DT_RIGHT;

    BOOL bSingleLine = !IsMultiLineString(lpszText);
    BOOL bIsVTop = ((uiAlignment & VERT_TOP) == VERT_TOP);
    BOOL bIsVCenter = ((uiAlignment & VERT_CENTER) == VERT_CENTER);
    BOOL bIsVBottom = ((uiAlignment & VERT_BOTTOM) == VERT_BOTTOM);

    if (bSingleLine || (!bSingleLine && bIsVTop))
    {
        CRect rect = stRect;

        if (bIsVCenter)
            uiFormat |= DT_VCENTER | DT_SINGLELINE;

        if (bIsVBottom)
        {
            CSize cSize = pDC->GetTextExtent(lpszText);

            rect.top += (stRect.bottom - stRect.top) - cSize.cy;
        }

        pDC->DrawText(lpszText, -1, &rect, uiFormat);

        return;
    }

    // MultiLine Drawing (TOP or BOTTOM)
    pDC->DrawText(_T("**********"), -1, &stRect, uiFormat);
}

// --------------------------------------------------------------------------

int CNewLabel::GetWindowText(LPTSTR lpszStringBuf, int nMaxCount)
{
    m_bBlockMessage = TRUE;

    int iRet = CStatic::GetWindowText(lpszStringBuf, nMaxCount);

    m_bBlockMessage = FALSE;

    return iRet;
}

// --------------------------------------------------------------------------

void CNewLabel::GetWindowText(CString& rString)
{
    m_bBlockMessage = TRUE;
    CStatic::GetWindowText(rString);
    m_bBlockMessage = FALSE;
}

// --------------------------------------------------------------------------

BOOL CNewLabel::IsMultiLineString(LPCTSTR lpszText)
{
    for (unsigned int I = 0; I < _tcslen(lpszText); I++)
    {
        if (lpszText[I] == '\n')
            return TRUE;
    }

    return FALSE;
}

// --------------------------------------------------------------------------

CString CNewLabel::MakeVerticalText(LPCTSTR lpszText)
{
    CString strSource, strDest;

    strSource = (lpszText) ? CString(lpszText) : m_strText;

    for (int I = 0; I < strSource.GetLength(); I++)
    {
        if (I > 0)
            strDest += '\n';

        strDest += strSource[I];
    }
    strSource.Empty();

    return strDest;
}

// --------------------------------------------------------------------------

BOOL CNewLabel::ReconstructFont()
{
    m_cFont.DeleteObject();

    BOOL bRet = m_cFont.CreateFontIndirect(&m_stLF);

    RedrawWindow();

    return bRet;
}

// --------------------------------------------------------------------------

void CNewLabel::SetWindowText(LPCTSTR lpszString)
{
    m_bBlockMessage = TRUE;
    CStatic::SetWindowText(lpszString);
    m_bBlockMessage = FALSE;
}

// --------------------------------------------------------------------------

void CNewLabel::TrackMouse(BOOL bHover, BOOL bLeave)
{
    TRACKMOUSEEVENT stTME;

    stTME.cbSize = sizeof(stTME);
    stTME.dwFlags = 0;
    stTME.hwndTrack = GetSafeHwnd();
    stTME.dwHoverTime = 50;

    if (bHover)
        stTME.dwFlags |= TME_HOVER;

    //if (!bHover) stTME.dwFlags |= TME_HOVER | TME_CANCEL;

    if (bLeave)
        stTME.dwFlags |= TME_LEAVE;

    //if (!bLeave) stTME.dwFlags |= TME_LEAVE | TME_CANCEL;

    _TrackMouseEvent(&stTME);
}

// --------------------------------------------------------------------------
// PUBLIC MEMBER FUNCTIONS
// --------------------------------------------------------------------------

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// COLOR FUNCTIONS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CNewLabel& CNewLabel::SetBkColor(COLORREF clr)
{
    m_bUseDefaultBackColor = FALSE;
    m_clrBackColor = clr;

    DeleteObject(m_hBackBrush);

    m_hBackBrush = CreateSolidBrush(m_clrBackColor);

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetDefaultBkColor()
{
    m_bUseDefaultBackColor = TRUE;
    m_clrBackColor = ::GetSysColor(COLOR_3DFACE);

    DeleteObject(m_hBackBrush);

    m_hBackBrush = CreateSolidBrush(m_clrBackColor);

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetDefaultTextBackColor()
{
    m_bUseDefaultTextBackColor = TRUE;
    m_clrTextBackColor = ::GetSysColor(COLOR_3DFACE);

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetDefaultTextColor()
{
    m_bUseDefaultTextColor = TRUE;
    m_clrTextColor = ::GetSysColor(COLOR_WINDOWTEXT);

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetHighLightColor(COLORREF clr)
{
    m_clrHightLight = clr;

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetTextBackColor(COLORREF clr)
{
    m_bUseDefaultTextBackColor = FALSE;
    m_clrTextBackColor = clr;

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetTextColor(COLORREF clr)
{
    m_bUseDefaultTextColor = FALSE;
    m_clrTextColor = clr;

    RedrawWindow();

    return *this;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// FONT FUNCTIONS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CNewLabel& CNewLabel::SetFontBold(BOOL bBold)
{
    m_stLF.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
    ReconstructFont();
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetFontItalic(BOOL bItalic)
{
    m_stLF.lfItalic = (BYTE)(bItalic);
    ReconstructFont();
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetFontName(LPCTSTR szFaceName)
{
    _tcscpy_s(m_stLF.lfFaceName, LF_FACESIZE, szFaceName);
    ReconstructFont();
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetFontSize(BOOL iSize)
{
    m_stLF.lfHeight = iSize;
    ReconstructFont();
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetFontUnderline(BOOL bUnderline)
{
    m_stLF.lfUnderline = (BYTE)(bUnderline);
    ReconstructFont();
    return *this;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// OTHER FUNCTIONS
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CNewLabel& CNewLabel::FlashBackground(BOOL bActivate, UINT uiTime)
{
    if (m_bBackFlash)
        KillTimer(2);

    if (bActivate)
        SetTimer(2, uiTime, NULL);
    else
        InvalidateRect(NULL, FALSE);

    m_bBackFlash = bActivate;

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::FlashText(BOOL bActivate, UINT uiTime)
{
    if (m_bTextFlash)
    {
        SetWindowText(m_strText);
        KillTimer(1);
    }

    if (bActivate)
    {
        SetTimer(1, uiTime, NULL);
    }
    else
        InvalidateRect(NULL, FALSE);

    m_bTextFlash = bActivate;

    return *this;
}

// --------------------------------------------------------------------------

void CNewLabel::ForceNoOwnerDraw(BOOL bForce)
{
    m_bForceNoOwnerDraw = bForce;
    RedrawWindow();
}

// --------------------------------------------------------------------------

BOOL CNewLabel::GetBorder()
{
    return ((GetStyle() & WS_BORDER) == WS_BORDER) ? TRUE : FALSE;
}

// --------------------------------------------------------------------------

UINT CNewLabel::GetHorzAlignment()
{
    if ((m_uiAlignment & HORZ_LEFT) == HORZ_LEFT)
        return HORZ_LEFT;

    if ((m_uiAlignment & HORZ_CENTER) == HORZ_CENTER)
        return HORZ_CENTER;

    if ((m_uiAlignment & HORZ_RIGHT) == HORZ_RIGHT)
        return HORZ_RIGHT;

    return 0;
}

// --------------------------------------------------------------------------

UINT CNewLabel::GetHorzAlignmentIndex()
{
    if ((m_uiAlignment & HORZ_LEFT) == HORZ_LEFT)
        return 0;

    if ((m_uiAlignment & HORZ_CENTER) == HORZ_CENTER)
        return 1;

    if ((m_uiAlignment & HORZ_RIGHT) == HORZ_RIGHT)
        return 2;

    return 0;
}

// --------------------------------------------------------------------------

BOOL CNewLabel::GetSunken()
{
    return ((GetExStyle() & WS_EX_STATICEDGE) == WS_EX_STATICEDGE) ? TRUE : FALSE;
}

// --------------------------------------------------------------------------

CString CNewLabel::GetText()
{
    return m_strText;
}

// --------------------------------------------------------------------------

UINT CNewLabel::GetVertAlignment()
{
    if ((m_uiAlignment & VERT_TOP) == VERT_TOP)
        return VERT_TOP;

    if ((m_uiAlignment & VERT_CENTER) == VERT_CENTER)
        return VERT_CENTER;

    if ((m_uiAlignment & VERT_BOTTOM) == VERT_BOTTOM)
        return VERT_BOTTOM;

    return 0;
}

// --------------------------------------------------------------------------

UINT CNewLabel::GetVertAlignmentIndex()
{
    if ((m_uiAlignment & VERT_TOP) == VERT_TOP)
        return 0;

    if ((m_uiAlignment & VERT_CENTER) == VERT_CENTER)
        return 1;

    if ((m_uiAlignment & VERT_BOTTOM) == VERT_BOTTOM)
        return 2;

    return 0;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetAlignment(UINT uiAlign)
{
    DWORD dwRemove = 0;
    DWORD dwHorz = 0;
    DWORD dwVert = 0;

    if ((uiAlign & HORZ_RIGHT) == HORZ_RIGHT)
    {
        dwHorz = SS_RIGHT;
        dwRemove = SS_LEFT | SS_CENTER;
        m_uiAlignment = HORZ_RIGHT;
    }

    if ((uiAlign & HORZ_CENTER) == HORZ_CENTER)
    {
        dwHorz = SS_CENTER;
        dwRemove = SS_LEFT | SS_RIGHT;
        m_uiAlignment = HORZ_CENTER;
    }

    if ((uiAlign & HORZ_LEFT) == HORZ_LEFT)
    {
        dwHorz = SS_LEFT;
        dwRemove = SS_RIGHT | SS_CENTER;
        m_uiAlignment = HORZ_LEFT;
    }

    ModifyStyle(dwRemove, dwHorz);

    if ((uiAlign & VERT_TOP) == VERT_TOP)
        dwVert = VERT_TOP;
    if ((uiAlign & VERT_CENTER) == VERT_CENTER)
        dwVert = VERT_CENTER;
    if ((uiAlign & VERT_BOTTOM) == VERT_BOTTOM)
        dwVert = VERT_BOTTOM;

    m_uiAlignment |= dwVert;

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetAngle(UINT uiAngle)
{
    m_uiAngle = uiAngle;
    RedrawWindow();
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetBackImage(UINT nID)
{
    if (m_hBackImage)
        DeleteObject(m_hBackImage);

    m_hBackImage = NULL;

    if (nID != -1)
    {
        m_hBackImage = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(nID));
    }

    InvalidateRect(NULL, TRUE);

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetBorder(BOOL bSet)
{
    ModifyStyle(bSet ? 0 : WS_BORDER, bSet ? WS_BORDER : 0, SWP_DRAWFRAME);

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetCursor(HCURSOR hCursor)
{
    m_hCursor = hCursor;
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetCursor(UINT uiCursorID)
{
    m_hCursor = LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(uiCursorID));
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetDisabled(BOOL bSet)
{
    ModifyStyle(bSet ? 0 : WS_DISABLED, bSet ? WS_DISABLED : 0, 0);

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetLink(BOOL bSet)
{
    m_bLink = bSet;
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetLinkCursor(HCURSOR hCursor)
{
    m_hCursor = hCursor;
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetLinkCursor(UINT uiCursorID)
{
    m_hCursor = LoadCursor(AfxGetInstanceHandle(), MAKEINTRESOURCE(uiCursorID));
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetLowerCase(BOOL bSet)
{
    m_bLowerCase = bSet;
    m_bUpperCase = FALSE;
    RedrawWindow();
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetText(LPCTSTR szText)
{
    m_strText = szText;

    if (m_bVerticalText)
    {
        SetWindowText(MakeVerticalText(szText));
    }
    else
        SetWindowText(szText);

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetSunken(BOOL bSet)
{
    ModifyStyleEx(bSet ? 0 : WS_EX_STATICEDGE, bSet ? WS_EX_STATICEDGE : 0, SWP_DRAWFRAME);

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetToolTipPosition(int iPosition)
{
    m_cControlToolTip->SetPosition(iPosition);
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetToolTipText(LPCTSTR lpszText)
{
    m_cControlToolTip->SetText(lpszText);
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetUpperCase(BOOL bSet)
{
    m_bLowerCase = FALSE;
    m_bUpperCase = bSet;
    RedrawWindow();
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::SetVerticalText(BOOL bSet)
{
    if (bSet && !m_bVerticalText)
    {
        SetWindowText(MakeVerticalText(m_strText));
    }
    else if (m_bVerticalText)
        SetWindowText(m_strText);

    m_bVerticalText = bSet;

    RedrawWindow();

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::ShowToolTipOnlyOnMouseClick(BOOL bUse)
{
    m_cControlToolTip->ShowOnlyOnMouseClickDown(bUse);
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::StretchBitmap(BOOL bStretch)
{
    m_bStretchBitmap = bStretch;
    if (m_hBackImage)
        RedrawWindow();
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::UseBevelLine(BOOL bUse)
{
    m_bUseBevelLine = bUse;
    RedrawWindow();
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::UseHighLighting(BOOL bUse)
{
    m_bUseHighLighting = bUse;

    if (!bUse)
        SetTextColor(m_clrSaveTextColor);

    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::UseHighLightingOnlyWithToolTip(BOOL bUse)
{
    m_bUseHighLightingOnlyWithToolTip = bUse;
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::UseLeftMouseButtonForToolTip(BOOL bUse)
{
    m_cControlToolTip->UseLeftMouseButton(bUse);
    return *this;
}

// --------------------------------------------------------------------------

CNewLabel& CNewLabel::UseToolTip(BOOL bUse)
{
    m_bUseToolTip = bUse;

    if (bUse)
    {
        m_cControlToolTip->Attach(this);
    }
    else
    {
        m_cControlToolTip->Detach();
    }

    return *this;
}

BOOL CNewLabel::UseToolTipOnLeftMouseClick()
{
    return m_cControlToolTip->IsLeftMouseButtonUsed();
}
BOOL CNewLabel::UseToolTipWithMouseClick()
{
    return m_cControlToolTip->ShowOnlyOnMouseClickDown();
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CNewLabel, CStatic)
//{{AFX_MSG_MAP(CNewLabel)
ON_WM_ENABLE()
ON_WM_ERASEBKGND()
ON_WM_SETCURSOR()
ON_WM_LBUTTONDOWN()
ON_WM_CTLCOLOR_REFLECT()
ON_WM_PAINT()
ON_WM_MOUSEMOVE()
ON_WM_TIMER()
ON_WM_LBUTTONUP()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewLabel message handlers

BOOL CNewLabel::DestroyWindow()
{
    return CStatic::DestroyWindow();
}

// --------------------------------------------------------------------------

HBRUSH CNewLabel::CtlColor(CDC* pDC, UINT nCtlColor)
{
    if (!m_bForceNoOwnerDraw)
        return NULL;

    if (CTLCOLOR_STATIC == nCtlColor)
    {
        pDC->SelectObject(&m_cFont);
        pDC->SetTextColor(!m_bUseDefaultTextColor ? m_clrTextColor : ::GetSysColor(COLOR_WINDOWTEXT));
        pDC->SetBkMode(TRANSPARENT);
    }

    return m_hBackBrush;
}

// --------------------------------------------------------------------------

void CNewLabel::PreSubclassWindow()
{
    CStatic::PreSubclassWindow();

    ModifyStyle(0, SS_NOTIFY);

    if ((GetStyle() & SS_CENTER) == SS_CENTER)
        m_uiAlignment = GetVertAlignment() | HORZ_CENTER;

    if ((GetStyle() & SS_RIGHT) == SS_RIGHT)
        m_uiAlignment = GetVertAlignment() | HORZ_RIGHT;

    if ((GetStyle() & SS_CENTERIMAGE) == SS_CENTERIMAGE)
        m_uiAlignment = GetHorzAlignment() | VERT_CENTER;

    GetWindowText(m_strText);
}

// --------------------------------------------------------------------------

LRESULT CNewLabel::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_MOUSEHOVER)
    {
        if (m_bUseHighLighting)
        {
            m_clrSaveTextColor = m_clrTextColor;

            CString str = m_cControlToolTip->GetText();

            if (!m_bUseHighLightingOnlyWithToolTip
                || (m_bUseHighLightingOnlyWithToolTip && m_bUseToolTip && !str.IsEmpty()))
            {
                SetTextColor(m_clrHightLight);
            }
            str.Empty();
        }
    }

    else if (message == WM_MOUSELEAVE)
    {
        // Determine if mouse pointer is within control when tooltip is used
        // because when mouse go over tooltip, this message is sended.
        TrackMouse(TRUE, FALSE);

        m_bInside = FALSE;

        if (m_bUseHighLighting)
            SetTextColor(m_clrSaveTextColor);
    }

    else if (message == WM_SETTEXT)
    {
        if (!m_bBlockMessage)
        {
            m_strText = (LPCTSTR)lParam;
        }
    }

    else if (message == WM_GETTEXT)
    {
        if (!m_bBlockMessage)
        {
            int iCount = (int)wParam;

            memset((char*)lParam, 0, iCount + 1);
            memcpy((char*)lParam, m_strText, iCount);
            return TRUE;
        }
    }

    return CStatic::WindowProc(message, wParam, lParam);
}

// --------------------------------------------------------------------------

void CNewLabel::OnEnable(BOOL bEnable)
{
    SetRedraw(FALSE);
    CStatic::OnEnable(bEnable);
    SetRedraw(TRUE);
    Invalidate();
}

// --------------------------------------------------------------------------

BOOL CNewLabel::OnEraseBkgnd(CDC* pDC)
{
    return CStatic::OnEraseBkgnd(pDC);
}

// --------------------------------------------------------------------------

void CNewLabel::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_bLink)
    {
        ShellExecute(NULL, _T("open"), m_strText, NULL, NULL, SW_SHOWNORMAL);
    }

    long low = point.y;
    //	long high = point.x;
    low = low << 16;

    nFlags = 35;

    CStatic::OnLButtonDown(nFlags, point);
}

// --------------------------------------------------------------------------

void CNewLabel::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_bInside)
    {
        m_bInside = TRUE;
        TrackMouse(TRUE, TRUE);
    }

    long low = point.y;
    //	long high = point.x;
    low = low << 16;

    nFlags = 35;

    CStatic::OnMouseMove(nFlags, point);
}

// --------------------------------------------------------------------------

BOOL CNewLabel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if (m_hCursor)
    {
        ::SetCursor(m_hCursor);
        return TRUE;
    }

    return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

// --------------------------------------------------------------------------

void CNewLabel::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent)
    {
        case 1: // TEXT FLASH
            if (!m_bTextFlash)
                break;

            m_bFlashTextState = !m_bFlashTextState;

            SetWindowText(m_bFlashTextState ? CString(_T("")) : m_strText);

            if (!m_bFlashTextState)
                InvalidateRect(NULL, FALSE);

            if (!m_bUseDefaultBackColor && m_clrBackColor != GetSysColor(COLOR_3DFACE))
            {
                InvalidateRect(NULL, FALSE);
            }
            break;

        case 2: // BACK FLASH
            if (!m_bBackFlash)
                break;

            m_bFlashBackState = !m_bFlashBackState;

            InvalidateRect(NULL, TRUE);
            break;
    }

    CStatic::OnTimer(nIDEvent);
}

// --------------------------------------------------------------------------

void CNewLabel::OnPaint()
{
    if (m_bForceNoOwnerDraw)
    {
        CStatic::OnPaint();
        return;
    }

    CPaintDC dc(this); // device context for painting
    CRect rectClient;
    CString strText = (!m_bVerticalText) ? m_strText : MakeVerticalText(m_strText);

    if (m_bLowerCase)
        strText.MakeLower();

    if (m_bUpperCase)
        strText.MakeUpper();

    GetClientRect(&rectClient);

    // Get Horz & Vert Alignment
    UINT uiHorzAlignment = DT_LEFT;
    UINT uiVertAlignment = 0;

    if ((m_uiAlignment & HORZ_CENTER) == HORZ_CENTER)
        uiHorzAlignment = DT_CENTER;

    if ((m_uiAlignment & HORZ_RIGHT) == HORZ_RIGHT)
        uiHorzAlignment = DT_RIGHT;

    if ((m_uiAlignment & VERT_CENTER) == VERT_CENTER)
        uiVertAlignment = 1;

    if ((m_uiAlignment & VERT_BOTTOM) == VERT_BOTTOM)
        uiVertAlignment = 2;

    // Set current font
    CFont* pOldFont = dc.SelectObject(&m_cFont);

    // Set Back Mode
    int iOldMode = dc.SetBkMode(TRANSPARENT);

    // Erase background
    CBrush cBrush(::GetSysColor(COLOR_3DFACE));

    if (!m_bUseDefaultBackColor)
    {
        cBrush.DeleteObject();
        cBrush.CreateSolidBrush(m_clrBackColor);
    }

    if (m_bBackFlash && m_bFlashBackState)
    {
        COLORREF clr = m_bUseDefaultBackColor ? m_clrBackColor / 2 : ::GetSysColor(COLOR_3DFACE);

        cBrush.DeleteObject();
        cBrush.CreateSolidBrush(clr);

        //		 cTextRotator.SetBackgroundColor(clr);
    }

    dc.FillRect(rectClient, &cBrush);

    cBrush.DeleteObject();

    if (m_hBackImage)
    {
        BITMAP stBitmap;

        GetObject(m_hBackImage, sizeof(stBitmap), &stBitmap);

        CDC memDC;

        memDC.CreateCompatibleDC(&dc);

        memDC.SelectObject(m_hBackImage);

        if (m_bStretchBitmap)
        {
            dc.StretchBlt(0, 0, rectClient.Width(), rectClient.Height(), &memDC, 0, 0, stBitmap.bmWidth,
                stBitmap.bmHeight, SRCCOPY);
        }
        else
        {
            int X = (rectClient.Width() - stBitmap.bmWidth) / 2;
            int Y = (rectClient.Height() - stBitmap.bmHeight) / 2;
            int W = rectClient.Width();
            int H = rectClient.Height();

            if (X < 0)
            {
                X = -X / 2;
                W = rectClient.Width() - X;
            }

            if (Y < 0)
            {
                Y = -Y / 2;
                H = rectClient.Height() - Y;
            }

            dc.BitBlt(X, Y, W, H, &memDC, 0, 0, SRCCOPY);
        }

        memDC.DeleteDC();
    }

    // Rotate Text
    //	 if (m_hBackImage &&
    //		 m_uiAngle == 0 &&
    //		 (!IsMultiLineString(strText) || (IsMultiLineString(strText) && GetVertAlignment() == VERT_TOP)))
    //	 {
    dc.SetTextColor(!m_bUseDefaultTextColor ? m_clrTextColor : ::GetSysColor(COLOR_WINDOWTEXT));

    DrawAlignedText(&dc, strText, rectClient, m_uiAlignment);
    //	 }
    //	 else {
    //		 BOOL bRet = cTextRotator.RotateText(m_uiAngle);

    //		 if (bRet)
    //       {
    //			 CRect rectRotated = cTextRotator.GetRotatedRect();

    // Draw Text
    //			 cTextRotator.BitBltText(rectClient);
    //       }
    //	 }

    // Restore
    dc.SelectObject(pOldFont);
    dc.SetBkMode(iOldMode);
}

// --------------------------------------------------------------------------

void CNewLabel::OnLButtonUp(UINT nFlags, CPoint point)
{
    long low = point.y;
    //	long high = point.x;
    low = low << 16;

    nFlags = 35;

    CStatic::OnLButtonUp(nFlags, point);
}
