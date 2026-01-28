//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionReusableMemory.h"

//CPP_2_________________________________ This project's headers
#include "ReusableMemory_Image.h"
#include "SurfaceLayerMask.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../DefineModules/dA_Base/BlobAttribute.h"
#include "../../DefineModules/dA_Base/semiinfo.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image16s.h>
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image64r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionReusableMemory::VisionReusableMemory(void)
    : m_pEdgeDetect(nullptr)
    , m_reusable_image(new ReusableMemory_Image)
    , m_maxBufferSizeX(0)
    , m_maxBufferSizeY(0)
    , m_inspSizeX(0)
    , m_inspSizeY(0)
    , m_alignSuccess(false)
    , m_bodyCenterX(0.f)
    , m_bodyCenterY(0.f)
    , m_packageAngle_deg(0.f)
{
}

VisionReusableMemory::~VisionReusableMemory(void)
{
    for (auto* data : m_vecBlobInfo)
        delete[] data;
    for (auto* data : m_vecBlobAttribute)
        delete[] data;
    for (auto* data : m_vecBlob)
        delete data;

    for (auto& layer : m_vecSurfaceLayerMask)
        delete layer.second;
    for (auto& layer : m_vecSurfaceLayerRoi)
        delete layer.second;

    delete m_pEdgeDetect;
    delete m_reusable_image;
}

void VisionReusableMemory::SetBufferImageSize(long maxSizeX, long maxSizeY, long nVisionType)
{
    long minimumBufferCount = 0;

    if (nVisionType == VISIONTYPE_2D_INSP || nVisionType == VISIONTYPE_SIDE_INSP || nVisionType == VISIONTYPE_TR
        || nVisionType == VISIONTYPE_SWIR_INSP) //kircheis_SWIR
    {
        minimumBufferCount = 8;
    }
    else
    {
        minimumBufferCount = 8;
    }

    m_reusable_image->Ready((size_t)(Ipvm::Image::CalcWidthBytes(maxSizeX, 64, 4)) * (size_t)maxSizeY, maxSizeX,
        maxSizeY, minimumBufferCount);

    m_maxBufferSizeX = maxSizeX;
    m_maxBufferSizeY = maxSizeY;

    if (nVisionType == VISIONTYPE_2D_INSP || nVisionType == VISIONTYPE_SIDE_INSP || nVisionType == VISIONTYPE_TR
        || nVisionType == VISIONTYPE_SWIR_INSP) //kircheis_SWIR
    {
        SetReserveBlob(3);
        SetReserveBlobInfo(2);
        SetReserveBlobAttribute(2);
    }
    else if (nVisionType == VISIONTYPE_3D_INSP)
    {
        SetReserveBlobInfo(1);
        SetReserveBlobAttribute(1);
        SetReserveBlob(1);
    }
}

void VisionReusableMemory::SetInspImageSize(long sizeX, long sizeY)
{
    m_inspSizeX = sizeX;
    m_inspSizeY = sizeY;
}

int VisionReusableMemory::GetMaxBufferImageSizeX() const
{
    return m_maxBufferSizeX;
}

int VisionReusableMemory::GetMaxBufferImageSizeY() const
{
    return m_maxBufferSizeY;
}

int VisionReusableMemory::GetInspImageSizeX() const
{
    return m_inspSizeX;
}

int VisionReusableMemory::GetInspImageSizeY() const
{
    return m_inspSizeY;
}

void VisionReusableMemory::SetReserveBlobInfo(long nNum)
{
    for (long i = (long)(m_vecBlobInfo.size()); i < nNum; i++)
    {
        Ipvm::BlobInfo* pData = new Ipvm::BlobInfo[BLOB_INFO_MAX_SIZE];
        m_vecBlobInfo.push_back(pData);
    }
}

void VisionReusableMemory::SetReserveBlobAttribute(long nNum)
{
    for (long i = (long)(m_vecBlobAttribute.size()); i < nNum; i++)
    {
        BlobAttribute* pData = new BlobAttribute[BLOB_INFO_MAX_SIZE];
        m_vecBlobAttribute.push_back(pData);
    }
}

void VisionReusableMemory::SetReserveBlob(long nNum)
{
    for (long i = (long)(m_vecBlob.size()); i < nNum; i++)
    {
        CPI_Blob* pData = new CPI_Blob;
        m_vecBlob.push_back(pData);
    }
}

void VisionReusableMemory::SetAlignInfo(const Ipvm::Point32r2& bodyCenter, float packageAngle_deg)
{
    m_alignSuccess = true;
    m_bodyCenterX = bodyCenter.m_x;
    m_bodyCenterY = bodyCenter.m_y;
    m_packageAngle_deg = packageAngle_deg;
}

bool VisionReusableMemory::GetFloatImage(Ipvm::Image32r& image, long sizeX, long sizeY)
{
    return m_reusable_image->get(image, sizeX, sizeY);
}

bool VisionReusableMemory::GetDoubleImage(Ipvm::Image64r& image, long sizeX, long sizeY)
{
    return m_reusable_image->get(image, sizeX, sizeY);
}

bool VisionReusableMemory::GetLongImage(Ipvm::Image32s& image, long sizeX, long sizeY)
{
    return m_reusable_image->get(image, sizeX, sizeY);
}

bool VisionReusableMemory::GetShortImage(Ipvm::Image16s& image, long sizeX, long sizeY)
{
    return m_reusable_image->get(image, sizeX, sizeY);
}

bool VisionReusableMemory::GetUShortImage(Ipvm::Image16u& image, long sizeX, long sizeY)
{
    return m_reusable_image->get(image, sizeX, sizeY);
}

bool VisionReusableMemory::GetWordImage(Ipvm::Image16u& image, long sizeX, long sizeY)
{
    return m_reusable_image->get(image, sizeX, sizeY);
}

bool VisionReusableMemory::GetColorImage(Ipvm::Image8u3& image, long sizeX, long sizeY)
{
    return m_reusable_image->get(image, sizeX, sizeY);
}

bool VisionReusableMemory::GetByteImage(Ipvm::Image8u& image, long sizeX, long sizeY)
{
    return m_reusable_image->get(image, sizeX, sizeY);
}

bool VisionReusableMemory::GetInspFloatImage(Ipvm::Image32r& image)
{
    return GetFloatImage(image, m_inspSizeX, m_inspSizeY);
}

bool VisionReusableMemory::GetInspDoubleImage(Ipvm::Image64r& image)
{
    return GetDoubleImage(image, m_inspSizeX, m_inspSizeY);
}

bool VisionReusableMemory::GetInspLongImage(Ipvm::Image32s& image)
{
    return GetLongImage(image, m_inspSizeX, m_inspSizeY);
}

bool VisionReusableMemory::GetInspShortImage(Ipvm::Image16s& image)
{
    return GetShortImage(image, m_inspSizeX, m_inspSizeY);
}

bool VisionReusableMemory::GetInspUShortImage(Ipvm::Image16u& image)
{
    return GetUShortImage(image, m_inspSizeX, m_inspSizeY);
}

bool VisionReusableMemory::GetInspWordImage(Ipvm::Image16u& image)
{
    return GetWordImage(image, m_inspSizeX, m_inspSizeY);
}

bool VisionReusableMemory::GetInspColorImage(Ipvm::Image8u3& image)
{
    return GetColorImage(image, m_inspSizeX, m_inspSizeY);
}

bool VisionReusableMemory::GetInspByteImage(Ipvm::Image8u& image)
{
    return GetByteImage(image, m_inspSizeX, m_inspSizeY);
}

BYTE* VisionReusableMemory::GetByteMemory()
{
    return (BYTE*)m_reusable_image->alloc_byte_image_size_buffer();
}

void VisionReusableMemory::Release_ByteMemory(BYTE* buffer)
{
    m_reusable_image->free(buffer);
}

Ipvm::BlobInfo* VisionReusableMemory::GetBlobInfo(long nOrder)
{
    if (nOrder >= (long)m_vecBlobInfo.size())
    {
        SetReserveBlobInfo(nOrder + 1);
    }

    return m_vecBlobInfo[nOrder];
}

BlobAttribute* VisionReusableMemory::GetBlobAttribute(long nOrder)
{
    if (nOrder >= (long)m_vecBlobAttribute.size())
    {
        SetReserveBlobAttribute(nOrder + 1);
    }

    return m_vecBlobAttribute[nOrder];
}

CPI_Blob* VisionReusableMemory::GetBlob(long nOrder)
{
    if (nOrder >= (long)m_vecBlob.size())
    {
        SetReserveBlob(nOrder + 1);
    }

    return m_vecBlob[nOrder];
}

CPI_EdgeDetect* VisionReusableMemory::GetEdgeDetect()
{
    if (m_pEdgeDetect == nullptr)
    {
        m_pEdgeDetect = new CPI_EdgeDetect;
    }

    return m_pEdgeDetect;
}

SurfaceLayerMask* VisionReusableMemory::AddSurfaceLayerMaskClass(LPCTSTR name)
{
    if (m_alignSuccess == false)
    {
        ASSERT(!_T("Align 결과를 받기도 전에 SurfaceLayerMask를 세팅할 수 없다"));
        return nullptr;
    }

    if (m_mapSurfaceLayerRoiNameToIndex.find(name) != m_mapSurfaceLayerRoiNameToIndex.end())
    {
        ASSERT(!_T("Roi로 이미 등록되어있는 Layer이다"));
        return nullptr;
    }

    if (m_mapSurfaceLayerMaskNameToIndex.find(name) == m_mapSurfaceLayerMaskNameToIndex.end())
    {
        m_mapSurfaceLayerMaskNameToIndex[name] = long(m_vecSurfaceLayerMask.size());

        auto* layerMask = new SurfaceLayerMask(this);
        if (!layerMask->Ready(Ipvm::Point32r2(m_bodyCenterX, m_bodyCenterY)))
        {
            delete layerMask;
            return nullptr;
        }

        m_vecSurfaceLayerMask.emplace_back(name, layerMask);
    }

    return m_vecSurfaceLayerMask[m_mapSurfaceLayerMaskNameToIndex[name]].second;
}

long VisionReusableMemory::GetSurfaceLayerMaskCount() const
{
    return long(m_vecSurfaceLayerMask.size());
}

const Ipvm::Image8u& VisionReusableMemory::GetSurfaceLayerMask(long index) const
{
    return m_vecSurfaceLayerMask[index].second->GetMask();
}

LPCTSTR VisionReusableMemory::GetSurfaceLayerMaskName(long index) const
{
    return m_vecSurfaceLayerMask[index].first;
}

const Ipvm::Image8u* VisionReusableMemory::GetSurfaceLayerMask(LPCTSTR name) const
{
    auto itIndex = m_mapSurfaceLayerMaskNameToIndex.find(name);
    if (itIndex == m_mapSurfaceLayerMaskNameToIndex.end())
        return nullptr;

    return &m_vecSurfaceLayerMask[itIndex->second].second->GetMask();
}

void VisionReusableMemory::DelSurfaceLayerMask(long index)
{
    delete m_vecSurfaceLayerMask[index].second;
    m_vecSurfaceLayerMask.erase(m_vecSurfaceLayerMask.begin() + index);

    m_mapSurfaceLayerMaskNameToIndex.clear();

    for (long check = 0; check < long(m_vecSurfaceLayerMask.size()); check++)
    {
        m_mapSurfaceLayerMaskNameToIndex[m_vecSurfaceLayerMask[check].first] = check;
    }
}

SurfaceLayerRoi* VisionReusableMemory::AddSurfaceLayerRoiClass(LPCTSTR name)
{
    if (m_alignSuccess == false)
    {
        ASSERT(!_T("Align 결과를 받기도 전에 SurfaceLayerRoi를 세팅할 수 없다"));
        return nullptr;
    }

    if (m_mapSurfaceLayerMaskNameToIndex.find(name) != m_mapSurfaceLayerMaskNameToIndex.end())
    {
        ASSERT(!_T("Mask로 이미 등록되어있는 Layer이다"));
        return nullptr;
    }

    if (m_mapSurfaceLayerRoiNameToIndex.find(name) == m_mapSurfaceLayerRoiNameToIndex.end())
    {
        m_mapSurfaceLayerRoiNameToIndex[name] = long(m_vecSurfaceLayerRoi.size());

        auto* layerRoi = new SurfaceLayerRoi(this);
        if (!layerRoi->Ready(Ipvm::Point32r2(m_bodyCenterX, m_bodyCenterY)))
        {
            delete layerRoi;
            return nullptr;
        }

        m_vecSurfaceLayerRoi.emplace_back(name, layerRoi);
    }

    return m_vecSurfaceLayerRoi[m_mapSurfaceLayerRoiNameToIndex[name]].second;
}

long VisionReusableMemory::GetSurfaceLayerRoiCount() const
{
    return long(m_vecSurfaceLayerRoi.size());
}

const SurfaceLayerRoi& VisionReusableMemory::GetSurfaceLayerRoiClass(long index) const
{
    return *m_vecSurfaceLayerRoi[index].second;
}

LPCTSTR VisionReusableMemory::GetSurfaceLayerRoiName(long index) const
{
    return m_vecSurfaceLayerRoi[index].first;
}

const SurfaceLayerRoi* VisionReusableMemory::GetSurfaceLayerRoiClass(LPCTSTR name) const
{
    auto itIndex = m_mapSurfaceLayerRoiNameToIndex.find(name);
    if (itIndex == m_mapSurfaceLayerRoiNameToIndex.end())
        return nullptr;

    return m_vecSurfaceLayerRoi[itIndex->second].second;
}

void VisionReusableMemory::ResetInspectionInfo()
{
    m_alignSuccess = false;
    m_mapSurfaceLayerMaskNameToIndex.clear();

    for (auto& layer : m_vecSurfaceLayerMask)
    {
        delete layer.second;
    }
    m_vecSurfaceLayerMask.clear();

    m_mapSurfaceLayerRoiNameToIndex.clear();

    for (auto& layer : m_vecSurfaceLayerRoi)
    {
        delete layer.second;
    }
    m_vecSurfaceLayerRoi.clear();
}
