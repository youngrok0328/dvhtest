#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Size32s2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionImageLotInfo
{
public:
    VisionImageLotInfo();
    ~VisionImageLotInfo();

    bool LoadInfo(LPCTSTR pathName, bool popupMessage);

    // 이미지 버퍼 할당을 위한 기본 정보
    int m_sensorSizeX;
    int m_sensorSizeY;
    int m_imageSizeX;
    int m_imageSizeY;
    int m_frameCount;
    int m_zmapCount;
    int m_vmapCount;

    // 기타 이미지 획득 관련 정보
    int m_trayIndex;
    int m_runTimeIndex;
    int m_scanAreaIndex;
    VisionScale m_scale;

    // 3D 전용 정보
    float m_heightRangeMin;
    float m_heightRangeMax;

    // Side image 정보
    int m_nSideModuleIndex; //Default Front 0, Rear 1

    //TR 전용 정보
    bool m_fillPocket;
};

class __VISION_COMMON_API__ VisionImageLot
{
public:
    VisionImageLot();
    ~VisionImageLot();

public: // 이미지 기본 정보
    Ipvm::Rect32s GetImageRect(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    int GetImageFrameCount(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    const Ipvm::Image8u& GetImageFrame(
        int frameIndex, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    const Ipvm::Image32r& GetZmapImage(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    const Ipvm::Image16u& GetVmapImage(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    Ipvm::Image8u& GetImageFrame(int frameIndex, const int i_idx = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    Ipvm::Image32r& GetZmapImage(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    Ipvm::Image16u& GetVmapImage(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    const Ipvm::Image8u3& GetColorImageFrame(
        int frameIndex, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    Ipvm::Image8u3& GetColorImageFrame(
        int frameIndex, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    int GetSensorSizeX(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    int GetSensorSizeY(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    int GetImageSizeX(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    int GetImageSizeY(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;

    const VisionImageLotInfo& GetInfo(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    const VisionScale& GetScale(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    bool CopyFrom(const VisionImageLot& object, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);

public: // 이미지 파일 관련
    BOOL Load(LPCTSTR pathName, bool popupMessage);
    BOOL Save(LPCTSTR pathName) const;
    BOOL SaveTo(LPCTSTR directoryPath, CString& o_strSaveFilePath,
        const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;

public: // 이미지 획득시 정의되는 것들에 대한 인터페이스
    void SetTrayIndex(const int trayIndex,
        const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetRunTimeIndex(const int runTimeIndex, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetScanAreaIndex(
        const int scanAreaIndex, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetScales(const float px2um_x, const float px2um_y, const float px2um_z,
        const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetScales(const VisionScale& scale, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void Set3DScanInfo(float heightRangeMin, float heightRangeMax,
        const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetFillPocketTR(const bool fillPocket, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    bool IsExistZMap(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;

public:
    // 이미지 획득 정보가 바뀌면 호출해 주어야 하는 함수.
    // 획득 정보는 이 함수에 인자로 들어가는 것들을 얘기함.
    static Ipvm::Size32s2 Get2DMaxBufferSize(
        const int sensorSizeX, const int sensorSizeY, const int stitchCountX, const int stitchCountY);
    static Ipvm::Size32s2 Get3DMaxBufferSize(const int sensorSizeX, const int stitchCountY, const int scanLength);

    void Setup2D(const int sensorSizeX, const int sensorSizeY, const Ipvm::Size32s2& bufferSize, const int frameCount,
        const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void Setup3D(const int sensorSizeX, const int sensorSizeY, const Ipvm::Size32s2& bufferSize,
        const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetupColorVision(const int sensorSizeX, const int sensorSizeY, const Ipvm::Size32s2& bufferSize,
        const int frameCount, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    bool SetRealImageSize(const int imageSizeX, const int imageSizeY,
        const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);

public:
    // 분할 그랩 영상 붙여넣기
    void Paste2D(const int stitchGrabIndex, BYTE** ppbyImage, const int imageCount, const int stitchCountX,
        const int stitchCountY,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    /*void Paste3D(const int stitchGrabIndex, const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);*/
    void PasteNGRV(const int stitchGrabIndex, BYTE** ppbyImage, const int imageCount, const int stitchCountX,
        const int stitchCountY, const int i_nReverseFrame,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    /*void UpdateGray3D(const int stitchGrabIndex,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);*/
    void Paste3D(const int stitchGrabIndex, const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap,
        const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void UpdateGray3D(const int stitchGrabIndex, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);

public:
    // Inline Raw Save 체크용이다..
    // 이곳에 들어가기 적합한 함수는 아닌데 위치가 마땅치 않아 넣었다
    void InlineRawSaveCheckReset();
    bool InlineRawSaveCheck();

public:
    void SetInspectionRepeatIndex(
        int repeatIndex); //mc_KMAT Crash, Host에서 repeat검사 진행 간, crash발생으로 인하여 vision에서 정보를 넘겨줌
    int GetInspectionRepeatIndex();
    void SetSideScanID(const int i_nSideScanID);
    int GetSideScanID();

private:
    CRITICAL_SECTION m_csInlineRawSaveCheck;
    bool m_inlineRawSaveCheck;

    //VisionImageLotInfo m_info[enSideVisionModule::SIDE_VISIONMODULE_END];
    VisionImageLotInfo m_info[enDualGrabImageType::DUAL_IMAGE_END];

    // 이미지 버퍼
    struct ImageBuffer
    {
        std::vector<Ipvm::Image8u> m_vecImages;
        std::vector<Ipvm::Image8u3> m_vecColorImages;
        Ipvm::Image32r m_zmapImage;
        Ipvm::Image16u m_vmapImage;
    };

    //mc_Side와 함께 기존 image구조체를 같이 사용하려면.. Side가 아닌 image들은 0번 index에 image를 할당
    //ImageBuffer m_buffer[enSideVisionModule::SIDE_VISIONMODULE_END]; // 할당 메모리 (최대한 잡음)
    //ImageBuffer m_realBuffer[enSideVisionModule::SIDE_VISIONMODULE_END]; // 검사 메모리 (할당 메모리를 Crop 해서 사용한다)

    ImageBuffer m_buffer[(int)enDualGrabImageType::DUAL_IMAGE_END]; // 할당 메모리 (최대한 잡음)
    ImageBuffer
        m_realBuffer[(int)enDualGrabImageType::DUAL_IMAGE_END]; // 검사 메모리 (할당 메모리를 Crop 해서 사용한다)

    /*void AllocateImageFromInfo(const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    bool CompatibilityCheck(
        const VisionImageLotInfo& object, const int i_index = (int)enSideVisionModule::SIDE_VISIONMODULE_FRONT); */
    
    void AllocateImageFromInfo(const int i_index = (int)enDualGrabImageType::DUAL_IMAGE_END);
    bool CompatibilityCheck(
        const VisionImageLotInfo& object, const int i_index = (int)enDualGrabImageType::DUAL_IMAGE_END);

    int m_nInspectionRepeatIndex;

    int m_nSideScanID;
};
