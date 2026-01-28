#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CDlgRnRReport : public CDialog
{
    DECLARE_DYNAMIC(CDlgRnRReport)

public:
    CDlgRnRReport(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgRnRReport();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DLGRNRREPORT
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    CGridCtrl m_gridRnR;
};
