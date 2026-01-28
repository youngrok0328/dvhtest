//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspFrameIndex.h"

//CPP_2_________________________________ This project's headers
#include "VisionBaseDef.h"
#include "VisionImageLot.h"
#include "VisionProcessing.h"
#include "VisionUnitAgent.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridSlideBox.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspFrameIndex::VisionInspFrameIndex(VisionProcessing& parent, BOOL useAllFrame)
    : m_index(0)
    , m_parent(parent)
    , m_useAllFrame(useAllFrame)
    , m_virtualMode(TRUE)
    , m_virtualKey(::CreateGUID())
{
}

VisionInspFrameIndex::VisionInspFrameIndex(LPCTSTR key, BOOL isVirtual, VisionProcessing& parent, BOOL useAllFrame)
    : m_index(0)
    , m_parent(parent)
    , m_useAllFrame(useAllFrame)
    , m_virtualMode(isVirtual)
    , m_virtualKey(key)
{
}

VisionInspFrameIndex::~VisionInspFrameIndex()
{
}

BOOL VisionInspFrameIndex::LinkDataBase(BOOL save, CiDataBase& db)
{
    if (!save)
    {
        short index = 0;
        if (db.Link(save, index))
        {
            getObject().m_index = index;
            return TRUE;
        }
    }

    return db.Link(save, getObject().m_index);
}

long VisionInspFrameIndex::getFrameIndex() const
{
    auto& obj = getObject();

    if (obj.m_useAllFrame)
    {
        if (obj.m_index < 0 || obj.m_index >= m_parent.m_visionUnit.getIllumInfo2D().getTotalFrameCount())
        {
            return -1;
        }

        return obj.m_index;
    }

    return m_parent.GetImageFrameIndex(obj.m_index);
}

long VisionInspFrameIndex::getFrameListIndex() const
{
    auto& obj = getObject();

    if (obj.m_useAllFrame)
    {
        return getFrameListIndex(obj.m_index);
    }

    if (obj.m_index < 0 || obj.m_index >= m_parent.GetImageFrameCount())
    {
        return -1;
    }

    return obj.m_index;
}

void VisionInspFrameIndex::setFrameIndex(long frameIndex)
{
    auto& obj = getObject();

    if (obj.m_useAllFrame)
    {
        getObject().m_index = frameIndex;
        return;
    }

    getObject().m_index = getFrameListIndex(frameIndex);
}

void VisionInspFrameIndex::setFrameListIndex(long index)
{
    auto& obj = getObject();

    if (obj.m_useAllFrame)
    {
        auto& frameList = m_parent.GetImageFrameList();
        if (index < 0 || index >= long(frameList.size()))
        {
            obj.m_index = -1;
            return;
        }

        obj.m_index = frameList[index];
    }

    obj.m_index = index;
}

bool VisionInspFrameIndex::useAllFrame() const
{
    return m_useAllFrame;
}

bool VisionInspFrameIndex::isValid() const
{
    auto& obj = getObject();

    long frameIndex = -1;

    if (obj.m_useAllFrame)
    {
        frameIndex = obj.m_index;
    }
    else
    {
        if (obj.m_index < 0 || obj.m_index >= m_parent.GetImageFrameCount())
        {
            return false;
        }

        frameIndex = m_parent.GetImageFrameIndex(obj.m_index);
    }

    if (frameIndex < 0 || frameIndex >= m_parent.m_visionUnit.getIllumInfo2D().getTotalFrameCount())
    {
        return false;
    }

    return true;
}

Ipvm::Image8u VisionInspFrameIndex::getImage(bool isRaw) const
{
    long frameIndex = getFrameIndex();

    if (isRaw)
    {
        if (frameIndex < 0 || frameIndex >= m_parent.m_visionUnit.getIllumInfo2D().getTotalFrameCount())
        {
            return Ipvm::Image8u();
        }

        return m_parent.getImageLot().GetImageFrame(frameIndex, m_parent.GetCurVisionModule_Status());
    }

    return m_parent.GetInspectionFrameImage(FALSE, getFrameIndex());
}

CString VisionInspFrameIndex::getImageName() const
{
    return m_parent.getIllumInfo2D().getIllumFullName(getFrameIndex());
}

VisionInspFrameIndex& VisionInspFrameIndex::operator=(const VisionInspFrameIndex& object)
{
    if (object.m_useAllFrame)
    {
        setFrameIndex(object.getFrameIndex());
    }
    else
    {
        setFrameListIndex(object.getFrameListIndex());
    }

    return *this;
}

CXTPPropertyGridItem* VisionInspFrameIndex::makePropertyGridItem(CXTPPropertyGridItem* parent, LPCTSTR caption, UINT id)
{
    auto* itemSelectFrameID
        = parent->AddChildItem(new CXTPPropertyGridItemEnum(caption, getObject().m_index, (int*)&getObject().m_index));

    if (m_useAllFrame)
    {
        long nFrameNum = m_parent.getImageLot().GetImageFrameCount();

        //조명 선택 여부와 관계 없이 모든 Frame을 사용 가능하도록 한다
        for (long frameIndex = 0; frameIndex < nFrameNum; frameIndex++)
        {
            CString name = m_parent.getIllumInfo2D().getIllumFullName(frameIndex);
            itemSelectFrameID->GetConstraints()->AddConstraint(name, frameIndex);
        }
    }
    else
    {
        for (long index = 0; index < m_parent.GetImageFrameCount(); index++)
        {
            long frameIndex = m_parent.GetImageFrameIndex(index);

            CString name = m_parent.getIllumInfo2D().getIllumFullName(frameIndex);
            itemSelectFrameID->GetConstraints()->AddConstraint(name, index);
        }
    }

    itemSelectFrameID->SetID(id);

    return itemSelectFrameID;
}

CGridSlideBox* VisionInspFrameIndex::makeGridSlideBox(CGridCtrl* parent, long row, long col)
{
    parent->SetCellType(row, col, RUNTIME_CLASS(CGridSlideBox));
    auto* itemControl = (CGridSlideBox*)parent->GetCell(row, col);
    long maxCount = 0;
    if (m_useAllFrame)
    {
        maxCount = m_parent.m_visionUnit.getIllumInfo2D().getTotalFrameCount();

        //조명 선택 여부와 관계 없이 모든 Frame을 사용 가능하도록 한다
        for (long frameIndex = 0; frameIndex < maxCount; frameIndex++)
        {
            CString name = m_parent.getIllumInfo2D().getIllumFullName(frameIndex);
            itemControl->AddString(name);
        }
    }
    else
    {
        maxCount = m_parent.GetImageFrameCount();
        for (long index = 0; index < maxCount; index++)
        {
            long frameIndex = m_parent.GetImageFrameIndex(index);

            CString name = m_parent.getIllumInfo2D().getIllumFullName(frameIndex);
            itemControl->AddString(name);
        }
    }

    if (maxCount == 0)
    {
        itemControl->AddString(_T("Not Setup"));
    }

    itemControl->SetCurSel(getObject().m_index);

    return itemControl;
}

bool VisionInspFrameIndex::fromGridSlideBox(CGridCtrl* parent, long row, long col)
{
    auto* itemControl = (CGridSlideBox*)parent->GetCell(row, col);

    long selectIndex = itemControl->GetCurSel();
    long maxCount
        = m_useAllFrame ? m_parent.m_visionUnit.getIllumInfo2D().getTotalFrameCount() : m_parent.GetImageFrameCount();

    if (selectIndex >= 0 && selectIndex < maxCount)
    {
        getObject().m_index = selectIndex;
        ((CGridSlideBox*)parent->GetCell(row, col))->SetCurSel(selectIndex);

        return true;
    }

    return false;
}

void VisionInspFrameIndex::toGridSlideBox(CGridCtrl* parent, long row, long col)
{
    if (m_useAllFrame)
    {
        ((CGridSlideBox*)parent->GetCell(row, col))->SetCurSel(getObject().getFrameIndex());
    }
    else
    {
        ((CGridSlideBox*)parent->GetCell(row, col))->SetCurSel(getObject().getFrameListIndex());
    }
}

long VisionInspFrameIndex::getFrameListIndex(long frameIndex) const
{
    for (long index = 0; index < m_parent.GetImageFrameCount(); index++)
    {
        if (m_parent.GetImageFrameIndex(index) == frameIndex)
        {
            return index;
        }
    }

    return -1;
}

VisionInspFrameIndex& VisionInspFrameIndex::getObject()
{
    if (m_virtualMode)
    {
        return m_parent.createInspFrame(m_virtualKey, m_useAllFrame);
    }

    return *this;
}

const VisionInspFrameIndex& VisionInspFrameIndex::getObject() const
{
    if (m_virtualMode)
    {
        return m_parent.createInspFrame(m_virtualKey, m_useAllFrame);
    }

    return *this;
}
