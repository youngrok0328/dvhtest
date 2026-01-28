//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "iDataType.h"
//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
LPCTSTR GetSurfaceMaskNameHeader()
{
    return _T("Global Mask - ");
}

CString GetSurfaceMaskNameFromLayerName(LPCTSTR layerName)
{
    CString name;
    name.Format(_T("%s%s"), GetSurfaceMaskNameHeader(), layerName);
    return name;
}

Ipvm::Rect32r operator*(const Ipvm::Rect32r& object1, const Ipvm::Point32r2& object2)
{
    Ipvm::Rect32r retValue;
    retValue.m_left = object1.m_left * object2.m_x;
    retValue.m_top = object1.m_top * object2.m_y;
    retValue.m_right = object1.m_right * object2.m_x;
    retValue.m_bottom = object1.m_bottom * object2.m_y;

    return retValue;
}

Ipvm::Rect32r operator*(const Ipvm::Rect32s& object1, const Ipvm::Point32r2& object2)
{
    Ipvm::Rect32r retValue;
    retValue.m_left = static_cast<float>(object1.m_left * object2.m_x);
    retValue.m_top = static_cast<float>(object1.m_top * object2.m_y);
    retValue.m_right = static_cast<float>(object1.m_right * object2.m_x);
    retValue.m_bottom = static_cast<float>(object1.m_bottom * object2.m_y);

    return retValue;
}

Ipvm::Rect32r operator-(const Ipvm::Rect32s& object1, const Ipvm::Point32r2& object2)
{
    Ipvm::Rect32r retValue;
    retValue.m_left = object1.m_left - object2.m_x;
    retValue.m_top = object1.m_top - object2.m_y;
    retValue.m_right = object1.m_right - object2.m_x;
    retValue.m_bottom = object1.m_bottom - object2.m_y;

    return retValue;
}

Ipvm::Point32r2 operator-(const Ipvm::Point32r2& object1, const Ipvm::Point32s2& object2)
{
    Ipvm::Point32r2 retValue;
    retValue.m_x = object1.m_x - object2.m_x;
    retValue.m_y = object1.m_y - object2.m_y;

    return retValue;
}

Ipvm::Point32r2 operator*(const Ipvm::Point32s2& object1, const Ipvm::Point32r2& object2)
{
    Ipvm::Point32r2 retValue;
    retValue.m_x = object1.m_x * object2.m_x;
    retValue.m_y = object1.m_y * object2.m_y;

    return retValue;
}

BOOL LinkEx(BOOL save, CiDataBase& db, Ipvm::Rect32r& io_data)
{
    if (!db[_T("left")].Link(save, io_data.m_left))
        return FALSE;
    if (!db[_T("top")].Link(save, io_data.m_top))
        return FALSE;
    if (!db[_T("right")].Link(save, io_data.m_right))
        return FALSE;
    if (!db[_T("bottom")].Link(save, io_data.m_bottom))
        return FALSE;

    return TRUE;
}

BOOL LinkEx(BOOL save, CiDataBase& db, Ipvm::Point32r2& io_data)
{
    if (!db[_T("x")].Link(save, io_data.m_x))
        return FALSE;
    if (!db[_T("y")].Link(save, io_data.m_y))
        return FALSE;

    return TRUE;
}
