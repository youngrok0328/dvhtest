//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionMainAgent.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DebugOptionData.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ImageSaveOption.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ImageSaveOptionData.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionMainAgent::VisionMainAgent(int32_t mainType)
    : m_ImageSaveOption(new DebugOptionData)
    , m_DebugStopOption(new DebugOptionData)
    , m_bMarkTeach(FALSE)
    , m_pMessageSocket(nullptr)
    , m_tuningMessageSocket(nullptr)
    , m_hwndMainFrame(NULL)
    , m_msgShowTeach(0)
    , m_msgShowMain(0)
    , m_msgAccessModeChanged(0)
    , m_mainType(mainType)
{
    m_saveOption.m_bisSaveReviewImage = FALSE;
    m_saveOption.m_bisSaveCroppingImage = FALSE;
    m_saveOption.m_vecRawImageSaveOption.clear();
    m_saveOption.m_bReviewImageOption_Pass = FALSE;
    m_saveOption.m_bReviewImageOption_Reject = FALSE;
    m_saveOption.m_bReviewImageOption_Marginal = FALSE;
    m_saveOption.m_nReviewImageROI = ImageSaveOption::ReviewROI_Not;
    m_saveOption.m_bReviewImageCombined = FALSE;
    m_saveOption.m_nReviewImageQuality = 30;
    m_saveOption.m_nReviewImageMargin = 100;
    m_saveOption.m_nCroppingImageSaveOption = ImageSaveOption::CroppingImage_NotPass;
    m_saveOption.m_nCroppingImageROI = ImageSaveOption::CroppingROI_Not;
    m_saveOption.m_nCroppingImageFrame_Ball = ImageSaveOption::CroppingFrame_NotSave;
    m_saveOption.m_nCroppingImageFrame_Land = ImageSaveOption::CroppingFrame_NotSave;
    m_saveOption.m_nCroppingImageFrame_Comp = ImageSaveOption::CroppingFrame_NotSave;
    m_saveOption.m_nCroppingImageFrame_Surface = ImageSaveOption::CroppingFrame_NotSave;
    m_saveOption.m_nCroppingImageQuality = 30;
}

VisionMainAgent::~VisionMainAgent()
{
    delete m_tuningMessageSocket;
    delete m_pMessageSocket;
    delete m_DebugStopOption;
    delete m_ImageSaveOption;
}
