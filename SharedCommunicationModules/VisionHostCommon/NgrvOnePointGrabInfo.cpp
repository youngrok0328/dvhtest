//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvOnePointGrabInfo.h"

//CPP_2_________________________________ This project's headers
#include "VisionHostBaseDef.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
NgrvOnePointGrabInfo::NgrvOnePointGrabInfo()
{
    Init();
}

NgrvOnePointGrabInfo::~NgrvOnePointGrabInfo()
{
    Init();
}

void NgrvOnePointGrabInfo::Init()
{
    m_defectPositionName = _T("");
    m_itemID = _T("000"); // Default item ID
    m_defectID = _T("1");
    m_defectCode = _T("");
    m_disposition = GetDispositionText(DISPOSITION_NULL, true);
    m_decision = GetResultText(NOT_MEASURED, true, true);
    m_imageSizeX = 9334; // Default image size X
    m_imageSizeY = 7000; // Default image size Y
    m_imageCenterX = 4667; // Default image center X
    m_imageCenterY = 3500; // Default image center Y
    m_afType = 0;
    m_grabHeightOffset_um = 0.f;
    m_stitchX = 0;
    m_stitchY = 0;
    m_vecIllumUse.clear();
    m_vecPointGrabPosition_um.clear();
}

CArchive& operator<<(CArchive& ar, const Ipvm::Point32r2& src)
{
    ar << src.m_x;
    ar << src.m_y;

    return ar;
}

CArchive& operator>>(CArchive& ar, Ipvm::Point32r2& dst)
{
    ar >> dst.m_x;
    ar >> dst.m_y;

    return ar;
}

CArchive& operator<<(CArchive& ar, const NgrvOnePointGrabInfo& src)
{
    CPoint sfptTemp;
    ar << src.m_defectPositionName;
    ar << src.m_itemID; // Added itemID to store the ID of the item
    ar << src.m_defectID; // Added defectID to store the ID of the defect
    ar << src.m_defectCode;
    ar << src.m_disposition;
    ar << src.m_decision;
    ar << src.m_imageSizeX;
    ar << src.m_imageSizeY;
    ar << src.m_imageCenterX;
    ar << src.m_imageCenterY;
    ar << src.m_afType;
    ar << src.m_grabHeightOffset_um;
    ar << src.m_stitchX;
    ar << src.m_stitchY;

    long illumSize = (long)src.m_vecIllumUse.size();
    ar << illumSize;
    for (long illumIndex = 0; illumIndex < illumSize; illumIndex++)
    {
        ar << src.m_vecIllumUse[illumIndex];
    }

    long posSize = (long)(src.m_vecPointGrabPosition_um.size());
    ar << posSize;
    for (long posIndex = 0; posIndex < posSize; posIndex++)
    {
        ar << src.m_vecPointGrabPosition_um[posIndex];
    }

    return ar;
}

CArchive& operator>>(CArchive& ar, NgrvOnePointGrabInfo& dst)
{
    dst.Init();
    ar >> dst.m_defectPositionName;
    ar >> dst.m_itemID; // Read itemID from the archive
    ar >> dst.m_defectID; // Read defectID from the archive
    ar >> dst.m_defectCode;
    ar >> dst.m_disposition;
    ar >> dst.m_decision;
    ar >> dst.m_imageSizeX;
    ar >> dst.m_imageSizeY;
    ar >> dst.m_imageCenterX;
    ar >> dst.m_imageCenterY;
    ar >> dst.m_afType;
    ar >> dst.m_grabHeightOffset_um;
    ar >> dst.m_stitchX;
    ar >> dst.m_stitchY;

    long illumSize = 0;
    ar >> illumSize;
    dst.m_vecIllumUse.resize(illumSize);
    for (long illumIndex = 0; illumIndex < illumSize; illumIndex++)
    {
        ar >> dst.m_vecIllumUse[illumIndex];
    }

    long posSize = 0;
    ar >> posSize;
    dst.m_vecPointGrabPosition_um.resize(posSize);
    for (long posIndex = 0; posIndex < posSize; posIndex++)
    {
        ar >> dst.m_vecPointGrabPosition_um[posIndex];
    }

    return ar;
}
