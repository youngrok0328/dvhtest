//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBottom3D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBottom3D.h"
#include "VisionInspectionBgaBottom3DPara.h"
#include "VisionInspectionBgaBottom3DResult.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <iomanip>
#include <map>
#include <sstream>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NOT_USE_GROUP_INSPECTION 4

//CPP_7_________________________________ Implementation body
//
VisionInspectionBgaBottom3D::VisionInspectionBgaBottom3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_BGA_BALL_3D, _VISION_INSP_NAME_BGA_BALL_3D, visionUnit, packageSpec)
    , m_sEdgeAlignResult(new VisionAlignResult)
    , m_pVisionPara(new VisionInspectionBgaBottom3DPara)
    , m_result(new VisionInspectionBgaBottom3DResult)
{
    m_pVisionInspDlg = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{51FF1A08-EFD5-4158-82A4-E8D51B2DEEA3}"),
        g_szBGA3DInspectionName[_3DINSP_COPL], _T("COPL"), _T("C"), _T("um"), HostReportCategory::BALL,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{2C12F23D-0F5B-4458-9A1F-8670E41BEAA2}"),
        g_szBGA3DInspectionName[_3DINSP_UNIT_COPL], _T("U.COPL"), _T("C"), _T("um"), HostReportCategory::BALL,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{17BF3666-777B-455F-A3B8-985AACB74AD8}"),
        g_szBGA3DInspectionName[_3DINSP_HEIGHT], _T("HEIG"), _T("C"), _T("um"), HostReportCategory::BALL,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{0FDFB15F-6A39-4C77-88B5-2D5F064F4571}"),
        g_szBGA3DInspectionName[_3DINSP_WARPAGE], _T("WARP"), _T("C"), _T("um"), HostReportCategory::BALL,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{14F13D57-A668-4290-8552-88F290687472}"),
        g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE], _T("U.WARP"), _T("C"), _T("um"), HostReportCategory::BALL,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    //{{//kircheis_MED2LW
    m_defaultFixedInspectionSpecs.emplace_back(_T("{E5890726-030B-478A-8315-2AFAE43AC570}"),
        g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE_AX], _T("AX"), _T("C"), _T("um"),
        HostReportCategory::BALL_WARP_SHAPE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{CC2E6605-E831-401C-BAB5-E9B24BC6D720}"),
        g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE_AV], _T("AV"), _T("C"), _T("um"),
        HostReportCategory::BALL_WARP_SHAPE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{75738D18-C5D2-4B9E-AB29-405AF9ABA098}"),
        g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE_AS], _T("AS"), _T("C"), _T("um"),
        HostReportCategory::BALL_WARP_SHAPE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{AD798122-888C-4ACE-84F9-6072FE631AC9}"),
        g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE_AO], _T("AO"), _T("C"), _T("um"),
        HostReportCategory::BALL_WARP_SHAPE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    //}}

    __super::ResetSpec();

    UpdateSpec();

    m_DebugInfoGroup.Add(_T("Ball 3D Spec ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Ball 3D Search ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Ball 3D Blob Positions"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Ball 3D Aligned Positions"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Ball 3D Aligned ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Ball 3D Aligned Ellipses"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("Ball 3D Substrate ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Substrate Mask Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Substrate z values"), enumDebugInfoType::Float);
    m_DebugInfoGroup.Add(_T("Invalid ball IDs [Copl]"), enumDebugInfoType::CSTRING); //kircheis_AIBC
    m_DebugInfoGroup.Add(_T("Invalid ball IDs [Ball Height]"), enumDebugInfoType::CSTRING); //kircheis_AIBC
    m_DebugInfoGroup.Add(_T("Warpage Shape Info"), enumDebugInfoType::None);

    for (long type = 0; type < long(enumGetherBallZType::END); type++)
    {
        CString typeName;
        typeName.Format(_T("[%s]"), GetBallZTypeString(enumGetherBallZType(type)));

        m_DebugInfoGroup.Add(typeName + _T(" Ball Logical Pixel Image"), enumDebugInfoType::Image_8u_C1);
        m_DebugInfoGroup.Add(typeName + _T(" Ball z values"), enumDebugInfoType::Float);
        m_DebugInfoGroup.Add(typeName + _T(" Masked Height Image"), enumDebugInfoType::Image_8u_C1);

        typeName.Empty();
    }

    m_DebugInfoGroup.Add(_T("[Debug] ZMap"), enumDebugInfoType::Image_32f_C1);
}

void VisionInspectionBgaBottom3D::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_pVisionPara->Init();
}

VisionInspectionBgaBottom3D::~VisionInspectionBgaBottom3D(void)
{
    CloseDlg();

    delete m_result;
    delete m_pVisionPara;
    delete m_sEdgeAlignResult;
}

void VisionInspectionBgaBottom3D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionBgaBottom3D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
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
        strError.Empty();
    }

    CollectBodyAlignResult();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionBgaBottom3D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionBgaBottom3D::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionInspectionBgaBottom3D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_pVisionPara->LinkDataBase(bSave, db[_T("{1BCE0510-AB7A-4301-B069-690DD2A3A919}")],
            long(m_packageSpec.m_originalballMap->m_ballTypes.size())))
        return FALSE;

    //kk 이 코드를 호출할 위치가 마땅치 않아 일단 여기에서 호출
    //Package Layout에서 MapData 수정 후 Land 3D 검사 들어갈 시 이전 MapData의 정보들을 들고 들어가는 문제로 인해
    //LinkDataBase에서 Data를 초기화 해주는 역할을 넣어놓음
    long nLayerNum = long(m_packageSpec.m_originalballMap->m_ballTypes.size());
    long SpecNum = long(m_pVisionPara->m_vecstrGroupInspName.size());

    if (nLayerNum < 3)
    {
        m_pVisionPara->m_nGroupUnitCoplCount = 0;
        m_pVisionPara->m_vec2UnitCoplGroup.resize(0);
        m_pVisionPara->m_vecstrGroupUnitCoplName.resize(0);
    }

    if (nLayerNum < 2)
    {
        m_pVisionPara->m_vecstrGroupInspName.resize(0);
    }
    else
    {
        if (SpecNum != nLayerNum)
        {
            m_pVisionPara->m_vecstrGroupInspName.resize(nLayerNum);

            if (SpecNum < nLayerNum)
            {
                for (int i = 0; i < nLayerNum; i++)
                {
                    auto groupID = m_packageSpec.m_originalballMap->m_ballTypes[i].m_groupID;
                    m_pVisionPara->m_vecstrGroupInspName[i].SetName(groupID);
                }
            }
        }
    }

    UpdateSpec();

    return TRUE;
}

// 영훈 20150116 : 검사가 끝나면 불량은 표시해주도록 한다.
void VisionInspectionBgaBottom3D::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    //for(long nInsp = _3DINSP_START ; nInsp < /*_3DINSP_END*/_3DINSP_WARPAGE ; nInsp++)
    for (long nInsp = _3DINSP_START; nInsp < _3DINSP_END; nInsp++) //kircheis_201611XX
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szBGA3DInspectionName[nInsp]);

            if (pResult != NULL)
            {
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

                    overlayResult->AddLine(m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, RGB(0, 255, 0));
                    overlayResult->AddLine(m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptRB, RGB(0, 255, 0));
                    overlayResult->AddLine(m_sEdgeAlignResult->fptRB, m_sEdgeAlignResult->fptLB, RGB(0, 255, 0));
                    overlayResult->AddLine(m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptLT, RGB(0, 255, 0));
                }
            }
        }
    }

    const long variableNum = long(m_variableInspectionSpecs.size());
    for (long nInsp = 0; nInsp < variableNum; nInsp++)
    {
        if (m_variableInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult
                = m_resultGroup.GetResultByName(m_variableInspectionSpecs[nInsp].m_specName);

            if (pResult != NULL)
            {
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

                    overlayResult->AddLine(m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, RGB(0, 255, 0));
                    overlayResult->AddLine(m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptRB, RGB(0, 255, 0));
                    overlayResult->AddLine(m_sEdgeAlignResult->fptRB, m_sEdgeAlignResult->fptLB, RGB(0, 255, 0));
                    overlayResult->AddLine(m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptLT, RGB(0, 255, 0));
                }
            }
        }
    }
}

void VisionInspectionBgaBottom3D::AppendTextResult(CString& textResult)
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

    if (vecbUseInsp[_3DINSP_WARPAGE] || vecbUseInsp[_3DINSP_UNIT_WARPAGE])
    {
        textResult.AppendFormat(_T("         Sign : %s\r\n         Shape : %s\r\n         R : %.4lf\r\n         B4 : ")
                                _T("%.4lf\r\n         B5 : %.4lf\r\n         Angle : %.4lf\r\n"),
            (LPCTSTR)m_visionWarpageShapeResult.m_sign, (LPCTSTR)m_visionWarpageShapeResult.m_shape,
            m_visionWarpageShapeResult.m_R, m_visionWarpageShapeResult.m_B4_um, m_visionWarpageShapeResult.m_B5_um,
            m_visionWarpageShapeResult.m_angle_deg);
    }

    long nObjectTotalNum = (long)m_packageSpec.m_ballMap->m_balls.size(); //Parameters for this module only.

    AppendDetailTextResult(textResult, nObjectTotalNum);

    for (int nIdx = 0; nIdx < vecstrInspShortName.size(); nIdx++)
    {
        vecstrInspShortName[nIdx].Empty();
    }
}

void VisionInspectionBgaBottom3D::ResetResult()
{
    __super::ResetResult();
}

void VisionInspectionBgaBottom3D::PrintOutResult(VisionInspectionResult* pResult, LPCTSTR szItemName,
    BOOL& bBallMeasured, BOOL bDeviceLevelResult, CString& strText, CString& strLogText)
{
    static CString strTemp;
    static CString strLogTemp;

    long nSize = (long)(pResult->m_objectErrorValues.size());
    float fWorst(-1.f);
    long nWorstID(0);

    if (nSize > 0)
    {
        for (long i = 0; i < nSize; i++)
        {
            if (fabs(fWorst) <= fabs(pResult->m_objectErrorValues[i]))
            {
                fWorst = pResult->m_objectErrorValues[i];
                nWorstID = i;
            }
        }

        if (fWorst == -1.f)
        {
            fWorst = 0.f;
        }

        LPCTSTR szStrResult[]
            = {_T("Not Measured"), _T("Pass"), _T("Marginal"), _T("Reject"), _T("Invalid"), _T("Empty"), _T("Double")};

        if (IsValid(pResult->m_totalResult))
        {
            if (bDeviceLevelResult)
            {
                strTemp.Format(_T("      %s : %.2f --> %s\n"), szItemName, fWorst, szStrResult[pResult->m_totalResult]);
                strLogTemp.Format(
                    _T("      %s,%.2f,-->,%s\n"), szItemName, fWorst, szStrResult[pResult->m_totalResult]);
            }
            else
            {
                bBallMeasured = TRUE;
                strTemp.Format(_T("      %s : %.2f, Worst ID : %s --> %s\n"), szItemName, fWorst,
                    (LPCTSTR)pResult->m_objectNames[nWorstID], szStrResult[pResult->m_totalResult]);
                strLogTemp.Format(_T("      %s,%.2f,Worst ID,%s,-->,%s\n"), szItemName, fWorst,
                    (LPCTSTR)pResult->m_objectNames[nWorstID], szStrResult[pResult->m_totalResult]);
            }
        }
        else //kircheis_201611XX
        {
            if (pResult->m_totalResult == NOT_MEASURED)
            {
                strTemp.Format(_T("      %s is [Not Measured].\n"), szItemName);
                strLogTemp.Format(_T("      %s is [Not Measured].\n"), szItemName);
            }
            else
            {
                CString strResultText;
                switch (pResult->m_totalResult)
                {
                    case INVALID:
                        strResultText.Format(_T("Invalid"));
                        break;
                    case EMPTY:
                        strResultText.Format(_T("Empty"));
                        break;
                    case DOUBLEDEVICE:
                        strResultText.Format(_T("Double Device"));
                        break;
                    default:
                        strResultText.Format(_T("NULL"));
                        break;
                }
                strTemp.Format(_T("      %s result is [%s].\n"), szItemName, (LPCTSTR)strResultText);
                strLogTemp.Format(_T("      %s result is [%s].\n"), szItemName, (LPCTSTR)strResultText);

                strResultText.Empty();
            }
        }
    }
    else
    {
        strTemp.Format(_T("      %s is NULL.\n"), szItemName);
        strLogTemp.Format(_T("      %s is NULL.\n"), szItemName);
    }

    strText += strTemp;
    strLogText += strLogTemp;

    strTemp.Empty();
    strLogTemp.Empty();
}

BOOL VisionInspectionBgaBottom3D::IsValid(int nResult)
{
    return nResult != NOT_MEASURED && nResult != INVALID && nResult != EMPTY && nResult != DOUBLEDEVICE;
}

void VisionInspectionBgaBottom3D::ResetInspItem(long nNum)
{
    for (long nInsp = _3DINSP_START; nInsp < _3DINSP_END; nInsp++)
    {
        m_resultGroup.GetResultByName(g_szBGA3DInspectionName[nInsp])->Clear();
        m_resultGroup.GetResultByName(g_szBGA3DInspectionName[nInsp])->Resize(nNum);
    }
}

void VisionInspectionBgaBottom3D::DeleteSpec(long specIndex)
{
    if (specIndex >= 0 && specIndex < long(m_variableInspectionSpecs.size()))
    {
        m_variableInspectionSpecs.erase(m_variableInspectionSpecs.begin() + specIndex);
    }

    UpdateSpec();
}

void VisionInspectionBgaBottom3D::UpdateSpec()
{
    UpdateSpec(nullptr);
}

void VisionInspectionBgaBottom3D::UpdateSpec(const CString i_strInspectionName)
{
    //kk 이전 버전 사용 Job들을 아예 다시 셋업 하면 상관이 없는데
    //이전 버전 Job을 그대로 사용할 때, GroupName이 만들어지지 않는게 문제
    //검사 항목 마다 초기화 하는 위치가 달라 한번 더 체크하도록 수정
    long nLayerNum = long(m_packageSpec.m_originalballMap->m_ballTypes.size());

    while (m_pVisionPara->m_vecstrGroupInspName.size() != nLayerNum)
    {
        if (m_pVisionPara->m_vecstrGroupInspName.size() < nLayerNum)
        {
            long index = long(m_pVisionPara->m_vecstrGroupInspName.size());
            BallGroupInsp Data;

            auto groupID = m_packageSpec.m_originalballMap->m_ballTypes[index].m_groupID;
            Data.SetName(groupID);

            m_pVisionPara->m_vecstrGroupInspName.push_back(Data);
        }
        else
        {
            m_pVisionPara->m_vecstrGroupInspName.erase(m_pVisionPara->m_vecstrGroupInspName.end() - 1);
        }
    }

    long SpecNum = long(m_variableInspectionSpecs.size());
    long DefaultNum
        = long(m_defaultFixedInspectionSpecs.size()) - NOT_USE_GROUP_INSPECTION; //kk Warapge항목중 AX,AV,AS,AO 제외
    long MakeSpecNum = long(m_pVisionPara->m_vecstrGroupUnitCoplName.size())
        + (long(m_pVisionPara->m_vecstrGroupInspName.size()) * DefaultNum);

    while (SpecNum < MakeSpecNum)
    {
        long Index = SpecNum;

        if (Index < 0)
            return;

        CString strSpecInital;
        strSpecInital.Format(_T("BUCP_G%d"), Index);
        m_variableInspectionSpecs.emplace_back(m_moduleGuid, ::CreateGUID(), m_strModuleName, i_strInspectionName,
            strSpecInital, _T("C"), _T("um"), HostReportCategory::BALL, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f,
            0.f, FALSE, FALSE, FALSE);

        SpecNum = long(m_variableInspectionSpecs.size());

        strSpecInital.Empty();
    }

    while (SpecNum > MakeSpecNum)
    {
        m_variableInspectionSpecs.erase(m_variableInspectionSpecs.end() - 1);

        SpecNum = long(m_variableInspectionSpecs.size());
    }

    std::vector<CString> InspectionName;
    long ballCount = long(m_packageSpec.m_originalballMap->m_ballTypes.size());

    if (ballCount != 1)
    {
        if (m_pVisionPara->m_vecstrGroupInspName.size() != 0)
        {
            for (int i = 0; i < ballCount; i++)
            {
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallCoplName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallUnitCoplName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallHeightName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallWarpageName);
                InspectionName.push_back(m_pVisionPara->m_vecstrGroupInspName[i].BallUnitWarpageName);
            }
        }

        InspectionName.insert(InspectionName.end(), m_pVisionPara->m_vecstrGroupUnitCoplName.begin(),
            m_pVisionPara->m_vecstrGroupUnitCoplName.end());
    }
    else if (ballCount <= 1) //kk Ball Type이 한개 이하일 경우 필요 없습니다.
    {
        m_variableInspectionSpecs.clear();
        m_pVisionPara->m_vecstrGroupInspName.clear();
        SpecNum = 0;
    }

    for (long index = 0; index < long(SpecNum); index++)
    {
        CString specName;
        specName.Format(_T("BUCP_G%d"), index);
        long idx = index;

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

BOOL VisionInspectionBgaBottom3D::MakeInspectionName(long targetindex, CString& o_InspectionName)
{
    long maxCount = long(m_pVisionPara->m_vec2UnitCoplGroup[targetindex].size());

    if (maxCount <= 0 || maxCount > long(m_packageSpec.m_originalballMap->m_ballTypes.size()))
    {
        o_InspectionName = _T("Untitle");
        return false;
    }

    o_InspectionName = _T("Ball Group Unit Copl ");

    for (int i = 0; i < maxCount; i++)
    {
        long groupIndex = m_pVisionPara->m_vec2UnitCoplGroup[targetindex][i];
        CString groupID = m_packageSpec.m_originalballMap->m_ballTypes[groupIndex].m_groupID;

        CString str;
        str.Format(_T("%s"), LPCTSTR(groupID));

        o_InspectionName += str;

        if (i != maxCount - 1)
            o_InspectionName += _T("_");

        groupID.Empty();
        str.Empty();
    }

    return true;
}

void VisionInspectionBgaBottom3D::GetVisionInspectionSpecs()
{
    m_pVisionPara->m_vecVisionInspectionSpecs.clear();

    for (auto& spec : m_fixedInspectionSpecs)
    {
        m_pVisionPara->m_vecVisionInspectionSpecs.push_back(spec);
    }

    for (auto& spec : m_variableInspectionSpecs)
    {
        m_pVisionPara->m_vecVisionInspectionSpecs.push_back(spec);
    }
}

void VisionInspectionBgaBottom3D::SetVisionInspectionSpecs()
{
    for (auto& spec : m_pVisionPara->m_vecVisionInspectionSpecs)
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

std::vector<CString> VisionInspectionBgaBottom3D::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_pVisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);
    std::vector<CString> vecstrInspectionSpec = ExportInspectionSpecToText(strVisionName);

    vecStrResult.insert(vecStrResult.end(), vecstrInspectionSpec.begin(), vecstrInspectionSpec.end());

    return vecStrResult;
}