//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingFOV.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingFOV.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100

//CPP_7_________________________________ Implementation body
//
enum GridChildItemID
{
    ITEM_ID_TRAY_POCKET_COUNT_X = 1,
    ITEM_ID_TRAY_POCKET_COUNT_Y,
    ITEM_ID_FOV_PANE_COUNT_X,
    ITEM_ID_FOV_PANE_COUNT_Y,
    ITEM_ID_FOV_PANE_PITCH_X,
    ITEM_ID_FOV_PANE_PITCH_Y,
    ITEM_ID_FOV_PANE_OFFSET_X,
    ITEM_ID_FOV_PANE_OFFSET_Y,
};

IMPLEMENT_DYNAMIC(CDlgVisionProcessingFOV, CDialog)

CDlgVisionProcessingFOV::CDlgVisionProcessingFOV(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingFOV* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionProcessingFOV::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_imageLotView(nullptr)
    , m_propertyGrid(nullptr)
{
}

CDlgVisionProcessingFOV::~CDlgVisionProcessingFOV()
{
    delete m_imageLotView;
    delete m_propertyGrid;
}

void CDlgVisionProcessingFOV::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgVisionProcessingFOV, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// CDlgVisionProcessingFOV 메시지 처리기입니다.

BOOL CDlgVisionProcessingFOV::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtClient;
    GetClientRect(rtClient);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, false, false, GetSafeHwnd());

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID);

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    {
        if (auto* trayInfo = m_propertyGrid->AddCategory(_T("Tray information")))
        {
            if (auto* item = trayInfo->AddChildItem(new CXTPPropertyGridItemNumber(
                    _T("Pocket count x"), m_pVisionInsp->getTrayScanSpec().m_pocketNumX)))
            {
                item->SetReadOnly();
                item->SetID(ITEM_ID_TRAY_POCKET_COUNT_X);
            }
            if (auto* item = trayInfo->AddChildItem(new CXTPPropertyGridItemNumber(
                    _T("Pocket count y"), m_pVisionInsp->getTrayScanSpec().m_pocketNumY)))
            {
                item->SetReadOnly();
                item->SetID(ITEM_ID_TRAY_POCKET_COUNT_Y);
            }
            trayInfo->Expand();
        }

        if (auto* fovInfo = m_propertyGrid->AddCategory(_T("Field of view information")))
        {
            if (auto* item = fovInfo->AddChildItem(new CXTPPropertyGridItemNumber(
                    _T("Pane count x"), (long)m_pVisionInsp->getInspectionAreaInfo().m_pocketArrayX.size())))
            {
                item->SetReadOnly();
                item->SetID(ITEM_ID_FOV_PANE_COUNT_X);
            }
            if (auto* item = fovInfo->AddChildItem(new CXTPPropertyGridItemNumber(
                    _T("Pane count y"), (long)m_pVisionInsp->getInspectionAreaInfo().m_pocketArrayY.size())))
            {
                item->SetReadOnly();
                item->SetID(ITEM_ID_FOV_PANE_COUNT_Y);
            }
            if (auto* item = fovInfo->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Pane pitch x"), m_pVisionInsp->getTrayScanSpec().m_pocketPitchX_mm, _T("%.3lf mm"))))
            {
                item->SetReadOnly();
                item->SetID(ITEM_ID_FOV_PANE_PITCH_X);
            }
            if (auto* item = fovInfo->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Pane pitch y"), m_pVisionInsp->getTrayScanSpec().m_pocketPitchY_mm, _T("%.3lf mm"))))
            {
                item->SetReadOnly();
                item->SetID(ITEM_ID_FOV_PANE_PITCH_Y);
            }
            fovInfo
                ->AddChildItem(new CXTPPropertyGridItemDouble(_T("Pane offset x"),
                    0.001 * m_pVisionInsp->getTrayScanSpec().GetHorPaneOffset_um(), _T("%.3lf mm")))
                ->SetID(ITEM_ID_FOV_PANE_OFFSET_X);
            fovInfo
                ->AddChildItem(new CXTPPropertyGridItemDouble(_T("Pane offset y"),
                    0.001 * m_pVisionInsp->getTrayScanSpec().GetVerPaneOffset_um(), _T("%.3lf mm")))
                ->SetID(ITEM_ID_FOV_PANE_OFFSET_Y);
            fovInfo->Expand();
        }

        m_propertyGrid->SetViewDivider(0.33);
        m_propertyGrid->HighlightChangedItems(TRUE);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    UpdatePaneOverlay();

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionProcessingFOV::OnDestroy()
{
    CDialog::OnDestroy();

    m_propertyGrid->DestroyWindow();
}

LRESULT CDlgVisionProcessingFOV::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

        switch (item->GetID())
        {
            case ITEM_ID_FOV_PANE_OFFSET_X:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                {
                    m_pVisionInsp->getTrayScanSpec().SetHorPaneOffset(value->GetDouble() * 1000.);
                }
                break;
            case ITEM_ID_FOV_PANE_OFFSET_Y:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                {
                    m_pVisionInsp->getTrayScanSpec().SetVerPaneOffset(value->GetDouble() * 1000.);
                }
                break;

            case ITEM_ID_TRAY_POCKET_COUNT_X:
            case ITEM_ID_TRAY_POCKET_COUNT_Y:
            case ITEM_ID_FOV_PANE_COUNT_X:
            case ITEM_ID_FOV_PANE_COUNT_Y:
            case ITEM_ID_FOV_PANE_PITCH_X:
            case ITEM_ID_FOV_PANE_PITCH_Y:
                ASSERT(!_T("편집 불가능한 항목들이다"));
                break;
        }

        UpdatePaneOverlay();
    }

    return 0;
}

void CDlgVisionProcessingFOV::UpdatePaneOverlay()
{
    m_imageLotView->Overlay_RemoveAll();

    const auto& trayScanInfo = m_pVisionInsp->getTrayScanSpec();
    const auto& inspAreaInfo = m_pVisionInsp->getInspectionAreaInfo();

    long imageSizeX = m_pVisionInsp->getImageLot().GetImageSizeX(m_pVisionInsp->GetCurVisionModule_Status());
    long imageSizeY = m_pVisionInsp->getImageLot().GetImageSizeY(m_pVisionInsp->GetCurVisionModule_Status());

    auto& scale = m_pVisionInsp->getImageLot().GetScale(m_pVisionInsp->GetCurVisionModule_Status());

    // 현재 Stitch 경우를 고려하지 않고 Unit Region을 표시하고 있다.
    // 나중에는 이것도 고려해서 뿌려줬으면 좋겠다

    int stitchCountX = inspAreaInfo.m_stichCountX;
    int stitchCountY = inspAreaInfo.m_stichCountY;
    int fovImageSizeX = (stitchCountX <= 0) ? 0 : imageSizeX / stitchCountX;
    int fovImageSizeY = (stitchCountY <= 0) ? 0 : imageSizeY / stitchCountY;

    for (int fovIndex = 0; fovIndex < (int)inspAreaInfo.m_fovList.size(); fovIndex++)
    {
        int fovID = inspAreaInfo.m_fovList[fovIndex];

        int stitchX = fovIndex % stitchCountX;
        int stitchY = fovIndex % stitchCountY;

        Ipvm::Rect32s fovImageRoi = Ipvm::Rect32s(stitchX * fovImageSizeX, stitchY * fovImageSizeY,
            (stitchX + 1) * fovImageSizeX, (stitchY + 1) * fovImageSizeY);

        Ipvm::Point32r2 fovCenter(
            (fovImageRoi.m_left + fovImageRoi.m_right) * 0.5f, (fovImageRoi.m_top + fovImageRoi.m_bottom) * 0.5f);

        for (auto& unitIndex : inspAreaInfo.m_unitIndexList)
        {
            auto pocketRegion = scale.convert_mmToPixel(trayScanInfo.GetUnitPocketRegionInFOV(fovID, unitIndex))
                + Ipvm::Conversion::ToPoint32s2(fovCenter);
            auto unitRegion = scale.convert_mmToPixel(trayScanInfo.GetUnitRegionInFOV(fovID, unitIndex))
                + Ipvm::Conversion::ToPoint32s2(fovCenter);
            m_imageLotView->Overlay_AddRectangle(pocketRegion, RGB(0, 255, 0));
            m_imageLotView->Overlay_AddRectangle(unitRegion, RGB(0, 0, 255));
        }
    }

    m_imageLotView->Overlay_Show(TRUE);
}
