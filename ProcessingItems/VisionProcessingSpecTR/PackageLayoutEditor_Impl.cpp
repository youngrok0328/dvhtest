// BallLayoutEditor_Impl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PackageLayoutEditor_Impl.h"
#include "DlgVisionProcessingSpec.h"
#include "DlgVisionProcessingSpec.h"
#include "VisionProcessingSpec.h"
#include "BodySizeEditor.h"

#include "../../PI_Modules/dPI_DataBase/FigureFactory.h"
#include "../../PI_Modules/dPI_DataBase/Figure.h"
#include "../../PI_Modules/dPI_DataBase/Figure_Ellipse.h"
#include "../../PI_Modules/dPI_DataBase/Figure_Rectangle.h"
#include "../../PI_Modules/dPI_DataBase/Figure_Polygon.h"
#include "../../PI_Modules/dPI_GridCtrl/GridCellCheck2.h"
#include "../../VisionCommon/VisionBaseDef.h"

#include "../../UserInterfaceModules/ToolkitProExtension/XTPPropertyGridItemCustomItems.h"

#include "Base/Image_8u_C1.h"
#include "MapDataEditor.h"

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
	ePID_SelectionMethod
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_CUSTOMOBJECTTRANSFER	(WM_USER+4445)		// WPARAM 0 : Normal Mode
// WPARAM 1 : Overwrap Mode
/////////////////////////////////////////////////////////////////////////////
// CPackageLayoutEditor_Impl dialog

CPackageLayoutEditor_Impl::CPackageLayoutEditor_Impl(SystemConfig &systemConfig, CPackageSpec &packageSpec, long nCurrentAccessMode, const CRect &rtPosition, CWnd* pParent /*=NULL*/)
	: CDialog(CPackageLayoutEditor_Impl::IDD, pParent)
	, m_systemConfig(systemConfig)
	, m_packageSpec(packageSpec)
	, m_rtInitPosition(rtPosition)
	, m_propertyGrid(new CXTPPropertyGrid)
	, m_selectLayer(0)
	, m_selectionMethod(2)
{
}

CPackageLayoutEditor_Impl::~CPackageLayoutEditor_Impl()
{
	delete m_propertyGrid;
}

void CPackageLayoutEditor_Impl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STC_VIEWER, m_stcViewer);
}


BEGIN_MESSAGE_MAP(CPackageLayoutEditor_Impl, CDialog)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_HSCROLL()
	ON_WM_CLOSE()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPackageLayoutEditor_Impl message handlers
BOOL CPackageLayoutEditor_Impl::OnInitDialog() 
{
	CDialog::OnInitDialog();

	MoveWindow(m_rtInitPosition, FALSE);

	CRect rtClient;
	GetClientRect(rtClient);

	CRect rtPropertyGrid = ::GetContentSubArea(rtClient, 0, 1, 3, 0, 1, 1);
	GetDlgItem(IDC_FRAME_PROPERTY_GRID)->MoveWindow(rtPropertyGrid);
	//ScreenToClient(rtPropertyGrid);
	m_propertyGrid->Create(rtPropertyGrid, this, IDC_FRAME_PROPERTY_GRID);

	m_stcViewer.MoveWindow(::GetContentSubArea(rtClient, 1, 3, 3, 0, 1, 1));

	//mc_분기작업 시작
	m_vecstrLayerName.clear();
	m_vecstrLayerName.resize(5); //고정값

	m_vecstrLayerName[0] = _T("Pad Layer");
	m_vecstrLayerName[1] = _T("Ball Layer");
	m_vecstrLayerName[2] = _T("Land Layer");
	m_vecstrLayerName[3] = _T("Component Layer");
	m_vecstrLayerName[4] = _T("Whole Device");

	Initialize_View();

	Update_PropertyGrid();
	propertyChange_SelectLayer();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPackageLayoutEditor_Impl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect ViewRect;
	m_stcViewer.GetWindowRect(ViewRect);
	ScreenToClient(ViewRect);

	CDC MemDC;
	MemDC.CreateCompatibleDC(&dc);
	CBitmap *OldBitmap = MemDC.SelectObject(&m_bitmapView);
	dc.BitBlt(ViewRect.left, ViewRect.top, ViewRect.Width(), ViewRect.Height(), &MemDC, 0,0, SRCCOPY);
	MemDC.SelectObject(OldBitmap);
}

void CPackageLayoutEditor_Impl::Update_Background(long i_nCurLayerIndex)
{
	CDC *pDC = GetDC();
	m_bitmapGrid.DeleteObject();
	m_bitmapGrid.CreateCompatibleBitmap(pDC, m_rtView.Width(), m_rtView.Height());

	CDC MemDC;
	MemDC.CreateCompatibleDC(pDC);
	CBitmap *OldBitmap = MemDC.SelectObject(&m_bitmapGrid);

	// 디바이스 테두리 그리기
	CBrush brushDarkGreen(RGB(0, 120, 0));
	CBrush *pOldBrush = MemDC.SelectObject(&brushDarkGreen);

	CPen penBrightGreen(PS_SOLID, 3, RGB(0, 80, 0));
	CPen *pOldPen = MemDC.SelectObject(&penBrightGreen);

	MemDC.Rectangle(m_rtDevice);

	// Ball Index by Bottom View
	CPen penLayerPinIndex(PS_SOLID, 1, RGB(0, 230, 0));
	MemDC.SelectObject(&penLayerPinIndex);
	CBrush brushLayerPinIndex(RGB(0, 230, 0));
	MemDC.SelectObject(&brushLayerPinIndex);

	MemDC.Polygon(m_ptPinIndex, 3);

	if(i_nCurLayerIndex == LayerIndex_Whole)
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

void CPackageLayoutEditor_Impl::Update_Foreground(long i_nCurLayerIndex)
{
	if(i_nCurLayerIndex < 0)
		return ;

	const long nBallLayerNum = 1;

	CDC *pDC = GetDC();
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
	case LayerIndex_Whole:
		SetDrawing_WholeForegroundData(pDC);
		break;
	}
	//

	ReleaseDC(pDC);
}

void CPackageLayoutEditor_Impl::SetDrawing_BallForegroundData(long i_nCurLayerIndex, CDC *i_pDC)
{
	if (i_pDC == NULL)
		return;

	CDC MemDC;
	MemDC.CreateCompatibleDC(i_pDC);
	MemDC.SetBkMode(TRANSPARENT);
	CBitmap *pOldBitmap = MemDC.SelectObject(&m_bitmapView);

	// Grid 이미지를 붙입니다
	CDC MemDC2;
	MemDC2.CreateCompatibleDC(i_pDC);
	CBitmap *OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
	MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
	MemDC2.SelectObject(&OldBitmap2);

	// 볼을 그립니다
	COLORREF rgbMain = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);

	CBrush brushMain(rgbMain);
	CBrush *pOldBrush = MemDC.SelectObject(&brushMain);

	for (const auto &ballInfo : m_vecBallInfo)
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

void CPackageLayoutEditor_Impl::SetDrawing_LandForegroundData(long i_nCurLayerIndex, CDC *i_pDC)
{
	if (i_pDC == NULL)
		return;

	CDC MemDC;
	MemDC.CreateCompatibleDC(i_pDC);
	MemDC.SetBkMode(TRANSPARENT);
	CBitmap *pOldBitmap = MemDC.SelectObject(&m_bitmapView);

	// Grid 이미지를 붙입니다
	CDC MemDC2;
	MemDC2.CreateCompatibleDC(i_pDC);
	CBitmap *OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
	MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
	MemDC2.SelectObject(&OldBitmap2);

	CPoint ptLandROI[4];

	CBrush *pOldBrush = NULL;
	for (const auto &LandInfo : m_vecLandInfo)
	{
		COLORREF rgbLand(0);
		switch (LandInfo.nGroupID) //mc_이후에 추가할꺼 있으면 수동으로 ..
		{
		case 0:
			rgbLand = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
			break;
		case 1:
			rgbLand = RGB(255 - (1 + i_nCurLayerIndex / 2) * 100, 128, 255);
			break;
		case 2:
			rgbLand = RGB(255 , 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
			break;
		default:
			rgbLand = RGB(243, 97, 166);
			break;
		}

		CBrush brushLand(rgbLand);
		pOldBrush = MemDC.SelectObject(&brushLand);

		if (LandInfo.bIgnore)
			MemDC.SelectObject(GetStockObject(NULL_BRUSH));
		else
			MemDC.SelectObject(&brushLand);

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

	MemDC.SelectObject(pOldBrush);

	MemDC.SelectObject(pOldBitmap);
}

void CPackageLayoutEditor_Impl::SetDrawing_CompForegroundData(long i_nCurLayerIndex, CDC *i_pDC)
{
	if (i_pDC == NULL)
		return;

	CDC MemDC;
	MemDC.CreateCompatibleDC(i_pDC);
	MemDC.SetBkMode(TRANSPARENT);
	CBitmap *pOldBitmap = MemDC.SelectObject(&m_bitmapView);

	// Grid 이미지를 붙입니다
	CDC MemDC2;
	MemDC2.CreateCompatibleDC(i_pDC);
	CBitmap *OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
	MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
	MemDC2.SelectObject(&OldBitmap2);

	// Comp를 그립니다
	COLORREF rgbComp = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);

	CBrush brushComp(rgbComp);
	CBrush *pOldBrush = MemDC.SelectObject(&brushComp);

	CPoint ptCompROI[4];

	for (const auto &CompInfo : m_vecCompInfo)
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

void CPackageLayoutEditor_Impl::SetDrawing_PadForegroundData(long i_nCurLayerIndex, CDC *i_pDC)
{
	if (i_pDC == NULL)
		return;

	CDC MemDC;
	MemDC.CreateCompatibleDC(i_pDC);
	MemDC.SetBkMode(TRANSPARENT);
	CBitmap *pOldBitmap = MemDC.SelectObject(&m_bitmapView);

	// Grid 이미지를 붙입니다
	CDC MemDC2;
	MemDC2.CreateCompatibleDC(i_pDC);
	CBitmap *OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
	MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
	MemDC2.SelectObject(&OldBitmap2);

	COLORREF rgbPad = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
	CRect rtROI;

	CBrush brushPad(rgbPad);
	CBrush *pOldBrush = MemDC.SelectObject(&brushPad);

	for (const auto &PadInfo : m_vecPadInfo)
	{
		if (PadInfo.bIgnore)
			MemDC.SelectObject(GetStockObject(NULL_BRUSH));
		else
			MemDC.SelectObject(&brushPad);

		if (PadInfo.nPadType == _typeRectangle)
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

		else if (PadInfo.nPadType == _typeCircle)
		{
			CRect rtPad_Circle;
			rtPad_Circle = IPVM::ToMFC(PadInfo.fsrtSpecROI.GetCRect());

			MemDC.Ellipse(rtPad_Circle);
		}

		else //넌무조건 PinIndex _typeTriangle
		{
			CPoint ptPad_PinIndex[3];
			GetPinIndexPoint(PadInfo.fsrtSpecROI, ptPad_PinIndex);

			MemDC.Polygon(ptPad_PinIndex, 3);
		}

	}

	MemDC.SelectObject(pOldBrush);

	MemDC.SelectObject(pOldBitmap);
}

void CPackageLayoutEditor_Impl::GetPinIndexPoint(FPI_RECT i_frtPinIndex, CPoint *o_pPinIndexPoint)
{
	long nPinIndexPos(0);

	if (m_rtDevice.CenterPoint().x < i_frtPinIndex.GetCenter().m_x && m_rtDevice.CenterPoint().y < i_frtPinIndex.GetCenter().m_x)
		nPinIndexPos = PinIndexPos_LT;

	else if (m_rtDevice.CenterPoint().x >= i_frtPinIndex.GetCenter().m_x && m_rtDevice.CenterPoint().y < i_frtPinIndex.GetCenter().m_y)
		nPinIndexPos = PinIndexPos_RT;

	if (m_rtDevice.CenterPoint().x >= i_frtPinIndex.GetCenter().m_x && m_rtDevice.CenterPoint().y >= i_frtPinIndex.GetCenter().m_y)
		nPinIndexPos = PinIndexPos_RB;

	if (m_rtDevice.CenterPoint().x < i_frtPinIndex.GetCenter().m_x && m_rtDevice.CenterPoint().y >= i_frtPinIndex.GetCenter().m_y)
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

void CPackageLayoutEditor_Impl::SetDrawing_WholeForegroundData(CDC *i_pDC)
{
	if (i_pDC == NULL)
		return;

	CDC MemDC;
	MemDC.CreateCompatibleDC(i_pDC);
	MemDC.SetBkMode(TRANSPARENT);
	CBitmap *pOldBitmap = MemDC.SelectObject(&m_bitmapView);

	// Grid 이미지를 붙입니다
	CDC MemDC2;
	MemDC2.CreateCompatibleDC(i_pDC);
	CBitmap *OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
	MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
	MemDC2.SelectObject(&OldBitmap2);

	// 볼을 그립니다
	COLORREF rgbWholeDevice_Ball = RGB(242, 150, 97);

	CBrush brushBall(rgbWholeDevice_Ball);
	CBrush *pBall_OldBrush = MemDC.SelectObject(&brushBall);

	for (const auto &ballInfo : m_vecBallInfo)
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
	CBrush *pLand_OldBrush = MemDC.SelectObject(&brushLand);

	CPoint ptLandROI[4];

	for (const auto &LandInfo : m_vecLandInfo)
	{
		if(LandInfo.bIgnore)
			MemDC.SelectObject(GetStockObject(NULL_BRUSH));
		else
			MemDC.SelectObject(&brushLand);

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

	//Comp를 그립니다
	COLORREF rgbWholeDevice_Comp = RGB(255, 255, 72);
	CBrush brushComp(rgbWholeDevice_Comp);
	CBrush *pComp_OldBrush = MemDC.SelectObject(&brushComp);

	CPoint ptCompROI[4];

	for (const auto &CompInfo : m_vecCompInfo)
	{
		if(CompInfo.bIgnore)
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
	CBrush *pPad_OldBrush = MemDC.SelectObject(&brushPad);

	CRect rtROI;
	float fOffsetX(0.f), fOffsetY(0.f);
	for (long n = 0; n < (long)m_vecPadInfo.size(); n++)
	{
		rtROI = IPVM::ToMFC(m_vecPadInfo[n].rtROI);
		fOffsetX = (rtROI.left + rtROI.right) * 0.5f;
		fOffsetY = (rtROI.top + rtROI.bottom) * 0.5f;

		if (m_vecPadInfo[n].nPadType == _typeTriangle)
		{
			CPoint ptPadIndex[4];

			if ((m_rtDevice.right / 2) >= m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x && (m_rtDevice.bottom / 2) >= m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y)
			{
				ptPadIndex[0].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
				ptPadIndex[0].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
				ptPadIndex[1].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
				ptPadIndex[1].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
				ptPadIndex[2].x = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x - m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x) / 2 + m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
				ptPadIndex[2].y = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y - m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y) / 2 + m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
				ptPadIndex[3].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
				ptPadIndex[3].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);
			}
			else if ((m_rtDevice.right / 2) < m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x && (m_rtDevice.bottom / 2) < m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y)
			{
				ptPadIndex[0].x = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x - m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x) / 2 + m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
				ptPadIndex[0].y = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y - m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y) / 2 + m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
				ptPadIndex[1].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
				ptPadIndex[1].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
				ptPadIndex[2].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
				ptPadIndex[2].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
				ptPadIndex[3].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
				ptPadIndex[3].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);
			}
			else if ((m_rtDevice.right / 2) >= m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x && (m_rtDevice.bottom / 2) < m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y)
			{
				ptPadIndex[0].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
				ptPadIndex[0].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
				ptPadIndex[1].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
				ptPadIndex[1].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
				ptPadIndex[2].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
				ptPadIndex[2].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
				ptPadIndex[3].x = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x - m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x) / 2 + m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
				ptPadIndex[3].y = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y - m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y) / 2 + m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
			}
			else if ((m_rtDevice.right / 2) < m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x && (m_rtDevice.bottom / 2) >= m_vecPadInfo[n].fsrtSpecROI.fptLB.m_y)
			{
				ptPadIndex[0].x = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
				ptPadIndex[0].y = long(m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
				ptPadIndex[1].x = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x - m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x) / 2 + m_vecPadInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
				ptPadIndex[1].y = long((m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y - m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y) / 2 + m_vecPadInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
				ptPadIndex[2].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
				ptPadIndex[2].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
				ptPadIndex[3].x = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
				ptPadIndex[3].y = long(m_vecPadInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);
			}

			MemDC.Polygon(ptPadIndex, 4);
			MemDC.SelectObject(brushPad);
		}
		else if (m_vecPadInfo[n].nPadType == _typeCircle)
		{
			CRect m_PadPosition;
			m_PadPosition = IPVM::ToMFC(m_vecPadInfo[n].fsrtSpecROI.GetCRect());
			MemDC.Ellipse(m_PadPosition);
		}
	}

	MemDC.SelectObject(pBall_OldBrush);
	MemDC.SelectObject(pLand_OldBrush);
	MemDC.SelectObject(pComp_OldBrush);
	MemDC.SelectObject(pPad_OldBrush);

	MemDC.SelectObject(pOldBitmap);
}

void CPackageLayoutEditor_Impl::Update_PropertyGrid()
{
	m_propertyGrid->ResetContent();

	if (auto* category = m_propertyGrid->AddCategory(_T("Device")))
	{
		category->AddChildItem(new CCustomItemButton(_T("Setup body size"), TRUE, FALSE))->SetID(ePID_BodySize);

		auto* item_rotate = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Rotate"), long(m_packageSpec.m_ballMapSource.m_ballMapRotation)));
		item_rotate->GetConstraints()->AddConstraint(_T("0 deg"), long(PackageSpec_BallMapSource::RotationIndex::Deg_0));
		item_rotate->GetConstraints()->AddConstraint(_T("90 deg"), long(PackageSpec_BallMapSource::RotationIndex::Deg_90));
		item_rotate->GetConstraints()->AddConstraint(_T("180 deg"), long(PackageSpec_BallMapSource::RotationIndex::Deg_180));
		item_rotate->GetConstraints()->AddConstraint(_T("270 deg"), long(PackageSpec_BallMapSource::RotationIndex::Deg_270));
		item_rotate->SetID(ePID_Rotate);

		auto* item_rotateComp = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Rotate Comp"), m_packageSpec.m_OriginCompData.m_nRotateCompIdx));
		item_rotateComp->GetConstraints()->AddConstraint(_T("0 deg"), 0);
		item_rotateComp->GetConstraints()->AddConstraint(_T("90 deg"), 1);
		item_rotateComp->SetID(ePID_RotateComp);

		category->Expand();
	}

	if (auto* category = m_propertyGrid->AddCategory(_T("Layer")))
	{
		auto* item_selectLayer = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Select layer"), m_selectLayer));

		for (long n = 0; n < m_vecstrLayerName.size(); n++)
		{
			item_selectLayer->GetConstraints()->AddConstraint(m_vecstrLayerName[n], n);
		}

		item_selectLayer->SetID(ePID_SelectLayer);

		category->AddChildItem(new CXTPPropertyGridItemBool(_T("Flip X"), getCurrentLayer_FlipX()))->SetID(ePID_LayerFlipX);
		category->AddChildItem(new CXTPPropertyGridItemBool(_T("Flip Y"), getCurrentLayer_FlipY()))->SetID(ePID_LayerFlipY);

		category->AddChildItem(new CCustomItemButton(_T("Import Map Data"), TRUE, FALSE))->SetID(ePID_LayerImportData);
		category->AddChildItem(new CCustomItemButton(_T("Export Map Data"), TRUE, FALSE))->SetID(ePID_LayerExportData);
		category->AddChildItem(new CCustomItemButton(_T("Clear Map Data"), TRUE, FALSE))->SetID(ePID_LayerClearMapData);

		category->Expand();
	}

	if (auto* category = m_propertyGrid->AddCategory(_T("Option")))
	{
		auto* item_selectionMethod = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Selection Method"), long(m_selectionMethod)));
		item_selectionMethod->GetConstraints()->AddConstraint(_T("Enable Selected Objects"), 0);
		item_selectionMethod->GetConstraints()->AddConstraint(_T("Disable Selected Objects"), 1);
		item_selectionMethod->GetConstraints()->AddConstraint(_T("Invert Selected Objects"), 2);
		item_selectionMethod->SetID(ePID_SelectionMethod);
		category->Expand();
	}

	Update_PropertyGrid_LayersSummy();

	m_propertyGrid->SetViewDivider(0.50);
	m_propertyGrid->HighlightChangedItems(TRUE);
}

void CPackageLayoutEditor_Impl::Update_PropertyGrid_LayersSummy()
{
	CString categoryName = _T("Layers summary");
	auto* category = m_propertyGrid->GetCategories()->FindItem(categoryName);

	if (category == nullptr)
	{
		category = m_propertyGrid->AddCategory(categoryName);
	}

	if (m_packageSpec.m_PadMapManager.vecPadData.size())
	{
		auto *PadCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Pad counts"), long(m_packageSpec.m_PadMapManager.vecPadData.size())));
		PadCounts->SetReadOnly();
	}

	if (m_packageSpec.m_ballMap.m_balls.size() > 0)
	{
		auto *ballCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Ball counts"), long(m_packageSpec.m_ballMap.m_balls.size())));
		ballCounts->SetReadOnly();
	}

	if (m_packageSpec.m_LandMapConvertOrigin.vecLandData.size() > 0)
	{
		auto *LandCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Land counts"), long(m_packageSpec.m_LandMapConvertOrigin.vecLandData.size())));
		LandCounts->SetReadOnly();
	}

	if (m_packageSpec.m_CompMapManager.vecCompData.size() > 0)
	{
		auto *CompCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Comp counts"), long(m_packageSpec.m_CompMapManager.vecCompData.size())));
		CompCounts->SetReadOnly();
	}

	{
		long nBallObejctNum = (long)m_packageSpec.m_ballMap.m_balls.size();
		long nLandObejctNum = (long)m_packageSpec.m_LandMapConvertOrigin.vecLandData.size();
		long nComponetObejectNum = (long)m_packageSpec.m_CompMapManager.vecCompData.size();
		long nPadObejctNum = (long)m_packageSpec.m_PadMapManager.vecPadData.size();

		long nTotalObjectNum = nBallObejctNum + nLandObejctNum + nComponetObejectNum + nPadObejctNum;

		auto *TotalCounts = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Total counts"), nTotalObjectNum));

		TotalCounts->SetReadOnly();
	}

	category->Expand();
}

void CPackageLayoutEditor_Impl::Initialize_View()
{
	m_stcViewer.GetWindowRect(m_rtView);

	m_rtView.OffsetRect(-m_rtView.left, -m_rtView.top);

	// 그려질 디바이스 크기 결정
	m_fViewAspectRatio = float(m_rtView.Width()) / m_rtView.Height();
	m_fDeviceAspectRatio = m_packageSpec.m_bodyInfoMaster.fBodySizeX / m_packageSpec.m_bodyInfoMaster.fBodySizeY;

	long nMargin = 80; // 영훈 20160422 : 여백을 둬서 그 부분에 Ball ID를 입력해주도록 한다.
	if(m_fViewAspectRatio >= m_fDeviceAspectRatio)
	{
		float fDeviceHalfHeightPixel = 0.5f * (m_rtView.Height() - nMargin);
		float fDeviceHalfWidthPixel = 0.5f * (m_rtView.Height() -nMargin) * m_fDeviceAspectRatio;

		m_rtDevice.left = long(m_rtView.CenterPoint().x - fDeviceHalfWidthPixel + 0.5f);
		m_rtDevice.right = long(m_rtView.CenterPoint().x + fDeviceHalfWidthPixel + 0.5f) + 1;
		m_rtDevice.top = long(m_rtView.CenterPoint().y - fDeviceHalfHeightPixel + 0.5f);
		m_rtDevice.bottom = long(m_rtView.CenterPoint().y + fDeviceHalfHeightPixel + 0.5f) + 1;
	}
	else
	{
		float fDeviceHalfHeightPixel = 0.5f * (m_rtView.Width() - nMargin) / m_fDeviceAspectRatio ;
		float fDeviceHalfWidthPixel = 0.5f * (m_rtView.Width() - nMargin);

		m_rtDevice.left = long(m_rtView.CenterPoint().x - fDeviceHalfWidthPixel + 0.5f);
		m_rtDevice.right = long(m_rtView.CenterPoint().x + fDeviceHalfWidthPixel + 0.5f) + 1;
		m_rtDevice.top = long(m_rtView.CenterPoint().y - fDeviceHalfHeightPixel + 0.5f);
		m_rtDevice.bottom = long(m_rtView.CenterPoint().y + fDeviceHalfHeightPixel + 0.5f) + 1;
	}

	m_fZoom = m_rtDevice.Width() / (m_packageSpec.m_bodyInfoMaster.fBodySizeX);
	m_fZoom = m_rtDevice.Height() / (m_packageSpec.m_bodyInfoMaster.fBodySizeY);
}

void CPackageLayoutEditor_Impl::OnLButtonDown(UINT nFlags, CPoint point) 
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
		case LayerIndex_Ball:
			Ignore_Ball(rtSelection, m_selectionMethod);
			break;
		case LayerIndex_Land:
			Ignore_Land(rtSelection, m_selectionMethod);
			break;
		case LayerIndex_Compnent:
			Ignore_Comp(rtSelection, m_selectionMethod);
			break;
		case LayerIndex_Pad:
			Ignore_Pad(rtSelection, m_selectionMethod);
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

void CPackageLayoutEditor_Impl::Ignore_Ball(CRect i_rtSelect, long i_nEditMode)
{
	CRect rtTemp;

	for (auto &ballInfo : m_vecBallInfo)
	{
		if (rtTemp.IntersectRect(i_rtSelect, ballInfo.m_ballPosition))
		{
			if (i_nEditMode == 0)
				m_packageSpec.m_ballMapSource.m_balls[ballInfo.m_index].m_ignored = false;
			else if (i_nEditMode == 1)
				m_packageSpec.m_ballMapSource.m_balls[ballInfo.m_index].m_ignored = true;
			else
				m_packageSpec.m_ballMapSource.m_balls[ballInfo.m_index].m_ignored = !m_packageSpec.m_ballMapSource.m_balls[ballInfo.m_index].m_ignored;
		}
	}

	m_packageSpec.CalcDependentVariables(m_systemConfig);
	Calc_DrawingPosition_Ball();
}

void CPackageLayoutEditor_Impl::Ignore_Land(CRect i_rtSelect, long i_nEditMode)
{
	CRect rtTemp;

	for (auto &LandInfo : m_packageSpec.m_LandMapConvertOrigin.vecLandData)
	{
		CRect rtSpecROI = IPVM::ToMFC(LandInfo.fsrtSpecROI.GetCRect());

		if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
		{
			if (i_nEditMode == 0)
				LandInfo.bIgnore = FALSE;
			else if (i_nEditMode == 1)
				LandInfo.bIgnore = TRUE;
			else
				LandInfo.bIgnore = !LandInfo.bIgnore;
		}
	}

	//Visible Data는 주자
	m_packageSpec.SetVisibleData_CurPerOrigin_Land();

	RefreshData_Land();
	Calc_DrawingPosition_Land();
}

void CPackageLayoutEditor_Impl::Ignore_Comp(CRect i_rtSelect, long i_nEditMode)
{
	CRect rtTemp;

	for (auto &CompInfo : m_packageSpec.m_CompMapManager.vecCompData)
	{
		CRect rtSpecROI = IPVM::ToMFC(CompInfo.fsrtROI.GetCRect());

		if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
		{
			if (i_nEditMode == 0)
				CompInfo.bIgnore = FALSE;
			else if (i_nEditMode == 1)
				CompInfo.bIgnore = TRUE;
			else
				CompInfo.bIgnore = !CompInfo.bIgnore;
		}
	}

	//Visible Data는 주자
	m_packageSpec.SetVisibleData_CurPerOrigin();

	RefreshData_Comp();
	Calc_DrawingPosition_Comp();
}

void CPackageLayoutEditor_Impl::Ignore_Pad(CRect i_rtSelect, long i_nEditMode)
{
	CRect rtTemp;

	for (auto &PadInfo : m_packageSpec.m_PadMapManager.vecPadData)
	{
		CRect rtSpecROI = IPVM::ToMFC(PadInfo.fsrtSpecROI.GetCRect());

		if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
		{
			if (i_nEditMode == 0)
				PadInfo.bIgnore = FALSE;
			else if (i_nEditMode == 1)
				PadInfo.bIgnore = TRUE;
			else
				PadInfo.bIgnore = !PadInfo.bIgnore;
		}
	}

	//Visible Data는 주자
	m_packageSpec.SetVisibleData_CurPerOrigin_Pad();

	RefreshData_Pad();
	Calc_DrawingPosition_Pad();
}

void CPackageLayoutEditor_Impl::propertyChange_SelectLayer()
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

	else if (nSelectLayer == LayerIndex_Whole) //처음에 바로 Whole 들어올때 
	{
		Calc_DrawingPosition_Ball();
		Calc_DrawingPosition_Land();
		Calc_DrawingPosition_Comp();
		Calc_DrawingPosition_Pad();
	}

	Update_Background(nSelectLayer);
	Update_Foreground(nSelectLayer);

	if (nSelectLayer == LayerIndex_Whole) //WholeDevice면 모든 BtnCtrl 비활성화
	{
		m_propertyGrid->FindItem(ePID_LayerImportData)->SetReadOnly(TRUE);
		m_propertyGrid->FindItem(ePID_LayerExportData)->SetReadOnly(TRUE);
		m_propertyGrid->FindItem(ePID_LayerClearMapData)->SetReadOnly(TRUE);

		m_propertyGrid->FindItem(ePID_LayerFlipX)->SetReadOnly(TRUE);
		m_propertyGrid->FindItem(ePID_LayerFlipY)->SetReadOnly(TRUE);
		m_propertyGrid->FindItem(ePID_SelectionMethod)->SetReadOnly(TRUE);
	}
	else
	{
		m_propertyGrid->FindItem(ePID_LayerImportData)->SetReadOnly(FALSE);
		m_propertyGrid->FindItem(ePID_LayerExportData)->SetReadOnly(FALSE);
		m_propertyGrid->FindItem(ePID_LayerClearMapData)->SetReadOnly(FALSE);

		m_propertyGrid->FindItem(ePID_LayerFlipX)->SetReadOnly(FALSE);
		m_propertyGrid->FindItem(ePID_LayerFlipY)->SetReadOnly(FALSE);
		m_propertyGrid->FindItem(ePID_SelectionMethod)->SetReadOnly(FALSE);

		((CXTPPropertyGridItemBool *)m_propertyGrid->FindItem(ePID_LayerFlipX))->SetBool(getCurrentLayer_FlipX());
		((CXTPPropertyGridItemBool *)m_propertyGrid->FindItem(ePID_LayerFlipY))->SetBool(getCurrentLayer_FlipY());
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

	Invalidate(FALSE);
}

void CPackageLayoutEditor_Impl::propertyButton_BodySize()
{
	CBodySizeEditor Dlg(m_packageSpec);

	if(Dlg.DoModal() == IDOK)
	{
		if(Dlg.m_bChangedDeadBug)
			::AfxMessageBox(_T("Dead bug option has been changed.\nThis option will take effect after job save."));

		long nCurLayer = m_selectLayer;

		Initialize_View();

		switch (nCurLayer)
		{
		case LayerIndex_Ball:
			Calc_DrawingPosition_Ball();
			break;
		case LayerIndex_Land:
			Calc_DrawingPosition_Land();
			break;
		case LayerIndex_Compnent:
			Calc_DrawingPosition_Comp();
			break;
		case LayerIndex_Pad:
			Calc_DrawingPosition_Pad();
			break;
		}

		Update_Background(nCurLayer);
		Update_Foreground(nCurLayer);

		Invalidate(FALSE);
	}
}

BOOL& CPackageLayoutEditor_Impl::getCurrentLayer_FlipX()
{
	switch (m_selectLayer)
	{
	case LayerIndex_Ball:		return m_packageSpec.m_ballMapSource.m_ballMapFlipX;
	case LayerIndex_Land:		return m_packageSpec.m_OriginLandData.m_bLandMapFlipX;
	case LayerIndex_Compnent:	return m_packageSpec.m_OriginCompData.m_bCompMapFlipX;
	case LayerIndex_Pad:		return m_packageSpec.m_OriginPadData.m_bPadMapFlipX;
	default:
		break;
	}

	ASSERT(!_T("??"));
	return m_packageSpec.m_ballMapSource.m_ballMapFlipX;
}
BOOL& CPackageLayoutEditor_Impl::getCurrentLayer_FlipY()
{
	switch (m_selectLayer)
	{
	case LayerIndex_Ball:		return m_packageSpec.m_ballMapSource.m_ballMapFlipY;
	case LayerIndex_Land:		return m_packageSpec.m_OriginLandData.m_bLandMapFlipY;
	case LayerIndex_Compnent:	return m_packageSpec.m_OriginCompData.m_bCompMapFlipY;
	case LayerIndex_Pad:		return m_packageSpec.m_OriginPadData.m_bPadMapFlipY;
	default:
		break;
	}

	ASSERT(!_T("??"));
	return m_packageSpec.m_ballMapSource.m_ballMapFlipY;
}

float CPackageLayoutEditor_Impl::GetIndexSize()
{
	float fIndexSize = 100.f;	// Minimum Default : um

	for (const auto &ballType : m_packageSpec.m_ballMapSource.m_ballTypes)
	{
		if (ballType.m_diameter_um > fIndexSize)
		{
			fIndexSize = (float)ballType.m_diameter_um;
		}
	}

	return fIndexSize;
}

void CPackageLayoutEditor_Impl::Calc_DrawingPosition_Ball()
{
	const auto ballCount = m_packageSpec.m_ballMap.m_balls.size();

	m_vecBallInfo.clear();

	const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
	const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

	for(const auto &ball : m_packageSpec.m_ballMap.m_balls)
	{
		const float fCenX = fGridOriginX + (float)(ball.m_posX_um * m_fZoom);
		const float fCenY = fGridOriginY - (float)(ball.m_posY_um * m_fZoom);
		const float fRadius = (float)(0.5 * ball.m_diameter_um * m_fZoom);

		m_vecBallInfo.emplace_back(long(fCenX - fRadius + 0.5f), long(fCenY - fRadius + 0.5f), long(fCenX + fRadius + 1.5f), long(fCenY + fRadius + 1.5f), ball.m_ignored, ball.m_index);
	}
}

void CPackageLayoutEditor_Impl::Calc_DrawingPosition_Comp()
{
	long nDataCount = (long)m_packageSpec.m_CompMapManager.vecCompData.size();

	if (nDataCount <= 0)
		return;
	else
		RefreshData_Comp();

	IPVM::Rect_32f tmp_sf_Roi;
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
			fWidth	= (m_vecCompInfo[n].fLength * 1000.f) * m_fZoom * 0.5f;
			fLength = (m_vecCompInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
		}
		else
		{
			fWidth	= (m_vecCompInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
			fLength = (m_vecCompInfo[n].fLength * 1000.f) * m_fZoom * 0.5f;
		}

		tmp_sf_Roi.m_left	= (float)(fOffsetX - fWidth);
		tmp_sf_Roi.m_right	= (float)(fOffsetX + fWidth);
		tmp_sf_Roi.m_top	= (float)(fOffsetY - fLength);
		tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);

		FPI_RECT frtChip = FPI_RECT(IPVM::Point_32f_C2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top), IPVM::Point_32f_C2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
									IPVM::Point_32f_C2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom), IPVM::Point_32f_C2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

		m_vecCompInfo[n].fsrtROI = frtChip; //SpecPos
	}

	UpdateMapPosition();
}

void CPackageLayoutEditor_Impl::Calc_DrawingPosition_Pad()
{
	long nDataCount = (long)m_packageSpec.m_PadMapManager.vecPadData.size();

	if (nDataCount <= 0)
		return;
	else
		RefreshData_Pad();

	IPVM::Rect rtROI;
	IPVM::Rect_32f tmp_sf_Roi;
	float fOffsetX(0.f), fOffsetY(0.f);
	float fWidth(0.f), fLength(0.f);

	const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
	const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

	for (long n = 0; n < nDataCount; n++)
	{
		fOffsetX = fGridOriginX + (m_vecPadInfo[n].fOffsetX * 1000.f) * m_fZoom;
		fOffsetY = fGridOriginY - (m_vecPadInfo[n].fOffsetY * 1000.f) * m_fZoom;

		if (m_vecPadInfo[n].nPadType == _typeTriangle)
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
		else if (m_vecPadInfo[n].nPadType == _typeCircle)
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

		FPI_RECT frtChip = FPI_RECT(IPVM::Point_32f_C2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top), IPVM::Point_32f_C2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
									IPVM::Point_32f_C2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom), IPVM::Point_32f_C2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

		m_vecPadInfo[n].fsrtSpecROI = frtChip;
		if (m_vecPadInfo[n].nPadType == _typeTriangle)
			GetPinIndexPoint(frtChip, m_ptPinIndex);
	}

	UpdateMapPosition_Pad();
}

void CPackageLayoutEditor_Impl::Calc_DrawingPosition_Land()
{
	long nDataCount = (long)m_packageSpec.m_LandMapConvertOrigin.vecLandData.size();

	if (nDataCount <= 0)
		return;
	else
		RefreshData_Land();

	const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
	const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

	IPVM::Rect rtROI;
	IPVM::Rect_32f tmp_sf_Roi;
	float fOffsetX(0.f), fOffsetY(0.f);
	float fWidth(0.f), fLength(0.f), fAngle(0.f);

	for (long n = 0; n < nDataCount; n++)
	{
		fOffsetX = fGridOriginX + (m_vecLandInfo[n].fOffsetX * 1000.f) * m_fZoom;
		fOffsetY = fGridOriginY - (m_vecLandInfo[n].fOffsetY * 1000.f) * m_fZoom;

		fWidth	= (m_vecLandInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
		fLength = (m_vecLandInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
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

		FPI_RECT frtChip = FPI_RECT(IPVM::Point_32f_C2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top), IPVM::Point_32f_C2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
									IPVM::Point_32f_C2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom), IPVM::Point_32f_C2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

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

void CPackageLayoutEditor_Impl::OnClose() 
{
	HWND hWnd = ::FindWindow(NULL, _T("IntekPlus Gerber Extractor"));

	if(hWnd != NULL)
	{
		::PostMessage(hWnd, WM_CLOSE, 0, 0);
	}

	m_propertyGrid->DestroyWindow();

	CDialog::OnClose();
}

void CPackageLayoutEditor_Impl::RefreshData_Ball(SystemConfig &systemParam)
{
	m_packageSpec.CalcDependentVariables(systemParam);

	long nCurLayer = m_selectLayer;

	Initialize_View();

	Calc_DrawingPosition_Ball();

	Update_Background(nCurLayer);
	Update_Foreground(nCurLayer);

	Invalidate(FALSE);
}

void CPackageLayoutEditor_Impl::RefreshData_Comp()
{
	m_vecCompInfo = m_packageSpec.m_CompMapManager.vecCompData;
}

void CPackageLayoutEditor_Impl::RefreshData_Pad()
{
	m_vecPadInfo = m_packageSpec.m_PadMapManager.vecPadData;
}

void CPackageLayoutEditor_Impl::RefreshData_Land()
{
	long vecMapSize = (long)m_vecLandInfo.size();

	if (vecMapSize > 0)
	{
		m_vecLandInfo.clear();
		m_vecLandInfo.resize(vecMapSize);
		m_vecLandInfo = m_packageSpec.m_LandMapConvertOrigin.vecLandData;
	}
	else
	{
		m_vecLandInfo.resize(m_packageSpec.m_LandMapConvertOrigin.GetCount());
		m_vecLandInfo = m_packageSpec.m_LandMapConvertOrigin.vecLandData;
	}
}

void CPackageLayoutEditor_Impl::UpdateMapPosition()
{
	long vecMapSize = m_packageSpec.m_CompMapManager.GetCount();
	if (vecMapSize > 0)
	{
		for (long n = 0; n < vecMapSize; n++)
			m_packageSpec.m_CompMapManager.vecCompData[n].fsrtROI = m_vecCompInfo[n].fsrtROI;
	}

	m_packageSpec.CreateDMSInfo();
}

void CPackageLayoutEditor_Impl::UpdateMapPosition_Pad()
{
	long vecMapSize = m_packageSpec.m_PadMapManager.GetCount();

	if (vecMapSize > 0)
	{
		for (long n = 0; n < vecMapSize; n++)
		{
			m_packageSpec.m_PadMapManager.vecPadData[n].rtROI = m_vecPadInfo[n].rtROI;
			m_packageSpec.m_PadMapManager.vecPadData[n].fsrtSpecROI = m_vecPadInfo[n].fsrtSpecROI;
		}
	}
}

void CPackageLayoutEditor_Impl::UpdateMapPosition_Land()
{
	long vecMapSize = m_packageSpec.m_LandMapConvertOrigin.GetCount();

	if (vecMapSize > 0)
	{
		for (long n = 0; n < vecMapSize; n++)
		{
			m_packageSpec.m_LandMapConvertOrigin.vecLandData[n].rtROI = m_vecLandInfo[n].rtROI;
			m_packageSpec.m_LandMapConvertOrigin.vecLandData[n].fsrtSpecROI = m_vecLandInfo[n].fsrtSpecROI;
		}
	}
	m_packageSpec.UpdateLandData();
}

void CPackageLayoutEditor_Impl::propertyChange_FlipX()
{
	long nCurSelectLayer = m_selectLayer;

	switch (nCurSelectLayer)
	{
	case LayerIndex_Ball:
		RefreshData_Ball(m_systemConfig);
		break;
	case LayerIndex_Land:
		m_packageSpec.m_LandMapConvertOrigin.MirrorMap();
		m_packageSpec.UpdateLandData();
		RefreshData_Land();
		Calc_DrawingPosition_Land();
		break;
	case LayerIndex_Compnent:
		m_packageSpec.m_CompMapManager.MirrorMap();
		RefreshData_Comp();
		Calc_DrawingPosition_Comp();
		break;
	case LayerIndex_Pad:
		m_packageSpec.m_PadMapManager.MirrorMap();
		RefreshData_Pad();
		Calc_DrawingPosition_Pad();
		break;
	default:
		break;
	}

	Update_Background(nCurSelectLayer);
	Update_Foreground(nCurSelectLayer);

	Invalidate(FALSE);
}

void CPackageLayoutEditor_Impl::propertyChange_FlipY()
{
	long nCurSelectLayer = m_selectLayer;

	switch (nCurSelectLayer)
	{
	case LayerIndex_Ball:
		RefreshData_Ball(m_systemConfig);
		break;
	case LayerIndex_Land:
		m_packageSpec.m_LandMapConvertOrigin.FlipMap();
		m_packageSpec.UpdateLandData();
		RefreshData_Land();
		Calc_DrawingPosition_Land();
		break;
	case LayerIndex_Compnent:
		m_packageSpec.m_CompMapManager.FlipMap();
		RefreshData_Comp();
		Calc_DrawingPosition_Comp();
		break;
	case LayerIndex_Pad:
		m_packageSpec.m_PadMapManager.FlipMap();
		RefreshData_Pad();
		Calc_DrawingPosition_Pad();
		break;
	default:
		break;
	}

	Update_Background(nCurSelectLayer);
	Update_Foreground(nCurSelectLayer);

	Invalidate(FALSE);

}

void CPackageLayoutEditor_Impl::propertyChange_Rotation()
{
	long nRotateAngleIndex = long(m_packageSpec.m_ballMapSource.m_ballMapRotation);
	
	//Pad
	m_packageSpec.m_PadMapManager.RotateMap(nRotateAngleIndex);
	RefreshData_Pad();
	Calc_DrawingPosition_Pad();

	//Ball
	RefreshData_Ball(m_systemConfig);

	//Land
	m_packageSpec.m_LandMapConvertOrigin.RotateMap(nRotateAngleIndex);
	m_packageSpec.UpdateLandData();
	RefreshData_Land();
	Calc_DrawingPosition_Land();

	//Comp
	m_packageSpec.m_CompMapManager.RotateMap(nRotateAngleIndex);
	RefreshData_Comp();
	Calc_DrawingPosition_Comp();

	long nCurSelectLayer = m_selectLayer;
	Update_Background(nCurSelectLayer);
	Update_Foreground(nCurSelectLayer);

	Invalidate(FALSE);
}

void CPackageLayoutEditor_Impl::propertyButton_ImportMapData()
{
	CFileDialog dlg(TRUE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("Comma separated values file format (*.csv)|*.csv||"));

	if (dlg.DoModal() != IDOK)
		return;

	long nCurComboIndex = m_selectLayer;
	long nRotateCurSel = long(m_packageSpec.m_ballMapSource.m_ballMapRotation);
	long nRotateCompCurSel = m_packageSpec.m_OriginCompData.m_nRotateCompIdx;

	if (nCurComboIndex == LayerIndex_Pad)
	{
		m_packageSpec.m_OriginPadData.LoadMap(dlg.GetPathName());
		m_packageSpec.SetPadData();
		m_packageSpec.m_PadMapManager.RotateMap(nRotateCurSel);
		RefreshData_Pad();
		Calc_DrawingPosition_Pad();
	}

	if(nCurComboIndex == LayerIndex_Ball)
	{
 		CString errorMessage;

		PackageSpec_BallMapSource bumpMapSource(dlg.GetPathName(), errorMessage);

		if (!errorMessage.IsEmpty())
		{
			::AfxMessageBox(errorMessage, MB_ICONERROR | MB_OK);
			return;
		}

		m_packageSpec.m_ballMapSource = bumpMapSource;
		m_packageSpec.m_ballMapSource.m_ballMapRotation = PackageSpec_BallMapSource::RotationIndex(m_packageSpec.m_OriginPadData.m_nRotateIdx);
		RefreshData_Ball(m_systemConfig);
	}

	if (nCurComboIndex == LayerIndex_Land)
	{
		m_packageSpec.m_OriginLandData.LoadMap(dlg.GetPathName());
		m_packageSpec.SetLandData();
		m_packageSpec.m_LandMapManager.RotateMap(nRotateCurSel);
		m_packageSpec.m_LandMapManager.RotateLand(nRotateCompCurSel);
		RefreshData_Land();
		Calc_DrawingPosition_Land();
	}

	if (nCurComboIndex == LayerIndex_Compnent)
	{
		m_packageSpec.m_OriginCompData.LoadMap(dlg.GetPathName());
		m_packageSpec.SetCompData();
		m_packageSpec.m_CompMapManager.RotateMap(nRotateCurSel);
		m_packageSpec.m_CompMapManager.RotateComp(nRotateCompCurSel);
		RefreshData_Comp();
		Calc_DrawingPosition_Comp();
		//ChipDB Check
		CompDBVerify();
	}

	Update_Background(nCurComboIndex);
	Update_Foreground(nCurComboIndex);

	Invalidate(FALSE);
}

void CPackageLayoutEditor_Impl::CompDBVerify()
{
	std::vector<std::vector<CString>> vec2strCompType;
	vec2strCompType.resize(_typeComponentEnd);

	LoadCompTypeInCompDB(vec2strCompType);

	std::vector<CComponentMap> vecExistCompData;
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
				for (auto &ChipData : vec2strCompType[_typeChip])
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
				BOOL bChipOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType) != existCompType.end();
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
				BOOL bPassiveOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType) != existCompType.end();
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
				for (auto &ArrayData : vec2strCompType[_typeArray])
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
				BOOL bArrayOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType) != existCompType.end();
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
				for (auto &HeatSyncData : vec2strCompType[_typeHeatSync])
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
				BOOL bHeatSyncOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType) != existCompType.end();
				if (!bHeatSyncOverlapValue)
				{
					existCompType.push_back(CompData.strCompType);
					vecExistCompData.push_back(CompData); //없는게 들어가는 정보
				}
			}
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

		CMapDataEditor MapDataEditor;
		DMSPassiveChip PassiveChipDB;
		if (!MapDataEditor.SetPackageSpecData(&m_packageSpec, &PassiveChipDB, vecExistCompData))
			return;

		MapDataEditor.DoModal();
	}
}

BOOL CPackageLayoutEditor_Impl::LoadCompTypeInCompDB(std::vector<std::vector<CString>> &o_vec2strCompType)
{
	if (o_vec2strCompType.size() <= 0)
		return FALSE;

	//Chip
	CString strChipDB(CONFIG_DIRECTORY _T("Chip.ini"));
	int nChipTypeNum = IPVM::IniHelper::LoadINT(strChipDB, _T("Chip"), _T("Chip Type Num"), 0);
	o_vec2strCompType[_typeChip].resize(nChipTypeNum);
	for (long nType = 0; nType < nChipTypeNum; nType++)
	{
		CString strSection;
		strSection.Format(_T("Chip_%d"), nType);
		o_vec2strCompType[_typeChip][nType] = IPVM::IniHelper::LoadSTRING(strChipDB, strSection, _T("Type Name"), _T(""));
	}

	//Passive
	CString strPassiveDB(CONFIG_DIRECTORY _T("Passive.ini"));
	int nPaasiveTypeNum = IPVM::IniHelper::LoadINT(strPassiveDB, _T("PassiveChip"), _T("Chip Type Num"), 0);
	o_vec2strCompType[_typePassive].resize(nPaasiveTypeNum);
	for (long nType = 0; nType < nPaasiveTypeNum; nType++)
	{
		CString strSection;
		strSection.Format(_T("PassiveChip_%d"), nType);
		o_vec2strCompType[_typePassive][nType] = IPVM::IniHelper::LoadSTRING(strPassiveDB, strSection, _T("Type Name"), _T(""));
	}

	//Array
	CString strArrayDB(CONFIG_DIRECTORY _T("Array.ini"));
	int nArrayTypeNum = IPVM::IniHelper::LoadINT(strArrayDB, _T("ArrayChip"), _T("Chip Type Num"), 0);
	o_vec2strCompType[_typeArray].resize(nArrayTypeNum);
	for (long nType = 0; nType < nArrayTypeNum; nType++)
	{
		CString strSection;
		strSection.Format(_T("ArrayChip_%d"), nType);
		o_vec2strCompType[_typeArray][nType] = IPVM::IniHelper::LoadSTRING(strArrayDB, strSection, _T("Type Name"), _T(""));
	}

	//HeatSync
	CString strHeatSyncDB(CONFIG_DIRECTORY _T("Heatsink.ini"));
	int nHeatSyncTypeNum = IPVM::IniHelper::LoadINT(strHeatSyncDB, _T("Heatsink"), _T("Heatsink Type Num"), 0);
	o_vec2strCompType[_typeHeatSync].resize(nHeatSyncTypeNum);
	for (long nType = 0; nType < nHeatSyncTypeNum; nType++)
	{
		CString strSection;
		strSection.Format(_T("Heatsink_%d"), nType);
		o_vec2strCompType[_typeHeatSync][nType] = IPVM::IniHelper::LoadSTRING(strHeatSyncDB, strSection, _T("Type Name"), _T(""));
	}

	return TRUE;
}

void CPackageLayoutEditor_Impl::propertyButton_ExportMapData()
{
	CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("Comma separated values file format (*.csv)|*.csv||"));

	if (dlg.DoModal() != IDOK)
		return;

	long nCurSelectLayer = m_selectLayer;
	
	switch (nCurSelectLayer)
	{
	case LayerIndex_Pad:
		m_packageSpec.m_OriginPadData.SaveMap(dlg.GetPathName());
		break;
	case LayerIndex_Ball:
		m_packageSpec.m_ballMapSource.Export(dlg.GetPathName());
		break;
	case LayerIndex_Land:
		m_packageSpec.m_OriginLandData.SaveMap(dlg.GetPathName());
		break;
	case LayerIndex_Compnent:
		m_packageSpec.m_OriginCompData.SaveMap(dlg.GetPathName());
		break;
	default:
		break;
	}

}

void CPackageLayoutEditor_Impl::propertyChange_RotateComp()
{
	long nCurSelectLayer = m_selectLayer;
	long nRotateCompAngle = m_packageSpec.m_OriginCompData.m_nRotateCompIdx;

	if (nCurSelectLayer == LayerIndex_Compnent)
	{
		m_packageSpec.m_CompMapManager.RotateComp(nRotateCompAngle);
		RefreshData_Comp();
		Calc_DrawingPosition_Comp();

	}

	if (nCurSelectLayer == LayerIndex_Land)
	{
		m_packageSpec.m_LandMapConvertOrigin.RotateLand(nRotateCompAngle);
		m_packageSpec.UpdateLandData();
		RefreshData_Land();
		Calc_DrawingPosition_Land();
	}

	Update_Background(nCurSelectLayer);
	Update_Foreground(nCurSelectLayer);

	Invalidate(FALSE);
}


void CPackageLayoutEditor_Impl::propertyButton_ClearMapData()
{
	long nCurLayerIndex = m_selectLayer;
	CString strWarringMsg;

	switch (nCurLayerIndex)
	{
	case LayerIndex_Ball:
		strWarringMsg = _T("[Warring] Ball MapData is Clear.. Are you sure you want to delete?");
		if (::SimpleMessage(strWarringMsg, MB_OKCANCEL) == IDOK)
			m_packageSpec.m_ballMapSource.m_balls.clear();
		else
			return;
		break;
	case LayerIndex_Land:
		strWarringMsg = _T("[Warring] Land MapData is Clear.. Are you sure you want to delete?");
		if (::SimpleMessage(strWarringMsg, MB_OKCANCEL) == IDOK)
			m_packageSpec.m_OriginLandData.vecOriginLandData.clear();
		else
			return;
		break;
	case LayerIndex_Compnent:
		strWarringMsg = _T("[Warring] Component MapData is Clear.. Are you sure you want to delete?");
		if (::SimpleMessage(strWarringMsg, MB_OKCANCEL) == IDOK)
			m_packageSpec.m_OriginCompData.vecOriginCompData.clear();
		else
			return;
		break;
	case LayerIndex_Pad:
		strWarringMsg = _T("[Warring] Pad MapData is Clear.. Are you sure you want to delete?");
		if (::SimpleMessage(strWarringMsg, MB_OKCANCEL) == IDOK)
			m_packageSpec.m_OriginPadData.vecOriginPadData.clear();
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
}

BOOL CPackageLayoutEditor_Impl::RefreshData(long i_nLayerIndex)
{
	if (i_nLayerIndex < 0) //혹시몰라 예외처리.. 음수값이면 쓰레기값이다.
		return FALSE;

	switch (i_nLayerIndex)
	{
	case LayerIndex_Ball:
		RefreshData_Ball(m_systemConfig);
		break;
	case LayerIndex_Land:
		m_packageSpec.SetLandData();
		Calc_DrawingPosition_Land();
		RefreshData_Land();
		break;
	case LayerIndex_Compnent:
		m_packageSpec.SetCompData();
		Calc_DrawingPosition_Comp();
		RefreshData_Comp();
		break;
	case LayerIndex_Pad:
		for (long i = 0; i < 3; i++)
			m_ptPinIndex[i] = CPoint(0, 0);
		m_packageSpec.SetPadData();
		Calc_DrawingPosition_Pad();
		RefreshData_Pad();
		break;
	default:
		break;
	}

	return TRUE;
}

LRESULT CPackageLayoutEditor_Impl::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
	if (wparam != XTP_PGN_ITEMVALUE_CHANGED) return 0;

	CXTPPropertyGridItem *item = (CXTPPropertyGridItem *)lparam;

	if (auto *value = dynamic_cast<CCustomItemButton *>(item))
	{
		switch (item->GetID())
		{
		case ePID_BodySize:				propertyButton_BodySize();			break;
		case ePID_LayerImportData:		propertyButton_ImportMapData();		break;
		case ePID_LayerExportData:		propertyButton_ExportMapData();		break;
		case ePID_LayerClearMapData:	propertyButton_ClearMapData();		break;
		}

		value->SetBool(FALSE);
		return 0;
	}

	if (auto *value = dynamic_cast<CXTPPropertyGridItemEnum *>(item))
	{
		int data = value->GetEnum();
		data = (int)max(data, 0);
		switch (item->GetID())
		{
		case ePID_Rotate:
			m_packageSpec.m_OriginPadData.m_nRotateIdx = data;
			m_packageSpec.m_ballMapSource.m_ballMapRotation = PackageSpec_BallMapSource::RotationIndex(data);
			m_packageSpec.m_OriginLandData.m_nRotateIdx = data;
			m_packageSpec.m_OriginCompData.m_nRotateIdx = data;
			propertyChange_Rotation();
			break;

		case ePID_RotateComp:
			m_packageSpec.m_OriginCompData.m_nRotateCompIdx = data;
			m_packageSpec.m_OriginLandData.m_nRotateLandIdx = data;
			propertyChange_RotateComp();
			break;

		case ePID_SelectLayer:
			m_selectLayer = data;
			propertyChange_SelectLayer();
			break;

		case ePID_SelectionMethod:
			m_selectionMethod = data;
			break;
		}

		return 0;
	}

	if (auto *value = dynamic_cast<CXTPPropertyGridItemBool *>(item))
	{
		BOOL data = value->GetBool();
		switch (item->GetID())
		{
		case ePID_LayerFlipX:
			getCurrentLayer_FlipX() = data;
			propertyChange_FlipX();
			break;

		case ePID_LayerFlipY:
			getCurrentLayer_FlipY() = data;
			propertyChange_FlipY();
			break;
		}
	}

	return 0;
}
