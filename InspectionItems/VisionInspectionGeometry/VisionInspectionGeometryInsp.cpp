//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionGeometry.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionGeometry.h"
#include "VisionInspectionGeometryPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionGeometry::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionGeometry::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck fTime;

    ResetResult();

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    // 영훈 20160612 : Align Info를 모아온다.
    if (!GetAlignInfoData())
        return FALSE;

    BOOL bInspResult = TRUE;

    if (SystemConfig::GetInstance().m_nVisionType == VISIONTYPE_SIDE_INSP
        && m_VisionPara->m_nGeometryInspType != eGeometryInspType::GeometryInspType_Both)
    {
        const long InspType = m_VisionPara->m_nGeometryInspType;
        if ((InspType == eGeometryInspType::GeometryInspType_Front_Vision
                && i_ModuleStatus == enSideVisionModule::SIDE_VISIONMODULE_REAR)
            || (InspType == eGeometryInspType::GeometryInspType_Rear_Vision
                && i_ModuleStatus == enSideVisionModule::SIDE_VISIONMODULE_FRONT))
        {
            bInspResult = SetPassResultGeometryForSide();

            m_fCalcTime = CAST_FLOAT(fTime.Elapsed_ms());

            return bInspResult;
        }
    }

    if (!DoAlign())
    {
        bInspResult = FALSE;
        m_bInvalid = TRUE;
    }

    SetDebugInfo(detailSetupMode);

    m_fCalcTime = CAST_FLOAT(fTime.Elapsed_ms());

    return bInspResult;
}

// 불러온 Align 정보와 이름을 저장한다.
BOOL VisionInspectionGeometry::GetAlignInfo(std::vector<CString>& vecstrUseAlignName)
{
    vecstrUseAlignName.clear();

    BOOL bEdgeAlignInfo = FALSE;

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
        bEdgeAlignInfo = TRUE;
        vecstrUseAlignName.push_back(_T("EDGE Align Result"));
    }
    else
        return FALSE;

    nDataNum = 0;
    pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_OTHER_ALIGN, _T("Detect Rect"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        m_frtOtherDetectROI = *(Ipvm::Rect32r*)pData;
        vecstrUseAlignName.push_back(_T("Rect Object Align Result"));
    }

    nDataNum = 0;
    pData = m_visionUnit.GetVisionDebugInfo(_T(""), _T("Round Circle"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        m_pfrtRoundDetectROI = (Ipvm::Rect32r*)pData;
        vecstrUseAlignName.push_back(_T("Round Circle"));
    }

    return TRUE;
}

BOOL VisionInspectionGeometry::DoAlign()
{
    Ipvm::Image8u dstImage;

    if (!getReusableMemory().GetInspByteImage(dstImage))
        return FALSE;

    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return FALSE;

    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_ImageProcMangePara, dstImage))
    {
        return FALSE;
    }

    m_vecfptlineData_Start.clear();
    m_vecfptlineData_End.clear();

    std::vector<float> vecfError;
    std::vector<Ipvm::Rect32s> vecrtROI;
    BOOL bInspResult = TRUE;
    long nSpecNum = (long)m_variableInspectionSpecs.size();

    m_vecAlignInfoData_Ref.clear();
    m_vecAlignInfoData_Ref.resize(nSpecNum);
    m_vecAlignInfoData_Tar.clear();
    m_vecAlignInfoData_Tar.resize(nSpecNum);

    for (long nSpecID = 0; nSpecID < nSpecNum; nSpecID++)
    {
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(m_variableInspectionSpecs[nSpecID].m_specName);
        if (pResult == NULL)
            return FALSE;

        VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == NULL)
            return FALSE;

        if (pSpec->m_use == FALSE)
            continue;

        if (nSpecID < 0 || nSpecID >= long(m_VisionPara->m_vecstrinfoName_Ref.size()))
            return FALSE;
        if (nSpecID < 0 || nSpecID >= long(m_VisionPara->m_vecstrinfoName_Tar.size()))
            return FALSE;

        // Reference Parameter 계산
        m_AlignInfoData_Ref.Init();
        if (!DoInspectoin_Align_Ref_Tar(m_AlignInfoData_Ref, m_VisionPara->m_vecstrinfoName_Ref[nSpecID],
                m_VisionPara->m_vecrtInspectionROI_Ref_BCU[nSpecID], m_VisionPara->m_vecInfoParameter_0_Ref[nSpecID],
                dstImage))
        {
            pResult->Resize(1);
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, _T("0"), _T(""), _T(""), Ipvm::k_noiseValue32r, *pSpec, 0.f, 0.f, 0.f, true);
            pResult->UpdateTypicalValue(pSpec);
            pResult->SetTotalResult();
            bInspResult = FALSE;

            continue;
        }

        // Target Parameter 계산
        // 영훈 Target은 없을 수도 있다. 그냥 진행한다.
        m_AlignInfoData_Tar.Init();
        DoInspectoin_Align_Ref_Tar(m_AlignInfoData_Tar, m_VisionPara->m_vecstrinfoName_Tar[nSpecID],
            m_VisionPara->m_vecrtInspectionROI_Tar_BCU[nSpecID], m_VisionPara->m_vecInfoParameter_0_Tar[nSpecID],
            dstImage);

        // Ref 와 Target의 align 정보를 이용해 최종 계산 진행
        if (!DoInspectoin_Final(pSpec, pResult, m_VisionPara->m_vecnInspectionType[nSpecID],
                m_VisionPara->m_vecfReferenceSpec[nSpecID], m_VisionPara->m_vecnInspectionDistanceResult[nSpecID],
                m_VisionPara->m_vecnInspectionCircleResult[nSpecID], vecfError, vecrtROI))
        {
            pResult->Resize(1);
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, _T("0"), _T(""), _T(""), Ipvm::k_noiseValue32r, *pSpec, 0.f, 0.f, 0.f, true);
            pResult->UpdateTypicalValue(pSpec);
            pResult->SetTotalResult();
            bInspResult = FALSE;

            continue;
        }

        m_vecAlignInfoData_Ref[nSpecID] = m_AlignInfoData_Ref;
        m_vecAlignInfoData_Tar[nSpecID] = m_AlignInfoData_Tar;

        if (pResult->m_totalResult == INVALID)
        {
            bInspResult = FALSE;
        }
    }

    return bInspResult;
}

BOOL VisionInspectionGeometry::DoInspectoin_Align_Ref_Tar(SAlignInfoDataList& AlignInfoList,
    std::vector<CString> strvecinfoName, const std::vector<Ipvm::Rect32r>& vecrtSearchROI_BCU,
    std::vector<std::vector<long>> vecInfoParameter_0, const Ipvm::Image8u& image)
{
    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    long nInfoNameCount = (long)strvecinfoName.size();
    long nAlignInfoCount = (long)m_vecAlignInfoList.size();

    if (nInfoNameCount <= 0)
        return FALSE;

    CString strInspeName;

    std::vector<Ipvm::Rect32s> vecrtSearchROI;
    vecrtSearchROI.resize(vecrtSearchROI_BCU.size());
    for (long n = 0; n < long(vecrtSearchROI_BCU.size()); n++)
    {
        vecrtSearchROI[n] = getScale().convert_BCUToPixel(vecrtSearchROI_BCU[n], imageCenter);
    }

    if (nInfoNameCount == 1)
    {
        strInspeName = strvecinfoName[0];

        // 현재 등록된 파라미터 이름을 통해 Align 정보를 얻어온다.
        for (long nAlignID = 0; nAlignID < nAlignInfoCount; nAlignID++)
        {
            if (m_vecAlignInfoList[nAlignID].strName == strInspeName)
            {
                AlignInfoList = m_vecAlignInfoList[nAlignID];
                break;
            }
        }

        if (!GetAlignData_Line_And_Point(AlignInfoList, vecInfoParameter_0, vecrtSearchROI, 0, image))
        {
            return FALSE;
        }
    }
    else
    {
        for (long nSpecID = 0; nSpecID < nInfoNameCount; nSpecID++)
        {
            strInspeName = strvecinfoName[nSpecID];

            if (AlignInfoList.strName != strInspeName)
            {
                // 현재 등록된 파라미터 이름을 통해 Align 정보를 얻어온다.
                for (long nAlignID = 0; nAlignID < nAlignInfoCount; nAlignID++)
                {
                    if (m_vecAlignInfoList[nAlignID].strName == strInspeName)
                    {
                        AlignInfoList = m_vecAlignInfoList[nAlignID];
                        break;
                    }
                }
            }

            if (!GetAlignData_Line_And_Point(AlignInfoList, vecInfoParameter_0, vecrtSearchROI, nSpecID, image))
            {
                return FALSE;
            }
        }

        AlignInfoList.vecfptData.clear();
        AlignInfoList.vecfptData.resize(1);

        Ipvm::Geometry::GetCrossPoint(AlignInfoList.LineData_F, AlignInfoList.LineData_S, AlignInfoList.vecfptData[0]);

        AlignInfoList.nDataType = DataTypeList::Point;
    }

    strInspeName.Empty();

    return TRUE;
}

BOOL VisionInspectionGeometry::DoInspectoin_Final(VisionInspectionSpec* pSpec, VisionInspectionResult* pResult,
    long nInspectionType, float fReferenceSpec, long nDistanceResult, long nCircleResult, std::vector<float>& vecfError,
    std::vector<Ipvm::Rect32s>& vecrtROI)
{
    m_resultFinalCheckError_reference.Empty();
    m_resultFinalCheckError_target.Empty();

    vecfError.clear();
    vecrtROI.clear();

    BOOL success = FALSE;

    switch (nInspectionType)
    {
        case Insp_Type_Shape_Distance:
            success = DoInspectoin_Final_Distance(nDistanceResult, vecfError, vecrtROI);
            break;

        case Insp_Type_Shape_Circle:
            success = DoInspectoin_Final_Circle(nCircleResult, vecfError, vecrtROI);
            break;

        case Insp_Type_Shape_Angle:
            success = DoInspectoin_Final_Angle(vecfError, vecrtROI);
            break;
    }

    if (!success)
    {
        return FALSE;
    }

    float fError = 0.f;
    long nDataCount = (long)vecfError.size();

    for (long nID = 0; nID < nDataCount; nID++)
    {
        fError = vecfError[nID] - fReferenceSpec;
        vecfError[nID] = fError;
    }

    Ipvm::Rect32s rtROI;
    if (pSpec != NULL && pResult != NULL)
    {
        long nStartID(0), nEndID(0);

        nStartID = 0;
        nEndID = nDataCount;
        pResult->Resize(nDataCount);
        CString sObjectID = NULL;

        for (long nID = 0; nID < nDataCount; nID++)
        {
            fError = vecfError[nID];
            sObjectID.Format(_T("%d"), nID + nStartID);
            if (!success)
                fError = Ipvm::k_noiseValue32r;

            if (vecrtROI.size() > nID) //kircheis_Crash
            {
                rtROI = vecrtROI[nID];
                if (rtROI.Width() < 2)
                    rtROI.InflateRect(1, 0);
                if (rtROI.Height() < 2)
                    rtROI.InflateRect(0, 1);
                pResult->SetRect(nID + nStartID, rtROI);
            }

            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                nID + nStartID, sObjectID, _T(""), _T(""), fError, *pSpec, fReferenceSpec, 0.f, 0.f);
            if (!success)
            {
                //kircheis_VSV
                pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    nID + nStartID, sObjectID, _T(""), _T(""), fError, *pSpec, 0.f, 0.f, fReferenceSpec, INVALID);
            }
            else
            {
                pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    nID + nStartID, sObjectID, _T(""), _T(""), fError, *pSpec, 0.f, 0.f, fReferenceSpec);
            }
        }

        pResult->UpdateTypicalValue(pSpec);
        pResult->SetTotalResult();

        sObjectID.Empty();
    }

    return TRUE;
}

BOOL VisionInspectionGeometry::DoInspectoin_Final_Distance(
    long nDistanceResult, std::vector<float>& vecfError, std::vector<Ipvm::Rect32s>& vecrtROI)
{
    Ipvm::Rect32s rtROI{};
    float fValue = Ipvm::k_noiseValue32r;
    Ipvm::Point32r2 fptBegin_Ref{};
    Ipvm::Point32r2 fptCenter_Ref{};
    Ipvm::Point32r2 ptEnd_Ref{};
    Ipvm::Point32r2 fptBegin_Tar{};
    Ipvm::Point32r2 fptCenter_Tar{};
    Ipvm::Point32r2 ptEnd_Tar{};

    SAlignInfoDataList* AlignInfoData_0 = NULL;
    SAlignInfoDataList* AlignInfoData_1 = NULL;

    BOOL success = TRUE;

    // 둘다 Circle 타입일 경우
    if ((m_AlignInfoData_Ref.strName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
            || m_AlignInfoData_Tar.strName == g_szAlignInfo_List_Name[Insp_Type_User_Round])
        && (m_AlignInfoData_Ref.strName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips]
            || m_AlignInfoData_Tar.strName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips]))
    {
        fValue = CPI_Geometry::GetDistance_PointToPoint(m_AlignInfoData_Ref.CircleData.m_x,
            m_AlignInfoData_Ref.CircleData.m_y, m_AlignInfoData_Tar.CircleData.m_x, m_AlignInfoData_Tar.CircleData.m_y);
        vecfError.push_back(fValue * m_fPixelToUm);

        rtROI.m_left = (long)m_AlignInfoData_Ref.CircleData.m_x;
        rtROI.m_top = (long)m_AlignInfoData_Ref.CircleData.m_y;
        rtROI.m_right = (long)m_AlignInfoData_Tar.CircleData.m_x;
        rtROI.m_bottom = (long)m_AlignInfoData_Tar.CircleData.m_x;
        rtROI.NormalizeRect();
        vecrtROI.push_back(rtROI);
    }
    // 하나만 Circle 타입일 경우
    else if (m_AlignInfoData_Ref.strName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
        || m_AlignInfoData_Ref.strName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
    {
        if (m_AlignInfoData_Ref.nDataType == DataTypeList::Point
            && m_AlignInfoData_Tar.nDataType == DataTypeList::Point)
        {
            if (nDistanceResult == Insp_Param_Dist_XY)
            {
                fValue = CPI_Geometry::GetDistance_PointToPoint(m_AlignInfoData_Ref.CircleData.m_x,
                    m_AlignInfoData_Ref.CircleData.m_y, m_AlignInfoData_Tar.CircleData.m_x,
                    m_AlignInfoData_Tar.CircleData.m_y);
            }
            else if (nDistanceResult == Insp_Param_Dist_X)
            {
                fValue = CAST_FLOAT(fabs(m_AlignInfoData_Ref.CircleData.m_x - m_AlignInfoData_Tar.CircleData.m_x));
            }
            else if (nDistanceResult == Insp_Param_Dist_Y)
            {
                fValue = CAST_FLOAT(fabs(m_AlignInfoData_Ref.CircleData.m_y - m_AlignInfoData_Tar.CircleData.m_y));
            }

            vecfError.push_back(fValue * m_fPixelToUm);

            //			if(m_bDetailSetupMode)
            {
                rtROI.m_left = (long)m_AlignInfoData_Ref.CircleData.m_x;
                rtROI.m_top = (long)m_AlignInfoData_Ref.CircleData.m_y;
                rtROI.m_right = (long)m_AlignInfoData_Tar.CircleData.m_x;
                rtROI.m_bottom = (long)m_AlignInfoData_Tar.CircleData.m_x;
                rtROI.NormalizeRect();
                vecrtROI.push_back(rtROI);
            }
        }
        else if (m_AlignInfoData_Ref.nDataType == DataTypeList::Line
            && m_AlignInfoData_Tar.nDataType == DataTypeList::Point)
        {
            fValue = CPI_Geometry::GetDistance_PointToPoint(m_AlignInfoData_Ref.CircleData.m_x,
                m_AlignInfoData_Ref.CircleData.m_y, m_AlignInfoData_Tar.vecfptData[0].m_x,
                m_AlignInfoData_Tar.vecfptData[0].m_y);
            fValue -= m_AlignInfoData_Ref.CircleData.m_radius;
            vecfError.push_back(fValue * m_fPixelToUm);

            //			if(m_bDetailSetupMode)
            {
                rtROI.m_left = (long)m_AlignInfoData_Ref.CircleData.m_x;
                rtROI.m_top = (long)m_AlignInfoData_Ref.CircleData.m_y;
                rtROI.m_right = (long)m_AlignInfoData_Tar.vecfptData[0].m_x;
                rtROI.m_bottom = (long)m_AlignInfoData_Tar.vecfptData[0].m_y;
                rtROI.NormalizeRect();
                vecrtROI.push_back(rtROI);
            }
        }
        else if (m_AlignInfoData_Ref.nDataType == DataTypeList::Point
            && m_AlignInfoData_Tar.nDataType == DataTypeList::Line)
        {
            fValue = CPI_Geometry::GetDistance_PointToLine(
                m_AlignInfoData_Ref.CircleData.m_x, m_AlignInfoData_Ref.CircleData.m_y, m_AlignInfoData_Tar.LineData_F);
            Ipvm::Geometry::GetFootOfPerpendicular(m_AlignInfoData_Tar.LineData_F,
                Ipvm::Point32r2(m_AlignInfoData_Ref.CircleData.m_x, m_AlignInfoData_Ref.CircleData.m_y),
                m_AlignInfoData_Tar.vecfptData[0]);
            vecfError.push_back(fValue * m_fPixelToUm);

            //		if(m_bDetailSetupMode)
            {
                rtROI.m_left = (long)m_AlignInfoData_Ref.CircleData.m_x;
                rtROI.m_top = (long)m_AlignInfoData_Ref.CircleData.m_y;
                rtROI.m_right = (long)m_AlignInfoData_Tar.vecfptData[0].m_x;
                rtROI.m_bottom = (long)m_AlignInfoData_Tar.vecfptData[0].m_y;
                rtROI.NormalizeRect();
                vecrtROI.push_back(rtROI);
            }
        }
        else
        {
            return FALSE;
        }
    }
    // 둘다 직선 타입일 경우
    else
    {
        if (m_AlignInfoData_Ref.nDataType == DataTypeList::Point
            && m_AlignInfoData_Tar.nDataType == DataTypeList::Point) // 포인트 간의 거리
        {
            if (nDistanceResult == Insp_Param_Dist_XY)
            {
                fValue = CPI_Geometry::GetDistance_PointToPoint(m_AlignInfoData_Ref.vecfptData[0].m_x,
                    m_AlignInfoData_Ref.vecfptData[0].m_y, m_AlignInfoData_Tar.vecfptData[0].m_x,
                    m_AlignInfoData_Tar.vecfptData[0].m_y);
            }
            else if (nDistanceResult == Insp_Param_Dist_X)
            {
                fValue
                    = CAST_FLOAT(fabs(m_AlignInfoData_Ref.vecfptData[0].m_x - m_AlignInfoData_Tar.vecfptData[0].m_x));
            }
            else if (nDistanceResult == Insp_Param_Dist_Y)
            {
                fValue
                    = CAST_FLOAT(fabs(m_AlignInfoData_Ref.vecfptData[0].m_y - m_AlignInfoData_Tar.vecfptData[0].m_y));
            }

            vecfError.push_back(fValue * m_fPixelToUm);

            //		if(m_bDetailSetupMode)
            {
                rtROI.m_left = (long)m_AlignInfoData_Ref.vecfptData[0].m_x;
                rtROI.m_top = (long)m_AlignInfoData_Ref.vecfptData[0].m_y;
                rtROI.m_right = (long)m_AlignInfoData_Tar.vecfptData[0].m_x;
                rtROI.m_bottom = (long)m_AlignInfoData_Tar.vecfptData[0].m_y;
                rtROI.NormalizeRect();
                vecrtROI.push_back(rtROI);
            }
        }
        else if ((m_AlignInfoData_Ref.nDataType == DataTypeList::Point
                     && m_AlignInfoData_Tar.nDataType == DataTypeList::Line)
            || (m_AlignInfoData_Ref.nDataType == DataTypeList::Line
                && m_AlignInfoData_Tar.nDataType == DataTypeList::Point)) // 포인트와 라인의 거리
        {
            if (m_AlignInfoData_Ref.nDataType == DataTypeList::Point
                && m_AlignInfoData_Tar.nDataType == DataTypeList::Line)
            {
                AlignInfoData_0 = &m_AlignInfoData_Ref;
                AlignInfoData_1 = &m_AlignInfoData_Tar;
            }
            else
            {
                AlignInfoData_0 = &m_AlignInfoData_Tar;
                AlignInfoData_1 = &m_AlignInfoData_Ref;
            }

            fValue = CPI_Geometry::GetDistance_PointToLine(
                AlignInfoData_0->vecfptData[0].m_x, AlignInfoData_0->vecfptData[0].m_y, AlignInfoData_1->LineData_F);

            Ipvm::Geometry::GetFootOfPerpendicular(
                AlignInfoData_1->LineData_F, AlignInfoData_0->vecfptData[0], AlignInfoData_1->vecfptData[0]);
            vecfError.push_back(fValue * m_fPixelToUm);

            //		if(m_bDetailSetupMode)
            {
                rtROI.m_left = (long)AlignInfoData_0->vecfptData[0].m_x;
                rtROI.m_top = (long)AlignInfoData_0->vecfptData[0].m_y;
                rtROI.m_right = (long)AlignInfoData_1->vecfptData[0].m_x;
                rtROI.m_bottom = (long)AlignInfoData_1->vecfptData[0].m_y;
                rtROI.NormalizeRect();
                vecrtROI.push_back(rtROI);
            }
        }
        else if ((m_AlignInfoData_Ref.nDataType == DataTypeList::Line
                     && m_AlignInfoData_Tar.nDataType == DataTypeList::All_Point)
            || (m_AlignInfoData_Ref.nDataType == DataTypeList::All_Point
                && m_AlignInfoData_Tar.nDataType == DataTypeList::Line)) // 라인과 여러개의 포인트 거리
        {
            // 0 : Line,  1 : Point
            if ((m_AlignInfoData_Ref.nDataType == DataTypeList::Line
                    && m_AlignInfoData_Tar.nDataType == DataTypeList::All_Point))
            {
                AlignInfoData_0 = &m_AlignInfoData_Ref;
                AlignInfoData_1 = &m_AlignInfoData_Tar;
            }
            else if (m_AlignInfoData_Ref.nDataType == DataTypeList::All_Point
                && m_AlignInfoData_Tar.nDataType == DataTypeList::Line)
            {
                AlignInfoData_0 = &m_AlignInfoData_Tar;
                AlignInfoData_1 = &m_AlignInfoData_Ref;
            }

            long nEdgeCount = (long)AlignInfoData_1->vecfptData.size();

            if (nEdgeCount > 0)
            {
                vecfError.reserve(nEdgeCount);

                float fMaxError = 0.f;
                long nMaxID = 0;
                Ipvm::Point32r2 fptProjPoint;

                for (long nID = 0; nID < nEdgeCount; nID++)
                {
                    fValue = CPI_Geometry::GetDistance_PointToLine(AlignInfoData_1->vecfptData[nID].m_x,
                        AlignInfoData_1->vecfptData[nID].m_y, AlignInfoData_0->LineData_F);
                    vecfError.push_back(fValue * m_fPixelToUm);

                    //			if(m_bDetailSetupMode)
                    {
                        Ipvm::Geometry::GetFootOfPerpendicular(
                            AlignInfoData_0->LineData_F, AlignInfoData_1->vecfptData[nID], fptProjPoint);
                        rtROI.m_left = (long)AlignInfoData_1->vecfptData[nID].m_x;
                        rtROI.m_top = (long)AlignInfoData_1->vecfptData[nID].m_y;
                        rtROI.m_right = (long)fptProjPoint.m_x;
                        rtROI.m_bottom = (long)fptProjPoint.m_y;
                        rtROI.NormalizeRect();
                        vecrtROI.push_back(rtROI);
                    }

                    if (fMaxError < fValue)
                    {
                        fMaxError = fValue;
                        nMaxID = nID;
                    }
                }

                AlignInfoData_1->vecfptData[0] = AlignInfoData_1->vecfptData[nMaxID];
                Ipvm::Geometry::GetFootOfPerpendicular(
                    AlignInfoData_0->LineData_F, AlignInfoData_1->vecfptData[0], AlignInfoData_1->vecfptData[1]);
                AlignInfoData_1->vecfptData.resize(2);

                Ipvm::Point32r2 fptTemp(AlignInfoData_0->vecfptData[0].m_x, AlignInfoData_0->vecfptData[0].m_y);
                Ipvm::Geometry::GetFootOfPerpendicular(
                    AlignInfoData_0->LineData_F, fptTemp, AlignInfoData_0->vecfptData[0]);
                AlignInfoData_0->vecfptData[1] = AlignInfoData_1->vecfptData[1];
                AlignInfoData_0->vecfptData.resize(2);
            }
            else
            {
                vecfError.resize(1);
                vecfError[0] = Ipvm::k_noiseValue32r;
            }
        }
        else if (m_AlignInfoData_Ref.nDataType == DataTypeList::Line
            && m_AlignInfoData_Tar.nDataType == DataTypeList::Line) // 라인과 라인의 거리
        {
            // 라인의 시작, 중간, 끝 포인트를 찾아주는 함수( 찾은 라인 기울기 및 포인트들, 라인의 시작 포인트, 라인의 센터포인트, 라인의 끝 포인트 )
            Get_LineCenter_Point(m_AlignInfoData_Ref, fptBegin_Ref, fptCenter_Ref, ptEnd_Ref);
            Get_LineCenter_Point(m_AlignInfoData_Tar, fptBegin_Tar, fptCenter_Tar, ptEnd_Tar);

            // Ref와 Tar의 직선 기울기를 서로 평행 시키기 위해 기울기의 차이를 알아온다.
            float fLineAngle_Ref = CPI_Geometry::GetAngleBetween2Lines(
                fptBegin_Ref, fptCenter_Ref, Ipvm::Point32r2(fptCenter_Ref.m_x + 100.f, fptCenter_Ref.m_y));
            float fLineAngle_Tar = CPI_Geometry::GetAngleBetween2Lines(
                fptBegin_Tar, fptCenter_Tar, Ipvm::Point32r2(fptCenter_Tar.m_x + 100.f, fptCenter_Tar.m_y));

            BOOL invalid = FALSE;
            if (fLineAngle_Ref < -99998.9f)
                invalid = TRUE;
            if (fLineAngle_Tar < -99998.9f)
                invalid = TRUE;

            // 찾은 기울기를 Radian 값으로 변환
            float fAngelOffset = (fLineAngle_Ref - fLineAngle_Tar) * ITP_DEG_TO_RAD;

            // 찾아온 기울기만큼 Line을 회전시킨다.
            CPI_Geometry::RotatePoint(fptBegin_Tar, -fAngelOffset, fptCenter_Tar);
            CPI_Geometry::RotatePoint(ptEnd_Tar, -fAngelOffset, fptCenter_Tar);

            std::vector<Ipvm::Point32r2> vecfptPoints(3);
            Ipvm::LineEq32r line;

            vecfptPoints[0] = fptBegin_Tar;
            vecfptPoints[1] = fptCenter_Tar;
            vecfptPoints[2] = ptEnd_Tar;

            Ipvm::DataFitting::FitToLine(3, &vecfptPoints[0], line);

            if (nDistanceResult == Insp_Param_Dist_XY)
            {
                float fValue0 = CPI_Geometry::GetDistance_PointToLine(fptCenter_Ref.m_x, fptCenter_Ref.m_y, line);
                float fValue1 = CPI_Geometry::GetDistance_PointToLine(
                    fptCenter_Tar.m_x, fptCenter_Tar.m_y, m_AlignInfoData_Ref.LineData_F);

                fValue = (fValue0 + fValue1) * 0.5f;
                vecfError.push_back(fValue * m_fPixelToUm);

                //	if(m_bDetailSetupMode)
                if (!invalid)
                {
                    rtROI.m_left = (long)fptCenter_Ref.m_x;
                    rtROI.m_top = (long)fptCenter_Ref.m_y;
                    rtROI.m_right = (long)fptCenter_Tar.m_x;
                    rtROI.m_bottom = (long)fptCenter_Tar.m_y;
                    rtROI.NormalizeRect();
                    vecrtROI.push_back(rtROI);
                }
                else
                {
                    success = FALSE;
                }

                // Display 용..
                m_AlignInfoData_Ref.vecfptData.clear();
                m_AlignInfoData_Tar.vecfptData.clear();

                m_AlignInfoData_Ref.vecfptData.resize(1);
                m_AlignInfoData_Tar.vecfptData.resize(1);

                // 화면에 표시될 라인은 실제 거리가 구해진 방향으로 보이도록 한다.
                Ipvm::Geometry::GetFootOfPerpendicular(line, fptCenter_Ref, fptCenter_Tar);
            }
            else if (nDistanceResult == Insp_Param_Dist_X) //수평 거리
            {
                Ipvm::LineEq32r lineHor;

                vecfptPoints[0] = fptCenter_Ref + Ipvm::Point32r2(-100.f, 0.f);
                vecfptPoints[1] = fptCenter_Ref;
                vecfptPoints[2] = fptCenter_Ref + Ipvm::Point32r2(100.f, 0.f);

                Ipvm::DataFitting::FitToLine(3, &vecfptPoints[0], lineHor);

                Ipvm::Point32r2 crossPointInTarget;
                Ipvm::Geometry::GetCrossPoint(line, lineHor, crossPointInTarget);
                fptCenter_Tar = crossPointInTarget;
                fValue = (float)fabs(fptCenter_Tar.m_x - fptCenter_Ref.m_x);
                vecfError.push_back(fValue * m_fPixelToUm);

                if (!invalid)
                {
                    rtROI.m_left = (long)fptCenter_Ref.m_x;
                    rtROI.m_top = (long)fptCenter_Ref.m_y;
                    rtROI.m_right = (long)fptCenter_Tar.m_x;
                    rtROI.m_bottom = (long)fptCenter_Tar.m_y;
                    rtROI.NormalizeRect();
                    vecrtROI.push_back(rtROI);
                }
                else
                {
                    success = FALSE;
                }

                m_AlignInfoData_Ref.vecfptData.clear();
                m_AlignInfoData_Tar.vecfptData.clear();

                m_AlignInfoData_Ref.vecfptData.resize(1);
                m_AlignInfoData_Tar.vecfptData.resize(1);
            }
            else if (nDistanceResult == Insp_Param_Dist_Y)
            {
                Ipvm::LineEq32r lineVer;

                vecfptPoints[0] = fptCenter_Ref + Ipvm::Point32r2(0.f, -100.f);
                vecfptPoints[1] = fptCenter_Ref;
                vecfptPoints[2] = fptCenter_Ref + Ipvm::Point32r2(0.f, 100.f);

                Ipvm::DataFitting::FitToLine(3, &vecfptPoints[0], lineVer);

                Ipvm::Point32r2 crossPointInTarget;
                Ipvm::Geometry::GetCrossPoint(line, lineVer, crossPointInTarget);
                fptCenter_Tar = crossPointInTarget;
                fValue = (float)fabs(fptCenter_Tar.m_y - fptCenter_Ref.m_y);
                vecfError.push_back(fValue * m_fPixelToUm);

                if (!invalid)
                {
                    rtROI.m_left = (long)fptCenter_Ref.m_x;
                    rtROI.m_top = (long)fptCenter_Ref.m_y;
                    rtROI.m_right = (long)fptCenter_Tar.m_x;
                    rtROI.m_bottom = (long)fptCenter_Tar.m_y;
                    rtROI.NormalizeRect();
                    vecrtROI.push_back(rtROI);
                }
                else
                {
                    success = FALSE;
                }

                m_AlignInfoData_Ref.vecfptData.clear();
                m_AlignInfoData_Tar.vecfptData.clear();

                m_AlignInfoData_Ref.vecfptData.resize(1);
                m_AlignInfoData_Tar.vecfptData.resize(1);
            }
            else
                return FALSE;

            m_AlignInfoData_Ref.vecfptData[0] = fptCenter_Ref;
            m_AlignInfoData_Tar.vecfptData[0] = fptCenter_Tar;
        }
        else
        {
            return FALSE;
        }
    }

    return success;
}

BOOL VisionInspectionGeometry::DoInspectoin_Final_Circle(
    long nCircleResult, std::vector<float>& vecfError, std::vector<Ipvm::Rect32s>& vecrtROI)
{
    Ipvm::Rect32s rtROI;
    float fValue = Ipvm::k_noiseValue32r;

    long nEdgeCount = (long)m_AlignInfoData_Ref.vecfptData.size();
    if (nEdgeCount > 0)
    {
        Ipvm::DataFitting::FitToCircle(nEdgeCount, &m_AlignInfoData_Ref.vecfptData[0], m_AlignInfoData_Ref.CircleData);
    }
    else
    {
        ASSERT(!_T("??"));
        return FALSE;
    }

    if (nCircleResult == Insp_Param_Circle_Radius)
        fValue = m_AlignInfoData_Ref.CircleData.m_radius;
    else
        fValue = m_AlignInfoData_Ref.CircleData.m_radius * 2.f;

    vecfError.push_back(fValue * m_fPixelToUm);

    //	if(m_bDetailSetupMode)
    {
        rtROI.m_left = (long)m_AlignInfoData_Ref.CircleData.m_x - 5;
        rtROI.m_top = (long)m_AlignInfoData_Ref.CircleData.m_y - 5;
        rtROI.m_right = (long)m_AlignInfoData_Ref.CircleData.m_x + 5;
        rtROI.m_bottom = (long)m_AlignInfoData_Ref.CircleData.m_y + 5;
        rtROI.NormalizeRect();
        vecrtROI.push_back(rtROI);
    }

    return TRUE;
}

BOOL VisionInspectionGeometry::DoInspectoin_Final_Angle(
    std::vector<float>& vecfError, std::vector<Ipvm::Rect32s>& vecrtROI)
{
    Ipvm::Rect32s rtROI;
    float fValue = Ipvm::k_noiseValue32r;

    if (m_AlignInfoData_Ref.nDataType == DataTypeList::Line && m_AlignInfoData_Tar.nDataType == DataTypeList::Line)
    {
        Ipvm::Point32r2 fptCrossPoint;

        Ipvm::Geometry::GetCrossPoint(m_AlignInfoData_Ref.LineData_F, m_AlignInfoData_Tar.LineData_F, fptCrossPoint);

        fValue = CPI_Geometry::GetAngleBetween2Lines(
            m_AlignInfoData_Ref.fptCenterPoint, fptCrossPoint, m_AlignInfoData_Tar.fptCenterPoint);

        vecfError.push_back(fValue);

        //	if(m_bDetailSetupMode)
        {
            rtROI.m_left = (long)fptCrossPoint.m_x - 5;
            rtROI.m_top = (long)fptCrossPoint.m_y - 5;
            rtROI.m_right = (long)fptCrossPoint.m_x + 5;
            rtROI.m_bottom = (long)fptCrossPoint.m_y + 5;
            rtROI.NormalizeRect();
            vecrtROI.push_back(rtROI);
        }
    }
    else
    {
        if (m_AlignInfoData_Ref.nDataType != DataTypeList::Line)
        {
            m_resultFinalCheckError_reference = _T("Only 'Line' Type is available.");
        }

        if (m_AlignInfoData_Tar.nDataType != DataTypeList::Line)
        {
            m_resultFinalCheckError_target = _T("Only 'Line' Type is available.");
        }

        vecfError.push_back(Ipvm::k_noiseValue32r);
    }

    return TRUE;
}

BOOL VisionInspectionGeometry::GetAlignInfoData()
{
    m_vecAlignInfoList.clear();

    BOOL bValidBodyAlignInfo = FALSE;
    BOOL bValidOtherAlignInfo = FALSE;
    BOOL bValidRoundAlignInfo = FALSE;

    CString strName;
    std::vector<CString> vecstrUseAlignName;

    // Align Info 얻어오기
    GetAlignInfo(vecstrUseAlignName);

    // 불러오기에 성공한 Align 정보를 체크한다.
    long nAlignInfoNum = (long)vecstrUseAlignName.size();
    for (long nID = 0; nID < nAlignInfoNum; nID++)
    {
        strName = vecstrUseAlignName[nID];
        if (strName == _T("EDGE Align Result"))
        {
            bValidBodyAlignInfo = TRUE;
        }
        else if (strName == _T("Rect Object Align Result"))
        {
            bValidOtherAlignInfo = TRUE;
        }
        else if (strName == _T("Round Circle"))
        {
            bValidRoundAlignInfo = TRUE;
        }
    }

    // Body Align이 되지 않으면 아래 쪽은 할 필요가 없다. 바로 종료
    if (bValidBodyAlignInfo == FALSE)
        return FALSE;

    GetAlignInfoData_BodyAlign(*m_sEdgeAlignResult);

    if (bValidOtherAlignInfo)
    {
        GetAlignInfoData_OtherAlign(*m_sEdgeAlignResult, m_frtOtherDetectROI);
    }

    if (bValidRoundAlignInfo)
    {
        GetAlignInfoData_RoundAlign(*m_sEdgeAlignResult, m_pfrtRoundDetectROI);
    }

    GetAlignInfoData_UserAlign();

    strName.Empty();

    for (int nIdx = 0; nIdx < vecstrUseAlignName.size(); nIdx++)
    {
        vecstrUseAlignName[nIdx].Empty();
    }

    return TRUE;
}

// body Align 정보를 입력한다.
BOOL VisionInspectionGeometry::GetAlignInfoData_BodyAlign(VisionAlignResult sEdgeAlignResult)
{
    SAlignInfoDataList AlignInfoList;
    AlignInfoList.Init();

    // 데이터 개수
    AlignInfoList.vecfptData.resize(enum_Direction_End);

    // Body Align 정보 입력 받기
    float fDx1 = m_sEdgeAlignResult->fptLT.m_x - m_sEdgeAlignResult->fptRT.m_x;
    float fDy1 = m_sEdgeAlignResult->fptLT.m_y - m_sEdgeAlignResult->fptRT.m_y;
    float fL1 = CAST_FLOAT(sqrt(fDx1 * fDx1 + fDy1 * fDy1));

    // 위에서 Image가 회전 된 만큼 Body align 된 Point도 같이 회전시킨다.
    AlignInfoList.vecfptData[enum_Direction_LeftTop].m_x = m_sEdgeAlignResult->fptLT.m_x;
    AlignInfoList.vecfptData[enum_Direction_LeftTop].m_y = m_sEdgeAlignResult->fptLT.m_y;

    float fDx3 = m_sEdgeAlignResult->fptLB.m_x - m_sEdgeAlignResult->fptRB.m_x;
    float fDy3 = m_sEdgeAlignResult->fptLB.m_y - m_sEdgeAlignResult->fptRB.m_y;
    float fL3 = CAST_FLOAT(sqrt(fDx3 * fDx3 + fDy3 * fDy3));

    AlignInfoList.vecfptData[enum_Direction_LeftBottom].m_x = m_sEdgeAlignResult->fptLB.m_x;
    AlignInfoList.vecfptData[enum_Direction_LeftBottom].m_y = m_sEdgeAlignResult->fptLB.m_y;

    float fBodyHalfSizeX = 0.25f * (fL1 + fL3);

    float fDx2 = m_sEdgeAlignResult->fptRT.m_x - m_sEdgeAlignResult->fptRB.m_x;
    float fDy2 = m_sEdgeAlignResult->fptRT.m_y - m_sEdgeAlignResult->fptRB.m_y;
    float fL2 = CAST_FLOAT(sqrt(fDx2 * fDx2 + fDy2 * fDy2));

    AlignInfoList.vecfptData[enum_Direction_RightTop].m_x = m_sEdgeAlignResult->fptRT.m_x;
    AlignInfoList.vecfptData[enum_Direction_RightTop].m_y = m_sEdgeAlignResult->fptRT.m_y;

    float fDx4 = m_sEdgeAlignResult->fptLT.m_x - m_sEdgeAlignResult->fptLB.m_x;
    float fDy4 = m_sEdgeAlignResult->fptLT.m_y - m_sEdgeAlignResult->fptLB.m_y;
    float fL4 = CAST_FLOAT(sqrt(fDx4 * fDx4 + fDy4 * fDy4));

    AlignInfoList.vecfptData[enum_Direction_RightBottom].m_x = m_sEdgeAlignResult->fptRB.m_x;
    AlignInfoList.vecfptData[enum_Direction_RightBottom].m_y = m_sEdgeAlignResult->fptRB.m_y;

    float fBodyHalfSizeY = 0.25f * (fL2 + fL4);

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    Ipvm::Rect32s rt(int32_t(imageCenter.m_x - fBodyHalfSizeX + 0.5f), int32_t(imageCenter.m_y - fBodyHalfSizeY + 0.5f),
        int32_t(imageCenter.m_x + fBodyHalfSizeX + 0.5f), int32_t(imageCenter.m_y + fBodyHalfSizeY + 0.5f));

    // 화면에 표시될 이름 저장
    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_Body_Line];

    // 화면에 클릭 시 디스플레이에 표시될 ROI 저장
    AlignInfoList.rtROI = rt;

    if (m_packageSpec.m_bLowtopEdge)
    {
        AlignInfoList.vecfptData.resize(enum_Direction_LowTop_End);

        AlignInfoList.vecfptData[enum_Direction_LowTop_Left].m_x = m_sEdgeAlignResult->fptLowtopLT.m_x;
        AlignInfoList.vecfptData[enum_Direction_LowTop_Left].m_y = m_sEdgeAlignResult->fptLowtopLT.m_y;

        AlignInfoList.vecfptData[enum_Direction_LowTop_Right].m_x = m_sEdgeAlignResult->fptLowtopRT.m_x;
        AlignInfoList.vecfptData[enum_Direction_LowTop_Right].m_y = m_sEdgeAlignResult->fptLowtopRT.m_y;
    }

    // 퐈면에 표시될 데이터 개수
    AlignInfoList.nDataCount = BL_Param_End;

    // 현제 정의된 Align 정보가 Line인지 Point인지 정해준다.
    AlignInfoList.nDataType = DataTypeList::Line;

    // 정리된 데이터를 저장
    m_vecAlignInfoList.push_back(AlignInfoList);

    AlignInfoList.Init();

    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_Body_Center_Info];

    rt = Ipvm::Rect32s(int32_t(imageCenter.m_x - 2.f), int32_t(imageCenter.m_y - 2.f), int32_t(imageCenter.m_x + 2.f),
        int32_t(imageCenter.m_y + 2.f));

    AlignInfoList.rtROI = rt;

    AlignInfoList.vecfptData.resize(1);
    AlignInfoList.vecfptData[0] = Ipvm::Point32r2(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    AlignInfoList.nDataCount = 1;
    AlignInfoList.nDataType = DataTypeList::Point;

    m_vecAlignInfoList.push_back(AlignInfoList);

    return TRUE;
}

BOOL VisionInspectionGeometry::GetAlignInfoData_OtherAlign(
    VisionAlignResult sEdgeAlignResult, Ipvm::Rect32r frtOtherDetectROI)
{
    SAlignInfoDataList AlignInfoList;
    AlignInfoList.Init();

    AlignInfoList.vecfptData.resize(enum_Direction_End);

    // Body Align 정보 입력 받기
    AlignInfoList.vecfptData[enum_Direction_LeftTop].m_x = frtOtherDetectROI.m_left;
    AlignInfoList.vecfptData[enum_Direction_LeftTop].m_y = frtOtherDetectROI.m_top;

    AlignInfoList.vecfptData[enum_Direction_LeftBottom].m_x = frtOtherDetectROI.m_left;
    AlignInfoList.vecfptData[enum_Direction_LeftBottom].m_y = frtOtherDetectROI.m_bottom;

    AlignInfoList.vecfptData[enum_Direction_RightTop].m_x = frtOtherDetectROI.m_right;
    AlignInfoList.vecfptData[enum_Direction_RightTop].m_y = frtOtherDetectROI.m_top;

    AlignInfoList.vecfptData[enum_Direction_RightBottom].m_x = frtOtherDetectROI.m_right;
    AlignInfoList.vecfptData[enum_Direction_RightBottom].m_y = frtOtherDetectROI.m_bottom;

    float fOtherHalfSizeX = frtOtherDetectROI.Width() * 0.5f;
    float fOtherHalfSizeY = frtOtherDetectROI.Height() * 0.5f;

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    Ipvm::Rect32s rt(int32_t(imageCenter.m_x - fOtherHalfSizeX + 0.5f),
        int32_t(imageCenter.m_y - fOtherHalfSizeY + 0.5f), int32_t(imageCenter.m_x + fOtherHalfSizeX + 0.5f),
        int32_t(imageCenter.m_y + fOtherHalfSizeY + 0.5f));

    AlignInfoList.nDataType = DataTypeList::Line;
    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_Other_Line];
    AlignInfoList.rtROI = rt;

    AlignInfoList.nDataCount = enum_Direction_End;

    m_vecAlignInfoList.push_back(AlignInfoList);

    return TRUE;
}

BOOL VisionInspectionGeometry::GetAlignInfoData_RoundAlign(
    VisionAlignResult sEdgeAlignResult, Ipvm::Rect32r* pfrtRoundDetectROI)
{
    SAlignInfoDataList AlignInfoList;
    AlignInfoList.Init();

    AlignInfoList.nDataCount = 1;
    AlignInfoList.vecfptData.resize(AlignInfoList.nDataCount);

    // Round Info Left Top
    AlignInfoList.vecfptData[0].m_x = pfrtRoundDetectROI[Round_Dir_LT].CenterPoint().m_x;
    AlignInfoList.vecfptData[0].m_y = pfrtRoundDetectROI[Round_Dir_LT].CenterPoint().m_y;

    float fOtherHalfSizeX = pfrtRoundDetectROI[Round_Dir_LT].Width() * 0.5f;
    float fOtherHalfSizeY = pfrtRoundDetectROI[Round_Dir_LT].Height() * 0.5f;

    Ipvm::Rect32s rt = Ipvm::Conversion::ToRect32s(pfrtRoundDetectROI[Round_Dir_LT]);

    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_Round_LT_Info];
    AlignInfoList.rtROI = rt;

    AlignInfoList.nDataType = DataTypeList::Point;

    m_vecAlignInfoList.push_back(AlignInfoList);

    // Round Info Right Top
    AlignInfoList.vecfptData[0].m_x = pfrtRoundDetectROI[Round_Dir_RT].CenterPoint().m_x;
    AlignInfoList.vecfptData[0].m_y = pfrtRoundDetectROI[Round_Dir_RT].CenterPoint().m_y;

    fOtherHalfSizeX = pfrtRoundDetectROI[Round_Dir_RT].Width() * 0.5f;
    fOtherHalfSizeY = pfrtRoundDetectROI[Round_Dir_RT].Height() * 0.5f;

    rt = Ipvm::Conversion::ToRect32s(pfrtRoundDetectROI[Round_Dir_RT]);

    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_Round_RT_Info];
    AlignInfoList.rtROI = rt;

    m_vecAlignInfoList.push_back(AlignInfoList);

    // Round Info Right Bottom
    AlignInfoList.vecfptData[0].m_x = pfrtRoundDetectROI[Round_Dir_RB].CenterPoint().m_x;
    AlignInfoList.vecfptData[0].m_y = pfrtRoundDetectROI[Round_Dir_RB].CenterPoint().m_y;

    fOtherHalfSizeX = pfrtRoundDetectROI[Round_Dir_RB].Width() * 0.5f;
    fOtherHalfSizeY = pfrtRoundDetectROI[Round_Dir_RB].Height() * 0.5f;

    rt = Ipvm::Conversion::ToRect32s(pfrtRoundDetectROI[Round_Dir_RB]);

    // Round Info Left Bottom
    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_Round_RB_Info];
    AlignInfoList.rtROI = rt;

    m_vecAlignInfoList.push_back(AlignInfoList);

    AlignInfoList.vecfptData[0].m_x = pfrtRoundDetectROI[Round_Dir_LB].CenterPoint().m_x;
    AlignInfoList.vecfptData[0].m_y = pfrtRoundDetectROI[Round_Dir_LB].CenterPoint().m_y;

    fOtherHalfSizeX = pfrtRoundDetectROI[Round_Dir_LB].Width() * 0.5f;
    fOtherHalfSizeY = pfrtRoundDetectROI[Round_Dir_LB].Height() * 0.5f;

    rt = Ipvm::Conversion::ToRect32s(pfrtRoundDetectROI[Round_Dir_LB]);

    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_Round_LB_Info];
    AlignInfoList.rtROI = rt;

    m_vecAlignInfoList.push_back(AlignInfoList);

    return TRUE;
}

BOOL VisionInspectionGeometry::GetAlignInfoData_UserAlign()
{
    // Edge Point 관련 파라미터
    SAlignInfoDataList AlignInfoList;
    AlignInfoList.Init();

    AlignInfoList.nDataType = DataTypeList::All_Point;
    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_User_Edge_Detect];

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    Ipvm::Rect32s rt(int32_t(imageCenter.m_x - 50.f), int32_t(imageCenter.m_y - 50.f), int32_t(imageCenter.m_x + 50.f),
        int32_t(imageCenter.m_y + 50.f));

    AlignInfoList.rtROI = rt;

    AlignInfoList.nDataCount = ED_Param_End;

    m_vecAlignInfoList.push_back(AlignInfoList);

    // Edge Line 관련 파라미터
    AlignInfoList.Init();

    AlignInfoList.nDataType = DataTypeList::Line;
    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_User_Line];

    rt = Ipvm::Rect32s(int32_t(imageCenter.m_x - 50.f), int32_t(imageCenter.m_y - 50.f),
        int32_t(imageCenter.m_x + 50.f), int32_t(imageCenter.m_y + 50.f));

    AlignInfoList.rtROI = rt;

    AlignInfoList.nDataCount = ED_Param_End;

    m_vecAlignInfoList.push_back(AlignInfoList);

    // Round 관련 파라미터
    AlignInfoList.Init();

    AlignInfoList.nDataType = DataTypeList::Circle;
    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_User_Round];

    rt = Ipvm::Rect32s(int32_t(imageCenter.m_x - 50.f), int32_t(imageCenter.m_y - 50.f),
        int32_t(imageCenter.m_x + 50.f), int32_t(imageCenter.m_y + 50.f));

    AlignInfoList.rtROI = rt;

    AlignInfoList.nDataCount = ED_Param_End;

    m_vecAlignInfoList.push_back(AlignInfoList);

    // Circle 관련 파라미터
    AlignInfoList.Init();

    AlignInfoList.nDataType = DataTypeList::Circle;
    AlignInfoList.strName = g_szAlignInfo_List_Name[Insp_Type_User_Ellips];

    rt = Ipvm::Rect32s(int32_t(imageCenter.m_x - 50.f), int32_t(imageCenter.m_y - 50.f),
        int32_t(imageCenter.m_x + 50.f), int32_t(imageCenter.m_y + 50.f));

    AlignInfoList.rtROI = rt;

    AlignInfoList.nDataCount = ED_Param_End;

    m_vecAlignInfoList.push_back(AlignInfoList);

    return TRUE;
}

BOOL VisionInspectionGeometry::GetAlignData_Line_And_Point(SAlignInfoDataList& AlignInfoList,
    std::vector<std::vector<long>> vecInfoParameter_0, std::vector<Ipvm::Rect32s> vecrtSearchROI, long nSpecID,
    const Ipvm::Image8u& image)
{
    Ipvm::LineEq32r* plineData = NULL;
    std::vector<Ipvm::Point32r2> vecfptData;

    // 이미 Align 된 정보를 이용할 경우 ( 직선 )
    if (AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_Body_Line]
        || AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_Other_Line])
    {
        long nSelectData(0);

        for (long nDataID = 0; nDataID < AlignInfoList.nDataCount; nDataID++)
        {
            if (vecInfoParameter_0[nSpecID][nDataID])
            {
                nSelectData = nDataID;
                break;
            }
        }

        vecfptData.clear();

        long index_s = -1;
        long index_e = -1;

        switch (nSelectData)
        {
            case 0:
                index_s = enum_Direction_LeftTop;
                index_e = enum_Direction_LeftBottom;
                break;
            case 1:
                index_s = enum_Direction_LeftTop;
                index_e = enum_Direction_RightTop;
                break;
            case 2:
                index_s = enum_Direction_RightTop;
                index_e = enum_Direction_RightBottom;
                break;
            case 3:
                index_s = enum_Direction_LeftBottom;
                index_e = enum_Direction_RightBottom;
                break;
            case 4:
                index_s = enum_Direction_LowTop_Left;
                index_e = enum_Direction_LowTop_Right;
                break;
            default:
                break;
        }

        if (index_s >= 0 && index_e >= 0 && index_s < long(AlignInfoList.vecfptData.size())
            && index_e < long(AlignInfoList.vecfptData.size()))
        {
            vecfptData.push_back(AlignInfoList.vecfptData[index_s]);
            vecfptData.push_back(AlignInfoList.vecfptData[index_e]);
        }
        else
        {
            return FALSE;
        }

        if (nSpecID == 0)
            plineData = &AlignInfoList.LineData_F;
        else
            plineData = &AlignInfoList.LineData_S;

        if (vecfptData.size() == 0
            || Ipvm::DataFitting::FitToLine(long(vecfptData.size()), &vecfptData[0], *plineData) != Ipvm::Status::e_ok)
        {
            return FALSE;
        }

        Ipvm::Point32r2 fptCenterPoint;
        fptCenterPoint.m_x = (vecfptData[0].m_x + vecfptData[1].m_x) * 0.5f;
        fptCenterPoint.m_y = (vecfptData[0].m_y + vecfptData[1].m_y) * 0.5f;

        Ipvm::Geometry::GetFootOfPerpendicular(*plineData, fptCenterPoint, AlignInfoList.fptCenterPoint);
    }
    // 사용자가 설정한 부분을 직접 Edge Detect 하여 Align할 경우 ( 직선 )
    else if (AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_User_Edge_Detect]
        || AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_User_Line])
    {
        if (!GetEdgeDetect(
                nSpecID, image, vecInfoParameter_0[nSpecID], vecrtSearchROI[nSpecID], AlignInfoList.vecfptData))
            return FALSE;

        if ((long)AlignInfoList.vecfptData.size() <= 0)
            return FALSE;

        // edge Filtering은 lifitting할때만 사용한다.
        if (AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_User_Line])
        {
            if (!RoughAlign_LineAlign(AlignInfoList.vecfptData))
                return FALSE;
        }

        if (nSpecID == 0)
            plineData = &AlignInfoList.LineData_F;
        else
            plineData = &AlignInfoList.LineData_S;

        CPI_Geometry::LineFitting_RemoveNoise(AlignInfoList.vecfptData, *plineData);
        Ipvm::Geometry::GetFootOfPerpendicular((*plineData),
            Ipvm::Point32r2(
                (float)vecrtSearchROI[nSpecID].CenterPoint().m_x, (float)vecrtSearchROI[nSpecID].CenterPoint().m_y),
            AlignInfoList.fptCenterPoint);

        BOOL bEdgeDetectInROI = (BOOL)vecInfoParameter_0[nSpecID][ED_Param_Edge_Detect_Area];

        if (bEdgeDetectInROI)
        {
            if (!GetEdgeDetect_All_Line(image, vecInfoParameter_0[nSpecID], vecrtSearchROI[nSpecID], (*plineData),
                    AlignInfoList.vecfptData))
                return FALSE;

            if (AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_User_Line])
            {
                if (!RoughAlign_LineAlign(AlignInfoList.vecfptData))
                    return FALSE;
            }

            CPI_Geometry::LineFitting_RemoveNoise(AlignInfoList.vecfptData, (*plineData));
            Ipvm::Geometry::GetFootOfPerpendicular((*plineData),
                Ipvm::Point32r2(
                    (float)vecrtSearchROI[nSpecID].CenterPoint().m_x, (float)vecrtSearchROI[nSpecID].CenterPoint().m_y),
                AlignInfoList.fptCenterPoint);
        }
    }
    // 사용자가 설정한 부분을 직접 Edge Detect 하여 Align할 경우 ( Round )
    else if (AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_User_Round])
    {
        if (!GetEdgeDetect_Round(
                nSpecID, image, vecInfoParameter_0[nSpecID], vecrtSearchROI[nSpecID], AlignInfoList.vecfptData))
            return FALSE;

        if (!NoiseFilter_CircleFitting(AlignInfoList, vecInfoParameter_0[nSpecID]))
            return FALSE;
    }
    // 사용자가 설정한 부분을 직접 Edge Detect 하여 Align할 경우 ( Circle 및 Ellips )
    else if (AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
    {
        if (!GetEdgeDetect_Ellips(
                nSpecID, image, vecInfoParameter_0[nSpecID], vecrtSearchROI[nSpecID], AlignInfoList.vecfptData))
            return FALSE;

        if (!NoiseFilter_CircleFitting(AlignInfoList, vecInfoParameter_0[nSpecID]))
            return FALSE;
    }
    // 이미 Align 된 정보를 이용할 경우 ( Round Align )
    else if (AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_Round_LT_Info]
        || AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_Round_RT_Info]
        || AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_Round_LB_Info]
        || AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_Round_RB_Info])
    {
        return TRUE;
    }
    else if (AlignInfoList.strName == g_szAlignInfo_List_Name[Insp_Type_Body_Center_Info])
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionGeometry::GetEdgeDetect(long nSpecID, const Ipvm::Image8u& image, std::vector<long> vecnParamData,
    Ipvm::Rect32s i_rtSearchROI, std::vector<Ipvm::Point32r2>& vecfptEdgeData)
{
    if (nSpecID < 0)
        return FALSE;

    Ipvm::Point32r2 fptlineData_Start;
    Ipvm::Point32r2 fptlineData_End;

    std::vector<Ipvm::Point32r2> vecLineData_Start;
    std::vector<Ipvm::Point32r2> vecLineData_End;
    vecLineData_Start.reserve(1000);
    vecLineData_End.reserve(1000);

    long nDataSize = (long)vecnParamData.size();
    if (nDataSize != ED_Param_End)
        return FALSE;

    vecfptEdgeData.clear();

    long nSearchDir = vecnParamData[ED_Param_Search_Dir];
    long nEdgeDir = vecnParamData[ED_Param_Edge_Dir] ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    BOOL bFirstEdge = (BOOL)vecnParamData[ED_Param_Edge_Type] ? TRUE : FALSE;
    float fEdgeThreshValue = (float)vecnParamData[ED_Param_Edge_Thresh];
    double fAngle = 0.;
    if (vecnParamData[ED_Param_Edge_Angle] == 1)
    {
        fAngle = -45.0 * ITP_DEG_TO_RAD;
    }
    else if (vecnParamData[ED_Param_Edge_Angle] == 2)
    {
        fAngle = 45.0 * ITP_DEG_TO_RAD;
    }

    PI_RECT rtSearchROI;
    rtSearchROI = i_rtSearchROI;
    rtSearchROI = rtSearchROI.Rotate(fAngle);

    //const long nEdgeAverageWidth = 3;

    float fEdgeThresh_Backup = 0.f;
    Ipvm::Point32r2 edgePointXY;
    long nEdgeSearchLength = 0, nEdgeSearchWidth = 0;
    Ipvm::Point32s2 ptStart(0, 0), ptEnd(0, 0);

    if (bFirstEdge)
    {
        fEdgeThresh_Backup = m_pEdgeDetect->SetMininumThreshold(fEdgeThreshValue);
    }

    if (nSearchDir == LEFT || nSearchDir == RIGHT)
    {
        nEdgeSearchWidth = (long)CPI_Geometry::GetDistance_PointToPoint(
            (float)rtSearchROI.ltX, (float)rtSearchROI.ltY, (float)rtSearchROI.blX, (float)rtSearchROI.blY);
        nEdgeSearchLength = (long)CPI_Geometry::GetDistance_PointToPoint(
            (float)rtSearchROI.ltX, (float)rtSearchROI.ltY, (float)rtSearchROI.rtX, (float)rtSearchROI.rtY);
        vecfptEdgeData.reserve(nEdgeSearchWidth);

        for (long nStep = 0; nStep < nEdgeSearchWidth; nStep += 3)
        {
            // Search ROI 재설정
            if (nSearchDir == LEFT)
            {
                fptlineData_Start.m_x = (float)rtSearchROI.rtX;
                fptlineData_Start.m_y = (float)(rtSearchROI.rtY + nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.rtX, (float)rtSearchROI.rtY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)rtSearchROI.ltX;
                fptlineData_End.m_y = (float)(rtSearchROI.ltY + nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }
            if (nSearchDir == RIGHT)
            {
                fptlineData_Start.m_x = (float)rtSearchROI.ltX;
                fptlineData_Start.m_y = (float)(rtSearchROI.ltY + nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)rtSearchROI.rtX;
                fptlineData_End.m_y = (float)(rtSearchROI.rtY + nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.rtX, (float)rtSearchROI.rtY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }

            if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(nEdgeDir, ptStart, ptEnd, image, edgePointXY, bFirstEdge))
            {
                vecfptEdgeData.push_back(edgePointXY);

                vecLineData_Start.push_back(fptlineData_Start);
                vecLineData_End.push_back(fptlineData_End);
            }
        }
    }
    else if (nSearchDir == UP || nSearchDir == DOWN)
    {
        nEdgeSearchWidth = (long)CPI_Geometry::GetDistance_PointToPoint(
            (float)rtSearchROI.ltX, (float)rtSearchROI.ltY, (float)rtSearchROI.rtX, (float)rtSearchROI.rtY);
        nEdgeSearchLength = (long)CPI_Geometry::GetDistance_PointToPoint(
            (float)rtSearchROI.ltX, (float)rtSearchROI.ltY, (float)rtSearchROI.blX, (float)rtSearchROI.blY);
        vecfptEdgeData.reserve(nEdgeSearchWidth);

        for (long nStep = 0; nStep < nEdgeSearchWidth; nStep += 3)
        {
            // Search ROI 재설정
            if (nSearchDir == UP)
            {
                fptlineData_Start.m_x = (float)(rtSearchROI.blX + nStep);
                fptlineData_Start.m_y = (float)rtSearchROI.blY;
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.blX, (float)rtSearchROI.blY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)(rtSearchROI.ltX + nStep);
                fptlineData_End.m_y = (float)rtSearchROI.ltY;
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }
            if (nSearchDir == DOWN)
            {
                fptlineData_Start.m_x = (float)(rtSearchROI.ltX + nStep);
                fptlineData_Start.m_y = (float)rtSearchROI.ltY;
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)(rtSearchROI.blX + nStep);
                fptlineData_End.m_y = (float)rtSearchROI.blY;
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.blX, (float)rtSearchROI.blY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }

            if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(nEdgeDir, ptStart, ptEnd, image, edgePointXY, bFirstEdge))
            {
                vecfptEdgeData.push_back(edgePointXY);

                vecLineData_Start.push_back(fptlineData_Start);
                vecLineData_End.push_back(fptlineData_End);
            }
        }
    }

    if (bFirstEdge)
    {
        m_pEdgeDetect->SetMininumThreshold(fEdgeThresh_Backup);
    }

    m_vecfptlineData_Start.push_back(vecLineData_Start);
    m_vecfptlineData_End.push_back(vecLineData_End);

    return TRUE;
}

BOOL VisionInspectionGeometry::GetEdgeDetect_Round(long nSpecID, const Ipvm::Image8u& image,
    std::vector<long> vecnParamData, Ipvm::Rect32s i_rtSearchROI, std::vector<Ipvm::Point32r2>& vecfptEdgeData)
{
    if (nSpecID < 0)
        return FALSE;

    const long nQuarterLineCount = 30;

    std::vector<Ipvm::Point32r2> vecLineData_Start;
    std::vector<Ipvm::Point32r2> vecLineData_End;
    vecLineData_Start.reserve(1000);
    vecLineData_End.reserve(1000);

    long nSearchDir = vecnParamData[ED_Param_Search_Dir];
    long nEdgeDir = vecnParamData[ED_Param_Edge_Dir] ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    BOOL bFirstEdge = (BOOL)vecnParamData[ED_Param_Edge_Type] ? TRUE : FALSE;
    float fEdgeThreshValue = (float)vecnParamData[ED_Param_Edge_Thresh];

    // 무게중심을 구할 영역의 볼 센터
    Ipvm::Point32r2 fptlineData_Start, fptlineData_End;
    Ipvm::Point32s2 ptStart(0, 0), ptEnd(0, 0);

    long nSection = 0;
    float fSweepLength = (float)max(i_rtSearchROI.Width(), i_rtSearchROI.Height());
    if (nSearchDir == 0) // LT
    {
        ptStart.m_x = i_rtSearchROI.m_right;
        ptStart.m_y = i_rtSearchROI.m_bottom;

        nSection = nQuarterLineCount * 2;
    }
    else if (nSearchDir == 1) // RT
    {
        ptStart.m_x = i_rtSearchROI.m_left;
        ptStart.m_y = i_rtSearchROI.m_bottom;

        nSection = nQuarterLineCount * 3;
    }
    else if (nSearchDir == 2) // LB
    {
        ptStart.m_x = i_rtSearchROI.m_right;
        ptStart.m_y = i_rtSearchROI.m_top;

        nSection = nQuarterLineCount;
    }
    else if (nSearchDir == 3) // RB
    {
        ptStart.m_x = i_rtSearchROI.m_left;
        ptStart.m_y = i_rtSearchROI.m_top;

        nSection = 0;
    }
    fptlineData_Start.m_x = (float)ptStart.m_x;
    fptlineData_Start.m_y = (float)ptStart.m_y;

    if (ptStart.m_x <= 0 || ptStart.m_y <= 0)
        return FALSE;

    float fEdgeThresh_Backup(0.f);
    if (bFirstEdge)
    {
        fEdgeThresh_Backup = m_pEdgeDetect->SetMininumThreshold(fEdgeThreshValue);
    }

    // 볼 센터 검색을 위한 스윕 각도의 삼각함수 테이블
    long nSectionNumber = nQuarterLineCount * 4;
    UpdateSweepAngleTable(nSectionNumber, m_vecfCosForBallCenter, m_vecfSinForBallCenter);

    // 각도에 따른 삼각함수 테이블을 얻어온다.
    float* pfCos = &m_vecfCosForBallCenter[0];
    float* pfSin = &m_vecfSinForBallCenter[0];

    Ipvm::Point32r2 edgePointXY;

    long nSectionNum = nSection + nQuarterLineCount;
    for (nSection; nSection < nSectionNum; nSection++)
    {
        fptlineData_End.m_x = ptStart.m_x + fSweepLength * pfCos[nSection];
        fptlineData_End.m_y = ptStart.m_y + fSweepLength * pfSin[nSection];
        ptEnd.m_x = (long)fptlineData_End.m_x;
        ptEnd.m_y = (long)fptlineData_End.m_y;

        if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(nEdgeDir, ptStart, ptEnd, image, edgePointXY, bFirstEdge))
        {
            vecfptEdgeData.push_back(edgePointXY);

            vecLineData_Start.push_back(fptlineData_Start);
            vecLineData_End.push_back(fptlineData_End);
        }
    }

    if (bFirstEdge)
    {
        m_pEdgeDetect->SetMininumThreshold(fEdgeThresh_Backup);
    }

    m_vecfptlineData_Start.push_back(vecLineData_Start);
    m_vecfptlineData_End.push_back(vecLineData_End);

    return TRUE;
}

BOOL VisionInspectionGeometry::GetEdgeDetect_Ellips(long nSpecID, const Ipvm::Image8u& image,
    std::vector<long> vecnParamData, Ipvm::Rect32s i_rtSearchROI, std::vector<Ipvm::Point32r2>& vecfptEdgeData)
{
    if (nSpecID < 0)
        return FALSE;

    std::vector<Ipvm::Point32r2> vecLineData_Start;
    std::vector<Ipvm::Point32r2> vecLineData_End;
    vecLineData_Start.reserve(1000);
    vecLineData_End.reserve(1000);

    // 무게중심을 구할 영역의 볼 센터
    Ipvm::Point32r2 fptlineData_Start, fptlineData_End;
    Ipvm::Point32s2 ptStart, ptEnd;
    ptStart = i_rtSearchROI.CenterPoint();
    fptlineData_Start.m_x = (float)ptStart.m_x;
    fptlineData_Start.m_y = (float)ptStart.m_y;

    if (ptStart.m_x <= 0 || ptStart.m_y <= 0)
        return FALSE;

    long nSearchDir = vecnParamData[ED_Param_Search_Dir];
    long nEdgeDir = vecnParamData[ED_Param_Edge_Dir] ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    BOOL bFirstEdge = (BOOL)vecnParamData[ED_Param_Edge_Type] ? TRUE : FALSE;
    float fEdgeThreshValue = (float)vecnParamData[ED_Param_Edge_Thresh];

    float fEdgeThresh_Backup(0.f);
    if (bFirstEdge)
    {
        fEdgeThresh_Backup = m_pEdgeDetect->SetMininumThreshold(fEdgeThreshValue);
    }

    // 볼 센터 검색을 위한 스윕 각도의 삼각함수 테이블
    long nSectionNumber = 50;
    UpdateSweepAngleTable(nSectionNumber, m_vecfCosForBallCenter, m_vecfSinForBallCenter);

    // 각도에 따른 삼각함수 테이블을 얻어온다.
    float* pfCos = &m_vecfCosForBallCenter[0];
    float* pfSin = &m_vecfSinForBallCenter[0];

    Ipvm::Point32r2 edgePointXY;
    float fSweepLength = (float)(i_rtSearchROI.Width() + i_rtSearchROI.Height()) * 0.35f;

    for (long nSection = 0; nSection < nSectionNumber; nSection++)
    {
        fptlineData_End.m_x = ptStart.m_x + fSweepLength * pfCos[nSection];
        fptlineData_End.m_y = ptStart.m_y + fSweepLength * pfSin[nSection];
        ptEnd.m_x = (long)fptlineData_End.m_x;
        ptEnd.m_y = (long)fptlineData_End.m_y;

        if (nSearchDir == 0) // in -> out
        {
            if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(nEdgeDir, ptStart, ptEnd, image, edgePointXY, bFirstEdge))
            {
                vecfptEdgeData.push_back(edgePointXY);

                vecLineData_Start.push_back(fptlineData_Start);
                vecLineData_End.push_back(fptlineData_End);
            }
        }
        else // out -> in
        {
            if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(nEdgeDir, ptEnd, ptStart, image, edgePointXY, bFirstEdge))
            {
                vecfptEdgeData.push_back(edgePointXY);

                vecLineData_Start.push_back(fptlineData_Start);
                vecLineData_End.push_back(fptlineData_End);
            }
        }
    }

    if (bFirstEdge)
    {
        m_pEdgeDetect->SetMininumThreshold(fEdgeThresh_Backup);
    }

    m_vecfptlineData_Start.push_back(vecLineData_Start);
    m_vecfptlineData_End.push_back(vecLineData_End);

    return TRUE;
}

BOOL VisionInspectionGeometry::NoiseFilter_CircleFitting(
    SAlignInfoDataList& AlignInfoList, std::vector<long> vecnParamData)
{
    long nEdgeCount = (long)AlignInfoList.vecfptData.size();

    if (nEdgeCount <= 0)
        return FALSE;

    std::vector<Ipvm::Point32r2> vecfptTemp = AlignInfoList.vecfptData;
    AlignInfoList.vecfptData.clear();

    if (nEdgeCount > 0)
    {
        Ipvm::DataFitting::FitToCircle(nEdgeCount, &vecfptTemp[0], AlignInfoList.CircleData);
    }
    else
    {
        ASSERT(!_T("??"));
    }

    AlignInfoList.vecfptData.reserve(nEdgeCount);

    // 구한 원의 원주에 대해서 일정 거리 이상 떨어진 놈들을 없애고 다시 피팅
    float fDistX, fDistY, fEdgeRadius, fSpec_Temp, fRadiusSpec_Min, fRadiusSpec_Max;
    for (long nSection = 0; nSection < nEdgeCount; nSection++)
    {
        fDistX = vecfptTemp[nSection].m_x - AlignInfoList.CircleData.m_x;
        fDistY = vecfptTemp[nSection].m_y - AlignInfoList.CircleData.m_y;
        fEdgeRadius = CAST_FLOAT(sqrt(fDistX * fDistX + fDistY * fDistY));

        fSpec_Temp = 0.2f * AlignInfoList.CircleData.m_radius;
        fRadiusSpec_Min = AlignInfoList.CircleData.m_radius - fSpec_Temp;
        fRadiusSpec_Max = AlignInfoList.CircleData.m_radius + fSpec_Temp;

        if (fEdgeRadius > fRadiusSpec_Min && fEdgeRadius < fRadiusSpec_Max)
        {
            AlignInfoList.vecfptData.push_back(vecfptTemp[nSection]);
        }
    }

    if (AlignInfoList.vecfptData.size())
    {
        Ipvm::DataFitting::FitToCircle(
            (long)AlignInfoList.vecfptData.size(), &AlignInfoList.vecfptData[0], AlignInfoList.CircleData);
    }
    else
    {
        ASSERT(!_T("??"));
    }

    BOOL fEdgeDetectInROI = (BOOL)vecnParamData[ED_Param_Edge_Detect_Area];

    if (fEdgeDetectInROI == FALSE)
    {
        AlignInfoList.nDataType = DataTypeList::Point;
    }
    else
    {
        AlignInfoList.nDataType = DataTypeList::Line;
    }

    return TRUE;
}

BOOL VisionInspectionGeometry::GetEdgeDetect_All_Line(const Ipvm::Image8u& image, std::vector<long> vecnParamData,
    Ipvm::Rect32s i_rtSearchROI, const Ipvm::LineEq32r& lineData, std::vector<Ipvm::Point32r2>& vecfptEdgeData)
{
    Ipvm::Point32r2 fptlineData_Start;
    Ipvm::Point32r2 fptlineData_End;

    std::vector<Ipvm::Point32r2> vecLineData_Start;
    std::vector<Ipvm::Point32r2> vecLineData_End;
    vecLineData_Start.reserve(1000);
    vecLineData_End.reserve(1000);

    long nDataSize = (long)vecnParamData.size();
    if (nDataSize != ED_Param_End)
        return FALSE;

    long nSearchDir = vecnParamData[ED_Param_Search_Dir];
    long nEdgeDir = vecnParamData[ED_Param_Edge_Dir] ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    BOOL bFirstEdge = (BOOL)vecnParamData[ED_Param_Edge_Type] ? TRUE : FALSE;
    float fEdgeThreshValue = (float)vecnParamData[ED_Param_Edge_Thresh];
    double fAngle = 0.;
    if (vecnParamData[ED_Param_Edge_Angle] == 1)
    {
        fAngle = -45.0 * ITP_DEG_TO_RAD;
    }
    else if (vecnParamData[ED_Param_Edge_Angle] == 2)
    {
        fAngle = 45.0 * ITP_DEG_TO_RAD;
    }

    PI_RECT rtSearchROI;
    rtSearchROI = i_rtSearchROI;
    rtSearchROI = rtSearchROI.Rotate(fAngle);

    const float fEdgeAverageWidth = 3.f;
    const long nEdgeDetectTryCount = 5;

    float fEdgeThresh_Backup = 0.f;
    Ipvm::Point32r2 edgePointXY;
    long nEdgeSearchLength = 0, nEdgeSearchWidth = 0;
    Ipvm::Point32s2 ptStart(0, 0), ptEnd(0, 0);
    float fFindEdgeAndLine_Dist;
    long nRepeatCount = 0;

    // 영훈 : Line에서 일정 픽셀 이상 벗어나면 일단 코너로 진입했다고 생각하자. 기울기까지 자동 계산 됨
    //const long nMarginValidEdgeDist = 3;

    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    long nHalfSizeX = imageSizeX / 2;
    long nHalfSizeY = imageSizeY / 2;

    if (bFirstEdge)
    {
        fEdgeThresh_Backup = m_pEdgeDetect->SetMininumThreshold(fEdgeThreshValue);
    }

    if (nSearchDir == LEFT || nSearchDir == RIGHT)
    {
        nEdgeSearchWidth = (long)CPI_Geometry::GetDistance_PointToPoint(
            (float)rtSearchROI.ltX, (float)rtSearchROI.ltY, (float)rtSearchROI.blX, (float)rtSearchROI.blY);
        nEdgeSearchLength = (long)CPI_Geometry::GetDistance_PointToPoint(
            (float)rtSearchROI.ltX, (float)rtSearchROI.ltY, (float)rtSearchROI.rtX, (float)rtSearchROI.rtY);
        vecfptEdgeData.reserve(nEdgeSearchWidth);

        nRepeatCount = 0;

        // 위로 올라가며 찾아보고
        for (long nStep = 0; nStep < nHalfSizeY; nStep += 3)
        {
            if (nSearchDir == LEFT)
            {
                fptlineData_Start.m_x = (float)rtSearchROI.rtX;
                fptlineData_Start.m_y = (float)(rtSearchROI.rtY - nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.rtX, (float)rtSearchROI.rtY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)rtSearchROI.ltX;
                fptlineData_End.m_y = (float)(rtSearchROI.ltY - nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }
            if (nSearchDir == RIGHT)
            {
                fptlineData_Start.m_x = (float)rtSearchROI.ltX;
                fptlineData_Start.m_y = (float)(rtSearchROI.ltY - nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)rtSearchROI.rtX;
                fptlineData_End.m_y = (float)(rtSearchROI.rtY - nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.rtX, (float)rtSearchROI.rtY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }

            if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(nEdgeDir, ptStart, ptEnd, image, edgePointXY, FALSE))
            {
                fFindEdgeAndLine_Dist
                    = CPI_Geometry::GetDistance_PointToLine(edgePointXY.m_x, edgePointXY.m_y, lineData);

                if (fFindEdgeAndLine_Dist >= fEdgeAverageWidth)
                {
                    ++nRepeatCount;
                }
                else
                {
                    vecfptEdgeData.push_back(edgePointXY);

                    vecLineData_Start.push_back(fptlineData_Start);
                    vecLineData_End.push_back(fptlineData_End);
                }
            }
            else
            {
                ++nRepeatCount;
            }

            // 영훈 : 찾다가 라인에서 크게 벗어나거나 찾지 못하는 엣지가 연속으로 3개 발생 시 종료한다.
            if (nRepeatCount > nEdgeDetectTryCount)
            {
                break;
            }
        }

        nRepeatCount = 0;

        // 아래로 내려가며 찾아보고
        for (long nStep = 0; nStep < nHalfSizeY; nStep += 3)
        {
            if (nSearchDir == LEFT)
            {
                fptlineData_Start.m_x = (float)rtSearchROI.rtX;
                fptlineData_Start.m_y = (float)(rtSearchROI.brY + nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.rtX, (float)rtSearchROI.rtY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)rtSearchROI.ltX;
                fptlineData_End.m_y = (float)(rtSearchROI.blY + nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }
            if (nSearchDir == RIGHT)
            {
                fptlineData_Start.m_x = (float)rtSearchROI.ltX;
                fptlineData_Start.m_y = (float)(rtSearchROI.blY + nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)rtSearchROI.rtX;
                fptlineData_End.m_y = (float)(rtSearchROI.brY + nStep);
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.rtX, (float)rtSearchROI.rtY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }

            if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(nEdgeDir, ptStart, ptEnd, image, edgePointXY, FALSE))
            {
                fFindEdgeAndLine_Dist
                    = CPI_Geometry::GetDistance_PointToLine(edgePointXY.m_x, edgePointXY.m_y, lineData);

                if (fFindEdgeAndLine_Dist >= fEdgeAverageWidth)
                {
                    ++nRepeatCount;
                }
                else
                {
                    vecfptEdgeData.push_back(edgePointXY);

                    vecLineData_Start.push_back(fptlineData_Start);
                    vecLineData_End.push_back(fptlineData_End);
                }
            }
            else
            {
                ++nRepeatCount;
            }

            if (nRepeatCount > nEdgeDetectTryCount)
                break;
        }
    }
    else if (nSearchDir == UP || nSearchDir == DOWN)
    {
        nEdgeSearchWidth = (long)CPI_Geometry::GetDistance_PointToPoint(
            (float)rtSearchROI.ltX, (float)rtSearchROI.ltY, (float)rtSearchROI.rtX, (float)rtSearchROI.rtY);
        nEdgeSearchLength = (long)CPI_Geometry::GetDistance_PointToPoint(
            (float)rtSearchROI.ltX, (float)rtSearchROI.ltY, (float)rtSearchROI.blX, (float)rtSearchROI.blY);
        vecfptEdgeData.reserve(nEdgeSearchWidth);

        nRepeatCount = 0;

        for (long nStep = 0; nStep < nHalfSizeX; nStep += 3)
        {
            if (nSearchDir == UP)
            {
                fptlineData_Start.m_x = (float)(rtSearchROI.blX - nStep);
                fptlineData_Start.m_y = (float)rtSearchROI.blY;
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.blX, (float)rtSearchROI.blY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)(rtSearchROI.ltX - nStep);
                fptlineData_End.m_y = (float)rtSearchROI.ltY;
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }
            if (nSearchDir == DOWN)
            {
                fptlineData_Start.m_x = (float)(rtSearchROI.ltX - nStep);
                fptlineData_Start.m_y = (float)rtSearchROI.ltY;
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)(rtSearchROI.blX - nStep);
                fptlineData_End.m_y = (float)rtSearchROI.blY;
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.blX, (float)rtSearchROI.blY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }

            if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(nEdgeDir, ptStart, ptEnd, image, edgePointXY, FALSE))
            {
                fFindEdgeAndLine_Dist
                    = CPI_Geometry::GetDistance_PointToLine(edgePointXY.m_x, edgePointXY.m_y, lineData);

                // 영훈 : Line에서 2픽셀 이상 벗어나면 일단 코너로 진입했다고 생각하자. 라인 각도까지 자동 계산 됨
                if (fFindEdgeAndLine_Dist >= fEdgeAverageWidth)
                {
                    ++nRepeatCount;
                }
                else
                {
                    vecfptEdgeData.push_back(edgePointXY);

                    vecLineData_Start.push_back(fptlineData_Start);
                    vecLineData_End.push_back(fptlineData_End);

                    nRepeatCount = 0;
                }
            }
            else
            {
                ++nRepeatCount;
            }

            if (nRepeatCount > nEdgeDetectTryCount)
            {
                break;
            }
        }

        nRepeatCount = 0;

        for (long nStep = 0; nStep < nHalfSizeX; nStep += 3)
        {
            if (nSearchDir == UP)
            {
                fptlineData_Start.m_x = (float)(rtSearchROI.brX + nStep);
                fptlineData_Start.m_y = (float)rtSearchROI.brY;
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.blX, (float)rtSearchROI.blY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)(rtSearchROI.rtX + nStep);
                fptlineData_End.m_y = (float)rtSearchROI.rtY;
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }
            if (nSearchDir == DOWN)
            {
                fptlineData_Start.m_x = (float)(rtSearchROI.rtX + nStep);
                fptlineData_Start.m_y = (float)rtSearchROI.rtY;
                CPI_Geometry::RotatePoint(
                    fptlineData_Start, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.ltX, (float)rtSearchROI.ltY));
                ptStart = Ipvm::Point32s2((long)(fptlineData_Start.m_x + 0.5f), (long)(fptlineData_Start.m_y + 0.5f));

                fptlineData_End.m_x = (float)(rtSearchROI.brX + nStep);
                fptlineData_End.m_y = (float)rtSearchROI.brY;
                CPI_Geometry::RotatePoint(
                    fptlineData_End, (float)-fAngle, Ipvm::Point32r2((float)rtSearchROI.blX, (float)rtSearchROI.blY));
                ptEnd = Ipvm::Point32s2((long)(fptlineData_End.m_x + 0.5f), (long)(fptlineData_End.m_y + 0.5f));
            }

            if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(nEdgeDir, ptStart, ptEnd, image, edgePointXY, FALSE))
            {
                fFindEdgeAndLine_Dist
                    = CPI_Geometry::GetDistance_PointToLine(edgePointXY.m_x, edgePointXY.m_y, lineData);

                if (fFindEdgeAndLine_Dist >= fEdgeAverageWidth)
                {
                    ++nRepeatCount;
                }
                else
                {
                    vecfptEdgeData.push_back(edgePointXY);

                    vecLineData_Start.push_back(fptlineData_Start);
                    vecLineData_End.push_back(fptlineData_End);

                    nRepeatCount = 0;
                }
            }
            else
            {
                ++nRepeatCount;
            }

            // 영훈 : 찾다가 라인에서 크게 벗어나거나 찾지 못하는 엣지가 연속으로 3개 발생 시 종료한다.
            if (nRepeatCount > nEdgeDetectTryCount)
            {
                break;
            }
        }
    }

    if (bFirstEdge)
    {
        m_pEdgeDetect->SetMininumThreshold(fEdgeThresh_Backup);
    }

    m_vecfptlineData_Start.push_back(vecLineData_Start);
    m_vecfptlineData_End.push_back(vecLineData_End);

    return TRUE;
}

BOOL VisionInspectionGeometry::RoughAlign_LineAlign(std::vector<Ipvm::Point32r2>& vecfptEdgeData)
{
    std::vector<Ipvm::Point32r2> vecfpTempEdge = vecfptEdgeData;

    long nPointSize = (long)vecfpTempEdge.size();

    if (nPointSize <= 0)
        return FALSE;

    std::vector<Ipvm::Point32r2> vecfTotalXY;

    for (long i = 0; i < nPointSize; i++)
    {
        vecfTotalXY.push_back(vecfpTempEdge[i]);
    }

    std::vector<Ipvm::Point32r2> vecfFilteredXY;
    Ipvm::LineEq32r line;

    if (!CPI_Geometry::RoughLineFitting(nPointSize, &vecfTotalXY[0], line, 5, 1.f, vecfFilteredXY))
    {
        return FALSE;
    }

    nPointSize = (long)vecfFilteredXY.size();

    if (nPointSize <= 0)
    {
        return FALSE;
    }

    vecfptEdgeData = vecfFilteredXY;

    return TRUE;
}

BOOL VisionInspectionGeometry::Get_LineCenter_Point(SAlignInfoDataList& AlignInfoData, Ipvm::Point32r2& fptBeginPoint,
    Ipvm::Point32r2& fptCenterPoint, Ipvm::Point32r2& fptEndPoint)
{
    Ipvm::LineEq32r line = AlignInfoData.LineData_F;
    std::vector<Ipvm::Point32r2> vecfptEdgePoint = AlignInfoData.vecfptData;

    long nEdgeCount = (long)vecfptEdgePoint.size();

    if (nEdgeCount < 2)
        return FALSE;

    Ipvm::Point32r2 fptData_In_0, fptData_In_1;

    fptData_In_0 = vecfptEdgePoint[0];
    fptData_In_1 = vecfptEdgePoint[nEdgeCount - 1];

    Ipvm::Geometry::GetFootOfPerpendicular(line, fptData_In_0, fptBeginPoint);
    Ipvm::Geometry::GetFootOfPerpendicular(line, fptData_In_1, fptEndPoint);

    fptCenterPoint.m_x = (fptBeginPoint.m_x + fptEndPoint.m_x) * 0.5f;
    fptCenterPoint.m_y = (fptBeginPoint.m_y + fptEndPoint.m_y) * 0.5f;

    return TRUE;
}

void VisionInspectionGeometry::UpdateSweepAngleTable(
    long nSectionNum, std::vector<float>& vecfCos, std::vector<float>& vecfSin)
{
    vecfCos.resize(nSectionNum);
    vecfSin.resize(nSectionNum);

    float* pfCos = &vecfCos[0];
    float* pfSin = &vecfSin[0];

    float fCurAngle;

    for (long nSection = 0; nSection < nSectionNum; nSection++)
    {
        fCurAngle = float(nSection) / (float)nSectionNum * (float)ITP_TwoPI;
        pfCos[nSection] = float(cos(fCurAngle));
        pfSin[nSection] = float(sin(fCurAngle));
    }
}

void VisionInspectionGeometry::SetDebugInfo(const bool detailSetupMode)
{
    if (detailSetupMode == false)
        return;

    std::vector<Ipvm::EllipseEq32r> vecfCircleData(1);

    CString strDebuginfoName;

    long nSpecNum = (long)m_VisionPara->m_vecstrinfoName_Ref.size();
    for (long nSpecID = 0; nSpecID < nSpecNum; nSpecID++)
    {
        // Circle 및 Ellipse 관련 Overlay 세팅
        if (m_VisionPara->m_vecstrinfoName_Ref[nSpecID].size() > 0)
        {
            if (m_VisionPara->m_vecstrinfoName_Ref[nSpecID][0] == g_szAlignInfo_List_Name[Insp_Type_User_Round]
                || m_VisionPara->m_vecstrinfoName_Ref[nSpecID][0] == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
            {
                vecfCircleData[0].m_x = m_AlignInfoData_Ref.CircleData.m_x;
                vecfCircleData[0].m_y = m_AlignInfoData_Ref.CircleData.m_y;
                vecfCircleData[0].m_xradius = m_AlignInfoData_Ref.CircleData.m_radius;
                vecfCircleData[0].m_yradius = m_AlignInfoData_Ref.CircleData.m_radius;

                strDebuginfoName.Format(_T("Reference Ellipse Data %d"), nSpecID);
                SetDebugInfoItem(detailSetupMode, strDebuginfoName, vecfCircleData);
            }
        }

        // Point 관련 Overlay 세팅
        std::vector<Ipvm::Point32r2> vecPointData;
        long nPointCount = (long)m_AlignInfoData_Ref.vecfptData.size();
        vecPointData.resize(nPointCount);

        for (long nData = 0; nData < nPointCount; nData++)
        {
            vecPointData[nData] = m_AlignInfoData_Ref.vecfptData[nData];
        }
        strDebuginfoName.Format(_T("Reference Point Data %d"), nSpecID);
        SetDebugInfoItem(detailSetupMode, strDebuginfoName, vecPointData);
    }

    nSpecNum = (long)m_VisionPara->m_vecstrinfoName_Tar.size();
    for (long nSpecID = 0; nSpecID < nSpecNum; nSpecID++)
    {
        // Circle 및 Ellipse 관련 Overlay 세팅
        if (m_VisionPara->m_vecstrinfoName_Tar[nSpecID].size() > 0)
        {
            if (m_VisionPara->m_vecstrinfoName_Tar[nSpecID][0] == g_szAlignInfo_List_Name[Insp_Type_User_Round]
                || m_VisionPara->m_vecstrinfoName_Tar[nSpecID][0] == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
            {
                vecfCircleData[0].m_x = m_AlignInfoData_Tar.CircleData.m_x;
                vecfCircleData[0].m_y = m_AlignInfoData_Tar.CircleData.m_y;
                vecfCircleData[0].m_xradius = m_AlignInfoData_Tar.CircleData.m_radius;
                vecfCircleData[0].m_yradius = m_AlignInfoData_Tar.CircleData.m_radius;

                strDebuginfoName.Format(_T("Target Ellipse Data %d"), nSpecID);
                SetDebugInfoItem(detailSetupMode, strDebuginfoName, vecfCircleData);
            }
        }

        // Point 관련 Overlay 세팅
        std::vector<Ipvm::Point32r2> vecPointData;
        long nPointCount = (long)m_AlignInfoData_Tar.vecfptData.size();
        vecPointData.resize(nPointCount);

        for (long nData = 0; nData < nPointCount; nData++)
        {
            vecPointData[nData] = m_AlignInfoData_Tar.vecfptData[nData];
        }
        strDebuginfoName.Format(_T("Target Point Data %d"), nSpecID);
        SetDebugInfoItem(detailSetupMode, strDebuginfoName, vecPointData);
    }

    // Line 관련 Overlay 세팅
    Ipvm::LineSeg32r nLineData;
    std::vector<Ipvm::LineSeg32r> veclineData;
    veclineData.reserve(10000);

    long nDataCount = (long)m_vecfptlineData_Start.size();
    for (long nData = 0; nData < nDataCount; nData++)
    {
        long nLineCount = (long)m_vecfptlineData_Start[nData].size();
        for (long nLine = 0; nLine < nLineCount; nLine++)
        {
            nLineData.m_sx = m_vecfptlineData_Start[nData][nLine].m_x;
            nLineData.m_sy = m_vecfptlineData_Start[nData][nLine].m_y;
            nLineData.m_ex = m_vecfptlineData_End[nData][nLine].m_x;
            nLineData.m_ey = m_vecfptlineData_End[nData][nLine].m_y;

            veclineData.push_back(nLineData);
        }
    }

    strDebuginfoName.Format(_T("All Line Data"));
    SetDebugInfoItem(detailSetupMode, strDebuginfoName, veclineData);

    strDebuginfoName.Empty();
}

LPCTSTR VisionInspectionGeometry::GetFinalCheckError_Reference() const
{
    return m_resultFinalCheckError_reference;
}

LPCTSTR VisionInspectionGeometry::GetFinalCheckError_Target() const
{
    return m_resultFinalCheckError_target;
}

BOOL VisionInspectionGeometry::SetPassResultGeometryForSide()
{
    //검사하지 않는 Side Vision은 BodyAlign Result로 표기하여 준다
    long nSpecNum = (long)m_variableInspectionSpecs.size();

    const Ipvm::Rect32s ResultROI = m_sEdgeAlignResult->getBodyRect32s();

    for (long nSpecID = 0; nSpecID < nSpecNum; nSpecID++)
    {
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(m_variableInspectionSpecs[nSpecID].m_specName);
        if (pResult == NULL)
            return FALSE;

        VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == NULL)
            return FALSE;

        if (pSpec->m_use == FALSE)
            continue;

        pResult->Resize(1);

        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("0"), _T(""), _T(""), 0, *pSpec, 0.f, 0.f, 0.f, true);

        pResult->SetRect(0, ResultROI);

        pResult->m_totalResult = PASS;
    }

    return TRUE;
}