#pragma once
#include "resource.h"
#include "../../PI_Modules/dPI_GridCtrl\GridCtrl.h"
#include "afxwin.h"

class CImageDisplay;
class CDlgVisionInspectionLgaBottom2D;
class CPackageSpec;

#define IDC_GRID_RESULT_DETAIL 100

// CDlgVisionInspectionLgaBottom2DResultDetail 대화 상자입니다.

class CDlgVisionInspectionLgaBottom2DResultDetail : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionLgaBottom2DResultDetail)

public:
	CDlgVisionInspectionLgaBottom2DResultDetail(CPackageSpec &packageSpec, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionLgaBottom2DResultDetail();

public:
	CPackageSpec &m_packageSpec;
	CDlgVisionInspectionLgaBottom2D*		m_pDlgVisionInspectionLgaBottom2D;
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
