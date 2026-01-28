//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingNGRV.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingNGRV.h"
#include "VisionProcessingNGRVPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
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
VisionProcessingNGRV::VisionProcessingNGRV(
    LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_NGRV, _T("NGRV Setup"), visionUnit, packageSpec)
    , m_VisionParaBTM(new VisionProcessingNGRVPara(*this))
    , m_VisionParaTOP(new VisionProcessingNGRVPara(*this))
{
    UNREFERENCED_PARAMETER(moduleGuid);
    UNREFERENCED_PARAMETER(moduleName);

    m_pVisionInspDlg = NULL;

    for (long nidx = 0; nidx < MAX_2D_VISION_TYPE; nidx++)
    {
        ref_image_sizeX[nidx] = 0;
        ref_image_sizeY[nidx] = 0;
    }
}

void VisionProcessingNGRV::ResetSpecAndPara()
{
    m_VisionParaBTM->Init();
    m_VisionParaTOP->Init();
}

void VisionProcessingNGRV::ResetResult()
{
}

void VisionProcessingNGRV::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    UNREFERENCED_PARAMETER(overlayResult);
    UNREFERENCED_PARAMETER(detailSetupMode);
}

void VisionProcessingNGRV::AppendTextResult(CString& textResult)
{
    UNREFERENCED_PARAMETER(textResult);
}

VisionProcessingNGRV::~VisionProcessingNGRV(void)
{
    delete m_VisionParaBTM;
    delete m_VisionParaTOP;
    CloseDlg();
}

BOOL VisionProcessingNGRV::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    UNREFERENCED_PARAMETER(detailSetupMode);
    UNREFERENCED_PARAMETER(i_ModuleStatus);

    return 0;
}

BOOL VisionProcessingNGRV::OnInspection()
{
    return 0;
}

BOOL VisionProcessingNGRV::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (__super::LinkDataBase(bSave, db) != TRUE)
    {
        return FALSE;
    }

    m_VisionParaBTM->SetNGRV_SingleRun_RefimageSize(ref_image_sizeX[VISION_BTM_2D], ref_image_sizeY[VISION_BTM_2D]);
    m_VisionParaTOP->SetNGRV_SingleRun_RefimageSize(ref_image_sizeX[VISION_TOP_2D], ref_image_sizeY[VISION_TOP_2D]);

    if (m_VisionParaBTM->LinkDataBase(bSave, db[_T("{86E0C8C4-F991-4644-80E3-66B04876AD60}")]) != TRUE)
    {
        return FALSE;
    }

    if (m_VisionParaTOP->LinkDataBase(bSave, db[_T("{4ABE0C10-C15D-42EE-BDD2-AE72FF58C2A0}")]) != TRUE)
    {
        return FALSE;
    }

    return TRUE;
}

void VisionProcessingNGRV::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

std::vector<CString> VisionProcessingNGRV::ExportRecipeToText()
{
    return std::vector<CString>();
}

long VisionProcessingNGRV::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    ResetDebugInfo();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionProcessingNGRV(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionProcessingNGRV::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

bool VisionProcessingNGRV::CheckmakedROIinimage_ChnageStitch(const Ipvm::Rect32s& srcROI, const long& ImageSizeX,
    const long& ImageSizeY, const long& ROI_Width, const long& ROI_Height, const long& StitchCountX,
    const long& StitchCountY)
{
    if (srcROI.IsRectEmpty() == true || ImageSizeX < 0 || ImageSizeY < 0)
    {
        ::SimpleMessage(_T("ROI or image size is incorrect Please check again"));
        return false;
    }

    Ipvm::Rect32s tempROI = srcROI;

    tempROI.m_right = tempROI.m_left + ROI_Width * StitchCountX;
    tempROI.m_bottom = tempROI.m_top + ROI_Height * StitchCountY;

    if (tempROI.m_right > ImageSizeX || tempROI.m_bottom > ImageSizeY)
    {
        ::SimpleMessage(_T("ROI must be within the image area\n Please adjust and try again"));
        return false;
    }

    return true;
}

void VisionProcessingNGRV::VerifyROIusingimageSize(
    const long& imageSizeX, const long& imageSizeY, const Ipvm::Rect32s& SrcROI, Ipvm::Rect32s& dstROI)
{
    if (imageSizeX <= 0 || imageSizeY <= 0) //imageSize가 0이하라면 그냥 recipe에 있는 ROI를 사용한다
    {
        dstROI = SrcROI;
        return;
    }

    bool NeedModify(false);

    if (SrcROI.m_left < 0 || SrcROI.m_left > imageSizeX)
        NeedModify = true;
    if (SrcROI.m_top < 0 || SrcROI.m_top > imageSizeY)
        NeedModify = true;
    if (SrcROI.m_right < 0 || SrcROI.m_right > imageSizeX)
        NeedModify = true;
    if (SrcROI.m_bottom < 0 || SrcROI.m_bottom > imageSizeY)
        NeedModify = true;

    if (NeedModify == true)
    {
        Ipvm::Rect32s dsttemp(0, 0, 0, 0);
        if (SrcROI.IsRectEmpty() == true)
            dstROI = Ipvm::Rect32s(0, 0, 0, 0);

        const long width = max(SrcROI.m_left, SrcROI.m_right) - min(SrcROI.m_left, SrcROI.m_right);
        const long height = max(SrcROI.m_top, SrcROI.m_bottom) - min(SrcROI.m_top, SrcROI.m_bottom);

        //유효성 검사 ImageSize를 넘어가지 않도록
        //반쯤 걸치는 순간도 있으니, 그냥 초기화시에 0으로 고정한다
        dsttemp.m_left = 0;
        dsttemp.m_right = dsttemp.m_left + width;
        dsttemp.m_top = 0;
        dsttemp.m_bottom = dsttemp.m_top + height;

        //if (SrcROI.m_left > imageSizeX)
        //    dsttemp.m_left = min(dsttemp.m_left, SrcROI.m_left);
        //else
        //    dsttemp.m_left = max(dsttemp.m_left, SrcROI.m_left);

        //dsttemp.m_right = min(imageSizeX, dsttemp.m_left + width);

        //if (SrcROI.m_top > imageSizeY)
        //    dsttemp.m_top = min(dsttemp.m_top, SrcROI.m_top);
        //else
        //    dsttemp.m_top = max(dsttemp.m_top, SrcROI.m_top);

        //dsttemp.m_bottom = min(imageSizeY, dsttemp.m_top + height);

        dstROI = dsttemp;

        ::SimpleMessage(_T("The ROI position must be within the image size"));
    }
}

void VisionProcessingNGRV::SetNGRV_ref_image_size(
    const long& vision_type, const long& imageSizeX, const long& imageSizeY)
{
    ref_image_sizeX[vision_type] = imageSizeX;
    ref_image_sizeY[vision_type] = imageSizeY;
}

long VisionProcessingNGRV::getNGRV_ref_image_size_X(const long& vision_type)
{
    return ref_image_sizeX[vision_type];
}

long VisionProcessingNGRV::getNGRV_ref_image_size_Y(const long& vision_type)
{
    return ref_image_sizeY[vision_type];
}