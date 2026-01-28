#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionUnit;

//HDR_6_________________________________ Header body
//
class DlgOverlayManager : public CDialog
{
    DECLARE_DYNAMIC(DlgOverlayManager)

public:
    DlgOverlayManager(VisionUnit& visionUnit, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~DlgOverlayManager();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_OVERLAY_MANAGER
    };

public:
    CGridCtrl m_grid;
    VisionUnit& m_visionUnit;
    std::vector<long> m_vecnOverlayMode;
    long m_nSpecCount;

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnGridSlideBoxClicked(NMHDR* pNotifyStruct, LRESULT* result);
};
