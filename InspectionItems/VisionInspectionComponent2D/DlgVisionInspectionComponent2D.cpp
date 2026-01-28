//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionComponent2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgChangeSpecName.h"
#include "DlgPassiveAlgorithm.h"
#include "DlgSpecDB.h"
#include "PassiveAlign.h"
#include "VisionInspectionComponent2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)

//CPP_7_________________________________ Implementation body
//
enum
{
    COLUMM_NO,
    COLUMN_CHIP,
    COLUMN_ANGLE,
    COLUMN_TYPES,
    COLUMN_SPEC,
    COLUMN_RESULT,
};

enum PropertyGridItemID
{
    ITEM_ID_BUTTON_ALGORITHM_DB,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionComponent2D, CDialog)

CDlgVisionInspectionComponent2D::CDlgVisionInspectionComponent2D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionComponent2D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionComponent2D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_imageLotView(nullptr)
    , m_propertyGrid(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_packageSpec(pVisionInsp->m_packageSpec)
{
    m_pDlgSpecDB = new CDlgSpecDB(pVisionInsp);

    m_pDlgPassiveAlgorithm = new CDlgPassiveAlgorithm(pVisionInsp);

    m_nImageID = 0;

    m_pVisionPara = m_pVisionInsp->m_VisionPara;
}

CDlgVisionInspectionComponent2D::~CDlgVisionInspectionComponent2D()
{
    m_pVisionInsp->SetVisionInspectionSpecs();

    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;
    delete m_propertyGrid;
    delete m_imageLotView;

    if (m_pDlgSpecDB)
    {
        delete m_pDlgSpecDB;
        m_pDlgSpecDB = NULL;
    }

    if (m_pDlgPassiveAlgorithm)
    {
        delete m_pDlgPassiveAlgorithm;
        m_pDlgPassiveAlgorithm = NULL;
    }

    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionInspectionComponent2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_CHIP_LIST, m_ChipListControl);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionComponent2D, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionComponent2D::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionComponent2D::OnShowDetailResult)

ON_NOTIFY(NM_KEYDOWN, IDC_CHIP_LIST, OnReportKeyDown)
ON_NOTIFY(NM_CLICK, IDC_CHIP_LIST, OnReportItemClick)
ON_NOTIFY(NM_RCLICK, IDC_CHIP_LIST, OnReportRClick)
ON_NOTIFY(NM_DBLCLK, IDC_CHIP_LIST, OnReportItemDblClick)

ON_COMMAND(ID_MENU_DELETE, &CDlgVisionInspectionComponent2D::OnMenuDelete)
ON_COMMAND(ID_MENU_DMS_INFO, &CDlgVisionInspectionComponent2D::OnMenuDmsInfo)
ON_COMMAND(ID_MENU_CHANGE_SPEC, &CDlgVisionInspectionComponent2D::OnMenuChangeSpec)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionComponent2D::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionComponent2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    // Dialog Control 위치 설정
    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Spec."));
    m_TabResult.InsertItem(1, _T("Result"));
    m_TabResult.InsertItem(2, _T("Detail"));
    m_TabResult.InsertItem(3, _T("Debug"));
    m_TabResult.InsertItem(4, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    //m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionInsp->m_fixedInspectionSpecs);
    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionPara->m_vecVisionInspectionSpecs);
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    //FrameIndex 확인 및 초기화
    m_pVisionInsp->VerifyFrame();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID);
    m_propertyGrid->ShowHelp(FALSE);

    SetPropertyGrid();
    SetpropertyList();

    m_pVisionInsp->MakePreInspInfo(true);
    UpdateChipList();

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionInspectionComponent2D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_pDlgPassiveAlgorithm->Create(CDlgPassiveAlgorithm::IDD);
    m_pDlgPassiveAlgorithm->ShowWindow(SW_HIDE);

    m_pDlgSpecDB->Create(CDlgSpecDB::IDD);
    m_pDlgSpecDB->ShowWindow(SW_HIDE);

    return 0;
}

void CDlgVisionInspectionComponent2D::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();

    //	m_pDlgAlgoPara->DestroyWindow();
    m_pDlgSpecDB->DestroyWindow();
    m_pDlgPassiveAlgorithm->DestroyWindow();
}

void CDlgVisionInspectionComponent2D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionComponent2D::SetPropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("2D Passive Parameter")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Algorithm DB"), TRUE, FALSE))
            ->SetID(ITEM_ID_BUTTON_ALGORITHM_DB);
        m_propertyGrid->FindItem(ITEM_ID_BUTTON_ALGORITHM_DB)->SetHeight(50);
        category->Expand();
    }
    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->Refresh();
}

void CDlgVisionInspectionComponent2D::SetpropertyList()
{
    CRect rtTemp;
    rtTemp = m_procDlgInfo.m_rtParaArea;
    rtTemp.top = rtTemp.top + 45;

    m_ChipListControl.MoveWindow(rtTemp);

    int nLen = (rtTemp.Width() - 110 - GetSystemMetrics(SM_CXVSCROLL)) / 3;
    m_ChipListControl.AddColumn(new CXTPGridColumn(COLUMM_NO, _T("No."), 50));
    m_ChipListControl.AddColumn(new CXTPGridColumn(COLUMN_CHIP, _T("Chip"), 70));
    m_ChipListControl.AddColumn(new CXTPGridColumn(COLUMN_ANGLE, _T("Angle"), 70));
    m_ChipListControl.AddColumn(new CXTPGridColumn(COLUMN_TYPES, _T("Type"), nLen));
    m_ChipListControl.AddColumn(new CXTPGridColumn(COLUMN_SPEC, _T("Spec"), nLen));

    m_ChipListControl.Populate();
    m_ChipListControl.SetMultipleSelection(TRUE);
    m_ChipListControl.SetTheme(xtpGridThemeVisualStudio2012Light);
    m_ChipListControl.SetGridStyle(0, xtpGridSmallDots);
    COLORREF rgbGridColor = RGB(0, 0, 0);
    m_ChipListControl.SetGridColor(rgbGridColor);
}

LRESULT CDlgVisionInspectionComponent2D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_BUTTON_ALGORITHM_DB:
                OnBnClickedBtnChipSpecdb();
                break;
        }

        value->SetBool(FALSE);
    }
    m_propertyGrid->Refresh();

    return 0;
}

void CDlgVisionInspectionComponent2D::UpdateChipList()
{
    m_ChipListControl.ResetContent(FALSE);

    if (m_pVisionInsp->m_vecsPassiveInfoDB.size() < 0)
        return;

    for (long nItem = 0; nItem < m_pVisionInsp->m_vecsPassiveInfoDB.size(); nItem++)
    {
        auto PassiveDBInfo = m_pVisionInsp->m_vecsPassiveInfoDB[nItem];
        auto* repRecord = new CXTPGridRecord();
        // No.
        CString strTemp("");
        strTemp.Format(_T("%d"), nItem + 1);
        repRecord->AddItem(new CXTPGridRecordItemText(strTemp));

        // PassiveID
        repRecord->AddItem(new CXTPGridRecordItemText(PassiveDBInfo.strCompName));

        // Angle
        strTemp.Format(_T("%.2f"), PassiveDBInfo.fPassiveAngle);
        repRecord->AddItem(new CXTPGridRecordItemText(strTemp));

        // Type
        repRecord->AddItem(new CXTPGridRecordItemText(PassiveDBInfo.strCompType));

        // Spec
        repRecord->AddItem(new CXTPGridRecordItemText(PassiveDBInfo.strSpecName));

        m_ChipListControl.AddRecord(repRecord);

        strTemp.Empty();
    }

    m_ChipListControl.Populate();
}

void CDlgVisionInspectionComponent2D::OnReportItemDblClick(NMHDR* pNotifyStruct, LRESULT* /*result*/)
{
    XTP_NM_GRIDRECORDITEM* pItemNotify = (XTP_NM_GRIDRECORDITEM*)pNotifyStruct;

    if (!pItemNotify->pRow || !pItemNotify->pColumn)
        return;

    if (m_pVisionInsp->m_vecsPassiveInfoDB.size() < 0)
        return;

    long nID = pItemNotify->pRow->GetRecord()->GetIndex();
    long nMaxPassiveCount = (long)m_pVisionInsp->m_vecsPassiveInfoDB.size();
    if (nID < 0 || nID >= nMaxPassiveCount)
        return;

    auto pPassiveInfoDB = &m_pVisionInsp->m_vecsPassiveInfoDB[nID];
    m_pCurPassiveInfoDB = pPassiveInfoDB;
    if (pPassiveInfoDB->eComponentType == eComponentTypeDefine::tyPASSIVE)
    {
        switch (pPassiveInfoDB->ePassiveType)
        {
            case enumPassiveTypeDefine::enum_PassiveType_Capacitor:
            case enumPassiveTypeDefine::enum_PassiveType_Register:
            case enumPassiveTypeDefine::enum_PassiveType_MIA:
                if (m_pDlgPassiveAlgorithm && m_pVisionInsp->m_pPassiveAlign)
                {
                    HideItemDialog();
                    PassiveAlignSpec::CapAlignSpec* pSpec
                        = m_pVisionInsp->m_pPassiveAlign->GetCapAlignSpec(pPassiveInfoDB->strSpecName);
                    if (pSpec)
                    {
                        m_pDlgPassiveAlgorithm->ShowWindow(SW_SHOW);
                    }
                }
                break;
        }
    }
}

void CDlgVisionInspectionComponent2D::OnReportItemClick(NMHDR* pNotifyStruct, LRESULT* result)
{
    XTP_NM_GRIDRECORDITEM* pItemNotify = (XTP_NM_GRIDRECORDITEM*)pNotifyStruct;

    if (!pItemNotify->pRow || !pItemNotify->pColumn)
        return;

    m_imageLotView->ROI_RemoveAll();
    m_imageLotView->Overlay_RemoveAll();

    long nID = pItemNotify->pRow->GetRecord()->GetIndex();
    long nMaxPassiveCount = (long)m_pVisionInsp->m_vecsPassiveInfoDB.size();
    if (nID < 0 || nID >= nMaxPassiveCount)
        return;

    std::vector<long> nvecID(0);
    long nSelCnt = m_ChipListControl.GetSelectedRows()->GetCount();
    POSITION pos = m_ChipListControl.GetSelectedRows()->GetFirstSelectedRowPosition();
    while (pos)
    {
        int nItem = m_ChipListControl.GetSelectedRows()->GetNextSelectedRow(pos)->GetRecord()->GetIndex();
        nvecID.push_back(nItem);
    }
    VERIFY(nSelCnt == (long)nvecID.size());

    for (long i = 0; i < nvecID.size(); i++)
    {
        auto PassiveInfoDB = m_pVisionInsp->m_vecsPassiveInfoDB[nvecID[i]];
        m_imageLotView->Overlay_AddRectangle(PassiveInfoDB.sfrtSpecROI_px, RGB(0, 255, 0));
    }

    m_imageLotView->Overlay_Show(TRUE);

    *result = 0;
}

void CDlgVisionInspectionComponent2D::OnReportKeyDown(NMHDR* pNotifyStruct, LRESULT* /*result*/)
{
    LPNMKEY lpNMKey = (LPNMKEY)pNotifyStruct;

    if (!m_ChipListControl.GetFocusedRow())
        return;

    if (lpNMKey->nVKey == VK_DOWN || lpNMKey->nVKey == VK_UP)
    {
        m_imageLotView->ROI_RemoveAll();
        m_imageLotView->Overlay_RemoveAll();

        long nID = m_ChipListControl.GetFocusedRow()->GetRecord()->GetIndex();
        long nMaxPassiveCount = (long)m_pVisionInsp->m_vecsPassiveInfoDB.size();
        if (nID < 0 || nID >= nMaxPassiveCount)
            return;

        std::vector<long> nvecID(0);
        long nSelCnt = m_ChipListControl.GetSelectedRows()->GetCount();
        POSITION pos = m_ChipListControl.GetSelectedRows()->GetFirstSelectedRowPosition();
        while (pos)
        {
            int nItem = m_ChipListControl.GetSelectedRows()->GetNextSelectedRow(pos)->GetRecord()->GetIndex();
            nvecID.push_back(nItem);
        }
        VERIFY(nSelCnt == (long)nvecID.size());

        for (long i = 0; i < nvecID.size(); i++)
        {
            m_imageLotView->Overlay_AddRectangle(
                m_pVisionInsp->m_vecsPassiveInfoDB[nvecID[i]].sfrtSpecROI_px, RGB(0, 255, 0));
        }

        m_imageLotView->Overlay_Show(TRUE);
    }
}

void CDlgVisionInspectionComponent2D::OnReportRClick(NMHDR* pNotifyStruct, LRESULT* pResult)
{
    VERIFY(NULL != pNotifyStruct);
    VERIFY(NULL != pResult);

    XTP_NM_GRIDRECORDITEM* pItemNotify = (XTP_NM_GRIDRECORDITEM*)pNotifyStruct;
    ASSERT(pItemNotify->pColumn);
    CPoint ptClick = pItemNotify->pt;

    if (m_pVisionInsp->m_vecsPassiveInfoDB.size() < 0)
        return;

    if (pItemNotify->pRow)
    {
        long nID = pItemNotify->pRow->GetRecord()->GetIndex();
        auto pPassiveInfoDB = &m_pVisionInsp->m_vecsPassiveInfoDB[nID];
        m_pCurPassiveInfoDB = pPassiveInfoDB;
    }

    CMenu MenuItem;
    if (!MenuItem.LoadMenu(IDR_MENU_SPEC))
        return;
    CMenu* pMenuSub = MenuItem.GetSubMenu(0);
    pMenuSub->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_NONOTIFY, ptClick.x, ptClick.y, this);
}

void CDlgVisionInspectionComponent2D::OnBnClickedButtonInspect()
{
    // Spec Update
    //m_pVisionInspSpecDlg->UpdateSpecData();

    m_pVisionInsp->SetVisionInspectionSpecs();

    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_imageLotView->Overlay_Show(TRUE);

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();

    // 검사 결과 화면 갱신
    m_procCommonResultDlg->Refresh();

    // 검사 결과 탭 보여주기
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);
}

void CDlgVisionInspectionComponent2D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    m_procCommonSpecDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonResultDlg->ShowWindow((nTab == 1) ? SW_SHOW : SW_HIDE);
    m_procCommonDetailResultDlg->ShowWindow((nTab == 2) ? SW_SHOW : SW_HIDE);
    m_procCommonDebugInfoDlg->ShowWindow(nTab == 3 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 4 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

void CDlgVisionInspectionComponent2D::ShowImage(BOOL bChange)
{
    if (m_pVisionInsp->GetImageFrameCount() == 0)
    {
        return;
    }

    if (!bChange)
    {
        m_nImageID = max(0, m_pVisionInsp->GetImageFrameIndex(0));
    }

    if ((long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() <= 0)
        return;

    if ((long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size()
        <= m_nImageID)
        m_nImageID
            = (long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() - 1;

    Ipvm::Image8u& image
        = m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()][m_nImageID];

    CString FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_nImageID);
    m_imageLotView->SetImage(image, FrameToString);

    FrameToString.Empty();
}

LRESULT CDlgVisionInspectionComponent2D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
{
    const long itemIndex = long(wparam);

    if (itemIndex < 0)
    {
        m_TabResult.SetCurSel(1);
    }
    else
    {
        m_TabResult.SetCurSel(2);

        m_procCommonDetailResultDlg->Refresh(itemIndex);
    }

    OnTcnSelchangeTabResult(nullptr, nullptr);

    return 0L;
}

void CDlgVisionInspectionComponent2D::HideItemDialog()
{
    if (m_pDlgPassiveAlgorithm)
        m_pDlgPassiveAlgorithm->ShowWindow(SW_HIDE);

    if (m_pDlgSpecDB)
        m_pDlgSpecDB->ShowWindow(SW_HIDE);
}

void CDlgVisionInspectionComponent2D::OnMenuDelete()
{
    std::vector<long> vecnID(0);
    std::vector<CString> vecstrName(0);
    long nSelCnt = m_ChipListControl.GetSelectedRows()->GetCount();
    POSITION pos = m_ChipListControl.GetSelectedRows()->GetFirstSelectedRowPosition();
    while (pos)
    {
        int nItem = m_ChipListControl.GetSelectedRows()->GetNextSelectedRow(pos)->GetRecord()->GetIndex();
        vecnID.push_back(nItem);

        CXTPGridRecord* repRecord = m_ChipListControl.GetRecords()->GetAt(nItem);
        CXTPGridRecordItem* pItem;
        pItem = DYNAMIC_DOWNCAST(CXTPGridRecordItem, repRecord->GetItem(1));
        vecstrName.push_back(pItem->GetCaption());
    }

    VERIFY(nSelCnt == (long)vecnID.size());

    //long nSel = m_lstChip.GetSelectionMark();
    long nSel = m_ChipListControl.GetSelectedRows()->GetAt(0)->GetRecord()->GetIndex();
    if (nSel < 0)
        return;

    //선택한 ID가 2개이상이면 내림차순으로 벡터를 정렬해주자
    long nSize = (long)vecnID.size();
    long nTmp = 0;

    if (nSize > 1)
    {
        for (long i = 0; i < nSize - 1; i++)
        {
            for (long j = i + 1; j < nSize; j++)
            {
                if (vecnID[i] < vecnID[j])
                {
                    nTmp = vecnID[i];
                    vecnID[i] = vecnID[j];
                    vecnID[j] = nTmp;
                }
            }
        }
    }

    for (long i = 0; i < (long)vecnID.size(); i++)
        m_packageSpec.DeleteCompMapData(vecnID[i]);

    m_packageSpec.CreateDMSInfo();

    CreatePackageInfo();

    for (int nIdx = 0; nIdx < vecstrName.size(); nIdx++)
    {
        vecstrName[nIdx].Empty();
    }
}

void CDlgVisionInspectionComponent2D::OnMenuDmsInfo()
{
    long nID = m_ChipListControl.GetSelectedRows()->GetAt(0)->GetRecord()->GetIndex();
    if (nID < 0 || nID >= m_pVisionInsp->m_vecsPassiveInfoDB.size())
        return;

    auto* pPassiveInfo = &m_pVisionInsp->m_vecsPassiveInfoDB[nID];

    if (pPassiveInfo == NULL)
        return;

    // Job 파일에 등록된 Dimension 정보 확인.
    CString strPassiveInfo;
    strPassiveInfo.AppendFormat(_T("Component Width=%2.3fmm\r\n"), pPassiveInfo->fPassiveSpecWidth_mm);
    strPassiveInfo.AppendFormat(_T("Component Length=%2.3fmm\r\n"), pPassiveInfo->fPassiveSpecLength_mm);
    strPassiveInfo.AppendFormat(_T("Component Angle=%2.3fdegree\r\n"), pPassiveInfo->fPassiveAngle);
    strPassiveInfo.AppendFormat(_T("Component Thickness=%2.3fmm\r\n"), pPassiveInfo->fPassiveHeight_mm);

    strPassiveInfo.AppendFormat(_T("Electrode Width=%2.3fmm\r\n"), pPassiveInfo->fPassiveElectrodeWidth_mm);
    strPassiveInfo.AppendFormat(_T("Electrode Thickness=%2.3fmm\r\n"), pPassiveInfo->fPassiveElectrodeHeight_mm);

    strPassiveInfo.AppendFormat(_T("Component Width With PAD=%2.3fmm\r\n"), pPassiveInfo->fPassivePAD_Width_mm);
    strPassiveInfo.AppendFormat(_T("Component Length With PAD=%2.3fmm\r\n"), pPassiveInfo->fPassivePAD_Length_mm);

    MessageBox(strPassiveInfo, pPassiveInfo->strCompType, MB_OK);

    strPassiveInfo.Empty();
}

void CDlgVisionInspectionComponent2D::OnMenuChangeSpec()
{
    if (m_pVisionInsp->m_vecsPassiveInfoDB.size() <= 0 || m_pCurPassiveInfoDB == nullptr)
        return;

    std::vector<long> vecnID(0);
    std::vector<CString> vecstrName(0);
    int nSelCnt = m_ChipListControl.GetSelectedRows()->GetCount();
    POSITION pos = m_ChipListControl.GetSelectedRows()->GetFirstSelectedRowPosition();

    while (pos)
    {
        int nItem = m_ChipListControl.GetSelectedRows()->GetNextSelectedRow(pos)->GetRecord()->GetIndex();
        vecnID.push_back(nItem);
        CXTPGridRecord* repRecord = m_ChipListControl.GetRecords()->GetAt(nItem);
        CXTPGridRecordItem* pItem;
        pItem = DYNAMIC_DOWNCAST(CXTPGridRecordItem, repRecord->GetItem(1));
        vecstrName.push_back(pItem->GetCaption());

        if (m_pCurPassiveInfoDB->eComponentType != m_pVisionInsp->m_vecsPassiveInfoDB[nItem].eComponentType)
        {
            MessageBox(_T("The type of selected object is different."));
            return;
        }
    }

    VERIFY(nSelCnt == (long)vecnID.size());

    CString strOriName = m_pCurPassiveInfoDB->strSpecName;
    CDlgChangeSpecName Dlg;
    if (Dlg.DoModal(m_pVisionPara, m_pCurPassiveInfoDB) != IDOK)
        return;
    if (Dlg.m_strReName.GetLength() == 0)
        return;

    bool updateChipInfo = false;
    auto& specLink = m_pVisionInsp->m_VisionPara->m_specLink;

    for (long i = 0; i < (long)vecnID.size(); i++)
    {
        long nID = vecnID[i];

        sPassive_InfoDB* pPassiveInfo = &m_pVisionInsp->m_vecsPassiveInfoDB[nID];
        if (pPassiveInfo == nullptr)
            continue;

        CString strName = vecstrName[i];
        specLink.GetSpecName(m_pCurPassiveInfoDB->eComponentType, strName) = Dlg.m_strReName;
        pPassiveInfo->strSpecName = Dlg.m_strReName;

        BOOL bNewAlgo = TRUE;
        long nAlgoNum = (long)m_pVisionPara->m_vecPassiveAlgoSpec.size();
        for (long nAlgo = 0; nAlgo < nAlgoNum; nAlgo++)
        {
            if (Dlg.m_strReName == m_pVisionPara->m_vecPassiveAlgoSpec[nAlgo].strSpecName)
            {
                bNewAlgo = FALSE;
            }
        }
        if (bNewAlgo) // 같은 이름의 Spec이 현재 job에 없다면
        {
            auto& NewPassiveAlgoSpec = m_pVisionPara->m_vecPassiveAlgoSpec.add();

            SaveAsAlgorithmPara(m_pCurPassiveInfoDB->eComponentType, strOriName);
            NewPassiveAlgoSpec.strSpecName = Dlg.m_strReName;
        }

        updateChipInfo = true;
        strName.Empty();
    }

    if (updateChipInfo)
    {
        UpdateChipList();
    }

    for (int nIdx = 0; nIdx < vecstrName.size(); nIdx++)
    {
        vecstrName[nIdx].Empty();
    }
    strOriName.Empty();
}

void CDlgVisionInspectionComponent2D::SaveAsAlgorithmPara(eComponentTypeDefine i_eComponentType, CString strOriName)
{
    long nPassiveAlgoNum = (long)m_pVisionPara->m_vecPassiveAlgoSpec.size();

    if (i_eComponentType == eComponentTypeDefine::tyPASSIVE)
    {
        for (long nAlgo = 0; nAlgo < nPassiveAlgoNum; nAlgo++)
        {
            if (strOriName == m_pVisionPara->m_vecPassiveAlgoSpec[nAlgo].strSpecName)
            {
                // 이전의 스펙값을 넣어준다.
                m_pVisionPara->m_vecPassiveAlgoSpec[nPassiveAlgoNum - 1].CopyFrom(
                    m_pVisionPara->m_vecPassiveAlgoSpec[nAlgo]);
                break;
            }
        }
    }
}

BOOL CDlgVisionInspectionComponent2D::CreatePackageInfo()
{
    float fAngle(0.f);
    Ipvm::Rect32r frtBody(0.f, 0.f, 0.f, 0.f);
    Ipvm::Point32r2 fptBodyCenter(0.f, 0.f);
    if (!m_pVisionInsp->GetBodyAlignInfo(fAngle, frtBody, fptBodyCenter))
    {
        return FALSE;
    }

    m_pVisionInsp->MakePackageInfo(true, fAngle, fptBodyCenter);
    m_pVisionInsp->SetPackageInfo_SpecLink();

    UpdateChipList();

    return TRUE;
}

void CDlgVisionInspectionComponent2D::OnBnClickedBtnChipSpecdb()
{
    HideItemDialog();

    if (m_pDlgSpecDB)
        m_pDlgSpecDB->ShowWindow(SW_SHOW);
}

LRESULT CDlgVisionInspectionComponent2D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    if (m_pVisionInsp->m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D))
        m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D), false,
            m_pVisionInsp->GetCurVisionModule_Status());
    else if (m_pVisionInsp->m_visionUnit.IsTheUsingVisionProcessingByGuid(
                 m_pVisionInsp->m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    return 0L;
}

void CDlgVisionInspectionComponent2D::ResetSpecAndResultDlg()
{
    m_procCommonSpecDlg->ResetInitGrid();
    m_procCommonResultDlg->Refresh();
}
