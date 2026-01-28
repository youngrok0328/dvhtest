//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessing.h"

//CPP_2_________________________________ This project's headers
#include "VisionBaseDef.h"
#include "VisionDebugInfo.h"
#include "VisionInspFrameIndex.h"
#include "VisionUnitAgent.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/SimpleFunction.h"
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image8u.h>
#include <afxmt.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
void VisionProcessingState::Reset()
{
    m_states.clear();
}

void VisionProcessingState::SetNum(long num)
{
    m_states.clear();
    m_states.resize(num, (long)enumState::Active_Not_Run);
}

void VisionProcessingState::SetState(long index, enumState state)
{
    if (index < 0 || index >= long(m_states.size()))
        return;
    m_states[index] = long(state);
}

long VisionProcessingState::Count() const
{
    return long(m_states.size());
}

VisionProcessingState::enumState VisionProcessingState::GetSate(long index)
{
    if (index < 0 || index >= long(m_states.size()))
        return enumState::Active_Not_Run;
    return (enumState)m_states[index];
}

VisionProcessing::VisionProcessing(
    LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : m_moduleGuid(moduleGuid)
    , m_enabled(FALSE)
    , m_nOverlayMode(OverlayType_ShowAll)
    , m_DebugInfoGroup(moduleGuid, moduleName)
    , m_strModuleName(moduleName)
    , m_visionUnit(visionUnit)
    , m_packageSpec(packageSpec)
    , m_bEmpty(FALSE)
    , m_bMarginal(FALSE)
    , m_bInvalid(FALSE)
    , m_bDoubleDevice(FALSE)
    , m_bCoupon(FALSE) //SDY_Coupon
    , m_enumAlignCenterDefine(enumCenterDefine::eCenterDefine_Body_Align)
    , m_bUse2ndInspection(FALSE)
    , m_str2ndInspCode(_T(""))
    , m_bUseOnlyDLInsp(FALSE)
    , m_bIsComponentInsp(FALSE)
    , m_eSideVisionModuleStatus(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
{
    //기본 적으로 첫 프레임을 Default로 할당
    m_imageFrameIndexList.clear();
    m_imageFrameIndexList.push_back(0);

    m_vecbUseComp2ndInsp.clear();
    m_vecstrComp2ndMatchCode.clear();
}

VisionProcessing::~VisionProcessing()
{
    for (auto& customFrame : m_inspFrames)
    {
        delete customFrame.second;
    }
}

BOOL VisionProcessing::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    const CString strDefaultModuleName
        = m_strModuleName; //GUID를 사용하니까 Module Name은 Display말고는 사용할데가 없다. 이에 각 Vision DLL 모듈의 생성자에서 만든 이름을 지속적으로 유지

    db[_T("ModuleName")].Link(bSave, m_strModuleName);

    if (!bSave && IsPredefineModule(m_moduleGuid))
        m_strModuleName = strDefaultModuleName;

    if (!db[_T("UseProcess")].Link(bSave, m_enabled))
        m_enabled = FALSE;

    long nSize = (long)m_imageFrameIndexList.size();
    if (!db[_T("IllumInfoSize")].Link(bSave, nSize))
        nSize = 0;

    m_imageFrameIndexList.resize(nSize);

    for (long n = 0; n < nSize; n++)
    {
        if (!db.GetSubDBFmt(_T("IllumInfo_%d"), n).Link(bSave, m_imageFrameIndexList[n]))
            m_imageFrameIndexList[n] = 0;
    }

    if (!bSave && nSize < 1) //처음 추가한 Vision Module의 경우 초기화를 위해...
        m_imageFrameIndexList.push_back(0);

    if (!db[_T("OverlayMode")].Link(bSave, m_nOverlayMode))
        m_nOverlayMode = OverlayType_ShowAll;

    if (!bSave)
    {
        for (auto& customFrame : m_inspFrames)
        {
            delete customFrame.second;
        }

        m_inspFrames.clear();
    }

    return TRUE;
}

void VisionProcessing::ResetDebugInfo()
{
    for (auto& debugInfo : m_DebugInfoGroup.m_vecDebugInfo)
    {
        debugInfo.Reset();
    }

    m_processingState.Reset();
}

void VisionProcessing::SetEnabled(BOOL enabled)
{
    m_enabled = enabled;
}

VisionProcessingState& VisionProcessing::GetResultState()
{
    return m_processingState;
}

BOOL VisionProcessing::IsEnabled() const
{
    return m_enabled;
}

const std::vector<long>& VisionProcessing::GetImageFrameList() const
{
    return m_imageFrameIndexList;
}

std::vector<long>& VisionProcessing::GetImageFrameList()
{
    return m_imageFrameIndexList;
}

long VisionProcessing::GetImageFrameIndex(long index) const
{
    if (index < 0 || index >= m_imageFrameIndexList.size())
        return -1;

    long frameIndex = m_imageFrameIndexList[index];
    if (frameIndex < 0 || frameIndex >= m_visionUnit.getIllumInfo2D().getTotalFrameCount())
        return -1;

    return frameIndex;
}

long VisionProcessing::GetImageFrameCount() const
{
    return long(m_imageFrameIndexList.size());
}

long VisionProcessing::GetCurrentPaneID() const
{
    return m_visionUnit.GetCurrentPaneID();
}

bool VisionProcessing::SetCurrentPaneID(long nID)
{
    return m_visionUnit.SetCurrentPaneID(nID);
}

// 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
Ipvm::Image8u VisionProcessing::GetInspectionFrameImage(BOOL bCheckFrameMode, long nFrameID)
{
    auto& imageLotInsp = getImageLotInsp();

    long nMaxFrameNum = (long)imageLotInsp.m_vecImages[m_eSideVisionModuleStatus].size();
    long nMaxFrameID = (long)m_imageFrameIndexList.size();

    if (nMaxFrameID <= 0 || nMaxFrameNum <= 0)
        return Ipvm::Image8u();

    if (nFrameID < 0)
        return Ipvm::Image8u();

    if (bCheckFrameMode
        == FALSE) // 검사항목이 선택한 Frame ID보다 높은 ID가 오게 될 경우 ( ex) image combine Aux Frame )
    {
        if (nMaxFrameNum > nFrameID) // 전체 Frame을 넘지 않으면 해당 영상을 보여준다.
        {
            return imageLotInsp.m_vecImages[m_eSideVisionModuleStatus][nFrameID];
        }
        else // 아니면 실패
        {
            return Ipvm::Image8u();
        }
    }
    else
    {
        long nSelectFrameID = (long)m_imageFrameIndexList.size();

        if (nSelectFrameID <= nFrameID)
            return Ipvm::Image8u();

        long nImageID = m_imageFrameIndexList[nFrameID];

        if (nImageID < 0)
            nImageID = 0;

        if (nMaxFrameNum > nImageID) // 설정된 Frame보다 높지 않으면 그 영상을 보여준다.
        {
            return imageLotInsp.m_vecImages[m_eSideVisionModuleStatus][nImageID];
        }
        else
        {
            return Ipvm::Image8u();
        }
    }
}

VisionReusableMemory& VisionProcessing::getReusableMemory()
{
    return m_visionUnit.getReusableMemory();
}

const VisionScale& VisionProcessing::getScale() const
{
    return m_visionUnit.getImageLot().GetScale(m_eSideVisionModuleStatus);
}

const bool VisionProcessing::IsSideVision() const
{
    return m_visionUnit.IsSideVision();
}

const long VisionProcessing::GetSideVisionNumber() const
{
    return m_visionUnit.GetSideVisionNumber();
}

void VisionProcessing::GetPackageSize(const bool& i_bUnitIsPixel, float& o_fBodySizeX, float& o_fBodySizeY)
{
    return m_visionUnit.GetPackageSize(i_bUnitIsPixel, o_fBodySizeX, o_fBodySizeY);
}

const VisionTapeSpec* VisionProcessing::getTapeSpec() const
{
    return m_visionUnit.getTapeSpec();
}

VisionTapeSpec* VisionProcessing::getTapeSpec()
{
    return m_visionUnit.getTapeSpec();
}

const VisionTrayScanSpec& VisionProcessing::getTrayScanSpec() const
{
    return m_visionUnit.getTrayScanSpec();
}

VisionTrayScanSpec& VisionProcessing::getTrayScanSpec()
{
    return m_visionUnit.getTrayScanSpec();
}

const InspectionAreaInfo& VisionProcessing::getInspectionAreaInfo() const
{
    return m_visionUnit.getInspectionAreaInfo();
}

const UnitAreaInfo& VisionProcessing::getUnitAreaInfo() const
{
    return m_visionUnit.getUnitAreaInfo();
}

VisionImageLot& VisionProcessing::getImageLot()
{
    return m_visionUnit.getImageLot();
}

const VisionImageLot& VisionProcessing::getImageLot() const
{
    return m_visionUnit.getImageLot();
}

VisionImageLotInsp& VisionProcessing::getImageLotInsp()
{
    return m_visionUnit.getImageLotInsp();
}

const VisionImageLotInsp& VisionProcessing::getImageLotInsp() const
{
    return m_visionUnit.getImageLotInsp();
}

const IllumInfo2D& VisionProcessing::getIllumInfo2D() const
{
    return m_visionUnit.getIllumInfo2D();
}

VisionInspFrameIndex& VisionProcessing::createInspFrame(LPCTSTR key, BOOL useAllFrame)
{
    if (m_inspFrames.find(key) == m_inspFrames.end())
    {
        m_inspFrames[key] = new VisionInspFrameIndex(key, false, *this, useAllFrame);
    }

    return *m_inspFrames[key];
}

VisionInspFrameIndex* VisionProcessing::findInspFrame(LPCTSTR key)
{
    auto itFind = m_inspFrames.find(key);
    if (itFind == m_inspFrames.end())
    {
        return nullptr;
    }

    return itFind->second;
}

const std::map<CString, VisionInspFrameIndex*>& VisionProcessing::getInspFrameMap() const
{
    return m_inspFrames;
}

const bool VisionProcessing::getValid_Fiducial_Align()
{
    long nFiducialAlign_DataNum(-1);
    void* pFiducialAlignData = m_visionUnit.GetVisionDebugInfo(
        _VISION_INSP_GUID_FIDUCIAL_ALIGN, _T("Apply PAD Align to Body Center"), nFiducialAlign_DataNum);

    if (pFiducialAlignData != NULL && nFiducialAlign_DataNum > 0)
        return true;
    else
        return false;
}

const Ipvm::Point32r2 VisionProcessing ::getAlignCenter(const enumCenterDefine i_enumCenterDefine_Type)
{
    Ipvm::Point32r2* pAlignCenter(nullptr);

    VisionAlignResult* pEdgeAlignResult(nullptr);
    CString strModuleGUID(""), strDebugInfoName("");

    switch (i_enumCenterDefine_Type)
    {
        case enumCenterDefine::eCenterDefine_Body_Align:
            strModuleGUID = m_visionUnit.GetVisionAlignProcessingModuleGUID();
            strDebugInfoName = _T("EDGE Align Result");
            break;
        case enumCenterDefine::eCenterDefine_Fiducial_Align:
            strModuleGUID = _VISION_INSP_GUID_FIDUCIAL_ALIGN;
            strDebugInfoName = _T("Apply PAD Align to Body Center");
            break;
        default:
            break;
    }

    long nAlignDataNum(-1);
    void* pAlignData = m_visionUnit.GetVisionDebugInfo(strModuleGUID, strDebugInfoName, nAlignDataNum);

    if (pAlignData != nullptr && nAlignDataNum > 0)
    {
        if (i_enumCenterDefine_Type == enumCenterDefine::eCenterDefine_Body_Align)
        {
            pEdgeAlignResult = (VisionAlignResult*)pAlignData;

            return Ipvm::Point32r2(pEdgeAlignResult->m_center.m_x, pEdgeAlignResult->m_center.m_y);
        }
        else
            pAlignCenter = (Ipvm::Point32r2*)pAlignData;
    }
    else
        return Ipvm::Point32r2(-1.f, -1.f);

    return Ipvm::Point32r2(pAlignCenter->m_x, pAlignCenter->m_y);
}

const CString VisionProcessing::getAlignCenterTypeName(const enumCenterDefine i_enumCenterDefine_Type)
{
    switch (i_enumCenterDefine_Type)
    {
        case enumCenterDefine::eCenterDefine_Body_Align:
            return _T("AlignCenter_Body");
            break;
        case enumCenterDefine::eCenterDefine_Fiducial_Align:
            return _T("AlignCenter_Fiducial");
            break;
        default:
            break;
    }

    return _T("");
}

const void VisionProcessing::SetAlignCenterInfo(const enumCenterDefine i_enumCenterDefine_Type)
{
    m_enumAlignCenterDefine = i_enumCenterDefine_Type;
}

const enumCenterDefine VisionProcessing::GetAlignCenterInfo()
{
    return m_enumAlignCenterDefine;
}

void VisionProcessing::SetDebugInfoItem(
    const bool detailSetupMode, LPCTSTR name, const Ipvm::Image8u& image, bool sharing)
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

    psDebugInfo->nDataNum = 1;

    auto* pData = new Ipvm::Image8u;
    pData->Create(image.GetSizeX(), image.GetSizeY());
    Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), *pData);

    psDebugInfo->pData = pData;
}

void VisionProcessing::SetDebugInfoItem(
    const bool detailSetupMode, LPCTSTR name, const Ipvm::Image32r& image, bool sharing)
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

    psDebugInfo->nDataNum = 1;

    auto* pData = new Ipvm::Image32r;
    pData->Create(image.GetSizeX(), image.GetSizeY());
    Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), *pData);

    psDebugInfo->pData = pData;
}

static CCriticalSection g_AddProcessingFunctionExcuteTimeSave_CS;

void VisionProcessing::AddProcessingFunctionExcuteTimeLog(
    const CString i_strModuleName, const CString i_strFunctionName, const float i_fFunctionExcuteTime_ms)
{
    if (i_strFunctionName.IsEmpty() == true
        || i_fFunctionExcuteTime_ms < 0) //함수 이름이 없거나 또는 진행된 시간이 0보다 아래일 수는 없다
        return;

    g_AddProcessingFunctionExcuteTimeSave_CS.Lock();

    std::map<CString, CString> mapstrProcessingFunctionExcuteTimeLog;
    CString strFunctionExcuteTime("");
    strFunctionExcuteTime.Format(_T("%.3f"), i_fFunctionExcuteTime_ms);
    CString strLogFullName = i_strFunctionName; //i_strModuleName + _T("_") + i_strFunctionName;
    mapstrProcessingFunctionExcuteTimeLog[strLogFullName] = strFunctionExcuteTime;

    m_vecInspModuleFunctionExcuteTimeLog.push_back(mapstrProcessingFunctionExcuteTimeLog);

    g_AddProcessingFunctionExcuteTimeSave_CS.Unlock();
}

static CCriticalSection g_InitProcessingFunctionExcuteTimeSave_CS;

void VisionProcessing::InitProcessingFunctionExcuteTimeLog()
{
    g_InitProcessingFunctionExcuteTimeSave_CS.Lock();

    m_vecInspModuleFunctionExcuteTimeLog.clear();

    g_InitProcessingFunctionExcuteTimeSave_CS.Unlock();
}

std::vector<std::map<CString, CString>> VisionProcessing::GetInspModuleFunctionExcuteTimeLog()
{
    return m_vecInspModuleFunctionExcuteTimeLog;
}

std::vector<CString> VisionProcessing::ExportRecipeIllumFrameInfoToText(CString strVisionName) //kircheis_TxtRecipe
{
    //Output Parameter 생성
    std::vector<CString> vecStrIllumInfo(0);
    if (m_enabled == FALSE || m_moduleGuid == _VISION_INSP_GUID_PACKAGE_SPEC
        || m_moduleGuid == _VISION_INSP_GUID_FOV) //Layout Editor와 FOV는 Frame 설정 저장이 의미 없다
        return vecStrIllumInfo; //검사하지도 않고 영향을 주지도 않는다

    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)m_strModuleName);

    //개별 Parameter용 변수
    CString strCategory = _T("Illumination Setting");
    CString strGroup = _T("Frame Setting Info");
    CString strParaName;

    long nFrameNum = GetImageFrameCount();
    vecStrIllumInfo.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Frame Number"), _T(""), nFrameNum));

    for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
    {
        strParaName.Format(_T("%s Frame ID"), (LPCTSTR)(SimpleFunction::GetStringCount(nFrame + 1)));
        vecStrIllumInfo.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T(""), GetImageFrameIndex(nFrame) + 1));
    }

    return vecStrIllumInfo;
}

void VisionProcessing::SetCurVisionModule_Status(const enSideVisionModule i_ModuleStatus)
{
    m_eSideVisionModuleStatus = i_ModuleStatus;
}

enSideVisionModule VisionProcessing::GetCurVisionModule_Status()
{
    return m_eSideVisionModuleStatus;
}
