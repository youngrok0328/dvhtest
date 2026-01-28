//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionUnit.h"

//CPP_2_________________________________ This project's headers
#include "DeviceType.h"
#include "VisionUnitDlg.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h" //MED#6_NGRV_JHB
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../InformationModule/dPI_DataBase/OldScanSpec.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../InspectionItems/VisionInspection2DMatrix/VisionInspection2DMatrix.h"
#include "../../InspectionItems/VisionInspectionBgaBallBridge/VisionInspectionBgaBallBridge.h"
#include "../../InspectionItems/VisionInspectionBgaBallPixelQuality/VisionInspectionBgaBallPixelQuality.h" //kircheis_BPQ//kircheis_AddInsp
#include "../../InspectionItems/VisionInspectionBgaBallPixelQuality/VisionInspectionBgaBallPixelQualityPara.h"
#include "../../InspectionItems/VisionInspectionBgaBottom2D/VisionInspectionBgaBottom2D.h"
#include "../../InspectionItems/VisionInspectionBgaBottom3D/VisionInspectionBgaBottom3D.h"
#include "../../InspectionItems/VisionInspectionComponent2D/VisionInspectionComponent2D.h"
#include "../../InspectionItems/VisionInspectionComponent3D/VisionInspectionComponent3D.h"
#include "../../InspectionItems/VisionInspectionDie2D/VisionInspectionDie2D.h" //kircheis_InspLid
#include "../../InspectionItems/VisionInspectionGeometry/VisionInspectionGeometry.h"
#include "../../InspectionItems/VisionInspectionGlassSubstrate2D/VisionInspectionGlassSubstrate2D.h"
#include "../../InspectionItems/VisionInspectionIntensityChecker2D/VisionInspectionIntensityChecker2D.h" //SDY_IntensityChecker2D
#include "../../InspectionItems/VisionInspectionKOZ2D/VisionInspectionKOZ2D.h" //kircheis_KOZ
#include "../../InspectionItems/VisionInspectionLgaBottom2D/VisionInspectionLgaBottom2D.h" //kircheis_Land
#include "../../InspectionItems/VisionInspectionLgaBottom3D/VisionInspectionLgaBottom3D.h" //kircheis_Land 3D
#include "../../InspectionItems/VisionInspectionLid2D/VisionInspectionLid2D.h" //kircheis_InspLid
#include "../../InspectionItems/VisionInspectionLid3D/VisionInspectionLid3D.h" //kircheis_InspLid
#include "../../InspectionItems/VisionInspectionMark/VisionInspectionMark.h"
#include "../../InspectionItems/VisionInspectionMark/VisionInspectionMarkPara.h"
#include "../../InspectionItems/VisionInspectionPackageSize/VisionInspectionPackageSize.h"
#include "../../InspectionItems/VisionInspectionPatch2D/VisionInspectionPatch2D.h" //kircheis_POI
#include "../../InspectionItems/VisionInspectionPatch3D/VisionInspectionPatch3D.h" //kircheis_POI
#include "../../InspectionItems/VisionInspectionSide2D/VisionInspectionSide2D.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurface.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurfacePara.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../ProcessingItems/VisionProcessingAlign2D/VisionProcessingAlign2D.h"
#include "../../ProcessingItems/VisionProcessingAlign3D/VisionProcessingAlign3D.h"
#include "../../ProcessingItems/VisionProcessingCoupon2D/VisionProcessingCoupon2D.h" //SDY_Coupon2D
#include "../../ProcessingItems/VisionProcessingFOV/VisionProcessingFOV.h"
#include "../../ProcessingItems/VisionProcessingFiducialAlign/VisionProcessingFiducialAlign.h"
#include "../../ProcessingItems/VisionProcessingGlobalMaskMaker/VisionProcessingGlobalMaskMaker.h"
#include "../../ProcessingItems/VisionProcessingNGRV/VisionProcessingNGRV.h" //MED#6_NGRV_JHB
#include "../../ProcessingItems/VisionProcessingOtherAlign/VisionProcessingOtherAlign.h"
#include "../../ProcessingItems/VisionProcessingOtherAlign/VisionProcessingOtherAlignPara.h"
#include "../../ProcessingItems/VisionProcessingPadAlign2D/VisionProcessingPadAlign2D.h"
#include "../../ProcessingItems/VisionProcessingSideDetailAlign/VisionProcessingSideDetailAlign.h"
#include "../../ProcessingItems/VisionProcessingSpec/VisionProcessingSpec.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/UnitAreaInfo.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionMainAgent.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Para.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PGM_MUTEX_NAME _T("IPACK_DL_MUTEX")
#define PGM_INSP_RESULT_MUTEX _T("IPACK_DL_RESULT_UPDATE_MUTEX")
#define INFO_SHARED_MEMNAME _T("iPack2ndInspLogInfomSharedMemorys")
#define IMAGE_SHARED_MEMNAME _T("iPack2ndInspImageSharedMemotys")
#define DEVRESULT_SHARED_MEMNAME _T("iPack2ndInspDevResultSharedMemorys")

//CPP_7_________________________________ Implementation body
//
static long g_threadIndex = 0;
constexpr auto FRAME_NUM_MAX = 20;

VisionUnit::VisionUnit(const IllumInfo2D& illum2D, VisionMainAgent& main)
    : VisionUnitAgent(SystemConfig::GetInstance(), main)
    , m_illum2D(illum2D)
    , m_threadIndex(g_threadIndex++)
    , m_pPackageSpec(new CPackageSpec)
    , m_trayScanSpec(new VisionTrayScanSpec)
    , m_unitAreaInfo(new UnitAreaInfo(this))
    , m_imageLot(std::make_shared<VisionImageLot>())
    , m_pImageLotInsp(new VisionImageLotInsp)
    , m_VisionReusableMemory(new VisionReusableMemory)
    , m_runOverlayResult(new VisionInspectionOverlayResult)
    , m_visionUnitDlg(nullptr)
    , m_lastInspection_InfoMake(false)
    , m_reviewColorImageName(_T(""))
    , m_Mutex(FALSE, PGM_MUTEX_NAME)
    , m_updateMutex(FALSE, PGM_INSP_RESULT_MUTEX)
    , m_nSideVisionSection(0)
{
    m_visionProcessingSpec_PackageLayoutEditor
        = new VisionProcessingSpec(_VISION_INSP_GUID_PACKAGE_SPEC, _T("Package Layout"), *this, *m_pPackageSpec);
    m_VisionProcessingFOV = new VisionProcessingFOV(*this, *m_pPackageSpec);
    m_visionProcessingFiducialAlign = new VisionProcessingFiducialAlign(*this, *m_pPackageSpec);
    m_visionProcessingPadAlign2D = new VisionProcessingPadAlign2D(*this, *m_pPackageSpec);
    m_visionProcessingOtherAlign = new VisionProcessingOtherAlign(*this, *m_pPackageSpec);
    m_visionProcessingGlobalMaskMaker = new VisionProcessingGlobalMaskMaker(*this, *m_pPackageSpec);
    m_visionProcessingAlign2D = new VisionProcessingAlign2D(*this, *m_pPackageSpec);
    m_visionProcessingAlign3D = new VisionProcessingAlign3D(*this, *m_pPackageSpec);
    m_visionProcessingCoupon2D = new VisionProcessingCoupon2D(*this, *m_pPackageSpec); //SDY_Coupon2D
    m_visionProcessingSideDetailAlign = new VisionProcessingSideDetailAlign(*this, *m_pPackageSpec);
    m_visionProcessingNGRV
        = new VisionProcessingNGRV(_VISION_INSP_GUID_NGRV, _T("NGRV Setup"), *this, *m_pPackageSpec); //MED#6_NGRV_JHB
    m_visionInspectionBgaBottom2D = new VisionInspectionBgaBottom2D(*this, *m_pPackageSpec);
    m_visionInspectionBgaBottom3D = new VisionInspectionBgaBottom3D(*this, *m_pPackageSpec);
    m_visionInspectionComponent2D = new VisionInspectionComponent2D(*this, *m_pPackageSpec);
    m_visionInspectionComponent3D = new VisionInspectionComponent3D(*this, *m_pPackageSpec);
    m_visionInspection2DMatrix = new VisionInspection2DMatrix(*this, *m_pPackageSpec);
    m_visionInspectionPackageSize = new VisionInspectionPackageSize(*this, *m_pPackageSpec);
    m_visionInspectionBgaBallBridge = new VisionInspectionBgaBallBridge(*this, *m_pPackageSpec);
    m_visionInspectionBgaBallPixelQuality = new VisionInspectionBgaBallPixelQuality(*this, *m_pPackageSpec);
    m_visionInspectionLgaBottom2D = new VisionInspectionLgaBottom2D(*this, *m_pPackageSpec);
    m_visionInspectionLgaBottom3D = new VisionInspectionLgaBottom3D(*this, *m_pPackageSpec);
    m_visionInspectionLid2D = new VisionInspectionLid2D(*this, *m_pPackageSpec); //kircheis_InspLID
    m_visionInspectionLid3D = new VisionInspectionLid3D(*this, *m_pPackageSpec); //kircheis_InspLID
    m_visionInspectionPatch2D = new VisionInspectionPatch2D(*this, *m_pPackageSpec); //kircheis_POI
    m_visionInspectionPatch3D = new VisionInspectionPatch3D(*this, *m_pPackageSpec); //kircheis_POI
    m_visionInspectionDie2D = new VisionInspectionDie2D(*this, *m_pPackageSpec); //kircheis_InspLID
    m_visionInspectionKOZ2D = new VisionInspectionKOZ2D(*this, *m_pPackageSpec); //kircheis_KOZ
    m_visionInspectionIntensityChecker2D
        = new VisionInspectionIntensityChecker2D(*this, *m_pPackageSpec); //SDY_IntensityChecker
    m_visionInspectionGlassSubstrate2D = new VisionInspectionGlassSubstrate2D(*this, *m_pPackageSpec);
    m_visionInspectionSide2D = new VisionInspectionSide2D(*this, *m_pPackageSpec);

    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionProcessingSpec_PackageLayoutEditor);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_VisionProcessingFOV);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionProcessingFiducialAlign);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionProcessingPadAlign2D);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionProcessingOtherAlign);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionProcessingGlobalMaskMaker);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionProcessingAlign2D);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionProcessingAlign3D);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionProcessingCoupon2D); //SDY_Coupon2D
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionProcessingSideDetailAlign);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionBgaBottom2D);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionBgaBottom3D);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionComponent2D);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionComponent3D);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionPackageSize);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspection2DMatrix);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionBgaBallBridge);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionBgaBallPixelQuality);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionLgaBottom2D);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionLgaBottom3D);
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionLid2D); //kircheis_InspLID
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionLid3D); //kircheis_InspLID
    //m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionPatch2D);//kircheis_POI
    //m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionPatch3D);//kircheis_POI
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionDie2D); //kircheis_InspLID
    //m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionKOZ2D);//kircheis_KOZ
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionIntensityChecker2D); //SDY_IntensityChecker
    m_vecAllPredefineVisionModulesBuffer.push_back(m_visionInspectionSide2D);
    m_ptrSharedMem = NULL;
    m_ptrSharedImageMem = NULL;
    m_pbySharedDevResult = NULL;

    if (SystemConfig::GetInstance().m_bUseAiInspection) // ksy
    {
        InitSharedMemBuffers();
    }

    m_vecPackageInfo_NGRV.clear(); // MED#6 - JHB_2025.01.06
    m_vecGrabInfo_NGRV.clear();
    m_afInfo_NGRV.Init();
}

VisionUnit::~VisionUnit(void)
{
    delete m_visionUnitDlg;

    for (auto* surface : m_visionInspectionSurfaces)
        delete surface;

    if (m_visionInspectionMarks.size())
    {
        for (long i = 0; i < (long)(m_visionInspectionMarks.size()); i++)
            delete m_visionInspectionMarks[i];
    }
    m_visionInspectionMarks.clear();

    if (m_visionInspectionGeometries.size())
    {
        for (long i = 0; i < (long)(m_visionInspectionGeometries.size()); i++)
            delete m_visionInspectionGeometries[i];
    }
    m_visionInspectionGeometries.clear();

    delete m_runOverlayResult;
    delete m_pPackageSpec;
    delete m_unitAreaInfo;
    delete m_trayScanSpec;
    delete m_visionInspectionGlassSubstrate2D;
    delete m_visionInspectionIntensityChecker2D; //SDY_IntensityChecker
    delete m_visionInspectionKOZ2D; //kircheis_KOZ
    delete m_visionInspectionDie2D; //kircheis_InspLID
    delete m_visionInspectionPatch3D; //kircheis_POI
    delete m_visionInspectionPatch2D; //kircheis_POI
    delete m_visionInspectionLid3D; //kircheis_InspLID
    delete m_visionInspectionLid2D; //kircheis_InspLID
    delete m_visionInspectionLgaBottom3D;
    delete m_visionInspectionLgaBottom2D;
    delete m_visionInspectionBgaBallPixelQuality;
    delete m_visionInspectionBgaBallBridge;
    delete m_visionInspection2DMatrix;
    delete m_visionInspectionPackageSize;
    delete m_visionInspectionComponent3D;
    delete m_visionInspectionComponent2D;
    delete m_visionInspectionBgaBottom3D;
    delete m_visionInspectionBgaBottom2D;
    delete m_visionInspectionSide2D;
    delete m_visionProcessingSideDetailAlign;
    delete m_visionProcessingNGRV; //MED#6_NGRV_JHB
    delete m_visionProcessingCoupon2D; //SDY_Coupon2D
    delete m_visionProcessingAlign3D;
    delete m_visionProcessingAlign2D;
    delete m_visionProcessingGlobalMaskMaker;
    delete m_visionProcessingOtherAlign;
    delete m_visionProcessingFiducialAlign;
    delete m_visionProcessingPadAlign2D;
    delete m_VisionProcessingFOV;
    delete m_visionProcessingSpec_PackageLayoutEditor;

    delete m_VisionReusableMemory;

    delete m_pImageLotInsp;

    if (m_systemConfig.m_bUseAiInspection) // ksy 20.05.19
    {
        ReleaseSharedMemBuffers();
    }
}

void VisionUnit::InspectionClear()
{
    // TODO : 여기 제대로 정리 필요
    m_VisionProcessingFOV->ResetSpecAndPara();
    m_visionProcessingFiducialAlign->ResetSpecAndPara();
    m_visionProcessingPadAlign2D->ResetSpecAndPara();
    m_visionProcessingOtherAlign->ResetSpecAndPara();
    m_visionProcessingGlobalMaskMaker->ResetSpecAndPara();
    m_visionProcessingAlign2D->ResetSpecAndPara();
    m_visionProcessingAlign3D->ResetSpecAndPara();
    m_visionProcessingCoupon2D->ResetSpecAndPara(); //SDY_Coupon2D
    m_visionProcessingSideDetailAlign->ResetSpecAndPara();
    m_visionProcessingNGRV->ResetSpecAndPara(); //MED#6_NGRV_JHB
    m_visionInspectionBgaBottom2D->ResetSpecAndPara();
    m_visionInspection2DMatrix->ResetSpecAndPara();
    m_visionInspectionPackageSize->ResetSpecAndPara();
    m_visionInspectionBgaBottom3D->ResetSpecAndPara();
    m_visionInspectionBgaBallBridge->ResetSpecAndPara();
    m_visionInspectionBgaBallPixelQuality->ResetSpecAndPara(); //kircheis_BPQ //kircheis_AddInsp
    m_visionInspectionComponent2D->ResetSpecAndPara();
    m_visionInspectionComponent3D->ResetSpecAndPara();
    m_visionInspectionLgaBottom2D->ResetSpecAndPara(); //kircheis_Land
    m_visionInspectionLgaBottom3D->ResetSpecAndPara(); //kircheis_Land 3D
    m_visionInspectionLid2D->ResetSpecAndPara(); //kircheis_InspLID
    m_visionInspectionLid3D->ResetSpecAndPara(); //kircheis_InspLID
    m_visionInspectionPatch2D->ResetSpecAndPara(); //kircheis_POI
    m_visionInspectionPatch3D->ResetSpecAndPara(); //kircheis_POI
    m_visionInspectionDie2D->ResetSpecAndPara(); //kircheis_InspLID
    m_visionInspectionKOZ2D->ResetSpecAndPara(); //kircheis_KOZ
    m_visionInspectionIntensityChecker2D->ResetSpecAndPara(); //SDY_IntensityChecker
    m_visionInspectionGlassSubstrate2D->ResetSpecAndPara();
    m_visionInspectionSide2D->ResetSpecAndPara();

    for (auto* surface : m_visionInspectionSurfaces)
    {
        delete surface;
    }
    m_visionInspectionSurfaces.clear();

    for (auto* deleteData : m_visionInspectionMarks)
    {
        delete deleteData;
    }
    m_visionInspectionMarks.clear();

    for (auto* deleteData : m_visionInspectionGeometries)
    {
        delete deleteData;
    }
    m_visionInspectionGeometries.clear();
}

std::vector<VisionInspectionSpec*> VisionUnit::GetVisionInspectionSpecs()
{
    std::vector<VisionInspectionSpec*> visionInspectionSpecs;

    for (auto* proc : m_vecVisionModules)
    {
        VisionInspection* insp = dynamic_cast<VisionInspection*>(proc);

        if (insp == nullptr)
        {
            continue;
        }

        for (auto& spec : insp->m_fixedInspectionSpecs)
        {
            visionInspectionSpecs.push_back(&spec);
        }

        for (auto& spec : insp->m_variableInspectionSpecs)
        {
            visionInspectionSpecs.push_back(&spec);
        }
    }

    return visionInspectionSpecs;
}

std::vector<VisionInspectionResult*> VisionUnit::GetVisionInspectionResults()
{
    std::vector<VisionInspectionResult*> visionInspectionResults;

    for (auto* proc : m_vecVisionModules)
    {
        VisionInspection* insp = dynamic_cast<VisionInspection*>(proc);

        if (insp == nullptr)
        {
            continue;
        }

        for (auto& result : insp->m_resultGroup.m_vecResult)
        {
            visionInspectionResults.push_back(&result);
        }
    }

    return visionInspectionResults;
}

std::vector<VisionDebugInfo*> VisionUnit::GetVisionDebugInfos()
{
    std::vector<VisionDebugInfo*> visionDebugInfos;

    for (auto* proc : m_vecVisionModules)
    {
        for (auto& info : proc->m_DebugInfoGroup.m_vecDebugInfo)
        {
            visionDebugInfos.push_back(&info);
        }
    }

    return visionDebugInfos;
}

void* VisionUnit::GetVisionDebugInfo(CString _strVisionModuleGUID, CString _strDebugInfoName, long& o_nDataNum)
{
    o_nDataNum = 0;
    if (_strDebugInfoName.IsEmpty())
        return nullptr;

    std::vector<VisionDebugInfo*> visionDebugInfos;

    BOOL bIsEmptyVisionModuleGUID = _strVisionModuleGUID.IsEmpty();

    for (auto* proc : m_vecVisionModules)
    {
        if (!bIsEmptyVisionModuleGUID && proc->m_moduleGuid != _strVisionModuleGUID)
        {
            continue;
        }
        for (auto& info : proc->m_DebugInfoGroup.m_vecDebugInfo)
        {
            if (info.pData == nullptr)
                continue;
            else if (info.strDebugInfoName == _strDebugInfoName)
            {
                o_nDataNum = info.nDataNum;
                return info.pData;
            }
        }
    }

    return nullptr;
}

VisionProcessing* VisionUnit::GetPreviousVisionProcessing(VisionProcessing* targetProc)
{
    VisionProcessing* ascendentProc = nullptr;

    for (auto* proc : m_vecVisionModules)
    {
        if (proc == targetProc)
        {
            return ascendentProc;
        }

        ascendentProc = proc;
    }

    return nullptr;
}

VisionProcessing* VisionUnit::GetVisionProcessingByGuid(LPCTSTR guid)
{
    for (auto* proc : m_vecVisionModules)
    {
        if (proc->m_moduleGuid == guid)
        {
            return proc;
        }
    }

    return nullptr;
}

VisionProcessing* VisionUnit::GetVisionAlignProcessing()
{
    return GetVisionProcessingByGuid(GetVisionAlignProcessingModuleGUID());
}

LPCTSTR VisionUnit::GetVisionProcessingModuleNameByGuid(LPCTSTR guid)
{
    for (auto* proc : m_vecAllPredefineVisionModulesBuffer)
    {
        if (proc->m_moduleGuid == guid)
        {
            return proc->m_strModuleName;
        }
    }

    ASSERT(!"This GUID is not used in the current recipe.");
    return _T("");
}

LPCTSTR VisionUnit::GetVisionAlignProcessingModuleGUID()
{
    if (m_systemConfig.m_nVisionType == VISIONTYPE_3D_INSP)
    {
        return _VISION_INSP_GUID_ALIGN_3D;
    }

    return _VISION_INSP_GUID_ALIGN_2D;
}

BOOL VisionUnit::IsTheUsingVisionProcessingByGuid(LPCTSTR guid)
{
    for (auto* proc : m_vecVisionModules)
    {
        if (proc->m_moduleGuid == guid && proc->IsEnabled())
        {
            return true;
        }
    }

    return false;
}

BOOL VisionUnit::IsMarkExist()
{
    for (auto* proc : m_vecVisionModules)
    {
        if (dynamic_cast<VisionInspectionMark*>(proc))
        {
            auto* markProc = dynamic_cast<VisionInspectionMark*>(proc);
            if (markProc != nullptr && markProc->m_VisionPara != nullptr)
            {
                if (markProc->m_VisionPara->m_nMarkInspMode == enumMarkInspctionModeIndex::MarkInspectionMode_Simple)
                    continue;
                else
                    return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL VisionUnit::IsNeedMarkTeach()
{
    for (auto* proc : m_vecVisionModules)
    {
        if (dynamic_cast<VisionInspectionMark*>(proc))
        {
            if (((VisionInspectionMark*)proc)->IsNeedMarkTeach())
                return true;
        }
    }

    return false;
}

bool VisionUnit::IsWindowVisible()
{
    return m_visionUnitDlg->GetSafeHwnd() && m_visionUnitDlg->IsWindowVisible();
}

BOOL VisionUnit::RunInspectionInTeachMode(
    VisionProcessing* currentModule, bool includeCurrentModule, const enSideVisionModule i_eSideVisionModule)
{
    return RunInspection(currentModule, includeCurrentModule, true, i_eSideVisionModule);
}

BOOL VisionUnit::RunInspection(
    VisionProcessing* currentModule, const bool noNeedSurfaceInspection, const enSideVisionModule i_eSideVisionModule)
{
    return RunInspection(currentModule, true, noNeedSurfaceInspection, i_eSideVisionModule);
}

BOOL VisionUnit::RunInspection(VisionProcessing* currentModule, bool includeCurrentModule, bool noNeedSurfaceInspection,
    const enSideVisionModule i_eSideVisionModule)
{
    Ipvm::TimeCheck RunInspectionTotalTime;

    m_runOverlayResult->Reset();

    if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
    {
        m_sUnitinspectionLogInfo.Init();
        m_sUnitinspectionLogInfo.m_nThreadindex = m_threadIndex;
    }

    m_bEmpty = FALSE;
    m_bReject = FALSE;
    m_bMarginal = FALSE;
    m_bInvalid = FALSE;
    m_bDoubleDevice = FALSE; //kircheis_3DEmpty
    m_bCoupon = FALSE; //SDY_Coupon2D

    Ipvm::TimeCheck inspTime;

    Ipvm::TimeCheck InspModuleResetTime;
    // 최근의 Result 를 Reset
    for (auto* proc : m_vecVisionModules)
    {
        proc->ResetDebugInfo();

        //mc_insepction Module의 Log를 초기화
        if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
        {
            proc->InitProcessingFunctionExcuteTimeLog();
        }
        //

        VisionInspection* insp = dynamic_cast<VisionInspection*>(proc);

        if (insp == nullptr)
        {
            continue;
        }

        insp->ResetResult();

        proc->SetCurVisionModule_Status(i_eSideVisionModule); //mc_Side inspection
    }

    if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
    {
        m_sUnitinspectionLogInfo.SetProcessExcuteTimeLog(
            _T("VisionUnit_ModuleRestTime"), (float)InspModuleResetTime.Elapsed_ms());
    }

    auto visionInspectionSpecs = GetVisionInspectionSpecs();
    auto visionInspectionResults = GetVisionInspectionResults();

    m_VisionReusableMemory->ResetInspectionInfo();

    long index_specSurface = GetIndex_GenerateSpecSurfaceMask(currentModule);

    BOOL bValidGeometryInsp = FALSE;
    long nInspectionNum = (long)m_vecVisionModules.size();
    for (long nid = 0; nid < nInspectionNum; nid++)
    {
        if (dynamic_cast<VisionInspectionGeometry*>(m_vecVisionModules[nid]))
        {
            // 영훈 20160611: Aligned Image 생성이 필요한지 판단한다.
            bValidGeometryInsp = TRUE;
        }
    }

    m_lastInspection_listToCollect.clear();
    m_lastInspection_InfoMake = false;

    static const long nVisionType = m_systemConfig.m_nVisionType;
    static const BOOL bIsVision3D = nVisionType == VISIONTYPE_3D_INSP;

    for (long nid = 0; nid < nInspectionNum; nid++)
    {
        auto* proc = m_vecVisionModules[nid];
        if (dynamic_cast<VisionInspection2DMatrix*>(proc))
        {
            ((VisionInspection2DMatrix*)proc)->ResetDebugInfo();
            break;
        }
    }

    sVIsionInspTime sVisionInspTimeData;
    sVisionInspTimeData.init();
    Ipvm::TimeCheck VisionInspTime;
    for (long nid = 0; nid < nInspectionNum; nid++)
    {
        auto* proc = m_vecVisionModules[nid];

        if (!includeCurrentModule && proc == currentModule)
        {
            return TRUE;
        }

        // Package Layout Editor와 F.O.V. Editor 일때는 검사하지 않는다.
        BOOL bInsp = TRUE;
        if ((dynamic_cast<VisionProcessingFOV*>(proc)
                && !bIsVision3D) //3D Vision일때는 FOV Edit 해야 한다. Stitch를 안하면 어차피 그쪽 함수에서 Return 시킴.
            || dynamic_cast<VisionProcessingSpec*>(proc))
        {
            bInsp = FALSE;
        }

        if (noNeedSurfaceInspection)
        {
            //-------------------------------------------------------------------------
            // Surface 검사를 하지 말라고 했을 경우
            // 검사 타켓이 Surface 검사인 경우를 제외하고
            // 검사를 수행하지 않는다. Surface 검사들은 각각 독립적인데
            // 괜히 셋업중 속도만 늦어지는 것을 방지하기 위해서다.
            // (물론 Surface Mask는 검사가 아니라 Mask를 만드는 녀석이므로 수행해야 한다)
            //-------------------------------------------------------------------------

            auto* surface = dynamic_cast<VisionInspectionSurface*>(proc);
            if (proc != currentModule && surface && !surface->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
            {
                bInsp = FALSE;
            }
        }

        const bool detailSetupMode = (currentModule == proc);

        if (bInsp)
        {
            if (dynamic_cast<VisionInspection2DMatrix*>(proc))
            {
                ((VisionInspection2DMatrix*)proc)->ResetDebugInfo();
            }

            // 영훈 : 잘 안보여서 주석처리한다. 검사 진입!!!!
            /************************************** Start inspeciton *******************************************/
            BOOL bReturnValue = proc->DoInspection(detailSetupMode, i_eSideVisionModule); //kircheis_SurfBug
            /**************************************************************************************************/

            auto& resultState = proc->GetResultState();
            if (resultState.Count() == 0)
            {
                // Result State를 내부적으로 만들지 않는 녀석들은
                // 여기서 검사결과에 따라 State를 만들어 주자
                resultState.SetNum(1);
                if (bReturnValue)
                {
                    if (auto* surface = dynamic_cast<VisionInspectionSurface*>(proc))
                    {
                        if (surface->m_fixedInspectionSpecs.size() && surface->m_fixedInspectionSpecs[0].m_use)
                        {
                            resultState.SetState(0, VisionProcessingState::enumState::Active_Success);
                        }
                    }
                    else
                    {
                        resultState.SetState(0, VisionProcessingState::enumState::Active_Success);
                    }

                    if (proc->m_bInvalid)
                    {
                        resultState.SetState(0, VisionProcessingState::enumState::Active_Invalid);
                    }
                }
                else
                {
                    resultState.SetState(0, VisionProcessingState::enumState::Active_Invalid);
                }
            }

            //if( !nReturnValue )//kircheis_SurfBug
            if (!bReturnValue)
            {
                if (dynamic_cast<VisionProcessingAlign2D*>(proc) || dynamic_cast<VisionProcessingAlign3D*>(proc))
                {
                    if (proc->m_bEmpty) //kircheis_3DEmpty
                    {
                        m_bEmpty = TRUE;
                        long nInspItemNum = (long)visionInspectionResults.size();
                        for (long i = 0; i < nInspItemNum; i++)
                        {
                            if (visionInspectionSpecs[i]->m_use)
                                visionInspectionResults[i]->m_totalResult = EMPTY;
                        }
                    }
                    if (proc->m_bDoubleDevice) //kircheis_3DEmpty
                    {
                        m_bDoubleDevice = TRUE;
                        long nInspItemNum = (long)visionInspectionResults.size();
                        for (long i = 0; i < nInspItemNum; i++)
                        {
                            if (visionInspectionSpecs[i]->m_use)
                                visionInspectionResults[i]->m_totalResult
                                    = DOUBLEDEVICE; //DOUBLEDEVICE;//아직 Double은 소팅룰이 없다//kircheis_3D Double
                        }
                    }
                    else if (proc->m_bInvalid)
                        m_bInvalid = TRUE;
                    //k 좌표
                    else
                        m_bReject = TRUE;
                }
                else if (dynamic_cast<VisionInspectionBgaBottom3D*>(proc) && proc->m_bEmpty) //kircheis_3DEmpty
                {
                    m_bEmpty = TRUE;
                    long nInspItemNum = (long)visionInspectionResults.size();
                    for (long i = 0; i < nInspItemNum; i++)
                    {
                        if (visionInspectionSpecs[i]->m_use)
                            visionInspectionResults[i]->m_totalResult = EMPTY;
                    }
                }
                else if (dynamic_cast<VisionInspectionBgaBottom3D*>(proc) && proc->m_bDoubleDevice) //kircheis_3DEmpty
                {
                    m_bDoubleDevice = TRUE; //kircheis_3DEmpty
                    long nInspItemNum = (long)visionInspectionResults.size();
                    for (long i = 0; i < nInspItemNum; i++)
                    {
                        if (visionInspectionSpecs[i]->m_use)
                            visionInspectionResults[i]->m_totalResult
                                = INVALID; //DOUBLEDEVICE;//아직 Double은 소팅룰이 없다
                    }
                }
                else if (dynamic_cast<VisionInspectionBgaBottom3D*>(proc) && proc->m_bInvalid)
                {
                    m_bInvalid = TRUE; //kircheis_3DEmpty
                    long nInspItemNum = (long)visionInspectionResults.size();
                    for (long i = 0; i < nInspItemNum; i++)
                    {
                        if (visionInspectionSpecs[i]->m_use)
                            visionInspectionResults[i]->m_totalResult = INVALID;
                    }
                }
                else if (dynamic_cast<VisionProcessingCoupon2D*>(proc) && proc->m_bCoupon) //SDY_Coupon2D
                {
                    m_bCoupon = TRUE; //kircheis_3DEmpty
                    long nInspItemNum = (long)visionInspectionResults.size();
                    for (long i = 0; i < nInspItemNum; i++)
                    {
                        if (visionInspectionSpecs[i]->m_use)
                            visionInspectionResults[i]->m_totalResult = COUPON;
                    }
                }
                else if (proc->m_bInvalid)
                {
                    m_bInvalid = TRUE;
                }
                else if (dynamic_cast<VisionInspectionBgaBottom3D*>(proc)
                    || dynamic_cast<VisionInspectionLgaBottom3D*>(proc))
                {
                    m_bInvalid = TRUE;
                }
                else
                {
                    m_bReject = TRUE;
                }

                proc->GetOverlayResult(m_runOverlayResult, detailSetupMode);

                if (1)
                {
                    if (detailSetupMode)
                    {
                        m_lastInspection_listToCollect.clear();
                    }

                    m_lastInspection_listToCollect.push_back(nid);
                }

                if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
                {
                    m_sUnitinspectionLogInfo.SetProcessingFunctionExcuteTimeLog(
                        proc->m_strModuleName, proc->GetInspModuleFunctionExcuteTimeLog());
                    m_sUnitinspectionLogInfo.SetProcessExcuteTimeLog(
                        _T("VisionUnit_RunInspectionTotal[FASLE]"), (float)RunInspectionTotalTime.Elapsed_ms());
                }

                return FALSE;
            }
        }

        proc->GetOverlayResult(m_runOverlayResult, detailSetupMode);

        if (1)
        {
            if (detailSetupMode)
            {
                m_lastInspection_listToCollect.clear();
            }

            m_lastInspection_listToCollect.push_back(nid);
        }

        if (index_specSurface == nid)
        {
            GenerateSpecSurfaceMask();
        }

        if (includeCurrentModule && proc == currentModule)
        {
            if (SystemConfig::GetInstance().Get_Enable_CPU_CALC_SAVE_LOG() == true)
            {
                sVisionInspTimeData.SetInspTimeData(getImageLot().GetInfo().m_trayIndex,
                    getImageLot().GetInfo().m_scanAreaIndex, GetCurrentPaneID(), m_threadIndex,
                    (float)VisionInspTime.Elapsed_ms());

                SystemConfig::GetInstance().SetVisionInspTime(sVisionInspTimeData.GetInspTimeData());
            }

            return TRUE;
        }

        //{{mc_Log Test
        if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
        {
            m_sUnitinspectionLogInfo.SetProcessingFunctionExcuteTimeLog(
                proc->m_strModuleName, proc->GetInspModuleFunctionExcuteTimeLog());
        }
        //}}
    }

    if (SystemConfig::GetInstance().Get_Enable_CPU_CALC_SAVE_LOG() == true)
    {
        sVisionInspTimeData.SetInspTimeData(getImageLot().GetInfo().m_trayIndex,
            getImageLot().GetInfo().m_scanAreaIndex, GetCurrentPaneID(), m_threadIndex,
            (float)VisionInspTime.Elapsed_ms());

        SystemConfig::GetInstance().SetVisionInspTime(sVisionInspTimeData.GetInspTimeData());
    }

    if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
    {
        m_sUnitinspectionLogInfo.SetProcessExcuteTimeLog(
            _T("VisionUnit_RunInspectionTotal"), (float)RunInspectionTotalTime.Elapsed_ms());
    }

    return TRUE;
}

LPCTSTR VisionUnit::GetLastInspection_Text(BOOL isLogSave)
{
    GenerateLastInspectionInfo(isLogSave);
    return m_lastInspeciton_Text;
}

VisionInspectionOverlayResult& VisionUnit::GetInspectionOverlayResult()
{
    return *m_runOverlayResult;
}

void VisionUnit::InitSharedMemBuffers()
{
    // 검사 정보 데이터 공유 메모리
    m_handleInform = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, INFO_SHARED_MEMNAME);

    if (m_handleInform == NULL)
        m_handleInform = ::CreateFileMapping(
            INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(sInfoSharedMemory), INFO_SHARED_MEMNAME);

    if (m_handleInform != NULL)
    {
        m_ptrSharedMem
            = (sInfoSharedMemory*)::MapViewOfFile(m_handleInform, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(sInfoSharedMemory));
        m_ptrSharedMem->bAvailable = FALSE;
    }

    if (m_ptrSharedMem != NULL)
        m_ptrSharedMem->bConnected = TRUE; // 공유 메모리 참조가 되면 연결 상태를 On

    // 이미지 데이터 공유 메모리
    m_handleImage = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, IMAGE_SHARED_MEMNAME);

    long nDataSize = MAX_DL_IMAGE_SIZE_X * MAX_DL_IMAGE_SIZE_Y * FRAME_NUM_MAX;

    if (m_handleImage == NULL)
        m_handleImage
            = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, nDataSize, IMAGE_SHARED_MEMNAME);

    if (m_handleImage != NULL)
    {
        m_ptrSharedImageMem = (BYTE*)::MapViewOfFile(m_handleImage, FILE_MAP_ALL_ACCESS, 0, 0, nDataSize);
    }

    // DeviceResult 데이터 공유 메모리
    m_handleDevResult = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, DEVRESULT_SHARED_MEMNAME);

    if (m_handleDevResult == NULL)
        m_handleDevResult = ::CreateFileMapping(
            INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_SEND_RESULT_SIZE, DEVRESULT_SHARED_MEMNAME);

    if (m_handleDevResult != NULL)
    {
        m_pbySharedDevResult
            = (BYTE*)::MapViewOfFile(m_handleDevResult, FILE_MAP_ALL_ACCESS, 0, 0, MAX_SEND_RESULT_SIZE);
    }
}

void VisionUnit::ReleaseSharedMemBuffers()
{
    if (m_ptrSharedMem != NULL)
        m_ptrSharedMem->bConnected = FALSE;

    if (!m_ptrSharedMem != NULL)
        ::UnmapViewOfFile(m_ptrSharedMem);
    if (!m_ptrSharedImageMem != NULL)
        ::UnmapViewOfFile(m_ptrSharedImageMem);
    if (!m_pbySharedDevResult != NULL)
        ::UnmapViewOfFile(m_pbySharedDevResult);

    if (m_handleImage != NULL)
        ::CloseHandle(m_handleImage);
    if (m_handleInform != NULL)
        ::CloseHandle(m_handleInform);
    if (m_handleDevResult != NULL)
        ::CloseHandle(m_handleDevResult);
}

void* VisionUnit::GetDebugInfoData(CString strDebugInfoName)
{
    auto visionDebugInfos = GetVisionDebugInfos();

    VisionDebugInfo* pDebugInfo = NULL;

    for (auto* info : visionDebugInfos)
    {
        if (info == NULL)
            continue;

        if (info->strDebugInfoName == strDebugInfoName)
        {
            pDebugInfo = info;

            return (void*)pDebugInfo->pData;
        }
    }

    return NULL;
}

long VisionUnit::GetIndex_GenerateSpecSurfaceMask(VisionProcessing* targetModule)
{
    long nInspectionNum = (long)m_vecVisionModules.size();

    long retIndex = nInspectionNum - 1;

    for (long nid = 0; nid < nInspectionNum; nid++)
    {
        if (m_vecVisionModules[nid] == targetModule)
        {
            retIndex = nid;
            break;
        }
    }

    for (long nid = 0; nid < nInspectionNum; nid++)
    {
        auto* proc = m_vecVisionModules[nid];

        if (dynamic_cast<VisionInspectionSurface*>(proc) || dynamic_cast<VisionProcessingGlobalMaskMaker*>(proc))
        {
            // Global Mask나 Surface 보다는 먼저해야 한다
            retIndex = min(retIndex, nid - 1);
        }
    }

    return retIndex;
}

void VisionUnit::MakeTraySpecFromOldRecipe(const OldScanSpec& oldScanSpec)
{
    long sensorSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    long sensorSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    float fovSizeX_mm = SystemConfig::GetInstance()
                            .GetScale(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
                            .convert_pixelToUmX((float)sensorSizeX)
        * 0.001f;
    float fovSizeY_mm = SystemConfig::GetInstance()
                            .GetScale(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
                            .convert_pixelToUmY((float)sensorSizeY)
        * 0.001f;

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
    {
        // 3D 에서는 sensorSizeX가 Y방향이다
        // 그리고 FOV SizeX 방향은 모두 커버 가능한 매우 큰값을 넣어주자
        fovSizeX_mm = 999.f;
        fovSizeY_mm = SystemConfig::GetInstance()
                          .GetScale(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
                          .convert_pixelToUmX((float)sensorSizeX)
            * 0.001f;
    }

    m_trayScanSpec->SetFromOldRecipe(fovSizeX_mm, fovSizeY_mm, *m_pPackageSpec, oldScanSpec);
}

void VisionUnit::GenerateSpecSurfaceMask()
{
    //-----------------------------------------------------------------------------------------------------------
    // Spec을 이용한 Surface Mask를 만드는 루틴입니다.
    //-----------------------------------------------------------------------------------------------------------

    long nDataNum = 0;
    void* pData = GetVisionDebugInfo(GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData == nullptr || nDataNum <= 0)
        return;

    auto* edgeAlignResult = (VisionAlignResult*)pData;
    if (edgeAlignResult == nullptr)
        return;

    Ipvm::Point32r2 px2mm(getScale().pixelToUm().m_x / 1000.f, getScale().pixelToUm().m_y / 1000.f);
    Ipvm::Point32r2 baseCenter = edgeAlignResult->m_center;

    long padDataNum = 0;
    void* padAlignData
        = GetVisionDebugInfo(_VISION_INSP_GUID_PAD_ALIGN_2D, _T("Apply PAD Align to Body Center"), padDataNum);

    if (padAlignData != nullptr && padDataNum > 0)
    {
        // PadAlign이 수행 되었으면 해당 Align 결과를 이용하여 Center 좌표를 계산한다
        baseCenter = *(Ipvm::Point32r2*)padAlignData;
    }

    if (m_pPackageSpec->m_ballMap->m_balls.size() > 0)
    {
        if (auto* surfaceRoi = getReusableMemory().AddSurfaceLayerRoiClass(_T("Spec - Ball")))
        {
            surfaceRoi->Reset();

            std::vector<Ipvm::EllipseEq32r> ballSpecs;
            ballSpecs.clear();
            ballSpecs.resize(m_pPackageSpec->m_ballMap->m_balls.size());

            for (const auto& ball : m_pPackageSpec->m_ballMap->m_balls)
            {
                Ipvm::Point32r2 pos(float(ball.m_posX_px + baseCenter.m_x), float(ball.m_posY_px + baseCenter.m_y));
                CPI_Geometry::RotatePoint(pos, edgeAlignResult->m_angle_rad, baseCenter);

                ballSpecs[ball.m_index].m_x = pos.m_x;
                ballSpecs[ball.m_index].m_y = pos.m_y;
                ballSpecs[ball.m_index].m_xradius = (float)ball.m_radiusX_px;
                ballSpecs[ball.m_index].m_yradius = (float)ball.m_radiusY_px;
            }

            for (auto& ball : ballSpecs)
            {
                surfaceRoi->Add(ball);
            }
        }
    }

    long objectCount_passive = long(m_pPackageSpec->vecDMSPassive.size());
    //long objectCount_chip		= long(m_pPackageSpec->vecDMSChip.size());
    //long objectCount_array		= long(m_pPackageSpec->vecDMSArray.size());
    //long objectCount_heatsink	= long(m_pPackageSpec->vecDMSHeatsink.size());

    if (objectCount_passive > 0 /*|| objectCount_chip > 0 || objectCount_array > 0 || objectCount_heatsink > 0*/)
    {
        if (auto* surfaceRoi = getReusableMemory().AddSurfaceLayerRoiClass(_T("Spec - Component")))
        {
            surfaceRoi->Reset();

            for (long type = 0; type < 4; type++)
            {
                long objectCount = 0;
                switch (type)
                {
                    case 0:
                        objectCount = objectCount_passive;
                        break;
                        //case 1: objectCount = objectCount_chip;		break;
                        //case 2: objectCount = objectCount_array;	break;
                        //case 3: objectCount = objectCount_heatsink;	break;
                }

                for (long index = 0; index < objectCount; index++)
                {
                    Chip::DMSInfo* object = nullptr; //kircheis_MED_Bug_20200305

                    switch (type)
                    {
                        case 0:
                            object = &m_pPackageSpec->vecDMSPassive[index];
                            break; //kircheis_MED_Bug_20200305
                            //case 0: object = &m_pPackageSpec->m_OriginCompData->vecOriginCompData[index];		break;
                            //case 1: object = &m_pPackageSpec->vecDMSChip[index];		break;
                            //case 2: object = &m_pPackageSpec->vecDMSArray[index];		break;
                            //case 3: object = &m_pPackageSpec->vecDMSHeatsink[index];	break;
                    }

                    /*Ipvm::Point32r2 objCenter(object->fChipOffsetX, object->fChipOffsetY);
					Ipvm::Point32r2 chipCenter(baseCenter.m_x + objCenter.m_x / px2mm.m_x,
						baseCenter.m_y - objCenter.m_y / px2mm.m_y);
					Ipvm::Rect32s roi;
					roi.m_left = chipCenter.m_x - (object->fChipLength * 0.5f) / px2mm.m_x;
					roi.m_top = chipCenter.m_y - (object->fChipWidth * 0.5f) / px2mm.m_y;
					roi.m_right = chipCenter.m_x + (object->fChipLength * 0.5f) / px2mm.m_x;
					roi.m_bottom = chipCenter.m_y + (object->fChipWidth * 0.5f) / px2mm.m_y;

					CPI_Geometry::RotateRect(roi, object->fChipAngle * IV_DEG_TO_RAD, chipCenter);
					roi.NormalizeRect();*/

                    //{{//kircheis_MED_Bug_20200305
                    Ipvm::Point32r2 objCenter(object->fChipOffsetX,
                        object->fChipOffsetY); //Ipvm::Point32r2 objCenter(object->fOffsetX, object->fOffsetY);
                    Ipvm::Point32r2 chipCenter(
                        baseCenter.m_x + objCenter.m_x / px2mm.m_x, baseCenter.m_y - objCenter.m_y / px2mm.m_y);
                    Ipvm::Rect32s roi;
                    roi.m_left = (int32_t)(chipCenter.m_x - (object->fChipLength * 0.5f) / px2mm.m_x
                        + 0.5f); //roi.m_left = chipCenter.m_x - (object->fLength * 0.5f) / px2mm.m_x;
                    roi.m_top = (int32_t)(chipCenter.m_y - (object->fChipWidth * 0.5f) / px2mm.m_y
                        + 0.5f); //roi.m_top = chipCenter.m_y - (object->fWidth * 0.5f) / px2mm.m_y;
                    roi.m_right = (int32_t)(chipCenter.m_x + (object->fChipLength * 0.5f) / px2mm.m_x
                        + 0.5f); //roi.m_right = chipCenter.m_x + (object->fLength * 0.5f) / px2mm.m_x;
                    roi.m_bottom = (int32_t)(chipCenter.m_y + (object->fChipWidth * 0.5f) / px2mm.m_y
                        + 0.5f); //roi.m_bottom = chipCenter.m_y + (object->fWidth * 0.5f) / px2mm.m_y;

                    CPI_Geometry::RotateRect(roi, (float)(object->fChipAngle * IV_DEG_TO_RAD),
                        Ipvm::Conversion::ToPoint32s2(
                            chipCenter)); //CPI_Geometry::RotateRect(roi, object->nAngle * IV_DEG_TO_RAD, chipCenter);
                    //}}
                    roi.NormalizeRect();

                    surfaceRoi->Add(roi);
                }
            }
        }
    }

    return;
}

void VisionUnit::NewInspItem()
{
    m_vecVisionModules.clear();

    std::vector<VisionProcessing*> availableFixedItems;

    GetAvailableFixedItemList(availableFixedItems);

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        m_vecVisionModules.push_back(m_visionProcessingNGRV); //MED#6_NGRV_JHB
    }
    else
    {
        m_vecVisionModules.push_back(m_visionProcessingSpec_PackageLayoutEditor); // Layout Editor for Ball
        m_vecVisionModules.push_back(m_VisionProcessingFOV); // FOV Editor
    }

    for (auto* proc : m_vecVisionModules)
    {
        proc->ResetSpecAndPara();
        proc->ResetResult();
    }

    for (auto* proc : availableFixedItems)
    {
        proc->SetEnabled(FALSE);
    }
}

void VisionUnit::UpdateInspItem(const std::vector<VisionProcessing*>& availableFixedItems, enPackageType nPackageType,
    BOOL bDeadBug) //kircheis_TRV 수정  검사항목추가
{
    if (nPackageType == enPackageType::UNKNOWN)
    {
        nPackageType = m_pPackageSpec->nPackageType;
        bDeadBug = m_pPackageSpec->m_deadBug;
    }

    m_vecVisionModules.clear();

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        m_vecVisionModules.push_back(m_visionProcessingNGRV); //MED#6_NGRV_JHB
    }
    else
    {
        m_vecVisionModules.push_back(m_visionProcessingSpec_PackageLayoutEditor); // Layout Editor for Ball
        m_vecVisionModules.push_back(m_VisionProcessingFOV); // FOV Editor
    }

    //-----------------------------------------------------------------------------
    // 일반 Processing 및 검사항목 추가
    //-----------------------------------------------------------------------------

    for (auto* proc : availableFixedItems)
    {
        if (proc->IsEnabled())
        {
            // 해당 Process는 다른 곳에 배치할 것임
            if (dynamic_cast<VisionProcessingGlobalMaskMaker*>(proc) != nullptr)
                continue;

            m_vecVisionModules.push_back(proc);
        }
    }

    for (auto* proc : m_visionInspectionMarks)
    {
        if (proc->IsEnabled())
        {
            m_vecVisionModules.push_back(proc);
        }
    }

    for (auto* proc : m_visionInspectionGeometries)
    {
        if (proc->IsEnabled())
        {
            m_vecVisionModules.push_back(proc);
        }
    }

    //-----------------------------------------------------------------------------
    // Surface Mask 추가
    //-----------------------------------------------------------------------------

    for (auto* proc : m_visionInspectionSurfaces)
    {
        if (proc->IsEnabled())
        {
            if (proc->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
            {
                m_vecVisionModules.push_back(proc);
            }
        }
    }

    //-----------------------------------------------------------------------------
    // Global Mask Maker 추가
    //-----------------------------------------------------------------------------

    for (auto* proc : availableFixedItems)
    {
        if (proc->IsEnabled())
        {
            // 해당 Process는 다른 곳에 배치할 것임
            if (dynamic_cast<VisionProcessingGlobalMaskMaker*>(proc) != nullptr)
            {
                m_vecVisionModules.push_back(proc);
            }
        }
    }

    //-----------------------------------------------------------------------------
    // Surface 추가
    //-----------------------------------------------------------------------------

    for (auto* proc : m_visionInspectionSurfaces)
    {
        if (proc->IsEnabled())
        {
            if (!proc->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
            {
                m_vecVisionModules.push_back(proc);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 영훈 20151118
// bHardwarJobEnable : Hardware job이 별도로 존재하는지 알아온다.
BOOL VisionUnit::LinkDataBase(long jobVersion, BOOL bSave, CiDataBase& JobDB)
{
    long version = 3;
    if (!JobDB[_T("Version")].Link(bSave, version))
        version = 0;

    if (!m_pPackageSpec->LinkDataBase(bSave, JobDB[_T("PACKAGE_SPEC")]))
        return FALSE;

    static const bool bIsSideVision
        = (bool)SystemConfig::GetInstance()
              .IsVisionTypeSide(); //Side Vision 일 경우 Geometry와 Mark는 절대 Open 못하게 한다.
    // 로딩시 모두 초기화
    if (!bSave)
    {
        InspectionClear();

        if (version < 3)
        {
            OldScanSpec oldScanSpec;

            if (!oldScanSpec.LinkAreaImageInfo(bSave, JobDB[_T("AREASCAN_IMAGEINFO")]))
            {
                return FALSE;
            }

            // 이곳저곳에서 참조할 Scan정보를 설정한다

            auto& mainDB = JobDB[_T("VisionMain")];

            switch (version)
            {
                case 0:
                    // 예전 Job은 반시계 방향으로 이미지가 회전되어 있어서 split 갯수가 반대였다.
                    if (!mainDB[_T("SplitGrabModeX")].Link(bSave, oldScanSpec.m_stitchCountY))
                        oldScanSpec.m_stitchCountY = 1;
                    if (!mainDB[_T("SplitGrabModeY")].Link(bSave, oldScanSpec.m_stitchCountX))
                        oldScanSpec.m_stitchCountX = 1;
                    break;

                default:
                    if (!mainDB[_T("SplitGrabModeX")].Link(bSave, oldScanSpec.m_stitchCountX))
                        oldScanSpec.m_stitchCountX = 1;
                    if (!mainDB[_T("SplitGrabModeY")].Link(bSave, oldScanSpec.m_stitchCountY))
                        oldScanSpec.m_stitchCountY = 1;
                    break;
            }

            if (jobVersion < 3)
            {
                // 예전에는 Align 2D에 해당 정보가 있었다
                //auto& align2d = JobDB[_T("{C19EBF32-85E4-4E03-8CD2-87A415A4FBFB}")];
                auto& alignSpec = JobDB[_T("{F99A3745-A427-49B5-AFC1-455B1337CDDF}")];

                if (!alignSpec[_T("fTemplateMoveFovX")].Link(bSave, oldScanSpec.m_moveDistanceXbetweenFOVs_mm))
                    oldScanSpec.m_moveDistanceXbetweenFOVs_mm = 0.f;
                if (!alignSpec[_T("fTemplateMoveFovY")].Link(bSave, oldScanSpec.m_moveDistanceYbetweenFOVs_mm))
                    oldScanSpec.m_moveDistanceYbetweenFOVs_mm = 0.f;
            }
            else
            {
                if (!mainDB[_T("Move Distance X between FOVs (mm)")].Link(
                        bSave, oldScanSpec.m_moveDistanceXbetweenFOVs_mm))
                    oldScanSpec.m_moveDistanceXbetweenFOVs_mm = 0.f;
                if (!mainDB[_T("Move Distance Y between FOVs (mm)")].Link(
                        bSave, oldScanSpec.m_moveDistanceYbetweenFOVs_mm))
                    oldScanSpec.m_moveDistanceYbetweenFOVs_mm = 0.f;
            }

            //{{ //kircheis_580 Debug
            if (oldScanSpec.m_stitchCountX < 1 || oldScanSpec.m_stitchCountX > 4)
                oldScanSpec.m_stitchCountX = 1;
            if (oldScanSpec.m_stitchCountY < 1 || oldScanSpec.m_stitchCountY > 4)
                oldScanSpec.m_stitchCountY = 1;
            //}}

            MakeTraySpecFromOldRecipe(oldScanSpec);
        }
    }

    if (version >= 3)
    {
        // 아직 작업중이므로 version을 올리지는 않았다.
        // 나중에 작업완료되면 version을 3으로 올리고 동작하게 할 것이다
        m_trayScanSpec->LinkDataBase(bSave, JobDB[_T("Tray Scan Spec")]);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //{{ Fixed Inspection Items
    {
        m_visionProcessingSpec_PackageLayoutEditor->LinkDataBase(
            bSave, JobDB[m_visionProcessingSpec_PackageLayoutEditor->m_moduleGuid]);
        m_VisionProcessingFOV->LinkDataBase(bSave, JobDB[m_VisionProcessingFOV->m_moduleGuid]);

        std::vector<VisionProcessing*> availableFixedItems;

        GetAvailableFixedItemList(availableFixedItems);
        UpdateInspItem(availableFixedItems, m_pPackageSpec->nPackageType, m_pPackageSpec->m_deadBug);

        long nProcNum = (long)availableFixedItems.size();
        for (long nProc = 0; nProc < nProcNum; nProc++)
        {
            auto* proc = availableFixedItems[nProc];

            if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP) //NGRV Single Run관련 예외처리
            {
                if (auto* NGRVvision = dynamic_cast<VisionProcessingNGRV*>(proc))
                {
                    //Recipe에는 크기문제로 원본이 아닌 1/4의 Size로 저장되어있기 때문에 이를 재 계산
                    auto imageInfo_Btm = m_ngrv2DImageInfo_BTM;
                    long resizeImageSizeX_Btm(0), resizeImageSizeY_Btm(0);
                    resizeImageSizeX_Btm = ((imageInfo_Btm.m_imageSizeX * 4 + 3) / 4) * 4;
                    resizeImageSizeY_Btm = ((imageInfo_Btm.m_imageSizeY * 4 + 3) / 4) * 4;

                    NGRVvision->SetNGRV_ref_image_size(VISION_BTM_2D, resizeImageSizeX_Btm, resizeImageSizeY_Btm);

                    auto imageInfo_Top = m_ngrv2DImageInfo_TOP;
                    long resizeImageSizeX_Top(0), resizeImageSizeY_Top(0);
                    resizeImageSizeX_Top = ((imageInfo_Top.m_imageSizeX * 4 + 3) / 4) * 4;
                    resizeImageSizeY_Top = ((imageInfo_Top.m_imageSizeY * 4 + 3) / 4) * 4;

                    NGRVvision->SetNGRV_ref_image_size(VISION_TOP_2D, resizeImageSizeX_Top, resizeImageSizeY_Top);
                }
            }

            if (!proc->LinkDataBase(bSave, JobDB[proc->m_moduleGuid]))
                return FALSE;

            if (version == 0)
            {
                // 옛날에는 Stitch와 Align이 따로 있었다
                auto& edgeAlignDB
                    = JobDB[_T("{94CCA928-E484-498D-978E-041453C25C72}")][_T("{2EAF49EA-878D-4EB4-AE0A-E22272C8E9DB}")];
                if (auto* align2D = dynamic_cast<VisionProcessingAlign2D*>(proc))
                {
                    float fPackageSizeX = 0.f;
                    float fPackageSizeY = 0.f;
                    GetPackageSize(false, fPackageSizeX, fPackageSizeY);
                    align2D->m_edgeAlign_para->LinkDataBase(bSave, fPackageSizeX, fPackageSizeY, edgeAlignDB);
                }

                if (auto* align3D = dynamic_cast<VisionProcessingAlign3D*>(proc))
                {
                    align3D->m_edgeAlign_para->LinkDataBase(bSave, m_pPackageSpec->m_bodyInfoMaster->GetBodySizeX(),
                        m_pPackageSpec->m_bodyInfoMaster->GetBodySizeY(), edgeAlignDB);
                }
            }
        }
    }
    //}} Fixed Inspection Items
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    auto& visionUnitDB = JobDB[_T("VisionUnit")];

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //{{ Mark Inspection Items
    {
        long nSize = (long)m_visionInspectionMarks.size();

        if (bIsSideVision && bSave)
            nSize = 0; //Side Vision 이면 Mark는 Save 하지마.

        if (!visionUnitDB[_T("VisionMarkGuidCount")].Link(bSave, nSize))
            nSize = 0;

        if (bIsSideVision)
            nSize = 0; //Side Vision 이면 Mark는 Save, Load 둘다 하지마.

        if (bSave)
        {
            for (long n = 0; n < nSize; n++)
            {
                CString guid = m_visionInspectionMarks[n]->m_moduleGuid;

                visionUnitDB.GetSubDBFmt(_T("VisionMarkGuid_%d"), n).Link(bSave, guid);
            }
        }
        else
        {
            // 로드시 기존 데이터를 제거
            for (long n = 0; n < (long)m_visionInspectionMarks.size(); n++)
            {
                delete m_visionInspectionMarks[n];
            }
            m_visionInspectionMarks.clear();

            std::vector<CString> visionMarkGuids;

            for (long n = 0; n < nSize; n++)
            {
                CString guid;

                visionUnitDB.GetSubDBFmt(_T("VisionMarkGuid_%d"), n).Link(bSave, guid);

                if (!guid.IsEmpty())
                {
                    visionMarkGuids.push_back(guid);
                }
            }

            for (const auto& guid : visionMarkGuids)
            {
                m_visionInspectionMarks.push_back(new VisionInspectionMark(guid, nullptr, *this, *m_pPackageSpec));
            }
        }

        for (auto* insp : m_visionInspectionMarks)
        {
            if (!insp->LinkDataBase(bSave, JobDB[insp->m_moduleGuid]))
                return FALSE;

            if (version < 2 && !bSave)
            {
                // 기존에 쓰지 않던 값을 앞으로는 사용하려고 한다. 기존것은 사용하는 것이 었다
                insp->SetEnabled(TRUE);
            }
        }
    }
    //}} Mark Inspection Items
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //{{ Geometry Inspection Items
    {
        long nSize = (long)m_visionInspectionGeometries.size();

        if (!visionUnitDB[_T("VisionGeometryGuidCount")].Link(bSave, nSize))
            nSize = 0;

        if (bSave)
        {
            for (long n = 0; n < nSize; n++)
            {
                CString guid = m_visionInspectionGeometries[n]->m_moduleGuid;

                visionUnitDB.GetSubDBFmt(_T("VisionGeometryGuid_%d"), n).Link(bSave, guid);
            }
        }
        else
        {
            // 로드시 기존 데이터를 제거
            for (long n = 0; n < (long)m_visionInspectionGeometries.size(); n++)
            {
                delete m_visionInspectionGeometries[n];
            }
            m_visionInspectionGeometries.clear();

            std::vector<CString> visionGeometryGuids;

            for (long n = 0; n < nSize; n++)
            {
                CString guid;

                visionUnitDB.GetSubDBFmt(_T("VisionGeometryGuid_%d"), n).Link(bSave, guid);

                if (!guid.IsEmpty())
                {
                    visionGeometryGuids.push_back(guid);
                }
            }

            for (const auto& guid : visionGeometryGuids)
            {
                m_visionInspectionGeometries.push_back(
                    new VisionInspectionGeometry(guid, nullptr, *this, *m_pPackageSpec));
            }
        }

        for (auto* insp : m_visionInspectionGeometries)
        {
            if (!insp->LinkDataBase(bSave, JobDB[insp->m_moduleGuid]))
                return FALSE;

            if (version < 2 && !bSave)
            {
                // 기존에 쓰지 않던 값을 앞으로는 사용하려고 한다. 기존것은 사용하는 것이 었다
                insp->SetEnabled(TRUE);
            }
        }
    }
    //}} Geometry Inspection Items
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //{{ Surface Inspection Items
    {
        long nSize = (long)m_visionInspectionSurfaces.size();
        if (!visionUnitDB[_T("VisionSurfaceGuidCount")].Link(bSave, nSize))
            nSize = 0;

        if (bSave)
        {
            for (long n = 0; n < nSize; n++)
            {
                CString guid = m_visionInspectionSurfaces[n]->m_moduleGuid;

                visionUnitDB.GetSubDBFmt(_T("VisionSurfaceGuid_%d"), n).Link(bSave, guid);
            }
        }
        else
        {
            // 로드시 기존 데이터를 제거
            for (long n = 0; n < (long)m_visionInspectionSurfaces.size(); n++)
            {
                delete m_visionInspectionSurfaces[n];
            }
            m_visionInspectionSurfaces.clear();

            std::vector<CString> visionSurfaceGuids;

            for (long n = 0; n < nSize; n++)
            {
                CString guid;

                visionUnitDB.GetSubDBFmt(_T("VisionSurfaceGuid_%d"), n).Link(bSave, guid);

                if (!guid.IsEmpty())
                {
                    visionSurfaceGuids.push_back(guid);
                }
            }

            for (long index = 0; index < long(visionSurfaceGuids.size()); index++)
            {
                const auto& guid = visionSurfaceGuids[index];
                m_visionInspectionSurfaces.push_back(
                    new VisionInspectionSurface(guid, nullptr, *this, *m_pPackageSpec));
            }
        }

        for (auto* insp : m_visionInspectionSurfaces)
        {
            if (!insp->LinkDataBase(bSave, JobDB[insp->m_moduleGuid]))
                return FALSE;

            if (version < 2 && !bSave)
            {
                // 기존에 쓰지 않던 값을 앞으로는 사용하려고 한다.
                insp->SetEnabled(insp->m_fixedInspectionSpecs[0].m_use);
            }
        }
    }
    //}} Surface Inspection Items
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (!bSave)
    {
        std::vector<VisionProcessing*> availableFixedItems;

        GetAvailableFixedItemList(availableFixedItems);

        UpdateInspItem(availableFixedItems, m_pPackageSpec->nPackageType, m_pPackageSpec->m_deadBug);
    }

    return TRUE;
}

BOOL VisionUnit::LinkDataBase_PackageSpec(BOOL bSave, CiDataBase& JobDB)
{
    if (!m_pPackageSpec->LinkDataBase(bSave, JobDB[_T("PACKAGE_SPEC")]))
        return false;

    return true;
}

void VisionUnit::GetAvailableFixedItemList(std::vector<VisionProcessing*>& availableFixedItems)
{
    availableFixedItems.clear();

    if (m_systemConfig.GetVisionType() == VISIONTYPE_2D_INSP) //kircheis_TRV
    {
        availableFixedItems.push_back(m_visionProcessingCoupon2D); //SDY_Coupon2D
        availableFixedItems.push_back(m_visionProcessingAlign2D);
        availableFixedItems.push_back(m_visionInspection2DMatrix);
        availableFixedItems.push_back(m_visionInspectionPackageSize);
        availableFixedItems.push_back(m_visionProcessingFiducialAlign);
        availableFixedItems.push_back(m_visionProcessingPadAlign2D);
        availableFixedItems.push_back(m_visionInspectionGlassSubstrate2D);
        availableFixedItems.push_back(m_visionInspectionBgaBottom2D);
        availableFixedItems.push_back(m_visionInspectionBgaBallBridge);
        availableFixedItems.push_back(m_visionInspectionBgaBallPixelQuality);
        availableFixedItems.push_back(m_visionInspectionLgaBottom2D);
        //availableFixedItems.push_back(m_visionInspectionPatch2D);//kircheis_POI //Component나 Die가 Patch 위에 있으면 Patch Align 결과를 사용해야 한다.
        availableFixedItems.push_back(m_visionInspectionComponent2D);
        availableFixedItems.push_back(m_visionInspectionDie2D); //kircheis_InspLID
        availableFixedItems.push_back(m_visionInspectionLid2D); //kircheis_InspLID
        //availableFixedItems.push_back(m_visionInspectionKOZ2D);//kircheis_KOZ
        availableFixedItems.push_back(m_visionInspectionIntensityChecker2D); //SDY_IntensityChecker
        availableFixedItems.push_back(m_visionProcessingOtherAlign);
        availableFixedItems.push_back(m_visionProcessingGlobalMaskMaker);
    }
    else if (m_systemConfig.GetVisionType() == VISIONTYPE_3D_INSP) // && bDeadBug)  // 3D Inspection 일때
    {
        availableFixedItems.push_back(m_visionProcessingAlign3D);
        availableFixedItems.push_back(m_visionInspectionPackageSize);
        availableFixedItems.push_back(m_visionProcessingFiducialAlign);
        availableFixedItems.push_back(m_visionInspectionBgaBottom3D);
        availableFixedItems.push_back(m_visionInspectionLgaBottom3D);
        //availableFixedItems.push_back(m_visionInspectionPatch3D);//kircheis_POI
        availableFixedItems.push_back(m_visionInspectionComponent3D);
        availableFixedItems.push_back(m_visionInspectionLid3D); //kircheis_InspLID
        availableFixedItems.push_back(m_visionProcessingOtherAlign);
        availableFixedItems.push_back(m_visionProcessingGlobalMaskMaker);
    }
    else if (m_systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP)
    {
        availableFixedItems.push_back(m_visionProcessingAlign2D);
        availableFixedItems.push_back(m_visionProcessingSideDetailAlign);
        availableFixedItems.push_back(m_visionInspectionSide2D);
        availableFixedItems.push_back(m_visionProcessingOtherAlign);
        availableFixedItems.push_back(m_visionProcessingGlobalMaskMaker);
    }
    else if (m_systemConfig.GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        availableFixedItems.push_back(m_visionProcessingNGRV); //MED#6_NGRV_JHB
    }
    else if (m_systemConfig.GetVisionType() == VISIONTYPE_SWIR_INSP) //kircheis_SWIR
    {
        availableFixedItems.push_back(m_visionProcessingAlign2D);
        availableFixedItems.push_back(m_visionProcessingGlobalMaskMaker);
    }
}

void VisionUnit::OnImagingConditionChanged(const long nVisionType, const long maxImageSizeX, const long maxImageSizeY)
{
    m_VisionReusableMemory->SetBufferImageSize(maxImageSizeX, maxImageSizeY, nVisionType);

    m_pPackageSpec->CalcDependentVariables(m_imageLot->GetScale());
}

// 현식 마크티치관련 수정
void VisionUnit::ShowProcessingTeachingUI(HWND hwndParent)
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("Show teaching UI"));

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseProcessingTeachingUI();

    CWnd* parentWnd = CWnd::FromHandle(hwndParent);

    m_visionUnitDlg = new VisionUnitDlg(*this, parentWnd);
    m_visionUnitDlg->Create(VisionUnitDlg::IDD, parentWnd);
}

void VisionUnit::CloseProcessingTeachingUI()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_visionUnitDlg->GetSafeHwnd())
    {
        m_visionUnitDlg->DestroyWindow();
    }

    delete m_visionUnitDlg;
    m_visionUnitDlg = nullptr;
}

BOOL VisionUnit::SetPackageType()
{
    CDeviceType DeviceType;

    if (DeviceType.DoModal() == IDOK)
    {
        InspectionClear();

        m_pPackageSpec->Init(); // New Job이니까 초기화 하자...
        m_pPackageSpec->nPackageType = DeviceType.m_nPackageType;
        m_pPackageSpec->nDeviceType = DeviceType.m_nDeviceType;
        m_pPackageSpec->m_deadBug = DeviceType.m_bDeadBug;

        m_main.m_strJobFileName = DeviceType.m_strJobName;

        NewInspItem();

        return TRUE;
    }

    return FALSE;
}

void VisionUnit::SetTrayScanSpec(const VisionTrayScanSpec& spec)
{
    *m_trayScanSpec = spec;
}

BOOL VisionUnit::GetParaData(CString strModuleName, ImageProcPara& i_ImageProcPara, long nMarkID /*= -1*/)
{
    for (int i = 0; i < m_visionInspectionSurfaces.size(); i++)
    {
        if (m_visionInspectionSurfaces[i]->m_fixedInspectionSpecs[0].m_inspName == strModuleName)
        {
            i_ImageProcPara = m_visionInspectionSurfaces[i]->m_surfacePara->m_ImageProcMangePara;

            return TRUE;
        }
    }

    if (nMarkID > -1)
    {
        i_ImageProcPara = m_visionInspectionMarks[nMarkID]->m_VisionPara->m_ImageProcMangePara;
    }
    else if (m_visionInspectionBgaBallPixelQuality->m_strModuleName == strModuleName) //kircheis_BPQ //kircheis_AddInsp
    {
        i_ImageProcPara = m_visionInspectionBgaBallPixelQuality->m_VisionPara->m_ImageProcMangePara;
    }
    //else if (m_visionProcessingAlign2D->m_strModuleName == strModuleName)
    //{
    //	i_ImageProcPara = m_visionProcessingAlign2D->m_VisionPara.m_ImageProcMangePara;
    //}
    //else if (m_visionProcessingPadAlign2D->m_strModuleName == strModuleName)
    //{
    //	i_ImageProcPara = m_visionProcessingPadAlign2D->m_VisionPara->GetAlignPAD_Para();
    //}
    else if (m_visionProcessingOtherAlign->m_strModuleName == strModuleName)
    {
        i_ImageProcPara = m_visionProcessingOtherAlign->m_VisionPara->m_ImageProcMangePara;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

void VisionUnit::GenerateLastInspectionInfo(BOOL isLogSave)
{
    if (!m_lastInspection_InfoMake || isLogSave)
    {
        m_lastInspeciton_Text = _T("");

        for (auto moduleIndex : m_lastInspection_listToCollect)
        {
            auto* proc = m_vecVisionModules[moduleIndex];
            proc->AppendTextResult(m_lastInspeciton_Text);
        }
        m_lastInspection_InfoMake = true;
    }
}

void VisionUnit::GetSpecData(
    LPCTSTR moduleName, std::vector<VisionInspectionSpec>& vecVisionItemSpec, long nMarkID /*= -1*/)
{
    VisionInspection* inspectionModule = nullptr;
    vecVisionItemSpec.clear();

    if (nMarkID > -1)
    {
        inspectionModule = m_visionInspectionMarks[nMarkID];
    }
    else
    {
        for (auto predefineModule : m_vecAllPredefineVisionModulesBuffer)
        {
            auto* checkInspModule = dynamic_cast<VisionInspection*>(predefineModule);
            if (checkInspModule == nullptr)
                continue;

            if (checkInspModule->m_strModuleName == moduleName)
            {
                inspectionModule = checkInspModule;
                break;
            }
        }
    }

    if (inspectionModule)
    {
        vecVisionItemSpec.insert(vecVisionItemSpec.end(), inspectionModule->m_fixedInspectionSpecs.begin(),
            inspectionModule->m_fixedInspectionSpecs.end());
        vecVisionItemSpec.insert(vecVisionItemSpec.end(), inspectionModule->m_variableInspectionSpecs.begin(),
            inspectionModule->m_variableInspectionSpecs.end());
    }
}

void VisionUnit::SetSpecData(
    LPCTSTR moduleName, const std::vector<VisionInspectionSpec>& vecVisionItemSpec, long nMarkID /*= -1*/)
{
    if (nMarkID > -1)
    {
        long nSize = (long)m_visionInspectionMarks[nMarkID]->m_fixedInspectionSpecs.size();

        for (long n = 0; n < nSize; n++)
        {
            m_visionInspectionMarks[nMarkID]->SetSpec(vecVisionItemSpec[n].m_specName, vecVisionItemSpec[n].m_use,
                vecVisionItemSpec[n].m_useMax, vecVisionItemSpec[n].m_useMin, vecVisionItemSpec[n].m_useMarginal,
                vecVisionItemSpec[n].m_marginalMax, vecVisionItemSpec[n].m_passMax, vecVisionItemSpec[n].m_passMin,
                vecVisionItemSpec[n].m_marginalMin);
        }
        return;
    }

    for (auto predefineModule : m_vecAllPredefineVisionModulesBuffer)
    {
        auto* inspectionModule = dynamic_cast<VisionInspection*>(predefineModule);
        if (inspectionModule == nullptr)
            continue;

        if (inspectionModule->m_strModuleName == moduleName)
        {
            long size = (long)(inspectionModule->m_fixedInspectionSpecs.size()
                + inspectionModule->m_variableInspectionSpecs.size());

            for (long n = 0; n < size; n++)
            {
                inspectionModule->SetSpec(vecVisionItemSpec[n].m_specName, vecVisionItemSpec[n].m_use,
                    vecVisionItemSpec[n].m_useMax, vecVisionItemSpec[n].m_useMin, vecVisionItemSpec[n].m_useMarginal,
                    vecVisionItemSpec[n].m_marginalMax, vecVisionItemSpec[n].m_passMax, vecVisionItemSpec[n].m_passMin,
                    vecVisionItemSpec[n].m_marginalMin);
            }

            return;
        }
    }
}

// 영훈 20141112_SurfaceJobManager : Surface 항목만 별도로 저장 및 열기를 할 수 있도록 기능추가
BOOL VisionUnit::SurfaceJobManager(BOOL bSave, long nSurfaceID, CString strFilePath)
{
    CiDataBase JobDB_Temp;
    JobDB_Temp.Reset();

    std::vector<long> vecnIllumInfo;
    CString strModule = _T("SurfaceInsepction");
    CString strKey = _T("Manager");
    CString strSurfaceModuleName = m_visionInspectionSurfaces[nSurfaceID]->m_strModuleName;
    CString strTemp;

    if (bSave)
    {
        if (!JobDB_Temp[strModule][strKey].Link(bSave, strSurfaceModuleName))
            return FALSE;

        if (!m_visionInspectionSurfaces[nSurfaceID]->LinkDataBase(bSave, JobDB_Temp))
            return FALSE;

        JobDB_Temp.Save(strFilePath);
    }
    else
    {
        vecnIllumInfo = m_visionInspectionSurfaces[nSurfaceID]->GetImageFrameList();

        JobDB_Temp.Load(strFilePath);

        if (!JobDB_Temp[strModule][strKey].Link(bSave, strSurfaceModuleName))
        {
            return FALSE;
        }

        VisionInspectionSurface* pVisionSurface
            = new VisionInspectionSurface(::CreateGUID(), strSurfaceModuleName, *this, *m_pPackageSpec);

        if (pVisionSurface->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode
            != m_visionInspectionSurfaces[nSurfaceID]->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
        {
            // Surface 자리에 Surface Mask를 넣는다든지는 허용하지 말자
            delete pVisionSurface;
            return FALSE;
        }

        if (m_visionInspectionSurfaces[nSurfaceID] != NULL)
        {
            delete m_visionInspectionSurfaces[nSurfaceID];
        }

        m_visionInspectionSurfaces[nSurfaceID] = pVisionSurface;

        if (!m_visionInspectionSurfaces[nSurfaceID]->LinkDataBase(bSave, JobDB_Temp))
            return FALSE;

        m_visionInspectionSurfaces[nSurfaceID]->GetImageFrameList() = vecnIllumInfo;
    }

    return TRUE;
}

void VisionUnit::RunInspectionCurrentProcessingModule()
{
    m_currentProcessingModule->SetCurVisionModule_Status(enSideVisionModule(GetSideVisionSection()));
    m_currentProcessingModule->OnInspection();
    if (m_visionUnitDlg)
    {
        m_visionUnitDlg->updateProcessorResult();
    }
}

long VisionUnit::GetVisionProcessingCount() const
{
    return long(m_vecVisionModules.size());
}

VisionProcessing* VisionUnit::GetVisionProcessing(long index)
{
    long nModuleCnt = (long)m_vecVisionModules.size();

    if (index < 0 || index >= nModuleCnt)
        return NULL;

    return m_vecVisionModules[index];
}

VisionProcessing* VisionUnit::GetVisionProcessing(CString strModuleGUID)
{
    for (auto& visionModule : m_vecVisionModules)
    {
        if (visionModule->m_moduleGuid == strModuleGUID)
            return visionModule;
    }

    return NULL;
}

long VisionUnit::GetInspTotalResult()
{
    long nTotalResult(NOT_MEASURED);

    auto visionInspectionSpecs = GetVisionInspectionSpecs();
    auto visionInspectionResults = GetVisionInspectionResults();

    for (long i = 0; i < (long)(visionInspectionResults.size()); i++)
    {
        CString strInspName = visionInspectionResults[i]->m_inspName;

        if (strInspName.Find(_T("MSK_")) > -1)
            continue;

        // 영훈 20151124 : BGA 검사항목을 예를 들어 주석을 담.
        // BGA Missing이 발생하면 Offset, pitch, Width는 검사하지 않고 Not measured 처리되어 들어온다.
        if (nTotalResult == PASS && visionInspectionSpecs[i]->m_use == TRUE)
        {
            long nResult(NOT_MEASURED);
            long nTotalSpecNum = (long)visionInspectionSpecs.size();

            for (long nSpecID = i; nSpecID < nTotalSpecNum;
                nSpecID++) // BGA Spec 만큼 검사하여 개별 검사항목의 Total Result를 확인하고
            {
                if (visionInspectionResults[nSpecID]->m_inspName == strInspName)
                {
                    nResult = max(nResult, visionInspectionResults[nSpecID]->m_totalResult);
                }
            }

            if (nResult
                == NOT_MEASURED) // 전단계까지 Pass인데 Ball 검사항목이 모두 Not measured라면 문제가 있는 것이다.
            {
                visionInspectionResults[i]->m_totalResult = INVALID; // 그럼 BGA Inspection은 모두 invalid 처리한다.
            }
        }

        nTotalResult = max(nTotalResult, visionInspectionResults[i]->m_totalResult);
    }

    //k 좌표
    if (m_bMarginal && nTotalResult < MARGINAL)
    {
        nTotalResult = MARGINAL;
    }
    if (m_bReject && nTotalResult < REJECT)
    {
        nTotalResult = REJECT;
    }
    if (m_bInvalid && nTotalResult < INVALID)
    {
        nTotalResult = INVALID;
    }
    if (m_bDoubleDevice && nTotalResult < DOUBLEDEVICE) //kircheis_3DEmpty
    {
        nTotalResult = DOUBLEDEVICE;
    }
    if (m_bCoupon && nTotalResult < COUPON) //SDY_Coupon2D
    {
        nTotalResult = COUPON;
    }
    if (m_bEmpty && nTotalResult < EMPTY)
    {
        nTotalResult = EMPTY;
    }

    return nTotalResult;
}

void VisionUnit::GetSwitchMarkTeachViewIndex(long& o_CurMarkTeachViewIndex)
{
    o_CurMarkTeachViewIndex = m_visionUnitDlg->m_CtrlTabSwitchAceesMode.GetCurSel();
}

long VisionUnit::GetCurrentPaneID() const
{
    long inspectionID = getImageLot().GetInfo().m_scanAreaIndex;
    if (inspectionID < 0 || inspectionID >= m_trayScanSpec->GetInspectionItemCount())
    {
        return m_trayScanSpec->GetCurrentPaneID();
    }

    int paneCountX = (int)m_trayScanSpec->GetInspectionItem(inspectionID).m_pocketArrayX.size();
    int paneCountY = (int)m_trayScanSpec->GetInspectionItem(inspectionID).m_pocketArrayY.size();

    return m_trayScanSpec->GetCurrentPaneID(paneCountX * paneCountY);
}

bool VisionUnit::SetCurrentPaneID(long nID)
{
    return m_trayScanSpec->SetCurrentPaneID(nID);
}

void VisionUnit::setImageLot(std::shared_ptr<VisionImageLot> imageLot)
{
    m_imageLot = imageLot;
    //m_pImageLotInsp->Set(*m_imageLot);

    //if (!m_pPackageSpec->IsSameDepedentScale(imageLot->GetScale()))
    //{
    //	m_pPackageSpec->CalcDependentVariables(imageLot->GetScale());
    //}
}

void VisionUnit::setImageLotInsp()
{
    static const long snRepeatSetCount
        = SystemConfig::GetInstance().IsVisionTypeSide() == TRUE ? 2 : 1; //얘가 전역인게 문제다

    for (long nidx = 0; nidx < snRepeatSetCount; nidx++)
        m_pImageLotInsp->Set(*m_imageLot, enSideVisionModule(nidx));

    if (!m_pPackageSpec->IsSameDepedentScale(m_imageLot->GetScale()))
    {
        m_pPackageSpec->CalcDependentVariables(m_imageLot->GetScale());
    }
}

const VisionScale& VisionUnit::getScale() const
{
    return m_imageLot->GetScale();
}

const VisionTrayScanSpec& VisionUnit::getTrayScanSpec() const
{
    return *m_trayScanSpec;
}

const InspectionAreaInfo& VisionUnit::getInspectionAreaInfo() const
{
    long inspectionID = getImageLot().GetInfo().m_scanAreaIndex;
    if (inspectionID < 0 || inspectionID >= m_trayScanSpec->GetInspectionItemCount())
    {
        static InspectionAreaInfo emtpy(nullptr, -1);
        return emtpy;
    }

    return m_trayScanSpec->GetInspectionItem(inspectionID);
}

const UnitAreaInfo& VisionUnit::getUnitAreaInfo() const
{
    return *m_unitAreaInfo;
}

const bool VisionUnit::IsSideVision() const
{
    return (SystemConfig::GetInstance().IsVisionTypeSide() == 1);
}

const long VisionUnit::GetSideVisionNumber() const
{
    if (SystemConfig::GetInstance().IsVisionTypeSide() != 1)
        return -1;

    static const long nSideVisionNumber = SystemConfig::GetInstance().GetSideVisionNumber();
    switch (nSideVisionNumber)
    {
        case enSideVisionNumber::SIDE_VISIONNUMBER_1:
        case enSideVisionNumber::SIDE_VISIONNUMBER_2:
            return nSideVisionNumber;
    }

    return -1;
}

void VisionUnit::GetPackageSize(const bool& i_bUnitIsPixel, float& o_fBodySizeX, float& o_fBodySizeY)
{
    static const bool bIsSideVision = IsSideVision();
    static const long nSideVisionNumber = GetSideVisionNumber();
    static const bool bIsNotSideVision = (bIsSideVision == false
        || nSideVisionNumber < enSideVisionNumber::SIDE_VISIONNUMBER_START
        || nSideVisionNumber >= enSideVisionNumber::
                SIDE_VISIONNUMBER_END); //Side Vision이 true이나 Side Vision Number가 유효하지 않을 수 있어 이를 보완하기 위함이다.
    const auto& scale = getScale();

    if (bIsNotSideVision == true) //Side Vision이 아니거나, Side Vision Number가 유효하지 않을 경우
    {
        o_fBodySizeX = m_pPackageSpec->m_bodyInfoMaster->GetBodySizeX();
        o_fBodySizeY = m_pPackageSpec->m_bodyInfoMaster->GetBodySizeY();
    }
    else //이하는 Side Vision일 경우
    {
        switch (nSideVisionNumber)
        {
            case enSideVisionNumber::SIDE_VISIONNUMBER_1:
                o_fBodySizeX = m_pPackageSpec->m_bodyInfoMaster->GetBodySizeX();
                break;
            case enSideVisionNumber::SIDE_VISIONNUMBER_2:
                o_fBodySizeX = m_pPackageSpec->m_bodyInfoMaster->GetBodySizeY();
                break;
        }
        o_fBodySizeY = m_pPackageSpec->m_bodyInfoMaster->fBodyThickness;
    }

    if (i_bUnitIsPixel == true)
    {
        o_fBodySizeX = scale.convert_umToPixelX(o_fBodySizeX);
        o_fBodySizeY = scale.convert_umToPixelY(o_fBodySizeY);
    }
}

const VisionImageLot& VisionUnit::getImageLot() const
{
    return *m_imageLot;
}

const VisionImageLotInsp& VisionUnit::getImageLotInsp() const
{
    return *m_pImageLotInsp;
}

VisionTrayScanSpec& VisionUnit::getTrayScanSpec()
{
    return *m_trayScanSpec;
}

VisionImageLot& VisionUnit::getImageLot()
{
    return *m_imageLot;
}

VisionImageLotInsp& VisionUnit::getImageLotInsp()
{
    return *m_pImageLotInsp;
}

VisionReusableMemory& VisionUnit::getReusableMemory()
{
    return *m_VisionReusableMemory;
}

const IllumInfo2D& VisionUnit::getIllumInfo2D() const
{
    return m_illum2D;
}

void VisionUnit::Set2DMatrixBypassMode(BOOL bUseBypassMode) //kircheis_NGRV Bypass
{
    m_visionInspection2DMatrix->m_bUseBypassMode = bUseBypassMode;
}

BOOL VisionUnit::Is2DMatrixReadingBypass() //kircheis_NGRV Bypass
{
    return m_visionInspection2DMatrix->Is2DMarixReading() && m_visionInspection2DMatrix->m_bUseBypassMode;
}

BOOL VisionUnit::GetNGRVAFInfo(BOOL& o_bIsValidPlaneRefInfo, std::vector<Ipvm::Point32s2>& o_vecptRefPos_UM,
    long& o_nAFImgSizeX, long& o_nAFImgSizeY, std::vector<BYTE>& o_vecbyAFImage) //kircheis_NGRVAF
{
    return m_visionProcessingAlign2D->GetNGRVAFInfo(
        o_bIsValidPlaneRefInfo, o_vecptRefPos_UM, o_nAFImgSizeX, o_nAFImgSizeY, o_vecbyAFImage);
}

BOOL VisionUnit::IsEnable2DMatrix() //kircheis_20230126
{
    BOOL bIsUse2DMatrix = m_visionInspection2DMatrix->IsEnabled() && m_visionInspection2DMatrix->IsTurnOn2DMatrix();

    return bIsUse2DMatrix;
}

BOOL VisionUnit::GetMajorRoiCenter(Ipvm::Point32s2& ptCenter)
{
    return m_visionInspection2DMatrix->GetMajorRoiCenter(ptCenter);
}

void VisionUnit::SetIntensityCheckerBypassMode(BOOL bUseBypassMode)
{
    m_visionInspectionIntensityChecker2D->m_bUseBypassMode = bUseBypassMode;
}

BOOL VisionUnit::IsIntensityCheckerBypass()
{
    return m_visionInspectionIntensityChecker2D->m_bUseBypassMode;
}

sUnitinspectionLogInfo VisionUnit::GetUnitinspLogInfo()
{
    return m_sUnitinspectionLogInfo;
}

std::vector<CString> VisionUnit::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    std::vector<CString> vecStrTotalText(0);

    static const CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    std::vector<CString> vecStrIllumFrameInfo(0);

    long nInspModuleNum = (long)m_vecVisionModules.size();
    for (long nModule = 0; nModule < nInspModuleNum; nModule++)
    {
        vecStrIllumFrameInfo = m_vecVisionModules[nModule]->ExportRecipeIllumFrameInfoToText(strVisionName);
        vecStrResult = m_vecVisionModules[nModule]->ExportRecipeToText();
        if (vecStrResult.size() > 0)
        {
            vecStrTotalText.insert(vecStrTotalText.end(), vecStrIllumFrameInfo.begin(), vecStrIllumFrameInfo.end());
            vecStrTotalText.insert(vecStrTotalText.end(), vecStrResult.begin(), vecStrResult.end());
        }
        vecStrIllumFrameInfo.clear();
        vecStrResult.clear();
    }

    return vecStrTotalText;
}

long VisionUnit::GetSideVisionSection()
{
    return m_nSideVisionSection;
}

void VisionUnit::SetSideVisionSection(long i_nSideVisionSection)
{
    m_nSideVisionSection = i_nSideVisionSection;
}

void VisionUnit::SetPackageInfo_NGRV(std::vector<NgrvPackageGrabInfo> packageInfo)
{
    m_vecPackageInfo_NGRV = packageInfo;
}

std::vector<NgrvPackageGrabInfo>& VisionUnit::GetPackageInfo_NGRV()
{
    return m_vecPackageInfo_NGRV;
}

void VisionUnit::SetAFInfo_NGRV(NgrvAfRefInfo afInfo)
{
    m_afInfo_NGRV = afInfo;
}

NgrvAfRefInfo& VisionUnit::GetAFInfo()
{
    return m_afInfo_NGRV;
}

void VisionUnit::SetGrabInfo_NGRV(std::vector<NgrvGrabInfo> grabInfo)
{
    m_vecGrabInfo_NGRV = grabInfo;
}

std::vector<NgrvGrabInfo>& VisionUnit::GetGrabInfo_NGRV()
{
    return m_vecGrabInfo_NGRV;
}

void VisionUnit::Set2DImageInfo_NGRV(long visionID, Ngrv2DImageInfo imageInfo)
{
    if (visionID == VISION_BTM_2D)
    {
        m_ngrv2DImageInfo_BTM = imageInfo;
    }
    else
    {
        m_ngrv2DImageInfo_TOP = imageInfo;
    }
}

Ngrv2DImageInfo& VisionUnit::Get2DImageInfo_NGRV(long visionID)
{
    if (visionID == VISION_BTM_2D)
    {
        return m_ngrv2DImageInfo_BTM;
    }
    else
    {
        return m_ngrv2DImageInfo_TOP;
    }
}

void VisionUnit::SetNGRVSingleRunInfo(long visionID, NgrvSingleRunInfo singleRunInfo)
{
    if (visionID == VISION_BTM_2D)
    {
        m_ngrvSingleRunInfo_BTM.Init();
        m_ngrvSingleRunInfo_BTM = singleRunInfo;
    }
    else
    {
        m_ngrvSingleRunInfo_TOP.Init();
        m_ngrvSingleRunInfo_TOP = singleRunInfo;
    }
}

NgrvSingleRunInfo& VisionUnit::GetNGRVSingleRunInfo(long visionID)
{
    if (visionID == VISION_BTM_2D)
    {
        return m_ngrvSingleRunInfo_BTM;
    }
    else
    {
        return m_ngrvSingleRunInfo_TOP;
    }
}

void VisionUnit::SetSocketInfo(Ipvm::SocketMessaging* pMessageSocket)
{
    m_pMessageSocket = pMessageSocket;
}

Ipvm::SocketMessaging& VisionUnit::GetSocketInfo()
{
    return *m_pMessageSocket;
}

int32_t VisionUnit::GetVisionMainType()
{
    return m_main.m_mainType;
}