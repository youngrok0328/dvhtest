#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionLgaBottom2DPara;
class LandPara;

//HDR_6_________________________________ Header body
//
class DlgLandParameter : public CDialog
{
    DECLARE_DYNAMIC(DlgLandParameter)

public:
    DlgLandParameter(VisionInspectionLgaBottom2DPara* VisionPara, long GroupIndex, CWnd* pParent = nullptr);
    virtual ~DlgLandParameter();

#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DLG_LAND_PARAMETER
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

private:
    VisionInspectionLgaBottom2DPara* m_VisionPara;
    CXTPPropertyGrid* m_propertyGrid;
    LandPara* m_LandParameter;
    long m_nGroupIndex;

private:
    void UpdatePropertyGrid();
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    virtual BOOL OnInitDialog();
};
