#pragma once
#include "resource.h"

class CGridCtrl;
class CImageDisplay;
class CDlgVisionInspectionSurface;

#define IDC_GRID_RESULT 100


// CDlgVisionInspectionSurfaceResult 대화 상자입니다.

class CDlgVisionInspectionSurfaceResult : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionSurfaceResult)

public:
	CDlgVisionInspectionSurfaceResult(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionSurfaceResult();

public:
	CDlgVisionInspectionSurface*						m_pDlgVisionInspectionSurface;
	CGridCtrl*								m_pgridResult;
	std::vector<VisionInspectionResult>*	m_pvecResult;
	CImageDisplay*							m_pImageDisplay;

public:
	void SetInitDialog();
	void SetInitGrid();
	void UpdateResultData();
	void OverlayDetailData(long nID);
	long GetTotalResult();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_RESULT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK() {};
	virtual void OnCancel() {};

	afx_msg void OnClickGridResult(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkGridResult(NMHDR* pNMHDR, LRESULT* pResult);
};
