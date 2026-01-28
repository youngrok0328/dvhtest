//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Constants.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingCarrierTapeAlign.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Constants::Constants(VisionProcessingCarrierTapeAlign* processor)
    : m_processor(processor)
{
}

bool Constants::Make()
{
    const auto& image = m_processor->getImageLot().GetImageFrame(0, m_processor->GetCurVisionModule_Status());
    if (image.GetMem() == nullptr)
        return false;

    m_rawImageSizeX = image.GetSizeX();
    m_rawImageSizeY = image.GetSizeY();
    m_rawImageCenter.m_x = static_cast<float>(m_rawImageSizeX) * 0.5f;
    m_rawImageCenter.m_y = static_cast<float>(m_rawImageSizeY) * 0.5f;

    m_sensorSizeX = m_processor->getImageLot().GetSensorSizeX();
    m_sensorSizeY = m_processor->getImageLot().GetSensorSizeY();

    return true;
}
