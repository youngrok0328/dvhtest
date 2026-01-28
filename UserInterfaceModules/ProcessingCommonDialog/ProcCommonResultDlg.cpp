//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ProcCommonResultDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionResultGroup.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GRID_NUM_X 6
#define IDC_GRID_RESULT 100

//CPP_7_________________________________ Implementation body
//
enum enumColindex
{
    Colindex_Number,
    Colindex_Item,
    Colindex_Result,
    Colindex_Value,
    Colindex_Unit,
    Colindex_Detail,
};

IMPLEMENT_DYNAMIC(ProcCommonResultDlg, CDialog)

ProcCommonResultDlg::ProcCommonResultDlg(CWnd* pParent, const CRect& rtPositionOnParent,
    VisionInspectionResultGroup& inspectionResultGroup, ImageLotView& imageLotView, const wchar_t* jobName,
    UINT uiDetailButtonClickedMsg)
    : CDialog(IDD_DIALOG_RESULT, pParent)
    , m_rtInitPosition(rtPositionOnParent)
    , m_inspectionResultGroup(inspectionResultGroup)
    , m_imageLotView(imageLotView)
    , m_jobName(jobName)
    , m_uiDetailButtonClickedMsg(uiDetailButtonClickedMsg)
    , m_pGridCtrl(new CGridCtrl)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    VERIFY(Create(IDD_DIALOG_RESULT, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

ProcCommonResultDlg::~ProcCommonResultDlg()
{
    delete m_pGridCtrl;
}

void ProcCommonResultDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ProcCommonResultDlg, CDialog)
ON_NOTIFY(GVN_SELCHANGED, IDC_GRID_RESULT, OnGridSelChanged)
ON_NOTIFY(NM_DBLCLK, IDC_GRID_RESULT, OnDblclkGridResult)
END_MESSAGE_MAP()

// ProcCommonResultDlg 메시지 처리기입니다.

BOOL ProcCommonResultDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_rtInitPosition, FALSE);

    CRect rect;
    GetClientRect(rect);

    m_pGridCtrl->Create(rect, this, IDC_GRID_RESULT);
    m_pGridCtrl->SetFont(GetFont(), FALSE);
    m_pGridCtrl->SetEditable(TRUE);
    m_pGridCtrl->EnableDragAndDrop(FALSE);

    // Grid 가로 세로 개수 설정
    long nResultNum = (long)(m_inspectionResultGroup.m_vecResult.size());
    m_pGridCtrl->SetColumnCount(GRID_NUM_X);
    m_pGridCtrl->SetRowCount(nResultNum + 1);
    m_pGridCtrl->SetFixedRowCount(1);
    m_pGridCtrl->SetFixedColumnCount(1);
    m_pGridCtrl->SetEditable(FALSE);

    // Grid 정렬
    m_pGridCtrl->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_pGridCtrl->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_pGridCtrl->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 0 번째 Row 에 각각의 Title 을 정의
    m_pGridCtrl->SetItemText(0, Colindex_Number, _T("No"));
    m_pGridCtrl->SetItemText(0, Colindex_Item, _T("Item"));
    m_pGridCtrl->SetItemText(0, Colindex_Result, _T("Result"));
    m_pGridCtrl->SetItemText(0, Colindex_Value, _T("Value"));
    m_pGridCtrl->SetItemText(0, Colindex_Unit, _T("Unit"));
    m_pGridCtrl->SetItemText(0, Colindex_Detail, _T("Detail"));

    m_pGridCtrl->SetItemBkColour(0, 0, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 1, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 2, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 3, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 4, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 5, RGB(200, 200, 250));

    m_pGridCtrl->SetColumnWidth(0, 30);
    m_pGridCtrl->SetColumnWidth(1, 200);
    m_pGridCtrl->SetColumnWidth(4, 40);

    Refresh();

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void ProcCommonResultDlg::OnDblclkGridResult(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;
    if (pGridView == NULL)
        return;

    CCellID CellID = m_pGridCtrl->GetFocusCell();

    if (CellID.col == Colindex_Detail && m_uiDetailButtonClickedMsg >= WM_USER)
    {
        GetParent()->PostMessage(m_uiDetailButtonClickedMsg, WPARAM(CellID.row - 1));
    }
    else if (CellID.col == Colindex_Item
        && m_inspectionResultGroup.m_vecResult[CellID.row - 1].m_objectValues.size()
            > 0) //선택한 Col이 Item Index && Item Index의 Data가 0이상일 때
    {
        if (!ModfiyItemidx(CellID.row - 1))
            return;
    }

    *pResult = 0;
}

bool ProcCommonResultDlg::ModfiyItemidx(long i_nRowidx)
{
    CString strinspName = m_inspectionResultGroup.m_vecResult[i_nRowidx].m_resultName;

    if (strinspName.Find(_T("Copl")) > 0 || strinspName.Find(_T("Warpage")) > 0)
    {
        if (strinspName.Find(_T("Unit")) < 0) //Unit Copl, Warpage는 수집 대상이 아니다
        {
            if (!SaveDetailValue_CSVFile(m_inspectionResultGroup, i_nRowidx))
            {
                ::AfxMessageBox(_T("Not Excute .CSV File Please Checking"));
                return false;
            }
        }
        else
            return false;
    }
    else
        return false;

    return true;
}

bool ProcCommonResultDlg::SaveDetailValue_CSVFile(
    VisionInspectionResultGroup i_VisionInspectionResultGroup, long i_nRowidx)
{
    CString strSaveDirectory = DynamicSystemPath::get(DefineFolder::Pacakge_ViewerData);
    CString strJobName = m_jobName;
    if (strJobName.IsEmpty())
        return false;

    //오늘 날짜 디렉토리 생성
    CString strDayFolerName("");
    SYSTEMTIME Time;
    Ipvm::TimeCheck::GetLocalTime(Time);
    strDayFolerName.Format(_T("%d.%d.%d\\"), Time.wYear, Time.wMonth, Time.wDay);

    CString strSaveDirectoryPath_Day = strSaveDirectory + strDayFolerName;
    if (GetFileAttributes(strSaveDirectoryPath_Day) == INVALID_FILE_ATTRIBUTES)
        CreateDirectory(strSaveDirectoryPath_Day, NULL);

    CString strSaveDirectoryPath_Job = strSaveDirectoryPath_Day + strJobName;
    if (GetFileAttributes(strSaveDirectoryPath_Job) == INVALID_FILE_ATTRIBUTES)
        CreateDirectory(strSaveDirectoryPath_Job, NULL);

    //File Name
    CString strFilePath;
    strFilePath.Format(_T("\\%dH-%dM"), Time.wHour, Time.wMinute);

    //Write
    CString strWriteData_Type = _T("ObjectID,ObjectPosX,ObejctPosY,Value\n");
    CString strWriteDataTemp = strWriteData_Type;

    for (long nidx = 0; nidx < i_VisionInspectionResultGroup.m_vecResult[i_nRowidx].m_objectValues.size(); nidx++)
    {
        CString strWriteData("");
        strWriteData.Format(_T("%s, %f, %f, %.2f\n"),
            (LPCTSTR)i_VisionInspectionResultGroup.m_vecResult[i_nRowidx].m_objectNames[nidx],
            (i_VisionInspectionResultGroup.m_vecResult[i_nRowidx].m_objectPositionX[nidx] * 0.001),
            (i_VisionInspectionResultGroup.m_vecResult[i_nRowidx].m_objectPositionY[nidx] * 0.001),
            i_VisionInspectionResultGroup.m_vecResult[i_nRowidx].m_objectErrorValues[nidx]);

        strWriteDataTemp += strWriteData;
    }

    CString strInspectionName = i_VisionInspectionResultGroup.m_vecResult[i_nRowidx].m_resultName;
    CString strFileName = strSaveDirectoryPath_Job + strFilePath + _T("_") + strInspectionName + _T(".csv");
    FILE* fp = nullptr;
    _tfopen_s(&fp, strFileName, _T("w"));
    if (fp == nullptr)
        return false;

    fprintf(fp, CStringA(strWriteDataTemp));

    fclose(fp);

    return true;
}

void ProcCommonResultDlg::OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;

    *pResult = 0;

    if (pGridView->iColumn == 5)
    {
        return;
    }

    const long itemIndex = pGridView->iRow - 1;

    if (itemIndex < 0 || (itemIndex >= m_inspectionResultGroup.m_vecResult.size()))
    {
        m_imageLotView.Overlay_RemoveAll();
        m_imageLotView.Overlay_Show(TRUE);
        return;
    }

    m_imageLotView.Overlay_RemoveAll();

    const auto& result = m_inspectionResultGroup.m_vecResult[itemIndex];

    long nRejectSize = (long)(result.vecrtRejectROI.size());
    long nMarginalSize = (long)(result.vecrtMarginalROI.size());

    for (long i = 0; i < nRejectSize; i++)
    {
        m_imageLotView.Overlay_AddRectangle(result.vecrtRejectROI[i], RGB(255, 0, 0));
    }

    for (long i = 0; i < nMarginalSize; i++)
    {
        m_imageLotView.Overlay_AddRectangle(result.vecrtMarginalROI[i], RGB(243, 157, 58));
    }

    m_imageLotView.Overlay_Show(TRUE);
}

void ProcCommonResultDlg::Refresh()
{
    m_pGridCtrl->SetRedraw(FALSE);

    // Grid 정렬
    long nResultNum = (long)(m_inspectionResultGroup.m_vecResult.size());

    m_pGridCtrl->SetRowCount(nResultNum + 1);

    long i;
    CString str;
    for (i = 0; i < nResultNum; i++)
    {
        auto& result = m_inspectionResultGroup.m_vecResult[i];

        m_pGridCtrl->SetItemBkColour(i + 1, 0, RGB(250, 250, 220));

        // ID
        str.Format(_T("%d"), i);
        m_pGridCtrl->SetItemText(i + 1, 0, str);
        m_pGridCtrl->SetItemFormat(i + 1, 0, DT_LEFT);

        // Name
        m_pGridCtrl->SetItemText(i + 1, 1, result.m_resultName);
        m_pGridCtrl->SetItemFormat(i + 1, 1, DT_LEFT);

        // Result
        m_pGridCtrl->SetItemText(i + 1, 2, Result2String(result.m_totalResult));
        m_pGridCtrl->SetItemBkColour(i + 1, 2, Result2Color(result.m_totalResult));
        m_pGridCtrl->SetItemFormat(i + 1, 2, DT_LEFT);

        // Value
        if (result.getObjectWorstErrorValue() == Ipvm::k_noiseValue32r)
        {
            str = _T("INV");
        }
        else
        {
            str.Format(_T("%.2f"), result.getObjectWorstErrorValue());
        }
        m_pGridCtrl->SetItemText(i + 1, 3, str);
        m_pGridCtrl->SetItemFormat(i + 1, 3, DT_LEFT);

        // Unit
        m_pGridCtrl->SetItemText(i + 1, 4, result.m_itemUnit);
        m_pGridCtrl->SetItemFormat(i + 1, 4, DT_LEFT);

        // Detail
        m_pGridCtrl->SetItemText(i + 1, 5, _T(">>"));
        m_pGridCtrl->SetItemFormat(i + 1, 5, DT_LEFT);
    }

    m_pGridCtrl->SetRedraw(TRUE, TRUE);
}
