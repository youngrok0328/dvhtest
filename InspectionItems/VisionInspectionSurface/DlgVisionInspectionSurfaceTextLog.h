#pragma once
#include "resource.h"

// CDlgVisionInspectionSurfaceTextLog 대화 상자입니다.

class CDlgVisionInspectionSurfaceTextLog : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionSurfaceTextLog)

public:
	CDlgVisionInspectionSurfaceTextLog(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionSurfaceTextLog();

public:
	void SetInitDialog();


// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_TEXT_LOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CString m_strText;
	CString m_strTextLog;
	CRichEditCtrl m_RE_Watch;
	void SetLogBoxText(CString strData, CString strLogData);
	afx_msg void OnBnClickedBtnViewLog();
	afx_msg void OnBnClickedBtnSaveLog();
};
