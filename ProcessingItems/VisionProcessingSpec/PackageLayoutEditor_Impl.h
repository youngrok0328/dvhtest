#pragma once

#include "Resource.h"
#include <vector>
#include "Base/Rect.h"
#include "afxwin.h"
#include "../../PI_Modules/dPI_DataBase/PackageSpec.h"

class CAutoSetting;
class CImageDisplay;
class CDlgVisionProcessingSpec;
class CPackageSpec;
class SystemConfig;

enum enCurLayerIndex
{
	LayerIndex_Pad,
	LayerIndex_Ball,
	LayerIndex_Land,
	LayerIndex_Compnent,
	LayerIndex_Whole,
};

enum enPinIndexPos
{
	PinIndexPos_LT,
	PinIndexPos_RT,
	PinIndexPos_RB,
	PinIndexPos_LB,
};

class CPackageLayoutEditor_Impl : public CDialog
{
// Construction
public:
	CPackageLayoutEditor_Impl(SystemConfig &systemConfig, CPackageSpec &packageSpec, long nCurrentAccessMode, const CRect &rtPosition, CWnd* pParent = NULL);   // standard constructor
	~CPackageLayoutEditor_Impl();
// Dialog Data
	//{{AFX_DATA(CPackageLayoutEditor_Impl)
	enum { IDD = IDD_DIALOG_PACKAGELAYOUT };
	CStatic		m_stcViewer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPackageLayoutEditor_Impl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Calc_DrawingPosition_Ball();	//Ball Data
	void Calc_DrawingPosition_Comp();	//Compnent Data
	void Calc_DrawingPosition_Pad();	//Pad Data
	void Calc_DrawingPosition_Land();	//Land Data

	BOOL& getCurrentLayer_FlipX();
	BOOL& getCurrentLayer_FlipY();

	float GetIndexSize();

	void Initialize_View();

	void Update_Background(long i_nCurLayerIndex);
	void Update_Foreground(long i_nCurLayerIndex);
		void SetDrawing_BallForegroundData(long i_nCurLayerIndex ,CDC *i_pDC);
		void SetDrawing_LandForegroundData(long i_nCurLayerIndex, CDC *i_pDC);
		void SetDrawing_CompForegroundData(long i_nCurLayerIndex, CDC *i_pDC);
		void SetDrawing_PadForegroundData(long i_nCurLayerIndex, CDC *i_pDC);
			void GetPinIndexPoint(FPI_RECT i_frtPinIndex, CPoint *o_pPinIndexPoint);
		void SetDrawing_WholeForegroundData(CDC *i_pDC);

	void Update_PropertyGrid();
	void Update_PropertyGrid_LayersSummy();

	SystemConfig &m_systemConfig;
	CPackageSpec &m_packageSpec;
	const CRect m_rtInitPosition;

	void RefreshData_Ball(SystemConfig &systemParam);
	void RefreshData_Comp(); //나중에 병합할 녀석임
	void RefreshData_Pad();
	void RefreshData_Land();
	void UpdateMapPosition();			// 좌표 계산 후 MapData에 Update
	void UpdateMapPosition_Pad();
	void UpdateMapPosition_Land();

	void propertyChange_SelectLayer();
	void propertyChange_FlipX();
	void propertyChange_FlipY();
	void propertyChange_Rotation();
	void propertyChange_RotateComp();
	void propertyButton_BodySize();
	void propertyButton_ImportMapData();
	void propertyButton_ExportMapData();
	void propertyButton_ClearMapData();

	struct DrawingBallInfo
	{
		DrawingBallInfo(const long left, const long top, const long right, const long bottom, const bool ignored, const long index)
			: m_ballPosition(left, top, right, bottom)
			, m_ignored(ignored)
			, m_index(index)
		{

		}

		CRect m_ballPosition;
		bool m_ignored;
		long m_index;
	};

	CXTPPropertyGrid *m_propertyGrid;
	long m_selectLayer;
	long m_selectionMethod;

	std::vector<DrawingBallInfo> m_vecBallInfo;
	std::vector<CComponentMap> m_vecCompInfo;
	std::vector<CPadMap> m_vecPadInfo;
	std::vector<CLandMap> m_vecLandInfo;

	CBitmap m_bitmapGrid;
	CBitmap m_bitmapView;

	CPoint m_ptPinIndex[3];

	float m_fViewAspectRatio;
	float m_fDeviceAspectRatio;

	CRect m_rtView;
	CRect m_rtDevice;
	float m_fZoom;

	std::vector<CString> m_vecstrLayerName;

	// Generated message map functions
	//{{AFX_MSG(CPackageLayoutEditor_Impl)
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		void Ignore_Ball(CRect i_rtSelect, long i_nEditMode);
		void Ignore_Land(CRect i_rtSelect, long i_nEditMode);
		void Ignore_Comp(CRect i_rtSelect, long i_nEditMode);
		void Ignore_Pad(CRect i_rtSelect, long i_nEditMode);
	void CompDBVerify();
			BOOL LoadCompTypeInCompDB(std::vector<std::vector<CString>> &o_vec2strCompType);
	afx_msg void OnClose();
	BOOL RefreshData(long i_nLayerIndex);
	afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()
};