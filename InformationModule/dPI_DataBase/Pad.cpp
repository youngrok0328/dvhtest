//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Pad.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Package
{
Pad::Pad()
{
    MakeCenter();
}

Pad::Pad(LPCTSTR name, float i_fOffsetX, float i_fOffsetY, float i_fWidth, float i_fLength, float i_fHeight,
    long i_nAngle, PAD_TYPE i_nPadType, BOOL i_bMount, BOOL i_bIgnore)
{
    strPadName = name;
    m_offset.m_x = i_fOffsetX;
    m_offset.m_y = i_fOffsetY;
    fWidth = i_fWidth;
    fLength = i_fLength;
    fHeight = i_fHeight;
    nAngle = i_nAngle;
    m_padType = i_nPadType;
    bMount = i_bMount;
    bIgnore = i_bIgnore;

    MakeCenter();
}

Pad::~Pad()
{
}

BOOL Pad::LinkDataBase(long version, long padIndex, BOOL bSave, CiDataBase& db)
{
    if (version < 2)
    {
        CString strTemp;
        strTemp.Format(_T("PadDataInMap_PadName_%d"), padIndex);
        if (!db[strTemp].Link(bSave, strPadName))
        {
            strPadName.Format(_T("%d"), padIndex);
        }

        strTemp.Format(_T("PadDataInMap_Xcoord_%d"), padIndex);
        if (!db[strTemp].Link(bSave, m_offset.m_x))
            m_offset.m_x = 0.f;

        strTemp.Format(_T("PadDataInMap_Ycoord_%d"), padIndex);
        if (!db[strTemp].Link(bSave, m_offset.m_y))
            m_offset.m_y = 0.f;

        strTemp.Format(_T("PadDataInMap_Width_%d"), padIndex);
        if (!db[strTemp].Link(bSave, fWidth))
            fWidth = 0.5f;

        strTemp.Format(_T("PadDataInMap_Length_%d"), padIndex);
        if (!db[strTemp].Link(bSave, fLength))
            fLength = 1.f;

        strTemp.Format(_T("PadDataInMap_Height_%d"), padIndex);
        if (!db[strTemp].Link(bSave, fHeight))
            fHeight = 0.3f;

        strTemp.Format(_T("PadDataInMap_Angle_%d"), padIndex);
        if (!db[strTemp].Link(bSave, nAngle))
            nAngle = 0;

        strTemp.Format(_T("PadDataInMap_PadType_%d"), padIndex);
        if (!db[strTemp].Link(bSave, (long&)m_padType))
            m_padType = _typeTriangle;

        strTemp.Format(_T("PadDataInMap_Mount_%d"), padIndex);
        if (!db[strTemp].Link(bSave, bMount))
            bMount = TRUE;

        strTemp.Format(_T("PadDataInMap_Visible_%d"), padIndex);
        if (!db[strTemp].Link(bSave, bIgnore))
            bIgnore = FALSE;
    }
    else
    {
        // 외부에서 PAD별로 iDatabase를 분리해서 전달해 주기 시작했다
        if (!db[_T("Name")].Link(bSave, strPadName))
        {
            strPadName.Format(_T("%d"), padIndex);
        }

        if (!db[_T("Xcoord")].Link(bSave, m_offset.m_x))
            m_offset.m_x = 0.f;
        if (!db[_T("Ycoord")].Link(bSave, m_offset.m_y))
            m_offset.m_y = 0.f;
        if (!db[_T("Width")].Link(bSave, fWidth))
            fWidth = 0.5f;
        if (!db[_T("Length")].Link(bSave, fLength))
            fLength = 1.f;
        if (!db[_T("Height")].Link(bSave, fHeight))
            fHeight = 0.3f;
        if (!db[_T("Angle")].Link(bSave, nAngle))
            nAngle = 0;
        if (!db[_T("Type")].Link(bSave, (long&)m_padType))
            m_padType = _typeTriangle;
        if (!db[_T("Mount")].Link(bSave, bMount))
            bMount = TRUE;
        if (!db[_T("Ignore")].Link(bSave, bIgnore))
            bIgnore = FALSE;
    }

    if (!bSave)
    {
        if (version == 0)
        {
            if (m_padType == _typeTriangle)
            {
                // 옛날 Job중 Triangle로 되어 있는 것은 PinIndex로 바꾸어주자
                // 예전에는 PinIndex Type이 없었다

                m_padType = _typePinIndex;
            }
        }
        MakeCenter();
    }

    return TRUE;
}

const Ipvm::Point32r2& Pad::GetCenter() const
{
    return m_center;
}

const Ipvm::Point32r2& Pad::GetOffset() const
{
    return m_offset;
}

const PAD_TYPE Pad::GetType() const
{
    return m_padType;
}

void Pad::SetDefault()
{
    strPadName = _T("");
    fWidth = 0.f;
    fLength = 0.f;
    fHeight = 0.f;
    nAngle = 0;
    m_padType = _typeTriangle;
    bMount = FALSE;
    bIgnore = FALSE;

    m_offset.m_x = 0.f;
    m_offset.m_y = 0.f;

    MakeCenter();
}

void Pad::SetOffsetX(float value)
{
    m_offset.m_x = value;
    MakeCenter();
}

void Pad::SetOffsetY(float value)
{
    m_offset.m_y = value;
    MakeCenter();
}

void Pad::SetType(PAD_TYPE type)
{
    m_padType = type;
    MakeCenter();
}

void Pad::MakeCenter()
{
    m_center = m_offset;

    if (m_padType == _typePinIndex)
    {
        // PinIndex는 Gerber좌표를 직각부분을 전달해 준다
        m_center.m_x += ((m_offset.m_x > 0) ? -fWidth : fWidth) * 0.5f;
        m_center.m_y += ((m_offset.m_y > 0) ? -fLength : fLength) * 0.5f;
    }
}

LPCTSTR GetPAD_Type_Name(PAD_TYPE type)
{
    switch (type)
    {
        case PAD_TYPE::_typeCircle:
            return _T("Circle");
            break;
        case PAD_TYPE::_typeRectangle:
            return _T("Rectangle");
            break;
        case PAD_TYPE::_typeTriangle:
            return _T("Triangle");
            break;
        case PAD_TYPE::_typePinIndex:
            return _T("PinIndex");
            break;
    }

    return _T("Unknown");
}

} // namespace Package