// ChipSelectType.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VisionProcessingSpec.h"
#include "ChipSelectType.h"

#include "../../PI_Modules/dPI_DataBase/PackageSpec_ChipInfoMaster.h"
#include "../../dA_Modules/dA_Base/ChipDB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CChipSelectType, CDialog)

CChipSelectType::CChipSelectType(CPassiveJobInfo *pPassiveJob, std::vector<SChipInfoMaster> &vecChipInfoMaster, long nSelectID, long nSelectType, CWnd* pParent /*=NULL*/)
	: CDialog(CChipSelectType::IDD, pParent)
{
	pvecChipInfoMaster = &vecChipInfoMaster;
	m_pPassiveJob = pPassiveJob;
	m_nSelectID = nSelectID;
	m_nSelectType = nSelectType;

	nType = 0;
	fSizeX = 0.f;
	fSizeY = 0.f;
	nAngle = 0;
	strModelName = "Untile";
}

CChipSelectType::~CChipSelectType()
{
}

void CChipSelectType::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CMB_TYPE_LIST, m_ComboChipType);
	DDX_Control(pDX, IDC_CMB_ANGLE, m_ComboAngle);
}


BEGIN_MESSAGE_MAP(CChipSelectType, CDialog)
	ON_BN_CLICKED(IDOK, &CChipSelectType::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_CMB_TYPE_LIST, &CChipSelectType::OnCbnSelchangeCmbTypeList)
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CChipSelectType 메시지 처리기입니다.

BOOL CChipSelectType::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_EDIT_SIZE_X)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_SIZE_Y)->EnableWindow(FALSE);

	CString strTemp; 

	m_ComboChipType.ResetContent();
	long nSize = (long)(m_pPassiveJob->m_vecsChipData.size());
	
	if(m_nSelectType >= nSize)
	{
		m_nSelectType = 0;
	}

	if(nSize > 0)
	{
		for(long n=0 ; n<nSize ; n++)
		{
			m_ComboChipType.AddString(m_pPassiveJob->m_vecsChipData[n].strModelName);
		}
		m_ComboChipType.SetCurSel(m_nSelectType);

		m_ComboAngle.AddString(_T("Dgree 0"));
		m_ComboAngle.AddString(_T("Dgree 90"));
		
		m_ComboAngle.SetCurSel(((*pvecChipInfoMaster)[m_nSelectID]).nAngleID);
		
		strTemp.Format(_T("%.2f"), m_pPassiveJob->m_vecsChipData[m_nSelectType].fChipWidth * 1000.f);
		GetDlgItem(IDC_EDIT_SIZE_X)->SetWindowText(strTemp);

		strTemp.Format(_T("%.2f"), m_pPassiveJob->m_vecsChipData[m_nSelectType].fChipLength * 1000.f);
		GetDlgItem(IDC_EDIT_SIZE_Y)->SetWindowText(strTemp);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

long CChipSelectType::GetType()
{
	return nType;
}
float CChipSelectType::GetSizeX()
{
	return fSizeX;
}
float CChipSelectType::GetSizeY()
{
	return fSizeY;
}
CString CChipSelectType::GetModelName()
{
	return strModelName;
}
long CChipSelectType::GetAngle()
{
	return nAngle;
}
void CChipSelectType::OnBnClickedOk()
{
	UpdateData(TRUE);

	CString strTemp;

	nType = m_ComboChipType.GetCurSel();
	
	fSizeX = (float)_ttof(strTemp);
	GetDlgItem(IDC_EDIT_SIZE_X)->GetWindowText(strTemp);
	fSizeX = (float)_ttof(strTemp);
	GetDlgItem(IDC_EDIT_SIZE_Y)->GetWindowText(strTemp);
	fSizeY = (float)_ttof(strTemp);

	nAngle = m_ComboAngle.GetCurSel();	

	OnOK();
}

void CChipSelectType::OnCbnSelchangeCmbTypeList()
{
	UpdateData(TRUE);

	CString strTemp;
	
	long nCursel = m_ComboChipType.GetCurSel();

	GetDlgItem(IDC_EDIT_SIZE_X)->SetWindowText(m_pPassiveJob->m_vecsChipData[nCursel].strModelName);

	strTemp.Format(_T("%.2f"), m_pPassiveJob->m_vecsChipData[nCursel].fChipWidth * 1000.f);
	GetDlgItem(IDC_EDIT_SIZE_X)->SetWindowText(strTemp);

	strTemp.Format(_T("%.2f"), m_pPassiveJob->m_vecsChipData[nCursel].fChipLength * 1000.f);
	GetDlgItem(IDC_EDIT_SIZE_Y)->SetWindowText(strTemp);

	UpdateData(FALSE);

	Invalidate(FALSE);
}

void CChipSelectType::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	long nDataSize = (long)m_pPassiveJob->m_vecsChipData.size();
	long nCursel = m_ComboChipType.GetCurSel();

	if(nCursel < 0 || nCursel >= nDataSize) return;

	CRect rtViewer;
	((CStatic*)GetDlgItem(IDC_STATIC_COLOR))->GetWindowRect(rtViewer);
	ScreenToClient(rtViewer);

	CBrush brush(m_pPassiveJob->m_vecsChipData[nCursel].GroupColor);
	CBrush *pOldBrush = dc.SelectObject(&brush);

	dc.Rectangle(rtViewer);

	dc.SelectObject(pOldBrush);
}
