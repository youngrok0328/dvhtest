#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/Component.h"
#include "../../InformationModule/dPI_DataBase/CustomPolygon.h"
#include "../../InformationModule/dPI_DataBase/DMSArrayChip.h"
#include "../../InformationModule/dPI_DataBase/DMSBall.h"
#include "../../InformationModule/dPI_DataBase/DMSChip.h"
#include "../../InformationModule/dPI_DataBase/DMSHeatsink.h"
#include "../../InformationModule/dPI_DataBase/DMSPassiveChip.h"
#include "../../InformationModule/dPI_DataBase/DMSPatch.h"
#include "../../InformationModule/dPI_DataBase/Land.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/Pad.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionProcessing;
class VisionProcessingSpec;
class VisionMapDataEditorUI;
struct ProcessingDlgInfo;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
enum enCurLayerIndex //mc_새로운 Layer를 추가할 시, Whole위에다가 추가할 것
{
    LayerIndex_Pad,
    LayerIndex_Ball,
    LayerIndex_Land,
    LayerIndex_Compnent,
    LayerIndex_CustomFixed,
    LayerIndex_CustomPolygon,
    LayerIndex_Whole,
};

static LPCTSTR g_szLayerIndexName[] = {
    _T("Pad Layer"),
    _T("Ball Layer"),
    _T("Land Layer"),
    _T("Component Layer"),
    _T("Custom Fixed Layer"),
    _T("Custom Polygon Layer"),
    _T("Whole Device"),
};

enum enPinIndexPos
{
    PinIndexPos_LT,
    PinIndexPos_RT,
    PinIndexPos_RB,
    PinIndexPos_LB,
};

enum enMapDataType
{
    MapDataType_Pad,
    MapDataType_Ball,
    MapDataType_Land,
    MapDataType_Comp,
    MapDataType_Custom,
};

class CDlgVisionProcessingSpec : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionProcessingSpec)

public:
    CDlgVisionProcessingSpec(const ProcessingDlgInfo& procDlgInfo, VisionProcessingSpec* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionProcessingSpec();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG
    };

protected:
    void Calc_DrawingPosition_Ball(); //Ball Data
    void Calc_DrawingPosition_Comp(); //Compnent Data
    void Calc_DrawingPosition_Pad(); //Pad Data
    void Calc_DrawingPosition_Land(); //Land Data
    void Calc_DrawingPosition_CustomFixed(); //Custom Data
    void Calc_DrawingPosition_CustomPolygon(); //Custom Data

    BOOL& getCurrentLayer_FlipX();
    BOOL& getCurrentLayer_FlipY();

    void Initialize_View();

    void Update_Background(long i_nCurLayerIndex);
    void Update_Foreground(long i_nCurLayerIndex);
    void Update_Warning(long i_nCurLayerIndex);
    void SetDrawing_BallForegroundData(long i_nCurLayerIndex, CDC* i_pDC);
    void SetDrawing_LandForegroundData(long i_nCurLayerIndex, CDC* i_pDC);
    void SetDrawing_CompForegroundData(long i_nCurLayerIndex, CDC* i_pDC);
    void SetDrawing_PadForegroundData(long i_nCurLayerIndex, CDC* i_pDC);
    void SetDrawing_CustomFixedForegroundData(long i_nCurLayerIndex, CDC* i_pDC);
    void SetDrawing_CustomPolygonForegroundData(long i_nCurLayerIndex, CDC* i_pDC);
    void GetPinIndexPoint(FPI_RECT i_frtPinIndex, CPoint* o_pPinIndexPoint);
    void SetDrawing_WholeForegroundData(CDC* i_pDC);

    void Update_PropertyGrid();
    void Update_PropertyGrid_LayersSummy();

    CPackageSpec& m_packageSpec;

    void RefreshData_Pad();
    void RefreshData_Ball();
    void RefreshData_Land();
    void RefreshData_Comp(); //나중에 병합할 녀석임
    void RefreshData_CustomFixed();
    void RefreshData_CustomPolygon();
    void UpdateMapPosition(); // 좌표 계산 후 MapData에 Update
    void UpdateMapPosition_Pad();
    void UpdateMapPosition_Land();
    void UpdateMapPosition_CustomFixed();
    void UpdateMapPosition_CustomPolygon();

    void propertyChange_SelectLayer();
    void propertyChange_FlipX();
    void propertyChange_FlipY();
    void propertyChange_Rotation();
    void propertyChange_RotateComp();
    void propertyButton_BodySize();
    void propertyButton_ImportMapData();
    void propertyButton_ExportMapData();
    void propertyButton_ClearMapData();
    void propertyButton_CreateMapData();
    void propertyButton_MapDataEditor();

    //MapData Utlity Function
    //pari 인자 목록 : long : MapData Type 0 : Pad, 1: Ball, 2: Land, 3: Component //mc_나중에 완료되면 Enum으로 Type 정의하여 변경해 주세요
    //				   CString : CSV 파일경로
    void AutoImport_MapDataByXML(std::vector<std::pair<long, CString>>
            i_vecConvertMapData); //mc_ATNS 관련 XML File Convert 후 MapData의 Data Draw
    void OriginMapData_AllClear();

    struct DrawingBallInfo
    {
        DrawingBallInfo(
            const long left, const long top, const long right, const long bottom, const bool ignored, const long index)
            : m_ballPosition(left, top, right, bottom)
            , m_ignored(ignored)
            , m_index(index)
        {
        }

        CRect m_ballPosition;
        bool m_ignored;
        long m_index;
    };

    long m_selectLayer;
    long m_selectionMethod;

    std::vector<DrawingBallInfo> m_vecBallInfo;
    std::vector<Package::Component> m_vecCompInfo;
    std::vector<Package::Pad> m_vecPadInfo;
    std::vector<Package::Land> m_vecLandInfo;
    std::vector<CustomFixed> m_vecCustomFixedInfo;
    std::vector<Package::CustomPolygon> m_vecCustomPolygonInfo;

    CBitmap m_bitmapGrid;
    CBitmap m_bitmapView;

    CPoint m_ptPinIndex[3];

    float m_fViewAspectRatio;
    float m_fDeviceAspectRatio;

    CRect m_rtView;
    CRect m_rtDevice;
    CRect m_rtGlassSubstrate;
    float m_fZoom;

    void Ignore_Ball(CRect i_rtSelect, long i_nEditMode);
    void Ignore_Land(CRect i_rtSelect, long i_nEditMode);
    void Ignore_Comp(CRect i_rtSelect, long i_nEditMode);
    void Ignore_Pad(CRect i_rtSelect, long i_nEditMode);
    void Ignore_CustomFixed(CRect i_rtSelect, long i_nEditMode);
    void Ignore_CustomPolygon(CRect i_rtSelect, long i_nEditMode);

    BOOL LoadCompTypeInCompDB(std::vector<std::vector<CString>>& o_vec2strCompType);
    BOOL RefreshData(long i_nLayerIndex);

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    void FireExchanged();

    DECLARE_MESSAGE_MAP()

public:
    VisionMapDataEditorUI* m_pVisionEditorDlg;
    const ProcessingDlgInfo& m_procDlgInfo;
    VisionProcessingSpec* m_pVisionInsp;
    CXTPPropertyGrid* m_propertyGrid;
    CStatic m_stcViewer;
    HINSTANCE m_hInstOld;

public:
    BOOL CompDBVerify();

    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnClose();
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnLandshapechange();
};