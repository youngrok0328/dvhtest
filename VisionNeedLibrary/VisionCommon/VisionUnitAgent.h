#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>
#include <vector>

//HDR_5_________________________________ Forward declarations
class IllumInfo2D;
class InspectionAreaInfo;
class Ngrv2DImageInfo;
class NgrvAfRefInfo;
class NgrvGrabInfo;
class NgrvPackageGrabInfo;
class NgrvSingleRunInfo;
class SystemConfig;
class UnitAreaInfo;
class VisionDebugInfo;
class VisionImageLot;
class VisionImageLotInsp;
class VisionInspectionOverlayResult;
class VisionMainAgent;
class VisionProcessing;
class VisionReusableMemory;
class VisionScale;
class VisionTapeSpec;
class VisionTrayScanSpec;

//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionUnitAgent
{
public:
    VisionUnitAgent(SystemConfig& systemConfig, VisionMainAgent& main);

    virtual BOOL RunInspectionInTeachMode(VisionProcessing* currentModule, bool includeCurrentModule,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT)
        = 0;
    virtual BOOL RunInspection(VisionProcessing* targetModule = nullptr, const bool noNeedSurfaceInspection = false,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT)
        = 0;
    virtual LPCTSTR GetLastInspection_Text(BOOL isLogSave = FALSE) = 0;
    virtual VisionInspectionOverlayResult& GetInspectionOverlayResult() = 0;
    virtual std::vector<VisionDebugInfo*> GetVisionDebugInfos() = 0;
    virtual void* GetVisionDebugInfo(CString _strVisionModuleGUID, CString _strDebugInfoName, long& o_nDataNum) = 0;
    virtual VisionProcessing* GetPreviousVisionProcessing(VisionProcessing* targetProc) = 0;
    virtual VisionProcessing* GetVisionProcessingByGuid(LPCTSTR guid) = 0;
    virtual VisionProcessing* GetVisionAlignProcessing() = 0;
    virtual LPCTSTR GetVisionProcessingModuleNameByGuid(LPCTSTR guid) = 0;
    virtual LPCTSTR GetVisionAlignProcessingModuleGUID() = 0;
    virtual BOOL IsTheUsingVisionProcessingByGuid(LPCTSTR guid) = 0;
    virtual void GetSwitchMarkTeachViewIndex(long& o_CurMarkTeachViewIndex) = 0; //mc_MarkTaechView 관련
    virtual long GetCurrentPaneID() const = 0;
    virtual bool SetCurrentPaneID(long nID) = 0;

    virtual void setImageLot(std::shared_ptr<VisionImageLot> imageLot) = 0;
    virtual void setImageLotInsp() = 0;
    virtual const VisionScale& getScale() const = 0;
    virtual const VisionTapeSpec* getTapeSpec() const;
    virtual const VisionTrayScanSpec& getTrayScanSpec() const = 0;
    virtual const InspectionAreaInfo& getInspectionAreaInfo() const = 0;
    virtual const UnitAreaInfo& getUnitAreaInfo() const = 0;
    virtual const VisionImageLot& getImageLot() const = 0;
    virtual const VisionImageLotInsp& getImageLotInsp() const = 0;
    virtual VisionTapeSpec* getTapeSpec();
    virtual VisionTrayScanSpec& getTrayScanSpec() = 0;
    virtual VisionImageLot& getImageLot() = 0;
    virtual VisionImageLotInsp& getImageLotInsp() = 0;
    virtual VisionReusableMemory& getReusableMemory() = 0;
    virtual const bool IsSideVision() const = 0;
    virtual const long GetSideVisionNumber() const = 0;
    virtual void GetPackageSize(const bool& i_bUnitIsPixel, float& o_fBodySizeX, float& o_fBodySizeY) = 0;

    virtual const IllumInfo2D& getIllumInfo2D() const = 0;

    // Side Vision 정보 가져오기 - JHB_2024.04.15
    virtual long GetSideVisionSection() = 0;
    virtual void SetSideVisionSection(long i_nSideVisionSection) = 0;

    // MED#6 - JHB_2025.01.06
    virtual void SetPackageInfo_NGRV(std::vector<NgrvPackageGrabInfo> packageInfo) = 0;
    virtual std::vector<NgrvPackageGrabInfo>& GetPackageInfo_NGRV() = 0;
    virtual void SetAFInfo_NGRV(NgrvAfRefInfo afInfo) = 0;
    virtual NgrvAfRefInfo& GetAFInfo() = 0;
    virtual void SetGrabInfo_NGRV(std::vector<NgrvGrabInfo> grabInfo) = 0;
    virtual std::vector<NgrvGrabInfo>& GetGrabInfo_NGRV() = 0;
    virtual void Set2DImageInfo_NGRV(long visionID, Ngrv2DImageInfo imageInfo) = 0;
    virtual Ngrv2DImageInfo& Get2DImageInfo_NGRV(long visionID) = 0;
    virtual void SetNGRVSingleRunInfo(long visionID, NgrvSingleRunInfo singleRunInfo) = 0;
    virtual NgrvSingleRunInfo& GetNGRVSingleRunInfo(long visionID) = 0;
    virtual void SetSocketInfo(Ipvm::SocketMessaging* pMessageSocket) = 0;
    virtual Ipvm::SocketMessaging& GetSocketInfo() = 0;
    virtual int32_t GetVisionMainType() = 0;

    const wchar_t* GetJobFileName() const;

    SystemConfig& m_systemConfig;
    VisionMainAgent& m_main;

    VisionProcessing* m_currentProcessingModule;
};
