//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingPadAlign2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgAlgorithmDB.h"
#include "DlgSelectPAD.h"
#include "ParaDB.h"
#include "VisionProcessingPadAlign2D.h"
#include "VisionProcessingPadAlign2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)
#define UM_IMAGE_LOT_VIEW_ROI_CHANGED (WM_USER + 1012)

//CPP_7_________________________________ Implementation body
//
enum GridChildItemID
{
    ITEM_ID_SET_PAD_ALGORITHM,
    ITEM_ID_SELECT_PAD,
    ITEM_ID_BLOB_THRESHOLD,
    ITEM_ID_BLOB_SEARCH_OFFSET,
    ITEM_ID_EDGE_SEARCH_OFFSET,
    ITEM_ID_EDGE_SEARCH_LENGTH,
    ITEM_ID_EDGE_SEARCH_DIRECTION,
    ITEM_ID_EDGE_THRESHOLD,
    ITEM_ID_EDGE_DRECTION,
    ITEM_ID_EDGE_FIRST_EDGE,
    ITEM_ID_EDGE_SEARCH_COUNT,
    ITEM_ID_EDGE_SEARCH_WIDTH,
    ITEM_ID_SET_IMAGE_COMBINE,
    ITEM_ID_SET_ALGORITHM_DB,
};

IMPLEMENT_DYNAMIC(DlgVisionProcessingPadAlign2D, CDialog)

DlgVisionProcessingPadAlign2D::DlgVisionProcessingPadAlign2D(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingPadAlign2D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(DlgVisionProcessingPadAlign2D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_property_para(new CXTPPropertyGrid)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_dialog_selectPAD(nullptr)
{
    m_paraDB = new ParaDB(*pVisionInsp);
    m_paraDB->Load();
    m_pVisionInsp = pVisionInsp;
}

DlgVisionProcessingPadAlign2D::~DlgVisionProcessingPadAlign2D()
{
    delete m_dialog_selectPAD;
    delete m_paraDB;
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_property_para;
    delete m_imageLotView;

    m_pVisionInsp->m_pVisionInspDlg = nullptr;
}

void DlgVisionProcessingPadAlign2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(DlgVisionProcessingPadAlign2D, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &DlgVisionProcessingPadAlign2D::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &DlgVisionProcessingPadAlign2D::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &DlgVisionProcessingPadAlign2D::OnImageLotViewRoiChanged)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// DlgVisionProcessingPadAlign2D 메시지 처리기입니다.

BOOL DlgVisionProcessingPadAlign2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    m_property_para->Create(m_procDlgInfo.m_rtParaArea, this, 99999);

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Debug"));
    m_TabResult.InsertItem(1, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);

    m_dialog_selectPAD = new DlgSelectPAD(m_pVisionInsp, m_imageLotView, m_paraDB, this);
    m_dialog_selectPAD->Create(IDD_DIALOG_SELECT_PAD, this);

    return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgVisionProcessingPadAlign2D::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_property_para->DestroyWindow();
}

void DlgVisionProcessingPadAlign2D::OnClose()
{
    CDialog::OnDestroy();
}

void DlgVisionProcessingPadAlign2D::SetInitParaWindow()
{
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0));

    UpdatePropertyGrid();
}

void DlgVisionProcessingPadAlign2D::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp);
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_imageLotView->Overlay_Show(TRUE);

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();
}

void DlgVisionProcessingPadAlign2D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    m_procCommonDebugInfoDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 1 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

LRESULT DlgVisionProcessingPadAlign2D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_pVisionInspParaDlg->SetROI();

    m_pVisionInsp->m_visionUnit.RunInspection(m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing());
    m_pVisionInsp->DoInspection(true);

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);

    return 0L;
}

LRESULT DlgVisionProcessingPadAlign2D::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_pVisionInspParaDlg->GetROI();

    return 0L;
}

void DlgVisionProcessingPadAlign2D::UpdatePropertyGrid()
{
    m_property_para->ResetContent();

    auto* visionPara = m_pVisionInsp->m_VisionPara;

    //-----------------------------------------------------------------------------------
    // 현재 Job과 관련없는 PAD 정보를 정리해 주자
    // 정리할 시점이 마땅치 않아서 이때 정리해 주었다
    //-----------------------------------------------------------------------------------

    auto& package_padList = m_pVisionInsp->m_packageSpec.m_PadMapManager->vecPadData;

    for (auto& package_pad : package_padList)
    {
        long paraIndex = visionPara->FindAlignPAD_Index(package_pad.strPadName);
        if (package_pad.bIgnore && paraIndex >= 0)
        {
            visionPara->DelAlignPAD(package_pad.strPadName);
            continue;
        }
    }

    for (long alignIndex = 0; alignIndex < visionPara->GetAlignPAD_Count(); alignIndex++)
    {
        CString padName = visionPara->GetAlignPAD_Name(alignIndex);
        bool findPad = false;
        for (auto& package_pad : package_padList)
        {
            if (package_pad.strPadName == padName)
            {
                findPad = true;
                break;
            }
        }

        if (!findPad)
        {
            visionPara->DelAlignPAD(padName);
        }

        padName.Empty();
    }

    //-----------------------------------------------------------------------------------

    CString algorithm_name;
    long select_pad_index = -1;
    for (long n = 0; n < 2; n++)
    {
        if (m_select_pad_name == _T("") && visionPara->GetAlignPAD_Count())
        {
            m_select_pad_name = visionPara->GetAlignPAD_Name(0);
        }

        for (long padIndex = 0; padIndex < visionPara->GetAlignPAD_Count(); padIndex++)
        {
            if (m_select_pad_name == visionPara->GetAlignPAD_Name(padIndex))
            {
                select_pad_index = padIndex;
                algorithm_name = visionPara->GetAlignPAD_ParaName(m_select_pad_name);
                break;
            }
        }

        if (select_pad_index >= 0)
            break;
        m_select_pad_name = _T("");

        algorithm_name.Empty();
    }

    {
        auto* category = m_property_para->AddCategory(_T("Algorithm"));
        category->AddChildItem(new CCustomItemButton(_T("Set PAD Alogirthm"), TRUE, FALSE))
            ->SetID(ITEM_ID_SET_PAD_ALGORITHM);
        auto* itemSelectPAD = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Select PAD"), select_pad_index));
        itemSelectPAD->GetConstraints()->AddConstraint(_T(""), -1); //kircheis_고민/확인필요

        // 등록된 PAD 순으로 보여주자

        std::vector<std::pair<CString, long>> padList;
        for (auto& package_pad : package_padList)
        {
            if (package_pad.bIgnore)
                continue;
            long paraIndex = visionPara->FindAlignPAD_Index(package_pad.strPadName);
            if (paraIndex >= 0)
            {
                CString name;
                name.Format(_T("%s (%s)"), LPCTSTR(package_pad.strPadName),
                    visionPara->GetAlignPAD_ParaName(package_pad.strPadName));

                padList.emplace_back(name, paraIndex);

                name.Empty();
            }
        }

        for (auto& padName : padList)
        {
            itemSelectPAD->GetConstraints()->AddConstraint(padName.first, padName.second);
        }

        itemSelectPAD->SetID(ITEM_ID_SELECT_PAD);

        category->AddChildItem(new CXTPPropertyGridItem(_T("Select PAD Algorithm"), algorithm_name))->SetReadOnly(TRUE);
        category->Expand();
    }

    if (!m_select_pad_name.IsEmpty())
    {
        auto* para = visionPara->GetAlignPAD_Para(m_select_pad_name);

        if (para == nullptr)
        {
            ASSERT(!_T("???"));
            return;
        }

        if (GetPadType(m_select_pad_name) != PAD_TYPE::_typeCircle)
        {
            auto* teachCategory = m_property_para->AddCategory(_T("Blob (Prealign)"));
            if (auto* item
                = teachCategory->AddChildItem(new CXTPPropertyGridItemNumber(_T("Threshold"), para->m_blobThreshold)))
            {
                if (auto* slider = item->AddSliderControl())
                {
                    slider->SetMin(0);
                    slider->SetMax(256);
                }

                if (auto* spin = item->AddSpinButton())
                {
                    spin->SetMin(0);
                    spin->SetMax(256);
                }

                item->SetID(ITEM_ID_BLOB_THRESHOLD);
            }
            teachCategory
                ->AddChildItem(new CXTPPropertyGridItemDouble(_T("Search Range (um)"), para->m_blobSearchOffset_um))
                ->SetID(ITEM_ID_BLOB_SEARCH_OFFSET);
            teachCategory->Expand();
        }

        if (GetPadType(m_select_pad_name) == PAD_TYPE::_typeCircle
            || (GetPadType(m_select_pad_name) == PAD_TYPE::_typePinIndex
                && visionPara->GetAlignPAD_Category(m_select_pad_name) == enumPAD_Category::PinIndex))
        {
            auto* edgeCategory = m_property_para->AddCategory(_T("Edge Detection"));
            edgeCategory
                ->AddChildItem(new CXTPPropertyGridItemDouble(_T("Search Offset (um)"), para->m_edgeSearchOffset_um))
                ->SetID(ITEM_ID_EDGE_SEARCH_OFFSET);
            edgeCategory
                ->AddChildItem(new CXTPPropertyGridItemDouble(_T("Search Length (um)"), para->m_edgeSearchLength_um))
                ->SetID(ITEM_ID_EDGE_SEARCH_LENGTH);

            auto* para_edgeSearchDirection = edgeCategory->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Search Direction"), para->m_edgeSearchDirection));
            para_edgeSearchDirection->GetConstraints()->AddConstraint(_T("In -> Out"), 0);
            para_edgeSearchDirection->GetConstraints()->AddConstraint(_T("Out -> In"), 1);
            para_edgeSearchDirection->SetID(ITEM_ID_EDGE_SEARCH_DIRECTION);

            edgeCategory->AddChildItem(new CXTPPropertyGridItemDouble(_T("Threshold"), para->m_edgeThreshold))
                ->SetID(ITEM_ID_EDGE_THRESHOLD);

            auto* para_edgeDirection
                = edgeCategory->AddChildItem(new CXTPPropertyGridItemEnum(_T("Edge Direction"), para->m_edgeType));
            para_edgeDirection->GetConstraints()->AddConstraint(_T("Both"), 0);
            para_edgeDirection->GetConstraints()->AddConstraint(_T("Rising"), 1);
            para_edgeDirection->GetConstraints()->AddConstraint(_T("Falling"), 2);
            para_edgeDirection->SetID(ITEM_ID_EDGE_DRECTION);

            if (auto* item = edgeCategory->AddChildItem(
                    new CXTPPropertyGridItemBool(_T("Find First Edge"), para->m_findFirstEdge)))
            {
                item->SetID(ITEM_ID_EDGE_FIRST_EDGE);
                (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            }

            edgeCategory->AddChildItem(new CXTPPropertyGridItemNumber(_T("Search count"), para->m_edgeSearchCount))
                ->SetID(ITEM_ID_EDGE_SEARCH_COUNT);
            edgeCategory->AddChildItem(new CXTPPropertyGridItemNumber(_T("Search Width"), para->m_edgeSearchWidth))
                ->SetID(ITEM_ID_EDGE_SEARCH_WIDTH);
            edgeCategory->Expand();
        }

        {
            auto* category = m_property_para->AddCategory(_T("ETC"));
            category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
                ->SetID(ITEM_ID_SET_IMAGE_COMBINE);
            category->AddChildItem(new CCustomItemButton(_T("Algorithm DB"), TRUE, FALSE))
                ->SetID(ITEM_ID_SET_ALGORITHM_DB);
            category->Expand();
        }
    }

    m_property_para->SetViewDivider(0.6);
    m_property_para->HighlightChangedItems(TRUE);
}

LRESULT DlgVisionProcessingPadAlign2D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_SET_PAD_ALGORITHM:
                CallSetPADAlgorithm();
                break;
            case ITEM_ID_SET_IMAGE_COMBINE:
                CallImageCombine();
                break;
            case ITEM_ID_SET_ALGORITHM_DB:
                CallAlgorithmDB();
                break;
        }

        value->SetBool(FALSE);
        return 0;
    }

    if (m_select_pad_name.IsEmpty())
    {
        return 0;
    }

    auto* para = m_pVisionInsp->m_VisionPara->GetAlignPAD_Para(m_select_pad_name);
    if (para == nullptr)
        return 0;

    if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
    {
        double data = value->GetDouble();

        switch (item->GetID())
        {
            case ITEM_ID_BLOB_SEARCH_OFFSET:
                para->m_blobSearchOffset_um = (float)data;
                break;
            case ITEM_ID_EDGE_SEARCH_OFFSET:
                para->m_edgeSearchOffset_um = (float)data;
                break;
            case ITEM_ID_EDGE_SEARCH_LENGTH:
                para->m_edgeSearchLength_um = (float)data;
                break;
            case ITEM_ID_EDGE_THRESHOLD:
                para->m_edgeThreshold = (float)data;
                break;
        }
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
    {
        long data = value->GetNumber();

        switch (item->GetID())
        {
            case ITEM_ID_BLOB_THRESHOLD:
                if (1)
                {
                    para->m_blobThreshold = (long)data;

                    Ipvm::Image8u image;
                    Ipvm::Rect32s roi;
                    m_pVisionInsp->getBlobThresholdImage(m_select_pad_name, image, roi);
                    m_imageLotView->SetImage(image, _T("Threshold Image"));
                    m_imageLotView->Overlay_RemoveAll();
                    m_imageLotView->Overlay_AddRectangle(roi, RGB(0, 255, 0));
                    m_imageLotView->Overlay_Show(TRUE);
                }
                break;
            case ITEM_ID_EDGE_SEARCH_COUNT:
                para->m_edgeSearchCount = data;
                break;
            case ITEM_ID_EDGE_SEARCH_WIDTH:
                para->m_edgeSearchWidth = data;
                break;
        }
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = value->GetEnum();
        data = (int)max(data, 0);
        switch (item->GetID())
        {
            case ITEM_ID_EDGE_SEARCH_DIRECTION:
                para->m_edgeSearchDirection = data;
                break;

            case ITEM_ID_EDGE_DRECTION:
                para->m_edgeType = data;
                break;

            case ITEM_ID_SELECT_PAD:
                m_select_pad_name = m_pVisionInsp->m_VisionPara->GetAlignPAD_Name(data);
                UpdatePropertyGrid();
                break;
        }

        return 0;
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemBool*>(item))
    {
        BOOL data = value->GetBool();
        switch (item->GetID())
        {
            case ITEM_ID_EDGE_FIRST_EDGE:
                para->m_findFirstEdge = data;
                break;
                break;
        }
    }

    return 0;
}

void DlgVisionProcessingPadAlign2D::CallSetPADAlgorithm()
{
    if (m_dialog_selectPAD->IsWindowVisible())
    {
        m_dialog_selectPAD->ShowWindow(SW_HIDE);
    }
    else
    {
        m_dialog_selectPAD->ShowWindow(SW_SHOW);
    }
}

PAD_TYPE DlgVisionProcessingPadAlign2D::GetPadType(LPCTSTR PAD_name)
{
    auto& padList = m_pVisionInsp->m_packageSpec.m_PadMapManager->vecPadData;
    for (auto& padInfo : padList)
    {
        if (padInfo.strPadName == PAD_name)
        {
            return padInfo.GetType();
        }
    }

    return _typeRectangle;
}

void DlgVisionProcessingPadAlign2D::CallImageCombine()
{
    if (m_select_pad_name.IsEmpty())
        return;

    auto* proc = m_pVisionInsp;
    auto* para = proc->m_VisionPara->GetAlignPAD_Para(m_select_pad_name);
    if (para == nullptr)
        return;

    ::Combine_SetParameter(*proc, proc->GetImageFrameIndex(0), false, &para->m_imageProc);
}

void DlgVisionProcessingPadAlign2D::CallAlgorithmDB()
{
    DlgAlgorithmDB dlg(*m_pVisionInsp, *m_pVisionInsp->m_VisionPara, *m_paraDB);
    if (dlg.DoModal() != IDOK)
        return;

    CiDataBase dbObject1, dbObject2;
    dlg.m_db.LinkDataBase(TRUE, dbObject1);
    dlg.m_para.LinkDataBase(TRUE, dbObject2);

    m_paraDB->LinkDataBase(FALSE, dbObject1);
    m_pVisionInsp->m_VisionPara->LinkDataBase(FALSE, dbObject2);

    m_paraDB->Save();
    UpdatePropertyGrid();
}
