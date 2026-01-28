//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionSurface.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionSurface.h"
#include "SurfaceAlgoPara.h"
#include "SurfacePrepairResult.h"
#include "VisionInspectionSurfacePara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../DefineModules/dA_Base/BlobAttribute.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTapeSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSurfaceResult.h" //kircheis_Criteria
#include "../../SharedCommunicationModules/VisionHostCommon/VisionSurfaceCriteriaDefinitions.h" //kircheis_Criteria
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image64r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/Size32s2.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
#include <cmath>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionSurface::OnInspection()
{
    if (m_pVisionInspDlg == NULL)
    {
        ::SimpleMessage(_T("This job have some problem.\nPlease check the surface inspection item name."));
        return FALSE;
    }
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionSurface::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    return DoInspection(detailSetupMode, enumRunMode::Normal, nullptr);
}

BOOL VisionInspectionSurface::DoTeach()
{
    return DoInspection(true, enumRunMode::TeachMask, nullptr);
}

BOOL VisionInspectionSurface::DoInspection(const bool detailSetupMode, enumRunMode mode, Ipvm::Image32s* o_imageLabel)
{
    Ipvm::TimeCheck TimeInspectionTotal;

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;

    m_bUse2ndInspection = m_surfacePara->m_SurfaceItem.AlgoPara.m_use2ndInspection;
    m_str2ndInspCode = m_surfacePara->m_SurfaceItem.AlgoPara.m_str2ndInspCode;
    m_bUseOnlyDLInsp = m_surfacePara->m_SurfaceItem.AlgoPara.m_useOnlyDLInsp;
    //}}

    if (m_fixedInspectionSpecs[0].m_use == FALSE)
    {
        SetPass_TotalResult();
        return TRUE;
    }

    BOOL bInsp = FALSE;
    BOOL bTeachMask = FALSE;

    switch (mode)
    {
        case enumRunMode::Normal:
            if (m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode == FALSE)
            {
                // Surface 검사에서는 무조건 검사
                bInsp = TRUE;
            }
            else
            {
                // Surface Mask 검사에서는 Teaching 해야 하는 시점에만 검사
                if (m_surfacePara->m_SurfaceItem.m_bSurfaceMask_GenerateAlways)
                {
                    bInsp = TRUE;
                    bTeachMask = TRUE;
                }
            }
            break;

        case enumRunMode::TeachMask:
            bInsp = TRUE;
            if (m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
            {
                bTeachMask = TRUE;
            }
            break;

        case enumRunMode::CriteriaTest:
            bInsp = TRUE;
            bTeachMask = FALSE;
            break;
    }

    if (bInsp)
    {
        ResetResult();

        static const BOOL bHWExist = SystemConfig::GetInstance().m_bHardwareExist;

        // ROI Mask 생성
        Surface_CreateMaskGroup(TRUE);

        MakeSurfaceROI(m_surfacePara->m_SurfaceItem);

        const CSurfaceAlgoPara& AlgoPara = m_surfacePara->m_SurfaceItem.AlgoPara;

        bool success = DoInsp(detailSetupMode, getReusableMemory().GetBlobInfo(0), getReusableMemory().GetBlobInfo(1),
            CAST_SHORT(AlgoPara.nMaxDarkBlobNum_New), CAST_SHORT(AlgoPara.nMaxBrightBlobNum_New),
            getReusableMemory().GetBlobAttribute(0), bTeachMask, o_imageLabel);

        if (!success)
        {
            m_result.nResult = INVALID;
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szSurfaceInspectionName);
            if (pResult != NULL)
                pResult->m_totalResult = INVALID; //kircheis_SurfBug

            return FALSE;
        }

        //////////////////////////////////////////////////////////////////////////////////////
        //{{ [Result] 의 [Small] 를 설정
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szSurfaceInspectionName);
        if (pResult == nullptr)
        {
            return FALSE;
        }
        VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == nullptr)
        {
            return FALSE;
        }

        pResult->vecnDefectAreaID.clear();

        Ipvm::Rect32s rtBodyROIbyImageCenter
            = getScale().convert_BCUToPixel(GetSurfaceAlgoPara()->m_rtROI_BCU, imageCenter);

        auto rejectReportPatchCount_X = SystemConfig::GetInstance().m_nSurfaceRejectReportPatchCount_X;
        auto rejectReportPatchCount_Y = SystemConfig::GetInstance().m_nSurfaceRejectReportPatchCount_Y;

        Ipvm::Point32s2 realBodyCenterOffset;
        realBodyCenterOffset.m_x
            = (long)(m_EdgeAlignResult->m_center.m_x + 0.5f) - rtBodyROIbyImageCenter.CenterPoint().m_x;
        realBodyCenterOffset.m_y
            = (long)(m_EdgeAlignResult->m_center.m_y + 0.5f) - rtBodyROIbyImageCenter.CenterPoint().m_y;

        pResult->nDefectAreaSize = rejectReportPatchCount_X * rejectReportPatchCount_Y;
        long nWidth = rtBodyROIbyImageCenter.Width() / rejectReportPatchCount_X;
        long nHeight = rtBodyROIbyImageCenter.Height() / rejectReportPatchCount_Y;
        std::vector<Ipvm::Rect32s> vecrtBodyROI(pResult->nDefectAreaSize, Ipvm::Rect32s(0, 0, 0, 0));
        std::vector<Ipvm::Rect32s> vecrtDefectROI;
        vecrtDefectROI.reserve(pResult->nDefectAreaSize);

        for (long nY = 0; nY < rejectReportPatchCount_Y; nY++)
        {
            long nIndex = nY * rejectReportPatchCount_X;
            for (long nX = 0; nX < rejectReportPatchCount_X; nX++)
            {
                long nTotalIndex = nIndex + nX;
                vecrtBodyROI[nTotalIndex].m_left = rtBodyROIbyImageCenter.m_left + (nWidth * nX);
                vecrtBodyROI[nTotalIndex].m_right = vecrtBodyROI[nTotalIndex].m_left + nWidth;
                vecrtBodyROI[nTotalIndex].m_top = rtBodyROIbyImageCenter.m_top + (nHeight * nY);
                vecrtBodyROI[nTotalIndex].m_bottom = vecrtBodyROI[nTotalIndex].m_top + nHeight;
            }
        }

        auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Find Area"));
        if (psDebugInfo != NULL)
        {
            if (psDebugInfo->pData != NULL)
            {
                psDebugInfo->Reset();
            }
            psDebugInfo->nDataNum = pResult->nDefectAreaSize;
            Ipvm::Rect32s* pData = new Ipvm::Rect32s[psDebugInfo->nDataNum];
            for (long i = 0; i < psDebugInfo->nDataNum; i++)
            {
                pData[i] = vecrtBodyROI[i];
                pData[i].OffsetRect(realBodyCenterOffset.m_x, realBodyCenterOffset.m_y);
            }
            psDebugInfo->pData = pData;
        }

        std::vector<long>& vecnResult = m_result.vecnResult;
        std::vector<SDetectedSurfaceObject>& vecDSOValue = m_result.vecDSOValue;

        Ipvm::Rect32s rtTempROI;
        long nSurfaceBlobSize = (long)vecDSOValue.size();
        long nMinBlobSize;

        pResult->Resize(nSurfaceBlobSize);

        CString sObjectID = NULL;

        for (long nBlobID = 0; nBlobID < nSurfaceBlobSize; nBlobID++)
        {
            // 영훈 [Surface ROI] 20130815 : Surface Detail Result에서 좌표 표기가 이상한 것을 상대 좌표계로 바꿔주자.
            Ipvm::Rect32s rtROI;
            rtROI.m_left = (long)((vecDSOValue[nBlobID].arROI.ltX + vecDSOValue[nBlobID].arROI.blX) * 0.5f);
            rtROI.m_top = (long)((vecDSOValue[nBlobID].arROI.ltY + vecDSOValue[nBlobID].arROI.rtY) * 0.5f);
            rtROI.m_right = (long)((vecDSOValue[nBlobID].arROI.rtX + vecDSOValue[nBlobID].arROI.brX) * 0.5f);
            rtROI.m_bottom = (long)((vecDSOValue[nBlobID].arROI.blY + vecDSOValue[nBlobID].arROI.brY) * 0.5f);
            sObjectID.Format(_T("%d"), nBlobID);

            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                nBlobID, sObjectID, _T(""), _T(""), 0.f, *pSpec, 0.f, 0.f);
            pResult->SetRect(nBlobID, rtROI);
            pResult->SetResult(nBlobID, vecnResult[nBlobID]);

            auto& blob_roi = vecDSOValue[nBlobID].rtROI;
            auto& blob_label = vecDSOValue[nBlobID].nLabel;

            if (vecnResult[nBlobID] != PASS)
            {
                std::vector<BOOL> vecbUseDefectArea(pResult->nDefectAreaSize);

                // 영훈 [Surface_Reject_ROI] 20130819 : Surface에서도 Reject ROI를 남기도록 하자.
                pResult->SetRejectROI(rtROI);

                // 영훈 20150908 : Surface에서 불량이 난 부분의 영역을 찾아 host로 보내주도록 한다.
                nMinBlobSize = (long)((float)vecDSOValue[nBlobID].nArea * 0.05f
                    + 0.5f); // 영훈 ; 5%는 대만에서 정해준 Spec.. Parameter로 뺄 수도 있다.

                pResult->vecnDefectAreaID.reserve(pResult->nDefectAreaSize);
                for (long nROIID = 0; nROIID < pResult->nDefectAreaSize; nROIID++)
                {
                    long nPixelCount = 0;
                    // 현재 영역에 불량이 발견된 적이 없고, 영역에 불량이 발생했다면 진입
                    if (vecbUseDefectArea[nROIID] == FALSE && rtTempROI.IntersectRect(vecrtBodyROI[nROIID], blob_roi))
                    {
                        // 그 영역에서 불량의 Area를 카운팅한다.
                        for (long nY = blob_roi.m_top; nY < blob_roi.m_bottom; nY++)
                        {
                            auto* label_y = m_plLabel_->GetMem(0, nY);
                            for (long nX = blob_roi.m_left; nX < blob_roi.m_right; nX++)
                            {
                                if (label_y[nX] == blob_label)
                                {
                                    ++nPixelCount;
                                }
                            }
                        }

                        // 카운팅 된 불량의 Pixel 개수가 5% 이상 넘어가면 현재 영역은 불량이 포함된 영역이라고 표시한다.
                        if (nPixelCount > nMinBlobSize)
                        {
                            vecbUseDefectArea[nROIID] = TRUE;
                            pResult->vecnDefectAreaID.push_back(nROIID);
                            vecrtDefectROI.push_back(vecrtBodyROI[nROIID]);
                        }
                    }
                }
            }
        }

        psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Defect Area"));
        if (psDebugInfo != NULL)
        {
            if (psDebugInfo->pData != NULL)
            {
                psDebugInfo->Reset();
            }
            psDebugInfo->nDataNum = (long)vecrtDefectROI.size();
            Ipvm::Rect32s* pData = new Ipvm::Rect32s[psDebugInfo->nDataNum];
            for (long i = 0; i < psDebugInfo->nDataNum; i++)
            {
                pData[i] = vecrtDefectROI[i];
                pData[i].OffsetRect(realBodyCenterOffset.m_x, realBodyCenterOffset.m_y);
            }
            psDebugInfo->pData = pData;
        }

        pResult->m_totalResult = (long)max(pResult->m_totalResult, m_result.nResult);

        //Surface Criteria Data Set
        SetSurfaceCriteriaData(realBodyCenterOffset, pResult, m_surfacePara->m_SurfaceItem, m_result);
    }

    if (m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
    {
        if (bInsp == FALSE)
        {
            // Surface Mask 모드인데 검사를 안해도 되면 Mask만 리턴하자
            auto& imageMask = m_surfacePara->m_SurfaceBitmapMask;
            auto* maskLayer = getReusableMemory().AddSurfaceLayerMaskClass(m_strModuleName);
            if (maskLayer)
            {
                maskLayer->CopyFrom(imageMask);
            }
        }

        SetPass_TotalResult();

        return TRUE;
    }

    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("InspTotalTime"), CAST_FLOAT(TimeInspectionTotal.Elapsed_ms()));

    return TRUE;
}

float VisionInspectionSurface::GetConvertUnitFactor(long nIdx)
{
    float fFactor = 1.f;

    float fFactorSquare = 25.4f * 25.4f;
    switch (VisionSurfaceCriteria_Column(nIdx))
    {
        case VisionSurfaceCriteria_Column::Area:
            fFactor = fFactorSquare;
            //fFactor = 25.4f;// fFactorSquare;
            break;

        case VisionSurfaceCriteria_Column::Width:
        case VisionSurfaceCriteria_Column::Length:
        case VisionSurfaceCriteria_Column::LengthX:
        case VisionSurfaceCriteria_Column::LengthY:
        case VisionSurfaceCriteria_Column::TotalLength:
        case VisionSurfaceCriteria_Column::Thickness:
        case VisionSurfaceCriteria_Column::Locus:
        case VisionSurfaceCriteria_Column::t3D_AvgDeltaHeight:
        case VisionSurfaceCriteria_Column::t3D_KeyDeltaHeight:
        case VisionSurfaceCriteria_Column::Dist_X_to_BodyCenter:
        case VisionSurfaceCriteria_Column::Dist_Y_to_BodyCenter:
        case VisionSurfaceCriteria_Column::Dist_to_Body:
            fFactor = 25.4f;
            break;
    }

    return fFactor;
}

void VisionInspectionSurface::SetSurfaceCriteriaData(const Ipvm::Point32s2& realBodyCenterOffset,
    VisionInspectionResult* i_pInspectionResult, const CSurfaceItem& i_cSurfaceItem,
    const CSurfaceItemResult& i_cSurfaceItemResult)
{
    long nCriteriaNum = (long)(i_cSurfaceItemResult.vecCriteriaResult.size());
    if (nCriteriaNum < 0) //할필요없다
        return;

    static const float fFactor = 25.4f; //단위변환

    long nDefectSize = (long)i_cSurfaceItemResult.vecDSOValue.size();

    static const LPCTSTR szColor[] = {_T("Dark"), _T("Bright")};
    static const LPCTSTR szResult[] = {_T("Not_Measured"), _T("Pass"), _T("Marginal"), _T("Reject"), _T("Invalid")};
    static const BOOL bIs3DVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP);
    const BOOL bUseVmap = (bIs3DVision && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP);

    VisionInspectionSurfaceResult cVisionSurfaceCriteriaResult;

    CString strCriteriaName;
    long nCriteriaSize = (long)i_cSurfaceItemResult.vecCriteriaResult.size();
    for (long nCriteriaIdx = 0; nCriteriaIdx < nCriteriaSize; nCriteriaIdx++)
    {
        auto& specItem = i_cSurfaceItem.vecCriteria[nCriteriaIdx];
        if (!specItem.bInsp)
            continue;

        CString strCriteriaNameBuf = specItem.strName;
        strCriteriaNameBuf.MakeUpper();
        if (strCriteriaNameBuf == _T("UNKNOWN") || strCriteriaNameBuf.IsEmpty())
        {
            strCriteriaNameBuf.Format(_T("C%d"), nCriteriaIdx);
        }
        else
        {
            strCriteriaNameBuf = i_cSurfaceItem.vecCriteria[nCriteriaIdx].strName;
        }

        if (!strCriteriaName.IsEmpty())
        {
            strCriteriaName += _T(",");
        }

        strCriteriaName += strCriteriaNameBuf;
        nDefectSize = (long)min(nDefectSize, i_cSurfaceItemResult.vecCriteriaResult[nCriteriaIdx].vecnResult.size());
    }

    float fImageCenX = getImageLotInsp().GetImageSizeX() * .5f;
    float fImageCenY = getImageLotInsp().GetImageSizeY() * .5f;
    float fPx2Um = getScale().pixelToUmXY();

    //float fCurAngleValue(0.f);

    //switch (m_packageSpec.m_OriginPadData->m_nRotateIdx)
    //{
    //case Rotate_0:
    //	fCurAngleValue = 0.f;
    //	break;
    //case Rotate_90:
    //	fCurAngleValue = -3.141592f * .5f;
    //	break;
    //case Rotate_180:
    //	fCurAngleValue = -3.141592f;
    //	break;
    //case Rotate_270:
    //	fCurAngleValue = -3.141592f * 1.5f;
    //	break;
    //}

    //float fReversValue(-1.f); //mc_MED#3 TDX Report 표기시, 실좌표계 기준으로 표기요청

    for (long nDetectIndex = 0; nDetectIndex < nDefectSize; nDetectIndex++)
    {
        cVisionSurfaceCriteriaResult.ResizeCriteria(bUseVmap);
        cVisionSurfaceCriteriaResult.m_strCriteriaName = strCriteriaName;
        cVisionSurfaceCriteriaResult.m_strCriteriaColor
            = szColor[i_cSurfaceItemResult.vecDSOValue[nDetectIndex].m_color];

        CString strCriteriaResult;
        for (long nCriteriaIdx = 0; nCriteriaIdx < nCriteriaSize; nCriteriaIdx++)
        {
            auto& specItem = i_cSurfaceItem.vecCriteria[nCriteriaIdx];
            if (!specItem.bInsp)
                continue;

            if (!strCriteriaResult.IsEmpty())
            {
                strCriteriaResult += _T(",");
            }

            strCriteriaResult
                += szResult[i_cSurfaceItemResult.vecCriteriaResult[nCriteriaIdx].vecnResult[nDetectIndex]];
        }

        cVisionSurfaceCriteriaResult.m_strCriteriaResult = strCriteriaResult;

        long nItemNum = (long)cVisionSurfaceCriteriaResult.m_vecstrCriteriaValue.size();

        //측정값 Set
        for (long nItemIndex = 0; nItemIndex < nItemNum; nItemIndex++)
        {
            CString strCriteriaValue("");
            //검사 Spec 값을 Min 또는 Max를 입력했을 시에만 보낸다
            if (GetSurfaceItem()->IsNeed_Spec(VisionSurfaceCriteria_Column(nItemIndex)))
            {
                float fCriteriaValue = i_cSurfaceItemResult.vecDSOValue[nDetectIndex].m_inspValue[nItemIndex]
                    * GetConvertUnitFactor(nItemIndex);
                if (std::isnan(fCriteriaValue) == false && std::isinf(fCriteriaValue) == false)
                    strCriteriaValue.Format(_T("%.2f"), fCriteriaValue);
                else
                    strCriteriaValue.Empty();

                cVisionSurfaceCriteriaResult.m_vecstrCriteriaValue[nItemIndex] = strCriteriaValue;
            }
        }

        cVisionSurfaceCriteriaResult.m_rtCriteriaROI = i_cSurfaceItemResult.vecDSOValue[nDetectIndex].rtROI;
        cVisionSurfaceCriteriaResult.m_rtCriteriaROI.OffsetRect(realBodyCenterOffset.m_x, realBodyCenterOffset.m_y);

        cVisionSurfaceCriteriaResult.m_fCriteriaROICenX
            = ((float)cVisionSurfaceCriteriaResult.m_rtCriteriaROI.CenterPoint().m_x - fImageCenX);
        cVisionSurfaceCriteriaResult.m_fCriteriaROICenY
            = (fImageCenY - (float)cVisionSurfaceCriteriaResult.m_rtCriteriaROI.CenterPoint().m_y);

        Ipvm::Point32r2 ptPoint(
            cVisionSurfaceCriteriaResult.m_fCriteriaROICenX, cVisionSurfaceCriteriaResult.m_fCriteriaROICenY);
        //CPI_Geometry::RotatePoint(ptPoint, fCurAngleValue, Ipvm::Point32r2(0.f, 0.f));

        cVisionSurfaceCriteriaResult.m_fCriteriaROICenX = (ptPoint.m_x) * fPx2Um;
        //cVisionSurfaceCriteriaResult.m_fCriteriaROICenY = ((ptPoint.m_y) * fPx2Um) * fReversValue;
        cVisionSurfaceCriteriaResult.m_fCriteriaROICenY = (ptPoint.m_y) * fPx2Um;

        //PushData
        i_pInspectionResult->AddSurfaceCriteriaResult(cVisionSurfaceCriteriaResult);
    }
}

CSurfaceItem* VisionInspectionSurface::GetSurfaceItem()
{
    return &m_surfacePara->m_SurfaceItem;
}

CSurfaceAlgoPara* VisionInspectionSurface::GetSurfaceAlgoPara()
{
    return &m_surfacePara->m_SurfaceItem.AlgoPara;
}

CSurfaceCustomROI* VisionInspectionSurface::GetSurfaceCustomROI()
{
    return &m_surfacePara->m_SurfaceCutemRoi;
}

const CSurfaceItemResult& VisionInspectionSurface::GetResult() const
{
    return m_result;
}

void VisionInspectionSurface::SetSurfaceItem(const CSurfaceItem& value)
{
    m_surfacePara->m_SurfaceItem = value;
    MakeSurfaceROI(m_surfacePara->m_SurfaceItem);
    m_result.InitnResults(m_surfacePara->m_SurfaceItem);
}

void VisionInspectionSurface::SetSurfaceAlgoPara(const CSurfaceAlgoPara& value)
{
    m_surfacePara->m_SurfaceItem.AlgoPara = value;
    MakeSurfaceROI(m_surfacePara->m_SurfaceItem);
    m_result.InitnResults(m_surfacePara->m_SurfaceItem);
}

bool VisionInspectionSurface::GetInspSize(const bool& i_bUnitIsPixel, float& o_fBodySizeX, float& o_fBodySizeY)
{
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_TR)
    {
        if (const auto* tapeSpec = getTapeSpec())
        {
            o_fBodySizeX = (tapeSpec->m_w0) * 1000.f; // Tape Width
            o_fBodySizeY = (tapeSpec->m_p1 * 2.f - tapeSpec->m_a0) * 1000.f; // Pocket Pitch * 2 - Pocket Size Y
            if (i_bUnitIsPixel)
            {
                o_fBodySizeX = getScale().convert_umToPixelX(o_fBodySizeX);
                o_fBodySizeY = getScale().convert_umToPixelY(o_fBodySizeY);
            }
            return true;
        }
        else
        {
            ASSERT(!L"여기로 들어오면 안되겠지만 혹시 모르니 일단 예외 처리");
            o_fBodySizeX = 0.f;
            o_fBodySizeY = 0.f;
            return false;
        }
    }

    GetPackageSize(i_bUnitIsPixel, o_fBodySizeX, o_fBodySizeY);
    return true;
}

bool VisionInspectionSurface::DoInsp(const bool detailSetupMode, Ipvm::BlobInfo* psDarkObjInfo,
    Ipvm::BlobInfo* psBrightObjInfo, short nDarkBlobMaxNum, short nBrightBlobMaxNum, BlobAttribute* blobAttributes,
    BOOL i_bTeachMask, Ipvm::Image32s* o_imageLabel)
{
    m_blobAttributes = blobAttributes;

    Ipvm::TimeCheck TotalTime;
    Ipvm::Image8u thresholdImage;

    InitEdgeResult();
    ResetResult();

    Ipvm::TimeCheck TimeInsp_Core;
    bool retValue = DoInsp_Core(detailSetupMode, psDarkObjInfo, psBrightObjInfo, nDarkBlobMaxNum, nBrightBlobMaxNum,
        thresholdImage, o_imageLabel);

    AddProcessingFunctionExcuteTimeLog(m_strModuleName, _T("InspCoreTime"), CAST_FLOAT(TimeInsp_Core.Elapsed_ms()));

    if (retValue && i_bTeachMask)
    {
        Ipvm::TimeCheck TimeMakeSurfaceMask;
        // Surface Mask 생성
        if (MakeObjectIgnorePattern(m_surfacePara->m_SurfaceItem, thresholdImage, *m_plLabel_))
        {
            retValue = false;
        }

        AddProcessingFunctionExcuteTimeLog(
            m_strModuleName, _T("SurfakceMaskCreateTime"), CAST_FLOAT(TimeMakeSurfaceMask.Elapsed_ms()));
    }

    AddProcessingFunctionExcuteTimeLog(m_strModuleName, _T("InspTime"), CAST_FLOAT(TotalTime.Elapsed_ms()));

    m_fCalcTime = CAST_FLOAT(TotalTime.Elapsed_ms());

    return retValue;
}

bool VisionInspectionSurface::DoInsp_Core(const bool detailSetupMode, Ipvm::BlobInfo* psDarkObjInfo,
    Ipvm::BlobInfo* psBrightObjInfo, short nDarkBlobMaxNum, short nBrightBlobMaxNum, Ipvm::Image8u& o_thresholdImage,
    Ipvm::Image32s* o_imageLabel)
{
    Ipvm::Image32s imageLabel_Dark;
    Ipvm::Image32s imageLabel_Bright;

    if (!getReusableMemory().GetInspLongImage(imageLabel_Dark))
        return false;
    if (!getReusableMemory().GetInspLongImage(imageLabel_Bright))
        return false;

    const auto& SurfaceItemSpec = m_surfacePara->m_SurfaceItem;

    SurfacePrepairResult prepairResult(detailSetupMode);

    Ipvm::TimeCheck TimeMakePrepairImage;
    if (!MakePrepairImage(m_surfacePara->m_SurfaceItem, prepairResult))
    {
        return false;
    }
    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("MakePrepairImageTime"), CAST_FLOAT(TimeMakePrepairImage.Elapsed_ms()));

    Ipvm::Image8u thresholdImage;

    Ipvm::TimeCheck TimeMakeInspImage;
    if (!MakeInspImage(m_surfacePara->m_SurfaceItem, prepairResult, thresholdImage))
    {
        return false;
    }

    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("MakeInspImageTime"), CAST_FLOAT(TimeMakeInspImage.Elapsed_ms()));

    // Bright 및 Dark Blob 추출
    Ipvm::TimeCheck TimeSegmentation;
    if (Segmentation(thresholdImage, imageLabel_Dark, psDarkObjInfo, nDarkBlobMaxNum, imageLabel_Bright,
            psBrightObjInfo, nBrightBlobMaxNum, o_imageLabel))
        return false;

    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("SegmentationTime"), CAST_FLOAT(TimeSegmentation.Elapsed_ms()));

    // Createria 검사
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP)
    {
        Ipvm::TimeCheck TimeCalcCharacteristics3D;
        if (CalcCharacteristics3D(detailSetupMode, thresholdImage, prepairResult.m_grayImage, prepairResult.m_maskImage,
                prepairResult.m_heightMap, SurfaceItemSpec))
        {
            return false;
        }

        AddProcessingFunctionExcuteTimeLog(
            m_strModuleName, _T("CalcCharacteristics3DTime"), CAST_FLOAT(TimeCalcCharacteristics3D.Elapsed_ms()));
    }
    else
    {
        Ipvm::TimeCheck TimeCalcCharacteristics2D;
        if (CalcCharacteristics2D(detailSetupMode, prepairResult.m_grayImage, &prepairResult.m_aux1GrayImage,
                &prepairResult.m_aux2GrayImage, prepairResult.m_maskImage, prepairResult.m_backgroundImage,
                SurfaceItemSpec))
        {
            return false;
        }

        AddProcessingFunctionExcuteTimeLog(
            m_strModuleName, _T("CalcCharacteristics2DTime"), CAST_FLOAT(TimeCalcCharacteristics2D.Elapsed_ms()));
    }

    // Criteria 결과 값 정리
    Ipvm::TimeCheck TimeGetCharacteristicResults;
    if (!GetCharacteristicResults(detailSetupMode, SurfaceItemSpec.AlgoPara))
    {
        return false;
    }
    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("GetCharacteristicResultsTime"), CAST_FLOAT(TimeGetCharacteristicResults.Elapsed_ms()));

    // Criteria 양불 판정하기
    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    if ((long)(SurfaceItemSpec.vecCriteria.size()) > 0)
    {
        m_result.UpdateResults(getScale(), prepairResult.m_is3D, imageCenter, SurfaceItemSpec);
    }

    o_thresholdImage = thresholdImage;

    return true;
}

long VisionInspectionSurface::Segmentation(const Ipvm::Image8u& thresholdImage, Ipvm::Image32s& imageLabel_Dark,
    Ipvm::BlobInfo* psDarkObjInfo, int32_t nDarkBlobMaxNum, Ipvm::Image32s& imageLabel_Bright,
    Ipvm::BlobInfo* psBrightObjInfo, int32_t nBrightBlobMaxNum, Ipvm::Image32s* o_imageLabel)
{
    Ipvm::Point32r2 imageCenter(thresholdImage.GetSizeX() * 0.5f, thresholdImage.GetSizeY() * 0.5f);

    float fMergePixelDistance = (float)(GetSurfaceAlgoPara()->m_mergeDistance * getScale().umToPixelXY());

    BOOL IsNeedDarkObject = GetSurfaceItem()->IsNeed_DarkObject(); // || !m_systemConfig.m_bInlineMode;
    BOOL IsNeedBrightObject = GetSurfaceItem()->IsNeed_BrightObject(); // || !m_systemConfig.m_bInlineMode;

    if (nDarkBlobMaxNum >= BLOB_INFO_MAX_SIZE)
        nDarkBlobMaxNum = BLOB_INFO_MAX_SIZE;

    if (nBrightBlobMaxNum >= BLOB_INFO_MAX_SIZE)
        nBrightBlobMaxNum = BLOB_INFO_MAX_SIZE;

    bool blobSuccess = false;

    Ipvm::Image8u thresholdImageForType;
    if (!getReusableMemory().GetInspByteImage(thresholdImageForType))
    {
        return -1;
    }

    Ipvm::Rect32s blobROI = getScale().convert_BCUToPixel(GetSurfaceAlgoPara()->m_rtROI_BCU, imageCenter);

    do
    {
        // Dark Object
        if (IsNeedDarkObject)
        {
            Ipvm::Rect32s rtLabelClearROI = blobROI;
            rtLabelClearROI.InflateRect(3, 3, 3, 3);
            rtLabelClearROI
                &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());

            Ipvm::ImageProcessing::Fill(rtLabelClearROI, 0, imageLabel_Dark);

            if (Ipvm::ImageProcessing::BinarizeEqual(
                    thresholdImage, Ipvm::Rect32s(thresholdImageForType), 0, thresholdImageForType)
                != Ipvm::Status::e_ok)
            {
                break;
            }

            long nDarkBlobArea = max(4,
                long(GetSurfaceAlgoPara()->fMininumDarkBlobArea_umSqure * getScale().umToPixel().m_x
                        * getScale().umToPixel().m_y
                    + 0.5f));

            if (m_pBlob->DoBlob(thresholdImageForType, blobROI, nDarkBlobMaxNum, imageLabel_Dark, psDarkObjInfo,
                    nDarkBlobMaxNum, nDarkBlobArea)
                != 0)
            {
                break;
            }

            if (fMergePixelDistance > 0 && GetSurfaceAlgoPara()->m_mergeOnlyTheSameColor)
            {
                if (m_pBlob->MergeBlobsByDistance(psDarkObjInfo, nDarkBlobMaxNum, fMergePixelDistance, imageLabel_Dark)
                    != 0)
                {
                    return -1;
                }
            }
        }

        // Bright Object
        if (IsNeedBrightObject)
        {
            Ipvm::Rect32s rtLabelClearROI = blobROI;
            rtLabelClearROI.InflateRect(3, 3, 3, 3);
            rtLabelClearROI
                &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());

            Ipvm::ImageProcessing::Fill(rtLabelClearROI, 0, imageLabel_Bright);

            if (Ipvm::ImageProcessing::BinarizeEqual(
                    thresholdImage, Ipvm::Rect32s(thresholdImageForType), 255, thresholdImageForType)
                != Ipvm::Status::e_ok)
            {
                break;
            }

            long nBrightBlobArea = max(4,
                long(GetSurfaceAlgoPara()->fMinimumBrightBlobArea_umSqure * getScale().umToPixel().m_x
                        * getScale().umToPixel().m_y
                    + 0.5f));
            if (m_pBlob->DoBlob(thresholdImageForType, blobROI, nBrightBlobMaxNum, imageLabel_Bright, psBrightObjInfo,
                    nBrightBlobMaxNum, nBrightBlobArea)
                != 0)
            {
                break;
            }

            if (fMergePixelDistance > 0 && GetSurfaceAlgoPara()->m_mergeOnlyTheSameColor)
            {
                if (m_pBlob->MergeBlobsByDistance(
                        psBrightObjInfo, nBrightBlobMaxNum, fMergePixelDistance, imageLabel_Bright)
                    != 0)
                {
                    return -1;
                }
            }
        }

        blobSuccess = true;
    }
    while (0);

    if (!blobSuccess)
    {
        return -1;
    }

    if (IsNeedDarkObject && IsNeedBrightObject)
    {
        if (m_pBlob->MixBlobGroup(
                imageLabel_Dark, psDarkObjInfo, nDarkBlobMaxNum, psBrightObjInfo, nBrightBlobMaxNum, imageLabel_Bright)
            != 0)
        {
            return -1;
        }

        m_nBlobNum = nDarkBlobMaxNum;
        *m_plLabel_ = imageLabel_Dark;
        *m_plMaskLabel_ = imageLabel_Bright;
        m_psObjInfo_ = psDarkObjInfo;
        m_psMaskInfo_ = psBrightObjInfo;
    }
    else if (IsNeedDarkObject)
    {
        m_nBlobNum = nDarkBlobMaxNum;
        *m_plLabel_ = imageLabel_Dark;
        *m_plMaskLabel_ = imageLabel_Bright;
        m_psObjInfo_ = psDarkObjInfo;
        m_psMaskInfo_ = psBrightObjInfo;
    }
    else if (IsNeedBrightObject)
    {
        m_nBlobNum = nBrightBlobMaxNum;
        *m_plLabel_ = imageLabel_Bright;
        *m_plMaskLabel_ = imageLabel_Dark;
        m_psObjInfo_ = psBrightObjInfo;
        m_psMaskInfo_ = psDarkObjInfo;
    }
    else
    {
        m_nBlobNum = 0;
        *m_plLabel_ = imageLabel_Dark;
        *m_plMaskLabel_ = imageLabel_Bright;
        m_psObjInfo_ = psDarkObjInfo;
        m_psMaskInfo_ = psBrightObjInfo;
    }

    if (fMergePixelDistance > 0 && !GetSurfaceAlgoPara()->m_mergeOnlyTheSameColor)
    {
        m_nBlobNum = (long)min(m_nBlobNum, BLOB_INFO_MAX_SIZE); //kircheis_20210202 Crash
        if (m_pBlob->MergeBlobsByDistance(m_psObjInfo_, m_nBlobNum, fMergePixelDistance, *m_plLabel_) != 0)
        {
            return -1;
        }
    }

    if (o_imageLabel)
    {
        if (getReusableMemory().GetInspLongImage(*o_imageLabel))
        {
            Ipvm::ImageProcessing::Copy(*m_plLabel_, Ipvm::Rect32s(*m_plLabel_), *o_imageLabel);
        }
    }

    return 0;
}

long VisionInspectionSurface::CalcCharacteristics2D(const bool detailSetupMode, const Ipvm::Image8u& grayImage,
    const Ipvm::Image8u* aux1GrayImage, const Ipvm::Image8u* aux2GrayImage, const Ipvm::Image8u& maskImage,
    const Ipvm::Image8u& imageBackground, const CSurfaceItem& SurfaceItem)
{
    const CSurfaceAlgoPara& AlgoPara = SurfaceItem.AlgoPara;

    float fMinAreaDarkContrast
        = (float)(getScale().umToPixel().m_x * getScale().umToPixel().m_y * AlgoPara.fMinAreaDarkContrast);
    float fMinAreaBrightContrast
        = (float)(getScale().umToPixel().m_x * getScale().umToPixel().m_y * AlgoPara.fMinAreaBrightContrast);

    BlobAttribute::ResetArray(m_blobAttributes, m_nBlobNum);

    // 무게중심은 무조건 구하자
    if (!m_pBlob->CalcBlobMassCenter(*m_plLabel_, m_psObjInfo_, m_nBlobNum, m_blobAttributes))
    {
        return -1;
    }

    // Key & Average Contrast
    // 2007.05.13 RaSTeR
    // 오브젝트의 색상 속성은 Key Contrast 를 계산할 때 정해지므로
    // 예산 필요 여부와 상관없이 무조건 계산해야 함

    if (m_pBlob->CalcBlobContrast(grayImage, imageBackground, *m_plLabel_, m_psObjInfo_, m_nBlobNum,
            fMinAreaDarkContrast, fMinAreaBrightContrast, m_blobAttributes))
        return -1;
    //	}

    // Local Contrast
    if ((SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_LocalContrast)
            && SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_AuxLocalContrast))
        || detailSetupMode)
    {
        if (Calc2D_LocalContrast(m_psObjInfo_, *m_plLabel_, m_nBlobNum, maskImage, &grayImage, aux1GrayImage,
                aux2GrayImage, m_blobAttributes))
            return -1;
    }
    else
    {
        if (SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_LocalContrast) || detailSetupMode)
        {
            if (Calc2D_LocalContrast(
                    m_psObjInfo_, *m_plLabel_, m_nBlobNum, maskImage, &grayImage, nullptr, nullptr, m_blobAttributes))
                return -1;
        }

        if (SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_AuxLocalContrast) || detailSetupMode)
        {
            if (Calc2D_LocalContrast(m_psObjInfo_, *m_plLabel_, m_nBlobNum, maskImage, nullptr, aux1GrayImage,
                    aux2GrayImage, m_blobAttributes))
                return -1;
        }
    }

    // Deviation
    if (SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_Deviation) || detailSetupMode)
    {
        if (Calc2D_Deviation(m_psObjInfo_, *m_plLabel_, m_nBlobNum, grayImage, m_blobAttributes))
            return -1;
    }

    // Aux Deviation
    if (SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_AuxDeviation) || detailSetupMode)
    {
        if (Calc2D_AuxDeviation(m_psObjInfo_, *m_plLabel_, m_nBlobNum, aux1GrayImage, aux2GrayImage, m_blobAttributes))
        {
            return -1;
        }
    }

    // Edge Energy
    if (SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_EdgeEnegry) || detailSetupMode)
    {
        if (!m_pBlob->CalcBlobEdgeEnergy(grayImage, *m_plLabel_, m_psObjInfo_, m_nBlobNum, maskImage, m_blobAttributes))
        {
            return false;
        }
    }

    BOOL bIsMergedCount = SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_MergedCount);
    BOOL bIsBolbGV = (SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_BlobGV_AVR)
        || SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_BlobGV_Min)
        || SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_BlobGV_Max));

    if (bIsMergedCount || bIsBolbGV || detailSetupMode)
    {
        if (AnalisysBlobInfo(bIsMergedCount, bIsBolbGV, detailSetupMode) != TRUE)
        {
            return false;
        }
    }

    if (!CalcCharacteristicsCommon(detailSetupMode, grayImage, maskImage, SurfaceItem))
    {
        return -1;
    }

    return 0;
}

long VisionInspectionSurface::CalcCharacteristics3D(const bool detailSetupMode, const Ipvm::Image8u& thresholdImage,
    const Ipvm::Image8u& grayImage, const Ipvm::Image8u& maskImage, const Ipvm::Image32r& heightMap,
    const CSurfaceItem& SurfaceItem)
{
    BlobAttribute::ResetArray(m_blobAttributes, m_nBlobNum);

    // Blob의 Color Type을 계산하자
    // Blob레벨에서 Color Type을 갖고 있지 않으므로 Threshold 값을 보고 판단하자

    for (long blobIndex = 0; blobIndex < m_nBlobNum; blobIndex++)
    {
        long label = m_psObjInfo_[blobIndex].m_label;
        auto roi = m_psObjInfo_[blobIndex].m_roi;

        long d_c = 0;
        long r_c = 0;

        m_blobAttributes[blobIndex].m_blobType = BlobAttribute::BRIGHT;

        for (long y = roi.m_top; y < roi.m_bottom; y++)
        {
            auto* threshold_y = thresholdImage.GetMem(0, y);
            auto* label_y = m_plLabel_->GetMem(0, y);
            for (long x = roi.m_left; x < roi.m_right; x++)
            {
                if (label_y[x] == label)
                {
                    if (threshold_y[x] == DARKOBJECTPIXELGV)
                        d_c++;
                    else
                    {
                        r_c++;
                    }
                }
            }
        }

        if (d_c > r_c)
            m_blobAttributes[blobIndex].m_blobType = BlobAttribute::DARK;
    }

    // 무게중심은 무조건 구하자
    if (!m_pBlob->CalcBlobMassCenter(*m_plLabel_, m_psObjInfo_, m_nBlobNum, m_blobAttributes))
    {
        return -1;
    }

    if (SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t3D_AvgDeltaHeight) || detailSetupMode)
    {
        if (!Calc3D_AvgDelaHeight(m_psObjInfo_, *m_plLabel_, m_nBlobNum, thresholdImage, heightMap, m_blobAttributes))
        {
            return -1;
        }
    }

    if (SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::t3D_KeyDeltaHeight) || detailSetupMode)
    {
        if (!Calc3D_KeyDelaHeight(
                SurfaceItem, m_psObjInfo_, *m_plLabel_, m_nBlobNum, thresholdImage, heightMap, m_blobAttributes))
        {
            return -1;
        }
    }

    if (!CalcCharacteristicsCommon(detailSetupMode, grayImage, maskImage, SurfaceItem))
    {
        return -1;
    }

    return 0;
}

bool VisionInspectionSurface::CalcCharacteristicsCommon(const bool detailSetupMode, const Ipvm::Image8u& grayImage,
    const Ipvm::Image8u& maskImage, const CSurfaceItem& SurfaceItem)
{
    Ipvm::Point32r2 imageCenter(grayImage.GetSizeX() * 0.5f, grayImage.GetSizeY() * 0.5f);

    const CSurfaceAlgoPara& AlgoPara = SurfaceItem.AlgoPara;

    // Width,  Length
    if (SurfaceItem.IsNeed_WidthLengthAspectRatioBinaryDensity() || detailSetupMode)
    {
        if (!m_pBlob->CalcBlobWidthLength(*m_plLabel_, m_psObjInfo_, m_nBlobNum, m_blobAttributes))
            return false;
    }

    // LengthX, LengthY
    if (SurfaceItem.IsNeed_LengthXY() || detailSetupMode)
    {
        if (m_pBlob->CalcBlobLengthXY(m_psObjInfo_, m_nBlobNum, m_blobAttributes))
            return false;
    }

    // Thickness
    if (SurfaceItem.IsNeed_ThicknessLocus() || (detailSetupMode && !m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode))
    {
        Ipvm::Image8u imageBuffer1;
        Ipvm::Image32r floatImageBuffer;

        if (!getReusableMemory().GetInspByteImage(imageBuffer1))
            return false;
        if (!getReusableMemory().GetInspFloatImage(floatImageBuffer))
            return false;

        if (!m_pBlob->CalcBlobThickness(
                m_psObjInfo_, *m_plLabel_, m_nBlobNum, imageBuffer1, floatImageBuffer, m_blobAttributes))
        {
            return false;
        }
    }

    // Local Area
    if (SurfaceItem.IsNeed_Local_Area() || detailSetupMode)
    {
        Ipvm::Rect32s rtROI = getScale().convert_BCUToPixel(AlgoPara.m_rtROI_BCU, imageCenter);

        // 영훈 20141223_PackageSpecCrash : Package spec에 body Size가 들어가있지 않으면 Crash가 남는 문제 수정.
        if (rtROI.Width() <= 0 || rtROI.Height() <= 0)
        {
            return false;
        }

        long nMinBlobSize = 10;
        int32_t nBlobNum = 100;

        m_pBlob->DoBlob(maskImage, rtROI, nBlobNum, *m_plMaskLabel_, m_psMaskInfo_, nBlobNum, nMinBlobSize);
        m_pBlob->FilteringROI(*m_plMaskLabel_, rtROI, 10, m_psMaskInfo_, nBlobNum);

        if (m_pBlob->CalcLocalArea(m_psMaskInfo_, nBlobNum, m_psObjInfo_, m_nBlobNum, m_blobAttributes))
            return false;
    }

    if (SurfaceItem.IsNeed_Spec(VisionSurfaceCriteria_Column::DefectROI_ratio) || detailSetupMode)
    {
        if (m_pBlob->CalcDefect_ROI_Ratio(m_psObjInfo_, m_nBlobNum, m_blobAttributes))
        {
            return false;
        }
    }

    return true;
}

long VisionInspectionSurface::Calc2D_LocalContrast(const Ipvm::BlobInfo* psBlob, const Ipvm::Image32s& imageLabel,
    const long nBlobNum, const Ipvm::Image8u& maskImage, const Ipvm::Image8u* grayImage, const Ipvm::Image8u* aux1Image,
    const Ipvm::Image8u* aux2Image, BlobAttribute* attributes)
{
    // Auxiliary Local Contrast 의 경우에는
    // (-) 값으로 나올 수 있도록 함.
    // 메인 오브젝트의 컬러에 따라서 반대색을 가지는 보조 오브젝트라면
    // (-) 값으로 나와야 이놈이 반대색인지 아닌지를 알 수 있으므로
    // 그렇게 동작하도록 코딩함.

    Ipvm::Image8u imageBinary;
    Ipvm::Image8u imageDilateX;
    Ipvm::Image8u imageBuffer;

    if (!getReusableMemory().GetInspByteImage(imageBinary))
        return -1;
    if (!getReusableMemory().GetInspByteImage(imageDilateX))
        return -1;
    if (!getReusableMemory().GetInspByteImage(imageBuffer))
        return -1;

    // 영훈 20130912 : Clearimage도 Surface가 많아지면 시간이 많이 걸리므로 bodyalign 영역정도만 초기화해준도록 하자.
    PI_RECT arBody(
        m_EdgeAlignResult->fptLT, m_EdgeAlignResult->fptRT, m_EdgeAlignResult->fptLB, m_EdgeAlignResult->fptRB);
    Ipvm::Rect32s rtBody(arBody.CreateExternalMinimumRect());
    long nOffsetX, nOffsetY;

    nOffsetX = (long)(rtBody.CenterPoint().m_x - (getImageLotInsp().GetImageSizeX() * 0.5f));
    nOffsetY = (long)(rtBody.CenterPoint().m_y - (getImageLotInsp().GetImageSizeY() * 0.5f));

    long offsetL = (long)(getScale().convert_umToPixelX(GetSurfaceAlgoPara()->m_offsetLeft_um) + 0.5f);
    long offsetR = (long)(getScale().convert_umToPixelX(GetSurfaceAlgoPara()->m_offsetRight_um) + 0.5f);
    long offsetT = (long)(getScale().convert_umToPixelY(GetSurfaceAlgoPara()->m_offsetTop_um) + 0.5f);
    long offsetB = (long)(getScale().convert_umToPixelY(GetSurfaceAlgoPara()->m_offsetBottom_um) + 0.5f);

    Ipvm::Rect32s rtInitBinary = rtBody;
    rtInitBinary.InflateRect(-offsetL, -offsetT, -offsetR, -offsetB);
    rtInitBinary.OffsetRect(-nOffsetX, -nOffsetY);
    rtInitBinary &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());

    Ipvm::ImageProcessing::Fill(rtInitBinary, 0, imageBinary);

    long nROIOffset = (long)(getScale().convert_umToPixelX(GetSurfaceAlgoPara()->fLocalContrastAreaSpan) + 0.5f);
    if (nROIOffset < 3)
        nROIOffset = 3; // 최소 마스크를 3x3으로 해주자.
    if (nROIOffset % 2 == 0)
        nROIOffset += 1; // 홀수만 입력하도록 한다.

    for (long nBlob = 0; nBlob < nBlobNum; nBlob++)
    {
        attributes[nBlob].m_localContrast = 0.f;
        attributes[nBlob].m_auxLocalContrast = 0.f;

        if (psBlob[nBlob].m_area == 0)
        {
            continue;
        }

        auto rtROI(psBlob[nBlob].m_roi);
        rtROI.InflateRect(nROIOffset, nROIOffset, nROIOffset, nROIOffset);
        rtROI &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());

        const long nBlobLabel = psBlob[nBlob].m_label;

        ///////////////////////////////////////
        //
        // 이 코드는 단순한게 박스를 취해서 계산하지 않고,
        // 모폴로지 연산으로 오브젝트를 불린 다음에,
        // 불어난 영역만을 배경으로 생각해서 계산하는 코드임.
        //
        // Make Source Image
        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++) // 영훈 : Blob ROI내부에서 Object를 찾는다.
        {
            auto* label_y = imageLabel.GetMem(0, y);
            auto* binary_y = imageBinary.GetMem(0, y);

            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (label_y[x] == nBlobLabel) // 영훈 : Image상에 하나의 Object를 찾아서 버퍼에 그려준다.
                {
                    binary_y[x] = 255;
                }
                else
                {
                    binary_y[x] = 0;
                }
            }
        }

        rtROI.InflateRect(nROIOffset, nROIOffset, nROIOffset, nROIOffset);
        rtROI &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());

        Ipvm::ImageProcessing::MorphDilate(imageBinary, rtROI, nROIOffset, 1, true, imageDilateX);
        Ipvm::ImageProcessing::MorphDilate(imageDilateX, rtROI, 1, nROIOffset, true, imageBuffer);

        // 영훈 : 두개의 이미지가 모두 있을 경우.. 하지만 Combine Image를 사용하지 않더라도 선택은 항상 되어 있으므로 음... 이건 생각 좀 해봐야 할 듯..
        if (grayImage && aux1Image && aux2Image)
        {
            long nForegroundGrayValueSum = 0;
            long nBackgroundGrayValueSum = 0;
            long nAux1ForegroundGrayValueSum = 0;
            long nAux1BackgroundGrayValueSum = 0;
            long nAux2ForegroundGrayValueSum = 0;
            long nAux2BackgroundGrayValueSum = 0;
            long nForegroundPixelCount = 0;
            long nBackgroundPixelCount = 0;

            // 영훈 : 기본 Size에서 모폴로지를 하면서 커져버린 영역만큼 보도록 한다.
            for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
            {
                if (rtInitBinary.m_top > y || rtInitBinary.m_bottom < y)
                    continue;

                auto* label_y = imageLabel.GetMem(0, y);
                auto* buffer_y = imageBuffer.GetMem(0, y);
                auto* mask_y = maskImage.GetMem(0, y);
                auto* gray_y = grayImage->GetMem(0, y);
                auto* aux1_y = aux1Image->GetMem(0, y);
                auto* aux2_y = aux2Image->GetMem(0, y);

                for (long x = rtROI.m_left; x < rtROI.m_right; x++)
                {
                    if (rtInitBinary.m_left > x || rtInitBinary.m_right < x)
                        continue;

                    if (label_y[x] == nBlobLabel)
                    {
                        nForegroundGrayValueSum
                            += gray_y[x]; // 영훈 : 실제 Current Image에서 오브젝트의 Pixel Value를 모두 더한다.
                        nAux1ForegroundGrayValueSum
                            += aux1_y[x]; // 영훈 : Aux Frame에서 오브젝트의 위치에 해당하는 Pixel Value를 모두 더한다.
                        nAux2ForegroundGrayValueSum += aux2_y[x];
                        nForegroundPixelCount++; // 여기 한번 진입했다고 카운트를 올린다. 그리고 이 값으로 평균을 구하도록 한다.
                    }
                    else if (buffer_y[x] && mask_y[x]
                        && gray_y
                            [x]) // 주변에 다른 오브젝트가 있다 하더라도, 배경의 일부로 취급함. // 영훈 : 확장을 시킨 부분이 마스크를 넘어가면 계산을 하지 않도록 한다.
                    {
                        nBackgroundGrayValueSum += gray_y
                            [x]; // 영훈 : 오브젝트에서 넘어간 값이면서 확장된 영역 내부이면 그 Pixel Value를 더한다.
                        nAux1BackgroundGrayValueSum += aux1_y[x]; // 이하 동문
                        nAux2BackgroundGrayValueSum += aux2_y[x];
                        nBackgroundPixelCount++; // 역시.. 카운팅
                    }
                }
            }

            if (nBackgroundPixelCount)
            {
                // 오브젝트의 평균 밝기 값을 구한다.
                float fForegroundAvgGrayValue = float(nForegroundGrayValueSum) / nForegroundPixelCount;
                float fAux1ForegroundAvgGrayValue = float(nAux1ForegroundGrayValueSum) / nForegroundPixelCount;
                float fAux2ForegroundAvgGrayValue = float(nAux2ForegroundGrayValueSum) / nForegroundPixelCount;
                float fAuxForegroundAvgGrayValue = (fAux1ForegroundAvgGrayValue + fAux2ForegroundAvgGrayValue) / 2;

                // 배경의 평균 밝기 값을 구한다.
                float fBackgroundAvgGrayValue = float(nBackgroundGrayValueSum) / nBackgroundPixelCount;
                float fAux1BackgroundAvgGrayValue = float(nAux1BackgroundGrayValueSum) / nBackgroundPixelCount;
                float fAux2BackgroundAvgGrayValue = float(nAux2BackgroundGrayValueSum) / nBackgroundPixelCount;
                float fAuxBackgroundAvgGrayValue = (fAux1BackgroundAvgGrayValue + fAux2BackgroundAvgGrayValue) / 2;

                // 밝은 오브젝트일 경우
                if (attributes[nBlob].m_blobType == BlobAttribute::BRIGHT)
                {
                    attributes[nBlob].m_localContrast = max(fForegroundAvgGrayValue - fBackgroundAvgGrayValue, 0.f);
                    attributes[nBlob].m_auxLocalContrast = fAuxForegroundAvgGrayValue - fAuxBackgroundAvgGrayValue;
                }
                // 어두운 오브젝트일 경우
                else if (attributes[nBlob].m_blobType == BlobAttribute::DARK)
                {
                    attributes[nBlob].m_localContrast = max(fBackgroundAvgGrayValue - fForegroundAvgGrayValue, 0.f);
                    attributes[nBlob].m_auxLocalContrast = fAuxBackgroundAvgGrayValue - fAuxForegroundAvgGrayValue;
                }
                else
                {
                    ASSERT(!"Not Implemented");
                }
            }
            else
            {
                // Background 가 없을 경우에는 컨트라스트는 무한대라고 본다.
                attributes[nBlob].m_localContrast = 255.f;
                attributes[nBlob].m_auxLocalContrast = 255.f;
            }
        }
        else if (grayImage)
        {
            long nForegroundGrayValueSum = 0;
            long nBackgroundGrayValueSum = 0;
            long nForegroundPixelCount = 0;
            long nBackgroundPixelCount = 0;

            for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
            {
                if (rtInitBinary.m_top > y || rtInitBinary.m_bottom < y)
                    continue;

                auto* label_y = imageLabel.GetMem(0, y);
                auto* buffer_y = imageBuffer.GetMem(0, y);
                auto* mask_y = maskImage.GetMem(0, y);
                auto* gray_y = grayImage->GetMem(0, y);

                for (long x = rtROI.m_left; x < rtROI.m_right; x++)
                {
                    if (rtInitBinary.m_left > x || rtInitBinary.m_right < x)
                        continue;

                    if (label_y[x] == nBlobLabel)
                    {
                        nForegroundGrayValueSum += gray_y[x];
                        nForegroundPixelCount++;
                    }
                    else if (buffer_y[x] && mask_y[x]
                        && gray_y[x]) // 주변에 다른 오브젝트가 있다 하더라도, 배경의 일부로 취급함.
                    {
                        nBackgroundGrayValueSum += gray_y[x];
                        nBackgroundPixelCount++;
                    }
                }
            }

            if (nBackgroundPixelCount)
            {
                float fForegroundAvgGrayValue = float(nForegroundGrayValueSum) / nForegroundPixelCount;
                float fBackgroundAvgGrayValue = float(nBackgroundGrayValueSum) / nBackgroundPixelCount;

                if (attributes[nBlob].m_blobType == BlobAttribute::BRIGHT)
                {
                    attributes[nBlob].m_localContrast = max(fForegroundAvgGrayValue - fBackgroundAvgGrayValue, 0.f);
                }
                else if (attributes[nBlob].m_blobType == BlobAttribute::DARK)
                {
                    attributes[nBlob].m_localContrast = max(fBackgroundAvgGrayValue - fForegroundAvgGrayValue, 0.f);
                }
                else
                {
                    ASSERT(!"Not Implemented");
                }
            }
            else
            {
                // Background 가 없을 경우에는 컨트라스트는 무한대라고 본다.
                attributes[nBlob].m_localContrast = 255.f;
            }
        }
        else if (aux1Image)
        {
            long nAuxForegroundGrayValueSum = 0;
            long nAuxBackgroundGrayValueSum = 0;
            long nForegroundPixelCount = 0;
            long nBackgroundPixelCount = 0;

            for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
            {
                if (rtInitBinary.m_top > y || rtInitBinary.m_bottom < y)
                    continue;

                auto* label_y = imageLabel.GetMem(0, y);
                auto* buffer_y = imageBuffer.GetMem(0, y);
                auto* mask_y = maskImage.GetMem(0, y);
                auto* aux_y = aux1Image->GetMem(0, y);

                for (long x = rtROI.m_left; x < rtROI.m_right; x++)
                {
                    if (rtInitBinary.m_left > x || rtInitBinary.m_right < x)
                        continue;

                    if (label_y[x] == nBlobLabel)
                    {
                        nAuxForegroundGrayValueSum += aux_y[x];
                        nForegroundPixelCount++;
                    }
                    else if (buffer_y[x] && mask_y[x]
                        && aux_y[x]) // 주변에 다른 오브젝트가 있다 하더라도, 배경의 일부로 취급함.
                    {
                        nAuxBackgroundGrayValueSum += aux_y[x];
                        nBackgroundPixelCount++;
                    }
                }
            }

            if (nBackgroundPixelCount)
            {
                float fAuxForegroundAvgGrayValue = float(nAuxForegroundGrayValueSum) / nForegroundPixelCount;
                float fAuxBackgroundAvgGrayValue = float(nAuxBackgroundGrayValueSum) / nBackgroundPixelCount;

                if (attributes[nBlob].m_blobType == BlobAttribute::BRIGHT)
                {
                    attributes[nBlob].m_auxLocalContrast = fAuxForegroundAvgGrayValue - fAuxBackgroundAvgGrayValue;
                }
                else if (attributes[nBlob].m_blobType == BlobAttribute::DARK)
                {
                    attributes[nBlob].m_auxLocalContrast = fAuxBackgroundAvgGrayValue - fAuxForegroundAvgGrayValue;
                }
                else
                {
                    ASSERT(!"Not Implemented");
                }
            }
            else
            {
                // Background 가 없을 경우에는 컨트라스트는 무한대라고 본다.
                attributes[nBlob].m_auxLocalContrast = 255.f;
            }
        }
        else if (aux1Image)
        {
            long nAuxForegroundGrayValueSum = 0;
            long nAuxBackgroundGrayValueSum = 0;
            long nForegroundPixelCount = 0;
            long nBackgroundPixelCount = 0;

            for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
            {
                if (rtInitBinary.m_top > y || rtInitBinary.m_bottom < y)
                    continue;

                auto* label_y = imageLabel.GetMem(0, y);
                auto* buffer_y = imageBuffer.GetMem(0, y);
                auto* mask_y = maskImage.GetMem(0, y);
                auto* aux_y = aux1Image->GetMem(0, y);

                for (long x = rtROI.m_left; x < rtROI.m_right; x++)
                {
                    if (rtInitBinary.m_left > x || rtInitBinary.m_right < x)
                        continue;

                    if (label_y[x] == nBlobLabel)
                    {
                        nAuxForegroundGrayValueSum += aux_y[x];
                        nForegroundPixelCount++;
                    }
                    else if (buffer_y[x] && mask_y[x]
                        && aux_y[x]) // 주변에 다른 오브젝트가 있다 하더라도, 배경의 일부로 취급함.
                    {
                        nAuxBackgroundGrayValueSum += aux_y[x];
                        nBackgroundPixelCount++;
                    }
                }
            }

            if (nBackgroundPixelCount)
            {
                float fAuxForegroundAvgGrayValue = float(nAuxForegroundGrayValueSum) / nForegroundPixelCount;
                float fAuxBackgroundAvgGrayValue = float(nAuxBackgroundGrayValueSum) / nBackgroundPixelCount;

                if (attributes[nBlob].m_blobType == BlobAttribute::BRIGHT)
                {
                    attributes[nBlob].m_auxLocalContrast = fAuxForegroundAvgGrayValue - fAuxBackgroundAvgGrayValue;
                }
                else if (attributes[nBlob].m_blobType == BlobAttribute::DARK)
                {
                    attributes[nBlob].m_auxLocalContrast = fAuxBackgroundAvgGrayValue - fAuxForegroundAvgGrayValue;
                }
                else
                {
                    ASSERT(!"Not Implemented");
                }
            }
            else
            {
                // Background 가 없을 경우에는 컨트라스트는 무한대라고 본다.
                attributes[nBlob].m_auxLocalContrast = 255.f;
            }
        }
        else if (aux2Image)
        {
            long nAuxForegroundGrayValueSum = 0;
            long nAuxBackgroundGrayValueSum = 0;
            long nForegroundPixelCount = 0;
            long nBackgroundPixelCount = 0;

            for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
            {
                if (rtInitBinary.m_top > y || rtInitBinary.m_bottom < y)
                    continue;

                auto* label_y = imageLabel.GetMem(0, y);
                auto* buffer_y = imageBuffer.GetMem(0, y);
                auto* mask_y = maskImage.GetMem(0, y);
                auto* aux_y = aux2Image->GetMem(0, y);

                for (long x = rtROI.m_left; x < rtROI.m_right; x++)
                {
                    if (rtInitBinary.m_left > x || rtInitBinary.m_right < x)
                        continue;

                    if (label_y[x] == nBlobLabel)
                    {
                        nAuxForegroundGrayValueSum += aux_y[x];
                        nForegroundPixelCount++;
                    }
                    else if (buffer_y[x] && mask_y[x]
                        && aux_y[x]) // 주변에 다른 오브젝트가 있다 하더라도, 배경의 일부로 취급함.
                    {
                        nAuxBackgroundGrayValueSum += aux_y[x];
                        nBackgroundPixelCount++;
                    }
                }
            }

            if (nBackgroundPixelCount)
            {
                float fAuxForegroundAvgGrayValue = float(nAuxForegroundGrayValueSum) / nForegroundPixelCount;
                float fAuxBackgroundAvgGrayValue = float(nAuxBackgroundGrayValueSum) / nBackgroundPixelCount;

                if (attributes[nBlob].m_blobType == BlobAttribute::BRIGHT)
                {
                    attributes[nBlob].m_auxLocalContrast = fAuxForegroundAvgGrayValue - fAuxBackgroundAvgGrayValue;
                }
                else if (attributes[nBlob].m_blobType == BlobAttribute::DARK)
                {
                    attributes[nBlob].m_auxLocalContrast = fAuxBackgroundAvgGrayValue - fAuxForegroundAvgGrayValue;
                }
                else
                {
                    ASSERT(!"Not Implemented");
                }
            }
            else
            {
                // Background 가 없을 경우에는 컨트라스트는 무한대라고 본다.
                attributes[nBlob].m_auxLocalContrast = 255.f;
            }
        }
    }

    return 0;
}

long VisionInspectionSurface::Calc2D_Deviation(const Ipvm::BlobInfo* psBlob, const Ipvm::Image32s& imageLabel,
    const long nBlobNum, const Ipvm::Image8u& image, BlobAttribute* attributes)
{
    for (long nBlob = 0; nBlob < nBlobNum; nBlob++)
    {
        attributes[nBlob].m_deviation = 0.f;

        if (psBlob[nBlob].m_area == 0)
        {
            continue;
        }

        const auto rtROI(psBlob[nBlob].m_roi);

        const long nBlobLabel = psBlob[nBlob].m_label;

        // Calc Foreground & Background Average Gray Value
        long nForegroundGrayValueSum = 0;
        long nForegroundPixelCount = 0;

        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            auto* image_y = image.GetMem(0, y);
            auto* label_y = imageLabel.GetMem(0, y);

            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (label_y[x] == nBlobLabel)
                {
                    nForegroundGrayValueSum += image_y[x];
                    nForegroundPixelCount++;
                }
            }
        }

        float fForegroundMean = float(nForegroundGrayValueSum) / nForegroundPixelCount;

        // Calc Foreground & Background Standard Deviation
        float fForegroundDiffSqrSum = 0.f;

        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            auto* image_y = image.GetMem(0, y);
            auto* label_y = imageLabel.GetMem(0, y);

            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (label_y[x] == nBlobLabel)
                {
                    fForegroundDiffSqrSum += (fForegroundMean - image_y[x]) * (fForegroundMean - image_y[x]);
                }
            }
        }

        attributes[nBlob].m_deviation = sqrt(fForegroundDiffSqrSum / nForegroundPixelCount);
    }

    return 0;
}

long VisionInspectionSurface::Calc2D_AuxDeviation(const Ipvm::BlobInfo* psBlob, const Ipvm::Image32s& imageLabel,
    const long nBlobNum, const Ipvm::Image8u* aux1Image, const Ipvm::Image8u* aux2Image, BlobAttribute* attributes)
{
    for (long nBlob = 0; nBlob < nBlobNum; nBlob++)
    {
        attributes[nBlob].m_auxDeviation = 0.f;

        if (aux1Image == nullptr)
            continue;
        if (aux2Image == nullptr)
            continue;
        if (psBlob[nBlob].m_area == 0)
            continue;

        const auto rtROI(psBlob[nBlob].m_roi);

        const long nBlobLabel = psBlob[nBlob].m_label;

        // Calc Foreground & Background Average Gray Value
        long nForegroundGray1ValueSum = 0;
        long nForegroundGray2ValueSum = 0;
        long nForegroundPixelCount = 0;

        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            const auto* aux1_y = aux1Image->GetMem(0, y);
            const auto* aux2_y = aux2Image->GetMem(0, y);
            const auto* label_y = imageLabel.GetMem(0, y);

            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (label_y[x] == nBlobLabel)
                {
                    nForegroundGray1ValueSum += aux1_y[x];
                    nForegroundGray2ValueSum += aux2_y[x];
                    nForegroundPixelCount++;
                }
            }
        }

        float fForeground1Mean = float(nForegroundGray1ValueSum) / nForegroundPixelCount;
        float fForeground2Mean = float(nForegroundGray2ValueSum) / nForegroundPixelCount;

        // Calc Foreground & Background Standard Deviation
        float fForeground1DiffSqrSum = 0.f;
        float fForeground2DiffSqrSum = 0.f;
        float fForegroundDiffSqrSum = 0.f;

        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            const auto* aux1_y = aux1Image->GetMem(0, y);
            const auto* aux2_y = aux2Image->GetMem(0, y);
            const auto* label_y = imageLabel.GetMem(0, y);

            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (label_y[x] == nBlobLabel)
                {
                    fForeground1DiffSqrSum += (fForeground1Mean - aux1_y[x]) * (fForeground1Mean - aux1_y[x]);
                    fForeground2DiffSqrSum += (fForeground2Mean - aux2_y[x]) * (fForeground2Mean - aux2_y[x]);
                }
            }
        }

        fForegroundDiffSqrSum = (fForeground1DiffSqrSum + fForeground2DiffSqrSum) / 2;

        attributes[nBlob].m_auxDeviation = sqrt(fForegroundDiffSqrSum / nForegroundPixelCount);
    }

    return 0;
}

bool VisionInspectionSurface::Calc3D_AvgDelaHeight(const Ipvm::BlobInfo* psBlob, const Ipvm::Image32s& imageLabel,
    const long nBlobNum, const Ipvm::Image8u& threshold, const Ipvm::Image32r& heightMap, BlobAttribute* attributes)
{
    for (long blobIndex = 0; blobIndex < nBlobNum; blobIndex++)
    {
        const auto& blob = psBlob[blobIndex];
        if (blob.m_area == 0)
            continue;

        long label = blob.m_label;
        double sum = 0.;
        long sumCount = 0;

        if (attributes[blobIndex].m_blobType == BlobAttribute::BRIGHT)
        {
            for (long y = blob.m_roi.m_top; y < blob.m_roi.m_bottom; y++)
            {
                auto* height_y = heightMap.GetMem(0, y);
                auto* threshold_y = threshold.GetMem(0, y);
                auto* label_y = imageLabel.GetMem(0, y);

                for (long x = blob.m_roi.m_left; x < blob.m_roi.m_right; x++)
                {
                    if (label_y[x] != label)
                        continue;
                    if (threshold_y[x] == DARKOBJECTPIXELGV)
                        continue;

                    sum += height_y[x];
                    sumCount++;
                }
            }
        }
        else
        {
            for (long y = blob.m_roi.m_top; y < blob.m_roi.m_bottom; y++)
            {
                auto* height_y = heightMap.GetMem(0, y);
                auto* threshold_y = threshold.GetMem(0, y);
                auto* label_y = imageLabel.GetMem(0, y);

                for (long x = blob.m_roi.m_left; x < blob.m_roi.m_right; x++)
                {
                    if (label_y[x] != label)
                        continue;
                    if (threshold_y[x] != DARKOBJECTPIXELGV)
                        continue;

                    sum += height_y[x];
                    sumCount++;
                }
            }
        }

        if (sumCount)
        {
            attributes[blobIndex].m_averageDeltaHeight = float(sum / sumCount) / g_fMilToUm;
        }
        else
        {
            attributes[blobIndex].m_averageDeltaHeight = 0.f;
        }
    }

    return true;
}

bool VisionInspectionSurface::Calc3D_KeyDelaHeight(const CSurfaceItem& sItem, const Ipvm::BlobInfo* psBlob,
    const Ipvm::Image32s& imageLabel, const long nBlobNum, const Ipvm::Image8u& threshold,
    const Ipvm::Image32r& heightMap, BlobAttribute* attributes)
{
    const CSurfaceAlgoPara& AlgoPara = sItem.AlgoPara;

    float fMinAreaDarkContrast
        = (float)(getScale().umToPixel().m_x * getScale().umToPixel().m_y * AlgoPara.fMinAreaDarkContrast);
    float fMinAreaBrightContrast
        = (float)(getScale().umToPixel().m_x * getScale().umToPixel().m_y * AlgoPara.fMinAreaBrightContrast);

    // Float 값은 Histogram 형태로 계산하기 힘드므로 계산 범위를 1mm로 제한, 최소단위는 0.1um 로 제한했다
    Ipvm::Image32s histogram;
    if (!getReusableMemory().GetLongImage(histogram, 10001, 1))
        return false;

    auto* pnHistoDist = histogram.GetMem();

    for (long blobIndex = 0; blobIndex < nBlobNum; blobIndex++)
    {
        const auto& blob = psBlob[blobIndex];
        if (blob.m_area == 0)
            continue;

        long label = blob.m_label;

        std::vector<float> deltaHeightList;

        histogram.FillZero();
        attributes[blobIndex].m_keyDeltaHeight = 0.f;

        float fMinAreaContrast = 0.f;

        if (attributes[blobIndex].m_blobType == BlobAttribute::BRIGHT)
        {
            fMinAreaContrast = fMinAreaBrightContrast;

            for (long y = blob.m_roi.m_top; y < blob.m_roi.m_bottom; y++)
            {
                auto* height_y = heightMap.GetMem(0, y);
                auto* threshold_y = threshold.GetMem(0, y);
                auto* label_y = imageLabel.GetMem(0, y);

                for (long x = blob.m_roi.m_left; x < blob.m_roi.m_right; x++)
                {
                    if (label_y[x] != label)
                        continue;
                    if (threshold_y[x] == DARKOBJECTPIXELGV)
                        continue;

                    long value = long(min(10000.f, fabs(height_y[x] * 10.f)));
                    pnHistoDist[value]++;
                }
            }

            // Find Key Delta Height
            long nAccHisto = 0;
            for (long nIntensity = 10000; nIntensity >= 0; nIntensity--)
            {
                nAccHisto += pnHistoDist[nIntensity];
                if (nAccHisto >= fMinAreaBrightContrast)
                {
                    attributes[blobIndex].m_keyDeltaHeight = nIntensity * 0.1f / g_fMilToUm;
                    break;
                }
            }
        }
        else
        {
            fMinAreaContrast = fMinAreaDarkContrast;

            for (long y = blob.m_roi.m_top; y < blob.m_roi.m_bottom; y++)
            {
                auto* height_y = heightMap.GetMem(0, y);
                auto* threshold_y = threshold.GetMem(0, y);
                auto* label_y = imageLabel.GetMem(0, y);

                for (long x = blob.m_roi.m_left; x < blob.m_roi.m_right; x++)
                {
                    if (label_y[x] != label)
                        continue;
                    if (threshold_y[x] != DARKOBJECTPIXELGV)
                        continue;

                    long value = long(min(10000.f, fabs(height_y[x] * 10.f)));
                    pnHistoDist[value]++;
                }
            }

            // Find Key Delta Height
            long nAccHisto = 0;
            for (long nIntensity = 10000; nIntensity >= 0; nIntensity--)
            {
                nAccHisto += pnHistoDist[nIntensity];
                if (nAccHisto >= fMinAreaDarkContrast)
                {
                    attributes[blobIndex].m_keyDeltaHeight = -nIntensity * 0.1f / g_fMilToUm;
                    break;
                }
            }
        }
    }

    return true;
}

bool VisionInspectionSurface::GetCharacteristicResults(const bool detailSetupMode, const CSurfaceAlgoPara& AlgoPara)
{
    UNREFERENCED_PARAMETER(AlgoPara);

    const float fPixelsToMils = getScale().pixelToMil().m_x;
    const float fPixelsToMils_y = getScale().pixelToMil().m_y;
    const float fPixelsToMils2 = fPixelsToMils * fPixelsToMils_y;

    const float fImageCenX = getImageLotInsp().GetImageSizeX() * 0.5f;
    const float fImageCenY = getImageLotInsp().GetImageSizeY() * 0.5f;

    const float fX1 = m_EdgeAlignResult->fptLT.m_x;
    const float fY1 = m_EdgeAlignResult->fptLT.m_y;
    const float fX2 = m_EdgeAlignResult->fptRT.m_x;
    const float fY2 = m_EdgeAlignResult->fptRT.m_y;
    const float fX3 = m_EdgeAlignResult->fptRB.m_x;
    const float fY3 = m_EdgeAlignResult->fptRB.m_y;
    const float fX4 = m_EdgeAlignResult->fptLB.m_x;
    const float fY4 = m_EdgeAlignResult->fptLB.m_y;

    const float L1 = sqrt((fX1 - fX2) * (fX1 - fX2) + (fY1 - fY2) * (fY1 - fY2));
    const float L2 = sqrt((fX2 - fX3) * (fX2 - fX3) + (fY2 - fY3) * (fY2 - fY3));
    const float L3 = sqrt((fX3 - fX4) * (fX3 - fX4) + (fY3 - fY4) * (fY3 - fY4));
    const float L4 = sqrt((fX4 - fX1) * (fX4 - fX1) + (fY4 - fY1) * (fY4 - fY1));

    const float fBodyLeftX = fImageCenX - 0.25f * (L1 + L3);
    const float fBodyRightX = fImageCenX + 0.25f * (L1 + L3);
    const float fBodyTopY = fImageCenY - 0.25f * (L2 + L4);
    const float fBodyBottomY = fImageCenY + 0.25f * (L2 + L4);

    // 발견된 Surface Object 들을 result에 넣기.
    long nValidObjNum = 0;
    long idx = 0;
    for (; idx < m_nBlobNum; idx++)
    {
        if (m_psObjInfo_[idx].m_area)
            nValidObjNum++;
    }

    m_result.vecDSOValue.resize(nValidObjNum);
    SDetectedSurfaceObject* pDSOBegin = nValidObjNum ? &m_result.vecDSOValue[0] : NULL;
    for (long n = 0; n < nValidObjNum; n++)
    {
        pDSOBegin[n].Clear();
    }

    bool is3D = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP);

    std::vector<BOOL> need_items(GetCriteriaColumnCount(is3D), detailSetupMode);

    for (long item = 0; item < GetCriteriaColumnCount(is3D); item++)
    {
        auto columnID = GetCriteriaColumnID(is3D, item);
        if (GetSurfaceItem()->IsNeed_Spec(columnID))
        {
            need_items[item] = TRUE;
        }
    }

    long nValid = 0;
    for (idx = 0; idx < m_nBlobNum; idx++)
    {
        auto& blob_info = m_psObjInfo_[idx];
        auto& blob_attribute = m_blobAttributes[idx];

        if (blob_info.m_area == 0)
            continue;

        SDetectedSurfaceObject* pDSO = pDSOBegin + nValid;

        pDSO->m_color = blob_attribute.m_blobType;

        for (long item = 0; item < GetCriteriaColumnCount(is3D); item++)
        {
            if (!need_items[item])
                continue;

            auto columnID = GetCriteriaColumnID(is3D, item);

            auto& result = pDSO->m_inspValue[long(columnID)];

            switch (columnID)
            {
                case VisionSurfaceCriteria_Column::Area:
                    result = max(0.f, blob_info.m_area * fPixelsToMils2);
                    break;
                case VisionSurfaceCriteria_Column::Width:
                    result = max(0.f, blob_attribute.m_width * fPixelsToMils);
                    break;
                case VisionSurfaceCriteria_Column::Length:
                    result = max(0.f, blob_attribute.m_length * fPixelsToMils);
                    break;
                case VisionSurfaceCriteria_Column::LengthX:
                    result = max(0.f, blob_attribute.m_lengthX * fPixelsToMils);
                    break;
                case VisionSurfaceCriteria_Column::LengthY:
                    result = max(0.f, blob_attribute.m_lengthY * fPixelsToMils);
                    break;
                case VisionSurfaceCriteria_Column::TotalLength:
                    result = max(0.f, blob_attribute.m_totalLength * fPixelsToMils);
                    break;
                case VisionSurfaceCriteria_Column::Binary_density:
                    result = min(float(blob_info.m_area) / (blob_attribute.m_length * blob_attribute.m_width), 1.f);
                    break;
                case VisionSurfaceCriteria_Column::Thickness:
                    result = blob_attribute.m_thickness * fPixelsToMils;
                    break;
                case VisionSurfaceCriteria_Column::Locus:
                    result = blob_attribute.m_locus * fPixelsToMils;
                    break;
                case VisionSurfaceCriteria_Column::DefectROI_ratio:
                    result = blob_attribute.m_defectROIRatio;
                    break;
                case VisionSurfaceCriteria_Column::LocalArea_Percent_all:
                    result = blob_attribute.m_localAreaPercent_All;
                    break;
                case VisionSurfaceCriteria_Column::LocalArea_Percent:
                    result = blob_attribute.m_localAreaPercent;
                    break;
                case VisionSurfaceCriteria_Column::t2D_KeyContrast:
                    result = blob_attribute.m_keyContrast;
                    break;
                case VisionSurfaceCriteria_Column::t2D_AvgContrast:
                    result = blob_attribute.m_averageContrast;
                    break;
                case VisionSurfaceCriteria_Column::t2D_LocalContrast:
                    result = blob_attribute.m_localContrast;
                    break;
                case VisionSurfaceCriteria_Column::t2D_AuxLocalContrast:
                    result = blob_attribute.m_auxLocalContrast;
                    break;
                case VisionSurfaceCriteria_Column::t2D_Deviation:
                    result = blob_attribute.m_deviation;
                    break;
                case VisionSurfaceCriteria_Column::t2D_AuxDeviation:
                    result = blob_attribute.m_auxDeviation;
                    break;
                case VisionSurfaceCriteria_Column::t2D_EdgeEnegry:
                    result = blob_attribute.m_edgeEnergy;
                    break;
                case VisionSurfaceCriteria_Column::t2D_MergedCount:
                    result = blob_attribute.m_fMergedCount;
                    break;
                case VisionSurfaceCriteria_Column::t2D_BlobGV_AVR:
                    result = blob_attribute.m_fBlobGV_AVR;
                    break;
                case VisionSurfaceCriteria_Column::t2D_BlobGV_Min:
                    result = blob_attribute.m_fBlobGV_Min;
                    break;
                case VisionSurfaceCriteria_Column::t2D_BlobGV_Max:
                    result = blob_attribute.m_fBlobGV_Max;
                    break;
                case VisionSurfaceCriteria_Column::t3D_AvgDeltaHeight:
                    result = blob_attribute.m_averageDeltaHeight;
                    break;
                case VisionSurfaceCriteria_Column::t3D_KeyDeltaHeight:
                    result = blob_attribute.m_keyDeltaHeight;
                    break;
                case VisionSurfaceCriteria_Column::Aspect_ratio:
                    if (blob_attribute.m_width > 1.f)
                    {
                        result = blob_attribute.m_length / blob_attribute.m_width;
                    }
                    else
                    {
                        result = blob_attribute.m_length * fPixelsToMils;
                    }
                    break;
                case VisionSurfaceCriteria_Column::Dist_X_to_BodyCenter:
                    if (fImageCenX > blob_attribute.m_massCenter.m_x)
                    {
                        result = fabs(blob_info.m_roi.m_right - 1 - fImageCenX) * fPixelsToMils;
                    }
                    else
                    {
                        result = fabs(blob_info.m_roi.m_left - fImageCenX) * fPixelsToMils;
                    }
                    break;

                case VisionSurfaceCriteria_Column::Dist_Y_to_BodyCenter:
                    if (fImageCenY > blob_attribute.m_massCenter.m_y)
                    {
                        result = fabs(blob_info.m_roi.m_bottom - 1 - fImageCenY) * fPixelsToMils;
                    }
                    else
                    {
                        result = fabs(blob_info.m_roi.m_top - fImageCenY) * fPixelsToMils;
                    }
                    break;

                case VisionSurfaceCriteria_Column::Dist_to_Body:
                    result = fabs(blob_info.m_roi.m_top - fBodyTopY);
                    result = min(result, fabs(blob_info.m_roi.m_right - 1 - fBodyRightX));
                    result = min(result, fabs(blob_info.m_roi.m_bottom - 1 - fBodyBottomY));
                    result = min(result, fabs(blob_info.m_roi.m_left - fBodyLeftX));
                    result *= fPixelsToMils;
                    break;
            }
        }

        // For Display
        pDSO->fCenX = blob_attribute.m_massCenter.m_x;
        pDSO->fCenY = blob_attribute.m_massCenter.m_y;
        pDSO->rtROI = blob_info.m_roi;

        //  영훈 20151117 : 실제 Left Top에서부터의 좌표를 보여준다.
        pDSO->fObjectRealCenterX = (blob_attribute.m_massCenter.m_x - fBodyLeftX) * fPixelsToMils;
        pDSO->fObjectRealCenterY = (blob_attribute.m_massCenter.m_y - fBodyTopY) * fPixelsToMils_y;

        pDSO->fCenX0 = pDSO->fCenX;
        pDSO->fCenY0 = pDSO->fCenY;

        pDSO->arROI = blob_info.m_roi;

        pDSO->nArea = blob_info.m_area;
        pDSO->nLabel = blob_info.m_label;

        nValid++;
    }

    return true;
}

bool VisionInspectionSurface::GetDisplayAlignImage(CSurfaceItem& sItem, bool applyCombine, Ipvm::Image8u& o_grayImage)
{
    // UI 에서 사용하는 용도이다
    MakeSurfaceROI(sItem);

    long nCurFrameID = GetImageFrameIndex(0);

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP)
    {
        // 3D의 경우는 정확치는 않지만, 그냥 ZMap2D Image를 보여주자
        nCurFrameID = 0;
    }
    else if (applyCombine)
    {
        SurfacePrepairResult result(true);
        auto retValue = MakePrepairImage(sItem, result);
        o_grayImage = result.m_grayImage;

        return retValue;
    }

    if (GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_VMAP)
        nCurFrameID = 1;

    auto* image = getImageLotInsp().GetSafeImagePtr(nCurFrameID, GetCurVisionModule_Status());
    if (image == nullptr)
        return false;

    o_grayImage = *image;

    return true;
}

bool VisionInspectionSurface::MakePrepairImage(CSurfaceItem& sItem, SurfacePrepairResult& o_result)
{
    o_result.m_profiles.clear();

    // UI 에서 사용하는 용도이다

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP)
    {
        o_result.m_is3D = true;
        o_result.m_success = MakePrepairImage3D(sItem, o_result);
    }
    else
    {
        o_result.m_is3D = false;
        o_result.m_success = MakePrepairImage2D(sItem, o_result);
    }

    return o_result.m_success;
}

bool VisionInspectionSurface::MakeInspImage(
    CSurfaceItem& sItem, const SurfacePrepairResult& i_prepairResult, Ipvm::Image8u& o_thresholdImage)
{
    if (!i_prepairResult.m_success)
    {
        return false;
    }

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP)
    {
        return MakeInspImage3D(sItem, i_prepairResult, o_thresholdImage);
    }
    else
    {
        return MakeInspImage2D(sItem, i_prepairResult, o_thresholdImage);
    }
}

bool VisionInspectionSurface::MakePrepairImage2D(CSurfaceItem& sItem, SurfacePrepairResult& o_result)
{
    if (!getReusableMemory().GetInspByteImage(o_result.m_grayImage))
        return false;
    if (!getReusableMemory().GetInspByteImage(o_result.m_maskImage))
        return false;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_result.m_maskImage), 0, o_result.m_maskImage);

    // 3D 가 아닌 경우 2D 프레임을 이용하여 계산한다
    Ipvm::Image8u image = GetInspectionFrameImage();

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_VMAP) //SDY_VMap 이미지를 스케일링 해서 받아온다.
    {
        Ipvm::ImageProcessing::Scale(Ipvm::Image16u(getImageLotInsp().m_vmapImage), Ipvm::Rect32s(image), image);
    }
    else
    {
        image = GetInspectionFrameImage();
    }

    if (image.GetMem() == nullptr)
        return false;

    Ipvm::Image8u firstImage;

    firstImage = image;
    o_result.m_aux1GrayImage = m_surfacePara->m_ImageProcMangePara.m_aux1FrameID.getImage(false);
    o_result.m_aux2GrayImage = m_surfacePara->m_ImageProcMangePara.m_aux2FrameID.getImage(false);

    // Surface Combine 영상 만들기
    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &firstImage, false, m_surfacePara->m_ImageProcMangePara, o_result.m_grayImage))
    {
        return false;
    }

    if (!GenerateMask(
            o_result.isDetailSetupMode(), sItem, o_result.m_maskImage, o_result.m_grayImage, GetSurfaceItem()))
        return false;

    if (!getReusableMemory().GetInspByteImage(o_result.m_backgroundImage))
        return false;

    if (!MakeGray_BackgroundImage(sItem, o_result.m_grayImage, o_result.m_maskImage, o_result.m_backgroundImage))
    {
        return false;
    }

    Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);

    auto algoRoi = getScale().convert_BCUToPixel(sItem.AlgoPara.m_rtROI_BCU, imageCenter);

    if (o_result.isDetailSetupMode())
    {
        Ipvm::Image32s histogram;
        histogram.Create(512, 1);
        histogram.FillZero();

        auto* histogramData = histogram.GetMem();

        for (long y = algoRoi.m_top; y < algoRoi.m_bottom; y++)
        {
            auto* image_y = o_result.m_grayImage.GetMem(0, y);
            auto* background_Y = o_result.m_backgroundImage.GetMem(0, y);
            auto* mask_y = o_result.m_maskImage.GetMem(0, y);

            for (long x = algoRoi.m_left; x < algoRoi.m_right; x++)
            {
                if (!mask_y[x])
                    continue;

                histogramData[255L + image_y[x] - background_Y[x]]++;
            }
        }

        o_result.m_profiles.clear();
        long startIndex = 0;
        long endIndex = 511;
        for (long n = 0; n < 512; n++)
        {
            if (histogram.GetMem()[n] == 0)
                continue;
            startIndex = n;
            break;
        }

        for (long n = 511; n >= 0; n--)
        {
            if (histogram.GetMem()[n] == 0)
                continue;
            endIndex = n;
            break;
        }

        for (long n = startIndex; n <= endIndex; n++)
        {
            o_result.m_profiles.emplace_back(float(n - 255), float(histogram.GetMem()[n]));
        }
    }

    return true;
}

bool VisionInspectionSurface::MakePrepairImage3D(CSurfaceItem& sItem, SurfacePrepairResult& o_result)
{
    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    auto algoRoi = getScale().convert_BCUToPixel(sItem.AlgoPara.m_rtROI_BCU, imageCenter);

    if (!getReusableMemory().GetInspByteImage(o_result.m_grayImage))
        return false;
    if (!getReusableMemory().GetInspByteImage(o_result.m_maskImage))
        return false;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_result.m_maskImage), 0, o_result.m_maskImage);

    // 3D 일 경우 ZMAP을 이용하여 Background Image와 ThresholdImage를 만든다
    Ipvm::Image32r zmap = getImageLotInsp().m_zmapImage;

    if (!getReusableMemory().GetInspFloatImage(o_result.m_heightMap))
        return false;

    Ipvm::Image32r background;

    if (!getReusableMemory().GetInspFloatImage(background))
        return false;

    Ipvm::Image8u image;
    if (!GenerateMask(o_result.isDetailSetupMode(), sItem, o_result.m_maskImage, image, GetSurfaceItem()))
    {
        return false;
    }

    if (!MakeZMap_BackgroundImage(sItem, zmap, o_result.m_maskImage, algoRoi, background))
    {
        return false;
    }

    if (!MakeZMap_to_HeightZmap(zmap, background, o_result.m_heightMap))
    {
        return false;
    }

    if (!getReusableMemory().GetInspByteImage(o_result.m_backgroundImage))
        return false;

    if (!MakeHeightMap_to_GrayImage(o_result.m_heightMap, algoRoi, o_result.m_grayImage))
    {
        return false;
    }

    if (!MakeGray_BackgroundImage(sItem, o_result.m_grayImage, o_result.m_maskImage, o_result.m_backgroundImage))
    {
        return false;
    }

    if (o_result.isDetailSetupMode())
    {
        Ipvm::Image32s histogram;
        if (!getReusableMemory().GetLongImage(histogram, 20001, 1))
            return false;

        histogram.FillZero();

        long noiseCount = 0;

        auto* histogramData = histogram.GetMem();

        for (long y = algoRoi.m_top; y < algoRoi.m_bottom; y++)
        {
            auto* image_y = o_result.m_heightMap.GetMem(0, y);
            auto* mask_y = o_result.m_maskImage.GetMem(0, y);

            for (long x = algoRoi.m_left; x < algoRoi.m_right; x++)
            {
                if (!mask_y[x])
                    continue;
                if (image_y[x] == Ipvm::k_noiseValue32r)
                {
                    noiseCount++;
                    continue;
                }

                long value = long(max(-10000.f, min(10000.f, image_y[x] * 10.f))) + 10000;
                histogramData[value]++;
            }
        }

        o_result.m_profiles.clear();
        long startIndex = 0;
        long endIndex = 20000;
        for (long n = 0; n < 20001; n++)
        {
            if (histogram.GetMem()[n] == 0)
                continue;
            startIndex = n;
            break;
        }

        for (long n = 20000; n >= 0; n--)
        {
            if (histogram.GetMem()[n] == 0)
                continue;
            endIndex = n;
            break;
        }

        for (long n = startIndex; n <= endIndex; n++)
        {
            o_result.m_profiles.emplace_back(float(n - 10000) * 0.1f, float(histogram.GetMem()[n]));
        }

        //o_profiles->emplace_back(Ipvm::k_noiseValue32r, float(noiseCount));
    }

    return true;
}

bool VisionInspectionSurface::MakeInspImage2D(
    CSurfaceItem& sItem, const SurfacePrepairResult& i_prepairResult, Ipvm::Image8u& o_thresholdImage)
{
    if (!getReusableMemory().GetInspByteImage(o_thresholdImage))
        return false;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_thresholdImage), BACKGROUNDGV, o_thresholdImage);

    if (!MakeGray_ThresholdedImage(sItem, i_prepairResult.m_grayImage, i_prepairResult.m_backgroundImage,
            i_prepairResult.m_maskImage, o_thresholdImage))
    {
        return false;
    }

    return true;
}

bool VisionInspectionSurface::MakeInspImage3D(
    CSurfaceItem& sItem, const SurfacePrepairResult& i_prepairResult, Ipvm::Image8u& o_thresholdImage)
{
    if (!getReusableMemory().GetInspByteImage(o_thresholdImage))
        return false;

    if (!MakeHeightMap_to_ThresholdImage(
            sItem, i_prepairResult.m_heightMap, i_prepairResult.m_maskImage, o_thresholdImage))
    {
        return false;
    }

    return true;
}

bool VisionInspectionSurface::MakeGray_BackgroundImage(const CSurfaceItem& sItem, const Ipvm::Image8u& i_image,
    const Ipvm::Image8u& imageMask, Ipvm::Image8u& o_imageBackground)
{
    Ipvm::Point32r2 imageCenter(i_image.GetSizeX() * 0.5f, i_image.GetSizeY() * 0.5f);

    auto algoRoi = getScale().convert_BCUToPixel(sItem.AlgoPara.m_rtROI_BCU, imageCenter);

    if (sItem.AlgoPara.GetThresholdNormal())
    {
        // Normal Threshold일 경우 Background Image를
        // Dark와 Bright의 경계값으로 채워서
        // 이것을 이용해 Dark, Bright 판정을 할 수 있게 하자

        BYTE minContrast_bright = BYTE(min(255, max(1, sItem.AlgoPara.GetThresholdHigh())));
        BYTE minContrast_dark = BYTE(min(255, max(1, sItem.AlgoPara.GetThresholdLow())));

        BYTE threshold = BYTE((long(minContrast_dark) + minContrast_bright) / 2);

        if (Ipvm::ImageProcessing::Fill(algoRoi, threshold, o_imageBackground) != Ipvm::Status::e_ok)
        {
            return false;
        }

        return true;
    }

    // Background Image 계산속도 때문에 1/16 (X,Y=1/4) 로 줄인다음 계산하게 수정하였다

    int scale = 4;

    if (scale <= 1)
    {
        float f2DScale = getScale().umToPixelXY();

        long windowSizeX = long((sItem.AlgoPara.m_dbackground_window_size_x * f2DScale) + .5f);
        long windowSizeY = long((sItem.AlgoPara.m_dbackground_window_size_y * f2DScale) + .5f);

        Ipvm::Image32s imageIntengral;
        Ipvm::Image32s maskIntengral;

        if (!getReusableMemory().GetInspLongImage(imageIntengral))
            return false;
        if (!getReusableMemory().GetInspLongImage(maskIntengral))
            return false;

        if (Ipvm::ImageProcessing::IntegralInclusive(i_image, imageMask, algoRoi, imageIntengral, maskIntengral)
            != Ipvm::Status::e_ok)
        {
            return false;
        }

        long win_size_x = long(windowSizeX + 0.5f);
        long win_size_y = long(windowSizeY + 0.5f);

        if (Ipvm::ImageProcessing::MakeBackgroundImage(
                imageIntengral, maskIntengral, algoRoi, Ipvm::Size32s2(win_size_x, win_size_y), o_imageBackground)
            != Ipvm::Status::e_ok)
        {
            return false;
        }

        //Ipvm::Image8u(o_imageBackground, roi).SaveBmp(_T("D:\\original.bmp"));
        return true;
    }

    auto roi_source = algoRoi;

    if ((Ipvm::Rect32s(i_image) & roi_source) != roi_source)
    {
        return false;
    }

    int scaleSizeX = roi_source.Width() / scale;
    int scaleSizeY = roi_source.Height() / scale;
    Ipvm::Rect32s roi_scale(0, 0, scaleSizeX, scaleSizeY);

    Ipvm::Image8u scale_image;
    Ipvm::Image8u scale_mask;
    Ipvm::Image8u scale_background;
    Ipvm::Image32s imageIntengral;
    Ipvm::Image32s maskIntengral;

    if (!getReusableMemory().GetByteImage(scale_image, scaleSizeX, scaleSizeY))
        return false;
    if (!getReusableMemory().GetByteImage(scale_mask, scaleSizeX, scaleSizeY))
        return false;
    if (!getReusableMemory().GetByteImage(scale_background, scaleSizeX, scaleSizeY))
        return false;
    if (!getReusableMemory().GetLongImage(imageIntengral, scaleSizeX, scaleSizeY))
        return false;
    if (!getReusableMemory().GetLongImage(maskIntengral, scaleSizeX, scaleSizeY))
        return false;

    Ipvm::ImageProcessing::ResizeLinearInterpolation(Ipvm::Image8u(i_image, roi_source), scale_image);
    Ipvm::ImageProcessing::ResizeNearestInterpolation(Ipvm::Image8u(imageMask, roi_source), scale_mask);

    float um2px_scale = getScale().umToPixelXY() / scale;

    long windowSizeX = long((sItem.AlgoPara.m_dbackground_window_size_x * um2px_scale) + .5f);
    long windowSizeY = long((sItem.AlgoPara.m_dbackground_window_size_y * um2px_scale) + .5f);

    if (Ipvm::ImageProcessing::IntegralInclusive(scale_image, scale_mask, roi_scale, imageIntengral, maskIntengral)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    long win_size_x = long(windowSizeX + 0.5f);
    long win_size_y = long(windowSizeY + 0.5f);

    win_size_x = (long)min(roi_scale.Width(), win_size_x);
    win_size_y = (long)min(roi_scale.Height(), win_size_y);

    if (Ipvm::ImageProcessing::MakeBackgroundImage(
            imageIntengral, maskIntengral, roi_scale, Ipvm::Size32s2(win_size_x, win_size_y), scale_background)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    //Ipvm::Image8u(scale_background, roi_scale).SaveBmp(_T("D:\\scale.bmp"));

    Ipvm::Image8u refBufferROI(o_imageBackground, roi_source);
    Ipvm::ImageProcessing::ResizeLinearInterpolation(scale_background, refBufferROI);

    return true;
}

bool VisionInspectionSurface::MakeGray_ThresholdedImage(const CSurfaceItem& sItem, const Ipvm::Image8u& i_image,
    const Ipvm::Image8u& i_imageBackground, const Ipvm::Image8u& i_maskImage, Ipvm::Image8u& o_imageDst)
{
    Ipvm::Point32r2 imageCenter(i_image.GetSizeX() * 0.5f, i_image.GetSizeY() * 0.5f);

    auto algoRoi = getScale().convert_BCUToPixel(sItem.AlgoPara.m_rtROI_BCU, imageCenter);

    Ipvm::Image8u imageTemp1;
    Ipvm::Image8u imageTemp2;
    Ipvm::Image8u imageTemp3;

    if (!getReusableMemory().GetInspByteImage(imageTemp1))
        return false;
    if (!getReusableMemory().GetInspByteImage(imageTemp2))
        return false;
    if (!getReusableMemory().GetInspByteImage(imageTemp3))
        return false;

    if (m_surfacePara->m_SurfaceItem.AlgoPara.GetThresholdNormal() == FALSE)
    {
        BYTE minContrast_bright = BYTE(min(255, max(1, sItem.AlgoPara.GetThresholdBright(false))));
        BYTE minContrast_dark = BYTE(min(255, max(1, sItem.AlgoPara.GetThresholdDark(false))));

        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
            && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP)
        {
            minContrast_bright = BYTE(min(255, max(1, sItem.AlgoPara.GetThresholdBright(TRUE))));
            minContrast_dark = BYTE(min(255, max(1, sItem.AlgoPara.GetThresholdDark(TRUE))));
        }

        int lutDark[256];
        int lutLevel[256];
        for (long n = 0; n < 256; n++)
        {
            lutLevel[n] = (BYTE)n;
            lutDark[n] = BACKGROUNDGV;
        }

        for (long n = minContrast_dark; n < 256; n++)
            lutDark[n] = 0;

        Ipvm::ImageProcessing::Subtract(i_imageBackground, i_image, algoRoi, 0, imageTemp1);
        Ipvm::ImageProcessing::ApplyLUT(algoRoi, lutDark, lutLevel, 256, imageTemp1);

        Ipvm::ImageProcessing::Subtract(i_image, i_imageBackground, algoRoi, 0, imageTemp2);
        Ipvm::ImageProcessing::BinarizeGreater(imageTemp2, algoRoi, minContrast_bright, o_imageDst);
        Ipvm::ImageProcessing::Add(imageTemp1, algoRoi, 0, o_imageDst);
    }
    else
    {
        BYTE minContrast_High = BYTE(min(255, max(1, sItem.AlgoPara.GetThresholdHigh())));
        BYTE minContrast_Low = BYTE(min(255, max(1, sItem.AlgoPara.GetThresholdLow())));

        CippModules::Binarize(i_image, imageTemp1, algoRoi, minContrast_Low, FALSE);

        for (int y = 0; y < imageTemp1.GetSizeY(); y++)
        {
            BYTE* bImage = imageTemp1.GetMem(0, y);

            for (int x = 0; x < imageTemp1.GetSizeX(); x++)
            {
                if (bImage[x] != 0)
                    bImage[x] = BACKGROUNDGV;
            }
        }

        CippModules::Binarize(i_image, o_imageDst, algoRoi, minContrast_High, FALSE);
        Ipvm::ImageProcessing::Add(imageTemp1, algoRoi, 0, o_imageDst);
    }

    if (!CippModules::SetInvalidPattern_with_constant(o_imageDst, i_maskImage, algoRoi, INGNOREPIXELGV))
    {
        return false;
    }

    return true;
}

bool VisionInspectionSurface::MakeZMap_BackgroundImage(const CSurfaceItem& sItem, const Ipvm::Image32r& i_zmap,
    const Ipvm::Image8u& imageMask, const Ipvm::Rect32s& roi, Ipvm::Image32r& o_imageBackground)
{
    float f2DScale = getScale().umToPixelXY();

    long windowSizeX = long((sItem.AlgoPara.m_dbackground_window_size_x * f2DScale) + .5f);
    long windowSizeY = long((sItem.AlgoPara.m_dbackground_window_size_y * f2DScale) + .5f);

    Ipvm::Image64r intImage;
    Ipvm::Image32s sumCountImage;

    intImage.Create(i_zmap.GetSizeX(), i_zmap.GetSizeY());

    if (!getReusableMemory().GetInspLongImage(sumCountImage))
        return false;

    Ipvm::Image8u mask_ignoreNoise;
    if (!getReusableMemory().GetInspByteImage(mask_ignoreNoise))
        return false;

    Ipvm::ImageProcessing::Copy(imageMask, roi, mask_ignoreNoise);

    for (long y = roi.m_top; y < roi.m_bottom; y++)
    {
        auto* zmap_y = i_zmap.GetMem(0, y);
        auto* mask_y = mask_ignoreNoise.GetMem(0, y);
        for (long x = roi.m_left; x < roi.m_right; x++)
        {
            if (zmap_y[x] == Ipvm::k_noiseValue32r)
                mask_y[x] = 0;
        }
    }

    if (Ipvm::ImageProcessing::IntegralInclusive(i_zmap, mask_ignoreNoise, roi, intImage, sumCountImage)
        != Ipvm::Status::e_ok)
        return false;

    Ipvm::Size32s2 windowSize(windowSizeX, windowSizeY);

    if (Ipvm::ImageProcessing::MakeBackgroundImageWithNoise(
            intImage, sumCountImage, roi, windowSize, Ipvm::k_noiseValue32r, o_imageBackground)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool VisionInspectionSurface::MakeZMap_to_HeightZmap(
    const Ipvm::Image32r& i_zmap, const Ipvm::Image32r& i_backgroundZMap, Ipvm::Image32r& o_hightMap)
{
    for (long y = 0; y < i_zmap.GetSizeY(); y++)
    {
        auto* zmap_y = i_zmap.GetMem(0, y);
        auto* background_y = i_backgroundZMap.GetMem(0, y);
        auto* height_y = o_hightMap.GetMem(0, y);

        for (long x = 0; x < i_zmap.GetSizeX(); x++)
        {
            if (zmap_y[x] == Ipvm::k_noiseValue32r)
            {
                height_y[x] = Ipvm::k_noiseValue32r;
            }
            else
            {
                height_y[x] = zmap_y[x] - background_y[x];
            }
        }
    }

    return true;
}

bool VisionInspectionSurface::MakeHeightMap_to_ThresholdImage(const CSurfaceItem& sItem,
    const Ipvm::Image32r& i_hightMap, const Ipvm::Image8u& i_maskImage, Ipvm::Image8u& o_thresholdImage)
{
    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    auto algoRoi = getScale().convert_BCUToPixel(sItem.AlgoPara.m_rtROI_BCU, imageCenter);

    auto inspROI = algoRoi;
    inspROI &= Ipvm::Rect32s(i_hightMap);

    float passMax = max(0.f, sItem.AlgoPara.GetThresholdBright(true));
    float passMin = min(0.f, -sItem.AlgoPara.GetThresholdDark(true));

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_thresholdImage), BACKGROUNDGV, o_thresholdImage);

    for (long y = inspROI.m_top; y < inspROI.m_bottom; y++)
    {
        auto* height_y = i_hightMap.GetMem(0, y);
        auto* threshold_y = o_thresholdImage.GetMem(0, y);

        for (long x = inspROI.m_left; x < inspROI.m_right; x++)
        {
            if (height_y[x] == Ipvm::k_noiseValue32r)
                continue;

            if (height_y[x] < passMin)
            {
                threshold_y[x] = DARKOBJECTPIXELGV;
            }
            else if (height_y[x] > passMax)
            {
                threshold_y[x] = BRIGHTOBJECTPIXELGV;
            }
        }
    }

    if (!CippModules::SetInvalidPattern_with_constant(o_thresholdImage, i_maskImage, inspROI, INGNOREPIXELGV))
    {
        return false;
    }

    return true;
}

bool VisionInspectionSurface::MakeHeightMap_to_GrayImage(
    const Ipvm::Image32r& i_hightMap, const Ipvm::Rect32s& roi, Ipvm::Image8u& o_grayImage)
{
    float min_height = getImageLotInsp().m_heightRangeMin;
    float max_height = getImageLotInsp().m_heightRangeMax;

    Ipvm::ImageProcessing::Fill(roi, 0, o_grayImage);
    Ipvm::ImageProcessing::ScaleInRangeWithNoise(
        i_hightMap, roi, Ipvm::k_noiseValue32r, min_height, max_height, 0, 1, 255, o_grayImage);

    return true;
}

void VisionInspectionSurface::SetCustomROI_SurfaceBitmapMask()
{
    CSurfaceCustomROI* ROIInfo = &m_surfacePara->m_SurfaceCutemRoi;

    auto& supportMask = m_result.m_supportMask;
    for (long n = 0; n < supportMask.count(); n++)
    {
        if (supportMask.GetBitmapMaskName(n) == m_strModuleName)
        {
            // 자기 자신것은 추가하지 말자
            continue;
        }

        ROIInfo->CustomROI_SetImage(supportMask.GetBitmapMaskName(n), supportMask.Get(n));
    }
}

void VisionInspectionSurface::MakeSurfaceROI(CSurfaceItem& ItemSpec)
{
    auto& Algo = ItemSpec.AlgoPara;

    Ipvm::Rect32s rtROI(getImageLotInsp().GetImageSizeX(), 0, 0, getImageLotInsp().GetImageSizeY());

    //==================================
    // Mask 영역의 최대 크기를 계산한다
    //==================================
    float bodySizeX_px = 0.f;
    float bodySizeY_px = 0.f;
    GetInspSize(true, bodySizeX_px, bodySizeY_px);

    int nBodySizeX = (int)(bodySizeX_px + 0.5f);
    int nBodySizeY = (int)(bodySizeY_px + 0.5f);
    int nLeft = long(0.5f * (getImageLotInsp().GetImageSizeX() - nBodySizeX) + 0.5f);
    int nTop = long(0.5f * (getImageLotInsp().GetImageSizeY() - nBodySizeY) + 0.5f);
    int nRight = nLeft + nBodySizeX;
    int nBottom = nTop + nBodySizeY;

    long offsetL = (long)(getScale().convert_umToPixelX(Algo.m_offsetLeft_um) + 0.5f);
    long offsetR = (long)(getScale().convert_umToPixelX(Algo.m_offsetRight_um) + 0.5f);
    long offsetT = (long)(getScale().convert_umToPixelY(Algo.m_offsetTop_um) + 0.5f);
    long offsetB = (long)(getScale().convert_umToPixelY(Algo.m_offsetBottom_um) + 0.5f);

    rtROI.SetRect(nLeft + offsetL, nTop + offsetT, nRight - offsetR, nBottom - offsetB);
    rtROI.NormalizeRect();
    rtROI &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    Algo.m_rtROI_BCU = getScale().convert_PixelToBCU(rtROI, imageCenter);
}

long VisionInspectionSurface::MakeDetectRect(const Ipvm::Image8u& image, const long lThreshX, const long lThreshY,
    Ipvm::Rect32s i_rtROI, Ipvm::Rect32s& o_rtDetectRect)
{
    long lWidth = i_rtROI.Width();
    long lHeight = i_rtROI.Height();

    i_rtROI &= Ipvm::Rect32s(image);

    std::vector<long> veclProfileX;
    veclProfileX.resize(lWidth);
    long* plProfileX = &veclProfileX[0];

    std::vector<long> veclProfileY;
    veclProfileY.resize(lHeight);
    long* plProfileY = &veclProfileY[0];

    //{{Profile 계산
    memset(plProfileX, 0, lWidth * sizeof(long));
    memset(plProfileY, 0, lHeight * sizeof(long));

    long lTop = i_rtROI.m_top;
    long lLeft = i_rtROI.m_left;
    long lBottom = i_rtROI.m_bottom;
    long lRight = i_rtROI.m_right;

    long lCntX, lCntY = 0;
    long x, y;

    for (y = lTop; y < lBottom; y++, lCntY++)
    {
        auto* image_y = image.GetMem(0, y);

        lCntX = 0;
        for (x = lLeft; x < lRight; x++, lCntX++)
        {
            plProfileX[lCntX] += (long)(image_y[x]);
            plProfileY[lCntY] += (long)(image_y[x]);
        }
        plProfileY[lCntY] /= lWidth;
    }
    for (x = 0; x < lWidth; x++)
        plProfileX[x] /= lHeight;
    //}}

    //{{각 지점 추출
    o_rtDetectRect = Ipvm::Rect32s(0, 0, 0, 0);
    for (x = 0; x < lWidth; x++)
    {
        if (plProfileX[x] > lThreshX)
        {
            o_rtDetectRect.m_left = x + lLeft;
            break;
        }
    }
    for (x = lWidth - 1; x >= 0; x--)
    {
        if (plProfileX[x] > lThreshX)
        {
            o_rtDetectRect.m_right = x + lLeft;
            break;
        }
    }
    for (y = 0; y < lHeight; y++)
    {
        if (plProfileY[y] > lThreshY)
        {
            o_rtDetectRect.m_top = y + lTop;
            break;
        }
    }
    for (y = lHeight - 1; y >= 0; y--)
    {
        if (plProfileY[y] > lThreshY)
        {
            o_rtDetectRect.m_bottom = y + lTop;
            break;
        }
    }
    if (o_rtDetectRect.m_left == 0 || o_rtDetectRect.m_top == 0 || o_rtDetectRect.m_right == 0
        || o_rtDetectRect.m_bottom == 0)
        return -1;

    return 0;
}

bool VisionInspectionSurface::GenerateMask(const bool detailSetupMode, CSurfaceItem& ItemSpec, Ipvm::Image8u& imageMask,
    Ipvm::Image8u& image, CSurfaceItem* pSurfaceItem)
{
    UNREFERENCED_PARAMETER(pSurfaceItem);

    Ipvm::Image8u imageBuffer;
    if (!getReusableMemory().GetInspByteImage(imageBuffer))
        return false;

    MakeSurfaceROI(ItemSpec);
    m_result.InitnResults(m_surfacePara->m_SurfaceItem);

    // 영훈 20130912 : Clearimage도 Surface가 많아지면 시간이 많이 걸리므로 bodyalign 영역정도만 초기화해준도록 하자.

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    float bodySizeX_px = 0.f;
    float bodySizeY_px = 0.f;
    GetInspSize(true, bodySizeX_px, bodySizeY_px);

    int nBodySizeX = (int)(bodySizeX_px + 0.5f);
    int nBodySizeY = (int)(bodySizeY_px + 0.5f);
    int nLeft = (int)(0.5f * (getImageLotInsp().GetImageSizeX() - nBodySizeX) + 0.5f);
    int nTop = (int)(0.5f * (getImageLotInsp().GetImageSizeY() - nBodySizeY) + 0.5f);
    int nRight = nLeft + nBodySizeX;
    int nBottom = nTop + nBodySizeY;
    Ipvm::Rect32s rtROI;

    auto& algoPara = ItemSpec.AlgoPara;

    long offsetL = (long)(getScale().convert_umToPixelX(algoPara.m_offsetLeft_um) + 0.5f);
    long offsetR = (long)(getScale().convert_umToPixelX(algoPara.m_offsetRight_um) + 0.5f);
    long offsetT = (long)(getScale().convert_umToPixelY(algoPara.m_offsetTop_um) + 0.5f);
    long offsetB = (long)(getScale().convert_umToPixelY(algoPara.m_offsetBottom_um) + 0.5f);

    rtROI.SetRect(nLeft + offsetL, nTop + offsetT, nRight - offsetR, nBottom - offsetB);
    rtROI.NormalizeRect();
    rtROI &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());

    PI_RECT arBody(
        m_EdgeAlignResult->fptLT, m_EdgeAlignResult->fptRT, m_EdgeAlignResult->fptLB, m_EdgeAlignResult->fptRB);
    Ipvm::Rect32s rtBody(rtROI);
    long nOffsetX, nOffsetY;
    nOffsetX = (long)(rtBody.CenterPoint().m_x - (getImageLotInsp().GetImageSizeX() * 0.5f));
    nOffsetY = (long)(rtBody.CenterPoint().m_y - (getImageLotInsp().GetImageSizeY() * 0.5f));
    Ipvm::Rect32s rtInitBinary = rtBody;
    rtInitBinary.InflateRect(-offsetL, -offsetT, -offsetR, -offsetB);
    rtInitBinary.OffsetRect(-nOffsetX, -nOffsetY);
    Ipvm::Rect32s rtMask = rtInitBinary;
    if (detailSetupMode)
        rtMask = Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());
    else
        rtMask.InflateRect(10, 10); //Mask는 좀 여유있게 배경을 지우자
    rtMask &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());
    rtInitBinary &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageMask), 0, imageMask);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageMask), 0, imageBuffer);

    Ipvm::TimeCheck Time;

    auto visionDebugInfos = m_visionUnit.GetVisionDebugInfos();
    auto maskOrder = algoPara.m_maskOrder;

    // 강제로 마지막에 Global Surface Ignore Mask 을 뺀다
    Para_MaskOrder::MaskInfo lastMask;
    lastMask.m_strName = GetSurfaceMaskNameFromLayerName(GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME);
    lastMask.m_makeType = 0;
    lastMask.m_validate = FALSE;
    lastMask.m_offset_um = Ipvm::Point32r2(0.f, 0.f);
    lastMask.m_dilateInUm = 0.f;
    maskOrder.Add(lastMask);

    float um2px = getScale().umToPixelXY();

    for (int orderIndex = 0; orderIndex < maskOrder.GetCount(); orderIndex++)
    {
        auto& maskInfo = maskOrder.GetAt(orderIndex);
        CString strName = maskInfo.m_strName;
        BOOL bValidate = maskInfo.m_validate;
        int nMaskType = maskInfo.m_makeType;
        short nDilate = short(maskInfo.m_dilateInUm * um2px + 0.5f);

        switch (nMaskType)
        {
            case 0: // Custom ROI
            {
                auto inspROI = getScale().convert_BCUToPixel(ItemSpec.AlgoPara.m_rtROI_BCU, imageCenter);

                Ipvm::Image8u layerMask;
                bool findLayerMask = GetSurfaceLayerImage(strName, layerMask);

                if (findLayerMask)
                {
                    Ipvm::Rect32s ignoreROI = inspROI;
                    ignoreROI.InflateRect(1, 1);
                    ignoreROI = ignoreROI & Ipvm::Rect32s(imageBuffer);

                    // Dilate 시 외곽 영역이 영역을 받아서 일단 외곽을 Clear 해줘야 하는 문제가 있다
                    Ipvm::ImageProcessing::Fill(ignoreROI, 0, imageBuffer);
                    Ipvm::ImageProcessing::Copy(layerMask, inspROI, imageBuffer);

                    if (nDilate > 0)
                    {
                        Ipvm::ImageProcessing::MorphDilateIter(inspROI, 3, 3, nDilate, imageBuffer);
                    }
                    else if (nDilate < 0)
                    {
                        Ipvm::ImageProcessing::MorphErodeIter(inspROI, 3, 3, abs(nDilate), imageBuffer);
                    }

                    if (bValidate)
                    {
                        Ipvm::ImageProcessing::BitwiseOr(imageBuffer, inspROI, imageMask);
                    }
                    else
                    {
                        Ipvm::ImageProcessing::BitwiseNot(inspROI, imageBuffer);
                        Ipvm::ImageProcessing::BitwiseAnd(imageBuffer, inspROI, imageMask);
                    }

                    if (strName != GetSurfaceMaskNameFromLayerName(GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME))
                    {
                        m_result.m_supportMask.Set(strName, layerMask);
                    }

                    SetCustomROI_SurfaceBitmapMask();
                }
                else if (strName == _T("Underfill Fillet Mask Image"))
                {
                    auto* visionDebug = FindVisionDebugInfo(visionDebugInfos, strName);
                    if (visionDebug)
                    {
                        auto* maskImage = (Ipvm::Image8u*)visionDebug->pData;

                        Ipvm::ImageProcessing::Copy(*maskImage, Ipvm::Rect32s(*maskImage), imageBuffer);

                        Ipvm::ImageProcessing::BitwiseNot(Ipvm::Rect32s(imageBuffer), imageBuffer);

                        if (bValidate)
                        {
                            Ipvm::ImageProcessing::BitwiseOr(imageBuffer, Ipvm::Rect32s(imageBuffer), imageMask);

                            if (nDilate > 0)
                            {
                                Ipvm::ImageProcessing::MorphDilateIter(
                                    Ipvm::Rect32s(imageMask), 3, 3, nDilate, imageMask);
                            }
                            else if (nDilate < 0)
                            {
                                Ipvm::ImageProcessing::MorphErodeIter(
                                    Ipvm::Rect32s(imageMask), 3, 3, abs(nDilate), imageMask);
                            }
                        }
                        else
                        {
                            Ipvm::ImageProcessing::BitwiseAnd(imageBuffer, Ipvm::Rect32s(imageBuffer), imageMask);

                            if (nDilate < 0)
                            {
                                Ipvm::ImageProcessing::MorphDilateIter(
                                    Ipvm::Rect32s(imageMask), 3, 3, abs(nDilate), imageMask);
                            }
                            else if (nDilate > 0)
                            {
                                Ipvm::ImageProcessing::MorphErodeIter(
                                    Ipvm::Rect32s(imageMask), 3, 3, nDilate, imageMask);
                            }
                        }

                        m_result.m_supportMask.Set(strName, imageMask);

                        SetCustomROI_SurfaceBitmapMask();
                    }
                }

                // 영훈 [SurfaceMask] 20130813 : Round Pad로 그리는 Mask는 이쪽으로 진입하면 안된다.. 아 젠장 이거때매 디버깅만 3시간..ㅠㅠ
                if (strName != _T("Underfill Fillet Mask Image") && !findLayerMask)
                {
                    GenerateMask_UseCustomROI(algoPara, strName, orderIndex, bValidate, imageMask, imageBuffer);
                }
                break;
            }
            case 1: // Legacy ROI
            {
                const auto* pOffsetArray_um = &maskInfo.m_pointOffsets_um[0];

                int nSerach = algoPara.m_ignore.FindIndex(strName);

                if (nSerach < 0)
                    break; // 그런 Ignore 없음

                BYTE bMaskValue = 0xff;
                BYTE bBackValue = 0x00;

                if (!bValidate)
                {
                    bMaskValue = 0x00;
                    bBackValue = 0xff;
                }

                Ipvm::Rect32s rtMaskArea;

                std::vector<Ipvm::Point32s2> vecptPolygon;
                vecptPolygon.resize(algoPara.m_ignore.GetAt(nSerach).vecptPolygon_BCU.size());

                if (algoPara.m_ignore.GetAt(nSerach).nPolygonType == 2)
                {
                    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
                        && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP)
                    {
                        // 3D에서는 MASK 가 있어야 3D MAP를 이용하여 2D 이미지를 만들어 낼 수 있는데
                        // MASK 를 2D 이미지로 계산하는 코드이므로 순서상으로 계산이 불가하다

                        ASSERT(!_T("3D Surface 검사에서는 동적으로 MASK를 만들어 낼 수 없다.."));
                        break;
                    }

                    int searchMask = maskOrder.FindIndex(strName);
                    if (searchMask < 0)
                        break; // 그런 Ignore 없음

                    Ipvm::Rect32s rtDetectROI;
                    Ipvm::Rect32s rtMaskROI = getScale().convert_BCUToPixel(
                        Ipvm::Rect32r(algoPara.m_ignore.GetAt(nSerach).vecptPolygon_BCU[0],
                            algoPara.m_ignore.GetAt(nSerach).vecptPolygon_BCU[1]),
                        imageCenter);
                    int nTreshX = maskOrder.GetAt(searchMask).m_threshDRX;
                    int nTreshY = maskOrder.GetAt(searchMask).m_threshDRY;
                    MakeDetectRect(image, nTreshX, nTreshY, rtMaskROI, rtDetectROI);

                    auto offset0 = getScale().convert_umToPixel(maskOrder.GetAt(searchMask).m_pointOffsets_um[0]);
                    auto offset1 = getScale().convert_umToPixel(maskOrder.GetAt(searchMask).m_pointOffsets_um[1]);

                    vecptPolygon[0] = rtDetectROI.TopLeft() + offset0;
                    vecptPolygon[1] = rtDetectROI.BottomRight() + offset1;
                }
                else
                {
                    for (int i = 0; i < (long)(vecptPolygon.size()); i++)
                    {
                        vecptPolygon[i] = getScale().convert_BCUToPixel(
                            algoPara.m_ignore.GetAt(nSerach).vecptPolygon_BCU[i], imageCenter);
                    }
                }

                GenerateMask_SetMask(algoPara.m_ignore.GetAt(nSerach).nPolygonType, &vecptPolygon[0], nOffsetX,
                    nOffsetY, pOffsetArray_um, bMaskValue, bBackValue, imageBuffer, rtMaskArea);

                if (bValidate)
                {
                    Ipvm::ImageProcessing::BitwiseOr(imageBuffer, rtMaskArea, imageMask);
                }
                else
                {
                    Ipvm::ImageProcessing::BitwiseAnd(imageBuffer, rtMaskArea, imageMask);
                }

                break;
            }
        }
        strName.Empty();
    }

    auto algoRoi = getScale().convert_BCUToPixel(algoPara.m_rtROI_BCU, imageCenter);

    // 막판 정리
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageBuffer), 0, imageBuffer);
    Ipvm::ImageProcessing::Fill(algoRoi, 255, imageBuffer);
    Ipvm::ImageProcessing::BitwiseAnd(imageBuffer, Ipvm::Rect32s(imageBuffer), imageMask);

    return true;
}

void VisionInspectionSurface::InitEdgeResult()
{
    m_bEdgeAlignValid = FALSE;

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *m_EdgeAlignResult = *(VisionAlignResult*)pData;
        m_bEdgeAlignValid = TRUE;
    }
}

long VisionInspectionSurface::GenerateMask_UseCustomROI(const CSurfaceAlgoPara& AlgoPara, LPCTSTR szROIName,
    int nMaskOrder, BOOL bValidate, Ipvm::Image8u& imageMask, Ipvm::Image8u& imageBuffer)
{
    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    auto algoRoi = getScale().convert_BCUToPixel(AlgoPara.m_rtROI_BCU, imageCenter);

    const auto* SurfaceCustomROIItem = m_surfacePara->m_SurfaceCutemRoi.CustomROI_Search(szROIName);
    if (SurfaceCustomROIItem == nullptr)
        return -1;

    // 이상한 마스크로 뭔가 하려고 하면 Invalid
    if (SurfaceCustomROIItem->nType < 0)
        return -1;

    long nReturn = 0;

    BYTE bMaskValue = 0xff;
    BYTE bBackValue = 0x00;

    if (!bValidate)
    {
        bMaskValue = 0x00;
        bBackValue = 0xff;
    }

    const auto maskOffset = getScale().convert_umToPixel(AlgoPara.m_maskOrder.GetAt(nMaskOrder).m_offset_um);

    switch (SurfaceCustomROIItem->nType)
    {
        case 2:
            if (SurfaceCustomROIItem->m_image.GetSizeX() == imageMask.GetSizeX()
                && SurfaceCustomROIItem->m_image.GetSizeY() == imageMask.GetSizeY())
            {
                if (bValidate)
                {
                    Ipvm::ImageProcessing::Add(imageMask, SurfaceCustomROIItem->m_image, algoRoi, 0, imageMask);
                }
                else
                {
                    Ipvm::ImageProcessing::Subtract(imageMask, SurfaceCustomROIItem->m_image, algoRoi, 0, imageMask);
                }
            }
            else
            {
                // 이미지 정보가 맞지 않음
                nReturn = -1;
            }
            break;
        default:
        {
            const auto* pOffsetArray_um = &AlgoPara.m_maskOrder.GetAt(nMaskOrder).m_pointOffsets_um[0];

            long nROINum = (long)SurfaceCustomROIItem->vecShape.size();
            long nType = SurfaceCustomROIItem->nType;
            CString strROIName = szROIName;

            for (long nROI = 0; nROI < nROINum; nROI++)
            {
                Ipvm::Rect32s rtMaskArea;
                //kircheis_HX Surf //여기 수정 필요
                GenerateMask_SetMask(nType, &SurfaceCustomROIItem->vecShape[nROI].vecptPos[0], maskOffset.m_x,
                    maskOffset.m_y, pOffsetArray_um, bMaskValue, bBackValue, imageBuffer, rtMaskArea);

                if (bValidate)
                {
                    Ipvm::ImageProcessing::BitwiseOr(imageBuffer, rtMaskArea, imageMask);
                }
                else
                {
                    Ipvm::ImageProcessing::BitwiseAnd(imageBuffer, rtMaskArea, imageMask);
                }
            }

            strROIName.Empty();
            break;
        }
    }

    return nReturn;
}

long VisionInspectionSurface::GenerateMask_SetMask(int nType, const Ipvm::Point32s2* pPointArray, long nOffsetX,
    long nOffsetY, const Ipvm::Point32r2* pOffsetArray_um, BYTE bMaskValue, BYTE bBackValue, Ipvm::Image8u& imageBuffer,
    Ipvm::Rect32s& rtMaskArea)
{
    if (nType == 0 || nType == 1 || nType == 2)
    {
        if (pOffsetArray_um == nullptr || pPointArray == nullptr)
            return 0;

        auto offset0 = getScale().convert_umToPixel(pOffsetArray_um[0]);
        auto offset1 = getScale().convert_umToPixel(pOffsetArray_um[1]);

        // Rectangle, Ellpise, Detect Rect
        rtMaskArea.m_left = pPointArray[0].m_x + nOffsetX + offset0.m_x;
        rtMaskArea.m_top = pPointArray[0].m_y + nOffsetY + offset0.m_y;
        rtMaskArea.m_right = pPointArray[1].m_x + nOffsetX + offset1.m_x;
        rtMaskArea.m_bottom = pPointArray[1].m_y + nOffsetY + offset1.m_y;

        if (nType == 0 || nType == 2)
        {
            Ipvm::ImageProcessing::Fill(rtMaskArea, bMaskValue, imageBuffer);
        }
        else
        {
            // Ellpise 형태
            Ipvm::ImageProcessing::Fill(rtMaskArea, bBackValue, imageBuffer);

            Ipvm::EllipseEq32r ellipse;
            ellipse.m_x = (rtMaskArea.m_left + rtMaskArea.m_right - 1) * 0.5f;
            ellipse.m_y = (rtMaskArea.m_top + rtMaskArea.m_bottom - 1) * 0.5f;
            ellipse.m_xradius = rtMaskArea.Width() * 0.5f;
            ellipse.m_yradius = rtMaskArea.Height() * 0.5f;

            Ipvm::ImageProcessing::Fill(ellipse, bMaskValue, imageBuffer);
        }

        return 0;
    }

    // Polygon 형태
    if (pPointArray == nullptr)
        return 0;

    Ipvm::Polygon32r polygon;
    polygon.SetVertexNum(nType);

    for (int nPT = 0; nPT < polygon.GetVertexNum(); nPT++)
    {
        auto offset = getScale().convert_umToPixel(pOffsetArray_um[nPT]);
        auto& point = polygon.GetVertices()[nPT];
        point.m_x = float(pPointArray[nPT].m_x + nOffsetX + offset.m_x);
        point.m_y = float(pPointArray[nPT].m_y + nOffsetY + offset.m_y);
    }

    // Polygon 형태

    rtMaskArea.SetRect(getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY(), 0, 0);

    for (int nPT = 0; nPT < polygon.GetVertexNum(); nPT++)
    {
        float x = polygon.GetVertices()[nPT].m_x;
        float y = polygon.GetVertices()[nPT].m_y;

        if (x < rtMaskArea.m_left)
        {
            rtMaskArea.m_left = long(x);
        }

        if (x + 1 > rtMaskArea.m_right)
        {
            rtMaskArea.m_right = long(x + 1.5f);
        }

        if (y < rtMaskArea.m_top)
        {
            rtMaskArea.m_top = long(y);
        }

        if (y + 1 > rtMaskArea.m_bottom)
        {
            rtMaskArea.m_bottom = long(y + 1.f);
        }
    }

    Ipvm::ImageProcessing::Fill(rtMaskArea, bBackValue, imageBuffer);
    Ipvm::ImageProcessing::Fill(polygon, bMaskValue, imageBuffer);

    return 0;
}

long VisionInspectionSurface::Surface_CreateMaskGroup(BOOL bInsp)
{
    BOOL bCreateAllMaskGroup = FALSE;
    if (bInsp == FALSE)
        bCreateAllMaskGroup = TRUE;

    BOOL bOtherAlign = FALSE;
    m_bEdgeAlignValid = FALSE;

    auto visionDebugInfos = m_visionUnit.GetVisionDebugInfos();

    for (long i = 0; i < (long)visionDebugInfos.size(); i++)
    {
        auto* debugInfo = visionDebugInfos[i];
        if (debugInfo->pData == NULL)
            continue;

        if (visionDebugInfos[i]->strDebugInfoName == _T("EDGE Align Result"))
        {
            *m_EdgeAlignResult = *(VisionAlignResult*)debugInfo->pData;
            m_bEdgeAlignValid = TRUE;
        }

        if (visionDebugInfos[i]->moduleGuid == _VISION_INSP_GUID_OTHER_ALIGN)
        {
            bOtherAlign = TRUE;
        }
    }

    //===========================================================================================
    // Surface 검사를 위한 Mask Group 을 만들어 준다
    //
    // 만들때 규칙
    // 1. Ignore 할 위치는 BodyAlign 이 화면가운데 였을때를 기준으로 바꾸어 놓는다
    // 2. BodyAlign Center 을 화면 Center 기준으로 바꾸므로 BodyAlign 이 반드시 되어 있어야 한다
    //===========================================================================================

    CSurfaceCustomROI* ROIInfo = &m_surfacePara->m_SurfaceCutemRoi;

    BOOL bIsRoundPKG = FALSE; //kircheis_RoundPKG
    BOOL bIsRoundUpDown = FALSE;
    Ipvm::Rect32s rtAligned(0, 0, 0, 0);

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    if (m_bEdgeAlignValid)
    {
        //======================================
        // BodyAlign 시 얻을 수 있는 CMP 항목들
        //======================================

        long nDx1 = long(m_EdgeAlignResult->fptLT.m_x - m_EdgeAlignResult->fptRT.m_x);
        long nDy1 = long(m_EdgeAlignResult->fptLT.m_y - m_EdgeAlignResult->fptRT.m_y);
        float fL1 = sqrt(float(nDx1 * nDx1 + nDy1 * nDy1));

        long nDx3 = long(m_EdgeAlignResult->fptLB.m_x - m_EdgeAlignResult->fptRB.m_x);
        long nDy3 = long(m_EdgeAlignResult->fptLB.m_y - m_EdgeAlignResult->fptRB.m_y);
        float fL3 = sqrt(float(nDx3 * nDx3 + nDy3 * nDy3));

        float fBodyHalfSizeX = 0.25f * (fL1 + fL3);

        long nDx2 = long(m_EdgeAlignResult->fptRT.m_x - m_EdgeAlignResult->fptRB.m_x);
        long nDy2 = long(m_EdgeAlignResult->fptRT.m_y - m_EdgeAlignResult->fptRB.m_y);
        float fL2 = sqrt(float(nDx2 * nDx2 + nDy2 * nDy2));

        long nDx4 = long(m_EdgeAlignResult->fptLT.m_x - m_EdgeAlignResult->fptLB.m_x);
        long nDy4 = long(m_EdgeAlignResult->fptLT.m_y - m_EdgeAlignResult->fptLB.m_y);
        float fL4 = sqrt(float(nDx4 * nDx4 + nDy4 * nDy4));

        float fBodyHalfSizeY = 0.25f * (fL2 + fL4);

        Ipvm::Rect32s rt(int32_t(imageCenter.m_x - fBodyHalfSizeX + 0.5f),
            int32_t(imageCenter.m_y - fBodyHalfSizeY + 0.5f), int32_t(imageCenter.m_x + fBodyHalfSizeX + 0.5f) + 1,
            int32_t(imageCenter.m_y + fBodyHalfSizeY + 0.5f) + 1);

        ROIInfo->CustomROI_AddRect(_T("BodyAlign Measured"), &rt, 1);

        //{{ kircheis_RoundPKG
        rtAligned = rt;
        bIsRoundPKG = (m_packageSpec.m_bodyInfoMaster->m_nRoundPackageOption != Round_Normal);
        if (m_packageSpec.m_bodyInfoMaster->m_nRoundPackageOption == Round_UpDown)
        {
            bIsRoundUpDown = TRUE;
            float fDistR = getScale().convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->m_fRoundPackageRadius);
            long nDistR = (long)(fDistR + .5f);
            Ipvm::Rect32s rtBodyFPS = rt;
            rtBodyFPS.m_top = (long)((float)(rt.m_top) + fDistR + .5f);
            rtBodyFPS.m_bottom = (long)((float)(rt.m_bottom) - fDistR - .5f);
            ROIInfo->CustomROI_AddRect(_T("Round_CenterBodySpec"), &rtBodyFPS, 1);

            Ipvm::Rect32s rtEllipseFPS = rtBodyFPS;
            rtEllipseFPS.m_top = rtBodyFPS.m_top - nDistR;
            rtEllipseFPS.m_bottom = rtBodyFPS.m_top + nDistR;
            ROIInfo->CustomROI_AddEllipse(_T("Round_EllipseTopSpec"), &rtEllipseFPS, 1);
            rtEllipseFPS.m_top = rtBodyFPS.m_bottom - nDistR;
            rtEllipseFPS.m_bottom = rtBodyFPS.m_bottom + nDistR;
            ROIInfo->CustomROI_AddEllipse(_T("Round_EllipseBottomSpec"), &rtEllipseFPS, 1);
        }
        //}}
        //영훈 - BodyAlign Polygon ( SD Card의 모양이 네모가 아니다.. 흐규흐규 )

        if (m_packageSpec.nDeviceType == enDeviceType::PACKAGE_LEADLESS && m_packageSpec.m_bLowtopEdge)
        {
            long nLowtopHeight;
            long nSpecDistFromLeft
                = (long)(getScale().convert_milToPixelX(m_packageSpec.m_bodyInfoMaster->m_fDistFromLeft) + 0.5f);

            long nBodySizeX = (long)(((m_EdgeAlignResult->fptRT.m_x - m_EdgeAlignResult->fptLT.m_x)
                                         + (m_EdgeAlignResult->fptRB.m_x - m_EdgeAlignResult->fptLB.m_x))
                    * 0.5f
                + 0.5f);
            long nBodySizeY = (long)(((m_EdgeAlignResult->fptLB.m_y - m_EdgeAlignResult->fptLT.m_y)
                                         + (m_EdgeAlignResult->fptRB.m_y - m_EdgeAlignResult->fptRT.m_y))
                    * 0.5f
                + 0.5f);

            long nPolygonCount = 6;
            std::vector<Ipvm::Point32s2> vecSpecBodyPoint;
            vecSpecBodyPoint.resize(nPolygonCount);

            if (m_packageSpec.m_deadBug)
            {
                nLowtopHeight = (long)(((m_EdgeAlignResult->fptLB.m_y - m_EdgeAlignResult->fptLowtopLT.m_y)
                                           + (m_EdgeAlignResult->fptRB.m_y - m_EdgeAlignResult->fptLowtopLT.m_y))
                        * 0.5f
                    + 0.5f);

                vecSpecBodyPoint[0] = Ipvm::Point32s2(0, 0);
                vecSpecBodyPoint[1] = Ipvm::Point32s2(nBodySizeX, 0);
                vecSpecBodyPoint[2] = Ipvm::Point32s2(nBodySizeX, nBodySizeY);
                vecSpecBodyPoint[3] = Ipvm::Point32s2(nSpecDistFromLeft + nLowtopHeight, nBodySizeY);
                vecSpecBodyPoint[4] = Ipvm::Point32s2(nSpecDistFromLeft, nBodySizeY - nLowtopHeight);
                vecSpecBodyPoint[5] = Ipvm::Point32s2(0, nBodySizeY - nLowtopHeight);
            }
            else
            {
                nLowtopHeight = (long)(((m_EdgeAlignResult->fptLowtopLT.m_y - m_EdgeAlignResult->fptLT.m_y)
                                           + (m_EdgeAlignResult->fptLowtopRT.m_y - m_EdgeAlignResult->fptRT.m_y))
                        * 0.5f
                    + 0.5f);

                vecSpecBodyPoint[0] = Ipvm::Point32s2(0, nLowtopHeight);
                vecSpecBodyPoint[1] = Ipvm::Point32s2(nSpecDistFromLeft, nLowtopHeight);
                vecSpecBodyPoint[2] = Ipvm::Point32s2(nSpecDistFromLeft + nLowtopHeight, 0);
                vecSpecBodyPoint[3] = Ipvm::Point32s2(nBodySizeX, 0);
                vecSpecBodyPoint[4] = Ipvm::Point32s2(nBodySizeX, nBodySizeY);
                vecSpecBodyPoint[5] = Ipvm::Point32s2(0, nBodySizeY);
            }

            for (long n = 0; n < nPolygonCount; n++)
            {
                vecSpecBodyPoint[n].m_x += (long)(m_EdgeAlignResult->fptLT.m_x);
                vecSpecBodyPoint[n].m_y += (long)(m_EdgeAlignResult->fptLT.m_y);
            }

            if (vecSpecBodyPoint.size() > 0)
            {
                ROIInfo->CustomROI_AddPolygon(_T("BodyAlign Measured(Poly)"), &vecSpecBodyPoint[0], 1, nPolygonCount);
            }
        }

        if (m_packageSpec.m_bodyInfoMaster->m_bOctagonMode)
        {
            long nBodySizeX = (long)(((m_EdgeAlignResult->fptRT.m_x - m_EdgeAlignResult->fptLT.m_x)
                                         + (m_EdgeAlignResult->fptRB.m_x - m_EdgeAlignResult->fptLB.m_x))
                    * 0.5f
                + 0.5f);
            long nBodySizeY = (long)(((m_EdgeAlignResult->fptLB.m_y - m_EdgeAlignResult->fptLT.m_y)
                                         + (m_EdgeAlignResult->fptRB.m_y - m_EdgeAlignResult->fptRT.m_y))
                    * 0.5f
                + 0.5f);

            long nBodySizeX_HALF = (long)(((m_EdgeAlignResult->fptRT.m_x - m_EdgeAlignResult->fptLT.m_x)
                                              + (m_EdgeAlignResult->fptRB.m_x - m_EdgeAlignResult->fptLB.m_x))
                    * 0.25f
                + 0.5f);
            long nBodySizeY_HALF = (long)(((m_EdgeAlignResult->fptLB.m_y - m_EdgeAlignResult->fptLT.m_y)
                                              + (m_EdgeAlignResult->fptRB.m_y - m_EdgeAlignResult->fptRT.m_y))
                    * 0.25f
                + 0.5f);

            long nPolygonCount = 8;
            std::vector<Ipvm::Point32s2> vecSpecBodyPoint;
            vecSpecBodyPoint.resize(nPolygonCount);

            long nDistOctagonChamfer
                = (long)(getScale().convert_milToPixelX(m_packageSpec.m_bodyInfoMaster->m_fDistOctagonChamfer) + 0.5f);

            vecSpecBodyPoint[0] = Ipvm::Point32s2(0, nDistOctagonChamfer);
            vecSpecBodyPoint[1] = Ipvm::Point32s2(nDistOctagonChamfer, 0);
            vecSpecBodyPoint[2] = Ipvm::Point32s2(nBodySizeX - nDistOctagonChamfer, 0);
            vecSpecBodyPoint[3] = Ipvm::Point32s2(nBodySizeX, nDistOctagonChamfer);
            vecSpecBodyPoint[4] = Ipvm::Point32s2(nBodySizeX, nBodySizeY - nDistOctagonChamfer);
            vecSpecBodyPoint[5] = Ipvm::Point32s2(nBodySizeX - nDistOctagonChamfer, nBodySizeY);
            vecSpecBodyPoint[6] = Ipvm::Point32s2(nDistOctagonChamfer, nBodySizeY);
            vecSpecBodyPoint[7] = Ipvm::Point32s2(0, nBodySizeY - nDistOctagonChamfer);

            for (long n = 0; n < nPolygonCount; n++)
            {
                vecSpecBodyPoint[n].m_x += ((long)(m_EdgeAlignResult->m_center.m_x + 0.5f)) - nBodySizeX_HALF;
                vecSpecBodyPoint[n].m_y += ((long)(m_EdgeAlignResult->m_center.m_y + 0.5f)) - nBodySizeY_HALF;
            }

            if ((long)vecSpecBodyPoint.size() > 0)
            {
                ROIInfo->CustomROI_AddPolygon(_T("Octagon Body Spec (Poly)"), &vecSpecBodyPoint[0], 1, nPolygonCount);
            }
        }
    }

    Ipvm::Image8u imageMask;
    if (!getReusableMemory().GetInspByteImage(imageMask))
        return FALSE;

    // 영훈 20130912 : Clearimage도 Surface가 많아지면 시간이 많이 걸리므로 bodyalign 영역정도만 초기화해준도록 하자.
    PI_RECT arBody(
        m_EdgeAlignResult->fptLT, m_EdgeAlignResult->fptRT, m_EdgeAlignResult->fptLB, m_EdgeAlignResult->fptRB);
    Ipvm::Rect32s rtBody(arBody.CreateExternalMinimumRect());
    long nOffsetX, nOffsetY;
    nOffsetX = (long)(rtBody.CenterPoint().m_x - (getImageLotInsp().GetImageSizeX() * 0.5f));
    nOffsetY = (long)(rtBody.CenterPoint().m_y - (getImageLotInsp().GetImageSizeY() * 0.5f));

    long offsetL = (long)(getScale().convert_umToPixelX(m_surfacePara->m_SurfaceItem.AlgoPara.m_offsetLeft_um) + 0.5f);
    long offsetR = (long)(getScale().convert_umToPixelX(m_surfacePara->m_SurfaceItem.AlgoPara.m_offsetRight_um) + 0.5f);
    long offsetT = (long)(getScale().convert_umToPixelY(m_surfacePara->m_SurfaceItem.AlgoPara.m_offsetTop_um) + 0.5f);
    long offsetB
        = (long)(getScale().convert_umToPixelY(m_surfacePara->m_SurfaceItem.AlgoPara.m_offsetBottom_um) + 0.5f);

    Ipvm::Rect32s rtInitBinary = rtBody;
    rtInitBinary.InflateRect(-offsetL, -offsetT, -offsetR, -offsetB);
    rtInitBinary.OffsetRect(-nOffsetX, -nOffsetY);
    rtInitBinary &= Ipvm::Rect32s(0, 0, getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());

    Ipvm::ImageProcessing::Fill(rtInitBinary, 0, imageMask);

    const auto& maskOrder = m_surfacePara->m_SurfaceItem.AlgoPara.m_maskOrder;

    if (bCreateAllMaskGroup)
    {
        for (long n = 0; n < getReusableMemory().GetSurfaceLayerMaskCount(); n++)
        {
            auto& layerMask = getReusableMemory().GetSurfaceLayerMask(n);
            CString maskName = getReusableMemory().GetSurfaceLayerMaskName(n);

            if (maskName == GetSurfaceMaskNameFromLayerName(GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME))
            {
                continue;
            }

            m_result.m_supportMask.Set(maskName, layerMask);

            maskName.Empty();
        }

        for (long n = 0; n < getReusableMemory().GetSurfaceLayerRoiCount(); n++)
        {
            CString name = getReusableMemory().GetSurfaceLayerRoiName(n);
            Ipvm::Image8u layerImage;

            if (GetSurfaceLayerImage(name, layerImage))
            {
                m_result.m_supportMask.Set(name, layerImage);
            }

            name.Empty();
        }

        SetCustomROI_SurfaceBitmapMask();
    }

    long nDebugInfoSize = (long)visionDebugInfos.size();
    for (long i = 0; i < nDebugInfoSize; i++)
    {
        auto* debugInfo = visionDebugInfos[i];
        if (debugInfo == nullptr)
            continue;
        if (debugInfo->pData == nullptr)
            continue;
        if (debugInfo->nDataNum <= 0)
            continue;

        if (bCreateAllMaskGroup)
        {
            if (debugInfo->strDebugInfoName == _T("Underfill Fillet Mask Image"))
            {
                Ipvm::Image8u mask;
                if (getReusableMemory().GetInspByteImage(mask))
                {
                    Ipvm::Rect32s rtPackage = rtInitBinary;
                    rtPackage.InflateRect(10, 10);
                    Ipvm::Image8u* debugImage = (Ipvm::Image8u*)debugInfo->pData;
                    Ipvm::ImageProcessing::Copy(*debugImage, rtPackage, rtPackage, mask);
                    m_result.m_supportMask.Set(debugInfo->strDebugInfoName, mask);

                    SetCustomROI_SurfaceBitmapMask();
                }
            }
            if (bIsRoundPKG && debugInfo->strDebugInfoName == "Round Research Ellipse")
            {
                long nDataNum = debugInfo->nDataNum;
                Ipvm::EllipseEq32r* psEllipseResult = (Ipvm::EllipseEq32r*)(debugInfo->pData);

                if (nDataNum == 2)
                {
                    Ipvm::Rect32s rtEllipse(0, 0, 0, 0);
                    Ipvm::Rect32s rtAlignedBody = rtAligned;
                    float fX = psEllipseResult[0].m_x;
                    float fY = psEllipseResult[0].m_y;
                    rtEllipse.m_left = (long)(fX - psEllipseResult[0].m_xradius + 0.5f);
                    rtEllipse.m_right = (long)(fX + psEllipseResult[0].m_xradius + 0.5f);
                    rtEllipse.m_top = (long)(fY - psEllipseResult[0].m_yradius + 0.5f);
                    rtEllipse.m_bottom = (long)(fY + psEllipseResult[0].m_yradius + 0.5f);
                    ROIInfo->CustomROI_AddEllipse(_T("Round_EllipseTopMeasured"), &rtEllipse, 1);
                    if (bIsRoundUpDown)
                        rtAlignedBody.m_top = (long)(fY + .5f);

                    fX = psEllipseResult[1].m_x;
                    fY = psEllipseResult[1].m_y;
                    rtEllipse.m_left = (long)(fX - psEllipseResult[1].m_xradius + 0.5f);
                    rtEllipse.m_right = (long)(fX + psEllipseResult[1].m_xradius + 0.5f);
                    rtEllipse.m_top = (long)(fY - psEllipseResult[1].m_yradius + 0.5f);
                    rtEllipse.m_bottom = (long)(fY + psEllipseResult[1].m_yradius + 0.5f);
                    ROIInfo->CustomROI_AddEllipse(_T("Round_EllipseBottomMeasured"), &rtEllipse, 1);
                    if (bIsRoundUpDown)
                        rtAlignedBody.m_bottom = (long)(fY + .5f);

                    ROIInfo->CustomROI_AddRect(_T("Round_CenterBodyMeasured"), &rtAlignedBody, 1);
                }
            }
        }

        if (debugInfo->moduleGuid == _VISION_INSP_GUID_BGA_BALL_2D)
        {
            VisionAlignResult AlignResult;

            if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Ball Measured"))
                || maskOrder.IsValid(_T("Ball Measured & Spec Radius")))
            {
                if (debugInfo->strDebugInfoName == "Ball Align Result by Ball Insp")
                {
                    long nDataNum = (long)debugInfo->nDataNum;
                    if (nDataNum <= 0 || debugInfo->pData == nullptr)
                        continue;

                    AlignResult = *(VisionAlignResult*)debugInfo->pData;

                    std::vector<Ipvm::Rect32s> vecBall1(AlignResult.m_ballWidths_px.size());
                    std::vector<Ipvm::Rect32s> vecBall2(AlignResult.m_ballWidths_px.size());

                    long nIter = 0;

                    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
                    {
                        const float fRadiusSpecX = (float)ball.m_radiusX_px;
                        const float fRadiusSpecY = (float)ball.m_radiusY_px;
                        const float widthCenterPosX = AlignResult.m_ballWidthCenterPos_px[ball.m_index].m_x;
                        const float widthCenterPosY = AlignResult.m_ballWidthCenterPos_px[ball.m_index].m_y;

                        float fX = widthCenterPosX;
                        float fY = widthCenterPosY;

                        const float fRadiusMeasure = AlignResult.m_ballWidths_px[ball.m_index] * 0.5f;

                        vecBall1[nIter]
                            = Ipvm::Rect32s(int32_t(fX - fRadiusMeasure + 0.5f), int32_t(fY - fRadiusMeasure + 0.5f),
                                int32_t(fX + fRadiusMeasure + 0.5f) + 1, int32_t(fY + fRadiusMeasure + 0.5f) + 1);
                        vecBall2[nIter]
                            = Ipvm::Rect32s(int32_t(fX - fRadiusSpecX + 0.5f), int32_t(fY - fRadiusSpecY + 0.5f),
                                int32_t(fX + fRadiusSpecX + 0.5f) + 1, int32_t(fY + fRadiusSpecY + 0.5f) + 1);

                        nIter++;
                    }

                    ROIInfo->CustomROI_AddEllipse(_T("Ball Measured"), &vecBall1[0], nIter);
                    ROIInfo->CustomROI_AddEllipse(_T("Ball Measured & Spec Radius"), &vecBall2[0], nIter);
                }
            }
        }

        if (debugInfo->moduleGuid == _VISION_INSP_GUID_BGA_BALL_3D)
        {
            VisionAlignResult AlignResult;

            if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Ball Measured"))
                || maskOrder.IsValid(_T("Ball Spec Circle")))
            {
                if (debugInfo->strDebugInfoName == _T("Ball 3D Aligned ROI")
                    || debugInfo->strDebugInfoName == "Ball 3D Spec ROI")
                {
                    CString strMaskOrder = (debugInfo->strDebugInfoName == _T("Ball 3D Aligned ROI"))
                        ? _T("Ball Measured")
                        : _T("Ball Spec Circle");
                    long nDataNum = (long)debugInfo->nDataNum;
                    if (nDataNum <= 0 || debugInfo->pData == nullptr)
                        continue;

                    std::vector<Ipvm::Rect32s> vecrt3DBallRect(nDataNum);
                    Ipvm::Rect32s* prt3DBallRect = (Ipvm::Rect32s*)debugInfo->pData;

                    const long nBallCount = CAST_LONG(m_packageSpec.m_ballMap->m_balls.size());
                    for (long nIter = 0; nIter < nBallCount; nIter++)
                    {
                        vecrt3DBallRect[nIter] = prt3DBallRect[nIter];
                    }

                    ROIInfo->CustomROI_AddEllipse(strMaskOrder, &vecrt3DBallRect[0], nBallCount);

                    strMaskOrder.Empty();
                }
            }
        }
        if (debugInfo->strInspName == "ChipLayer Inspection") //kircheis_580 ChipLayerInsp
        {
            if (debugInfo->strDebugInfoName == _T("Chip SpecRect")) //kircheis_580 ChipLayerInsp
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Chip Layer Spec Rect")))
                {
                    long nDataNum = (long)debugInfo->nDataNum;
                    if (nDataNum <= 0)
                        continue;

                    Ipvm::Rect32r* pfrtDetectROI;
                    pfrtDetectROI = (Ipvm::Rect32r*)debugInfo->pData;
                    std::vector<Ipvm::Rect32s> vecrtChipLayerSpecRect(nDataNum);

                    for (long nROIID = 0; nROIID < nDataNum; nROIID++)
                    {
                        vecrtChipLayerSpecRect[nROIID] = Ipvm::Conversion::ToRect32s(pfrtDetectROI[nROIID]);
                    }
                    ROIInfo->CustomROI_AddRect(_T("Chip Layer Spec Rect"), &vecrtChipLayerSpecRect[0], nDataNum);
                }
            }
            else if (debugInfo->strDebugInfoName == "Chip Align Result")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Chip Layer Align Result")))
                {
                    long nDataNum = (long)debugInfo->nDataNum;
                    if (nDataNum <= 0)
                        continue;
                    PI_RECT* pArbitraryRect = (PI_RECT*)debugInfo->pData;
                    std::vector<Ipvm::Point32s2> vecptChipLayerAlignResult(0);

                    for (long nChip = 0; nChip < nDataNum; nChip++)
                    {
                        vecptChipLayerAlignResult.push_back(
                            Ipvm::Point32s2(pArbitraryRect[nChip].ltX, pArbitraryRect[nChip].ltY));
                        vecptChipLayerAlignResult.push_back(
                            Ipvm::Point32s2(pArbitraryRect[nChip].rtX, pArbitraryRect[nChip].rtY));
                        vecptChipLayerAlignResult.push_back(
                            Ipvm::Point32s2(pArbitraryRect[nChip].brX, pArbitraryRect[nChip].brY));
                        vecptChipLayerAlignResult.push_back(
                            Ipvm::Point32s2(pArbitraryRect[nChip].blX, pArbitraryRect[nChip].blY));
                    }

                    ROIInfo->CustomROI_AddPolygon(
                        _T("Chip Layer Align Result"), &vecptChipLayerAlignResult[0], nDataNum, 4);
                }
            }
        }
        if (bOtherAlign && debugInfo->moduleGuid == _VISION_INSP_GUID_OTHER_ALIGN)
        {
            if (debugInfo->strDebugInfoName == _T("Detect Rect")) //kircheis_USI_OA
            {
                Ipvm::Rect32r* pfrtDetectRect;
                std::vector<Ipvm::Rect32s> vecFoot;
                vecFoot.resize(1);
                CString strSurfaceItemName;

                pfrtDetectRect = (Ipvm::Rect32r*)debugInfo->pData;

                for (long nROIID = 0; nROIID < debugInfo->nDataNum; nROIID++)
                {
                    strSurfaceItemName.Format(_T("Rect Object Align Result(Rect) %d"), nROIID);

                    if (bCreateAllMaskGroup || maskOrder.IsValid(strSurfaceItemName))
                    {
                        vecFoot[0] = Ipvm::Conversion::ToRect32s(pfrtDetectRect[nROIID]);
                        ROIInfo->CustomROI_AddRect(strSurfaceItemName, &vecFoot[0], 1);
                    }
                }

                strSurfaceItemName.Empty();
            }
            else if (debugInfo->strDebugInfoName == _T("Detect ROI")) //kircheis_USI_OA
            {
                PI_RECT* psrtDetectROI;
                CString strSurfaceItemName;

                long nDataNum = (long)debugInfo->nDataNum;

                psrtDetectROI = (PI_RECT*)debugInfo->pData;

                std::vector<Ipvm::Point32s2> vecptOtherAlignResult(0);
                for (long nROIID = 0; nROIID < nDataNum; nROIID++)
                {
                    strSurfaceItemName.Format(_T("Rect Object Align Result %d"), nROIID);

                    if (bCreateAllMaskGroup || maskOrder.IsValid(strSurfaceItemName))
                    {
                        vecptOtherAlignResult.push_back(
                            Ipvm::Point32s2(psrtDetectROI[nROIID].ltX, psrtDetectROI[nROIID].ltY));
                        vecptOtherAlignResult.push_back(
                            Ipvm::Point32s2(psrtDetectROI[nROIID].rtX, psrtDetectROI[nROIID].rtY));
                        vecptOtherAlignResult.push_back(
                            Ipvm::Point32s2(psrtDetectROI[nROIID].brX, psrtDetectROI[nROIID].brY));
                        vecptOtherAlignResult.push_back(
                            Ipvm::Point32s2(psrtDetectROI[nROIID].blX, psrtDetectROI[nROIID].blY));

                        ROIInfo->CustomROI_AddPolygon(strSurfaceItemName, &vecptOtherAlignResult[0], nDataNum, 4);
                    }
                }

                strSurfaceItemName.Empty();
            }
        }

        if (debugInfo->moduleGuid == _VISION_INSP_GUID_2D_MATRIX)
        {
            if (debugInfo->strDebugInfoName == "2D Matrix ROI")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("2D Matrix ROI")))
                {
                    Ipvm::Rect32s* prt2DMatrix = (Ipvm::Rect32s*)debugInfo->pData;
                    long nDataNum = debugInfo->nDataNum;
                    std::vector<Ipvm::Rect32s> vecrt2DID(nDataNum);
                    for (long nID = 0; nID < nDataNum; nID++)
                    {
                        vecrt2DID[nID] = prt2DMatrix[nID];
                    }

                    ROIInfo->CustomROI_AddRect(_T("2D Matrix ROI"), &vecrt2DID[0], nDataNum);
                }
            }
        }

        if (debugInfo->strInspName == "CIS Tape Inspection")
        {
            if (debugInfo->strDebugInfoName == "Taping Align Result ROI")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Tape Measured")))
                {
                    PI_RECT* pArbitraryRect = (PI_RECT*)debugInfo->pData;
                    Ipvm::Point32s2 pt[4];

                    Ipvm::Point32s2 lt(pArbitraryRect->ltX, pArbitraryRect->ltY);
                    Ipvm::Point32s2 rt(pArbitraryRect->rtX, pArbitraryRect->rtY);
                    Ipvm::Point32s2 br(pArbitraryRect->brX, pArbitraryRect->brY);
                    Ipvm::Point32s2 bl(pArbitraryRect->blX, pArbitraryRect->blY);

                    pt[0] = Ipvm::Point32s2(lt);
                    pt[1] = Ipvm::Point32s2(rt);
                    pt[2] = Ipvm::Point32s2(br);
                    pt[3] = Ipvm::Point32s2(bl);

                    ROIInfo->CustomROI_AddPolygon(_T("Tape Measured"), &pt[0], 1, 4);
                }
            }
        }

        //{{Passive
        if (debugInfo->moduleGuid == _VISION_INSP_GUID_PASSIVE_2D
            || debugInfo->moduleGuid == _VISION_INSP_GUID_PASSIVE_3D)
        {
            if (debugInfo->strDebugInfoName == "Spec Passive ROI")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Passive Spec ROI")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2pt;

                    PI_RECT* psfrt = (PI_RECT*)debugInfo->pData;
                    for (long nChipID = 0; nChipID < debugInfo->nDataNum; nChipID++)
                    {
                        const auto& srt = psfrt[nChipID];

                        std::vector<Ipvm::Point32s2> vecpt(4);
                        vecpt[0] = Ipvm::Point32s2(srt.ltX, srt.ltY);
                        vecpt[1] = Ipvm::Point32s2(srt.rtX, srt.rtY);
                        vecpt[2] = Ipvm::Point32s2(srt.brX, srt.brY);
                        vecpt[3] = Ipvm::Point32s2(srt.blX, srt.blY);

                        vec2pt.push_back(vecpt);
                    }

                    ROIInfo->CustomROI_AddPolygon(_T("Passive Spec ROI"), vec2pt, (long)vec2pt.size(), 4);
                }
            }

            //{{mc_PassiveDetailAlignResult
            if (debugInfo->strDebugInfoName == "DetailAlign_Passive_Result"
                && debugInfo->moduleGuid != _VISION_INSP_GUID_PASSIVE_3D)
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Passive Align Reulst")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2ptPassiveDetailAlignResult;

                    if (debugInfo != NULL && debugInfo->pData != NULL && debugInfo->nDataNum > 0)
                    {
                        FPI_RECT* psfrt = (FPI_RECT*)debugInfo->pData;
                        for (long nCount = 0; nCount < debugInfo->nDataNum; nCount++)
                        {
                            const auto& sfrtPassive = psfrt[nCount];
                            std::vector<Ipvm::Point32s2> vecpt(4);
                            vecpt[0] = Ipvm::Point32s2(
                                (long)(sfrtPassive.fptLT.m_x + .5f), (long)(sfrtPassive.fptLT.m_y + .5f)); //LT
                            vecpt[1] = Ipvm::Point32s2(
                                (long)(sfrtPassive.fptRT.m_x + .5f), (long)(sfrtPassive.fptRT.m_y + .5f)); //RT
                            vecpt[2] = Ipvm::Point32s2(
                                (long)(sfrtPassive.fptRB.m_x + .5f), (long)(sfrtPassive.fptRB.m_y + .5f)); //RB
                            vecpt[3] = Ipvm::Point32s2(
                                (long)(sfrtPassive.fptLB.m_x + .5f), (long)(sfrtPassive.fptLB.m_y + .5f)); //LB

                            vec2ptPassiveDetailAlignResult.push_back(vecpt);
                        }
                        ROIInfo->CustomROI_AddPolygon(_T("Passive Align Reulst"), vec2ptPassiveDetailAlignResult,
                            (long)vec2ptPassiveDetailAlignResult.size(), 4);
                    }
                }
            }

            if (debugInfo->strDebugInfoName == "Valid Component ROI"
                && debugInfo->moduleGuid == _VISION_INSP_GUID_PASSIVE_3D)
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Passive Align Result")))
                {
                    if (debugInfo != NULL && debugInfo->pData != NULL && debugInfo->nDataNum > 0)
                    {
                        std::vector<std::vector<Ipvm::Point32s2>> vec2pt;

                        PI_RECT* psfrt = (PI_RECT*)debugInfo->pData;
                        for (long nChipID = 0; nChipID < debugInfo->nDataNum; nChipID++)
                        {
                            const auto& srt = psfrt[nChipID];

                            std::vector<Ipvm::Point32s2> vecpt(4);
                            vecpt[0] = Ipvm::Point32s2(srt.ltX, srt.ltY);
                            vecpt[1] = Ipvm::Point32s2(srt.rtX, srt.rtY);
                            vecpt[2] = Ipvm::Point32s2(srt.brX, srt.brY);
                            vecpt[3] = Ipvm::Point32s2(srt.blX, srt.blY);

                            vec2pt.push_back(vecpt);
                        }
                        ROIInfo->CustomROI_AddPolygon(_T("Passive Align Result"), vec2pt, (long)vec2pt.size(), 4);
                    }
                }
            }

            //k DetailAlign_Passive_Result 방향에 따라 나눔
            if (debugInfo->strDebugInfoName == "DetailAlign_Passive_Result_Horizontal"
                && debugInfo->moduleGuid != _VISION_INSP_GUID_PASSIVE_3D)
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Passive Horizontal")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2ptPassiveDetailAlignResult_Horizontal;

                    if (debugInfo->nDataNum > 0)
                    {
                        FPI_RECT* psfrt = (FPI_RECT*)debugInfo->pData;
                        for (long nCount = 0; nCount < debugInfo->nDataNum; nCount++)
                        {
                            const auto& sfrtPassive_Horizontal = psfrt[nCount];
                            std::vector<Ipvm::Point32s2> vecpt(4);
                            vecpt[0] = Ipvm::Point32s2((long)(sfrtPassive_Horizontal.fptLT.m_x + .5f),
                                (long)(sfrtPassive_Horizontal.fptLT.m_y + .5f)); //LT
                            vecpt[1] = Ipvm::Point32s2((long)(sfrtPassive_Horizontal.fptRT.m_x + .5f),
                                (long)(sfrtPassive_Horizontal.fptRT.m_y + .5f)); //RT
                            vecpt[2] = Ipvm::Point32s2((long)(sfrtPassive_Horizontal.fptRB.m_x + .5f),
                                (long)(sfrtPassive_Horizontal.fptRB.m_y + .5f)); //RB
                            vecpt[3] = Ipvm::Point32s2((long)(sfrtPassive_Horizontal.fptLB.m_x + .5f),
                                (long)(sfrtPassive_Horizontal.fptLB.m_y + .5f)); //LB

                            vec2ptPassiveDetailAlignResult_Horizontal.push_back(vecpt);
                        }
                        ROIInfo->CustomROI_AddPolygon(_T("Passive Horizontal"),
                            vec2ptPassiveDetailAlignResult_Horizontal,
                            (long)vec2ptPassiveDetailAlignResult_Horizontal.size(), 4);
                    }
                }
            }

            if (debugInfo->strDebugInfoName == "DetailAlign_Passive_Result_Vertical"
                && debugInfo->moduleGuid != _VISION_INSP_GUID_PASSIVE_3D)
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Passive Vertical")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2ptPassiveDetailAlignResult_Vertical;

                    if (debugInfo->nDataNum > 0)
                    {
                        FPI_RECT* psfrt = (FPI_RECT*)debugInfo->pData;
                        for (long nCount = 0; nCount < debugInfo->nDataNum; nCount++)
                        {
                            const auto& sfrtPassive_Vertical = psfrt[nCount];
                            std::vector<Ipvm::Point32s2> vecpt(4);
                            vecpt[0] = Ipvm::Point32s2((long)(sfrtPassive_Vertical.fptLT.m_x + .5f),
                                (long)(sfrtPassive_Vertical.fptLT.m_y + .5f)); //LT
                            vecpt[1] = Ipvm::Point32s2((long)(sfrtPassive_Vertical.fptRT.m_x + .5f),
                                (long)(sfrtPassive_Vertical.fptRT.m_y + .5f)); //RT
                            vecpt[2] = Ipvm::Point32s2((long)(sfrtPassive_Vertical.fptRB.m_x + .5f),
                                (long)(sfrtPassive_Vertical.fptRB.m_y + .5f)); //RB
                            vecpt[3] = Ipvm::Point32s2((long)(sfrtPassive_Vertical.fptLB.m_x + .5f),
                                (long)(sfrtPassive_Vertical.fptLB.m_y + .5f)); //LB

                            vec2ptPassiveDetailAlignResult_Vertical.push_back(vecpt);
                        }
                        ROIInfo->CustomROI_AddPolygon(_T("Passive Vertical"), vec2ptPassiveDetailAlignResult_Vertical,
                            (long)vec2ptPassiveDetailAlignResult_Vertical.size(), 4);
                    }
                }
            }

            if (debugInfo->strDebugInfoName == "DetailAlign_Passive_Elect_Result"
                && debugInfo->moduleGuid != _VISION_INSP_GUID_PASSIVE_3D)
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Passive Elect")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2ptElectDetailAlignResult;

                    if (debugInfo->nDataNum > 0)
                    {
                        FPI_RECT* psfrt = (FPI_RECT*)debugInfo->pData;
                        for (long nCount = 0; nCount < debugInfo->nDataNum; nCount++)
                        {
                            const auto& sfrtElect = psfrt[nCount];
                            std::vector<Ipvm::Point32s2> vecpt(4);
                            vecpt[0] = Ipvm::Point32s2(
                                (long)(sfrtElect.fptLT.m_x + .5f), (long)(sfrtElect.fptLT.m_y + .5f)); //LT
                            vecpt[1] = Ipvm::Point32s2(
                                (long)(sfrtElect.fptRT.m_x + .5f), (long)(sfrtElect.fptRT.m_y + .5f)); //RT
                            vecpt[2] = Ipvm::Point32s2(
                                (long)(sfrtElect.fptRB.m_x + .5f), (long)(sfrtElect.fptRB.m_y + .5f)); //RB
                            vecpt[3] = Ipvm::Point32s2(
                                (long)(sfrtElect.fptLB.m_x + .5f), (long)(sfrtElect.fptLB.m_y + .5f)); //LB

                            vec2ptElectDetailAlignResult.push_back(vecpt);
                        }
                        ROIInfo->CustomROI_AddPolygon(_T("Passive Elect"), vec2ptElectDetailAlignResult,
                            (long)vec2ptElectDetailAlignResult.size(), 4);
                    }
                }
            }

            if (debugInfo->strDebugInfoName == "DetailAlign_Passive_Body_Result"
                && debugInfo->moduleGuid != _VISION_INSP_GUID_PASSIVE_3D)
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Passive Body")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2ptBodyDetailAlignResult;

                    if (debugInfo->nDataNum > 0)
                    {
                        FPI_RECT* psfrt = (FPI_RECT*)debugInfo->pData;
                        for (long nCount = 0; nCount < debugInfo->nDataNum; nCount++)
                        {
                            const auto& sfrtBody = psfrt[nCount];
                            std::vector<Ipvm::Point32s2> vecpt(4);
                            vecpt[0] = Ipvm::Point32s2(
                                (long)(sfrtBody.fptLT.m_x + .5f), (long)(sfrtBody.fptLT.m_y + .5f)); //LT
                            vecpt[1] = Ipvm::Point32s2(
                                (long)(sfrtBody.fptRT.m_x + .5f), (long)(sfrtBody.fptRT.m_y + .5f)); //RT
                            vecpt[2] = Ipvm::Point32s2(
                                (long)(sfrtBody.fptRB.m_x + .5f), (long)(sfrtBody.fptRB.m_y + .5f)); //RB
                            vecpt[3] = Ipvm::Point32s2(
                                (long)(sfrtBody.fptLB.m_x + .5f), (long)(sfrtBody.fptLB.m_y + .5f)); //LB

                            vec2ptBodyDetailAlignResult.push_back(vecpt);
                        }
                        ROIInfo->CustomROI_AddPolygon(_T("Passive Body"), vec2ptBodyDetailAlignResult,
                            (long)vec2ptBodyDetailAlignResult.size(), 4);
                    }
                }
            }
            //}}

            //{{ Chip
            if (debugInfo->strDebugInfoName == "Chip ROI")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Chip ROI")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2pt;

                    PI_RECT* psfrt = (PI_RECT*)debugInfo->pData;
                    for (long nChipID = 0; nChipID < debugInfo->nDataNum; nChipID++)
                    {
                        const auto& srt = psfrt[nChipID];

                        std::vector<Ipvm::Point32s2> vecpt(4);
                        vecpt[0] = Ipvm::Point32s2(srt.ltX, srt.ltY);
                        vecpt[1] = Ipvm::Point32s2(srt.rtX, srt.rtY);
                        vecpt[2] = Ipvm::Point32s2(srt.brX, srt.brY);
                        vecpt[3] = Ipvm::Point32s2(srt.blX, srt.blY);

                        vec2pt.push_back(vecpt);
                    }

                    ROIInfo->CustomROI_AddPolygon(_T("Chip ROI"), vec2pt, (long)vec2pt.size(), 4);
                }
            }

            if (debugInfo->strDebugInfoName == "Chip Underfill ROI")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Chip Underfill ROI")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2pt;

                    PI_RECT* psfrt = (PI_RECT*)debugInfo->pData;
                    for (long nChipID = 0; nChipID < debugInfo->nDataNum; nChipID++)
                    {
                        const auto& srt = psfrt[nChipID];

                        std::vector<Ipvm::Point32s2> vecpt(4);
                        vecpt[0] = Ipvm::Point32s2(srt.ltX, srt.ltY);
                        vecpt[1] = Ipvm::Point32s2(srt.rtX, srt.rtY);
                        vecpt[2] = Ipvm::Point32s2(srt.brX, srt.brY);
                        vecpt[3] = Ipvm::Point32s2(srt.blX, srt.blY);

                        vec2pt.push_back(vecpt);
                    }

                    ROIInfo->CustomROI_AddPolygon(_T("Chip Underfill ROI"), vec2pt, (long)vec2pt.size(), 4);
                }
            }

            if (debugInfo->strDebugInfoName == _T("Underfill Fillet ROI"))
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Underfill Fillet ROI")))
                {
                    std::vector<Ipvm::Rect32s> vecrt;

                    Ipvm::Rect32s* prt = (Ipvm::Rect32s*)debugInfo->pData;
                    for (long nChipID = 0; nChipID < debugInfo->nDataNum; nChipID++)
                    {
                        const auto& rt = prt[nChipID];
                        vecrt.push_back(rt);
                    }

                    ROIInfo->CustomROI_AddRect(_T("Underfill Fillet ROI"), &vecrt[0], (long)vecrt.size());
                }
            }

            if (debugInfo->strDebugInfoName == "Spec Chip ROI")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Spec Chip ROI")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2pt;

                    PI_RECT* psfrt = (PI_RECT*)debugInfo->pData;
                    for (long nChipID = 0; nChipID < debugInfo->nDataNum; nChipID++)
                    {
                        const auto& srt = psfrt[nChipID];

                        std::vector<Ipvm::Point32s2> vecpt(4);
                        vecpt[0] = Ipvm::Point32s2(srt.ltX, srt.ltY);
                        vecpt[1] = Ipvm::Point32s2(srt.rtX, srt.rtY);
                        vecpt[2] = Ipvm::Point32s2(srt.brX, srt.brY);
                        vecpt[3] = Ipvm::Point32s2(srt.blX, srt.blY);

                        vec2pt.push_back(vecpt);
                    }

                    ROIInfo->CustomROI_AddPolygon(_T("Spec Chip ROI"), vec2pt, (long)vec2pt.size(), 4);
                }
            }

            if (debugInfo->strDebugInfoName == "Underfill Dispensing ROI")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Underfill Dispensing ROI")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2pt;

                    PI_RECT* psfrt = (PI_RECT*)debugInfo->pData;
                    for (long nChipID = 0; nChipID < debugInfo->nDataNum; nChipID++)
                    {
                        const auto& srt = psfrt[nChipID];

                        std::vector<Ipvm::Point32s2> vecpt(4);
                        vecpt[0] = Ipvm::Point32s2(srt.ltX, srt.ltY);
                        vecpt[1] = Ipvm::Point32s2(srt.rtX, srt.rtY);
                        vecpt[2] = Ipvm::Point32s2(srt.brX, srt.brY);
                        vecpt[3] = Ipvm::Point32s2(srt.blX, srt.blY);

                        vec2pt.push_back(vecpt);
                    }

                    ROIInfo->CustomROI_AddPolygon(_T("Underfill Dispensing ROI"), vec2pt, (long)vec2pt.size(), 4);
                }
            }

            if (debugInfo->strDebugInfoName == "Heatsink ROI")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Heatsink ROI")))
                {
                    std::vector<std::vector<Ipvm::Point32s2>> vec2pt;

                    PI_RECT* psfrt = (PI_RECT*)debugInfo->pData;
                    for (long nChipID = 0; nChipID < debugInfo->nDataNum; nChipID++)
                    {
                        const auto& srt = psfrt[nChipID];

                        std::vector<Ipvm::Point32s2> vecpt(4);
                        vecpt[0] = Ipvm::Point32s2(srt.ltX, srt.ltY);
                        vecpt[1] = Ipvm::Point32s2(srt.rtX, srt.rtY);
                        vecpt[2] = Ipvm::Point32s2(srt.brX, srt.brY);
                        vecpt[3] = Ipvm::Point32s2(srt.blX, srt.blY);

                        vec2pt.push_back(vecpt);
                    }

                    ROIInfo->CustomROI_AddPolygon(_T("Heatsink ROI"), vec2pt, (long)vec2pt.size(), 4);
                }
            }

            if (debugInfo->strDebugInfoName == "Heatsink Hole ROI")
            {
                if (bCreateAllMaskGroup || maskOrder.IsValid(_T("Heatsink Hole ROI")))
                {
                    std::vector<Ipvm::Rect32s> vecrt;

                    Ipvm::Rect32r* pfrt = (Ipvm::Rect32r*)debugInfo->pData;
                    for (long nChipID = 0; nChipID < debugInfo->nDataNum; nChipID++)
                    {
                        vecrt.push_back(Ipvm::Conversion::ToRect32s(pfrt[nChipID]));
                    }

                    ROIInfo->CustomROI_AddEllipse(_T("Heatsink Hole ROI"), &vecrt[0], 1);
                }
            }
        }
    }

    return 0;
}

long VisionInspectionSurface::MakeObjectIgnorePattern(
    const CSurfaceItem& SurfaceItem, const Ipvm::Image8u& thresholdImage, const Ipvm::Image32s& imageLabel)
{
    Ipvm::Image8u imageMaskTemp;
    getReusableMemory().GetInspByteImage(imageMaskTemp);

    BOOL bAdjustCriteria = FALSE;

    std::vector<CSurfaceCriteria>::const_iterator itrCriteria = SurfaceItem.vecCriteria.begin();
    std::vector<CSurfaceCriteria>::const_iterator itrCriteriaEnd = SurfaceItem.vecCriteria.end();

    while (itrCriteria != itrCriteriaEnd)
    {
        if (itrCriteria->bInsp)
        {
            bAdjustCriteria = TRUE;
            break;
        }

        itrCriteria++;
    }

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageMaskTemp), 0, imageMaskTemp);

    if (bAdjustCriteria)
    {
        std::vector<SDetectedSurfaceObject>& vecDSOValue = m_result.vecDSOValue;
        std::vector<long>& vecnResult = m_result.vecnResult;

        std::vector<SDetectedSurfaceObject>::iterator itrObject = vecDSOValue.begin();
        std::vector<SDetectedSurfaceObject>::iterator itrObjectEnd = vecDSOValue.end();

        std::vector<long>::iterator itrResult = vecnResult.begin();
        std::vector<long>::iterator itrResultEnd = vecnResult.end();

        while (itrResult != itrResultEnd && itrObject != itrObjectEnd)
        {
            if (*itrResult == REJECT)
            {
                Ipvm::Rect32s& rtROI = itrObject->rtROI;

                long nBeginY = rtROI.m_top;
                long nEndY = rtROI.m_bottom;
                long nBeginX = rtROI.m_left;
                long nEndX = rtROI.m_right;

                long lLabel = itrObject->nLabel;

                for (long y = nBeginY; y < nEndY; y++)
                {
                    auto* imageTempMask_y = imageMaskTemp.GetMem(0, y);
                    auto* label_y = imageLabel.GetMem(0, y);
                    for (long x = nBeginX; x < nEndX; x++)
                    {
                        if (lLabel == label_y[x])
                        {
                            imageTempMask_y[x] = 0xff;
                        }
                    }
                }
            }

            itrResult++;
            itrObject++;
        }
    }
    else
    {
        //long nImageSize = getImageLotInsp().GetImageSizeX() * getImageLotInsp().GetImageSizeY();

        for (long y = 0; y < getImageLotInsp().GetImageSizeY(); y++)
        {
            auto* imageTempMask_y = imageMaskTemp.GetMem(0, y);
            auto* threshold_y = thresholdImage.GetMem(0, y);

            for (long x = 0; x < getImageLotInsp().GetImageSizeX(); x++)
            {
                if (threshold_y[x] == 0 || threshold_y[x] == 255)
                    imageTempMask_y[x] = 0xff;
            }
        }
    }

    Ipvm::Image8u& imageMask = m_surfacePara->m_SurfaceBitmapMask;
    imageMask.Create(imageMaskTemp.GetSizeX(), imageMaskTemp.GetSizeY());
    Ipvm::ImageProcessing::Copy(imageMaskTemp, Ipvm::Rect32s(imageMaskTemp), imageMask);

    auto* maskLayer = getReusableMemory().AddSurfaceLayerMaskClass(m_strModuleName);
    if (maskLayer)
    {
        maskLayer->CopyFrom(imageMask);
        //mc_Surface Teach Mask관련[Teaching UI에서 Mask가 보일수 있어야한다 Para에 저장되는 Image를 Memory상에서만 등록하는 것이니 함부로 사용하지 말자]
        m_surfacePara->m_SurfaceBitmapMask.Free();
        m_surfacePara->m_SurfaceBitmapMask = maskLayer->GetMask();
    }

    return 0;
}

const VisionDebugInfo* VisionInspectionSurface::FindVisionDebugInfo(
    const std::vector<VisionDebugInfo*>& i_infos, LPCTSTR name)
{
    for (long i = 0; i < (long)(i_infos.size()); i++)
    {
        if (i_infos[i]->strDebugInfoName == name)
        {
            VisionDebugInfo* pDebugInfo = i_infos[i];
            if (pDebugInfo != NULL && pDebugInfo->pData != NULL && pDebugInfo->nDataNum > 0)
            {
                return pDebugInfo;
            }
        }
    }

    return nullptr;
}

void VisionInspectionSurface::SetPass_TotalResult()
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szSurfaceInspectionName);
    if (pResult == nullptr)
        return;
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == nullptr)
        return;

    pResult->Resize(1);

    pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("0"), _T(""), _T(""), 0, *pSpec, 0.f, 0.f, 0.f, PASS); //kircheis_VSV

    pResult->m_totalResult = PASS;
}

bool VisionInspectionSurface::GetSurfaceLayerImage(LPCTSTR name, Ipvm::Image8u& io_image)
{
    if (getReusableMemory().GetSurfaceLayerMask(name))
    {
        io_image = *getReusableMemory().GetSurfaceLayerMask(name);
        return true;
    }

    if (getReusableMemory().GetSurfaceLayerRoiClass(name))
    {
        if (!getReusableMemory().GetInspByteImage(io_image))
        {
            return false;
        }

        io_image.FillZero();
        auto* layerInfo = getReusableMemory().GetSurfaceLayerRoiClass(name);
        layerInfo->AddToMask(0, 0, io_image);

        return true;
    }

    return false;
}

bool VisionInspectionSurface::AnalisysBlobInfo(
    BOOL bUseBlobCount, BOOL bUseGVCheker, BOOL bDetailSetupMode) //kircheis_MED5_7_9
{
    if (bUseBlobCount == FALSE && bUseGVCheker == FALSE) //둘다 안하는 옵션이면 함수 실행 안함
    {
        if (bDetailSetupMode == TRUE)
        {
            bUseBlobCount = TRUE;
            bUseGVCheker = TRUE;
        }
        else
        {
            return true;
        }
    }

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && GetSurfaceAlgoPara()->nUse3DImageType == SURFACE_3D_USE_ZMAP) //3D의 높이로 하는 Surface면 함수 실행 안함
        return true;

    const long nBlobNum = m_nBlobNum;
    const auto& SurfaceItemSpec = m_surfacePara->m_SurfaceItem;

    if (nBlobNum <= 0) //검출된 Blob이 없으면 이 함수 의미 없다.
        return true;

    Ipvm::Image8u imageCurrent = GetInspectionFrameImage(); //GV 확인용 현재 설정 프레임
    Ipvm::Image32s imageBinaryLabel; //BlobCount 확인용 영상
    std::vector<BYTE> vecbyImageBinary(0); //imageBinary에 할당할 영상
    std::vector<Ipvm::BlobInfo> vecBlobInfo(0); //Merged Blob Count를 위한 BlobInfo

    const long nImageSizeX = imageCurrent.GetSizeX();
    const long nImageSizeY = imageCurrent.GetSizeY();
    const long nSize = nImageSizeX * nImageSizeY;

    if (bUseBlobCount == TRUE) //Blob Count를 확인할 때만 메모리를 확보한다.
    {
        if (!getReusableMemory().GetInspLongImage(imageBinaryLabel))
            return false;
        imageBinaryLabel.FillZero();
        vecbyImageBinary.resize(nSize);
        vecBlobInfo.resize(5000);
    }

    long nCurPixelLabel(0);

    ////검사 결과 저장용 변수. 추후 멤버 변수화 해서 관리해야 한다. //Surface Result는 기본적으로 float로 관리되니까 정수형 결과 라도 float로 관리하자
    //std::vector<float> vecfMergedBlobCount(nBlobNum);
    //std::vector<float> vecfGV_AVR(nBlobNum);
    //std::vector<float> vecfGV_Min(nBlobNum);
    //std::vector<float> vecfGV_Max(nBlobNum);

    for (long nBlob = 0; nBlob < nBlobNum; nBlob++)
    {
        const Ipvm::Rect32s rtBlob = m_psObjInfo_[nBlob].m_roi; //현재 Blob의 ROI 가져오고
        const long nCurBlobLabel = m_psObjInfo_[nBlob].m_label; //현재 Blob의 Label 정보 가져오고

        std::vector<BYTE> vecbyOriginImageData(0); //원본영상의 GV 수집할 버퍼
        long nIndex(0), nIndexY(0);
        BYTE bMin(255), bMax(0), bGV(0);

        if (bUseBlobCount == TRUE) //Blob Count를 확인할 때만 메모리를 클리어한다.
        {
            memset(&vecbyImageBinary[0], 0, nSize);
        }

        for (long nY = rtBlob.m_top; nY < rtBlob.m_bottom; nY++)
        {
            nIndexY = nImageSizeX * nY;
            for (long nX = rtBlob.m_left; nX < rtBlob.m_right; nX++)
            {
                nIndex = nIndexY + nX;

                nCurPixelLabel = (*m_plLabel_)[nY][nX]; //해당 좌표의 Label 가져와서
                if (nCurBlobLabel == nCurPixelLabel) //현재 Pixel의 Label이 지금 확인 중인 Blob의 Label과 같으면
                {
                    if (bUseGVCheker == TRUE) //GV 계산 옵션이다
                    {
                        bGV = imageCurrent[nY][nX]; //해당 좌표의 GV 가져와서
                        vecbyOriginImageData.push_back(bGV); //Data 모으고
                        bMin = (BYTE)min(bMin, bGV); // Min/Max 계산하고
                        bMax = (BYTE)max(bMax, bGV);
                    }

                    if (bUseBlobCount == TRUE) //Blob Count를 확인해야하면
                        vecbyImageBinary[nIndex] = 255; //해당 좌표를 255로 만든다
                }
            }
        }

        if (bUseGVCheker == TRUE)
        {
            double dMean = 0.;
            int nDataNum = (int)vecbyOriginImageData.size(); //GV 수집용 버퍼의 데이터 개수 파악하고
            Ipvm::Rect32s rtData(0, 0, nDataNum, 1); //ImageProcessing 돌릴거니까 데이터 개수 만큼 ROI 만들고
            Ipvm::Image8u imageOriginImageData(
                nDataNum, 1, &vecbyOriginImageData[0], nDataNum); //Vector BYTE를 Image_8u_C1으로 만들어서
            Ipvm::ImageProcessing::GetMean(imageOriginImageData, rtData, dMean); //평균을 구한다

            if (bDetailSetupMode || SurfaceItemSpec.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_BlobGV_AVR))
            {
                m_blobAttributes[nBlob].m_fBlobGV_AVR = (float)dMean;
            }
            if (bDetailSetupMode || SurfaceItemSpec.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_BlobGV_Min))
            {
                m_blobAttributes[nBlob].m_fBlobGV_Min = (float)bMin;
            }
            if (bDetailSetupMode || SurfaceItemSpec.IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_BlobGV_Max))
            {
                m_blobAttributes[nBlob].m_fBlobGV_Max = (float)bMax;
            }

            imageOriginImageData.Free();
            vecbyOriginImageData.clear();
        }

        if (bUseBlobCount == TRUE)
        {
            Ipvm::Image8u imageBinary(
                nImageSizeX, nImageSizeY, &vecbyImageBinary[0], nImageSizeX); //BlobCount 확인용 영상을 8u_C1으로 만들고
            Ipvm::ImageProcessing::Fill(rtBlob, 0, imageBinaryLabel); // Blob을 위한 Label 버퍼 초기화 하고
            int32_t nBlobCount(0);
            //Blob하고 카운트 세서 결과에 집어 넣을 것. BlobInfo는 미리 선언했음
            m_pBlob->DoBlob(imageBinary, rtBlob, 5000, imageBinaryLabel, m_psMaskInfo_, nBlobCount, 1);

            m_blobAttributes[nBlob].m_fMergedCount = (float)nBlobCount;
        }
    }
    imageBinaryLabel.Free();
    vecbyImageBinary.clear();

    return true;
}