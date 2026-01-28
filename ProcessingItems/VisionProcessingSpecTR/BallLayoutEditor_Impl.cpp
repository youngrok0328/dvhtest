// BallLayoutEditor_Impl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "BallLayoutEditor_Impl.h"
#include "DlgVisionProcessingSpec.h"
#include "DlgVisionProcessingSpec.h"
#include "VisionProcessingSpec.h"
#include "BodySizeEditor.h"

#include "../../PI_Modules/dPI_DataBase/FigureFactory.h"
#include "../../PI_Modules/dPI_DataBase/Figure.h"
#include "../../PI_Modules/dPI_DataBase/Figure_Ellipse.h"
#include "../../PI_Modules/dPI_DataBase/Figure_Rectangle.h"
#include "../../PI_Modules/dPI_DataBase/Figure_Polygon.h"
#include "../../PI_Modules/dPI_GridCtrl\GridCellCheck2.h"
#include "../../VisionCommon/VisionBaseDef.h"

#include "Base/Image_8u_C1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_CUSTOMOBJECTTRANSFER	(WM_USER+4445)		// WPARAM 0 : Normal Mode
// WPARAM 1 : Overwrap Mode
/////////////////////////////////////////////////////////////////////////////
// CBallLayoutEditor_Impl dialog

CBallLayoutEditor_Impl::CBallLayoutEditor_Impl(SystemConfig &systemConfig, CPackageSpec &packageSpec, long nCurrentAccessMode, const CRect &rtPosition, CWnd* pParent /*=NULL*/)
	: CDialog(CBallLayoutEditor_Impl::IDD, pParent)
	, m_systemConfig(systemConfig)
	, m_packageSpec(packageSpec)
	, m_rtInitPosition(rtPosition)
	, m_propertyGrid(new CXTPPropertyGrid)
{
	//{{AFX_DATA_INIT(CBallLayoutEditor_Impl)
	//}}AFX_DATA_INIT
}

CBallLayoutEditor_Impl::~CBallLayoutEditor_Impl()
{
	delete m_propertyGrid;
}

void CBallLayoutEditor_Impl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBallLayoutEditor_Impl)
	DDX_Control(pDX, IDC_COMBO_LAYER, m_comboLayer);
	DDX_Control(pDX, IDC_STC_VIEWER, m_stcViewer);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CHECK_BALL_MAP_FLIP_X, m_checkBallMapFlipX);
	DDX_Control(pDX, IDC_CHECK_BALL_MAP_FLIP_Y, m_checkBallMapFlipY);
	DDX_Control(pDX, IDC_COMBO_BALL_MAP_ROTATION, m_comboBallMapRotation);
}


BEGIN_MESSAGE_MAP(CBallLayoutEditor_Impl, CDialog)
	//{{AFX_MSG_MAP(CBallLayoutEditor_Impl)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_COMBO_LAYER, OnSelchangeComboLayer)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_BODY_SIZE, OnButtonEditBodySize)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT_MAP_DATA, &CBallLayoutEditor_Impl::OnBnClickedButtonImportMapData)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_MAP_DATA, &CBallLayoutEditor_Impl::OnBnClickedButtonExportMapData)
	ON_WM_CLOSE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_BALL_MAP_FLIP_X, &CBallLayoutEditor_Impl::OnBnClickedCheckBallMapFlipX)
	ON_BN_CLICKED(IDC_CHECK_BALL_MAP_FLIP_Y, &CBallLayoutEditor_Impl::OnBnClickedCheckBallMapFlipY)
	ON_CBN_SELCHANGE(IDC_COMBO_BALL_MAP_ROTATION, &CBallLayoutEditor_Impl::OnCbnSelchangeComboBallMapRotation)
	ON_BN_CLICKED(IDC_BUTTON_LAYER_NAME, &CBallLayoutEditor_Impl::OnBnClickedButtonLayerName)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_COMP_DATA, &CBallLayoutEditor_Impl::OnBnClickedButtonRotateCompData)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBallLayoutEditor_Impl message handlers
BOOL CBallLayoutEditor_Impl::OnInitDialog() 
{
	CDialog::OnInitDialog();

	MoveWindow(m_rtInitPosition, FALSE);

	CRect rtPropertyGrid;
	GetDlgItem(IDC_FRAME_PROPERTY_GRID)->GetWindowRect(rtPropertyGrid);
	ScreenToClient(rtPropertyGrid);
	m_propertyGrid->Create(rtPropertyGrid, this, IDC_FRAME_PROPERTY_GRID);

	m_checkBallMapFlipX.SetCheck(m_packageSpec.m_ballMapSource.m_ballMapFlipX ? BST_CHECKED : BST_UNCHECKED);
	m_checkBallMapFlipY.SetCheck(m_packageSpec.m_ballMapSource.m_ballMapFlipY ? BST_CHECKED : BST_UNCHECKED);
	m_comboBallMapRotation.SetCurSel(long(m_packageSpec.m_ballMapSource.m_ballMapRotation));

	((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_INVERT))->SetCheck(TRUE);

	CRect rtClient;
	GetClientRect(rtClient);

	m_stcViewer.MoveWindow(::GetContentSubArea(rtClient, 1, 3, 3, 0, 1, 1));

	//mc_분기작업 시작
	m_vecstrLayerName.clear();
	m_vecstrLayerName.resize(4); //고정값

	if (m_packageSpec.nPackageType == enPackageType::BGA)
		m_vecstrLayerName[0] = "Ball Layer";
	else if(m_packageSpec.nPackageType == enPackageType::LGA)
		m_vecstrLayerName[0] = "Land Layer";

	m_vecstrLayerName[1] = "Component Layer";
	m_vecstrLayerName[2] = "Pad Layer";
	m_vecstrLayerName[3] = "Whole Device";

	Update_Control(m_vecstrLayerName);
	Update_BtnControl();
	Initialize_View();

	m_comboLayer.SetCurSel(0);
	OnSelchangeComboLayer();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CBallLayoutEditor_Impl::OnPaint() 
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

void CBallLayoutEditor_Impl::Update_Control(std::vector<CString> i_vecstrLayerName)
{
	if (i_vecstrLayerName.size() <= 0 || i_vecstrLayerName.size() > 4)  //각 항목당 Layer는 1개씩만 존재함으로 4개 이상 할당되면 안된다.
		return;

	m_comboLayer.ResetContent();
	
	for (long n = 0; n < i_vecstrLayerName.size(); n++)
		m_comboLayer.AddString(i_vecstrLayerName[n]);

}

void CBallLayoutEditor_Impl::Update_BtnControl()
{
	long nCurLayer = m_comboLayer.GetCurSel();

	if (nCurLayer == LayerIndex_Whole) //WholeDevice면 모든 BtnCtrl 비활성화
	{
		//Button
		((CEdit *)GetDlgItem(IDC_EDIT_LAYER_NAME))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_BUTTON_LAYER_NAME))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_BUTTON_IMPORT_MAP_DATA))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_BUTTON_EXPORT_MAP_DATA))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_BUTTON_ROTATE_COMP_DATA))->EnableWindow(FALSE);

		//Check RadioBox
		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_X))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_Y))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_ENABLE))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_DISABLE))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_INVERT))->EnableWindow(FALSE);

		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_X))->SetCheck(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_Y))->SetCheck(FALSE);
	}
	else
	{
		//Button
		((CEdit *)GetDlgItem(IDC_EDIT_LAYER_NAME))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_BUTTON_LAYER_NAME))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_BUTTON_IMPORT_MAP_DATA))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_BUTTON_EXPORT_MAP_DATA))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_BUTTON_ROTATE_COMP_DATA))->EnableWindow(TRUE);

		//Check & Radio Box
		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_X))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_Y))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_ENABLE))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_DISABLE))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_INVERT))->EnableWindow(TRUE);

		//ComboBox
		((CComboBox *)GetDlgItem(IDC_COMBO_BALL_MAP_ROTATION))->EnableWindow(TRUE);
	}

	if (nCurLayer != LayerIndex_Compnent) //CompLayer가 아니면
		((CButton *)GetDlgItem(IDC_BUTTON_ROTATE_COMP_DATA))->EnableWindow(FALSE); //Rotate Comp Btn 비활성화

	//{{각 Layer의 Check할당 여부를 기억한다.
	switch (nCurLayer)
	{
	case LayerIndex_Main:
		if (m_packageSpec.nPackageType == enPackageType::BGA)
		{
			((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_X))->SetCheck(m_packageSpec.m_ballMapSource.m_ballMapFlipX);
			((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_Y))->SetCheck(m_packageSpec.m_ballMapSource.m_ballMapFlipY);
		}
		else if (m_packageSpec.nPackageType == enPackageType::LGA)
		{
		}
		break;
	case LayerIndex_Compnent:
		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_X))->SetCheck(m_packageSpec.m_CompMapManager.m_bCompMapFlipX);
		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_Y))->SetCheck(m_packageSpec.m_CompMapManager.m_bCompMapFlipY);
		break;
	case LayerIndex_Pad:
		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_X))->SetCheck(m_packageSpec.m_PadMapManager.m_bPadMapFlipX);
		((CButton *)GetDlgItem(IDC_CHECK_BALL_MAP_FLIP_Y))->SetCheck(m_packageSpec.m_PadMapManager.m_bPadMapFlipY);
	default:
		break;
	}
	//}}
}

void CBallLayoutEditor_Impl::Update_Background(long i_nCurLayerIndex)
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
	CPen penBallIndex(PS_SOLID, 1, RGB(0, 230, 0));
	MemDC.SelectObject(&penBallIndex);
	CBrush brushBallIndex(RGB(0, 230, 0));
	MemDC.SelectObject(&brushBallIndex);

	MemDC.Polygon(m_ptPinIndex, 4);

	if(i_nCurLayerIndex == LayerIndex_Whole)
	{
		// Index by Bottom View
		CPen penBallIndex(PS_SOLID, 1, RGB(0, 230, 0));
		MemDC.SelectObject(&penBallIndex);
		CBrush brushBallIndex(RGB(0, 230, 0));
		MemDC.SelectObject(&brushBallIndex);

		MemDC.Polygon(m_ptPinIndex, 4);
	}

	MemDC.SelectObject(OldBitmap);
	MemDC.SelectObject(pOldBrush);
	MemDC.SelectObject(pOldPen);
}

void CBallLayoutEditor_Impl::Update_Foreground(long i_nCurLayerIndex)
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
	case LayerIndex_Main:
		if(m_packageSpec.nPackageType == enPackageType::BGA)
			SetDrawing_MainForegroundData(i_nCurLayerIndex, pDC);
		break;
	case LayerIndex_Compnent:
		SetDrawing_CompForegroundData(i_nCurLayerIndex, pDC);
		break;
	case LayerIndex_Pad:
		SetDrawing_PadForegroundData(i_nCurLayerIndex, pDC);
		break;
	case LayerIndex_Whole:
		SetDrawing_WholeForegroundData(i_nCurLayerIndex, pDC);
		break;
	}
	//

	ReleaseDC(pDC);
}

void CBallLayoutEditor_Impl::SetDrawing_MainForegroundData(long i_nCurLayerIndex, CDC *i_pDC)
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

	if (m_packageSpec.nPackageType == enPackageType::BGA)
	{
		for (const auto &ballInfo : m_vecBallInfo)
		{
			if (ballInfo.m_ignored)
			{
				MemDC.SelectObject(GetStockObject(NULL_BRUSH));
			}
			else
			{
				MemDC.SelectObject(&brushMain);
			}

			MemDC.Ellipse(ballInfo.m_ballPosition);
		}
	}

	//if (m_packageSpec.nPackageType == enPackageType::LGA)
	//{

	//}

	MemDC.SelectObject(pOldBrush);

	MemDC.SelectObject(pOldBitmap);
}

void CBallLayoutEditor_Impl::SetDrawing_CompForegroundData(long i_nCurLayerIndex, CDC *i_pDC)
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
		if (CompInfo.bVisible)
		{
			MemDC.SelectObject(GetStockObject(NULL_BRUSH));
		}
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

void CBallLayoutEditor_Impl::SetDrawing_PadForegroundData(long i_nCurLayerIndex, CDC *i_pDC)
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
		if (PadInfo.bVisible)
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

void CBallLayoutEditor_Impl::GetPinIndexPoint(FPI_RECT i_frtPinIndex, CPoint *o_pPinIndexPoint)
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

void CBallLayoutEditor_Impl::SetDrawing_WholeForegroundData(long i_nCurLayerIndex, CDC *i_pDC)
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
	COLORREF rgbWholeDevice_Ball = RGB(128, 245, 255);

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

	COLORREF rgbWholeDevice_Comp = RGB(255, 255, 72);
	CBrush brushComp(rgbWholeDevice_Comp);
	CBrush *pComp_OldBrush = MemDC.SelectObject(&brushComp);

	CPoint ptCompROI[4];

	for (const auto &CompInfo : m_vecCompInfo)
	{
		if(CompInfo.bVisible)
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
	MemDC.SelectObject(pComp_OldBrush);
	MemDC.SelectObject(pPad_OldBrush);

	MemDC.SelectObject(pOldBitmap);
}

void CBallLayoutEditor_Impl::Update_PropertyGrid(long nLayer)
{
	m_propertyGrid->ResetContent();

	if(nLayer == LayerIndex_Main)
	{
	}
	else if (nLayer == LayerIndex_Compnent)
	{

	}
	else if (nLayer == LayerIndex_Pad)
	{

	}
	else if(nLayer == LayerIndex_Whole)
	{
		auto *ballLayer = m_propertyGrid->AddCategory(_T("Ball Layer"));
		{
			auto *ballCounts = ballLayer->AddChildItem(new CXTPPropertyGridItemNumber(_T("Ball counts"), long(m_packageSpec.m_ballMap.m_balls.size())));
			ballCounts->SetReadOnly();

			ballLayer->Expand();
		}

		auto *CompLayer = m_propertyGrid->AddCategory(_T("Compnent Layer"));
		{
			auto *CompCounts = CompLayer->AddChildItem(new CXTPPropertyGridItemNumber(_T("Comp counts"), long(m_packageSpec.m_CompMapManager.vecCompData.size())));
			CompCounts->SetReadOnly();

			CompLayer->Expand();
		}

		auto *PadLayer = m_propertyGrid->AddCategory(_T("Pad Layer"));
		{
			auto *PadCounts = PadLayer->AddChildItem(new CXTPPropertyGridItemNumber(_T("Pad counts"), long(m_packageSpec.m_PadMapManager.vecPadData.size())));

			PadCounts->SetReadOnly();

			PadLayer->Expand();
		}

		auto *TotalCount = m_propertyGrid->AddCategory(_T("Total Counts"));
		{
			long nMainLayerObejctNum = GetMainLayerObjectNum();
			long nComponetObejectNum = (long)m_packageSpec.m_CompMapManager.vecCompData.size();
			long nPadObejctNum = (long)m_packageSpec.m_PadMapManager.vecPadData.size();

			long nTotalObjectNum = nMainLayerObejctNum + nComponetObejectNum + nPadObejctNum;

			auto *TotalCounts = TotalCount->AddChildItem(new CXTPPropertyGridItemNumber(_T("Total counts"), nTotalObjectNum));

			TotalCounts->SetReadOnly();

			TotalCount->Expand();
		}
	}

	m_propertyGrid->SetViewDivider(0.50);
	m_propertyGrid->HighlightChangedItems(TRUE);
}

long CBallLayoutEditor_Impl::GetMainLayerObjectNum()
{
	long nObjectNum(0);

	switch (m_packageSpec.nPackageType)
	{
	case enPackageType::BGA:
		nObjectNum = (long)m_packageSpec.m_ballMap.m_balls.size();
		break;
	case enPackageType::LGA:
		nObjectNum = (long)m_packageSpec.vecLandInfo.size();
		break;
	default:
		break;
	}

	return nObjectNum;
}

void CBallLayoutEditor_Impl::Initialize_View()
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

	m_fZoom = m_rtDevice.Width() / (m_packageSpec.m_bodyInfoMaster.fBodySizeX * 25.4f);
	m_fZoom = m_rtDevice.Height() / (m_packageSpec.m_bodyInfoMaster.fBodySizeY * 25.4f);
}

void CBallLayoutEditor_Impl::OnLButtonDown(UINT nFlags, CPoint point) 
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

		long nEditMode(0);

		if (((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_ENABLE))->GetCheck())
			nEditMode = 0;
		else if (((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_DISABLE))->GetCheck())
			nEditMode = 1;
		else if (((CButton *)GetDlgItem(IDC_RADIO_SELECTED_BALL_INVERT))->GetCheck())
			nEditMode = 2;

		//분리
		long nCurSelectLayer = m_comboLayer.GetCurSel();
		switch (nCurSelectLayer)
		{
		case LayerIndex_Main:
			Ignore_Ball(rtSelection, nEditMode);
			break;
		case LayerIndex_Compnent:
			Ignore_Comp(rtSelection, nEditMode);
			break;
		case LayerIndex_Pad:
			Ignore_Pad(rtSelection, nEditMode);
			break;
		default:
			break;
		}

		//
		Update_Foreground(m_comboLayer.GetCurSel());

		Invalidate(FALSE);
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CBallLayoutEditor_Impl::Ignore_Ball(CRect i_rtSelect, long i_nEditMode)
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

	m_packageSpec.CalcDependentVariables(m_systemConfig, m_packageSpec.bDeadBug);
	Calc_DrawingPosition_Ball();
}

void CBallLayoutEditor_Impl::Ignore_Comp(CRect i_rtSelect, long i_nEditMode)
{
	CRect rtTemp;

	for (auto &CompInfo : m_packageSpec.m_CompMapManager.vecCompData)
	{
		CRect rtSpecROI = IPVM::ToMFC(CompInfo.fsrtROI.GetCRect());

		if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
		{
			if (i_nEditMode == 0)
				CompInfo.bVisible = FALSE;
			else if (i_nEditMode == 1)
				CompInfo.bVisible = TRUE;
			else
				CompInfo.bVisible = !CompInfo.bVisible;
		}

	}

	RefreshData_Comp();
	Calc_DrawingPosition_Comp();
}

void CBallLayoutEditor_Impl::Ignore_Pad(CRect i_rtSelect, long i_nEditMode)
{
	CRect rtTemp;

	for (auto &PadInfo : m_packageSpec.m_PadMapManager.vecPadData)
	{
		CRect rtSpecROI = IPVM::ToMFC(PadInfo.fsrtSpecROI.GetCRect());

		if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
		{
			if (i_nEditMode == 0)
				PadInfo.bVisible = FALSE;
			else if (i_nEditMode == 1)
				PadInfo.bVisible = TRUE;
			else
				PadInfo.bVisible = !PadInfo.bVisible;
		}
	}

	RefreshData_Pad();
	Calc_DrawingPosition_Pad();
}

void CBallLayoutEditor_Impl::OnSelchangeComboLayer()
{
	long nSelectLayer = m_comboLayer.GetCurSel();

	if (m_packageSpec.nPackageType == enPackageType::BGA && nSelectLayer == LayerIndex_Main)
		Calc_DrawingPosition_Ball();

	if (m_packageSpec.nPackageType == enPackageType::LGA && nSelectLayer == LayerIndex_Main)
		Calc_DrawingPosition_Land();

	if (nSelectLayer == LayerIndex_Compnent)
		Calc_DrawingPosition_Comp();

	if (nSelectLayer == LayerIndex_Pad)
		Calc_DrawingPosition_Pad();

	else if (nSelectLayer == LayerIndex_Whole) //처음에 바로 Whole 들어올때 
	{
		Calc_DrawingPosition_Ball();
		Calc_DrawingPosition_Comp();
		Calc_DrawingPosition_Pad();
	}

	Update_Background(nSelectLayer);
	Update_Foreground(nSelectLayer);
	Update_PropertyGrid(nSelectLayer); //Whole Device 관련
	Update_BtnControl();

	((CEdit *)GetDlgItem(IDC_EDIT_LAYER_NAME))->SetWindowText(m_vecstrLayerName[nSelectLayer]);

	Invalidate(FALSE);
}

void CBallLayoutEditor_Impl::OnButtonEditBodySize() 
{
	CBodySizeEditor Dlg(m_packageSpec);

	if(Dlg.DoModal() == IDOK)
	{
		if(Dlg.m_bChangedDeadBug)
			::AfxMessageBox(_T("Dead bug option has been changed.\nThis option will take effect after job save."));

		long nCurLayer = m_comboLayer.GetCurSel();

		Initialize_View();

		switch (nCurLayer)
		{
		case LayerIndex_Main:
			if (m_packageSpec.nPackageType == enPackageType::BGA)
				Calc_DrawingPosition_Ball();
			else if (m_packageSpec.nPackageType == enPackageType::LGA)
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

float CBallLayoutEditor_Impl::GetIndexSize()
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

void CBallLayoutEditor_Impl::Calc_DrawingPosition_Ball()
{
	//float fDrawIndex = m_fZoom * GetIndexSize();	
	//m_ptPinIndex[0].x = long(m_rtDevice.left + 3 + 0.5f);
	//m_ptPinIndex[0].y = long(m_rtDevice.top + 3 + 0.5f);
	//m_ptPinIndex[1].x = long(m_rtDevice.left + 3 + 0.5f);
	//m_ptPinIndex[1].y = long(m_rtDevice.top + 3 + 0.8f * fDrawIndex + 0.5f);
	//m_ptPinIndex[2].x = long(m_rtDevice.left + 3 + 0.8f * fDrawIndex + 0.5f);
	//m_ptPinIndex[2].y = long(m_rtDevice.top + 3 + 0.5f);
	//m_ptPinIndex[3].x = long(m_rtDevice.left + 3 + 0.5f);
	//m_ptPinIndex[3].y = long(m_rtDevice.top + 3 + 0.5f);

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

void CBallLayoutEditor_Impl::Calc_DrawingPosition_Comp()
{
	//float fDrawIndex = m_fZoom * m_packageSpec.m_CompMapManager.vecCompData.size();
	//m_ptPinIndex[0].x = long(m_rtDevice.left + 3 + 0.5f);
	//m_ptPinIndex[0].y = long(m_rtDevice.top + 3 + 0.5f);
	//m_ptPinIndex[1].x = long(m_rtDevice.left + 3 + 0.5f);
	//m_ptPinIndex[1].y = long(m_rtDevice.top + 3 + 0.8f * fDrawIndex + 0.5f);
	//m_ptPinIndex[2].x = long(m_rtDevice.left + 3 + 0.8f * fDrawIndex + 0.5f);
	//m_ptPinIndex[2].y = long(m_rtDevice.top + 3 + 0.5f);
	//m_ptPinIndex[3].x = long(m_rtDevice.left + 3 + 0.5f);
	//m_ptPinIndex[3].y = long(m_rtDevice.top + 3 + 0.5f);

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
		/*float tmpOffsetX = ((m_packageSpec.m_bodyInfoMaster.fBodySizeX * 25.4f) * 0.5f) + m_vecCompInfo[n].fOffsetX * 1000.f;
		float tmpOffsetY = ((m_packageSpec.m_bodyInfoMaster.fBodySizeY * 25.4f) * 0.5f) + m_vecCompInfo[n].fOffsetY * 1000.f;

		fOffsetX = (tmpOffsetX * m_fZoom) + (float)m_rtDevice.left;
		fOffsetY = (tmpOffsetY * m_fZoom) + (float)m_rtDevice.top;*/

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
			IPVM::Point_32f_C2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom), IPVM::Point_32f_C2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom));

		m_vecCompInfo[n].fsrtROI = frtChip; //SpecPos
	}

	UpdateMapPosition();
}

void CBallLayoutEditor_Impl::Calc_DrawingPosition_Pad()
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
		/*float tmpOffsetX = ((m_packageSpec.m_bodyInfoMaster.fBodySizeX * 25.4f) * 0.5f) + m_vecPadInfo[n].fOffsetX * 1000.f;
		float tmpOffsetY = ((m_packageSpec.m_bodyInfoMaster.fBodySizeY * 25.4f) * 0.5f) + m_vecPadInfo[n].fOffsetY * 1000.f;

		fOffsetX = (tmpOffsetX * m_fZoom) + (float)m_rtDevice.left;
		fOffsetY = (tmpOffsetY * m_fZoom) + (float)m_rtDevice.top;*/

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
			IPVM::Point_32f_C2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom), IPVM::Point_32f_C2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom));

		m_vecPadInfo[n].fsrtSpecROI = frtChip;
	}

	UpdateMapPosition_Pad();
}

void CBallLayoutEditor_Impl::Calc_DrawingPosition_Land()
{

}

void CBallLayoutEditor_Impl::OnClose() 
{
	HWND hWnd = ::FindWindow(NULL, _T("IntekPlus Gerber Extractor"));

	if(hWnd != NULL)
	{
		::PostMessage(hWnd, WM_CLOSE, 0, 0);
	}

	m_propertyGrid->DestroyWindow();

	CDialog::OnClose();
}

void CBallLayoutEditor_Impl::Refresh(SystemConfig &systemParam, BOOL bDeadBug)
{
	m_packageSpec.CalcDependentVariables(systemParam, bDeadBug);

	long nCurLayer = m_comboLayer.GetCurSel();

	Initialize_View();

	Calc_DrawingPosition_Ball();

	Update_Background(nCurLayer);
	Update_Foreground(nCurLayer);

	Invalidate(FALSE);
}

void CBallLayoutEditor_Impl::RefreshData_Comp()
{
	long vecMapSize = m_packageSpec.m_CompMapManager.GetCount();

	if (vecMapSize > 0)
	{
		m_vecCompInfo.clear();
		m_vecCompInfo.resize(vecMapSize);
		m_vecCompInfo = m_packageSpec.m_CompMapManager.vecCompData;
	}
}

void CBallLayoutEditor_Impl::RefreshData_Pad()
{
	long vecMapSize = m_packageSpec.m_PadMapManager.GetCount();
	if (vecMapSize > 0)
	{
		m_vecPadInfo.clear();
		m_vecPadInfo.resize(vecMapSize);
		m_vecPadInfo = m_packageSpec.m_PadMapManager.vecPadData;
	}
}

void CBallLayoutEditor_Impl::UpdateMapPosition()
{
	long vecMapSize = m_packageSpec.m_CompMapManager.GetCount();
	if (vecMapSize > 0)
	{
		for (long n = 0; n < vecMapSize; n++)
			m_packageSpec.m_CompMapManager.vecCompData[n].fsrtROI = m_vecCompInfo[n].fsrtROI;
	}

	m_packageSpec.CreateDMSInfo();
}

void CBallLayoutEditor_Impl::UpdateMapPosition_Pad()
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

void CBallLayoutEditor_Impl::OnBnClickedCheckBallMapFlipX() //Mirror
{
	long nCurSelectLayer = m_comboLayer.GetCurSel();

	switch (nCurSelectLayer)
	{
	case LayerIndex_Main:
		if (m_packageSpec.nPackageType == enPackageType::BGA)
		{
			m_packageSpec.m_ballMapSource.m_ballMapFlipX = m_checkBallMapFlipX.GetCheck() == BST_CHECKED;
			Refresh(m_systemConfig, m_packageSpec.bDeadBug);
		}
		else if (m_packageSpec.nPackageType == enPackageType::LGA)
		{
		}
		break;
	case LayerIndex_Compnent:
		m_packageSpec.m_CompMapManager.MirrorMap();
		RefreshData_Comp();
		Calc_DrawingPosition_Comp();
		m_packageSpec.m_CompMapManager.m_bCompMapFlipX = m_checkBallMapFlipX.GetCheck() == BST_CHECKED ? TRUE : FALSE;
		break;
	case LayerIndex_Pad:
		m_packageSpec.m_PadMapManager.MirrorMap();
		RefreshData_Pad();
		Calc_DrawingPosition_Pad();
		m_packageSpec.m_PadMapManager.m_bPadMapFlipX = m_checkBallMapFlipX.GetCheck() == BST_CHECKED ? TRUE : FALSE;
		break;
	default:
		break;
	}

	Update_Background(nCurSelectLayer);
	Update_Foreground(nCurSelectLayer);

	Invalidate(FALSE);
}

void CBallLayoutEditor_Impl::OnBnClickedCheckBallMapFlipY() //Flip
{
	long nCurSelectLayer = m_comboLayer.GetCurSel();

	switch (nCurSelectLayer)
	{
	case LayerIndex_Main:
		if (m_packageSpec.nPackageType == enPackageType::BGA)
		{
			m_packageSpec.m_ballMapSource.m_ballMapFlipY = m_checkBallMapFlipY.GetCheck() == BST_CHECKED;
			Refresh(m_systemConfig, m_packageSpec.bDeadBug);
		}
		else if (m_packageSpec.nPackageType == enPackageType::LGA)
		{
		}
		break;
	case LayerIndex_Compnent:
		m_packageSpec.m_CompMapManager.FlipMap();
		RefreshData_Comp();
		Calc_DrawingPosition_Comp();
		m_packageSpec.m_CompMapManager.m_bCompMapFlipY = m_checkBallMapFlipY.GetCheck() == BST_CHECKED ? TRUE : FALSE;
		break;
	case LayerIndex_Pad:
		m_packageSpec.m_PadMapManager.FlipMap();
		RefreshData_Pad();
		Calc_DrawingPosition_Pad();
		m_packageSpec.m_PadMapManager.m_bPadMapFlipY = m_checkBallMapFlipY.GetCheck() == BST_CHECKED ? TRUE : FALSE;
		break;
	default:
		break;
	}

	Update_Background(nCurSelectLayer);
	Update_Foreground(nCurSelectLayer);

	Invalidate(FALSE);

}

void CBallLayoutEditor_Impl::OnCbnSelchangeComboBallMapRotation()
{
	//한번 돌릴때 다같이 적용 될수 있도록한다.
	long nRotateAngleIndex = m_comboBallMapRotation.GetCurSel();
	m_packageSpec.m_ballMapSource.m_ballMapRotation = PackageSpec_BallMapSource::RotationIndex(nRotateAngleIndex);
	Refresh(m_systemConfig, m_packageSpec.bDeadBug);

	//m_packageSpec.m_CompMapManager.RotateMap(nRotateAngleIndex);
	RefreshData_Comp();
	Calc_DrawingPosition_Comp();

	//m_packageSpec.m_PadMapManager.RotateMap(nRotateAngleIndex);
	RefreshData_Pad();
	Calc_DrawingPosition_Pad();

	long nCurSelectLayer = m_comboLayer.GetCurSel();
	Update_Background(nCurSelectLayer);
	Update_Foreground(nCurSelectLayer);

	Invalidate(FALSE);
}

void CBallLayoutEditor_Impl::OnBnClickedButtonImportMapData()
{
	CFileDialog dlg(TRUE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("Comma separated values file format (*.csv)|*.csv||"));

	if (dlg.DoModal() != IDOK)
		return;

	long nCurComboIndex = m_comboLayer.GetCurSel();
	if(m_packageSpec.nPackageType == enPackageType::BGA && nCurComboIndex == LayerIndex_Main)
	{
 		CString errorMessage;

		PackageSpec_BallMapSource bumpMapSource(dlg.GetPathName(), errorMessage);

		if (!errorMessage.IsEmpty())
		{
			::AfxMessageBox(errorMessage, MB_ICONERROR | MB_OK);
			return;
		}

		m_packageSpec.m_ballMapSource = bumpMapSource;
		Refresh(m_systemConfig, m_packageSpec.bDeadBug);
	}

	if (m_packageSpec.nPackageType == enPackageType::LGA && nCurComboIndex == LayerIndex_Main)
	{

	}

	if (nCurComboIndex == LayerIndex_Compnent)
	{
		m_packageSpec.m_CompMapManager.LoadMap(dlg.GetPathName());
		RefreshData_Comp();
		Calc_DrawingPosition_Comp();
	}

	if (nCurComboIndex == LayerIndex_Pad)
	{
		m_packageSpec.m_PadMapManager.LoadMap(dlg.GetPathName());
		RefreshData_Pad();
		Calc_DrawingPosition_Pad();
	}

	Update_Background(nCurComboIndex);
	Update_Foreground(nCurComboIndex);

	Invalidate(FALSE);
}

void CBallLayoutEditor_Impl::OnBnClickedButtonExportMapData()
{
	CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("Comma separated values file format (*.csv)|*.csv||"));

	if (dlg.DoModal() != IDOK)
		return;

	long nCurSelectLayer = m_comboLayer.GetCurSel();
	
	switch (nCurSelectLayer)
	{
	case LayerIndex_Main:
		if(m_packageSpec.nPackageType == enPackageType::BGA)
			m_packageSpec.m_ballMapSource.Export(dlg.GetPathName());
		//else if
			//Land
		break;
	case LayerIndex_Compnent:
		m_packageSpec.m_CompMapManager.SaveMap(dlg.GetPathName());
		break;
	case LayerIndex_Pad:
		m_packageSpec.m_PadMapManager.SaveMap(dlg.GetPathName());
		break;
	default:
		break;
	}

}

void CBallLayoutEditor_Impl::OnBnClickedButtonLayerName() //Update Layer
{
	CString strAddLayerName;
	((CEdit *)GetDlgItem(IDC_EDIT_LAYER_NAME))->GetWindowText(strAddLayerName);

	long nSelectLayer = m_comboLayer.GetCurSel();

	if (nSelectLayer == LayerIndex_Whole) //Whole Device일 경우 반응하지 않음
		return;

	m_vecstrLayerName[nSelectLayer] = strAddLayerName;

	Update_Control(m_vecstrLayerName);

	m_comboLayer.SetCurSel(nSelectLayer);
}

void CBallLayoutEditor_Impl::OnBnClickedButtonRotateCompData()
{
	long nCurSelectLayer = m_comboLayer.GetCurSel();

	if (nCurSelectLayer == LayerIndex_Compnent)
	{
		m_packageSpec.m_CompMapManager.RotateComp();
		RefreshData_Comp();
		Calc_DrawingPosition_Comp();

		Update_Background(nCurSelectLayer);
		Update_Foreground(nCurSelectLayer);

		Invalidate(FALSE);
	}
	else
		return;
}

