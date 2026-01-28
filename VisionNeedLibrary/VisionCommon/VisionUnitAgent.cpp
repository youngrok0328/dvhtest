//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionUnitAgent.h"

//CPP_2_________________________________ This project's headers
#include "VisionMainAgent.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionUnitAgent::VisionUnitAgent(SystemConfig& systemConfig, VisionMainAgent& main)
    : m_systemConfig(systemConfig)
    , m_currentProcessingModule(nullptr)
    , m_main(main)
{
}

const VisionTapeSpec* VisionUnitAgent::getTapeSpec() const
{
    return nullptr;
}

VisionTapeSpec* VisionUnitAgent::getTapeSpec()
{
    return nullptr;
}

const wchar_t* VisionUnitAgent::GetJobFileName() const
{
    return m_main.m_strJobFileName;
}
