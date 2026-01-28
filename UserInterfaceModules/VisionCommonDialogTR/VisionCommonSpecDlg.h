#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CGridCtrl;
class VisionUnit;

//HDR_6_________________________________ Header body
//
class DPI_VISION_COMMON_DLG_API VisionCommonSpecDlg : public CDialog
{
    DECLARE_DYNAMIC(VisionCommonSpecDlg)

public:
    VisionCommonSpecDlg(CWnd* pParent, const CRect& rtPositionOnParent, VisionUnit& visionUnit); // 표준 생성자입니다.
    virtual ~VisionCommonSpecDlg();

    void Refresh();

protected:
    const CRect m_rtInitPosition;
    VisionUnit& m_visionUnit;
    CGridCtrl* m_pGridCtrl;

    void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV 지원입니다.
    BOOL OnInitDialog() override;
    void OnOK() override {};
    void OnCancel() override {};
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};
