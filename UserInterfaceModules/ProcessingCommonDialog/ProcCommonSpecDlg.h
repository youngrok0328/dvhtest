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
class VisionInspectionSpec;

//HDR_6_________________________________ Header body
//
class DPI_PROCESSING_COMMON_DLG_API ProcCommonSpecDlg : public CDialog
{
    DECLARE_DYNAMIC(ProcCommonSpecDlg)

public:
    ProcCommonSpecDlg(CWnd* pParent, const CRect& rtPositionOnParent,
        std::vector<VisionInspectionSpec>& inspectionSpecs); // 표준 생성자입니다.
    virtual ~ProcCommonSpecDlg();

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    void SetInitGrid();
    void ResetInitGrid();
    void SelectDisableCoulmn(long i_nIndex);

    afx_msg void OnGridDataChanged(NMHDR* pNotifyStruct, LRESULT* result);

private:
    const CRect m_rtInitPosition;
    std::vector<VisionInspectionSpec>& m_inspectionSpecs;
    CGridCtrl* m_pGridCtrl;
};
