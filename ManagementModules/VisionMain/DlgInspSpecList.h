#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CInspectionSpecConfig;

//HDR_6_________________________________ Header body
//
class CDlgInspSpecList : public CDialog
{
    DECLARE_DYNAMIC(CDlgInspSpecList)

public:
    CDlgInspSpecList(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgInspSpecList();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_VISION_INSP_INIT_SPEC
    };

public:
    CInspectionSpecConfig* m_pInspectionSpecINI;

    CGridCtrl m_gridSpecList;
    void SetData(std::vector<VisionInspectionSpec> vecstSpecData);
    std::vector<VisionInspectionSpec> GetData();
    void SetGrid();
    void GetGridData();

    std::vector<VisionInspectionSpec> m_vecstSpecData;

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtnLoadDefault();
    afx_msg void OnBnClickedBtnSetDefault();
    afx_msg void OnGridDataChanged(NMHDR* pNotifyStruct, LRESULT* result);
};
