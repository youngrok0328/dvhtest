#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionImageLotInsp;

//HDR_6_________________________________ Header body
class CDlg3DView : public CDialog
{
    DECLARE_DYNAMIC(CDlg3DView)

public:
    CDlg3DView(ImageLotView& view, VisionImageLotInsp& imageLotInsp, CWnd* pParent = NULL);
    virtual ~CDlg3DView();

    Ipvm::SurfaceView* m_p3DViewerAgent;
    ImageLotView& m_imageLotView;
    VisionImageLotInsp& m_imageLotInsp;

    enum
    {
        IDD = IDD_3DVIEWER
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedButtonRefresh();
    afx_msg void OnBnClickedButtonExit();
    afx_msg void OnBnClickedCheckMedianFilter();
    BOOL m_bMedianFilter;
    long m_nMedianRepeat;
    afx_msg void OnEnChangeEditMedianRepeat();
    float m_f3DViewScale;
    afx_msg void OnEnChangeEditViewScale();
    long m_nMaskSize;
};
