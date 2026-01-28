//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingSpecTR.h"

//CPP_2_________________________________ This project's headers
#include "BgaGridGeneratorDlg.h"
#include "BodySizeEditorTR.h"
#include "DlgLandShapeEditor.h"
#include "VisionProcessingSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/BallCollectionOriginal.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTapeSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../VisionProcessingMapDataEditor/VisionMapDataEditorUI.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <map>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_LIST_PROCESSING_SPEC 100
#define UM_CUSTOMOBJECTTRANSFER (WM_USER + 4445) // WPARAM 0 : Normal Mode

//CPP_7_________________________________ Implementation body
//
enum enumPropertyID
{
    ePID_BodySize,
    ePID_SprocketHoleExistType,
    ePID_CategoryWarning,
};

IMPLEMENT_DYNAMIC(DlgVisionProcessingSpecTR, CDialog)

DlgVisionProcessingSpecTR::DlgVisionProcessingSpecTR(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingSpec* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(DlgVisionProcessingSpecTR::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_packageSpec(pVisionInsp->m_packageSpec)
    , m_tapeSpec(*pVisionInsp->getTapeSpec())
    , m_propertyGrid(new CXTPPropertyGrid)
    , m_pVisionEditorDlg(nullptr)
{
}

DlgVisionProcessingSpecTR::~DlgVisionProcessingSpecTR()
{
    delete m_propertyGrid;
    delete m_pVisionEditorDlg;

    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void DlgVisionProcessingSpecTR::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STC_VIEWER, m_stcViewer);
    DDX_Control(pDX, IDC_TAPE_LAYOUT_BOTH, m_staticTapeLayoutBoth);
    DDX_Control(pDX, IDC_TAPE_LAYOUT_LEFT, m_staticTapeLayoutLeft);
    DDX_Control(pDX, IDC_TAPE_LAYOUT_RIGHT, m_staticTapeLayoutRight);
}

BEGIN_MESSAGE_MAP(DlgVisionProcessingSpecTR, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_WM_PAINT()
ON_WM_LBUTTONDOWN()
ON_WM_HSCROLL()
ON_WM_CLOSE()
ON_WM_LBUTTONDBLCLK()
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// DlgVisionProcessingSpecTR 메시지 처리기입니다.

BOOL DlgVisionProcessingSpecTR::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect tapeLayoutRect;
    GetDlgItem(IDC_TAPE_LAYOUT_BOTH)->GetWindowRect(tapeLayoutRect);
    ScreenToClient(tapeLayoutRect);

    CRect rtPropertyGrid;
    rtPropertyGrid.left = m_procDlgInfo.m_rtParaArea.left;
    rtPropertyGrid.top = m_procDlgInfo.m_rtParaArea.top;
    rtPropertyGrid.right = m_procDlgInfo.m_rtParaArea.right;
    rtPropertyGrid.bottom = m_procDlgInfo.m_rtDataArea.bottom;

    // Layout을 Property Grid 밑에 위치시키고 Property 크기를 조절한다
    tapeLayoutRect.MoveToX(rtPropertyGrid.left + (rtPropertyGrid.Width() - tapeLayoutRect.Width()) / 2);
    tapeLayoutRect.MoveToY(rtPropertyGrid.bottom - tapeLayoutRect.Height());

    rtPropertyGrid.bottom = tapeLayoutRect.top - 5; // Property Grid와 Layout 사이에 여백을 둔다.

    GetDlgItem(IDC_FRAME_PROPERTY_GRID)->MoveWindow(rtPropertyGrid);
    m_staticTapeLayoutBoth.MoveWindow(tapeLayoutRect);
    m_staticTapeLayoutLeft.MoveWindow(tapeLayoutRect);
    m_staticTapeLayoutRight.MoveWindow(tapeLayoutRect);

    m_propertyGrid->Create(rtPropertyGrid, this, IDC_FRAME_PROPERTY_GRID);

    m_stcViewer.MoveWindow(m_procDlgInfo.m_rtImageArea);
    m_drawView.Create(this, m_procDlgInfo.m_rtImageArea);

    RefreshSpecLayout();
    UpdatePropertyGrid();
    RefreshView();

    return TRUE; // return TRUE  unless you set the focus to a control
}

int DlgVisionProcessingSpecTR::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void DlgVisionProcessingSpecTR::OnDestroy()
{
    CDialog::OnDestroy();
}

void DlgVisionProcessingSpecTR::Update_Warning()
{
    // Warning List 만들기
    auto* warningCategory = (CXTPPropertyGridItemCategory*)m_propertyGrid->FindItem(ePID_CategoryWarning);
    warningCategory->GetChilds()->Clear();

    warningCategory->SetHidden(warningCategory->GetChilds()->GetCount() == 0);
}

void DlgVisionProcessingSpecTR::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Device")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Setup body size"), TRUE, FALSE))->SetID(ePID_BodySize);
        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Tape Layer Spec (Unit : mm)")))
    {
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Sprocket Hole Exist"), int32_t(m_tapeSpec.m_sprocketHoleExistType))))
        {
            item->GetConstraints()->AddConstraint(_T("Both"), long(TapeSprocketHoleExistType::e_both));
            item->GetConstraints()->AddConstraint(_T("Left"), long(TapeSprocketHoleExistType::e_leftOnly));
            item->GetConstraints()->AddConstraint(_T("Right"), long(TapeSprocketHoleExistType::e_rightOnly));
            item->SetID(ePID_SprocketHoleExistType);
        }

        AddPropertyFloat(category, L"W0", m_tapeSpec.m_w0);
        AddPropertyFloat(category, L"A0", m_tapeSpec.m_a0);
        AddPropertyFloat(category, L"B0", m_tapeSpec.m_b0);
        AddPropertyFloat(category, L"P0", m_tapeSpec.m_p0);
        AddPropertyFloat(category, L"P1", m_tapeSpec.m_p1);
        AddPropertyFloat(category, L"D0", m_tapeSpec.m_d0);
        AddPropertyFloat(category, L"D1", m_tapeSpec.m_d1);
        AddPropertyFloat(category, L"S0", m_tapeSpec.m_s0);
        AddPropertyFloat(category, L"S1", m_tapeSpec.m_s1);
        AddPropertyFloat(category, L"S2", m_tapeSpec.m_s2);
        AddPropertyFloat(category, L"E1", m_tapeSpec.m_e1);
        AddPropertyFloat(category, L"F0", m_tapeSpec.m_f0);
        AddPropertyFloat(category, L"F1", m_tapeSpec.m_f1);
        AddPropertyFloat(category, L"F2", m_tapeSpec.m_f2);

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Warning List")))
    {
        category->SetID(ePID_CategoryWarning);
        category->Expand();
    }

    m_propertyGrid->SetViewDivider(0.30);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

CXTPPropertyGridItem* DlgVisionProcessingSpecTR::AddPropertyFloat(
    CXTPPropertyGridItem* parent, const wchar_t* name, float& value)
{
    auto* item
        = parent->AddChildItem(new XTPPropertyGridItemCustomFloat(name, value, L"%.3f", value, 0.f, 100.f, 0.001f));
    return item;
}

void DlgVisionProcessingSpecTR::propertyButton_BodySize()
{
    BodySizeEditorTR Dlg(m_packageSpec);

    if (Dlg.DoModal() == IDOK)
    {
        if (Dlg.m_bChangedDeadBug)
            ::AfxMessageBox(_T("Dead bug option has been changed.\nThis option will take effect after job save."));

        FireExchanged();
        RefreshView();

        Invalidate(FALSE);
    }
}

void DlgVisionProcessingSpecTR::OnClose()
{
    HWND hWnd = ::FindWindow(NULL, _T("IntekPlus Gerber Extractor"));

    if (hWnd != NULL)
    {
        ::PostMessage(hWnd, WM_CLOSE, 0, 0);
    }

    m_propertyGrid->DestroyWindow();

    CDialog::OnClose();
}

void DlgVisionProcessingSpecTR::RefreshSpecLayout()
{
    auto type = m_tapeSpec.m_sprocketHoleExistType;
    m_staticTapeLayoutBoth.ShowWindow(type == TapeSprocketHoleExistType::e_both ? SW_SHOW : SW_HIDE);
    m_staticTapeLayoutLeft.ShowWindow(type == TapeSprocketHoleExistType::e_leftOnly ? SW_SHOW : SW_HIDE);
    m_staticTapeLayoutRight.ShowWindow(type == TapeSprocketHoleExistType::e_rightOnly ? SW_SHOW : SW_HIDE);
}

void DlgVisionProcessingSpecTR::RefreshView()
{
    m_drawView.Set(m_tapeSpec);
    Update_Warning();
}

LRESULT DlgVisionProcessingSpecTR::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ePID_BodySize:
                propertyButton_BodySize();
                break;
        }

        value->SetBool(FALSE);
        return 0;
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = value->GetEnum();
        data = (int)max(data, 0);
        switch (item->GetID())
        {
            case ePID_SprocketHoleExistType:
            {
                auto type = (TapeSprocketHoleExistType)data;
                m_tapeSpec.m_sprocketHoleExistType = type;

                RefreshSpecLayout();
                RefreshView();
                FireExchanged();
                break;
            }
        }

        return 0;
    }

    m_tapeSpec.ApplyToTrayScanSpec(m_pVisionInsp->getTrayScanSpec());
    RefreshView();
    return 0;
}

void DlgVisionProcessingSpecTR::FireExchanged()
{
    SystemConfig::GetInstance().m_bIsExchangedPackageSpec = true;
}
