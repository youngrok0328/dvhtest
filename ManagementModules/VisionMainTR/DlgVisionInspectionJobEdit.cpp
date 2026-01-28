// DlgVisionInspectionJobEdit.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "resource.h"
#include "VisionMain.h"

#include "DlgVisionInspectionJobEdit.h"
#include <map>
#include "../VisionUnit/VisionUnit.h"
#include "../VisionCommon/VisionProcessing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDlgVisionInspectionJobEdit, CDialog)
enum
{
	FLAOT,
	LONGG,
	BOOLL,
	CSTRING,
	CRECT,
};

const LPCTSTR gl_szType[] =
{
	_T("float"),
	_T("long"),
	_T("BOOL"),
	_T("CString"),
	_T("CRect"),
};

CDlgVisionInspectionJobEdit::CDlgVisionInspectionJobEdit(CVisionUnit* pVisionUnit, CVisionMain* pVisionMain, long nModuleID, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVisionInspectionJobEdit::IDD, pParent)
{
	m_pVisionUnit = pVisionUnit;
	m_pVisionMain = pVisionMain;
	m_nModuleID = nModuleID;
	m_pGirdJobInfo = NULL;
}

CDlgVisionInspectionJobEdit::~CDlgVisionInspectionJobEdit()
{
	delete m_pGirdJobInfo;
}

void CDlgVisionInspectionJobEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

}

BOOL CDlgVisionInspectionJobEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_pGirdJobInfo == NULL)
	{
		m_pGirdJobInfo = new CGridCtrl;

	}

	SetInitGrid();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BEGIN_MESSAGE_MAP(CDlgVisionInspectionJobEdit, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDlgVisionInspectionJobEdit::OnBnClickedButtonApply)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_INSPITEM, OnGridEditEnd)
	ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_INSPITEM, OnGridEditEnd)

	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CDlgVisionInspectionJobEdit::OnBnClickedButtonImport)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &CDlgVisionInspectionJobEdit::OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgVisionInspectionJobEdit::OnBnClickedButtonCancel)
END_MESSAGE_MAP()


void CDlgVisionInspectionJobEdit::SetInitGrid()
{
	CRect rect;
	GetClientRect(rect);


	CString strFontName = _T("Arial");
	CFont fontGrid;
	fontGrid.CreateFont(20, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, strFontName);

	CRect rtJobInfo;

	long nEachWidth = 370;
	long nEachHeight = 399 - 65;

	long nWidth = 250 + 332;
	long nHeight = 500 + 65;

	MoveWindow(0, 0, nWidth, nHeight);


	rtJobInfo.top = 0;
	rtJobInfo.left = 0;
	rtJobInfo.right = nEachWidth + 200;
	rtJobInfo.bottom = nEachHeight + 150;
	m_pGirdJobInfo->Create(rtJobInfo, this, IDC_GRID_INSPITEM);
	m_pGirdJobInfo->SetEditable(TRUE);
	m_pGirdJobInfo->EnableDragAndDrop(FALSE);

	SetGrid();
}

void CDlgVisionInspectionJobEdit::SetFloatGrid(long nItemStartNum)
{
	long nSpecNum = (long)m_mapFParameters.size();//(mapFParameters.size());

	std::map<CString, float>::const_iterator itr = m_mapFParameters.begin();//mapFParameters.begin();

	std::vector<CString> vecNames;
	std::vector<float> vecValues;
	while (itr != m_mapFParameters.end())//mapFParameters.end())
	{
		vecNames.push_back(LPCTSTR(itr->first));
		vecValues.push_back(itr->second);
		itr++;
	}

	CString str;
	long nIDex = 0;
	for (long i = nItemStartNum; i < nItemStartNum + nSpecNum; i++)
	{
		// ID
		m_pGirdJobInfo->SetItemBkColour(i + 1, 0, RGB(250, 250, 200));
		str.Format(_T("%d"), i);
		m_pGirdJobInfo->SetItemText(i + 1, 0, str);

		// Name
		CString strName = vecNames[nIDex];
		m_pGirdJobInfo->SetItemText(i + 1, 1, strName);

		// Value
		CString strValue;
		strValue.Format(_T("%2.2f"), vecValues[nIDex]);
		m_pGirdJobInfo->SetItemText(i + 1, 2, strValue);

		// Type
		m_pGirdJobInfo->SetItemText(i + 1, 3, gl_szType[FLAOT]);
		m_pGirdJobInfo->SetItemBkColour(i + 1, 3, RGB(255, 128, 128));

		m_pGirdJobInfo->GetCell(i + 1, 0)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 1)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 2)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 3)->SetFormat(DT_CENTER);

		m_pGirdJobInfo->SetItemState(i + 1, 1, GVIS_READONLY);
		m_pGirdJobInfo->SetItemState(i + 1, 3, GVIS_READONLY);

		nIDex++;
	}
}
void CDlgVisionInspectionJobEdit::SetBOOLGrid(long nItemStartNum)
{
	long nSpecNum = (long)m_mapBParameters.size();//(mapBParameters.size());

	std::map<CString, BOOL>::const_iterator itr = m_mapBParameters.begin();//mapBParameters.begin();

	std::vector<CString> vecNames;
	std::vector<BOOL> vecValues;
	while (itr != m_mapBParameters.end())//mapBParameters.end())
	{
		vecNames.push_back(LPCTSTR(itr->first));
		vecValues.push_back(itr->second);
		itr++;
	}

	CString str;
	long nIDex = 0;
	for (long i = nItemStartNum; i < nItemStartNum + nSpecNum; i++)
	{
		// ID
		m_pGirdJobInfo->SetItemBkColour(i + 1, 0, RGB(250, 250, 200));
		str.Format(_T("%d"), i);
		m_pGirdJobInfo->SetItemText(i + 1, 0, str);

		// Name
		CString strName = vecNames[nIDex];
		m_pGirdJobInfo->SetItemText(i + 1, 1, strName);

		// Value
		CString strValue;
		strValue.Format(_T("%d"), vecValues[nIDex]);
		m_pGirdJobInfo->SetItemText(i + 1, 2, strValue);

		// Type
		m_pGirdJobInfo->SetItemText(i + 1, 3, gl_szType[BOOLL]);
		m_pGirdJobInfo->SetItemBkColour(i + 1, 3, RGB(128, 128, 255));

		m_pGirdJobInfo->GetCell(i + 1, 0)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 1)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 2)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 3)->SetFormat(DT_CENTER);

		m_pGirdJobInfo->SetItemState(i + 1, 1, GVIS_READONLY);
		m_pGirdJobInfo->SetItemState(i + 1, 3, GVIS_READONLY);


		nIDex++;
	}
}

void CDlgVisionInspectionJobEdit::SetLongGrid(long nItemStartNum)
{
	long nSpecNum = (long)m_mapLParameters.size();//(mapLParameters.size());

	std::map<CString, long>::const_iterator itr = m_mapLParameters.begin();//mapLParameters.begin();

	std::vector<CString> vecNames;
	std::vector<long> vecValues;
	while (itr != m_mapLParameters.end())//mapLParameters.end())
	{
		vecNames.push_back(LPCTSTR(itr->first));
		vecValues.push_back(itr->second);
		itr++;
	}

	CString str;
	long nIDex = 0;
	for (long i = nItemStartNum; i < nItemStartNum + nSpecNum; i++)
	{
		// ID
		m_pGirdJobInfo->SetItemBkColour(i + 1, 0, RGB(250, 250, 200));
		str.Format(_T("%d"), i);
		m_pGirdJobInfo->SetItemText(i + 1, 0, str);

		// Name
		CString strName = vecNames[nIDex];
		m_pGirdJobInfo->SetItemText(i + 1, 1, strName);

		// Value
		CString strValue;
		strValue.Format(_T("%d"), vecValues[nIDex]);
		m_pGirdJobInfo->SetItemText(i + 1, 2, strValue);

		// Type
		m_pGirdJobInfo->SetItemText(i + 1, 3, gl_szType[LONGG]);
		m_pGirdJobInfo->SetItemBkColour(i + 1, 3, RGB(128, 255, 128));

		m_pGirdJobInfo->GetCell(i + 1, 0)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 1)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 2)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 3)->SetFormat(DT_CENTER);

		m_pGirdJobInfo->SetItemState(i + 1, 1, GVIS_READONLY);
		m_pGirdJobInfo->SetItemState(i + 1, 3, GVIS_READONLY);

		nIDex++;
	}
}
void CDlgVisionInspectionJobEdit::SetCStringLGrid(long nItemStartNum)
{
	long nSpecNum = (long)m_mapStrParameters.size();//(mapStrParameters.size());

	std::map<CString, CString>::const_iterator itr = m_mapStrParameters.begin();//mapStrParameters.begin();

	std::vector<CString> vecNames;
	std::vector<CString> vecValues;
	while (itr != m_mapStrParameters.end())//mapStrParameters.end())
	{
		vecNames.push_back(LPCTSTR(itr->first));
		vecValues.push_back(itr->second);
		itr++;
	}

	CString str;
	long nIDex = 0;
	for (long i = nItemStartNum; i < nItemStartNum + nSpecNum; i++)
	{
		// ID
		m_pGirdJobInfo->SetItemBkColour(i + 1, 0, RGB(250, 250, 200));
		str.Format(_T("%d"), i);
		m_pGirdJobInfo->SetItemText(i + 1, 0, str);

		// Name
		CString strName = vecNames[nIDex];
		m_pGirdJobInfo->SetItemText(i + 1, 1, strName);

		// Value
		CString strValue;
		strValue.Format(_T("%s"), (LPCTSTR)vecValues[nIDex]);
		m_pGirdJobInfo->SetItemText(i + 1, 2, strValue);

		// Type
		m_pGirdJobInfo->SetItemText(i + 1, 3, gl_szType[CSTRING]);
		m_pGirdJobInfo->SetItemBkColour(i + 1, 3, RGB(255, 255, 0));


		m_pGirdJobInfo->GetCell(i + 1, 0)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 1)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 2)->SetFormat(DT_CENTER);
		m_pGirdJobInfo->GetCell(i + 1, 3)->SetFormat(DT_CENTER);

		m_pGirdJobInfo->SetItemState(i + 1, 1, GVIS_READONLY);
		m_pGirdJobInfo->SetItemState(i + 1, 3, GVIS_READONLY);

		nIDex++;
	}
}

void CDlgVisionInspectionJobEdit::SetEditPara()
{
	m_mapLParameters = m_pVisionUnit->GetVisionProcessing(m_nModuleID)->GetParaLong();
	m_mapFParameters = m_pVisionUnit->GetVisionProcessing(m_nModuleID)->GetParaFloat();
	m_mapBParameters = m_pVisionUnit->GetVisionProcessing(m_nModuleID)->GetParaBOOL();
	m_mapStrParameters = m_pVisionUnit->GetVisionProcessing(m_nModuleID)->GetParaCString();

}
void CDlgVisionInspectionJobEdit::SetGrid()
{
	UpdateData(TRUE);

	// 간헐적으로 Insp Para, Alogorithm Para로 구분 되어 있는 경우가 있다
	// 이미 만들어진 Job이므로 따로 분리 해서 관리 해야 한다
	SetEditPara();

	long nFloatSpecNum = (long)m_mapFParameters.size();
	long nLongSpecNum = (long)m_mapLParameters.size();
	long nBOOLSpecNum = (long)m_mapBParameters.size();
	long nStringSpecNum = (long)m_mapStrParameters.size();

	m_pGirdJobInfo->SetColumnCount(4);
	m_pGirdJobInfo->SetRowCount(nFloatSpecNum + nLongSpecNum + nBOOLSpecNum + nStringSpecNum + 1);
	m_pGirdJobInfo->SetFixedRowCount(1);
	m_pGirdJobInfo->SetFixedColumnCount(1);
	m_pGirdJobInfo->SetEditable(TRUE);

	long nInspTotal = 0;
	SetFloatGrid(nInspTotal);
	SetLongGrid(nInspTotal + nFloatSpecNum);			nInspTotal += nFloatSpecNum;
	SetBOOLGrid(nInspTotal + nLongSpecNum);			nInspTotal += nLongSpecNum;
	SetCStringLGrid(nInspTotal + nBOOLSpecNum);		nInspTotal += nBOOLSpecNum;

	// 0 번째 Row 에 각각의 Title 을 정의
	CString strModule = m_pVisionUnit->GetVisionProcessing(m_nModuleID)->m_strModuleName;
	m_pGirdJobInfo->SetItemText(0, 0, _T("No"));
	m_pGirdJobInfo->GetCell(0, 0)->SetFormat(DT_CENTER);
	m_pGirdJobInfo->SetItemText(0, 1, strModule + _T(" Parameter Name"));
	m_pGirdJobInfo->GetCell(0, 1)->SetFormat(DT_CENTER);
	m_pGirdJobInfo->SetItemText(0, 2, _T("Parameter Value"));
	m_pGirdJobInfo->GetCell(0, 2)->SetFormat(DT_CENTER);
	m_pGirdJobInfo->SetItemText(0, 3, _T("Parameter Type"));
	m_pGirdJobInfo->GetCell(0, 3)->SetFormat(DT_CENTER);

	m_pGirdJobInfo->SetItemBkColour(0, 0, RGB(200, 200, 250));
	m_pGirdJobInfo->SetItemBkColour(0, 1, RGB(200, 200, 250));
	m_pGirdJobInfo->SetColumnWidth(0, 45);
	m_pGirdJobInfo->SetColumnWidth(1, 260);

	// Grid 정렬
	m_pGirdJobInfo->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_pGirdJobInfo->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_pGirdJobInfo->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	UpdateData(FALSE);
}

void CDlgVisionInspectionJobEdit::OnGridEditEnd(NMHDR* pNotifyStruct, LRESULT* result)
{
	//NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*) pNotifyStruct;

	//long row = pNotify->iRow - 1;
	//long column = pNotify->iColumn - 1;	

	*result = 0;
}

// Open
void CDlgVisionInspectionJobEdit::OnBnClickedButtonImport()
{
	CString strModlue = m_pVisionUnit->GetVisionProcessing(m_nModuleID)->m_strModuleName;
	::CreateDirectory(JOBPARA_DIRECTORY + strModlue, NULL);

	CFileDialog Dlg(TRUE, _T("item"), JOBPARA_DIRECTORY + strModlue + _T("\\*.item"), OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY
		, strModlue + _T(" Para File Format (*.sitem)|*.item||"));


	CString sPath = JOBPARA_DIRECTORY + strModlue + _T("\\");

	Dlg.m_ofn.lpstrInitialDir = sPath;

	// TODO : 이현민 - 힙 사용하기
	TCHAR filenamebuff[10000] = _T("");
	Dlg.m_ofn.lpstrFile = filenamebuff;
	Dlg.m_ofn.nMaxFile = 10000;

	if (Dlg.DoModal() != IDOK)
		return;


	// 일부러 Job을 두번 불러 주는 것임(수정 금지) -  inteksjs
	for (long i = 0; i < 2; i++)
	{
		if (!m_pVisionUnit->GetVisionProcessing(m_nModuleID)->LinkDataBasebyEdit(FALSE, *m_pVisionMain->m_JobDB_Total, m_pVisionUnit->GetVisionProcessing(m_nModuleID)->m_strModuleName, Dlg.GetPathName()))
		{
			AfxMessageBox(_T("Job is Crash"));
		}
	}


	SetGrid();
	m_pGirdJobInfo->Refresh();
}

// Save
void CDlgVisionInspectionJobEdit::OnBnClickedButtonExport()
{
	CString strModlue = m_pVisionUnit->GetVisionProcessing(m_nModuleID)->m_strModuleName;
	::CreateDirectory(JOBPARA_DIRECTORY + strModlue, NULL);

	CFileDialog Dlg(FALSE, _T("item"), JOBPARA_DIRECTORY + strModlue + _T("\\*.item"), OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY
		, strModlue + _T(" Para File Format (*.sitem)|*.item||"));

	CString sPath = JOBPARA_DIRECTORY + strModlue + _T("\\");

	Dlg.m_ofn.lpstrInitialDir = sPath;

	// TODO : 이현민 - 힙 사용하기
	TCHAR filenamebuff[10000] = _T("");
	Dlg.m_ofn.lpstrFile = filenamebuff;
	Dlg.m_ofn.nMaxFile = 10000;

	if (Dlg.DoModal() != IDOK)
		return;

	if (!m_pVisionUnit->GetVisionProcessing(m_nModuleID)->LinkDataBasebyEdit(TRUE, *m_pVisionMain->m_JobDB_Total, m_pVisionUnit->GetVisionProcessing(m_nModuleID)->m_strModuleName, Dlg.GetPathName()))
	{
		AfxMessageBox(_T("Job is Crash"));
	}

}

void CDlgVisionInspectionJobEdit::OnBnClickedButtonApply()
{
	CString strParaName;
	int nTemp;
	long lTemp;
	CString strtemp;
	float ftemp;
	CString nType;
	BOOL bTemp;

	long nRowCount = m_pGirdJobInfo->GetRowCount();
	for (long nRow = 0; nRow < nRowCount; nRow++)
	{
		CString strParaName = m_pGirdJobInfo->GetItemText(nRow + 1, 1);
		nTemp = _ttoi(m_pGirdJobInfo->GetItemText(nRow + 1, 2));
		lTemp = _ttoi(m_pGirdJobInfo->GetItemText(nRow + 1, 2));
		strtemp = m_pGirdJobInfo->GetItemText(nRow + 1, 2);
		ftemp = (float)_ttof(m_pGirdJobInfo->GetItemText(nRow + 1, 2));
		nType = m_pGirdJobInfo->GetItemText(nRow + 1, 3);
		bTemp = nTemp == 1 ? TRUE : FALSE;

		auto* visionProcessing = m_pVisionUnit->GetVisionProcessing(m_nModuleID);

		if (nType == gl_szType[FLAOT])
		{
			visionProcessing->SetParaFloat(strParaName, ftemp);
		}

		if (nType == gl_szType[LONGG])
		{
			visionProcessing->SetParaLong(strParaName, lTemp);
		}

		if (nType == gl_szType[BOOLL])
		{
			visionProcessing->SetParaBOOL(strParaName, bTemp);
		}

		if (nType == gl_szType[CSTRING])
		{
			visionProcessing->SetParaCString(strParaName, strtemp);
		}
	}

	const CString strModule = m_pVisionUnit->GetVisionProcessing(m_nModuleID)->m_strModuleName;
	const CString strJobname = m_pVisionUnit->m_strJobFilePath;

	// Job을 저장후 다시 전부 뿌려 줘야 한다
	// Job은 각자 모듈에 멤버로 갖고 있기 때문에 멤버의 값을 바꿔 줘야 하기 때문이다
	m_pVisionUnit->GetVisionProcessing(m_nModuleID)->LinkDataBasebyEdit(TRUE, *m_pVisionMain->m_JobDB_Total, strModule, strJobname, TRUE);

	if (!m_JobDB_Total.Load(strJobname))
	{
		AfxMessageBox(_T("Job is Crash"));
	}

	if (!m_pVisionUnit->LinkDataBase(FALSE, m_JobDB_Total, strJobname, FALSE))
	{
		AfxMessageBox(_T("Job is Crash"));
	}

	OnOK();
}

void CDlgVisionInspectionJobEdit::OnBnClickedButtonCancel()
{
	OnCancel();
}
