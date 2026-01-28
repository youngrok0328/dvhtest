#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#define PI 3.1415926535f
#define PI2 PI + PI
#define BLOB_INFO_SMALL_SIZE 500
#define BLOB_INFO_MEDIUM_SIZE 2000
#define BLOB_INFO_LARGE_SIZE 5000
#define BLOB_INFO_MAX_SIZE 20000

#define HALF_BYTE 128 // 2D Grab Fail 구분을 위한 Default 값

#define SEND_IMAGE_RESCALE_FACTOR (long)8

#define CAST_FLOAT static_cast<float>
#define CAST_DOUBLE static_cast<double>
#define CAST_INT static_cast<int>
#define CAST_UINT static_cast<unsigned int>
#define CAST_INT32T static_cast<int32_t>
#define CAST_UINT32T static_cast<uint32_t>
#define CAST_INT64T static_cast<int64_t>
#define CAST_UINT64T static_cast<uint64_t>
#define CAST_INT16T static_cast<int16_t>
#define CAST_UINT16T static_cast<uint16_t>
#define CAST_INT8T static_cast<int8_t>
#define CAST_UINT8T static_cast<uint8_t>
#define CAST_LONG static_cast<long>
#define CAST_ULONG static_cast<unsigned long>
#define CAST_LONG64 static_cast<long long>
#define CAST_ULONG64 static_cast<unsigned long long>
#define CAST_DWORD static_cast<DWORD>
#define CAST_BYTE static_cast<BYTE>
#define CAST_SHORT static_cast<short>

enum enum_ACCESS_MODE
{
    _OPERATOR,
    _ENGINEER,
    _INTEKPLUS,
};

enum en3DSurfaceUsedImage
{
    SURFACE_3D_USE_ZMAP,
    SURFACE_3D_USE_VMAP,
};

enum enVisionType
{
    VISIONTYPE_UNKNOWN = -1,
    VISIONTYPE_START = 0,
    VISIONTYPE_2D_INSP = VISIONTYPE_START,
    VISIONTYPE_3D_INSP,
    VISIONTYPE_NGRV_INSP,
    VISIONTYPE_SIDE_INSP, //kircheis_SideInsp
    VISIONTYPE_SWIR_INSP, //kircheis_SWIR
    VISIONTYPE_TR,
    VISIONTYPE_END,
};

static LPCTSTR g_szVisionTypeText[] = {
    _T("2D Vision"),
    _T("3D Vision"),
    _T("NGRV Vision"),
    _T("Side Vision"),
    _T("SWIR Vision"),
    _T("TR Vision"),
};

enum en2DVisionCameraType
{
    VISIONCAMERATYPE_START = 0,
    CAMERATYPE_LAON_PEOPLE_CXP25M = VISIONCAMERATYPE_START,
    CAMERATYPE_DALSA_GENIE_NANO_CXP_67M,
    VISIONCAMERATYPE_END,
};

enum en3DVisionCameraType
{
    VISION_3D_CAMERATYPE_START = 0,
    CAMERATYPE_VIEWWORKS_12MX = VISION_3D_CAMERATYPE_START,
    CAMERATYPE_MIKROTRON_EOSENS_21CXP2,
    VISION_3D_CAMERATYPE_END,
};

enum en3DVisionGrabMode
{
    VISION_3D_GRABMODE_START = 0,
    VISION_3D_GRABMODE_SPECULAR = VISION_3D_GRABMODE_START,
    VISION_3D_GRABMODE_DEFUSED,
    VISION_3D_GRABMODE_INTEGRATED,
    VISION_3D_GRABMODE_END,
};

enum enNgrvVisionOpticsType
{
    NGRV_VISION_OPTICS_TYPE_UNKNOWN = -1,
    NGRV_VISION_OPTICS_TYPE_START = 0,
    NGRV_VISION_OPTICS_TYPE_COLOR = NGRV_VISION_OPTICS_TYPE_START,
    NGRV_VISION_OPTICS_TYPE_SWIR,
    NGRV_VISION_OPTICS_TYPE_END,
};

enum enSideVisionNumber //kircheis_SideInsp
{
    SIDE_VISIONNUMBER_NOSIDEVISION = -1,
    SIDE_VISIONNUMBER_START = 0,
    SIDE_VISIONNUMBER_1 = SIDE_VISIONNUMBER_START,
    SIDE_VISIONNUMBER_2,
    SIDE_VISIONNUMBER_END,
};

enum enFrameGrabberType //kircheis_SWIR
{
    FG_TYPE_START = 0,
    FG_TYPE_IGRAB_G2_SERIES = FG_TYPE_START,
    FG_TYPE_IGRAB_XE,
    FG_TYPE_IGRAB_XQ,
    FG_TYPE_END,
};

//kircheis_SideInsp //System 속성이 아닌 Grab 및 연산 과정의 Flag로 사용//이걸 관리하는 변수는 VisionUnit 이하에 필요해 보인다.
//mc_해당 enum이 수정되면 VisionHostBaseDef.h의 enCommonSideVisionModule에도 추가가 되어야함 Handler랑 같이 사용.
enum enSideVisionModule
{
    SIDE_VISIONMODULE_NOSIDEVISION = -1,
    SIDE_VISIONMODULE_START = 0,
    SIDE_VISIONMODULE_FRONT = SIDE_VISIONMODULE_START,
    SIDE_VISIONMODULE_REAR,
    SIDE_VISIONMODULE_END,
};

enum enTRVisionmodule
{
    TR_VISIONMODULE_START = 0,
    TR_VISIONMODULE_INPOCKET = TR_VISIONMODULE_START,
    TR_VISIONMODULE_OTI,
    TR_VISIONMODULE_END,
};

enum enTRExecuteType
{
    TR_EXECUTE_ALL = -1,
    TR_EXECUTE_START = 0,
    TR_EXECUTE_IN_POCKET = TR_EXECUTE_START,
    TR_EXECUTE_OTI,
    TR_EXECUTE_END,
};

enum enHandlerType
{
    HANDLER_TYPE_380BRIDGE,
    HANDLER_TYPE_500I,
    HANDLER_TYPE_500A,
};

enum enStitchGrabDir
{
    _STITCH_GRAB_NORMAL,
    _STITCH_GRAB_REVERSE,
};

enum enMultiGrabPos
{
    _MULTI_GRAB_POS_NORMAL,
    _MULTI_GRAB_POS_HIGH,
};

enum enDualGrabImageType
{
    DUAL_IMAGE_EMPTY = -1,
    DUAL_IMAGE_START = 0,
    DUAL_IMAGE_TOTAL_REFLECTION = DUAL_IMAGE_START,
    DUAL_IMAGE_DIFFUSE_REFLECTION,
    DUAL_IMAGE_COMPLEX_REFLECTION,
    DUAL_IMAGE_END,
};

enum en3DGrabMode
{
    TOTAL_REFLECTION = 0,
    DIFFUSE_REFLECTION,
    COMPLEX_REFLECTION,
};

enum enNGRV_FrameInfo
{
    NGRV_FrameID_Reverse = 7,
    NGRV_FrameID_UV,
    NGRV_FrameID_IR,
};

enum enDirection
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    HOR_CENTER_LINE,
    VER_CENTER_LINE,
    HORIZONTAL,
    VERTICAL,
    LOWTOP
};

enum EdgeDetectMode
{
    EdgeDetectMode_Begin_ = 0,
    EdgeDetectMode_BestEdge = EdgeDetectMode_Begin_,
    EdgeDetectMode_FirstEdge,
    EdgeDetectMode_AutoDetect,
    EdgeDetectMode_End_,
};

enum NGRVImageSaveMode
{
    NGRV_FILE_FORMAT_BMP = 0,
    NGRV_FILE_FORMAT_JPG,
    NGRV_FILE_FORMAT_PNG,
};

#define CATEGORY_VISION_SETTING _T("SETTING")

#define PI_ED_DIR_BOTH 0 // 양방향 Edge
#define PI_ED_DIR_RISING 1 // 상승 Edge
#define PI_ED_DIR_FALLING 2 // 하강 Edge

#define PI_ED_DIR_OUTER 0 // 안에서 밖으로
#define PI_ED_DIR_INNER 1 // 밖에서 안으로

#define PI_ED_FIRST_EDGE 1 //kircheis_InspLid
#define PI_ED_MAX_EDGE 0

#ifndef ENUM_MARKID
#define ENUM_MARKID

enum MarkID
{
    MARK_ID_1,
    MARK_ID_2,
    MARK_ID_MAX,
};
#endif

enum PadDirection
{
    enum_Left = 0,
    enum_Top,
    enum_Right,
    enum_Bottom,
    enum_LowTop,
};

enum MATCH_2DID_TYPE
{ //kircheis_MED5_13
    TYPE_2DID_START = 0,
    TYPE_2DID_MAJOR = TYPE_2DID_START,
    TYPE_2DID_SUB_0,
    TYPE_2DID_SUB_1,
    TYPE_2DID_BABY_0,
    TYPE_2DID_BABY_1,
    TYPE_2DID_END,
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////  Vision Inspection Item Define Start ///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//{{ //kircheis_20171031 //개별 검사 항목 등록 구간...
// 검사 항목 ID를 enum으로 동록
// 검사 항목명을 String으로 등록
// 앞으로는 절대 검사 항목명에 '_'를 붙이지 말것

enum enumMarkInspection
{
    MARK_INSPECTION_START = 0,
    MARK_INSPECTION_MARK_COUNT = MARK_INSPECTION_START,
    MARK_INSPECTION_MARK_POS,
    MARK_INSPECTION_MARK_ANGLE,
    MARK_INSPECTION_MATCH_RATE,
    MARK_INSPECTION_CONTRAST,
    MARK_INSPECTION_OVER_PRINT,
    MARK_INSPECTION_UNDER_PRINT,
    MARK_INSPECTION_BLOB_SIZE,
    MARK_INSPECTION_END,
};

static LPCTSTR g_szMarkInspectionName[] = {
    _T("Mark Count"),
    _T("Mark Pos"),
    _T("Mark Angle"),
    _T("Match Rate"),
    _T("Contrast"),
    _T("Over Print"),
    _T("Under Print"),
    _T("Blob Size"),
};

enum enumPackageSizeInspection
{
    PACKAGE_SIZE_INSPECTION_BODYSIZE_X,
    PACKAGE_SIZE_INSPECTION_BODYSIZE_Y,
    PACKAGE_SIZE_INSPECTION_PARALLELISM,
    PACKAGE_SIZE_INSPECTION_ORTHOGONALITY,
};

static LPCTSTR g_szPackageSizeInspectionName[] = {
    _T("BodySizeX"),
    _T("BodySizeY"),
    _T("BodyParallelism"),
    _T("BodyOrth"),
};

enum enumBallInspection
{
    BALL_INSPECTION_START = 0,
    BALL_INSPECTION_BALL_MISSING = BALL_INSPECTION_START,
    BALL_INSPECTION_BALL_OFFSET_X,
    BALL_INSPECTION_BALL_OFFSET_Y,
    BALL_INSPECTION_BALL_OFFSET_R,
    BALL_INSPECTION_BALL_GRID_OFFSET_X,
    BALL_INSPECTION_BALL_GRID_OFFSET_Y,
    BALL_INSPECTION_BALL_WIDTH,
    BALL_INSPECTION_BALL_QUALITY,
    BALL_INSPECTION_BALL_CONTRAST,
    BALL_INSPECTION_BALL_ELLIPTICITY, //kircheis_TMI
    BALL_INSPECTION_END,
};

static LPCTSTR g_szBallInspectionName[] = {
    _T("Ball Missing"), _T("Ball Offset X"), _T("Ball Offset Y"), _T("Ball Offset R"), _T("Ball Grid dX"),
    _T("Ball Grid dY"), _T("Ball Width"), _T("Ball Quality"), _T("Ball Contrast"),
    _T("Ball Ellipticity"), //kircheis_TMI
};

enum enumLandInspection //kircheis_Land
{
    LAND_INSPECTION_START = 0,
    LAND_INSPECTION_LAND_ALIGN = LAND_INSPECTION_START, //kircheis_LandMissing
    LAND_INSPECTION_LAND_OFFSET_X,
    LAND_INSPECTION_LAND_OFFSET_Y,
    LAND_INSPECTION_LAND_OFFSET_R,
    LAND_INSPECTION_LAND_WIDTH,
    LAND_INSPECTION_LAND_LENGTH,
    LAND_INSPECTION_END,
};

static LPCTSTR g_szLandInspectionName[] = //kircheis_Land
    {
        _T("Land Align"), //kircheis_LandMissing
        _T("Land Offset X"),
        _T("Land Offset Y"),
        _T("Land Offset R"),
        _T("Land Width"),
        _T("Land Length"),
};

enum enumLand3DInspection //kircheis_Land 3D
{
    LAND3D_INSPECTION_START = 0,
    LAND3D_INSPECTION_LAND_COPL = LAND3D_INSPECTION_START,
    LAND3D_INSPECTION_LAND_UNIT_COPL,
    LAND3D_INSPECTION_LAND_HEIGHT,
    LAND3D_INSPECTION_LAND_WARPAGE,
    LAND3D_INSPECTION_LAND_UNIT_WARPAGE,
    //{{//kircheis_MED2LW
    LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AX,
    LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AV,
    LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AS,
    LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AO,
    //}}

    LAND3D_INSPECTION_END,
};

static LPCTSTR g_szLand3DInspectionName[] = //kircheis_Land 3D
    {
        _T("Land Copl"), _T("Land Unit Copl"), _T("Land Height"), _T("Land Warpage"), _T("Land UnitWarpage"),
        //{{ //kircheis_MED2LW
        _T("Land AX Warpage"), _T("Land AV Warpage"), _T("Land AS Warpage"), _T("Land AO Warpage"),
        //}}
};

//{{mc_2017.08.03 //kircheis_580 ChipLayerInsp
enum enumChipLayerInspection
{
    CHIP_INSPECTION_START = 0,
    CHIP_INSPECTION_CHIPALIGN = CHIP_INSPECTION_START,
    CHIP_INSPECTION_BODYSIZE_X,
    CHIP_INSPECTION_BODYSIZE_Y,
    CHIP_INSPECTION_BODYOFFSET_X,
    CHIP_INSPECTION_BODYOFFSET_Y,
    CHIP_INSPECTION_END,
};

static LPCTSTR g_szChipLayerInspectionName[] = {
    _T("Chip Align"),
    _T("Chip BodySize X"),
    _T("Chip BodySize Y"),
    _T("Chip BodyOffset X"),
    _T("Chip BodyOffset Y"),
};
//}}

enum enumBGA3DInspection
{
    _3DINSP_START = 0,
    _3DINSP_COPL = _3DINSP_START,
    _3DINSP_UNIT_COPL,
    _3DINSP_HEIGHT,
    _3DINSP_WARPAGE,
    _3DINSP_UNIT_WARPAGE,
    //{{//kircheis_MED2LW
    _3DINSP_UNIT_WARPAGE_AX,
    _3DINSP_UNIT_WARPAGE_AV,
    _3DINSP_UNIT_WARPAGE_AS,
    _3DINSP_UNIT_WARPAGE_AO,
    //}}
    _3DINSP_END,
};

static LPCTSTR g_szSurfaceInspectionName = _T("Surface");

static LPCTSTR g_szBGA3DInspectionName[]
    = //kircheis_201611XX 여기 검사 이름이 "3D XXXX"가 아닌 검사는 Job Open 시 지우게 한다. 이름 지을 때 신중히 할 것!
    {
        _T("3D Copl"), _T("3D Unit Copl"), _T("3D Height"), _T("3D Warpage"), _T("3D Unit Warpage"),
        //{{//kircheis_MED2LW
        _T("3D AX Warpage"), _T("3D AV Warpage"), _T("3D AS Warpage"), _T("3D AO Warpage"),
        //}}
};

enum //kircheis_BPQ
{
    INSPECT_BALL_PIXEL_QUALITY_ITEM_BEGIN_ = 0,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_BEGIN_FILLRATIO = INSPECT_BALL_PIXEL_QUALITY_ITEM_BEGIN_,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_1 = INSPECT_BALL_PIXEL_QUALITY_ITEM_BEGIN_,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_2,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_3,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_4,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_5,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_END_FILLRATIO,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_BEGIN_CONTRAST = INSPECT_BALL_PIXEL_QUALITY_ITEM_END_FILLRATIO,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_1 = INSPECT_BALL_PIXEL_QUALITY_ITEM_END_FILLRATIO,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_2,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_3,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_4,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_5,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_END_CONTRAST,
    INSPECT_BALL_PIXEL_QUALITY_ITEM_END_ = INSPECT_BALL_PIXEL_QUALITY_ITEM_END_CONTRAST,
};

//InspItemNames.cpp 참조.
static LPCTSTR gl_szStrInspectPixelQualityItem[] = //kircheis_BPQ
    {
        _T("Ball PxQ 1st R"),
        _T("Ball PxQ 2nd R"),
        _T("Ball PxQ 3rd R"),
        _T("Ball PxQ 4th R"),
        _T("Ball PxQ 5th R"),
        _T("Ball PxQ 1st C"),
        _T("Ball PxQ 2nd C"),
        _T("Ball PxQ 3rd C"),
        _T("Ball PxQ 4th C"),
        _T("Ball PxQ 5th C"),
};

enum enum2DMatrixInspection
{
    MATRIX_INSPECTION_START = 0,
    MATRIX_INSPECTION_2D_MATRIX = MATRIX_INSPECTION_START,
    MATRIX_INSPECTION_STRING_MATCH, //kircheis_MED5_13
    //MATRIX_INSPECTION_OCV,
    MATRIX_INSPECTION_END,
};

static LPCTSTR g_sz2DMatrixInspName[] = {
    _T("2D Matrix"),
    _T("2DID string match"), //kircheis_MED5_13
    //_T("OCV"),
};

enum enumPassive2DDInspection //mc_Insp Item Index 정의
{
    PASSIVE_INSPECTION_START = 0,
    PASSIVE_INSPECTION_2D_MISSING = PASSIVE_INSPECTION_START, //PASSIVE
    PASSIVE_INSPECTION_2D_ROTATE,
    PASSIVE_INSPECTION_2D_OFFSET_X,
    PASSIVE_INSPECTION_2D_OFFSET_Y,
    PASSIVE_INSPECTION_2D_WIDTH,
    PASSIVE_INSPECTION_2D_LENGTH,

    PASSIVE_INSPECTION_END,
};

static LPCTSTR g_szPassive2DInspectionName[] = //kircheis_Land
    {
        _T("PASSIVE_MISSING"),
        _T("PASSIVE_ROTATE"),
        _T("PASSIVE_SHIFT_LENGTH"),
        _T("PASSIVE_SHIFT_WIDTH"),
        _T("PASSIVE_WIDTH"),
        _T("PASSIVE_LENGTH"),
};

//{{
enum enumComponentInspection3D
{
    COMP_INSPECTION_3D_START = 0,
    COMP_INSPECTION_3D_COMP_MISSING = COMP_INSPECTION_3D_START,
    COMP_INSPECTION_3D_COMP_HEIGHT,
    COMP_INSPECTION_3D_COMP_QUALITY,
    COMP_INSPECTION_3D_ELECTRODE_HEIGHT,
    COMP_INSPECTION_3D_ELECTRODE_TILT,
    COMP_INSPECTION_3D_BODY_HEIGHT,
    COMP_INSPECTION_3D_BOTY_TILT,
    COMP_INSPECTION_3D_END,
};

static LPCTSTR g_szCompInspection3DName[] = {
    _T("COMP3D_MISSING"),
    _T("COMP_HEIGHT"),
    _T("COMP_QUAL"),
    _T("ELECTRODE_HEIGHT"),
    _T("ELECTRODE_TILT"),
    _T("BODY_HEIGHT"),
    _T("BODY_TILT"),
};
//}}

//{{ //kircheis_LidInsp
enum enumLidInspection2D
{
    LID_INSPECTION_2D_START = 0,
    LID_INSPECTION_2D_OFFSET_X = LID_INSPECTION_2D_START,
    LID_INSPECTION_2D_OFFSET_Y,
    LID_INSPECTION_2D_OFFSET_THETA,
    //	LID_INSPECTION_2D_SIZE_X, //kircheis_LidInspOpt
    //	LID_INSPECTION_2D_SIZE_Y,
    LID_INSPECTION_2D_END,
};

static LPCTSTR g_szLidInspection2DName[] = {
    _T("Lid dX"), _T("Lid dY"), _T("Lid Rotation"),
    //	_T("Lid Size X"),//kircheis_LidInspOpt
    //	_T("Lid Size Y"),
};

enum enumLidInspection3D
{
    LID_INSPECTION_3D_START = 0,
    LID_INSPECTION_3D_UNIT_LID_TILT_X = LID_INSPECTION_3D_START,
    LID_INSPECTION_3D_UNIT_LID_TILT_Y,
    LID_INSPECTION_3D_UNIT_LID_HIGHT,
    LID_INSPECTION_3D_UNIT_LID_HIGHT_ABS,
    LID_INSPECTION_3D_LID_WARPAGE,
    LID_INSPECTION_3D_UNIT_LID_WARPAGE,
    //{{//kircheis_MED2LW
    LID_INSPECTION_3D_UNIT_LID_WARPAGE_AX,
    LID_INSPECTION_3D_UNIT_LID_WARPAGE_AV,
    LID_INSPECTION_3D_UNIT_LID_WARPAGE_AS,
    LID_INSPECTION_3D_UNIT_LID_WARPAGE_AO,
    //}}
    LID_INSPECTION_3D_END,
};

static LPCTSTR g_szLidInspection3DName[] = {
    _T("Unit Lid Tilt X"), _T("Unit Lid Tilt Y"), _T("Unit Lid Height"), _T("Unit Lid MHeight"), _T("Lid Warpage"),
    _T("Unit Lid Warpage"),
    //{{ //kircheis_MED2LW
    _T("Lid AX Warpage"), _T("Lid AV Warpage"), _T("Lid AS Warpage"), _T("Lid AO Warpage"),
    //}}
};

//{{ //kircheis_SideInsp
enum enumGlassSubstrateInspection2D
{
    GLASS_SUBSTRATE_INSPECTION_2D_START = 0,
    GLASS_SUBSTRATE_INSPECTION_2D_OFFSET_X = GLASS_SUBSTRATE_INSPECTION_2D_START,
    GLASS_SUBSTRATE_INSPECTION_2D_OFFSET_Y,
    GLASS_SUBSTRATE_INSPECTION_2D_OFFSET_THETA,
    GLASS_SUBSTRATE_INSPECTION_2D_SIZE_X,
    GLASS_SUBSTRATE_INSPECTION_2D_SIZE_Y,
    GLASS_SUBSTRATE_INSPECTION_2D_END,
};

static LPCTSTR g_szGlassSubstrateInspection2DName[] = {
    _T("Glass Substrate dX"),
    _T("Glass Substrate dY"),
    _T("Glass Substrate Rotation"),
    _T("Glass Substrate Size X"),
    _T("Glass Substrate Size Y"),
};

enum enumSideInspection2D
{
    SIDE_INSPECTION_2D_START = 0,
    SIDE_INSPECTION_2D_TOTAL_THICKNESS = SIDE_INSPECTION_2D_START,
    SIDE_INSPECTION_2D_GLASS_THICKNESS,
    SIDE_INSPECTION_2D_TOP_SUBSTRATE_THICKNESS,
    SIDE_INSPECTION_2D_BOTTOM_SUBSTRATE_THICKNESS,
    SIDE_INSPECTION_2D_SIDE_WARPAGE,
    SIDE_INSPECTION_2D_END,
};

static LPCTSTR g_szSide2DInspectionName[] = {
    _T("Total Thickness"),
    _T("Glass Thickness"),
    _T("Top Substrate Thickness"),
    _T("Bottom Substrate Thickness"),
    _T("Side Warpage"),
};
//}}
//{{ //kircheis_POI
enum enumPatchInspection2D
{
    PATCH_INSPECTION_2D_START = 0,
    PATCH_INSPECTION_2D_OFFSET_X = PATCH_INSPECTION_2D_START,
    PATCH_INSPECTION_2D_OFFSET_Y,
    PATCH_INSPECTION_2D_OFFSET_THETA,
    //	PATCH_INSPECTION_2D_SIZE_X, //kircheis_POIOpt
    //	PATCH_INSPECTION_2D_SIZE_Y,
    PATCH_INSPECTION_2D_END,
};

static LPCTSTR g_szPatchInspection2DName[] = {
    _T("Patch dX"), _T("Patch dY"), _T("Patch Rotation"),
    //	_T("Patch Size X"),//kircheis_PatchInspOpt
    //	_T("Patch Size Y"),
};

enum enumPatchInspection3D
{
    PATCH_INSPECTION_3D_START = 0,
    PATCH_INSPECTION_3D_PATCH_HEIGHT = PATCH_INSPECTION_3D_START,
    PATCH_INSPECTION_3D_UNIT_PATCH_HIGHT,
    PATCH_INSPECTION_3D_UNIT_PATCH_HIGHT_ABS,
    PATCH_INSPECTION_3D_END,
};

static LPCTSTR g_szPatchInspection3DName[] = {
    _T("Patch Height"),
    _T("Unit Patch Height"),
    _T("Unit Patch Height (ABS)"),
};
//}}

enum enumDieInspection2D
{
    DIE_INSPECTION_2D_START = 0,
    DIE_INSPECTION_2D_OFFSET_X = LID_INSPECTION_2D_START,
    DIE_INSPECTION_2D_OFFSET_Y,
    DIE_INSPECTION_2D_OFFSET_THETA,
    DIE_INSPECTION_2D_SIZE_X,
    DIE_INSPECTION_2D_SIZE_Y,
    DIE_INSPECTION_2D_END,
};

static LPCTSTR g_szDieInspection2DName[] = {
    _T("Die dX"),
    _T("Die dY"),
    _T("Die Rotation"),
    _T("Die Size X"),
    _T("Die Size Y"),
};
//}}

//{{//kircheis_KOZ2D
enum enumUseOptionStatus
{
    OPT_NOT_USE = 0,
    OPT_USE,
};

enum enumDefectColor
{
    DEFECT_DARK = 0,
    DEFECT_BRIGHT,
    DEFECT_ALL,
};

enum enumKOZInspection2D
{
    KOZ_INSPECTION_2D_START = 0,
    KOZ_INSPECTION_2D_WIDTH = KOZ_INSPECTION_2D_START,
    KOZ_INSPECTION_2D_CHIPPING,
    KOZ_INSPECTION_2D_END,
};

static LPCTSTR g_szKOZInspection2DName[] = {
    _T("KOZ Width"),
    _T("KOZ Chipping"),
};

#define GUID_KOZ2D_MODULE_PARA_LINK_ID _T("{81DD7049-A87A-4215-8A84-283CEDC63E05}")
#define GUID_KOZ2D_MODULE_IMAGECOMBINE_NOTCH _T("{D09E4C3F-7865-448F-AF3A-43A7ED5C6341}")
#define GUID_KOZ2D_MODULE_IMAGECOMBINE _T("{8AA8A2C7-DCAE-404E-8A78-89824EDD732E}")
#define GUID_KOZ2D_MODULE_IMAGECOMBINE_CHIPPING _T("{E0B56555-1142-4BB6-BC81-6867A2A01D16}")
#define GUID_KOZ2D_MODULE_NOTCH_ALIGN_FRAME_IDX _T("{874EA094-8073-4040-B765-2E9443AD8E74}")
#define GUID_KOZ2D_MODULE_KOZ_ALIGN_FRAME_IDX _T("{4441E3F3-48EF-4E52-B3F2-37D8C16DDE92}")
#define GUID_KOZ2D_MODULE_CHIPPING_INSP_FRAME_IDX _T("{27997AE2-AD1C-4EEF-B920-0460E752814C}")
#define GUID_KOZ2D_INSP_ITEM_KOZWIDTH _T("{387E18BE-34F4-4498-AEC6-5F85B1C98069}")
#define GUID_KOZ2D_INSP_ITEM_KOZCHIPPING _T("{C48BED8A-030F-4F14-9B7C-FAC4310D9BEB}")

#define KOZ2D_EDGE_THRESH_LEVEL_LOW 1.f
#define KOZ2D_EDGE_THRESH_LEVEL_MID 3.f
#define KOZ2D_EDGE_THRESH_LEVEL_HIGH 5.f
#define KOZ2D_EDGE_THRESH_LEVEL_HIGHEST 10.f

enum enumKOZEdgeThresholdLevel
{
    EDGE_THRESHOLD_LEVEL_START = 0,
    EDGE_THRESHOLD_LEVEL_LOW = EDGE_THRESHOLD_LEVEL_START, //1.f
    EDGE_THRESHOLD_LEVEL_MIDDLE, //3.f
    EDGE_THRESHOLD_LEVEL_HIGH, //5.f
    EDGE_THRESHOLD_LEVEL_HIGHEST, //10.f
    EDGE_THRESHOLD_LEVEL_END,
};
//}}

enum enumIntensityCheckerInspection2D
{
    INTENSITYCHECKER_INSPECTION_2D_START = 0,
    INTENSITYCHECKER_INSPECTION_ROI0_1 = INTENSITYCHECKER_INSPECTION_2D_START,
    INTENSITYCHECKER_INSPECTION_ROI0_2,
    INTENSITYCHECKER_INSPECTION_ROI0_3,
    INTENSITYCHECKER_INSPECTION_ROI1_1,
    INTENSITYCHECKER_INSPECTION_ROI1_2,
    INTENSITYCHECKER_INSPECTION_ROI1_3,
    INTENSITYCHECKER_INSPECTION_ROI2_1,
    INTENSITYCHECKER_INSPECTION_ROI2_2,
    INTENSITYCHECKER_INSPECTION_ROI2_3,
    INTENSITYCHECKER_INSPECTION_2D_END,
};

enum enumIntensityCheckMode
{
    INTENSITY_CHECK_START = 0,
    INTENSITY_CHECK_MEAN = INTENSITY_CHECK_START,
    INTENSITY_CHECK_CALIBRATION,
    INTENSITY_CHECK_END,
};

static LPCTSTR g_szIntensityCheckerInspection2DName[] = {
    _T("ROI0 1st Frame Intensity"),
    _T("ROI0 2nd Frame Intensity"),
    _T("ROI0 3rd Frame Intensity"),
    _T("ROI1 1st Frame Intensity"),
    _T("ROI1 2nd Frame Intensity"),
    _T("ROI1 3rd Frame Intensity"),
    _T("ROI2 1st Frame Intensity"),
    _T("ROI2 2nd Frame Intensity"),
    _T("ROI2 3rd Frame Intensity"),
};

enum enumCouponInspection2D
{
    COUPON_INSPECTION_2D_START = 0,
    COUPON_INSPECTION_2D_ALGORITHM = COUPON_INSPECTION_2D_START,
    COUPON_INSPECTION_2D_THRESHOLD,
    COUPON_INSPECTION_2D_END,
};

static LPCTSTR g_szCouponInspection2DName[] = {
    _T("Coupon Threshold"),
};

static LPCTSTR g_szEmptyCheckName = _T("Empty Checker");

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////  Vision Inspection Item Define End ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////

enum RoundSearchDirection
{
    Round_Dir_LT = 0,
    Round_Dir_RT,
    Round_Dir_RB,
    Round_Dir_LB,
    Round_End,
};

enum enumAutoMarkTeachMode //kircheis_AutoTeach
{
    AutoMarkTestMode = 1,
    AutoMarkTeachMode,
};

//mc_MED4 Body Center or Fiducial Center
enum class enumCenterDefine : long
{
    eCenterDefine_Body_Align,
    eCenterDefine_Fiducial_Align,
};

//{{Mark Inspection Mode
static LPCTSTR g_szMarkInspctionModeName[] = {
    _T("Normal"),
    _T("Simple"),
};

enum enumMarkInspctionModeIndex
{
    MarkInspectionMode_Start = 0,
    MarkInspectionMode_Normal = MarkInspectionMode_Start,
    MarkInspectionMode_Simple,
    MarkInspectionMode_End,
};
//}}

//{{Mark Inspection Mode
enum enumUseROISettingMethod
{
    ROISettingMethod_Start = 0,
    ROISettingMethod_Manual = MarkInspectionMode_Start,
    ROISettingMethod_Mapdata,
    ROISettingMethod_End,
};
//}}


#define RECIPE_KEY_VALUE_VISION_TYPE        _T("{A1B2C3D4-E5F6-4712-ABCD-1234567890AB}")
#define RECIPE_KEY_VALUE_SIDE_VISION_NUMBER _T("{B1C2D3E4-F5A6-4721-BCDA-0987654321BA}")