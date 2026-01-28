#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/ChipInfoCollection.h"

//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ChipInfo;
class VisionMapDataEditorUI;

//HDR_6_________________________________ Header body
//
class CDlgChangeDimension : public CDialog
{
    DECLARE_DYNAMIC(CDlgChangeDimension)

public:
    CDlgChangeDimension(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgChangeDimension();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_CHANGE_DIMENSION
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:

private:
    CComboBox m_cmbDimension;
    long m_nDMSType;
    std::vector<Chip::ChipInfo*> m_vecSelChipInfo;
    VisionMapDataEditorUI* m_pVisionMapDataEditorUI;

public:
    virtual INT_PTR DoModal(
        std::vector<Chip::ChipInfo*> i_vecSelChipInfo, long i_nDMSType, VisionMapDataEditorUI* i_pMapDataEditorUI);
    CString m_strCompType;
    float m_fCompWidth;
    float m_fCompLength;
    float m_fCompHeight;
    long m_nCompAngle;

public: //Evnet
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
};
