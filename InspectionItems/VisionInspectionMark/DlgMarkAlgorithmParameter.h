#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/TreeColumn.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionMarkPara;

//HDR_6_________________________________ Header body
//
class CDlgMarkAlgorithmParameter : public CDialog
{
    DECLARE_DYNAMIC(CDlgMarkAlgorithmParameter)

public:
    CDlgMarkAlgorithmParameter(
        VisionInspectionMarkPara* pVisionInspectionMarkPara, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgMarkAlgorithmParameter();

    virtual BOOL OnInitDialog();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_ALGORITHM_PARAMETER
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    VisionInspectionMarkPara* m_pVisionInspectionMarkPara;

    CXTPPropertyGrid* m_propertyGrid;

    void UpdatePropertyGrid();
    CGridCtrl m_gridMarkAlgorithmParameter;
    CTreeColumn m_TreeColumnMarkAlgorithmParameter;

    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
};
