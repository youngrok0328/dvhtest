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
	LayerIndex_Main, //딱히 생각이 안난다.
	LayerIndex_Compnent,
	LayerIndex_Pad,
	LayerIndex_Whole,
};

enum enPinIndexPos
{
	PinIndexPos_LT,
	PinIndexPos_RT,
	PinIndexPos_RB,
	PinIndexPos_LB,
};

class CBallLayoutEditor_Impl : public CDialog
{
// Construction
public:
	CBallLayoutEditor_Impl(SystemConfig &systemConfig, CPackageSpec &packageSpec, long nCurrentAccessMode, const CRect &rtPosition, CWnd* pParent = NULL);   // standard constructor
	~CBallLayoutEditor_Impl();
// Dialog Data
	//{{AFX_DATA(CBallLayoutEditor_Impl)
	enum { IDD = IDD_DIALOG_BALL };
	CComboBox	m_comboLayer;
	CStatic	m_stcViewer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBallLayoutEditor_Impl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Calc_DrawingPosition_Ball();	//Ball Data
	void Calc_DrawingPosition_Comp();	//Compnent Data
	void Calc_DrawingPosition_Pad();	//Pad Data
	void Calc_DrawingPosition_Land();	//Land Data

	float GetIndexSize();

	void Initialize_View();

	void Update_Control(std::vector<CString> i_vecstrLayerName);
		void Update_BtnControl();
	void Update_Background(long i_nCurLayerIndex);
	void Update_Foreground(long i_nCurLayerIndex);
		void SetDrawing_MainForegroundData(long i_nCurLayerIndex ,CDC *i_pDC);
		void SetDrawing_CompForegroundData(long i_nCurLayerIndex, CDC *i_pDC);
		void SetDrawing_PadForegroundData(long i_nCurLayerIndex, CDC *i_pDC);
			void GetPinIndexPoint(FPI_RECT i_frtPinIndex, CPoint *o_pPinIndexPoint);
		void SetDrawing_WholeForegroundData(long i_nCurLayerIndex, CDC *i_pDC);

	void Update_PropertyGrid(long nLayer);
		long GetMainLayerObjectNum();

	SystemConfig &m_systemConfig;
	CPackageSpec &m_packageSpec;
	const CRect m_rtInitPosition;

	void Refresh(SystemConfig &systemParam, BOOL bDeadBug);
	void RefreshData_Comp(); //나중에 병합할 녀석임
	void RefreshData_Pad();
	void UpdateMapPosition();			// 좌표 계산 후 MapData에 Update
	void UpdateMapPosition_Pad();

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

	std::vector<DrawingBallInfo> m_vecBallInfo;
	std::vector<CComponentMap> m_vecCompInfo;
	std::vector<CPadMap> m_vecPadInfo;

	CBitmap m_bitmapGrid;
	CBitmap m_bitmapView;

	CPoint m_ptPinIndex[4];

	float m_fViewAspectRatio;
	float m_fDeviceAspectRatio;

	CRect m_rtView;
	CRect m_rtDevice;
	float m_fZoom;

	std::vector<CString> m_vecstrLayerName;

	// Generated message map functions
	//{{AFX_MSG(CBallLayoutEditor_Impl)
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		void Ignore_Ball(CRect i_rtSelect, long i_nEditMode);
		void Ignore_Land(CRect i_rtSelect, long i_nEditMode);
		void Ignore_Comp(CRect i_rtSelect, long i_nEditMode);
		void Ignore_Pad(CRect i_rtSelect, long i_nEditMode);
	afx_msg void OnSelchangeComboLayer();
	afx_msg void OnButtonEditBodySize();
	afx_msg void OnBnClickedButtonImportMapData();
	afx_msg void OnBnClickedButtonExportMapData();
	afx_msg void OnClose();
	afx_msg void OnBnClickedCheckBallMapFlipX();
	afx_msg void OnBnClickedCheckBallMapFlipY();
	afx_msg void OnCbnSelchangeComboBallMapRotation();
	afx_msg void OnBnClickedButtonLayerName();
	afx_msg void OnBnClickedButtonRotateCompData();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CButton m_checkBallMapFlipX;
	CButton m_checkBallMapFlipY;
	CComboBox m_comboBallMapRotation;
};