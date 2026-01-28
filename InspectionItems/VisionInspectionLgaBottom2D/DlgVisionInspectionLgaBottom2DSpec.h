#pragma once
#include "resource.h"
#include "../../PI_Modules/dPI_GridCtrl\GridCtrl.h"
#include "../../PI_Modules/dPI_GridCtrl\GridCellCheck2.h"
#include "afxwin.h"

class CDlgVisionInspectionLgaBottom2D;

#define IDC_GRID_SPEC 100

// CDlgVisionInspectionLgaBottom2DSpec 대화 상자입니다.

class CDlgVisionInspectionLgaBottom2DSpec : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionLgaBottom2DSpec)

public:
	CDlgVisionInspectionLgaBottom2DSpec(CPackageSpec &packageSpec, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionLgaBottom2DSpec();

	//변수
public:
	CPackageSpec &m_packageSpec;
	CDlgVisionInspectionLgaBottom2D*			m_pVisionInspDlg;
	CGridCtrl*							m_pgridSpec;
	std::vector<VisionInspectionSpec>*	m_pvecSpec;
	
	// 함수
public:
	void SetInitDialog();
	void SetInitGrid();
	void UpdateSpecData(long nClickRow);

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_SPEC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK() {};
	virtual void OnCancel() {};

	afx_msg void OnGridBallInspectionSpecParameterEditEnd(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnGridBallInspectionSpecParameterCheckClicked(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnDblclkGridResult(NMHDR* pNotifyStruct, LRESULT* result);
};
