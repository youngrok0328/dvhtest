//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionBatchInspectionUI.h"

//CPP_2_________________________________ This project's headers
#include "DlgPrimaryBatchList.h"
#include "DlgRnRHelper.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/VisionCommonDialog/VisionCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialog/VisionCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialog/VisionCommonResultDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialog/VisionCommonSpecDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialog/VisionCommonTextResultDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgVisionBatchInspectionUI, CDialog)

DlgVisionBatchInspectionUI::DlgVisionBatchInspectionUI(
    VisionMainAgent& visionMainAgent, VisionUnit& visionUnit, CWnd* pParent /*=NULL*/)
    : CDialog(DlgVisionBatchInspectionUI::IDD, pParent)
    , m_visionMainAgent(visionMainAgent)
    , m_visionUnit(visionUnit)
    , m_batchList(nullptr)
    , m_imageLotView(nullptr)
    , m_visionCommonSpecDlg(nullptr)
    , m_visionCommonResultDlg(nullptr)
    , m_visionCommonDetailResultDlg(nullptr)
    , m_visionCommonTextResultDlg(nullptr)
    , m_visionCommonDebugInfoDlg(nullptr)
{
}

DlgVisionBatchInspectionUI::~DlgVisionBatchInspectionUI()
{
    delete m_visionCommonDebugInfoDlg;
    delete m_visionCommonTextResultDlg;
    delete m_visionCommonDetailResultDlg;
    delete m_visionCommonResultDlg;
    delete m_visionCommonSpecDlg;
    delete m_imageLotView;
    delete m_batchList;
}

void DlgVisionBatchInspectionUI::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_tabResult);
}

BEGIN_MESSAGE_MAP(DlgVisionBatchInspectionUI, CDialog)
ON_WM_DESTROY()
ON_WM_CLOSE()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &DlgVisionBatchInspectionUI::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &DlgVisionBatchInspectionUI::OnShowDetailResult)
END_MESSAGE_MAP()

// DlgVisionBatchInspectionUI 메시지 처리기입니다.
BOOL DlgVisionBatchInspectionUI::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtClient;
    GetClientRect(&rtClient);
    rtClient.DeflateRect(5, 5);

    m_imageLotView
        = new ImageLotView(::GetContentSubArea(rtClient, 2, 4, 4, 0, 3, 3), m_visionUnit, true, GetSafeHwnd());

    m_batchList = new CDlgPrimaryBatchList(
        m_visionMainAgent, m_visionUnit, ::GetContentSubArea(rtClient, 0, 2, 4, 0, 2, 3), this);
    m_batchList->Create(CDlgPrimaryBatchList::IDD, this);
    m_batchList->ShowWindow(SW_SHOW);

    CRect rtTab = ::GetContentSubArea(rtClient, 0, 2, 4, 2, 3, 3);
    m_tabResult.MoveWindow(rtTab);

    m_tabResult.InsertItem(0, _T("Spec."));
    m_tabResult.InsertItem(1, _T("Result"));
    m_tabResult.InsertItem(2, _T("Detail"));
    m_tabResult.InsertItem(3, _T("Debug"));
    m_tabResult.InsertItem(4, _T("Log"));

    m_tabResult.AdjustRect(FALSE, rtTab);

    m_visionCommonSpecDlg = new VisionCommonSpecDlg(this, rtTab, m_visionUnit);
    m_visionCommonResultDlg
        = new VisionCommonResultDlg(this, rtTab, m_visionUnit, *m_imageLotView, UM_SHOW_DETAIL_RESULT);
    m_visionCommonDetailResultDlg = new VisionCommonDetailResultDlg(this, rtTab, m_visionUnit, *m_imageLotView);
    m_visionCommonTextResultDlg = new VisionCommonTextResultDlg(this, rtTab);
    m_visionCommonDebugInfoDlg
        = new VisionCommonDebugInfoDlg(this, rtTab, m_visionUnit, *m_imageLotView, *m_visionCommonTextResultDlg);

    m_visionCommonSpecDlg->ShowWindow(SW_SHOW);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void DlgVisionBatchInspectionUI::OnDestroy()
{
    CDialog::OnDestroy();

    m_visionCommonDebugInfoDlg->DestroyWindow();
    m_visionCommonTextResultDlg->DestroyWindow();
    m_visionCommonResultDlg->DestroyWindow();
    m_visionCommonDetailResultDlg->DestroyWindow();
    m_visionCommonSpecDlg->DestroyWindow();
    m_batchList->DestroyWindow();
}

void DlgVisionBatchInspectionUI::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_tabResult.GetCurSel();

    m_visionCommonSpecDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_visionCommonResultDlg->ShowWindow((nTab == 1) ? SW_SHOW : SW_HIDE);
    m_visionCommonDetailResultDlg->ShowWindow((nTab == 2) ? SW_SHOW : SW_HIDE);
    m_visionCommonDebugInfoDlg->ShowWindow(nTab == 3 ? SW_SHOW : SW_HIDE);
    m_visionCommonTextResultDlg->ShowWindow(nTab == 4 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

LRESULT DlgVisionBatchInspectionUI::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
{
    const long itemIndex = long(wparam);

    if (itemIndex < 0)
    {
        m_tabResult.SetCurSel(1);
    }
    else
    {
        m_tabResult.SetCurSel(2);

        m_visionCommonDetailResultDlg->Refresh(itemIndex);
    }

    OnTcnSelchangeTabResult(nullptr, nullptr);

    return 0L;
}

void DlgVisionBatchInspectionUI::OnJobChanged()
{
    m_visionCommonDebugInfoDlg->Refresh();
    m_visionCommonResultDlg->Refresh();
    m_visionCommonSpecDlg->Refresh();
    m_batchList->Refresh();
}

BOOL DlgVisionBatchInspectionUI::isInspectionEnable()
{
    return m_batchList->isInspectionEnable();
}

BOOL DlgVisionBatchInspectionUI::isImageOpenEnable()
{
    return m_batchList->isImageOpenEnable();
}

void DlgVisionBatchInspectionUI::call_inspection()
{
    m_batchList->call_inspection();
}

void DlgVisionBatchInspectionUI::call_inspection(long nRunMode)
{
    m_batchList->call_inspection(nRunMode);
}

void DlgVisionBatchInspectionUI::call_openImageFiles()
{
    m_batchList->call_openImageFiles();
}

void DlgVisionBatchInspectionUI::call_openImageFiles(std::vector<CString> FileNames)
{
    m_batchList->call_openImageFilesString(FileNames);
}

void DlgVisionBatchInspectionUI::call_quit()
{
    m_batchList->call_quit();
}
