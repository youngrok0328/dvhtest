#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
enum class VI_DATATYPEDEF; //이건 이전 Code의 흔적일까?
enum class II_RESULT_TEMP; //이건 이전 Code의 흔적일까?
class ImageViewEx;

//HDR_6_________________________________ Header body
//
enum class enumDebugInfoType
{
    None, // 데이터 타입 : NONE
    Image_8u_C1, // 데이터 타입 : Ipvm::Image8u
    Image_32f_C1, // 데이터 타입 : Ipvm::Image32r
    Long, // 데이터 타입 : long
    Float, // 데이터 타입 : float
    Double, // 데이터 타입 : double
    Point, // 데이터 타입 : Ipvm::Point32s2
    Point_32f_C2, // 데이터 타입 : Ipvm::Point32r2
    LineSeg_32f, // 데이터 타입 : Ipvm::LineSeg32r
    Rect, // 데이터 타입 : Ipvm::Rect32s
    Rect_32f, // 데이터 타입 : Ipvm::Rect32r
    PI_Rect, // 데이터 타입 : PI_RECT
    FPI_Rect, // 데이터 타입 : FPI_RECT
    BlobInfo, // 데이터 타입 : Ipvm::BlobInfo
    EllipseEq_32f, // 데이터 타입 : Ipvm::EllipseEq32r
    Polygon_32f, // 데이터 타입 : Ipvm::Polygon32r
    Quadrangle_32f, // 데이터 타입 : Ipvm::Quadrangle32r
    CSTRING, // 데이터 타입 : CString
};

__VISION_COMMON_API__ CString DebugInfoType2String(enumDebugInfoType type);

class __VISION_COMMON_API__ VisionDebugInfo
{
public:
    VisionDebugInfo() = delete;
    VisionDebugInfo(LPCTSTR _moduleGUID, LPCTSTR moduleName, LPCTSTR infoName, enumDebugInfoType type, long key);
    ~VisionDebugInfo();

    void ToTextOverlay(CString& strData, ImageViewEx* imageView);

    void Reset();

public:
    const CString moduleGuid;
    CString strInspName; ///< 검사항목 이름
    CString strDebugInfoName; ///< DebugInfo 이름
    enumDebugInfoType nDataType; ///< Data Type
    long nDataNum; ///< 데이터의 갯수
    void* pData; ///< 데이터 포인터
    long m_key;
};

class __VISION_COMMON_API__ VisionDebugInfoGroup
{
public:
    VisionDebugInfoGroup() = delete;
    VisionDebugInfoGroup(LPCTSTR _moduleGUID, LPCTSTR moduleName);
    ~VisionDebugInfoGroup(); //kircheis_ClearBlob

    void Add(LPCTSTR infoName, enumDebugInfoType type, long key = 0);

    VisionDebugInfo* GetDebugInfo(LPCTSTR infoName);

public:
    std::vector<VisionDebugInfo> m_vecDebugInfo;
    const CString m_moduleName;
    const CString m_moduleGuid;
};