#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionImageLot;

//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionImageLotInsp
{
public:
    VisionImageLotInsp();
    ~VisionImageLotInsp();

    void Set(const VisionImageLot& imageLot,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void CopyFrom(const VisionImageLotInsp& object,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    const Ipvm::Image8u* GetSafeImagePtr(
        long index, const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;

    Ipvm::Rect32s GetImageRect(
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    int GetImageFrameCount(
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    int GetImageSizeX(const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    int GetImageSizeY(const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;

    const Ipvm::Image8u3* GetSafeColorImagePtr(long index) const;
    int GetColorImageFrameCount() const;
    int GetColorImageSizeX() const;
    int GetColorImageSizeY() const;

    // 검사영상이 어떤 Pane Index 것으로 만들어 졌는가
    // (저장되지 않는 데이터이다) 검사가 Pane 단위로 동작하므로
    // 디스플레이시 해당 Pane Index에서만 Image가 유효하다
    int m_paneIndexForCalculationImage;

    // 기본 이미지 버퍼
    //mc_Side와 함께 기존 image구조체를 같이 사용하려면.. Side가 아닌 image들은 0번 index에 image를 할당
    std::vector<Ipvm::Image8u> m_vecImages[enSideVisionModule::SIDE_VISIONMODULE_END];
    std::vector<Ipvm::Image8u3> m_vecColorImages;

    // 3D 전용 이미지 버퍼
    float m_heightRangeMin;
    float m_heightRangeMax;

    Ipvm::Image32r m_zmapImage;
    Ipvm::Image16u m_vmapImage;
};
