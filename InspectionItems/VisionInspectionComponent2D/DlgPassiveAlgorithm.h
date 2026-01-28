#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "VisionInspectionComponent2DPara.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/TreeColumn.h"

//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionComponent2D;
class CPassiveAlign;
struct FunctionResult;

//HDR_6_________________________________ Header body
//
enum eDlgPassiveDebugView
{
    DebugView_Start = 0,
    DebugView_SpecPassiveROI = DebugView_Start,
    DebugView_SpecElectROI,
    DebugView_SpecBodyROI,
    DebugView_SpecPADROI,
    DebugView_ResPADROI,
    DebugView_ResPADBaseEdge,
    DebugView_ResPADTopBottomEdge,
    DebugView_RoughAlignSearchROI,
    DebugView_RoughAlignResultROI,
    DebugView_DetailSearchROI,
    DebugView_DetailShortenROI,
    DebugView_DetailShortenEdgePoints,
    DebugView_DetailBodyEdgePoints,
    DebugView_DetailAlignResult,
    DebugView_DetailAlignElect,
    DebugView_DetailAlignBody,
    DebugView_End,
};

static LPCTSTR g_szPassiveDeubgViewName[] = {
    _T("Spec Passive ROI"),
    _T("Spec Elect ROIs"),
    _T("Spec Body ROI"),
    _T("Spec PAD ROIs"),

    _T("Res PAD ROIs"),
    _T("Res PAD Base Edge"),
    _T("Res PAD Top/Bottom Edge"),

    _T("RoughAlign SearchROI"),
    _T("RoughAlign Result"),

    _T("Edge Search ROI"),
    _T("Shorten ROI"),
    _T("Shorten Edge Point"),
    _T("Find EdgePoints"),
    _T("Body Align Result"),
    _T("Passive Align Elect Result"),
    _T("Passive Align Body Result"),
};

class CDlgPassiveAlgorithm : public CDialog
{
    DECLARE_DYNAMIC(CDlgPassiveAlgorithm)

public:
    CDlgPassiveAlgorithm(VisionInspectionComponent2D* pChipAlign, CWnd* pParent = NULL);
    virtual ~CDlgPassiveAlgorithm();

    CPassiveAlign* m_pItem;
    VisionInspectionComponent2D* m_pChipVision;

    enum
    {
        IDD = IDD_GRID_PASSIVEALGO
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    CGridCtrl m_gridPassive;
    CTreeColumn m_TreeEDGEAlignParameter;

    void TeachRun(FunctionResult* result = nullptr);

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

    afx_msg void OnBnClickedButtonTeach();
    afx_msg void ShowImage(long nFrame, sPassive_InfoDB* i_pPassiveInfoDB);

    CStatic m_strName;
    CComboBox m_cmbDebug;

    CXTPPropertyGrid* m_propertyGrid;
    void SetPropertyGrid(sPassive_InfoDB* i_pPassiveInfoDB);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

    afx_msg void OnCbnSelchangeCmbDebug();

private:
    BOOL ShowEdgeSearchROI(sPassive_InfoDB* i_pPassiveInfoDB, int nType);
    void SetDebugText(LPCTSTR i_strCompType, long i_nDebugIndex);
};
