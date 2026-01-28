// DlgVisionInspectionLgaBottom2DResultDetail.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VisionInspectionLgaBottom2D.h"
#include "DlgVisionInspectionLgaBottom2DResultDetail.h"
#include "DlgVisionInspectionLgaBottom2D.h"

#include "../../VisionCommon/VisionBaseDef.h"
#include "../../VisionHostCommon/VisionHostBaseDef.h"

#include "../../PI_Modules/dPI_DataBase/PackageSpec.h"
#include "../../PI_Modules/dPI_ImageDisplay/ImageDisplay.h"
#include "../../PI_Modules/dPI_MsgDialog/SimpleMessage.h"
#include "../../PI_Modules/dPI_SystemIni/SystemConfig.h"
#include "../../VisionReusableMemory/VisionReusableMemory.h"
#include "../../PI_Modules/dPI_DataBase/PackageSpec.h"

#define	GRID_NUM_X	5

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDlgVisionInspectionLgaBottom2DResultDetail, CDialog)

CDlgVisionInspectionLgaBottom2DResultDetail::CDlgVisionInspectionLgaBottom2DResultDetail(CPackageSpec &packageSpec, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVisionInspectionLgaBottom2DResultDetail::IDD, pParent)
	, m_packageSpec(packageSpec)
{
	m_pDlgVisionInspectionLgaBottom2D	= (CDlgVisionInspectionLgaBottom2D*) pParent;
	m_pgridResultDetail			= NULL;
	m_nCurGrid					= 0;
	m_pvecResult				= &m_pDlgVisionInspectionLgaBottom2D->m_pVisionInsp->m_resultGroup.m_vecResult;
	m_pImageDisplay				= m_pDlgVisionInspectionLgaBottom2D->m_pImageDisplay;
}

CDlgVisionInspectionLgaBottom2DResultDetail::~CDlgVisionInspectionLgaBottom2DResultDetail()
{
	delete m_pgridResultDetail;
}

void CDlgVisionInspectionLgaBottom2DResultDetail::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_BACK, m_btnBack);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionLgaBottom2DResultDetail, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BACK, &CDlgVisionInspectionLgaBottom2DResultDetail::OnBnClickedBack)
	ON_NOTIFY(NM_CLICK, IDC_GRID_RESULT_DETAIL, OnClickGridResultDetail)
END_MESSAGE_MAP()


// CDlgVisionInspectionLgaBottom2DResultDetail 메시지 처리기입니다.

BOOL CDlgVisionInspectionLgaBottom2DResultDetail::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_pgridResultDetail == NULL)
	{
		m_pgridResultDetail = new CGridCtrl;
		if(m_pgridResultDetail == NULL)
		{
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVisionInspectionLgaBottom2DResultDetail::OnBnClickedBack()
{
	m_pDlgVisionInspectionLgaBottom2D->m_bShowResultDetail = FALSE;
	m_pDlgVisionInspectionLgaBottom2D->ShowResultDetailDialog();
}

void CDlgVisionInspectionLgaBottom2DResultDetail::SetInitDialog()
{
	SetInitGrid();
	SetInitButton();
}

void CDlgVisionInspectionLgaBottom2DResultDetail::SetInitButton()
{
	CRect rtDlg;
	GetClientRect(rtDlg);

	CRect rtBtnBack;
	m_btnBack.GetClientRect(rtBtnBack);

	long nBackWidth		= rtBtnBack.Width();
	long nBackHeight	= rtBtnBack.Height();

	m_btnBack.MoveWindow(rtDlg.right - nBackWidth, rtDlg.bottom - nBackHeight, nBackWidth, nBackHeight); 
}

void CDlgVisionInspectionLgaBottom2DResultDetail::SetInitGrid()
{
	CRect rect;
	GetClientRect(rect);

	CRect rtCombo;
	rtCombo.left = rect.right -80;
	rtCombo.right = rect.right;
	rtCombo.top = rect.top;
	rtCombo.bottom = rect.top +20 ;

	rect.top = rect.top+20;

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

void CDlgVisionInspectionLgaBottom2DResultDetail::SetGrid()
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
		m_pgridResultDetail->SetItemText(i+1, 0, (*m_pvecResult)[m_nCurGrid].m_objectNames[i]);

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

void CDlgVisionInspectionLgaBottom2DResultDetail::OnClickGridResultDetail(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pGridView = (NM_GRIDVIEW *)pNMHDR;

	CCellID	CellID = m_pgridResultDetail->GetFocusCell();

	CString str;
	long nID(0);
	str = m_pgridResultDetail->GetItemText(CellID.row, 0);

	m_packageSpec.GetSelectGridSelIndex((*m_pvecResult)[m_nCurGrid].m_objectNames, str, nID);

	OverlayDetailData(CellID.row);

	*pResult = 0;
}

void CDlgVisionInspectionLgaBottom2DResultDetail::OverlayDetailData(long nID)
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

