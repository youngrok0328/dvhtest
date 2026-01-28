#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CGridCtrl;
class ImageLotView;
class VisionCommonTextResultDlg;
class VisionUnit;
class VisionDebugInfo;

//HDR_6_________________________________ Header body
//
class DPI_VISION_COMMON_DLG_API VisionCommonDebugInfoDlg : public CDialog
{
    DECLARE_DYNAMIC(VisionCommonDebugInfoDlg)

public:
    VisionCommonDebugInfoDlg() = delete;
    VisionCommonDebugInfoDlg(CWnd* pParent, const CRect& rtPositionOnParent, VisionUnit& visionUnit,
        ImageLotView& imageLotView, VisionCommonTextResultDlg& textResultDlg);
    virtual ~VisionCommonDebugInfoDlg();

    void Refresh(VisionUnit& visionUnit);
    void Refresh();

protected:
    CGridCtrl* m_pGridCtrl;
    const CRect m_rtInitPosition;
    VisionUnit* m_visionUnit;
    ImageLotView& m_imageLotView;
    VisionCommonTextResultDlg& m_textResultDlg;

    void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV 지원입니다.
    BOOL OnInitDialog() override;
    void OnOK() override {};
    void OnCancel() override {};

    afx_msg void OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};
