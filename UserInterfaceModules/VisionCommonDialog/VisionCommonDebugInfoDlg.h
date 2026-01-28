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
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult);

private:
    CGridCtrl* m_pGridCtrl;
    const CRect m_rtInitPosition;
    VisionUnit* m_visionUnit;
    ImageLotView& m_imageLotView;
    VisionCommonTextResultDlg& m_textResultDlg;
};
