#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../InformationModule/dPI_DataBase/Component.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CMarkup;

//HDR_6_________________________________ Header body
//
struct Gerber_Header
{
    CString strVersion;
    CString strUnit;
    long nObjectNumber;
    long nZoneID_Column;
    long nReflowZCompensation_Column;
    long nConrnerBending_column;
    long nShapeNumber;
    long nFileType;
    float fDominantRotation;
    float fPackageSizeX;
    float fPackageSizeY;
    long bAutoGroping;

    Gerber_Header()
    {
        Init();
    }

    void Init()
    {
        strVersion = _T("");
        strUnit = _T("");
        nObjectNumber = 0;
        nZoneID_Column = 0;
        nReflowZCompensation_Column = 0;
        nConrnerBending_column = 0;
        nShapeNumber = 0;
        nFileType = 0;
        fDominantRotation = 0.f;
        fPackageSizeX = 0.f;
        fPackageSizeY = 0.f;
        bAutoGroping = 0;
    }
};

struct Gerber_Shape
{
    CString strType;
    CString strBalltop;
    long nGerberType;
    float fWidth;
    float fHeigth;
    float fOffsetX;
    float fOffsetY;
    long nRounding;
    long nOrientation;

    Gerber_Shape()
    {
        Init();
    }

    void Init()
    {
        strType = _T("");
        strBalltop = _T("");
        nGerberType = 0;
        fWidth = 0.f;
        fHeigth = 0.f;
        fOffsetX = 0.f;
        fOffsetY = 0.f;
        nRounding = 0;
        nOrientation = 0;
    }
};

struct Gerber_Object
{
    long nID;
    long nShapeNumber;
    long nPadType;
    long nLandGroupNo;
    float fPosX;
    float fPosY;
    float fRotation;
    float fTickness;
    bool bSelected;
    bool bVisible;
    long nCompGrouping;
    long nCompChipType;
    long nCompPassiveType;
    float fCompWidth;
    float fCompLenght;
    Ipvm::Rect32s rtROI; // LayoutEditor에서의 위치
    FPI_RECT fsrtSpecROI; // SpecROI
    CRect ballPosition; // Ball Position

    Gerber_Object()
    {
        Init();
    }

    void Init()
    {
        nID = 0;
        nShapeNumber = 0;
        nPadType = 2;
        nLandGroupNo = 1;
        fPosX = 0.f;
        fPosY = 0.f;
        fRotation = 0.f;
        fTickness = 0.3f;
        bSelected = false;
        bVisible = true;
        nCompGrouping = 0;
        nCompChipType = -1;
        nCompPassiveType = -1;
        fCompWidth = 0.;
        fCompLenght = 0.;
        rtROI = Ipvm::Rect32s{};
        fsrtSpecROI = FPI_RECT{};
        ballPosition = CRect{};
    }
};

struct Gerber_XML_PAS
{
    CString strName;
    CString strFoot;
};

struct Gerber_XML_FPS
{
    CString strName;
    long nShapeNum;
};

struct Land_Information
{
    long GroupID;
    float fLandHeight;
    float fLandWidth;
    float fLandLenght;
    long nLandAngle;
};

enum enPinIndexPos
{
    PinIndexPos_LT,
    PinIndexPos_RT,
    PinIndexPos_RB,
    PinIndexPos_LB,
};

class CPackageSpec;
class ImageLotView;
struct FPI_RECT;

class CMapDataConverterDlg : public CDialog
{
    DECLARE_DYNAMIC(CMapDataConverterDlg)

public:
    CMapDataConverterDlg(CWnd* pParent /*= nullptr*/, CPackageSpec* pSpec,
        std::vector<std::pair<long, CString>>& convMapData, BOOL modifyMode); // 표준 생성자입니다.
    virtual ~CMapDataConverterDlg();
    virtual INT_PTR DoModal();
    virtual BOOL OnInitDialog();
    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DLG_MAPDATA_CONVERTER
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    enum class MapDataType : long
    {
        TYPE_PAD = 0,
        TYPE_LAND,
        TYPE_BALL,
        TYPE_COMPONENT,
        TYPE_ALL,
    };
    MapDataType m_MapDataType;
    MapDataType m_PreviewDataType;

    std::vector<std::pair<long, CString>>& convMapData;

    CPackageSpec& m_packageSpec;
    CXTPPropertyGrid* m_propertyGrid;
    CXTPPropertyGrid* m_pGridMapDataType;

    BOOL m_bReEditMode;
    BOOL m_bVerifyMode;

    CStatic m_stcViewer;
    CBitmap m_bitmapGrid;
    CBitmap m_bitmapView;
    CRect m_rtView;
    CRect m_rtDevice;
    float m_fUnit;
    float m_fZoom;
    float m_fViewAspectRatio;
    float m_fDeviceAspectRatio;

    CPoint m_ptMenu; // Menu
    CPoint m_ptPinIndex[3]; // Pin Index
    CPoint m_ptSelection[2]; // Selected DC

    // Selected ComboBox
    long m_nSelectedLayer;
    long m_nSelectedGroup;

    Gerber_Header m_stHeaderInfo;
    std::vector<Gerber_Shape> m_vecstShapeInfo;
    std::vector<Gerber_Object> m_vecstObjectInfo;
    std::vector<Gerber_XML_PAS> m_vecst_XML_PAS;
    std::vector<Gerber_XML_FPS> m_vecst_XML_FPS;

    std::vector<Package::Component> m_vecCompInfo; // Created Components ( Grouping )
    std::vector<Package::Component> m_vecSelectedCompInfo; // Selected Components

    CString m_strImportFilePathName;
    int m_nAutoGroup;
    float m_fZoomingValue;
    long m_nGroupIdx;

    BOOL m_bLButtonDown;
    BOOL m_bInputGridMode;

    double m_fLandHeight;
    double m_fBallHeight;

public:
    void SetPropertyGrid();
    void SetPropertyGridMapData();
    void UpdatePropertyGridMapData();

    void ImportXML();
    void Get_GerberHeaderInfo_XML(CMarkup& xml);
    void Get_GerberShapeInfo_XML(CMarkup& xml);
    void Get_GerberObjectInfo_XML(CMarkup& xml);

    bool ConvertToCSV();
    bool CheckMapDataTypes(bool& Cir, bool& Rc, bool& Ob, bool& Land);
    bool GetPosition(const float fPosX, const float fPosY, float& outPosX, float& outPosY);
    void ClearMapData();

    void Initialize_View();
    void Calc_DrawingPosition_Ball();
    void Calc_DrawingPosition_Comp(bool bPreviewMode = false);
    void Calc_DrawingPosition_Land();
    void Calc_DrawingPosition_Pad();
    void Calc_DrawingPosition_ComponentAutoGroup(long nPadIndex1, long nPadIndex2, float fRotation);
    void Calc_DrawingPosition_ComponentAutoGroup(long nPadIndex);

    void GetPinIndexPoint(FPI_RECT i_frtPinIndex, CPoint* o_pPinIndexPoint);
    void CreateComponent_Auto();
    void CreateComponent_Manual();
    void CreateComponent(long nMountPadIdx1, long nMountPadIdx2);
    long GetGroupIndex();
    long GetPadGroupingIndex(long nGroupIDX);

    void Update_Background(long i_nCurLayerIndex);
    void Update_Foreground(long i_nCurLayerIndex, long i_nPartIndex = -1);
    void SetDrawing_BallForegroundData(long i_nCurLayerIndex, CDC* i_pDC, long i_nPartIndex = -1);
    void SetDrawing_LandForegroundData(long i_nCurLayerIndex, CDC* i_pDC, long i_nPartIndex = -1);
    void SetDrawing_CompForegroundData(
        long i_nCurLayerIndex, CDC* i_pDC, long i_nPartIndex = -1, bool bPreviewMode = false);
    void SetDrawing_PadForegroundData(long i_nCurLayerIndex, CDC* i_pDC, long i_nPartIndex = -1);
    void SetDrawing_WholeForegroundData(CDC* i_pDC, bool bPreviewMode = false);

    void SetMenu();
    void Set_SelectedData(CRect i_rtSelect, bool bOneClick = false);
    void ClearSelection(std::vector<long> i_idx);
    void Select_AllData();
    void Unselect_AllData();

    long GetComponentIndexFromMountMap(long nPadID);
    BOOL CompTypeConvert();
    CString CompTypeFloattoString(float fValue);
    float round(float var);

    // Mount Pad Grouping (make component)
    BOOL CheckPadGrouping(long& nPadMountIdx1, long& nPadMountIdx2);
    void RemoveGroup(long nPadMountIdx1, long nPadMountIdx2);

    void DeleteSelectedMapData();
    long GetDataIndexFromID(long dataID);
    long GetDataIndexFromID(CString dataID);
    void ClearAllComponents();

    // Data 다시 수정
    void ReEdit_Get_GerberHeaderInfo_XML(CMarkup& xml);
    void ReEdit_Get_GerberShapeInfo_XML(CMarkup& xml);
    void ReEdit_Get_GerberObjectInfo_XML(CMarkup& xml);
    void ReEdit_SaveXMLModifyData();
    void ReEdit_LoadXMLModifyData();

    // Chip DB Information
    void CompDBVerify(Package::Component selectedComp);
    BOOL LoadCompTypeInCompDB(std::vector<std::vector<CString>>& o_vec2strCompType);

public:
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnComponentmenuEditdimensions();
    afx_msg void OnGroupmenuMakegroup();
    afx_msg void OnComponentmenuRemovegroup();
    afx_msg void OnComponentmenuSelectall();
    afx_msg void OnComponentmenuUnselectall();
    afx_msg void OnPadmenuChangetype();
    afx_msg void OnLandmenuChangegroupid();
    afx_msg void OnPadmenuSelectall();
    afx_msg void OnPadmenuUnselectall();
    afx_msg void OnLandmenuSelectall();
    afx_msg void OnLandmenuUnselectall();
    afx_msg void OnBallmenuBallspec();
    afx_msg void OnBallmenuSelectall();
    afx_msg void OnBallmenuUnselectall();
    afx_msg void OnPadmenuDelete();
    afx_msg void OnComponentmenuDelete();
    afx_msg void OnLandmenuDelete();
    afx_msg void OnBallmenuDelte();
    afx_msg void OnOtherDelete();
    afx_msg void OnComponentmenuAdd();
    afx_msg void OnPrevmenuEditdimensions();
    afx_msg void OnPrevmenuSelectall();
    afx_msg void OnPrevmenuUnselectall();
    afx_msg void OnPrevmenuAdd();
    afx_msg void OnPrevmenuDelete();
};
