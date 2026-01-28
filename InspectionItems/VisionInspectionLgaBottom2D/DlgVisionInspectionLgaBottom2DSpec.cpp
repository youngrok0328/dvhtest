// DlgVisionInspectionLgaBottom2DSpec.cpp : 구현 파일입니다.
//
#include "stdafx.h"
#include "VisionInspectionLgaBottom2D.h"
#include "DlgVisionInspectionLgaBottom2DSpec.h"
#include "DlgVisionInspectionLgaBottom2D.h"

#include "../../VisionCommon/VisionUnitAgent.h"
#include "../../PI_Modules/dPI_DataBase/PackageSpec.h"
#include "../../PI_Modules/dPI_SystemIni/SystemConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDlgVisionInspectionLgaBottom2DSpec, CDialog)

CDlgVisionInspectionLgaBottom2DSpec::CDlgVisionInspectionLgaBottom2DSpec(CPackageSpec &packageSpec, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVisionInspectionLgaBottom2DSpec::IDD, pParent)
	, m_packageSpec(packageSpec)
{
	m_pgridSpec = NULL;
	m_pVisionInspDlg = (CDlgVisionInspectionLgaBottom2D*) pParent;
	m_pvecSpec = &m_pVisionInspDlg->m_pVisionInsp->m_fixedInspectionSpecs;
}

CDlgVisionInspectionLgaBottom2DSpec::~CDlgVisionInspectionLgaBottom2DSpec()
{
	if(m_pgridSpec->GetSafeHwnd())
		m_pgridSpec->DestroyWindow();

	if(m_pgridSpec)
	delete m_pgridSpec;
}

void CDlgVisionInspectionLgaBottom2DSpec::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionLgaBottom2DSpec, CDialog)
	ON_NOTIFY(GVN_ENDLABELEDIT,	IDC_GRID_SPEC, &CDlgVisionInspectionLgaBottom2DSpec::OnGridBallInspectionSpecParameterEditEnd)
	ON_NOTIFY(GVN_STATECHANGED,	IDC_GRID_SPEC, &CDlgVisionInspectionLgaBottom2DSpec::OnGridBallInspectionSpecParameterCheckClicked)
	ON_NOTIFY(NM_DBLCLK, IDC_GRID_SPEC, &CDlgVisionInspectionLgaBottom2DSpec::OnDblclkGridResult)

END_MESSAGE_MAP()

// CDlgVisionInspectionLgaBottom2DSpec 메시지 처리기입니다.

BOOL CDlgVisionInspectionLgaBottom2DSpec::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_pgridSpec == NULL)
		m_pgridSpec = new CGridCtrl;

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionInspectionLgaBottom2DSpec::SetInitDialog()
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
	if(m_pgridSpec->GetSafeHwnd() == NULL)
		m_pgridSpec->Create(rect, this, IDC_GRID_SPEC);

	SetInitGrid();
}

void CDlgVisionInspectionLgaBottom2DSpec::SetInitGrid()
{
	// Grid 가로 세로 개수 설정
	long nColCount = COL_PARAM_SPEC_END;
	long nHeaderCnt = 2;
	m_pgridSpec->SetColumnCount(nColCount);

	long nSpecNum = (long)m_pvecSpec->size();
	m_pgridSpec->SetRowCount(nSpecNum + nHeaderCnt);
	m_pgridSpec->SetFixedRowCount(nHeaderCnt);
	m_pgridSpec->SetFixedColumnCount(2);
	m_pgridSpec->SetItemBkColour(0, 0, RGB(255, 255, 255));
	m_pgridSpec->SetItemState(0, 0, GVIS_READONLY);

	long nTotalRowCount = nHeaderCnt;
	CString str;
	for(long i = 0; i < nSpecNum; i++)
	{	
		m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_NO, RGB(250, 250, 220));

		// No
		str.Format(_T("%d"), i);
		m_pgridSpec->SetItemText(nTotalRowCount, COL_PARAM_SPEC_NO, str);

		// Name
		m_pgridSpec->SetItemText(nTotalRowCount, COL_PARAM_SPEC_NAME, (*m_pvecSpec)[i].m_specName);
		m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_NAME, GVIS_READONLY);

		// Use
		m_pgridSpec->SetCellType(nTotalRowCount, COL_PARAM_SPEC_USE, RUNTIME_CLASS(CGridCellCheck2));
		((CGridCellCheck2*) m_pgridSpec->GetCell(nTotalRowCount, COL_PARAM_SPEC_USE))->SetCheck((*m_pvecSpec)[i].m_use);

		// Marginal Min		
		str.Format(_T("%.2f"),  (*m_pvecSpec)[i].m_marginalMin);
		m_pgridSpec->SetItemText(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, str);

		// Pass Min		
		str.Format(_T("%.2f"), (*m_pvecSpec)[i].m_passMin);
		m_pgridSpec->SetItemText(nTotalRowCount, COL_PARAM_SPEC_PASS_MIN_SPEC, str);

		// Pass Max
		str.Format(_T("%.2f"), (*m_pvecSpec)[i].m_passMax);
		m_pgridSpec->SetItemText(nTotalRowCount, COL_PARAM_SPEC_PASS_MAX_SPEC, str);

		// Marginal Max
		str.Format(_T("%.2f"),  (*m_pvecSpec)[i].m_marginalMax);
		m_pgridSpec->SetItemText(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, str);

		// Unit
		m_pgridSpec->SetItemText(nTotalRowCount, COL_PARAM_SPEC_UNIT, (*m_pvecSpec)[i].m_unit);
		m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_UNIT, GVIS_READONLY);

		// Use Min
		m_pgridSpec->SetCellType(nTotalRowCount, COL_PARAM_SPEC_USE_MIN, RUNTIME_CLASS(CGridCellCheck2));
		((CGridCellCheck2*) m_pgridSpec->GetCell(nTotalRowCount, COL_PARAM_SPEC_USE_MIN))->SetCheck((*m_pvecSpec)[i].m_useMin);

		// Use Max
		m_pgridSpec->SetCellType(nTotalRowCount, COL_PARAM_SPEC_USE_MAX, RUNTIME_CLASS(CGridCellCheck2));
		((CGridCellCheck2*) m_pgridSpec->GetCell(nTotalRowCount, COL_PARAM_SPEC_USE_MAX))->SetCheck((*m_pvecSpec)[i].m_useMax);

		// Use Marginal
		m_pgridSpec->SetCellType(nTotalRowCount, COL_PARAM_SPEC_USE_MARGINAL, RUNTIME_CLASS(CGridCellCheck2));
		((CGridCellCheck2*)m_pgridSpec->GetCell(nTotalRowCount, COL_PARAM_SPEC_USE_MARGINAL))->SetCheck((*m_pvecSpec)[i].m_useMarginal);

		if ((*m_pvecSpec)[i].m_marginalMin > (*m_pvecSpec)[i].m_passMin || !(*m_pvecSpec)[i].m_useMarginal)
		{
			(*m_pvecSpec)[i].m_marginalMin = (*m_pvecSpec)[i].m_passMin;
			str.Format(_T("%.2f"), (*m_pvecSpec)[i].m_marginalMin);
			m_pgridSpec->SetItemText(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, str);
		}

		if ((*m_pvecSpec)[i].m_marginalMax < (*m_pvecSpec)[i].m_passMax || !(*m_pvecSpec)[i].m_useMarginal)
		{
			(*m_pvecSpec)[i].m_marginalMax = (*m_pvecSpec)[i].m_passMax;
			str.Format(_T("%.2f"), (*m_pvecSpec)[i].m_marginalMax);
			m_pgridSpec->SetItemText(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, str);
		}

		//k Spec 활성화 && 비활성화
		if ((*m_pvecSpec)[i].m_use)
		{
			m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_UNIT, RGB(255, 255, 255));
			m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_USE_MIN, GVIS_MODIFIED);
			m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_USE_MIN, RGB(255, 255, 255));
			m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_USE_MAX, GVIS_MODIFIED);
			m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_USE_MAX, RGB(255, 255, 255));
			m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_USE_MARGINAL, GVIS_MODIFIED);
			m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_USE_MARGINAL, RGB(255, 255, 255));

			if ((*m_pvecSpec)[i].m_useMin)
			{
				m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_PASS_MIN_SPEC, GVIS_MODIFIED);
				m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_PASS_MIN_SPEC, RGB(255, 255, 255));

				if ((*m_pvecSpec)[i].m_useMarginal)
				{
					m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, GVIS_MODIFIED);
					m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, RGB(255, 255, 255));
				}
				else
				{
					m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, GVIS_READONLY);
					m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, RGB(134, 135, 137));
				}
			}
			else
			{
				m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_PASS_MIN_SPEC, GVIS_READONLY);
				m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_PASS_MIN_SPEC, RGB(134, 135, 137));
				m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, GVIS_READONLY);
				m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, RGB(134, 135, 137));
			}

			if ((*m_pvecSpec)[i].m_useMax)
			{
				m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_PASS_MAX_SPEC, GVIS_MODIFIED);
				m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_PASS_MAX_SPEC, RGB(255, 255, 255));

				if ((*m_pvecSpec)[i].m_useMarginal)
				{
					m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, GVIS_MODIFIED);
					m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, RGB(255, 255, 255));
				}
				else
				{
					m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, GVIS_READONLY);
					m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, RGB(134, 135, 137));
				}
			}
			else
			{
				m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_PASS_MAX_SPEC, GVIS_READONLY);
				m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_PASS_MAX_SPEC, RGB(134, 135, 137));
				m_pgridSpec->SetItemState(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, GVIS_READONLY);
				m_pgridSpec->SetItemBkColour(nTotalRowCount, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, RGB(134, 135, 137));
			}
		}
		else
		{
			for (int nCol = COL_PARAM_SPEC_MARGINAL_MIN_SPEC; nCol <= COL_PARAM_SPEC_USE_MARGINAL; nCol++)
			{
				m_pgridSpec->SetItemState(nTotalRowCount, nCol, GVIS_READONLY);
				m_pgridSpec->SetItemBkColour(nTotalRowCount, nCol, RGB(134, 135, 137));
			}
		}

		nTotalRowCount++;
	}

	// 0 번째 Row 에 각각의 Title 을 정의
	for (long nRow = 0; nRow < 2; nRow++)
	{
		long nIdxR = COL_PARAM_SPEC_END * nRow;

		for (long nCol = 0; nCol <= COL_PARAM_SPEC_USE_MARGINAL; nCol++)
		{
			long nIdxC = nIdxR + nCol;

			m_pgridSpec->SetItemText(nRow, nCol, g_strSpecDialogColumnName[nIdxC]);
			m_pgridSpec->SetItemState(nRow, nCol, GVIS_READONLY);
			m_pgridSpec->GetCell(nRow, nCol)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_pgridSpec->SetItemBkColour(nRow, nCol, RGB(200, 200, 250));
		}
	}

	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_NO, 30);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_NAME, 170);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_USE, 30);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_MARGINAL_MIN_SPEC, 55);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_PASS_MIN_SPEC, 55);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_PASS_MAX_SPEC, 55);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_MARGINAL_MAX_SPEC, 55);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_UNIT, 40);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_USE_MIN, 40);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_USE_MAX, 40);
	m_pgridSpec->SetColumnWidth(COL_PARAM_SPEC_USE_MARGINAL, 50);

	m_pgridSpec->MergeCells(0, COL_PARAM_SPEC_NO, 1, COL_PARAM_SPEC_NO);
	m_pgridSpec->MergeCells(0, COL_PARAM_SPEC_NAME, 1, COL_PARAM_SPEC_NAME);
	m_pgridSpec->MergeCells(0, COL_PARAM_SPEC_USE, 1, COL_PARAM_SPEC_USE);
	m_pgridSpec->MergeCells(0, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, 0, COL_PARAM_SPEC_PASS_MIN_SPEC);
	m_pgridSpec->MergeCells(0, COL_PARAM_SPEC_PASS_MAX_SPEC, 0, COL_PARAM_SPEC_MARGINAL_MAX_SPEC);
	m_pgridSpec->MergeCells(0, COL_PARAM_SPEC_UNIT, 1, COL_PARAM_SPEC_UNIT);
	m_pgridSpec->MergeCells(0, COL_PARAM_SPEC_USE_MIN, 0, COL_PARAM_SPEC_USE_MARGINAL);

	str.Empty();
	m_pgridSpec->Refresh();
}

void CDlgVisionInspectionLgaBottom2DSpec::UpdateSpecData(long nClickRow)
{
	CString str;
	long nSpecID = nClickRow-2;

		// Use
	(*m_pvecSpec)[nSpecID].m_use = ((CGridCellCheck2*) m_pgridSpec->GetCell(nClickRow, COL_PARAM_SPEC_USE))->GetCheck();

		// Marginal Min
	str = m_pgridSpec->GetItemText(nClickRow, COL_PARAM_SPEC_MARGINAL_MIN_SPEC);
	(*m_pvecSpec)[nSpecID].m_marginalMin =(float)_ttof(str);
	str.Empty();

		// Pass Min
	str = m_pgridSpec->GetItemText(nClickRow, COL_PARAM_SPEC_PASS_MIN_SPEC);
	(*m_pvecSpec)[nSpecID].m_passMin = (float)_ttof(str);
	str.Empty();

		// Pass Max
	str = m_pgridSpec->GetItemText(nClickRow, COL_PARAM_SPEC_PASS_MAX_SPEC);
	(*m_pvecSpec)[nSpecID].m_passMax = (float)_ttof(str);
	str.Empty();

		// Marginal Max
	str = m_pgridSpec->GetItemText(nClickRow, COL_PARAM_SPEC_MARGINAL_MAX_SPEC);
	(*m_pvecSpec)[nSpecID].m_marginalMax =(float)_ttof(str);
	str.Empty();

		// Use Min
	(*m_pvecSpec)[nSpecID].m_useMin = ((CGridCellCheck2*) m_pgridSpec->GetCell(nClickRow, COL_PARAM_SPEC_USE_MIN))->GetCheck();

		// Use Max
	(*m_pvecSpec)[nSpecID].m_useMax = ((CGridCellCheck2*) m_pgridSpec->GetCell(nClickRow, COL_PARAM_SPEC_USE_MAX))->GetCheck();

		// Use Marginal
	(*m_pvecSpec)[nSpecID].m_useMarginal = ((CGridCellCheck2*)m_pgridSpec->GetCell(nClickRow, COL_PARAM_SPEC_USE_MARGINAL))->GetCheck();

	if ((*m_pvecSpec)[nSpecID].m_marginalMin > (*m_pvecSpec)[nSpecID].m_passMin || !(*m_pvecSpec)[nSpecID].m_useMarginal)
		(*m_pvecSpec)[nSpecID].m_marginalMin = (*m_pvecSpec)[nSpecID].m_passMin;
								 
	if ((*m_pvecSpec)[nSpecID].m_marginalMax < (*m_pvecSpec)[nSpecID].m_passMax || !(*m_pvecSpec)[nSpecID].m_useMarginal)
		(*m_pvecSpec)[nSpecID].m_marginalMax = (*m_pvecSpec)[nSpecID].m_passMax;

	SetInitGrid();
}

void CDlgVisionInspectionLgaBottom2DSpec::OnGridBallInspectionSpecParameterEditEnd(NMHDR* pNotifyStruct, LRESULT* result)
{
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*) pNotifyStruct;
	int	row = pNotify->iRow;

	CString strSpecName = m_pgridSpec->GetItemText(row, COL_PARAM_SPEC_NAME);

	UpdateSpecData(row);

	*result = 0;
}

void CDlgVisionInspectionLgaBottom2DSpec::OnGridBallInspectionSpecParameterCheckClicked(NMHDR* pNotifyStruct, LRESULT* result)
{
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*) pNotifyStruct;
	int	row = pNotify->iRow;

	CString strSpecName = m_pgridSpec->GetItemText(row, COL_PARAM_SPEC_NAME);

	UpdateSpecData(row);

	*result = 0;
}

void CDlgVisionInspectionLgaBottom2DSpec::OnDblclkGridResult(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pGridView = (NM_GRIDVIEW *)pNMHDR;

	*pResult = 0;
}