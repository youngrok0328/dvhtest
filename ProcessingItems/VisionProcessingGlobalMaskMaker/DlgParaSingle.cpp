//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgParaSingle.h"

//CPP_2_________________________________ This project's headers
#include "DlgPara.h"
#include "DlgSetupUI.h"
#include "VisionProcessingGlobalMaskMaker.h"
#include "VisionProcessingGlobalMaskMakerPara.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"

//CPP_4_________________________________ External library headers
#include <afxdialogex.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_MASK_SELECTED (WM_USER + 100)
#define UM_MASK_MOVED (WM_USER + 102)
#define UM_MASK_STATE_DBCLICKED (WM_USER + 104)
#define UM_MASK_RCLICKED (WM_USER + 106)

//CPP_7_________________________________ Implementation body
//
enum enumParamID
{
    ROI_TYPE,
    ROI_OPERATION,
    ROI_PREPARED_MASKNAME,
    ROI_PREPARED_MASKDILATE,
    ROI_PREPARED_ROIEXPAND_X,
    ROI_PREPARED_ROIEXPAND_Y,
    DISPLAY_OTHER_LAYER_MASK = 10000,
};

enum Menu
{
    Menu_ChangeOrder_Delete,
};

IMPLEMENT_DYNAMIC(DlgParaSingle, CDialogEx)

DlgParaSingle::DlgParaSingle(VisionProcessingGlobalMaskMaker* pVisionInsp, DlgPara& parent, CWnd* parentWnd)
    : CDialogEx(IDD_DIALOG_PARA_SINGLE, parentWnd)
    , m_property(nullptr)
    , m_parent(parent)
    , m_pVisionInsp(pVisionInsp)
    , m_pVisionPara(pVisionInsp->m_VisionPara)
    , m_selectMenuRoiIndex(-1)
    , m_maskList(MaskListStyle::None)
{
}

DlgParaSingle::~DlgParaSingle()
{
    delete m_property;
}

void DlgParaSingle::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MASKLIST, m_maskList);
    DDX_Control(pDX, IDC_LIST_PREPARED_IMAGE, m_lstPreparedImage);
    DDX_Control(pDX, IDC_LIST_USER_ROI, m_lstUserRoi);
}

BEGIN_MESSAGE_MAP(DlgParaSingle, CDialogEx)
ON_WM_SIZE()
ON_WM_DESTROY()
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_LBN_DBLCLK(IDC_LIST_USER_ROI, &DlgParaSingle::OnLbnDblclkListUserRoi)
ON_LBN_DBLCLK(IDC_LIST_PREPARED_IMAGE, &DlgParaSingle::OnLbnDblclkListPreparedImage)
ON_MESSAGE(UM_MASK_SELECTED, &DlgParaSingle::OnMaskSelected)
ON_MESSAGE(UM_MASK_MOVED, &DlgParaSingle::OnMaskMoved)
ON_MESSAGE(UM_MASK_STATE_DBCLICKED, &DlgParaSingle::OnMaskDBClicked)
ON_MESSAGE(UM_MASK_RCLICKED, &DlgParaSingle::OnMaskRClicked)
END_MESSAGE_MAP()

// DlgParaSingle message handlers

BOOL DlgParaSingle::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CRect rtWindow;
    GetClientRect(rtWindow);

    m_property = new CXTPPropertyGrid;
    m_property->Create(rtWindow, this, IDC_FRAME_PROPERTY_GRID);

    m_userROITypes.push_back(_T("Rectangle"));
    m_userROITypes.push_back(_T("Ellipse"));
    m_userROITypes.push_back(_T("Triangle"));
    m_userROITypes.push_back(_T("4 Points Polygon"));
    m_userROITypes.push_back(_T("5 Points Polygon"));
    m_userROITypes.push_back(_T("6 Points Polygon"));
    m_userROITypes.push_back(_T("7 Points Polygon"));
    m_userROITypes.push_back(_T("8 Points Polygon"));

    for (auto& type : m_userROITypes)
    {
        m_lstUserRoi.AddString(type);
    }

    m_maskList.SetEvent_ItemSelected(GetSafeHwnd(), UM_MASK_SELECTED);
    m_maskList.SetEvent_ItemMoved(GetSafeHwnd(), UM_MASK_MOVED);
    m_maskList.SetEvent_StateDBClick(GetSafeHwnd(), UM_MASK_STATE_DBCLICKED);
    m_maskList.SetEvent_ItemRClick(GetSafeHwnd(), UM_MASK_RCLICKED);

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgParaSingle::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (!m_property->GetSafeHwnd())
    {
        return;
    }

    long nMidX = cx / 3;
    long nMidY = cy / 3 * 2;

    m_lstPreparedImage.MoveWindow(0, 0, nMidX, nMidY / 2);
    m_lstUserRoi.MoveWindow(0, nMidY / 2, nMidX, nMidY / 2);

    m_maskList.MoveWindow(nMidX, 0, cx - nMidX, nMidY);
    m_property->MoveWindow(0, nMidY, cx, cy - nMidY);
}

void DlgParaSingle::OnDestroy()
{
    CDialogEx::OnDestroy();

    m_property->DestroyWindow();
}

void DlgParaSingle::UpdateLayerMaskList()
{
    long currentLayer = m_parent.GetSelectLayerIndex();
    long currentMask = m_maskList.GetSelectedItemKey();

    m_maskList.DeleteAllItems();

    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
    {
        UpdateProperty();
        return;
    }

    auto& specSingleLayer = m_pVisionPara->getLayer(currentLayer).m_single;

    long oldMaskScroll = m_maskList.GetScrollPos(SB_VERT);

    m_maskList.SetRedraw(FALSE);

    for (long roiIndex = 0; roiIndex < specSingleLayer.count(); roiIndex++)
    {
        CString state = _T("");
        switch (specSingleLayer[roiIndex].m_operation)
        {
            case OperationType::And:
                state = _T("&");
                break;
            case OperationType::Plus:
                state = _T("+");
                break;
            case OperationType::Minus:
                state = _T("-");
                break;
        }

        m_maskList.AddItem(roiIndex, GetCategortyName(roiIndex), state);

        state.Empty();
    }

    m_maskList.SelectItem(currentMask);
    m_maskList.SetScrollPos(SB_VERT, oldMaskScroll, FALSE);
    m_maskList.SetRedraw(TRUE);
    m_maskList.Invalidate(FALSE);

    UpdateProperty();
}

void DlgParaSingle::UpdateProperty()
{
    //---------------------------------------------------------------------------------
    // 미리 만들어진 Surface Layer Mask를 수집한다
    //---------------------------------------------------------------------------------

    auto& reusableMemory = m_pVisionInsp->getReusableMemory();

    m_preparedMasks.clear();

    for (long n = 0; n < reusableMemory.GetSurfaceLayerRoiCount(); n++)
    {
        m_preparedMasks.emplace_back(reusableMemory.GetSurfaceLayerRoiName(n), 0);
    }

    CString header = GetSurfaceMaskNameHeader();

    for (long n = 0; n < reusableMemory.GetSurfaceLayerMaskCount(); n++)
    {
        CString name = reusableMemory.GetSurfaceLayerMaskName(n);
        if (name.Left(header.GetLength()) == header)
        {
            // Global Mask Maker가 만든 Layer는 감춰주자
            continue;
        }

        m_preparedMasks.emplace_back(reusableMemory.GetSurfaceLayerMaskName(n), 1);

        name.Empty();
    }

    long currentLayer = m_parent.GetSelectLayerIndex();

    for (long layerIndex = 0; layerIndex < currentLayer; layerIndex++)
    {
        CString layerName = m_pVisionPara->getLayer(layerIndex).m_name;
        m_preparedMasks.emplace_back(layerName, 1);

        layerName.Empty();
    }

    m_lstPreparedImage.ResetContent();

    for (auto& type : m_preparedMasks)
    {
        m_lstPreparedImage.AddString(type.first);
    }

    //---------------------------------------------------------------------------------

    m_property->ResetContent();

    UpdateProperty_ROIInfo();

    if (auto* category = m_property->AddCategory(_T("Other")))
    {
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemBool(_T("Display Other Layer Mask"), m_parent.m_displayOtherLayerMask)))
        {
            item->SetID(DISPLAY_OTHER_LAYER_MASK);
        }

        category->Expand();
    }

    m_property->SetViewDivider(0.5);
    m_property->HighlightChangedItems(TRUE);

    header.Empty();
}

void DlgParaSingle::UpdateProperty_ROIInfo()
{
    long currentLayer = m_parent.GetSelectLayerIndex();
    long maskIndex = m_maskList.GetSelectedItemKey();

    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return;

    auto& specSingleLayer = m_pVisionPara->getLayer(currentLayer).m_single;

    if (maskIndex < 0 || maskIndex >= specSingleLayer.count())
        return;

    auto* category = m_property->AddCategory(_T("Property") + GetCategortyName(maskIndex));

    const auto& scale = m_pVisionInsp->getScale();
    float um2px = scale.umToPixelXY();
    auto& spec = specSingleLayer[maskIndex];

    long roi_type = (long)spec.getType();

    if (spec.getType() == UserRoiType::Polygon_32f)
    {
        roi_type += spec.getPolygonPointCount() - 3;
    }

    if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("ROI Type"), roi_type)))
    {
        item->GetConstraints()->AddConstraint(_T("Prepeared Mask"), -1); //kircheis_고민/확인필요
        for (long opt = 0; opt < long(m_userROITypes.size()); opt++)
        {
            item->GetConstraints()->AddConstraint(m_userROITypes[opt], opt);
        }

        item->SetID(ROI_TYPE);
    }

    if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Operation"), (long)spec.m_operation)))
    {
        item->GetConstraints()->AddConstraint(_T("+"), long(OperationType::Plus));
        item->GetConstraints()->AddConstraint(_T("-"), long(OperationType::Minus));
        item->GetConstraints()->AddConstraint(_T("&"), long(OperationType::And));
        item->SetID(ROI_OPERATION);
    }

    if (auto* item
        = category->AddChildItem(new CXTPPropertyGridItem(_T("Prepared Object Name"), spec.m_preparedObjectName)))
    {
        for (auto& text : m_preparedMasks)
        {
            item->GetConstraints()->AddConstraint(text.first);
        }

        item->AddComboButton();
        item->SetID(ROI_PREPARED_MASKNAME);
    }

    // 편집하기 편하게 Pixel 단위로 표시한다
    if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(
            _T("Prepared Mask Dilate"), spec.m_preparedMaskDilateInUm * um2px, _T("%.1f"))))
    {
        item->SetID(ROI_PREPARED_MASKDILATE);
    }

    if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Prepared ROI Expand X (um)"),
            spec.m_preparedROIExpandX_um, _T("%.1f"), spec.m_preparedROIExpandX_um, -10000.f, 10000.f, 0.1f)))
    {
        item->SetID(ROI_PREPARED_ROIEXPAND_X);
    }

    if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Prepared ROI Expand Y (um)"),
            spec.m_preparedROIExpandY_um, _T("%.1f"), spec.m_preparedROIExpandY_um, -10000.f, 10000.f, 0.1f)))
    {
        item->SetID(ROI_PREPARED_ROIEXPAND_Y);
    }

    category->Expand();

    UpdateProperty_AuthorityAndRefreshMaskInfo();
}

void DlgParaSingle::UpdateRefreshMaskInfo(long maskIndex)
{
    long currentLayer = m_parent.GetSelectLayerIndex();

    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return;

    auto& specSingleLayer = m_pVisionPara->getLayer(currentLayer).m_single;

    if (maskIndex < 0 || maskIndex >= specSingleLayer.count())
        return;

    CString categoryName = GetCategortyName(maskIndex);

    CString state = _T("");
    switch (specSingleLayer[maskIndex].m_operation)
    {
        case OperationType::And:
            state = _T("&");
            break;
        case OperationType::Plus:
            state = _T("+");
            break;
        case OperationType::Minus:
            state = _T("-");
            break;
    }

    m_maskList.SetItemName(maskIndex, categoryName);
    m_maskList.SetItemState(maskIndex, state);

    categoryName.Empty();
    state.Empty();
}

void DlgParaSingle::UpdateProperty_AuthorityAndRefreshMaskInfo()
{
    long currentLayer = m_parent.GetSelectLayerIndex();
    long maskIndex = m_maskList.GetSelectedItemKey();

    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return;

    auto& specSingleLayer = m_pVisionPara->getLayer(currentLayer).m_single;

    if (maskIndex < 0 || maskIndex >= specSingleLayer.count())
        return;

    UpdateRefreshMaskInfo(maskIndex);

    auto& specROI = specSingleLayer[maskIndex];

    bool isPreparedType = specROI.getType() == UserRoiType::PreparedMask;
    bool isPreparedRoi = IsPreparedROI(specROI.m_preparedObjectName);

    m_property->FindItem(ROI_PREPARED_MASKNAME)->SetReadOnly(!(isPreparedType));
    m_property->FindItem(ROI_PREPARED_MASKDILATE)->SetReadOnly(!(isPreparedType && !isPreparedRoi));
    m_property->FindItem(ROI_PREPARED_ROIEXPAND_X)->SetReadOnly(!(isPreparedType && isPreparedRoi));
    m_property->FindItem(ROI_PREPARED_ROIEXPAND_Y)->SetReadOnly(!(isPreparedType && isPreparedRoi));
}

CString DlgParaSingle::GetCategortyName(long roiIndex)
{
    long currentLayer = m_parent.GetSelectLayerIndex();

    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return _T("");

    auto& specSingleLayer = m_pVisionPara->getLayer(currentLayer).m_single;

    auto& spec = specSingleLayer[roiIndex];
    long roi_type = (long)spec.getType();

    if (spec.getType() == UserRoiType::Polygon_32f)
    {
        roi_type += spec.getPolygonPointCount() - 3;
    }

    CString categoryName;
    categoryName.Format(_T("[%s] ROI%02d"),
        ((spec.m_operation == OperationType::Plus)           ? _T("+")
                : (spec.m_operation == OperationType::Minus) ? _T("-")
                                                             : _T("&")),
        roiIndex + 1);

    const auto& scale = m_pVisionInsp->getScale();
    float um2px = scale.umToPixelXY();

    if (spec.getType() == UserRoiType::PreparedMask)
    {
        categoryName.AppendFormat(
            _T(" - %s (%.1f)"), (LPCTSTR)spec.m_preparedObjectName, spec.m_preparedMaskDilateInUm * um2px);
    }
    else
    {
        categoryName.AppendFormat(_T(" - %s"), (LPCTSTR)m_userROITypes[roi_type]);
    }

    return categoryName;
}

bool DlgParaSingle::IsPreparedROI(LPCTSTR name)
{
    for (auto& text : m_preparedMasks)
    {
        if (text.first == name)
        {
            return text.second == 0;
        }
    }

    return false;
}

void DlgParaSingle::menuFunction_deleteRoiIndex(long roiIndex)
{
    long currentLayer = m_parent.GetSelectLayerIndex();
    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return;

    if (MessageBox(_T("Do you want to delete ROI?"), _T("Message"), MB_YESNO) == IDYES)
    {
        auto& specLayer = m_pVisionPara->getLayer(currentLayer);
        specLayer.m_single.del(roiIndex);
        m_parent.m_parent->PostMessage(UM_REFRESH_UI);
    }
}

LRESULT DlgParaSingle::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    switch (wparam)
    {
        case XTP_PGN_ITEMVALUE_CHANGED:
            OnGridNotify_itemValueChanged(item);
            break;
    }

    return 0;
}

void DlgParaSingle::OnGridNotify_itemValueChanged(CXTPPropertyGridItem* item)
{
    if (item->GetID() == DISPLAY_OTHER_LAYER_MASK)
    {
        if (auto* value = dynamic_cast<CXTPPropertyGridItemBool*>(item))
        {
            m_parent.m_displayOtherLayerMask = value->GetBool();
            m_parent.m_parent->PostMessage(UM_REFRESH_UI);
        }

        return;
    }

    long currentLayer = m_parent.GetSelectLayerIndex();
    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return;

    long roiIndex = m_maskList.GetSelectedItemKey();
    auto& specROI = m_pVisionPara->getLayer(currentLayer).m_single[roiIndex];
    const auto& scale = m_pVisionInsp->getScale();
    float px2um = (scale.pixelToUm().m_x + scale.pixelToUm().m_y) * 0.5f;

    switch (item->GetID())
    {
        case ROI_TYPE:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
            {
                SetROIType(roiIndex, value->GetEnum());
                m_parent.m_parent->SetROI();
                m_parent.m_parent->SetDisplayImage();

                UpdateProperty_AuthorityAndRefreshMaskInfo();
            }
            break;

        case ROI_PREPARED_MASKNAME:
            if (auto* value = dynamic_cast<CXTPPropertyGridItem*>(item))
            {
                specROI.m_preparedObjectName = value->GetValue();
                m_parent.m_parent->SetROI();
                m_parent.m_parent->SetDisplayImage();

                UpdateProperty_AuthorityAndRefreshMaskInfo();
            }
            break;

        case ROI_PREPARED_MASKDILATE:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
            {
                specROI.m_preparedMaskDilateInUm = float(value->GetDouble() * px2um);
                m_parent.m_parent->SetROI();
                m_parent.m_parent->SetDisplayImage();

                UpdateProperty_AuthorityAndRefreshMaskInfo();
            }
            break;

        case ROI_PREPARED_ROIEXPAND_X:
            if (auto* value = dynamic_cast<XTPPropertyGridItemCustomFloat<float>*>(item))
            {
                m_parent.m_parent->SetROI();
                m_parent.m_parent->SetDisplayImage();

                UpdateProperty_AuthorityAndRefreshMaskInfo();
            }
            break;

        case ROI_PREPARED_ROIEXPAND_Y:
            if (auto* value = dynamic_cast<XTPPropertyGridItemCustomFloat<float>*>(item))
            {
                m_parent.m_parent->SetROI();
                m_parent.m_parent->SetDisplayImage();

                UpdateProperty_AuthorityAndRefreshMaskInfo();
            }
            break;

        case ROI_OPERATION:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
            {
                specROI.m_operation = (OperationType)value->GetEnum();
                m_parent.m_parent->SetROI();
                m_parent.m_parent->SetDisplayImage();

                UpdateProperty_AuthorityAndRefreshMaskInfo();
            }
            break;

        case DISPLAY_OTHER_LAYER_MASK:
            if (auto* value = dynamic_cast<CXTPPropertyGridItemBool*>(item))
            {
                m_parent.m_displayOtherLayerMask = value->GetBool();
                m_parent.m_parent->PostMessage(UM_REFRESH_UI);
            }
            break;
    }
}

void DlgParaSingle::SetROIType(long index, long userType)
{
    long currentLayer = m_parent.GetSelectLayerIndex();

    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return;

    auto& specSingleLayer = m_pVisionPara->getLayer(currentLayer).m_single;

    switch (userType)
    {
        case -1:
            specSingleLayer[index].setType(UserRoiType::PreparedMask);
            break;
        case 0:
            specSingleLayer[index].setType(UserRoiType::Rect);
            break;
        case 1:
            specSingleLayer[index].setType(UserRoiType::Ellipse_32f);
            break;
        default:
            specSingleLayer[index].setType(UserRoiType::Polygon_32f);
            specSingleLayer[index].setPolygonPointCount(userType + 1);
            break;
    }
}

void DlgParaSingle::OnLbnDblclkListUserRoi()
{
    long selectType = m_lstUserRoi.GetCurSel();
    long currentLayer = m_parent.GetSelectLayerIndex();
    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return;
    if (selectType < 0)
        return;

    auto& specSingleLayer = m_pVisionPara->getLayer(currentLayer).m_single;
    specSingleLayer.add(UserRoiType::Rect);
    SetROIType(specSingleLayer.count() - 1, selectType);

    m_parent.m_parent->SetROI();
    m_parent.m_parent->SetDisplayImage();

    UpdateLayerMaskList();
}

void DlgParaSingle::OnLbnDblclkListPreparedImage()
{
    if (m_lstPreparedImage.GetCurSel() < 0)
        return;
    CString maskName = m_preparedMasks[m_lstPreparedImage.GetCurSel()].first;
    long currentLayer = m_parent.GetSelectLayerIndex();
    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return;

    auto& specSingleLayer = m_pVisionPara->getLayer(currentLayer).m_single;
    specSingleLayer.add(UserRoiType::PreparedMask);
    specSingleLayer[specSingleLayer.count() - 1].m_preparedObjectName = maskName;

    m_parent.m_parent->SetROI();
    m_parent.m_parent->SetDisplayImage();

    UpdateLayerMaskList();
}

BOOL DlgParaSingle::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (HIWORD(wParam) == 0)
    {
        long nID = LOWORD(wParam);

        switch (nID)
        {
            case Menu_ChangeOrder_Delete:
                menuFunction_deleteRoiIndex(m_selectMenuRoiIndex);
                return TRUE;
                break;
        }
    }

    return CDialogEx::OnCommand(wParam, lParam);
}

LRESULT DlgParaSingle::OnMaskSelected(WPARAM, LPARAM)
{
    UpdateProperty();
    return 0;
}

LRESULT DlgParaSingle::OnMaskMoved(WPARAM wParam, LPARAM lParam)
{
    long itemIndex = (long)wParam;
    long itemDestIndex = (long)lParam;

    auto* specLayer = m_parent.GetSelectSpecLayer();
    if (specLayer == nullptr)
        return 0;

    while (itemIndex != itemDestIndex)
    {
        long nextIndex = itemIndex > itemDestIndex ? itemIndex - 1 : itemIndex + 1;
        specLayer->m_single.swap(itemIndex, nextIndex);

        itemIndex = nextIndex;
    }

    m_maskList.SelectItem(itemDestIndex);
    UpdateLayerMaskList();

    return 0;
}

LRESULT DlgParaSingle::OnMaskDBClicked(WPARAM wParam, LPARAM)
{
    long currentLayer = m_parent.GetSelectLayerIndex();
    long maskIndex = long(wParam);

    if (currentLayer < 0 || currentLayer >= m_pVisionPara->getLayerCount())
        return 0;

    auto& specSingleLayer = m_pVisionPara->getLayer(currentLayer).m_single;

    if (maskIndex < 0 || maskIndex >= specSingleLayer.count())
        return 0;

    auto* item = m_property->FindItem(ROI_OPERATION);

    OperationType setOperation = OperationType::Plus;

    switch (specSingleLayer[maskIndex].m_operation)
    {
        case OperationType::Plus:
            setOperation = OperationType::Minus;
            break;
        case OperationType::Minus:
            setOperation = OperationType::Plus;
            break;
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        specSingleLayer[maskIndex].m_operation = setOperation;
        m_parent.m_parent->SetROI();
        m_parent.m_parent->SetDisplayImage();

        UpdateRefreshMaskInfo(maskIndex);
        UpdateProperty_AuthorityAndRefreshMaskInfo();
    }

    return 0;
}

LRESULT DlgParaSingle::OnMaskRClicked(WPARAM wParam, LPARAM)
{
    auto* specLayer = m_parent.GetSelectSpecLayer();
    if (specLayer == nullptr)
        return 0;

    long roiIndex = (long)wParam;
    if (roiIndex < 0 || roiIndex >= specLayer->m_single.count())
        return 0;

    m_selectMenuRoiIndex = roiIndex;

    // Category가 선택된 상황이다
    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING | MF_DISABLED, 0, GetCategortyName(roiIndex));
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, Menu_ChangeOrder_Delete, _T("Delete"));

    CPoint ptMouse;
    ::GetCursorPos(&ptMouse);

    menu.TrackPopupMenu(TPM_RIGHTALIGN | TPM_LEFTBUTTON, ptMouse.x, ptMouse.y, this);

    return 0;
}
