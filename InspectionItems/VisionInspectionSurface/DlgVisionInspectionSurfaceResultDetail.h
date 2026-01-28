#pragma once
#include "resource.h"
#include "afxwin.h"

class CGridCtrl;
class CImageDisplay;
class CDlgVisionInspectionSurface;

class CDlgVisionInspectionSurfaceResultDetail : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionSurfaceResultDetail)

public:
	CDlgVisionInspectionSurfaceResultDetail(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionSurfaceResultDetail();

public:
	CDlgVisionInspectionSurface*		m_pDlgVisionInspectionSurface;
	CGridCtrl*					m_pgridResultDetail;
	std::vector<VisionInspectionResult>*	m_pvecResult;
	CImageDisplay*				m_pImageDisplay;

	long						m_nCurGrid;

public:
	void SetInitDialog();
	void SetInitGrid();
	void SetInitButton();
	void SetGrid();
	void OverlayDetailData(long nID);

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_RESULT_DETAIL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CButton m_btnBack;

	afx_msg void OnBnClickedBack();
	afx_msg void OnClickGridResultDetail(NMHDR* pNMHDR, LRESULT* pResult);
	
	virtual BOOL OnInitDialog();
	virtual void OnOK() {};
	virtual void OnCancel() {};
};
