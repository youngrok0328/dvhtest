#pragma once
#include "resource.h"

class CGridCtrl;
class CDlgVisionInspectionSurface;
class VisionDebugInfo;
class CImageDisplay;

#define IDC_GRID_DEBUG_INFO 100


// CDlgVisionInspectionSurfaceDebugInfo 대화 상자입니다.

class CDlgVisionInspectionSurfaceDebugInfo : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionSurfaceDebugInfo)

public:
	CDlgVisionInspectionSurfaceDebugInfo(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionSurfaceDebugInfo();

public:
	CDlgVisionInspectionSurface*				m_pDlgVisionInspectionSurface;
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

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK() {};
	virtual void OnCancel() {};

	afx_msg void OnClickGridDebugInfo(NMHDR* pNMHDR, LRESULT* pResult);
};
