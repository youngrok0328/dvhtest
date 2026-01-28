#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;

//HDR_6_________________________________ Header body
//
class CBodySizeEditor : public CDialog
{
    // Construction

public:
    CBodySizeEditor(CPackageSpec& packageSpec, CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    //{{AFX_DATA(CBodySizeEditor)
    enum
    {
        IDD = IDD_DIALOG_BODY_SIZE
    };
    float m_fBodySizeX;
    float m_fBodySizeY;
    float m_fBodySizeZ;

    float m_fRoundRadius;

    BOOL m_bSDCardMode;
    float m_fDistFromLeft;
    float m_fLowFromTop;

    BOOL m_bOctagonMode;
    float m_fDistOctagonChamfer;

    int m_nUnit;

    BOOL m_bDeadBug; //kircheis_20160622
    BOOL m_bChangedDeadBug;

    long m_nRoundPackageOption; //kircheis_RoundPKG
    //}}AFX_DATA

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CBodySizeEditor)

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:
    CPackageSpec& m_packageSpec;

    // Generated message map functions
    //{{AFX_MSG(CBodySizeEditor)
    virtual void OnOK();
    afx_msg void OnRadioUnitMil();
    afx_msg void OnRadioUnitMm();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedCheckSdCard();
    //CComboBox m_CtrlDeviceType;
    afx_msg void OnBnClickedCheckOctagon();
    afx_msg void OnBnClickedCheckDeadBug();
    CComboBox m_ctrlCmbRoundOption;
    float m_fRoundPackageRadius;
    afx_msg void OnCbnSelchangeComboRoundOption();
    BOOL m_bCheckGlassCorePackage;
    float m_fGlassCoreThickness;
    float m_fGlassCoreSubstrateSizeX;
    float m_fGlassCoreSubstrateSizeY;
    float m_fGlassCoreTopSubstrateThickness;
    float m_fGlassCoreBottomSubstrateThickness;
    afx_msg void OnBnClickedCheckGlassCorePackage();
    afx_msg void OnEnChangeEditGlassThickness();
    afx_msg void OnEnChangeEditGlassTopSubstrateThickness();
    afx_msg void OnEnChangeEditGlassBottomSubstrateThickness();
    afx_msg void OnEnChangeEditGlassSubstrateSizeX();
    afx_msg void OnEnChangeEditGlassSubstrateSizeY();
    void SetDlgGlassItemEnable(BOOL bGlassCorePackage);
    BOOL IsValidGlassCorePackageInfo();
};