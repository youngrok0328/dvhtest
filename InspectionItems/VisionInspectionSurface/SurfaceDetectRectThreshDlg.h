#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Para_MaskOrder.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxcmn.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CSurfaceROIEditorDlg;
class VisionScale;

//HDR_6_________________________________ Header body
//
class CSurfaceDetectRectThreshDlg : public CDialog
{
    DECLARE_DYNAMIC(CSurfaceDetectRectThreshDlg)

public:
    CSurfaceDetectRectThreshDlg(const VisionScale& scale, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CSurfaceDetectRectThreshDlg();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_SURFACE_DETECT_RECT_THRESHOLD
    };

protected:
    const VisionScale& m_scale;
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    virtual BOOL OnInitDialog();

protected:
    Para_MaskOrder::MaskInfo* m_pMaskOrder;
    Para_MaskOrder::MaskInfo m_MaskOrderOld;
    std::vector<long> m_veclProfileX;
    std::vector<long> m_veclProfileY;
    BOOL m_bDrawGraph;

public:
    void SetMaskOrder(Para_MaskOrder::MaskInfo* pMaskOder);
    afx_msg void OnBnClickedCancel();
    void uUpdateData(void);

    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    // 그래프를 그리기위한 Image를 전달한다 //kircheis_Hy
    void SetImageData(const Ipvm::Image8u& image, const Ipvm::Point32r2* pptRect_BCU);
    afx_msg void OnPaint();
    void DrawProfileGraphX(CPaintDC& dc);
    void DrawProfileGraphY(CPaintDC& dc);
    void DrawValidOption(CPaintDC& dc);
    afx_msg void OnEnChangeEditMinWidth();
};
