#pragma once
#include "afxwin.h"

#define TYPE1 0
#define TYPE2 1
#define TYPE3 2
#define DEGREE1 3
#define DEGREE2 4

// CChip_DB_Edit 대화 상자입니다.
class CPassiveJobInfo;
class ChipDB;

class CChip_DB_Edit : public CDialog
{
	DECLARE_DYNAMIC(CChip_DB_Edit)

public:
	CChip_DB_Edit(CPassiveJobInfo *pPassiveJob, long nSelectID, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CChip_DB_Edit();
	
	CPassiveJobInfo *m_pPassiveJob;
	long m_nSelectID;

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_CHIP_DB_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	// 영훈 : Backup용..
	std::vector<ChipDB> m_vecsChipData_Backup;

	CComboBox m_ComboChipType;
	int m_nSelectTypeID;

	BOOL ValidDataCheck(long &nDataSize);
	void SetCombobox(long nCursel = 0);

	void Save();
	void Open();

public:
	afx_msg void OnCbnSelchangeComboType();
	afx_msg void OnCbnSelchangeComboDegree();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonSave();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnColor();
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedCancel();
};
