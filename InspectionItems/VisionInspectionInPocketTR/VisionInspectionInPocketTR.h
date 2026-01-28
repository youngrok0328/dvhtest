#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Quadrangle32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionInPocketTRPara;
class CDlgVisionInspectionInPocketTR;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;
class VisionInspectionSpec;
class VisionInspectionResult;
class SprocketHoleInfo;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_IN_POCKET_TR_CLASS__ VisionInspectionInPocketTR : public VisionInspection
{
public:
    VisionInspectionInPocketTR(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionInPocketTR(void);

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

    void SetFillPocketInfoInImageLot(bool receivedFillPocket);

    VisionInspectionInPocketTRPara* m_VisionPara;
    CDlgVisionInspectionInPocketTR* m_pVisionInspDlg;

    CPI_EdgeDetect* m_pEdgeDetect;

    float m_fCalcTime;
    
    void SetReceivedFillPocketStatus(const bool& receivedFillPocket);
    bool GetReceivedFillPocketStatus() const;
    

private:
    bool InspInPocketEmptyStatus(const bool detailSetupMode,
        const Ipvm::Rect32s&
            i_rtPocket); //비어 있다고 불량이 아니다. Handler로부터 받은 Fill Pocket 여부와 비교하여 다르면 불량이다.
    bool InspInPocketSprocketHoleWidth(const bool detailSetupMode);
    bool InspInPocketSprocketHoleDamage(const bool detailSetupMode);
    bool InspInPocketMisplace(const bool detailSetupMode, const Ipvm::Rect32s& i_rtPocket);
    bool InspInPocketMisplaceAngle(const bool detailSetupMode, const Ipvm::Rect32s& i_rtPocket);
    bool InspInPocketDeviceOrientation(const bool detailSetupMode, const Ipvm::Rect32s& i_rtDevice);

    void SetDebugInfo(const bool detailSetupMode);
    VisionAlignResult* m_sEdgeAlignResult;

    SprocketHoleInfo* m_sprocketHoleInfo;
    Ipvm::Rect32s m_rtPocket; // 현재 Pocket 영역
    Ipvm::Quadrangle32r m_qrDevice; // 현재 Pocket 영역 (Quadrangle 형태로 변환)
    bool CollectAlignInfoInDebugInfo(const Ipvm::Point32r2& pointImageCenter);

    bool m_receivedFillPocket; // Handler로부터 받은 Fill Pocket 여부
    bool m_alignedFillPocket; // Detail Tape Align에서 확인한 Fill Pocket 여부
};
