//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgLayer.h"

//CPP_2_________________________________ This project's headers
#include "DlgImportLayer.h"
#include "DlgLayerName.h"
#include "DlgSetupUI.h"
#include "VisionProcessingGlobalMaskMakerPara.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../UserInterfaceModules/CommonControlExtension/AutoHidePopup.h"

//CPP_4_________________________________ External library headers
#include <afxdialogex.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_LAYER_ITEM_RCLICK (WM_USER + 100)
#define UM_LAYER_ITEM_SELECTED (WM_USER + 102)
#define UM_LAYER_ITEM_MOVED (WM_USER + 104)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgLayer, CDialogEx)

DlgLayer::DlgLayer(CDlgSetupUI* parent)
    : CDialogEx(IDD_DIALOG_LAYER, parent)
    , m_listLayer(MaskListStyle::LastItemFixed)
    , m_parent(parent)
    , m_popupSelectLayerIndex(-1)
{
}

DlgLayer::~DlgLayer()
{
}

void DlgLayer::UpdateLayerList()
{
    m_listLayer.SetRedraw(FALSE);
    long oldLayerScroll = m_listLayer.GetScrollPos(SB_VERT);
    m_listLayer.DeleteAllItems();

    auto& para = m_parent->GetPara();

    for (long layerIndex = 0; layerIndex < para.getLayerCount(); layerIndex++)
    {
        const auto& layerInfo = para.getLayer(layerIndex);
        CString textNo;
        if (layerIndex == para.getLayerCount() - 1)
        {
            // 마지막 SURFACE COMMON IGNORE MASK는 특별하게 표시해 주자
            textNo = _T("**");
        }
        else
        {
            textNo.Format(_T("%02d"), layerIndex + 1);
        }

        m_listLayer.AddItem(layerIndex, layerInfo.m_name, textNo);

        if (!m_parent->isLayerValid(layerIndex))
        {
            m_listLayer.SetItemInvalid(layerIndex, true);
        }

        textNo.Empty();
    }

    long selectLayer = m_parent->GetCurrentLayer();

    if (selectLayer < 0 || selectLayer >= para.getLayerCount())
    {
        selectLayer = -1;
    }

    m_listLayer.SelectItem(selectLayer);
    m_listLayer.SetScrollPos(SB_VERT, oldLayerScroll, FALSE);
    m_listLayer.SetRedraw(TRUE);
    m_listLayer.Invalidate(FALSE);

    if (selectLayer != m_parent->GetCurrentLayer())
    {
        m_parent->SetCurrentLayer(selectLayer);
    }
}

void DlgLayer::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LAYERLIST, m_listLayer);
}

BEGIN_MESSAGE_MAP(DlgLayer, CDialogEx)
ON_COMMAND(ID_LM_ADDLAYER, &DlgLayer::OnLmAddlayer)
ON_COMMAND(ID_LM_DELETELAYER, &DlgLayer::OnLmDeletelayer)
ON_COMMAND(ID_LM_RENAMELAYER, &DlgLayer::OnLmRenamelayer)
ON_COMMAND(ID_LM_EMPTYLAYER, &DlgLayer::OnLmEmptylayer)
ON_MESSAGE(UM_LAYER_ITEM_RCLICK, &DlgLayer::OnLayerItemRClick)
ON_MESSAGE(UM_LAYER_ITEM_SELECTED, &DlgLayer::OnLayerItemSelected)
ON_MESSAGE(UM_LAYER_ITEM_MOVED, &DlgLayer::OnLayerItemMoved)
ON_WM_SIZE()
ON_COMMAND(ID_LM_EXPORTALLLAYER, &DlgLayer::OnLmExportalllayer)
ON_COMMAND(ID_LM_IMPORTLAYER, &DlgLayer::OnLmImportalllayer)
END_MESSAGE_MAP()

// DlgLayer message handlers

BOOL DlgLayer::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_listLayer.SetEvent_ItemRClick(GetSafeHwnd(), UM_LAYER_ITEM_RCLICK);
    m_listLayer.SetEvent_ItemSelected(GetSafeHwnd(), UM_LAYER_ITEM_SELECTED);
    m_listLayer.SetEvent_ItemMoved(GetSafeHwnd(), UM_LAYER_ITEM_MOVED);

    UpdateLayerList();

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgLayer::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_listLayer.GetSafeHwnd())
    {
        m_listLayer.MoveWindow(0, 0, cx, cy);
    }
}

LRESULT DlgLayer::OnLayerItemRClick(WPARAM wParam, LPARAM)
{
    CPoint ptClick;
    GetCursorPos(&ptClick);

    CMenu MenuItem;
    if (!MenuItem.LoadMenu(IDR_MENU_MANAGE_LAYER))
    {
        return 0;
    }

    m_popupSelectLayerIndex = (int)wParam;
    CMenu* pMenuSub = nullptr;

    auto& para = m_parent->GetPara();
    if (m_popupSelectLayerIndex >= 0 && m_popupSelectLayerIndex < para.getLayerCount())
    {
        // 선택된 Layer가 있다
        if (para.getLayer(m_popupSelectLayerIndex).m_name == GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME)
        {
            // Ignore Mask을 선택했다. 이것은 지우거나 이름을 바꿀 수 없다. 필수 Layer
            pMenuSub = MenuItem.GetSubMenu(2);
        }
        else
        {
            pMenuSub = MenuItem.GetSubMenu(1);
        }
    }
    else
    {
        // 선택된 Layer가 없다
        pMenuSub = MenuItem.GetSubMenu(0);
    }

    if (pMenuSub)
    {
        pMenuSub->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_NONOTIFY, ptClick.x, ptClick.y, this);
    }

    return 0;
}

void DlgLayer::OnLmAddlayer()
{
    DlgLayerName dlg(this);

    auto& para = m_parent->GetPara();

    //long insertIndex = m_popupSelectLayerIndex;
    long insertIndex = para.getLayerCount() - 1;
    do
    {
        if (dlg.DoModal() != IDOK)
            return;

        if (para.addLayer(insertIndex, dlg.m_name))
        {
            break;
        }

        MessageBox(_T("This name already exists."));
    }
    while (1);

    m_parent->SetCurrentLayer(insertIndex);
}

void DlgLayer::OnLmDeletelayer()
{
    auto& para = m_parent->GetPara();

    if (m_popupSelectLayerIndex >= 0 && m_popupSelectLayerIndex < para.getLayerCount())
    {
        CString message;
        message.Format(
            _T("Do you want to delete this layer [%s]?"), (LPCTSTR)para.getLayer(m_popupSelectLayerIndex).m_name);
        if (MessageBox(message, _T("Message"), MB_YESNO) == IDYES)
        {
            para.deleteLayer(m_popupSelectLayerIndex);
            m_parent->PostMessage(UM_REFRESH_UI);
        }

        message.Empty();
    }
}

void DlgLayer::OnLmRenamelayer()
{
    auto& para = m_parent->GetPara();

    if (m_popupSelectLayerIndex < 0 || m_popupSelectLayerIndex >= para.getLayerCount())
    {
        return;
    }

    DlgLayerName dlg;
    dlg.m_name = para.getLayer(m_popupSelectLayerIndex).m_name;

    do
    {
        if (dlg.DoModal() != IDOK)
            return;

        if (para.setLayerName(m_popupSelectLayerIndex, dlg.m_name))
        {
            break;
        }

        MessageBox(_T("This name already exists."));
    }
    while (1);

    m_parent->PostMessage(UM_REFRESH_UI);
}

void DlgLayer::OnLmEmptylayer()
{
    auto& para = m_parent->GetPara();

    if (m_popupSelectLayerIndex >= 0 && m_popupSelectLayerIndex <= para.getLayerCount())
    {
        CString message;
        message.Format(
            _T("Do you want to empty this layer [%s]?"), (LPCTSTR)para.getLayer(m_popupSelectLayerIndex).m_name);
        if (MessageBox(message, _T("Message"), MB_YESNO) == IDYES)
        {
            para.getLayer(m_popupSelectLayerIndex).Init();
            m_parent->PostMessage(UM_REFRESH_UI);
        }

        message.Empty();
    }
}

LRESULT DlgLayer::OnLayerItemSelected(WPARAM wParam, LPARAM)
{
    m_parent->SetCurrentLayer(long(wParam));

    return 0;
}

LRESULT DlgLayer::OnLayerItemMoved(WPARAM wParam, LPARAM lParam)
{
    long itemIndex = (long)wParam;
    long itemDestIndex = (long)lParam;

    auto& para = m_parent->GetPara();

    while (itemIndex != itemDestIndex)
    {
        long nextIndex = itemIndex > itemDestIndex ? itemIndex - 1 : itemIndex + 1;
        para.swapLayer(itemIndex, nextIndex);

        itemIndex = nextIndex;
    }

    m_parent->SetCurrentLayer(itemDestIndex);

    return 0;
}

void DlgLayer::OnLmExportalllayer()
{
    CString strFileType = _T("Global Mask Layer file format(*.gML)|*.gML|all files(*.*)|*.*|");
    CString strFileExtension = _T("*.gML");

    CFileDialog dlg(
        FALSE, strFileExtension, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, strFileType);
    if (dlg.DoModal() != IDOK)
        return;

    CString filePath = dlg.GetPathName();

    auto& para = m_parent->GetPara();

    CiDataBase db;
    para.LinkDataBase(TRUE, db);
    if (db.Save(filePath))
    {
        AutoHidePopup::add(AutoHidePopupType::Normal, _T("Save Complete!"));
    }
    else
    {
        AutoHidePopup::add(AutoHidePopupType::Error, _T("File Save Error!"));
    }

    strFileType.Empty();
    strFileExtension.Empty();
    filePath.Empty();
}

void DlgLayer::OnLmImportalllayer()
{
    CString strFileType = _T("Global Mask Layer file format(*.gML)|*.gML|all files(*.*)|*.*|");
    CString strFileExtension = _T("*.gML");

    CFileDialog dlg(
        TRUE, strFileExtension, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, strFileType);

    if (dlg.DoModal() != IDOK)
        return;

    CVisionProcessingGlobalMaskMakerPara importPara;

    CiDataBase db;
    if (!db.Load(dlg.GetPathName()))
    {
        AutoHidePopup::add(AutoHidePopupType::Error, _T("File Load Error!"));
        return;
    }

    importPara.LinkDataBase(FALSE, db);

    auto& para = m_parent->GetPara();
    DlgImportLayer importLayer(para, importPara);
    if (importLayer.DoModal() != IDOK)
        return;

    AutoHidePopup::add(AutoHidePopupType::Normal, _T("Load Complete!"));
    m_parent->PostMessage(UM_REFRESH_UI);

    strFileType.Empty();
    strFileExtension.Empty();
}
