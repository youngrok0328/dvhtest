#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineSeg32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionBgaBallBridgePara;
class CDlgVisionInspectionBgaBallBridge;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;
class VisionInspectionSpec;
class VisionInspectionResult;
class Ball;

//HDR_6_________________________________ Header body
//
struct SBallBridgeCalcData
{
    Ipvm::LineSeg32r lineBallLink; //Ball Align 결과 기준으로 자신(Ball)의 Center와 Link Ball의 Center 정보
    float fBallLinkAngle_Rad; //Link Ball과의 각도
    Ipvm::Rect32s rtBallBridgeROI; //알고리즘 상의 검사 영역 (영상을 0도 기준으로 회전한 상황에서 검사 영역)
    PI_RECT pirtBallBridgeShowROI; //실제 검사 영역 (원본 영상에서의 검사 영역. 알고리즘 영역을 만들 후 각도를 적용한다.
};

class __VISION_INSP_BGA_BALL_BRIDGE_CLASS__ VisionInspectionBgaBallBridge : public VisionInspection
{
public:
    VisionInspectionBgaBallBridge(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionBgaBallBridge(void);

public: // VisionProcessing virtual functions
    virtual BOOL DoInspection(const bool detailSetupMode,
        const enSideVisionModule i_ModuleStatus = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual BOOL OnInspection() override;
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db) override;
    virtual void ResetSpecAndPara() override;
    virtual void ResetResult() override;
    virtual void GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode) override;
    virtual void AppendTextResult(CString& textResult) override;
    virtual long ShowDlg(const ProcessingDlgInfo& procDlgInfo) override;
    virtual void CloseDlg() override;
    virtual std::vector<CString> ExportRecipeToText() override; //kircheis_TxtRecipe

public:
    BOOL DoInsp(const bool detailSetupMode);
    BOOL InspBallBridge(const bool detailSetupMode, Ipvm::Image8u& combineImage);

    VisionInspectionBgaBallBridgePara* m_VisionPara;
    CDlgVisionInspectionBgaBallBridge* m_pVisionInspDlg;

    CPI_EdgeDetect* m_pEdgeDetect;

    float m_fCalcTime;

public:
    void SetDebugInfo(const bool detailSetupMode);

    VisionAlignResult* m_sEdgeAlignResult;

    void RefreshPackageSpecBallLink();

    BOOL MakeRefData(BOOL
            detailSetupMode); //아래의 함수들을 연속적으로 호출해야한다. 관리를 위해 아래의 함수를 한번에 호출해주는 함수를 만든다. 아래의 함수 외에 ResetBufferResult()도 호출.
    BOOL GetBallCircle(); //m_vecrtBallCircle&(m_vecrtBallOriginCircle)에 Ball 좌표 넣기
    void
    CalcBallLink(); //m_vec2BallCalcData Size할당 및  m_vecrtBallCircle과 package Spec으로 SBallBridgeCalcData::lineBallLink & SBallBridgeCalcData::fBallLinkAngle_Rad구성
    void
    MakeBallBridgeInspROI(); //SBallBridgeCalcData::lineBallLink와 Algo Para로 SBallBridgeCalcData::rtBallBridgeROI구성
    void
    MakeBallBridgeShowpROI(); //SBallBridgeCalcData::rtBallBridgeROI와 SBallBridgeCalcData::fBallLinkAngle_Rad로 SBallBridgeCalcData::pirtBallBridgeShowROI구성

    void MakeBallBridgeMask(const BOOL detailSetupMode); //kircheis_MED3 //Surface용 Mask를 만든다.

    void ResetBufferResult(); //아래의 모든 vector 변수들을 Clear
    std::vector<Ipvm::Rect32s> m_vecrtBallROI; //Ball2D에서 가져오는 Data
    std::vector<Ipvm::EllipseEq32r> m_vecrtBallCircle; //Ball2D에서 가져오는 Data//얘는 Dialate 적용된거
    std::vector<Ipvm::EllipseEq32r> m_vecrtBallOriginCircle; //kircheis_MED3
    //만들어야 하는 데이터 //[BallNum][LinkNum]
    std::vector<std::vector<SBallBridgeCalcData>> m_vec2BallCalcData;

    Ipvm::Image8u* m_imageMaskInspROI;
};
