#pragma once

#include "resource.h"
// CDlgAddPad 대화 상자입니다.

class CDlgAddPad : public CDialog
{
	DECLARE_DYNAMIC(CDlgAddPad)

public:
	CDlgAddPad(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgAddPad();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_ADD_PAD };

	CString m_strName;
	float m_fOffsetX;
	float m_fOffsetY;
	float m_fWidth;
	float m_fLength;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
