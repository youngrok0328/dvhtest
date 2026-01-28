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
class VisionInspectionResultGroup;

//HDR_6_________________________________ Header body
//
class DPI_PROCESSING_COMMON_DLG_API ProcCommonDetailResultDlg : public CDialog
{
    DECLARE_DYNAMIC(ProcCommonDetailResultDlg)

public:
    ProcCommonDetailResultDlg() = delete;
    ProcCommonDetailResultDlg(CWnd* pParent, const CRect& rtPositionOnParent,
        VisionInspectionResultGroup& inspectionResultGroup, ImageLotView& imageLotView); // 표준 생성자입니다.
    virtual ~ProcCommonDetailResultDlg();

    void Refresh(long itemIndex = -1);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult);

    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

private:
    const CRect m_rtInitPosition;
    VisionInspectionResultGroup& m_inspectionResultGroup;
    ImageLotView& m_imageLotView;
    CGridCtrl* m_pGridCtrl;
    long m_nCurGrid;

    static int compareGridSort(LPARAM source1, LPARAM source2, LPARAM data);
};
