#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "FPI_RECT.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
struct PI_RECT;
class CiDataBase;

//HDR_6_________________________________ Header body
//
/**
	데이터 연동을 위한 타입 선언
**/

#define GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME _T("SURFACE COMMON IGNORE MASK")
#define ITP_PI 3.1415926535897932384626433832795
#define ITP_TwoPI (2 * ITP_PI)
#define ITP_FourPI (4 * ITP_PI)
#define ITP_HalfPI (0.5 * ITP_PI)
#define DEF_RAD_TO_DEG 57.295779513082 // = (180 / ITP_PI), radian to degree
#define DEF_DEG_TO_RAD 0.017453292519943 // = (ITP_PI / 180), degree to radian
#define ITP_RAD_TO_DEG 57.296f // = (180 / ITP_PI), radian to degree
#define ITP_DEG_TO_RAD 0.0175f // = (ITP_PI / 180), degree to radian

LPCTSTR _DA_BASE_MODULE_API GetSurfaceMaskNameHeader();
CString _DA_BASE_MODULE_API GetSurfaceMaskNameFromLayerName(LPCTSTR layerName);

enum VBResult
{
    VISION_NOT_MEASURED = 1,
    VISION_PASS = 2,
    VISION_MARGINAL = 4,
    VISION_RETOUCH = 8,
    VISION_REJECT = 16,
    VISION_INVALID = 32,
    VISION_EMPTY = 64
};

Ipvm::Rect32r _DA_BASE_MODULE_API operator*(const Ipvm::Rect32r& object1, const Ipvm::Point32r2& object2);
Ipvm::Rect32r _DA_BASE_MODULE_API operator*(const Ipvm::Rect32s& object1, const Ipvm::Point32r2& object2);
Ipvm::Rect32r _DA_BASE_MODULE_API operator-(const Ipvm::Rect32s& object1, const Ipvm::Point32r2& object2);
Ipvm::Point32r2 _DA_BASE_MODULE_API operator-(const Ipvm::Point32r2& object1, const Ipvm::Point32s2& object2);
Ipvm::Point32r2 _DA_BASE_MODULE_API operator*(const Ipvm::Point32s2& object1, const Ipvm::Point32r2& object2);

BOOL _DA_BASE_MODULE_API LinkEx(BOOL save, CiDataBase& db, Ipvm::Rect32r& io_data);
BOOL _DA_BASE_MODULE_API LinkEx(BOOL save, CiDataBase& db, Ipvm::Point32r2& io_data);

struct _DA_BASE_MODULE_API SDieLidPatchInfoDB
{ //kircheis_InspLid
    CString strName;
    CString strType;

    float fOffsetX_um;
    float fOffsetY_um;
    float fWidth_um;
    float fLength_um;
    float fHeight_um;
    float fAngle_Deg;

    Ipvm::Rect32s rtSpecROI;
};

enum eComponentTypeDefine
{
    tyCHIP = 0,
    tyPASSIVE,
    tyARRAY,
    tyHEATSINK,
    tyBALL,
};

enum enumPassiveTypeDefine
{
    enum_PassiveType_Capacitor,
    enum_PassiveType_Register,
    enum_PassiveType_MIA,
    euum_PassiveType_Unknown, //항상 마지막이 되게 한다.
};

class BaseAlignResult
{
public:
    BaseAlignResult()
    {
    }
    virtual ~BaseAlignResult()
    {
    }
};

struct _DA_BASE_MODULE_API sPassive_InfoDB
{
    enumPassiveTypeDefine ePassiveType;
    eComponentTypeDefine eComponentType;

    CString strCompName;
    CString strCompSpec;
    CString strCompType;
    CString strSpecName;

    Ipvm::Point32r2 fSpecPos_mm;
    Ipvm::Point32r2 fSpecPos_px;
    Ipvm::Point32r2 fSpecPos_um;

    float fPassiveSpecWidth_mm;
    float fPassiveSpecLength_mm;
    float fPassiveAngle;
    float fPassiveHeight_mm;

    float fPassivePAD_Gap_mm;
    float fPassivePAD_Width_mm;
    float fPassivePAD_Length_mm;

    float fPassiveElectrodeWidth_mm;
    float fPassiveElectrodeHeight_mm;

    FPI_RECT sfrtSpecROI_mm;
    FPI_RECT sfrtSpecROI_px;

    BaseAlignResult* pResult;
};