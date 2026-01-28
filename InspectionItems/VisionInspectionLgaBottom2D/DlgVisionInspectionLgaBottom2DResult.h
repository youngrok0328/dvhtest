#pragma once
#include "resource.h"
#include "../../PI_Modules/dPI_GridCtrl\GridCtrl.h"
#include "afxwin.h"

class CImageDisplay;
class CDlgVisionInspectionLgaBottom2D;

#define IDC_GRID_RESULT 100

// CDlgVisionInspectionLgaBottom2DResult 대화 상자입니다.

class CDlgVisionInspectionLgaBottom2DResult : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionLgaBottom2DResult)

public:
	CDlgVisionInspectionLgaBottom2DResult(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionLgaBottom2DResult();

public:
	CDlgVisionInspectionLgaBottom2D*				m_pDlgVisionInspectionLgaBottom2D;
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
