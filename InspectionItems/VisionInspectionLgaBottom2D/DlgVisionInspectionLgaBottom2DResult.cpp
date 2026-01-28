// DlgVisionInspectionLgaBottom2DResult.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VisionInspectionLgaBottom2D.h"
#include "DlgVisionInspectionLgaBottom2DResult.h"
#include "DlgVisionInspectionLgaBottom2D.h"

#include "../../VisionCommon/VisionBaseDef.h"
#include "../../VisionCommon/VisionUnitAgent.h"
#include "../../VisionHostCommon/VisionHostBaseDef.h"

#include "../../PI_Modules/dPI_DataBase/PackageSpec.h"
#include "../../PI_Modules/dPI_ImageDisplay/ImageDisplay.h"
#include "../../PI_Modules/dPI_MsgDialog/SimpleMessage.h"
#include "../../PI_Modules/dPI_SystemIni/SystemConfig.h"
#include "../../VisionReusableMemory/VisionReusableMemory.h"

#define GRID_NUM_X	6

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDlgVisionInspectionLgaBottom2DResult, CDialog)

	CDlgVisionInspectionLgaBottom2DResult::CDlgVisionInspectionLgaBottom2DResult(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVisionInspectionLgaBottom2DResult::IDD, pParent)
{
	m_pDlgVisionInspectionLgaBottom2D	= (CDlgVisionInspectionLgaBottom2D*) pParent;
	m_pvecResult				= &m_pDlgVisionInspectionLgaBottom2D->m_pVisionInsp->m_resultGroup.m_vecResult;
	m_pgridResult				= NULL;
	m_pImageDisplay				= m_pDlgVisionInspectionLgaBottom2D->m_pImageDisplay;
}

CDlgVisionInspectionLgaBottom2DResult::~CDlgVisionInspectionLgaBottom2DResult()
{
	delete m_pgridResult;
}

void CDlgVisionInspectionLgaBottom2DResult::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionLgaBottom2DResult, CDialog)
	ON_NOTIFY(NM_CLICK, IDC_GRID_RESULT, OnClickGridResult)
	ON_NOTIFY(NM_DBLCLK, IDC_GRID_RESULT, OnDblclkGridResult)
END_MESSAGE_MAP()


// CDlgVisionInspectionLgaBottom2DResult 메시지 처리기입니다.

BOOL CDlgVisionInspectionLgaBottom2DResult::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_pgridResult == NULL)
	{
		m_pgridResult = new CGridCtrl;
		if(m_pgridResult == NULL)
		{
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVisionInspectionLgaBottom2DResult::SetInitDialog()
{
	SetInitGrid();
}

void CDlgVisionInspectionLgaBottom2DResult::SetInitGrid()
{
	CRect rect;
	GetClientRect(rect);

	CRect rtCombo;
	rtCombo.left = rect.right -80;
	rtCombo.right = rect.right;
	rtCombo.top = rect.top;
	rtCombo.bottom = rect.top +20 ;

	rect.top = rect.top+20;

	// 영훈 : 이걸 사용하는 모든 부분에서 Create 시 이미 생성됐는지 확인하는 예외처리를 추가한다. 메모리 누수 및 UI 버그를 유발한다.
	if(m_pgridResult->GetSafeHwnd() == NULL)
		m_pgridResult->Create(rect, this, IDC_GRID_RESULT);

	m_pgridResult->SetEditable(TRUE);
	m_pgridResult->EnableDragAndDrop(FALSE);

	// Grid 가로 세로 개수 설정
	long nResultNum = (long)( m_pvecResult->size());
	m_pgridResult->SetColumnCount(GRID_NUM_X);
	m_pgridResult->SetRowCount(nResultNum+1);	
	m_pgridResult->SetFixedRowCount(1);
	m_pgridResult->SetFixedColumnCount(1);
	m_pgridResult->SetEditable(FALSE);

	// Grid 정렬
	m_pgridResult->GetDefaultCell(FALSE, TRUE) ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_pgridResult->GetDefaultCell(TRUE,  FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_pgridResult->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// 0 번째 Row 에 각각의 Title 을 정의
	m_pgridResult->SetItemText(0, 0, _T("No"));
	m_pgridResult->SetItemText(0, 1, _T("Item"));
	m_pgridResult->SetItemText(0, 2, _T("Result"));
	m_pgridResult->SetItemText(0, 3, _T("Value"));
	m_pgridResult->SetItemText(0, 4, _T("Unit"));
	m_pgridResult->SetItemText(0, 5, _T("Detail"));


	m_pgridResult->SetItemBkColour(0, 0, RGB(200,200,250));
	m_pgridResult->SetItemBkColour(0, 1, RGB(200,200,250));
	m_pgridResult->SetItemBkColour(0, 2, RGB(200,200,250));
	m_pgridResult->SetItemBkColour(0, 3, RGB(200,200,250));
	m_pgridResult->SetItemBkColour(0, 4, RGB(200,200,250));
	m_pgridResult->SetItemBkColour(0, 5, RGB(200,200,250));
	//m_pgridResult->SetItemBkColour(0, 6, RGB(200,200,250));

	m_pgridResult->SetColumnWidth(0, 30);
	m_pgridResult->SetColumnWidth(1, 200);
	m_pgridResult->SetColumnWidth(4, 40);

	// 값 채우기
	long i;
	CString str;
	for( i = 0; i < nResultNum; i++ )
	{
		m_pgridResult->SetItemBkColour(i+1, 0, RGB(250,250,220));

		// ID
		str.Format(_T("%d"), i);
		m_pgridResult->SetItemText(i+1, 0, str);

		// Name
		m_pgridResult->SetItemText(i+1, 1, (*m_pvecResult)[i].m_resultName);

		// Result
		m_pgridResult->SetItemText(i+1, 2, Result2String((*m_pvecResult)[i].m_totalResult));
		m_pgridResult->SetItemBkColour(i+1, 2,Result2Color((*m_pvecResult)[i].m_totalResult));

		// Value
		str.Format(_T("%.2f"), (*m_pvecResult)[i].m_worstErrorValue);
		m_pgridResult->SetItemText(i+1, 3, str);

		// Unit
		m_pgridResult->SetItemText(i+1, 4, (*m_pvecResult)[i].m_itemUnit);

		// Detail
		m_pgridResult->SetItemText(i+1, 5, _T(">>"));
		str.Empty();
	}

	m_pgridResult->Refresh();
}

void CDlgVisionInspectionLgaBottom2DResult::OnDblclkGridResult(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pGridView = (NM_GRIDVIEW *)pNMHDR;

	CCellID	CellID = m_pgridResult->GetFocusCell();
	if( CellID.col == 5 )
	{
		m_pDlgVisionInspectionLgaBottom2D->m_bShowResultDetail = TRUE;
		m_pDlgVisionInspectionLgaBottom2D->ShowResultDetailDialog(CellID.row-1);
	}

	*pResult = 0;
}

void CDlgVisionInspectionLgaBottom2DResult::OnClickGridResult(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pGridView = (NM_GRIDVIEW *)pNMHDR;

	CCellID	CellID = m_pgridResult->GetFocusCell();
	if( CellID.col != 5 )
	{
		OverlayDetailData(CellID.row-1);
	}

	*pResult = 0;
}

void CDlgVisionInspectionLgaBottom2DResult::OverlayDetailData(long nID)
{
	m_pImageDisplay->Overlay_RemoveAll();

	if( nID < 0 )
		return;

	if( nID > (long)( m_pvecResult->size())-1 )
		return;

	IPVM::Rect rtRejectROI;
	long i, nSize;
	nSize = (long)( (*m_pvecResult)[nID].vecrtRejectROI.size());

	for(i=0; i<nSize; i++)
	{
		CRect rtTemp = (*m_pvecResult)[nID].vecrtRejectROI[i];
		rtRejectROI = IPVM::Rect(rtTemp.left, rtTemp.top, rtTemp.right, rtTemp.bottom);
		m_pImageDisplay->Overlay_AddRectangle(rtRejectROI, RGB(255,0, 0));
	}

	m_pImageDisplay->Overlay_Show(TRUE);
}

void CDlgVisionInspectionLgaBottom2DResult::UpdateResultData()
{
	long nResultNum = (long)( m_pvecResult->size());
	CString str;
	for( long i = 0; i < nResultNum; i++ )
	{
		m_pgridResult->SetItemText(i+1, 2, Result2String((*m_pvecResult)[i].m_totalResult));

		str.Format(_T("%.2f"), (*m_pvecResult)[i].m_worstErrorValue );
		m_pgridResult->SetItemText(i+1, 3, str);
		m_pgridResult->SetItemBkColour(i+1, 2,Result2Color((*m_pvecResult)[i].m_totalResult));
	}

	m_pgridResult->Refresh();
}

long CDlgVisionInspectionLgaBottom2DResult::GetTotalResult()
{
	long nResult = NOT_MEASURED;

	long nResultNum = (long)(m_pvecResult->size());
	for( long n = 0; n < nResultNum; n++ )
	{
		nResult = max((*m_pvecResult)[n].m_totalResult, nResult);
	}

	return nResult;
}
