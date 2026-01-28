//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceROIEditorDlg.h"

//CPP_2_________________________________ This project's headers
#include "DlgSubThresholdAdaptive.h"
#include "DlgSubThresholdNormal.h"
#include "SurfaceCustomROI.h"
#include "SurfaceCustomROIItem.h"
#include "SurfaceDetectRectThreshDlg.h"
#include "SurfacePrepairResult.h"
#include "VisionInspectionSurface.h"
#include "VisionInspectionSurfacePara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../InformationModule/dPI_SystemIni/Config.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridSlideBox.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../UserInterfaceModules/ImageViewEx/PatternImage.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Widget/ProfileView.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
const int HistogramLevel = 256;

CSurfaceROIEditorDlg::CSurfaceROIEditorDlg(VisionInspectionSurface* pVisionInsp, BOOL bUseUserMask,
    CSurfaceItem& SurfaceItem, long nSurfaceID, long bUseVmapImage, CWnd* pParent /*=NULL*/)
    : CDialog(CSurfaceROIEditorDlg::IDD, pParent)
    , m_pSurfaceItem(new CSurfaceItem(SurfaceItem))
    , m_pAlgoPara(&m_pSurfaceItem->AlgoPara)
    , m_nSurfaceID(nSurfaceID)
    , m_imageView(nullptr)
    , m_profileView(nullptr)
    , m_is3D(false)
    , m_profileThreshold(-1)
    , m_Used3DImageType(bUseVmapImage)
{
    m_prepairResult = new SurfacePrepairResult(true);
    m_subThresholdAdaptive
        = new DlgSubThresholdAdaptive(m_pAlgoPara->nUse3DImageType, this); //SDY_3D Surface VMap UI 오류 수정
    m_subThresholdNormal
        = new DlgSubThresholdNormal(m_pAlgoPara->nUse3DImageType, this); //SDY_3D Surface VMap UI 오류 수정

    m_pVisionInsp = pVisionInsp;
    m_nCurSubArea_y = 0;
    m_nCurSubArea_x = -1;

    m_inspROIOffset_Left = m_pAlgoPara->m_offsetLeft_um;
    m_inspROIOffset_Top = m_pAlgoPara->m_offsetTop_um;
    m_inspROIOffset_Right = m_pAlgoPara->m_offsetRight_um;
    m_inspROIOffset_Bottom = m_pAlgoPara->m_offsetBottom_um;

    m_bUseUserMask = bUseUserMask;
    m_bFirstMode = TRUE;

    MakeGrayImage(false);
    UpdateMainPrepairResult(false);
}

CSurfaceROIEditorDlg::~CSurfaceROIEditorDlg()
{
    delete m_subThresholdNormal;
    delete m_subThresholdAdaptive;
    delete m_pSurfaceItem;
    delete m_imageView;
    delete m_profileView;
    delete m_prepairResult;
}

CSurfaceAlgoPara& CSurfaceROIEditorDlg::GetAlgoPara()
{
    return *m_pAlgoPara;
}

void CSurfaceROIEditorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_ProfileType, m_cmbProfileType);
    DDX_Control(pDX, IDC_LIST_CustomROI, m_lstCustomROI);
    DDX_Text(pDX, IDC_EDIT_InspROIOffset_Left, m_inspROIOffset_Left);
    DDX_Text(pDX, IDC_EDIT_InspROIOffset_Top, m_inspROIOffset_Top);
    DDX_Text(pDX, IDC_EDIT_InspROIOffset_Right, m_inspROIOffset_Right);
    DDX_Text(pDX, IDC_EDIT_InspROIOffset_Bottom, m_inspROIOffset_Bottom);
    DDX_Control(pDX, IDC_GRID_IGNORE, m_gridIgnore);
    DDX_Control(pDX, IDC_GRID_Mask, m_gridMask);
    DDX_Control(pDX, IDC_COMBO_THRESHOLD_TYPE, m_ThresholdType);
}

BEGIN_MESSAGE_MAP(CSurfaceROIEditorDlg, CDialog)
ON_BN_CLICKED(IDC_BUTTON_IGNORE_ADD, OnButtonIgnoreAdd)
ON_BN_CLICKED(IDC_BUTTON_IGNORE_SUB, OnButtonIgnoreSub)
ON_BN_CLICKED(IDC_BTN_ADD_ADD, OnBtnAddAdd)
ON_BN_CLICKED(IDC_BTN_ADD_SUB, OnBtnAddSub)
ON_BN_CLICKED(IDC_BTN_MaskOrder_UP, OnBTNMaskOrderUP)
ON_BN_CLICKED(IDC_BTN_MaskOrder_DOWN, OnBTNMaskOrderDOWN)
ON_BN_CLICKED(IDC_BTN_MaskOrder_Del, OnBTNMaskOrderDel)
ON_EN_CHANGE(IDC_EDIT_InspROIOffset_Left, OnChangeEDITInspROIOffsetLeft)
ON_EN_CHANGE(IDC_EDIT_InspROIOffset_Right, OnChangeEDITInspROIOffsetRight)
ON_EN_CHANGE(IDC_EDIT_InspROIOffset_Top, OnChangeEDITInspROIOffsetTop)
ON_EN_CHANGE(IDC_EDIT_InspROIOffset_Bottom, OnChangeEDITInspROIOffsetBottom)

ON_NOTIFY(NM_CLICK, IDC_GRID_Mask, OnGridClick)
ON_LBN_SELCHANGE(IDC_LIST_CustomROI, &CSurfaceROIEditorDlg::OnLbnSelchangeListCustomroi)

ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_Mask, OnGridMaskEditEnd)
ON_NOTIFY(NM_CLICK, IDC_GRID_Mask, OnGridModifyClick) //kircheis_Hy
ON_NOTIFY(NM_DBLCLK, IDC_GRID_Mask, OnGridModifyClick) //kircheis_Hy
ON_BN_CLICKED(IDOK, &CSurfaceROIEditorDlg::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CSurfaceROIEditorDlg::OnBnClickedCancel)
ON_BN_CLICKED(IDC_BTN_VIEW_INPUT_IMAGE, &CSurfaceROIEditorDlg::OnBnClickedBtnViewInputImage)
ON_BN_CLICKED(IDC_CHECK_COMBINE_IMAGE, &CSurfaceROIEditorDlg::OnBnClickedCheckCombineImage)
ON_CBN_SELCHANGE(IDC_COMBO_THRESHOLD_TYPE, &CSurfaceROIEditorDlg::OnCbnSelchangeComboThresholdType)
ON_CBN_SELCHANGE(IDC_COMBO_ProfileType, &CSurfaceROIEditorDlg::OnCbnSelchangeComboProfiletype)
ON_MESSAGE(UM_THRESHOLD_CHANGED, &CSurfaceROIEditorDlg::OnChangedThreshold)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSurfaceROIEditorDlg message handlers

BOOL CSurfaceROIEditorDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_cmbProfileType.AddString(_T("All"));
    m_cmbProfileType.AddString(_T("Only Dark"));
    m_cmbProfileType.AddString(_T("Only Bright"));
    m_cmbProfileType.SetCurSel(0);

    CRect profileRegion;
    GetDlgItem(IDC_STATIC_PROFILE)->GetWindowRect(profileRegion);
    ScreenToClient(profileRegion);

    m_ThresholdType.SetCurSel(GetThresholdNormal() == TRUE ? 1 : 0);

    m_profileView = new Ipvm::ProfileView(GetSafeHwnd(), Ipvm::FromMFC(profileRegion));
    m_profileView->ShowRangeControlPanel();

    CRect view;
    GetDlgItem(IDC_STATIC_IMAGE_FRAME)->GetWindowRect(view);
    ScreenToClient(view);

    m_imageView = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(view), 0, m_pVisionInsp->GetCurVisionModule_Status());
    m_imageView->RegisterCallback_ROIChangeEnd(GetSafeHwnd(), this, ROI_Changed);

    const auto& ROIInfo = m_pVisionInsp->m_surfacePara->m_SurfaceCutemRoi;

    for (int n = 0; n < ROIInfo.CustomROI_GetCount(); n++)
    {
        CString strName = ROIInfo.CustomROI_GetName(n);

        m_lstCustomROI.AddString(strName);

        strName.Empty();
    }

    m_gridIgnore.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridIgnore.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridIgnore.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridIgnore.SetDefCellMargin(0);
    m_gridIgnore.SetFixedColumnSelection(FALSE);
    m_gridIgnore.SetFixedRowSelection(FALSE);
    m_gridIgnore.SetSingleColSelection(FALSE);
    m_gridIgnore.SetSingleRowSelection(FALSE);
    m_gridIgnore.SetTrackFocusCell(FALSE);
    m_gridIgnore.SetRowResize(FALSE);
    m_gridIgnore.SetColumnResize(FALSE);

    m_gridIgnore.DeleteAllItems();
    //m_gridIgnore.SetRowCount(4);	// Items + Header
    //m_gridIgnore.SetRowCount(5);	// Items + Header//kircheis_Hy
    m_gridIgnore.SetRowCount(4); // Items + Header//영훈 20151002 : 숨긴다.
    m_gridIgnore.SetFixedRowCount(1);
    m_gridIgnore.SetColumnCount(1);
    m_gridIgnore.SetItemText(0, 0, _T("Ignore Shape"));

    CRect rtCtrl;
    m_gridIgnore.GetWindowRect(rtCtrl);
    m_gridIgnore.SetColumnWidth(0, rtCtrl.Width() - 20);

    long nID = 1;
    m_gridIgnore.SetItemText(nID, 0, _T("Rect"));
    m_gridIgnore.SetItemState(nID, 0, GVIS_READONLY);
    nID++;
    m_gridIgnore.SetItemText(nID, 0, _T("Ellipse"));
    m_gridIgnore.SetItemState(nID, 0, GVIS_READONLY);
    nID++;
    m_gridIgnore.SetItemText(nID, 0, _T("Triangle"));
    m_gridIgnore.SetItemState(nID, 0, GVIS_READONLY);
    nID++;

    m_gridMask.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridMask.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridMask.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridMask.SetDefCellMargin(0);
    m_gridMask.SetFixedColumnSelection(FALSE);
    m_gridMask.SetFixedRowSelection(FALSE);
    m_gridMask.SetSingleColSelection(FALSE);
    m_gridMask.SetSingleRowSelection(FALSE);
    m_gridMask.SetTrackFocusCell(FALSE);
    m_gridMask.SetRowResize(FALSE);
    m_gridMask.SetColumnResize(FALSE);
    m_gridMask.SetFont(GetFont(), FALSE);
    m_gridMask.SetEditable(TRUE);
    m_gridMask.EnableDragAndDrop(FALSE);

    SetROI(m_pAlgoPara->m_ignore);

    CRect thresholdRegion;
    GetDlgItem(IDC_VIEW_THRESHOLD)->GetWindowRect(thresholdRegion);
    ScreenToClient(thresholdRegion);

    m_subThresholdAdaptive->Create(IDD_SUB_THRESHOLD_ADAPTIVE, this);
    m_subThresholdNormal->Create(IDD_SUB_THRESHOLD_NORMAL, this);

    m_subThresholdAdaptive->MoveWindow(thresholdRegion);
    m_subThresholdNormal->MoveWindow(thresholdRegion);

    if (Config::getVisionType() == VISIONTYPE_3D_INSP && m_Used3DImageType == SURFACE_3D_USE_ZMAP)
    {
        m_ThresholdType.SetCurSel(0);
        m_ThresholdType.EnableWindow(FALSE);

        m_is3D = true;
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_CHECK_COMBINE_IMAGE))->EnableWindow(TRUE);
    }

    uOnUpdateThresholdScrollBar();
    uOnUpdateROISize();
    Refresh_MaskList();
    OnCbnSelchangeComboThresholdType();

    return TRUE; // return TRUE  unless you set the focus to a control
}

float& CSurfaceROIEditorDlg::GetThresholdDark()
{
    return m_pAlgoPara->GetThresholdDark(m_is3D);
}

float& CSurfaceROIEditorDlg::GetThresholdBright()
{
    return m_pAlgoPara->GetThresholdBright(m_is3D);
}

long& CSurfaceROIEditorDlg::GetThresholdLow()
{
    return m_pAlgoPara->GetThresholdLow();
}

long& CSurfaceROIEditorDlg::GetThresholdHigh()
{
    return m_pAlgoPara->GetThresholdHigh();
}

BOOL& CSurfaceROIEditorDlg::GetThresholdNormal()
{
    return m_pAlgoPara->GetThresholdNormal();
}

//////////////////////////////////////////////////////////////////////////

void CSurfaceROIEditorDlg::OnGridMaskEditEnd(NMHDR* pNotifyStruct, LRESULT* result)
{
    //	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*) pNotifyStruct;

    GetParameter();
    GetROI(m_pAlgoPara->m_ignore);

    UpdateMainPrepairResult();

    pNotifyStruct = NULL;
    *result = 0;
}

void CSurfaceROIEditorDlg::OnGridModifyClick(NMHDR* pNotifyStruct, LRESULT* result) //kircheis_Hy
{
    int iGridRow = m_gridMask.GetFocusCell().row;
    int iGridCol = m_gridMask.GetFocusCell().col;

    if (m_gridMask.GetItemText(iGridRow, iGridCol) == "Thresh. Opt")
    {
        CSurfaceDetectRectThreshDlg dlg(m_pVisionInsp->getScale(), this);
        dlg.SetMaskOrder(&m_pAlgoPara->m_maskOrder.GetAt(iGridRow - 1));

        Ipvm::Image8u image;
        if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(image))
            return;

        CString maskName = m_pAlgoPara->m_maskOrder.GetAt(iGridRow - 1).m_strName;

        Ipvm::Point32r2* pptRect_BCU = NULL;
        for (int i = 0; i < m_pAlgoPara->m_ignore.GetCount(); i++)
        {
            auto& ignoreInfo = m_pAlgoPara->m_ignore.GetAt(i);
            if (ignoreInfo.nPolygonType == 2 && ignoreInfo.strName == maskName)
            {
                pptRect_BCU = &ignoreInfo.vecptPolygon_BCU[0];
                break;
            }
        }
        if (pptRect_BCU != NULL)
        {
            dlg.SetImageData(image, pptRect_BCU);
            if (dlg.DoModal() != IDOK)
            {
                UpdateMainPrepairResult();
            }
        }

        maskName.Empty();
    }

    pNotifyStruct = NULL;
    *result = 0;
}

void CSurfaceROIEditorDlg::OnOK()
{
    GetParameter();
    GetROI(m_pAlgoPara->m_ignore);
    m_pVisionInsp->SetSurfaceItem(*m_pSurfaceItem);

    CDialog::OnOK();
}

void CSurfaceROIEditorDlg::ROI_Changed(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength)
{
    ((CSurfaceROIEditorDlg*)userData)->ROI_Changed(id, key, keyLength);
}

void CSurfaceROIEditorDlg::ROI_Changed(const int32_t id, const wchar_t* key, const size_t keyLength)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(keyLength);

    uOnUpdateROISize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void CSurfaceROIEditorDlg::uOnUpdateThresholdScrollBar()
{
    if (GetThresholdNormal())
    {
        m_subThresholdNormal->Set(GetThresholdLow(), GetThresholdHigh());
    }
    else
    {
        m_subThresholdAdaptive->Set(GetThresholdDark(), GetThresholdBright());
    }
}

void CSurfaceROIEditorDlg::UpdateMainPrepairResult(bool updateThreshold)
{
    if (!m_pVisionInsp->MakePrepairImage(*m_pSurfaceItem, *m_prepairResult))
    {
        return;
    }

    if (updateThreshold)
    {
        UpdateProfileView();
        UpdateMainThresholdImage();
    }
}

void CSurfaceROIEditorDlg::UpdateMainThresholdImage()
{
    Ipvm::Image8u maskImage;

    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(maskImage))
        return;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(m_thresholdImage))
        return;

    const auto& scale = m_pVisionInsp->getScale();

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(maskImage), 0, maskImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(m_thresholdImage), 0, m_thresholdImage);

    Ipvm::Point32r2 imageCenter(maskImage.GetSizeX() * 0.5f, maskImage.GetSizeY() * 0.5f);

    const auto validROI = scale.convert_BCUToPixel(m_pAlgoPara->m_rtROI_BCU, imageCenter);

    if (1)
    {
        Ipvm::Image8u valid_thresholdImage;

        if (!m_pVisionInsp->MakeInspImage(*m_pSurfaceItem, *m_prepairResult, valid_thresholdImage))
        {
            return;
        }

        Ipvm::ImageProcessing::Copy(m_prepairResult->m_maskImage, validROI, maskImage);
        Ipvm::ImageProcessing::Copy(valid_thresholdImage, validROI, m_thresholdImage);
    }

    if (m_bFirstMode)
    {
        m_imageView->NavigateTo(Ipvm::Rect32s(m_grayImage));
        m_bFirstMode = FALSE;
    }

    SetROI(m_pAlgoPara->m_ignore);

    BYTE backIntensity[256];
    for (long n = 0; n < 256; n++)
    {
        backIntensity[n] = BYTE(252L * n / 255 + 1);
    }

    for (long y = 0; y < m_grayImage.GetSizeY(); y++)
    {
        auto* filtered_y = m_grayImage.GetMem(0, y);
        auto* pattern_y = maskImage.GetMem(0, y);
        auto* dst_y = m_thresholdImage.GetMem(0, y);

        for (long x = 0; x < m_grayImage.GetSizeX(); x++)
        {
            if (pattern_y[x] == 0)
            {
                dst_y[x] = backIntensity[filtered_y[x]];
            }
        }
    }

    PatternImage pattern;
    if (pattern.setThresholdImage(m_thresholdImage))
    {
        m_imageView->SetImage(pattern);
    }
    else
    {
        MessageBox(_T("Not enough memory."));
    }
}

void CSurfaceROIEditorDlg::UpdateProfileView()
{
    m_profileView->Clear();

    long minIndex = 0;
    long maxIndex = long(m_prepairResult->m_profiles.size() - 1);

    if (m_cmbProfileType.GetCurSel() == 1)
    {
        // Only Dark
        maxIndex = -1;
        for (long n = 0; n < long(m_prepairResult->m_profiles.size()); n++)
        {
            if (m_prepairResult->m_profiles[n].m_x >= -GetThresholdDark())
            {
                maxIndex = n;
                break;
            }
        }
    }
    else if (m_cmbProfileType.GetCurSel() == 2)
    {
        // Only Bright
        minIndex = -1;
        for (long n = long(m_prepairResult->m_profiles.size()) - 1; n >= 0; n--)
        {
            if (m_prepairResult->m_profiles[n].m_x <= GetThresholdBright())
            {
                minIndex = n;
                break;
            }
        }
    }

    if (minIndex >= 0 && minIndex < maxIndex)
    {
        m_profileView->SetData(0, &m_prepairResult->m_profiles[minIndex], maxIndex - minIndex + 1,
            Ipvm::k_noiseValue32r, RGB(0, 255, 0), RGB(0, 255, 0));

        switch (m_profileThreshold)
        {
            case 0:
                m_profileView->SetCursorPosition(-GetThresholdDark());
                break;
            case 1:
                m_profileView->SetCursorPosition(+GetThresholdBright());
                break;
        }
    }
}

void CSurfaceROIEditorDlg::uOnUpdateROISize()
{
    HCURSOR oldCursorHandle = SetCursor(LoadCursor(NULL, IDC_WAIT));

    GetROI(m_pAlgoPara->m_ignore);

    UpdateMainPrepairResult();

    SetCursor(oldCursorHandle);
}

long CSurfaceROIEditorDlg::GetSelectedIgnoreType()
{
    CCellRange CellRange = m_gridIgnore.GetSelectedCellRange();

    if (!CellRange.IsValid())
        return -1;

    if (CellRange.Count() != 1)
        return -1;

    long nSelItem = CellRange.GetTopLeft().row - 1;

    return nSelItem;
}

long CSurfaceROIEditorDlg::GetSelectedMask()
{
    CCellRange CellRange = m_gridMask.GetSelectedCellRange();

    if (!CellRange.IsValid())
        return -1;

    if (CellRange.GetMinRow() != CellRange.GetMaxRow())
        return -1;

    long nSelItem = CellRange.GetTopLeft().row - 1;

    return nSelItem;
}

void CSurfaceROIEditorDlg::GetROI(Para_PolygonRects& ignorePara)
{
    const auto& scale = m_pVisionInsp->getScale();

    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    Ipvm::Rect32s rtROI;
    m_imageView->ROIGet(_T("ROI"), rtROI);

    for (long index = 0; index < ignorePara.GetCount(); index++)
    {
        //CString key;
        //key.Format(_T("I%d"), index);

        auto& ignoreInfo = ignorePara.GetAt(index);
        if (ignoreInfo.nPolygonType >= 3)
        {
            Ipvm::Polygon32r polygon;
            m_imageView->ROIGet(ignoreInfo.strROIKey, polygon);

            std::vector<Ipvm::Point32s2> points;
            ImageViewEx::PolygonToPoints(polygon, points);

            ignoreInfo.vecptPolygon_BCU = scale.convert_PixelToBCU(points, imageCenter);
        }
        else
        {
            m_imageView->ROIGet(ignoreInfo.strROIKey, rtROI);
            ignoreInfo.vecptPolygon_BCU[0] = scale.convert_PixelToBCU(rtROI.TopLeft(), imageCenter);
            ignoreInfo.vecptPolygon_BCU[1] = scale.convert_PixelToBCU(rtROI.BottomRight(), imageCenter);
        }
        //key.Empty();
    }
}

void CSurfaceROIEditorDlg::SetROI(const Para_PolygonRects& ignorePara)
{
    const auto& scale = m_pVisionInsp->getScale();

    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    m_imageView->ROIClear();

    Ipvm::Rect32s imageRoi = Ipvm::Conversion::ToRect32s(
        scale.convert_PixelToBCU(Ipvm::Conversion::ToRect32s(m_pAlgoPara->m_rtROI_BCU), imageCenter));
    m_imageView->ROISet(_T("ROI"), _T("ROI"), imageRoi, RGB(255, 0, 255), FALSE, FALSE);

    for (long index = 0; index < ignorePara.GetCount(); index++)
    {
        CString key;
        key.Format(_T("I%d"), index);

        m_pAlgoPara->m_ignore.GetAt(index).strROIKey = key;
        auto& ignoreInfo = ignorePara.GetAt(index);

        if (ignoreInfo.nPolygonType >= 3)
        {
            Ipvm::Polygon32r polygon;
            polygon.SetVertexNum((long)ignoreInfo.vecptPolygon_BCU.size());
            for (long n = 0; n < (long)ignoreInfo.vecptPolygon_BCU.size(); n++)
            {
                polygon.GetVertices()[n] = Ipvm::Conversion::ToPoint32r2(
                    scale.convert_BCUToPixel(ignoreInfo.vecptPolygon_BCU[n], imageCenter));
            }

            m_imageView->ROISet(ignoreInfo.strROIKey, ignoreInfo.strName, polygon, RGB(255, 128, 0), TRUE, TRUE);
        }
        else
        {
            Ipvm::Rect32s rtROI = scale.convert_BCUToPixel(
                Ipvm::Rect32r(ignoreInfo.vecptPolygon_BCU[0].m_x, ignoreInfo.vecptPolygon_BCU[0].m_y,
                    ignoreInfo.vecptPolygon_BCU[1].m_x, ignoreInfo.vecptPolygon_BCU[1].m_y),
                imageCenter);

            m_imageView->ROISet(ignoreInfo.strROIKey, ignoreInfo.strName, rtROI, RGB(255, 128, 0), TRUE, TRUE);
        }
        key.Empty();
    }

    m_imageView->ROIShow();
}

void CSurfaceROIEditorDlg::GetParameter()
{
    //==============================================================================
    // 수정된 Grid Ctrl 의 이름 정보에 따라 Ignore Name 정보를 갱신하고
    // 이를 사용하는 MaskOrder 의 이름도 같이 꾸어 준다
    //==============================================================================

    long ignoreIndex = 0;
    auto& maskOrder = m_pAlgoPara->m_maskOrder;

    const auto& scale = m_pVisionInsp->getScale();
    float px2um = (scale.pixelToUm().m_x + scale.pixelToUm().m_y) * 0.5f;

    for (int n = 1; n < m_gridMask.GetRowCount(); n++)
    {
        auto& maskInfo = maskOrder.GetAt(n - 1);
        if (m_gridMask.GetItemText(n, 1) == _T(" "))
        {
            m_pAlgoPara->m_ignore.GetAt(ignoreIndex).strName = m_gridMask.GetItemText(n, 2);
            maskInfo.m_strName = m_pAlgoPara->m_ignore.GetAt(ignoreIndex).strName;
            ignoreIndex++;
        }

        // Offset 정보를 수정
        maskInfo.m_offset_um.m_x = (float)scale.convert_pixelToUmX(_ttof(m_gridMask.GetItemText(n, 3)));
        maskInfo.m_offset_um.m_y = (float)scale.convert_pixelToUmY(_ttof(m_gridMask.GetItemText(n, 4)));

        int n2, n3;
        for (n2 = 5, n3 = 0; n2 < m_gridMask.GetColumnCount(); n2 += 2, n3++)
        {
            if ((long)(maskInfo.m_pointOffsets_um.size()) > n3)
            {
                maskInfo.m_pointOffsets_um[n3].m_x
                    = (float)scale.convert_pixelToUmX(_ttof(m_gridMask.GetItemText(n, n2)));
                maskInfo.m_pointOffsets_um[n3].m_y
                    = (float)scale.convert_pixelToUmY(_ttof(m_gridMask.GetItemText(n, n2 + 1)));
            }
        }

        //{{ Detect Rect Auto Thresh  //kircheis_Hy
        if (maskInfo.m_strName == "Auto Thresh")
        {
            //		maskInfo.bUseAutoThreshDR = m_gridMask.GetItem(;
        }
        //}}

        if (maskInfo.m_strName == "Underfill Fillet Mask Image"
            || m_pVisionInsp->getReusableMemory().GetSurfaceLayerMask(maskInfo.m_strName) != nullptr
            || m_pVisionInsp->getReusableMemory().GetSurfaceLayerRoiClass(maskInfo.m_strName) != nullptr)
        {
            maskInfo.m_dilateInUm = CAST_FLOAT(_ttof(m_gridMask.GetItemText(n, n2 - 2)) * px2um);
        }
    }
}

void CSurfaceROIEditorDlg::OnBtnAddAdd()
{
    int nSelROI = m_lstCustomROI.GetCurSel();

    if (nSelROI >= 0)
    {
        Para_MaskOrder::MaskInfo data;
        CString strROIName;
        m_lstCustomROI.GetText(nSelROI, strROIName);

        data.m_validate = TRUE;
        data.m_strName = strROIName;
        data.m_makeType = 0;
        data.m_offset_um = Ipvm::Point32r2(0.f, 0.f);
        data.m_dilateInUm = 0.f;
        data.m_useBodyEdge = FALSE;

        //{{ //kircheis_Hy
        data.m_useAutoThreshDR = FALSE;
        data.m_threshDRX = 110;
        data.m_threshDRY = 110;
        //}}

        const auto& roiInfos = m_pVisionInsp->m_surfacePara->m_SurfaceCutemRoi;
        const auto* roiInfo = roiInfos.CustomROI_Search(strROIName);
        if (roiInfo)
        {
            int nType = roiInfo->nType;

            switch (nType)
            {
                case 0:
                    data.m_pointOffsets_um.resize(2, Ipvm::Point32r2(0.f, 0.f));
                    break;
                case 1:
                    data.m_pointOffsets_um.resize(2, Ipvm::Point32r2(0.f, 0.f));
                    break;
                default:
                    data.m_pointOffsets_um.resize(nType, Ipvm::Point32r2(0.f, 0.f));
                    break;
            }

            m_pAlgoPara->m_maskOrder.Add(data);
            Refresh_MaskList();
        }
        strROIName.Empty();
    }
}

void CSurfaceROIEditorDlg::OnBtnAddSub()
{
    int nSelROI = m_lstCustomROI.GetCurSel();

    if (nSelROI >= 0)
    {
        Para_MaskOrder::MaskInfo data;
        CString strROIName;
        m_lstCustomROI.GetText(nSelROI, strROIName);

        data.m_validate = FALSE;
        data.m_strName = strROIName;
        data.m_makeType = 0;
        data.m_offset_um = Ipvm::Point32r2(0.f, 0.f);
        data.m_dilateInUm = 0.f;
        data.m_useBodyEdge = FALSE;

        //{{ //kircheis_Hy
        data.m_useAutoThreshDR = FALSE;
        data.m_threshDRX = 110;
        data.m_threshDRY = 110;
        //}}

        const auto& roiInfos = m_pVisionInsp->m_surfacePara->m_SurfaceCutemRoi;
        const auto* roiInfo = roiInfos.CustomROI_Search(strROIName);
        if (roiInfo)
        {
            int nType = roiInfo->nType;

            switch (nType)
            {
                case 0:
                    data.m_pointOffsets_um.resize(2, Ipvm::Point32r2(0.f, 0.f));
                    break;
                case 1:
                    data.m_pointOffsets_um.resize(2, Ipvm::Point32r2(0.f, 0.f));
                    break;
                default:
                    if (nType >= 3)
                        data.m_pointOffsets_um.resize(nType, Ipvm::Point32r2(0.f, 0.f));
                    break;
            }

            m_pAlgoPara->m_maskOrder.Add(data);
            Refresh_MaskList();
        }
        strROIName.Empty();
    }
}

void CSurfaceROIEditorDlg::OnButtonIgnoreAdd()
{
    CString strName;
    int nPoint = AddIgnoreData(strName);

    // 영훈 20151002 : 작동되지 않는 문법이다.
    //int nPoint;
    //if(nPoint == AddIgnoreData(strName))
    if (nPoint > 0)
    {
        Para_MaskOrder::MaskInfo data;

        data.m_validate = TRUE;
        data.m_strName = strName;
        data.m_makeType = 1;
        data.m_offset_um = Ipvm::Point32r2(0.f, 0.f);
        data.m_dilateInUm = 0.f;
        data.m_useBodyEdge = FALSE;

        //{{ //kircheis_Hy
        data.m_useAutoThreshDR = FALSE;
        data.m_threshDRX = 110;
        data.m_threshDRY = 110;
        //}}
        data.m_pointOffsets_um.resize(nPoint, Ipvm::Point32r2(0.f, 0.f));

        m_pAlgoPara->m_maskOrder.Add(data);
        Refresh_MaskList();
    }

    strName.Empty();
}

void CSurfaceROIEditorDlg::OnButtonIgnoreSub()
{
    CString strName;
    int nPoint = AddIgnoreData(strName);

    if (nPoint > 0)
    {
        Para_MaskOrder::MaskInfo data;

        data.m_validate = FALSE;
        data.m_strName = strName;
        data.m_makeType = 1;
        data.m_offset_um = Ipvm::Point32r2(0.f, 0.f);
        data.m_dilateInUm = 0.f;
        data.m_useBodyEdge = FALSE;

        //{{ //kircheis_Hy
        data.m_useAutoThreshDR = FALSE;
        data.m_threshDRX = 110;
        data.m_threshDRY = 110;
        //}}
        data.m_pointOffsets_um.resize(nPoint, Ipvm::Point32r2(0.f, 0.f));

        m_pAlgoPara->m_maskOrder.Add(data);
        Refresh_MaskList();
    }
    strName.Empty();
}

void CSurfaceROIEditorDlg::Refresh_MaskList()
{
    UpdateData(TRUE);

    int nItemNum = m_pAlgoPara->m_maskOrder.GetCount();
    int nOldSelect = GetSelectedMask();

    // Polygon 최대수 를 얻음
    int nPolygon = 0;
    BOOL bUserMorphologic = FALSE;
    BOOL bUseDetectRect = FALSE; //Detect Rect가 있는지 확인 //kircheis_Hy
    BOOL bUseRoundPadMeasured = FALSE;
    for (int n = 0; n < nItemNum; n++)
    {
        const auto& maskInfo = m_pAlgoPara->m_maskOrder.GetAt(n);

        if (nPolygon < (long)(maskInfo.m_pointOffsets_um.size()))
        {
            nPolygon = (long)maskInfo.m_pointOffsets_um.size();
        }

        if (maskInfo.m_strName == _T("Underfill Fillet Mask Image"))
        {
            bUserMorphologic = TRUE;
        }
        else if (m_pVisionInsp->getReusableMemory().GetSurfaceLayerMask(maskInfo.m_strName) != nullptr
            || m_pVisionInsp->getReusableMemory().GetSurfaceLayerRoiClass(maskInfo.m_strName) != nullptr)
        {
            bUserMorphologic = TRUE;
        }
    }

    for (int n = 0; n < m_pAlgoPara->m_ignore.GetCount(); n++) //kircheis_Hy
    {
        if (m_pAlgoPara->m_ignore.GetAt(n).nPolygonType == 2)
        {
            bUseDetectRect = TRUE;
        }
    }

    m_gridMask.DeleteAllItems();
    m_gridMask.SetRowCount(nItemNum + 1); // Items + Header
    m_gridMask.SetFixedRowCount(1);
    if (bUseDetectRect || bUserMorphologic) //kircheis_Hy
    {
        // "Mark Measured" SurfaceBitmapMask가 있으면 "Dilate" 칸을 하나더 삽입한다. 권오석
        // 영훈 [RoundPad] 20130821 : Round Pad Mask가 있으면 칸을 하나 더 삽입한다.
        int nInsert = (bUseDetectRect ? 1 : 0) + (bUseRoundPadMeasured ? 1 : 0) + (bUserMorphologic ? 1 : 0);

        m_gridMask.SetColumnCount(5 + nPolygon * 2 + nInsert);
    }
    else
    {
        m_gridMask.SetColumnCount(5 + nPolygon * 2);
    }

    m_gridMask.SetItemText(0, 0, _T("+/-"));
    m_gridMask.SetItemText(0, 1, _T("T"));
    m_gridMask.SetItemText(0, 2, _T("Name"));
    m_gridMask.SetItemText(0, 3, _T("O-X"));
    m_gridMask.SetItemText(0, 4, _T("O-Y"));

    m_gridMask.SetColumnWidth(0, 30);
    m_gridMask.SetColumnWidth(1, 20);
    m_gridMask.SetColumnWidth(2, 170);
    m_gridMask.SetColumnWidth(3, 50);
    m_gridMask.SetColumnWidth(4, 50);

    for (int n = 0; n < nPolygon; n++)
    {
        CString strTitle1;
        CString strTitle2;
        strTitle1.Format(_T("P%dX"), n + 1);
        strTitle2.Format(_T("P%dY"), n + 1);
        m_gridMask.SetItemText(0, 5 + n * 2, strTitle1);
        m_gridMask.SetItemText(0, 5 + n * 2 + 1, strTitle2);
        m_gridMask.SetColumnWidth(5 + n * 2, 50);
        m_gridMask.SetColumnWidth(5 + n * 2 + 1, 50);
        strTitle1.Empty();
        strTitle2.Empty();
    }

    long nColumnTotalCount = m_gridMask.GetColumnCount();

    // 영훈 20130821 : 아따 보기 힘들다 Surface ROI Editor의 Grid는 이렇게 바꾸자.
    long nColNum = 5 + nPolygon * 2;
    if (bUserMorphologic)
    {
        m_gridMask.SetItemText(0, nColNum, _T("Dilate"));
        m_gridMask.SetColumnWidth(nColNum, 50);
        nColNum++;

        // "Mark Measured" SurfaceBitmapMask가 있으면 "Dilate" 삽입. 권오석
        if (bUseRoundPadMeasured)
        {
            m_gridMask.SetItemText(0, nColNum, _T("Use BodyEdge"));
            m_gridMask.SetColumnWidth(nColNum, 100);
            nColNum++;
        }
    }
    if (bUseDetectRect) //kircheis_Hy
    {
        m_gridMask.SetItemText(0, nColNum, _T("Thresh. Opt"));
        m_gridMask.SetColumnWidth(nColNum, 90);
    }

    CString strPlus;
    CString strTarget;
    CString strOffsetX;
    CString strOffsetY;

    const auto& scale = m_pVisionInsp->getScale();

    for (int n = 0; n < nItemNum; n++)
    {
        const auto& maskInfo = m_pAlgoPara->m_maskOrder.GetAt(n);

        strPlus.Format(_T("%c"), maskInfo.m_validate ? _T('+') : _T('-'));
        strTarget.Format(_T("%c"), maskInfo.m_makeType ? _T(' ') : _T('T'));

        // 설정하기 편하게 설정은 Pixel 단위로 한다
        strOffsetX.Format(_T("%.1f"), scale.convert_umToPixelX(maskInfo.m_offset_um.m_x));
        strOffsetY.Format(_T("%.1f"), scale.convert_umToPixelY(maskInfo.m_offset_um.m_y));

        m_gridMask.SetItemText(n + 1, 0, strPlus);
        m_gridMask.SetItemText(n + 1, 1, strTarget);
        m_gridMask.SetItemText(n + 1, 2, maskInfo.m_strName);

        BOOL isImageTypeMask = FALSE;

        if (maskInfo.m_makeType == 0)
        {
            const auto& roiInfos = m_pVisionInsp->m_surfacePara->m_SurfaceCutemRoi;
            const auto* roiInfo = roiInfos.CustomROI_Search(maskInfo.m_strName);
            if (roiInfo == nullptr)
                continue;

            int nType = roiInfo->nType;

            if (nType < 0)
                continue;

            if (nType == 2 && maskInfo.m_strName != _T("Mark Measured"))
            {
                isImageTypeMask = TRUE;
            }
        }

        if (isImageTypeMask)
        {
            m_gridMask.SetItemText(n + 1, 3, strOffsetX);
            m_gridMask.SetItemText(n + 1, 4, strOffsetY);
        }
        else
        {
            m_gridMask.SetItemState(n + 1, 3, GVIS_READONLY);
            m_gridMask.SetItemState(n + 1, 4, GVIS_READONLY);
            m_gridMask.SetItemBkColour(n + 1, 3, RGB(128, 128, 128));
            m_gridMask.SetItemBkColour(n + 1, 4, RGB(128, 128, 128));
        }

        m_gridMask.SetItemState(n + 1, 0, GVIS_READONLY);
        m_gridMask.SetItemState(n + 1, 1, GVIS_READONLY);

        if (maskInfo.m_makeType == 0)
        {
            m_gridMask.SetItemState(n + 1, 2, GVIS_READONLY);
        }

        const auto& ptOffset_um = maskInfo.m_pointOffsets_um;

        int n2 = 0;
        nColNum = 5 + n2 * 2;
        for (n2 = 0; n2 < nPolygon; n2++)
        {
            // 영훈 20130821 : 아따 보기 힘들다 Surface ROI Editor의 Grid는 이렇게 바꾸자.
            nColNum = 5 + n2 * 2;
            if (isImageTypeMask || (long)(ptOffset_um.size()) < n2 + 1)
            {
                m_gridMask.SetItemBkColour(n + 1, nColNum, RGB(128, 128, 128));
                m_gridMask.SetItemBkColour(n + 1, nColNum + 1, RGB(128, 128, 128));
                m_gridMask.SetItemState(n + 1, nColNum, GVIS_READONLY);
                m_gridMask.SetItemState(n + 1, nColNum + 1, GVIS_READONLY);
            }
            else
            {
                // 설정하기 편하게 설정은 Pixel 단위로 한다
                CString textX;
                CString textY;
                textX.Format(_T("%.1f"), scale.convert_umToPixelX(ptOffset_um[n2].m_x));
                textY.Format(_T("%.1f"), scale.convert_umToPixelY(ptOffset_um[n2].m_y));
                m_gridMask.SetItemText(n + 1, nColNum, textX);
                m_gridMask.SetItemText(n + 1, nColNum + 1, textY);

                textX.Empty();
                textY.Empty();
            }
        }
        nColNum = 5 + n2 * 2;

        // Dilate 칸 상태 설정 할 것. 권오석
        // "Mark Measured"는 매 Inspection마다 새로 만드는 Mask
        // 아직은 이런 타입의 Mask가 없고, 앞으로도 이런 종류는 없을것 같아서
        // 따로 타입을 정해두지 않았음.
        // 혹시라도 비슷한 종류의 Mask가 새로 생기믄 타입을 나누어 관리할 것.
        // 지금은 일단 이름으로 구별
        BOOL bUsebodyEdge = FALSE;

        if (bUserMorphologic)
        {
            if (bUserMorphologic)
            {
                // 설정하기 편하게 설정은 Pixel 단위로 한다
                float um2px = scale.umToPixelXY();

                m_gridMask.SetItemTextFmt(n + 1, nColNum, _T("%.1f"), maskInfo.m_dilateInUm * um2px);
            }
            else //BUG: else 로 들어올 일이 없음
            {
                m_gridMask.SetItemBkColour(n + 1, nColNum, RGB(128, 128, 128));
                m_gridMask.SetItemState(n + 1, nColNum, GVIS_READONLY);
            }

            nColNum++;
        }

        // 영훈 : QFN의 경우 Pad가 Body Edge에 붙어있으나 가끔 떨어진 녀석이 있어서 강제로 Body Edge 까지 mask를 확장할지 말지 결정하는 옵션을 주도록 한다.
        if (bUsebodyEdge)
        {
            if (nColumnTotalCount > nColNum)
            {
                // Slide Box...
                m_gridMask.SetCellType(n + 1, nColNum, RUNTIME_CLASS(CGridSlideBox));
                ((CGridSlideBox*)m_gridMask.GetCell(n + 1, nColNum))->AddString(_T("Not Use"));
                ((CGridSlideBox*)m_gridMask.GetCell(n + 1, nColNum))->AddString(_T("Use"));
                ((CGridSlideBox*)m_gridMask.GetCell(n + 1, nColNum))->SetCurSel(maskInfo.m_useBodyEdge);
            }
            else
            {
                m_gridMask.SetItemBkColour(n + 1, nColNum, RGB(128, 128, 128));
                m_gridMask.SetItemState(n + 1, nColNum, GVIS_READONLY);
            }

            nColNum++;
        }

        // Surface Thresh Option 기능을 사용할 경우
        if (bUseDetectRect)
        {
            if (nColumnTotalCount > nColNum) //kircheis_Hy
            {
                bool bDetectRectBtn = m_pAlgoPara->m_ignore.IsValid(maskInfo.m_strName);

                if (!bDetectRectBtn)
                {
                    m_gridMask.SetItemBkColour(n + 1, nColNum, RGB(128, 128, 128));
                    m_gridMask.SetItemState(n + 1, nColNum, GVIS_READONLY);
                }
                else
                {
                    m_gridMask.SetItemState(n + 1, nColNum, GVIS_READONLY);
                    m_gridMask.SetItemText(n + 1, nColNum, _T("Thresh. Opt"));
                    m_gridMask.SetItemBkColour(n + 1, nColNum, RGB(0xB0, 0xB0, 0xFF));
                    m_gridMask.GetCell(n + 1, nColNum)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
            }
            else
            {
                m_gridMask.SetItemBkColour(n + 1, nColNum, RGB(128, 128, 128));
                m_gridMask.SetItemState(n + 1, nColNum, GVIS_READONLY);
            }

            nColNum++;
        }

        if (nColumnTotalCount > nColNum)
        {
            m_gridMask.SetItemBkColour(n + 1, nColNum, RGB(128, 128, 128));
            m_gridMask.SetItemState(n + 1, nColNum, GVIS_READONLY);
        }
        ////////////////////////////////////
    }

    strPlus.Empty();
    strTarget.Empty();
    strOffsetX.Empty();
    strOffsetY.Empty();

    m_gridMask.SetSelectedRange(nOldSelect + 1, 0, nOldSelect + 1, 2);
    m_gridMask.Refresh();

    UpdateMainPrepairResult();
}

int CSurfaceROIEditorDlg::AddIgnoreData(CString& o_NewName)
{
    auto visionDebugInfos = m_pVisionInsp->m_visionUnit.GetVisionDebugInfos();

    int nSelIgnore = GetSelectedIgnoreType();

    if (nSelIgnore >= 0)
    {
        VisionAlignResult sEdgeAlignResult;
        VisionDebugInfo* pDebugInfo = NULL;
        for (long i = 0; i < (long)(visionDebugInfos.size()); i++)
        {
            pDebugInfo = visionDebugInfos[i];
            if (pDebugInfo->pData == NULL)
                continue;

            if (visionDebugInfos[i]->strDebugInfoName == _T("EDGE Align Result"))
            {
                sEdgeAlignResult = *(VisionAlignResult*)pDebugInfo->pData;
            }
        }

        int nCheck = 0;

        while (1)
        {
            // 영훈 20130910 : 앞으로 polygon은 Temp가 아니라 자신의 이름이 남도록 한다.
            if (nSelIgnore == enum_Rectangle)
                o_NewName.Format(_T("Rectangle %d"), nCheck + 1);
            else if (nSelIgnore == enum_Ellipse)
                o_NewName.Format(_T("Ellipse %d"), nCheck + 1);
            else if (nSelIgnore == enum_Polygon)
                o_NewName.Format(_T("Triangle %d"), nCheck + 1);
            else
                o_NewName.Format(_T("Temp %d"), nCheck + 1);

            long findIndex = m_pAlgoPara->m_maskOrder.FindIndex(o_NewName);
            if (findIndex < 0)
                break;

            nCheck++;
        }

        Para_PolygonRects::SPolygonRectNamePair data;

        int nPoint = 0;
        switch (nSelIgnore)
        {
            case enum_Rectangle:
                data.nPolygonType = 0;
                nPoint = 2;
                break;
            case enum_Ellipse:
                data.nPolygonType = 1;
                nPoint = 2;
                break;
            case enum_Detect_Rectangle:
                data.nPolygonType = 2;
                nPoint = 2;
                break; //kircheis_Hy
            default:
                data.nPolygonType = (short)(nSelIgnore + 1);
                nPoint += nSelIgnore + 1;
                break;
        }
        data.strName = o_NewName;
        data.vecptPolygon_BCU.resize(nPoint);

        if (data.strName.Find(_T("Triangle")) >= 0)
        {
            float fPackageSizeX = ((sEdgeAlignResult.fptRT.m_x - sEdgeAlignResult.fptLT.m_x)
                                      + (sEdgeAlignResult.fptRB.m_x - sEdgeAlignResult.fptLB.m_x))
                * 0.5f;
            float fPackageSizeY = ((sEdgeAlignResult.fptLB.m_y - sEdgeAlignResult.fptLT.m_y)
                                      + (sEdgeAlignResult.fptRB.m_y - sEdgeAlignResult.fptRT.m_y))
                * 0.5f;

            long nOffsetX = (long)(fPackageSizeX * 0.6f + 0.5f);
            long nOffsetY = (long)(fPackageSizeY * 0.6f + 0.5f);

            long nOffsetX_Half = (long)(nOffsetX * 0.5f + 0.5f);
            long nOffsetY_Half = (long)(nOffsetX * 0.5f + 0.5f);

            float roi_sizeX = ((nCheck / 4) * 10.f);
            float roi_sizeY = ((nCheck / 4) * 10.f);

            if (nCheck % 4 == 0)
            {
                data.vecptPolygon_BCU[0] = Ipvm::Point32r2(roi_sizeX - nOffsetX, roi_sizeY - nOffsetY);
                data.vecptPolygon_BCU[1] = Ipvm::Point32r2(roi_sizeX - nOffsetX, roi_sizeY - nOffsetY_Half);
                data.vecptPolygon_BCU[2] = Ipvm::Point32r2(roi_sizeX - nOffsetX_Half, roi_sizeY - nOffsetY);
            }
            else if (nCheck % 4 == 1)
            {
                data.vecptPolygon_BCU[0] = Ipvm::Point32r2(roi_sizeX + nOffsetX, roi_sizeY - nOffsetY);
                data.vecptPolygon_BCU[1] = Ipvm::Point32r2(roi_sizeX + nOffsetX, roi_sizeY - nOffsetY_Half);
                data.vecptPolygon_BCU[2] = Ipvm::Point32r2(roi_sizeX + nOffsetX_Half, roi_sizeY - nOffsetY);
            }
            else if (nCheck % 4 == 2)
            {
                data.vecptPolygon_BCU[0] = Ipvm::Point32r2(roi_sizeX + nOffsetX, roi_sizeY + nOffsetY);
                data.vecptPolygon_BCU[1] = Ipvm::Point32r2(roi_sizeX + nOffsetX, roi_sizeY + nOffsetY_Half);
                data.vecptPolygon_BCU[2] = Ipvm::Point32r2(roi_sizeX + nOffsetX_Half, roi_sizeY + nOffsetY);
            }
            else if (nCheck % 4 == 3)
            {
                data.vecptPolygon_BCU[0] = Ipvm::Point32r2(roi_sizeX - nOffsetX, roi_sizeY + nOffsetY);
                data.vecptPolygon_BCU[1] = Ipvm::Point32r2(roi_sizeX - nOffsetX, roi_sizeY + nOffsetY_Half);
                data.vecptPolygon_BCU[2] = Ipvm::Point32r2(roi_sizeX - nOffsetX_Half, roi_sizeY + nOffsetY);
            }
        }
        else
        {
            //Package의 Size를 가져옴. Body Align을 사용해 Pxl 단위이다
            float fPackageSizeX_pxl = ((sEdgeAlignResult.fptRT.m_x - sEdgeAlignResult.fptLT.m_x)
                                          + (sEdgeAlignResult.fptRB.m_x - sEdgeAlignResult.fptLB.m_x))
                * 0.5f;
            float fPackageSizeY_pxl = ((sEdgeAlignResult.fptLB.m_y - sEdgeAlignResult.fptLT.m_y)
                                          + (sEdgeAlignResult.fptRB.m_y - sEdgeAlignResult.fptRT.m_y))
                * 0.5f;

            static const long nMinSize_um = 1000; //가장 작은 크기는 1000um이다.

            const auto& scale = m_pVisionInsp->getScale();
            //Package Size의 1/5(1/10 + 1/10) 과 1000um 중에 큰 값을 쓴다. fPackageSizeX,Y는 Pxl 단위이므로 um 단위로의 변환이 필요하다.
            long nApplySizeX_um = (long)max(nMinSize_um, (long)(fPackageSizeX_pxl * scale.pixelToUm().m_x * .1f + .5f));
            long nApplySizeY_um = (long)max(nMinSize_um, (long)(fPackageSizeY_pxl * scale.pixelToUm().m_y * .1f + .5f));

            if (nPoint == 2) //단, Rectangle이나 Ellipse 일때만
            {
                data.vecptPolygon_BCU[0] = Ipvm::Point32r2(CAST_FLOAT(-nApplySizeX_um), CAST_FLOAT(-nApplySizeY_um));
                data.vecptPolygon_BCU[1] = Ipvm::Point32r2(CAST_FLOAT(nApplySizeX_um), CAST_FLOAT(nApplySizeY_um));
            }
            else //딴 넘들은 원래 방식으로...
            {
                for (int nP = 0; nP < nPoint; nP++)
                {
                    float roi_sizeX = 100.f;
                    float roi_sizeY = 100.f;

                    if (nP == 2)
                    {
                        roi_sizeX = -50;
                        roi_sizeY = 50;
                    }

                    data.vecptPolygon_BCU[nP] = Ipvm::Point32r2(roi_sizeX * nP - 50, roi_sizeY * nP - 50);
                }
            }
        }

        m_pAlgoPara->m_ignore.Add(data);

        SetROI(m_pAlgoPara->m_ignore);

        return nPoint;
    }

    return 0;
}

void CSurfaceROIEditorDlg::OnBTNMaskOrderUP()
{
    int nSelMask = GetSelectedMask();

    if (nSelMask > 0)
    {
        auto sMaskCurrent = m_pAlgoPara->m_maskOrder.GetAt(nSelMask);

        CString strCurrentName = m_pAlgoPara->m_maskOrder.GetAt(nSelMask).m_strName;
        CString strBufferName = m_pAlgoPara->m_maskOrder.GetAt(nSelMask - 1).m_strName;

        m_pAlgoPara->m_maskOrder.GetAt(nSelMask) = m_pAlgoPara->m_maskOrder.GetAt(nSelMask - 1);
        m_pAlgoPara->m_maskOrder.GetAt(nSelMask - 1) = sMaskCurrent;
        Refresh_MaskList();

        // 영훈 [ 20130821 ] : 정말 바보같은 버그다. Gird List와 Polygon의 List는 항상 동기화 시켜줘야한다. 원래는 이름만 바꾸고 Type과 Position을 바꾸지 않아 이상한 Mask를 그린다.
        long nCurrentID = m_pAlgoPara->m_ignore.FindIndex(strCurrentName);
        long nBackupID = m_pAlgoPara->m_ignore.FindIndex(strBufferName);

        if (nCurrentID >= 0 && nBackupID >= 0)
        {
            auto sPolygonFBuffer = m_pAlgoPara->m_ignore.GetAt(nCurrentID);
            m_pAlgoPara->m_ignore.GetAt(nCurrentID) = m_pAlgoPara->m_ignore.GetAt(nBackupID);
            m_pAlgoPara->m_ignore.GetAt(nBackupID) = sPolygonFBuffer;
        }

        m_gridMask.SetSelectedRange(nSelMask, 0, nSelMask, 2);
        m_gridMask.Refresh();

        strCurrentName.Empty();
        strBufferName.Empty();
    }
}

void CSurfaceROIEditorDlg::OnBTNMaskOrderDOWN()
{
    int nSelMask = GetSelectedMask();

    if (nSelMask >= 0 && nSelMask < m_gridMask.GetRowCount() - 2)
    {
        auto sMaskCurrent = m_pAlgoPara->m_maskOrder.GetAt(nSelMask);

        CString strCurrentName = m_pAlgoPara->m_maskOrder.GetAt(nSelMask).m_strName;
        CString strBufferName = m_pAlgoPara->m_maskOrder.GetAt(nSelMask + 1).m_strName;

        m_pAlgoPara->m_maskOrder.GetAt(nSelMask) = m_pAlgoPara->m_maskOrder.GetAt(nSelMask + 1);
        m_pAlgoPara->m_maskOrder.GetAt(nSelMask + 1) = sMaskCurrent;
        Refresh_MaskList();

        long nCurrentID = m_pAlgoPara->m_ignore.FindIndex(strCurrentName);
        long nBackupID = m_pAlgoPara->m_ignore.FindIndex(strBufferName);

        if (nCurrentID >= 0 && nBackupID >= 0)
        {
            auto sPolygonFBuffer = m_pAlgoPara->m_ignore.GetAt(nCurrentID);
            m_pAlgoPara->m_ignore.GetAt(nCurrentID) = m_pAlgoPara->m_ignore.GetAt(nBackupID);
            m_pAlgoPara->m_ignore.GetAt(nBackupID) = sPolygonFBuffer;
        }

        m_gridMask.SetSelectedRange(nSelMask + 2, 0, nSelMask + 2, 2);
        m_gridMask.Refresh();

        strCurrentName.Empty();
        strBufferName.Empty();
    }
}

void CSurfaceROIEditorDlg::OnBTNMaskOrderDel()
{
    int nSelMask = GetSelectedMask();

    if (nSelMask >= 0)
    {
        if (m_pAlgoPara->m_maskOrder.GetAt(nSelMask).m_makeType)
        {
            //==================================================
            // Ignore 을 지울때는 메모리의 내용도 같이 지워준다
            // 1. 이름의 Index 를 찾음
            // 2. 해당 Index 를 제거
            //==================================================

            CString strName = m_pAlgoPara->m_maskOrder.GetAt(nSelMask).m_strName;
            long deleteIndex = m_pAlgoPara->m_ignore.FindIndex(strName);
            if (deleteIndex >= 0)
            {
                m_pAlgoPara->m_ignore.Del(deleteIndex);
            }

            strName.Empty();
        }

        m_pAlgoPara->m_maskOrder.Del(nSelMask);

        Refresh_MaskList();
        SetROI(m_pAlgoPara->m_ignore);

        m_gridMask.SetSelectedRange(nSelMask + 1, 0, nSelMask + 1, 2);
        m_gridMask.Refresh();
    }
}

void CSurfaceROIEditorDlg::OnChangeEDITInspROIOffsetLeft()
{
    CString text;
    GetDlgItem(IDC_EDIT_InspROIOffset_Left)->GetWindowText(text);
    m_pAlgoPara->m_offsetLeft_um = float(_ttof(text));
    uOnUpdateROISize();
    text.Empty();
}

void CSurfaceROIEditorDlg::OnChangeEDITInspROIOffsetRight()
{
    CString text;
    GetDlgItem(IDC_EDIT_InspROIOffset_Right)->GetWindowText(text);
    m_pAlgoPara->m_offsetRight_um = float(_ttof(text));
    uOnUpdateROISize();
    text.Empty();
}

void CSurfaceROIEditorDlg::OnChangeEDITInspROIOffsetTop()
{
    CString text;
    GetDlgItem(IDC_EDIT_InspROIOffset_Top)->GetWindowText(text);
    m_pAlgoPara->m_offsetTop_um = float(_ttof(text));
    uOnUpdateROISize();
    text.Empty();
}

void CSurfaceROIEditorDlg::OnChangeEDITInspROIOffsetBottom()
{
    CString text;
    GetDlgItem(IDC_EDIT_InspROIOffset_Bottom)->GetWindowText(text);
    m_pAlgoPara->m_offsetBottom_um = float(_ttof(text));
    uOnUpdateROISize();
    text.Empty();
}

void CSurfaceROIEditorDlg::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    OnOK();
}

void CSurfaceROIEditorDlg::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    OnCancel();
}

void CSurfaceROIEditorDlg::OnGridClick(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    int row = pNotify->iRow;
    int col = pNotify->iColumn;

    CString strROIName;
    CString strItemName;

    strROIName = m_gridMask.GetItemText(row, 2);
    strItemName = m_gridMask.GetItemText(0, col);
    //}}

    m_imageView->ImageOverlayClear();

    const auto& roiInfos = m_pVisionInsp->m_surfacePara->m_SurfaceCutemRoi;
    const auto* roiInfo = roiInfos.CustomROI_Search(strROIName);

    if (roiInfo == nullptr)
    {
        PatternImage pattern;
        if (pattern.setThresholdImage(m_thresholdImage))
        {
            m_imageView->SetImage(pattern);
        }
        else
        {
            MessageBox(_T("Not enough memory."));
        }

        *result = 0;
        return;
    }

    int nType = roiInfo->nType;
    int nvecSize = (long)roiInfo->vecShape.size();
    int nvecptSize;
    if (nType == 2)
    {
        const auto& image = roiInfo->m_image;

        if (image.GetSizeX() != m_pVisionInsp->getImageLotInsp().GetImageSizeX()
            || image.GetSizeY() != m_pVisionInsp->getImageLotInsp().GetImageSizeY())
        {
            PatternImage pattern;
            if (pattern.setThresholdImage(m_thresholdImage))
            {
                m_imageView->SetImage(pattern);
            }
            else
            {
                MessageBox(_T("Not enough memory."));
            }
            return;
        }

        PatternImage pattern;
        if (pattern.setThresholdImage(m_thresholdImage))
        {
            pattern.addPattern(image, RGB(255, 255, 0));
            m_imageView->SetImage(pattern);
        }
        else
        {
            MessageBox(_T("Not enough memory."));
        }
    }
    else
    {
        PatternImage pattern;
        if (pattern.setThresholdImage(m_thresholdImage))
        {
            m_imageView->SetImage(pattern);
        }
        else
        {
            MessageBox(_T("Not enough memory."));
        }
    }

    if (nvecSize > 0)
    {
        std::vector<Ipvm::Point32s2> vecptDraw;
        std::vector<Ipvm::Rect32r> vecfrtDraw;
        Ipvm::Rect32r frt;

        nvecptSize = (long)roiInfo->vecShape[0].vecptPos.size();
        vecptDraw.resize(nvecptSize);

        long offsetX = _ttoi(m_gridMask.GetItemText(row, 3));
        long offsetY = _ttoi(m_gridMask.GetItemText(row, 4));
        long PLeft = _ttoi(m_gridMask.GetItemText(row, 5));
        long PTop = _ttoi(m_gridMask.GetItemText(row, 6));
        long PRight = _ttoi(m_gridMask.GetItemText(row, 7));
        long PBottom = _ttoi(m_gridMask.GetItemText(row, 8));

        // 영훈 20140630_Surface_Thresold : Grid 선택시 너무 느려지는 부분이 있어 코드를 최적화 하고 Ellipse 부분 Vector 함수 추가
        for (int j = 0; j < nvecSize; j++)
        {
            if (nType <= 1) // 사각형, 원형
            {
                vecptDraw = roiInfo->vecShape[j].vecptPos;
                frt = Ipvm::Rect32r(
                    (float)vecptDraw[0].m_x, (float)vecptDraw[0].m_y, (float)vecptDraw[1].m_x, (float)vecptDraw[1].m_y);

                frt.m_left = frt.m_left + PLeft + offsetX;
                frt.m_top = frt.m_top + PTop + offsetY;
                frt.m_right = frt.m_right + PRight + offsetX;
                frt.m_bottom = frt.m_bottom + PBottom + offsetY;

                vecfrtDraw.push_back(frt);
            }
            else // 기타
            {
                vecptDraw = roiInfo->vecShape[j].vecptPos;

                for (int i = 0; i < (long)vecptDraw.size(); i++)
                {
                    long pX = _ttoi(m_gridMask.GetItemText(row, (5 + i * 2)));
                    long pY = _ttoi(m_gridMask.GetItemText(row, (5 + i * 2 + 1)));
                    vecptDraw[i].m_x = vecptDraw[i].m_x + offsetX + pX;
                    vecptDraw[i].m_y = vecptDraw[i].m_y + offsetY + pY;
                }

                //m_imageView->Overlay_RemoveAll();
                Ipvm::Polygon32r polygon;
                ImageViewEx::PointsToPolygon(vecptDraw, polygon);
                m_imageView->ImageOverlayAdd(polygon, RGB(255, 255, 0));
                m_imageView->ImageOverlayAdd(Ipvm::Conversion::ToPoint32r2(vecptDraw[0]), RGB(255, 255, 0));
                m_imageView->ImageOverlayAdd(
                    Ipvm::Conversion::ToPoint32r2(vecptDraw[vecptDraw.size() - 1]), RGB(255, 255, 0));
                for (int i = 0; i < (long)vecptDraw.size(); i++)
                {
                    CString str;
                    str.Format(_T("%d"), i + 1);
                    if (i == 0)
                    {
                        str = str + _T(" s");
                    }
                    else if (i == vecptDraw.size() - 1)
                    {
                        str = str + _T(" e");
                    }
                    m_imageView->ImageOverlayAdd(vecptDraw[i], str, RGB(255, 255, 0), 10);

                    str.Empty();
                }
            }
        }

        if (nType == 0) // 사각형
        {
            for (auto& object : vecfrtDraw)
            {
                m_imageView->ImageOverlayAdd(object, RGB(255, 255, 0));
            }
        }
        else if (nType == 1) // 원형
        {
            for (auto& object : vecfrtDraw)
            {
                Ipvm::EllipseEq32r ellipse;
                ImageViewEx::RectToEllipse(object, ellipse);
                m_imageView->ImageOverlayAdd(ellipse, RGB(255, 255, 0));
            }
        }

        m_imageView->ImageOverlayShow();
    }

    strROIName.Empty();
    strItemName.Empty();

    *result = 0;
}

void CSurfaceROIEditorDlg::OnLbnSelchangeListCustomroi()
{
    long nSelROI = m_lstCustomROI.GetCurSel();
    m_imageView->ImageOverlayClear();

    if (nSelROI >= 0)
    {
        CString strROIName;
        m_lstCustomROI.GetText(nSelROI, strROIName);

        const auto& roiInfos = m_pVisionInsp->m_surfacePara->m_SurfaceCutemRoi;
        const auto* roiInfo = roiInfos.CustomROI_Search(strROIName);

        if (roiInfo == nullptr)
            return;

        long nType = roiInfo->nType;
        long nvecSize = (long)roiInfo->vecShape.size();
        long nvecptSize;

        if (nvecSize > 0)
        {
            nvecptSize = (long)roiInfo->vecShape[0].vecptPos.size();

            std::vector<Ipvm::Point32s2> vecptDraw;
            std::vector<Ipvm::Rect32r> vecfrtDraw;
            Ipvm::Rect32r frt;

            vecptDraw.resize(nvecptSize);

            for (long shapeIndex = 0; shapeIndex < nvecSize; shapeIndex++)
            {
                auto& shape = roiInfo->vecShape[shapeIndex];

                if (nType <= 1)
                {
                    vecptDraw = shape.vecptPos;
                    frt = Ipvm::Rect32r((float)vecptDraw[0].m_x, (float)vecptDraw[0].m_y, (float)vecptDraw[1].m_x,
                        (float)vecptDraw[1].m_y);

                    vecfrtDraw.push_back(frt);
                }
                else
                {
                    vecptDraw = shape.vecptPos;

                    Ipvm::Polygon32r polygon;
                    ImageViewEx::PointsToPolygon(vecptDraw, polygon);
                    m_imageView->ImageOverlayAdd(polygon, RGB(255, 255, 0));
                    m_imageView->ImageOverlayAdd(Ipvm::Conversion::ToPoint32r2(vecptDraw[0]), RGB(255, 255, 0));
                    m_imageView->ImageOverlayAdd(
                        Ipvm::Conversion::ToPoint32r2(vecptDraw[vecptDraw.size() - 1]), RGB(255, 255, 0));
                    for (int i = 0; i < (long)vecptDraw.size(); i++)
                    {
                        CString str;
                        str.Format(_T("%d"), i + 1);
                        if (i == 0)
                        {
                            str = str + _T(" s");
                        }
                        else if (i == vecptDraw.size() - 1)
                        {
                            str = str + _T(" e");
                        }
                        m_imageView->ImageOverlayAdd(vecptDraw[i], str, RGB(255, 255, 0), 10);
                        str.Empty();
                    }
                }
            }

            if (nType == 0)
            {
                for (auto& object : vecfrtDraw)
                {
                    m_imageView->ImageOverlayAdd(object, RGB(255, 255, 0));
                }
            }
            else if (nType == 0)
            {
                for (auto& object : vecfrtDraw)
                {
                    Ipvm::EllipseEq32r ellipse;
                    ImageViewEx::RectToEllipse(object, ellipse);
                    m_imageView->ImageOverlayAdd(ellipse, RGB(255, 255, 0));
                }
            }

            m_imageView->ImageOverlayShow();
        }
        strROIName.Empty();
    }
}

void CSurfaceROIEditorDlg::OnBnClickedBtnViewInputImage()
{
    m_imageView->SetImage(m_grayImage);
}

void CSurfaceROIEditorDlg::OnBnClickedCheckCombineImage()
{
    bool applyCombine = ((CButton*)GetDlgItem(IDC_CHECK_COMBINE_IMAGE))->GetCheck() ? true : false;

    MakeGrayImage(applyCombine);
    UpdateMainPrepairResult();
}

void CSurfaceROIEditorDlg::MakeGrayImage(bool applyCombine)
{
    m_pVisionInsp->GetDisplayAlignImage(*m_pSurfaceItem, applyCombine, m_grayImage);
}

void CSurfaceROIEditorDlg::OnCbnSelchangeComboThresholdType()
{
    GetThresholdNormal() = (BOOL)(m_ThresholdType.GetCurSel() == 1);
    m_pVisionInsp->SetSurfaceAlgoPara(GetAlgoPara());

    uOnUpdateThresholdScrollBar();
    UpdateMainPrepairResult();

    m_subThresholdAdaptive->ShowWindow(GetThresholdNormal() ? SW_HIDE : SW_SHOW);
    m_subThresholdNormal->ShowWindow(GetThresholdNormal() ? SW_SHOW : SW_HIDE);
}

void CSurfaceROIEditorDlg::OnCbnSelchangeComboProfiletype()
{
    UpdateProfileView();
}

LRESULT CSurfaceROIEditorDlg::OnChangedThreshold(WPARAM target, LPARAM)
{
    m_profileThreshold = CAST_LONG(target);

    UpdateMainThresholdImage();

    return 0;
}
