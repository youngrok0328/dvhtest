//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "IllumInfo2D.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <algorithm>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IllumInfo2D::IllumInfo2D()
    : m_2D_colorFrame(FALSE)
    , m_2D_colorFrameIndex_red(0)
    , m_2D_colorFrameIndex_green(0)
    , m_2D_colorFrameIndex_blue(0)
{
}

IllumInfo2D::~IllumInfo2D()
{
}

BOOL IllumInfo2D::LinkDataBase(BOOL bSave, long jobVerion, CiDataBase& jobDB)
{
    long nSize = long(m_illuminations_ms.size());

    if (bSave)
        nSize = (long)min(10, nSize);

    if (jobVerion < 4)
    {
        if (!jobDB[_T("VisionUnit")][_T("IllumBright")].Link(bSave, nSize))
            nSize = 1;
    }
    else
    {
        if (!jobDB[_T("frame count")].Link(bSave, nSize))
            nSize = 1;
    }

    if (!bSave && SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_NGRV_INSP)
    {
        if (nSize == 0)
        {
            nSize = 1;
        }

        nSize = (long)min(10, nSize);
    }
    else if (!bSave && SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        nSize = LED_ILLUM_FRAME_MAX; // NGRV Frame은 9개를 기본으로 초기화 시킨다 - JHB_NGRV
    }

    if (!bSave)
    {
        resetInfo();
        m_illuminations_ms.resize(nSize);
        m_illuminations_name.resize(nSize);
    }

    // 2D Illum 관련
    if (jobVerion < 2)
    {
        for (long n = 0; n < nSize; n++)
        {
            auto& illums = m_illuminations_ms[n];
            auto& db = jobDB.GetSubDBFmt(_T("{146462E3-A673-4A3D-8DD8-1D9337E3400F}_%d"), n);

            for (long m = 0; m < illums.size(); m++)
            {
                if (!db.GetSubDBFmt(_T("{146462E3-A673-4A3D-8DD8-1D9337E3400F}_%d"), m).Link(bSave, illums[m]))
                    illums[m] = 0;
            }
        }
    }
    else
    {
        // GUID Key가 똑같아서 Compare Job시에 이름을 바꿔서 표시하기 힘들어서 이름을 바꾼다
        for (long n = 0; n < nSize; n++)
        {
            auto& illums = m_illuminations_ms[n];
            auto& db = jobDB.GetSubDBFmt(_T("{7B371BA0-9634-458D-995F-25811D1DF0E0}_%d"), n);

            for (long m = 0; m < illums.size(); m++)
            {
                db.GetSubDBFmt(_T("{3BA6F119-E78C-401F-A87A-BA4AC05D2F96}_%d"), m).Link(bSave, m_illuminations_name[n]);
                if (!db.GetSubDBFmt(_T("{146462E3-A673-4A3D-8DD8-1D9337E3400F}_%d"), m).Link(bSave, illums[m]))
                    illums[m] = 0;
            }
        }
    }

    //k High Position 조명값 Frame 번호 저장
    {
        if (jobVerion < 4)
        {
            nSize = (long)m_vecnHighIllumInfo.size();
            if (!jobDB[_T("VisionUnit")][_T("HighIllumInfo")].Link(bSave, nSize))
                nSize = 0;
        }
        else
        {
            nSize = (long)m_vecnHighIllumInfo.size();
            if (!jobDB[_T("High IllumInfo count")].Link(bSave, nSize))
                nSize = 0;
        }

        m_vecnHighIllumInfo.resize(nSize);

        for (long n = 0; n < nSize; n++)
        {
            if (!jobDB.GetSubDBFmt(_T("{3EF5D3B3-3543-475E-AB2C-5132D20699C9}_%d"), n)
                    .Link(bSave, m_vecnHighIllumInfo[n]))
                m_vecnHighIllumInfo[n] = 0;
        }
    }

    if (!jobDB.GetSubDBFmt(_T("{31DFD801-EFE3-4E07-985D-B8F647706820}")).Link(bSave, m_2D_colorFrame))
    {
        m_2D_colorFrame = FALSE;
    }

    if (!jobDB.GetSubDBFmt(_T("{F93AC20F-077D-465C-97A1-3BC06DDA6AB0}")).Link(bSave, m_2D_colorFrameIndex_red))
    {
        m_2D_colorFrameIndex_red = 0;
    }

    if (!jobDB.GetSubDBFmt(_T("{315F2719-E54F-46F7-9859-B46A9BE5CAD7}")).Link(bSave, m_2D_colorFrameIndex_green))
    {
        m_2D_colorFrameIndex_green = 0;
    }

    if (!jobDB.GetSubDBFmt(_T("{89D343CF-BDD4-4ABE-9A50-95935896E2BF}")).Link(bSave, m_2D_colorFrameIndex_blue))
    {
        m_2D_colorFrameIndex_blue = 0;
    }

    return TRUE;
}

long IllumInfo2D::getTotalFrameCount() const
{
    return long(m_illuminations_ms.size());
}

long IllumInfo2D::getHighFrameCount() const
{
    return long(m_vecnHighIllumInfo.size());
}

bool IllumInfo2D::needPositionMovement() const
{
    return m_vecnHighIllumInfo.size() > 0;
}

bool IllumInfo2D::isHighFrame(long frameIndex) const
{
    return getHighFrameVectorIndex(frameIndex) >= 0;
}

long IllumInfo2D::getHeighFrameIndex(long index) const
{
    return m_vecnHighIllumInfo[index];
}

std::array<float, 16>& IllumInfo2D::getIllum(long frameIndex)
{
    return m_illuminations_ms[frameIndex];
}

const std::array<float, 16>& IllumInfo2D::getIllum(long frameIndex) const
{
    return m_illuminations_ms[frameIndex];
}

LPCTSTR IllumInfo2D::getIllumName(long frameIndex) const
{
    if (frameIndex < 0 || frameIndex >= long(m_illuminations_name.size()))
    {
        return _T("");
    }

    return m_illuminations_name[frameIndex];
}

CString IllumInfo2D::getIllumFullName(long frameIndex) const
{
    CString text;
    switch (frameIndex)
    {
        case 0:
            text = _T("1st frame");
            break;
        case 1:
            text = _T("2nd frame");
            break;
        case 2:
            text = _T("3rd frame");
            break;
        default:
            text.Format(_T("%dth frame"), frameIndex + 1);
            break;
    }

    if (frameIndex >= 0 && frameIndex < long(m_illuminations_name.size()))
    {
        if (!m_illuminations_name[frameIndex].IsEmpty())
        {
            text.AppendFormat(_T(" - %s"), LPCTSTR(m_illuminations_name[frameIndex]));
        }
    }

    return text;
}

CString* IllumInfo2D::getIllumNamePtr(long frameIndex)
{
    if (frameIndex < 0 || frameIndex >= long(m_illuminations_name.size()))
    {
        return nullptr;
    }

    return &m_illuminations_name[frameIndex];
}

const float& IllumInfo2D::getIllumChannel_ms(long frameIndex, long channelIndex) const
{
    return getIllum(frameIndex)[channelIndex];
}

float& IllumInfo2D::getIllumChannel_ms(long frameIndex, long channelIndex)
{
    return getIllum(frameIndex)[channelIndex];
}

void IllumInfo2D::resetInfo()
{
    m_vecnHighIllumInfo.clear();
    m_illuminations_ms.clear();
    m_illuminations_name.clear();

    m_2D_colorFrame = FALSE;
    m_2D_colorFrameIndex_red = 0;
    m_2D_colorFrameIndex_green = 0;
    m_2D_colorFrameIndex_blue = 0;
}

void IllumInfo2D::resetHighFrame()
{
    m_vecnHighIllumInfo.clear();
}

void IllumInfo2D::disableHighFrame(long frameIndex)
{
    if (frameIndex < 0 || frameIndex >= long(m_illuminations_ms.size()))
    {
        return;
    }

    for (int idx = 0; idx < m_vecnHighIllumInfo.size(); idx++)
    {
        if (m_vecnHighIllumInfo[idx] == frameIndex)
        {
            m_vecnHighIllumInfo.erase(m_vecnHighIllumInfo.begin() + idx);
            break;
        }
    }
}

bool IllumInfo2D::enableHighFrame(long frameIndex)
{
    if (frameIndex < 0 || frameIndex >= long(m_illuminations_ms.size()))
    {
        return false;
    }

    if (m_vecnHighIllumInfo.size() == LED_ILLUM_FRAME_MAX - 1)
    {
        return false;
    }

    if (isHighFrame(frameIndex))
    {
        // 이미 켜져 있다
        return true;
    }

    m_vecnHighIllumInfo.push_back(frameIndex);
    std::sort(m_vecnHighIllumInfo.begin(), m_vecnHighIllumInfo.end());

    return true;
}

void IllumInfo2D::setIllumName(long frameIndex, LPCTSTR name)
{
    m_illuminations_name[frameIndex] = name;
}

bool IllumInfo2D::insertFrame(long frameIndex, bool isHighFrame, LPCTSTR name)
{
    if (frameIndex < 0 || frameIndex > m_illuminations_ms.size())
    {
        ASSERT(!_T("어디에?"));
        return false;
    }

    std::array<float, 16> illum;
    illum.fill(0.f);

    m_illuminations_ms.insert(m_illuminations_ms.begin() + frameIndex, illum);
    m_illuminations_name.insert(m_illuminations_name.begin() + frameIndex, name);

    for (int idx = 0; idx < long(m_vecnHighIllumInfo.size()); idx++)
    {
        if (m_vecnHighIllumInfo[idx] >= frameIndex)
        {
            m_vecnHighIllumInfo[idx]++;
        }
    }

    if (isHighFrame)
    {
        enableHighFrame(frameIndex);
    }

    return true;
}

bool IllumInfo2D::eraseIllum(long frameIndex)
{
    if (frameIndex < 0 || frameIndex >= m_illuminations_ms.size())
        return false;

    m_illuminations_ms.erase(m_illuminations_ms.begin() + frameIndex);
    m_illuminations_name.erase(m_illuminations_name.begin() + frameIndex);

    for (int idx = 0; idx < long(m_vecnHighIllumInfo.size()); idx++)
    {
        if (m_vecnHighIllumInfo[idx] == frameIndex)
        {
            m_vecnHighIllumInfo.erase(m_vecnHighIllumInfo.begin() + idx);
            idx--;
        }
        else if (m_vecnHighIllumInfo[idx] > frameIndex)
        {
            m_vecnHighIllumInfo[idx]--;
        }
    }

    return true;
}

long IllumInfo2D::getHighFrameVectorIndex(long frameIndex) const
{
    for (long index = 0; index < long(m_vecnHighIllumInfo.size()); index++)
    {
        long highFrameId = m_vecnHighIllumInfo[index];

        if (highFrameId == frameIndex)
        {
            return index;
        }
    }

    return -1;
}
