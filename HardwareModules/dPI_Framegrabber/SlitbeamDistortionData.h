#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class SlitbeamDistortionData
{
public:
    SlitbeamDistortionData(long imageSizeY);
    ~SlitbeamDistortionData();

    void ready(
        double scanDepth, bool cameraVerticalBinning, long imageSizeX, long imageSizeY, bool useDistortionCompensation);
    bool reloadProfile();
    void resetSelectDistortionFull();

    Ipvm::Image32r* getDistortionProfile();
    void applyAndRotateCw90_zmap(const Ipvm::Image32r& source, Ipvm::Image32r& dest);

private:
    enum class enumDistortionType
    {
        None,
        Profile,
        Full
    };

    const long m_imageSizeY;
    Ipvm::Image32r m_distortionProfile;
    enumDistortionType m_useDistortionType;

    struct SlitbeamDistortionFull
    {
        double m_scanDepth_um;
        Ipvm::Image32r* m_distortionImage;
        Ipvm::Image32r* m_distortionBuffer;
    };

    SlitbeamDistortionFull m_slitbeamDistortionFull;
    Ipvm::Image32r* m_zmapResizeTempImage;

    bool loadSlitbeamDistortionFull(double scanDepth_um, bool cameraVerticalBinning, long imageSizeX, long imageSizeY);
};
