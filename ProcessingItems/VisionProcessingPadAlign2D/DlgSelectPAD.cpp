//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSelectPAD.h"

//CPP_2_________________________________ This project's headers
#include "DlgSetFiducial.h"
#include "DlgVisionProcessingPadAlign2D.h"
#include "ParaDB.h"
#include "Result.h"
#include "VisionProcessingPadAlign2D.h"
#include "VisionProcessingPadAlign2DPara.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgSelectPAD, CDialog)

DlgSelectPAD::DlgSelectPAD(VisionProcessingPadAlign2D* visionInsp, ImageLotView* imageLotView, ParaDB* paraDB,
    DlgVisionProcessingPadAlign2D* parent)
    : CDialog(IDD_DIALOG_SELECT_PAD, parent)
    , m_visionInsp(visionInsp)
    , m_imageLotView(imageLotView)
    , m_paraDB(paraDB)
    , m_parent(parent)
    , m_padListUpdateOK(false)
{
}

DlgSelectPAD::~DlgSelectPAD()
{
}

void DlgSelectPAD::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PAD, m_lstPAD);
    DDX_Control(pDX, IDC_CHECK_FixTheBoundary, m_chkFixTheBoundary);
}

BEGIN_MESSAGE_MAP(DlgSelectPAD, CDialog)
ON_WM_CONTEXTMENU()
ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PAD, &DlgSelectPAD::OnLvnItemchangedListPad)
ON_COMMAND(ID_MENU_SETALGO, &DlgSelectPAD::OnMenuSetAlgo)
ON_COMMAND(ID_MENU_DELETEALGO, &DlgSelectPAD::OnMenuDeleteAlgo)
ON_BN_CLICKED(IDC_CHECK_FixTheBoundary, &DlgSelectPAD::OnBnClickedCheckFixtheboundary)
END_MESSAGE_MAP()

// DlgSelectPAD message handlers

BOOL DlgSelectPAD::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_lstPAD.SetExtendedStyle(m_lstPAD.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
    m_lstPAD.InsertColumn(0, _T("No"), LVCFMT_LEFT, 50);
    m_lstPAD.InsertColumn(1, _T("Name"), LVCFMT_LEFT, 100);
    m_lstPAD.InsertColumn(2, _T("Shape"), LVCFMT_LEFT, 100);
    m_lstPAD.InsertColumn(3, _T("Algorithm"), LVCFMT_LEFT, 100);
    m_lstPAD.InsertColumn(4, _T("Category"), LVCFMT_LEFT, 100);

    m_selectBoundary = Ipvm::Rect32s(0, 0, m_visionInsp->getReusableMemory().GetInspImageSizeX(),
        m_visionInsp->getReusableMemory().GetInspImageSizeY());

    m_selectBoundary.DeflateRect(100, 100);

    UpdatePadList();

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgSelectPAD::UpdatePadList()
{
    std::vector<long> selectList;
    GetSelectPadIndexList(selectList);
    m_padListUpdateOK = false;

    m_lstPAD.DeleteAllItems();
    auto& padManager = m_visionInsp->m_packageSpec.m_PadMapManager;

    long padCount = 0;
    for (long padIndex = 0; padIndex < long(padManager->vecPadData.size()); padIndex++)
    {
        auto& padInfo = padManager->vecPadData[padIndex];
        if (padInfo.bIgnore)
            continue;

        CString text_no;
        text_no.Format(_T("%d"), padCount + 1);

        CString algorithm_name = m_visionInsp->m_VisionPara->GetAlignPAD_ParaName(padInfo.strPadName);
        CString category_name
            = GetPAD_CategoryName(m_visionInsp->m_VisionPara->GetAlignPAD_Category(padInfo.strPadName));

        m_lstPAD.InsertItem(padCount, text_no);
        m_lstPAD.SetItemText(padCount, 1, padInfo.strPadName);
        m_lstPAD.SetItemText(padCount, 2, Package::GetPAD_Type_Name(padInfo.GetType()));
        m_lstPAD.SetItemText(padCount, 3, algorithm_name);
        m_lstPAD.SetItemText(padCount, 4, category_name);
        m_lstPAD.SetItemData(padCount, padIndex);
        padCount++;

        text_no.Empty();
        algorithm_name.Empty();
        category_name.Empty();
    }

    if (selectList.size() && selectList[0] >= 0 && selectList[0] < long(padManager->vecPadData.size()))
    {
        m_lstPAD.SetItemState(selectList[0], LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }

    m_padListUpdateOK = true;
    m_parent->UpdatePropertyGrid();
}

void DlgSelectPAD::UpdateFixTheBoundary()
{
    auto* imageView = m_parent->m_imageLotView;
    imageView->ROI_RemoveAll();

    if (m_chkFixTheBoundary.GetCheck())
    {
        imageView->ROI_Add(_T("Boundary"), _T("Boundary"), m_selectBoundary, RGB(0, 255, 0), TRUE, TRUE);
    }

    imageView->ROI_Show(TRUE);
}

bool DlgSelectPAD::GetSelectPadIndexList(std::vector<long>& o_list)
{
    o_list.clear();
    for (long index = 0; index < m_lstPAD.GetItemCount(); index++)
    {
        if (m_lstPAD.GetItemState(index, LVIS_SELECTED) & LVIS_SELECTED)
        {
            o_list.push_back(long(m_lstPAD.GetItemData(index)));
        }
    }

    return o_list.size() != 0;
}

void DlgSelectPAD::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    auto& padManager = m_visionInsp->m_packageSpec.m_PadMapManager;

    std::vector<long> selectList;
    GetSelectPadIndexList(selectList);
    if (selectList.size() == 0)
        return;

    m_select_padIndexList_for_menu.clear();

    CMenu menus;
    if (!menus.LoadMenu(IDR_MENU_SELECT_FIDUCIAL))
        return;
    bool all_no_exist = true;

    for (auto& select : selectList)
    {
        CString algorithm_name
            = m_visionInsp->m_VisionPara->GetAlignPAD_ParaName(padManager->vecPadData[select].strPadName);
        if (!algorithm_name.IsEmpty())
        {
            all_no_exist = false;
        }

        if (m_chkFixTheBoundary.GetCheck())
        {
            m_imageLotView->ROI_Get(_T("Boundary"), m_selectBoundary);

            auto& padInfo = m_visionInsp->m_result->m_pads;
            if (select >= 0 && select < long(padInfo.size()))
            {
                auto roi = padInfo[select].m_spec_px.GetCRect();
                if ((roi & m_selectBoundary) == roi)
                {
                    // Boundary 영역내로 들어오는 PAD만 선택하자
                    m_select_padIndexList_for_menu.push_back(select);
                }
            }
        }
        else
        {
            m_select_padIndexList_for_menu.push_back(select);
        }

        algorithm_name.Empty();
    }

    CMenu* subMenu = menus.GetSubMenu(all_no_exist ? 0 : 1);
    subMenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_NONOTIFY, point.x, point.y, this);
}

void DlgSelectPAD::OnLvnItemchangedListPad(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    //LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    if (m_padListUpdateOK)
    {
        std::vector<long> selectList;
        GetSelectPadIndexList(selectList);

        auto* imageLotView = m_imageLotView;
        imageLotView->Overlay_RemoveAll();

        for (auto& selectIndex : selectList)
        {
            auto& padInfo = m_visionInsp->m_result->m_pads;
            if (selectIndex >= 0 && selectIndex < long(padInfo.size()))
            {
                imageLotView->Overlay_AddRectangle(padInfo[selectIndex].m_spec_px, RGB(0, 255, 0));
            }
        }

        imageLotView->Overlay_Show(TRUE);
    }

    *pResult = 0;
}

void DlgSelectPAD::OnMenuSetAlgo()
{
    auto& firstPad = m_visionInsp->m_packageSpec.m_PadMapManager->vecPadData[m_select_padIndexList_for_menu[0]];
    for (auto padIndex : m_select_padIndexList_for_menu)
    {
        if (firstPad.GetType() != m_visionInsp->m_packageSpec.m_PadMapManager->vecPadData[padIndex].GetType())
        {
            // 타입이 다른 것끼리 알고리즘을 설정 못한다
            return;
        }
    }

    CString base_pad_name
        = m_visionInsp->m_packageSpec.m_PadMapManager->vecPadData[m_select_padIndexList_for_menu[0]].strPadName;

    DlgSetFiducial dlg(m_paraDB, m_visionInsp->m_VisionPara);
    dlg.m_algorithm_name = m_visionInsp->m_VisionPara->GetAlignPAD_ParaName(base_pad_name);
    dlg.m_category = int(m_visionInsp->m_VisionPara->GetAlignPAD_Category(base_pad_name));

    if (firstPad.GetType() == _typePinIndex)
    {
        dlg.m_category = long(enumPAD_Category::PinIndex);
    }

    if (dlg.DoModal() != IDOK)
        return;

    CString algo_name = dlg.m_algorithm_name;
    algo_name = algo_name.MakeUpper();
    algo_name = algo_name.Trim();

    for (auto index : m_select_padIndexList_for_menu)
    {
        CString pad_name = m_visionInsp->m_packageSpec.m_PadMapManager->vecPadData[index].strPadName;
        m_visionInsp->m_VisionPara->SetAlignPAD(*m_paraDB, pad_name, algo_name, enumPAD_Category(dlg.m_category));

        pad_name.Empty();
    }

    UpdatePadList();

    base_pad_name.Empty();
    algo_name.Empty();
}

void DlgSelectPAD::OnMenuDeleteAlgo()
{
    for (auto index : m_select_padIndexList_for_menu)
    {
        CString pad_name = m_visionInsp->m_packageSpec.m_PadMapManager->vecPadData[index].strPadName;
        m_visionInsp->m_VisionPara->DelAlignPAD(pad_name);
    }

    UpdatePadList();
}

void DlgSelectPAD::OnBnClickedCheckFixtheboundary()
{
    UpdateFixTheBoundary();
}
