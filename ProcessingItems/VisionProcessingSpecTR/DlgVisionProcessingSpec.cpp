//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingSpec.h"

//CPP_2_________________________________ This project's headers
#include "BgaGridGeneratorDlg.h"
#include "BodySizeEditor.h"
#include "DlgLandShapeEditor.h"
#include "VisionProcessingSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/BallCollectionOriginal.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
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
    ePID_Rotate,
    ePID_RotateComp,
    ePID_SelectLayer,
    ePID_LayerFlipX,
    ePID_LayerFlipY,
    ePID_LayerImportData,
    ePID_LayerExportData,
    ePID_LayerClearMapData,
    ePID_LayerCreateMapData,
    ePID_LayerEditor,
    ePID_SelectionMethod,
    ePID_CategoryWarning,
};

IMPLEMENT_DYNAMIC(CDlgVisionProcessingSpec, CDialog)

CDlgVisionProcessingSpec::CDlgVisionProcessingSpec(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingSpec* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionProcessingSpec::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_selectLayer(enCurLayerIndex::LayerIndex_Whole)
    , m_selectionMethod(2)
    , m_packageSpec(pVisionInsp->m_packageSpec)
    , m_propertyGrid(new CXTPPropertyGrid)
    , m_pVisionEditorDlg(nullptr)
{
}

CDlgVisionProcessingSpec::~CDlgVisionProcessingSpec()
{
    delete m_propertyGrid;
    delete m_pVisionEditorDlg;

    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionProcessingSpec::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STC_VIEWER, m_stcViewer);
}

BEGIN_MESSAGE_MAP(CDlgVisionProcessingSpec, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_WM_PAINT()
ON_WM_LBUTTONDOWN()
ON_WM_HSCROLL()
ON_WM_CLOSE()
ON_WM_LBUTTONDBLCLK()
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_WM_CONTEXTMENU()
ON_COMMAND(ID_LANDMENU_LANDSHAPECHANGE, &CDlgVisionProcessingSpec::OnLandshapechange)
END_MESSAGE_MAP()

// CDlgVisionProcessingSpec 메시지 처리기입니다.

BOOL CDlgVisionProcessingSpec::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtPropertyGrid;
    rtPropertyGrid.left = m_procDlgInfo.m_rtParaArea.left;
    rtPropertyGrid.top = m_procDlgInfo.m_rtParaArea.top;
    rtPropertyGrid.right = m_procDlgInfo.m_rtParaArea.right;
    rtPropertyGrid.bottom = m_procDlgInfo.m_rtDataArea.bottom;

    GetDlgItem(IDC_FRAME_PROPERTY_GRID)->MoveWindow(rtPropertyGrid);
    m_propertyGrid->Create(rtPropertyGrid, this, IDC_FRAME_PROPERTY_GRID);

    m_stcViewer.MoveWindow(m_procDlgInfo.m_rtImageArea);

    Initialize_View();

    Update_PropertyGrid();
    propertyChange_SelectLayer();

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionProcessingSpec::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CDlgVisionProcessingSpec::OnDestroy()
{
    CDialog::OnDestroy();
}

void CDlgVisionProcessingSpec::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect ViewRect;
    m_stcViewer.GetWindowRect(ViewRect);
    ScreenToClient(ViewRect);

    CDC MemDC;
    MemDC.CreateCompatibleDC(&dc);
    CBitmap* OldBitmap = MemDC.SelectObject(&m_bitmapView);
    dc.BitBlt(ViewRect.left, ViewRect.top, ViewRect.Width(), ViewRect.Height(), &MemDC, 0, 0, SRCCOPY);
    MemDC.SelectObject(OldBitmap);
}

void CDlgVisionProcessingSpec::Update_Background(long i_nCurLayerIndex)
{
    CDC* pDC = GetDC();
    m_bitmapGrid.DeleteObject();
    m_bitmapGrid.CreateCompatibleBitmap(pDC, m_rtView.Width(), m_rtView.Height());

    CDC MemDC;
    MemDC.CreateCompatibleDC(pDC);
    CBitmap* OldBitmap = MemDC.SelectObject(&m_bitmapGrid);

    // 디바이스 테두리 그리기
    CBrush* pOldBrush = NULL;
    CPen* pOldPen = NULL;

    if (m_packageSpec.m_bodyInfoMaster->m_bGlassCorePackage == TRUE)
    {
        CBrush brushGlassGray(RGB(160, 160, 160));
        pOldBrush = MemDC.SelectObject(&brushGlassGray);

        CPen penBGlassGray(PS_SOLID, 1, RGB(120, 120, 120));
        pOldPen = MemDC.SelectObject(&penBGlassGray);

        MemDC.Rectangle(m_rtDevice);

        MemDC.SelectObject(pOldBrush);
        MemDC.SelectObject(pOldPen);

        CBrush brushDarkGreen(RGB(0, 120, 0));
        pOldBrush = MemDC.SelectObject(&brushDarkGreen);

        CPen penBrightGreen(PS_SOLID, 3, RGB(0, 80, 0));
        pOldPen = MemDC.SelectObject(&penBrightGreen);

        if (m_rtGlassSubstrate.Width() * m_rtGlassSubstrate.Height() > 0) //이 조건은 Substrate 코팅이 없다는 의미다.
            MemDC.Rectangle(m_rtGlassSubstrate);

        MemDC.SelectObject(pOldBrush);
        MemDC.SelectObject(pOldPen);
    }
    else
    {
        CBrush brushDarkGreen(RGB(0, 120, 0));
        pOldBrush = MemDC.SelectObject(&brushDarkGreen);

        CPen penBrightGreen(PS_SOLID, 3, RGB(0, 80, 0));
        pOldPen = MemDC.SelectObject(&penBrightGreen);

        MemDC.Rectangle(m_rtDevice);

        MemDC.SelectObject(pOldBrush);
        MemDC.SelectObject(pOldPen);
    }

    // Ball Index by Bottom View
    CPen penLayerPinIndex(PS_SOLID, 1, RGB(0, 230, 0));
    MemDC.SelectObject(&penLayerPinIndex);
    CBrush brushLayerPinIndex(RGB(0, 230, 0));
    MemDC.SelectObject(&brushLayerPinIndex);

    MemDC.Polygon(m_ptPinIndex, 3);

    if (i_nCurLayerIndex == LayerIndex_Whole)
    {
        // Index by Bottom View
        CPen penWholePinIndex(PS_SOLID, 1, RGB(0, 230, 0));
        MemDC.SelectObject(&penWholePinIndex);
        CBrush brushWholePinIndex(RGB(0, 230, 0));
        MemDC.SelectObject(&brushWholePinIndex);

        MemDC.Polygon(m_ptPinIndex, 3);
    }

    MemDC.SelectObject(OldBitmap);
    MemDC.SelectObject(pOldBrush);
    MemDC.SelectObject(pOldPen);
}

void CDlgVisionProcessingSpec::Update_Foreground(long i_nCurLayerIndex)
{
    if (i_nCurLayerIndex < 0)
        return;

    CDC* pDC = GetDC();
    m_bitmapView.DeleteObject();
    m_bitmapView.CreateCompatibleBitmap(pDC, m_rtView.Width(), m_rtView.Height());

    //mc_Combo에 선택된 Index에 따라 그리기를 달리한다.
    switch (i_nCurLayerIndex)
    {
        case LayerIndex_Ball:
            SetDrawing_BallForegroundData(i_nCurLayerIndex, pDC);
            break;
        case LayerIndex_Land:
            SetDrawing_LandForegroundData(i_nCurLayerIndex, pDC);
            break;
        case LayerIndex_Compnent:
            SetDrawing_CompForegroundData(i_nCurLayerIndex, pDC);
            break;
        case LayerIndex_Pad:
            SetDrawing_PadForegroundData(i_nCurLayerIndex, pDC);
            break;
        case LayerIndex_CustomFixed:
            SetDrawing_CustomFixedForegroundData(i_nCurLayerIndex, pDC);
            break;
        case LayerIndex_CustomPolygon:
            SetDrawing_CustomPolygonForegroundData(i_nCurLayerIndex, pDC);
            break;
        case LayerIndex_Whole:
            SetDrawing_WholeForegroundData(pDC);
            break;
    }
    //

    ReleaseDC(pDC);
}

void CDlgVisionProcessingSpec::Update_Warning(long i_nCurLayerIndex)
{
    // Warning List 만들기
    auto* warningCategory = (CXTPPropertyGridItemCategory*)m_propertyGrid->FindItem(ePID_CategoryWarning);
    warningCategory->GetChilds()->Clear();

    if (i_nCurLayerIndex == LayerIndex_Compnent || i_nCurLayerIndex == LayerIndex_Whole)
    {
        std::map<CString, BOOL> warningTypeMap;
        for (long index = 0; index < m_packageSpec.m_CompMapManager->GetCount(); index++)
        {
            auto* comp = m_packageSpec.m_CompMapManager->GetMapDataPtr(index);
            if (comp->GetCompType() >= 0)
                continue;

            warningTypeMap[comp->strCompType] = TRUE;
        }

        for (auto& itWarning : warningTypeMap)
        {
            CString value;
            value.Format(_T("Type is wrong. (%s)"), LPCTSTR(itWarning.first));

            warningCategory->AddChildItem(new CXTPPropertyGridItem(_T("Component"), value));

            value.Empty();
        }
    }

    warningCategory->SetHidden(warningCategory->GetChilds()->GetCount() == 0);
}

void CDlgVisionProcessingSpec::SetDrawing_BallForegroundData(long i_nCurLayerIndex, CDC* i_pDC)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    // 볼을 그립니다
    COLORREF rgbMain = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);

    CBrush brushMain(rgbMain);
    CBrush* pOldBrush = MemDC.GetCurrentBrush();

    for (const auto& ballInfo : m_vecBallInfo)
    {
        if (ballInfo.m_ignored)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushMain);

        MemDC.Ellipse(ballInfo.m_ballPosition);
    }

    MemDC.SelectObject(pOldBrush);
    MemDC.SelectObject(pOldBitmap);
}

void CDlgVisionProcessingSpec::SetDrawing_LandForegroundData(long i_nCurLayerIndex, CDC* i_pDC)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    CPoint ptLandROI[4];

    CBrush* pOldBrush = MemDC.GetCurrentBrush();

    std::map<CString, long> groupToIndex;
    long lastGroupIndex = -1;

    for (const auto& LandInfo : m_vecLandInfo)
    {
        if (groupToIndex.find(LandInfo.m_groupID) == groupToIndex.end())
        {
            lastGroupIndex++;
            groupToIndex[LandInfo.m_groupID] = lastGroupIndex;
        }
    }

    //}}
    for (const auto& LandInfo : m_vecLandInfo)
    {
        long groupIndex = groupToIndex[LandInfo.m_groupID];
        long nSlpiter = groupIndex % 3; //kircheis_LandShape
        COLORREF rgbLand(0);
        switch (groupIndex) //mc_이후에 추가할꺼 있으면 수동으로 ..
        {
            case 1:
                rgbLand = RGB(255 - (1 + i_nCurLayerIndex / 2) * 100, 128, 255);
                break;
            case 2:
                rgbLand = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
                break;
            default: //kircheis_LandShape
            {
                long nColorOffset = (long)(((float)(groupIndex) / (float)lastGroupIndex) * 128.f);
                switch (nSlpiter) //mc_이후에 추가할꺼 있으면 수동으로 ..
                {
                    case 0:
                        rgbLand = RGB(200, 255 - nColorOffset, 200);
                        break;
                    case 1:
                        rgbLand = RGB(200, 200, 255 - nColorOffset);
                        break;
                    default:
                        rgbLand = RGB(255 - nColorOffset, 200, 200);
                        break;
                }
            }
            break;
        }

        CBrush brushLand(rgbLand);

        if (LandInfo.bIgnore)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushLand);

        if (LandInfo.nLandShapeType == Shape_Base_Circle) //kircheis_LandShape
        {
            Ipvm::Rect32s rtCircle = LandInfo.rtROI;
            long nWidth = rtCircle.Width();
            long nHeight = rtCircle.Height();
            long nOffset = (long)((float)(nWidth - nHeight) * .5f + .5f);
            if (nOffset > 0)
                rtCircle.DeflateRect(nOffset, 0);
            else if (nOffset < 0)
                rtCircle.InflateRect(0, nOffset);

            MemDC.Ellipse(LandInfo.rtROI.m_left, LandInfo.rtROI.m_top, LandInfo.rtROI.m_right, LandInfo.rtROI.m_bottom);
        }
        else
        {
            ptLandROI[0].x = long(LandInfo.fsrtSpecROI.fptLT.m_x + 0.5f);
            ptLandROI[0].y = long(LandInfo.fsrtSpecROI.fptLT.m_y + 0.5f);
            ptLandROI[1].x = long(LandInfo.fsrtSpecROI.fptLB.m_x + 0.5f);
            ptLandROI[1].y = long(LandInfo.fsrtSpecROI.fptLB.m_y + 0.5f);
            ptLandROI[2].x = long(LandInfo.fsrtSpecROI.fptRB.m_x + 0.5f);
            ptLandROI[2].y = long(LandInfo.fsrtSpecROI.fptRB.m_y + 0.5f);
            ptLandROI[3].x = long(LandInfo.fsrtSpecROI.fptRT.m_x + 0.5f);
            ptLandROI[3].y = long(LandInfo.fsrtSpecROI.fptRT.m_y + 0.5f);

            MemDC.Polygon(ptLandROI, 4);
        }
    }

    MemDC.SelectObject(pOldBrush);
    MemDC.SelectObject(pOldBitmap);
}

void CDlgVisionProcessingSpec::SetDrawing_CompForegroundData(long i_nCurLayerIndex, CDC* i_pDC)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    // Comp를 그립니다
    COLORREF rgbComp = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);

    CBrush brushComp(rgbComp);
    CBrush* pOldBrush = MemDC.GetCurrentBrush();

    CPoint ptCompROI[4];

    for (const auto& CompInfo : m_vecCompInfo)
    {
        if (CompInfo.bIgnore)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushComp);

        ptCompROI[0].x = long(CompInfo.fsrtROI.fptLT.m_x + 0.5f);
        ptCompROI[0].y = long(CompInfo.fsrtROI.fptLT.m_y + 0.5f);
        ptCompROI[1].x = long(CompInfo.fsrtROI.fptLB.m_x + 0.5f);
        ptCompROI[1].y = long(CompInfo.fsrtROI.fptLB.m_y + 0.5f);
        ptCompROI[2].x = long(CompInfo.fsrtROI.fptRB.m_x + 0.5f);
        ptCompROI[2].y = long(CompInfo.fsrtROI.fptRB.m_y + 0.5f);
        ptCompROI[3].x = long(CompInfo.fsrtROI.fptRT.m_x + 0.5f);
        ptCompROI[3].y = long(CompInfo.fsrtROI.fptRT.m_y + 0.5f);

        MemDC.Polygon(ptCompROI, 4);
    }

    MemDC.SelectObject(pOldBrush);
    MemDC.SelectObject(pOldBitmap);
}

void CDlgVisionProcessingSpec::SetDrawing_PadForegroundData(long i_nCurLayerIndex, CDC* i_pDC)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    COLORREF rgbPad = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
    CRect rtROI;

    CBrush brushPad(rgbPad);
    CBrush* pOldBrush = MemDC.GetCurrentBrush();

    for (const auto& PadInfo : m_vecPadInfo)
    {
        if (PadInfo.bIgnore)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushPad);

        if (PadInfo.GetType() == _typeRectangle)
        {
            CPoint ptPad_Rect[4];

            ptPad_Rect[0].x = long(PadInfo.fsrtSpecROI.fptLT.m_x + 0.5f);
            ptPad_Rect[0].y = long(PadInfo.fsrtSpecROI.fptLT.m_y + 0.5f);
            ptPad_Rect[1].x = long(PadInfo.fsrtSpecROI.fptLB.m_x + 0.5f);
            ptPad_Rect[1].y = long(PadInfo.fsrtSpecROI.fptLB.m_y + 0.5f);
            ptPad_Rect[2].x = long(PadInfo.fsrtSpecROI.fptRB.m_x + 0.5f);
            ptPad_Rect[2].y = long(PadInfo.fsrtSpecROI.fptRB.m_y + 0.5f);
            ptPad_Rect[3].x = long(PadInfo.fsrtSpecROI.fptRT.m_x + 0.5f);
            ptPad_Rect[3].y = long(PadInfo.fsrtSpecROI.fptRT.m_y + 0.5f);

            MemDC.Polygon(ptPad_Rect, 4);
        }
        else if (PadInfo.GetType() == _typeCircle)
        {
            CRect rtPad_Circle;
            rtPad_Circle = Ipvm::ToMFC(PadInfo.fsrtSpecROI.GetCRect());

            MemDC.Ellipse(rtPad_Circle);
        }
        else if (PadInfo.GetType() == _typeRectangle)
        {
            CRect rtPad_Rectangle;
            rtPad_Rectangle = Ipvm::ToMFC(PadInfo.fsrtSpecROI.GetCRect());

            MemDC.Rectangle(rtPad_Rectangle);
        }
        else
        {
            // __typePinIndex or _typeTriangle
            CPoint ptPad_PinIndex[3];
            GetPinIndexPoint(PadInfo.fsrtSpecROI, ptPad_PinIndex);

            MemDC.Polygon(ptPad_PinIndex, 3);
        }
    }

    MemDC.SelectObject(pOldBrush);
    MemDC.SelectObject(pOldBitmap);
}

void CDlgVisionProcessingSpec::GetPinIndexPoint(FPI_RECT i_frtPinIndex, CPoint* o_pPinIndexPoint)
{
    long nPinIndexPos(0);

    if (m_rtDevice.CenterPoint().x < i_frtPinIndex.GetCenter().m_x
        && m_rtDevice.CenterPoint().y < i_frtPinIndex.GetCenter().m_x)
        nPinIndexPos = PinIndexPos_LT;

    else if (m_rtDevice.CenterPoint().x >= i_frtPinIndex.GetCenter().m_x
        && m_rtDevice.CenterPoint().y < i_frtPinIndex.GetCenter().m_y)
        nPinIndexPos = PinIndexPos_RT;

    if (m_rtDevice.CenterPoint().x >= i_frtPinIndex.GetCenter().m_x
        && m_rtDevice.CenterPoint().y >= i_frtPinIndex.GetCenter().m_y)
        nPinIndexPos = PinIndexPos_RB;

    if (m_rtDevice.CenterPoint().x < i_frtPinIndex.GetCenter().m_x
        && m_rtDevice.CenterPoint().y >= i_frtPinIndex.GetCenter().m_y)
        nPinIndexPos = PinIndexPos_LB;

    switch (nPinIndexPos)
    {
        case PinIndexPos_LT:
            o_pPinIndexPoint[0].x = long(i_frtPinIndex.fptLB.m_x + 0.5f);
            o_pPinIndexPoint[0].y = long(i_frtPinIndex.fptLB.m_y + 0.5f);
            o_pPinIndexPoint[1].x = long(i_frtPinIndex.fptRB.m_x + 0.5f);
            o_pPinIndexPoint[1].y = long(i_frtPinIndex.fptRB.m_y + 0.5f);
            o_pPinIndexPoint[2].x = long(i_frtPinIndex.fptRT.m_x + 0.5f);
            o_pPinIndexPoint[2].y = long(i_frtPinIndex.fptRT.m_y + 0.5f);
            break;
        case PinIndexPos_RT:
            o_pPinIndexPoint[0].x = long(i_frtPinIndex.fptLT.m_x + 0.5f);
            o_pPinIndexPoint[0].y = long(i_frtPinIndex.fptLT.m_y + 0.5f);
            o_pPinIndexPoint[1].x = long(i_frtPinIndex.fptLB.m_x + 0.5f);
            o_pPinIndexPoint[1].y = long(i_frtPinIndex.fptLB.m_y + 0.5f);
            o_pPinIndexPoint[2].x = long(i_frtPinIndex.fptRB.m_x + 0.5f);
            o_pPinIndexPoint[2].y = long(i_frtPinIndex.fptRB.m_y + 0.5f);
            break;
        case PinIndexPos_RB:
            o_pPinIndexPoint[0].x = long(i_frtPinIndex.fptLT.m_x + 0.5f);
            o_pPinIndexPoint[0].y = long(i_frtPinIndex.fptLT.m_y + 0.5f);
            o_pPinIndexPoint[1].x = long(i_frtPinIndex.fptLB.m_x + 0.5f);
            o_pPinIndexPoint[1].y = long(i_frtPinIndex.fptLB.m_y + 0.5f);
            o_pPinIndexPoint[2].x = long(i_frtPinIndex.fptRT.m_x + 0.5f);
            o_pPinIndexPoint[2].y = long(i_frtPinIndex.fptRT.m_y + 0.5f);
            break;
        case PinIndexPos_LB:
            o_pPinIndexPoint[0].x = long(i_frtPinIndex.fptLT.m_x + 0.5f);
            o_pPinIndexPoint[0].y = long(i_frtPinIndex.fptLT.m_y + 0.5f);
            o_pPinIndexPoint[1].x = long(i_frtPinIndex.fptRB.m_x + 0.5f);
            o_pPinIndexPoint[1].y = long(i_frtPinIndex.fptRB.m_y + 0.5f);
            o_pPinIndexPoint[2].x = long(i_frtPinIndex.fptRT.m_x + 0.5f);
            o_pPinIndexPoint[2].y = long(i_frtPinIndex.fptRT.m_y + 0.5f);
            break;
        default:
            break;
    }
}

void CDlgVisionProcessingSpec::SetDrawing_CustomFixedForegroundData(long i_nCurLayerIndex, CDC* i_pDC)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);
    CBrush* pOldBrush = MemDC.GetCurrentBrush();

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    COLORREF rgbCustomFixed = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
    COLORREF rgbNotch = RGB(0, 0, 0);
    CBrush brushCustomFixed(rgbCustomFixed);
    CBrush brushNotch(rgbNotch);

    for (const auto& CustomFixedInfo : m_vecCustomFixedInfo)
    {
        bool bisIgnore = CustomFixedInfo.m_bIgnore;
        if (bisIgnore == true)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushCustomFixed);

        if (CustomFixedInfo.m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Circle)
        {
            CRect rtCustomFixed_Circle = Ipvm::ToMFC(CustomFixedInfo.m_fsrtSpecROI.GetCRect());
            MemDC.Ellipse(rtCustomFixed_Circle);
        }
        else if (CustomFixedInfo.m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_NotchHole)
        {
            //Notch는 Black으로 고정한다
            if (bisIgnore == false)
            {
                MemDC.SelectObject(&brushNotch);
            }

            CRect rtCustomFixed_Circle = Ipvm::ToMFC(CustomFixedInfo.m_fsrtSpecROI.GetCRect());
            MemDC.Ellipse(rtCustomFixed_Circle);
        }
        else if (CustomFixedInfo.m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Rectangle)
        {
            CRect rtCustomFixed_Rectangle = Ipvm::ToMFC(CustomFixedInfo.m_fsrtSpecROI.GetCRect());
            MemDC.Rectangle(rtCustomFixed_Rectangle);
        }
    }

    MemDC.SelectObject(pOldBrush);
    MemDC.SelectObject(pOldBitmap);
}

void CDlgVisionProcessingSpec::SetDrawing_CustomPolygonForegroundData(long i_nCurLayerIndex, CDC* i_pDC)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);
    CBrush* pOldBrush = MemDC.GetCurrentBrush();

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    COLORREF rgbCustomPolygon = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
    CBrush brushCustomPolygon(rgbCustomPolygon);
    //const float fSystemScale = (((SystemConfig::GetInstance().Get2DScaleX() + SystemConfig::GetInstance().Get2DScaleY()) *.5f) + .5f);

    for (const auto& CustomPolygonInfo : m_vecCustomPolygonInfo)
    {
        bool bisIgnore = CustomPolygonInfo.m_bIgnore;
        if (bisIgnore == true)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushCustomPolygon);

        if (CustomPolygonInfo.m_eCustomPolygonCategory == CustomPolygonCategory::CustomPolygonCategory_Polygon)
        {
            CPoint* pptPolygon = new CPoint[CustomPolygonInfo.m_nPointNum];
            for (long nPolygonidx = 0; nPolygonidx < CustomPolygonInfo.m_nPointNum; nPolygonidx++)
            {
                pptPolygon[nPolygonidx] = CPoint(CAST_INT(CustomPolygonInfo.m_vecfptPointInfo_mm[nPolygonidx].m_x),
                    CAST_INT(CustomPolygonInfo.m_vecfptPointInfo_mm[nPolygonidx].m_y));
            }

            MemDC.Polygon(pptPolygon, CustomPolygonInfo.m_nPointNum);
            delete[] pptPolygon;
        }
        //else if (CustomPolygonInfo.m_eCustomPolygonCategory == CustomPolygonCategory::CustomCategory_InkBarrier)
        //{
        //	if (bisIgnore == true)
        //		rgbCustom = RGB(0, 0, 0);

        //	int nPenWidth = ((((CustomPolygonInfo.fWidth_mm * 1000.f) / fSystemScale) * fZoomFactor) + .5f);

        //	CPen PenCustom(PS_SOLID, nPenWidth, rgbCustom);
        //	CPen *pOldPen = MemDC.SelectObject(&PenCustom);
        //	for (long nInkBarrieridx = 0; nInkBarrieridx < CustomInfo.nUsePointNum - 1; nInkBarrieridx++)
        //	{
        //		if (nInkBarrieridx == 0)
        //			MemDC.MoveTo(CustomInfo.vecfptPointInfo_mm[nInkBarrieridx].m_x, CustomInfo.vecfptPointInfo_mm[nInkBarrieridx].m_y);

        //		MemDC.LineTo(CustomInfo.vecfptPointInfo_mm[nInkBarrieridx + 1].m_x, CustomInfo.vecfptPointInfo_mm[nInkBarrieridx + 1].m_y);
        //	}

        //	MemDC.SelectObject(pOldPen);
        //	PenCustom.DeleteObject();
        //}
    }

    MemDC.SelectObject(pOldBrush);
    MemDC.SelectObject(pOldBitmap);
}

void CDlgVisionProcessingSpec::SetDrawing_WholeForegroundData(CDC* i_pDC)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);
    CBrush* pOldBrush = MemDC.GetCurrentBrush();

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    // 볼을 그립니다
    COLORREF rgbWholeDevice_Ball = RGB(242, 150, 97);

    CBrush brushBall(rgbWholeDevice_Ball);

    for (const auto& ballInfo : m_vecBallInfo)
    {
        if (ballInfo.m_ignored)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushBall);

        MemDC.Ellipse(ballInfo.m_ballPosition);
    }

    //Land를 그립니다.
    COLORREF rgbWholeDevice_Land = RGB(242, 150, 97);
    CBrush brushLand(rgbWholeDevice_Land);

    CPoint ptLandROI[4];

    for (const auto& LandInfo : m_vecLandInfo)
    {
        if (LandInfo.bIgnore)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushLand);

        if (LandInfo.nLandShapeType == Shape_Base_Circle) //kircheis_LandShape
        {
            Ipvm::Rect32s rtCircle = LandInfo.rtROI;
            long nWidth = rtCircle.Width();
            long nHeight = rtCircle.Height();
            long nOffset = (long)((float)(nWidth - nHeight) * .5f + .5f);
            if (nOffset > 0)
                rtCircle.DeflateRect(nOffset, 0);
            else if (nOffset < 0)
                rtCircle.InflateRect(0, nOffset);

            MemDC.Ellipse(LandInfo.rtROI.m_left, LandInfo.rtROI.m_top, LandInfo.rtROI.m_right, LandInfo.rtROI.m_bottom);
        }
        else
        {
            ptLandROI[0].x = long(LandInfo.fsrtSpecROI.fptLT.m_x + 0.5f);
            ptLandROI[0].y = long(LandInfo.fsrtSpecROI.fptLT.m_y + 0.5f);
            ptLandROI[1].x = long(LandInfo.fsrtSpecROI.fptLB.m_x + 0.5f);
            ptLandROI[1].y = long(LandInfo.fsrtSpecROI.fptLB.m_y + 0.5f);
            ptLandROI[2].x = long(LandInfo.fsrtSpecROI.fptRB.m_x + 0.5f);
            ptLandROI[2].y = long(LandInfo.fsrtSpecROI.fptRB.m_y + 0.5f);
            ptLandROI[3].x = long(LandInfo.fsrtSpecROI.fptRT.m_x + 0.5f);
            ptLandROI[3].y = long(LandInfo.fsrtSpecROI.fptRT.m_y + 0.5f);

            MemDC.Polygon(ptLandROI, 4);
        }
    }

    //Comp를 그립니다
    COLORREF rgbWholeDevice_Comp = RGB(255, 255, 72);
    CBrush brushComp(rgbWholeDevice_Comp);

    CPoint ptCompROI[4];

    for (const auto& CompInfo : m_vecCompInfo)
    {
        if (CompInfo.bIgnore)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushComp);

        ptCompROI[0].x = long(CompInfo.fsrtROI.fptLT.m_x + 0.5f);
        ptCompROI[0].y = long(CompInfo.fsrtROI.fptLT.m_y + 0.5f);
        ptCompROI[1].x = long(CompInfo.fsrtROI.fptLB.m_x + 0.5f);
        ptCompROI[1].y = long(CompInfo.fsrtROI.fptLB.m_y + 0.5f);
        ptCompROI[2].x = long(CompInfo.fsrtROI.fptRB.m_x + 0.5f);
        ptCompROI[2].y = long(CompInfo.fsrtROI.fptRB.m_y + 0.5f);
        ptCompROI[3].x = long(CompInfo.fsrtROI.fptRT.m_x + 0.5f);
        ptCompROI[3].y = long(CompInfo.fsrtROI.fptRT.m_y + 0.5f);

        MemDC.Polygon(ptCompROI, 4);
    }

    //Pad를 그립니다.
    COLORREF rgbWholeDevice_Pad = RGB(201, 138, 255);
    CBrush brushPad(rgbWholeDevice_Pad);

    CRect rtROI;
    float fOffsetX(0.f), fOffsetY(0.f);
    for (long n = 0; n < (long)m_vecPadInfo.size(); n++)
    {
        if (m_vecPadInfo[n].bIgnore)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushPad);

        rtROI = Ipvm::ToMFC(m_vecPadInfo[n].rtROI);
        fOffsetX = (rtROI.left + rtROI.right) * 0.5f;
        fOffsetY = (rtROI.top + rtROI.bottom) * 0.5f;

        if (m_vecPadInfo[n].GetType() == _typeTriangle || m_vecPadInfo[n].GetType() == _typePinIndex)
        {
            CPoint ptPadIndex[4];

            if ((m_rtDevice.right / 2) >= m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x
                && (m_rtDevice.bottom / 2) >= m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y)
            {
                ptPadIndex[0].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                ptPadIndex[0].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptPadIndex[1].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptPadIndex[1].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                ptPadIndex[2].x
                    = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x - m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x) / 2
                        + m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptPadIndex[2].y
                    = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y - m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y) / 2
                        + m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptPadIndex[3].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                ptPadIndex[3].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);
            }
            else if ((m_rtDevice.right / 2) < m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x
                && (m_rtDevice.bottom / 2) < m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y)
            {
                ptPadIndex[0].x
                    = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x - m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x) / 2
                        + m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptPadIndex[0].y
                    = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y - m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y) / 2
                        + m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptPadIndex[1].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptPadIndex[1].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                ptPadIndex[2].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                ptPadIndex[2].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                ptPadIndex[3].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                ptPadIndex[3].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);
            }
            else if ((m_rtDevice.right / 2) >= m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x
                && (m_rtDevice.bottom / 2) < m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y)
            {
                ptPadIndex[0].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                ptPadIndex[0].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptPadIndex[1].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptPadIndex[1].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                ptPadIndex[2].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                ptPadIndex[2].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                ptPadIndex[3].x
                    = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x - m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x) / 2
                        + m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptPadIndex[3].y
                    = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y - m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y) / 2
                        + m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
            }
            else if ((m_rtDevice.right / 2) < m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x
                && (m_rtDevice.bottom / 2) >= m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y)
            {
                ptPadIndex[0].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                ptPadIndex[0].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptPadIndex[1].x
                    = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x - m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x) / 2
                        + m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptPadIndex[1].y
                    = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y - m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y) / 2
                        + m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptPadIndex[2].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                ptPadIndex[2].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                ptPadIndex[3].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                ptPadIndex[3].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);
            }

            MemDC.Polygon(ptPadIndex, 4);
            MemDC.SelectObject(brushPad);
        }
        else if (m_vecPadInfo[n].GetType() == _typeCircle)
        {
            CRect m_PadPosition;
            m_PadPosition = Ipvm::ToMFC(m_vecPadInfo[n].fsrtSpecROI.GetCRect());
            MemDC.Ellipse(m_PadPosition);
        }
        else if (m_vecPadInfo[n].GetType() == _typeRectangle)
        {
            CRect m_PadPosition;
            m_PadPosition = Ipvm::ToMFC(m_vecPadInfo[n].fsrtSpecROI.GetCRect());
            MemDC.Rectangle(m_PadPosition);
        }
    }

    //Custom을 그립니다.
    COLORREF rgbWholeDevice_Custom = RGB(54, 138, 255);
    COLORREF rgbNotch = RGB(0, 0, 0); //Notch는 Black으로 고정한다
    CBrush brushCustom(rgbWholeDevice_Custom);
    CBrush brushNotch(rgbNotch);

    //const float fSystemScale = (((SystemConfig::GetInstance().Get2DScaleX() + SystemConfig::GetInstance().Get2DScaleY()) *.5f) + .5f);

    for (const auto& CustomInfoFixed : m_vecCustomFixedInfo)
    {
        bool bisIgnore = CustomInfoFixed.m_bIgnore;
        if (bisIgnore == true)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushCustom);

        if (CustomInfoFixed.m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Circle)
        {
            CRect rtCustom_CircleFixed = Ipvm::ToMFC(CustomInfoFixed.m_fsrtSpecROI.GetCRect());
            MemDC.Ellipse(rtCustom_CircleFixed);
        }
        else if (CustomInfoFixed.m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_NotchHole)
        {
            //Notch는 Black으로 고정한다
            if (bisIgnore == false)
            {
                MemDC.SelectObject(&brushNotch);
            }

            CRect rtCustom_CircleFixed = Ipvm::ToMFC(CustomInfoFixed.m_fsrtSpecROI.GetCRect());
            MemDC.Ellipse(rtCustom_CircleFixed);
        }
        else if (CustomInfoFixed.m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Rectangle)
        {
            CRect rtCustom_RectangleFixed = Ipvm::ToMFC(CustomInfoFixed.m_fsrtSpecROI.GetCRect());
            MemDC.Rectangle(rtCustom_RectangleFixed);
        }
    }

    for (const auto& CustomInfoPolygon : m_vecCustomPolygonInfo)
    {
        bool bisIgnore = CustomInfoPolygon.m_bIgnore;
        if (bisIgnore == true)
            MemDC.SelectObject(GetStockObject(NULL_BRUSH));
        else
            MemDC.SelectObject(&brushCustom);

        if (CustomInfoPolygon.m_eCustomPolygonCategory == CustomPolygonCategory::CustomPolygonCategory_Polygon)
        {
            CPoint* pptPolygon = new CPoint[CustomInfoPolygon.m_nPointNum];
            for (long nPolygonidx = 0; nPolygonidx < CustomInfoPolygon.m_nPointNum; nPolygonidx++)
            {
                pptPolygon[nPolygonidx] = CPoint(CAST_INT(CustomInfoPolygon.m_vecfptPointInfo_mm[nPolygonidx].m_x),
                    CAST_INT(CustomInfoPolygon.m_vecfptPointInfo_mm[nPolygonidx].m_y));
            }

            MemDC.Polygon(pptPolygon, CustomInfoPolygon.m_nPointNum);
            delete[] pptPolygon;
        }
        //else if (CustomInfo.eCustomCategory == CustomCategory::CustomCategory_InkBarrier)
        //{
        //if (bisIgnore == true)
        //	rgbWholeDevice_Custom = RGB(0, 0, 0);

        //int nPenWidth = ((((CustomInfo.fWidth_mm * 1000.f) / fSystemScale) * fZoomFactor) + .5f);

        //CPen PenCustom(PS_SOLID, nPenWidth, rgbWholeDevice_Custom);
        //CPen *pOldPen = MemDC.SelectObject(&PenCustom);
        //for (long nInkBarrieridx = 0; nInkBarrieridx < CustomInfo.nUsePointNum - 1; nInkBarrieridx++)
        //{
        //	if (nInkBarrieridx == 0)
        //		MemDC.MoveTo(CustomInfo.vecfptPointInfo_mm[nInkBarrieridx].m_x, CustomInfo.vecfptPointInfo_mm[nInkBarrieridx].m_y);

        //	MemDC.LineTo(CustomInfo.vecfptPointInfo_mm[nInkBarrieridx + 1].m_x, CustomInfo.vecfptPointInfo_mm[nInkBarrieridx + 1].m_y);
        //}

        //MemDC.SelectObject(pOldPen);
        //PenCustom.DeleteObject();
        //}
    }

    MemDC.SelectObject(pOldBrush);
    MemDC.SelectObject(pOldBitmap);
}

void CDlgVisionProcessingSpec::Update_PropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Device")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Setup body size"), TRUE, FALSE))->SetID(ePID_BodySize);

        auto* item_rotate = category->AddChildItem(
            new CXTPPropertyGridItemEnum(_T("Rotate"), long(m_packageSpec.m_originalballMap->m_ballMapRotation)));
        item_rotate->GetConstraints()->AddConstraint(
            _T("0 deg"), long(Package::BallCollectionOriginal::RotationIndex::Deg_0));
        item_rotate->GetConstraints()->AddConstraint(
            _T("90 deg"), long(Package::BallCollectionOriginal::RotationIndex::Deg_90));
        item_rotate->GetConstraints()->AddConstraint(
            _T("180 deg"), long(Package::BallCollectionOriginal::RotationIndex::Deg_180));
        item_rotate->GetConstraints()->AddConstraint(
            _T("270 deg"), long(Package::BallCollectionOriginal::RotationIndex::Deg_270));
        item_rotate->SetID(ePID_Rotate);

        auto* item_rotateComp = category->AddChildItem(
            new CXTPPropertyGridItemEnum(_T("Rotate Comp"), m_packageSpec.m_OriginCompData->m_nRotateCompIdx));
        item_rotateComp->GetConstraints()->AddConstraint(_T("0 deg"), 0);
        item_rotateComp->GetConstraints()->AddConstraint(_T("90 deg"), 1);
        item_rotateComp->SetID(ePID_RotateComp);

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Layer")))
    {
        auto* item_selectLayer
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Select layer"), m_selectLayer));

        for (long nLayeridx = enCurLayerIndex::LayerIndex_Pad; nLayeridx < enCurLayerIndex::LayerIndex_Whole + 1;
            nLayeridx++)
        {
            item_selectLayer->GetConstraints()->AddConstraint(g_szLayerIndexName[nLayeridx], nLayeridx);
        }

        item_selectLayer->SetID(ePID_SelectLayer);

        category->AddChildItem(new CXTPPropertyGridItemBool(_T("Flip X"), getCurrentLayer_FlipX()))
            ->SetID(ePID_LayerFlipX);
        category->AddChildItem(new CXTPPropertyGridItemBool(_T("Flip Y"), getCurrentLayer_FlipY()))
            ->SetID(ePID_LayerFlipY);

        auto* itemImport = category->AddChildItem(new CCustomItemButton(_T("Import Map Data"), TRUE, FALSE));
        auto* itemExport = category->AddChildItem(new CCustomItemButton(_T("Export Map Data"), TRUE, FALSE));
        auto* itemClear = category->AddChildItem(new CCustomItemButton(_T("Clear Map Data"), TRUE, FALSE));
        auto* itemCreate = category->AddChildItem(new CCustomItemButton(_T("Create Map Data"), TRUE, FALSE));
        auto* itemEditor = category->AddChildItem(new CCustomItemButton(_T("MapData Editor"), TRUE, FALSE));

        itemImport->SetReadOnly(m_selectLayer == enCurLayerIndex::LayerIndex_Whole);
        itemExport->SetReadOnly(m_selectLayer == enCurLayerIndex::LayerIndex_Whole);
        itemImport->SetID(ePID_LayerImportData);
        itemExport->SetID(ePID_LayerExportData);
        itemClear->SetID(ePID_LayerClearMapData);
        itemCreate->SetID(ePID_LayerCreateMapData);
        itemEditor->SetID(ePID_LayerEditor);
        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Option")))
    {
        auto* item_selectionMethod
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Selection Method"), long(m_selectionMethod)));
        item_selectionMethod->GetConstraints()->AddConstraint(_T("Enable Selected Objects"), 0);
        item_selectionMethod->GetConstraints()->AddConstraint(_T("Disable Selected Objects"), 1);
        item_selectionMethod->GetConstraints()->AddConstraint(_T("Invert Selected Objects"), 2);
        item_selectionMethod->SetID(ePID_SelectionMethod);
        category->Expand();
    }

    Update_PropertyGrid_LayersSummy();

    if (auto* category = m_propertyGrid->AddCategory(_T("Warning List")))
    {
        category->SetID(ePID_CategoryWarning);
        category->Expand();
    }

    m_propertyGrid->SetViewDivider(0.30);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

void CDlgVisionProcessingSpec::Update_PropertyGrid_LayersSummy()
{
    CString categoryName = _T("Layers summary");
    auto* category = m_propertyGrid->GetCategories()->FindItem(categoryName);

    if (category == nullptr)
    {
        category = m_propertyGrid->AddCategory(categoryName);
    }

    if (m_packageSpec.m_PadMapManager->vecPadData.size() > 0)
    {
        auto* PadCounts = category->AddChildItem(
            new CXTPPropertyGridItemNumber(_T("Pad counts"), long(m_packageSpec.m_PadMapManager->vecPadData.size())));
        PadCounts->SetReadOnly();
    }

    if (m_packageSpec.m_ballMap->m_balls.size() > 0)
    {
        auto* ballCounts = category->AddChildItem(
            new CXTPPropertyGridItemNumber(_T("Ball counts"), long(m_packageSpec.m_ballMap->m_balls.size())));
        ballCounts->SetReadOnly();
    }

    if (m_packageSpec.m_LandMapConvertOrigin->vecLandData.size() > 0)
    {
        auto* LandCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(
            _T("Land counts"), long(m_packageSpec.m_LandMapConvertOrigin->vecLandData.size())));
        LandCounts->SetReadOnly();
    }

    if (m_packageSpec.m_CompMapManager->vecCompData.size() > 0)
    {
        auto* CompCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(
            _T("Comp counts"), long(m_packageSpec.m_CompMapManager->vecCompData.size())));
        CompCounts->SetReadOnly();
    }

    if (m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData.size() > 0)
    {
        auto* CustomCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(
            _T("CustomFixed counts"), long(m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData.size())));
        CustomCounts->SetReadOnly();
    }

    if (m_packageSpec.m_CustomPolygonMapManager->vecCustomPolygonData.size() > 0)
    {
        auto* CustomCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(
            _T("CustomPolygon counts"), long(m_packageSpec.m_CustomPolygonMapManager->vecCustomPolygonData.size())));
        CustomCounts->SetReadOnly();
    }

    {
        long nPadObejctNum = (long)m_packageSpec.m_PadMapManager->vecPadData.size();
        long nBallObejctNum = (long)m_packageSpec.m_ballMap->m_balls.size();
        long nLandObejctNum = (long)m_packageSpec.m_LandMapConvertOrigin->vecLandData.size();
        long nComponetObejectNum = (long)m_packageSpec.m_CompMapManager->vecCompData.size();
        long nCustomFixedObjectNum = (long)m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData.size();
        long nCustomPolygonObjectNum = (long)m_packageSpec.m_CustomPolygonMapManager->vecCustomPolygonData.size();

        long nTotalObjectNum = nPadObejctNum + nBallObejctNum + nLandObejctNum + nComponetObejectNum
            + nCustomFixedObjectNum + nCustomPolygonObjectNum;

        auto* TotalCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Total counts"), nTotalObjectNum));

        TotalCounts->SetReadOnly();
    }

    category->Expand();

    categoryName.Empty();
}

void CDlgVisionProcessingSpec::Initialize_View()
{
    m_stcViewer.GetWindowRect(m_rtView);

    m_rtView.OffsetRect(-m_rtView.left, -m_rtView.top);

    // 그려질 디바이스 크기 결정
    float fBodySizeX = m_packageSpec.m_bodyInfoMaster->fBodySizeX;
    float fBodySizeY = m_packageSpec.m_bodyInfoMaster->fBodySizeY;
    float fSubstrateSizeX = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeX;
    float fSubstrateSizeY = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeY;
    float fFactorX = fSubstrateSizeX / fBodySizeX;
    float fFactorY = fSubstrateSizeY / fBodySizeY;

    m_fViewAspectRatio = float(m_rtView.Width()) / m_rtView.Height();
    m_fDeviceAspectRatio = fBodySizeX / fBodySizeY;

    BOOL bIsGlassPackage = m_packageSpec.m_bodyInfoMaster->m_bGlassCorePackage;
    if (bIsGlassPackage == FALSE)
        m_rtGlassSubstrate = CRect(0, 0, 0, 0);

    long nMargin = 80; // 영훈 20160422 : 여백을 둬서 그 부분에 Ball ID를 입력해주도록 한다.
    if (m_fViewAspectRatio >= m_fDeviceAspectRatio)
    {
        float fDeviceHalfHeightPixel = 0.5f * (m_rtView.Height() - nMargin);
        float fDeviceHalfWidthPixel = 0.5f * (m_rtView.Height() - nMargin) * m_fDeviceAspectRatio;

        m_rtDevice.left = long(m_rtView.CenterPoint().x - fDeviceHalfWidthPixel + 0.5f);
        m_rtDevice.right = long(m_rtView.CenterPoint().x + fDeviceHalfWidthPixel + 0.5f) + 1;
        m_rtDevice.top = long(m_rtView.CenterPoint().y - fDeviceHalfHeightPixel + 0.5f);
        m_rtDevice.bottom = long(m_rtView.CenterPoint().y + fDeviceHalfHeightPixel + 0.5f) + 1;
        if (bIsGlassPackage == TRUE)
        {
            float fGlassSubstrateHalfHeightPixel = 0.5f * (m_rtView.Height() - nMargin) * fFactorY;
            float fGlassSubstrateHalfWidthPixel
                = 0.5f * (m_rtView.Height() - nMargin) * m_fDeviceAspectRatio * fFactorX;

            m_rtGlassSubstrate.left = long(m_rtView.CenterPoint().x - fGlassSubstrateHalfWidthPixel + 0.5f);
            m_rtGlassSubstrate.right = long(m_rtView.CenterPoint().x + fGlassSubstrateHalfWidthPixel + 0.5f) + 1;
            m_rtGlassSubstrate.top = long(m_rtView.CenterPoint().y - fGlassSubstrateHalfHeightPixel + 0.5f);
            m_rtGlassSubstrate.bottom = long(m_rtView.CenterPoint().y + fGlassSubstrateHalfHeightPixel + 0.5f) + 1;
        }
    }
    else
    {
        float fDeviceHalfHeightPixel = 0.5f * (m_rtView.Width() - nMargin) / m_fDeviceAspectRatio;
        float fDeviceHalfWidthPixel = 0.5f * (m_rtView.Width() - nMargin);

        m_rtDevice.left = long(m_rtView.CenterPoint().x - fDeviceHalfWidthPixel + 0.5f);
        m_rtDevice.right = long(m_rtView.CenterPoint().x + fDeviceHalfWidthPixel + 0.5f) + 1;
        m_rtDevice.top = long(m_rtView.CenterPoint().y - fDeviceHalfHeightPixel + 0.5f);
        m_rtDevice.bottom = long(m_rtView.CenterPoint().y + fDeviceHalfHeightPixel + 0.5f) + 1;

        if (bIsGlassPackage == TRUE)
        {
            float fGlassSubstrateHalfHeightPixel
                = 0.5f * (m_rtView.Width() - nMargin) / m_fDeviceAspectRatio * fFactorY;
            float fGlassSubstrateHalfWidthPixel = 0.5f * (m_rtView.Width() - nMargin) * fFactorX;
            m_rtGlassSubstrate.left = long(m_rtView.CenterPoint().x - fGlassSubstrateHalfWidthPixel + 0.5f);
            m_rtGlassSubstrate.right = long(m_rtView.CenterPoint().x + fGlassSubstrateHalfWidthPixel + 0.5f) + 1;
            m_rtGlassSubstrate.top = long(m_rtView.CenterPoint().y - fGlassSubstrateHalfHeightPixel + 0.5f);
            m_rtGlassSubstrate.bottom = long(m_rtView.CenterPoint().y + fGlassSubstrateHalfHeightPixel + 0.5f) + 1;
        }
    }

    m_fZoom = m_rtDevice.Width() / (m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    m_fZoom = m_rtDevice.Height() / (m_packageSpec.m_bodyInfoMaster->fBodySizeY);
}

void CDlgVisionProcessingSpec::OnLButtonDown(UINT nFlags, CPoint point)
{
    CRectTracker tracker;

    if (tracker.TrackRubberBand(this, point))
    {
        CRect rtSelection;

        tracker.GetTrueRect(&rtSelection);

        CRect rtViewer;
        m_stcViewer.GetWindowRect(rtViewer);
        ScreenToClient(rtViewer);

        rtSelection -= rtViewer.TopLeft();

        //분리
        long nCurSelectLayer = m_selectLayer;
        switch (nCurSelectLayer)
        {
            case LayerIndex_Pad:
                Ignore_Pad(rtSelection, m_selectionMethod);
                break;
            case LayerIndex_Ball:
                Ignore_Ball(rtSelection, m_selectionMethod);
                break;
            case LayerIndex_Land:
                Ignore_Land(rtSelection, m_selectionMethod);
                break;
            case LayerIndex_Compnent:
                Ignore_Comp(rtSelection, m_selectionMethod);
                break;
            case LayerIndex_CustomFixed:
                Ignore_CustomFixed(rtSelection, m_selectionMethod);
                break;
            case LayerIndex_CustomPolygon:
                Ignore_CustomPolygon(rtSelection, m_selectionMethod);
                break;
            default:
                break;
        }

        //
        Update_Foreground(m_selectLayer);

        Invalidate(FALSE);
    }

    CDialog::OnLButtonDown(nFlags, point);
}

void CDlgVisionProcessingSpec::Ignore_Ball(CRect i_rtSelect, long i_nEditMode)
{
    CRect rtTemp;

    for (auto& ballInfo : m_vecBallInfo)
    {
        if (rtTemp.IntersectRect(i_rtSelect, ballInfo.m_ballPosition))
        {
            if (i_nEditMode == 0)
                m_packageSpec.m_originalballMap->m_balls[ballInfo.m_index].m_ignored = false;
            else if (i_nEditMode == 1)
                m_packageSpec.m_originalballMap->m_balls[ballInfo.m_index].m_ignored = true;
            else
                m_packageSpec.m_originalballMap->m_balls[ballInfo.m_index].m_ignored
                    = !m_packageSpec.m_originalballMap->m_balls[ballInfo.m_index].m_ignored;

            FireExchanged();
        }
    }

    m_packageSpec.CalcDependentVariables(m_pVisionInsp->getScale());
    Calc_DrawingPosition_Ball();
}

void CDlgVisionProcessingSpec::Ignore_Land(CRect i_rtSelect, long i_nEditMode)
{
    CRect rtTemp;

    for (auto& LandInfo : m_packageSpec.m_LandMapConvertOrigin->vecLandData)
    {
        CRect rtSpecROI = Ipvm::ToMFC(LandInfo.fsrtSpecROI.GetCRect());

        if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
        {
            if (i_nEditMode == 0)
                LandInfo.bIgnore = FALSE;
            else if (i_nEditMode == 1)
                LandInfo.bIgnore = TRUE;
            else
                LandInfo.bIgnore = !LandInfo.bIgnore;

            FireExchanged();
        }
    }

    //Visible Data는 주자
    m_packageSpec.SetVisibleData_CurPerOrigin_Land();

    RefreshData_Land();
    Calc_DrawingPosition_Land();
}

void CDlgVisionProcessingSpec::Ignore_Comp(CRect i_rtSelect, long i_nEditMode)
{
    CRect rtTemp;

    for (auto& CompInfo : m_packageSpec.m_CompMapManager->vecCompData)
    {
        CRect rtSpecROI = Ipvm::ToMFC(CompInfo.fsrtROI.GetCRect());

        if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
        {
            if (i_nEditMode == 0)
                CompInfo.bIgnore = FALSE;
            else if (i_nEditMode == 1)
                CompInfo.bIgnore = TRUE;
            else
                CompInfo.bIgnore = !CompInfo.bIgnore;

            FireExchanged();
        }
    }

    //Visible Data는 주자
    m_packageSpec.SetVisibleData_CurPerOrigin();

    RefreshData_Comp();
    Calc_DrawingPosition_Comp();
}

void CDlgVisionProcessingSpec::Ignore_Pad(CRect i_rtSelect, long i_nEditMode)
{
    CRect rtTemp;

    for (auto& PadInfo : m_packageSpec.m_PadMapManager->vecPadData)
    {
        CRect rtSpecROI = Ipvm::ToMFC(PadInfo.fsrtSpecROI.GetCRect());

        if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
        {
            if (i_nEditMode == 0)
                PadInfo.bIgnore = FALSE;
            else if (i_nEditMode == 1)
                PadInfo.bIgnore = TRUE;
            else
                PadInfo.bIgnore = !PadInfo.bIgnore;

            FireExchanged();
        }
    }

    //Visible Data는 주자
    m_packageSpec.SetVisibleData_CurPerOrigin_Pad();

    RefreshData_Pad();
    Calc_DrawingPosition_Pad();
}

void CDlgVisionProcessingSpec::Ignore_CustomFixed(CRect i_rtSelect, long i_nEditMode)
{
    CRect rtTemp;

    for (auto& CustomFixedInfo : m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData)
    {
        CRect rtSpecROI = Ipvm::ToMFC(CustomFixedInfo.m_fsrtSpecROI.GetCRect());

        if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
        {
            if (i_nEditMode == 0)
                CustomFixedInfo.m_bIgnore = FALSE;
            else if (i_nEditMode == 1)
                CustomFixedInfo.m_bIgnore = TRUE;
            else
                CustomFixedInfo.m_bIgnore = !CustomFixedInfo.m_bIgnore;
        }

        FireExchanged();
    }

    //Visible Data는 주자
    m_packageSpec.SetVisibleData_CurPerOrigin_CustomFixed();

    RefreshData_CustomFixed();
    Calc_DrawingPosition_CustomFixed();
}

void CDlgVisionProcessingSpec::Ignore_CustomPolygon(CRect i_rtSelect, long i_nEditMode)
{
    for (auto& CustomPolygonInfo : m_packageSpec.m_CustomPolygonMapManager->vecCustomPolygonData)
    {
        if (CustomPolygonInfo.m_eCustomPolygonCategory
            == CustomPolygonCategory::
                CustomPolygonCategory_Polygon) // CustomInfo.eCustomCategory == CustomCategory::CustomCategory_InkBarrier)
        {
            const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
            const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

            for (auto PolygonPos : CustomPolygonInfo.m_vecfptPointInfo_mm)
            {
                float fLayoutPointPosX = fGridOriginX + (PolygonPos.m_x * 1000.f) * m_fZoom;
                float fLayoutPointPosY = fGridOriginY + (PolygonPos.m_y * 1000.f) * m_fZoom;

                Ipvm::Point32r2 fptLayoutPointPos(fLayoutPointPosX, fLayoutPointPosY);
                CPoint ptPointPos = CPoint((long)(fptLayoutPointPos.m_x + .5f), (long)(fptLayoutPointPos.m_y + .5f));

                if (i_rtSelect.PtInRect(ptPointPos))
                {
                    if (i_nEditMode == 0)
                        CustomPolygonInfo.m_bIgnore = FALSE;
                    else if (i_nEditMode == 1)
                        CustomPolygonInfo.m_bIgnore = TRUE;
                    else
                        CustomPolygonInfo.m_bIgnore = !CustomPolygonInfo.m_bIgnore;

                    break;
                }
            }
        }

        FireExchanged();
    }

    //Visible Data는 주자
    m_packageSpec.SetVisibleData_CurPerOrigin_CustomPolygon();

    RefreshData_CustomPolygon();
    Calc_DrawingPosition_CustomPolygon();
}

void CDlgVisionProcessingSpec::propertyChange_SelectLayer()
{
    long nSelectLayer = m_selectLayer;

    if (nSelectLayer == LayerIndex_Pad)
        Calc_DrawingPosition_Pad();

    if (nSelectLayer == LayerIndex_Ball)
        Calc_DrawingPosition_Ball();

    if (nSelectLayer == LayerIndex_Land)
        Calc_DrawingPosition_Land();

    if (nSelectLayer == LayerIndex_Compnent)
        Calc_DrawingPosition_Comp();

    if (nSelectLayer == LayerIndex_CustomFixed)
        Calc_DrawingPosition_CustomFixed();

    if (nSelectLayer == LayerIndex_CustomPolygon)
        Calc_DrawingPosition_CustomPolygon();

    else if (nSelectLayer == LayerIndex_Whole) //처음에 바로 Whole 들어올때
    {
        Calc_DrawingPosition_Pad();
        Calc_DrawingPosition_Ball();
        Calc_DrawingPosition_Land();
        Calc_DrawingPosition_Comp();
        Calc_DrawingPosition_CustomFixed();
        Calc_DrawingPosition_CustomPolygon();
    }

    Update_Background(nSelectLayer);
    Update_Foreground(nSelectLayer);

    if (nSelectLayer == LayerIndex_Whole) //WholeDevice면 모든 BtnCtrl 비활성화
    {
        m_propertyGrid->FindItem(ePID_LayerImportData)->SetReadOnly(TRUE);
        m_propertyGrid->FindItem(ePID_LayerExportData)->SetReadOnly(TRUE);
        m_propertyGrid->FindItem(ePID_LayerCreateMapData)->SetReadOnly(TRUE);

        m_propertyGrid->FindItem(ePID_LayerFlipX)->SetReadOnly(TRUE);
        m_propertyGrid->FindItem(ePID_LayerFlipY)->SetReadOnly(TRUE);
        m_propertyGrid->FindItem(ePID_SelectionMethod)->SetReadOnly(TRUE);
    }
    else
    {
        m_propertyGrid->FindItem(ePID_LayerImportData)->SetReadOnly(FALSE);
        m_propertyGrid->FindItem(ePID_LayerExportData)->SetReadOnly(FALSE);
        m_propertyGrid->FindItem(ePID_LayerCreateMapData)->SetReadOnly(FALSE);

        m_propertyGrid->FindItem(ePID_LayerFlipX)->SetReadOnly(FALSE);
        m_propertyGrid->FindItem(ePID_LayerFlipY)->SetReadOnly(FALSE);
        m_propertyGrid->FindItem(ePID_SelectionMethod)->SetReadOnly(FALSE);

        ((CXTPPropertyGridItemBool*)m_propertyGrid->FindItem(ePID_LayerFlipX))->SetBool(getCurrentLayer_FlipX());
        ((CXTPPropertyGridItemBool*)m_propertyGrid->FindItem(ePID_LayerFlipY))->SetBool(getCurrentLayer_FlipY());
    }

    if (nSelectLayer == LayerIndex_Compnent || nSelectLayer == LayerIndex_Land)
    {
        m_propertyGrid->FindItem(ePID_RotateComp)->SetReadOnly(FALSE);
    }
    else
    {
        //CompLayer가 아니면
        m_propertyGrid->FindItem(ePID_RotateComp)->SetReadOnly(TRUE);
    }

    Update_Warning(nSelectLayer);

    Invalidate(FALSE);
}

void CDlgVisionProcessingSpec::propertyButton_BodySize()
{
    CBodySizeEditor Dlg(m_packageSpec);

    if (Dlg.DoModal() == IDOK)
    {
        if (Dlg.m_bChangedDeadBug)
            ::AfxMessageBox(_T("Dead bug option has been changed.\nThis option will take effect after job save."));

        FireExchanged();

        long nCurLayer = m_selectLayer;

        Initialize_View();

        switch (nCurLayer)
        {
            case LayerIndex_Pad:
                Calc_DrawingPosition_Pad();
                break;
            case LayerIndex_Ball:
                Calc_DrawingPosition_Ball();
                break;
            case LayerIndex_Land:
                Calc_DrawingPosition_Land();
                break;
            case LayerIndex_Compnent:
                Calc_DrawingPosition_Comp();
                break;
            case LayerIndex_CustomFixed:
                Calc_DrawingPosition_CustomFixed();
                break;
            case LayerIndex_CustomPolygon:
                Calc_DrawingPosition_CustomPolygon();
                break;
        }

        Update_Background(nCurLayer);
        Update_Foreground(nCurLayer);

        Invalidate(FALSE);
    }
}

BOOL& CDlgVisionProcessingSpec::getCurrentLayer_FlipX()
{
    switch (m_selectLayer)
    {
        case LayerIndex_Pad:
            return m_packageSpec.m_OriginPadData->m_bPadMapFlipX;
        case LayerIndex_Ball:
            return m_packageSpec.m_originalballMap->m_ballMapFlipX;
        case LayerIndex_Land:
            return m_packageSpec.m_OriginLandData->m_bLandMapFlipX;
        case LayerIndex_Compnent:
            return m_packageSpec.m_OriginCompData->m_bCompMapFlipX;
        case LayerIndex_CustomFixed:
            return m_packageSpec.m_OriginCustomFixedData->m_bCustomMapFlipX;
        case LayerIndex_CustomPolygon:
            return m_packageSpec.m_OriginCustomPolygonData->m_bCustomMapFlipX;
        case LayerIndex_Whole:
            return m_packageSpec.m_originalballMap->m_ballMapFlipX;
        default:
            break;
    }

    ASSERT(!_T("??"));
    return m_packageSpec.m_originalballMap->m_ballMapFlipX;
}
BOOL& CDlgVisionProcessingSpec::getCurrentLayer_FlipY()
{
    switch (m_selectLayer)
    {
        case LayerIndex_Pad:
            return m_packageSpec.m_OriginPadData->m_bPadMapFlipY;
        case LayerIndex_Ball:
            return m_packageSpec.m_originalballMap->m_ballMapFlipY;
        case LayerIndex_Land:
            return m_packageSpec.m_OriginLandData->m_bLandMapFlipY;
        case LayerIndex_Compnent:
            return m_packageSpec.m_OriginCompData->m_bCompMapFlipY;
        case LayerIndex_CustomFixed:
            return m_packageSpec.m_OriginCustomFixedData->m_bCustomMapFlipY;
        case LayerIndex_CustomPolygon:
            return m_packageSpec.m_OriginCustomPolygonData->m_bCustomMapFlipY;
        case LayerIndex_Whole:
            return m_packageSpec.m_originalballMap->m_ballMapFlipX;
        default:
            break;
    }

    ASSERT(!_T("??"));
    return m_packageSpec.m_originalballMap->m_ballMapFlipY;
}

void CDlgVisionProcessingSpec::Calc_DrawingPosition_Ball()
{
    m_vecBallInfo.clear();

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    for (const auto& ball : m_packageSpec.m_ballMapAll->m_balls)
    {
        const float fCenX = fGridOriginX + (float)(ball.m_posX_um * m_fZoom);
        const float fCenY = fGridOriginY - (float)(ball.m_posY_um * m_fZoom);
        const float fRadius = (float)(0.5 * ball.m_diameter_um * m_fZoom);

        m_vecBallInfo.emplace_back(long(fCenX - fRadius + 0.5f), long(fCenY - fRadius + 0.5f),
            long(fCenX + fRadius + 1.5f), long(fCenY + fRadius + 1.5f), ball.m_ignored, ball.m_index);
    }
}

void CDlgVisionProcessingSpec::Calc_DrawingPosition_Comp()
{
    long nDataCount = (long)m_packageSpec.m_CompMapManager->vecCompData.size();

    if (nDataCount <= 0)
        return;
    else
        RefreshData_Comp();

    Ipvm::Rect32r tmp_sf_Roi;
    float fOffsetX(0.f), fOffsetY(0.f);
    float fWidth(0.f), fLength(0.f);

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    for (long n = 0; n < nDataCount; n++)
    {
        fOffsetX = fGridOriginX + (m_vecCompInfo[n].fOffsetX * 1000.f) * m_fZoom;
        fOffsetY = fGridOriginY - (m_vecCompInfo[n].fOffsetY * 1000.f) * m_fZoom;

        if (m_vecCompInfo[n].nAngle == 0 || m_vecCompInfo[n].nAngle == 180)
        {
            fWidth = (m_vecCompInfo[n].fLength * 1000.f) * m_fZoom * 0.5f;
            fLength = (m_vecCompInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
        }
        else
        {
            fWidth = (m_vecCompInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
            fLength = (m_vecCompInfo[n].fLength * 1000.f) * m_fZoom * 0.5f;
        }

        tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
        tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
        tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
        tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);

        FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

        m_vecCompInfo[n].fsrtROI = frtChip; //SpecPos
    }

    UpdateMapPosition();
}

void CDlgVisionProcessingSpec::Calc_DrawingPosition_Pad()
{
    long nDataCount = (long)m_packageSpec.m_PadMapManager->vecPadData.size();

    if (nDataCount <= 0)
        return;
    else
        RefreshData_Pad();

    Ipvm::Rect32s rtROI;
    Ipvm::Rect32r tmp_sf_Roi;
    float fWidth(0.f), fLength(0.f);

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    for (long n = 0; n < nDataCount; n++)
    {
        float fOffsetX = fGridOriginX + (m_vecPadInfo[n].GetCenter().m_x * 1000.f) * m_fZoom;
        float fOffsetY = fGridOriginY - (m_vecPadInfo[n].GetCenter().m_y * 1000.f) * m_fZoom;

        if (m_vecPadInfo[n].GetType() == _typeTriangle || m_vecPadInfo[n].GetType() == _typePinIndex)
        {
            if (m_vecPadInfo[n].nAngle == 0 || m_vecPadInfo[n].nAngle == 180)
            {
                fWidth = (m_vecPadInfo[n].fLength * 1000.f) * m_fZoom * 0.5f;
                fLength = (m_vecPadInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
            }
            else
            {
                fWidth = (m_vecPadInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
                fLength = (m_vecPadInfo[n].fLength * 1000.f) * m_fZoom * 0.5f;
            }
        }
        else if (m_vecPadInfo[n].GetType() == _typeCircle)
        {
            fWidth = (m_vecPadInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
            fLength = (m_vecPadInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
        }
        else if (m_vecPadInfo[n].GetType() == _typeRectangle)
        {
            fWidth = (m_vecPadInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
            fLength = (m_vecPadInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
        }

        rtROI.m_left = (long)(fOffsetX - fWidth + 0.5f);
        rtROI.m_right = (long)(fOffsetX + fWidth + 0.5f);
        rtROI.m_top = (long)(fOffsetY - fLength + 0.5f);
        rtROI.m_bottom = (long)(fOffsetY + fLength + 0.5f);
        m_vecPadInfo[n].rtROI = rtROI;

        tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
        tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
        tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
        tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);

        FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

        m_vecPadInfo[n].fsrtSpecROI = frtChip;
        if (m_vecPadInfo[n].GetType() == _typeTriangle || m_vecPadInfo[n].GetType() == _typePinIndex)
        {
            for (long i = 0; i < 3; i++)
                m_ptPinIndex[i] = CPoint(0, 0);

            if (!m_vecPadInfo[n].bIgnore)
                GetPinIndexPoint(frtChip, m_ptPinIndex);
        }
    }

    UpdateMapPosition_Pad();
}

void CDlgVisionProcessingSpec::Calc_DrawingPosition_Land()
{
    long nDataCount = (long)m_packageSpec.m_LandMapConvertOrigin->vecLandData.size();

    if (nDataCount <= 0)
        return;
    else
        RefreshData_Land();

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    Ipvm::Rect32s rtROI;
    Ipvm::Rect32r tmp_sf_Roi;
    float fOffsetX(0.f), fOffsetY(0.f);
    float fWidth(0.f), fLength(0.f), fAngle(0.f);

    for (long n = 0; n < nDataCount; n++)
    {
        fOffsetX = fGridOriginX + (m_vecLandInfo[n].fOffsetX * 1000.f) * m_fZoom;
        fOffsetY = fGridOriginY - (m_vecLandInfo[n].fOffsetY * 1000.f) * m_fZoom;

        fWidth = (m_vecLandInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
        fLength = (m_vecLandInfo[n].fLength * 1000.f) * m_fZoom * 0.5f;
        fAngle = (float)m_vecLandInfo[n].nAngle;

        rtROI.m_left = (long)(fOffsetX - fWidth + 0.5f);
        rtROI.m_right = (long)(fOffsetX + fWidth + 0.5f);
        rtROI.m_top = (long)(fOffsetY - fLength + 0.5f);
        rtROI.m_bottom = (long)(fOffsetY + fLength + 0.5f);
        m_vecLandInfo[n].rtROI = rtROI;

        tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
        tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
        tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
        tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);

        FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

        if (fAngle != 0)
        {
            double lfRadian = (double)fAngle * DEF_DEG_TO_RAD;

            // 시계 반대 방향이 +
            frtChip = frtChip.Rotate(lfRadian);
        }

        m_vecLandInfo[n].fsrtSpecROI = frtChip;
    }

    UpdateMapPosition_Land();
}

void CDlgVisionProcessingSpec::Calc_DrawingPosition_CustomFixed()
{
    long nDataCount = (long)m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData.size();

    if (nDataCount <= 0)
        return;
    else
        RefreshData_CustomFixed();

    Ipvm::Rect32r tmp_sf_Roi(0.f, 0.f, 0.f, 0.f);
    float fWidth(0.f), fLength(0.f);

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    for (long n = 0; n < nDataCount; n++)
    {
        float fOffsetX = fGridOriginX + (m_vecCustomFixedInfo[n].m_fSpecCenterPos_mm.m_x * 1000.f) * m_fZoom;
        float fOffsetY = fGridOriginY - (m_vecCustomFixedInfo[n].m_fSpecCenterPos_mm.m_y * 1000.f) * m_fZoom;

        if (m_vecCustomFixedInfo[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Circle
            || m_vecCustomFixedInfo[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_NotchHole)
        {
            fWidth = (m_vecCustomFixedInfo[n].m_fSpecWidth_mm * 1000.f) * m_fZoom * 0.5f;
            fLength = (m_vecCustomFixedInfo[n].m_fSpecLength_mm * 1000.f) * m_fZoom * 0.5f;

            //if (fWidth != fLength)
            //{
            //	::SimpleMessage(_T("Shpae Type is Circle or NotchHole Width, Length Mismatching"));
            //	m_vecCustomFixedInfo.clear();
            //	return;
            //}

            m_vecCustomFixedInfo[n].m_fSpec_ellipse_mm = Ipvm::EllipseEq32r(fOffsetX, fOffsetY, fWidth, fLength);
        }
        else if (m_vecCustomFixedInfo[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Rectangle)
        {
            fWidth = (m_vecCustomFixedInfo[n].m_fSpecWidth_mm * 1000.f) * m_fZoom * 0.5f;
            fLength = (m_vecCustomFixedInfo[n].m_fSpecLength_mm * 1000.f) * m_fZoom * 0.5f;
        }

        tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
        tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
        tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
        tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);

        FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
            Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

        m_vecCustomFixedInfo[n].m_fsrtSpecROI = frtChip;
    }

    UpdateMapPosition_CustomFixed();
}

void CDlgVisionProcessingSpec::Calc_DrawingPosition_CustomPolygon()
{
    long nDataCount = (long)m_packageSpec.m_CustomPolygonMapManager->vecCustomPolygonData.size();

    if (nDataCount <= 0)
        return;
    else
        RefreshData_CustomPolygon();

    Ipvm::Rect32r tmp_sf_Roi(0.f, 0.f, 0.f, 0.f);

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    for (long n = 0; n < nDataCount; n++)
    {
        if (m_vecCustomPolygonInfo[n].m_eCustomPolygonCategory
            == CustomPolygonCategory::
                CustomPolygonCategory_Polygon) //|| m_vecCustomInfo[n].eCustomCategory == CustomCategory::CustomCategory_InkBarrier)
        {
            for (auto& ptPolygonPos : m_vecCustomPolygonInfo[n].m_vecfptPointInfo_mm)
            {
                ptPolygonPos.m_x = fGridOriginX + (ptPolygonPos.m_x * 1000.f) * m_fZoom;
                ptPolygonPos.m_y = fGridOriginY - (ptPolygonPos.m_y * 1000.f) * m_fZoom;
            }
        }
    }

    UpdateMapPosition_CustomPolygon();
}

void CDlgVisionProcessingSpec::OnClose()
{
    HWND hWnd = ::FindWindow(NULL, _T("IntekPlus Gerber Extractor"));

    if (hWnd != NULL)
    {
        ::PostMessage(hWnd, WM_CLOSE, 0, 0);
    }

    m_propertyGrid->DestroyWindow();

    CDialog::OnClose();
}

void CDlgVisionProcessingSpec::RefreshData_Ball()
{
    m_packageSpec.CalcDependentVariables(m_pVisionInsp->getScale());

    long nCurLayer = m_selectLayer;

    Initialize_View();

    Calc_DrawingPosition_Ball();

    Update_Background(nCurLayer);
    Update_Foreground(nCurLayer);

    Invalidate(FALSE);
}

void CDlgVisionProcessingSpec::RefreshData_Pad()
{
    m_vecPadInfo = m_packageSpec.m_PadMapManager->vecPadData;
}

void CDlgVisionProcessingSpec::RefreshData_Land()
{
    long vecMapSize = (long)m_vecLandInfo.size();

    if (vecMapSize > 0)
    {
        m_vecLandInfo.clear();
        m_vecLandInfo.resize(vecMapSize);
        m_vecLandInfo = m_packageSpec.m_LandMapConvertOrigin->vecLandData;
    }
    else
    {
        m_vecLandInfo.resize(m_packageSpec.m_LandMapConvertOrigin->GetCount());
        m_vecLandInfo = m_packageSpec.m_LandMapConvertOrigin->vecLandData;
    }
}

void CDlgVisionProcessingSpec::RefreshData_Comp()
{
    m_vecCompInfo = m_packageSpec.m_CompMapManager->vecCompData;
}

void CDlgVisionProcessingSpec::RefreshData_CustomFixed()
{
    m_vecCustomFixedInfo = m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData;
}

void CDlgVisionProcessingSpec::RefreshData_CustomPolygon()
{
    m_vecCustomPolygonInfo = m_packageSpec.m_CustomPolygonMapManager->vecCustomPolygonData;
}

void CDlgVisionProcessingSpec::UpdateMapPosition()
{
    long vecMapSize = m_packageSpec.m_CompMapManager->GetCount();
    if (vecMapSize > 0)
    {
        for (long n = 0; n < vecMapSize; n++)
            m_packageSpec.m_CompMapManager->vecCompData[n].fsrtROI = m_vecCompInfo[n].fsrtROI;
    }

    m_packageSpec.CreateDMSInfo();
}

void CDlgVisionProcessingSpec::UpdateMapPosition_Pad()
{
    long vecMapSize = m_packageSpec.m_PadMapManager->GetCount();

    if (vecMapSize > 0)
    {
        for (long n = 0; n < vecMapSize; n++)
        {
            m_packageSpec.m_PadMapManager->vecPadData[n].rtROI = m_vecPadInfo[n].rtROI;
            m_packageSpec.m_PadMapManager->vecPadData[n].fsrtSpecROI = m_vecPadInfo[n].fsrtSpecROI;
        }
    }
}

void CDlgVisionProcessingSpec::UpdateMapPosition_Land()
{
    long vecMapSize = m_packageSpec.m_LandMapConvertOrigin->GetCount();

    if (vecMapSize > 0)
    {
        for (long n = 0; n < vecMapSize; n++)
        {
            m_packageSpec.m_LandMapConvertOrigin->vecLandData[n].rtROI = m_vecLandInfo[n].rtROI;
            m_packageSpec.m_LandMapConvertOrigin->vecLandData[n].fsrtSpecROI = m_vecLandInfo[n].fsrtSpecROI;
        }
    }
    m_packageSpec.UpdateLandData();
}

void CDlgVisionProcessingSpec::UpdateMapPosition_CustomFixed()
{
    long vecMapSize = m_packageSpec.m_CustomFixedMapManager->GetCount();

    if (vecMapSize > 0)
    {
        for (long n = 0; n < vecMapSize; n++)
        {
            m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData[n].m_fSpec_ellipse_mm
                = m_vecCustomFixedInfo[n].m_fSpec_ellipse_mm;
            m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData[n].m_fsrtSpecROI
                = m_vecCustomFixedInfo[n].m_fsrtSpecROI;
        }
    }
}

void CDlgVisionProcessingSpec::UpdateMapPosition_CustomPolygon()
{
}

void CDlgVisionProcessingSpec::propertyChange_FlipX()
{
    long nCurSelectLayer = m_selectLayer;

    switch (nCurSelectLayer)
    {
        case LayerIndex_Pad:
            m_packageSpec.m_PadMapManager->MirrorMap();
            RefreshData_Pad();
            Calc_DrawingPosition_Pad();
            break;
        case LayerIndex_Ball:
            RefreshData_Ball();
            break;
        case LayerIndex_Land:
            m_packageSpec.m_LandMapConvertOrigin->MirrorMap();
            m_packageSpec.UpdateLandData();
            RefreshData_Land();
            Calc_DrawingPosition_Land();
            break;
        case LayerIndex_Compnent:
            m_packageSpec.m_CompMapManager->MirrorMap();
            RefreshData_Comp();
            Calc_DrawingPosition_Comp();
            break;
        case LayerIndex_CustomFixed:
            m_packageSpec.m_CustomFixedMapManager->MirrorMap();
            RefreshData_CustomFixed();
            Calc_DrawingPosition_CustomFixed();
            break;
        case LayerIndex_CustomPolygon:
            m_packageSpec.m_CustomPolygonMapManager->MirrorMap();
            RefreshData_CustomPolygon();
            Calc_DrawingPosition_CustomPolygon();
            break;

        default:
            break;
    }

    Update_Background(nCurSelectLayer);
    Update_Foreground(nCurSelectLayer);

    Invalidate(FALSE);
}

void CDlgVisionProcessingSpec::propertyChange_FlipY()
{
    long nCurSelectLayer = m_selectLayer;

    switch (nCurSelectLayer)
    {
        case LayerIndex_Pad:
            m_packageSpec.m_PadMapManager->FlipMap();
            RefreshData_Pad();
            Calc_DrawingPosition_Pad();
            break;
        case LayerIndex_Ball:
            RefreshData_Ball();
            break;
        case LayerIndex_Land:
            m_packageSpec.m_LandMapConvertOrigin->FlipMap();
            m_packageSpec.UpdateLandData();
            RefreshData_Land();
            Calc_DrawingPosition_Land();
            break;
        case LayerIndex_Compnent:
            m_packageSpec.m_CompMapManager->FlipMap();
            RefreshData_Comp();
            Calc_DrawingPosition_Comp();
            break;
        case LayerIndex_CustomFixed:
            m_packageSpec.m_CustomFixedMapManager->FlipMap();
            RefreshData_CustomFixed();
            Calc_DrawingPosition_CustomFixed();
            break;
        case LayerIndex_CustomPolygon:
            m_packageSpec.m_CustomPolygonMapManager->FlipMap();
            RefreshData_CustomPolygon();
            Calc_DrawingPosition_CustomPolygon();
            break;
        default:
            break;
    }

    Update_Background(nCurSelectLayer);
    Update_Foreground(nCurSelectLayer);

    Invalidate(FALSE);
}

void CDlgVisionProcessingSpec::propertyChange_Rotation()
{
    long nRotateAngleIndex = long(m_packageSpec.m_originalballMap->m_ballMapRotation);

    //Pad
    m_packageSpec.m_PadMapManager->RotateMap(nRotateAngleIndex);
    RefreshData_Pad();
    Calc_DrawingPosition_Pad();

    //Ball
    RefreshData_Ball();

    //Land
    m_packageSpec.m_LandMapConvertOrigin->RotateMap(nRotateAngleIndex);
    m_packageSpec.UpdateLandData();
    RefreshData_Land();
    Calc_DrawingPosition_Land();

    //Comp
    m_packageSpec.m_CompMapManager->RotateMap(nRotateAngleIndex);
    RefreshData_Comp();
    Calc_DrawingPosition_Comp();

    //Custom Fixed
    m_packageSpec.m_CustomFixedMapManager->RotateMap(nRotateAngleIndex);
    RefreshData_CustomFixed();
    Calc_DrawingPosition_CustomFixed();

    //Custom Polygon
    m_packageSpec.m_CustomPolygonMapManager->RotateMap(nRotateAngleIndex);
    RefreshData_CustomPolygon();
    Calc_DrawingPosition_CustomPolygon();

    long nCurSelectLayer = m_selectLayer;
    Update_Background(nCurSelectLayer);
    Update_Foreground(nCurSelectLayer);

    Invalidate(FALSE);
}

void CDlgVisionProcessingSpec::propertyButton_ImportMapData()
{
    CFileDialog dlg(TRUE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Comma separated values file format (*.csv)|*.csv||"));

    if (dlg.DoModal() != IDOK)
        return;

    FireExchanged();

    long nCurComboIndex = m_selectLayer;
    long nRotateCurSel = long(m_packageSpec.m_originalballMap->m_ballMapRotation);
    long nRotateCompCurSel = m_packageSpec.m_OriginCompData->m_nRotateCompIdx;
    CString strErrorMessage;

    if (nCurComboIndex == LayerIndex_Pad)
    {
        if (!m_packageSpec.m_OriginPadData->LoadMap(dlg.GetPathName(), strErrorMessage))
        {
            SimpleMessage(strErrorMessage, MB_OK);
            return;
        }
        m_packageSpec.SetPadData();
        m_packageSpec.m_PadMapManager->RotateMap(nRotateCurSel);
        RefreshData_Pad();
        Calc_DrawingPosition_Pad();
    }

    if (nCurComboIndex == LayerIndex_Ball)
    {
        m_packageSpec.m_originalballMap->Import(dlg.GetPathName(), strErrorMessage);
        if (!strErrorMessage.IsEmpty())
        {
            SimpleMessage(strErrorMessage, MB_OK);
            return;
        }

        m_packageSpec.m_originalballMap->m_ballMapRotation
            = Package::BallCollectionOriginal::RotationIndex(m_packageSpec.m_OriginPadData->m_nRotateIdx);
        RefreshData_Ball();
    }

    if (nCurComboIndex == LayerIndex_Land)
    {
        if (!m_packageSpec.m_OriginLandData->LoadMap(dlg.GetPathName(), strErrorMessage))
        {
            SimpleMessage(strErrorMessage, MB_OK);
            return;
        }
        m_packageSpec.SetLandData();
        m_packageSpec.m_LandMapManager->RotateMap(nRotateCurSel);
        m_packageSpec.m_LandMapManager->RotateLand(nRotateCompCurSel);
        RefreshData_Land();
        Calc_DrawingPosition_Land();
    }

    if (nCurComboIndex == LayerIndex_Compnent)
    {
        if (!m_packageSpec.m_OriginCompData->LoadMap(dlg.GetPathName(), strErrorMessage))
        {
            SimpleMessage(strErrorMessage, MB_OK);
            return;
        }
        m_packageSpec.SetCompData();
        m_packageSpec.m_CompMapManager->RotateMap(nRotateCurSel);
        m_packageSpec.m_CompMapManager->RotateComp(nRotateCompCurSel);
        RefreshData_Comp();

        //{{//kircheis_Comp3DHeightBug
        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
        {
            long nTotalCompNum(0), nErrorCompNum(0);
            CString strErrorMSG;
            if (m_packageSpec.m_OriginCompData->CheckIntegrityOfCompMapData(nErrorCompNum, nTotalCompNum) == false)
            {
                strErrorMSG.Format(_T("%d data of the component map data (width, length, height) has a ")
                                   _T("problem.\nPlease check the map data."),
                    nErrorCompNum);
                SimpleMessage(strErrorMSG);
            }

            strErrorMSG.Empty();
        }
        //}}

        //ChipDB Check
        /*if (!CompDBVerify())
		{
			m_packageSpec.m_OriginCompData->vecOriginCompData.clear();
			RefreshData_Comp();
			::SimpleMessage(_T("Fail to Component DataList. \n Please Setup to Component DataList."));
			return;
		}		*/

        Calc_DrawingPosition_Comp();
    }

    if (nCurComboIndex == LayerIndex_CustomFixed)
    {
        if (!m_packageSpec.m_OriginCustomFixedData->LoadMap(dlg.GetPathName(), strErrorMessage))
        {
            SimpleMessage(strErrorMessage, MB_OK);
            return;
        }

        m_packageSpec.SetCustomFixedData();
        m_packageSpec.m_CustomFixedMapManager->RotateMap(nRotateCurSel);
        RefreshData_CustomFixed();
        Calc_DrawingPosition_CustomFixed();
    }

    if (nCurComboIndex == LayerIndex_CustomPolygon)
    {
        if (!m_packageSpec.m_OriginCustomPolygonData->LoadMap(dlg.GetPathName(), strErrorMessage))
        {
            SimpleMessage(strErrorMessage, MB_OK);
            return;
        }

        m_packageSpec.SetCustomPolygonData();
        m_packageSpec.m_CustomPolygonMapManager->RotateMap(nRotateCurSel);
        RefreshData_CustomPolygon();
        Calc_DrawingPosition_CustomPolygon();
    }

    Update_Background(nCurComboIndex);
    Update_Foreground(nCurComboIndex);
    Update_Warning(nCurComboIndex);

    Invalidate(FALSE);

    strErrorMessage.Empty();
}

void CDlgVisionProcessingSpec::propertyButton_CreateMapData()
{
    if (m_selectLayer != LayerIndex_Ball)
    {
        return;
    }

    BgaGridGeneratorDlg dlg;

    if (dlg.DoModal() != IDOK)
        return;

    FireExchanged();

    CString errorString;

    Package::BallCollectionOriginal bumpMapSource(dlg.m_gridCountX, dlg.m_gridCountY, dlg.m_gridPitchX_um,
        dlg.m_gridPitchY_um, dlg.m_ballDiameter_um, dlg.m_ballHeight_um, dlg.m_bIgnore, errorString);

    if (!errorString.IsEmpty())
    {
        ::AfxMessageBox(errorString, MB_ICONERROR | MB_OK);
        return;
    }

    m_packageSpec.m_originalballMap = &bumpMapSource; //BUG: 지역 변수의 주소를 넘기고 있음
    m_packageSpec.m_originalballMap->m_ballMapRotation
        = Package::BallCollectionOriginal::RotationIndex(m_packageSpec.m_OriginPadData->m_nRotateIdx);
    RefreshData_Ball();

    Update_Background(m_selectLayer);
    Update_Foreground(m_selectLayer);
    Update_Warning(m_selectLayer);

    Invalidate(FALSE);

    errorString.Empty();
}

BOOL CDlgVisionProcessingSpec::CompDBVerify()
{
    std::vector<std::vector<CString>> vec2strCompType;
    vec2strCompType.resize(_typeComponentEnd);

    //LoadCompTypeInCompDB(vec2strCompType);

    std::vector<Package::Component> vecExistCompData;
    std::vector<CString> existCompType;

    for (auto CompData : m_vecCompInfo)
    {
        switch (CompData.nCompType)
        {
            case _typeChip:
            {
                BOOL bChipDBCheck(FALSE);
                if (vec2strCompType[_typeChip].size() > 0)
                {
                    for (auto& ChipData : vec2strCompType[_typeChip])
                    {
                        if (CompData.strCompType == ChipData)
                        {
                            bChipDBCheck = TRUE;
                            break;
                        }
                    }
                }
                if (!bChipDBCheck)
                {
                    BOOL bChipOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType)
                        != existCompType.end();
                    if (!bChipOverlapValue)
                    {
                        existCompType.push_back(CompData.strCompType);
                        vecExistCompData.push_back(CompData); //없는게 들어가는 정보
                    }
                }
                break;
            }
            case _typePassive:
            {
                BOOL bPassiveDBCheck(FALSE);
                if (vec2strCompType[_typePassive].size() > 0)
                {
                    for (auto PassiveData : vec2strCompType[_typePassive])
                    {
                        if (CompData.strCompType == PassiveData)
                        {
                            bPassiveDBCheck = TRUE;
                            break;
                        }
                    }
                }

                if (!bPassiveDBCheck)
                {
                    BOOL bPassiveOverlapValue
                        = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType)
                        != existCompType.end();
                    if (!bPassiveOverlapValue)
                    {
                        existCompType.push_back(CompData.strCompType);
                        vecExistCompData.push_back(CompData); //없는게 들어가는 정보
                    }
                }
                break;
            }
            case _typeArray:
            {
                BOOL bArrayDBCheck(FALSE);
                if (vec2strCompType[_typeArray].size() > 0)
                {
                    for (auto& ArrayData : vec2strCompType[_typeArray])
                    {
                        if (CompData.strCompType == ArrayData)
                        {
                            bArrayDBCheck = TRUE;
                            break;
                        }
                    }
                }
                if (!bArrayDBCheck)
                {
                    BOOL bArrayOverlapValue
                        = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType)
                        != existCompType.end();
                    if (!bArrayOverlapValue)
                    {
                        existCompType.push_back(CompData.strCompType);
                        vecExistCompData.push_back(CompData); //없는게 들어가는 정보
                    }
                }
                break;
            }
            case _typeHeatSync:
            {
                BOOL bHeatSyncDBCheck(FALSE);
                if (vec2strCompType[_typeHeatSync].size() > 0)
                {
                    for (auto& HeatSyncData : vec2strCompType[_typeHeatSync])
                    {
                        if (CompData.strCompType == HeatSyncData)
                        {
                            bHeatSyncDBCheck = TRUE;
                            break;
                        }
                    }
                }
                if (!bHeatSyncDBCheck)
                {
                    BOOL bHeatSyncOverlapValue
                        = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType)
                        != existCompType.end();
                    if (!bHeatSyncOverlapValue)
                    {
                        existCompType.push_back(CompData.strCompType);
                        vecExistCompData.push_back(CompData); //없는게 들어가는 정보
                    }
                }
                break;
            }
            case _typePatch: //kircheis_POI
            {
                BOOL bPatchDBCheck(FALSE);
                if (vec2strCompType[_typePatch].size() > 0)
                {
                    for (auto& PatchData : vec2strCompType[_typePatch])
                    {
                        if (CompData.strCompType == PatchData)
                        {
                            bPatchDBCheck = TRUE;
                            break;
                        }
                    }
                }
                //if (!bPatchDBCheck)
                //{
                //	BOOL bHeatSyncOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType) != existCompType.end();
                //	if (!bHeatSyncOverlapValue)
                //	{
                //		existCompType.push_back(CompData.strCompType);
                //		vecExistCompData.push_back(CompData); //없는게 들어가는 정보
                //	}
                //}
                break;
            }
        }
    }

    //없는걸 다 찾았다면
    long nExistCompDataNum = (long)vecExistCompData.size();
    if (nExistCompDataNum > 0)
    {
        CString strMsg;
        strMsg.Format(_T("%dData does Not Exist."), nExistCompDataNum);
        SimpleMessage((strMsg), MB_OK);

        Chip::DMSChip DmsChipInfo;
        Chip::DMSPassiveChip DMSPassiveInfo;
        Chip::DMSArrayChip DmsArrayInfo;
        Chip::DMSHeatsink DmsHeatsink;
        Chip::DMSPatch DmsPatch; //kircheis_POI
        Chip::DMSBall DmsBallInfo;

        VisionMapDataEditorUI VisionEditorDlg(GetSafeHwnd(), &m_packageSpec, &DmsChipInfo, &DMSPassiveInfo,
            &DmsArrayInfo, &DmsHeatsink, &DmsPatch, &DmsBallInfo, vecExistCompData, TRUE);

        if (VisionEditorDlg.DoModal() == IDOK)
            ::AfxMessageBox(_T("Complete"));
        else
            return FALSE;
    }

    for (int nIdx = 0; nIdx < vec2strCompType.size(); nIdx++)
    {
        for (int nIdx2 = 0; nIdx2 < vec2strCompType[nIdx].size(); nIdx2++)
        {
            vec2strCompType[nIdx][nIdx2].Empty();
        }
    }

    for (int nIdx = 0; nIdx < existCompType.size(); nIdx++)
    {
        existCompType[nIdx].Empty();
    }
    return TRUE;
}

//BOOL CDlgVisionProcessingSpec::LoadCompTypeInCompDB(std::vector<std::vector<CString>> &o_vec2strCompType)
//{
//	if (o_vec2strCompType.size() <= 0)
//		return FALSE;
//
//	//Chip
//	CString strChipDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Chip.ini"));
//	int nChipTypeNum = IPVM::IniHelper::LoadINT(strChipDB, _T("Chip"), _T("Chip Type Num"), 0);
//	o_vec2strCompType[_typeChip].resize(nChipTypeNum);
//	for (long nType = 0; nType < nChipTypeNum; nType++)
//	{
//		CString strSection;
//		strSection.Format(_T("Chip_%d"), nType);
//		o_vec2strCompType[_typeChip][nType] = IPVM::IniHelper::LoadSTRING(strChipDB, strSection, _T("Type Name"), _T(""));
//	}
//
//	//Passive
//	CString strPassiveDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Passive.ini"));
//	int nPaasiveTypeNum = IPVM::IniHelper::LoadINT(strPassiveDB, _T("PassiveChip"), _T("Chip Type Num"), 0);
//	o_vec2strCompType[_typePassive].resize(nPaasiveTypeNum);
//	for (long nType = 0; nType < nPaasiveTypeNum; nType++)
//	{
//		CString strSection;
//		strSection.Format(_T("PassiveChip_%d"), nType);
//		o_vec2strCompType[_typePassive][nType] = IPVM::IniHelper::LoadSTRING(strPassiveDB, strSection, _T("Type Name"), _T(""));
//	}
//
//	//Array
//	CString strArrayDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Array.ini"));
//	int nArrayTypeNum = IPVM::IniHelper::LoadINT(strArrayDB, _T("ArrayChip"), _T("Chip Type Num"), 0);
//	o_vec2strCompType[_typeArray].resize(nArrayTypeNum);
//	for (long nType = 0; nType < nArrayTypeNum; nType++)
//	{
//		CString strSection;
//		strSection.Format(_T("ArrayChip_%d"), nType);
//		o_vec2strCompType[_typeArray][nType] = IPVM::IniHelper::LoadSTRING(strArrayDB, strSection, _T("Type Name"), _T(""));
//	}
//
//	//HeatSync
//	CString strHeatSyncDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Heatsink.ini"));
//	int nHeatSyncTypeNum = IPVM::IniHelper::LoadINT(strHeatSyncDB, _T("Heatsink"), _T("Heatsink Type Num"), 0);
//	o_vec2strCompType[_typeHeatSync].resize(nHeatSyncTypeNum);
//	for (long nType = 0; nType < nHeatSyncTypeNum; nType++)
//	{
//		CString strSection;
//		strSection.Format(_T("Heatsink_%d"), nType);
//		o_vec2strCompType[_typeHeatSync][nType] = IPVM::IniHelper::LoadSTRING(strHeatSyncDB, strSection, _T("Type Name"), _T(""));
//	}
//
//	return TRUE;
//}

void CDlgVisionProcessingSpec::propertyButton_ExportMapData()
{
    CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("Comma separated values file format (*.csv)|*.csv||"));

    if (dlg.DoModal() != IDOK)
        return;

    long nCurSelectLayer = m_selectLayer;

    switch (nCurSelectLayer)
    {
        case LayerIndex_Pad:
            m_packageSpec.m_OriginPadData->SaveMap(dlg.GetPathName());
            break;
        case LayerIndex_Ball:
            m_packageSpec.m_originalballMap->Export(dlg.GetPathName());
            break;
        case LayerIndex_Land:
            m_packageSpec.m_OriginLandData->SaveMap(dlg.GetPathName());
            break;
        case LayerIndex_Compnent:
            m_packageSpec.m_OriginCompData->SaveMap(dlg.GetPathName());
            break;
        case LayerIndex_CustomFixed:
            m_packageSpec.m_OriginCustomFixedData->SaveMap(dlg.GetPathName());
            break;
        case LayerIndex_CustomPolygon:
            m_packageSpec.m_OriginCustomPolygonData->SaveMap(dlg.GetPathName());
            break;
        default:
            break;
    }
}

void CDlgVisionProcessingSpec::propertyChange_RotateComp()
{
    long nCurSelectLayer = m_selectLayer;
    long nRotateCompAngle = m_packageSpec.m_OriginCompData->m_nRotateCompIdx;

    if (nCurSelectLayer == LayerIndex_Compnent)
    {
        m_packageSpec.m_CompMapManager->RotateComp(nRotateCompAngle);
        RefreshData_Comp();
        Calc_DrawingPosition_Comp();
    }

    if (nCurSelectLayer == LayerIndex_Land)
    {
        m_packageSpec.m_LandMapConvertOrigin->RotateLand(nRotateCompAngle);
        m_packageSpec.UpdateLandData();
        RefreshData_Land();
        Calc_DrawingPosition_Land();
    }

    Update_Background(nCurSelectLayer);
    Update_Foreground(nCurSelectLayer);

    Invalidate(FALSE);
}

void CDlgVisionProcessingSpec::propertyButton_ClearMapData()
{
    long nCurLayerIndex = m_selectLayer;
    CString strWarningMsg;

    switch (nCurLayerIndex)
    {
        case LayerIndex_Pad:
            strWarningMsg = _T("[Warning] Pad map data will be removed.. Are you sure you want to delete?");
            if (::SimpleMessage(strWarningMsg, MB_OKCANCEL) == IDOK)
            {
                m_packageSpec.m_OriginPadData->vecOriginPadData.clear();
                FireExchanged();
            }
            else
                return;
            break;
        case LayerIndex_Ball:
            strWarningMsg = _T("[Warning] Ball map data will be removed.. Are you sure you want to delete?");
            if (::SimpleMessage(strWarningMsg, MB_OKCANCEL) == IDOK)
            {
                m_packageSpec.m_originalballMap->m_balls.clear();
                FireExchanged();
            }
            else
                return;
            break;
        case LayerIndex_Land:
            strWarningMsg = _T("[Warning] Land map data will be removed.. Are you sure you want to delete?");
            if (::SimpleMessage(strWarningMsg, MB_OKCANCEL) == IDOK)
            {
                m_packageSpec.m_OriginLandData->vecOriginLandData.clear();
                FireExchanged();
            }
            else
                return;
            break;
        case LayerIndex_Compnent:
            strWarningMsg = _T("[Warning] Component map data will be removed.. Are you sure you want to delete?");
            if (::SimpleMessage(strWarningMsg, MB_OKCANCEL) == IDOK)
            {
                m_packageSpec.m_OriginCompData->vecOriginCompData.clear();
                FireExchanged();
            }
            else
                return;
            break;
        case LayerIndex_CustomFixed:
            strWarningMsg = _T("[Warning] Custom fixed map data will be removed.. Are you sure you want to delete?");
            if (::SimpleMessage(strWarningMsg, MB_OKCANCEL) == IDOK)
            {
                m_packageSpec.m_OriginCustomFixedData->vecOriginCustomFixedData.clear();
                FireExchanged();
            }
            else
                return;
            break;
        case LayerIndex_CustomPolygon:
            strWarningMsg = _T("[Warning] Custom polygon map data will be removed.. Are you sure you want to delete?");
            if (::SimpleMessage(strWarningMsg, MB_OKCANCEL) == IDOK)
            {
                m_packageSpec.m_OriginCustomPolygonData->vecOriginCustomPolygonData.clear();
                FireExchanged();
            }
            else
                return;
            break;

        case LayerIndex_Whole:
            strWarningMsg = _T("[Warning] All map data will be removed.. Are you sure you want to delete?");
            if (::SimpleMessage(strWarningMsg, MB_OKCANCEL) == IDOK)
            {
                m_packageSpec.m_originalballMap->m_balls.clear();
                m_packageSpec.m_OriginLandData->vecOriginLandData.clear();
                m_packageSpec.m_OriginCompData->vecOriginCompData.clear();
                m_packageSpec.m_OriginPadData->vecOriginPadData.clear();
                m_packageSpec.m_OriginCustomFixedData->vecOriginCustomFixedData.clear();
                m_packageSpec.m_OriginCustomPolygonData->vecOriginCustomPolygonData.clear();
                FireExchanged();
            }
            else
                return;
            break;

        default:
            break;
    }

    RefreshData(nCurLayerIndex);
    Update_Background(nCurLayerIndex);
    Update_Foreground(nCurLayerIndex);

    Invalidate(FALSE);

    strWarningMsg.Empty();
}

BOOL CDlgVisionProcessingSpec::RefreshData(long i_nLayerIndex)
{
    if (i_nLayerIndex < 0) //혹시몰라 예외처리.. 음수값이면 쓰레기값이다.
        return FALSE;

    bool allRefresh = (i_nLayerIndex == LayerIndex_Whole);
    if (i_nLayerIndex == LayerIndex_Ball || allRefresh)
    {
        RefreshData_Ball();
    }

    if (i_nLayerIndex == LayerIndex_Land || allRefresh)
    {
        m_packageSpec.SetLandData();
        Calc_DrawingPosition_Land();
        RefreshData_Land();
    }

    if (i_nLayerIndex == LayerIndex_Compnent || allRefresh)
    {
        m_packageSpec.SetCompData();
        Calc_DrawingPosition_Comp();
        RefreshData_Comp();
    }

    if (i_nLayerIndex == LayerIndex_Pad || allRefresh)
    {
        for (long i = 0; i < 3; i++)
            m_ptPinIndex[i] = CPoint(0, 0);
        m_packageSpec.SetPadData();
        Calc_DrawingPosition_Pad();
        RefreshData_Pad();
    }

    if (i_nLayerIndex == LayerIndex_CustomFixed || allRefresh)
    {
        m_packageSpec.SetCustomFixedData();
        Calc_DrawingPosition_CustomFixed();
        RefreshData_CustomFixed();
    }

    if (i_nLayerIndex == LayerIndex_CustomPolygon || allRefresh)
    {
        m_packageSpec.SetCustomPolygonData();
        Calc_DrawingPosition_CustomPolygon();
        RefreshData_CustomPolygon();
    }

    Update_Warning(i_nLayerIndex);

    return TRUE;
}

LRESULT CDlgVisionProcessingSpec::OnGridNotify(WPARAM wparam, LPARAM lparam)
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
            case ePID_LayerImportData:
                propertyButton_ImportMapData();
                break;
            case ePID_LayerExportData:
                propertyButton_ExportMapData();
                break;
            case ePID_LayerClearMapData:
                propertyButton_ClearMapData();
                break;
            case ePID_LayerCreateMapData:
                propertyButton_CreateMapData();
                break;
            case ePID_LayerEditor:
                propertyButton_MapDataEditor();
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
            case ePID_Rotate:
                FireExchanged();
                m_packageSpec.m_OriginPadData->m_nRotateIdx = data;
                m_packageSpec.m_originalballMap->m_ballMapRotation
                    = Package::BallCollectionOriginal::RotationIndex(data);
                m_packageSpec.m_OriginLandData->m_nRotateIdx = data;
                m_packageSpec.m_OriginCompData->m_nRotateIdx = data;
                m_packageSpec.m_OriginCustomFixedData->m_nRotateIdx = data;
                m_packageSpec.m_OriginCustomPolygonData->m_nRotateIdx = data;
                propertyChange_Rotation();
                break;

            case ePID_RotateComp:
                FireExchanged();
                m_packageSpec.m_OriginCompData->m_nRotateCompIdx = data;
                m_packageSpec.m_OriginLandData->m_nRotateLandIdx = data;
                propertyChange_RotateComp();
                break;

            case ePID_SelectLayer:
                if (1)
                {
                    m_selectLayer = data;

                    auto* itemImport = m_propertyGrid->FindItem(ePID_LayerImportData);
                    auto* itemExport = m_propertyGrid->FindItem(ePID_LayerExportData);
                    auto* itemCreate = m_propertyGrid->FindItem(ePID_LayerCreateMapData);

                    itemImport->SetReadOnly(m_selectLayer == enCurLayerIndex::LayerIndex_Whole);
                    itemExport->SetReadOnly(m_selectLayer == enCurLayerIndex::LayerIndex_Whole);
                    itemCreate->SetReadOnly(m_selectLayer == enCurLayerIndex::LayerIndex_Whole);

                    propertyChange_SelectLayer();
                }
                break;

            case ePID_SelectionMethod:
                m_selectionMethod = data;
                break;
        }

        return 0;
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemBool*>(item))
    {
        BOOL data = value->GetBool();
        switch (item->GetID())
        {
            case ePID_LayerFlipX:
                FireExchanged();
                getCurrentLayer_FlipX() = data;
                propertyChange_FlipX();
                break;

            case ePID_LayerFlipY:
                FireExchanged();
                getCurrentLayer_FlipY() = data;
                propertyChange_FlipY();
                break;
        }
    }

    return 0;
}

void CDlgVisionProcessingSpec::FireExchanged()
{
    SystemConfig::GetInstance().m_bIsExchangedPackageSpec = true;
}

void CDlgVisionProcessingSpec::propertyButton_MapDataEditor()
{
    std::vector<std::pair<long, CString>> convMapData;

    VisionMapDataEditorUI VisionMapDataEditorDlg(this->GetSafeHwnd(), &m_packageSpec, convMapData, TRUE);
    if (VisionMapDataEditorDlg.ConverterDoModal() == IDOK)
    {
        AutoImport_MapDataByXML(convMapData);
    }
}

void CDlgVisionProcessingSpec::AutoImport_MapDataByXML(std::vector<std::pair<long, CString>> i_vecConvertMapData)
{
    //mc_여기에 들어왔다는건 기존의 모든 Data Clear 부터 진행을 해야한다
    OriginMapData_AllClear();

    //MapData 경로를 통하여 한번에 읽고 DataSet을 진행한다
    long nRotateCurSel = long(m_packageSpec.m_originalballMap->m_ballMapRotation);
    long nRotateCompCurSel = m_packageSpec.m_OriginCompData->m_nRotateCompIdx;

    CString strErrorMsg = _T("");
    for (auto vecMapDataPath : i_vecConvertMapData)
    {
        switch (vecMapDataPath.first)
        {
            case MapDataType_Pad:
            {
                m_packageSpec.m_OriginPadData->LoadMap(vecMapDataPath.second, strErrorMsg);

                m_packageSpec.SetPadData();
                m_packageSpec.m_PadMapManager->RotateMap(nRotateCurSel);
                RefreshData_Pad();
                Calc_DrawingPosition_Pad();
                break;
            }
            case MapDataType_Ball:
            {
                m_packageSpec.m_originalballMap->Import(vecMapDataPath.second, strErrorMsg);

                m_packageSpec.m_originalballMap->m_ballMapRotation
                    = Package::BallCollectionOriginal::RotationIndex(m_packageSpec.m_OriginPadData->m_nRotateIdx);
                RefreshData_Ball();
                break;
            }
            case MapDataType_Land:
            {
                m_packageSpec.m_OriginLandData->LoadMap(vecMapDataPath.second, strErrorMsg);

                m_packageSpec.SetLandData();
                m_packageSpec.m_LandMapManager->RotateMap(nRotateCurSel);
                m_packageSpec.m_LandMapManager->RotateLand(nRotateCompCurSel);
                RefreshData_Land();
                Calc_DrawingPosition_Land();
                break;
            }
            case MapDataType_Comp:
            {
                m_packageSpec.m_OriginCompData->LoadMap(vecMapDataPath.second, strErrorMsg);

                m_packageSpec.SetCompData();
                m_packageSpec.m_CompMapManager->RotateMap(nRotateCurSel);
                m_packageSpec.m_CompMapManager->RotateComp(nRotateCompCurSel);
                RefreshData_Comp();
                Calc_DrawingPosition_Comp();

                //ChipDB Check
                //CompDBVerify(); //나중에 푸세요..

                break;
            }
            default:
                break;
        }

        Update_Background(vecMapDataPath.first);
        Update_Foreground(vecMapDataPath.first);

        Invalidate(FALSE);
    }

    //다한 후에는 Whole Device 항목으로 고정한다.
    Update_Background(LayerIndex_Whole);
    Update_Foreground(LayerIndex_Whole);

    //Job Save..
    FireExchanged();

    strErrorMsg.Empty();
}

void CDlgVisionProcessingSpec::OriginMapData_AllClear()
{
    //{{기존 Data Clear
    m_packageSpec.m_OriginPadData->vecOriginPadData.clear();
    m_packageSpec.m_originalballMap->m_balls.clear();
    m_packageSpec.m_OriginLandData->vecOriginLandData.clear();
    m_packageSpec.m_OriginCompData->vecOriginCompData.clear();

    m_packageSpec.m_CompMapManager->vecCompData.clear();

    m_vecBallInfo.clear();
    m_vecCompInfo.clear();
    m_vecLandInfo.clear();
    m_vecPadInfo.clear();

    for (long i = 0; i < 3; i++)
        m_ptPinIndex[i] = CPoint(0, 0);
    //}}

    //Job Save..
    FireExchanged();
}

void CDlgVisionProcessingSpec::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    // TODO: 여기에 메시지 처리기 코드를 추가합니다.
    CMenu mnuPopupSubmit;
    mnuPopupSubmit.LoadMenu(IDR_LAND_SHAPE_MENU);

    // Get a pointer to the first item of the menu
    CMenu* mnuPopupMenu = mnuPopupSubmit.GetSubMenu(0);
    ASSERT(mnuPopupMenu);

    CRect rtViewr;
    m_stcViewer.GetWindowRect(rtViewr);

    if (rtViewr.PtInRect(point) && m_selectLayer == LayerIndex_Land)
        mnuPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

void CDlgVisionProcessingSpec::OnLandshapechange()
{
    // TODO: 여기에 명령 처리기 코드를 추가합니다.
    DlgLandShapeEditor Dialog(m_pVisionInsp);

    if (Dialog.DoModal() == IDOK)
    {
        m_packageSpec.SetShapeData_CurPerOrigin_Land();

        RefreshData_Land();
        Calc_DrawingPosition_Land();
        Update_Foreground(m_selectLayer);

        Invalidate(FALSE);
    }
}
