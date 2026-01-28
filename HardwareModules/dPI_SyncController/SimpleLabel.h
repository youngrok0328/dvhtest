#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CSimpleLabel : public CStatic
{
public:
    CSimpleLabel(void);
    ~CSimpleLabel(void);

    void SetBkColor(COLORREF color);
    void SetText(LPCTSTR text);

private:
    CBrush* m_pBrush;
    COLORREF m_backgroundColor;
    CString m_strText;

    virtual void PreSubclassWindow();
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};
