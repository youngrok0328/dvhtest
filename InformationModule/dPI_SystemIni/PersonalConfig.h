#pragma once
//========================================================================================
// 개인 설정에 관련된 내용이다.
// : 해당정보는 Config가 아닌 컴퓨터의 개인공간에 저장된다.
// : 설정과 상관없는 단순 편리 기능 관련된 정보이외의 정보를 저장하여서
// : 테스트시 편리하게 작업하기 위함이다.
//
// 또한 개발도중의 기능들을 장비에서 테스트하고 싶은데 정식 기능도 여기다 넣겠다.
// : 옵션이 정식으로 넣게 되었을 때는 여기서 나중에 제외시키자.
//========================================================================================

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DPI_SYSTEMINISLITDEF_API PersonalConfig
{
public:
    ~PersonalConfig(void);

    static PersonalConfig& getInstance();

    //----------------------------------------------------------------------------------------------
    // Personal Installation 관련 - 개인 설정
    //----------------------------------------------------------------------------------------------

    bool isPersonalInstallationEnabled() const;
    long getMultiCoreCount() const;
    long getImageSampling() const;
    void setImageSampling(long sampling);
    bool isSocketConnectionEnabled() const;
    void setSocketConnectionEnabled(bool enable);

    //----------------------------------------------------------------------------------------------
    // 편리 관련 - 개인 설정
    //----------------------------------------------------------------------------------------------

    CString getLastAccessDirectory_Job();
    CString getLastAccessDirectory_Image();
    void setLastAccessDirectory_Job(LPCTSTR directory);
    void setLastAccessDirectory_Image(LPCTSTR directory);
    BOOL isSaveReviewImageOptionEnableInBatchInspection() const;
    int getSequenceIntervalTimeInBatchInspection() const;
    void setSequenceIntervalTimeInBatchInspection(long milisecond);

    //----------------------------------------------------------------------------------------------
    // 개발 테스트 관련 - 개인 설정
    //----------------------------------------------------------------------------------------------

    BOOL isToUseNearestDistortionXEnabled() const;
    BOOL isSaveCropImageOptionEnableInBatchInspection() const;
    BOOL isMatrixClientMode() const;
    LPCTSTR getMatrixClientProgramPath() const;

private:
    CString m_configFilePath;

    //----------------------------------------------------------------------------------------------
    // Personal Installation 관련 - 개인 설정
    //----------------------------------------------------------------------------------------------

    bool m_isPersonalInstallationEnabled; // Install 시 개인용도로 설치함
    long m_multicoreCount; // Install 시 설정한 Multicore 수
    int m_imageSampling; // 노트북에서 Image Open이 힘든 사람들을 위한 Sampling 파라메터
    bool m_isSocketConnectionEnabled; // Socket 접속을 허용하겠는가

    //----------------------------------------------------------------------------------------------
    // 편리 관련 - 개인 설정
    //----------------------------------------------------------------------------------------------

    CString m_lastDirectory_Job; // 마지막으로 Open한 Job 폴더
    CString m_lastDirectory_Image; // 마지막으로 Open한 Image 폴더
    int m_sequenceIntervalTimeInBatchInspection; // Batch Inspection 내에서 Seqeuence Interval Time
    BOOL m_isSaveReviewImageOptionEnableInBatchInspection; // Batch Inspection 내에서 Review 이미지를 저장하는가?

    //----------------------------------------------------------------------------------------------
    // 개발 테스트 관련 - 개인 설정
    //----------------------------------------------------------------------------------------------

    BOOL m_isToUseNearestDistortionXEnabled;
    BOOL m_isSaveCropImageOptionEnableInBatchInspection; // Batch Inspection 내에서 Crop 이미지를 저장하는가?
    BOOL m_matrixClientMode; // Matrix Client Mode (개발용)
    CString m_matrixClientProgramPath; // Matrix Client 프로그램 Path

    PersonalConfig();
};
