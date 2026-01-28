//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSelectEnableMarkMapData.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionMark.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_IMAGE_LOT_VIEW_ROI_CHANGED (WM_USER + 1012)
#define MK_MERGE_ROI_COUNT_MAX 3
#define MK_IGNORE_ROI_COUNT_MAX 3

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgSelectEnableMarkMapData, CDialog)

CDlgSelectEnableMarkMapData::CDlgSelectEnableMarkMapData(VisionInspectionMark* i_pVisionMarkInsp,
    const sReadMarkMapInfo& i_sMarkReadDataInfo, bool i_bIsTeachROI, bool i_bImportMode, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DIALOG_SELECT_ENABLE_MARK_MAPDATA, pParent)
    , m_pVisionMarkInsp(i_pVisionMarkInsp)
    , m_propertyGrid(nullptr)
    , m_imageLotView(nullptr)
    , m_sMarkReadDataInfo(i_sMarkReadDataInfo)
    , m_nPreSelect_TeachROI_idx(0)
    , m_nTeachROI_Count((long)m_sMarkReadDataInfo.vecTeachROI_Info.size())
    , m_nIgnoreROI_Count((long)m_sMarkReadDataInfo.vecIgnoreROI_Info.size())
    , m_nMergeROI_Count((long)m_sMarkReadDataInfo.vecMergeROI_Info.size())
    , m_bIsTeachROI(i_bIsTeachROI)
    , m_bImportMode(i_bImportMode)
{
    m_vecbTeachROI_Enable.clear();
    m_vecbIgnoreROI_Enable.clear();
    m_vecbMergeROI_Enable.clear();
    m_sEnableMarkMapInfo_px.Clear();
}

CDlgSelectEnableMarkMapData::~CDlgSelectEnableMarkMapData()
{
    delete m_propertyGrid;
    delete m_imageLotView;
}

void CDlgSelectEnableMarkMapData::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgSelectEnableMarkMapData, CDialog)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &CDlgSelectEnableMarkMapData::OnRoiChanged)
END_MESSAGE_MAP()

BOOL CDlgSelectEnableMarkMapData::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_bImportMode == true)
    {
        DWORD old_style = GetClassLong(m_hWnd, GCL_STYLE);
        SetClassLong(m_hWnd, GCL_STYLE, old_style | CS_NOCLOSE);
    }

    CRect rtimageview;
    GetDlgItem(IDC_MARK_MAPDATA_PREVIEW)->GetWindowRect(rtimageview);
    ScreenToClient(rtimageview);

    m_imageLotView = new ImageLotView(rtimageview, *m_pVisionMarkInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);

    m_imageLotView->ShowImage(m_pVisionMarkInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    CRect rtPropertyGrid;
    GetDlgItem(IDC_PROPERTYGRID_AREA)->GetWindowRect(rtPropertyGrid);
    ScreenToClient(rtPropertyGrid);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtPropertyGrid, this, IDC_FRAME_PROPERTY_GRID);

    UpdatePropertyGrid();

    return TRUE;
}

void CDlgSelectEnableMarkMapData::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    m_propertyGrid->SetVariableItemsHeight();

    if (auto* category = m_propertyGrid->AddCategory(_T("Mark MapData List")))
    {
        if (auto* TeachROI_category = m_propertyGrid->AddCategory(_T("TeachROI List")))
        {
            long nTeachROICount = m_nTeachROI_Count;

            if (m_bIsTeachROI && m_nTeachROI_Count <= 0)
            {
                SetUserROI();
                nTeachROICount = 1;
                m_nTeachROI_Count = 1;
            }

            m_vecbTeachROI_Enable.resize(nTeachROICount);
            if (m_bIsTeachROI)
                m_vecbTeachROI_Enable[0] = TRUE;

            for (long nidx = 0; nidx < nTeachROICount; nidx++)
            {
                auto TeachROIInfo = m_sMarkReadDataInfo.vecTeachROI_Info[nidx];
                if (auto* item = TeachROI_category->AddChildItem(new CXTPPropertyGridItemBool(
                        TeachROIInfo.strName, m_vecbTeachROI_Enable[nidx], &m_vecbTeachROI_Enable[nidx])))
                {
                    item->SetID(ITEM_ID_TEACH_ROI_DEFINE + nidx);
                    (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
                }
            }

            if (auto* item
                = TeachROI_category->AddChildItem(new CCustomItemButton(_T("Show Select Teach ROI"), TRUE, FALSE)))
            {
                item->SetID(ITEM_ID_SHOW_SELECT_TEACH_ROI);

                if (nTeachROICount <= 0)
                    item->SetReadOnly(TRUE);
            }

            TeachROI_category->Expand();
        }

        CString strIgnoreROI_Title("");
        strIgnoreROI_Title.Format(_T("IgnoreROI List(Select Max Count : %d)"), MK_IGNORE_ROI_COUNT_MAX);
        if (auto* IgnoreROI_category = m_propertyGrid->AddCategory(strIgnoreROI_Title))
        {
            long nIgnoreROICount = m_nIgnoreROI_Count;
            m_vecbIgnoreROI_Enable.resize(nIgnoreROICount);
            for (long nidx = 0; nidx < nIgnoreROICount; nidx++)
            {
                auto IgnoreROIInfo = m_sMarkReadDataInfo.vecIgnoreROI_Info[nidx];
                if (auto* item = IgnoreROI_category->AddChildItem(new CXTPPropertyGridItemBool(
                        IgnoreROIInfo.strName, m_vecbIgnoreROI_Enable[nidx], &m_vecbIgnoreROI_Enable[nidx])))
                {
                    item->SetID(ITEM_ID_IGNORE_ROI_DEFINE + nidx);
                    (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
                }
            }

            if (auto* item
                = IgnoreROI_category->AddChildItem(new CCustomItemButton(_T("Show Select Ignore ROI"), TRUE, FALSE)))
            {
                item->SetID(ITEM_ID_SHOW_SELECT_IGNORE_ROI);

                if (nIgnoreROICount <= 0)
                    item->SetReadOnly(TRUE);
            }

            IgnoreROI_category->Expand();
        }

        CString strMergeROI_Title("");
        strIgnoreROI_Title.Format(_T("MergeROI List(Select Max Count : %d)"), MK_MERGE_ROI_COUNT_MAX);
        if (auto* MergeROI_category = m_propertyGrid->AddCategory(strIgnoreROI_Title))
        {
            long nMergeROICount = m_nMergeROI_Count;
            m_vecbMergeROI_Enable.resize(nMergeROICount);
            for (long nidx = 0; nidx < nMergeROICount; nidx++)
            {
                auto MergeROIInfo = m_sMarkReadDataInfo.vecMergeROI_Info[nidx];
                if (auto* item = MergeROI_category->AddChildItem(new CXTPPropertyGridItemBool(
                        MergeROIInfo.strName, m_vecbMergeROI_Enable[nidx], &m_vecbMergeROI_Enable[nidx])))
                {
                    item->SetID(ITEM_ID_MERGE_ROI_DEFINE + nidx);
                    (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
                }
            }

            if (auto* item
                = MergeROI_category->AddChildItem(new CCustomItemButton(_T("Show Select Merge ROI"), TRUE, FALSE)))
            {
                item->SetID(ITEM_ID_SHOW_SELECT_MERGE_ROI);

                if (m_nMergeROI_Count <= 0)
                    item->SetReadOnly(TRUE);
            }

            MergeROI_category->Expand();
        }

        if (auto* Item = category->AddChildItem(new CCustomItemButton(_T("Apply"), TRUE, FALSE)))
            Item->SetID(ITEM_ID_ENABLE_APPLY);

        if (auto* Item = category->AddChildItem(new CCustomItemButton(_T("Show All Setting ROI"), TRUE, FALSE)))
            Item->SetID(ITEM_ID_SHOW_ALL_SETTING_ROI);

        category->Expand();

        strIgnoreROI_Title.Empty();
        strMergeROI_Title.Empty();
    }

    SetDefalutEnable();

    SetReadOnlyItem(DefineSelectROI_Type::Teach);
    SetReadOnlyItem(DefineSelectROI_Type::Ignore);
    SetReadOnlyItem(DefineSelectROI_Type::Merge);

    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->Refresh();
}

void CDlgSelectEnableMarkMapData::SetDefalutEnable()
{
    long nTeachROICount = (long)m_vecbTeachROI_Enable.size();
    for (long nTeachROIidx = 0; nTeachROIidx < nTeachROICount;
        nTeachROIidx++) //첫번째는 무조건 Check 및 Teach ROI는 View가 되어야한다
    {
        m_vecbTeachROI_Enable[nTeachROIidx] = TRUE;
        ShowSelectedROI(DefineSelectROI_Type::Teach, m_sMarkReadDataInfo.vecTeachROI_Info);
        break;
    }

    long nIgnoreROICount = (long)m_vecbIgnoreROI_Enable.size();
    for (long nIgnoreROIidx = 0; nIgnoreROIidx < nIgnoreROICount; nIgnoreROIidx++) //최대 3개
    {
        m_vecbIgnoreROI_Enable[nIgnoreROIidx] = TRUE;
        if (nIgnoreROIidx >= MK_IGNORE_ROI_COUNT_MAX)
            break;
    }

    long nMergeROICount = (long)m_vecbMergeROI_Enable.size();
    for (long nMergeROIidx = 0; nMergeROIidx < nMergeROICount; nMergeROIidx++)
    {
        m_vecbMergeROI_Enable[nMergeROIidx] = TRUE;
        if (nMergeROIidx >= MK_MERGE_ROI_COUNT_MAX)
            break;
    }
}

LRESULT CDlgSelectEnableMarkMapData::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* Item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (Item->GetID())
        {
            case ITEM_ID_ENABLE_APPLY:
            {
                if (::SimpleMessage(_T("Do you want Apply Enable Setting?"), MB_YESNO) == IDYES)
                {
                    if (m_bImportMode == true)
                    {
                        DWORD old_style = GetClassLong(m_hWnd, GCL_STYLE);
                        SetClassLong(m_hWnd, GCL_STYLE, old_style ^ CS_NOCLOSE);
                    }
                    SetEnableMarkData();
                    OnOK();
                }
                else
                    return 0;
            }
            //BUG: 여기에 break 없는 것은 의도된 것인지?? 의도된 것이라면 [[fallthrough]] 속성을 사용할 것.
            case ITEM_ID_SHOW_ALL_SETTING_ROI:
                ShowAllOverlay();
                break;
            case ITEM_ID_SHOW_SELECT_TEACH_ROI:
            case ITEM_ID_SHOW_SELECT_IGNORE_ROI:
            case ITEM_ID_SHOW_SELECT_MERGE_ROI:
                ShowViewROI(Item->GetID());
                break;
            default:
                break;
        }

        UINT nID = Item->GetID();
        if (nID >= ITEM_ID_TEACH_ROI_DEFINE && nID < ITEM_ID_IGNORE_ROI_DEFINE)
        {
            SetReadOnlyItem(DefineSelectROI_Type::Teach);
            if (m_sMarkReadDataInfo.vecTeachROI_Info.size() > 0)
                ShowSelectedROI(DefineSelectROI_Type::Teach, m_sMarkReadDataInfo.vecTeachROI_Info);
        }

        if (nID >= ITEM_ID_IGNORE_ROI_DEFINE && nID < ITEM_ID_MERGE_ROI_DEFINE)
        {
            SetReadOnlyItem(DefineSelectROI_Type::Ignore);
            if (m_sMarkReadDataInfo.vecIgnoreROI_Info.size() > 0)
                ShowSelectedROI(DefineSelectROI_Type::Ignore, m_sMarkReadDataInfo.vecIgnoreROI_Info);
        }
        else if (nID >= ITEM_ID_MERGE_ROI_DEFINE)
        {
            SetReadOnlyItem(DefineSelectROI_Type::Merge);
            if (m_sMarkReadDataInfo.vecMergeROI_Info.size() > 0)
                ShowSelectedROI(DefineSelectROI_Type::Merge, m_sMarkReadDataInfo.vecMergeROI_Info);
        }
        else //Unknown Evnet
            return 0;
    }

    return 0;
}

void CDlgSelectEnableMarkMapData::ShowViewROI(long i_nSelect_Show_ROIType)
{
    switch (i_nSelect_Show_ROIType)
    {
        case ITEM_ID_SHOW_SELECT_TEACH_ROI:
            ShowSelectedROI(DefineSelectROI_Type::Teach, m_sMarkReadDataInfo.vecTeachROI_Info);
            break;
        case ITEM_ID_SHOW_SELECT_IGNORE_ROI:
            ShowSelectedROI(DefineSelectROI_Type::Ignore, m_sMarkReadDataInfo.vecIgnoreROI_Info);
            break;
        case ITEM_ID_SHOW_SELECT_MERGE_ROI:
            ShowSelectedROI(DefineSelectROI_Type::Merge, m_sMarkReadDataInfo.vecMergeROI_Info);
            break;
        default:
            break;
    }
}

void CDlgSelectEnableMarkMapData::SetReadOnlyItem(DefineSelectROI_Type i_DefineSelectROIType)
{
    long nEnableDataCount(0);
    switch (i_DefineSelectROIType)
    {
        case DefineSelectROI_Type::Teach:
        {
            if (m_vecbTeachROI_Enable.size() > 1)
                m_vecbTeachROI_Enable[m_nPreSelect_TeachROI_idx] = FALSE;

            for (long nidx = 0; nidx < m_vecbTeachROI_Enable.size(); nidx++)
            {
                auto bEnableCheck = m_vecbTeachROI_Enable[nidx];
                if (bEnableCheck == TRUE)
                {
                    m_nPreSelect_TeachROI_idx = nidx;
                    m_propertyGrid->FindItem(ITEM_ID_TEACH_ROI_DEFINE + nidx)->SetReadOnly(TRUE);
                }
                else
                    m_propertyGrid->FindItem(ITEM_ID_TEACH_ROI_DEFINE + nidx)->SetReadOnly(FALSE);
            }
            break;
        }
        case DefineSelectROI_Type::Ignore:
        {
            for (long nidx = 0; nidx < m_vecbIgnoreROI_Enable.size(); nidx++)
            {
                auto bEnableCheck = m_vecbIgnoreROI_Enable[nidx];
                if (bEnableCheck == TRUE)
                    nEnableDataCount++;
            }

            if (nEnableDataCount >= MK_IGNORE_ROI_COUNT_MAX)
            {
                for (long nidx = 0; nidx < m_vecbIgnoreROI_Enable.size(); nidx++)
                {
                    auto bEnableCheck = m_vecbIgnoreROI_Enable[nidx];
                    if (bEnableCheck == TRUE)
                        continue;

                    m_propertyGrid->FindItem(ITEM_ID_IGNORE_ROI_DEFINE + nidx)->SetReadOnly(TRUE);
                }
            }
            else if (nEnableDataCount < MK_IGNORE_ROI_COUNT_MAX)
            {
                for (long nidx = 0; nidx < m_vecbIgnoreROI_Enable.size(); nidx++)
                    m_propertyGrid->FindItem(ITEM_ID_IGNORE_ROI_DEFINE + nidx)->SetReadOnly(FALSE);
            }

            break;
        }
        case DefineSelectROI_Type::Merge:
        {
            for (long nidx = 0; nidx < m_vecbMergeROI_Enable.size(); nidx++)
            {
                auto bEnableCheck = m_vecbMergeROI_Enable[nidx];
                if (bEnableCheck == TRUE)
                    nEnableDataCount++;
            }

            if (nEnableDataCount >= MK_IGNORE_ROI_COUNT_MAX)
            {
                for (long nidx = 0; nidx < m_vecbMergeROI_Enable.size(); nidx++)
                {
                    auto bEnableCheck = m_vecbMergeROI_Enable[nidx];
                    if (bEnableCheck == TRUE)
                        continue;

                    m_propertyGrid->FindItem(ITEM_ID_MERGE_ROI_DEFINE + nidx)->SetReadOnly(TRUE);
                }
            }
            else if (nEnableDataCount < MK_IGNORE_ROI_COUNT_MAX)
            {
                for (long nidx = 0; nidx < m_vecbMergeROI_Enable.size(); nidx++)
                    m_propertyGrid->FindItem(ITEM_ID_MERGE_ROI_DEFINE + nidx)->SetReadOnly(FALSE);
            }

            break;
        }
        default:
            break;
    }

    m_propertyGrid->Refresh();
}

void CDlgSelectEnableMarkMapData::ShowSelectedROI(
    DefineSelectROI_Type i_DefineSelectROIType, std::vector<sMarkROI_Info> i_vecsMarkROI_Info)
{
    m_imageLotView->Overlay_RemoveAll();
    COLORREF color(RGB(0, 0, 0));

    std::vector<BOOL> vecbShowROIidx;
    switch (i_DefineSelectROIType)
    {
        case DefineSelectROI_Type::Teach:
            vecbShowROIidx = GetShowROIidx(m_vecbTeachROI_Enable);
            color = RGB(0, 255, 0);
            break;
        case DefineSelectROI_Type::Merge:
            vecbShowROIidx = GetShowROIidx(m_vecbMergeROI_Enable);
            color = RGB(0, 0, 255);
            break;
        case DefineSelectROI_Type::Ignore:
            vecbShowROIidx = GetShowROIidx(m_vecbIgnoreROI_Enable);
            color = RGB(255, 0, 0);
            break;
        default:
            break;
    }

    CString strMarkType("");
    for (long nShowROIidx = 0; nShowROIidx < vecbShowROIidx.size(); nShowROIidx++)
    {
        switch (i_DefineSelectROIType)
        {
            case DefineSelectROI_Type::Teach:
                strMarkType.Format(_T("Mark Teach ROI"));
                break;
            case DefineSelectROI_Type::Merge:
                strMarkType.Format(_T("Merge ROI_%d"), nShowROIidx);
                break;
            case DefineSelectROI_Type::Ignore:
                strMarkType.Format(_T("Ignore ROI_%d"), nShowROIidx);
                break;
            default:
                break;
        }

        auto Enableidx = vecbShowROIidx[nShowROIidx];
        FPI_RECT frtMarkROI = GetMarkROI_px(i_vecsMarkROI_Info[Enableidx]);

        Ipvm::Point32r2 fptOverlayDisplay = GetOverlayMarkNamePos(frtMarkROI);

        m_imageLotView->Overlay_AddText(fptOverlayDisplay, strMarkType, color, 50);
        m_imageLotView->Overlay_AddRectangle(frtMarkROI, color);
    }

    m_imageLotView->Overlay_Show(TRUE);

    strMarkType.Empty();
}

void CDlgSelectEnableMarkMapData::ShowAllOverlay()
{
    m_imageLotView->Overlay_RemoveAll();

    std::vector<BOOL> vecShowTeach_ROIidx;
    std::vector<BOOL> vecShowIgnore_ROIidx;
    std::vector<BOOL> vecShowMerge_ROIidx;

    if (m_vecbTeachROI_Enable.size() > 0)
        vecShowTeach_ROIidx = GetShowROIidx(m_vecbTeachROI_Enable);
    if (m_vecbMergeROI_Enable.size() > 0)
        vecShowMerge_ROIidx = GetShowROIidx(m_vecbMergeROI_Enable);
    if (m_vecbIgnoreROI_Enable.size() > 0)
        vecShowIgnore_ROIidx = GetShowROIidx(m_vecbIgnoreROI_Enable);

    for (long nShow_TeachROI_idx = 0; nShow_TeachROI_idx < vecShowTeach_ROIidx.size(); nShow_TeachROI_idx++)
    {
        CString strTeachROI("");
        strTeachROI.Format(_T("Mark Teach ROI"));

        auto Enableidx = vecShowTeach_ROIidx[nShow_TeachROI_idx];
        if (m_sMarkReadDataInfo.vecTeachROI_Info.size() <= 0) //없을 수가 없다
            return;

        FPI_RECT frtMarkROI = GetMarkROI_px(m_sMarkReadDataInfo.vecTeachROI_Info[Enableidx]);

        Ipvm::Point32r2 fptOverlayDisplay = GetOverlayMarkNamePos(frtMarkROI);

        m_imageLotView->Overlay_AddText(fptOverlayDisplay, strTeachROI, RGB(0, 255, 0), 50);
        m_imageLotView->Overlay_AddRectangle(frtMarkROI, RGB(0, 255, 0));

        strTeachROI.Empty();
    }

    for (long nShow_MergeROI_idx = 0; nShow_MergeROI_idx < vecShowMerge_ROIidx.size(); nShow_MergeROI_idx++)
    {
        CString strMergeROI("");
        strMergeROI.Format(_T("Merge ROI_%d"), nShow_MergeROI_idx);

        if (m_sMarkReadDataInfo.vecMergeROI_Info.size() <= 0)
            break;

        auto Enableidx = vecShowMerge_ROIidx[nShow_MergeROI_idx];
        FPI_RECT frtMarkROI = GetMarkROI_px(m_sMarkReadDataInfo.vecMergeROI_Info[Enableidx]);

        Ipvm::Point32r2 fptOverlayDisplay = GetOverlayMarkNamePos(frtMarkROI);

        m_imageLotView->Overlay_AddText(fptOverlayDisplay, strMergeROI, RGB(0, 0, 255), 50);
        m_imageLotView->Overlay_AddRectangle(frtMarkROI, RGB(0, 0, 255));

        strMergeROI.Empty();
    }

    for (long nShow_IgnoreROI_idx = 0; nShow_IgnoreROI_idx < vecShowIgnore_ROIidx.size(); nShow_IgnoreROI_idx++)
    {
        CString strIgnoreROI("");
        strIgnoreROI.Format(_T("ignore ROI_%d"), nShow_IgnoreROI_idx);

        if (m_sMarkReadDataInfo.vecIgnoreROI_Info.size() <= 0)
            break;

        auto Enableidx = vecShowIgnore_ROIidx[nShow_IgnoreROI_idx];
        FPI_RECT frtMarkROI = GetMarkROI_px(m_sMarkReadDataInfo.vecIgnoreROI_Info[Enableidx]);

        Ipvm::Point32r2 fptOverlayDisplay = GetOverlayMarkNamePos(frtMarkROI);

        m_imageLotView->Overlay_AddText(fptOverlayDisplay, strIgnoreROI, RGB(255, 0, 0), 50);
        m_imageLotView->Overlay_AddRectangle(frtMarkROI, RGB(255, 0, 0));

        strIgnoreROI.Empty();
    }

    m_imageLotView->Overlay_Show(TRUE);
}

std::vector<BOOL> CDlgSelectEnableMarkMapData::GetShowROIidx(std::vector<BOOL> i_vecbROI_Enable)
{
    std::vector<BOOL> vecbEnableidx;
    for (long nidx = 0; nidx < i_vecbROI_Enable.size(); nidx++)
    {
        if (i_vecbROI_Enable[nidx] == TRUE)
            vecbEnableidx.push_back(nidx);
    }

    return vecbEnableidx;
}

FPI_RECT CDlgSelectEnableMarkMapData::GetMarkROI_px(sMarkROI_Info i_sROI_Info)
{
    float fMarkHalfWidth = i_sROI_Info.fWidth / 2.f;
    float fMarkHalfLength = i_sROI_Info.fLength / 2.f;
    float fMarkAngle = -i_sROI_Info.fAngle;

    FPI_RECT sfrtMarkROI(
        Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f));
    Ipvm::Point32r2 fCenter(i_sROI_Info.fOffsetX, i_sROI_Info.fOffsetY);
    Ipvm::Rect32r frtROI;
    frtROI.m_left = (fCenter.m_x - fMarkHalfWidth);
    frtROI.m_top = (fCenter.m_y - fMarkHalfLength);
    frtROI.m_right = (fCenter.m_x + fMarkHalfWidth);
    frtROI.m_bottom = (fCenter.m_y + fMarkHalfLength);

    sfrtMarkROI = FPI_RECT(Ipvm::Point32r2(frtROI.m_left, frtROI.m_top), Ipvm::Point32r2(frtROI.m_right, frtROI.m_top),
        Ipvm::Point32r2(frtROI.m_left, frtROI.m_bottom), Ipvm::Point32r2(frtROI.m_right, frtROI.m_bottom));

    if (fMarkAngle != 0)
    {
        float fAngle = fMarkAngle * ITP_DEG_TO_RAD;

        // 시계 반대 방향이 +
        sfrtMarkROI = sfrtMarkROI.Rotate(fAngle);
    }

    return sfrtMarkROI;
}

Ipvm::Point32r2 CDlgSelectEnableMarkMapData::GetOverlayMarkNamePos(FPI_RECT i_frtMarkROIPos_px)
{
    Ipvm::Point32r2 fptreturnValue(i_frtMarkROIPos_px.fptLB);

    float fOffsetValue = 10.f;

    fptreturnValue.Offset(fOffsetValue, fOffsetValue);

    return fptreturnValue;
}

void CDlgSelectEnableMarkMapData::SetEnableMarkData()
{
    m_sEnableMarkMapInfo_px.Clear();

    for (long nTeachEnableidx = 0; nTeachEnableidx < m_vecbTeachROI_Enable.size(); nTeachEnableidx++)
    {
        if (m_vecbTeachROI_Enable[nTeachEnableidx] == TRUE)
        {
            m_sEnableMarkMapInfo_px.sTeachROI_Info.strName
                = m_sMarkReadDataInfo.vecTeachROI_Info[nTeachEnableidx].strName;
            m_sEnableMarkMapInfo_px.sTeachROI_Info.strID = m_sMarkReadDataInfo.vecTeachROI_Info[nTeachEnableidx].strID;
            m_sEnableMarkMapInfo_px.sTeachROI_Info.fOffsetX
                = m_sMarkReadDataInfo.vecTeachROI_Info[nTeachEnableidx].fOffsetX;
            m_sEnableMarkMapInfo_px.sTeachROI_Info.fOffsetY
                = m_sMarkReadDataInfo.vecTeachROI_Info[nTeachEnableidx].fOffsetY;
            m_sEnableMarkMapInfo_px.sTeachROI_Info.fWidth
                = m_sMarkReadDataInfo.vecTeachROI_Info[nTeachEnableidx].fWidth;
            m_sEnableMarkMapInfo_px.sTeachROI_Info.fLength
                = m_sMarkReadDataInfo.vecTeachROI_Info[nTeachEnableidx].fLength;
            m_sEnableMarkMapInfo_px.sTeachROI_Info.fAngle
                = m_sMarkReadDataInfo.vecTeachROI_Info[nTeachEnableidx].fAngle;
            m_sEnableMarkMapInfo_px.sTeachROI_Info.nType = m_sMarkReadDataInfo.vecTeachROI_Info[nTeachEnableidx].nType;
        }
    }

    for (long nIgnoreEnableidx = 0; nIgnoreEnableidx < m_vecbIgnoreROI_Enable.size(); nIgnoreEnableidx++)
    {
        if (m_vecbIgnoreROI_Enable[nIgnoreEnableidx] == TRUE)
        {
            m_sEnableMarkMapInfo_px.vecIgnoreROI_Info.push_back(
                m_sMarkReadDataInfo.vecIgnoreROI_Info[nIgnoreEnableidx]);
        }
    }

    for (long nMergeEnableidx = 0; nMergeEnableidx < m_vecbMergeROI_Enable.size(); nMergeEnableidx++)
    {
        if (m_vecbMergeROI_Enable[nMergeEnableidx] == TRUE)
        {
            m_sEnableMarkMapInfo_px.vecMergeROI_Info.push_back(m_sMarkReadDataInfo.vecMergeROI_Info[nMergeEnableidx]);
        }
    }
}

sMarkMapInfo CDlgSelectEnableMarkMapData::GetEnableMarkMapInfo()
{
    return m_sEnableMarkMapInfo_px;
}

void CDlgSelectEnableMarkMapData::SetUserROI()
{
    const int nSizeX = m_pVisionMarkInsp->getImageLotInsp().GetImageSizeX();
    const int nSizeY = m_pVisionMarkInsp->getImageLotInsp().GetImageSizeY();

    Ipvm::Rect32s imageRoi(0, 0, nSizeX, nSizeY);
    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    m_imageLotView->ROI_RemoveAll();

    const auto& scale = m_pVisionMarkInsp->getScale();

    /// Mark Teach ROI
    Ipvm::Rect32r frtDefalutROI = Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f);
    Ipvm::Rect32s rtROI = scale.convert_BCUToPixel(frtDefalutROI, imageCenter);
    rtROI &= imageRoi;

    SetUserROIData(rtROI, true);

    m_imageLotView->ROI_Add(_T("Mark Teach ROI"), _T("Mark Teach ROI"), rtROI, RGB(0, 255, 0), TRUE, TRUE);

    m_imageLotView->ROI_Show(TRUE);
}

LRESULT CDlgSelectEnableMarkMapData::OnRoiChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    Ipvm::Rect32s roi;
    m_imageLotView->ROI_Get(_T("Mark Teach ROI"), roi);

    SetUserROIData(roi);

    return 0;
}

void CDlgSelectEnableMarkMapData::SetUserROIData(
    Ipvm::Rect32s i_rtROI_px, bool i_bInitailize) //여기까지 왔다는건 MapData에 ROI가 없다는거
{
    sMarkROI_Info sUserTeachROI_Info;
    sUserTeachROI_Info.strID = _T("User ROI");
    sUserTeachROI_Info.strName = _T("User Teach ROI");
    sUserTeachROI_Info.fOffsetX = (float)i_rtROI_px.CenterPoint().m_x;
    sUserTeachROI_Info.fOffsetY = (float)i_rtROI_px.CenterPoint().m_y;
    sUserTeachROI_Info.fWidth = (float)i_rtROI_px.Width();
    sUserTeachROI_Info.fLength = (float)i_rtROI_px.Height();
    sUserTeachROI_Info.fAngle = 0.f;
    sUserTeachROI_Info.nType = MARK_ROI_TEACH;

    if (i_bInitailize == true)
        m_sMarkReadDataInfo.vecTeachROI_Info.push_back(sUserTeachROI_Info);
    else
        m_sMarkReadDataInfo.vecTeachROI_Info[0] = sUserTeachROI_Info;
}

BOOL CDlgSelectEnableMarkMapData::PreTranslateMessage(MSG* pMsg)
{
    if (m_bImportMode == true)
    {
        if (WM_KEYDOWN == pMsg->message)
        {
            if (VK_RETURN == pMsg->wParam || VK_ESCAPE == pMsg->wParam)
            {
                return TRUE;
            }
        }
    }

    return CDialog::PreTranslateMessage(pMsg);
}
