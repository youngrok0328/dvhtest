//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionImageLot.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/AlgorithmGpu/ImageProcessingGpu.h>
#include <Ipvm/Base/ColorMap.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image16s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
#include <memory>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FILE_EXTENSION_TAG "IPIS_IMAGE_BITMAP_EXTENSION_{283A612E-CBD1-42CC-AA28-7C4F60857B01}"

//CPP_7_________________________________ Implementation body
//
struct ExtensionInfoHeader_V0
{
    // 이미지 버퍼 할당을 위한 기본 정보
    int m_sensorSizeX;
    int m_sensorSizeY;
    int m_imageSizeX;
    int m_imageSizeY;
    int m_stitchCountX;
    int m_stitchCountY;
    int m_frameCount;
    int m_zmapCount;
    int m_vmapCount;

    // 기타 이미지 획득 관련 정보
    int m_trayIndex;
    int m_runTimeIndex;
    int m_scanAreaIndex;
    float m_px2um_x;
    float m_px2um_y;
    float m_px2um_z;
};

struct ExtensionInfoHeader_V1
{
    // 이미지 버퍼 할당을 위한 기본 정보
    int m_sensorSizeX;
    int m_sensorSizeY;
    int m_imageSizeX;
    int m_imageSizeY;
    int m_stitchCountX;
    int m_stitchCountY;
    int m_frameCount;
    int m_zmapCount;
    int m_vmapCount;

    // 기타 이미지 획득 관련 정보
    int m_trayIndex;
    int m_runTimeIndex;
    int m_scanAreaIndex;
    float m_px2um_x;
    float m_px2um_y;
    float m_px2um_z;
    float m_heightRangeMin;
    float m_heightRangeMax;
};

struct ExtensionInfoHeader_V2
{
    // 이미지 버퍼 할당을 위한 기본 정보
    int m_sensorSizeX;
    int m_sensorSizeY;
    int m_imageSizeX;
    int m_imageSizeY;
    int m_stitchCountX;
    int m_stitchCountY;
    int m_frameCount;
    int m_zmapCount;
    int m_vmapCount;

    // 기타 이미지 획득 관련 정보
    int m_trayIndex;
    int m_runTimeIndex;
    int m_scanAreaIndex;
    float m_px2um_x;
    float m_px2um_y;
    float m_px2um_z;
    float m_heightRangeMin;
    float m_heightRangeMax;

    // Side image 정보
    int m_nSideModuleIndex; //Default Front 0, Rear 1
};

struct ExtensionInfoHeader_V3
{
    // 이미지 버퍼 할당을 위한 기본 정보
    int m_sensorSizeX;
    int m_sensorSizeY;
    int m_imageSizeX;
    int m_imageSizeY;
    int m_stitchCountX;
    int m_stitchCountY;
    int m_frameCount;
    int m_zmapCount;
    int m_vmapCount;

    // 기타 이미지 획득 관련 정보
    int m_trayIndex;
    int m_runTimeIndex;
    int m_scanAreaIndex;
    float m_px2um_x;
    float m_px2um_y;
    float m_px2um_z;
    float m_heightRangeMin;
    float m_heightRangeMax;

    // Side image 정보
    int m_nSideModuleIndex; //Default Front 0, Rear 1

    //TR Pocket 정보
    bool m_fillPocket; // TR Pocket Fill 여부
};

VisionImageLotInfo::VisionImageLotInfo()
    : m_sensorSizeX(0)
    , m_sensorSizeY(0)
    , m_imageSizeX(0)
    , m_imageSizeY(0)
    , m_frameCount(0)
    , m_trayIndex(0)
    , m_runTimeIndex(0)
    , m_scanAreaIndex(0)
    , m_heightRangeMin(-1000.f)
    , m_heightRangeMax(1000.f)
    , m_nSideModuleIndex(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    , m_fillPocket(false)
{
}

VisionImageLotInfo::~VisionImageLotInfo()
{
}

bool VisionImageLotInfo::LoadInfo(LPCTSTR pathName, bool popupMessage)
{
    CFile file;

    if (!file.Open(pathName, CFile::modeRead | CFile::shareDenyWrite))
    {
        if (popupMessage)
        {
            ::AfxMessageBox(_T("Cannot open file!"), MB_OK | MB_ICONERROR);
        }

        return false;
    }

    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    RGBQUAD palette[256];

    auto fileSize = file.GetLength();

    if (fileSize <= sizeof(bmfh) + sizeof(bmih) + sizeof(palette))
    {
        if (popupMessage)
        {
            ::AfxMessageBox(_T("File size is too small!"), MB_OK | MB_ICONERROR);
        }
        return false;
    }

    file.Read(&bmfh, sizeof(bmfh));
    file.Read(&bmih, sizeof(bmih));
    file.Read(&palette, sizeof(palette));

    if (bmfh.bfType != 0x4d42)
    {
        if (popupMessage)
        {
            ::AfxMessageBox(_T("Cannot find bitmap info!"), MB_OK | MB_ICONERROR);
        }
        return false;
    }

    file.Seek(bmih.biSizeImage, CFile::current);

    char fileTag[] = FILE_EXTENSION_TAG;

    if (fileSize - file.GetPosition() < sizeof(fileTag))
    {
        if (popupMessage)
        {
            ::AfxMessageBox(_T("No extension tag!"), MB_OK | MB_ICONERROR);
        }
        return false;
    }

    file.Read(&fileTag, sizeof(fileTag));

    if (::strcmp(fileTag, FILE_EXTENSION_TAG))
    {
        if (popupMessage)
        {
            ::AfxMessageBox(_T("Extension tag mismatch!"), MB_OK | MB_ICONERROR);
        }
        return false;
    }

    int fileVersion = 1;

    if (fileSize - file.GetPosition() < sizeof(fileVersion))
    {
        if (popupMessage)
        {
            ::AfxMessageBox(_T("No extension version number!"), MB_OK | MB_ICONERROR);
        }
        return false;
    }

    file.Read(&fileVersion, sizeof(fileVersion));

    if (fileVersion == 0)
    {
        ExtensionInfoHeader_V0 eih;

        if (fileSize - file.GetPosition() < sizeof(eih))
        {
            if (popupMessage)
            {
                ::AfxMessageBox(_T("No extension information!"), MB_OK | MB_ICONERROR);
            }
            return false;
        }

        file.Read(&eih, sizeof(eih));

        // 이미지 버퍼 할당을 위한 기본 정보
        this->m_sensorSizeX = eih.m_sensorSizeX;
        this->m_sensorSizeY = eih.m_sensorSizeY;
        this->m_imageSizeX = eih.m_imageSizeX;
        this->m_imageSizeY = eih.m_imageSizeY;
        this->m_frameCount = eih.m_frameCount;
        this->m_zmapCount = eih.m_zmapCount;
        this->m_vmapCount = eih.m_vmapCount;
        this->m_trayIndex = eih.m_trayIndex;
        this->m_runTimeIndex = eih.m_runTimeIndex;
        this->m_scanAreaIndex = eih.m_scanAreaIndex;
        this->m_heightRangeMin = -1000.f;
        this->m_heightRangeMax = 1000.f;
        this->m_scale.setScale(eih.m_px2um_x, eih.m_px2um_y, eih.m_px2um_z);

        ULONGLONG extensionImageTotalBytes = 0;

        extensionImageTotalBytes += (ULONGLONG)(this->m_frameCount - 1)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 8, 4) * this->m_imageSizeY;
        extensionImageTotalBytes += (ULONGLONG)(this->m_zmapCount)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 16, 4) * this->m_imageSizeY;
        extensionImageTotalBytes += (ULONGLONG)(this->m_vmapCount)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 16, 4) * this->m_imageSizeY;

        if (fileSize - file.GetPosition() < extensionImageTotalBytes)
        {
            if (popupMessage)
            {
                ::AfxMessageBox(_T("Extension image contents corrupted!"), MB_OK | MB_ICONERROR);
            }
            return false;
        }
    }
    else if (fileVersion == 1)
    {
        ExtensionInfoHeader_V1 eih;

        if (fileSize - file.GetPosition() < sizeof(eih))
        {
            if (popupMessage)
            {
                ::AfxMessageBox(_T("No extension information!"), MB_OK | MB_ICONERROR);
            }
            return false;
        }

        file.Read(&eih, sizeof(eih));

        // 이미지 버퍼 할당을 위한 기본 정보
        this->m_sensorSizeX = eih.m_sensorSizeX;
        this->m_sensorSizeY = eih.m_sensorSizeY;
        this->m_imageSizeX = eih.m_imageSizeX;
        this->m_imageSizeY = eih.m_imageSizeY;
        this->m_frameCount = eih.m_frameCount;
        this->m_zmapCount = eih.m_zmapCount;
        this->m_vmapCount = eih.m_vmapCount;
        this->m_trayIndex = eih.m_trayIndex;
        this->m_runTimeIndex = eih.m_runTimeIndex;
        this->m_scanAreaIndex = eih.m_scanAreaIndex;
        this->m_heightRangeMin = eih.m_heightRangeMin;
        this->m_heightRangeMax = eih.m_heightRangeMax;
        this->m_scale.setScale(eih.m_px2um_x, eih.m_px2um_y, eih.m_px2um_z);

        ULONGLONG extensionImageTotalBytes = 0;

        extensionImageTotalBytes += (ULONGLONG)(this->m_frameCount - 1)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 8, 4) * this->m_imageSizeY;
        extensionImageTotalBytes += (ULONGLONG)(this->m_zmapCount)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 16, 4) * this->m_imageSizeY;
        extensionImageTotalBytes += (ULONGLONG)(this->m_vmapCount)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 16, 4) * this->m_imageSizeY;

        if (fileSize - file.GetPosition() < extensionImageTotalBytes)
        {
            if (popupMessage)
            {
                ::AfxMessageBox(_T("Extension image contents corrupted!"), MB_OK | MB_ICONERROR);
            }
            return false;
        }
    }
    else if (fileVersion == 2)
    {
        ExtensionInfoHeader_V2 eih;

        if (fileSize - file.GetPosition() < sizeof(eih))
        {
            if (popupMessage)
            {
                ::AfxMessageBox(_T("No extension information!"), MB_OK | MB_ICONERROR);
            }
            return false;
        }

        file.Read(&eih, sizeof(eih));

        // 이미지 버퍼 할당을 위한 기본 정보
        this->m_sensorSizeX = eih.m_sensorSizeX;
        this->m_sensorSizeY = eih.m_sensorSizeY;
        this->m_imageSizeX = eih.m_imageSizeX;
        this->m_imageSizeY = eih.m_imageSizeY;
        this->m_frameCount = eih.m_frameCount;
        this->m_zmapCount = eih.m_zmapCount;
        this->m_vmapCount = eih.m_vmapCount;
        this->m_trayIndex = eih.m_trayIndex;
        this->m_runTimeIndex = eih.m_runTimeIndex;
        this->m_scanAreaIndex = eih.m_scanAreaIndex;
        this->m_heightRangeMin = eih.m_heightRangeMin;
        this->m_heightRangeMax = eih.m_heightRangeMax;
        this->m_scale.setScale(eih.m_px2um_x, eih.m_px2um_y, eih.m_px2um_z);
        this->m_nSideModuleIndex = eih.m_nSideModuleIndex;

        ULONGLONG extensionImageTotalBytes = 0;

        extensionImageTotalBytes += (ULONGLONG)(this->m_frameCount - 1)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 8, 4) * this->m_imageSizeY;
        extensionImageTotalBytes += (ULONGLONG)(this->m_zmapCount)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 16, 4) * this->m_imageSizeY;
        extensionImageTotalBytes += (ULONGLONG)(this->m_vmapCount)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 16, 4) * this->m_imageSizeY;

        if (fileSize - file.GetPosition() < extensionImageTotalBytes)
        {
            if (popupMessage)
            {
                ::AfxMessageBox(_T("Extension image contents corrupted!"), MB_OK | MB_ICONERROR);
            }
            return false;
        }
    }
    else if (fileVersion == 3)
    {
        ExtensionInfoHeader_V3 eih;

        if (fileSize - file.GetPosition() < sizeof(eih))
        {
            if (popupMessage)
            {
                ::AfxMessageBox(_T("No extension information!"), MB_OK | MB_ICONERROR);
            }
            return false;
        }

        file.Read(&eih, sizeof(eih));

        // 이미지 버퍼 할당을 위한 기본 정보
        this->m_sensorSizeX = eih.m_sensorSizeX;
        this->m_sensorSizeY = eih.m_sensorSizeY;
        this->m_imageSizeX = eih.m_imageSizeX;
        this->m_imageSizeY = eih.m_imageSizeY;
        this->m_frameCount = eih.m_frameCount;
        this->m_zmapCount = eih.m_zmapCount;
        this->m_vmapCount = eih.m_vmapCount;
        this->m_trayIndex = eih.m_trayIndex;
        this->m_runTimeIndex = eih.m_runTimeIndex;
        this->m_scanAreaIndex = eih.m_scanAreaIndex;
        this->m_heightRangeMin = eih.m_heightRangeMin;
        this->m_heightRangeMax = eih.m_heightRangeMax;
        this->m_scale.setScale(eih.m_px2um_x, eih.m_px2um_y, eih.m_px2um_z);
        this->m_nSideModuleIndex = eih.m_nSideModuleIndex;
        this->m_fillPocket = eih.m_fillPocket;

        ULONGLONG extensionImageTotalBytes = 0;

        extensionImageTotalBytes += (ULONGLONG)(this->m_frameCount - 1)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 8, 4) * this->m_imageSizeY;
        extensionImageTotalBytes += (ULONGLONG)(this->m_zmapCount)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 16, 4) * this->m_imageSizeY;
        extensionImageTotalBytes += (ULONGLONG)(this->m_vmapCount)
            * Ipvm::Image::CalcWidthBytes(this->m_imageSizeX, 16, 4) * this->m_imageSizeY;

        if (fileSize - file.GetPosition() < extensionImageTotalBytes)
        {
            if (popupMessage)
            {
                ::AfxMessageBox(_T("Extension image contents corrupted!"), MB_OK | MB_ICONERROR);
            }
            return false;
        }
    }
    else
    {
        if (popupMessage)
        {
            ::AfxMessageBox(_T("Extension version number is invalid!"), MB_OK | MB_ICONERROR);
        }
        return false;
    }

    int sampling = PersonalConfig::getInstance().getImageSampling();

    if (sampling != 1)
    {
        this->m_sensorSizeX /= sampling;
        this->m_sensorSizeY /= sampling;
        this->m_imageSizeX /= sampling;
        this->m_imageSizeY /= sampling;

        this->m_scale.setScale(this->m_scale.pixelToUm().m_x * sampling, this->m_scale.pixelToUm().m_y * sampling,
            this->m_scale.pixelToUmZ());
    }

    return true;
}

//===========================================================================================

VisionImageLot::VisionImageLot()
    : m_inlineRawSaveCheck(false)
    , m_nInspectionRepeatIndex(0)
    , m_nSideScanID(0)
{
    InitializeCriticalSection(&m_csInlineRawSaveCheck);

    // 기본 Scale은 System 설정으로 하자
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
    {
        for (int nSideModuleidx = enSideVisionModule::SIDE_VISIONMODULE_START;
            nSideModuleidx < enSideVisionModule::SIDE_VISIONMODULE_END; nSideModuleidx++)
        {
            SetScales(SystemConfig::GetInstance().GetScale(enSideVisionModule(nSideModuleidx)),
                enSideVisionModule(nSideModuleidx));
        }
    }
    else if (SystemConfig::GetInstance().IsDual3DGrabMode())
    {
        for (int nDualModuleidx = enDualGrabImageType::DUAL_IMAGE_START;
            nDualModuleidx < enDualGrabImageType::DUAL_IMAGE_END; nDualModuleidx++)
        {
            SetScales(SystemConfig::GetInstance().GetScale(enSideVisionModule(nDualModuleidx)),
                enSideVisionModule(nDualModuleidx));
        }
    }
    else
        SetScales(SystemConfig::GetInstance().GetScale(enSideVisionModule::SIDE_VISIONMODULE_FRONT));
}

VisionImageLot::~VisionImageLot()
{
    DeleteCriticalSection(&m_csInlineRawSaveCheck);
}

Ipvm::Size32s2 VisionImageLot::Get2DMaxBufferSize(
    const int sensorSizeX, const int sensorSizeY, const int stitchCountX, const int stitchCountY)
{
    return Ipvm::Size32s2(sensorSizeX * stitchCountX, sensorSizeY * stitchCountY);
}

Ipvm::Size32s2 VisionImageLot::Get3DMaxBufferSize(const int sensorSizeX, const int stitchCountY, const int scanLength)
{
    return Ipvm::Size32s2(scanLength, sensorSizeX * stitchCountY);
}

void VisionImageLot::Setup2D(const int sensorSizeX, const int sensorSizeY, const Ipvm::Size32s2& bufferSize,
    const int frameCount, const int i_index)
{
    m_info[i_index].m_sensorSizeX = sensorSizeX;
    m_info[i_index].m_sensorSizeY = sensorSizeY;
    m_info[i_index].m_imageSizeX = bufferSize.m_w;
    m_info[i_index].m_imageSizeY = bufferSize.m_h;
    m_info[i_index].m_frameCount = frameCount;
    m_info[i_index].m_zmapCount = 0;
    m_info[i_index].m_vmapCount = 0;
    m_info[i_index].m_nSideModuleIndex = (int)i_index;

    AllocateImageFromInfo(i_index);
    SetRealImageSize(m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);
}

void VisionImageLot::Setup3D(
    const int sensorSizeX, const int sensorSizeY, const Ipvm::Size32s2& bufferSize, const int i_index)
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("VisionImageLot::Setup3D - PTR(%p) S(%d:%d)"), this,
        bufferSize.m_w, bufferSize.m_h);

    int oldBufferSizeX = m_buffer[i_index].m_vmapImage.GetSizeX();
    int oldBufferSizeY = m_buffer[i_index].m_vmapImage.GetSizeY();

    m_info[i_index].m_sensorSizeX = sensorSizeX;
    m_info[i_index].m_sensorSizeY = sensorSizeY;
    m_info[i_index].m_frameCount = 2;
    m_info[i_index].m_zmapCount = 1;
    m_info[i_index].m_vmapCount = 1;
    m_info[i_index].m_nSideModuleIndex = enSideVisionModule::SIDE_VISIONMODULE_FRONT;

    if (oldBufferSizeX != bufferSize.m_w || oldBufferSizeY != bufferSize.m_h)
    {
        // Buffer Size가 달라졌다면 Real Buffer는 일단 Buffer Size 크기로 새로 설정해 주자
        m_info[i_index].m_imageSizeX = bufferSize.m_w;
        m_info[i_index].m_imageSizeY = bufferSize.m_h;

        AllocateImageFromInfo(i_index);
        SetRealImageSize(m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY, i_index);
    }
}

void VisionImageLot::SetupColorVision(const int sensorSizeX, const int sensorSizeY, const Ipvm::Size32s2& bufferSize,
    const int frameCount, const int i_index)
{
    m_info[i_index].m_sensorSizeX = sensorSizeX;
    m_info[i_index].m_sensorSizeY = sensorSizeY;
    m_info[i_index].m_imageSizeX = bufferSize.m_w;
    m_info[i_index].m_imageSizeY = bufferSize.m_h;
    m_info[i_index].m_frameCount = frameCount;
    m_info[i_index].m_zmapCount = 0;
    m_info[i_index].m_vmapCount = 0;
    //m_info[i_index].m_nSideModuleIndex = (int)i_eSideVisionModule;

    AllocateImageFromInfo(i_index);
    SetRealImageSize(m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);
}

bool VisionImageLot::SetRealImageSize(const int imageSizeX, const int imageSizeY, const int i_index)
{
    m_info[i_index].m_imageSizeX = imageSizeX;
    m_info[i_index].m_imageSizeY = imageSizeY;

    int nVisionType = SystemConfig::GetInstance().GetVisionType();

    switch (nVisionType)
    {
        case VISIONTYPE_2D_INSP:
        case VISIONTYPE_3D_INSP:
        case VISIONTYPE_TR:
        case VISIONTYPE_SWIR_INSP:
            m_realBuffer[i_index].m_vecImages.resize(m_info[i_index].m_frameCount);

            for (int index = 0; index < m_info[i_index].m_frameCount; index++)
            {
                auto& srcImage = m_buffer[i_index].m_vecImages[index];
                if (srcImage.GetSizeX() < imageSizeX)
                    return false;
                if (srcImage.GetSizeY() < imageSizeY)
                    return false;

                m_realBuffer[i_index].m_vecImages[index] = Ipvm::Image8u(
                    imageSizeX, imageSizeY, srcImage.GetMem(), Ipvm::Image::CalcWidthBytes(imageSizeX, 8, 4));
            }
            break;

        case VISIONTYPE_NGRV_INSP:
            m_realBuffer[i_index].m_vecImages.resize(m_info[i_index].m_frameCount);
            m_realBuffer[i_index].m_vecColorImages.resize(m_info[i_index].m_frameCount);

            for (int index = 0; index < m_info[i_index].m_frameCount; index++)
            {
                auto& srcImageMono = m_buffer[i_index].m_vecImages[index];
                auto& srcImageColor = m_buffer[i_index].m_vecColorImages[index];
                if (srcImageMono.GetSizeX() < imageSizeX || srcImageColor.GetSizeX() < imageSizeX)
                    return false;
                if (srcImageMono.GetSizeY() < imageSizeY || srcImageColor.GetSizeX() < imageSizeX)
                    return false;

                m_realBuffer[i_index].m_vecImages[index] = Ipvm::Image8u(
                    imageSizeX, imageSizeY, srcImageMono.GetMem(), Ipvm::Image::CalcWidthBytes(imageSizeX, 8, 4));

                m_realBuffer[i_index].m_vecColorImages[index] = Ipvm::Image8u3(
                    imageSizeX, imageSizeY, srcImageColor.GetMem(), Ipvm::Image::CalcWidthBytes(imageSizeX, 8, 4));
            }
            break;
        case VISIONTYPE_SIDE_INSP:

            for (int nimageidx = SIDE_VISIONMODULE_START; nimageidx < SIDE_VISIONMODULE_END; nimageidx++)
            {
                m_realBuffer[nimageidx].m_vecImages.resize(m_info[nimageidx].m_frameCount);

                for (int index = 0; index < m_info[nimageidx].m_frameCount; index++)
                {
                    auto& srcImage = m_buffer[nimageidx].m_vecImages[index];
                    if (srcImage.GetSizeX() < imageSizeX)
                        return false;
                    if (srcImage.GetSizeY() < imageSizeY)
                        return false;

                    m_realBuffer[nimageidx].m_vecImages[index] = Ipvm::Image8u(
                        imageSizeX, imageSizeY, srcImage.GetMem(), Ipvm::Image::CalcWidthBytes(imageSizeX, 8, 4));
                }
            }

            break;
    }

    if (m_buffer[i_index].m_zmapImage.GetSizeX() > 0)
    {
        if (m_buffer[i_index].m_zmapImage.GetSizeX() < imageSizeX)
            return false;
        if (m_buffer[i_index].m_zmapImage.GetSizeY() < imageSizeY)
            return false;

        // 3D Buffer 가 있는 경우에만 할당한다
        m_realBuffer[i_index].m_zmapImage = Ipvm::Image32r(imageSizeX, imageSizeY,
            m_buffer[i_index].m_zmapImage.GetMem(), Ipvm::Image::CalcWidthBytes(imageSizeX, 32, 4));

        m_realBuffer[i_index].m_vmapImage = Ipvm::Image16u(imageSizeX, imageSizeY,
            m_buffer[i_index].m_vmapImage.GetMem(), Ipvm::Image::CalcWidthBytes(imageSizeX, 16, 4));
    }

    return true;
}

void VisionImageLot::Paste2D(const int stitchGrabIndex, BYTE** ppbyImage, const int imageCount, const int stitchCountX,
    const int stitchCountY, const enSideVisionModule i_eSideVisionModule)
{
    const int sensorSizeX = GetSensorSizeX(i_eSideVisionModule);
    const int sensorSizeY = GetSensorSizeY(i_eSideVisionModule);

    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    const bool bIsSideFrontModule = (i_eSideVisionModule == enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    if (stitchGrabIndex >= stitchCountX * stitchCountY)
    {
        return;
    }

    //{{//kircheis_SideStitching
    //const int stitchIndexX = stitchGrabIndex % stitchCountX;//원래의 stitchIndexX 계산식
    const int recievedStitchIndexX = stitchGrabIndex
        % stitchCountX; //순방향 계산. 수식이 두가지 인데 역순 계산식이 이 계산의 결과를 참조하므로 순방향과 역방향 모두 계산 후 삼항 연산 했다.
    const int reversedStitchIndexX = stitchCountX - 1 - recievedStitchIndexX; //역방향 계산
    //const int stitchIndexX = (bIsSideVision && bIsSideFrontModule) ? reversedStitchIndexX : recievedStitchIndexX; //Side Vision이면서 Front Camera면 역순으로 영상 붙임.
    const int stitchIndexX = (bIsSideVision == false || (bIsSideVision && bIsSideFrontModule))
        ? recievedStitchIndexX
        : reversedStitchIndexX; //StitchIndex가 역순입력일 경우 Side Vision이면서 Rear Camera면 역순으로 영상 붙임.
    //}}
    const int stitchIndexY = stitchGrabIndex / stitchCountX;

    const Ipvm::Rect32s roi(int32_t(stitchIndexX * sensorSizeX), int32_t(stitchIndexY * sensorSizeY),
        int32_t((stitchIndexX + 1) * sensorSizeX), int32_t((stitchIndexY + 1) * sensorSizeY));

    for (int frame = 0; frame < min(imageCount, m_realBuffer[i_eSideVisionModule].m_vecImages.size()); frame++)
    {
        Ipvm::Image8u grabImage(sensorSizeX, sensorSizeY, ppbyImage[frame], sensorSizeX);

        Ipvm::ImageProcessing::Copy(
            grabImage, Ipvm::Rect32s(grabImage), roi, m_realBuffer[i_eSideVisionModule].m_vecImages[frame]);
    }
}

//void VisionImageLot::Paste3D(const int stitchGrabIndex, const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap,
//    const enSideVisionModule i_eSideVisionModule)
//{
//    if (m_realBuffer[i_eSideVisionModule].m_vecImages.size() < 2)
//    {
//        ASSERT(!_T("??"));
//        return;
//    }
//
//    Ipvm::Rect32s pastRoi(0, m_info[i_eSideVisionModule].m_sensorSizeX * stitchGrabIndex,
//        m_info[i_eSideVisionModule].m_imageSizeX, m_info[i_eSideVisionModule].m_sensorSizeX * (stitchGrabIndex + 1));
//
//    Ipvm::Rect32s srcRoi(zmap);
//
//    Ipvm::Image32r dstZmap(m_realBuffer[i_eSideVisionModule].m_zmapImage, pastRoi);
//    Ipvm::Image16u dstVmap(m_realBuffer[i_eSideVisionModule].m_vmapImage, pastRoi);
//
//    Ipvm::ImageProcessing::Copy(zmap, srcRoi, dstZmap);
//    Ipvm::ImageProcessing::Copy(vmap, srcRoi, dstVmap);
//
//    UpdateGray3D(stitchGrabIndex);
//}

void VisionImageLot::Paste3D(const int stitchGrabIndex, const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap, const int i_index)
{
    if (m_realBuffer[i_index].m_vecImages.size() < 2)
    {
        ASSERT(!_T("??"));
        return;
    }

    Ipvm::Rect32s pastRoi(0, m_info[i_index].m_sensorSizeX * stitchGrabIndex, m_info[i_index].m_imageSizeX,
        m_info[i_index].m_sensorSizeX * (stitchGrabIndex + 1));

    Ipvm::Rect32s srcRoi(zmap);

    Ipvm::Image32r dstZmap(m_realBuffer[i_index].m_zmapImage, pastRoi);
    Ipvm::Image16u dstVmap(m_realBuffer[i_index].m_vmapImage, pastRoi);

    Ipvm::ImageProcessing::Copy(zmap, srcRoi, dstZmap);
    Ipvm::ImageProcessing::Copy(vmap, srcRoi, dstVmap);

    UpdateGray3D(stitchGrabIndex);
}

void VisionImageLot::PasteNGRV(const int stitchGrabIndex, BYTE** ppbyImage, const int imageCount,
    const int stitchCountX, const int stitchCountY, const int i_nReverseFrame,
    const enSideVisionModule i_eSideVisionModule)
{
    std::vector<Ipvm::Image8u3> vecGrabImageColor;
    vecGrabImageColor.resize(imageCount);

    const int sensorSizeX = GetSensorSizeX();
    const int sensorSizeY = GetSensorSizeY();

    if (stitchGrabIndex >= stitchCountX * stitchCountY)
    {
        return;
    }

    const int stitchIndexX = stitchGrabIndex % stitchCountX;
    const int stitchIndexY = stitchGrabIndex / stitchCountX;

    // RGB Gain 변수
    float fGainR;
    float fGainG;
    float fGainB;
    //

    const Ipvm::Rect32s roi(int32_t(stitchIndexX * sensorSizeX), int32_t(stitchIndexY * sensorSizeY),
        int32_t((stitchIndexX + 1) * sensorSizeX), int32_t((stitchIndexY + 1) * sensorSizeY));

    for (int nframe = 0; nframe < min(imageCount, m_realBuffer[i_eSideVisionModule].m_vecColorImages.size()); nframe++)
    {
        Ipvm::Image8u grabImageMono(
            sensorSizeX, sensorSizeY, ppbyImage[nframe], sensorSizeX); // Grabbed Image change BYTE -> 8u_C1

        vecGrabImageColor[nframe].Create(
            grabImageMono.GetSizeX(), grabImageMono.GetSizeY()); //	Create color image buffer
        vecGrabImageColor[nframe].FillZero();

        if (nframe == i_nReverseFrame)
        {
            fGainR = SystemConfig::GetInstance().m_fReverse_Channel_GainR;
            fGainG = SystemConfig::GetInstance().m_fReverse_Channel_GainG;
            fGainB = SystemConfig::GetInstance().m_fReverse_Channel_GainB;
        }
        else
        {
            fGainR = SystemConfig::GetInstance().m_fNormal_Channel_GainR;
            fGainG = SystemConfig::GetInstance().m_fNormal_Channel_GainG;
            fGainB = SystemConfig::GetInstance().m_fNormal_Channel_GainB;
        }

        // Bayer image color conversion
        if (Ipvm::ImageProcessingGpu::DemosaicBilinear(grabImageMono, Ipvm::Rect32s(grabImageMono),
                Ipvm::BayerPattern::e_gbrg, fGainR, fGainG, fGainB, vecGrabImageColor[nframe])
            != Ipvm::Status::e_ok)
        {
            vecGrabImageColor[nframe].FillZero();
        }

        m_realBuffer[i_eSideVisionModule].m_vecImages[nframe] = grabImageMono;
        m_realBuffer[i_eSideVisionModule].m_vecColorImages[nframe] = vecGrabImageColor[nframe];
    }
}

//void VisionImageLot::UpdateGray3D(const int stitchGrabIndex, const enSideVisionModule i_eSideVisionModule)
//{
//    if (m_realBuffer[i_eSideVisionModule].m_vecImages.size() < 2)
//    {
//        ASSERT(!_T("??"));
//        return;
//    }
//
//    Ipvm::Rect32s pastRoi(0, m_info[i_eSideVisionModule].m_sensorSizeX * stitchGrabIndex,
//        m_info[i_eSideVisionModule].m_imageSizeX, m_info[i_eSideVisionModule].m_sensorSizeX * (stitchGrabIndex + 1));
//
//    Ipvm::Image8u dstZmapGray(m_realBuffer[i_eSideVisionModule].m_vecImages[0], pastRoi);
//    Ipvm::Image8u dstVmapGray(m_realBuffer[i_eSideVisionModule].m_vecImages[1], pastRoi);
//
//    Ipvm::ImageProcessing::ScaleInRangeWithNoise(Ipvm::Image32r(m_realBuffer[i_eSideVisionModule].m_zmapImage, pastRoi),
//        Ipvm::Rect32s(dstZmapGray), Ipvm::k_noiseValue32r, m_info[i_eSideVisionModule].m_heightRangeMin,
//        m_info[i_eSideVisionModule].m_heightRangeMax, 0, 1, 255, dstZmapGray);
//
//    Ipvm::ImageProcessing::Scale(Ipvm::Image16u(m_realBuffer[i_eSideVisionModule].m_vmapImage, pastRoi),
//        Ipvm::Rect32s(dstVmapGray), dstVmapGray);
//}

void VisionImageLot::UpdateGray3D(const int stitchGrabIndex, const int i_index)
{
    if (m_realBuffer[i_index].m_vecImages.size() < 2)
    {
        ASSERT(!_T("??"));
        return;
    }

    Ipvm::Rect32s pastRoi(0, m_info[i_index].m_sensorSizeX * stitchGrabIndex, m_info[i_index].m_imageSizeX,
        m_info[i_index].m_sensorSizeX * (stitchGrabIndex + 1));

    Ipvm::Image8u dstZmapGray(m_realBuffer[i_index].m_vecImages[0], pastRoi);
    Ipvm::Image8u dstVmapGray(m_realBuffer[i_index].m_vecImages[1], pastRoi);

    Ipvm::ImageProcessing::ScaleInRangeWithNoise(Ipvm::Image32r(m_realBuffer[i_index].m_zmapImage, pastRoi),
        Ipvm::Rect32s(dstZmapGray), Ipvm::k_noiseValue32r, m_info[i_index].m_heightRangeMin,
        m_info[i_index].m_heightRangeMax, 0, 1, 255, dstZmapGray);

    Ipvm::ImageProcessing::Scale(
        Ipvm::Image16u(m_realBuffer[i_index].m_vmapImage, pastRoi),
        Ipvm::Rect32s(dstVmapGray), dstVmapGray);
}

Ipvm::Rect32s VisionImageLot::GetImageRect(const int i_index) const
{
    return Ipvm::Rect32s(0, 0, m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);
}

int VisionImageLot::GetImageFrameCount(const int i_index) const
{
    int nVisiontype = SystemConfig::GetInstance().GetVisionType();

    switch (nVisiontype)
    {
        case VISIONTYPE_2D_INSP:
        case VISIONTYPE_SIDE_INSP:
        case VISIONTYPE_TR:
        case VISIONTYPE_SWIR_INSP:
            return (int)m_realBuffer[i_index].m_vecImages.size();
            break;
        case VISIONTYPE_NGRV_INSP:
            return (int)m_realBuffer[i_index].m_vecColorImages.size();
            break;
        case VISIONTYPE_3D_INSP:
            return (int)m_realBuffer[i_index].m_vecImages.size();
            break;
    }

    return 0;
}

void VisionImageLot::SetSideScanID(const int i_nSideScanID)
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision == false)
        return;

    m_nSideScanID = i_nSideScanID;
}
int VisionImageLot::GetSideScanID()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision == false)
        return 0;

    return m_nSideScanID;
}
const Ipvm::Image8u& VisionImageLot::GetImageFrame(int frameIndex, const int i_index) const
{
    return m_realBuffer[i_index].m_vecImages[frameIndex];
}

const Ipvm::Image32r& VisionImageLot::GetZmapImage(const int i_index) const
{
    return m_realBuffer[i_index].m_zmapImage;
}

const Ipvm::Image16u& VisionImageLot::GetVmapImage(const int i_index) const
{
    return m_realBuffer[i_index].m_vmapImage;
}

Ipvm::Image8u& VisionImageLot::GetImageFrame(int frameIndex, const int i_index)
{
    return m_realBuffer[i_index].m_vecImages[frameIndex];
}

Ipvm::Image32r& VisionImageLot::GetZmapImage(const int i_index)
{
    return m_realBuffer[i_index].m_zmapImage;
}

Ipvm::Image16u& VisionImageLot::GetVmapImage(const int i_index)
{
    return m_realBuffer[i_index].m_vmapImage;
}

const Ipvm::Image8u3& VisionImageLot::GetColorImageFrame(int frameIndex, const int i_index) const
{
    return m_realBuffer[i_index].m_vecColorImages[frameIndex];
}

Ipvm::Image8u3& VisionImageLot::GetColorImageFrame(int frameIndex, const int i_index)
{
    return m_realBuffer[i_index].m_vecColorImages[frameIndex];
}

int VisionImageLot::GetSensorSizeX(const int i_index) const
{
    return m_info[i_index].m_sensorSizeX;
}

int VisionImageLot::GetSensorSizeY(const int i_index) const
{
    return m_info[i_index].m_sensorSizeY;
}

int VisionImageLot::GetImageSizeX(const int i_index) const
{
    return m_info[i_index].m_imageSizeX;
}

int VisionImageLot::GetImageSizeY(const int i_index) const
{
    return m_info[i_index].m_imageSizeY;
}

const VisionImageLotInfo& VisionImageLot::GetInfo(const int i_index) const
{
    return m_info[i_index];
}

const VisionScale& VisionImageLot::GetScale(const int i_index) const
{
    return m_info[i_index].m_scale;
}

bool VisionImageLot::CopyFrom(const VisionImageLot& object, const int i_index)
{
    m_info[i_index] = object.m_info[i_index];

    // 버퍼 할당
    m_buffer[i_index].m_vecImages.resize(object.m_info[i_index].m_frameCount);
    m_realBuffer[i_index].m_vecImages.resize(object.m_info[i_index].m_frameCount);

    for (int index = 0; index < int(object.m_buffer[i_index].m_vecImages.size()); index++)
    {
        auto& src = object.m_buffer[i_index].m_vecImages[index];
        m_buffer[i_index].m_vecImages[index].Create(src.GetSizeX(), src.GetSizeY());
    }

    if (true)
    {
        auto& src = object.m_buffer[i_index].m_zmapImage;
        m_buffer[i_index].m_zmapImage.Create(src.GetSizeX(), src.GetSizeY());
    }

    if (true)
    {
        auto& src = object.m_buffer[i_index].m_vmapImage;
        m_buffer[i_index].m_vmapImage.Create(src.GetSizeX(), src.GetSizeY());
    }

    // Real Buffer 설정
    SetRealImageSize(m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);

    // Real Buffer 복사
    for (int index = 0; index < int(object.m_realBuffer[i_index].m_vecImages.size()); index++)
    {
        auto& dst = m_realBuffer[i_index].m_vecImages[index];
        auto& src = object.m_realBuffer[i_index].m_vecImages[index];

        Ipvm::Rect32s validRegion(0, 0, src.GetSizeX(), src.GetSizeY());

        dst = Ipvm::Image8u(m_buffer[i_index].m_vecImages[index], validRegion);
        Ipvm::ImageProcessing::Copy(src, validRegion, dst);
    }

    if (true)
    {
        auto& dst = m_realBuffer[i_index].m_zmapImage;
        auto& src = object.m_realBuffer[i_index].m_zmapImage;

        Ipvm::Rect32s validRegion(0, 0, src.GetSizeX(), src.GetSizeY());

        dst = Ipvm::Image32r(m_buffer[i_index].m_zmapImage, validRegion);
        Ipvm::ImageProcessing::Copy(src, validRegion, dst);
    }

    if (true)
    {
        auto& dst = m_realBuffer[i_index].m_vmapImage;
        auto& src = object.m_realBuffer[i_index].m_vmapImage;

        Ipvm::Rect32s validRegion(0, 0, src.GetSizeX(), src.GetSizeY());

        dst = Ipvm::Image16u(m_buffer[i_index].m_vmapImage, validRegion);
        Ipvm::ImageProcessing::Copy(src, validRegion, dst);
    }

    return true;
}

void VisionImageLot::SetTrayIndex(const int trayIndex, const int i_index)
{
    m_info[i_index].m_trayIndex = trayIndex;
}

void VisionImageLot::SetRunTimeIndex(const int runTimeIndex, const int i_index)
{
    m_info[i_index].m_runTimeIndex = runTimeIndex;
}

void VisionImageLot::SetScanAreaIndex(const int scanAreaIndex, const int i_index)
{
    m_info[i_index].m_scanAreaIndex = scanAreaIndex;
}

void VisionImageLot::SetScales(const float px2um_x, const float px2um_y, const float px2um_z, const int i_index)
{
    m_info[i_index].m_scale.setScale(px2um_x, px2um_y, px2um_z);
}

void VisionImageLot::SetScales(const VisionScale& scale, const int i_index)
{
    m_info[i_index].m_scale.setScale(scale.pixelToUm().m_x, scale.pixelToUm().m_y, scale.pixelToUmZ());
}

void VisionImageLot::Set3DScanInfo(float heightRangeMin, float heightRangeMax, const int i_index)
{
    m_info[i_index].m_heightRangeMin = heightRangeMin;
    m_info[i_index].m_heightRangeMax = heightRangeMax;
}

void VisionImageLot::SetFillPocketTR(const bool fillPocket, const int i_index)
{
    m_info[i_index].m_fillPocket = fillPocket;
}

bool VisionImageLot::IsExistZMap(const int i_index) const
{
    return m_info[i_index].m_zmapCount > 0;
}

BOOL VisionImageLot::Load(LPCTSTR pathName, bool popupMessage)
{
    const int nRepeatCount = SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP ? 2 : 1;

    VisionImageLotInfo fileInfo;

    CFile file;

    if (!file.Open(pathName, CFile::modeRead | CFile::shareDenyWrite))
    {
        if (popupMessage)
        {
            ::AfxMessageBox(_T("Cannot open file!"), MB_OK | MB_ICONERROR);
        }

        return FALSE;
    }

    for (int nidx = enSideVisionModule::SIDE_VISIONMODULE_START; nidx < nRepeatCount; nidx++)
    {
        if (!fileInfo.LoadInfo(pathName, popupMessage))
        {
            return FALSE;
        }

        if (m_buffer[nidx].m_vecImages.size() == 0)
        {
            if (popupMessage)
            {
                ::AfxMessageBox(_T("job file mismatch!"), MB_OK | MB_ICONERROR);
            }
            return FALSE;
        }

        if (!CompatibilityCheck(fileInfo, enSideVisionModule(nidx)))
        {
            CString strOutput;
            CString strTemp;

            strOutput += _T("Imaging condition mismatch!\r\n\r\n");

            strOutput += _T("Current recipe's imaging condition\r\n");
            strTemp.Format(_T("Camera image size x : %d\r\n"), m_info[nidx].m_sensorSizeX);
            strOutput += strTemp;
            strTemp.Format(_T("Camera image size y : %d\r\n"), m_info[nidx].m_sensorSizeY);
            strOutput += strTemp;
            strTemp.Format(_T("Real Image size x : %d\r\n"), m_info[nidx].m_imageSizeX);
            strOutput += strTemp;
            strTemp.Format(_T("Real Image size y : %d\r\n"), m_info[nidx].m_imageSizeY);
            strOutput += strTemp;
            strTemp.Format(_T("Frame count : %d\r\n"), m_info[nidx].m_frameCount);
            strOutput += strTemp;
            strTemp.Format(_T("Z-Map count : %d\r\n"), m_info[nidx].m_zmapCount);
            strOutput += strTemp;
            strTemp.Format(_T("V-Map count : %d\r\n"), m_info[nidx].m_vmapCount);
            strOutput += strTemp;

            strOutput += _T("\r\n");

            strOutput += _T("Image file's imaging condition\r\n");
            strTemp.Format(_T("Camera image size x : %d\r\n"), fileInfo.m_sensorSizeX);
            strOutput += strTemp;
            strTemp.Format(_T("Camera image size y : %d\r\n"), fileInfo.m_sensorSizeY);
            strOutput += strTemp;
            strTemp.Format(_T("Real Image size x : %d\r\n"), fileInfo.m_imageSizeX);
            strOutput += strTemp;
            strTemp.Format(_T("Real Image size y : %d\r\n"), fileInfo.m_imageSizeY);
            strOutput += strTemp;
            strTemp.Format(_T("Frame count : %d\r\n"), fileInfo.m_frameCount);
            strOutput += strTemp;
            strTemp.Format(_T("Z-Map count : %d\r\n"), fileInfo.m_zmapCount);
            strOutput += strTemp;
            strTemp.Format(_T("V-Map count : %d\r\n"), fileInfo.m_vmapCount);
            strOutput += strTemp;

            if (popupMessage)
            {
                ::AfxMessageBox(strOutput, MB_OK | MB_ICONERROR);
            }

            strOutput.Empty();
            strTemp.Empty();

            return FALSE;
        }

        CString title;
        title.Format(_T("Load image file : \"%s\" "), pathName);

        // 여기서부터는 모든 조건이 맞아 떨어진 것이 확인되었으므로, 무조건 읽어들이기만 한다.
        std::shared_ptr<Ipvm::AsyncProgress> progress;
        if (popupMessage)
        {
            progress = std::make_shared<Ipvm::AsyncProgress>(title);
        }

        BITMAPFILEHEADER bmfh;
        BITMAPINFOHEADER bmih;
        RGBQUAD palette[256];

        file.Read(&bmfh, sizeof(bmfh));
        file.Read(&bmih, sizeof(bmih));
        file.Read(&palette, sizeof(palette));

        auto firstImagePos = file.GetPosition();
        file.Seek(bmih.biSizeImage, CFile::current);

        int sampling = PersonalConfig::getInstance().getImageSampling();

        // 추가 정보를 위한 헤더
        char fileTag[] = FILE_EXTENSION_TAG;
        int fileVersion = 0;
        file.Read(fileTag, sizeof(fileTag));
        file.Read(&fileVersion, sizeof(fileVersion));

        int realImageSizeX = 0;
        int realImageSizeY = 0;
        int realFrameCount = 0;

        if (fileVersion == 0)
        {
            ExtensionInfoHeader_V0 eih;
            file.Read(&eih, sizeof(eih));

            realImageSizeX = eih.m_imageSizeX;
            realImageSizeY = eih.m_imageSizeY;
            realFrameCount = eih.m_frameCount;
        }
        else if (fileVersion == 1)
        {
            ExtensionInfoHeader_V1 eih;
            file.Read(&eih, sizeof(eih));

            if (eih.m_frameCount > m_info[nidx].m_frameCount)
            {
                if (popupMessage)
                {
                    CString message;
                    message.Format(_T("Number of frames do not match : %d (Job Setting : %d)"), eih.m_frameCount,
                        m_info[nidx].m_frameCount);
                    ::AfxMessageBox(message, MB_OK | MB_ICONERROR);

                    message.Empty();
                }

                return FALSE;
            }

            realImageSizeX = eih.m_imageSizeX;
            realImageSizeY = eih.m_imageSizeY;
            realFrameCount = eih.m_frameCount;
            m_info[nidx].m_imageSizeX = eih.m_imageSizeX / sampling;
            m_info[nidx].m_imageSizeY = eih.m_imageSizeY / sampling;
            m_info[nidx].m_trayIndex = eih.m_trayIndex;
            m_info[nidx].m_runTimeIndex = eih.m_runTimeIndex;
            m_info[nidx].m_scanAreaIndex = eih.m_scanAreaIndex;
            m_info[nidx].m_heightRangeMin = eih.m_heightRangeMin;
            m_info[nidx].m_heightRangeMax = eih.m_heightRangeMax;

            m_info[nidx].m_scale.setScale(eih.m_px2um_x * sampling, eih.m_px2um_y * sampling, eih.m_px2um_z);
        }
        else if (fileVersion == 2)
        {
            ExtensionInfoHeader_V2 eih;
            file.Read(&eih, sizeof(eih));

            if (eih.m_frameCount > m_info[nidx].m_frameCount)
            {
                if (popupMessage)
                {
                    CString message;
                    message.Format(_T("Number of frames do not match : %d (Job Setting : %d)"), eih.m_frameCount,
                        m_info[nidx].m_frameCount);
                    ::AfxMessageBox(message, MB_OK | MB_ICONERROR);

                    message.Empty();
                }

                return FALSE;
            }

            realImageSizeX = eih.m_imageSizeX;
            realImageSizeY = eih.m_imageSizeY;
            realFrameCount = eih.m_frameCount;
            m_info[nidx].m_imageSizeX = eih.m_imageSizeX / sampling;
            m_info[nidx].m_imageSizeY = eih.m_imageSizeY / sampling;
            m_info[nidx].m_trayIndex = eih.m_trayIndex;
            m_info[nidx].m_runTimeIndex = eih.m_runTimeIndex;
            m_info[nidx].m_scanAreaIndex = eih.m_scanAreaIndex;
            m_info[nidx].m_heightRangeMin = eih.m_heightRangeMin;
            m_info[nidx].m_heightRangeMax = eih.m_heightRangeMax;
            m_info[nidx].m_scale.setScale(eih.m_px2um_x * sampling, eih.m_px2um_y * sampling, eih.m_px2um_z);
            m_info[nidx].m_nSideModuleIndex = eih.m_nSideModuleIndex;
        }
        else if (fileVersion == 3)
        {
            ExtensionInfoHeader_V3 eih;
            file.Read(&eih, sizeof(eih));

            if (eih.m_frameCount > m_info[nidx].m_frameCount)
            {
                if (popupMessage)
                {
                    CString message;
                    message.Format(_T("Number of frames do not match : %d (Job Setting : %d)"), eih.m_frameCount,
                        m_info[nidx].m_frameCount);
                    ::AfxMessageBox(message, MB_OK | MB_ICONERROR);

                    message.Empty();
                }

                return FALSE;
            }

            realImageSizeX = eih.m_imageSizeX;
            realImageSizeY = eih.m_imageSizeY;
            realFrameCount = eih.m_frameCount;
            m_info[nidx].m_imageSizeX = eih.m_imageSizeX / sampling;
            m_info[nidx].m_imageSizeY = eih.m_imageSizeY / sampling;
            m_info[nidx].m_trayIndex = eih.m_trayIndex;
            m_info[nidx].m_runTimeIndex = eih.m_runTimeIndex;
            m_info[nidx].m_scanAreaIndex = eih.m_scanAreaIndex;
            m_info[nidx].m_heightRangeMin = eih.m_heightRangeMin;
            m_info[nidx].m_heightRangeMax = eih.m_heightRangeMax;
            m_info[nidx].m_scale.setScale(eih.m_px2um_x * sampling, eih.m_px2um_y * sampling, eih.m_px2um_z);
            m_info[nidx].m_nSideModuleIndex = eih.m_nSideModuleIndex;
            m_info[nidx].m_fillPocket = eih.m_fillPocket;
        }

        if (!SetRealImageSize(m_info[nidx].m_imageSizeX, m_info[nidx].m_imageSizeY, enSideVisionModule(nidx)))
        {
            file.Close();
            return FALSE;
        }

        // 2D 이미지 읽기
        auto secondImagePos = file.GetPosition();

        for (int frame = 0; frame < m_info[nidx].m_frameCount; frame++)
        {
            if (frame == 0)
            {
                file.Seek(firstImagePos, CFile::begin);
            }

            if (sampling != 1)
            {
                // MED#6 NGRV SingleRun
                if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_NGRV_INSP)
                {
                    Ipvm::Image8u image(realImageSizeX, realImageSizeY);
                    file.Read(image.GetMem(), bmih.biSizeImage);
                    Ipvm::ImageProcessing::ResizeLinearInterpolation(image, m_realBuffer[nidx].m_vecImages[frame]);
                }
                else
                {
                    Ipvm::Image8u image(realImageSizeX, realImageSizeY);
                    file.Read(image.GetMem(), bmih.biSizeImage);
                    Ipvm::ImageProcessing::ResizeLinearInterpolation(image, m_realBuffer[nidx].m_vecImages[frame]);

                    Ipvm::ImageProcessingGpu::DemosaicBilinear(m_realBuffer[nidx].m_vecImages[frame],
                        Ipvm::Rect32s(m_realBuffer[nidx].m_vecImages[frame]), Ipvm::BayerPattern::e_gbrg,
                        m_realBuffer[nidx].m_vecColorImages[frame]);
                }
            }
            else
            {
                // MED#6 NGRV SingleRun
                if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_NGRV_INSP)
                {
                    file.Read(m_realBuffer[nidx].m_vecImages[frame].GetMem(), bmih.biSizeImage);
                }
                else
                {
                    file.Read(m_realBuffer[nidx].m_vecImages[frame].GetMem(), bmih.biSizeImage);

                    Ipvm::Image8u3 imageColor(realImageSizeX, realImageSizeY);

                    Ipvm::ImageProcessingGpu::DemosaicBilinear(m_realBuffer[nidx].m_vecImages[frame],
                        Ipvm::Rect32s(m_realBuffer[nidx].m_vecImages[frame]), Ipvm::BayerPattern::e_gbrg, imageColor);

                    m_realBuffer[nidx].m_vecColorImages[frame] = imageColor;
                }
                ///////////////////////
            }

            if (frame == 0)
            {
                file.Seek(secondImagePos, CFile::begin);
            }
        }

        // Z-Map 읽기
        if (m_info[nidx].m_zmapCount > 0)
        {
            Ipvm::Image16s zmapImageTemp(realImageSizeX, realImageSizeY);

            file.Read(zmapImageTemp.GetMem(), zmapImageTemp.GetWidthBytes() * zmapImageTemp.GetSizeY());

            for (int y = 0, srcY = 0; y < m_info[nidx].m_imageSizeY; y++, srcY += sampling)
            {
                const auto* zmapSrc = zmapImageTemp.GetMem(0, srcY);
                auto* zmapDst = m_realBuffer[nidx].m_zmapImage.GetMem(0, y);

                for (int x = 0, srcX = 0; x < m_info[nidx].m_imageSizeX; x++, srcX += sampling)
                {
                    if (zmapSrc[srcX] == -32768)
                    {
                        zmapDst[x] = Ipvm::k_noiseValue32r;
                    }
                    else
                    {
                        zmapDst[x] = 0.1f * zmapSrc[srcX];
                    }
                }
            }
        }

        // V-Map 읽기
        if (m_info[nidx].m_vmapCount > 0)
        {
            if (sampling != 1)
            {
                Ipvm::Image16u image(realImageSizeX, realImageSizeY);
                file.Read(image.GetMem(), image.GetWidthBytes() * image.GetSizeY());

                Ipvm::ImageProcessing::ResizeNearestInterpolation(image, m_realBuffer[nidx].m_vmapImage);
            }
            else
            {
                file.Read(m_realBuffer[nidx].m_vmapImage.GetMem(),
                    m_realBuffer[nidx].m_vmapImage.GetWidthBytes() * m_realBuffer[nidx].m_vmapImage.GetSizeY());
            }
        }

        title.Empty();
        file.Seek(file.GetPosition(), CFile::begin);
    }

    file.Close();

    ////////// 임시 Code ///////////////
    //3D Vision 이면서 컨트롤키와 시프트키가 눌려 있으면 별도의 Raw 파일을 읽어들인다.
    if (SystemConfig::GetInstance().IsVisionType3D() == TRUE && GetAsyncKeyState(VK_CONTROL) & 0x8000
        && GetAsyncKeyState(VK_SHIFT) & 0x8000)
    {
        //raw 파일을 읽기 위한 파일 다이얼로그를 띄우고 선택항 파일 경로를 strFileName에 저장한다.

        CString strFileName;

        CFileDialog fileDlg(
            TRUE, _T("raw"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Raw Files (*.raw)|*.raw||"));
        if (fileDlg.DoModal() == IDOK)
            strFileName = fileDlg.GetPathName();
        else
            return TRUE;

        CFile file2;
        if (file2.Open(strFileName, CFile::modeRead))
        {
            CArchive archive(&file2, CArchive::load);
            int nImageSizeX = 0;
            int nImageSizeY = 0;
            archive >> nImageSizeX;
            archive >> nImageSizeY;

            int nImageSizeNew = nImageSizeX * nImageSizeY;
            archive.Read(m_realBuffer[0].m_vecImages[0].GetMem(), nImageSizeNew * sizeof(BYTE));
            archive.Read(m_realBuffer[0].m_zmapImage.GetMem(), nImageSizeNew * sizeof(float));
            archive.Read(m_realBuffer[0].m_vmapImage.GetMem(), nImageSizeNew * sizeof(uint16_t));
            archive.Close();

            float fMinZ = FLT_MAX;
            float fMaxZ = -FLT_MAX;
            float* pfZmap = m_realBuffer[0].m_zmapImage.GetMem();
            for (int nIndex = 0; nIndex < nImageSizeNew; nIndex++)
            {
                if (pfZmap[nIndex] != -FLT_MAX)
                {
                    if (pfZmap[nIndex] < fMinZ)
                        fMinZ = pfZmap[nIndex];
                    if (pfZmap[nIndex] > fMaxZ)
                        fMaxZ = pfZmap[nIndex];
                }
            }
            fMinZ -= 200.f; // 200um 정도 여유를 둔다.
            fMaxZ += 200.f; // 200um 정도 여유를 둔다.

            Ipvm::ImageProcessing::ScaleInRangeWithNoise(m_realBuffer[0].m_zmapImage,
                Ipvm::Rect32s(m_realBuffer[0].m_zmapImage), Ipvm::k_noiseValue32r, fMinZ, fMaxZ, 0, 1, 255,
                m_realBuffer[0].m_vecImages[0]);

            Ipvm::ImageProcessing::Scale(m_realBuffer[0].m_vmapImage, Ipvm::Rect32s(m_realBuffer[0].m_vmapImage),
                m_realBuffer[0].m_vecImages[1]);

            file2.Close();
        }
    }

    return TRUE;
}

BOOL VisionImageLot::Save(LPCTSTR pathName) const
{
    const int nRepeatCount = SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP ? 2 : 1;

    for (int nidx = enSideVisionModule::SIDE_VISIONMODULE_START; nidx < nRepeatCount; nidx++)
    {
        if (m_realBuffer[nidx].m_vecImages.size() <= 0)
        {
            ::AfxMessageBox(_T("Image count is invalid!"), MB_OK | MB_ICONERROR);
            return FALSE;
        }

        if (GetImageSizeX() <= 0 || GetImageSizeY() <= 0)
        {
            ::AfxMessageBox(_T("Image size is invalid!"), MB_OK | MB_ICONERROR);
            return FALSE;
        }
    }

    //여기서 부터는 Buffer에 문제가 없으므로 그냥 저장한다
    CFile file;

    if (!file.Open(pathName, CFile::modeCreate | CFile::modeWrite))
    {
        ::AfxMessageBox(_T("Cannot open file!"), MB_OK | MB_ICONERROR);
        return FALSE;
    }

    for (int nidx = enSideVisionModule::SIDE_VISIONMODULE_START; nidx < nRepeatCount; nidx++)
    {
        if (m_realBuffer[nidx].m_vecImages.size() <= 0)
        {
            ::AfxMessageBox(_T("Image count is invalid!"), MB_OK | MB_ICONERROR);
            return FALSE;
        }

        if (GetImageSizeX() <= 0 || GetImageSizeY() <= 0)
        {
            ::AfxMessageBox(_T("Image size is invalid!"), MB_OK | MB_ICONERROR);
            return FALSE;
        }

        // Image Size 40% 줄여서 다시 저장 - 이거 왜 했지?
        /*int nImageSize = m_realBuffer[nidx].m_vecImages.size();
        auto& image = m_realBuffer[nidx].m_vecImages[0];
        std::vector<Ipvm::Image8u> changedImages;
        changedImages.resize(nImageSize);

        Ipvm::Rect32s copyRect(
            (image.GetSizeX() * 0.2), (image.GetSizeY() * 0.2), (image.GetSizeX() * 0.8), (image.GetSizeY() * 0.8));
        for (int nIndex = 0; nIndex < nImageSize; nIndex++)
        {
            changedImages[nIndex].Create(copyRect.Width(), copyRect.Height());
            Ipvm::ImageProcessing::Copy(m_realBuffer[nidx].m_vecImages[nIndex], copyRect, changedImages[nIndex]);
        }
        auto& firstImage = changedImages[0];
        */
        ///////////////////////////////////

        BITMAPFILEHEADER bmfh;
        BITMAPINFOHEADER bmih;
        RGBQUAD palette[256];

        auto& firstImage = m_realBuffer[nidx].m_vecImages[0];

        const int imageBitmapSize = firstImage.GetWidthBytes() * firstImage.GetSizeY();
        const int headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(palette);

        bmfh.bfType = 0x4d42;
        bmfh.bfSize = headerSize + imageBitmapSize;
        bmfh.bfReserved1 = 0;
        bmfh.bfReserved2 = 0;
        bmfh.bfOffBits = headerSize;

        bmih.biBitCount = 8;
        bmih.biClrImportant = 0;
        bmih.biClrUsed = 0;
        bmih.biCompression = 0;
        bmih.biHeight = -firstImage.GetSizeY();
        bmih.biPlanes = 1;
        bmih.biSize = sizeof(BITMAPINFOHEADER);
        bmih.biSizeImage = imageBitmapSize;
        bmih.biWidth = firstImage.GetSizeX();
        bmih.biXPelsPerMeter = 0;
        bmih.biYPelsPerMeter = 0;

        if (m_realBuffer[nidx].m_zmapImage.GetMem())
        {
            // 3D Images
            palette[0].rgbBlue = 0;
            palette[0].rgbGreen = 0;
            palette[0].rgbRed = 0;
            palette[0].rgbReserved = 0;

            Ipvm::ColorMap::Make(Ipvm::ColorMapIndex::e_blueGreenRed, 255, palette + 1);
        }
        else
        {
            // 2D Images
            for (int idx = 0; idx < 256; idx++)
            {
                palette[idx].rgbBlue = (BYTE)idx;
                palette[idx].rgbGreen = (BYTE)idx;
                palette[idx].rgbRed = (BYTE)idx;
                palette[idx].rgbReserved = (BYTE)0;
            }
        }

        CString title;
        title.Format(_T("Save image file : \"%s\" "), pathName);

        //Ipvm::AsyncProgress progress(title);

        file.Write(&bmfh, sizeof(bmfh));
        file.Write(&bmih, sizeof(bmih));
        file.Write(&palette, sizeof(palette));
        file.Write(firstImage.GetMem(), imageBitmapSize);

        // 추가 정보를 위한 헤더
        const char fileTag[] = FILE_EXTENSION_TAG;
        const int fileVersion = 3;
        ExtensionInfoHeader_V3 eih;

        eih.m_sensorSizeX = m_info[nidx].m_sensorSizeX;
        eih.m_sensorSizeY = m_info[nidx].m_sensorSizeY;
        eih.m_imageSizeX = m_info[nidx].m_imageSizeX;
        eih.m_imageSizeY = m_info[nidx].m_imageSizeY;
        eih.m_frameCount = m_info[nidx].m_frameCount;
        eih.m_zmapCount = m_info[nidx].m_zmapCount;
        eih.m_vmapCount = m_info[nidx].m_vmapCount;
        eih.m_trayIndex = m_info[nidx].m_trayIndex;
        eih.m_runTimeIndex = m_info[nidx].m_runTimeIndex;
        eih.m_scanAreaIndex = m_info[nidx].m_scanAreaIndex;
        eih.m_px2um_x = m_info[nidx].m_scale.pixelToUm().m_x;
        eih.m_px2um_y = m_info[nidx].m_scale.pixelToUm().m_y;
        eih.m_px2um_z = m_info[nidx].m_scale.pixelToUmZ();
        eih.m_heightRangeMin = m_info[nidx].m_heightRangeMin;
        eih.m_heightRangeMax = m_info[nidx].m_heightRangeMax;
        eih.m_nSideModuleIndex = m_info[nidx].m_nSideModuleIndex;
        eih.m_fillPocket = m_info[nidx].m_fillPocket;

        file.Write(fileTag, sizeof(fileTag));
        file.Write(&fileVersion, sizeof(fileVersion));
        file.Write(&eih, sizeof(eih));

        // 2D 이미지 저장
        for (int frame = 1; frame < m_info[nidx].m_frameCount; frame++)
        {
            auto& image = m_realBuffer[nidx].m_vecImages[frame];
            file.Write(image.GetMem(), image.GetWidthBytes() * image.GetSizeY());
        }

        // Z-Map 저장
        if (m_info[nidx].m_zmapCount > 0)
        {
            Ipvm::Image16s zmapImageTemp(m_info[nidx].m_imageSizeX, m_info[nidx].m_imageSizeY);

            for (int y = 0; y < m_info[nidx].m_imageSizeY; y++)
            {
                const auto* zmapSrc = m_realBuffer[nidx].m_zmapImage.GetMem(0, y);
                auto* zmapDst = zmapImageTemp.GetMem(0, y);

                for (int x = 0; x < m_info[nidx].m_imageSizeX; x++)
                {
                    if (zmapSrc[x] == Ipvm::k_noiseValue32r)
                    {
                        zmapDst[x] = -32768;
                    }
                    else
                    {
                        zmapDst[x] = (short)(zmapSrc[x] / 0.1f);
                    }
                }
            }

            file.Write(zmapImageTemp.GetMem(), zmapImageTemp.GetWidthBytes() * zmapImageTemp.GetSizeY());
        }

        // V-Map 저장
        if (m_info[nidx].m_vmapCount > 0)
        {
            file.Write(m_realBuffer[nidx].m_vmapImage.GetMem(),
                m_realBuffer[nidx].m_vmapImage.GetWidthBytes() * m_realBuffer[nidx].m_vmapImage.GetSizeY());
        }

        title.Empty();
    }

    file.Close(); // 파일 클로즈 없어서 추가함

    return TRUE;
}

BOOL VisionImageLot::SaveTo(LPCTSTR directoryPath, CString& o_strSaveFilePath, const int i_index) const
{
    CString strImagePathName;
    int nTrayIndex = m_info[i_index].m_trayIndex;
    int nScanAreaIndex = m_info[i_index].m_scanAreaIndex;
    int nRepeatIndex = 0;
    strImagePathName.Format(
        _T("%sTray[%03d]_ScanArea[%03d]_Repeat[%02d].bmp"), directoryPath, nTrayIndex, nScanAreaIndex, nRepeatIndex);

    CFileFind ff;

    while (nRepeatIndex < 100)
    {
        if (ff.FindFile(strImagePathName))
        {
            nRepeatIndex++;
            strImagePathName.Format(_T("%sTray[%03d]_ScanArea[%03d]_Repeat[%02d].bmp"), directoryPath, nTrayIndex,
                nScanAreaIndex, nRepeatIndex);
        }
        else
            break;
    }

    o_strSaveFilePath = strImagePathName;

    return Save(strImagePathName);
}

void VisionImageLot::InlineRawSaveCheckReset()
{
    EnterCriticalSection(&m_csInlineRawSaveCheck);
    m_inlineRawSaveCheck = false;
    LeaveCriticalSection(&m_csInlineRawSaveCheck);
}

bool VisionImageLot::InlineRawSaveCheck()
{
    EnterCriticalSection(&m_csInlineRawSaveCheck);
    if (m_inlineRawSaveCheck)
    {
        LeaveCriticalSection(&m_csInlineRawSaveCheck);
        return false;
    }

    m_inlineRawSaveCheck = true;
    LeaveCriticalSection(&m_csInlineRawSaveCheck);

    return true;
}

void VisionImageLot::AllocateImageFromInfo(const int i_index)
{
    int nVisionType = SystemConfig::GetInstance().GetVisionType();

    switch (nVisionType)
    {
        case VISIONTYPE_2D_INSP:
        case VISIONTYPE_3D_INSP:
        case VISIONTYPE_TR:
        case VISIONTYPE_SWIR_INSP:
        {
            m_buffer[i_index].m_vecImages.resize(m_info[i_index].m_frameCount);
            m_realBuffer[i_index].m_vecImages.resize(m_info[i_index].m_frameCount);

            for (int index = 0; index < m_info[i_index].m_frameCount; index++)
            {
                m_buffer[i_index].m_vecImages[index].Create(m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);
            }
        }
        break;

        case VISIONTYPE_NGRV_INSP:
        {
            m_buffer[i_index].m_vecImages.resize(m_info[i_index].m_frameCount);
            m_buffer[i_index].m_vecColorImages.resize(m_info[i_index].m_frameCount);

            m_realBuffer[i_index].m_vecImages.resize(m_info[i_index].m_frameCount);
            m_realBuffer[i_index].m_vecColorImages.resize(m_info[i_index].m_frameCount);

            for (int index = 0; index < m_info[i_index].m_frameCount; index++)
            {
                m_buffer[i_index].m_vecImages[index].Create(m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);
                m_buffer[i_index].m_vecColorImages[index].Create(
                    m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);
            }
        }
        break;

        case VISIONTYPE_SIDE_INSP:
        {
            for (int nimageidx = SIDE_VISIONMODULE_START; nimageidx < SIDE_VISIONMODULE_END; nimageidx++)
            {
                m_buffer[nimageidx].m_vecImages.resize(m_info[i_index].m_frameCount);
                m_realBuffer[nimageidx].m_vecImages.resize(m_info[i_index].m_frameCount);

                for (int index = 0; index < m_info[i_index].m_frameCount; index++)
                {
                    m_buffer[nimageidx].m_vecImages[index].Create(
                        m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);
                }
            }
        }
        break;
    }

    if (m_info[i_index].m_zmapCount > 0)
    {
        m_buffer[i_index].m_zmapImage.Create(m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);
        m_buffer[i_index].m_vmapImage.Create(m_info[i_index].m_imageSizeX, m_info[i_index].m_imageSizeY);
    }
    else
    {
        m_buffer[i_index].m_zmapImage.Free();
        m_buffer[i_index].m_vmapImage.Free();
    }
}

bool VisionImageLot::CompatibilityCheck(const VisionImageLotInfo& object, const int i_index)
{
    //---------------------------------------------------------------------------------------
    // 이미지 버퍼가 충분한지 체크
    //---------------------------------------------------------------------------------------

    if (m_info[i_index].m_zmapCount != object.m_zmapCount || m_info[i_index].m_vmapCount != object.m_vmapCount
        || (int)m_realBuffer[i_index].m_vecImages.size() != object.m_frameCount)
    {
        return false;
    }

    for (int index = 0; index < int(m_buffer[i_index].m_vecImages.size()); index++)
    {
        if (object.m_imageSizeX > m_buffer[i_index].m_vecImages[index].GetSizeX()
            || object.m_imageSizeY > m_buffer[i_index].m_vecImages[index].GetSizeY())
        {
            return false;
        }
    }

    // ZMAP과 VMAP도 체크하자
    if (m_info[i_index].m_zmapCount > 0)
    {
        if (object.m_imageSizeX > m_buffer[i_index].m_zmapImage.GetSizeX()
            || object.m_imageSizeY > m_buffer[i_index].m_zmapImage.GetSizeY())
        {
            return false;
        }
    }

    if (m_info[i_index].m_vmapCount > 0)
    {
        if (object.m_imageSizeX > m_buffer[i_index].m_vmapImage.GetSizeX()
            || object.m_imageSizeY > m_buffer[i_index].m_vmapImage.GetSizeY())
        {
            return false;
        }
    }

    return true;
}

void VisionImageLot::SetInspectionRepeatIndex(int repeatIndex)
{
    m_nInspectionRepeatIndex = repeatIndex;
}

int VisionImageLot::GetInspectionRepeatIndex()
{
    return m_nInspectionRepeatIndex;
}