// DlgVisionInspectionSurfaceSpec.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VisionInspectionSurface.h"
#include "DlgVisionInspectionSurfaceSpec.h"
#include "DlgVisionInspectionSurface.h"

#define	GRID_NUM_X	8


// CDlgVisionInspectionSurfaceSpec 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgVisionInspectionSurfaceSpec, CDialog)

CDlgVisionInspectionSurfaceSpec::CDlgVisionInspectionSurfaceSpec(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVisionInspectionSurfaceSpec::IDD, pParent)
{
	m_pVisionInspDlg = (CDlgVisionInspectionSurface*) pParent;
	m_pgridSpec = NULL;
	m_pvecSpec = &m_pVisionInspDlg->m_pVisionInsp->m_fixedInspectionSpecs;
}

CDlgVisionInspectionSurfaceSpec::~CDlgVisionInspectionSurfaceSpec()
{
	delete m_pgridSpec;
}

void CDlgVisionInspectionSurfaceSpec::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgVisionInspectionSurfaceSpec, CDialog)
	ON_NOTIFY(GVN_ENDLABELEDIT,	IDC_GRID_SPEC, &CDlgVisionInspectionSurfaceSpec::OnGridSurfaceSpecParameterEditEnd)
	ON_NOTIFY(GVN_STATECHANGED,	IDC_GRID_SPEC, &CDlgVisionInspectionSurfaceSpec::OnGridSurfaceSpecParameterCheckClicked)
END_MESSAGE_MAP()


// CDlgVisionInspectionSurfaceSpec 메시지 처리기입니다.

BOOL CDlgVisionInspectionSurfaceSpec::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_pgridSpec == NULL)
	{
		m_pgridSpec = new CGridCtrl;
		if(m_pgridSpec == NULL)
		{
//			AfxMessageBox(_T("Don't Create GridCtrl."));
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionInspectionSurfaceSpec::SetInitDialog()
{
	SetInitGrid();
}

void CDlgVisionInspectionSurfaceSpec::SetInitGrid()
{
	CRect rect;
	GetClientRect(rect);

	rect.top = rect.top/*+20*/;
	
	// 영훈 : 이걸 사용하는 모든 부분에서 Create 시 이미 생성됐는지 확인하는 예외처리를 추가한다. 메모리 누수 및 UI 버그를 유발한다.
	if(m_pgridSpec->GetSafeHwnd() == NULL)
		m_pgridSpec->Create(rect, this, IDC_GRID_SPEC);
	
	m_pgridSpec->SetEditable(TRUE);
	m_pgridSpec->EnableDragAndDrop(FALSE);


	// Grid 가로 세로 개수 설정
	long nSpecNum = (long)( m_pvecSpec->size());
	m_pgridSpec->SetColumnCount(GRID_NUM_X);
	m_pgridSpec->SetRowCount(nSpecNum+1);	
	m_pgridSpec->SetFixedRowCount(1);
	m_pgridSpec->SetFixedColumnCount(1);
	m_pgridSpec->SetEditable(TRUE);
	
	// Grid 정렬
	m_pgridSpec->GetDefaultCell(FALSE, TRUE) ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_pgridSpec->GetDefaultCell(TRUE,  FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_pgridSpec->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);


	long i;
	CString str;
	for(i = 0; i < nSpecNum; i++)
	{	
		m_pgridSpec->SetItemBkColour(i+1, 0, RGB(250,250,220));
		// ID
		str.Format(_T("%d"), i);
		m_pgridSpec->SetItemText(i+1, 0, str);
		// Name
		m_pgridSpec->SetItemText(i+1, 1, (*m_pvecSpec)[i].m_specName);
		m_pgridSpec->SetItemState(i+1,1,  m_pgridSpec->GetItemState(i+1,1) | GVIS_READONLY);	// SpecName은 읽기전용
		// Use
		m_pgridSpec->SetCellType(i+1, 2, RUNTIME_CLASS(CGridCellCheck2));
		((CGridCellCheck2*) m_pgridSpec->GetCell(i+1, 2))->SetCheck((*m_pvecSpec)[i].m_use);
		// Min
		str.Format(_T("%.2f"),  (*m_pvecSpec)[i].m_passMin);
		m_pgridSpec->SetItemText(i+1, 3, str);
		// Max
		str.Format(_T("%.2f"),  (*m_pvecSpec)[i].m_passMax);
		m_pgridSpec->SetItemText(i+1, 4, str);
		// Unit
		str = _T("[") + (*m_pvecSpec)[i].m_unit + _T("]");
		m_pgridSpec->SetItemText(i+1, 5, str);
		// Use Min
		m_pgridSpec->SetCellType(i+1, 6, RUNTIME_CLASS(CGridCellCheck2));
		((CGridCellCheck2*) m_pgridSpec->GetCell(i+1, 6))->SetCheck((*m_pvecSpec)[i].m_useMin);
		// Use Max
		m_pgridSpec->SetCellType(i+1, 7, RUNTIME_CLASS(CGridCellCheck2));
		((CGridCellCheck2*) m_pgridSpec->GetCell(i+1, 7))->SetCheck((*m_pvecSpec)[i].m_useMax);
	}

	// 모든 Cell 을 가운데 정렬
//	for( i = 0; i < m_pgridSpec->GetRowCount(); i++ )
//	{
//		for( j = 0; j < GRID_NUM_X; j++ )
//		{
//			m_pgridSpec->SetItemFormat(i, j, DT_CENTER);
//		}
//	}

	// 0 번째 Row 에 각각의 Title 을 정의
	m_pgridSpec->SetItemText(0, 0, _T("No"));
	m_pgridSpec->SetItemText(0, 1, _T("Name"));
	m_pgridSpec->SetItemText(0, 2, _T("Use"));
	m_pgridSpec->SetItemText(0, 3, _T("Min"));
	m_pgridSpec->SetItemText(0, 4, _T("Max"));
	m_pgridSpec->SetItemText(0, 5, _T("Unit"));
	m_pgridSpec->SetItemText(0, 6, _T("Use Min"));
	m_pgridSpec->SetItemText(0, 7, _T("Use Max"));

	m_pgridSpec->SetItemBkColour(0, 0, RGB(200,200,250));
	m_pgridSpec->SetItemBkColour(0, 1, RGB(200,200,250));
	m_pgridSpec->SetItemBkColour(0, 2, RGB(200,200,250));
	m_pgridSpec->SetItemBkColour(0, 3, RGB(200,200,250));
	m_pgridSpec->SetItemBkColour(0, 4, RGB(200,200,250));
	m_pgridSpec->SetItemBkColour(0, 5, RGB(200,200,250));
	m_pgridSpec->SetItemBkColour(0, 6, RGB(200,200,250));
	m_pgridSpec->SetItemBkColour(0, 7, RGB(200,200,250));

	m_pgridSpec->SetColumnWidth(0, 30);
	m_pgridSpec->SetColumnWidth(1, 200);
	m_pgridSpec->SetColumnWidth(2, 30);
	m_pgridSpec->SetColumnWidth(3, 70);
	m_pgridSpec->SetColumnWidth(4, 70);
	m_pgridSpec->SetColumnWidth(5, 40);
	m_pgridSpec->SetColumnWidth(6, 60);
	m_pgridSpec->SetColumnWidth(7, 60);

	str.Empty();
	m_pgridSpec->Refresh();
}

void CDlgVisionInspectionSurfaceSpec::UpdateSpecData()
{
	long nSpecNum = (long)( m_pvecSpec->size());
	long i;
	CString str;

	for(i = 0; i < nSpecNum; i++)
	{
		// Use
		(*m_pvecSpec)[i].m_use = ((CGridCellCheck2*) m_pgridSpec->GetCell(i+1, 2))->GetCheck();
		
		// Min
		str = m_pgridSpec->GetItemText(i+1, 3);
		(*m_pvecSpec)[i].m_passMin = (float)_ttof(str);

		// Max
		str = m_pgridSpec->GetItemText(i+1, 4);
		(*m_pvecSpec)[i].m_passMax = (float)_ttof(str);
		
		// Use Min
		(*m_pvecSpec)[i].m_useMin = ((CGridCellCheck2*) m_pgridSpec->GetCell(i+1, 6))->GetCheck();

		// Use Max
		(*m_pvecSpec)[i].m_useMax = ((CGridCellCheck2*) m_pgridSpec->GetCell(i+1, 7))->GetCheck();
	}

	str.Empty();
}

void CDlgVisionInspectionSurfaceSpec::OnGridSurfaceSpecParameterEditEnd(NMHDR* /* pNotifyStruct */, LRESULT* result)
{
	UpdateSpecData();

	Invalidate(FALSE);

	*result = 0;
}

void CDlgVisionInspectionSurfaceSpec::OnGridSurfaceSpecParameterCheckClicked(NMHDR* /* pNotifyStruct */, LRESULT* result)
{
	UpdateSpecData();

	Invalidate(FALSE);

	*result = 0;
}
