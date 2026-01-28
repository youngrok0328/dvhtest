//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionMark.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionMark.h"
#include "DlgVisionInspectionMarkOperator.h"
#include "InspResult.h"
#include "VisionInspectionMarkPara.h"
#include "VisionInspectionMarkSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
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
VisionInspectionMark::VisionInspectionMark(
    LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(moduleGuid, moduleName, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionMarkPara(*this))
    , m_oriViewer(new Ipvm::Image8u)
    , m_binViewer(new Ipvm::Image8u)
    , m_overViewer(new Ipvm::Image8u)
    , m_underViewer(new Ipvm::Image8u)
    , m_blobSizeUnderViewer(new Ipvm::Image8u)
    , m_blobSizeOverViewer(new Ipvm::Image8u)
    , m_result(new InspResult)
{
    ///=============변수 초기화=====================///
    m_pVisionInspDlg = NULL;

    for (long nMarkID = 0; nMarkID < NUM_OF_MARKMULTI; nMarkID++)
    {
        m_VisionSpec[nMarkID] = new VisionInspectionMarkSpec;

        m_VisionTempSpec = m_VisionSpec[nMarkID];
    }

    m_pBlob = NULL;

    m_psBlobInfo = nullptr;
    m_blobAttributes = nullptr;

    m_bodyAlignResult = nullptr;

    m_nTotalResult = PASS;

    // Save 필요없는 변수들...
    m_rtAllChar = Ipvm::Rect32s(0, 0, 0, 0);

    m_bTeach = FALSE;

    for (long i = 0; i < NUM_OF_LOCATOR; i++)
    {
        m_pptLocater[i] = Ipvm::Point32s2(0, 0);
        m_prtLocaterSearchROI[i] = Ipvm::Rect32s(0, 0, 0, 0);
    }
    ///===========================================///

    m_bMarkSpliteOptionTeach = FALSE;

    m_bAutoTeachMode = FALSE; //kircheis_AutoTeach

    m_defaultFixedInspectionSpecs.emplace_back(_T("{F7CD5848-52AD-46C3-B08F-28FA13E0598E}"),
        g_szMarkInspectionName[MARK_INSPECTION_MARK_COUNT], _T("mCOU"), _T("M"), _T("Num"), HostReportCategory::MARK,
        II_RESULT_TYPE::COUNT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{EB3BEBFF-80E1-4593-A4D1-86FF84ED70AD}"),
        g_szMarkInspectionName[MARK_INSPECTION_MARK_POS], _T("mPOS"), _T("M"), _T("um"), HostReportCategory::MARK,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{B1C1846C-6B20-4217-AAF0-24BA55315203}"),
        g_szMarkInspectionName[MARK_INSPECTION_MARK_ANGLE], _T("mANG"), _T("M"), _T("Deg"), HostReportCategory::MARK,
        II_RESULT_TYPE::DEGREE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{4162BF4C-D37B-4ECA-8AB3-ED08B61D5D15}"),
        g_szMarkInspectionName[MARK_INSPECTION_MATCH_RATE], _T("mMAT"), _T("M"), _T("%"), HostReportCategory::MARK,
        II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{3DF0AFE8-DCDC-4616-AAF6-E60CC53CC274}"),
        g_szMarkInspectionName[MARK_INSPECTION_CONTRAST], _T("mCON"), _T("M"), _T("GV"), HostReportCategory::MARK,
        II_RESULT_TYPE::GRAY_VALUE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{9D622305-21AF-4357-9C1C-778E414799EF}"),
        g_szMarkInspectionName[MARK_INSPECTION_OVER_PRINT], _T("mOVP"), _T("M"), _T("%"), HostReportCategory::MARK,
        II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{48BA66D5-CA78-4E11-9FA2-C39D8B489ABE}"),
        g_szMarkInspectionName[MARK_INSPECTION_UNDER_PRINT], _T("mUNP"), _T("M"), _T("%"), HostReportCategory::MARK,
        II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{8BB223F1-D900-4978-ACA3-0B7754E74749}"),
        g_szMarkInspectionName[MARK_INSPECTION_BLOB_SIZE], _T("mBLO"), _T("M"), _T("px"), HostReportCategory::MARK,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    __super::ResetSpec();

    m_DebugInfoGroup.Add(_T("Simple Mark Blob Size(Pxl)"), enumDebugInfoType::Long);
    m_DebugInfoGroup.Add(_T("Simple Mark Contrast(GV)"), enumDebugInfoType::Float);
    m_DebugInfoGroup.Add(_T("Mark Locater Search ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Mark Find Locater Point"), enumDebugInfoType::Point);
    m_DebugInfoGroup.Add(_T("Mark ROI Spec Rect"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Mark Search ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Mark Blob ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Mark Char Position"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Mark Image Spec"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Mark User Ignore ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Mark All Char"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Mark Char Num"), enumDebugInfoType::Long);
    m_DebugInfoGroup.Add(_T("Inspection Blob ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Find Mark Image"), enumDebugInfoType::Image_8u_C1);
}

void VisionInspectionMark::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_VisionPara->Init();
}

VisionInspectionMark::~VisionInspectionMark(void)
{
    CloseDlg();

    for (long nMultiID = 0; nMultiID < NUM_OF_MARKMULTI; nMultiID++)
    {
        if (m_VisionSpec[nMultiID]->m_plTeachCharArea != NULL)
            delete[] m_VisionSpec[nMultiID]->m_plTeachCharArea;
        if (m_VisionSpec[nMultiID]->m_plTeachCharROI != NULL)
            delete[] m_VisionSpec[nMultiID]->m_plTeachCharROI;
    }

    delete m_oriViewer;
    delete m_binViewer;
    delete m_overViewer;
    delete m_underViewer;
    delete m_blobSizeUnderViewer;
    delete m_blobSizeOverViewer;

    for (long i = 0; i < NUM_OF_MARKMULTI; i++)
    {
        delete m_VisionSpec[i];
    }

    delete m_result;
    delete m_VisionPara;
}

void VisionInspectionMark::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

void VisionInspectionMark::SetPoint(Ipvm::Point32s2 ptDispoint)
{
    m_pVisionInspDlg->m_pVisionInspOpParaDlg->SetPoint(ptDispoint);
}

long VisionInspectionMark::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    //m_visionUnit.RunInspection(m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_EDGE_ALIGN));
    m_visionUnit.RunInspection(m_visionUnit.GetPreviousVisionProcessing(this), false, GetCurVisionModule_Status());

    m_bodyAlignResult = GetAlignInfo();
    if (m_bodyAlignResult == nullptr)
    {
        CString strTemp;
        strTemp.Format(_T("Please check bodyalign."));
        SimpleMessage(strTemp, MB_OK);
        return 1;
    }

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionMark(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionMark::IDD, parent);

    m_pVisionInspDlg->ShowImage(m_VisionPara->sMarkAlgoParam.nThresholdMode != ENUM_THRESHOLD_MANUAL, 0);

    ModifyMemory();
    /// Overlay에 사용할 이미지 buffer 할당.
    //const long imageSizeX = getImageLotInsp().GetImageSizeX();
    //const long imageSizeY = getImageLotInsp().GetImageSizeY();

    //m_oriViewer->Create(imageSizeX, imageSizeY);
    //m_binViewer->Create(imageSizeX, imageSizeY);
    //m_overViewer->Create(imageSizeX, imageSizeY);
    //m_underViewer->Create(imageSizeX, imageSizeY);
    //m_blobSizeUnderViewer->Create(imageSizeX, imageSizeY);
    //m_blobSizeOverViewer->Create(imageSizeX, imageSizeY);

    if (m_visionUnit.RunInspection(m_visionUnit.GetPreviousVisionProcessing(this), false, GetCurVisionModule_Status()))
    {
        if (SystemConfig::GetInstance().m_nCurrentAccessMode == _OPERATOR)
        {
            m_pVisionInspDlg->m_pVisionInspOpParaDlg->SetCurSpecValue(0);
        }

        m_pVisionInspDlg->ShowImage(m_VisionPara->sMarkAlgoParam.nThresholdMode != ENUM_THRESHOLD_MANUAL, 0);
    }

    //m_visionUnit.GetInspectionOverlayResult().Apply(m_pVisionInspDlg->m_imageLotView->GetCoreView());
    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionInspectionMark::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    CString strTemp;

    if (!bSave)
    {
    }

    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Teach된 이미지를 Spec Image로 저장한다.
    strTemp = m_strModuleName;
    strTemp.Delete(0, 5);
    //long nCurMarkNum = _ttoi(strTemp); // "Mark_" 지우고 저장

    //{{//kirchies_MarkTeachSplit
    BOOL bUseMarkTeachImageSplitSave = SystemConfig::GetInstance().m_bUseMarkTeachImageSplitSave;
    CString strROI;
    Ipvm::Rect32s rtMarkTeachROI;
    int nTeachRoiWidth, nTeachRoiHeight;
    Ipvm::Rect32s rtTargetTeachROI;
    //}}

    // Link Parameter
    // Detail Setup의 파라메타
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{B138DE7D-3525-4CAF-8C6A-75E1B12CE0F2}")]))
        return FALSE;

    static LPCTSTR g_markMultiSpecGuid[] = {
        _T("{644C094B-8CA5-46AD-AE17-A5CAC804B044}"),
        _T("{2D9D422E-14B6-4D6F-AEC0-66B4DB10CF63}"),
        _T("{5F8C01AE-CE6A-4771-80B4-0700DC82A100}"),
    };

    long imageSizeX = getReusableMemory().GetInspImageSizeX();
    long imageSizeY = getReusableMemory().GetInspImageSizeY();

    db[_T("{76212625-2A7B-43BF-BD45-8D7A2EADE706}")].Link(bSave, imageSizeX);
    db[_T("{8CC5B048-48A5-4E91-9702-89DEFAC155F1}")].Link(bSave, imageSizeY);

    if (m_VisionPara->m_nMarkInspMode == Mark_InspMode_Normal)
    {
        for (long nMultiID = 0; nMultiID < NUM_OF_MARKMULTI; nMultiID++)
        {
            auto* multiSpec = m_VisionSpec[nMultiID];

            auto& multiSpecDB = db[g_markMultiSpecGuid[nMultiID]];

            if (!multiSpec->LinkDataBase(bSave, multiSpecDB))
            {
                return FALSE;
            }

            multiSpec->m_specImage.Create(imageSizeX, imageSizeY);

            //Mark 영상은 Mark가 n개로 늘어 나고 Multi Teaching도 하기 때문에 다음과 같이 할당 해야 함
            if (!bUseMarkTeachImageSplitSave) //kirchies_MarkTeachSplit
            {
                if (!multiSpecDB.LinkArray(bSave, multiSpec->m_specImage.GetMem(),
                        multiSpec->m_specImage.GetWidthBytes() * multiSpec->m_specImage.GetSizeY()))
                {
                    //DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, _T("[%s]\nThe size of the image stored in the file differs from the size of the machine setting.\nTeaching of this item is required."), (LPCTSTR)m_strModuleName);
                    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(multiSpec->m_specImage), 0, multiSpec->m_specImage);
                }
            }
            else //kirchies_MarkTeachSplit
            {
                Ipvm::Point32r2 imageCenter(imageSizeX * 0.5f, imageSizeY * 0.5f);
                rtMarkTeachROI = getScale().convert_BCUToPixel(multiSpec->m_rtMarkTeachROI_BCU, imageCenter);

                strROI.Format(_T("%s_TeachROI_ForPartial"), g_markMultiSpecGuid[nMultiID]);
                if (!db[strROI].Link(bSave, rtMarkTeachROI))
                    rtMarkTeachROI = Ipvm::Rect32s(0, 0, 0, 0);

                nTeachRoiWidth = rtMarkTeachROI.Width();
                nTeachRoiHeight = rtMarkTeachROI.Height();
                rtTargetTeachROI = Ipvm::Rect32s(0, 0, nTeachRoiWidth, nTeachRoiHeight);
                //long nTargetRoiWidth = rtTargetTeachROI.Width();
                //long nTargetRoiHeight = rtTargetTeachROI.Height();
                long nSizeOfROI = nTeachRoiWidth * nTeachRoiHeight;
                if (nSizeOfROI <= 0 || nSizeOfROI >= imageSizeX * imageSizeY)
                {
                    if (!multiSpecDB.LinkArray(bSave, multiSpec->m_specImage.GetMem(),
                            multiSpec->m_specImage.GetWidthBytes() * multiSpec->m_specImage.GetSizeY()))
                    {
                        //DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, _T("[%s]\nThe size of the image stored in the file differs from the size of the machine setting.\nTeaching of this item is required."), (LPCTSTR)m_strModuleName);

                        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(multiSpec->m_specImage), 0, multiSpec->m_specImage);
                    }
                }
                else
                {
                    Ipvm::Image8u markImageSpecBuf;
                    markImageSpecBuf.Create(nTeachRoiWidth, nTeachRoiHeight);
                    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(markImageSpecBuf), 0, markImageSpecBuf);

                    CString strMarkSpecPartial(g_markMultiSpecGuid[nMultiID]);
                    strMarkSpecPartial += _T("_Partial");

                    if (bSave)
                    {
                        Ipvm::ImageProcessing::Copy(Ipvm::Image8u(multiSpec->m_specImage, rtMarkTeachROI),
                            Ipvm::Rect32s(markImageSpecBuf), markImageSpecBuf);

                        if (!db[strMarkSpecPartial].LinkArray(bSave, markImageSpecBuf.GetMem(),
                                markImageSpecBuf.GetWidthBytes() * markImageSpecBuf.GetSizeY()))
                        {
                            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(markImageSpecBuf), 0, markImageSpecBuf);
                        }
                    }
                    else
                    {
                        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(multiSpec->m_specImage), 0, multiSpec->m_specImage);

                        if (!db[strMarkSpecPartial].LinkArray(bSave, markImageSpecBuf.GetMem(),
                                markImageSpecBuf.GetWidthBytes() * markImageSpecBuf.GetSizeY()))
                        {
                            Ipvm::ImageProcessing::Fill(
                                Ipvm::Rect32s(multiSpec->m_specImage), 0, multiSpec->m_specImage);
                        }
                        else
                        {
                            Ipvm::Rect32s rtAbsROI = rtMarkTeachROI;

                            if ((Ipvm::Rect32s(multiSpec->m_specImage) & rtMarkTeachROI) != rtMarkTeachROI)
                            {
                                DevelopmentLog::AddLog(DevelopmentLog::Type::Warning,
                                    _T("[%s] Mark position is wrong. Teach again."), (LPCTSTR)m_strModuleName);
                                Ipvm::ImageProcessing::Fill(
                                    Ipvm::Rect32s(multiSpec->m_specImage), 0, multiSpec->m_specImage);
                            }
                            else
                            {
                                Ipvm::Image8u targetImage(multiSpec->m_specImage, rtAbsROI);
                                Ipvm::ImageProcessing::Copy(
                                    markImageSpecBuf, Ipvm::Rect32s(markImageSpecBuf), targetImage);
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (long nMultiID = 0; nMultiID < NUM_OF_MARKMULTI; nMultiID++)
        {
            auto* multiSpec = m_VisionSpec[nMultiID];

            auto& multiSpecDB = db[g_markMultiSpecGuid[nMultiID]];

            if (!multiSpec->LinkDataBase(bSave, multiSpecDB))
            {
                return FALSE;
            }

            multiSpec->m_specImage.Create(imageSizeX, imageSizeY);
            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(multiSpec->m_specImage), 0, multiSpec->m_specImage);
        }
    }
    return TRUE;
}

void VisionInspectionMark::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    Ipvm::Rect32s imageRoi(0, 0, getReusableMemory().GetInspImageSizeX(), getReusableMemory().GetInspImageSizeY());

    if (m_nOverlayMode == OverlayType_ShowAll)
    {
        if (m_bodyAlignResult)
        {
            if (m_nTotalResult == PASS && m_visionUnit.m_currentProcessingModule)
            {
                auto bodyLT = m_bodyAlignResult->getBodyRect().TopLeft();
                Ipvm::Point32s2 ptLeftTop = Ipvm::Point32s2((long)(bodyLT.m_x + 0.5f), (long)(bodyLT.m_y - 4.5f));
                overlayResult->AddText(ptLeftTop, _T("Mark Inspection Pass"), RGB(0, 255, 0), 10);
            }
        }

        // 찾은 Locator 표시...
        long nLocater(0);
        CString strLocater;
        long nCharNum = (long)m_result->m_vecrtCharPositionforImage.size();

        for (long i = 0; i < nCharNum; i++)
        {
            if ((i == m_VisionTempSpec->m_pnLocID[0]) || (i == m_VisionTempSpec->m_pnLocID[1]))
            {
                // locater character ROI for test
                nLocater++;
                strLocater.Format(_T("%d"), nLocater);

                Ipvm::Rect32s rtROI(m_result->m_vecrtCharPositionforImage[i]);
                rtROI.InflateRect(10, 10, 10, 10);

                if ((rtROI & imageRoi) != rtROI)
                {
                    continue;
                }

                Ipvm::Point32s2 pt(0, 0);
                pt.m_x = rtROI.m_left + ((rtROI.m_right - rtROI.m_left) / 2);
                pt.m_y = rtROI.m_bottom + 1;

                overlayResult->AddRectangle(rtROI, RGB(255, 255, 0));
                overlayResult->AddText(pt, strLocater, RGB(255, 255, 0), 10);
            }

            // 찾은 Mark ROI...
            Ipvm::Rect32s rtROI(m_result->m_vecrtCharPositionforImage[i]);

            if ((rtROI & imageRoi) == rtROI)
            {
                overlayResult->AddRectangle(rtROI, RGB(0, 255, 0));
            }
        }
    }

    if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
    {
        if (m_bodyAlignResult)
        {
            auto bodyLT = m_bodyAlignResult->getBodyRect().TopLeft();

            //k
            if (m_nTotalResult == REJECT && m_visionUnit.m_currentProcessingModule)
            {
                Ipvm::Point32s2 ptLeftTop = Ipvm::Point32s2((long)(bodyLT.m_x + 0.5f), (long)(bodyLT.m_y - 4.5f));
                overlayResult->AddText(ptLeftTop, _T("Mark Inspection Reject"), RGB(255, 0, 0), 10);
            }
            if (m_nTotalResult == MARGINAL && m_visionUnit.m_currentProcessingModule)
            {
                Ipvm::Point32s2 ptLeftTop = Ipvm::Point32s2((long)(bodyLT.m_x + 0.5f), (long)(bodyLT.m_y - 4.5f));
                overlayResult->AddText(ptLeftTop, _T("Mark Inspection Marginal"), RGB(243, 157, 58), 10);
            }
        }

        if (m_VisionTempSpec->m_plTeachCharROI == NULL
            && m_VisionPara->m_nMarkInspMode == Mark_InspMode_Normal) //Mark Teaching이 안된 Job 일때 죽는 문제 처리
        {
            if (detailSetupMode)
                ::SimpleMessage(_T("Mark teach not yet."));

            return;
        }
        /// Mark Pos 검사.
        VisionInspectionResult* pMarkPosResult
            = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_MARK_POS]);

        if (pMarkPosResult == nullptr)
        {
            return;
        }

        if (pMarkPosResult->m_totalResult == MARGINAL)
        {
            Ipvm::Rect32s rtROI;
            for (long i = 0; i < NUM_OF_LOCATOR; i++)
            {
                rtROI.m_left = m_pptLocater[i].m_x;
                rtROI.m_top = m_pptLocater[i].m_y;
                rtROI.m_right
                    = rtROI.m_left + m_VisionTempSpec->m_plTeachCharROI[m_VisionTempSpec->m_pnLocID[i]].Width();
                rtROI.m_bottom
                    = rtROI.m_top + m_VisionTempSpec->m_plTeachCharROI[m_VisionTempSpec->m_pnLocID[i]].Height();

                overlayResult->AddRectangle(rtROI, RGB(255, 0, 0));
            }

            overlayResult->AddLine(Ipvm::Conversion::ToPoint32r2(m_pptLocater[0]),
                Ipvm::Conversion::ToPoint32r2(m_pptLocater[1]), RGB(255, 0, 0));
        }
        else if (pMarkPosResult->m_totalResult > PASS)
        {
            Ipvm::Rect32s rtROI;
            for (long i = 0; i < NUM_OF_LOCATOR; i++)
            {
                rtROI.m_left = m_pptLocater[i].m_x;
                rtROI.m_top = m_pptLocater[i].m_y;
                rtROI.m_right
                    = rtROI.m_left + m_VisionTempSpec->m_plTeachCharROI[m_VisionTempSpec->m_pnLocID[i]].Width();
                rtROI.m_bottom
                    = rtROI.m_top + m_VisionTempSpec->m_plTeachCharROI[m_VisionTempSpec->m_pnLocID[i]].Height();

                overlayResult->AddRectangle(rtROI, RGB(243, 157, 58));
            }

            overlayResult->AddLine(Ipvm::Conversion::ToPoint32r2(m_pptLocater[0]),
                Ipvm::Conversion::ToPoint32r2(m_pptLocater[1]), RGB(243, 157, 58));
        }
    }
}
void VisionInspectionMark::AppendTextResult(CString& textResult)
{
    LPCTSTR szResult[] = {_T(" "), _T("PASS"), _T("MARGINAL"), _T("REJECT"), _T("INVALID")};

    // Display Text
    int nMeaMarkNum = m_VisionTempSpec->m_nCharNum;
    long nTotalResult(NOT_MEASURED);

    // Total Result...
    for (long nInsp = MARK_INSPECTION_START; nInsp < MARK_INSPECTION_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[nInsp]);
            if (pResult != nullptr && pResult->m_totalResult > nTotalResult)
            {
                nTotalResult = pResult->m_totalResult;
            }
        }
    }

    textResult.AppendFormat(_T("\n< %s Result - %s >\r\n"), (LPCTSTR)m_strModuleName, szResult[nTotalResult]);
    textResult.AppendFormat(_T("Inspection Time : %.2f\r\n"), m_fCalcTime);
    textResult.AppendFormat(_T("Mark Character Number : %d\r\n"), nMeaMarkNum);

    if (nTotalResult == MARGINAL || nTotalResult == INVALID || nTotalResult == NOT_MEASURED)
    {
        return;
    }

    // Full Data Display

    std::wstringstream stream;
    stream << std::setiosflags(std::ios::fixed) << std::setprecision(2) << std::setiosflags(std::ios::right);

    for (long nInspID = MARK_INSPECTION_START; nInspID < MARK_INSPECTION_END; nInspID++)
    {
        if (m_fixedInspectionSpecs[nInspID].m_use)
        {
            VisionInspectionResult* pMarkResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[nInspID]);

            float fWorst(-1.f);
            fWorst = pMarkResult->getObjectWorstErrorValue();
            //		for(long i = 0 ; i < nSize ; i++)//가장 큰게 아니라 워스트를 써야하잖
            //		{
            //			if(fabs(fWorst) <= fabs(pMarkResult->vecfEachValue[i]))
            //			{
            //				fWorst = pMarkResult->vecfEachValue[i];
            //			}
            //		}
            textResult.AppendFormat(_T("%s : %.2f [%s]\r\n"), (LPCTSTR)m_fixedInspectionSpecs[nInspID].m_specName,
                fWorst, szResult[pMarkResult->m_totalResult]);
        }
    }

    AppendDetailTextResult(textResult);
}

void VisionInspectionMark::InitMemory(BOOL bJobChange, long& nTeachSlot)
{
    ResetResult();

    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    CString str;
    if (bJobChange)
    {
        for (long nMultiID = 0; nMultiID < NUM_OF_MARKMULTI; nMultiID++)
        {
            m_VisionSpec[nTeachSlot]->m_specImage.Create(imageSizeX, imageSizeY);
        }
    }
    else
    {
        if (NUM_OF_MARKMULTI <= nTeachSlot || 0 > nTeachSlot)
            nTeachSlot = 0;

        m_VisionSpec[nTeachSlot]->m_specImage.Create(imageSizeX, imageSizeY);
    }

    /// Overlay에 사용할 이미지 buffer 할당.
    //m_oriViewer->Create(imageSizeX, imageSizeY);
    //m_binViewer->Create(imageSizeX, imageSizeY);
    //m_overViewer->Create(imageSizeX, imageSizeY);
    //m_underViewer->Create(imageSizeX, imageSizeY);
    //m_blobSizeUnderViewer->Create(imageSizeX, imageSizeY);
    //m_blobSizeOverViewer->Create(imageSizeX, imageSizeY);

    str.Empty();
}

void VisionInspectionMark::ResetResult()
{
    __super::ResetResult();

    if (!m_VisionPara->m_bLocatorTargetROI)
    {
        for (long i = 0; i < NUM_OF_LOCATOR; i++)
        {
            m_pptLocater[i] = Ipvm::Point32s2(0, 0);
            m_prtLocaterSearchROI[i] = Ipvm::Rect32s(0, 0, 0, 0);
        }
    }

    m_rtAllChar = Ipvm::Rect32s(0, 0, 0, 0);

    // BlobInfo Buffer...
    m_psBlobInfo = getReusableMemory().GetBlobInfo();
    m_blobAttributes = getReusableMemory().GetBlobAttribute();

    // Get Algorithm...
    m_pBlob = getReusableMemory().GetBlob();
    m_fCalcTime = 0.f;

    m_bTeach = FALSE;

    m_result->InitializeMarkInfoforPixel(getScale(), *m_VisionPara);

    ModifyMemory();
}

BOOL VisionInspectionMark::IsNeedMarkTeach()
{
    if (m_VisionPara->m_nMarkInspMode == MarkInspectionMode_Simple)
        return false;

    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (spec.m_use)
            return true;
    }

    return false;
}

void VisionInspectionMark::SetSwitchMarkParameterView(long i_nTabCurAccessMode)
{
    if (i_nTabCurAccessMode < 0)
        return;

    if (m_pVisionInspDlg != nullptr)
        m_pVisionInspDlg->SwitchParameterDialog(i_nTabCurAccessMode);
}

void VisionInspectionMark::ModifyMemory()
{
    //어차피 똑같은 Size로 할당하기 때문에 Origin 하나만 보자
    long CurImageSizeX = m_oriViewer->GetSizeX();
    long CurImageSizeY = m_oriViewer->GetSizeY();

    long imageSizeX = getImageLotInsp().GetImageSizeX();
    long imageSizeY = getImageLotInsp().GetImageSizeY();

    if (CurImageSizeX != imageSizeX || CurImageSizeY != imageSizeY) //같지 않을때만 할당
    {
        m_oriViewer->Create(imageSizeX, imageSizeY);
        m_binViewer->Create(imageSizeX, imageSizeY);
        m_overViewer->Create(imageSizeX, imageSizeY);
        m_underViewer->Create(imageSizeX, imageSizeY);
        m_blobSizeUnderViewer->Create(imageSizeX, imageSizeY);
        m_blobSizeOverViewer->Create(imageSizeX, imageSizeY);
    }
    else
        return;
}

std::vector<CString> VisionInspectionMark::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    //VisionInspectionMarkSpec은 m_rtMarkTeachROI_BCU, m_nMarkThreshold, m_prtLocaterTeachingBox를 제외한 나머지는 모두 Teach로 계산한 Data이다.
    //그런데다 Para는 Spec을 모른다. 그래서 저어기 아래에 별도의 저장이 가능하도록 별도의 함수를 Spec에 만들어 호출하도록 했는데
    //문제는 Parameter UI에 Multi Spec 중 첫번째의 Threhsold Value가 들어가네...
    //그리고 Ignore ROI와 Merge ROI는 Para소속이고....
    //어쩔수 없이 Threshold Value만 별도로 아래의 함수에 전달해 준다.
    //그리고 ROI는 Para와 Spec에 나뉘어 있으므로, 저장은 여기에서 별도로 한다(혓바닥이 길쥬? 얘만 구조가 달라서 설명하는거임)
    vecStrResult
        = m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName, m_VisionSpec[0]->m_nMarkThreshold);

    //ROI //여기에서 저장하는건 MarkSpec 클래스의 N개의 객체 중 엔지니어 모드에서 관리하는 [0] 의 값만 쓴다.
    //실제 알고리즘에서 사용하는 MarkSpec의 모든 객체의 ROI는 아래의 ExportAlgoMarkSpecToText()함수에서 저장한다.
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)m_strModuleName);
    std::vector<CString> vecStrROI = ExportROIRecipeToText(strHeader, _T("Algorithm Parameters"));
    vecStrResult.insert(vecStrResult.end(), vecStrROI.begin(), vecStrROI.end());

    //개별 Parameter용 변수
    CString strGroup;
    std::vector<CString> vecStrMarkSpec(0);
    for (long nMark = 0; nMark < NUM_OF_MARKMULTI; nMark++)
    {
        vecStrMarkSpec.clear();
        if (nMark == 0)
            strGroup.Format(_T("Mark Spec %d"), nMark);
        else
            strGroup.Format(_T("Mark Multi Teach Spec %d"), nMark);

        vecStrMarkSpec
            = m_VisionSpec[nMark]->ExportAlgoMarkSpecToText(strHeader, _T("Mark Algorithm Parameter"), strGroup);
        vecStrResult.insert(vecStrResult.end(), vecStrMarkSpec.begin(), vecStrMarkSpec.end());
    }

    //Inspection Spec
    std::vector<CString> vecstrInspectionSpec = ExportInspectionSpecToText(strVisionName);

    vecStrResult.insert(vecStrResult.end(), vecstrInspectionSpec.begin(), vecstrInspectionSpec.end());

    return vecStrResult;
}

std::vector<CString> VisionInspectionMark::ExportROIRecipeToText(
    const CString strHeader, const CString strCategory) //kircheis_TxtRecipe
{
    CString strGroup;
    strGroup.Format(_T("ROI"));
    std::vector<CString> vecstrROI(0);
    CString strROIName;

    //Teach ROI
    std::vector<CString> vecStrResult = vecstrROI = CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Mark Teach ROI"), m_VisionSpec[0]->m_rtMarkTeachROI_BCU);

    //Merge ROI
    long nROI_Num = (long)m_VisionPara->m_teach_merge_infos.size();
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Merge ROI Num"), _T(""), nROI_Num));
    for (long nROI = 0; nROI < nROI_Num; nROI++)
    {
        strROIName.Format(_T("Mark Merge_%d"), nROI);
        vecstrROI = CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strROIName, m_VisionPara->m_teach_merge_infos[nROI].m_roi);
        vecStrResult.insert(vecStrResult.end(), vecstrROI.begin(), vecstrROI.end());
        vecstrROI.clear();
    }

    //Ignore ROI
    nROI_Num = (long)m_VisionPara->m_vecrtUserIgnore.size();
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Ignore ROI Num"), _T(""), nROI_Num));
    for (long nROI = 0; nROI < nROI_Num; nROI++)
    {
        strROIName.Format(_T("Mark Ignore_%d"), nROI);
        vecstrROI = CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strROIName, m_VisionPara->m_vecrtUserIgnore[nROI]);
        vecStrResult.insert(vecStrResult.end(), vecstrROI.begin(), vecstrROI.end());
        vecstrROI.clear();
    }

    //Locator ROI 2개
    vecstrROI.clear();
    vecstrROI = CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Locator Target 1"), m_VisionSpec[0]->m_rtLocatorTargetROI[0]);
    vecStrResult.insert(vecStrResult.end(), vecstrROI.begin(), vecstrROI.end());

    vecstrROI.clear();
    vecstrROI = CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Locator Target 2"), m_VisionSpec[0]->m_rtLocatorTargetROI[1]);
    vecStrResult.insert(vecStrResult.end(), vecstrROI.begin(), vecstrROI.end());

    return vecStrResult;
}