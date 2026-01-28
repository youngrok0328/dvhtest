#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "AutoHidePopup.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/TimeCheck.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class AutoHidePopupWindow;

//HDR_6_________________________________ Header body
//
class AutoHidePopupEngine
{
public:
    AutoHidePopupEngine();
    ~AutoHidePopupEngine();

    void add(AutoHidePopupType type, LPCTSTR message);
    void del(AutoHidePopupWindow* window);

private:
    CString m_className;

    bool m_firstCall;
    CCriticalSection m_csFunction;
    std::vector<AutoHidePopupWindow*> m_windows;
};

class AutoHidePopupWindow : public CWnd
{
    DECLARE_DYNAMIC(AutoHidePopupWindow)

public:
    AutoHidePopupWindow(AutoHidePopupEngine* parent, AutoHidePopupType type, LPCTSTR message);
    virtual ~AutoHidePopupWindow();

    bool create(LPCTSTR className, const CRect& rect);

protected:
    AutoHidePopupEngine* m_parent;
    AutoHidePopupType m_type;
    Ipvm::TimeCheck m_timer;
    BYTE m_alpha;
    CString m_message;
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClose();
};
