#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "FOVImageGrabber.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>

//HDR_4_________________________________ Standard library headers
#include <memory>
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
struct Grab3DVerification
{
    std::vector<float> vecZValueData;
    std::vector<unsigned short> vecIntensityData;

    void InitData()
    {
        vecZValueData.clear();
        vecIntensityData.clear();
    }

    void CopyData(std::vector<float> i_vecZValueData, std::vector<unsigned short> i_vecIntensityData)
    {
        InitData();

        for (const auto& data : i_vecZValueData)
        {
            vecZValueData.push_back(data);
        }

        for (const auto& data : i_vecIntensityData)
        {
            vecIntensityData.push_back(data);
        }
    }
};

struct sGrab3DErrorLogMessageFormat;
struct iGrab3D_Error_LOG;

class FOVImageGrabber3D : public FOVImageGrabber
{
public:
    FOVImageGrabber3D(CVisionMain* visionMain, long i_bufferSize = 0);
    virtual ~FOVImageGrabber3D();

    void allocateBuffer(long logicalScanLength_px);

protected:
    class FOVBuffer
    {
    public:
        float* m_bufferFloat;
        unsigned short* m_bufferShort;

        Ipvm::Image32r m_imageZmap;
        Ipvm::Image16u m_imageVmap;
        float m_heightRangeMin;
        float m_heightRangeMax;

        FOVBuffer()
            : m_bufferFloat(nullptr)
            , m_bufferShort(nullptr)
        {
        }

        ~FOVBuffer()
        {
            delete[] m_bufferFloat;
            delete[] m_bufferShort;
        }
    };

    std::vector<std::shared_ptr<FOVBuffer>> m_buffers;
    long m_bufferSize;

    //kk 3D Image Verification 변수 (이전 Grab Image 정보가 담겨져 있음)
    Grab3DVerification m_imageVerifyData;

    //kk GPU Error로 인한 Image에 문제가 있는지 확인하는 함수
    bool grab3D_Image_Verification(
        const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap, Ipvm::AsyncProgress* progress);
    bool save_VerifyReport(std::vector<float> i_vecZValueData, std::vector<unsigned short> i_vecIntensityData);

    //mc_GPU Test로 인한 연산시간 기록
    struct Inline3D_GrabTime
    {
        float m_fWaitGrabEnd_Time;
        float m_fWaitCalcEnd_Time;
        bool m_bWatiGrabEndSuccess;
        bool m_bWatiCalcEndSuccess;
        long m_nTrayidx;
        long m_nFovidx;
        bool m_bCameraGrabSuccess;

        void InitData()
        {
            m_fWaitGrabEnd_Time = 0.f;
            m_fWaitCalcEnd_Time = 0.f;
            m_nTrayidx = -1;
            m_nFovidx = -1;
            m_bWatiGrabEndSuccess = false;
            m_bWatiCalcEndSuccess = false;
            m_bCameraGrabSuccess = true;
        }
    };

    Inline3D_GrabTime m_sInline3D_GrabTime;
    std::vector<Inline3D_GrabTime> m_vecsInline3D_GrabTime;

    virtual bool buffer_grab(long bufferIndex, const FOVID& fovID,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual void buffer_paste(long bufferIndex, long stitchIndex, long stitchCountX, long stitchCountY,
        std::shared_ptr<VisionImageLot> inspectionImage) override;
    virtual void ClearImageGrabber_GrabTime() override;
    virtual void SaveImageGrabber_GrabTime(const CString i_strSavePath) override;

private:
    void Send_Grab3D_Error_LogMessage(
        const CString i_strCategory, const sGrab3DErrorLogMessageFormat i_sGrab3DErrorForamt);
    void IncreaseGrabFailCount(const bool i_bisGrabRetryMode, const iGrab3D_Error_LOG i_sErrorLog);
};
