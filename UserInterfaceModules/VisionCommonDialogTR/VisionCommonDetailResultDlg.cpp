//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionCommonDetailResultDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GRID_NUM_X 6
#define IDC_GRID_RESULT_DETAIL 100

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(VisionCommonDetailResultDlg, CDialog)

VisionCommonDetailResultDlg::VisionCommonDetailResultDlg(
    CWnd* pParent, const CRect& rtPositionOnParent, VisionUnit& visionUnit, ImageLotView& imageLotView)
    : CDialog(IDD_DIALOG_RESULT_DETAIL, pParent)
    , m_rtInitPosition(rtPositionOnParent)
    , m_visionUnit(&visionUnit)
    , m_imageLotView(imageLotView)
    , m_pGridCtrl(new CGridCtrl)
    , m_nCurGrid(0)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    VERIFY(Create(IDD_DIALOG_RESULT_DETAIL, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

VisionCommonDetailResultDlg::~VisionCommonDetailResultDlg()
{
    ClearVisionInspectionResult();
    delete m_pGridCtrl;
}

void VisionCommonDetailResultDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(VisionCommonDetailResultDlg, CDialog)
ON_NOTIFY(GVN_SELCHANGED, IDC_GRID_RESULT_DETAIL, OnGridSelChanged)
ON_WM_SIZE()
END_MESSAGE_MAP()

// VisionCommonDetailResultDlg 메시지 처리기입니다.

BOOL VisionCommonDetailResultDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_rtInitPosition, FALSE);
    CRect rect;
    GetClientRect(rect);

    m_pGridCtrl->Create(rect, this, IDC_GRID_RESULT_DETAIL);
    m_pGridCtrl->SetFont(GetFont(), FALSE);
    m_pGridCtrl->SetEditable(FALSE);
    m_pGridCtrl->EnableDragAndDrop(FALSE);

    long nResultNum = 0;

    // Grid 가로 세로 개수 설정
    m_pGridCtrl->SetColumnCount(GRID_NUM_X);
    m_pGridCtrl->SetRowCount(nResultNum + 1);
    m_pGridCtrl->SetFixedRowCount(1);

    // 0 번째 Row 에 각각의 Title 을 정의
    m_pGridCtrl->SetItemText(0, 0, _T("NO"));
    m_pGridCtrl->SetItemText(0, 1, _T("ID"));
    m_pGridCtrl->SetItemText(0, 2, _T("X"));
    m_pGridCtrl->SetItemText(0, 3, _T("Y"));
    m_pGridCtrl->SetItemText(0, 4, _T("Value"));
    m_pGridCtrl->SetItemText(0, 5, _T("Result"));
    m_pGridCtrl->SetColumnWidth(0, 50);
    m_pGridCtrl->SetColumnWidth(1, 50);

    m_pGridCtrl->SetHeaderSort();

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void VisionCommonDetailResultDlg::SetTarget(VisionUnit& visionUnit)
{
    m_visionUnit = &visionUnit;
}

bool VisionCommonDetailResultDlg::Refresh(long itemIndex)
{
    const long nInspResultNum = (long)m_vecpVisionInspectionResult.size();
    const bool bIsUseResultInVisionUnit = (nInspResultNum <= 0);

    auto inspectionResults
        = bIsUseResultInVisionUnit ? m_visionUnit->GetVisionInspectionResults() : m_vecpVisionInspectionResult;

    if (itemIndex < -1)
    {
        m_nCurGrid;
    }
    else if (itemIndex >= 0 && itemIndex < inspectionResults.size())
    {
        m_nCurGrid = itemIndex;
    }
    else
    {
        m_pGridCtrl->SetRedraw(FALSE);
        m_pGridCtrl->SetRowCount(1);
        m_pGridCtrl->SetFixedRowCount(2);
        m_pGridCtrl->SetRedraw(TRUE, TRUE);
        return false;
    }

    long nResultNum = 0;
    if (inspectionResults.size() > 0)
    {
        if (m_nCurGrid >= 0 && m_nCurGrid < (long)(inspectionResults.size()))
        {
            nResultNum = (long)(inspectionResults[m_nCurGrid]->m_objectResults.size());
        }
    }

    m_pGridCtrl->SetRedraw(FALSE);

    // Grid 가로 세로 개수 설정
    m_pGridCtrl->SetRowCount(nResultNum + 1);
    m_pGridCtrl->SetFixedRowCount(1);

    if (nResultNum <= 0)
    {
        m_pGridCtrl->SetRedraw(TRUE, TRUE);
        return true;
    }

    const auto& result = inspectionResults[m_nCurGrid];

    // 값 채우기
    CString strObjectName;
    for (long i = 0; i < nResultNum; i++)
    {
        // NO
        m_pGridCtrl->SetItemTextFmt(i + 1, 0, _T("%d"), i + 1);

        // ID
        strObjectName = result->m_objectNames[i];
        if (strObjectName.IsEmpty())
            m_pGridCtrl->SetItemTextFmt(i + 1, 0, _T("%d"), i + 1);
        else
            m_pGridCtrl->SetItemText(i + 1, 1, strObjectName);

        // X
        m_pGridCtrl->SetItemTextFmt(i + 1, 2, _T("%d"), result->m_objectRects[i].CenterPoint().m_x);

        // Y
        m_pGridCtrl->SetItemTextFmt(i + 1, 3, _T("%d"), result->m_objectRects[i].CenterPoint().m_y);

        // Value
        if (result->m_objectErrorValues[i] == Ipvm::k_noiseValue32r)
        {
            m_pGridCtrl->SetItemTextFmt(i + 1, 4, _T("INV"));
        }
        else
        {
            m_pGridCtrl->SetItemTextFmt(i + 1, 4, _T("%.2f"), result->m_objectErrorValues[i]);
        }

        // Result
        m_pGridCtrl->SetItemText(i + 1, 5, Result2String(result->m_objectResults[i]));
        m_pGridCtrl->SetItemBkColour(i + 1, 5, Result2Color(result->m_objectResults[i]));
    }

    m_pGridCtrl->SetRedraw(TRUE, TRUE);

    return true;
}

void VisionCommonDetailResultDlg::OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;

    *pResult = 0;

    const long nInspResultNum = (long)m_vecpVisionInspectionResult.size();
    const bool bIsUseResultInVisionUnit = (nInspResultNum <= 0);

    auto inspectionResults
        = bIsUseResultInVisionUnit ? m_visionUnit->GetVisionInspectionResults() : m_vecpVisionInspectionResult;

    const long index = ::_ttoi(m_pGridCtrl->GetItemText(pGridView->iRow, 0)) - 1;

    if (index < 0 || index >= inspectionResults[m_nCurGrid]->m_objectNames.size())
    {
        return;
    }

    const auto& rect = inspectionResults[m_nCurGrid]->m_objectRects[index];

    Ipvm::Point32s2 ptCenter = rect.CenterPoint();

    m_imageLotView.Overlay_RemoveAll();

    Ipvm::Point32s2 ptX_Begin, ptX_End, ptY_Begin, ptY_End;
    ptX_Begin.Set(ptCenter.m_x - 40, ptCenter.m_y);
    ptX_End.Set(ptCenter.m_x + 40, ptCenter.m_y);

    ptY_Begin.Set(ptCenter.m_x, ptCenter.m_y - 40);
    ptY_End.Set(ptCenter.m_x, ptCenter.m_y + 40);

    m_imageLotView.Overlay_AddLine(ptX_Begin, ptX_End, RGB(0, 255, 0));
    m_imageLotView.Overlay_AddLine(ptY_Begin, ptY_End, RGB(0, 255, 0));

    m_imageLotView.Overlay_Show(TRUE);
}

bool VisionCommonDetailResultDlg::CollectVisionInspectionResult()
{
    ClearVisionInspectionResult();
    auto inspectionResults = m_visionUnit->GetVisionInspectionResults();
    const long nInspResultNum = (long)inspectionResults.size();

    if (nInspResultNum <= 0)
        return false;

    m_vecpVisionInspectionResult.resize(nInspResultNum);
    for (long nIdx = 0; nIdx < nInspResultNum; nIdx++)
    {
        m_vecpVisionInspectionResult[nIdx] = new VisionInspectionResult(*inspectionResults[nIdx]);
    }

    return true;
}

void VisionCommonDetailResultDlg::ClearVisionInspectionResult()
{
    const long nInspResultNum = (long)m_vecpVisionInspectionResult.size();
    if (nInspResultNum <= 0)
        return;

    for (long nIdx = 0; nIdx < nInspResultNum; nIdx++)
        delete m_vecpVisionInspectionResult[nIdx];

    m_vecpVisionInspectionResult.clear();
}

void VisionCommonDetailResultDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (m_pGridCtrl->GetSafeHwnd())
    {
        CRect rect;
        GetClientRect(rect);

        m_pGridCtrl->MoveWindow(rect, TRUE);
    }
}
