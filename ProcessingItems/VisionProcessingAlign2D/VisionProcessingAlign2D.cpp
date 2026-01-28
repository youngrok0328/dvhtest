//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingAlign2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingAlign2D.h"
#include "EngineBasedOnBodySize.h"
#include "EngineBasedOnMatching.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Inspection.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Para.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionProcessingAlign2D::VisionProcessingAlign2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_ALIGN_2D, _T("Align 2D"), visionUnit, packageSpec)
    , m_VisionPara(*this)
    , m_constants(this)
{
    m_basedOnMatching = new EngineBasedOnMatching(this, m_constants);
    m_basedOnBodySize = new EngineBasedOnBodySize(this, m_constants);

    m_pVisionInspDlg = NULL;

    m_edgeAlign_para = new VisionEdgeAlign::Para(*this);
    m_edgeAlign_inspection = new VisionEdgeAlign::Inspection(*this);
}

void VisionProcessingAlign2D::ResetSpecAndPara()
{
    m_VisionPara.Init();
    m_edgeAlign_para->Init();
}

VisionProcessingAlign2D::~VisionProcessingAlign2D(void)
{
    CloseDlg();

    delete m_edgeAlign_inspection;
    delete m_edgeAlign_para;

    delete m_basedOnBodySize;
    delete m_basedOnMatching;
}

void VisionProcessingAlign2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingAlign2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    GetConstants();
    ResetResult();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new DlgVisionProcessingAlign2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(DlgVisionProcessingAlign2D::IDD, parent);

    m_visionUnit.RunInspection(m_visionUnit.GetPreviousVisionProcessing(this), false, GetCurVisionModule_Status());

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionProcessingAlign2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    if (!m_VisionPara.LinkDataBase(bSave, db[_T("{F99A3745-A427-49B5-AFC1-455B1337CDDF}")]))
        return FALSE;

    float fPackageSizeX = 0.f;
    float fPackageSizeY = 0.f;
    GetPackageSize(false, fPackageSizeX, fPackageSizeY);

    if (!m_edgeAlign_para->LinkDataBase(
            bSave, fPackageSizeX, fPackageSizeY, db[_T("{2EAF49EA-878D-4EB4-AE0A-E22272C8E9DB}")]))
        return FALSE;

    return TRUE;
}

void VisionProcessingAlign2D::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    m_edgeAlign_inspection->getOverlayResult(*m_edgeAlign_para, overlayResult, m_nOverlayMode, detailSetupMode);
}

void VisionProcessingAlign2D::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);

    if (m_constants.m_stitchCountX * m_constants.m_stitchCountY < 2)
        return;

    float fMin = 99999.999999f;
    float fSum = 0.f;
    long nDnum = 0;

    textResult.AppendFormat(
        _T("    Template match rate (It is recommended that all results be greater than 90%%.)\r\n"));

    float fMatchScore;
    float fShiftX, fShiftY;

    for (long y = 0; y < m_constants.m_stitchCountY; y++)
    {
        for (long x = 0; x < m_constants.m_stitchCountX; x++)
        {
            long stitchIndex = y * m_constants.m_stitchCountX + x;
            auto& curStitchInfo = m_result.m_stitchRois[stitchIndex];

            if (y != 0)
            {
                auto& verMatchingInfo = curStitchInfo.m_verMatchingInfo;
                for (long j = 0; j < 2; j++)
                {
                    fMatchScore = verMatchingInfo.m_fMatchScore[j];
                    fShiftX = verMatchingInfo.m_ptShift[j].m_x;
                    fShiftY = verMatchingInfo.m_ptShift[j].m_y;

                    textResult.AppendFormat(_T("     S%d-V%d : %.1f%%, Xo:%.0f (um), Yo:%.0f (um)\r\n"),
                        stitchIndex + 1, j + 1, fMatchScore, fShiftX, fShiftY);
                    fSum += fMatchScore;
                    nDnum++;
                    fMin = (float)min(fMin, fMatchScore);
                }
                //textResult.AppendFormat(_T("     S%d-V Rotate Angle : %.2f (DEG)\r\n"), stitchIndex + 1, verMatchingInfo.m_angle_diff);
            }
            if (x != 0)
            {
                auto& horMatchingInfo = curStitchInfo.m_horMatchingInfo;
                for (long j = 0; j < 2; j++)
                {
                    fMatchScore = horMatchingInfo.m_fMatchScore[j];
                    fShiftX = horMatchingInfo.m_ptShift[j].m_x;
                    fShiftY = horMatchingInfo.m_ptShift[j].m_y;
                    textResult.AppendFormat(_T("     S%d-H%d : %.2f%%, Xo:%.0f (um), Yo:%.0f (um)\r\n"),
                        stitchIndex + 1, j + 1, fMatchScore, fShiftX, fShiftY);
                    fSum += fMatchScore;
                    nDnum++;
                    fMin = (float)min(fMin, fMatchScore);
                }
                //textResult.AppendFormat(_T("     S%d-H Rotate Angle : %.2f (DEG)\r\n"), stitchIndex + 1, horMatchingInfo.m_angle_diff);
            }

            if (x != 0 || y != 0)
            {
                textResult.AppendFormat(
                    _T("     S%d Rotate Angle : %.2f (DEG)\r\n\n"), stitchIndex + 1, curStitchInfo.GetAngle());
            }
        }
    }

    textResult.AppendFormat(_T("\r\n      Min : %.2f%%\r\n"), fMin);
    textResult.AppendFormat(_T("      Avr : %.2f%%\r\n"), fSum / (float)nDnum);

    m_edgeAlign_inspection->appendTextResult(textResult);

    textResult += m_result.m_additionalCriticalLog;
}

void VisionProcessingAlign2D::ResetResult()
{
    m_result.Prepare(getInspectionAreaInfo().m_stichCountX * getInspectionAreaInfo().m_stichCountY);

    m_edgeAlign_inspection->resetResult();

    ResetDebugInfo();
}

BOOL VisionProcessingAlign2D::GetNGRVAFInfo(BOOL& o_bIsValidPlaneRefInfo,
    std::vector<Ipvm::Point32s2>& o_vecptRefPos_UM, long& o_nAFImgSizeX, long& o_nAFImgSizeY,
    std::vector<BYTE>& o_vecbyAFImage) //kircheis_NGRVAF
{
    o_bIsValidPlaneRefInfo = FALSE;
    o_vecptRefPos_UM.clear();
    o_nAFImgSizeX = 0;
    o_nAFImgSizeY = 0;
    o_vecbyAFImage.clear();

    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_2D_INSP || IsEnabled() == FALSE)
        return FALSE;

    o_bIsValidPlaneRefInfo = m_edgeAlign_para->m_bIsValidPlaneRefInfo;

    o_vecptRefPos_UM.resize(4);
    for (long idx = 0; idx < 4; idx++)
        o_vecptRefPos_UM[idx] = m_edgeAlign_para->m_vecptRefPos_UM[idx];

    o_nAFImgSizeX = m_edgeAlign_para->m_nAFImgSizeX;
    o_nAFImgSizeY = m_edgeAlign_para->m_nAFImgSizeY;

    static const long nSizeOfByte = sizeof(BYTE);
    long nImageSize = o_nAFImgSizeX * o_nAFImgSizeY * 3;
    o_vecbyAFImage.resize(nImageSize);
    memcpy(&o_vecbyAFImage[0], &m_edgeAlign_para->m_vecbyAFImage[0], nImageSize * nSizeOfByte);

    return TRUE;
}

std::vector<CString> VisionProcessingAlign2D::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    m_VisionPara.m_nStitchCountX = m_constants.m_stitchCountX;
    m_VisionPara.m_nStitchCountY = m_constants.m_stitchCountY;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara.ExportAlgoParaToText(strVisionName, m_strModuleName);
    std::vector<CString> vecstrEdgeAlignPara
        = m_edgeAlign_para->ExportAlgoParaToText(strVisionName, m_strModuleName); //만들어야 함.
    vecStrResult.insert(vecStrResult.end(), vecstrEdgeAlignPara.begin(), vecstrEdgeAlignPara.end());

    return vecStrResult;
}