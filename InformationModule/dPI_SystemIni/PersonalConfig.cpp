//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PersonalConfig.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedComponent/Persistence/IniHelper.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/Miscellaneous.h>
#include <Shlobj.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
PersonalConfig::PersonalConfig()
    : m_isPersonalInstallationEnabled(false)
    , m_multicoreCount(1)
    , m_imageSampling(1)
    , m_isSocketConnectionEnabled(true)
    , m_sequenceIntervalTimeInBatchInspection(2000)
    , m_isSaveReviewImageOptionEnableInBatchInspection(FALSE)

    , m_isToUseNearestDistortionXEnabled(FALSE)
    , m_isSaveCropImageOptionEnableInBatchInspection(FALSE)
    , m_matrixClientMode(FALSE)
{
    // Install 정보를 확인한다
    {
        TCHAR* appDataPath = nullptr;
        SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appDataPath);

        CString installFolder;
        installFolder.Format(_T("%s\\iPIS-500I Vision"), appDataPath);
        Ipvm::CreateDirectories(installFolder);

        m_configFilePath.Format(_T("%s\\Config.ini"), (LPCTSTR)installFolder);

        CoTaskMemFree(appDataPath);

        m_isPersonalInstallationEnabled
            = GetPrivateProfileInt(_T("Computer"), _T("personalInstallation"), 0, m_configFilePath);

        if (m_isPersonalInstallationEnabled)
        {
            m_multicoreCount = GetPrivateProfileInt(_T("Computer"), _T("personalMulticore"), 1, m_configFilePath);
            m_imageSampling = GetPrivateProfileInt(_T("Computer"), _T("imageSampling"), 1, m_configFilePath);
            m_isSocketConnectionEnabled
                = GetPrivateProfileInt(_T("Computer"), _T("socketConnection"), 1, m_configFilePath) ? true : false;
        }

        m_lastDirectory_Job = IniHelper::LoadSTRING(m_configFilePath, _T("Last Directory"), _T("Job"), _T(""));
        m_lastDirectory_Image = IniHelper::LoadSTRING(m_configFilePath, _T("Last Directory"), _T("Image"), _T(""));

        m_sequenceIntervalTimeInBatchInspection
            = IniHelper::LoadINT(m_configFilePath, _T("Batch Inspection"), _T("Sequence Interval Time"), 2000);
        m_isSaveReviewImageOptionEnableInBatchInspection
            = IniHelper::LoadINT(m_configFilePath, _T("Batch Inspection"), _T("Save Review"), 0);

        // 개발관련 파라메터
        m_isToUseNearestDistortionXEnabled
            = IniHelper::LoadINT(m_configFilePath, _T("Distortion X"), _T("To Use Nearest DistortionX Enabled"), 0)
            ? TRUE
            : FALSE;
        m_isSaveCropImageOptionEnableInBatchInspection
            = IniHelper::LoadINT(m_configFilePath, _T("Batch Inspection"), _T("Save Crop"), 0);
        m_matrixClientMode = IniHelper::LoadINT(m_configFilePath, _T("Matrix"), _T("Client Mode"), 0);
        m_matrixClientProgramPath
            = IniHelper::LoadSTRING(m_configFilePath, _T("Matrix"), _T("Client Program Path"), _T(""));
    }
}

PersonalConfig::~PersonalConfig(void)
{
}

PersonalConfig& PersonalConfig::getInstance()
{
    static PersonalConfig singleton;

    return singleton;
}

bool PersonalConfig::isPersonalInstallationEnabled() const
{
    return m_isPersonalInstallationEnabled;
}

long PersonalConfig::getMultiCoreCount() const
{
    return m_multicoreCount;
}

long PersonalConfig::getImageSampling() const
{
    return m_imageSampling;
}

void PersonalConfig::setImageSampling(long sampling)
{
    if (!m_isPersonalInstallationEnabled)
        return;

    m_imageSampling = sampling;

    IniHelper::SaveINT(m_configFilePath, _T("Computer"), _T("imageSampling"), m_imageSampling);
}

bool PersonalConfig::isSocketConnectionEnabled() const
{
    return m_isSocketConnectionEnabled;
}

void PersonalConfig::setSocketConnectionEnabled(bool enable)
{
    if (!m_isPersonalInstallationEnabled)
        return;

    m_isSocketConnectionEnabled = enable;
    IniHelper::SaveINT(m_configFilePath, _T("Computer"), _T("socketConnection"), m_isSocketConnectionEnabled);
}

CString PersonalConfig::getLastAccessDirectory_Job()
{
    return m_lastDirectory_Job;
}

void PersonalConfig::setLastAccessDirectory_Job(LPCTSTR directory)
{
    m_lastDirectory_Job = directory;
    IniHelper::SaveSTRING(m_configFilePath, _T("Last Directory"), _T("Job"), m_lastDirectory_Job);
}

CString PersonalConfig::getLastAccessDirectory_Image()
{
    return m_lastDirectory_Image;
}

void PersonalConfig::setLastAccessDirectory_Image(LPCTSTR directory)
{
    m_lastDirectory_Image = directory;
    IniHelper::SaveSTRING(m_configFilePath, _T("Last Directory"), _T("Image"), m_lastDirectory_Image);
}

BOOL PersonalConfig::isSaveReviewImageOptionEnableInBatchInspection() const
{
    return m_isSaveReviewImageOptionEnableInBatchInspection;
}

int PersonalConfig::getSequenceIntervalTimeInBatchInspection() const
{
    return m_sequenceIntervalTimeInBatchInspection;
}

void PersonalConfig::setSequenceIntervalTimeInBatchInspection(long milisecond)
{
    m_sequenceIntervalTimeInBatchInspection = milisecond;
    IniHelper::SaveINT(m_configFilePath, _T("Batch Inspection"), _T("Sequence Interval Time"),
        m_sequenceIntervalTimeInBatchInspection);
}

BOOL PersonalConfig::isToUseNearestDistortionXEnabled() const
{
    return m_isToUseNearestDistortionXEnabled;
}

BOOL PersonalConfig::isSaveCropImageOptionEnableInBatchInspection() const
{
    return m_isSaveCropImageOptionEnableInBatchInspection;
}

BOOL PersonalConfig::isMatrixClientMode() const
{
    return m_matrixClientMode;
}

LPCTSTR PersonalConfig::getMatrixClientProgramPath() const
{
    return m_matrixClientProgramPath;
}
