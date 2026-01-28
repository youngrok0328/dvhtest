#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//

#define MAX_2D_ILLUM_CHANNEL 16

// H : Host
// V : Vision
// 사전순 배열을 기준으로 한다.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 메세지 보낼 때, 메세지에 고유 번호를 매겨서 Token 에 넣어 보낸다.
enum class MSG_H2V : long
{
    GET_2D_SCALE,
    GET_3D_SCALE,
    GET_ILLUMINATION_TABLE_2D,
    GRAB_EACH_CHANNEL_2D,
    PREPARE_SCAN_3D,
    SET_2D_SCALE,
    SET_3D_SCALE,
    SET_ILLUMINATION_TABLE_2D,
    SET_SLITBEAM_CAMERA_OFFSET,
    SET_SLITBEAM_DISTORTION_PROFILE,
    SET_SLITBEAM_DISTORTION_FULL,
    SINGLE_GRAB_2D,
    SINGLE_GRAB_2D_UNIFORMITY_FOR_GV,
    SINGLE_GRAB_2D_UNIFORMITY_FOR_MS,
    SINGLE_SCAN_3D,
    START_LIVE_2D,
    START_LIVE_3D,
    STOP_LIVE,

    GET_ILLUMINATION_EXTENSION_COUNT,
    GET_ILLUMINATION_CAL_USE_ALLDATA,

    GET_WHITE_BALANCE_GAIN,
    SET_WHITE_BALANCE_GAIN,

    GET_UVIR_CHANNEL_IDX,
    GET_2D_CAMERA_OFFSET_Y,
    SET_2D_CAMERA_OFFSET_Y,

    SET_ILLUMINATION_MIRROR_TABLE_2D,
    SINGLE_GRAB_2D_CALIPER_TYPE_CHECK,

    GET_3D_SHEARING_FACTOR,
    SET_3D_SHEARING_FACTOR,

    _END,
};

struct MSG_H2V_DEF
{
    MSG_H2V_DEF(const MSG_H2V messageIndex, const long structSize)
        : m_messageIndex(messageIndex)
        , m_structSize(structSize)
        , m_messageToken(0)
        , m_messageLength(structSize)
    {
    }

    const MSG_H2V m_messageIndex;
    const long m_structSize;
    mutable UINT m_messageToken;
    size_t m_messageLength;
};

template<MSG_H2V messageIndex, typename ChildStructure>
struct MSG_H2V_TEMPLATE : public MSG_H2V_DEF
{
    MSG_H2V_TEMPLATE(const MSG_H2V_TEMPLATE& rhs) = default;
    MSG_H2V_TEMPLATE()
        : MSG_H2V_DEF(messageIndex, sizeof(ChildStructure))
    {
    }

    MSG_H2V_TEMPLATE& operator=(const MSG_H2V_TEMPLATE& rhs) = default;
};

#define DECLARE_H2V_MSG_STRUCT(MSG_NAME) \
    struct MSG_H2V_##MSG_NAME : public MSG_H2V_TEMPLATE<MSG_H2V::##MSG_NAME, MSG_H2V_##MSG_NAME>

DECLARE_H2V_MSG_STRUCT(GET_2D_SCALE)
{
    long m_nCurSideVisionidx;
};

DECLARE_H2V_MSG_STRUCT(GET_3D_SHEARING_FACTOR){};

DECLARE_H2V_MSG_STRUCT(GET_3D_SCALE){};

DECLARE_H2V_MSG_STRUCT(GET_ILLUMINATION_TABLE_2D)
{
    long m_nCurSideVisionidx;
};

DECLARE_H2V_MSG_STRUCT(GRAB_EACH_CHANNEL_2D)
{
    float m_illuminationTime_ms;
    float m_grabPeriod_sec;
    bool m_useForEachChannel[MAX_2D_ILLUM_CHANNEL];
    bool m_useIlluminationCalibration;
    long m_nCurSideVisionidx;
    bool m_bisColorimage;
    bool m_bisimageResize;
    bool m_bisGreenimage;
    bool m_bisVerification;
    BYTE m_bIllumVerificiationType;
};

DECLARE_H2V_MSG_STRUCT(PREPARE_SCAN_3D)
{
    bool m_useHeightScale;
    bool m_useHeightQuantization;
    bool m_useDistortionCompensation;
    BOOL m_cameraVerticalBinning;
    float m_illuminationTime_ms;
    BYTE m_illuminationValue;
    BYTE m_scanStep_um;
    double m_scanDepth_um;
    double m_scanLength_mm;
    double m_noiseThreshold;
    double m_noiseThreshold_FinalFiltering; //kircheis_SDK127
    BYTE m_beamThickness_um;
};

DECLARE_H2V_MSG_STRUCT(SET_2D_SCALE)
{
    float m_newScaleX;
    float m_newScaleY;
    long m_nCurSideVisionidx;
};

DECLARE_H2V_MSG_STRUCT(SET_3D_SHEARING_FACTOR)
{
    float m_newShearingFactor;
    bool m_applySystemFile;
};

DECLARE_H2V_MSG_STRUCT(SET_3D_SCALE)
{
    float m_newSlitbeamHeightScaling;
    bool m_applySystemFile;
};

DECLARE_H2V_MSG_STRUCT(SET_ILLUMINATION_TABLE_2D)
{
    size_t fileSize;
    long m_nCurSideVisionidx;
};

DECLARE_H2V_MSG_STRUCT(SET_SLITBEAM_CAMERA_OFFSET)
{
    int m_newSlitbeamCameraOffsetY;
};

DECLARE_H2V_MSG_STRUCT(SET_ILLUMINATION_MIRROR_TABLE_2D)
{
    size_t fileSize;
    long m_nCurSideVisionidx;
};

DECLARE_H2V_MSG_STRUCT(SINGLE_GRAB_2D_CALIPER_TYPE_CHECK)
{
    float m_illuminationTime_ms;
    float m_mean_gv;
    bool m_useIlluminationCompensation;
};

DECLARE_H2V_MSG_STRUCT(SET_SLITBEAM_DISTORTION_PROFILE)
{
    size_t fileSize;
};

DECLARE_H2V_MSG_STRUCT(SET_SLITBEAM_DISTORTION_FULL)
{
    double m_scanDepth_um;
    bool m_cameraVerticalBinning;
    size_t fileSize;
};

DECLARE_H2V_MSG_STRUCT(SINGLE_GRAB_2D)
{
    float m_illuminationTimes_ms[MAX_2D_ILLUM_CHANNEL];
    bool m_useIlluminationCompensation;
    long m_nCurSideVisionidx;
    bool m_bisColorimage;
    bool m_bisimageResize;
    bool m_bisGreenimage;
    bool m_bisWhiteBalanceFunction;
};

DECLARE_H2V_MSG_STRUCT(SINGLE_GRAB_2D_UNIFORMITY_FOR_GV)
{
    int m_channel_no;
    float m_illuminationIntensity_gv[MAX_2D_ILLUM_CHANNEL];
    bool m_useIlluminationCompensation;
    long m_nCurSideVisionidx;
    bool m_bisColorimage;
};

DECLARE_H2V_MSG_STRUCT(SINGLE_GRAB_2D_UNIFORMITY_FOR_MS)
{
    int m_channel_no;
    float m_illuminationIntensity_ms[MAX_2D_ILLUM_CHANNEL];
    bool m_useIlluminationCompensation;
    long m_nCurSideVisionidx;
    bool m_bisColorimage;
};

DECLARE_H2V_MSG_STRUCT(SINGLE_SCAN_3D)
{
    bool m_getVmap; // VMAP도 같이 받을 것인가?
};

DECLARE_H2V_MSG_STRUCT(START_LIVE_2D)
{
    float m_illuminationTimes_ms[MAX_2D_ILLUM_CHANNEL];
    bool m_useIlluminationCompensation;
    long m_nCurSideVisionidx;
    bool m_bisColorimage;
};

DECLARE_H2V_MSG_STRUCT(START_LIVE_3D)
{
    float m_illuminationTime_ms;
    BYTE m_illuminationValue;
};

DECLARE_H2V_MSG_STRUCT(STOP_LIVE)
{
    long m_nCurSideVisionidx;
};

DECLARE_H2V_MSG_STRUCT(GET_ILLUMINATION_EXTENSION_COUNT){};

DECLARE_H2V_MSG_STRUCT(GET_ILLUMINATION_CAL_USE_ALLDATA)
{
    long m_nCurSideVisionidx;
};

DECLARE_H2V_MSG_STRUCT(GET_WHITE_BALANCE_GAIN)
{
    bool m_isReverseGain;
};

DECLARE_H2V_MSG_STRUCT(SET_WHITE_BALANCE_GAIN)
{
    bool m_bisReverseGain;
    float m_newWhiteBalanceGainValue_R;
    float m_newWhiteBalanceGainValue_G;
    float m_newWhiteBalanceGainValue_B;
};

DECLARE_H2V_MSG_STRUCT(GET_UVIR_CHANNEL_IDX){};

DECLARE_H2V_MSG_STRUCT(GET_2D_CAMERA_OFFSET_Y)
{
    long m_nCurSideVisionidx;
};

DECLARE_H2V_MSG_STRUCT(SET_2D_CAMERA_OFFSET_Y)
{
    int m_nCameraOffsetY;
    long m_nCurSideVisionidx;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 응답 메세지일 경우에 호스트에서 온 토큰을 그대로 보내도록 한다.
enum class MSG_V2H : long
{
    // Acknowledges
    GET_2D_SCALE_ACK,
    GET_3D_SCALE_ACK,
    GET_ILLUMINATION_TABLE_2D_ACK,
    GRAB_EACH_CHANNEL_2D_ACK,
    PREPARE_SCAN_3D_ACK,
    SET_2D_SCALE_ACK,
    SET_3D_SCALE_ACK,
    SET_ILLUMINATION_TABLE_2D_ACK,
    SET_SLITBEAM_CAMERA_OFFSET_ACK,
    SET_SLITBEAM_DISTORTION_PROFILE_ACK,
    SET_SLITBEAM_DISTORTION_FULL_ACK,
    SINGLE_GRAB_2D_ACK,
    SINGLE_GRAB_2D_UNIFORMITY_FOR_GV_ACK,
    SINGLE_GRAB_2D_UNIFORMITY_FOR_MS_ACK,
    SINGLE_SCAN_3D_ACK,
    START_LIVE_2D_ACK,
    START_LIVE_3D_ACK,
    STOP_LIVE_ACK,

    // Async messages
    GRAB_EACH_CHANNEL_2D_IMAGE,
    LIVE_IMAGE,
    SINGLE_GRAB_IMAGE,
    SINGLE_ZMAP_IMAGE,

    GET_ILLUMINATION_EXTENSION_COUNT_ACK,
    GET_ILLUMINATION_CAL_USE_ALLDATA_ACK,

    GET_WHITE_BALANCE_GAIN_ACK,
    SET_WHITE_BALANCE_GAIN_ACK,

    GET_UVIR_CHANNEL_IDX_ACK,
    GET_2D_CAMERA_OFFSET_Y,
    SET_2D_CAMERA_OFFSET_Y,

    SET_ILLUMINATION_MIRROR_TABLE_2D_ACK,
    SINGLE_GRAB_2D_CALIPER_TYPE_CHECK_ACK,

    GET_3D_SHEARING_FACTOR_ACK,
    SET_3D_SHEARING_FACTOR_ACK,

    _END,
};

struct MSG_V2H_DEF
{
    MSG_V2H_DEF(const MSG_V2H messageIndex, const long structSize, UINT token)
        : m_messageIndex(messageIndex)
        , m_structSize(structSize)
        , m_messageToken(token)
        , m_messageLength(structSize)
    {
    }

    MSG_V2H m_messageIndex;
    long m_structSize;
    UINT m_messageToken;
    long m_messageLength;
};

template<MSG_V2H messageIndex, typename ChildStructure>
struct MSG_V2H_TEMPLATE : public MSG_V2H_DEF
{
    MSG_V2H_TEMPLATE(const MSG_V2H_TEMPLATE& rhs) = default;
    MSG_V2H_TEMPLATE(UINT token)
        : MSG_V2H_DEF(messageIndex, sizeof(ChildStructure), token)
    {
    }

    MSG_V2H_TEMPLATE& operator=(const MSG_V2H_TEMPLATE& rhs) = default;
};

struct MSG_V2H_GET_2D_SCALE_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::GET_2D_SCALE_ACK, MSG_V2H_GET_2D_SCALE_ACK>
{
    MSG_V2H_GET_2D_SCALE_ACK() = delete;
    MSG_V2H_GET_2D_SCALE_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    float m_currentScaleX;
    float m_currentScaleY;
    bool m_result;
};

struct MSG_V2H_GET_3D_SCALE_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::GET_3D_SCALE_ACK, MSG_V2H_GET_3D_SCALE_ACK>
{
    MSG_V2H_GET_3D_SCALE_ACK() = delete;
    MSG_V2H_GET_3D_SCALE_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
    float m_scaleZ;
    float m_slitbeamHeightScaling;
    int m_slitbeamCameraOffsetY;
};

struct MSG_V2H_GET_3D_SHEARING_FACTOR_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::GET_3D_SHEARING_FACTOR_ACK, MSG_V2H_GET_3D_SHEARING_FACTOR_ACK>
{
    MSG_V2H_GET_3D_SHEARING_FACTOR_ACK() = delete;
    MSG_V2H_GET_3D_SHEARING_FACTOR_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
    float m_ShearingFactor;
};

struct MSG_V2H_GET_ILLUMINATION_TABLE_2D_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::GET_ILLUMINATION_TABLE_2D_ACK, MSG_V2H_GET_ILLUMINATION_TABLE_2D_ACK>
{
    MSG_V2H_GET_ILLUMINATION_TABLE_2D_ACK() = delete;
    MSG_V2H_GET_ILLUMINATION_TABLE_2D_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    size_t m_size;
    bool m_result;
};

struct MSG_V2H_GRAB_EACH_CHANNEL_2D_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::GRAB_EACH_CHANNEL_2D_ACK, MSG_V2H_GRAB_EACH_CHANNEL_2D_ACK>
{
    MSG_V2H_GRAB_EACH_CHANNEL_2D_ACK() = delete;
    MSG_V2H_GRAB_EACH_CHANNEL_2D_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_PREPARE_SCAN_3D_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::PREPARE_SCAN_3D_ACK, MSG_V2H_PREPARE_SCAN_3D_ACK>
{
    MSG_V2H_PREPARE_SCAN_3D_ACK() = delete;
    MSG_V2H_PREPARE_SCAN_3D_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_scanVelocity_mm_per_sec(0.)
        , m_result(false)
    {
    }

    double m_scanVelocity_mm_per_sec;
    bool m_result;
};

struct MSG_V2H_SET_2D_SCALE_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::SET_2D_SCALE_ACK, MSG_V2H_SET_2D_SCALE_ACK>
{
    MSG_V2H_SET_2D_SCALE_ACK() = delete;
    MSG_V2H_SET_2D_SCALE_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SET_3D_SCALE_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::SET_3D_SCALE_ACK, MSG_V2H_SET_3D_SCALE_ACK>
{
    MSG_V2H_SET_3D_SCALE_ACK() = delete;
    MSG_V2H_SET_3D_SCALE_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SET_3D_SHEARING_FACTOR_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SET_3D_SHEARING_FACTOR_ACK, MSG_V2H_SET_3D_SHEARING_FACTOR_ACK>
{
    MSG_V2H_SET_3D_SHEARING_FACTOR_ACK() = delete;
    MSG_V2H_SET_3D_SHEARING_FACTOR_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SET_ILLUMINATION_TABLE_2D_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SET_ILLUMINATION_TABLE_2D_ACK, MSG_V2H_SET_ILLUMINATION_TABLE_2D_ACK>
{
    MSG_V2H_SET_ILLUMINATION_TABLE_2D_ACK() = delete;
    MSG_V2H_SET_ILLUMINATION_TABLE_2D_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SINGLE_GRAB_2D_CALIPER_TYPE_CHECK_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SINGLE_GRAB_2D_CALIPER_TYPE_CHECK_ACK,
          MSG_V2H_SINGLE_GRAB_2D_CALIPER_TYPE_CHECK_ACK>
{
    MSG_V2H_SINGLE_GRAB_2D_CALIPER_TYPE_CHECK_ACK() = delete;
    MSG_V2H_SINGLE_GRAB_2D_CALIPER_TYPE_CHECK_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    float m_illuminationTime_ms;
    float m_mean_gv;
    bool m_result;
};

struct MSG_V2H_SET_ILLUMINATION_MIRROR_TABLE_2D_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SET_ILLUMINATION_MIRROR_TABLE_2D_ACK,
          MSG_V2H_SET_ILLUMINATION_MIRROR_TABLE_2D_ACK>
{
    MSG_V2H_SET_ILLUMINATION_MIRROR_TABLE_2D_ACK() = delete;
    MSG_V2H_SET_ILLUMINATION_MIRROR_TABLE_2D_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SET_SLITBEAM_CAMERA_OFFSET_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SET_SLITBEAM_CAMERA_OFFSET_ACK, MSG_V2H_SET_SLITBEAM_CAMERA_OFFSET_ACK>
{
    MSG_V2H_SET_SLITBEAM_CAMERA_OFFSET_ACK() = delete;
    MSG_V2H_SET_SLITBEAM_CAMERA_OFFSET_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SET_SLITBEAM_DISTORTION_PROFILE_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SET_SLITBEAM_DISTORTION_PROFILE_ACK, MSG_V2H_SET_SLITBEAM_DISTORTION_PROFILE_ACK>
{
    MSG_V2H_SET_SLITBEAM_DISTORTION_PROFILE_ACK() = delete;
    MSG_V2H_SET_SLITBEAM_DISTORTION_PROFILE_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SET_SLITBEAM_DISTORTION_FULL_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SET_SLITBEAM_DISTORTION_PROFILE_ACK, MSG_V2H_SET_SLITBEAM_DISTORTION_FULL_ACK>
{
    MSG_V2H_SET_SLITBEAM_DISTORTION_FULL_ACK() = delete;
    MSG_V2H_SET_SLITBEAM_DISTORTION_FULL_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SINGLE_GRAB_2D_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::SINGLE_GRAB_2D_ACK, MSG_V2H_SINGLE_GRAB_2D_ACK>
{
    MSG_V2H_SINGLE_GRAB_2D_ACK() = delete;
    MSG_V2H_SINGLE_GRAB_2D_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_GV_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SINGLE_GRAB_2D_UNIFORMITY_FOR_GV_ACK,
          MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_GV_ACK>
{
    MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_GV_ACK() = delete;
    MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_GV_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    float m_illumtime_ms;
    bool m_result;
};

struct MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_MS_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SINGLE_GRAB_2D_UNIFORMITY_FOR_MS_ACK,
          MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_MS_ACK>
{
    MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_MS_ACK() = delete;
    MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_MS_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_SINGLE_SCAN_3D_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::SINGLE_SCAN_3D_ACK, MSG_V2H_SINGLE_SCAN_3D_ACK>
{
    MSG_V2H_SINGLE_SCAN_3D_ACK() = delete;
    MSG_V2H_SINGLE_SCAN_3D_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_START_LIVE_2D_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::START_LIVE_2D_ACK, MSG_V2H_START_LIVE_2D_ACK>
{
    MSG_V2H_START_LIVE_2D_ACK() = delete;
    MSG_V2H_START_LIVE_2D_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_START_LIVE_3D_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::START_LIVE_3D_ACK, MSG_V2H_START_LIVE_3D_ACK>
{
    MSG_V2H_START_LIVE_3D_ACK() = delete;
    MSG_V2H_START_LIVE_3D_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_STOP_LIVE_ACK : public MSG_V2H_TEMPLATE<MSG_V2H::STOP_LIVE_ACK, MSG_V2H_STOP_LIVE_ACK>
{
    MSG_V2H_STOP_LIVE_ACK() = delete;
    MSG_V2H_STOP_LIVE_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE
    : public MSG_V2H_TEMPLATE<MSG_V2H::GRAB_EACH_CHANNEL_2D_IMAGE, MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE>
{
    MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE()
        : MSG_V2H_TEMPLATE(0)
    {
    }

    long m_imageSizeX; // Image width bytes 는 4 의 배수로 맞추는 것을 가정한다.
    long m_imageSizeY;
    long m_channel;
    float m_illuminationTime_ms;
    bool m_bisColorimage; //mc_Color image 사용여부
    bool m_bisGreenimage;
};

struct MSG_V2H_LIVE_IMAGE : public MSG_V2H_TEMPLATE<MSG_V2H::LIVE_IMAGE, MSG_V2H_LIVE_IMAGE>
{
    MSG_V2H_LIVE_IMAGE()
        : MSG_V2H_TEMPLATE(0)
    {
    }

    long m_imageSizeX; // Image width bytes 는 4 의 배수로 맞추는 것을 가정한다.
    long m_imageSizeY;
    bool m_bisColorimage; //mc_Color image 사용여부
};

struct MSG_V2H_SINGLE_GRAB_IMAGE : public MSG_V2H_TEMPLATE<MSG_V2H::SINGLE_GRAB_IMAGE, MSG_V2H_SINGLE_GRAB_IMAGE>
{
    MSG_V2H_SINGLE_GRAB_IMAGE()
        : MSG_V2H_TEMPLATE(0)
    {
    }

    long m_imageSizeX; // Image width bytes 는 4 의 배수로 맞추는 것을 가정한다.
    long m_imageSizeY;
    bool m_bisColorimage; //mc_Color image 사용여부
    bool m_bisGreenimage;
};

struct MSG_V2H_SINGLE_ZMAP_IMAGE : public MSG_V2H_TEMPLATE<MSG_V2H::SINGLE_ZMAP_IMAGE, MSG_V2H_SINGLE_ZMAP_IMAGE>
{
    MSG_V2H_SINGLE_ZMAP_IMAGE()
        : MSG_V2H_TEMPLATE(0)
    {
    }

    long m_imageSizeX; // Image width bytes 는 4 의 배수로 맞추는 것을 가정한다.
    long m_imageSizeY;
    float m_heightRangeMin;
    float m_heightRangeMax;
};

struct MSG_V2H_GET_2D_ILLUMINATION_EXTENSION_COUNT_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::GET_ILLUMINATION_EXTENSION_COUNT_ACK,
          MSG_V2H_GET_2D_ILLUMINATION_EXTENSION_COUNT_ACK>
{
    MSG_V2H_GET_2D_ILLUMINATION_EXTENSION_COUNT_ACK() = delete;
    MSG_V2H_GET_2D_ILLUMINATION_EXTENSION_COUNT_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
        , m_illumination_extension_count(0)
    {
    }

    int m_illumination_extension_count;
    bool m_result;
};

struct MSG_V2H_GET_2D_ILLUMINATION_CAL_USE_ALLDATA_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::GET_ILLUMINATION_CAL_USE_ALLDATA_ACK,
          MSG_V2H_GET_2D_ILLUMINATION_CAL_USE_ALLDATA_ACK>
{
    MSG_V2H_GET_2D_ILLUMINATION_CAL_USE_ALLDATA_ACK() = delete;
    MSG_V2H_GET_2D_ILLUMINATION_CAL_USE_ALLDATA_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_bIsExistingRef;
    bool m_bIsRefAll;
    bool m_result;
};

struct MSG_V2H_GET_WHITE_BALANCE_GAIN_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::GET_WHITE_BALANCE_GAIN_ACK, MSG_V2H_GET_WHITE_BALANCE_GAIN_ACK>
{
    MSG_V2H_GET_WHITE_BALANCE_GAIN_ACK() = delete;
    MSG_V2H_GET_WHITE_BALANCE_GAIN_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
        , m_isReverseGain(false)
        , m_currentWhiteBalanceGainValue_R(-1.f)
        , m_currentWhiteBalanceGainValue_G(-1.f)
        , m_currentWhiteBalanceGainValue_B(-1.f)
    {
    }

    bool m_isReverseGain;
    float m_currentWhiteBalanceGainValue_R;
    float m_currentWhiteBalanceGainValue_G;
    float m_currentWhiteBalanceGainValue_B;
    bool m_result;
};

struct MSG_V2H_SET_WHITE_BALANCE_GAIN_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SET_WHITE_BALANCE_GAIN_ACK, MSG_V2H_SET_WHITE_BALANCE_GAIN_ACK>
{
    MSG_V2H_SET_WHITE_BALANCE_GAIN_ACK() = delete;
    MSG_V2H_SET_WHITE_BALANCE_GAIN_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};

struct MSG_V2H_GET_UVIR_CHANNEL_IDX_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::GET_UVIR_CHANNEL_IDX_ACK, MSG_V2H_GET_UVIR_CHANNEL_IDX_ACK>
{
    MSG_V2H_GET_UVIR_CHANNEL_IDX_ACK() = delete;
    MSG_V2H_GET_UVIR_CHANNEL_IDX_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
        , m_nUV_Channel_idx(-1)
        , m_nIR_Channel_idx(-1)
    {
    }

    bool m_result;
    long m_nUV_Channel_idx;
    long m_nIR_Channel_idx;
};

struct MSG_V2H_GET_2D_CAMERA_OFFSET_Y_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::GET_2D_CAMERA_OFFSET_Y, MSG_V2H_GET_2D_CAMERA_OFFSET_Y_ACK>
{
    MSG_V2H_GET_2D_CAMERA_OFFSET_Y_ACK() = delete;
    MSG_V2H_GET_2D_CAMERA_OFFSET_Y_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
    int m_nCameraOffsetY;
};

struct MSG_V2H_SET_2D_CAMERA_OFFSET_Y_ACK
    : public MSG_V2H_TEMPLATE<MSG_V2H::SET_2D_CAMERA_OFFSET_Y, MSG_V2H_SET_2D_CAMERA_OFFSET_Y_ACK>
{
    MSG_V2H_SET_2D_CAMERA_OFFSET_Y_ACK() = delete;
    MSG_V2H_SET_2D_CAMERA_OFFSET_Y_ACK(UINT token)
        : MSG_V2H_TEMPLATE(token)
        , m_result(false)
    {
    }

    bool m_result;
};
