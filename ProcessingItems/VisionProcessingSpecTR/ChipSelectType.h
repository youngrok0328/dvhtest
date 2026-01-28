#pragma once

#include "resource.h"
#include "../../PI_Modules/dPI_DATABASE\PassiveJob.h"

struct SChipInfoMaster;
// CChipSelectType 대화 상자입니다.

class CChipSelectType : public CDialog
{
	DECLARE_DYNAMIC(CChipSelectType)

public:
	CChipSelectType(CPassiveJobInfo *pPassiveJob, std::vector<SChipInfoMaster> &vecChipInfoMaster, long nSelectID, long nSelectType, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CChipSelectType();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_CHIP_SELECT_TYPE };

	CPassiveJobInfo *m_pPassiveJob;
	std::vector<SChipInfoMaster> *pvecChipInfoMaster;

	CComboBox m_ComboChipType;
	CComboBox m_ComboAngle;

	long m_nSelectID;
	long m_nSelectType;
	long nType;
	float fSizeX;
	float fSizeY;
	long nAngle;
	CString strModelName;

	long GetType();
	float GetSizeX();
	float GetSizeY();
	CString GetModelName();
	long GetAngle();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeCmbTypeList();
	afx_msg void OnPaint();
};
