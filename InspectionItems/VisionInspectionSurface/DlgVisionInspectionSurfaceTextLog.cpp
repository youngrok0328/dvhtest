// DlgVisionInspectionSurfaceTextLog.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VisionInspectionSurface.h"
#include "DlgVisionInspectionSurfaceTextLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDlgVisionInspectionSurfaceTextLog 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgVisionInspectionSurfaceTextLog, CDialog)

CDlgVisionInspectionSurfaceTextLog::CDlgVisionInspectionSurfaceTextLog(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVisionInspectionSurfaceTextLog::IDD, pParent)
	, m_strText(_T(""))
{
	AfxInitRichEdit();
}

CDlgVisionInspectionSurfaceTextLog::~CDlgVisionInspectionSurfaceTextLog()
{
}

void CDlgVisionInspectionSurfaceTextLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT22, m_RE_Watch);
	DDX_Text(pDX, IDC_EDIT_LOG, m_strText);
}


BEGIN_MESSAGE_MAP(CDlgVisionInspectionSurfaceTextLog, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_VIEW_LOG, &CDlgVisionInspectionSurfaceTextLog::OnBnClickedBtnViewLog)
	ON_BN_CLICKED(IDC_BTN_SAVE_LOG, &CDlgVisionInspectionSurfaceTextLog::OnBnClickedBtnSaveLog)
END_MESSAGE_MAP()


// CDlgVisionInspectionSurfaceTextLog 메시지 처리기입니다.

BOOL CDlgVisionInspectionSurfaceTextLog::OnInitDialog()
{
	CDialog::OnInitDialog();
	CHARFORMAT cf;
	m_RE_Watch.GetDefaultCharFormat(cf);
	cf.dwMask = CFM_FACE;
	_tcscpy_s(cf.szFaceName, 20, _T("FixedSys"));
	m_RE_Watch.SetDefaultCharFormat(cf);
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgVisionInspectionSurfaceTextLog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CRect rtRect;
	GetClientRect(rtRect);

	CRect rtSize(0, 0, cx, cy - 30);
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_LOG);
	CButton* pButtonViewLog = (CButton*)GetDlgItem(IDC_BTN_VIEW_LOG);
	CButton* pButtonSaveLog = (CButton*)GetDlgItem(IDC_BTN_SAVE_LOG);

	if(pEdit->GetSafeHwnd())
	{
		pEdit->MoveWindow(rtSize);
		m_RE_Watch.MoveWindow(rtSize);
		pEdit->ShowWindow(SW_HIDE);
	}

	if(pButtonViewLog->GetSafeHwnd())
	{
		rtRect.left = rtRect.right - 150;
		rtRect.top += 210;
		rtRect.right = rtRect.left + 70;
		rtRect.bottom = rtRect.top + 25;

		pButtonViewLog->MoveWindow(rtRect);
	}

	if(pButtonSaveLog->GetSafeHwnd())
	{
		rtRect.left += 75;
		rtRect.right = rtRect.left + 70;
		pButtonSaveLog->MoveWindow(rtRect);
	}
}

void CDlgVisionInspectionSurfaceTextLog::SetInitDialog()
{
}

void CDlgVisionInspectionSurfaceTextLog::SetLogBoxText(CString strData, CString strLogData)
{
	m_strText = strData;
	m_strTextLog = strLogData;

	//UpdateData(FALSE); // 요고 하면 어서트 발생 아래처럼써야 안남 
	GetDlgItem(IDC_EDIT_LOG)->SetWindowText(strData);

	m_RE_Watch.SetWindowText(_T(""));


	m_RE_Watch.ReplaceSel( strData );
}

void CDlgVisionInspectionSurfaceTextLog::OnBnClickedBtnViewLog()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CDlgVisionInspectionSurfaceTextLog::OnBnClickedBtnSaveLog()
{
	if(m_strTextLog == "")
	{
		AfxMessageBox(_T("Do not Have Text Log."), MB_OK);
		return;
	}

	CString strFilename;

	CFileDialog Dlg(FALSE, _T("csv"), NULL, OFN_NOCHANGEDIR|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, _T("CSV File Format (*.csv)|*.csv||"));
	if(Dlg.DoModal() == IDOK)
	{
		strFilename = Dlg.GetPathName();
		if( Dlg.GetFileExt() == "" )
		{
			strFilename += ".csv";
		}
	}
	else
	{
		return;
	}

	FILE* fp = NULL;
	_tfopen_s(&fp, strFilename,_T("w"));

	if(fp != NULL)
	{
		fprintf(fp, "%s\n", LPCSTR(CStringA(m_strTextLog))); // 원래 m_strTextLog 이지만 지금 임시로 m_strText를 저장한다.
		fclose(fp);
		AfxMessageBox(_T("Save Complete!"), MB_OK);
	}
	else
	{
		AfxMessageBox(_T("Save Error!"), MB_OK);
	}
}
