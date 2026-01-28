// DlgVisionInspectionSurfaceResultDetail.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VisionInspectionSurface.h"
#include "DlgVisionInspectionSurfaceResultDetail.h"
#include "DlgVisionInspectionSurface.h"

#include "../../PI_Modules/dPI_GridCtrl\GridCtrl.h"
#include "../../PI_Modules/dPI_ImageDisplay/ImageDisplay.h"
#include "../../VisionCommon/VisionBaseDef.h"
#include "../../VisionHostCommon/VisionHostBaseDef.h"

#define	GRID_NUM_X	5
#define IDC_GRID_RESULT_DETAIL 100

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDlgVisionInspectionSurfaceResultDetail, CDialog)

CDlgVisionInspectionSurfaceResultDetail::CDlgVisionInspectionSurfaceResultDetail(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVisionInspectionSurfaceResultDetail::IDD, pParent)
{
	m_pDlgVisionInspectionSurface	= (CDlgVisionInspectionSurface*) pParent;
	m_pgridResultDetail			= NULL;
	m_pvecResult				= &m_pDlgVisionInspectionSurface->m_pVisionInsp->m_resultGroup.m_vecResult;
	m_nCurGrid					= 0;
	m_pImageDisplay				= m_pDlgVisionInspectionSurface->m_pImageDisplay;
}

CDlgVisionInspectionSurfaceResultDetail::~CDlgVisionInspectionSurfaceResultDetail()
{
	delete m_pgridResultDetail;
}

void CDlgVisionInspectionSurfaceResultDetail::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_BACK, m_btnBack);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionSurfaceResultDetail, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BACK, &CDlgVisionInspectionSurfaceResultDetail::OnBnClickedBack)
	ON_NOTIFY(NM_CLICK, IDC_GRID_RESULT_DETAIL, OnClickGridResultDetail)
END_MESSAGE_MAP()


// CDlgVisionInspectionSurfaceResultDetail 메시지 처리기입니다.

BOOL CDlgVisionInspectionSurfaceResultDetail::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_pgridResultDetail == NULL)
	{
		m_pgridResultDetail = new CGridCtrl;
		if(m_pgridResultDetail == NULL)
		{
//			AfxMessageBox(_T("Don't Create GridCtrl."));
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVisionInspectionSurfaceResultDetail::OnBnClickedBack()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pDlgVisionInspectionSurface->m_bShowResultDetail = FALSE;
	//m_pDlgVisionInspectionSurface->ShowResultDetailDialog();
}

void CDlgVisionInspectionSurfaceResultDetail::SetInitDialog()
{
	SetInitGrid();
	SetInitButton();
}

void CDlgVisionInspectionSurfaceResultDetail::SetInitButton()
{
	CRect rtDlg;
	GetClientRect(rtDlg);

	CRect rtBtnBack;
	m_btnBack.GetClientRect(rtBtnBack);

	long nBackWidth		= rtBtnBack.Width();
	long nBackHeight	= rtBtnBack.Height();

	m_btnBack.MoveWindow(rtDlg.right - nBackWidth, rtDlg.bottom - nBackHeight, nBackWidth, nBackHeight); 
}

void CDlgVisionInspectionSurfaceResultDetail::SetInitGrid()
{
	CRect rect;
	GetClientRect(rect);
	rect.bottom = rect.bottom - 30;

	m_pgridResultDetail->Create(rect, this, IDC_GRID_RESULT_DETAIL);
	m_pgridResultDetail->SetEditable(FALSE);
	m_pgridResultDetail->EnableDragAndDrop(FALSE);

	
	long nResultNum = 0;


	// Grid 가로 세로 개수 설정
	m_pgridResultDetail->SetColumnCount(GRID_NUM_X);
	m_pgridResultDetail->SetRowCount(nResultNum+1);
	m_pgridResultDetail->SetFixedRowCount(1);


	// 0 번째 Row 에 각각의 Title 을 정의
	m_pgridResultDetail->SetItemText(0, 0, _T("ID"));
	m_pgridResultDetail->SetItemText(0, 1, _T("X"));
	m_pgridResultDetail->SetItemText(0, 2, _T("Y"));
	m_pgridResultDetail->SetItemText(0, 3, _T("Value"));
	m_pgridResultDetail->SetItemText(0, 4, _T("Result"));
	m_pgridResultDetail->SetColumnWidth(0, 50);
	
	m_pgridResultDetail->SetHeaderSort();
}

void CDlgVisionInspectionSurfaceResultDetail::SetGrid()
{
	long nResultNum = 0;
	if( m_pvecResult->size() > 0 )
	{
		if( m_nCurGrid >= 0 && m_nCurGrid < (long)( m_pvecResult->size()) )
		{
			nResultNum = (long)( (*m_pvecResult)[m_nCurGrid].m_objectResults.size());
		}
	}


	// Grid 가로 세로 개수 설정
	m_pgridResultDetail->SetColumnCount(GRID_NUM_X);
	m_pgridResultDetail->SetRowCount(nResultNum+1);
	m_pgridResultDetail->SetFixedRowCount(1);


	// 0 번째 Row 에 각각의 Title 을 정의
	m_pgridResultDetail->SetItemText(0, 0, _T("ID"));
	m_pgridResultDetail->SetItemText(0, 1, _T("X"));
	m_pgridResultDetail->SetItemText(0, 2, _T("Y"));
	m_pgridResultDetail->SetItemText(0, 3, _T("Value"));
	m_pgridResultDetail->SetItemText(0, 4, _T("Result"));
	m_pgridResultDetail->SetColumnWidth(0, 50);


	// 값 채우기
	long i;
	CString str;
	for( i = 0; i < nResultNum; i++ )
	{
		// ID
		str.Format(_T("%d"), i);
		m_pgridResultDetail->SetItemText(i+1, 0, str);

		// X
		str.Format(_T("%d"), (*m_pvecResult)[m_nCurGrid].m_objectRects[i].CenterPoint().x);
		m_pgridResultDetail->SetItemText(i+1, 1, str);

		// Y
		str.Format(_T("%d"), (*m_pvecResult)[m_nCurGrid].m_objectRects[i].CenterPoint().y);
		m_pgridResultDetail->SetItemText(i+1, 2, str);

		// Value
		str.Format(_T("%.2f"), (*m_pvecResult)[m_nCurGrid].m_objectErrorValues[i]);
		m_pgridResultDetail->SetItemText(i+1, 3, str);

		// Result
		str = Result2String((*m_pvecResult)[m_nCurGrid].m_objectResults[i]);
		m_pgridResultDetail->SetItemText(i+1, 4, str);

		m_pgridResultDetail->SetItemBkColour(i+1, 4,Result2Color((*m_pvecResult)[m_nCurGrid].m_objectResults[i]));
	}

	str.Empty();
	m_pgridResultDetail->Refresh();
}

void CDlgVisionInspectionSurfaceResultDetail::OnClickGridResultDetail(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pGridView = (NM_GRIDVIEW *)pNMHDR;
	*pResult = 0;

	CCellID	CellID = m_pgridResultDetail->GetFocusCell();

	CString str;
	long nID;

	str = m_pgridResultDetail->GetItemText(CellID.row, 0);
	nID =  _ttol(str);
	str.Empty();

	OverlayDetailData(nID);
}

void CDlgVisionInspectionSurfaceResultDetail::OverlayDetailData(long nID)
{
	if( nID < 0 )
		return;

	if( nID > (long)( (*m_pvecResult)[m_nCurGrid].m_objectRects.size())-1 )
		return;

	IPVM::Point_32s_C2 ptCenter = IPVM::Point_32s_C2((*m_pvecResult)[m_nCurGrid].m_objectRects[nID].CenterPoint().x, (*m_pvecResult)[m_nCurGrid].m_objectRects[nID].CenterPoint().y);

	m_pImageDisplay->Overlay_RemoveAll();
	
	IPVM::Point_32s_C2 ptX_Begin, ptX_End, ptY_Begin, ptY_End;
	ptX_Begin.Set(ptCenter.m_x - 40, ptCenter.m_y);
	ptX_End  .Set(ptCenter.m_x + 40, ptCenter.m_y);
					
	ptY_Begin.Set(ptCenter.m_x, ptCenter.m_y - 40);
	ptY_End  .Set(ptCenter.m_x, ptCenter.m_y + 40);
		
	m_pImageDisplay->Overlay_AddLine(ptX_Begin, ptX_End, RGB(0,255,0));
	m_pImageDisplay->Overlay_AddLine(ptY_Begin, ptY_End, RGB(0,255,0));

	m_pImageDisplay->Overlay_Show(TRUE);
}