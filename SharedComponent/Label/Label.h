#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#define NM_LINKCLICK (WM_APP + 0x200)

class __INTEKPLUS_SHARED_LABEL_API__ CLabel : public CStatic
{
    // Construction

public:
    CLabel();
    virtual ~CLabel();

    enum FillDirection
    {
        Horizontal,
        Vertical
    };
    enum LinkStyle
    {
        LinkNone,
        HyperLink,
        MailLink
    };
    enum FlashType
    {
        None,
        Text,
        Background
    };
    enum EyeCatchType
    {
        Disable,
        Enable
    };
    enum Type3D
    {
        Raised,
        Sunken
    };
    enum BackFillMode
    {
        Normal,
        Gradient,
        ByDegrees
    };

    virtual CLabel& SetBkColor(COLORREF crBkgnd, COLORREF crBkgndHigh = 0, BackFillMode mode = Normal);
    virtual CLabel& SetTextColor(COLORREF crText);
    virtual CLabel& SetTabClientColor(COLORREF crBordergnd);
    virtual CLabel& SetText(const CString& strText, BOOL bValidate = TRUE);
    virtual CLabel& SetFontBold(BOOL bBold);
    virtual CLabel& SetFontName(const CString& strFont, BYTE byCharSet = ANSI_CHARSET);
    virtual CLabel& SetFontUnderline(BOOL bSet);
    virtual CLabel& SetFontItalic(BOOL bSet);
    virtual CLabel& SetFontSize(int nSize);
    virtual CLabel& SetSunken(BOOL bSet);
    virtual CLabel& SetBorder(BOOL bSet);
    virtual CLabel& SetTabCtrl(BOOL bSet);
    virtual CLabel& SetTabSelected(BOOL bSet);
    virtual CLabel& SetTabBorderWidth(int nWidth);

    virtual CLabel& SetTransparent(BOOL bSet);
    virtual CLabel& FlashText(BOOL bActivate);
    virtual CLabel& EyeCatch(BOOL bActivate);
    virtual CLabel& FlashBackground(BOOL bActivate);
    virtual CLabel& SetLink(BOOL bLink, BOOL bNotifyParent);
    virtual CLabel& SetLinkCursor(HCURSOR hCursor);
    virtual CLabel& SetFont3D(BOOL bSet, Type3D type = Raised);
    virtual CLabel& SetRotationAngle(UINT nAngle, BOOL bRotation);
    virtual CLabel& SetText3DHiliteColor(COLORREF cr3DHiliteColor);
    virtual CLabel& SetFont(LOGFONT lf);
    virtual CLabel& SetMailLink(BOOL bEnable, BOOL bNotifyParent);
    virtual CLabel& SetHyperLink(const CString& sLink);
    virtual COLORREF GetBkColor();
    virtual void SetFillFullStep(int nStep);
    virtual void SetFillCurrentStep(int nStep);
    virtual void SetFillByDegree(COLORREF crBkgnd, COLORREF crBkgndHigh, FillDirection nDirection);
    BOOL SetBitmap(HBITMAP hBitmap, BOOL bFit = TRUE, COLORREF rgbTransparent = 0xFF000000);
    BOOL SetBitmap(UINT nIDResource, BOOL bFit = TRUE,
        COLORREF rgbTransparent = 0xFF000000); // Loads bitmap from resource ID
    BOOL SetBitmap(LPCTSTR lpszResName, BOOL bFit = TRUE,
        COLORREF rgbTransparent = 0xFF000000); // Loads bitmap from resource name

    // Attributes

public:

protected:
    void UpdateSurface();
    void ReconstructFont();
    void DrawGradientFill(CDC* pDC, CRect* pRect, COLORREF crStart, COLORREF crEnd, int nSegments);
    void FillByDegrees(CDC* pDC, CRect* pRect, COLORREF crStart, COLORREF crEnd);
    void DrawBitmap(CDC* pDC, CRect* pRect);
    COLORREF m_crText;
    COLORREF m_cr3DHiliteColor;
    HBRUSH m_hwndBrush;
    HBRUSH m_hBackBrush;
    HBRUSH m_hTabBorderBrush;
    LOGFONT m_lf;
    CFont m_font;
    BOOL m_bState;
    BOOL m_bECState;
    BOOL m_bTimer;
    LinkStyle m_Link;
    BOOL m_bTransparent;
    BOOL m_bFont3d;
    BOOL m_bToolTips;
    BOOL m_bNotifyParent;
    BOOL m_bRotation;
    BOOL m_bTabLabel;
    BOOL m_bTabSelected;
    FlashType m_FlashType;
    EyeCatchType m_ECType;

    HCURSOR m_hCursor;
    Type3D m_3dType;
    BackFillMode m_fillmode;
    COLORREF m_crHiColor;
    COLORREF m_crLoColor;
    CString m_sLink;
    int m_nFontSize;

    int m_nFillFullStep;
    int m_nFillCurrentStep;
    FillDirection m_nFillDirection;
    int m_nTabBorderWidth;
    // BITMAP ATTRIBUTES

    COLORREF m_rgbTransparent;
    int m_EDispMode;
    BOOL m_bBitmap;
    int m_nResourceID;
    HBITMAP m_hBitmap;
    BITMAP m_bmInfo;
    CString m_strResourceName;
    // Operations

public:
    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CLabel)

protected:
    virtual void PreSubclassWindow();
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

    // Implementation

    // Generated message map functions

protected:
    //{{AFX_MSG(CLabel)
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnSysColorChange();
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};
