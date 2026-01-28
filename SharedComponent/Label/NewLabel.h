#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CControlToolTip;

//HDR_6_________________________________ Header body
//
class __INTEKPLUS_SHARED_LABEL_API__ CNewLabel : public CStatic
{
public:
    enum
    {
        HORZ_LEFT = 0x01,
        HORZ_CENTER = 0x02,
        HORZ_RIGHT = 0x04,
        VERT_TOP = 0x08,
        VERT_CENTER = 0x10,
        VERT_BOTTOM = 0x20
    };

protected:
    COLORREF m_clrBackColor;
    COLORREF m_clrTextBackColor;
    COLORREF m_clrTextColor;
    COLORREF m_clrHightLight;
    COLORREF m_clrSaveTextColor;
    HBRUSH m_hBackBrush;
    BOOL m_bForceNoOwnerDraw;
    BOOL m_bUseDefaultBackColor;
    BOOL m_bUseDefaultTextBackColor;
    BOOL m_bUseDefaultTextColor;
    BOOL m_bLink;
    BOOL m_bUseBevelLine;
    BOOL m_bUseHighLighting;
    BOOL m_bVerticalText;
    BOOL m_bInside;
    BOOL m_bTextFlash;
    BOOL m_bBackFlash;
    BOOL m_bFlashTextState;
    BOOL m_bFlashBackState;
    BOOL m_bUseToolTip;
    BOOL m_bUseHighLightingOnlyWithToolTip;
    BOOL m_bBlockMessage;
    BOOL m_bLowerCase;
    BOOL m_bUpperCase;
    BOOL m_bStretchBitmap;
    LOGFONT m_stLF;
    CFont m_cFont;
    HCURSOR m_hCursor;
    HBITMAP m_hBackImage;
    UINT m_uiAlignment;
    UINT m_uiAngle;
    CString m_strText;
    CControlToolTip* m_cControlToolTip;

    void DrawAlignedText(CDC* pDC, LPCTSTR lpszText, RECT stRect, UINT uiAlignment);
    int GetWindowText(LPTSTR lpszStringBuf, int nMaxCount);
    void GetWindowText(CString& rString);
    BOOL IsMultiLineString(LPCTSTR lpszText);
    CString MakeVerticalText(LPCTSTR lpszText = NULL);
    BOOL ReconstructFont();
    void SetWindowText(LPCTSTR lpszString);
    void TrackMouse(BOOL bHover, BOOL bLeave);

public:
    CNewLabel();
    virtual ~CNewLabel();

    // Color functions
    CNewLabel& SetBkColor(COLORREF clr = ::GetSysColor(COLOR_3DFACE));
    CNewLabel& SetDefaultBkColor();
    CNewLabel& SetDefaultTextBackColor();
    CNewLabel& SetDefaultTextColor();
    CNewLabel& SetHighLightColor(COLORREF clr = ::GetSysColor(COLOR_HIGHLIGHT));
    CNewLabel& SetTextBackColor(COLORREF clr = ::GetSysColor(COLOR_3DFACE));
    CNewLabel& SetTextColor(COLORREF clr = ::GetSysColor(COLOR_WINDOWTEXT));

    // Font functions
    CNewLabel& SetFontBold(BOOL bBold);
    CNewLabel& SetFontItalic(BOOL bItalic);
    CNewLabel& SetFontName(LPCTSTR szFaceName);
    CNewLabel& SetFontSize(int iSize);
    CNewLabel& SetFontUnderline(BOOL bUnderline);

    // Other functions
    CNewLabel& FlashBackground(BOOL bActivate, UINT uiTime = 500);
    CNewLabel& FlashText(BOOL bActivate, UINT uiTime = 500);

    void ForceNoOwnerDraw(BOOL bForce);

    BOOL GetBorder();
    UINT GetHorzAlignment();
    UINT GetHorzAlignmentIndex();
    BOOL GetSunken();
    CString GetText();
    UINT GetVertAlignment();
    UINT GetVertAlignmentIndex();

    CNewLabel& SetAlignment(UINT uiAlign = HORZ_LEFT | VERT_TOP);
    CNewLabel& SetAngle(UINT uiAngle = 0);
    CNewLabel& SetBackImage(UINT nID = -1);
    CNewLabel& SetBorder(BOOL bSet);
    CNewLabel& SetCursor(HCURSOR hCursor);
    CNewLabel& SetCursor(UINT uiCursorID);
    CNewLabel& SetDisabled(BOOL bSet);
    CNewLabel& SetLink(BOOL bSet);
    CNewLabel& SetLinkCursor(HCURSOR hCursor);
    CNewLabel& SetLinkCursor(UINT uiCursorID);
    CNewLabel& SetLowerCase(BOOL bSet);
    CNewLabel& SetText(LPCTSTR szText);
    CNewLabel& SetSunken(BOOL bSet);
    CNewLabel& SetToolTipPosition(int iPosition = 0);
    CNewLabel& SetToolTipText(LPCTSTR lpszText = _T(""));
    CNewLabel& SetUpperCase(BOOL bSet);
    CNewLabel& SetVerticalText(BOOL bSet);
    CNewLabel& ShowToolTipOnlyOnMouseClick(BOOL bUse);
    CNewLabel& StretchBitmap(BOOL bStretch);
    CNewLabel& UseBevelLine(BOOL bUse);
    CNewLabel& UseHighLighting(BOOL bUse);
    CNewLabel& UseHighLightingOnlyWithToolTip(BOOL bUse);
    CNewLabel& UseLeftMouseButtonForToolTip(BOOL bUse);
    CNewLabel& UseToolTip(BOOL bUse);

    // INLINE
    COLORREF GetBkColor()
    {
        return m_clrBackColor;
    }
    COLORREF GetHighLightColor()
    {
        return m_clrHightLight;
    }
    COLORREF GetTextBackColor()
    {
        return m_clrTextBackColor;
    }
    COLORREF GetTextColor()
    {
        return m_clrTextColor;
    }
    BOOL IsDefaultBkColor()
    {
        return m_bUseDefaultBackColor;
    }
    BOOL IsDefaultTextBackColor()
    {
        return m_bUseDefaultTextBackColor;
    }
    BOOL IsDefaultTextColor()
    {
        return m_bUseDefaultTextColor;
    }

    BOOL GetFontBold()
    {
        return ((m_stLF.lfWeight & FW_BOLD) == FW_BOLD) ? TRUE : FALSE;
    }
    BOOL GetFontItalic()
    {
        return m_stLF.lfItalic;
    }
    CString GetFontName()
    {
        return m_stLF.lfFaceName;
    }
    LONG GetFontSize()
    {
        return m_stLF.lfHeight;
    }
    BOOL GetFontUnderline()
    {
        return m_stLF.lfUnderline;
    }

    UINT GetAlignment()
    {
        return m_uiAlignment;
    }
    UINT GetAngle()
    {
        return m_uiAngle;
    }
    BOOL GetBevelLine()
    {
        return m_bUseBevelLine;
    }
    BOOL GetLink()
    {
        return m_bLink;
    }
    BOOL GetVerticalText()
    {
        return m_bVerticalText;
    }
    BOOL IsBitmapStretched()
    {
        return m_bStretchBitmap;
    }
    BOOL IsDisabled()
    {
        return ((GetStyle() & WS_DISABLED) == WS_DISABLED);
    }
    BOOL IsLowerCase()
    {
        return m_bLowerCase;
    }
    BOOL IsToolTipUsed()
    {
        return m_bUseToolTip;
    }
    BOOL IsUpperCase()
    {
        return m_bUpperCase;
    }
    BOOL UseHighLightingOnlyWithToolTip()
    {
        return m_bUseHighLightingOnlyWithToolTip;
    }
    BOOL UseToolTipOnLeftMouseClick();
    BOOL UseToolTipWithMouseClick();

public:
    //{{AFX_VIRTUAL(CNewLabel)

public:
    virtual BOOL DestroyWindow();

protected:
    virtual void PreSubclassWindow();
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    //}}AFX_VIRTUAL

protected:
    //{{AFX_MSG(CNewLabel)
    afx_msg void OnEnable(BOOL bEnable);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};
