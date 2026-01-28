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
enum II_RESULT_VALUE
{
    NOT_MEASURED,
    PASS,
    MARGINAL,
    REJECT,
    INVALID,
    EMPTY,
    COUPON,
    DOUBLEDEVICE,
    RESULT_TYPE_END
};

static LPCTSTR g_szResultText[RESULT_TYPE_END] = {_T("Not Measured"), _T("Pass"), _T("Marginal"), _T("Reject"),
    _T("Invalid"), _T("Empty"), _T("Coupon"), _T("Double Device")};

static LPCTSTR GetResultText(II_RESULT_VALUE result, bool makeUpper, bool isNGRV_TDx)
{
    static TCHAR szBuffer[32];

    if (result < NOT_MEASURED || result >= RESULT_TYPE_END)
    {
        return makeUpper ? _T("UNKNOWN") : _T("Unknown");
    }

    if (result == NOT_MEASURED && isNGRV_TDx)
    {
        // NGRV TDx에서는 Not Measured를 "Not Measured"로 표시하지 않고 ""로 표시한다.
        return _T("");
    }

    _tcscpy_s(szBuffer, _countof(szBuffer), g_szResultText[result]);

    if (makeUpper)
        _tcsupr_s(szBuffer, _countof(szBuffer));

    return szBuffer;
}

static LPCTSTR GetResultText(long result, bool makeUpper, bool isNGRV_TDx)
{
    return GetResultText(static_cast<II_RESULT_VALUE>(result), makeUpper, isNGRV_TDx);
}

enum class II_RESULT_TYPE : long
{
    MEASURED, // U M mm mil
    DEGREE, // 각도
    PERCENT, // %
    COUNT, // Number
    GRAY_VALUE, // Empty Check, BlobSize
    SURFACE, // Sur
};

enum class HostReportCategory : long
{
    NO_INSP, // Empty Check같은 항목
    MEASURE, // 일반 검사 항목
    BALL,
    LAND,
    COMPONENT,
    MARK, // 마크 검사
    PIN1, // PIN1
    CODE2D, // 2D Matrix
    SURFACE,
    INVALID,
    GEOMETRY,
    LID, //Lid	//kircheis_LidInsp
    DIE, //Die
    BALL_WARP_SHAPE, //kircheis_MED2LW
    LAND_WARP_SHAPE, //kircheis_MED2LW
    LID_WARP_SHAPE, //kircheis_MED2LW
    KOZ, //kircheis_KOZ//Host에 공유 필요
    GLASS_SUBSTRATE2D, //kircheis_SideInsp//Host에 공유 필요
    END
};

enum enCommonSideVisionModule //mc_Handler와 공용으로 사용하기 위한 enumDefine이 필요.
{
    Common_SIDE_VISIONMODULE_NOSIDEVISION = -1,
    Common_SIDE_VISIONMODULE_START = 0,
    Common_SIDE_VISIONMODULE_FRONT = Common_SIDE_VISIONMODULE_START,
    Common_SIDE_VISIONMODULE_REAR,
    Common_SIDE_VISIONMODULE_END,
};

enum enDispositionType
{
    DISPOSITION_START = 0,
    DISPOSITION_NULL = DISPOSITION_START,
    DISPOSITION_PASS,
    DISPOSITION_MARGINAL,
    DISPOSITION_INVALID,
    DISPOSITION_PASSIVE,
    DISPOSITION_REJECT,
    DISPOSITION_END,
};

static LPCTSTR g_szDispositionText[] = {
    _T(""),
    _T("Pass"),
    _T("Marginal"),
    _T("Invalid"),
    _T("Passive"),
    _T("Reject"),
};

static LPCTSTR GetDispositionText(enDispositionType type, bool makeUpper)
{
    static TCHAR szBuffer[32];

    if (type < DISPOSITION_START || type >= DISPOSITION_END)
        return makeUpper ? _T("UNKNOWN") : _T("Unknown");

    _tcscpy_s(szBuffer, _countof(szBuffer), g_szDispositionText[type]);

    if (makeUpper)
        _tcsupr_s(szBuffer, _countof(szBuffer));

    return szBuffer;
}

static LPCTSTR GetDispositionText(long type, bool makeUpper)
{
    return GetDispositionText(static_cast<enDispositionType>(type), makeUpper);
}

#if defined(USE_BY_HANDLER)
// 쓰지 않는 enum과 String이다.
// 이것을 쓰면 안된다. Vision에서 쓰지도 않는 값이다.
// 그러나 현재 호스트에서 해당 정의를 사용하는 코드가 있어서 일단 살려는 두지만
// 사용하는 코드도 잘못된 코드이다.

enum SurfaceCriteria
{
    Cirteria_Start = 0,
    Cirteria_KeyContrast = Cirteria_Start,
    Cirteria_AverageContrast,
    Cirteria_LocalContrast,
    Cirteria_AuxLocalContrast,
    Cirteria_Deviation,
    Cirteria_AuxDeviation,
    Cirteria_EdgeEnergy,
    Cirteria_BinaryDensity,
    Cirteria_Area,
    Cirteria_Width,
    Cirteria_Length,
    Cirteria_LengthX,
    Cirteria_LengthY,
    Cirteria_ToalLength,
    Cirteria_Thickness,
    Cirteria_Locus,
    Cirteria_AspectRatio,
    Cirteria_DistXtoBodyCenter,
    Cirteria_DistYtoBodyCenter,
    Cirteria_DistToBody,
    Cirteria_LocalAreaPercentAll,
    Cirteria_LocalAreaPercent,
    Cirteria_DefectROIRatio,
    Cirteria_End,
};

static const char* g_chSurfaceCriteriaName[] = {
    "KeyContrast",
    "AverageContrast",
    "LocalContrast",
    "AuxLocalContrast",
    "Deviation",
    "AuxDeviation",
    "EdgeEnergy",
    "BinaryDensity",
    "Area",
    "Width",
    "Length",
    "LengthX",
    "LengthY",
    "TotalLength",
    "Thickness",
    "Locus",
    "AspectRatio",
    "DistXtoBodyCenter",
    "DistYtoBodyCenter",
    "DistToBody",
    "LocalAreaPercentAll",
    "LocalAreaPercent",
    "DefectROIRatio",

};
#endif

#define STR_FULL_NAME_INVALID _T("[Invalid]Invalid")
#define STR_INVALID _T("Invalid")

__VISION_HOST_COMMON_API__ COLORREF Result2Color(long nResult);
__VISION_HOST_COMMON_API__ CString Result2String(long nResult);

// 2D 조명 정보
#define LED_ILLUM_CHANNEL_DEFAULT 13
static const long g_illuminationChannelCount = 16;

static LPCTSTR g_illuminationChannelNames[g_illuminationChannelCount]
    = {_T("CH1"), _T("CH2"), _T("CH3"), _T("CH4"), _T("CH5"), _T("CH6"), _T("CH7"), _T("CH8"), _T("CH9"), _T("CH10"),
        _T("CH11"), _T("CH12"), _T("CH13"), _T("CH14"), _T("CH15"), _T("CH16")};

static const COLORREF g_illuminationChannelColors[g_illuminationChannelCount] = {
    RGB(255, 0, 0),
    RGB(0, 200, 0),
    RGB(0, 0, 255),
    RGB(255, 0, 0),
    RGB(0, 200, 0),
    RGB(0, 0, 255),
    RGB(255, 0, 0),
    RGB(0, 200, 0),
    RGB(0, 0, 255),
    RGB(255, 0, 0),
    RGB(255, 0, 0),
    RGB(255, 0, 0),
    RGB(0, 0, 255),
    RGB(255, 0, 0),
    RGB(255, 0, 0),
    RGB(255, 0, 0),
};

// NGRV 조명 정보
#define LED_ILLUM_NGRV_CHANNEL_MAX 12
static const long g_illuminationChannelCount_NGRV = 12;

static LPCTSTR g_illuminationChannelNames_NGRV[g_illuminationChannelCount_NGRV] = {_T("CH01"), _T("CH02"), _T("CH03"),
    _T("CH04"), _T("CH05"), _T("CH06"), _T("CH07"), _T("CH08"), _T("CH09"), _T("CH10"), _T("CH11"), _T("CH12")};

static const COLORREF g_illuminationChannelColors_NGRV[g_illuminationChannelCount_NGRV] = {
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
    RGB(0, 0, 0),
};

// Side 조명 정보
#define LED_ILLUM_SIDE_CHANNEL_MAX 10
static const long g_illuminationChannelCount_SIDE = 10;

static LPCTSTR g_illuminationChannelNames_SIDE[g_illuminationChannelCount_SIDE]
    = {_T("CH1"), _T("CH2"), _T("CH3"), _T("CH4"), _T("CH5"), _T("CH6"), _T("CH7"), _T("CH8"), _T("CH9"), _T("CH10")};

static const COLORREF g_illuminationChannelColors_SIDE[g_illuminationChannelCount_SIDE]
    = {RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255), RGB(255, 0, 0), RGB(0, 200, 0), RGB(0, 0, 255), RGB(255, 0, 0),
        RGB(0, 200, 0), RGB(0, 0, 255), RGB(255, 0, 0)};

// SWIR 조명 정보
#define LED_ILLUM_SWIR_CHANNEL_MAX 14
static const long g_illuminationChannelCount_SWIR = 14;

static LPCTSTR g_illuminationChannelNames_SWIR[g_illuminationChannelCount_SWIR]
    = {_T("CH1 (1200nm) Layer1"), _T("CH2 (1200nm) Layer1"), _T("CH3 (1200nm) Layer1"), _T("CH4 (1200nm) Layer1"),
        _T("CH5 (1200nm) Layer1"), _T("CH6 (1650nm) Layer2"), _T("CH7 (1650nm) Layer2"), _T("CH8 (1650nm) Layer2"),
        _T("CH9 (1650nm) Layer2"), _T("CH10 (1200nm) Layer3"), _T("CH11 (1200nm) Layer3"), _T("CH12 (1200nm) Layer3"),
        _T("CH13 (1650nm) Coax.1"), _T("CH14 (1200nm) Coax.2")};

static const COLORREF g_illuminationChannelColors_SWIR[g_illuminationChannelCount_SWIR]
    = {RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
        RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0)};

// TR 조명 정보
#define LED_ILLUM_INPOCKET_CHANNEL_MAX 3
static const long g_illuminationChannelCount_INPOCKET = 3;

static LPCTSTR g_illuminationChannelNames_INPOCKET[g_illuminationChannelCount_INPOCKET]
    = {_T("CH1"), _T("CH2"), _T("CH3")};

static const COLORREF g_illuminationChannelColors_INPOCKET[g_illuminationChannelCount_INPOCKET]
    = {RGB(255, 0, 0), RGB(0, 0, 255), RGB(0, 0, 255)};

#define LED_ILLUM_OTI_CHANNEL_MAX 3
static const long g_illuminationChannelCount_OTI = 3;

static LPCTSTR g_illuminationChannelNames_OTI[g_illuminationChannelCount_OTI] = {_T("CH1"), _T("CH2"), _T("CH3")};

static const COLORREF g_illuminationChannelColors_OTI[g_illuminationChannelCount_OTI]
    = {RGB(255, 0, 0), RGB(0, 0, 255), RGB(0, 0, 0)};
