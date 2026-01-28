#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/iDataType.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/CircleEq32r.h>
#include <Ipvm/Base/LineEq32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum SelectRefAndTar
{
    enum_Select_Reference = 0,
    enum_Select_Target,
};

enum class DataTypeList
{
    Unknown,
    Line,
    Point,
    All_Point,
    Circle,
};

enum SInspectionType_Distance
{
    Insp_Type_Shape_Begin = 0,
    Insp_Type_Shape_Distance = Insp_Type_Shape_Begin,
    Insp_Type_Shape_Circle,
    Insp_Type_Shape_Angle,
    Insp_Type_Shape_End,
};

static LPCTSTR g_szSInspectionType_DistanceName[] = {
    _T("Distance"),
    _T("Circle"),
    _T("Angle"),
};

struct SAlignInfoDataList
{
    SAlignInfoDataList()
    {
        Init();
    }

    // 데이터 타입
    DataTypeList nDataType;

    // 표시할 이름
    CString strName;

    // 화면에 표시될 ROI
    Ipvm::Rect32s rtROI;

    // Grid Parameter 개수
    long nDataCount;

    // 계산에 사용될 Edge Data
    std::vector<Ipvm::Point32r2> vecfptData;

    Ipvm::Point32r2 fptCenterPoint;

    // 계산에 사용될 Line Data
    Ipvm::LineEq32r LineData_F;
    Ipvm::LineEq32r LineData_S;

    // 계산에 사용될 Circle Data
    Ipvm::CircleEq32r CircleData;

    void Init()
    {
        nDataType = DataTypeList::Unknown;
        strName = _T("");
        rtROI.SetRectEmpty();
        nDataCount = 0;
        vecfptData.clear();

        LineData_F = Ipvm::LineEq32r(0.f, 0.f, 0.f);
        LineData_S = Ipvm::LineEq32r(0.f, 0.f, 0.f);
    }

    SAlignInfoDataList& operator=(const SAlignInfoDataList& Src)
    {
        nDataType = Src.nDataType;
        strName = Src.strName;
        rtROI = Src.rtROI;
        nDataCount = Src.nDataCount;

        vecfptData = Src.vecfptData;

        LineData_F = Src.LineData_F;
        LineData_S = Src.LineData_S;

        return *this;
    }
};

enum GeometryDirection
{
    enum_Direction_Begin = 0,
    enum_Direction_LeftTop = enum_Direction_Begin,
    enum_Direction_LeftBottom,
    enum_Direction_RightTop,
    enum_Direction_RightBottom,
    enum_Direction_End,

    enum_Direction_LowTop_Begin = enum_Direction_End,
    enum_Direction_LowTop_Left = enum_Direction_LowTop_Begin,
    enum_Direction_LowTop_Right,
    enum_Direction_LowTop_End,
};

enum AlignInfo_List
{
    Insp_Type_Begin = 0,
    Insp_Type_Body_Line = Insp_Type_Begin,
    Insp_Type_Body_Center_Info,
    Insp_Type_Other_Line,
    Insp_Type_Other_Center_Info,
    Insp_Type_Round_LT_Info,
    Insp_Type_Round_RT_Info,
    Insp_Type_Round_LB_Info,
    Insp_Type_Round_RB_Info,
    Insp_Type_User_Edge_Detect,
    Insp_Type_User_Line,
    Insp_Type_User_Round,
    Insp_Type_User_Ellips,
    Insp_Type_End = Insp_Type_User_Ellips,
};

static LPCTSTR g_szAlignInfo_List_Name[] = {
    _T("Body Line Info"),
    _T("Body Center Info"),
    _T("Rect Object Line Info"),
    _T("Rect Object Center Info"),
    _T("Round LT Info"),
    _T("Round RT Info"),
    _T("Round LB Info"),
    _T("Round RB Info"),
    _T("Find Edge Point"),
    _T("Find Line Data"),
    _T("Find Round"),
    _T("Find Circle"),
};

enum SBodyLineParameter
{
    BL_Param_Begin = 0,
    BL_Param_Left = BL_Param_Begin,
    BL_Param_Top,
    BL_Param_Right,
    BL_Param_Bottom,
    BL_Param_Lowtop,
    BL_Param_End,
};

static LPCTSTR g_szBodyLineParameter_Name[] = {
    _T("Left"),
    _T("Top"),
    _T("Right"),
    _T("Bottom"),
    _T("Lowtop"),
};

enum SEdgeDetectParameter
{
    ED_Param_Begin = 0,
    ED_Param_Search_Dir = ED_Param_Begin,
    ED_Param_Edge_Dir,
    ED_Param_Edge_Type,
    ED_Param_Edge_Thresh,
    ED_Param_Edge_Angle,
    ED_Param_Edge_Detect_Area,
    ED_Param_End,
};

static LPCTSTR g_szEdgeDetectParameterName[] = {
    _T("Search Dir."),
    _T("ED Dir."),
    _T("ED Type"),
    _T("ED Thresh."),
    _T("ROI Angle"),
    _T("ED Detect Area"),
};

static LPCTSTR g_szEdgeDetectParameterName_Circle[] = {
    _T("Search Dir."),
    _T("ED Dir."),
    _T("ED Type"),
    _T("ED Thresh."),
    _T("ROI Angle"),
    _T("Use Data"),
};

enum InspectionParameter
{
    Insp_Param_Begin = 0,
    Insp_Param_Type = Insp_Param_Begin,
    Insp_Param_Dist_Result,
    Insp_Param_Circle_Result,
    Insp_Param_Spec,
    Insp_Param_End,
};

static LPCTSTR g_szInspectionParameterName[] = {
    _T("Insp Type"),
    _T("Distance Result"),
    _T("Circle Result"),
    _T("Insp Spec (um)"),
};

enum InspectionDistanceResult
{
    Insp_Param_Dist_Begin = 0,
    Insp_Param_Dist_XY = Insp_Param_Dist_Begin,
    Insp_Param_Dist_X,
    Insp_Param_Dist_Y,
    Insp_Param_Dist_End,
};

enum InspectionCircleResult
{
    Insp_Param_Circle_Begin = 0,
    Insp_Param_Circle_Radius = Insp_Param_Circle_Begin,
    Insp_Param_Circle_Diameter,
    Insp_Param_Circle_End,
};

enum InspectionType
{
    INSP_PARAM_TYPE_BEGIN = 0,
    INSP_PARAM_TYPE_DISTANCE = INSP_PARAM_TYPE_BEGIN,
    INSP_PARAM_TYPE_CIRCLE,
    INSP_PARAM_TYPE_ANGLE,
    INSP_PARAM_TYPE_END,
};