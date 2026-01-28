#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxcmn.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __INTEKPLUS_SHARED_BUTTON_API__ CPolyBtn : public CButton
{
    // Construction

public:
    CPolyBtn();

    // Operations

public:
    // when using the SetPoints method, you can decide to highlight lines
    // that are more than -45 degrees below horizontal (default false)
    void AllowHighlightOnGtrNeg45(bool b)
    {
        m_bAllowHighlightOnGtrNeg45 = b;
    }

    // should the button change color when the mouse moves over ? (default false)
    void AllowRollover(bool b)
    {
        m_bAllowRollover = b;
    }

    // optionally set the colors used.
    void SetColors(
        COLORREF highlightClr, COLORREF downclr, COLORREF focusclr, COLORREF disabledclr, COLORREF rolloverclr)
    {
        m_clrHighlight = highlightClr; // default ::GetSysColor(COLOR_3DHIGHLIGHT)
        m_clrShadow = downclr; // default ::GetSysColor(COLOR_3DSHADOW);
        m_clrFocus = focusclr; // default ::GetSysColor(COLOR_3DHIGHLIGHT)
        m_clrDisabled = disabledclr; // default ::GetSysColor(COLOR_3DSHADOW);
        m_clrRollover = rolloverclr; // default ::GetSysColor(COLOR_3DHIGHLIGHT);
    }

    // set the color to fill with. default ::GetSysColor(COLOR_BTNFACE);
    //YR
    void SetFillColor(COLORREF fillClr); //{m_clrFill = fillClr;}
    void MakeArrowButton(CWnd* pWnd, int iDepth);
    void MakeHexagonButton(CWnd* pWnd, int iDepth);

    // give it a vector of points and it will figure out the colors by itself.
    // points must be in clockwise order for the automatic lightsource coloring to work correctly.
    // if the points are in clockwise order, the highlights will appear to come from the bottom-right
    // instead of the typical top-left.
    void SetPoints(const std::vector<CPoint>& points);

    // or, supply all the edges, colored individually.
    struct CEdge
    {
        CPoint start, end;
        COLORREF clr;
        COLORREF downclr;
        COLORREF focusclr;
        COLORREF disabledclr;
        COLORREF rolloverclr;
    };
    // this method offers more flexible coloring, but is a little more tedious to use
    void SetEdges(const std::vector<CEdge>& edges);

    // set a bitmap to be drawn
    void SetBMPID(UINT id, COLORREF clrTrans, UINT disabledBitmapID = 0, COLORREF clrTransDisabled = 0)
    {
        m_bmpID = id;
        m_clrTrans = clrTrans;
        m_bmpIDD = disabledBitmapID;
        m_clrTransD = clrTransDisabled;
    }

    // draw bitmap at this offset
    void SetBMPOffset(int x, int y)
    {
        m_bmpOffset = CPoint(x, y);
    }

    // draw window text at this offset
    void SetTextOffset(int x, int y)
    {
        m_textOffset = CPoint(x, y);
    }

    // optional tooltip
    void SetTooltipText(LPCTSTR lpszText, BOOL bActivate = TRUE);
    void ActivateTooltip(BOOL bEnable = TRUE);

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CPolyBtn)

public:
    void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
    BOOL PreTranslateMessage(MSG* pMsg) override;

protected:
    void PreSubclassWindow() override;
    //}}AFX_VIRTUAL

    // Implementation

public:
    ~CPolyBtn() override;

    // Generated message map functions

protected:
    //{{AFX_MSG(CPolyBtn)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    //}}AFX_MSG
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    void _TransparentBlt(CDC& dc, CBitmap& bmp, int xpos, int ypos, COLORREF clrTrans);

    void InitToolTip();
    CToolTipCtrl m_ToolTip;

    ////////////////

    std::vector<CEdge> m_edges;
    POINT* m_pPoints;

    CRgn m_rgn;
    CRgn m_rgnWnd;

    CBitmap m_bmp;
    CBitmap m_bmpD;
    UINT m_bmpID;
    UINT m_bmpIDD;
    COLORREF m_clrTrans;
    COLORREF m_clrTransD;

    CPoint m_bmpOffset;
    CPoint m_textOffset;

    bool m_bAllowRollover;

    COLORREF m_clrHighlight, m_clrShadow, m_clrRollover, m_clrFocus, m_clrDisabled, m_clrFill;

    bool m_bAllowHighlightOnGtrNeg45;
    bool m_bMouseOnButton;
};
