//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionGeometry.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionGeometry.h"
#include "VisionInspectionGeometryPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
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

//CPP_7_________________________________ Implementation body
//
VisionInspectionGeometry::VisionInspectionGeometry(
    LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(moduleGuid, moduleName, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionGeometryPara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
{
    m_pVisionInspDlg = NULL;
    UpdateSpec();

    __super::ResetSpec();

    ResizeResultAndDebugInfo();
}

void VisionInspectionGeometry::ResetSpecAndPara()
{
    __super::ResetSpec();

    ResizeResultAndDebugInfo();

    m_VisionPara->Init();
}

void VisionInspectionGeometry::ResetResult()
{
    __super::ResetResult();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    m_fPixelToUm = getScale().pixelToUmXY();
}

VisionInspectionGeometry::~VisionInspectionGeometry(void)
{
    CloseDlg();

    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspectionGeometry::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionGeometry::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    m_visionUnit.RunInspection(
        m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_OTHER_ALIGN), false, GetCurVisionModule_Status());

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionGeometry(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionGeometry::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->ShowImage();
    //m_pVisionInspDlg->SetInitParaWindow();

    return 0;
}

void VisionInspectionGeometry::DeleteSpec(long specIndex)
{
    if (specIndex >= 0 && specIndex < long(m_variableInspectionSpecs.size()))
    {
        m_variableInspectionSpecs.erase(m_variableInspectionSpecs.begin() + specIndex);
    }

    UpdateSpec();
}

void VisionInspectionGeometry::UpdateSpec()
{
    while (m_variableInspectionSpecs.size() < m_VisionPara->m_vecstrInspectionName.size())
    {
        CString strSpecInital;
        strSpecInital.Format(_T("Geo%d"), m_variableInspectionSpecs.size());
        m_variableInspectionSpecs.emplace_back(m_moduleGuid, ::CreateGUID(), m_strModuleName,
            m_VisionPara->m_vecstrInspectionName[m_variableInspectionSpecs.size()], strSpecInital, _T("W"), _T("um"),
            HostReportCategory::GEOMETRY, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

        strSpecInital.Empty();
    }

    while (m_variableInspectionSpecs.size() > m_VisionPara->m_vecstrInspectionName.size())
    {
        m_variableInspectionSpecs.erase(m_variableInspectionSpecs.end() - 1);
    }

    for (long index = 0; index < long(m_variableInspectionSpecs.size()); index++)
    {
        CString specName;
        specName.Format(_T("Geo%d"), index);

        m_variableInspectionSpecs[index].m_specName = m_VisionPara->m_vecstrInspectionName[index];
        m_variableInspectionSpecs[index].m_shortName = specName;

        if (m_VisionPara->m_vecnInspectionType[index] == Insp_Type_Shape_Angle)
        {
            m_variableInspectionSpecs[index].m_unit = _T("Deg");
        }
        else
        {
            m_variableInspectionSpecs[index].m_unit = _T("um");
        }

        m_variableInspectionSpecs[index].MakeSpecFullName();

        specName.Empty();
    }

    ResizeResultAndDebugInfo();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->ResetSpecAndResultDlg();
    }
}

BOOL VisionInspectionGeometry::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{FFBD470B-F132-48DE-8037-E10709DCB418}")]))
        return FALSE;

    if (!bSave)
    {
        UpdateSpec();
    }

    // Multi Spec과 마찬가지로 Setup 이후 모든 Spec은 job이 저장하고 있으므로 Result를 다시 꾸며준다.
    ResizeResultAndDebugInfo();

    return TRUE;
}

void VisionInspectionGeometry::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_3D_INSP)
    {
        if (m_nOverlayMode == OverlayType_ShowAll)
        {
            long nInspNum = (long)m_VisionPara->m_vecnInspectionType.size();
            long nParaInfoRefNum = (long)m_VisionPara->m_vecInfoParameter_0_Ref.size(); //kircheis_Crash20170201

            std::vector<std::vector<long>> vecInfoParameter_0_Ref;

            long refNum = (long)m_vecAlignInfoData_Ref.size();
            long targetNum = (long)m_vecAlignInfoData_Tar.size();

            long dataNum = (long)min(nInspNum, min(refNum, targetNum));

            for (long nInspID = 0; nInspID < dataNum; nInspID++)
            {
                //{{ kircheis_Crash20170201
                if (nInspID >= nParaInfoRefNum)
                    return;
                //}}

                std::vector<Ipvm::Point32r2> vecfptData;

                long InspectionType = m_VisionPara->m_vecnInspectionType[nInspID];
                vecInfoParameter_0_Ref = m_VisionPara->m_vecInfoParameter_0_Ref[nInspID];

                if (InspectionType == Insp_Type_Shape_Distance)
                {
                    if ((m_vecAlignInfoData_Ref[nInspID].strName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
                            || m_vecAlignInfoData_Ref[nInspID].strName
                                == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
                        && (m_vecAlignInfoData_Tar[nInspID].strName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
                            || m_vecAlignInfoData_Tar[nInspID].strName
                                == g_szAlignInfo_List_Name[Insp_Type_User_Ellips]))
                    {
                        if (vecInfoParameter_0_Ref[0][ED_Param_Edge_Detect_Area] == 0)
                        {
                            vecfptData.emplace_back(m_vecAlignInfoData_Ref[nInspID].CircleData.m_x,
                                m_vecAlignInfoData_Ref[nInspID].CircleData.m_y);
                            vecfptData.emplace_back(m_vecAlignInfoData_Tar[nInspID].CircleData.m_x,
                                m_vecAlignInfoData_Tar[nInspID].CircleData.m_y);

                            overlayResult->AddLine(vecfptData[0], vecfptData[1], RGB(0, 255, 0));
                            vecfptData.clear();
                        }
                    }
                    else if ((m_vecAlignInfoData_Ref[nInspID].strName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
                                 || m_vecAlignInfoData_Ref[nInspID].strName
                                     == g_szAlignInfo_List_Name[Insp_Type_User_Ellips]))
                    {
                        if (vecInfoParameter_0_Ref[0][ED_Param_Edge_Detect_Area] == 0)
                        {
                            vecfptData.emplace_back(m_vecAlignInfoData_Ref[nInspID].CircleData.m_x,
                                m_vecAlignInfoData_Ref[nInspID].CircleData.m_y);
                        }
                        else
                        {
                            Ipvm::Point32r2 pftStart(m_vecAlignInfoData_Ref[nInspID].CircleData.m_x,
                                m_vecAlignInfoData_Ref[nInspID].CircleData.m_y);
                            Ipvm::Point32r2 pftEnd(m_vecAlignInfoData_Tar[nInspID].vecfptData[0].m_x,
                                m_vecAlignInfoData_Tar[nInspID].vecfptData[0].m_y);

                            Ipvm::Point32r2 pftData = CPI_Geometry::GetLinePoint(
                                pftStart, pftEnd, m_vecAlignInfoData_Ref[nInspID].CircleData.m_radius);
                            vecfptData.push_back(pftData);
                        }

                        if ((long)m_vecAlignInfoData_Tar[nInspID].vecfptData.size() > 0)
                        {
                            vecfptData.emplace_back(m_vecAlignInfoData_Tar[nInspID].vecfptData[0].m_x,
                                m_vecAlignInfoData_Tar[nInspID].vecfptData[0].m_y);

                            overlayResult->AddLine(vecfptData[0], vecfptData[1], RGB(0, 255, 0));
                        }
                        vecfptData.clear();
                    }
                    else
                    {
                        if ((long)m_vecAlignInfoData_Ref[nInspID].vecfptData.size()
                            && (long)m_vecAlignInfoData_Tar[nInspID].vecfptData.size())
                        {
                            if ((m_vecAlignInfoData_Ref[nInspID].nDataType == DataTypeList::Line
                                    && m_vecAlignInfoData_Tar[nInspID].nDataType == DataTypeList::All_Point)
                                || (m_vecAlignInfoData_Ref[nInspID].nDataType == DataTypeList::All_Point
                                    && m_vecAlignInfoData_Tar[nInspID].nDataType
                                        == DataTypeList::Line)) // 라인과 여러개의 포인트 거리
                            {
                                vecfptData.push_back(m_vecAlignInfoData_Ref[nInspID].vecfptData[0]);
                                vecfptData.push_back(m_vecAlignInfoData_Ref[nInspID].vecfptData[1]);

                                overlayResult->AddLine(vecfptData[0], vecfptData[1], RGB(0, 255, 0));
                                vecfptData.clear();

                                vecfptData.push_back(m_vecAlignInfoData_Ref[nInspID].vecfptData[0]);
                                vecfptData.push_back(m_vecAlignInfoData_Tar[nInspID].vecfptData[1]);

                                overlayResult->AddLine(vecfptData[0], vecfptData[1], RGB(0, 255, 0));
                                vecfptData.clear();
                            }
                            else
                            {
                                vecfptData.push_back(m_vecAlignInfoData_Ref[nInspID].vecfptData[0]);
                                vecfptData.push_back(m_vecAlignInfoData_Tar[nInspID].vecfptData[0]);

                                overlayResult->AddLine(vecfptData[0], vecfptData[1], RGB(0, 255, 0));
                                vecfptData.clear();
                            }
                        }
                    }
                }
                else if (nInspNum == Insp_Type_Shape_Circle)
                {
                    Ipvm::Point32r2 pos(
                        m_vecAlignInfoData_Ref[nInspID].CircleData.m_x, m_vecAlignInfoData_Ref[nInspID].CircleData.m_y);

                    Ipvm::Rect32r frtROI;
                    frtROI.m_left = pos.m_x - m_vecAlignInfoData_Ref[nInspID].CircleData.m_radius;
                    frtROI.m_right = pos.m_x + m_vecAlignInfoData_Ref[nInspID].CircleData.m_radius;
                    frtROI.m_top = pos.m_y - m_vecAlignInfoData_Ref[nInspID].CircleData.m_radius;
                    frtROI.m_bottom = pos.m_y + m_vecAlignInfoData_Ref[nInspID].CircleData.m_radius;
                    overlayResult->AddEllipse(frtROI, RGB(0, 255, 0));

                    Ipvm::Point32r2 fptStart = pos;
                    Ipvm::Point32r2 fptEnd(pos.m_x - m_vecAlignInfoData_Ref[nInspID].CircleData.m_radius, pos.m_y);

                    overlayResult->AddLine(fptStart, fptEnd, RGB(0, 255, 0));
                }
            }
        }
    }
}

void VisionInspectionGeometry::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);
}

void VisionInspectionGeometry::ResizeResultAndDebugInfo()
{
    ResetResult();

    m_DebugInfoGroup.m_vecDebugInfo.clear();

    m_DebugInfoGroup.Add(_T("All Line Data"), enumDebugInfoType::LineSeg_32f);

    CString strDebugInfoName;

    for (long n = 0; n < m_variableInspectionSpecs.size(); n++)
    {
        strDebugInfoName.Format(_T("Reference Point Data %d"), n);
        m_DebugInfoGroup.Add(strDebugInfoName, enumDebugInfoType::Point_32f_C2);

        strDebugInfoName.Format(_T("Reference Rectangle_32f Data %d"), n);
        m_DebugInfoGroup.Add(strDebugInfoName, enumDebugInfoType::Rect_32f);

        strDebugInfoName.Format(_T("Reference Ellipse Data %d"), n);
        m_DebugInfoGroup.Add(strDebugInfoName, enumDebugInfoType::EllipseEq_32f);

        strDebugInfoName.Format(_T("Target Point Data %d"), n);
        m_DebugInfoGroup.Add(strDebugInfoName, enumDebugInfoType::Point_32f_C2);

        strDebugInfoName.Format(_T("Target Rectangle_32f Data %d"), n);
        m_DebugInfoGroup.Add(strDebugInfoName, enumDebugInfoType::Rect_32f);

        strDebugInfoName.Format(_T("Target Ellipse Data %d"), n);
        m_DebugInfoGroup.Add(strDebugInfoName, enumDebugInfoType::EllipseEq_32f);
    }

    strDebugInfoName.Empty();
}

std::vector<CString> VisionInspectionGeometry::ExportRecipeToText() //kircheis_TxtRecipe
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