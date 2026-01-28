//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingFiducialAlign.h"

//CPP_2_________________________________ This project's headers
#include "Result.h"
#include "VisionProcessingFiducialAlign.h"
#include "VisionProcessingFiducialAlignPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/Config.h"
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
    ITEM_ID_USE_SELECT_IMAGE,
    ITEM_ID_SELECT_PAD1,
    ITEM_ID_SELECT_PAD2,
    ITEM_ID_EDGE_SEARCH_OFFSET,
    ITEM_ID_SET_IMAGE_COMBINE,
};

IMPLEMENT_DYNAMIC(DlgVisionProcessingFiducialAlign, CDialog)

DlgVisionProcessingFiducialAlign::DlgVisionProcessingFiducialAlign(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingFiducialAlign* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(DlgVisionProcessingFiducialAlign::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_property_para(new CXTPPropertyGrid)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
{
    m_pVisionInsp = pVisionInsp;
}

DlgVisionProcessingFiducialAlign::~DlgVisionProcessingFiducialAlign()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_property_para;
    delete m_imageLotView;

    m_pVisionInsp->m_pVisionInspDlg = nullptr;
}

void DlgVisionProcessingFiducialAlign::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(DlgVisionProcessingFiducialAlign, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &DlgVisionProcessingFiducialAlign::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &DlgVisionProcessingFiducialAlign::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &DlgVisionProcessingFiducialAlign::OnImageLotViewRoiChanged)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// DlgVisionProcessingFiducialAlign 메시지 처리기입니다.

BOOL DlgVisionProcessingFiducialAlign::OnInitDialog()
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

    return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgVisionProcessingFiducialAlign::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_property_para->DestroyWindow();
}

void DlgVisionProcessingFiducialAlign::OnClose()
{
    CDialog::OnDestroy();
}

void DlgVisionProcessingFiducialAlign::SetInitParaWindow()
{
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0));

    UpdatePropertyGrid();
}

void DlgVisionProcessingFiducialAlign::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_imageLotView->Overlay_Show(TRUE);

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();
}

void DlgVisionProcessingFiducialAlign::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    m_procCommonDebugInfoDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 1 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

LRESULT DlgVisionProcessingFiducialAlign::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_pVisionInspParaDlg->SetROI();

    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());
    m_pVisionInsp->DoInspection(true);

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);

    return 0L;
}

LRESULT DlgVisionProcessingFiducialAlign::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_pVisionInspParaDlg->GetROI();

    return 0L;
}

void DlgVisionProcessingFiducialAlign::UpdatePropertyGrid()
{
    m_property_para->ResetContent();

    auto* visionPara = m_pVisionInsp->m_VisionPara;

    //-----------------------------------------------------------------------------------
    // 현재 Job과 관련없는 PAD 정보를 정리해 주자
    // 정리할 시점이 마땅치 않아서 이때 정리해 주었다
    //-----------------------------------------------------------------------------------

    auto& package_padList = m_pVisionInsp->m_packageSpec.m_PadMapManager->vecPadData;

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
    // 08/02/2019
    // 기존 : PAD 값을 넣지 않고 보여주기만 강제로 첫번째 PAD 를 선택한 것처럼 보여줌
    // 수정 : PAD 값이 안들어 있으면 실제로 선택해줌
    // 이유 : PAD 는 첫번째 것이 선택되어 있는 것처럼 보이나
    //        마침 첫번째 PAD가 올바른 PAD라서 값을 바꾸지 않고 진행하면
    //        실제로는 선택된 것이 없어서 동작을 제대로 하지 않음

    while (visionPara->m_use_pad_names.size() < 2)
    {
        // PAD 선택 공간이 두개는 있어야 한다. 일단 강제로 넣어주자.
        visionPara->m_use_pad_names.push_back(_T(""));
    }

    if (visionPara->m_pad_name_to_index.size())
    {
        for (long index = 0; index < long(visionPara->m_use_pad_names.size()); index++)
        {
            if (visionPara->FindAlignPAD_Index(visionPara->m_use_pad_names[index]) < 0)
            {
                visionPara->m_use_pad_names[index] = visionPara->m_pad_name_to_index.begin()->first;
            }
        }
    }

    //-----------------------------------------------------------------------------------

    if (auto* category = m_property_para->AddCategory(_T("Algorithm")))
    {
        long selectPad1Index = visionPara->FindAlignPAD_Index(visionPara->m_use_pad_names[0]);
        long selectPad2Index = visionPara->FindAlignPAD_Index(visionPara->m_use_pad_names[1]);

        if (Config::getVisionType() == VISIONTYPE_3D_INSP)
        {
            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemEnum(_T("Select Image"), visionPara->m_select_use_image)))
            {
                item->GetConstraints()->AddConstraint(_T("Z-map"), 0);
                item->GetConstraints()->AddConstraint(_T("V-map"), 1);
                item->SetID(ITEM_ID_USE_SELECT_IMAGE);
            }
        }

        auto* itemSelectPAD1
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Select PAD (1)"), selectPad1Index));
        auto* itemSelectPAD2
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Select PAD (2)"), selectPad2Index));

        if (selectPad1Index
            >= 0) //kircheis_20230404 Crash 대응//등록된 Pad가 없는 상황에서 Fiducial Align으로 들어보면 아랫 줄에서 Crash 발생하는데, 이 변수가 -1이면 죽는다. 이를 막기 위한 code
            m_select_pad1_name = visionPara->GetAlignPAD_Name(selectPad1Index);

        if (selectPad2Index
            >= 0) //kircheis_20230404 Crash 대응//등록된 Pad가 없는 상황에서 Fiducial Align으로 들어보면 아랫 줄에서 Crash 발생하는데, 이 변수가 -1이면 죽는다. 이를 막기 위한 code
            m_select_pad2_name = visionPara->GetAlignPAD_Name(selectPad2Index);

        // 등록된 PAD 순으로 보여주자

        std::vector<std::pair<CString, long>> padList;
        for (auto& package_pad : package_padList)
        {
            if (package_pad.bIgnore)
                continue;
            long paraIndex = visionPara->FindAlignPAD_Index(package_pad.strPadName);
            if (paraIndex >= 0)
            {
                padList.emplace_back(package_pad.strPadName, paraIndex);
            }
        }

        for (auto& padName : padList)
        {
            itemSelectPAD1->GetConstraints()->AddConstraint(padName.first, padName.second);
            itemSelectPAD2->GetConstraints()->AddConstraint(padName.first, padName.second);
        }

        itemSelectPAD1->SetID(ITEM_ID_SELECT_PAD1);
        itemSelectPAD2->SetID(ITEM_ID_SELECT_PAD2);

        category->Expand();
    }

    if (auto* edgeCategory = m_property_para->AddCategory(_T("Center Detection")))
    {
        edgeCategory
            ->AddChildItem(new CXTPPropertyGridItemDouble(_T("Search Offset (um)"), visionPara->m_SearchOffset_um))
            ->SetID(ITEM_ID_EDGE_SEARCH_OFFSET);
        edgeCategory->Expand();
    }

    {
        auto* category = m_property_para->AddCategory(_T("ETC"));
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(ITEM_ID_SET_IMAGE_COMBINE);
        category->Expand();
    }

    m_property_para->SetViewDivider(0.6);
    m_property_para->HighlightChangedItems(TRUE);
}

LRESULT DlgVisionProcessingFiducialAlign::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
    {
        if (wparam == XTP_PGN_SELECTION_CHANGED)
        {
            switch (item->GetID())
            {
                case ITEM_ID_SELECT_PAD1:
                    updateOverlayPad(0);
                    break;
                case ITEM_ID_SELECT_PAD2:
                    updateOverlayPad(1);
                    break;
            }
        }
        return 0;
    }

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_SET_IMAGE_COMBINE:
                CallImageCombine();
                break;
        }

        value->SetBool(FALSE);
        return 0;
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
    {
        double data = value->GetDouble();

        switch (item->GetID())
        {
            case ITEM_ID_EDGE_SEARCH_OFFSET:
                m_pVisionInsp->m_VisionPara->m_SearchOffset_um = (float)data;
                break;
        }
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = value->GetEnum();
        CString dataValue = value->GetValue();

        data = (int)max(data, 0);
        switch (item->GetID())
        {
            case ITEM_ID_USE_SELECT_IMAGE:
                if (data < use_end)
                    m_pVisionInsp->m_VisionPara->m_select_use_image = data;
                break;
            case ITEM_ID_SELECT_PAD1:
                m_select_pad1_name = dataValue;

                m_pVisionInsp->m_VisionPara->m_use_pad_names.clear();

                m_pVisionInsp->m_VisionPara->m_use_pad_names.push_back(m_select_pad1_name);
                m_pVisionInsp->m_VisionPara->m_use_pad_names.push_back(m_select_pad2_name);

                updateOverlayPad(0);
                UpdatePropertyGrid();
                break;
            case ITEM_ID_SELECT_PAD2:
                m_select_pad2_name = dataValue;

                m_pVisionInsp->m_VisionPara->m_use_pad_names.clear();

                m_pVisionInsp->m_VisionPara->m_use_pad_names.push_back(m_select_pad1_name);
                m_pVisionInsp->m_VisionPara->m_use_pad_names.push_back(m_select_pad2_name);

                updateOverlayPad(1);
                UpdatePropertyGrid();
                break;
        }

        return 0;
    }

    return 0;
}

PAD_TYPE DlgVisionProcessingFiducialAlign::GetPadType(LPCTSTR PAD_name)
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

void DlgVisionProcessingFiducialAlign::CallImageCombine()
{
    if (m_select_pad1_name.IsEmpty() || m_select_pad2_name.IsEmpty())
        return;

    auto* proc = m_pVisionInsp;

    ::Combine_SetParameter(*proc, proc->GetImageFrameIndex(0), false, &proc->m_VisionPara->m_imageProc);
}

void DlgVisionProcessingFiducialAlign::updateOverlayPad(long selectPADType)
{
    m_imageLotView->Overlay_RemoveAll();

    auto& padNames = m_pVisionInsp->m_VisionPara->m_use_pad_names;

    if (selectPADType < 0 || selectPADType >= long(padNames.size()))
        return;

    CString name = padNames[selectPADType];
    long idx = m_pVisionInsp->m_VisionPara->FindAlignPAD_Index(name);
    if (idx < 0)
        return;

    m_imageLotView->Overlay_AddRectangle(m_pVisionInsp->m_result->m_debugResult_spec_pad_rois[idx], RGB(0, 255, 0));
    m_imageLotView->Overlay_Show(TRUE);

    name.Empty();
}