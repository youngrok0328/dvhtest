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
class DPI_VISION_COMMON_DLG_API VisionCommonDetailResultDlg : public CDialog
{
    DECLARE_DYNAMIC(VisionCommonDetailResultDlg)

public:
    VisionCommonDetailResultDlg() = delete;
    VisionCommonDetailResultDlg(CWnd* pParent, const CRect& rtPositionOnParent, VisionUnit& visionUnit,
        ImageLotView& imageLotView); // 표준 생성자입니다.
    virtual ~VisionCommonDetailResultDlg();

    void SetTarget(VisionUnit& visionUnit);
    bool Refresh(long itemIndex = -1);

    bool CollectVisionInspectionResult();
    void ClearVisionInspectionResult();

protected:
    const CRect m_rtInitPosition;
    VisionUnit* m_visionUnit;
    ImageLotView& m_imageLotView;
    CGridCtrl* m_pGridCtrl;
    long m_nCurGrid;

    std::vector<VisionInspectionResult*> m_vecpVisionInspectionResult;

    void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV 지원입니다.
    BOOL OnInitDialog() override;
    void OnOK() override {};
    void OnCancel() override {};
    afx_msg void OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};
