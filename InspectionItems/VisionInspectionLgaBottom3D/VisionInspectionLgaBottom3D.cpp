//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionLgaBottom3D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionLgaBottom3D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <algorithm>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BaseSpecCount 9
#define NOT_USE_GROUP_INSPECTION 4

//CPP_7_________________________________ Implementation body
//
VisionInspectionLgaBottom3D::VisionInspectionLgaBottom3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_LGA_LAND_3D, _VISION_INSP_NAME_LGA_LAND_3D, visionUnit, packageSpec)
    , m_sEdgeAlignResult(new VisionAlignResult)
{
    m_pVisionInspDlg = NULL;

    m_fptPixelperMM = Ipvm::Point32r2(0.f, 0.f);
    m_fAngle = 0.f;

    m_fConvertScale = 1.f;
    m_fConvertScaleInten = 1.f;

    m_fCalcTime = 0.f;

    // Spec 를 미리 결정해서 생성자 부분에 하드코딩
    m_defaultFixedInspectionSpecs.emplace_back(_T("{8263D0E8-2754-4AB6-9726-B1DF54C28D53}"),
        g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_COPL], _T("LCOP"), _T("C"), _T("um"), HostReportCategory::LAND,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{4A6FB4EA-D942-45EA-991F-69DE5BAF458B}"),
        g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_COPL], _T("LUCP"), _T("C"), _T("um"),
        HostReportCategory::LAND, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{ECD14CF3-B733-4CBA-98EC-C49780EBA246}"),
        g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_HEIGHT], _T("LHEI"), _T("C"), _T("um"),
        HostReportCategory::LAND, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{60DD012C-5691-430F-B412-7867C67D2107}"),
        g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_WARPAGE], _T("LWAP"), _T("C"), _T("um"),
        HostReportCategory::LAND, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{752C4957-0BBC-42CA-97EB-AA2DF28B8F77}"),
        g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE], _T("LUWP"), _T("C"), _T("um"),
        HostReportCategory::LAND, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    //{{//kircheis_MED2LW
    m_defaultFixedInspectionSpecs.emplace_back(_T("{DCC5D826-71DC-422E-B3F0-091F31A3B3B3}"),
        g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AX], _T("AX"), _T("C"), _T("um"),
        HostReportCategory::LAND_WARP_SHAPE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{90A8302D-2613-47FC-8021-786C5CB4D9D9}"),
        g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AV], _T("AV"), _T("C"), _T("um"),
        HostReportCategory::LAND_WARP_SHAPE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{9705CE98-FAF2-42F0-ACC6-EB9C73DACAAE}"),
        g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AS], _T("AS"), _T("C"), _T("um"),
        HostReportCategory::LAND_WARP_SHAPE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{9553D364-3AE6-4B66-8FCE-E3D14FC03A8F}"),
        g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AO], _T("AO"), _T("C"), _T("um"),
        HostReportCategory::LAND_WARP_SHAPE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    //}}
    UpdateSpec();

    __super::ResetSpec();

    m_DebugInfoGroup.Add(_T("PACKAGE BODY ROI"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("Object Spec Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Object Spec ROI"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("Object Search ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Gray Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Binary Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Object Blob Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Object Blob ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Object Point"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Object ROI"), enumDebugInfoType::PI_Rect);
    m_DebugInfoGroup.Add(_T("Mask Blob ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("SR Measure ROI"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("Object Mask Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Average Gray Mask Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Average Mask Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("SR Mask Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("SR Selected Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Warpage Shape Info"), enumDebugInfoType::None);
}

VisionInspectionLgaBottom3D::~VisionInspectionLgaBottom3D(void)
{
    CloseDlg();
    delete m_sEdgeAlignResult;
}

void VisionInspectionLgaBottom3D::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_VisionPara.Init();
}

void VisionInspectionLgaBottom3D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionLgaBottom3D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_visionUnit.RunInspection(m_visionUnit.GetVisionAlignProcessing(), false, GetCurVisionModule_Status());
    else
    {
        CString strError;
        strError.Format(_T("This module need execute the [%s] in the pre-process."),
            m_visionUnit.GetVisionProcessingModuleNameByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()));
        SimpleMessage(strError);
    }

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionLgaBottom3D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionLgaBottom3D::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->ShowImage();
    //m_pVisionInspDlg->SetInitParaWindow();

    return 0;
}

void VisionInspectionLgaBottom3D::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    /*if(m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
	{
		long nRejRectNum = (long)m_vecrtReject.size();
		std::vector<Ipvm::Rect32s> vecrtResizeReject(nRejRectNum);
		for(long i=0; i<nRejRectNum; i++)
		{
			vecrtResizeReject[i] = m_vecrtReject[i];

			vecrtResizeReject[i].m_left = m_vecrtReject[i].m_left / m_p3DMap->fResizeRateX + .5f;
			vecrtResizeReject[i].m_right = m_vecrtReject[i].m_right / m_p3DMap->fResizeRateX + .5f;
			vecrtResizeReject[i].m_top = m_vecrtReject[i].m_top / m_p3DMap->fResizeRateY + .5f;
			vecrtResizeReject[i].m_bottom = m_vecrtReject[i].m_bottom / m_p3DMap->fResizeRateY + .5f;
		}

		overlayResult->AddRectangles(vecrtResizeReject, RGB(255,0,0));
	}*/

    for (long nInsp = LAND3D_INSPECTION_START; nInsp < LAND3D_INSPECTION_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[nInsp].m_specName);
            if (pResult == nullptr)
                continue;
            if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                if (pResult->vecrtRejectROI.size())
                {
                    std::vector<Ipvm::Rect32s> vrtTemp;

                    overlayResult->AddRectangles(
                        *(std::vector<Ipvm::Rect32s>*)&pResult->vecrtRejectROI, RGB(255, 0, 0));

                    if (m_fixedInspectionSpecs[nInsp].m_useMarginal && pResult->vecrtMarginalROI.size())
                    {
                        overlayResult->AddRectangles(
                            *(std::vector<Ipvm::Rect32s>*)&pResult->vecrtMarginalROI, RGB(243, 157, 58));
                    }
                }

                else if (pResult->vecrtMarginalROI.size())
                {
                    overlayResult->AddRectangles(
                        *(std::vector<Ipvm::Rect32s>*)&pResult->vecrtMarginalROI, RGB(243, 157, 58));
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
                    std::vector<Ipvm::Rect32s> vrtTemp;

                    overlayResult->AddRectangles(
                        *(std::vector<Ipvm::Rect32s>*)&pResult->vecrtRejectROI, RGB(255, 0, 0));

                    if (m_variableInspectionSpecs[nInsp].m_useMarginal && pResult->vecrtMarginalROI.size())
                    {
                        overlayResult->AddRectangles(
                            *(std::vector<Ipvm::Rect32s>*)&pResult->vecrtMarginalROI, RGB(243, 157, 58));
                    }
                }

                else if (pResult->vecrtMarginalROI.size())
                {
                    overlayResult->AddRectangles(
                        *(std::vector<Ipvm::Rect32s>*)&pResult->vecrtMarginalROI, RGB(243, 157, 58));
                }
            }
        }
    }
}

void VisionInspectionLgaBottom3D::AppendTextResult(CString& textResult)
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
    //{{//Special parameters for this module only.
    if (vecbUseInsp[_3DINSP_WARPAGE] || vecbUseInsp[_3DINSP_UNIT_WARPAGE])
    {
        textResult.AppendFormat(_T("         Sign : %s\r\n         Shape : %s\r\n         R : %.4lf\r\n         B4 : ")
                                _T("%.4lf\r\n         B5 : %.4lf\r\n         Angle : %.4lf\r\n"),
            (LPCTSTR)m_visionWarpageShapeResult.m_sign, (LPCTSTR)m_visionWarpageShapeResult.m_shape,
            m_visionWarpageShapeResult.m_R, m_visionWarpageShapeResult.m_B4_um, m_visionWarpageShapeResult.m_B5_um,
            m_visionWarpageShapeResult.m_angle_deg);
    }

    long nObjectTotalNum = m_packageSpec.m_LandMapManager->GetCount(); //Parameters for this module only.

    AppendDetailTextResult(textResult, nObjectTotalNum);
}

void VisionInspectionLgaBottom3D::ResetResult()
{
    __super::ResetResult();

    // Get Algorithm...
    m_psBlobInfo = getReusableMemory().GetBlobInfo();
    m_pBlob = getReusableMemory().GetBlob();

    // 무조건 X:Y Scale 1:1 모드로...

    float f2DSacale = getScale().pixelToUm().m_x;
    if (f2DSacale > getScale().pixelToUm().m_y)
        f2DSacale = getScale().pixelToUm().m_y;

    m_fptPixelperMM = Ipvm::Point32r2(1000.f / f2DSacale, 1000.f / f2DSacale);

    //{{mc_GroupID 갯수를 Layer개념으로 사용한다.
    m_vecLandData.clear();
    for (const auto& LandMapData : m_packageSpec.m_LandMapManager->vecLandData)
    {
        if (LandMapData.bIgnore)
        {
            continue;
        }

        m_vecLandData.push_back(LandMapData);
    }

    m_result.vecstrID.clear();
    m_result.vecstrGroupID.clear(); //kircheis_MED2.5
    m_result.vecfptSpecLandUM.clear();

    for (const auto& landInfo : m_vecLandData)
    {
        m_result.vecstrID.push_back(landInfo.strLandID);
        m_result.vecstrGroupID.push_back(landInfo.m_groupID); //kircheis_MED2.5
        m_result.vecfptSpecLandUM.push_back(Ipvm::Point32r2(landInfo.fOffsetX, landInfo.fOffsetY));
    }

    getGroupInfo(m_packageSpec.m_LandMapManager->vecLandData, m_group_id_list);

    std::sort(m_group_id_list.begin(), m_group_id_list.end());
    m_vec2LandData.clear();

    long ParameterSize = long(m_VisionPara.m_vecLandParameter.size());

    //LandInfo를 Layer개념으로 분기한다.
    m_vec2LandData.resize(m_group_id_list.size());
    m_VisionPara.m_vecLandParameter.resize(m_group_id_list.size());

    if (ParameterSize < long(m_group_id_list.size()) && ParameterSize != 0)
    {
        for (int i = ParameterSize; i < long(m_group_id_list.size()); i++)
        {
            m_VisionPara.m_vecLandParameter[i].Copy(m_VisionPara.m_vecLandParameter[0]);
        }
    }

    for (long nLayer = 0; nLayer < long(m_group_id_list.size()); nLayer++)
    {
        long LandType = -1;

        for (const auto& landInfo : m_vecLandData)
        {
            if (m_group_id_list[nLayer] == landInfo.m_groupID)
            {
                m_vec2LandData[nLayer].push_back(landInfo);
                LandType = landInfo.nLandShapeType;
            }
        }

        if (LandType != -1)
            m_VisionPara.m_vecLandParameter[nLayer].LandType = LandType;
    }
}

BOOL VisionInspectionLgaBottom3D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_VisionPara.LinkDataBase(bSave, db[_T("{F3414C66-B5C1-4186-87EB-2762800B4A45}")]))
        return FALSE;

    //kk 이 코드를 호출할 위치가 마땅치 않아 일단 여기에서 호출
    //Package Layout에서 MapData 수정 후 Land 3D 검사 들어갈 시 이전 MapData의 정보들을 들고 들어가는 문제로 인해
    //LinkDataBase에서 Data를 초기화 해주는 역할을 넣어놓음

    std::vector<CString> newGroupList;
    getGroupInfo(m_packageSpec.m_LandMapManager->vecLandData, newGroupList);

    long new_groupCount = long(newGroupList.size());

    if (new_groupCount < 2)
    {
        m_VisionPara.m_nGroupUnitCoplCount = 0;
        m_VisionPara.m_vec2UnitCoplGroup.resize(0);
        m_VisionPara.m_vecstrGroupUnitCoplName.resize(0);
        m_VisionPara.m_vecstrGroupInspName.resize(0);
    }
    else if (long(m_group_id_list.size()) != new_groupCount)
    {
        m_VisionPara.m_vecstrGroupInspName.resize(new_groupCount);

        if (long(m_group_id_list.size()) < new_groupCount)
        {
            for (int i = long(m_group_id_list.size()); i < new_groupCount; i++)
                m_VisionPara.m_vecstrGroupInspName[i].SetName(newGroupList[i]);
        }
    }

    m_group_id_list = newGroupList;

    UpdateSpec();

    return TRUE;
}

void VisionInspectionLgaBottom3D::DeleteSpec(long specIndex)
{
    if (specIndex >= 0 && specIndex < long(m_variableInspectionSpecs.size()))
    {
        m_variableInspectionSpecs.erase(m_variableInspectionSpecs.begin() + specIndex);
    }

    UpdateSpec();
}

void VisionInspectionLgaBottom3D::UpdateSpec()
{
    UpdateSpec(nullptr);
}

void VisionInspectionLgaBottom3D::UpdateSpec(const CString i_strInspectionName)
{
    //kk 이전 버전 사용 Job들을 아예 다시 셋업 하면 상관이 없는데
    //이전 버전 Job을 그대로 사용할 때, GroupName이 만들어지지 않는게 문제
    //검사 항목 마다 초기화 하는 위치가 달라 한번 더 체크하도록 수정
    while (m_VisionPara.m_vecstrGroupInspName.size() != m_group_id_list.size())
    {
        if (m_VisionPara.m_vecstrGroupInspName.size() < m_group_id_list.size())
        {
            long index = long(m_VisionPara.m_vecstrGroupInspName.size());

            Land3DGroupInsp Data;

            Data.SetName(m_group_id_list[index]);

            m_VisionPara.m_vecstrGroupInspName.push_back(Data);
        }
        else
        {
            m_VisionPara.m_vecstrGroupInspName.erase(m_VisionPara.m_vecstrGroupInspName.end() - 1);
        }
    }

    long SpecNum = long(m_variableInspectionSpecs.size());
    long DefaultNum
        = (long)m_defaultFixedInspectionSpecs.size() - NOT_USE_GROUP_INSPECTION; //kk Warapge항목중 AX,AV,AS,AO 제외
    long MakeSpecNum = (long)m_VisionPara.m_vecstrGroupUnitCoplName.size()
        + ((long)m_VisionPara.m_vecstrGroupInspName.size() * DefaultNum);

    while (SpecNum < MakeSpecNum)
    {
        long Index = SpecNum;

        if (Index < 0)
            return;

        CString strSpecInital;
        strSpecInital.Format(_T("LUCP_G%d"), Index);
        m_variableInspectionSpecs.emplace_back(m_moduleGuid, ::CreateGUID(), m_strModuleName, i_strInspectionName,
            strSpecInital, _T("C"), _T("um"), HostReportCategory::LAND, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f,
            0.f, FALSE, FALSE, FALSE);

        SpecNum = long(m_variableInspectionSpecs.size());
    }

    while (SpecNum > MakeSpecNum)
    {
        m_variableInspectionSpecs.erase(m_variableInspectionSpecs.end() - 1);

        SpecNum = long(m_variableInspectionSpecs.size());
    }

    std::vector<CString> InspectionName;

    if (m_group_id_list.size() != 1)
    {
        if (m_VisionPara.m_vecstrGroupInspName.size() != 0)
        {
            for (int i = 0; i < long(m_group_id_list.size()); i++)
            {
                InspectionName.push_back(m_VisionPara.m_vecstrGroupInspName[i].LandCoplName);
                InspectionName.push_back(m_VisionPara.m_vecstrGroupInspName[i].LandUnitCoplName);
                InspectionName.push_back(m_VisionPara.m_vecstrGroupInspName[i].LandHeightName);
                InspectionName.push_back(m_VisionPara.m_vecstrGroupInspName[i].LandWarpageName);
                InspectionName.push_back(m_VisionPara.m_vecstrGroupInspName[i].LandUnitWarpageName);
            }
        }

        InspectionName.insert(InspectionName.end(), m_VisionPara.m_vecstrGroupUnitCoplName.begin(),
            m_VisionPara.m_vecstrGroupUnitCoplName.end());
    }
    else if (m_group_id_list.size() <= 1) //kk Ball Type이 한개 이하일 경우 필요 없습니다. 다시 호출해서 설정합시다.
    {
        m_variableInspectionSpecs.clear();
        m_VisionPara.m_vecstrGroupInspName.clear();
        SpecNum = 0;
    }

    for (long idx = 0; idx < long(SpecNum); idx++)
    {
        CString specName;
        specName.Format(_T("LUCP_G%d"), idx);

        m_variableInspectionSpecs[idx].m_specName = InspectionName[idx];
        m_variableInspectionSpecs[idx].m_shortName = specName;

        m_variableInspectionSpecs[idx].m_unit = _T("um");

        m_variableInspectionSpecs[idx].MakeSpecFullName();
    }

    GetVisionInspectionSpecs();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->ResetSpecAndResultDlg();
    }
}

BOOL VisionInspectionLgaBottom3D::MakeInspectionName(long targetindex, CString& o_InspectionName)
{
    long maxCount = long(m_VisionPara.m_vec2UnitCoplGroup[targetindex].size());

    if (maxCount <= 0 || maxCount > long(m_group_id_list.size()))
    {
        o_InspectionName = _T("Untitle");
        return false;
    }

    o_InspectionName = _T("Land Group Unit Copl ");

    for (int i = 0; i < maxCount; i++)
    {
        long groupIndex = m_VisionPara.m_vec2UnitCoplGroup[targetindex][i];
        CString groupID = m_group_id_list[groupIndex];

        CString str;
        str.Format(_T("%s"), LPCTSTR(groupID));

        o_InspectionName += str;

        if (i != maxCount - 1)
            o_InspectionName += _T("_");
    }

    return true;
}

void VisionInspectionLgaBottom3D::GetVisionInspectionSpecs()
{
    m_VisionPara.m_vecVisionInspectionSpecs.clear();

    for (auto& spec : m_fixedInspectionSpecs)
    {
        m_VisionPara.m_vecVisionInspectionSpecs.push_back(spec);
    }

    for (auto& spec : m_variableInspectionSpecs)
    {
        m_VisionPara.m_vecVisionInspectionSpecs.push_back(spec);
    }
}

void VisionInspectionLgaBottom3D::SetVisionInspectionSpecs()
{
    for (auto& spec : m_VisionPara.m_vecVisionInspectionSpecs)
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

std::vector<CString> VisionInspectionLgaBottom3D::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara.ExportAlgoParaToText(strVisionName, m_strModuleName);
    std::vector<CString> vecstrInspectionSpec = ExportInspectionSpecToText(strVisionName);

    vecStrResult.insert(vecStrResult.end(), vecstrInspectionSpec.begin(), vecstrInspectionSpec.end());

    return vecStrResult;
}