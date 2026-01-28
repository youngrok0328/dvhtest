#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionProcessing;
class CiDataBase;
class CXTPPropertyGridItem;
class CGridCtrl;
class CGridSlideBox;

//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionInspFrameIndex
{
public:
    VisionInspFrameIndex(const VisionInspFrameIndex& object) = delete;
    VisionInspFrameIndex(VisionProcessing& parent, BOOL useAllFrame);
    ~VisionInspFrameIndex();

    BOOL LinkDataBase(BOOL save, CiDataBase& db);
    long getFrameIndex() const;
    long getFrameListIndex() const;
    void setFrameIndex(long frameIndex);
    void setFrameListIndex(long index);
    bool useAllFrame() const;
    bool isValid() const;

    Ipvm::Image8u getImage(bool isRaw) const;
    CString getImageName() const;

    VisionInspFrameIndex& operator=(const VisionInspFrameIndex& object);

    CXTPPropertyGridItem* makePropertyGridItem(CXTPPropertyGridItem* parent, LPCTSTR caption, UINT id);

    CGridSlideBox* makeGridSlideBox(CGridCtrl* parent, long row, long col);
    bool fromGridSlideBox(CGridCtrl* parent, long row, long col);
    void toGridSlideBox(CGridCtrl* parent, long row, long col);

private:
    VisionProcessing& m_parent;
    BOOL m_useAllFrame;
    BOOL m_virtualMode;
    CString m_virtualKey;

    long m_index;
    long getFrameListIndex(long frameIndex) const;

    VisionInspFrameIndex(LPCTSTR key, BOOL isVirtual, VisionProcessing& parent, BOOL useAllFrame);

    VisionInspFrameIndex& getObject();
    const VisionInspFrameIndex& getObject() const;

    friend VisionProcessing;
};
