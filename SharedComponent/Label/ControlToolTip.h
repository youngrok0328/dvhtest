#pragma once

// ==========================================================================
// ControlToolTip.h
//
// Author : Marquet Mike
//          mike.marquet@altavista.net
//
// Date of creation  : 26/07/2000
// Last modification : 27/07/2000
// ==========================================================================

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CControlToolTip : public CWnd
{
protected:
    CWnd* m_pControl;
    CString m_strText;
    CRect m_rectText;
    int m_iPosition; // 0 = OVER CONTROL
        // 1 = UNDER CONTROL
        // 2 = ABOVE CONTROL
    COLORREF m_clrText;
    COLORREF m_clrBkgnd;
    BOOL m_bUseBgndColor;
    BOOL m_bShowOnlyOnMouseClickDown;
    BOOL m_bUseLeftButton;

    CSize GetMultiLineTextExtent(CClientDC* pDC, LPCTSTR lpszText);
    int GetNumberOfLines(LPCTSTR lpszText = NULL);
    BOOL IsMouseInControl();
    void Show();

public:
    CControlToolTip();
    virtual ~CControlToolTip();

    static void RegisterWindowClass();

    BOOL Attach(CWnd* pControl);
    BOOL Detach();

    void SetBkgndColor(BOOL bUse = FALSE, COLORREF clr = ::GetSysColor(COLOR_INFOBK));
    void SetPosition(int iPosition = 0);
    void SetText(LPCTSTR lpszText = _T(""));
    void SetTextColor(COLORREF clr = ::GetSysColor(COLOR_INFOTEXT));
    void ShowOnlyOnMouseClickDown(BOOL bShow);

    void UseLeftMouseButton(BOOL bUse);

    // INLINE
    int GetPosition()
    {
        return m_iPosition;
    }

    CString GetText()
    {
        return m_strText;
    }

    BOOL ShowOnlyOnMouseClickDown()
    {
        return m_bShowOnlyOnMouseClickDown;
    }

    BOOL IsAttached()
    {
        return (m_pControl != NULL);
    }

    BOOL IsLeftMouseButtonUsed()
    {
        return m_bUseLeftButton;
    }

public:
    //{{AFX_VIRTUAL(CControlToolTip)
    //}}AFX_VIRTUAL

protected:
    //{{AFX_MSG(CControlToolTip)
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnPaint();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};
