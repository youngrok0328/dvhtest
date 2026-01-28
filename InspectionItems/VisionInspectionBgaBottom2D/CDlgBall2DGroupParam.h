#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "VisionInspectionBgaBottom2DPara.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CDlgVisionInspectionBgaBottom2D;

//HDR_6_________________________________ Header body
//
class CDlgBall2DGroupParam : public CDialog
{
    DECLARE_DYNAMIC(CDlgBall2DGroupParam)

public:
    CDlgBall2DGroupParam(CDlgVisionInspectionBgaBottom2D* i_pCParentDlg, BallLevelParameter i_sGroupBallParam,
        Package::BallTypeDefinition i_sBallTypeDefinition, long i_nBallGroupidx, CWnd* pParent = nullptr);
    virtual ~CDlgBall2DGroupParam();

    enum
    {
        IDD = IDD_DIALOG_PARAMETER
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

private:
    CDlgVisionInspectionBgaBottom2D* m_pCParentDlg;
    CXTPPropertyGrid* m_propertyGrid;
    BallLevelParameter m_sUpdateGroupBallParam;
    BallLevelParameter m_sRetunGroupBallParam;
    Package::BallTypeDefinition m_sCurBallType;
    long m_nBallGroupidx;

private:
    void SetPropertyGridParam();
    void UpdateGroupParameter(bool i_bParamUpdate);

private:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedButtonApply();
    afx_msg void OnBnClickedButtonCancel();
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    void SetReadyOnlyParam(long i_qualityCheckMethod);

public:
    BallLevelParameter GetGroupParam();
};
