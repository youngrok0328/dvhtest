//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SlitbeamDistortionData.h"

//CPP_2_________________________________ This project's headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/AlgorithmGpu/ImageProcessingGpu.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
#include <fstream>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SlitbeamDistortionData::SlitbeamDistortionData(long imageSizeY)
    : m_imageSizeY(imageSizeY)
    , m_distortionProfile(imageSizeY, 0.f)
    , m_useDistortionType(enumDistortionType::None)
    , m_zmapResizeTempImage(new Ipvm::Image32r)
{
    m_slitbeamDistortionFull.m_distortionImage = new Ipvm::Image32r;
    m_slitbeamDistortionFull.m_distortionBuffer = new Ipvm::Image32r;
    m_slitbeamDistortionFull.m_scanDepth_um = 0.;
}

SlitbeamDistortionData::~SlitbeamDistortionData()
{
    delete m_zmapResizeTempImage;
    delete m_slitbeamDistortionFull.m_distortionBuffer;
    delete m_slitbeamDistortionFull.m_distortionImage;
}

void SlitbeamDistortionData::ready(
    double scanDepth, bool cameraVerticalBinning, long imageSizeX, long imageSizeY, bool useDistortionCompensation)
{
    m_useDistortionType = enumDistortionType::None;
    m_zmapResizeTempImage->Create(imageSizeX, imageSizeY);

    if (useDistortionCompensation)
    {
        if (loadSlitbeamDistortionFull(scanDepth, cameraVerticalBinning, imageSizeX, imageSizeY))
        {
            m_useDistortionType = enumDistortionType::Full;
            DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("Ready distortion full"));
        }

        if (m_useDistortionType == enumDistortionType::None)
        {
            m_useDistortionType = enumDistortionType::Profile;
            DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("Ready distortion profile"));
        }
    }
}

bool SlitbeamDistortionData::reloadProfile()
{
    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ifstream infile(DynamicSystemPath::get(DefineFile::SlitbeamDistortionProfile));

    if (infile.is_open() == false)
    {
        return false;
    }

    char buffer[1000];

    std::vector<float> slitbeamDistortionProfile;

    while (infile.getline(buffer, 1000))
    {
        slitbeamDistortionProfile.push_back((float)::atof(buffer));
    }

    if (slitbeamDistortionProfile.size() == m_imageSizeY)
    {
        m_distortionProfile.Create(static_cast<int32_t>(slitbeamDistortionProfile.size()), 1);

        ::memcpy(m_distortionProfile.GetMem(), slitbeamDistortionProfile.data(),
            slitbeamDistortionProfile.size() * sizeof(float));

        return true;
    }

    return false;
}

void SlitbeamDistortionData::resetSelectDistortionFull()
{
    // 선택된 Distortion Full 정보를 클리어 해줘서 다시 로드 하게 하자
    // Case 1 : Distortion Full 정보를 다시 만들었을 때 다시 로드하고자 할 때 호출한다.

    m_slitbeamDistortionFull.m_distortionImage->Free();
    m_slitbeamDistortionFull.m_distortionBuffer->Free();
    m_slitbeamDistortionFull.m_scanDepth_um = 0.;
}

Ipvm::Image32r* SlitbeamDistortionData::getDistortionProfile()
{
    if (m_useDistortionType != enumDistortionType::Profile)
        return nullptr;

    return &m_distortionProfile;
}

void SlitbeamDistortionData::applyAndRotateCw90_zmap(const Ipvm::Image32r& source, Ipvm::Image32r& dest)
{
    Ipvm::Image32r* beforeZmap = &dest;

    if (m_useDistortionType == enumDistortionType::Full)
    {
        // Distortion Full을 사용할 때는 임시 버퍼에 결과를 넣었다가
        // Distortion 을 뺀 결과를 결과버퍼에 넣는다.

        beforeZmap = m_slitbeamDistortionFull.m_distortionBuffer;
    }

    if (source.GetSizeY() != dest.GetSizeX())
    {
        // Resize가 필요함
        Ipvm::ImageProcessingGpu::RotateCw90(source, *m_zmapResizeTempImage);
        Ipvm::ImageProcessing::ResizeNearestInterpolation(*m_zmapResizeTempImage, *beforeZmap);
    }
    else
    {
        Ipvm::ImageProcessingGpu::RotateCw90(source, *beforeZmap);
    }

    if (m_useDistortionType == enumDistortionType::Full)
    {
        DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("Apply distortion full"));
        Ipvm::ImageProcessing::Subtract(
            *beforeZmap, *m_slitbeamDistortionFull.m_distortionImage, Ipvm::Rect32s(dest), dest);
    }
}

bool SlitbeamDistortionData::loadSlitbeamDistortionFull(
    double scanDepth_um, bool cameraVerticalBinning, long imageSizeX, long imageSizeY)
{
    if (m_slitbeamDistortionFull.m_scanDepth_um == scanDepth_um)
    {
        // Scan Depth가 같다면 현재 로드된 Slitbeam Disotrtion을 사용하자
        return true;
    }

    CString distortionFilePath = DynamicSystemPath::getDistortionFullFilePath(
        scanDepth_um, cameraVerticalBinning, PersonalConfig::getInstance().isToUseNearestDistortionXEnabled());

    if (distortionFilePath.IsEmpty())
    {
        m_slitbeamDistortionFull.m_distortionImage->Free();
        m_slitbeamDistortionFull.m_distortionBuffer->Free();
        m_slitbeamDistortionFull.m_scanDepth_um = 0.;

        return false;
    }

    CFile file;

    if (!file.Open(distortionFilePath, CFile::modeRead))
    {
        m_slitbeamDistortionFull.m_distortionImage->Free();
        m_slitbeamDistortionFull.m_distortionBuffer->Free();
        m_slitbeamDistortionFull.m_scanDepth_um = 0.;

        return false;
    }

    long sizeX = 0;
    long sizeY = 0;

    file.Read(&sizeX, sizeof(sizeX));
    file.Read(&sizeY, sizeof(sizeY));

    if (sizeX != imageSizeX || sizeY != imageSizeY)
    {
        m_slitbeamDistortionFull.m_distortionImage->Free();
        m_slitbeamDistortionFull.m_distortionBuffer->Free();
        m_slitbeamDistortionFull.m_scanDepth_um = 0.;

        // Size가 안맞는디?
        file.Close();
        return false;
    }

    m_slitbeamDistortionFull.m_scanDepth_um = scanDepth_um;
    m_slitbeamDistortionFull.m_distortionImage->Create(sizeX, sizeY);
    m_slitbeamDistortionFull.m_distortionBuffer->Create(sizeX, sizeY);

    for (long y = 0; y < sizeY; y++)
    {
        auto* zmap_y = m_slitbeamDistortionFull.m_distortionImage->GetMem(0, y);
        file.Read(zmap_y, sizeX * sizeof(float));
    }

    file.Close();

    return true;
}
