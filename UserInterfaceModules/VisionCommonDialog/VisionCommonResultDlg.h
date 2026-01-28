#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CGridCtrl;
class ImageLotView;
class VisionUnit;
class VisionInspectionResult;

//HDR_6_________________________________ Header body
//
class DPI_VISION_COMMON_DLG_API VisionCommonResultDlg : public CDialog
{
    DECLARE_DYNAMIC(VisionCommonResultDlg)

public:
    VisionCommonResultDlg(CWnd* pParent, const CRect& rtPositionOnParent, VisionUnit& visionUnit,
        ImageLotView& imageLotView, UINT uiDetailButtonClickedMsg); // 표준 생성자입니다.
    virtual ~VisionCommonResultDlg();

    void Refresh(VisionUnit& visionUnit);
    void Refresh();

    bool CollectVisionInspectionResult();
    void ClearVisionInspectionResult();

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg void OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclkGridResult(NMHDR* pNMHDR, LRESULT* pResult);

private:
    const CRect m_rtInitPosition;
    VisionUnit* m_visionUnit;
    ImageLotView& m_imageLotView;
    const UINT m_uiDetailButtonClickedMsg;
    CGridCtrl* m_pGridCtrl;

    std::vector<VisionInspectionResult*> m_vecpVisionInspectionResult;
};
