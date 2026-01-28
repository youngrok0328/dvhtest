#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "VisionDebugInfo.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DebugInfoName.h"
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
class IllumInfo2D;
class InspectionAreaInfo;
class CiDataBase;
class UnitAreaInfo;
class VisionDebugInfoGroup;
class VisionImageLot;
class VisionInspectionOverlayResult;
class VisionInspFrameIndex;
class VisionImageLotInsp;
class VisionReusableMemory;
class VisionScale;
class VisionTapeSpec;
class VisionTrayScanSpec;
class VisionUnitAgent;
class VisionReusableMemory;
class VisionInspectionOverlayResult;
class CPackageSpec;
class SystemConfig;
class ScanAreaImageInfo;

namespace Package
{
class CPackageSpec;
}

//HDR_6_________________________________ Header body
//
struct ProcessingDlgInfo
{
    HWND m_hwndParent;
    CRect m_rtSelectionArea;
    CRect m_rtParaArea;
    CRect m_rtDataArea;
    CRect m_rtImageArea;
    CRect m_rtSideFrontStatusArea;
    CRect m_rtSideRearStatusArea;
};

struct Specs
{
    float fMin;
    float fMax;
};

struct sCriteria
{
    Specs length;
    Specs lengthX;
    Specs lengthY;
    Specs thickness;
    Specs rectArea;
    Specs locus;
    Specs width;
    Specs area;
};

struct sInfoSharedMemory
{
    BOOL bAvailable;
    BOOL bConnected;
    BOOL bUpdatedResult;
    CTime cTime;

    int nTrayID;
    int nPocketID;
    int nPaneID;
    char lotID[200];
    char pathInform[200];
    char jobName[200];

    int nNum2ndInsp;
    char c2ndInspName[20][100];
    char c2ndInspCode[20][100];
    BOOL bDLInspOnly[20];

    BOOL bIsCompInsp[20]; // 08.21.20 KSY 검사 리스트 중 COMPONENT검사는 TRUE아니면 FALSE
    BOOL bCompUse2ndInsp[1000]; // 08.21.20 KSY 해당 컴포넌트 소자 중 딥러닝 검사를 하는 소자표시 (모든 컴포넌트 개수)
    char cComp2ndInspCode[1000][100]; // 08.21.20 KSY 컴포넌트 소자 중 딥러닝 검사 코드 표시 (모든 컴포넌트 개수)
    int nNumTotRejectComponent;
    int nRejCompIndex[1000];

    int nNumFrame;
    int nWidth;
    int nHeight;
    int nDepth;

    CPoint ptPaneLT;
    CPoint ptAlign[4];

    sCriteria sCriteriaSpec[20];

    // SurfaceMask 관련 변수 추가.. 20.04.02 승열
    int nNumSurfMask;
    char cSurfMaskName[20][100];

    // Vision Waiting관련 변수
    int nQueueSize;

    sInfoSharedMemory()
    {
        bAvailable = FALSE;
        bConnected = FALSE;
        bUpdatedResult = FALSE;
        nNumTotRejectComponent = 0;
        nQueueSize = 0;
    }
};

class __VISION_COMMON_API__ VisionProcessingState
{
public:
    enum class enumState
    {
        Active_Not_Run, // 수행 자체를 하지 않음
        Active_Invalid, // 수행을 하려고 했으나 실패하였다
        Active_Success, // 수행을 성공하였다
    };

    void Reset();
    void SetNum(long num);
    void SetState(long index, enumState state);
    long Count() const;
    enumState GetSate(long index);

private:
    std::vector<long> m_states;
};

class __VISION_COMMON_API__ VisionProcessing
{
public:
    VisionProcessing() = delete;
    VisionProcessing(LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessing();

public:
    virtual BOOL DoInspection(const bool detailSetupMode,
        const enSideVisionModule i_ModuleStatus = enSideVisionModule::SIDE_VISIONMODULE_FRONT)
        = 0;
    virtual BOOL OnInspection() = 0;
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    virtual void ResetSpecAndPara() = 0;
    virtual void ResetResult() = 0;
    virtual void GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode) = 0;
    virtual void AppendTextResult(CString& textResult) = 0;
    virtual long ShowDlg(const ProcessingDlgInfo& procDlgInfo) = 0;
    virtual void CloseDlg() = 0;
    virtual std::vector<CString> ExportRecipeToText() = 0; //kircheis_TxtRecipe

public:
    void ResetDebugInfo();
    void SetEnabled(BOOL enabled);
    VisionProcessingState& GetResultState();

    BOOL IsEnabled() const;

    const std::vector<long>& GetImageFrameList() const;
    std::vector<long>& GetImageFrameList();
    long GetImageFrameIndex(long index) const;
    long GetImageFrameCount() const;
    long GetCurrentPaneID() const;
    bool SetCurrentPaneID(long nID);

    std::vector<CString> ExportRecipeIllumFrameInfoToText(CString strVisionName); //kircheis_TxtRecipe

    Ipvm::Image8u GetInspectionFrameImage(BOOL bCheckFrameMode = TRUE, long nFrameID = 0);

    VisionReusableMemory& getReusableMemory();
    const VisionScale& getScale() const;
    const VisionTapeSpec* getTapeSpec() const;
    VisionTapeSpec* getTapeSpec();
    const VisionTrayScanSpec& getTrayScanSpec() const;
    VisionTrayScanSpec& getTrayScanSpec();
    const InspectionAreaInfo& getInspectionAreaInfo() const;
    const UnitAreaInfo& getUnitAreaInfo() const;
    VisionImageLot& getImageLot();
    const VisionImageLot& getImageLot() const;
    VisionImageLotInsp& getImageLotInsp();
    const VisionImageLotInsp& getImageLotInsp() const;
    const IllumInfo2D& getIllumInfo2D() const;
    const bool IsSideVision() const;
    const long GetSideVisionNumber() const;
    void GetPackageSize(const bool& i_bUnitIsPixel, float& o_fBodySizeX, float& o_fBodySizeY);

    VisionInspFrameIndex& createInspFrame(LPCTSTR key, BOOL useAllFrame);
    VisionInspFrameIndex* findInspFrame(LPCTSTR key);
    const std::map<CString, VisionInspFrameIndex*>& getInspFrameMap() const;

    //mc_MED4 Body Center or Fiducial Center
    const bool getValid_Fiducial_Align();
    const Ipvm::Point32r2 getAlignCenter(
        const enumCenterDefine i_enumCenterDefine_Type = enumCenterDefine::eCenterDefine_Body_Align);
    const CString getAlignCenterTypeName(
        const enumCenterDefine i_enumCenterDefine_Type = enumCenterDefine::eCenterDefine_Body_Align);
    const void SetAlignCenterInfo(
        const enumCenterDefine i_enumCenterDefine_Type = enumCenterDefine::eCenterDefine_Body_Align);
    const enumCenterDefine GetAlignCenterInfo();

    // 다음 검사항목에 전달해야하는 정보가 아니며, Detail Setup Mode가 아니면 굳이 저장하지 않는다.
    void SetDebugInfoItem(const bool detailSetupMode, LPCTSTR name, const Ipvm::Image8u& image, bool sharing = false);
    void SetDebugInfoItem(const bool detailSetupMode, LPCTSTR name, const Ipvm::Image32r& image, bool sharing = false);

    //mc_Inspection Log
    void AddProcessingFunctionExcuteTimeLog(
        const CString i_strModuleName, const CString i_strFunctionName, const float i_fFunctionExcuteTime_ms);
    void InitProcessingFunctionExcuteTimeLog();
    std::vector<std::map<CString, CString>> GetInspModuleFunctionExcuteTimeLog();

    //mc_Side Vision
    void SetCurVisionModule_Status(const enSideVisionModule i_ModuleStatus
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); //Front or Rear Default는 Front를 따른다
    enSideVisionModule GetCurVisionModule_Status();

    template<typename T>
    void SetDebugInfoItem(
        const bool detailSetupMode, LPCTSTR name, const std::vector<std::vector<T>>& vecDebugItem, bool sharing = false)
    {
        if (sharing == false && detailSetupMode == FALSE)
            return;

        auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(name);

        if (psDebugInfo == nullptr)
        {
            return;
        }

        if (psDebugInfo->pData)
        {
            psDebugInfo->Reset();
        }

        psDebugInfo->nDataNum = 0;

        for (const auto& vec : vecDebugItem)
        {
            psDebugInfo->nDataNum += (long)vec.size();
        }

        if (psDebugInfo->nDataNum <= 0)
        {
            psDebugInfo->nDataNum = 0;
            psDebugInfo->pData = NULL;
            return;
        }

        auto* pData = new T[psDebugInfo->nDataNum];

        int idx = 0;

        for (const auto& vec : vecDebugItem)
        {
            for (const auto& item : vec)
            {
                pData[idx++] = item;
            }
        }

        psDebugInfo->pData = pData;
    }

    template<typename T>
    void SetDebugInfoItem(
        const bool detailSetupMode, LPCTSTR name, const std::vector<T>& vecDebugItem, bool sharing = false)
    {
        if (sharing == false && detailSetupMode == false)
            return;

        auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(name);

        if (psDebugInfo == nullptr)
        {
            return;
        }

        if (psDebugInfo->pData)
        {
            psDebugInfo->Reset();
        }

        psDebugInfo->nDataNum = (long)vecDebugItem.size();

        if (psDebugInfo->nDataNum <= 0)
        {
            psDebugInfo->nDataNum = 0;
            psDebugInfo->pData = NULL;
            return;
        }

        auto* pData = new T[psDebugInfo->nDataNum];
        for (long i = 0; i < psDebugInfo->nDataNum; i++)
        {
            pData[i] = vecDebugItem[i];
        }

        psDebugInfo->pData = pData;
    }

    template<typename T>
    void SetDebugInfoItem(const bool detailSetupMode, LPCTSTR name, const T& debugItem, bool sharing = false)
    {
        if (sharing == false && detailSetupMode == FALSE)
            return;

        auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(name);

        if (psDebugInfo == nullptr)
        {
            return;
        }

        if (psDebugInfo->pData)
        {
            psDebugInfo->Reset();
        }

        psDebugInfo->nDataNum = 1;
        psDebugInfo->pData = new T[1];
        ((T*)psDebugInfo->pData)[0] = debugItem;
    }

    template<typename T>
    void AddDebugInfoItem(const bool detailSetupMode, LPCTSTR name, const T& debugItem, bool sharing = false)
    {
        if (sharing == false && detailSetupMode == FALSE)
            return;

        auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(name);

        if (psDebugInfo == nullptr)
        {
            return;
        }

        T* newData = new T[psDebugInfo->nDataNum + 1];

        for (long idx = 0; idx < psDebugInfo->nDataNum; idx++)
        {
            newData[idx] = ((T*)psDebugInfo->pData)[idx];
        }

        newData[psDebugInfo->nDataNum] = debugItem;

        if (psDebugInfo->pData)
        {
            psDebugInfo->Reset();
        }

        psDebugInfo->nDataNum = psDebugInfo->nDataNum + 1;
        psDebugInfo->pData = newData;
    }

public:
    // 아래 GUID 는 레시피 저장시 키 값으로 사용하므로 소스 코드 레벨에서도 변경하면 안됨.
    const CString m_moduleGuid;

    CString m_strModuleName;
    BOOL m_bEmpty;
    BOOL m_bMarginal;
    BOOL m_bInvalid;
    BOOL m_bDoubleDevice; //kircheis_3DEmpty
    BOOL m_bCoupon; //SDY_Coupon2D

    VisionUnitAgent& m_visionUnit;
    VisionDebugInfoGroup m_DebugInfoGroup;
    CPackageSpec& m_packageSpec;

    // 영훈 - 이 Overlay를 쓸껀지 말껀지
    // 0 : show All
    // 1 : Reject ROI
    // 2 : Hide All
    long m_nOverlayMode;

    // 승열 - 20.05.12 딥러닝 사용 관련44
    BOOL m_bUse2ndInspection;
    CString m_str2ndInspCode;
    BOOL m_bUseOnlyDLInsp;

    // 승열 08.20.20 Component 딥러닝 검사 관련
    BOOL m_bIsComponentInsp;
    std::vector<BOOL> m_vecbUseComp2ndInsp;
    std::vector<CString> m_vecstrComp2ndMatchCode;

private:
    BOOL m_enabled; // 이검사항목 사용할건지
    std::vector<long> m_imageFrameIndexList; // 이검사항목이 쓰는 이미지 프레임 넘버 0부터 시작
    std::map<CString, VisionInspFrameIndex*> m_inspFrames; // 사용자 Custom Frames 저장
    enumCenterDefine m_enumAlignCenterDefine; //mc_MED4 Body Center or Fiducial Center
    std::vector<std::map<CString, CString>> m_vecInspModuleFunctionExcuteTimeLog; //mc_Inspection Log
    enSideVisionModule
        m_eSideVisionModuleStatus; //mc_Side Vision Front, Rear 관리용, Side를 사용하지 않는 Vision은 무조건 Front index를 Default로 따른다

protected:
    VisionProcessingState m_processingState;
};
