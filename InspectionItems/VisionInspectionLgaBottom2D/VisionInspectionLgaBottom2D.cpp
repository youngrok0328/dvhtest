//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionLgaBottom2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionLgaBottom2D.h"
#include "LandInspectionResult.h"
#include "VisionInspectionLgaBottom2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NOT_USE_GROUP_INSPECTION 1 //kircheis_LandMissing

//CPP_7_________________________________ Implementation body
//
VisionInspectionLgaBottom2D::VisionInspectionLgaBottom2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_LGA_LAND_2D, _VISION_INSP_NAME_LGA_LAND_2D, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionLgaBottom2DPara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
    , m_result(new LandInspectionResult)
    , m_pfptGetCenterPosbyPadAlign(nullptr)
{
    m_pVisionInspDlg = NULL;

    m_pEdgeDetect = NULL;
    m_pBlob = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{EBADD071-1F72-4F36-BF2B-AD1D044CF82C}"),
        g_szLandInspectionName[LAND_INSPECTION_LAND_ALIGN], _T("Lmis"), _T("L"), _T("Num"), HostReportCategory::LAND,
        II_RESULT_TYPE::COUNT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE); //kircheis_LandMissing
    m_defaultFixedInspectionSpecs.emplace_back(_T("{F970F09F-2C15-4006-8D96-25B19016989C}"),
        g_szLandInspectionName[LAND_INSPECTION_LAND_OFFSET_X], _T("ldX"), _T("L"), _T("um"), HostReportCategory::LAND,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{8219583F-3BD4-4B9E-88A5-F72F431B5D62}"),
        g_szLandInspectionName[LAND_INSPECTION_LAND_OFFSET_Y], _T("ldY"), _T("L"), _T("um"), HostReportCategory::LAND,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{695BCDCF-CDCB-4095-B954-150BF9F9A0C1}"),
        g_szLandInspectionName[LAND_INSPECTION_LAND_OFFSET_R], _T("lR"), _T("L"), _T("um"), HostReportCategory::LAND,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{B9FDF326-8565-4FC0-989C-9D2BFA165DBC}"),
        g_szLandInspectionName[LAND_INSPECTION_LAND_WIDTH], _T("lLWD"), _T("L"), _T("um"), HostReportCategory::LAND,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{BF4AE723-96AB-44DA-85A4-44E7B4B01029}"),
        g_szLandInspectionName[LAND_INSPECTION_LAND_LENGTH], _T("lLLN"), _T("L"), _T("um"), HostReportCategory::LAND,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    __super::ResetSpec();

    UpdateSpec();

    m_DebugInfoGroup.Add(_T("Spec Land Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Spec Land ROI"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("Land Search ROI"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("Circle Land Edge Sweep Line(Rough)"), enumDebugInfoType::LineSeg_32f); //kircheis_LandShape
    m_DebugInfoGroup.Add(_T("Circle Land Edge Sweep Line(Final)"), enumDebugInfoType::LineSeg_32f); //kircheis_LandShape
    m_DebugInfoGroup.Add(_T("Land Edge Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Land ROI"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("Land Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Land Measured ROI"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("Circle Land Measured Circle"), enumDebugInfoType::EllipseEq_32f); //kircheis_LandShape
    m_DebugInfoGroup.Add(_T("Land Origin Mask Image"), enumDebugInfoType::Image_8u_C1);
}

void VisionInspectionLgaBottom2D::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_VisionPara->Init();
}

VisionInspectionLgaBottom2D::~VisionInspectionLgaBottom2D(void)
{
    CloseDlg();

    //{{//kircheis_LandShape
    m_vecfCosForBallCenter.clear();
    m_vecfSinForBallCenter.clear();
    //}}

    delete m_result;
    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspectionLgaBottom2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionLgaBottom2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D), false, GetCurVisionModule_Status());
    else if (m_visionUnit.IsTheUsingVisionProcessingByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_visionUnit.RunInspection(m_visionUnit.GetVisionAlignProcessing(), false, GetCurVisionModule_Status());
    else
    {
        CString strError;
        strError.Format(_T("This module need execute the [%s] in the pre-process."),
            m_visionUnit.GetVisionProcessingModuleNameByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()));
        SimpleMessage(strError);
        strError.Empty();
    }

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionLgaBottom2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionLgaBottom2D::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->ShowImage();

    return 0;
}

BOOL VisionInspectionLgaBottom2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{F61CFE32-AC7C-41BE-80D3-34840BBBBAFC}")]))
        return FALSE;

    //kk 이 코드를 호출할 위치가 마땅치 않아 일단 여기에서 호출
    //Package Layout에서 MapData 수정 후 Land 3D 검사 들어갈 시 이전 MapData의 정보들을 들고 들어가는 문제로 인해
    //LinkDataBase에서 Data를 초기화 해주는 역할을 넣어놓음

    std::vector<CString> vecGroupLand;
    getGroupInfo(m_packageSpec.m_LandMapManager->vecLandData, vecGroupLand);

    long nLayerNum = long(vecGroupLand.size());

    if (nLayerNum < 2)
    {
        m_VisionPara->m_vecstrGroupInspName.resize(0);
    }
    else if (long(m_group_id_list.size()) != nLayerNum)
    {
        m_VisionPara->m_vecstrGroupInspName.resize(nLayerNum);

        if (long(m_group_id_list.size()) < nLayerNum)
        {
            for (int i = long(m_group_id_list.size()); i < nLayerNum; i++)
            {
                m_VisionPara->m_vecstrGroupInspName[i].SetName(vecGroupLand[i]);
            }
        }
    }

    m_group_id_list = vecGroupLand;

    UpdateSpec();

    for (int nIdx = 0; nIdx < vecGroupLand.size(); nIdx++)
    {
        vecGroupLand[nIdx].Empty();
    }

    return TRUE;
}

// 영훈 20141218_BallOverlay : 검사 시 ball 불량 시 화면에 표시해주도록 한다.
void VisionInspectionLgaBottom2D::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    for (long nInsp = LAND_INSPECTION_START; nInsp < LAND_INSPECTION_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szLandInspectionName[nInsp]);
            if (pResult == nullptr)
                continue;
            if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                if (pResult->vecrtRejectROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtRejectROI, RGB(255, 0, 0));

                    if (m_fixedInspectionSpecs[nInsp].m_useMarginal && pResult->vecrtMarginalROI.size())
                    {
                        overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                    }
                }

                else if (pResult->vecrtMarginalROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                }
            }
        }
    }
    long variableNum = (long)m_variableInspectionSpecs.size();
    for (long nInsp = 0; nInsp < variableNum; nInsp++)
    {
        if (m_variableInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult
                = m_resultGroup.GetResultByName(m_variableInspectionSpecs[nInsp].m_specName);
            if (pResult == nullptr)
                continue;
            if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                if (pResult->vecrtRejectROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtRejectROI, RGB(255, 0, 0));

                    if (m_variableInspectionSpecs[nInsp].m_useMarginal && pResult->vecrtMarginalROI.size())
                    {
                        overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                    }
                }

                else if (pResult->vecrtMarginalROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                }
            }
        }
    }
}

void VisionInspectionLgaBottom2D::AppendTextResult(CString& textResult)
{
    long fixedNum = (long)m_fixedInspectionSpecs.size();
    long variableNum = (long)m_variableInspectionSpecs.size();
    long nInspNum = fixedNum + variableNum;
    long nResultNum = (long)m_resultGroup.m_vecResult.size();
    if (nInspNum != nResultNum)
    {
        ASSERT(!(_T("Check the inspection item's number")));
        return;
    }

    std::vector<VisionInspectionResult*> vecpInspResult(nInspNum);
    std::vector<VisionInspectionSpec*> vecpInspSpec(nInspNum);
    std::vector<BOOL> vecbUseInsp(nInspNum);
    std::vector<CString> vecstrInspShortName(nInspNum);
    long nVisionModuleTotalResult = NOT_MEASURED;

    for (long nID = 0; nID < nInspNum; nID++)
    {
        if (nID < fixedNum)
            vecpInspResult[nID] = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[nID].m_specName);
        else
            vecpInspResult[nID] = m_resultGroup.GetResultByName(m_variableInspectionSpecs[nID - fixedNum].m_specName);

        if (vecpInspResult[nID] == nullptr)
            ASSERT(!(_T("Check the inspection item's number")));
        vecpInspSpec[nID] = GetSpecByName(vecpInspResult[nID]->m_resultName);
        if (vecpInspSpec[nID] == nullptr)
            ASSERT(!(_T("Check the inspection item's number")));

        vecbUseInsp[nID] = -vecpInspSpec[nID]->m_use;
        vecstrInspShortName[nID] = vecpInspSpec[nID]->m_shortName;
        nVisionModuleTotalResult = (long)max(nVisionModuleTotalResult, vecpInspResult[nID]->m_totalResult);
    }

    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);
    textResult.AppendFormat(
        _T("    %s Total Result : %s\r\n"), (LPCTSTR)m_strModuleName, (LPCTSTR)Result2String(nVisionModuleTotalResult));

    for (long nID = 0; nID < nInspNum; nID++)
    {
        if (!vecbUseInsp[nID])
            continue;

        auto worstErrorValue = vecpInspResult[nID]->getObjectWorstErrorValue();

        if (worstErrorValue == Ipvm::k_noiseValue32r)
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ INV ])\r\n"),
                (LPCTSTR)vecpInspResult[nID]->m_resultName, (LPCTSTR)vecstrInspShortName[nID],
                (LPCTSTR)Result2String(vecpInspResult[nID]->m_totalResult),
                (LPCTSTR)vecpInspResult[nID]->getObjectWorstID());
        }
        else
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ %.2f ])\r\n"),
                (LPCTSTR)vecpInspResult[nID]->m_resultName, (LPCTSTR)vecstrInspShortName[nID],
                (LPCTSTR)Result2String(vecpInspResult[nID]->m_totalResult),
                (LPCTSTR)vecpInspResult[nID]->getObjectWorstID(), worstErrorValue);
        }
    }

    //{{//Parameters for this module only.
    long nObjectTotalNum = 0;
    for (auto& layer : m_vec2LandInfoPerLayer)
    {
        nObjectTotalNum += (long)layer.size();
    }
    //}}

    AppendDetailTextResult(textResult, nObjectTotalNum);

    for (int nIdx = 0; nIdx < vecstrInspShortName.size(); nIdx++)
    {
        vecstrInspShortName[nIdx].Empty();
    }
}

void VisionInspectionLgaBottom2D::ResetResult()
{
    __super::ResetResult();

    m_result->Clear();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    m_bInvalid = FALSE;

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    //// Blob Parameter...
    m_pBlob = getReusableMemory().GetBlob();

    //{{mc_Land 수정
    getGroupInfo(m_packageSpec.m_LandMapManager->vecLandData, m_group_id_list);
    m_vec2LandInfoPerLayer.clear();

    long ParameterSize = long(m_VisionPara->m_vecLandParameter.size());

    //LandInfo를 Layer개념으로 분기한다.
    m_vec2LandInfoPerLayer.resize(m_group_id_list.size());
    m_VisionPara->m_vecLandParameter.resize(m_group_id_list.size());

    if (ParameterSize < long(m_group_id_list.size()) && ParameterSize != 0)
    {
        for (int i = ParameterSize; i < long(m_group_id_list.size()); i++)
        {
            m_VisionPara->m_vecLandParameter[i].Copy(m_VisionPara->m_vecLandParameter[0]);
        }
    }

    for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
    {
        long LandType = -1;

        for (long nIndex = 0; nIndex < m_packageSpec.m_LandMapManager->vecLandData.size(); nIndex++)
        {
            if (!m_packageSpec.m_LandMapManager->vecLandData[nIndex].bIgnore)
            {
                if (m_group_id_list[nLayer] == m_packageSpec.m_LandMapManager->vecLandData[nIndex].m_groupID)
                {
                    m_vec2LandInfoPerLayer[nLayer].push_back(m_packageSpec.m_LandMapManager->vecLandData[nIndex]);
                    LandType = m_packageSpec.m_LandMapManager->vecLandData[nIndex].nLandShapeType;
                }
            }
        }

        if (LandType != -1)
            m_VisionPara->m_vecLandParameter[nLayer].LandType = LandType;
    }
    //}}

    long nEdgeAlignDataNum(0);
    void* pEdgeAlignData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nEdgeAlignDataNum);
    if (pEdgeAlignData != nullptr && nEdgeAlignDataNum > 0)
        *m_sEdgeAlignResult = *(VisionAlignResult*)pEdgeAlignData;

    m_pfptGetCenterPosbyPadAlign = nullptr;
    long nPadDataNum(0);
    void* pPadAlignData = m_visionUnit.GetVisionDebugInfo(
        _VISION_INSP_GUID_FIDUCIAL_ALIGN, _T("Apply PAD Align to Body Center"), nPadDataNum);
    if (pPadAlignData != nullptr && nPadDataNum > 0)
        m_pfptGetCenterPosbyPadAlign = (Ipvm::Point32r2*)pPadAlignData;
}

void VisionInspectionLgaBottom2D::UpdateSpec()
{
    //kk 이전 버전 사용 Job들을 아예 다시 셋업 하면 상관이 없는데
    //이전 버전 Job을 그대로 사용할 때, GroupName이 만들어지지 않는게 문제
    //검사 항목 마다 초기화 하는 위치가 달라 한번 더 체크하도록 수정
    while (m_VisionPara->m_vecstrGroupInspName.size() != m_group_id_list.size())
    {
        if (m_VisionPara->m_vecstrGroupInspName.size() < m_group_id_list.size())
        {
            long index = long(m_VisionPara->m_vecstrGroupInspName.size());
            Land2DGroupInsp Data;

            Data.SetName(m_group_id_list[index]);

            m_VisionPara->m_vecstrGroupInspName.push_back(Data);
        }
        else
        {
            m_VisionPara->m_vecstrGroupInspName.erase(m_VisionPara->m_vecstrGroupInspName.end() - 1);
        }
    }

    long SpecNum = (long)m_variableInspectionSpecs.size();
    long DefaultNum = (long)m_defaultFixedInspectionSpecs.size() - NOT_USE_GROUP_INSPECTION;
    long MakeSpecNum = ((long)m_VisionPara->m_vecstrGroupInspName.size() * DefaultNum);

    while (SpecNum < MakeSpecNum)
    {
        long Index = SpecNum;

        if (Index < 0)
            return;

        CString strSpecInital;
        strSpecInital.Format(_T("LUCP_G%d"), Index);
        m_variableInspectionSpecs.emplace_back(m_moduleGuid, ::CreateGUID(), m_strModuleName, _T(""), strSpecInital,
            _T("C"), _T("um"), HostReportCategory::LAND, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE,
            FALSE, FALSE);

        SpecNum = long(m_variableInspectionSpecs.size());

        strSpecInital.Empty();
    }

    while (SpecNum > MakeSpecNum)
    {
        m_variableInspectionSpecs.erase(m_variableInspectionSpecs.end() - 1);

        SpecNum = long(m_variableInspectionSpecs.size());
    }

    std::vector<CString> InspectionName;

    if (m_group_id_list.size() != 1)
    {
        if (m_VisionPara->m_vecstrGroupInspName.size() != 0)
        {
            for (int i = 0; i < long(m_group_id_list.size()); i++)
            {
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].LandOffsetXName);
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].LandOffsetYName);
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].LandOffsetRName);
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].LandWidthName);
                InspectionName.push_back(m_VisionPara->m_vecstrGroupInspName[i].LandLengthName);
            }
        }
    }
    else if (long(m_group_id_list.size())
        <= 1) //kk Ball Type이 한개 이하일 경우 필요 없습니다. 다시 호출해서 설정합시다.
    {
        m_variableInspectionSpecs.clear();
        m_VisionPara->m_vecstrGroupInspName.clear();
        SpecNum = 0;
    }

    for (long idx = 0; idx < long(SpecNum); idx++)
    {
        CString specName;
        specName.Format(_T("LIGS_%d"), idx);

        m_variableInspectionSpecs[idx].m_specName = InspectionName[idx];
        m_variableInspectionSpecs[idx].m_shortName = specName;

        m_variableInspectionSpecs[idx].m_unit = _T("um");

        m_variableInspectionSpecs[idx].MakeSpecFullName();

        specName.Empty();
    }

    GetVisionInspectionSpecs();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->ResetSpecAndResultDlg();
    }

    for (int nIdx = 0; nIdx < InspectionName.size(); nIdx++)
    {
        InspectionName[nIdx].Empty();
    }
}

void VisionInspectionLgaBottom2D::GetVisionInspectionSpecs()
{
    m_VisionPara->m_vecVisionInspectionSpecs.clear();

    for (auto& spec : m_fixedInspectionSpecs)
    {
        m_VisionPara->m_vecVisionInspectionSpecs.push_back(spec);
    }

    for (auto& spec : m_variableInspectionSpecs)
    {
        m_VisionPara->m_vecVisionInspectionSpecs.push_back(spec);
    }
}

void VisionInspectionLgaBottom2D::SetVisionInspectionSpecs()
{
    for (auto& spec : m_VisionPara->m_vecVisionInspectionSpecs)
    {
        for (auto& item : m_fixedInspectionSpecs)
        {
            if (item.m_specGuid == spec.m_specGuid)
            {
                item = spec;
            }
        }

        for (auto& item : m_variableInspectionSpecs)
        {
            if (item.m_specGuid == spec.m_specGuid)
            {
                item = spec;
            }
        }
    }
}

std::vector<CString> VisionInspectionLgaBottom2D::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);
    std::vector<CString> vecstrInspectionSpec = ExportInspectionSpecToText(strVisionName);

    vecStrResult.insert(vecStrResult.end(), vecstrInspectionSpec.begin(), vecstrInspectionSpec.end());

    return vecStrResult;
}