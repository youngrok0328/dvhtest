#pragma once

// CDlgUseMemoryViewer 대화 상자입니다.

class CSystemConfig;
class CNewLabel;
class CVisionUnit;

class CDlgUseMemoryViewer : public CDialog
{
	DECLARE_DYNAMIC(CDlgUseMemoryViewer)

public:
	CDlgUseMemoryViewer(CVisionUnit *pVisinUnit, CSystemConfig* pSysparam, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgUseMemoryViewer();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_USE_MEMORY_VIEWER };

public:
	BOOL SetMemoryBoxControl(long nCount);
	long GetUsingMemorySize(CString strProcessName);

public:
	CVisionUnit *m_pVisinUnit;
	CSystemConfig* m_pSysparam;
	std::vector<CNewLabel *> m_pvecLabel_Memory_Box;

	BOOL m_bTimerMode;
	long m_nThreadNum;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnStart();
};
