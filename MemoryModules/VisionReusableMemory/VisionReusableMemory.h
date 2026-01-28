#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class BlobAttribute;
class ReusableMemory_Image;
class SurfaceLayerMask;
class SurfaceLayerRoi;

//HDR_6_________________________________ Header body
//
class __VISION_REUSABLE_MEMORY_CLASS__ VisionReusableMemory
{
public:
    VisionReusableMemory(void);
    ~VisionReusableMemory(void);

    void SetBufferImageSize(long maxSizeX, long maxSizeY, long nVisionType = 0);
    void SetInspImageSize(long sizeX, long sizeY);

    int GetMaxBufferImageSizeX() const;
    int GetMaxBufferImageSizeY() const;
    int GetInspImageSizeX() const;
    int GetInspImageSizeY() const;

    Ipvm::BlobInfo* GetBlobInfo(long nOrder = 0);
    BlobAttribute* GetBlobAttribute(long nOrder = 0);
    CPI_Blob* GetBlob(long nOrder = 0);
    CPI_EdgeDetect* GetEdgeDetect();

    SurfaceLayerMask* AddSurfaceLayerMaskClass(LPCTSTR name);
    long GetSurfaceLayerMaskCount() const;
    const Ipvm::Image8u& GetSurfaceLayerMask(long index) const;
    LPCTSTR GetSurfaceLayerMaskName(long index) const;
    const Ipvm::Image8u* GetSurfaceLayerMask(LPCTSTR name) const;
    void DelSurfaceLayerMask(long index);

    SurfaceLayerRoi* AddSurfaceLayerRoiClass(LPCTSTR name);
    long GetSurfaceLayerRoiCount() const;
    const SurfaceLayerRoi& GetSurfaceLayerRoiClass(long index) const;
    LPCTSTR GetSurfaceLayerRoiName(long index) const;
    const SurfaceLayerRoi* GetSurfaceLayerRoiClass(LPCTSTR name) const;

    void ResetInspectionInfo();

    void SetAlignInfo(const Ipvm::Point32r2& bodyCenter, float packageAngle_deg);
    bool GetFloatImage(Ipvm::Image32r& image, long sizeX, long sizeY);
    bool GetDoubleImage(Ipvm::Image64r& image, long sizeX, long sizeY);
    bool GetLongImage(Ipvm::Image32s& image, long sizeX, long sizeY);
    bool GetShortImage(Ipvm::Image16s& image, long sizeX, long sizeY);
    bool GetUShortImage(Ipvm::Image16u& image, long sizeX, long sizeY);
    bool GetWordImage(Ipvm::Image16u& image, long sizeX, long sizeY);
    bool GetColorImage(Ipvm::Image8u3& image, long sizeX, long sizeY);
    bool GetByteImage(Ipvm::Image8u& image, long sizeX, long sizeY);

    bool GetInspFloatImage(Ipvm::Image32r& image);
    bool GetInspDoubleImage(Ipvm::Image64r& image);
    bool GetInspLongImage(Ipvm::Image32s& image);
    bool GetInspShortImage(Ipvm::Image16s& image);
    bool GetInspUShortImage(Ipvm::Image16u& image);
    bool GetInspWordImage(Ipvm::Image16u& image);
    bool GetInspColorImage(Ipvm::Image8u3& image);
    bool GetInspByteImage(Ipvm::Image8u& image);

    BYTE* GetByteMemory();
    void Release_ByteMemory(BYTE* buffer);

private:
    void SetReserveBlobInfo(long nNum);
    void SetReserveBlobAttribute(long nNum);
    void SetReserveBlob(long nNum);

public:
    CPI_EdgeDetect* m_pEdgeDetect;

    //Mark
    std::vector<Ipvm::BlobInfo*> m_vecBlobInfo;
    std::vector<BlobAttribute*> m_vecBlobAttribute;
    std::vector<CPI_Blob*> m_vecBlob;

private:
    ReusableMemory_Image* m_reusable_image;
    std::map<CString, long> m_mapSurfaceLayerMaskNameToIndex;
    std::vector<std::pair<CString, SurfaceLayerMask*>> m_vecSurfaceLayerMask;

    std::map<CString, long> m_mapSurfaceLayerRoiNameToIndex;
    std::vector<std::pair<CString, SurfaceLayerRoi*>> m_vecSurfaceLayerRoi;

    // Align 정보
    bool m_alignSuccess;
    float m_bodyCenterX;
    float m_bodyCenterY;
    float m_packageAngle_deg;

    // Image Size
    int m_maxBufferSizeX;
    int m_maxBufferSizeY;
    int m_inspSizeX;
    int m_inspSizeY;
};
