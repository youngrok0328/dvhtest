#pragma once
#include "resource.h"
#include "../../PI_Modules/dPI_GridCtrl\GridCtrl.h"

class CDlgVisionInspectionLgaBottom2D;
class VisionDebugInfo;
class CImageDisplay;

#define IDC_GRID_DEBUG_INFO 100


// CDlgVisionInspectionLgaBottom2DDebugInfo 대화 상자입니다.

class CDlgVisionInspectionLgaBottom2DDebugInfo : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionLgaBottom2DDebugInfo)

public:
	CDlgVisionInspectionLgaBottom2DDebugInfo(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionLgaBottom2DDebugInfo();

public:
	CDlgVisionInspectionLgaBottom2D*		m_pDlgVisionInspectionLgaBottom2D;
	CGridCtrl*						m_pgridDebugInfo;
	std::vector<VisionDebugInfo>*	m_pvecDebugInfo;
	CImageDisplay*					m_pImageDisplay;

public:
	void SetInitDialog();
	void SetInitGrid();
	void ShowDebugInfo(long nID);
	void UpdateDebugInfoColor();


// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_DEBUG_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	virtual void OnOK() {};
	virtual void OnCancel() {};

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickGridDebugInfo(NMHDR* pNMHDR, LRESULT* pResult);
};
