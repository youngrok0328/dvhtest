#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "MaskListCtrl.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxdialogex.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CVisionProcessingGlobalMaskMakerPara;

//HDR_6_________________________________ Header body
//
class DlgImportLayer : public CDialogEx
{
    DECLARE_DYNAMIC(DlgImportLayer)

public:
    DlgImportLayer(CVisionProcessingGlobalMaskMakerPara& para, CVisionProcessingGlobalMaskMakerPara& importPara,
        CWnd* pParent = nullptr); // standard constructor
    virtual ~DlgImportLayer();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_IMPORT_LAYER
    };
#endif

protected:
    CVisionProcessingGlobalMaskMakerPara& m_para;
    CVisionProcessingGlobalMaskMakerPara& m_importPara;

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    CComboBox m_cmbImportType;
    MaskListCtrl m_layerList;

    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnCbnSelchangeComboImportType();
};
