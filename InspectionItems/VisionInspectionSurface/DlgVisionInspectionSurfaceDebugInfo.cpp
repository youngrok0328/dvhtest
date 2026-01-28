// DlgVisionInspectionSurfaceDebugInfo.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VisionInspectionSurface.h"
#include "DlgVisionInspectionSurfaceDebugInfo.h"
#include "DlgVisionInspectionSurface.h"
#include "DlgVisionInspectionSurfaceTextLog.h"

#include "../../PI_Modules/dPI_GridCtrl\GridCtrl.h"
#include "../../PI_Modules/dPI_ImageDisplay/ImageDisplay.h"
#include "../../VisionCommon/VisionBaseDef.h"

#define	GRID_NUM_X	2

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDlgVisionInspectionSurfaceDebugInfo, CDialog)

CDlgVisionInspectionSurfaceDebugInfo::CDlgVisionInspectionSurfaceDebugInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVisionInspectionSurfaceDebugInfo::IDD, pParent)
{
	m_pDlgVisionInspectionSurface	= (CDlgVisionInspectionSurface*) pParent;
	m_pgridDebugInfo			= NULL;
	m_pvecDebugInfo			= &m_pDlgVisionInspectionSurface->m_pVisionInsp->m_DebugInfoGroup.m_vecDebugInfo;
	m_pImageDisplay			= m_pDlgVisionInspectionSurface->m_pImageDisplay;
}

CDlgVisionInspectionSurfaceDebugInfo::~CDlgVisionInspectionSurfaceDebugInfo()
{
	delete m_pgridDebugInfo;
}

void CDlgVisionInspectionSurfaceDebugInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgVisionInspectionSurfaceDebugInfo, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_GRID_DEBUG_INFO, OnClickGridDebugInfo)
END_MESSAGE_MAP()


// CDlgVisionInspectionSurfaceDebugInfo 메시지 처리기입니다.

BOOL CDlgVisionInspectionSurfaceDebugInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_pgridDebugInfo == NULL)
	{
		m_pgridDebugInfo = new CGridCtrl;
		if(m_pgridDebugInfo == NULL)
		{
//			AfxMessageBox(_T("Don't Create GridCtrl."));
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVisionInspectionSurfaceDebugInfo::SetInitDialog()
{
	SetInitGrid();
	UpdateDebugInfoColor();
}

void CDlgVisionInspectionSurfaceDebugInfo::SetInitGrid()
{
	CRect rect;
	GetClientRect(rect);

	if(m_pgridDebugInfo->GetSafeHwnd() == NULL) // 영훈 : 여기도 추가
		m_pgridDebugInfo->Create(rect, this, IDC_GRID_DEBUG_INFO);
	m_pgridDebugInfo->SetEditable(TRUE);
	m_pgridDebugInfo->EnableDragAndDrop(FALSE);


	// Grid 가로 세로 개수 설정
	long nDebugInfoNum = (long)( m_pvecDebugInfo->size());
	m_pgridDebugInfo->SetColumnCount(GRID_NUM_X);
	m_pgridDebugInfo->SetRowCount(nDebugInfoNum + 1);	
	m_pgridDebugInfo->SetFixedRowCount(1);
	m_pgridDebugInfo->SetEditable(FALSE);
	m_pgridDebugInfo->SetColumnWidth(0, 480);
	m_pgridDebugInfo->SetColumnWidth(1, 100);


	// 0 번째 Row 에 각각의 Title 을 정의
	m_pgridDebugInfo->SetItemText(0, 0, _T("Name"));
	m_pgridDebugInfo->SetItemText(0, 1, _T("Type"));
	m_pgridDebugInfo->SetItemBkColour(0, 0, RGB(200,200,250));
	m_pgridDebugInfo->SetItemBkColour(0, 1, RGB(200,200,250));


	// Debug Info Grid 내용 채우기
	long i;
	for(i = 0; i < (long)( m_pvecDebugInfo->size()); i++)
	{
		m_pgridDebugInfo->SetItemText(i+1, 0, (*m_pvecDebugInfo)[i].strDebugInfoName);
		m_pgridDebugInfo->SetItemText(i+1, 1, DebugInfoType2String((*m_pvecDebugInfo)[i].nDataType));
	}

	m_pgridDebugInfo->Refresh();
}

void CDlgVisionInspectionSurfaceDebugInfo::OnClickGridDebugInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pGridView = (NM_GRIDVIEW *)pNMHDR;
	*pResult = 0;

	CCellID	CellID = m_pgridDebugInfo->GetFocusCell();
	ShowDebugInfo(CellID.row-1);
}

void CDlgVisionInspectionSurfaceDebugInfo::ShowDebugInfo(long nID)
{
	// Text Log Clear
	//m_pDlgVisionInspectionSurface->m_pVisionInspTextLogDlg->SetLogBoxText(_T(""), _T(""));

	// Overlay Clear
	m_pImageDisplay->Overlay_RemoveAll();
 
	if( nID < 0 )
		return;

	if( nID > (long)( m_pvecDebugInfo->size())-1 )
		return;

	CString strData, strDataLog;

	(*m_pvecDebugInfo)[nID].ToTextOverlay(strData, m_pImageDisplay);

	// Text Log Show
	//m_pDlgVisionInspectionSurface->m_pVisionInspTextLogDlg->SetLogBoxText(strData, strDataLog);

	// Overlay Show
	m_pImageDisplay->Overlay_Show(TRUE);
	strData.Empty();
	strDataLog.Empty();

}

void CDlgVisionInspectionSurfaceDebugInfo::UpdateDebugInfoColor()
{
	long i, j;
	for(i = 0; i < (long)( m_pvecDebugInfo->size()); i++)
	{
		if( (*m_pvecDebugInfo)[i].nDataNum == 0 )
		{
			for(j = 0; j < GRID_NUM_X; j++)
			{
				m_pgridDebugInfo->SetItemBkColour(i+1, j, RGB(128,128,128));
			}
		}
		else
		{
			for(j = 0; j < GRID_NUM_X; j++)
			{
				m_pgridDebugInfo->SetItemBkColour(i+1, j, RGB(255,255,255));
			}
		}
	}

	m_pgridDebugInfo->Refresh();
}
