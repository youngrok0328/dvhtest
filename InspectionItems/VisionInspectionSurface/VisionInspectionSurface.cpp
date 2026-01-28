//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionSurface.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionSurface.h"
#include "VisionInspectionSurfacePara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/Config.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
#include <iomanip>
#include <sstream>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionSurface::VisionInspectionSurface(
    LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(moduleGuid, moduleName, visionUnit, packageSpec)
    , m_surfacePara(new VisionInspectionSurfacePara(*this))
    , m_EdgeAlignResult(new VisionAlignResult)
    , m_blobAttributes(nullptr)
    , m_plLabel_(new Ipvm::Image32s)
    , m_plMaskLabel_(new Ipvm::Image32s)
{
    m_nBlobNum = 0;

    m_bEdgeAlignValid = FALSE;

    m_pVisionInspDlg = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{22211CE4-705F-4A69-BBB2-3698BE06FC00}"), g_szSurfaceInspectionName,
        _T("PVI"), _T("S"), _T("S"), HostReportCategory::SURFACE, II_RESULT_TYPE::SURFACE, TRUE, 0.f, 0.f, 0.f, 0.f,
        TRUE, TRUE, FALSE);

    __super::ResetSpec();

    m_DebugInfoGroup.Add(_T("Find Area"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Defect Area"), enumDebugInfoType::Rect);
}

VisionInspectionSurface::~VisionInspectionSurface(void)
{
    CloseDlg();

    delete m_EdgeAlignResult;
    delete m_surfacePara;

    delete m_plMaskLabel_;
    delete m_plLabel_;
}

void VisionInspectionSurface::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionSurface::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionSurface(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionSurface::IDD, parent);

    m_pVisionInspDlg->ShowImage();

    // 한번 실행시켜야 출력물들이 보여짐.
    if (m_visionUnit.RunInspectionInTeachMode(this, false, GetCurVisionModule_Status()))
    {
        //m_pVisionInspDlg->SetInitParaWindow();
        m_pVisionInspDlg->ShowImage();
    }

    ResetResult();
    InitEdgeResult();
    m_visionUnit.GetInspectionOverlayResult().Apply(m_pVisionInspDlg->m_imageLotView->GetCoreView());

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

void VisionInspectionSurface::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (m_result.nResult == NOT_MEASURED)
        return;

    if (detailSetupMode)
        overlayResult->OverlayReset();

    long nImageSizeX = getReusableMemory().GetInspImageSizeX();
    long nImageSizeY = getReusableMemory().GetInspImageSizeY();

    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return;

    if (m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
    {
        if (!detailSetupMode)
            return;
        VisionAlignResult sBodyAlignResult;

        long nDataNum = 0;
        void* pData = m_visionUnit.GetVisionDebugInfo(
            m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
        if (pData != nullptr && nDataNum > 0)
            sBodyAlignResult = *(VisionAlignResult*)pData;

        Ipvm::Image8u thresholdImage;
        getReusableMemory().GetInspByteImage(thresholdImage);

        auto* surfaceMask = getReusableMemory().GetSurfaceLayerMask(m_strModuleName);

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(thresholdImage), 1, thresholdImage);

        if (surfaceMask != nullptr) //kircheis_Shiny //Anti Crash
        {
            for (long y = 0; y < nImageSizeY; y++)
            {
                auto* mask_y = surfaceMask->GetMem(0, y);
                auto* threshold_y = thresholdImage.GetMem(0, y);
                auto* image_y = image.GetMem(0, y);

                for (long x = 0; x < nImageSizeX; x++)
                {
                    if (mask_y[x])
                    {
                        threshold_y[x] = 255;
                    }
                    else
                    {
                        threshold_y[x] = 252 * image_y[x] / 255 + 1;
                    }
                }
            }
        }

        overlayResult->SetThresholdImage(thresholdImage);
    }
    else
    {
        COLORREF dwPass = RGB(0, 255, 128);
        COLORREF dwMarginal = RGB(243, 157, 58);
        COLORREF dwFail = RGB(255, 0, 0);

        for (long index = 0; index < long(m_result.vecnResult.size()); index++)
        {
            auto& result = m_result.vecnResult[index];
            auto& object = m_result.vecDSOValue[index];

            COLORREF color = RGB(0, 0, 0);

            if (result == PASS)
            {
                if (m_nOverlayMode == OverlayType_ShowAll)
                {
                    color = dwPass;
                }
            }
            else if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                if (result == MARGINAL)
                {
                    color = dwMarginal;
                }
                else
                {
                    color = dwFail;
                }
            }

            if (color != RGB(0, 0, 0))
            {
                overlayResult->AddRectangle(object.arROI, color);

                if (detailSetupMode)
                {
                    CString textNo;
                    textNo.Format(_T("%d"), index + 1);
                    overlayResult->AddText(object.arROI.GetLTPoint(), textNo, color);
                    textNo.Empty();
                }
            }
        }
    }
}
void VisionInspectionSurface::AppendTextResult(CString& textResult)
{
    CSurfaceItem& SurfaceItem = m_surfacePara->m_SurfaceItem;

    if (m_result.nResult == NOT_MEASURED)
        return;

    textResult.AppendFormat(_T("\r\n< %s Result >"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("\r\n  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);
    textResult.AppendFormat(
        _T("      %s = %s\r\n"), (LPCTSTR)m_strModuleName, (LPCTSTR)Result2String(m_result.nResult));

    if (m_result.nResult == INVALID)
        return;

    std::wstringstream stream;
    stream << std::setiosflags(std::ios::fixed) << std::setprecision(2) << std::setiosflags(std::ios::right);

    LPCTSTR szResult[] = {_T(" "), _T("Pass"), _T("Marginal"), _T("Reject"), _T("Invalid")};
    LPCTSTR szResultSymbol[] = {_T(" "), _T(" "), _T("x"), _T("X"), _T("-")};
    LPCTSTR szColor[] = {_T("Dark"), _T("Bright")};

    std::vector<SDetectedSurfaceObject>& vecObject = m_result.vecDSOValue;
    std::vector<SDetectedSurfaceObject>::iterator itrObject = vecObject.begin();
    std::vector<SDetectedSurfaceObject>::iterator itrObjectBegin = vecObject.begin();
    std::vector<SDetectedSurfaceObject>::iterator itrObjectEnd = vecObject.end();

    std::vector<long>& vecnObjectResult = m_result.vecnResult;
    std::vector<long>::iterator itrObjectResult = vecnObjectResult.begin();
    std::vector<long>::iterator itrObjectResultEnd = vecnObjectResult.end();

    std::vector<CSurfaceCriteria>& vecCriteria = SurfaceItem.vecCriteria;
    std::vector<CSurfaceCriteria>::iterator itrCriteria = vecCriteria.begin();
    std::vector<CSurfaceCriteria>::iterator itrCriteriaBegin = vecCriteria.begin();
    std::vector<CSurfaceCriteria>::iterator itrCriteriaEnd = vecCriteria.end();

    std::vector<CSurfaceCriteriaResult>& vecCriteriaResult = m_result.vecCriteriaResult;
    std::vector<CSurfaceCriteriaResult>::iterator itrCriteriaResult = vecCriteriaResult.begin();
    std::vector<CSurfaceCriteriaResult>::iterator itrCriteriaResultBegin = vecCriteriaResult.begin();
    std::vector<CSurfaceCriteriaResult>::iterator itrCriteriaResultEnd = vecCriteriaResult.end();

    while (itrCriteria != itrCriteriaEnd && itrCriteriaResult != itrCriteriaResultEnd)
    {
        stream << LPCTSTR(itrCriteria->strName) << " : " << LPCTSTR(szResult[itrCriteriaResult->nResult]) << "\r\n";

        itrCriteria++;
        itrCriteriaResult++;
    }

    itrCriteria = itrCriteriaBegin;
    itrCriteriaResult = itrCriteriaResultBegin;

    stream << std::setw(2) << "T|";
    while (itrCriteria != itrCriteriaEnd)
    {
        CString strCriteria;
        strCriteria.Format(_T("C%d|"), long(itrCriteria - itrCriteriaBegin));
        stream << std::setw(3) << LPCTSTR(strCriteria);
        itrCriteria++;
        strCriteria.Empty();
    }
    stream << std::setw(10) << "Color|";
    stream << std::setw(10) << "Pos X|";
    stream << std::setw(10) << "Pos Y|";

    bool is3D = (Config::getVisionType() == VISIONTYPE_3D_INSP
        && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP);
    for (long n = 0; n < GetCriteriaColumnCount(is3D); n++)
    {
        auto columnID = GetCriteriaColumnID(is3D, n);
        CStringA name(GetCriteriaColumnName(columnID));
        stream << std::setw(12) << (LPCSTR)name << "|";
        name.Empty();
    }
    stream << "\r\n";

    stream << "---------------------------------------------------------------------"
           << "\r\n";

    while (itrObject != itrObjectEnd)
    {
        stream << std::setw(1) << szResultSymbol[*itrObjectResult];

        itrCriteriaResult = itrCriteriaResultBegin;

        while (itrCriteriaResult != itrCriteriaResultEnd)
        {
            stream << std::setw(3) << szResultSymbol[itrCriteriaResult->vecnResult[itrObject - itrObjectBegin]];
            itrCriteriaResult++;
        }

        stream << std::setw(18) << szColor[itrObject->m_color];
        stream << std::setw(10) << itrObject->fCenX0;
        stream << std::setw(10) << itrObject->fCenY0;
        for (long n = 0; n < GetCriteriaColumnCount(is3D); n++)
        {
            auto columnID = GetCriteriaColumnID(is3D, n);
            stream << std::setw(12) << itrObject->m_inspValue[long(columnID)];
        }

        stream << "\r\n";

        itrObject++;
        itrObjectResult++;
    }

    stream << "\r\n" << std::ends;

    textResult += CString(stream.str().c_str());
}

BOOL VisionInspectionSurface::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_surfacePara->LinkDataBase(bSave, db[_T("{0BE9E57A-1FFA-4795-B5CC-AFC7E9E3A4F9}")], m_strModuleName))
        return FALSE;

    //{{//window size X/Y가 Package Size보다 크게 Setting 되어 있으면 오차 감안해서 2 Pixel에 해당하는 값을 뺀 결과를 해당 변수에 적용한다.
    // getScale()은 Image를 불러오기 전에는 System Scale을 가지고 있고, Image를 불러온 후에는 Image Scale을 가지고 있다.
    // 정확한 계산을 위해서는 Image의 Scale을 사용해야 하지만, 예외처리가 어려워 2Pixel 정도는 무시했다.

    if (!bSave)
    {
        const auto& px2um = getScale().pixelToUm();
        float bodySizeX_um = 0.f;
        float bodySizeY_um = 0.f;
        if (GetInspSize(false, bodySizeX_um, bodySizeY_um))
        {
            const float fWindowSizeX = CAST_FLOAT(m_surfacePara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_x);
            const float fWindowSizeY = CAST_FLOAT(m_surfacePara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_y);

            m_surfacePara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_x
                = (float)min(fWindowSizeX, bodySizeX_um - (px2um.m_x * 2.f));
            m_surfacePara->m_SurfaceItem.AlgoPara.m_dbackground_window_size_y
                = (float)min(fWindowSizeY, bodySizeY_um - (px2um.m_y * 2.f));
        }
    }
    //}}

    // 영훈 20151030 : Surface에서 Criteria가 모두 꺼져있는 항목은 검사를 하지 않도록 한다.
    BOOL bInsp = FALSE;
    long nCriteriaSize = (long)m_surfacePara->m_SurfaceItem.vecCriteria.size();
    for (long nID = 0; nID < nCriteriaSize; nID++)
    {
        bInsp |= m_surfacePara->m_SurfaceItem.vecCriteria[nID].bInsp;
    }

    // Surface는 Spec이 1개이므로 하드코딩 하도록 한다.
    if (bInsp == FALSE)
    {
        m_fixedInspectionSpecs[0].m_use = bInsp;
    }

    return TRUE;
}

void VisionInspectionSurface::ResetSpecAndPara()
{
    __super::ResetSpec();
}

void VisionInspectionSurface::ResetResult()
{
    __super::ResetResult();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();
    m_pBlob = getReusableMemory().GetBlob();
}

std::vector<CString> VisionInspectionSurface::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_surfacePara->ExportAlgoParaToText(strVisionName, m_strModuleName);
    std::vector<CString> vecstrInspectionSpec = ExportInspectionSpecToText(strVisionName);

    vecStrResult.insert(vecStrResult.end(), vecstrInspectionSpec.begin(), vecstrInspectionSpec.end());

    return vecStrResult;
}