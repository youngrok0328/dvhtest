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
class DPI_PROCESSING_COMMON_DLG_API ProcCommonResultDlg : public CDialog
{
    DECLARE_DYNAMIC(ProcCommonResultDlg)

public:
    ProcCommonResultDlg(CWnd* pParent, const CRect& rtPositionOnParent,
        VisionInspectionResultGroup& inspectionResultGroup, ImageLotView& imageLotView, const wchar_t* jobName,
        UINT uiDetailButtonClickedMsg); // 표준 생성자입니다.
    virtual ~ProcCommonResultDlg();

    void Refresh();

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
    VisionInspectionResultGroup& m_inspectionResultGroup;
    ImageLotView& m_imageLotView;
    CString m_jobName;
    const UINT m_uiDetailButtonClickedMsg;
    CGridCtrl* m_pGridCtrl;

    bool ModfiyItemidx(long i_nRowidx);
    bool SaveDetailValue_CSVFile(VisionInspectionResultGroup i_VisionInspectionResultGroup, long i_nRowidx);
};
