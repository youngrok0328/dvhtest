//CPP_0_________________________________ Precompiled header
#include "StdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspection2DMatrix.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspection2DMatrix.h"
#include "VisionInspection2DMatrixPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h" //kircheis_MED5_13
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedComponent/Persistence/IniHelper.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h" //kircheis_NGRV
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>
#include <Ipvm/Gadget/Miscellaneous.h>
#include <afxmt.h>

// 포함 순서를 지켜야 해서 분리한다.
#include <Easy_MainHeader.h>
//
#include <EasyMatrixCode_MainHeader.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
bool sort_toptobottom(Ipvm::Rect32s _F, Ipvm::Rect32s _S)
{
    return _F.m_top < _S.m_top;
};
bool sort_lefttoright(Ipvm::Rect32s _F, Ipvm::Rect32s _S)
{
    return _F.m_left < _S.m_left;
};

BOOL VisionInspection2DMatrix::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

BOOL VisionInspection2DMatrix::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    Ipvm::TimeCheck time_2DMatrix;

    BOOL bResult = TRUE;

    //{{//kircheis_MED5_13
    m_bIsTurnOn2DMatrix = FALSE; //초기화
    for (long nIdx = 0; nIdx < TYPE_2DID_END;
        nIdx++) //2DID 검사를 시작하기 전에 일단 문자열 버퍼와 Rect를 지우고 시작한다
    {
        m_vecstr2DID[nIdx].Empty();
        m_vecrtROI[nIdx].SetRect(0, 0, 0, 0);
    }
    //}}

    long nImageID(0);
    std::vector<Ipvm::Image8u> vecImage(0);
    std::vector<s2DID_Using_Frame_info> vecsUsingFrameInfo; //mc_ATM Hold Test
    vecsUsingFrameInfo.clear();

    if (GetImageFrameCount() == 0)
    {
        return FALSE;
    }
    else
    {
        long nImageNum = GetImageFrameCount();
        long nImageMaxNum = (long)getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].size();

        for (long nID = 0; nID < nImageNum; nID++)
        {
            nImageID = GetImageFrameIndex(nID);
            nImageID = (long)max(0, nImageID);
            nImageID = (long)min(nImageMaxNum - 1, nImageID);

            if (m_bUseBypassMode) //kircheis_NGRV Bypass
            {
                if (getImageLot().GetImageFrame(nImageID, GetCurVisionModule_Status()).GetMem() != nullptr)
                    vecImage.push_back(getImageLot().GetImageFrame(nImageID, GetCurVisionModule_Status()));
            }
            else
            {
                if (getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][nImageID].GetMem() != nullptr)
                {
                    vecImage.push_back(getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][nImageID]);
                    s2DID_Using_Frame_info sCur2DID_Using_Frame_info;
                    sCur2DID_Using_Frame_info.SetFrameInfo(
                        nImageID, getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][nImageID]);
                    vecsUsingFrameInfo.push_back(sCur2DID_Using_Frame_info);
                }
            }
        }
    }

    if (vecImage.size() == 0)
        return FALSE;

    Ipvm::Rect32r frtBody = GetBodyRect();
    if ((frtBody.Width() <= 0.f || frtBody.Height() <= 0.f) && m_bUseBypassMode == false) //kircheis_NGRV Bypass
        return FALSE;
    else if (m_bUseBypassMode)
    {
        frtBody = GetPaneRect();
    }

    // Init...
    ResetResult();
    if (m_pVisionInspDlg != NULL && m_pVisionInspDlg->IsWindowVisible())
    {
        // Teaching
        if (detailSetupMode)
        {
            m_pVisionInspDlg->GetROI();
        }

        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    bResult &= DoInsp2DMatrix(detailSetupMode, vecImage, frtBody);
    bResult &= DoInspMatchString(detailSetupMode); //kircheis_MED5_13

    Save_2DID_Cropping_image_Debug(SystemConfig::GetInstance().Get2DIDCroppingimageSaveOption(), vecsUsingFrameInfo,
        m_s2DID_Cropping_image_Save_Info);

    m_fCalcTime = static_cast<float>(time_2DMatrix.Elapsed_ms());

    return bResult;
}

Ipvm::Rect32r VisionInspection2DMatrix::GetBodyRect()
{
    Ipvm::Rect32r frtBodyRect = Ipvm::Rect32r(0.f, 0.f, 0.f, 0.f);

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
        frtBodyRect = m_sEdgeAlignResult->getBodyRect();
    }

    return frtBodyRect;
}

Ipvm::Rect32r VisionInspection2DMatrix::GetPaneRect() //kircheis_NGRV Bypass
{
    long nSizeX = getImageLot().GetImageSizeX();
    long nSizeY = getImageLot().GetImageSizeY();

    Ipvm::Point32r2 ptImageCenter(getImageLot().GetImageSizeX() * 0.5f, getImageLot().GetImageSizeY() * 0.5f);

    long paneID = GetCurrentPaneID();
    auto& scale = getImageLot().GetScale();

    Ipvm::Rect32s roi(0, 0, 0, 0);
    const auto& inspectionAreaInfo = getInspectionAreaInfo();

    for (auto& fovIndex : inspectionAreaInfo.m_fovList)
    {
        if (paneID >= 0 && paneID < (long)inspectionAreaInfo.m_unitIndexList.size())
        {
            long unitID = inspectionAreaInfo.m_unitIndexList[paneID];
            auto pocketRegion
                = scale.convert_mmToPixel(inspectionAreaInfo.m_parent->GetUnitPocketRegionInFOV(fovIndex, unitID))
                + Ipvm::Conversion::ToPoint32s2(ptImageCenter);
            roi |= pocketRegion;
        }
    }

    roi.m_left = (long)max(0, min(nSizeX, roi.m_left));
    roi.m_top = (long)max(0, min(nSizeY, roi.m_top));
    roi.m_right = (long)max(0, min(nSizeX, roi.m_right));
    roi.m_bottom = (long)max(0, min(nSizeY, roi.m_bottom));

    Ipvm::Rect32r fROI = Ipvm::Rect32r((float)roi.m_left, (float)roi.m_top, (float)roi.m_right, (float)roi.m_bottom);

    return fROI;
}

BOOL VisionInspection2DMatrix::Is2DMarixReading() //kircheis_NGRV Bypass
{
    if (IsEnabled() == false)
        return false;

    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_sz2DMatrixInspName[MATRIX_INSPECTION_2D_MATRIX]);
    if (pResult == nullptr)
        return false;
    pResult->Clear();

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == nullptr || !pSpec->m_use)
        return false;

    return pSpec->m_use;
}

void VisionInspection2DMatrix::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    if ((frtBodyRect.Width() <= 0.f || frtBodyRect.Height() <= 0.f) && m_bUseBypassMode == false) //kircheis_NGRV Bypass
        return;
    else if (m_bUseBypassMode)
    {
        frtBodyRect = GetPaneRect();
    }
    AbsoluteRectToImageRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspection2DMatrix::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left += (long)(fptCenter.m_x + .5f);
    rtROI.m_top += (long)(fptCenter.m_y + .5f);
    rtROI.m_right += (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom += (long)(fptCenter.m_y + .5f);
}

void VisionInspection2DMatrix::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();

    if ((frtBodyRect.Width() <= 0.f || frtBodyRect.Height() <= 0.f) && m_bUseBypassMode == false) //kircheis_NGRV Bypass
        return;
    else if (m_bUseBypassMode)
    {
        frtBodyRect = GetPaneRect();
    }

    ImageRectToAbsoluteRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspection2DMatrix::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left -= (long)(fptCenter.m_x + .5f);
    rtROI.m_top -= (long)(fptCenter.m_y + .5f);
    rtROI.m_right -= (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom -= (long)(fptCenter.m_y + .5f);
}

BOOL VisionInspection2DMatrix::DoInsp2DMatrix(
    const bool detailSetupMode, std::vector<Ipvm::Image8u> vecImage, Ipvm::Rect32r frtBody)
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_sz2DMatrixInspName[MATRIX_INSPECTION_2D_MATRIX]);
    if (pResult == nullptr)
        return FALSE;
    pResult->Clear();

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == nullptr || !pSpec->m_use)
        return TRUE;

    if (frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        return FALSE;

    const long nReadingNumMajor = m_VisionPara->m_nMajorID_Count > 0 ? 1 : 0;
    const long nReadingNumSub = m_VisionPara->m_nSubID_Count;
    const long nReadingNumBaby = m_VisionPara->m_nBabyID_Count;
    const long nReadingNum = nReadingNumMajor + nReadingNumSub + nReadingNumBaby;
    if (nReadingNum <= 0)
        return TRUE;

    m_bIsTurnOn2DMatrix = TRUE; //kircheis_MED5_13 //검사가 정삭적으로 켜있다.

    pResult->Resize(nReadingNum);

    std::vector<Ipvm::Rect32s> vecrt2DID_ROI(nReadingNum);
    std::vector<CString> vecStrResultTotal2DID(nReadingNum);
    std::vector<CString> vecStrResultMajor2DID(nReadingNumMajor);
    std::vector<CString> vecStrResultSub2DID(nReadingNumSub);
    std::vector<CString> vecStrResultBaby2DID(nReadingNumBaby);

    long nImageNum = (long)vecImage.size();
    Ipvm::Rect32s rtSearchROI(0, 0, 0, 0);
    BOOL bResult(FALSE);
    float fResult(0.f);
    long nResult(0);
    CString strResult;
    CString strObjName;
    long nIndex(0);
    strResult = _T("NULL");
    strObjName.Format(_T("Major"));
    if (vecStrResultMajor2DID.size() == 0)
        vecStrResultMajor2DID.push_back(strResult);

    if (nReadingNumMajor > 0)
    {
        rtSearchROI = getScale().convert_umToPixel(m_VisionPara->m_vecrtSearchMajorID_BCU[0]);
        AbsoluteRectToImageRectByBodyCenter(frtBody, rtSearchROI);
        for (long nImage = 0; nImage < nImageNum; nImage++)
        {
            bResult = Reading2DID(vecImage[nImage], rtSearchROI, vecStrResultMajor2DID[0], vecrt2DID_ROI[nIndex]);
            if (bResult)
                break;
        }
        if (bResult && vecStrResultMajor2DID[0].IsEmpty())
            bResult = FALSE;

        if (bResult)
        {
            fResult = 0.f;
            nResult = PASS;
            strResult = vecStrResultMajor2DID[0];
            m_vecstr2DID[TYPE_2DID_MAJOR] = vecStrResultMajor2DID[0]; //kircheis_MED5_13
        }
        else
        {
            CString batchBarcode = getTrayScanSpec().GetBatchBarcode();
            if (batchBarcode.IsEmpty() || batchBarcode == _T("NOREAD"))
            {
                fResult = -1.f;
                nResult = REJECT;
                vecStrResultMajor2DID[0] = strResult = _T("NOREAD");
            }
            else
            {
                //------------------------------------------------------
                // Batch Inspection에서 Barcode 값이 들어가 있으면
                // 해당 정보를 읽은 것처럼 보여주자..
                //------------------------------------------------------

                vecStrResultMajor2DID[0] = batchBarcode;
                fResult = 0.f;
                nResult = PASS;
                strResult = vecStrResultMajor2DID[0];
            }
        }
        strObjName.Format(_T("Major"));
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nIndex, strObjName, _T(""), _T(""), fResult, *pSpec, 0.f, 0.f, 0.f, nResult); //kircheis_VSV
        pResult->SetResult(nIndex, nResult);
        pResult->SetRect(nIndex, vecrt2DID_ROI[nIndex]);
        m_vecrtROI[0] = vecrt2DID_ROI[nIndex]; //kircheis_MED5_13
        nIndex++;
    }
    vecStrResultTotal2DID[0].Format(_T("%s : %s"), (LPCTSTR)strObjName, (LPCTSTR)strResult);

    for (long nID = 0; nID < nReadingNumSub; nID++)
    {
        rtSearchROI = getScale().convert_umToPixel(m_VisionPara->m_vecrtSearchSubID_BCU[nID]);
        AbsoluteRectToImageRectByBodyCenter(frtBody, rtSearchROI);
        for (long nImage = 0; nImage < nImageNum; nImage++)
        {
            bResult = Reading2DID(vecImage[nImage], rtSearchROI, vecStrResultSub2DID[nID], vecrt2DID_ROI[nIndex]);
            if (bResult)
                break;
        }
        if (bResult && vecStrResultSub2DID[nID].IsEmpty())
            bResult = FALSE;

        if (bResult)
        {
            fResult = 0.f;
            nResult = PASS;
            strResult = vecStrResultSub2DID[nID];
            m_vecstr2DID[TYPE_2DID_SUB_0 + nID] = vecStrResultSub2DID[nID]; //kircheis_MED5_13
        }
        else
        {
            fResult = -1.f;
            nResult = REJECT;
            vecStrResultSub2DID[nID] = strResult = _T("NOREAD");
        }
        strObjName.Format(_T("Sub %d"), nID + 1);
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nIndex, strObjName, _T(""), _T(""), fResult, *pSpec, 0.f, 0.f, 0.f, nResult); //kircheis_VSV
        pResult->SetResult(nIndex, nResult);
        pResult->SetRect(nIndex, vecrt2DID_ROI[nIndex]);
        vecStrResultTotal2DID[nIndex].Format(_T("%s : %s"), (LPCTSTR)strObjName, (LPCTSTR)strResult);

        m_vecrtROI[TYPE_2DID_SUB_0 + nID] = vecrt2DID_ROI[nIndex]; //kircheis_MED5_13

        nIndex++;
    }
    for (long nID = 0; nID < nReadingNumBaby; nID++)
    {
        rtSearchROI = getScale().convert_umToPixel(m_VisionPara->m_vecrtSearchBabyID_BCU[nID]);
        AbsoluteRectToImageRectByBodyCenter(frtBody, rtSearchROI);
        for (long nImage = 0; nImage < nImageNum; nImage++)
        {
            bResult = Reading2DID(vecImage[nImage], rtSearchROI, vecStrResultBaby2DID[nID], vecrt2DID_ROI[nIndex]);
            if (bResult)
                break;
        }
        if (bResult && vecStrResultBaby2DID[nID].IsEmpty())
            bResult = FALSE;

        if (bResult)
        {
            fResult = 0.f;
            nResult = PASS;
            strResult = vecStrResultBaby2DID[nID];
            m_vecstr2DID[TYPE_2DID_BABY_0 + nID] = vecStrResultBaby2DID[nID]; //kircheis_MED5_13
        }
        else
        {
            fResult = -1.f;
            nResult = REJECT;
            vecStrResultBaby2DID[nID] = strResult = _T("NOREAD");
        }
        strObjName.Format(_T("Baby %d"), nID + 1);
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nIndex, strObjName, _T(""), _T(""), fResult, *pSpec, 0.f, 0.f, 0.f, nResult); //kircheis_VSV
        pResult->SetResult(nIndex, nResult);
        pResult->SetRect(nIndex, vecrt2DID_ROI[nIndex]);
        vecStrResultTotal2DID[nIndex].Format(_T("%s : %s"), (LPCTSTR)strObjName, (LPCTSTR)strResult);
        m_vecrtROI[TYPE_2DID_BABY_0 + nID] = vecrt2DID_ROI[nIndex]; //kircheis_MED5_13
        nIndex++;
    }
    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    if (auto* surfaceROI = getReusableMemory().AddSurfaceLayerRoiClass(_T("2D Matrix ROI")))
    {
        surfaceROI->Reset();
        for (auto& roi : vecrt2DID_ROI)
        {
            surfaceROI->Add(roi);
        }
    }

    SetDebugInfoItem(detailSetupMode, _T("2D Matrix ROI"), vecrt2DID_ROI, true);
    SetDebugInfoItem(detailSetupMode, _T("2D Matrix Result"), vecStrResultTotal2DID, true);
    SetDebugInfoItem(detailSetupMode, _T("2D Matrix Major Result"), vecStrResultMajor2DID, true);
    SetDebugInfoItem(detailSetupMode, _T("2D Matrix Sub Result"), vecStrResultSub2DID, true);
    SetDebugInfoItem(detailSetupMode, _T("2D Matrix Baby Result"), vecStrResultBaby2DID, true);

    m_s2DID_Cropping_image_Save_Info.SetCropping_image_Save_info(
        vecrt2DID_ROI, vecStrResultMajor2DID, vecStrResultSub2DID, vecStrResultBaby2DID);

    for (int nIdx = 0; nIdx < vecStrResultTotal2DID.size(); nIdx++)
    {
        vecStrResultTotal2DID[nIdx].Empty();
    }

    for (int nIdx = 0; nIdx < vecStrResultMajor2DID.size(); nIdx++)
    {
        vecStrResultMajor2DID[nIdx].Empty();
    }

    for (int nIdx = 0; nIdx < vecStrResultSub2DID.size(); nIdx++)
    {
        vecStrResultSub2DID[nIdx].Empty();
    }

    for (int nIdx = 0; nIdx < vecStrResultBaby2DID.size(); nIdx++)
    {
        vecStrResultBaby2DID[nIdx].Empty();
    }

    strResult.Empty();
    strObjName.Empty();

    return TRUE;
}

BOOL VisionInspection2DMatrix::Reading2DID(
    Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtROI, CString& o_strBarcode, Ipvm::Rect32s& o_rtBarcodeROI)
{
    BOOL retValue = FALSE;

    if (PersonalConfig::getInstance().isMatrixClientMode())
    {
        retValue = Reading2DIDbyClient(image, i_rtROI, o_strBarcode, o_rtBarcodeROI);
    }
    else
    {
        retValue = Reading2DIDbyEuresys(image, i_rtROI, o_strBarcode, o_rtBarcodeROI);
    }

    if (!retValue)
    {
        o_strBarcode = _T("");
        o_rtBarcodeROI = i_rtROI;
        o_rtBarcodeROI.DeflateRect(10, 10);
    }

    return retValue;
}

BOOL VisionInspection2DMatrix::Reading2DIDbyEuresys(
    Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtROI, CString& o_strBarcode, Ipvm::Rect32s& o_rtBarcodeROI)
{
    o_strBarcode = _T("");
    o_rtBarcodeROI = Ipvm::Rect32s(0, 0, 0, 0);

    //bool success = false;
    try
    {
        Euresys::Open_eVision::EImageBW8 eImageBW;
        Euresys::Open_eVision::EROIBW8 eROIBW8;
        Euresys::Open_eVision::EasyMatrixCode2::EMatrixCodeReader matrixCodeReader;

        Ipvm::Image8u matrixImage;
        if (!getReusableMemory().GetByteImage(matrixImage, i_rtROI.Width(), i_rtROI.Height()))
        {
            return FALSE;
        }

        Ipvm::ImageProcessing::Copy(image, i_rtROI, Ipvm::Rect32s(matrixImage), matrixImage);

        eROIBW8.SetPlacement(0, 0, matrixImage.GetSizeX(), matrixImage.GetSizeY());

        eImageBW.SetImagePtr(matrixImage.GetWidthBytes(), matrixImage.GetSizeY(), (void*)matrixImage.GetMem());

        eROIBW8.Attach(&eImageBW);

        auto matrixCodes = matrixCodeReader.Read(eROIBW8);

        if (matrixCodes.size() == 0)
        {
            return FALSE;
        }

        auto& matrixCode = matrixCodes.front();

        long offsetX_px = i_rtROI.m_left;
        long offsetY_px = i_rtROI.m_top;

        o_strBarcode = matrixCode.GetDecodedString().c_str();

        o_rtBarcodeROI.m_left = matrixImage.GetSizeX();
        o_rtBarcodeROI.m_top = matrixImage.GetSizeY();
        o_rtBarcodeROI.m_right = -matrixImage.GetSizeX();
        o_rtBarcodeROI.m_bottom = -matrixImage.GetSizeY();

        for (int i = 0; i < 4; i++)
        {
            long xx = CAST_LONG(matrixCode.GetPosition().GetPoint(i).GetX() + .5f);
            long yy = CAST_LONG(matrixCode.GetPosition().GetPoint(i).GetY() + .5f);

            o_rtBarcodeROI.m_left = (long)min(o_rtBarcodeROI.m_left, xx);
            o_rtBarcodeROI.m_top = (long)min(o_rtBarcodeROI.m_top, yy);
            o_rtBarcodeROI.m_right = (long)max(o_rtBarcodeROI.m_right, xx);
            o_rtBarcodeROI.m_bottom = (long)max(o_rtBarcodeROI.m_bottom, yy);
        }
        o_rtBarcodeROI.m_left += offsetX_px;
        o_rtBarcodeROI.m_right += offsetX_px;
        o_rtBarcodeROI.m_top += offsetY_px;
        o_rtBarcodeROI.m_bottom += offsetY_px;
    }
    catch (...)
    {
        return FALSE;
    }

    bool validCharector = true;

    if (m_VisionPara->m_validOnlyNumberAndAlphabet)
    {
        CString checkBarcode = o_strBarcode;
        while (!checkBarcode.IsEmpty())
        {
            long find = checkBarcode.FindOneOf(_T("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"));
            if (find != 0)
            {
                // 숫자나 영문이 아닌 문자가 들어왔다
                validCharector = false;
                break;
            }
            checkBarcode = checkBarcode.Mid(1);
        }
        checkBarcode.Empty();
    }

    if (o_strBarcode.GetLength() < m_VisionPara->m_minimumLengthForNormalString)
        validCharector = false;
    if (o_strBarcode.GetLength() > m_VisionPara->m_maximumLengthForNormalString)
        validCharector = false;

    if (!validCharector)
    {
        return FALSE;
    }

    return TRUE;
}

CCriticalSection g_csClient;

BOOL VisionInspection2DMatrix::Reading2DIDbyClient(
    Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtROI, CString& o_strBarcode, Ipvm::Rect32s& o_rtBarcodeROI)
{
    CSingleLock lock(&g_csClient, TRUE);

    TCHAR tempPath[MAX_PATH];
    GetTempPath(MAX_PATH, tempPath);

    CString binFilePath = PersonalConfig::getInstance().getMatrixClientProgramPath();
    CString imageFilePath;
    CString resultFilePath;
    CString parameter;

    imageFilePath.Format(_T("%sBarcode.bmp"), tempPath);
    resultFilePath.Format(_T("%sBarcode.txt"), tempPath);

    parameter.Format(_T("\"%s\" \"%s\""), LPCTSTR(imageFilePath), LPCTSTR(resultFilePath));

    //  실행파일 이미지.bmp, result.txt
    Ipvm::Image8u matrixImage(image, i_rtROI);
    Ipvm::ImageFile::SaveAsBmp(matrixImage, imageFilePath);
    DeleteFile(resultFilePath);

    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi = {};

    binFilePath += _T(" ") + parameter;

    if (!CreateProcess(nullptr, LPTSTR(LPCTSTR(binFilePath)), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        return FALSE;
    }
    auto hProcess = pi.hProcess;
    auto hThread = pi.hThread;

    if (hProcess)
    {
        WaitForSingleObject(hProcess, INFINITE);

        if (hThread)
            CloseHandle(hThread);

        CloseHandle(hProcess);
    }

    bool enable = IniHelper::LoadINT(resultFilePath, _T("Barcode Text"), _T("enable"), 0);
    if (!enable)
    {
        return FALSE;
    }

    o_rtBarcodeROI.m_left = IniHelper::LoadINT(resultFilePath, _T("Barcode ROI"), _T("left"));
    o_rtBarcodeROI.m_top = IniHelper::LoadINT(resultFilePath, _T("Barcode ROI"), _T("top"));
    o_rtBarcodeROI.m_right = IniHelper::LoadINT(resultFilePath, _T("Barcode ROI"), _T("right"));
    o_rtBarcodeROI.m_bottom = IniHelper::LoadINT(resultFilePath, _T("Barcode ROI"), _T("bottom"));
    o_strBarcode = IniHelper::LoadSTRING(resultFilePath, _T("Barcode Text"), _T("text"));

    o_rtBarcodeROI.OffsetRect(i_rtROI.TopLeft().m_x, i_rtROI.TopLeft().m_y);

    binFilePath.Empty();
    imageFilePath.Empty();
    resultFilePath.Empty();
    parameter.Empty();

    return TRUE;
}

BOOL VisionInspection2DMatrix::GetMajorRoiCenter(Ipvm::Point32s2& ptCenter)
{
    ptCenter.Set(0, 0);

    long nMajorNum = (long)m_VisionPara->m_vecrtSearchMajorID_BCU.size();
    static const BOOL bIs2DVision = SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP
        || SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_TR;

    if (bIs2DVision == FALSE || IsEnabled() == FALSE || nMajorNum <= 0)
        return FALSE;

    Ipvm::Point32r2 fptCenter = m_VisionPara->m_vecrtSearchMajorID_BCU[0].CenterPoint();

    ptCenter.m_x = (fptCenter.m_x >= 0.f) ? (long)(fptCenter.m_x + .5f) : (long)(fptCenter.m_x - .5f);

    ptCenter.m_y = (fptCenter.m_y >= 0.f) ? (long)(fptCenter.m_y + .5f) : (long)(fptCenter.m_y - .5f);

    return TRUE;
}

BOOL VisionInspection2DMatrix::IsTurnOn2DMatrix() //kircheis_20230126
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_sz2DMatrixInspName[MATRIX_INSPECTION_2D_MATRIX]);
    if (pResult == nullptr)
        return FALSE;

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == nullptr)
        return FALSE;

    return pSpec->m_use;
}

BOOL VisionInspection2DMatrix::DoInspMatchString(const bool detailSetupMode) //kircheis_MED5_13
{
    VisionInspectionResult* pResult
        = m_resultGroup.GetResultByName(g_sz2DMatrixInspName[MATRIX_INSPECTION_STRING_MATCH]);
    if (pResult == nullptr)
        return FALSE;
    pResult->Clear();

    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == nullptr || !pSpec->m_use)
        return TRUE;

    //선행 검사가 켜져 있는지 확인
    if (m_bIsTurnOn2DMatrix == FALSE)
    {
        if (detailSetupMode == TRUE)
            SimpleMessage(
                _T("[2DID string match] inspection item requires [2D Matrix] inspection item to be turned on."));

        return FALSE;
    }

    //비교 대상으로 설정되어 있는 2DID들이 정상적으로 읽혔는지 확인하고 정상적이지 않은 2DID를 별도의 CString에 저장(검사 ID 이름에 넣을 수도 있음. 사전 작업)
    long nFirstID = m_VisionPara->m_n1st2DIDforMatch;
    long nSecondID = m_VisionPara->m_n2nd2DIDforMatch;

    CString strFirstText = Get2DIDText(nFirstID);
    CString strSecondText = Get2DIDText(nSecondID);

    CString strError;
    long nRejID = 0;
    if (strFirstText.IsEmpty())
    {
        strError = Get2DIDName(nFirstID);
        nRejID = nFirstID;
    }
    if (strSecondText.IsEmpty())
    {
        if (strError.IsEmpty())
        {
            strError = Get2DIDName(nSecondID);
            nRejID = nSecondID;
        }
        else
            strError += (_T(", ") + Get2DIDName(nSecondID));
    }

    pResult->Resize(1);
    if (strError.IsEmpty() == FALSE)
    {
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, strError, _T(""), _T(""), -2.f, *pSpec, 0.f, 0.f, 0.f, REJECT); //kircheis_VSV
        pResult->SetResult(0, REJECT);
        pResult->SetRect(0, m_vecrtROI[nRejID]);

        pResult->UpdateTypicalValue(pSpec);
        pResult->SetTotalResult();

        strFirstText.Empty();
        strSecondText.Empty();
        strError.Empty();
        return TRUE;
    }

    //다 정상적이면 분자열 비교 하고 양불 판정

    long nResult = REJECT;
    float fResult = -1.f;
    if (strFirstText == strSecondText)
    {
        nResult = PASS;
        fResult = 0.f;
    }

    pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("Not match"), _T(""), _T(""), fResult, *pSpec, 0.f, 0.f, 0.f, nResult); //kircheis_VSV
    pResult->SetResult(0, nResult);
    pResult->SetRect(0, m_vecrtROI[nFirstID]);

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strFirstText.Empty();
    strSecondText.Empty();

    return TRUE;
}

CString VisionInspection2DMatrix::Get2DIDText(const long n2DIDType) //kircheis_MED5_13
{
    CString strError;
    strError.Empty();

    if (n2DIDType < TYPE_2DID_MAJOR || n2DIDType >= TYPE_2DID_END)
        return strError;

    return m_vecstr2DID[n2DIDType];
}

CString VisionInspection2DMatrix::Get2DIDName(const long n2DIDType) //kircheis_MED5_13
{
    CString strIDName;
    strIDName.Empty();

    switch (n2DIDType)
    {
        case TYPE_2DID_MAJOR:
            strIDName.Format(_T("MajorID"));
            break;
        case TYPE_2DID_SUB_0:
            strIDName.Format(_T("Sub 0"));
            break;
        case TYPE_2DID_SUB_1:
            strIDName.Format(_T("Sub 1"));
            break;
        case TYPE_2DID_BABY_0:
            strIDName.Format(_T("Baby 0"));
            break;
        case TYPE_2DID_BABY_1:
            strIDName.Format(_T("Baby 1"));
            break;
    }

    return strIDName;
}

void VisionInspection2DMatrix::Save_2DID_Cropping_image_Debug(const e2DIDCroppingimageSaveOption i_eOption,
    const std::vector<s2DID_Using_Frame_info> i_vecFrameinfo, const s2DID_Cropping_image_Save_Info i_sSaveInfo)
{
    if (i_eOption == e2DIDCroppingimageSaveOption::e2DID_Cropping_image_SaveOption_NotSave
        || i_eOption == e2DIDCroppingimageSaveOption::e2DID_Cropping_image_SaveOption_SaveNotMatchedimage
        || i_vecFrameinfo.size() == 0 || i_sSaveInfo.m_vecrt2DID_ROI.size() == 0)
        return;

    DeleteOldDirectory(_T("D:\\2DID\\")); //7일이 지나면 무조건 삭제한다

    const auto& imageInfo = m_visionUnit.getImageLot().GetInfo();
    const CString strLotIDFromHost = SystemConfig::GetInstance().Get_Lot_IDFromHost();
    const CString strLotIDUnknown = _T("Unknown");
    const CString strLotID = strLotIDFromHost.IsEmpty() == true ? strLotIDUnknown : strLotIDFromHost;
    const CTime LotStartTime = SystemConfig::GetInstance().Get_Lot_Start_TimeFromHost();
    const long nTrayID = imageInfo.m_trayIndex;
    const long nScanID = imageInfo.m_scanAreaIndex;
    const long nPaneID = m_visionUnit.GetCurrentPaneID();

    const long nExtensionSize_um = 200; //200um정도 늘려서 저장할 것이다
    const Ipvm::Point32r2 fExtensionSize_px = Ipvm::Point32r2(
        nExtensionSize_um * getScale().umToPixel().m_x, nExtensionSize_um * getScale().umToPixel().m_y);

    //어차피 Debug용도로 사용하고 버릴것이니, Path는 별도로 빼지않는다
    const CString strBaseDirectory(_T("D:\\2DID\\"));
    CString strSubDirectory = strBaseDirectory;
    CString strSaveFileName("");

    strSubDirectory.AppendFormat(_T("%d-%d-%d-%d-%d\\"), LotStartTime.GetYear(), LotStartTime.GetMonth(),
        LotStartTime.GetDay(), LotStartTime.GetHour(), LotStartTime.GetMinute());

    Ipvm::CreateDirectories(strBaseDirectory);
    Ipvm::CreateDirectories(strSubDirectory);

    strSubDirectory.AppendFormat(_T("%s\\"), LPCTSTR(strLotID));

    Ipvm::CreateDirectories(strSubDirectory);

    for (long nMajoridx = 0; nMajoridx < i_sSaveInfo.m_vecstr_Major_Result.size(); nMajoridx++)
    {
        strSaveFileName.Format(_T("T%d_S%d_P%d_%s_M%d"), nTrayID, nScanID, nPaneID,
            LPCTSTR(i_sSaveInfo.m_vecstr_Major_Result[nMajoridx]), nMajoridx);

        for (long nFrameidx = 0; nFrameidx < i_vecFrameinfo.size(); nFrameidx++)
        {
            auto FrameInfo = i_vecFrameinfo[nFrameidx];

            CString strSaveFilePath = strSubDirectory + strSaveFileName;
            strSaveFilePath.AppendFormat(_T("_F%d.jpg"), FrameInfo.m_nFrameidx);

            Ipvm::Rect32s rtSaveROI = i_sSaveInfo.m_vecrt2DID_ROI[nMajoridx];
            rtSaveROI.InflateRect(
                static_cast<int32_t>(fExtensionSize_px.m_x), static_cast<int32_t>(fExtensionSize_px.m_y));

            Ipvm::Image8u SaveMajorimage(FrameInfo.m_image, rtSaveROI);

            Ipvm::ImageFile::SaveAsJpeg(SaveMajorimage, strSaveFilePath, 99);
        }
    }

    for (long nSubidx = 0; nSubidx < i_sSaveInfo.m_vecstr_Sub_Result.size(); nSubidx++)
    {
        strSaveFileName.Format(_T("T%d_S%d_P%d_%s_S%d"), nTrayID, nScanID, nPaneID,
            LPCTSTR(i_sSaveInfo.m_vecstr_Sub_Result[nSubidx]), nSubidx);

        for (long nFrameidx = 0; nFrameidx < i_vecFrameinfo.size(); nFrameidx++)
        {
            auto FrameInfo = i_vecFrameinfo[nFrameidx];

            CString strSaveFilePath = strSubDirectory + strSaveFileName;
            strSaveFilePath.AppendFormat(_T("_F%d.jpg"), FrameInfo.m_nFrameidx);

            long nSub2DIDROI_index = (long)i_sSaveInfo.m_vecstr_Major_Result.size() + nSubidx;

            Ipvm::Rect32s rtSaveROI = i_sSaveInfo.m_vecrt2DID_ROI[nSub2DIDROI_index];
            rtSaveROI.InflateRect(
                static_cast<int32_t>(fExtensionSize_px.m_x), static_cast<int32_t>(fExtensionSize_px.m_y));

            Ipvm::Image8u SaveMajorimage(FrameInfo.m_image, rtSaveROI);

            Ipvm::ImageFile::SaveAsJpeg(SaveMajorimage, strSaveFilePath, 99);
        }
    }

    for (long nBabyidx = 0; nBabyidx < i_sSaveInfo.m_vecstr_Baby_Result.size(); nBabyidx++)
    {
        strSaveFileName.Format(_T("T%d_S%d_P%d_%s_B%d"), nTrayID, nScanID, nPaneID,
            LPCTSTR(i_sSaveInfo.m_vecstr_Baby_Result[nBabyidx]), nBabyidx);

        for (long nFrameidx = 0; nFrameidx < i_vecFrameinfo.size(); nFrameidx++)
        {
            auto FrameInfo = i_vecFrameinfo[nFrameidx];

            CString strSaveFilePath = strSubDirectory + strSaveFileName;
            strSaveFilePath.AppendFormat(_T("_F%d.jpg"), FrameInfo.m_nFrameidx);

            long nBaby2DIDROI_index = (long)i_sSaveInfo.m_vecstr_Major_Result.size()
                + (long)i_sSaveInfo.m_vecstr_Sub_Result.size() + nBabyidx;

            Ipvm::Rect32s rtSaveROI = i_sSaveInfo.m_vecrt2DID_ROI[nBaby2DIDROI_index];
            rtSaveROI.InflateRect(
                static_cast<int32_t>(fExtensionSize_px.m_x), static_cast<int32_t>(fExtensionSize_px.m_y));

            Ipvm::Image8u SaveMajorimage(FrameInfo.m_image, rtSaveROI);

            Ipvm::ImageFile::SaveAsJpeg(SaveMajorimage, strSaveFilePath, 99);
        }
    }
}

void VisionInspection2DMatrix::DeleteOldDirectory(const CString i_strSearchDirectoryPath)
{
    if (i_strSearchDirectoryPath.IsEmpty() == true)
        return;

    CFileFind cFileFind;

    CString strSearchPath = i_strSearchDirectoryPath + _T("*.*");
    BOOL bSearchDirecotryFindSuccess = cFileFind.FindFile(strSearchPath);

    while (bSearchDirecotryFindSuccess)
    {
        bSearchDirecotryFindSuccess = cFileFind.FindNextFile();
        if (cFileFind.IsDirectory() && !cFileFind.IsDots())
        {
            CTime cTimeCreateDirectoryInfo;
            cFileFind.GetCreationTime(cTimeCreateDirectoryInfo);

            CString strDebugFilePath = cFileFind.GetFilePath();

            if (isRemoveOldDirectory(cTimeCreateDirectoryInfo) == true)
            {
                std::vector<CString> vecstrFolderPathinOldDirectory;
                //폴더안에 있는 내용을 검색
                if (isRemoveOldDirectoryinFolder(cFileFind.GetFilePath(), vecstrFolderPathinOldDirectory)
                    == true) //폴더가 있다면
                {
                    //폴더이름내의 File을 검색 및 삭제한다
                    for (auto CurFolderPath : vecstrFolderPathinOldDirectory)
                    {
                        if (RemoveOldDirectoryinFolderinFiles(CurFolderPath) == true) //File을 다 지웠다면 삭제해야지
                        {
                            ::RemoveDirectory(CurFolderPath);
                        }
                    }
                }

                //마지막에 Directory를 삭제한다
                ::RemoveDirectory(cFileFind.GetFilePath());
            }
        }
    }
    cFileFind.Close();
}

bool VisionInspection2DMatrix::isRemoveOldDirectory(const CTime i_CurCreateDirecotryDate)
{
    SYSTEMTIME sytime_CurLocal;
    GetLocalTime(&sytime_CurLocal);

    tm tmCur = {0};
    tmCur.tm_year = sytime_CurLocal.wYear - 1900;
    tmCur.tm_mon = sytime_CurLocal.wMonth - 1;
    tmCur.tm_mday = sytime_CurLocal.wDay;
    tmCur.tm_hour = sytime_CurLocal.wHour;
    tmCur.tm_min = sytime_CurLocal.wMinute;
    tmCur.tm_sec = sytime_CurLocal.wSecond;
    tmCur.tm_isdst = 0;

    time_t tm_CurDate = mktime(&tmCur);

    long nDayDate = 24 * 60 * 60; //Hour, Minute, Second, Day
    long nDeleteDate = nDayDate * 7; //현재일로부터 7일이상 되었으면 그폴더는 무조건 삭제한다

    tm tmFolder = {0};

    tmFolder.tm_year = i_CurCreateDirecotryDate.GetYear() - 1900;
    tmFolder.tm_mon = i_CurCreateDirecotryDate.GetMonth() - 1;
    tmFolder.tm_mday = i_CurCreateDirecotryDate.GetDay();
    tmFolder.tm_hour = i_CurCreateDirecotryDate.GetHour();
    tmFolder.tm_min = i_CurCreateDirecotryDate.GetMinute();
    tmFolder.tm_sec = i_CurCreateDirecotryDate.GetSecond();
    tmFolder.tm_isdst = 0;

    time_t tm_TargetFolderDate = mktime(&tmFolder);

    double diff = fabs(difftime(tm_TargetFolderDate, tm_CurDate));

    if (diff > nDeleteDate)
        return true;

    return false;
}

bool VisionInspection2DMatrix::isRemoveOldDirectoryinFolder(
    const CString i_strSearchDirectoryPath, std::vector<CString>& o_vecFolderPath)
{
    o_vecFolderPath.clear();

    CFileFind cFileFind;

    CString strSearchPath = i_strSearchDirectoryPath + _T("\\*.*");
    BOOL bSearchDirecotryFindSuccess = cFileFind.FindFile(strSearchPath);

    if (bSearchDirecotryFindSuccess != TRUE)
        return false;

    while (bSearchDirecotryFindSuccess)
    {
        bSearchDirecotryFindSuccess = cFileFind.FindNextFile();
        if (cFileFind.IsDirectory() && !cFileFind.IsDots())
        {
            o_vecFolderPath.push_back(cFileFind.GetFilePath());
        }
    }

    cFileFind.Close();

    if (o_vecFolderPath.size() != 0)
        return true;

    return false;
}

bool VisionInspection2DMatrix::RemoveOldDirectoryinFolderinFiles(const CString i_strSearchDirectoryPath)
{
    if (i_strSearchDirectoryPath.IsEmpty() == true)
        return false;

    std::vector<CString> vecstrFileNames, vecstrFilePaths;
    if (SystemConfig::GetInstance().GetFileNamesinDirecotry(
            i_strSearchDirectoryPath, _T("\\*.jpg"), vecstrFileNames, vecstrFilePaths)
        == true) //파일이 있는것이고
    {
        for (auto DeleteFilePath : vecstrFilePaths)
            ::DeleteFile(DeleteFilePath);

        return true;
    }
    else //파일이 없으니 폴더자체를 날리면 된다
        ::RemoveDirectory(i_strSearchDirectoryPath);

    return false;
}
