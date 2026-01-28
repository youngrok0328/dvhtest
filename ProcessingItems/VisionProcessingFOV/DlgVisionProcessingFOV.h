#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionProcessingFOV;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionProcessingFOV : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionProcessingFOV)

public:
    CDlgVisionProcessingFOV(const ProcessingDlgInfo& procDlgInfo, VisionProcessingFOV* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionProcessingFOV();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG
    };

    void UpdatePaneOverlay();

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    const ProcessingDlgInfo& m_procDlgInfo;
    VisionProcessingFOV* m_pVisionInsp;
    CXTPPropertyGrid* m_propertyGrid;
    ImageLotView* m_imageLotView;

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg void OnDestroy();
    LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
};