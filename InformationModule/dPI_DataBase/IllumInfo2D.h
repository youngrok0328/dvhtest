#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <array>
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class DPI_DATABASE_API IllumInfo2D
{
public:
    IllumInfo2D();
    ~IllumInfo2D();

    BOOL LinkDataBase(BOOL bSave, long jobVerion, CiDataBase& jobDB);

    long getTotalFrameCount() const;
    long getHighFrameCount() const;
    bool needPositionMovement() const;

    bool isHighFrame(long frameIndex) const;
    long getHeighFrameIndex(long index) const;

    std::array<float, 16>& getIllum(long frameIndex);
    const std::array<float, 16>& getIllum(long frameIndex) const;
    LPCTSTR getIllumName(long frameIndex) const;
    CString getIllumFullName(long frameIndex) const;
    CString* getIllumNamePtr(long frameIndex);

    const float& getIllumChannel_ms(long frameIndex, long channelIndex) const;
    float& getIllumChannel_ms(long frameIndex, long channelIndex);

    void resetInfo();
    void resetHighFrame();
    void disableHighFrame(long frameIndex);
    bool enableHighFrame(long frameIndex);
    void setIllumName(long frameIndex, LPCTSTR name);
    bool insertFrame(long frameIndex, bool isHighFrame, LPCTSTR name);
    bool eraseIllum(long frameIndex);

    BOOL m_2D_colorFrame;
    long m_2D_colorFrameIndex_red;
    long m_2D_colorFrameIndex_green;
    long m_2D_colorFrameIndex_blue;

private:
    // 조명 프레임 별 / 채널 별 LED 점등 시간
    std::vector<std::array<float, 16>> m_illuminations_ms;
    std::vector<CString> m_illuminations_name;
    std::vector<long> m_vecnHighIllumInfo;

    long getHighFrameVectorIndex(long frameIndex) const;
};
