#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum class enDeviceType : long
{
    PACKAGE_UNKNOWN = -1,
    PACKAGE_BALL,
    PACKAGE_GULLWING,
    PACKAGE_LEADLESS,
    PACKAGE_CHIP,
    PACKAGE_LAND,
};

enum class enPackageType : long
{
    UNKNOWN = -1,

    // Gullwing Type Device
    TSOP_1 = 0,
    TSOP_2,
    QFP, // WSOP, QFN,

    // Grid Array Type Device
    BGA = 100, // wBGA, CSP, BCC, etCSP, etLGA, ULGA, WFP,//kircheis_WLP

    // Leadless Type Device
    QFN = 200,
    CARD,

    CIS = 300,

    FC_BGA = 400,

    LGA = 500,

    // Golden device
    ACCU_TSOP_2 = 1000,
    ACCU_TSOP_1,
    ACCU_QFP,
    ACCU_BGA,

    // NULL
    PACKAGE_TYPE_NULL = 10000,
};

enum enRoundPackageOption //kircheis_RoundPKG
{
    Round_Normal = 0, //일반 Package
    Round_UpDown, // 상하면 Round
    Round_LeftRight, //좌우면 Round
};

enum FileType
{
    enum_FileType_XML,
    enum_FileType_DEF,
    enum_FileType_CSV,
};

enum eLayoutEditerType
{
    eBallLayoutEditor = 0,
    eLeadLayoutEditor,
    ePadLayoutEditor,
    eChipLayoutEditor,
    eLayoutEditerTypeNum,
};

enum COMPONENT_TYPE
{
    _typeComponentStart = 0,
    _typeChip = _typeComponentStart,
    _typePassive,
    _typeArray,
    _typeHeatSync,
    _typePatch, //kircheis_POI
    _typeComponentEnd,
};

enum RotateIndex
{
    RotateStart = 0,
    Rotate_0 = RotateStart,
    Rotate_90,
    Rotate_180,
    Rotate_270,
    RotateEnd,
};

enum eunmLandShapeType //kircheis_LandShape
{
    Shape_Base_Rect = 0,
    Shape_Base_Circle,
    Shape_Base_Multiple,
};

enum CompMapDataInCSVType
{
    MapDataTypeStart = 0,
    MapDataType_CompID = MapDataTypeStart,
    MapDataType_CenterX,
    MapDataType_CenterY,
    MapDataType_CompCategory,
    MapDataType_CompType,
    MapDataType_CompWidth,
    MapDataType_CompLength,
    MapDataType_CompHeight,
    MapDataType_CompAngle,
    MapDataType_ElectLenght,
    MapDataType_ElectHeight,
    MapDataType_PassivePadGap,
    MapDataType_PassivePadWidth,
    MapDataType_PassivePadLenght,
    MapDataType_Visible,
    MapDataTypeEnd,
};

enum LandMapDataInCSVType
{
    LandDataTypeStart = 0,
    LandDataType_Name = LandDataTypeStart,
    LandDataType_CenterX,
    LandDataType_CenterY,
    LandDataType_Width,
    LandDataType_Length,
    LandDataType_Height,
    LandDataType_Angle,
    LandDataType_GroupID,
    LandDataType_Visible,
    LandDataTypeEnd,
};

enum PAD_TYPE : long
{
    _typePadStart = 0,
    _typeTriangle = _typePadStart,
    _typeCircle,
    _typeRectangle,
    _typePinIndex,
    _typePadEnd,
};

enum PadMapDataInCSVType
{
    PadDataTypeStart = 0,
    PadDataType_PadName = PadDataTypeStart,
    PadDataType_CenterX,
    PadDataType_CenterY,
    PadDataType_PadWidth,
    PadDataType_PadLength,
    PadDataType_PadHeight,
    PadDataType_PadAngle,
    PadDataType_PadType,
    PadDataType_Mount,
    PadDataType_Visible,
    PadDataTypeEnd,
};

enum CustomPolygonCategory : long
{
    CustomPolygonCategory_Unknown = -99999,
    CustomPolygonCategory_Start = 0,
    CustomPolygonCategory_Polygon = CustomPolygonCategory_Start,
    CustomPolygonCategory_End,
};

enum CustomPolygonLayerMapDataInCSVType
{
    CustomPolygonLayerDataTypeStart = 0,
    CustomPolygonLayerDataType_ID = CustomPolygonLayerDataTypeStart,
    CustomPolygonLayerDataType_Catrgory,
    CustomPolygonLayerDataType_PointNum,
    CustomPolygonLayerDataType_Height,
    CustomPolygonLayerDataType_Visible,
    CustomLayerDataTypeEnd,
};

enum fixColumn_CustomPolygonMap
{
    FX_ID,
    FX_CATEGORY,
    FX_POINTNUM,
    FX_HEIGHT,
    FX_VISIBLE,
    FX_END,
};

enum CustomFixedLayerMapDataInCSVType
{
    CustomFixedLayerDataTypeStart = 0,
    CustomLayerDataType_ID = CustomFixedLayerDataTypeStart,
    CustomLayerDataType_Catrgory,
    CustomLayerDataType_CenterX,
    CustomLayerDataType_CenterY,
    CustomLayerDataType_Width,
    CustomLayerDataType_Length,
    CustomLayerDataType_Height,
    CustomLayerDataType_Visible,
    CustomFixedLayerDataTypeEnd,
};

enum CustomFixedCategory : long
{
    CustomFixedCategory_Unknown = -99999,
    CustomFixedCategory_Start = 0,
    CustomFixedCategory_Circle = CustomFixedCategory_Start,
    CustomFixedCategory_Rectangle,
    CustomFixedCategory_NotchHole,
    CustomFixedCategory_End,
};

enum class TapeSprocketHoleExistType : int32_t
{
    e_both,
    e_leftOnly,
    e_rightOnly,
};
