#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class SimpleRibbonBar : public CMFCRibbonBar
{
    DECLARE_DYNAMIC(SimpleRibbonBar)

public:
    SimpleRibbonBar();
    ~SimpleRibbonBar();

    virtual BOOL OnShowRibbonContextMenu(CWnd* /*pWnd*/, int /*x*/, int /*y*/, CMFCRibbonBaseElement* /*pHit*/)
    {
        return FALSE;
    }
    virtual BOOL OnShowRibbonQATMenu(CWnd* /*pWnd*/, int /*x*/, int /*y*/, CMFCRibbonBaseElement* /*pHit*/)
    {
        return FALSE;
    }
    virtual void OnPaneContextMenu(CWnd* /*pParentFrame*/, CPoint /*point*/)
    {
        return;
    }

protected:
    DECLARE_MESSAGE_MAP()
};
