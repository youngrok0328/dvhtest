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
class VisionMainTR;
class ImageViewEx;

//HDR_6_________________________________ Header body
//
class DlgVisionIllumSetup3D : public CDialog
{
    DECLARE_DYNAMIC(DlgVisionIllumSetup3D)

public:
    DlgVisionIllumSetup3D(VisionMainTR& visionMain, CWnd* pParent); // 표준 생성자입니다.
    virtual ~DlgVisionIllumSetup3D();

    VisionMainTR& m_visionMain;
    ImageViewEx* m_imageView;
    CXTPPropertyGrid* m_propertyGrid;

    int m_profileOverlayMode;
    long m_sharpnessProfileScaling;
    long m_intensityScaling;

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_ILLUM_SETUP_3D
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg LRESULT OnGrabEndEvent(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg void OnDestroy();
};
