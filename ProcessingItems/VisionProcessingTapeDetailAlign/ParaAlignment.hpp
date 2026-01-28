#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ParaEdgeParameters.h"
#include "ParaEdgeSearchRoi.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <array>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class VisionProcessing;

//HDR_6_________________________________ Header body
//

template<typename Base>
class ParaAlignment : public Base
{
public:
    ParaAlignment(VisionProcessing& parent)
        : m_imageFrameIndex(parent, false)
        , m_imageProc(parent)
    {
    }

    VisionInspFrameIndex m_imageFrameIndex;
    ImageProcPara m_imageProc;

    void Init()
    {
        m_imageFrameIndex.setFrameIndex(0);
        m_imageProc.Init();
        *((Base*)this) = {};
    }
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db) override
    {
        if (!bSave)
        {
            this->Init();
        }

        m_imageFrameIndex.LinkDataBase(bSave, db[_T("ImageFrameIndex")]);
        m_imageProc.LinkDataBase(bSave, db[_T("ImageProc")]);

        return Base::LinkDataBase(bSave, db);
    }
};
