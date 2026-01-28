#pragma once

#include "../PI_Modules/dPI_GridCtrl\GridCtrl.h"
#include "../VisionCommon/DBObject.h"
#include <map>

class CVisionUnit;
class CVisionMain;

// CDlgVisionInspectionJobEdit 대화 상자입니다.
#define IDC_GRID_INSPITEM		103

class CDlgVisionInspectionJobEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgVisionInspectionJobEdit)

public:
	CDlgVisionInspectionJobEdit(CVisionUnit* pVisionUnit,CVisionMain* pVisionMain,long nModuleID,CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgVisionInspectionJobEdit();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_VISION_INSP_JobEDIT };

	CVisionUnit*                m_pVisionUnit;
	CVisionMain*				m_pVisionMain;
	long	m_nModuleID;

	CGridCtrl*					m_pGirdJobInfo;

	void SetInitGrid();
	void SetGrid();

		void SetFloatGrid(long nItemStartNum);
		void SetLongGrid(long nItemStartNum);
		void SetBOOLGrid(long nItemStartNum);
		void SetCStringLGrid(long nItemStartNum);
		void SetEditPara();

		std::map<CString, long>		m_mapLParameters ; 
		std::map<CString, float>	m_mapFParameters ; 
		std::map<CString, BOOL>		m_mapBParameters ; 
		std::map<CString, CString>  m_mapStrParameters ; 

		CiDataBase m_JobDB_Total;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnGridEditEnd(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnBnClickedButtonImport();
	afx_msg void OnBnClickedButtonExport();
	afx_msg void OnBnClickedButtonCancel();
};
