//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Config.h"

//CPP_2_________________________________ This project's headers
#include "SystemConfig.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
int Config::getVisionType()
{
    return SystemConfig::GetInstance().GetVisionType();
}

int Config::getHandlerType()
{
    return SystemConfig::GetInstance().GetHandlerType();
}
