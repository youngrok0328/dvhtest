#pragma once
#include "resource.h"
#include "../../PI_Modules/dPI_GridCtrl\GridCtrl.h"
#include "../../PI_Modules/dPI_GridCtrl\GridCellCheck2.h"

class CDlgVisionInspectionSurface;

#define IDC_GRID_SPEC 100

// CDlgVisionInspectionSurfaceSpec 대화 상자입니다.

class CDlgVisionInspectionSurfaceSpec : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionSurfaceSpec)

public:
	CDlgVisionInspectionSurfaceSpec(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionSurfaceSpec();

	//변수
public:
	CDlgVisionInspectionSurface*					m_pVisionInspDlg;
	CGridCtrl*							m_pgridSpec;
	std::vector<VisionInspectionSpec>*	m_pvecSpec;

	// 함수
public:
	void SetInitDialog();
	void SetInitGrid();
	void UpdateSpecData();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_SPEC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK() {};
	virtual void OnCancel() {};

	afx_msg void OnGridSurfaceSpecParameterEditEnd(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnGridSurfaceSpecParameterCheckClicked(NMHDR* pNotifyStruct, LRESULT* result);
};
