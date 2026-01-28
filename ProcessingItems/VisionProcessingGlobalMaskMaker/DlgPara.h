#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxdialogex.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionProcessingGlobalMaskMaker;
class CVisionProcessingGlobalMaskMakerPara;
class CDlgSetupUI;
class DlgParaSingle;
class SpecLayer;

//HDR_6_________________________________ Header body
//
class DlgPara : public CDialogEx
{
    DECLARE_DYNAMIC(DlgPara)

public:
    DlgPara(const CRect& dialogArea, VisionProcessingGlobalMaskMaker* pVisionInsp,
        CDlgSetupUI* parent); // standard constructor
    virtual ~DlgPara();

    CDlgSetupUI* m_parent;
    BOOL m_displayOtherLayerMask;

    void UpdateLayerMaskList();
    long GetSelectLayerIndex() const;
    SpecLayer* GetSelectSpecLayer();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_PARA
    };
#endif

protected:
    VisionProcessingGlobalMaskMaker* m_pVisionInsp;
    CVisionProcessingGlobalMaskMakerPara* m_pVisionPara;
    DlgParaSingle* m_subSingle;

    const CRect& m_dialogArea;
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    CEdit m_editLayerName;

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
};
