//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionMainTR.h"

//CPP_2_________________________________ This project's headers
#include "InlineGrab.h"
#include "Resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../HardwareModules/dPI_SyncController/IlluminationTable.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InspectionItems/VisionInspectionSurface/SurfaceCriteria.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurface.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurfacePara.h"
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ArchiveAllType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/BinaryFileData.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/SideVisionDeviceResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionDeviceResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionSystemParameters.h"
#include "../../SharedCommunicationModules/VisionHostCommon/iPIS_MsgDefinitions.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
void VisionMainTR::SendVersionInfo()
{
    long nID = 0;
    DEFMSGPARAM Data;
    Data.P1 = -1;
    Data.P2 = -1;
    Data.P3 = -1;
    Data.P4 = -1;
    Data.P5 = -1;
    Data.P6 = -1;
    Data.P7 = -1;
    Data.P8 = -1;

    CString strVersioin = m_strDetailFileVersion;

    // [180424/송준호]
    // 아직 Version을 어떻게 관리할지에 대한 부분이 결정된 것이 없어서
    // FileVersion이라는 것은 무시했다

    /*
	CString strFileVersion = _T("360.02.04.0001R");
	//Job File Version 추출
	long nP1 = -1, nP2 = -1, nP3 = -1, nP4 = -1;

	nID = strFileVersion.Find('.');
	nP1 = _ttoi(strFileVersion.Left(nID));
	strFileVersion = strFileVersion.Right(strFileVersion.GetLength() - nID - 1);

	nID = strFileVersion.Find('.');
	nP2 = _ttoi(strFileVersion.Left(nID));
	strFileVersion = strFileVersion.Right(strFileVersion.GetLength() - nID - 1);

	nID = strFileVersion.Find('.');
	nP3 = _ttoi(strFileVersion.Left(nID));
	strFileVersion = strFileVersion.Right(strFileVersion.GetLength() - nID - 1);

	nID = strFileVersion.Find('R');

	if (nID < 0)
		nID = strFileVersion.GetLength();

	nP4 = _ttoi(strFileVersion.Left(nID));
	*/
    // Program Version Send
    nID = 0;

    nID = strVersioin.Find('.');
    Data.P1 = _ttoi(strVersioin.Left(nID));
    strVersioin = strVersioin.Right(strVersioin.GetLength() - nID - 1);

    nID = strVersioin.Find('.');
    Data.P2 = _ttoi(strVersioin.Left(nID));
    strVersioin = strVersioin.Right(strVersioin.GetLength() - nID - 1);

    nID = strVersioin.Find('.');
    Data.P3 = _ttoi(strVersioin.Left(nID));
    strVersioin = strVersioin.Right(strVersioin.GetLength() - nID - 1);

    nID = strVersioin.Find('R');

    if (nID < 0)
        nID = strVersioin.GetLength();

    Data.P4 = _ttoi(strVersioin.Left(nID));

    // [180424/송준호]
    // 아직 Version을 어떻게 관리할지에 대한 부분이 결정된 것이 없어서 일단 무조건 Version을 PASS로 만들어 놨다.

    Data.P5 = 1; // PASS
    /*
	// 영훈 20141212_SendToHost_JobfileVersion : 현재 프로그램과 Job의 Version을 비교하여 호스트에 알려준다. Program and job version compare function
	if (Data.P3 < nP3 && nP1 != 360) // Job의 Version이 Program보다 높다면 ? 380에서 360 잡은 그냥 호환 된다 하자
	{
		if (Data.P2 >= nP2) // Job 버전은 낮지만 통신 버전이 높다면 OK.//같아도 괜춘한걸로 하자/
		{
			Data.P5 = 1; // PASS
		}
		else
		{
			Data.P5 = 0; // 버전에 문제 발생 ( FALSE )
		}
	}
	else
	{
		Data.P5 = 1; // PASS
	}
	*/
    long length = sizeof(Data);

    m_pMessageSocket->Write(MSG_PROBE_VERSION_INFO, length, (BYTE*)&Data);
}

void VisionMainTR::iPIS_Send_ImageSize(long nSendImageSizeX, long nSendImageSizeY)
{
    if ((m_nSendImageSizeX != nSendImageSizeX) || (m_nSendImageSizeY != nSendImageSizeY))
    {
        long length;
        DEFMSGPARAM Data;
        Data.P1 = nSendImageSizeX;
        Data.P2 = nSendImageSizeY;

        // 영훈 20140103 : Host에서 Position 및 Focus Teaching 시 원본 이미지를 사용할 수 있도록 한다.
        // 3D Image는 Size가 다르므로 이걸 덮어 써버리면 영상이 이상하게 나온다. 여기서 예외처리를 해놓지만 핸들러에서 처리해줘야할 수도 있다.
        if (SystemConfig::GetInstance().IsVisionType2DorSide() == true
            || SystemConfig::GetInstance().m_nVisionType == VISIONTYPE_NGRV_INSP)
        {
            Data.P3 = FrameGrabber::GetInstance().get_grab_image_width();
            Data.P4 = FrameGrabber::GetInstance().get_grab_image_height();
        }

        m_nSendImageSizeX = nSendImageSizeX;
        m_nSendImageSizeY = nSendImageSizeY;

        length = sizeof(Data);

        m_pMessageSocket->Write(MSG_PROBE_IMAGE_SIZE, length, (BYTE*)&Data);
    }
}

// 영훈 20150703 : image Size 전송하는 함수는 하나로 묶어서 관리한다.
void VisionMainTR::iPIS_Send_Image(const Ipvm::Image8u& image)
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    // 영훈 20150702 : 1/8배로 압축해서 호스트로 보내도록 한다.
    long nResizeScale = SEND_IMAGE_RESCALE_FACTOR;

    // Reszie image Size 계산
    long nSendImageSizeX = ((image.GetSizeX() / nResizeScale + 3) / 4) * 4;
    long nSendImageSizeY = image.GetSizeY() / nResizeScale;

    // 3D는 image size가 바뀔 수 있으므로 항상 체크한다.
    iPIS_Send_ImageSize(nSendImageSizeX, nSendImageSizeY);

    // image reszie시 for문 연산을 IPP 연산을 하도록 한다.
    Ipvm::Image8u resizeImage;
    resizeImage.Create(nSendImageSizeX, nSendImageSizeY);
    Ipvm::ImageProcessing::ResizeLinearInterpolation(image, resizeImage);

    // Host로 Size가 변경된 Image 전송 ( 모든 Thread가전송 한다. )
    m_pMessageSocket->Write(MSG_PROBE_IMAGE_SEND_1, m_nSendImageSizeX * m_nSendImageSizeY, resizeImage.GetMem());
}

void VisionMainTR::iPIS_Send_Image(const Ipvm::Image8u& imageFront, const Ipvm::Image8u& imageRear)
{
    static const bool bIsSideVision = SystemConfig::GetInstance().IsVisionTypeSide();

    if (m_pMessageSocket->IsConnected() == false || bIsSideVision == false)
        return;

    long nImageFrontSizeX = (long)imageFront.GetSizeX();
    long nImageFrontSizeY = (long)imageFront.GetSizeY();
    long nImageRearSizeX = (long)imageRear.GetSizeX();
    long nImageRearSizeY = (long)imageRear.GetSizeY();

    if (nImageFrontSizeX != nImageRearSizeX
        || nImageFrontSizeY != nImageRearSizeY) //Front와 Rear는 영상의 크기가 같아야 한다.
        return;

    // 영훈 20150702 : 1/8배로 압축해서 호스트로 보내도록 한다.
    long nResizeScale = SEND_IMAGE_RESCALE_FACTOR;

    // Reszie image Size 계산
    long nSendImageSizeX = ((nImageFrontSizeX / nResizeScale + 3) / 4) * 4;
    long nResizeSingleImageSizeY = nImageFrontSizeY / nResizeScale; //Front와 Rear의 각각 Resize된 Image의 SizeY
    long nSendImageSizeY = nResizeSingleImageSizeY * 2; //두 영상을 상하로 붙인 후 전송하는 Image의 SizeY

    // 3D는 image size가 바뀔 수 있으므로 항상 체크한다.
    iPIS_Send_ImageSize(nSendImageSizeX, nSendImageSizeY);

    // image reszie시 for문 연산을 IPP 연산을 하도록 한다.
    Ipvm::Image8u resizeImage; //Resize용 버퍼
    resizeImage.Create(nSendImageSizeX, nResizeSingleImageSizeY);
    Ipvm::Image8u mergedImage; //합친 후 전송할 버퍼
    mergedImage.Create(nSendImageSizeX, nSendImageSizeY);

    Ipvm::ImageProcessing::ResizeLinearInterpolation(imageFront, resizeImage); //Front Image Resize

    Ipvm::Rect32s rtROI = (Ipvm::Rect32s)resizeImage; //기본 ROI 위치 정보
    Ipvm::Rect32s rtRearDstROI = rtROI; //Rear Resize Image를 붙일 위치. 기본 위치를 넣고
    rtRearDstROI.OffsetRect(0, nResizeSingleImageSizeY); //Single Size 만큼 Y축 이동

    Ipvm::ImageProcessing::Copy(resizeImage, rtROI, rtROI, mergedImage); //머지 버퍼에 Front Resize Image 복사하고
    Ipvm::ImageProcessing::ResizeLinearInterpolation(imageRear, resizeImage); //Rear Image를 Resize
    Ipvm::ImageProcessing::Copy(
        resizeImage, rtROI, rtRearDstROI, mergedImage); //머지 버퍼의 하단부에 Rear Resize Image 복사

    // Host로 Size가 변경된 Image 전송 ( 모든 Thread가전송 한다. )
    m_pMessageSocket->Write(MSG_PROBE_IMAGE_SEND_1, nSendImageSizeX * nSendImageSizeY, mergedImage.GetMem());
}

void VisionMainTR::IPIS_Send_NGRV_Image(const Ipvm::Image8u3& image)
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    // 영훈 20150702 : 1/8배로 압축해서 호스트로 보내도록 한다.
    long nResizeScale = SEND_IMAGE_RESCALE_FACTOR;

    // Reszie image Size 계산
    long nSendImageSizeX = ((image.GetSizeX() / nResizeScale + 3) / 4) * 4;
    long nSendImageSizeY = image.GetSizeY() / nResizeScale;

    // 3D는 image size가 바뀔 수 있으므로 항상 체크한다.
    iPIS_Send_ImageSize(nSendImageSizeX, nSendImageSizeY);

    // image reszie시 for문 연산을 IPP 연산을 하도록 한다.
    Ipvm::Image8u3 resizeImage;
    resizeImage.Create(nSendImageSizeX, nSendImageSizeY);
    Ipvm::ImageProcessing::ResizeLinearInterpolation(image, resizeImage);

    // Host로 Size가 변경된 Image 전송 ( 모든 Thread가전송 한다. )
    m_pMessageSocket->Write(MSG_PROBE_IMAGE_SEND_1, m_nSendImageSizeX * m_nSendImageSizeY, resizeImage.GetMem());
}

void VisionMainTR::iPIS_Send_2DAllFrame()
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        std::vector<CString> vecstr2DAll_ReviewImage_FilePath = getAllFrame_2D_FilePath(false);
        std::vector<CString> vecstr2DAll_RawImage_FilePath = getAllFrame_2D_FilePath(true);

        if (vecstr2DAll_ReviewImage_FilePath.size() <= 0 || vecstr2DAll_RawImage_FilePath.size() <= 0)
            return;

        std::vector<CString> vecstr2DAll_SummaryFilePath = vecstr2DAll_ReviewImage_FilePath;

        for (auto RawImagePath : vecstr2DAll_RawImage_FilePath)
            vecstr2DAll_SummaryFilePath.push_back(RawImagePath);

        long nSendFilePathSize = (long)vecstr2DAll_SummaryFilePath.size();

        ar << nSendFilePathSize;
        for (auto SendFilePath : vecstr2DAll_SummaryFilePath)
            ar << SendFilePath;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_SAMPLE_IMAGE_FILES, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send 2D All Frame FilePath to Host"));

    ::free(pByte);
}

// iPIS_Send_Job에는 iPis_Send_JobInfo가 포함되어 있다.
// 두번 보내면 문제 될수 있어서 iPis_Send_Job에서 한 메시지 처리했다. 수정시 같이 수정 필요
void VisionMainTR::iPIS_Send_Job(CiDataBase& jobDB, long nMsg) //CJobFile* 를 전송을 하는 모든 일을 처리함.
{
    BOOL bSendItemList = TRUE;
    if (SystemConfig::GetInstance().IsBtm2DVision() == TRUE && GetPrimaryVisionUnit().Is2DMatrixReadingBypass() == TRUE)
    {
        if (nMsg == MSG_PROBE_TEACH_END)
        {
            nMsg = MSG_PROBE_TEACH_END_FOR_BYPASS;
            bSendItemList = FALSE;
        }
        else if (nMsg == MSG_PROBE_UPDATE_JOB)
        {
            nMsg = MSG_PROBE_UPDATE_JOB_FOR_BYPASS;
            bSendItemList = FALSE;
        }
    }

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    static const bool bIs380Bridge = SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE;

    auto& primaryUnit = GetPrimaryVisionUnit();
    BinaryFileData jobBinary;

    try
    {
        // 다음에 전체 job을 보내준다.  >> host는 이 job을 서버로 전송
        jobBinary.FromDB(jobDB);
        jobBinary.SerializeForComm(ar);

        // 그 뒤는 똑같이 한다.
        auto visionInspectionSpecs = primaryUnit.GetVisionInspectionSpecs();

        long nSize = (long)(visionInspectionSpecs.size());

        // 영훈 : inspection Spec 전송
        ar << nSize;

        SetSpecNormalValue(visionInspectionSpecs);

        for (int n = 0; n < nSize; n++)
        {
            ar << *(visionInspectionSpecs[n]);
        }

        // 영훈 20150903 : Host에 Ball이나 Lead, Pad의 개수를 함게 보내도록 한다.
        CPackageSpec* pPackageSpec = primaryUnit.m_pPackageSpec;

        long nBallNumber = (long)pPackageSpec->m_ballMap->m_balls.size();
        long nLandNumber = (long)pPackageSpec->m_LandMapManager->GetCount();
        long nPassiveNumber = (long)pPackageSpec->m_OriginCompData->vecOriginCompData.size();

        ar << nBallNumber;
        ar << nLandNumber;
        ar << nPassiveNumber;

        ar << pPackageSpec->m_bodyInfoMaster->fBodySizeX;
        ar << pPackageSpec->m_bodyInfoMaster->fBodySizeY;
        ar << pPackageSpec->m_bodyInfoMaster->fBodyThickness;

        ar << primaryUnit.IsNeedMarkTeach();

        auto& systemConfig = SystemConfig::GetInstance();
        static const long nVisionType = systemConfig.GetVisionType();
        long nErrorCode(RECIPE_VALID), nFrameNum(0);
        float fScaleZ = (nVisionType == VISIONTYPE_3D_INSP) ? systemConfig.GetScale3D() : 1.f;
        CString strErrorContents, strPinIndexDirection;
        std::vector<CString> vecstrIllumExposureTimeMS(0);

        //{{//kircheis_VerifyRecipe
        GetErrorInfo(nErrorCode, strErrorContents);
        ar << nErrorCode << strErrorContents;
        //}}

        ar << systemConfig.Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
           << systemConfig.Get2DScaleY(enSideVisionModule::SIDE_VISIONMODULE_FRONT) << fScaleZ;

        //{{//kircheis_SideVisionScale
        static const BOOL bIsSideVision = systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP;
        float fRearScaleX = 1.f;
        float fRearScaleY = 1.f;
        float fRearScaleZ = 1.f;
        if (bIsSideVision)
        {
            fRearScaleX = systemConfig.Get2DScaleX(SIDE_VISIONMODULE_REAR);
            fRearScaleY = systemConfig.Get2DScaleY(SIDE_VISIONMODULE_REAR);
            ar << fRearScaleX << fRearScaleY << fRearScaleZ;
        }
        //}}

        strPinIndexDirection = pPackageSpec->GetPinIndexDirection(); //kircheis_PinIdxDir
        ar << strPinIndexDirection; //kircheis_PinIdxDir

        //{{//kircheis_IllumData
        GetIllumInfo(vecstrIllumExposureTimeMS);
        nFrameNum = (long)vecstrIllumExposureTimeMS.size();
        ar << nFrameNum;
        for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
            ar << vecstrIllumExposureTimeMS[nFrame];
        //}}

        long nImageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
        long nImageSizeY = FrameGrabber::GetInstance().get_grab_image_height();

        ar << nImageSizeX << nImageSizeY;

        Ipvm::Point32s2 ptMajorMatrixRoiCenter(0, 0);
        BOOL bValidMajorMatrixROICenter = FALSE;
        if (GetPrimaryVisionUnit().Is2DMatrixReadingBypass() == false)
            bValidMajorMatrixROICenter = primaryUnit.GetMajorRoiCenter(ptMajorMatrixRoiCenter);

        ar << bValidMajorMatrixROICenter << ptMajorMatrixRoiCenter.m_x << ptMajorMatrixRoiCenter.m_y; //kircheis_Hobin
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(nMsg, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send Job to Host"));

    ::free(pByte);

    if (bSendItemList)
        iPIS_Send_InspectionItemsName(); //kircheis_NGRV
}

void VisionMainTR::iPIS_Send_PackageSpec()
{
    static const bool bIs380Bridge = SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE;
    static const bool bIsNGRV = SystemConfig::GetInstance().IsVisionTypeNGRV();
    const bool bIsCurrentJobFor2DMatrixByPass = GetPrimaryVisionUnit().Is2DMatrixReadingBypass();

    if (bIs380Bridge || bIsNGRV
        || bIsCurrentJobFor2DMatrixByPass) //NGRV Vision이거나 ByPass Mode Recipe이면 Package Spec을 보내면 안된다
        return;

    CiDataBase jobDB_PackSpec;

    if (!m_primaryVisionUnit->LinkDataBase_PackageSpec(true, jobDB_PackSpec))
    {
        return;
    }

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        jobDB_PackSpec.Serialize(ar);
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_SEND_PACKAGE_SPEC_TO_HOST, length, pByte);

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_InspResult(VisionUnit& visionUnit, VisionDeviceResult& CurResult, BOOL& bSendSuccess)
{
    DevelopmentLog::AddLog(
        DevelopmentLog::Type::TCP, _T("Send Insp Result : T%d, P%d"), CurResult.m_nTrayID, CurResult.m_nPocketID);

    if (m_pMessageSocket->IsConnected() == false)
        return;

    BYTE* pbySendBuffer = visionUnit.m_VisionReusableMemory->GetByteMemory();

    long maxSendResultSize = visionUnit.m_VisionReusableMemory->GetMaxBufferImageSizeX()
        * visionUnit.m_VisionReusableMemory->GetMaxBufferImageSizeY();
    try
    {
        CMemFile memFile(pbySendBuffer, maxSendResultSize, maxSendResultSize);

        ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);
        CurResult.Serialize(ar);

        ar.Close();

        memFile.SeekToBegin();
        long length = CAST_LONG(((CFile*)&memFile)->GetLength());
        BYTE* pByte = memFile.Detach();

        bSendSuccess = m_pMessageSocket->Write(MSG_PROBE_INSPECT_RESULT, length, pByte);

        SystemConfig::GetInstance().Save_VisionLog_InspectionTimeLog(visionUnit.m_threadIndex, CurResult.m_nTrayID,
            CurResult.m_nPocketID, CurResult.m_nPane, _T("Inspection Result Send"));

        if (pByte != pbySendBuffer)
        {
            ASSERT(!"A new memory allocation occurs.");
            ::free(pByte);
        }
    }
    catch (...)
    {
    }

    visionUnit.m_VisionReusableMemory->Release_ByteMemory(pbySendBuffer);
}

void VisionMainTR::iPIS_Send_InspResult(VisionUnit& visionUnit, SideVisionDeviceResult& CurResult, BOOL& bSendSuccess)
{
    DevelopmentLog::AddLog(
        DevelopmentLog::Type::TCP, _T("Send Insp Result : T%d, P%d"), CurResult.m_nTrayID, CurResult.m_nPocketID);

    if (m_pMessageSocket->IsConnected() == false)
        return;

    BYTE* pbySendBuffer = visionUnit.m_VisionReusableMemory->GetByteMemory();

    long maxSendResultSize = visionUnit.m_VisionReusableMemory->GetMaxBufferImageSizeX()
        * visionUnit.m_VisionReusableMemory->GetMaxBufferImageSizeY();
    try
    {
        CMemFile memFile(pbySendBuffer, maxSendResultSize, maxSendResultSize);

        ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);
        CurResult.Serialize(ar);

        ar.Close();

        memFile.SeekToBegin();
        long length = CAST_LONG(((CFile*)&memFile)->GetLength());
        BYTE* pByte = memFile.Detach();

        bSendSuccess = m_pMessageSocket->Write(MSG_PROBE_INSPECT_RESULT, length, pByte);

        SystemConfig::GetInstance().Save_VisionLog_InspectionTimeLog(visionUnit.m_threadIndex, CurResult.m_nTrayID,
            CurResult.m_nPocketID, CurResult.m_nPane, _T("Inspection Result Send"));

        if (pByte != pbySendBuffer)
        {
            ASSERT(!"A new memory allocation occurs.");
            ::free(pByte);
        }
    }
    catch (...)
    {
    }

    visionUnit.m_VisionReusableMemory->Release_ByteMemory(pbySendBuffer);
}

void VisionMainTR::iPIS_Send_DLInspResult(VisionDeviceResult& CurResult, BOOL& bSendSuccess)
{
    DevelopmentLog::AddLog(
        DevelopmentLog::Type::TCP, _T("Send DLInsp Result : T%d, P%d"), CurResult.m_nTrayID, CurResult.m_nPocketID);

    if (m_pMessageSocket->IsConnected() == false)
        return;

    try
    {
        CMemFile memFile(m_pbySendResultBuffer, MAX_SEND_RESULT_SIZE, MAX_SEND_RESULT_SIZE);

        ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);
        CurResult.Serialize(ar);

        ar.Close();

        memFile.SeekToBegin();
        long length = CAST_LONG(((CFile*)&memFile)->GetLength());
        BYTE* pByte = memFile.Detach();

        bSendSuccess = m_pMessageSocket->Write(MSG_PROBE_INSPECT_RESULT, length, pByte);

        if (pByte != m_pbySendResultBuffer)
        {
            ASSERT(!"A new memory allocation occurs.");
            ::free(pByte);
        }
    }
    catch (...)
    {
    }
}

void VisionMainTR::iPIS_Send_SystemParameters()
{
    static const bool bIs380Bridge = SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE;

    if (bIs380Bridge)
        return;

    VisionSystemParameters visionSystemParams = MakeVisionSystemParameters();

    //////////////////////////// ASNC Log Test Code ////////////////////////////
    /*CString strFileName;
	strFileName.Format(_T("C:\\Work\\VisionASNC.ini"));

	CString strVisionType, strFileNameLoad, strCategory, strKeyName, strGlobalParam, strValue2, strNum;
	strVisionType.Format(_T("%s"), SystemConfig().GetInstance().m_strVisionInfo);	

	FILE* pfile = fopen("C:\\Work\\VisionASNC.ini", "w");

	strFileNameLoad.Format(_T("Vision_%s_Parameter\nFileName,Category,KeyName,GlobalParam,Value\n"), strVisionType);

	fprintf(pfile, "%S", (LPCTSTR)strFileNameLoad);
	
	for (auto& sysParam : visionSystemParams.m_vecVisionSysParam)
	{
		strFileNameLoad.Format(_T("%s"), sysParam.m_strFileName);
		strCategory.Format(_T("%s"), sysParam.m_strCategory);
		strKeyName.Format(_T("%s"), sysParam.m_strKeyName);
		strGlobalParam.Format(_T("%d"), sysParam.m_bIsGlobalParam);
		strValue2.Format(_T("%s"), sysParam.m_strValue);

		fprintf(pfile, "%S,%S,%S,%S,%S\n", (LPCTSTR)strFileNameLoad, (LPCTSTR)strCategory, (LPCTSTR)strKeyName, (LPCTSTR)strGlobalParam, (LPCTSTR)strValue2);
	}

	fclose(pfile);*/
    /////////////////////////////////////////////////////////////////////////////

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << visionSystemParams;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_SEND_SYSTEM_PARAMETERS_TO_HOST, length, pByte); //kircheis_ASNC

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_SystemParametersMatchResult(VisionSystemParameters& receivedSysParam)
{
    static const bool bIs380Bridge = SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE;

    if (bIs380Bridge)
        return;

    VisionSystemParameters curSysParams = MakeVisionSystemParameters();
    VisionSystemParameters mismatchSysParams;

    for (auto& recSysParam : receivedSysParam.m_vecVisionSysParam)
    {
        if (!recSysParam.m_bIsGlobalParam)
            continue;

        BOOL bIsKeyNameMatch = FALSE;
        BOOL bIsKeyValueMatch = FALSE;
        for (auto& curSysParam : curSysParams.m_vecVisionSysParam)
        {
            if (!curSysParam.m_bIsGlobalParam)
                continue;

            if (recSysParam.m_strFileName == curSysParam.m_strFileName
                && recSysParam.m_strCategory == curSysParam.m_strCategory
                && recSysParam.m_strKeyName == curSysParam.m_strKeyName)
            {
                bIsKeyNameMatch = TRUE;
                if (recSysParam.m_strValue == curSysParam.m_strValue)
                    bIsKeyValueMatch = TRUE;
                break;
            }
        }
        if (bIsKeyNameMatch && !bIsKeyValueMatch)
        {
            mismatchSysParams.AddVisionSysParam(recSysParam);
        }
    }

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << mismatchSysParams;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_SEND_SYSTEM_PARAMETERS_MATCH_RESULT_TO_HOST, length, pByte); //kircheis_ASNC

    ::free(pByte);
}

VisionSystemParameters VisionMainTR::MakeVisionSystemParameters()
{
    VisionSystemParameters visionSysParams;
    auto& systemConfig = SystemConfig::GetInstance();
    if (systemConfig.GetHandlerType() < HANDLER_TYPE_500I)
        return visionSysParams;

    long nVisionType = systemConfig.GetVisionType();
    const static bool bIs2DVision = (nVisionType == VISIONTYPE_2D_INSP);
    const static bool bIs3DVision = (nVisionType == VISIONTYPE_3D_INSP);
    const static bool bIsNGRV = (nVisionType == VISIONTYPE_NGRV_INSP);
    const static bool bIsSideVision = (nVisionType == VISIONTYPE_SIDE_INSP);
    const static bool bIsTRVision = (nVisionType == VISIONTYPE_TR);

    if (bIs3DVision == true)
        return MakeVisionSystemParametersFor3D();

    if (bIsNGRV == true) //kircheis_820.190
        return MakeVisionSystemParametersForNGRV();

    // Vision Type이 2D, 3D, NGRV 모두 아닐 경우, Host에 경고 메세지 전달 및 출력
    if (bIs2DVision == false && bIs3DVision == false && bIsNGRV == false && bIsSideVision == false
        && bIsTRVision == false)
    {
        CString strError;
        strError.Format(_T("Unknown Vision Type."));
        iPIS_Send_ErrorMessageForPopup(strError);
    }

    return MakeVisionSystemParametersFor2DandSide();
}

VisionSystemParameters VisionMainTR::MakeVisionSystemParametersFor2DandSide()
{
    VisionSystemParameters visionSysParams;
    auto& systemConfig = SystemConfig::GetInstance();
    static const bool bIs2DVision = (systemConfig.GetVisionType() == VISIONTYPE_2D_INSP);
    static const bool bIsSideVision = (systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);
    static const bool bIsTRVision = (systemConfig.GetVisionType() == VISIONTYPE_TR);
    static const bool bIsValidVisionType = (bIs2DVision || bIsSideVision || bIsTRVision);
    if (systemConfig.GetHandlerType() < HANDLER_TYPE_500I || bIsValidVisionType == false)
        return visionSysParams;

    CString strFileName;
    CString strCategory;
    CString strKeyName;
    CString strKeyValue;

    visionSysParams.Add(_T("Software"), _T("Software"), _T("Vision version number"), true, m_strDetailFileVersion);

    strFileName = _T("System.ini");

    //strCategory : Hardware
    strCategory = _T("Hardware");
    visionSysParams.Add(strFileName, strCategory, _T("Hardware Exist"), true, (long)systemConfig.m_bHardwareExist);
    visionSysParams.Add(
        strFileName, strCategory, _T("Use2ndDLInspection"), false, (long)systemConfig.m_bUseAiInspection);
    visionSysParams.Add(
        strFileName, strCategory, _T("Wait DL Send Result"), true, (long)systemConfig.m_nDLWaitTimeSendRejectData);

    //strCategory : Setting
    strCategory = _T("Setting");
    visionSysParams.Add(strFileName, strCategory, _T("Handler_Type"), true, (long)systemConfig.m_nHandlerType);
    visionSysParams.Add(strFileName, strCategory, _T("Vision_Type"), true, (long)systemConfig.m_nVisionType);
    visionSysParams.Add(
        strFileName, strCategory, _T("StitchGrabDirection"), true, (long)systemConfig.m_nStitchGrabDirection);
    visionSysParams.Add(strFileName, strCategory, _T("Grab Verify Matching Count"), true,
        (long)systemConfig.m_nGrabVerifyMatchingCount); //3D Parameter 확인하자
    visionSysParams.Add(strFileName, strCategory, _T("Use Grab Retry"), true, (long)systemConfig.m_bUseGrabRetry);
    visionSysParams.Add(strFileName, strCategory, _T("GrabDuration"), true, (long)systemConfig.m_nGrabDuration);

    //strCategory : Vision_Setting
    strCategory = _T("Vision_Setting");
    visionSysParams.Add(
        strFileName, strCategory, _T("Send_Host_Frame_Num"), false, (long)systemConfig.m_nSendHostFrameNumber);
    visionSysParams.Add(
        strFileName, strCategory, _T("ReviewImageSaveDrive"), false, (long)systemConfig.m_nReviewImageSaveDrive);
    visionSysParams.Add(strFileName, strCategory, _T("Surface_Reject_Report_Patch_Count_X"), true,
        (long)systemConfig.m_nSurfaceRejectReportPatchCount_X);
    visionSysParams.Add(strFileName, strCategory, _T("Surface_Reject_Report_Patch_Count_Y"), true,
        (long)systemConfig.m_nSurfaceRejectReportPatchCount_Y);
    visionSysParams.Add(strFileName, strCategory, _T("ThreadNum"), true, (long)systemConfig.GetThreadNum());
    visionSysParams.Add(strFileName, strCategory, _T("GrabBufferNum"), true, (long)systemConfig.GetGrabBufferNum());

    //strCategory : Calibration
    strCategory = _T("Calibration");
    if (bIs2DVision)
    {
        visionSysParams.Add(strFileName, strCategory, _T("2D Scale X"), false,
            systemConfig.Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_FRONT));
        visionSysParams.Add(strFileName, strCategory, _T("2D Scale Y"), false,
            systemConfig.Get2DScaleY(enSideVisionModule::SIDE_VISIONMODULE_FRONT));
    }
    else if (bIsSideVision)
    {
        visionSysParams.Add(strFileName, strCategory, _T("Front 2D Scale X"), false,
            systemConfig.Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_FRONT));
        visionSysParams.Add(strFileName, strCategory, _T("Front 2D Scale Y"), false,
            systemConfig.Get2DScaleY(enSideVisionModule::SIDE_VISIONMODULE_FRONT));
        visionSysParams.Add(strFileName, strCategory, _T("Rear 2D Scale X"), false,
            systemConfig.Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_REAR));
        visionSysParams.Add(strFileName, strCategory, _T("Rear 2D Scale Y"), false,
            systemConfig.Get2DScaleY(enSideVisionModule::SIDE_VISIONMODULE_REAR));

        visionSysParams.Add(strFileName, strCategory, _T("Front Camera Offset Y"), false,
            (long)systemConfig.GetSideVisionFrontCameraOffsetY());
        visionSysParams.Add(strFileName, strCategory, _T("Rear Camera Offset Y"), false,
            (long)systemConfig.GetSideVisionRearCameraOffsetY());
    }
    else if (bIsTRVision)
    {
        visionSysParams.Add(strFileName, strCategory, _T("2D Scale X"), false,
            systemConfig.Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_FRONT));
        visionSysParams.Add(strFileName, strCategory, _T("2D Scale Y"), false,
            systemConfig.Get2DScaleY(enSideVisionModule::SIDE_VISIONMODULE_FRONT));
    }

    CString strIllumCalType = GetIllumCalTypeString(systemConfig.m_nIlluminationCalType);
    visionSysParams.Add(strFileName, strCategory, _T("Illumination Cal.Type"), true, strIllumCalType);
    visionSysParams.Add(
        strFileName, strCategory, _T("Lock Illum Cal. to Linear+"), true, (long)systemConfig.m_bLockIllumCalLinearPlus);

    if (bIs2DVision == TRUE)
    {
        AddVisionSystemParametersFor2DIllum(TRUE, visionSysParams);
        AddVisionSystemParametersFor2DIllum(FALSE, visionSysParams);
    }
    else if (bIsSideVision == TRUE)
    {
        AddVisionSystemParametersForSideIllum(TRUE, visionSysParams);
        AddVisionSystemParametersForSideIllum(FALSE, visionSysParams);
    }
    else if (bIsTRVision == TRUE)
    {
        AddVisionSystemParametersFor2DIllum(TRUE, visionSysParams);
        AddVisionSystemParametersFor2DIllum(FALSE, visionSysParams);
    }

    visionSysParams.Add(strFileName, _T("SpecialOption"), _T("UseMarkTeachImageSplitSave"), true,
        (long)systemConfig.m_bUseMarkTeachImageSplitSave);

    return visionSysParams;
}

void VisionMainTR::AddVisionSystemParametersFor2DIllum(const BOOL bIsRef, VisionSystemParameters& o_visionParams)
{
    auto& systemConfig = SystemConfig::GetInstance();
    static const bool bIs2DVision
        = (systemConfig.GetVisionType() == VISIONTYPE_2D_INSP || systemConfig.GetVisionType() == VISIONTYPE_TR);
    if (systemConfig.GetHandlerType() < HANDLER_TYPE_500I || bIs2DVision == false)
        return;

    //Ref or Current
    auto& vecfCoeffA = bIsRef ? systemConfig.m_vecfCoeffA_Ref : systemConfig.m_vecfCoeffA_Cur;
    auto& vecfCoeffB = bIsRef ? systemConfig.m_vecfCoeffB_Ref : systemConfig.m_vecfCoeffB_Cur;
    auto& vecfCoeffA_Mirror = bIsRef ? systemConfig.m_vecfCoeffA_RefMirror : systemConfig.m_vecfCoeffA_CurMirror;
    auto& vecfCoeffB_Mirror = bIsRef ? systemConfig.m_vecfCoeffB_RefMirror : systemConfig.m_vecfCoeffB_CurMirror;

    const long nCoeffA_Num = (long)vecfCoeffA.size();
    const long nCoeffB_Num = (long)vecfCoeffB.size();
    const long nCoeffAmirror_Num = (long)vecfCoeffA_Mirror.size();
    const long nCoeffBmirror_Num = (long)vecfCoeffB_Mirror.size();

    if (nCoeffA_Num < LED_ILLUM_CHANNEL_DEFAULT)
        return;

    if (nCoeffA_Num != nCoeffB_Num || nCoeffAmirror_Num != nCoeffBmirror_Num)
        return;

    //조명 구성 및 조명 보정 상태에 대한 정보 수집
    static const long nNormalChannelNum = LED_ILLUM_CHANNEL_DEFAULT;
    const BOOL bExistRingIllum = (systemConfig.GetExistRingillumination() && nCoeffA_Num == LED_ILLUM_CHANNEL_MAX);
    const BOOL bIsCalibratedCoaxMirror = (nCoeffAmirror_Num > 0);

    //기본 키 값 File Name & Category
    static const CString strFileName = bIsRef == TRUE ? _T("IlluminationTableRef.csv") : _T("IlluminationTable.csv");
    static const CString strCategory = bIsRef == TRUE ? _T("Illum.Calibration_Ref") : _T("Illum.Calibration_Target");
    static const CString strFileNameMirror
        = bIsRef == TRUE ? _T("IlluminationTableRefMirror.csv") : _T("IlluminationTableMirror.csv");
    static const CString strCategoryMirror
        = bIsRef == TRUE ? _T("Illum.Calibration_RefMirror") : _T("Illum.Calibration_TargetMirror");
    CString strKeyName;
    CString strKeyValue;

    //일반 조명 Ch01 - Ch 13
    for (long nCh = 0; nCh < LED_ILLUM_CHANNEL_DEFAULT; nCh++)
    {
        strKeyName.Format(_T("Ch%02d_CoeffA"), nCh + 1);
        o_visionParams.Add(strFileName, strCategory, strKeyName, false, vecfCoeffA[nCh]);
        strKeyName.Format(_T("Ch%02d_CoeffB"), nCh + 1);
        o_visionParams.Add(strFileName, strCategory, strKeyName, false, vecfCoeffB[nCh]);
    }

    //동축 Mirror Ch12, 13
    if (bIsCalibratedCoaxMirror == TRUE)
    {
        for (long nCh = LED_ILLUM_CHANNEL_OBLIQUE; nCh < LED_ILLUM_CHANNEL_DEFAULT; nCh++)
        {
            strKeyName.Format(_T("Ch%02d_MirrorCoeffA"), nCh + 1);
            o_visionParams.Add(strFileName, strCategory, strKeyName, false, vecfCoeffA_Mirror[nCh]);
            strKeyName.Format(_T("Ch%02d_MirrorCoeffB"), nCh + 1);
            o_visionParams.Add(strFileName, strCategory, strKeyName, false, vecfCoeffB_Mirror[nCh]);
        }
    }

    //Ring 조명 Ch13 - Ch 16
    if (bExistRingIllum == TRUE)
    {
        for (long nCh = LED_ILLUM_CHANNEL_DEFAULT; nCh < LED_ILLUM_CHANNEL_MAX; nCh++)
        {
            strKeyName.Format(_T("Ch%02d_RingCoeffA"), nCh + 1);
            o_visionParams.Add(strFileName, strCategory, strKeyName, false, vecfCoeffA[nCh]);
            strKeyName.Format(_T("Ch%02d_RingCoeffB"), nCh + 1);
            o_visionParams.Add(strFileName, strCategory, strKeyName, false, vecfCoeffB[nCh]);
        }
    }
}

void VisionMainTR::AddVisionSystemParametersForSideIllum(const BOOL bIsRef, VisionSystemParameters& o_visionParams)
{
    auto& systemConfig = SystemConfig::GetInstance();
    static const bool bIsSideVision = (systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);
    if (systemConfig.GetHandlerType() < HANDLER_TYPE_500I || bIsSideVision == false)
        return;

    const long nLoopCnt = bIsRef == TRUE ? 1 : SIDE_VISIONMODULE_END;

    //Ref or Current & File Name & Category
    std::vector<std::vector<float>> vec2fCoeffA(nLoopCnt);
    std::vector<std::vector<float>> vec2fCoeffB(nLoopCnt);
    std::vector<std::vector<float>> vec2fCoeffA_Mirror(nLoopCnt);
    std::vector<std::vector<float>> vec2fCoeffB_Mirror(nLoopCnt);

    std::vector<CString> vecStrFileName(nLoopCnt);
    std::vector<CString> vecStrCategory(nLoopCnt);
    std::vector<CString> vecStrFileNameMirror(nLoopCnt);
    std::vector<CString> vecStrCategoryMirror(nLoopCnt);

    if (bIsRef == TRUE)
    {
        vec2fCoeffA[0].insert(
            vec2fCoeffA[0].end(), systemConfig.m_vecfCoeffA_Ref.begin(), systemConfig.m_vecfCoeffA_Ref.end());
        vec2fCoeffB[0].insert(
            vec2fCoeffB[0].end(), systemConfig.m_vecfCoeffB_Ref.begin(), systemConfig.m_vecfCoeffB_Ref.end());

        vec2fCoeffA_Mirror[0].insert(vec2fCoeffA_Mirror[0].end(), systemConfig.m_vecfCoeffA_RefMirror.begin(),
            systemConfig.m_vecfCoeffA_RefMirror.end());
        vec2fCoeffB_Mirror[0].insert(vec2fCoeffB_Mirror[0].end(), systemConfig.m_vecfCoeffB_RefMirror.begin(),
            systemConfig.m_vecfCoeffB_RefMirror.end());

        vecStrFileName[0] = _T("IlluminationTableRef.csv");
        vecStrCategory[0] = _T("Illum.Calibration_Ref");
        vecStrFileNameMirror[0] = _T("IlluminationTableRefMirror.csv");
        vecStrCategoryMirror[0] = _T("Illum.Calibration_RefMirror");
    }
    else
    {
        vec2fCoeffA[0].insert(
            vec2fCoeffA[0].end(), systemConfig.m_vecfCoeffA_Cur.begin(), systemConfig.m_vecfCoeffA_Cur.end());
        vec2fCoeffB[0].insert(
            vec2fCoeffB[0].end(), systemConfig.m_vecfCoeffB_Cur.begin(), systemConfig.m_vecfCoeffB_Cur.end());

        vec2fCoeffA_Mirror[0].insert(vec2fCoeffA_Mirror[0].end(), systemConfig.m_vecfCoeffA_CurMirror.begin(),
            systemConfig.m_vecfCoeffA_CurMirror.end());
        vec2fCoeffB_Mirror[0].insert(vec2fCoeffB_Mirror[0].end(), systemConfig.m_vecfCoeffB_CurMirror.begin(),
            systemConfig.m_vecfCoeffB_CurMirror.end());

        vecStrFileName[0] = _T("IlluminationTable.csv");
        vecStrCategory[0] = _T("Illum.Calibration_Target");
        vecStrFileNameMirror[0] = _T("IlluminationTableRefMirror.csv");
        vecStrCategoryMirror[0] = _T("Illum.Calibration_RefMirror");

        vec2fCoeffA[1].insert(vec2fCoeffA[1].end(), systemConfig.m_vecfCoeffA_Cur_SideRear.begin(),
            systemConfig.m_vecfCoeffA_Cur_SideRear.end());
        vec2fCoeffB[1].insert(vec2fCoeffB[1].end(), systemConfig.m_vecfCoeffB_Cur_SideRear.begin(),
            systemConfig.m_vecfCoeffB_Cur_SideRear.end());

        vec2fCoeffA_Mirror[1].insert(vec2fCoeffA_Mirror[1].end(), systemConfig.m_vecfCoeffA_CurMirror_SideRear.begin(),
            systemConfig.m_vecfCoeffA_CurMirror_SideRear.end());
        vec2fCoeffB_Mirror[1].insert(vec2fCoeffB_Mirror[1].end(), systemConfig.m_vecfCoeffB_CurMirror_SideRear.begin(),
            systemConfig.m_vecfCoeffB_CurMirror_SideRear.end());

        vecStrFileName[1] = _T("IlluminationTable_Rear.csv");
        vecStrCategory[1] = _T("Illum.Calibration_TargetRear");
        vecStrFileNameMirror[1] = _T("IlluminationTableRearMirror.csv");
        vecStrCategoryMirror[1] = _T("Illum.Calibration_TargetRearMirror");
    }

    //기본 키 값
    CString strKeyName;
    CString strKeyValue;

    //Front / Rear Loop (Ref는 Loop 타면 안되거나 한번만 탄다)
    for (long nLoop = 0; nLoop < nLoopCnt; nLoop++)
    {
        //일반 조명 Ch01 - Ch 10
        if (vec2fCoeffA[nLoop].size() != LED_ILLUM_CHANNEL_SIDE_DEFAULT
            || vec2fCoeffB[nLoop].size() != LED_ILLUM_CHANNEL_SIDE_DEFAULT)
            continue;

        for (long nCh = 0; nCh < LED_ILLUM_CHANNEL_SIDE_DEFAULT; nCh++)
        {
            strKeyName.Format(_T("Ch%02d_CoeffA"), nCh + 1);
            o_visionParams.Add(
                vecStrFileName[nLoop], vecStrCategory[nLoop], strKeyName, false, vec2fCoeffA[nLoop][nCh]);
            strKeyName.Format(_T("Ch%02d_CoeffB"), nCh + 1);
            o_visionParams.Add(
                vecStrFileName[nLoop], vecStrCategory[nLoop], strKeyName, false, vec2fCoeffB[nLoop][nCh]);
        }

        //동축 Mirror Ch10
        if (vec2fCoeffA_Mirror[nLoop].size() != LED_ILLUM_CHANNEL_SIDE_DEFAULT
            || vec2fCoeffB_Mirror[nLoop].size() != LED_ILLUM_CHANNEL_SIDE_DEFAULT)
            continue;

        for (long nCh = LED_ILLUM_CHANNEL_SIDE_OBLIQUE; nCh < LED_ILLUM_CHANNEL_SIDE_DEFAULT; nCh++)
        {
            strKeyName.Format(_T("Ch%02d_MirrorCoeffA"), nCh + 1);
            o_visionParams.Add(
                vecStrFileName[nLoop], vecStrCategory[nLoop], strKeyName, false, vec2fCoeffA_Mirror[nLoop][nCh]);
            strKeyName.Format(_T("Ch%02d_MirrorCoeffB"), nCh + 1);
            o_visionParams.Add(
                vecStrFileName[nLoop], vecStrCategory[nLoop], strKeyName, false, vec2fCoeffB_Mirror[nLoop][nCh]);
        }
    }
}

VisionSystemParameters VisionMainTR::MakeVisionSystemParametersFor3D()
{
    VisionSystemParameters visionSysParams;
    auto& systemConfig = SystemConfig::GetInstance();
    if (systemConfig.GetHandlerType() < HANDLER_TYPE_500I || systemConfig.GetVisionType() != VISIONTYPE_3D_INSP)
        return visionSysParams;

    CString strFileName;
    CString strCategory;
    CString strKeyName;
    CString strKeyValue;

    visionSysParams.Add(_T("Software"), _T("Software"), _T("Vision version number"), true, m_strDetailFileVersion);

    strFileName = _T("System.ini");

    //strCategory : Hardware
    strCategory = _T("Hardware");
    visionSysParams.Add(strFileName, strCategory, _T("Hardware Exist"), true, (long)systemConfig.m_bHardwareExist);
    visionSysParams.Add(
        strFileName, strCategory, _T("Use2ndDLInspection"), false, (long)systemConfig.m_bUseAiInspection);
    visionSysParams.Add(
        strFileName, strCategory, _T("Wait DL Send Result"), true, (long)systemConfig.m_nDLWaitTimeSendRejectData);

    visionSysParams.Add(
        strFileName, strCategory, _T("SlitbeamCameraOffsetY"), false, (long)systemConfig.m_slitbeamCameraOffsetY);
    visionSysParams.Add(strFileName, strCategory, _T("SlitbeamUseEncoderMultiply"), true,
        (long)systemConfig.m_slitbeamUseEncoderMultiply);

    visionSysParams.Add(strFileName, strCategory, _T("3D Camera Serial Number"), false, systemConfig.m_str3DCameraSN);
    visionSysParams.Add(strFileName, strCategory, _T("3D Camera Analog Offset1 Origin"), false,
        (long)systemConfig.m_n3DCameraAnalogOffset1Origin);
    visionSysParams.Add(
        strFileName, strCategory, _T("3D Camera Analog Offset1"), false, (long)systemConfig.m_n3DCameraAnalogOffset1);
    visionSysParams.Add(strFileName, strCategory, _T("3D Camera Analog Offset2 Origin"), false,
        (long)systemConfig.m_n3DCameraAnalogOffset2Origin);
    visionSysParams.Add(
        strFileName, strCategory, _T("3D Camera Analog Offset2"), false, (long)systemConfig.m_n3DCameraAnalogOffset2);

    //strCategory : Setting
    strCategory = _T("Setting");
    visionSysParams.Add(strFileName, strCategory, _T("Handler_Type"), true, (long)systemConfig.m_nHandlerType);
    visionSysParams.Add(strFileName, strCategory, _T("Vision_Type"), true, (long)systemConfig.m_nVisionType);
    visionSysParams.Add(
        strFileName, strCategory, _T("StitchGrabDirection"), true, (long)systemConfig.m_nStitchGrabDirection);
    visionSysParams.Add(strFileName, strCategory, _T("Grab Verify Matching Count"), true,
        (long)systemConfig.m_nGrabVerifyMatchingCount);
    visionSysParams.Add(strFileName, strCategory, _T("Use Grab Retry"), true, (long)systemConfig.m_bUseGrabRetry);
    //visionSysParams.Add(strFileName, strCategory, _T("Vision_AccessMode"),						false, (long)systemConfig.m_nCurrentAccessMode);

    visionSysParams.Add(strFileName, strCategory, _T("m_slitbeamIlluminationAngle_deg"), false,
        systemConfig.m_slitbeamIlluminationAngle_deg);
    visionSysParams.Add(
        strFileName, strCategory, _T("m_slitbeamImage_px2um_y"), true, systemConfig.m_slitbeamImage_px2um_y);
    visionSysParams.Add(
        strFileName, strCategory, _T("m_slitbeamCameraAngle_deg"), false, systemConfig.m_slitbeamCameraAngle_deg);
    visionSysParams.Add(
        strFileName, strCategory, _T("m_slitbeamHeightScaling"), false, systemConfig.m_slitbeamHeightScaling);
    visionSysParams.Add(
        strFileName, strCategory, _T("m_slitbeamScanSpeedDownRatio"), true, systemConfig.m_slitbeamScanSpeedDownRatio);
    visionSysParams.Add(
        strFileName, strCategory, _T("m_slitbeamIlluminationGain"), true, systemConfig.m_slitbeamIlluminationGain);

    //strCategory : Vision_Setting
    strCategory = _T("Vision_Setting");
    visionSysParams.Add(
        strFileName, strCategory, _T("Send_Host_Frame_Num"), false, (long)systemConfig.m_nSendHostFrameNumber);
    visionSysParams.Add(
        strFileName, strCategory, _T("ReviewImageSaveDrive"), false, (long)systemConfig.m_nReviewImageSaveDrive);
    visionSysParams.Add(strFileName, strCategory, _T("Surface_Reject_Report_Patch_Count_X"), true,
        (long)systemConfig.m_nSurfaceRejectReportPatchCount_X);
    visionSysParams.Add(strFileName, strCategory, _T("Surface_Reject_Report_Patch_Count_Y"), true,
        (long)systemConfig.m_nSurfaceRejectReportPatchCount_Y);
    visionSysParams.Add(strFileName, strCategory, _T("ThreadNum"), true, (long)systemConfig.GetThreadNum());
    visionSysParams.Add(strFileName, strCategory, _T("GrabBufferNum"), true, (long)systemConfig.GetGrabBufferNum());

    //strCategory : Calibration
    strCategory = _T("Calibration");

    visionSysParams.Add(strFileName, strCategory, _T("3D Scale X"), false,
        systemConfig.Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_FRONT));
    visionSysParams.Add(strFileName, strCategory, _T("3D Scale Y"), false,
        systemConfig.Get2DScaleY(enSideVisionModule::SIDE_VISIONMODULE_FRONT));

    // [2019/03/28] EC Setup Data에 3D Scale 대신 SlitbeamHeightScale 값을 넣어보내주기로 했다
    visionSysParams.Add(strFileName, strCategory, _T("3D Scale"), false, systemConfig.m_slitbeamHeightScaling);
    visionSysParams.Add(
        strFileName, strCategory, _T("Default 3D Scale X"), true, (long)(systemConfig.GetDefaultScaleX_3DVision()));

    strCategory = _T("SyncPort");
    visionSysParams.Add(strFileName, strCategory, _T("SyncPort"), false, (long)systemConfig.m_nSyncPort);

    return visionSysParams;
}

VisionSystemParameters VisionMainTR::MakeVisionSystemParametersForNGRV() //kircheis_820.190
{
    VisionSystemParameters visionSysParams;
    auto& systemConfig = SystemConfig::GetInstance();
    if (systemConfig.GetHandlerType() < HANDLER_TYPE_500I || systemConfig.IsVisionTypeNGRV() != 1) //kircheis_820.190
        return visionSysParams;

    //kircheis_MED5_5 //ASNC Data 정리 및 튜닝 관련 파리미터 외 모두 Global화. NGRV 관련 파라미터 추가 필요
    CString strFileName;
    CString strCategory;
    CString strKeyName;
    CString strKeyValue;
    //long nVisionType = systemConfig.GetVisionType();
    //bool bIs2DVision = (nVisionType == VISIONTYPE_2D_INSP || nVisionType == VISIONTYPE_NGRV_INSP);
    //bool bIsNGRV = (nVisionType == VISIONTYPE_NGRV_INSP);

    visionSysParams.Add(_T("Software"), _T("Software"), _T("Vision version number"), false, m_strDetailFileVersion);

    strFileName = _T("System.ini");

    //strCategory : Hardware
    strCategory = _T("Hardware");
    visionSysParams.Add(strFileName, strCategory, _T("Hardware Exist"), false, (long)systemConfig.m_bHardwareExist);
    visionSysParams.Add(
        strFileName, strCategory, _T("Use2ndDLInspection"), false, (long)systemConfig.m_bUseAiInspection);
    visionSysParams.Add(
        strFileName, strCategory, _T("Wait DL Send Result"), false, (long)systemConfig.m_nDLWaitTimeSendRejectData);

    //strCategory : Setting
    strCategory = _T("Setting");
    visionSysParams.Add(strFileName, strCategory, _T("Handler_Type"), false, (long)systemConfig.m_nHandlerType);
    visionSysParams.Add(strFileName, strCategory, _T("Vision_Type"), false, (long)systemConfig.m_nVisionType);
    visionSysParams.Add(
        strFileName, strCategory, _T("StitchGrabDirection"), false, (long)systemConfig.m_nStitchGrabDirection);
    visionSysParams.Add(strFileName, strCategory, _T("Grab Verify Matching Count"), false,
        (long)systemConfig.m_nGrabVerifyMatchingCount);
    visionSysParams.Add(strFileName, strCategory, _T("Use Grab Retry"), false, (long)systemConfig.m_bUseGrabRetry);
    visionSysParams.Add(strFileName, strCategory, _T("GrabDuration"), false, (long)systemConfig.m_nGrabDuration);

    //strCategory : Vision_Setting
    strCategory = _T("Vision_Setting");
    visionSysParams.Add(
        strFileName, strCategory, _T("Send_Host_Frame_Num"), false, (long)systemConfig.m_nSendHostFrameNumber);
    visionSysParams.Add(
        strFileName, strCategory, _T("ReviewImageSaveDrive"), false, (long)systemConfig.m_nReviewImageSaveDrive);
    visionSysParams.Add(strFileName, strCategory, _T("Surface_Reject_Report_Patch_Count_X"), false,
        (long)systemConfig.m_nSurfaceRejectReportPatchCount_X);
    visionSysParams.Add(strFileName, strCategory, _T("Surface_Reject_Report_Patch_Count_Y"), false,
        (long)systemConfig.m_nSurfaceRejectReportPatchCount_Y);
    visionSysParams.Add(strFileName, strCategory, _T("ThreadNum"), false, (long)systemConfig.GetThreadNum());
    visionSysParams.Add(strFileName, strCategory, _T("GrabBufferNum"), false, (long)systemConfig.GetGrabBufferNum());

    //strCategory : Calibration
    strCategory = _T("Calibration");
    visionSysParams.Add(strFileName, strCategory, _T("2D Scale X"), false,
        systemConfig.Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_FRONT));
    visionSysParams.Add(strFileName, strCategory, _T("2D Scale Y"), false,
        systemConfig.Get2DScaleY(enSideVisionModule::SIDE_VISIONMODULE_FRONT));

    CString strIllumCalType = GetIllumCalTypeString(systemConfig.m_nIlluminationCalType);
    visionSysParams.Add(strFileName, strCategory, _T("Illumination Cal.Type"), false, strIllumCalType);
    visionSysParams.Add(strFileName, strCategory, _T("Lock Illum Cal. to Linear+"), false,
        (long)systemConfig.m_bLockIllumCalLinearPlus);

    strCategory = _T("SpecialOption");
    visionSysParams.Add(strFileName, strCategory, _T("UseMarkTeachImageSplitSave"), false,
        (long)systemConfig.m_bUseMarkTeachImageSplitSave);

    strCategory = _T("NGRV Parameters");
    visionSysParams.Add(
        strFileName, strCategory, _T("Use Bayer Pattern GPU"), false, (long)systemConfig.m_bUseBayerPatternGPU);
    visionSysParams.Add(
        strFileName, strCategory, _T("Image Save Option for NGRV"), false, (long)systemConfig.m_nSaveImageTypeForNGRV);

    visionSysParams.Add(
        strFileName, strCategory, _T("Normal Channel Red Gain"), false, systemConfig.m_fNormal_Channel_GainR);
    visionSysParams.Add(
        strFileName, strCategory, _T("Normal Channel Green Gain"), false, systemConfig.m_fNormal_Channel_GainG);
    visionSysParams.Add(
        strFileName, strCategory, _T("Normal Channel Blue Gain"), false, systemConfig.m_fNormal_Channel_GainB);

    visionSysParams.Add(
        strFileName, strCategory, _T("Reverse Channel Red Gain"), false, systemConfig.m_fReverse_Channel_GainR);
    visionSysParams.Add(
        strFileName, strCategory, _T("Reverse Channel Green Gain"), false, systemConfig.m_fReverse_Channel_GainG);
    visionSysParams.Add(
        strFileName, strCategory, _T("Reverse Channel Blue Gain"), false, systemConfig.m_fReverse_Channel_GainB);

    visionSysParams.Add(
        strFileName, strCategory, _T("Use long exposure for NGRV"), false, (long)systemConfig.m_bUseLongExposureNGRV);
    visionSysParams.Add(
        strFileName, strCategory, _T("NGRV IR Channel ID(0~15)"), false, (long)systemConfig.m_nNgrvIRchID);
    visionSysParams.Add(
        strFileName, strCategory, _T("NGRV UV Channel ID(0~15)"), false, (long)systemConfig.m_nNgrvUVchID);

    visionSysParams.Add(
        strFileName, strCategory, _T("NGRV Default Channel Gain"), false, systemConfig.m_fDefault_Camera_Gain);
    visionSysParams.Add(strFileName, strCategory, _T("NGRV IR Channel Gain"), false, systemConfig.m_fIR_Camera_Gain);
    visionSysParams.Add(strFileName, strCategory, _T("NGRV UV Channel Gain"), false, systemConfig.m_fUV_Camera_Gain);

    visionSysParams.Add(strFileName, strCategory, _T("NGRV ByPass IR"), false, (long)systemConfig.m_bIsByPassIRBIT);

    CString errorDesc;

    long nChannelNum = (long)systemConfig.m_vecfCoeffA_Ref.size(); //kircheis_IllumCalBug
    //long nMirrorChannelNum = (long)systemConfig.m_vecfCoeffA_RefMirror.size();
    if (nChannelNum > 0) //kircheis_IllumCalBug
    {
        strFileName = _T("IlluminationTableRef.csv");
        strCategory = _T("Illum.Calibration_Ref");
        for (long channel = 0; channel < nChannelNum; channel++)
        {
            if (channel == (long)systemConfig.m_nNgrvUVchID)
            {
                strKeyName.Format(_T("Ch%02d_UV"), channel + 1);
            }
            else if (channel == (long)systemConfig.m_nNgrvIRchID)
            {
                strKeyName.Format(_T("Ch%02d_IR"), channel + 1);
            }
            else
            {
                strKeyName.Format(_T("Ch%02d"), channel + 1);
            }

            float fValue;
            CString strNotExist;

            fValue = (channel < nChannelNum) ? systemConfig.m_vecfCoeffA_Ref[channel] : 0.f;
            strNotExist.Format(_T("Not Exist"));

            if (channel == (long)systemConfig.m_nNgrvUVchID || channel == (long)systemConfig.m_nNgrvIRchID
                || channel == 9 || channel == 10 || channel == 12)
            {
                visionSysParams.Add(strFileName, strCategory, strKeyName, false, strNotExist);
            }
            else
            {
                visionSysParams.Add(strFileName, strCategory, strKeyName, false, fValue);
            }
        }
    }
    else // illum Line Fitting이 실패한 경우, 모든 값을 INVALID 처리한다....
    {
        // 실패한 경우 채널 개수를 MAX로 할당
        nChannelNum = LED_ILLUM_CHANNEL_DEFAULT;

        strFileName = _T("IlluminationTableRef.csv");
        strCategory = _T("Illum.Calibration_Ref");
        for (long channel = 0; channel < nChannelNum; channel++)
        {
            if (channel == (long)systemConfig.m_nNgrvUVchID)
            {
                strKeyName.Format(_T("Ch%02d_UV"), channel + 1);
            }
            else if (channel == (long)systemConfig.m_nNgrvIRchID)
            {
                strKeyName.Format(_T("Ch%02d_IR"), channel + 1);
            }
            else
            {
                strKeyName.Format(_T("Ch%02d"), channel + 1);
            }

            CString strNotExist, strInvalid;

            strNotExist.Format(_T("Not Exist"));
            strInvalid.Format(_T("Invalid"));

            if (channel == (long)systemConfig.m_nNgrvUVchID || channel == (long)systemConfig.m_nNgrvIRchID
                || channel == 9 || channel == 10 || channel == 12)
            {
                visionSysParams.Add(strFileName, strCategory, strKeyName, false, strNotExist);
            }
            else
            {
                visionSysParams.Add(strFileName, strCategory, strKeyName, false, strInvalid);
            }
        }
    }

    nChannelNum = (long)systemConfig.m_vecfCoeffA_Cur.size(); //kircheis_IllumCalBug

    if (nChannelNum > 0) //kircheis_IllumCalBug
    {
        strFileName = _T("IlluminationTable.csv");
        strCategory = _T("Illum.Calibration_Target");
        for (long channel = 0; channel < nChannelNum; channel++)
        {
            if (channel == (long)systemConfig.m_nNgrvUVchID)
            {
                strKeyName.Format(_T("Ch%02d_UV"), channel + 1);
            }
            else if (channel == (long)systemConfig.m_nNgrvIRchID)
            {
                strKeyName.Format(_T("Ch%02d_IR"), channel + 1);
            }
            else
            {
                strKeyName.Format(_T("Ch%02d"), channel + 1);
            }

            float fValue;
            CString strNotExist;

            fValue = (channel < nChannelNum) ? systemConfig.m_vecfCoeffA_Cur[channel] : 0.f;
            strNotExist.Format(_T("Not Exist"));

            if (channel == (long)systemConfig.m_nNgrvUVchID || channel == (long)systemConfig.m_nNgrvIRchID
                || channel == 9 || channel == 10 || channel == 12)
            {
                visionSysParams.Add(strFileName, strCategory, strKeyName, false, strNotExist);
            }
            else
            {
                visionSysParams.Add(strFileName, strCategory, strKeyName, false, fValue);
            }
        }
    }
    else // illum Line Fitting이 실패한 경우, 모든 값을 INVALID 처리한다....
    {
        // 실패한 경우 채널 개수를 MAX로 할당
        nChannelNum = LED_ILLUM_CHANNEL_DEFAULT;

        strFileName = _T("IlluminationTable.csv");
        strCategory = _T("Illum.Calibration_Target");
        for (long channel = 0; channel < nChannelNum; channel++)
        {
            if (channel == (long)systemConfig.m_nNgrvUVchID)
            {
                strKeyName.Format(_T("Ch%02d_UV"), channel + 1);
            }
            else if (channel == (long)systemConfig.m_nNgrvIRchID)
            {
                strKeyName.Format(_T("Ch%02d_IR"), channel + 1);
            }
            else
            {
                strKeyName.Format(_T("Ch%02d"), channel + 1);
            }

            CString strNotExist, strInvalid;

            strNotExist.Format(_T("Not Exist"));
            strInvalid.Format(_T("Invalid"));

            if (channel == (long)systemConfig.m_nNgrvUVchID || channel == (long)systemConfig.m_nNgrvIRchID
                || channel == 9 || channel == 10 || channel == 12)
            {
                visionSysParams.Add(strFileName, strCategory, strKeyName, false, strNotExist);
            }
            else
            {
                visionSysParams.Add(strFileName, strCategory, strKeyName, false, strInvalid);
            }
        }
    }

    return visionSysParams;
}

CString VisionMainTR::GetIllumCalTypeString(long nIllumCalType)
{
    CString strIllumCalType;
    strIllumCalType.Empty();
    switch (nIllumCalType)
    {
        case IllumCalType_Gain:
            strIllumCalType = _T("Gain");
            break;
        case IllumCalType_OnlyLine:
            strIllumCalType = _T("Linear");
            break;
        case IllumCalType_Curve_Line:
            strIllumCalType = _T("Curve + Linear");
            break;
        case IllumCalType_Linear_Plus:
            strIllumCalType = _T("Linear+");
            break;
        default:
            strIllumCalType = _T("Setting is wrong");
            break;
    }

    return strIllumCalType;
}

long VisionMainTR::GetIllumCalTypeLong(CString strIllumCalType)
{
    long nIllumCalType = IllumCalType_Linear_Plus;

    if (strIllumCalType == _T("Gain"))
        nIllumCalType = IllumCalType_Gain;
    else if (strIllumCalType == _T("Linear"))
        nIllumCalType = IllumCalType_OnlyLine;
    else if (strIllumCalType == _T("Curve + Linear"))
        nIllumCalType = IllumCalType_Curve_Line;
    else if (strIllumCalType == _T("Linear+"))
        nIllumCalType = IllumCalType_Linear_Plus;

    return nIllumCalType;
}

void VisionMainTR::SetVisionSystemParameters(VisionSystemParameters visionSysParams)
{
    auto& systemConfig = SystemConfig::GetInstance();
    if (systemConfig.GetHandlerType() < HANDLER_TYPE_500I)
        return;

    CString strFileNameSystemConfig = _T("System.ini");
    CString strCategoryVision = _T("Vision_Setting");
    CString strCategoryHardware = _T("Hardware");
    CString strCategorySetting = _T("Setting");
    CString strCategoryCalibration = _T("Calibration");
    BOOL bChanged = FALSE;

    for (auto& sysParam : visionSysParams.m_vecVisionSysParam)
    {
        if (!sysParam.m_bIsGlobalParam)
            continue;

        auto value_int = _ttoi(sysParam.m_strValue);
        auto value_bool = ((long)_ttoi(sysParam.m_strValue)) == 1 ? TRUE : FALSE;

        if (sysParam.m_strFileName == strFileNameSystemConfig) //System.ini
        {
            if (sysParam.m_strCategory == strCategoryVision)
            {
                if (sysParam.m_strKeyName == _T("ThreadNum"))
                {
                    if (systemConfig.GetThreadNum() != value_int)
                    {
                        systemConfig.SetThreadNum(value_int);
                        bChanged = TRUE;
                    }
                }
                else if (sysParam.m_strKeyName == _T("GrabBufferNum"))
                {
                    if (systemConfig.GetGrabBufferNum(false) != value_int)
                    {
                        systemConfig.SetGrabBufferNum(value_int);
                        bChanged = TRUE;
                    }
                }
                continue;
            }
            else if (sysParam.m_strCategory == strCategoryHardware)
            {
                if (sysParam.m_strKeyName == _T("Hardware Exist"))
                {
                    if (systemConfig.m_bHardwareExist != value_bool)
                    {
                        systemConfig.m_bHardwareExist = value_bool;
                        bChanged = TRUE;
                    }
                }
                else if (sysParam.m_strKeyName == _T("SlitbeamUseEncoderMultiply"))
                {
                    if (systemConfig.m_slitbeamUseEncoderMultiply != value_bool)
                    {
                        systemConfig.m_slitbeamUseEncoderMultiply = value_bool;
                        bChanged = TRUE;
                    }
                }
                continue;
            }
            else if (sysParam.m_strCategory == strCategorySetting)
            {
                if (sysParam.m_strKeyName == _T("Handler_Type"))
                {
                    if (systemConfig.m_nHandlerType != value_int)
                    {
                        systemConfig.m_nHandlerType = value_int;
                        bChanged = TRUE;
                    }
                }
                else if (sysParam.m_strKeyName == _T("Vision_Type"))
                {
                    if (systemConfig.m_nVisionType != value_int)
                    {
                        systemConfig.m_nVisionType = value_int;
                        bChanged = TRUE;
                    }
                }
                else if (sysParam.m_strKeyName == _T("StitchGrabDirection"))
                {
                    if (systemConfig.m_nStitchGrabDirection != value_int)
                    {
                        systemConfig.m_nStitchGrabDirection = value_int;
                        bChanged = TRUE;
                    }
                }
                else if (sysParam.m_strKeyName == _T("GrabDuration")
                    && (systemConfig.m_nVisionType == VISIONTYPE_2D_INSP
                        || systemConfig.m_nVisionType == VISIONTYPE_NGRV_INSP
                        || systemConfig.m_nVisionType == VISIONTYPE_TR))
                {
                    if (systemConfig.m_nGrabDuration != value_int)
                    {
                        systemConfig.m_nGrabDuration = value_int;
                        bChanged = TRUE;
                    }
                }
                continue;
            }
            //else if (sysParam.m_strCategory == strCategoryCalibration)
            //{
            //	if(sysParam.m_strKeyName == _T("Default 3D Scale X"))
            //	{
            //		if (systemConfig.GetDefaultScaleX_3DVision() != value_int)
            //		{
            //			systemConfig.SetDefaultScaleX_3DVision(value_int);
            //			bChanged = TRUE;
            //		}
            //	}
            //	continue;
            //}
        }
    }

    if (bChanged)
    {
        SystemConfig::GetInstance().SaveIni(DynamicSystemPath::get(DefineFile::System));
        SystemConfig::GetInstance().SaveCurrentScaleXY();
    }

    iPIS_Send_SystemParametersChanged(bChanged);
}

void VisionMainTR::iPIS_Send_SystemParametersChanged(BOOL bChanged)
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    DEFMSGPARAM turnData = {
        bChanged ? 1 : 0,
    };

    m_pMessageSocket->Write(MSG_PROBE_SEND_SYSTEM_PARAMETERS_CHANGED, sizeof(turnData), (BYTE*)&turnData);
}

void VisionMainTR::iPIS_ChangeDisplay(short nTarget)
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    DEFMSGPARAM turnData = {
        nTarget,
    };

    m_pMessageSocket->Write(MSG_PROBE_CHANGE_SCREEN, sizeof(turnData), (BYTE*)&turnData);
}

void VisionMainTR::iPIS_Send_MultiGrabProbePosZ(long nProbePosZ)
{
    static const bool bIS380Bridge = (SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE);
    static const bool bIs3DVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP);
    if (bIS380Bridge || bIs3DVision)
        return;

    if (m_pMessageSocket->IsConnected() == false)
        return;

    DEFMSGPARAM turnData = {
        nProbePosZ,
    };

    m_pMessageSocket->Write(MSG_PROBE_MOVE_TO_GRAB_POS_Z, sizeof(turnData), (BYTE*)&turnData);
}

// 핸들러가 알아낼수 없는 잡 정보를 보내 준다.
void VisionMainTR::iPis_Send_JobInfo()
{
    auto& primaryUnit = GetPrimaryVisionUnit();
    auto visionInspectionSpecs = primaryUnit.GetVisionInspectionSpecs();

    static const bool bIs380Bridge = SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE;

    long nSize = (long)(visionInspectionSpecs.size());
    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << nSize;
        SetSpecNormalValue(visionInspectionSpecs);

        for (int n = 0; n < nSize; n++)
        {
            ar << *(visionInspectionSpecs[n]);
        }

        // 영훈 20150903 : Host에 Ball이나 Lead, Pad의 개수를 함게 보내도록 한다.
        CPackageSpec* pPackageSpec = primaryUnit.m_pPackageSpec;

        long nBallNumber = (long)pPackageSpec->m_ballMap->m_balls.size();
        long nLandNumber = (long)pPackageSpec->m_LandMapManager->GetCount();
        long nPassiveNumber = (long)pPackageSpec->m_OriginCompData->vecOriginCompData.size();

        ar << nBallNumber;
        ar << nLandNumber;
        ar << nPassiveNumber;

        ar << pPackageSpec->m_bodyInfoMaster->fBodySizeX;
        ar << pPackageSpec->m_bodyInfoMaster->fBodySizeY;
        ar << pPackageSpec->m_bodyInfoMaster->fBodyThickness;

        ar << primaryUnit.IsNeedMarkTeach();

        if (!bIs380Bridge)
        {
            ar << (long)(m_illum2D.needPositionMovement() ? 1 : 0);
        }

        auto& systemConfig = SystemConfig::GetInstance();
        static const long nVisionType = systemConfig.GetVisionType();
        long nErrorCode(RECIPE_VALID), nFrameNum(0);
        float fScaleZ = (nVisionType == VISIONTYPE_3D_INSP) ? systemConfig.GetScale3D() : 1.f;
        CString strErrorContents, strPinIndexDirection;
        std::vector<CString> vecstrIllumExposureTimeMS(0);

        //{{//kircheis_VerifyRecipe
        GetErrorInfo(nErrorCode, strErrorContents);
        ar << nErrorCode << strErrorContents;
        //}}

        ar << systemConfig.Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
           << systemConfig.Get2DScaleY(enSideVisionModule::SIDE_VISIONMODULE_FRONT) << fScaleZ;
        //{{//kircheis_SideVisionScale
        static const BOOL bIsSideVision = systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP;
        float fRearScaleX = 1.f;
        float fRearScaleY = 1.f;
        float fRearScaleZ = 1.f;
        if (bIsSideVision)
        {
            fRearScaleX = systemConfig.Get2DScaleX(SIDE_VISIONMODULE_REAR);
            fRearScaleY = systemConfig.Get2DScaleY(SIDE_VISIONMODULE_REAR);
            ar << fRearScaleX << fRearScaleY << fRearScaleZ;
        }
        //}}
        strPinIndexDirection = pPackageSpec->GetPinIndexDirection(); //kircheis_PinIdxDir
        ar << strPinIndexDirection; //kircheis_PinIdxDir

        //{{//kircheis_IllumData
        GetIllumInfo(vecstrIllumExposureTimeMS);
        nFrameNum = (long)vecstrIllumExposureTimeMS.size();
        ar << nFrameNum;
        for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
            ar << vecstrIllumExposureTimeMS[nFrame];
        //}}
        long nImageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
        long nImageSizeY = FrameGrabber::GetInstance().get_grab_image_height();

        ar << nImageSizeX << nImageSizeY;

        Ipvm::Point32s2 ptMajorMatrixRoiCenter(0, 0);
        BOOL bValidMajorMatrixROICenter = FALSE;
        if (GetPrimaryVisionUnit().Is2DMatrixReadingBypass() == false)
            bValidMajorMatrixROICenter = primaryUnit.GetMajorRoiCenter(ptMajorMatrixRoiCenter);

        ar << bValidMajorMatrixROICenter << ptMajorMatrixRoiCenter.m_x << ptMajorMatrixRoiCenter.m_y; //kircheis_Hobin
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    long nMSG = MSG_PROBE_UPDATE_JOB;

    if (SystemConfig::GetInstance().IsBtm2DVision() == TRUE && GetPrimaryVisionUnit().Is2DMatrixReadingBypass() == TRUE)
        nMSG = MSG_PROBE_UPDATE_JOB_FOR_BYPASS;

    m_pMessageSocket->Write(nMSG, length, pByte);

    ::free(pByte);
}

BOOL VisionMainTR::GetErrorInfo(
    long& o_nErrorCode, CString& o_srtErrorContent) //kircheis_MED3_SendInfo //Error 없으면 이 함수는 return false
{
    auto& systemConfig = SystemConfig::GetInstance();
    static const long nVisionType = systemConfig.GetVisionType();
    auto& primaryUnit = GetPrimaryVisionUnit();
    CPackageSpec* pPackageSpec = primaryUnit.m_pPackageSpec;

    o_nErrorCode = RECIPE_VALID;
    o_srtErrorContent.Format(_T("None"));

    if (nVisionType == VISIONTYPE_3D_INSP)
    {
        long nTotalCompNum(0), nErrorCompNum(0);
        if (pPackageSpec->m_OriginCompData->CheckIntegrityOfCompMapData(nErrorCompNum, nTotalCompNum) == false)
        {
            o_nErrorCode = RECIPE_ERROR_COMP_MAP;
            o_srtErrorContent.Format(_T("Component Map Data have a problem"));
        }
    }

    return (o_nErrorCode != RECIPE_VALID);
}

void VisionMainTR::GetIllumInfo(std::vector<CString>& vecstrIllumExposureTimeMS) //kircheis_MED3_SendInfo
{
    auto& systemConfig = SystemConfig::GetInstance();
    static const long nVisionType = systemConfig.GetVisionType();

    long nFrameNum = 0;
    vecstrIllumExposureTimeMS.clear();

    if (nVisionType == VISIONTYPE_3D_INSP)
        return;

    nFrameNum = m_illum2D.getTotalFrameCount();

    vecstrIllumExposureTimeMS.resize(nFrameNum);
    CString strIllumExposureTimeMS;
    long nIllumChannelMax = 16;
    long nIllumChannel = 0;
    for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
    {
        auto illumData = m_illum2D.getIllum(nFrame);
        if (illumData.size() < 1)
            continue;
        vecstrIllumExposureTimeMS[nFrame].Format(_T("%.3f"), illumData[0]);
        nIllumChannel = (long)min(nIllumChannelMax, illumData.size());
        for (long nCh = 1; nCh < nIllumChannel; nCh++)
        {
            strIllumExposureTimeMS.Format(_T(",%.3f"), illumData[nCh]);
            vecstrIllumExposureTimeMS[nFrame] += strIllumExposureTimeMS;
        }
    }
}

void VisionMainTR::iPIS_Send_Start_Handler(long nMessage)
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    m_pMessageSocket->Write(nMessage, NULL, NULL);
}

void VisionMainTR::SendToHost_MarkImage(std::vector<CString>& vecstrMarkImagePath)
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    long nSize = (long)vecstrMarkImagePath.size();
    long nLength;
    CFile fImageFile;
    BYTE* pbyData;

    // 전송될 버퍼 생성
    BYTE* pbyBuffer = (BYTE*)malloc(100000000);

    CMemFile memFile(pbyBuffer, 100000000, 10000000);
    CArchive ar((CFile*)&memFile, CArchive::store, 100000000, pbyBuffer);

    for (long n = 0; n < vecstrMarkImagePath.size(); n++)
    {
        // File 열어서 파일의 유효함을 확인 한다.
        if (!fImageFile.Open(vecstrMarkImagePath[n], CFile::modeRead))
        {
            vecstrMarkImagePath.erase(vecstrMarkImagePath.begin() + n); //유효하지 않은 파일이면 리스트에서 지운다.
            n--;
        }
        else
        {
            fImageFile.Close();
        }
    }

    nSize = (long)vecstrMarkImagePath.size();

    ar << nSize; // 이미지의 개수를 먼저 보낸다.

    for (long n = 0; n < nSize; n++)
    {
        // File 열어서 이진화 데이터에 쓰기
        if (!fImageFile.Open(vecstrMarkImagePath[n], CFile::modeRead))
        {
            continue;
        }

        nLength = CAST_LONG(fImageFile.GetLength());
        pbyData = new BYTE[nLength];

        fImageFile.SeekToBegin();
        fImageFile.Read(pbyData, nLength);
        fImageFile.Close();
        //////// 쓰기 완료

        // 전송될 Data Archive에 작성하기
        ar << nLength; // 현재 보내는 이미지의 크기를 보낸다.
            //	ar.Write(pbyData, _msize(pbyData));
        ar.Write(pbyData, nLength); //kircheis_20150505

        delete[] pbyData;
    }

    ar.Close();
    nLength = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();
    /// 작성 완료

    // 전송
    m_pMessageSocket->Write(MSG_PROBE_MARK_IMAGE, nLength, pByte);

    ::free(pByte);
}

void VisionMainTR::iPIS_HandlerStop()
{
    m_pMessageSocket->Write(MSG_PROBE_HANDLER_STOP, 0, NULL);
}

void VisionMainTR::iPIS_Send_ErrorMessageForPopup(CString message)
{
    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << message;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_VISION_ERROR_POPUP, length, pByte); //kircheis_ASNC

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_ErrorMessageForLog(CString category, CString message)
{
    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << category << message;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_VISION_ERROR_LOG, length, pByte); //kircheis_ASNC

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_JobdownloadAck(BOOL done, CString message)
{
    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << done;
        ar << message;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_JOB_DOWNLOAD_ACK, length, pByte); //kircheis_ASNC

    ::free(pByte);
}

//kk Grab Retry : Host에게 Retry하라고 전송!
void VisionMainTR::iPIS_Send_GrabRetry()
{
    m_pMessageSocket->Write(MSG_PROBE_3D_GRAB_RETRY, NULL, NULL);
}

void VisionMainTR::iPIS_Send_GrabMove(long nGrabDir, long nMessage, float scanLength3D_mm, bool isGoldenDevice)
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (nMessage == MSG_PROBE_GRAB_MOVE || nMessage == MSG_PROBE_3DVISION_GRAB_MOVE)
    {
        long nScanLength3D = -1;
        if (scanLength3D_mm >= 0)
            nScanLength3D = long(scanLength3D_mm * 1000.f); // um 단위로 전송

        DEFMSGPARAM _turnData = {nGrabDir, nScanLength3D, long(isGoldenDevice), 0, 0, 0, 0, 0};
        m_pMessageSocket->Write(nMessage, sizeof(_turnData), (BYTE*)&_turnData);
    }
}

void VisionMainTR::iPIS_MarkTeach_Skip()
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    m_pMessageSocket->Write(MSG_MARKTEACH_SKIP, NULL, NULL);
}

void VisionMainTR::iPIS_MarkTeach_Done()
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    m_pMessageSocket->Write(MSG_MARKTEACH_DONE, NULL, NULL);
    m_bMarkTeach = FALSE;
}

void VisionMainTR::SetSpecNormalValue(std::vector<VisionInspectionSpec*>& specList)
{
    for (int n = 0; n < long(specList.size()); n++)
    {
        auto* spec = specList[n];
        if (spec->m_moduleGuid == _VISION_INSP_GUID_PACKAGE_SIZE) // 3D inspection
        {
            spec->m_vecfNominal = GetPackageSizeSpec(spec->m_specName);
        }
        else if (spec->m_moduleGuid == _VISION_INSP_GUID_BGA_BALL_3D) // 3D inspection
        {
            spec->m_vecfNominal = Get3DInspectionSpec(spec->m_specName);
        }
        else if (spec->m_moduleGuid == _VISION_INSP_GUID_BGA_BALL_2D) // BGA inspection
        {
            spec->m_vecfNominal = GetBallInspectionSpec(spec->m_specName);
        }
        else if (spec->m_moduleGuid
            == _VISION_INSP_GUID_BGA_BALL_PQ) // BGA Ball Pixel Quality inspection //kircheis_BPQ
        {
            spec->m_vecfNominal = GetBallPixelQualityInspectionSpec(spec->m_specName);
        }
        else if (spec->m_inspName.Find(_T("Mark_")) > -1)
        {
            spec->m_vecfNominal = GetMarkInspectionSpec(spec->m_specName);
        }
        else if (spec->m_unit == 'S') // Surface
        {
            GetSurfaceCriteriaSpec(spec->m_inspName, spec->m_vecSurfaceCriteriaSpec);
        }
    }
}

std::vector<float> VisionMainTR::GetPackageSizeSpec(CString strSpecName)
{
    auto& primaryUnit = GetPrimaryVisionUnit();

    std::vector<float> vecfNominal;

    const auto& packageSpec = *primaryUnit.m_pPackageSpec;

    if (strSpecName == g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_BODYSIZE_X])
    {
        vecfNominal.clear();
        vecfNominal.push_back(packageSpec.m_bodyInfoMaster->fBodySizeX);
    }
    else if (strSpecName == g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_BODYSIZE_Y])
    {
        vecfNominal.clear();
        vecfNominal.push_back(packageSpec.m_bodyInfoMaster->fBodySizeY);
    }
    else if (strSpecName == g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_PARALLELISM]
        || strSpecName == g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_ORTHOGONALITY])
    {
        vecfNominal.clear();
        vecfNominal.push_back(0.f);
    }

    return vecfNominal;
}

std::vector<float> VisionMainTR::Get3DInspectionSpec(CString strSpecName)
{
    auto& primaryUnit = GetPrimaryVisionUnit();
    std::vector<float> vecfNominal;
    float fSpec = 0.f;

    const auto& packageSpec = *primaryUnit.m_pPackageSpec;

    if (strSpecName == g_szBGA3DInspectionName[_3DINSP_HEIGHT])
    {
        if (primaryUnit.m_pPackageSpec->nDeviceType == enDeviceType::PACKAGE_BALL)
        {
            if (packageSpec.m_originalballMap->m_ballTypes.size())
            {
                fSpec = CAST_FLOAT(packageSpec.m_originalballMap->m_ballTypes.front().m_height_um);
            }
        }
    }
    else if (strSpecName == g_szBGA3DInspectionName[_3DINSP_COPL]
        || strSpecName == g_szBGA3DInspectionName[_3DINSP_UNIT_COPL])
    {
    }
    else if (strSpecName == g_szBGA3DInspectionName[_3DINSP_WARPAGE]
        || strSpecName == g_szBGA3DInspectionName[_3DINSP_UNIT_WARPAGE])
    {
    }
    else
    {
        fSpec = -100000000.f;
    }

    vecfNominal.push_back(fSpec);

    return vecfNominal;
}

std::vector<float> VisionMainTR::GetMarkInspectionSpec(CString strSepcName)
{
    std::vector<float> vecfNominal;
    float fSpec = 10000000.f;

    if (strSepcName == g_szMarkInspectionName[MARK_INSPECTION_OVER_PRINT]
        || strSepcName == g_szMarkInspectionName[MARK_INSPECTION_UNDER_PRINT])
    {
        fSpec = 100.f;
    }
    else if (strSepcName == g_szMarkInspectionName[MARK_INSPECTION_BLOB_SIZE])
    {
        fSpec = 0.f;
    }
    else
    {
        fSpec = -100000000.f;
    }
    vecfNominal.push_back(fSpec);

    return vecfNominal;
}

std::vector<float> VisionMainTR::GetBallInspectionSpec(CString strSpecName)
{
    auto& primaryUnit = GetPrimaryVisionUnit();

    std::vector<float> vecfNominal;
    float fSpec = 0.f;

    const auto& packageSpec = *primaryUnit.m_pPackageSpec;

    if (strSpecName == g_szBallInspectionName[BALL_INSPECTION_BALL_WIDTH])
    {
        if (packageSpec.m_originalballMap->m_ballTypes.size())
        {
            fSpec = CAST_FLOAT(packageSpec.m_originalballMap->m_ballTypes.front().m_diameter_um);
        }
        else
        {
            fSpec = 0.f;
        }
    }
    else if (strSpecName == g_szBallInspectionName[BALL_INSPECTION_BALL_QUALITY])
    {
        fSpec = 100.f;
    }
    else
    {
        fSpec = -100000000.f;
    }

    long nSize = (long)vecfNominal.size();
    BOOL bCheckValue = FALSE;
    for (long n = 0; n < nSize; n++)
    {
        if (vecfNominal[0] == fSpec)
        {
            bCheckValue = TRUE;
        }
    }

    if (bCheckValue == FALSE)
    {
        vecfNominal.push_back(fSpec);
    }

    return vecfNominal;
}

std::vector<float> VisionMainTR::GetBallPixelQualityInspectionSpec(CString strSpecName) //kircheis_BPQ
{
    std::vector<float> vecfNominal;
    float fSpec = 0.f;

    for (long nID = INSPECT_BALL_PIXEL_QUALITY_ITEM_BEGIN_FILLRATIO;
        nID < INSPECT_BALL_PIXEL_QUALITY_ITEM_END_FILLRATIO; nID++)
    {
        if (strSpecName == gl_szStrInspectPixelQualityItem[nID])
        {
            fSpec = 100.f;
            vecfNominal.push_back(fSpec);
            return vecfNominal;
        }
    }
    for (long nID = INSPECT_BALL_PIXEL_QUALITY_ITEM_BEGIN_CONTRAST; nID < INSPECT_BALL_PIXEL_QUALITY_ITEM_END_CONTRAST;
        nID++)
    {
        if (strSpecName == gl_szStrInspectPixelQualityItem[nID])
        {
            fSpec = 0.f;
            vecfNominal.push_back(fSpec);
            return vecfNominal;
        }
    }

    fSpec = -100000000.f;
    vecfNominal.push_back(fSpec);

    return vecfNominal;
}

void VisionMainTR::GetSurfaceCriteriaSpec(
    CString strSurfaceName, std::vector<std::vector<SurfaceSpec>>& vecsSurfaceCriteria)
{
    auto& primaryUnit = GetPrimaryVisionUnit();
    std::vector<CSurfaceCriteria> vecCriteria;

    vecsSurfaceCriteria.clear();

    SurfaceSpec sSurfaceCriteria;

    long nSize = (long)primaryUnit.m_visionInspectionSurfaces.size();
    //	vecsSurfaceCriteria.resize(nSize);//kircheis_20150504

    for (long nID = 0; nID < nSize; nID++)
    {
        if (primaryUnit.m_visionInspectionSurfaces[nID]->m_strModuleName == strSurfaceName)
        {
            vecCriteria = primaryUnit.m_visionInspectionSurfaces[nID]->m_surfacePara->m_SurfaceItem.vecCriteria;
            long nCiteriaSize = (long)vecCriteria.size();
            vecsSurfaceCriteria.resize(nCiteriaSize); //kircheis_20150504

            for (long n = 0; n < nCiteriaSize; n++)
            {
                if (vecCriteria[n].bInsp)
                {
                    for (long item = 0; item < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); item++)
                    {
                        auto columnID = VisionSurfaceCriteria_Column(item);

                        if (vecCriteria[n].GetSpecMin(columnID).GetPassMax()
                            || vecCriteria[n].GetSpecMax(columnID).GetPassMin())
                        {
                            CString unit;
                            switch (GetCriteriaColumnSpecType(columnID))
                            {
                                case SItemSpec::enumSpecType::NUMBER:
                                    unit = _T("GV");
                                    break;
                                case SItemSpec::enumSpecType::MEASURE:
                                    unit = _T("um");
                                    break;
                                case SItemSpec::enumSpecType::MEASURE2:
                                    unit = _T("um^2");
                                    break;
                                case SItemSpec::enumSpecType::PERCENT:
                                    unit = _T("%");
                                    break;
                            }

                            if (columnID == VisionSurfaceCriteria_Column::Defect_num)
                            {
                                unit = _T("num");
                            }

                            vecsSurfaceCriteria[n].push_back(GetCiriteriaSpec(columnID, vecCriteria[n].m_color,
                                vecCriteria[n].GetSpecMin(columnID), vecCriteria[n].GetSpecMax(columnID), unit));
                        }
                    }
                }
            }
        }
    }
}

SurfaceSpec VisionMainTR::GetCiriteriaSpec(
    VisionSurfaceCriteria_Column columnID, long nColor, SItemSpec iTemSpecMin, SItemSpec iTemSpecMax, CString strUnit)
{
    SurfaceSpec sSurfaceCriteria;
    float fTemp = 0.f;

    sSurfaceCriteria.nType = long(columnID);
    sSurfaceCriteria.strName = GetCriteriaColumnName(columnID);
    sSurfaceCriteria.nColor = nColor;

    fTemp = iTemSpecMax.GetPassMin();
    sSurfaceCriteria.fMin = fTemp == 0.f ? -10000000.f : fTemp;
    fTemp = iTemSpecMin.GetPassMax();
    sSurfaceCriteria.fMax = fTemp == 0.f ? -10000000.f : fTemp;

    sSurfaceCriteria.strUnit = strUnit;

    return sSurfaceCriteria;
}

bool VisionMainTR::IsInlineMode() const
{
    return m_inline_grab->IsInlineMode();
}

//{{ //kircheis_NGRV_MSG
void VisionMainTR::
    iPIS_Send_InspectionItemsName() //일반 비전용. vector CString 전송. 각 Vision이 검사하는 검사 Module 이름을 벡터에 담아 송부.Recipe Upload시 송부
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == TRUE)
        return;

    auto& primaryUnit = GetPrimaryVisionUnit();
    auto visionInspectionSpecs = primaryUnit.GetVisionInspectionSpecs();
    long nSize = (long)primaryUnit.m_vecVisionModules.size();

    std::vector<CString> vecstrInspModuleName(0);

    for (long i = 0; i < nSize; i++)
    {
        CString strInspModule = primaryUnit.m_vecVisionModules[i]->m_strModuleName;
        auto guidModule = primaryUnit.m_vecVisionModules[i]->m_moduleGuid;
        if (guidModule == _VISION_INSP_GUID_PACKAGE_SPEC || guidModule == _VISION_INSP_GUID_FOV
            || guidModule == _VISION_INSP_GUID_ALIGN_2D || guidModule == _VISION_INSP_GUID_ALIGN_3D
            || guidModule == _VISION_INSP_GUID_FIDUCIAL_ALIGN || guidModule == _VISION_INSP_GUID_PAD_ALIGN_2D
            || guidModule == _VISION_INSP_GUID_PACKAGE_SIZE || guidModule == _VISION_INSP_GUID_LID_2D
            || guidModule == _VISION_INSP_GUID_LID_3D || guidModule == _VISION_INSP_GUID_PATCH_2D
            || guidModule == _VISION_INSP_GUID_PATCH_3D || guidModule == _VISION_INSP_GUID_DIE_2D
            || guidModule == _VISION_INSP_GUID_OTHER_ALIGN || guidModule == _VISION_INSP_GUID_GLOBAL_MASK_MAKER)
            continue;

        if (strInspModule.Find(_T("MSK_")) == 0)
            continue;

        if (strInspModule.Find(_T("Geo_")) == 0)
            continue;

        vecstrInspModuleName.push_back(strInspModule);
    }

    nSize = (long)(vecstrInspModuleName.size());
    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << nSize;
        for (long i = 0; i < nSize; i++)
            ar << vecstrInspModuleName[i];
        //}}
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_VISION_INSPECTION_ITEMS, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send Inspection module name to Host"));

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_WarningBTM2DRecipe() //NGRV Lot Start가 수신되었을 때, BTM2D가 NGRV Recipe가 아님을 알림
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == TRUE)
        return;

    DEFMSGPARAM turnData = {
        0,
    };

    m_pMessageSocket->Write(MSG_PROBE_WARNING_BTM2D_RECIPE_FOR_BYPASS, sizeof(turnData), (BYTE*)&turnData);
}

void VisionMainTR::
    iPIS_Send_NGRV_WarningVisionName() //NGRV Vision 전용. 검사 모듈 이름에 Vision Name이 누락된 경우 발생// 누락된 Vision 이름이 뭔지 알 면 보내겠지만 몰라서 못 보냄
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    DEFMSGPARAM turnData = {
        0,
    };

    m_pMessageSocket->Write(MSG_PROBE_NGRV_WARNING_VISION_NAME, sizeof(turnData), (BYTE*)&turnData);
}

void VisionMainTR::
    iPIS_Send_NGRV_WarningRecipeType() //NGRV Vision 전용. 검사 Download 받은 Recipe가 NGRV 전용 Recipe가 아닐 경우 발생
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    DEFMSGPARAM turnData = {
        0,
    };

    m_pMessageSocket->Write(MSG_PROBE_NGRV_WARNING_RECIPE_TYPE, sizeof(turnData), (BYTE*)&turnData);
}

void VisionMainTR::iPIS_Send_NGRV_GrabZposInfo(std::vector<NgrvGrabPositionInfo>
        vecGrabPosInfo) //NGRV Vision 전용. vector Class NgrvGrabPositionInfo. IR 조명 사용에 따른 Grab Height 관련 정보를 전달.  NGRV의 Recipe Upload 시 송부
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    long nSize = (long)vecGrabPosInfo.size();
    if (nSize < 1)
        return;

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << nSize;
        for (long i = 0; i < nSize; i++)
            ar << vecGrabPosInfo[i];
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_NGRV_GRAB_Z_POSITION_INFO, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send Grab Position Info"));

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_NGRV_CompleteImageSave(NgrvGrabbedImageInfo
        grabbedImgFileInfo) //NGRV Vision 전용. Class NgrvGrabbedImageInfo. 하나의 Defect에 대한 영상 저장이 완료되었음을 알림
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    long nSize = (long)grabbedImgFileInfo.m_vecstrImageFileName.size();
    if (nSize < 1)
        return;

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << grabbedImgFileInfo;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_NGRV_COMPLETE_SAVE_IMG, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send Image File Name"));

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_NGRV_GrabFrameInfo(NgrvFrameInfo
        ngrvGrabFrameInfo) //NGRV Vision 전용. Class NgrvFrameInfo. 각 Frame에서 획득하는 검사 모듈에 대한 정보를 전송..Recipe Upload시 송부
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << ngrvGrabFrameInfo;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_NGRV_GRAB_FRAME_INFO, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send NGRV Frame Info"));

    ::free(pByte);
}
//}}

//GrabInfo Transfer Done : Host에게 완료되었다고 전송! - JHB_NGRV
void VisionMainTR::iPIS_Send_PackageGrabInfo_Receive_Done()
{
    m_pMessageSocket->Write(MSG_PROBE_NGRV_PACKAGE_INFO_IN_TRAY_ACK, 0, NULL);
}

void VisionMainTR::iPIS_Send_NGRV_AF_Info() //kircheis_NGRVAF //2D Vision만 사용. NGRV AF Setup 정보를 송부
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionType2D() == false)
        return;

    auto& primaryUnit = GetPrimaryVisionUnit();

    BOOL bIsValidPlaneRefInfo(FALSE);
    std::vector<Ipvm::Point32s2> vecptRefPos_UM(0);
    long nImageSizeX(0);
    long nImageSizeY(0);
    std::vector<BYTE> vecbyImage(0);
    BOOL bSuccess
        = primaryUnit.GetNGRVAFInfo(bIsValidPlaneRefInfo, vecptRefPos_UM, nImageSizeX, nImageSizeY, vecbyImage);
    if (bSuccess == FALSE)
        return;

    long nPointNum = CAST_LONG(vecptRefPos_UM.size());
    NgrvAfRefInfo ngrvAFInfo;
    ngrvAFInfo.Init();

    ngrvAFInfo.m_nVisionID = SystemConfig::GetInstance().m_nVisionInfo_NumType;
    ngrvAFInfo.m_bIsValidPlaneRefInfo = bIsValidPlaneRefInfo;
    ngrvAFInfo.m_vecptRefPos_UM.resize(nPointNum);
    for (long idx = 0; idx < nPointNum; idx++)
        ngrvAFInfo.m_vecptRefPos_UM[idx] = CPoint(vecptRefPos_UM[idx].m_x, vecptRefPos_UM[idx].m_y);
    ngrvAFInfo.SetImageSize(nImageSizeX, nImageSizeY);
    long nImageBufSize = nImageSizeX * nImageSizeY * 3 * sizeof(BYTE);
    memcpy(&ngrvAFInfo.m_vecbyImage[0], &vecbyImage[0], nImageBufSize);

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << ngrvAFInfo;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_AF_PLANE_REF_INFO, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send NGRV AF Ref Info"));

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_NGRV_SingleRunInfo() // NGRV SingleRunInfo를 Host로 송부한다 - JHB_MED#6
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    auto& primaryUnit = GetPrimaryVisionUnit();

    BOOL isManualRun(FALSE);

    std::vector<NgrvSingleRunInfo> ngrvSingleRunInfo;
    ngrvSingleRunInfo.resize(2);
    for (long index = 0; index < (long)ngrvSingleRunInfo.size(); index++)
    {
        ngrvSingleRunInfo[index].Init();
    }

    long size = (long)ngrvSingleRunInfo.size();

    // 2D BTM/TOP
    ngrvSingleRunInfo[VISION_BTM_2D] = primaryUnit.GetNGRVSingleRunInfo(VISION_BTM_2D);
    ngrvSingleRunInfo[VISION_TOP_2D] = primaryUnit.GetNGRVSingleRunInfo(VISION_TOP_2D);

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << size;
        ar << isManualRun;
        ar << ngrvSingleRunInfo[VISION_TOP_2D];
        ar << ngrvSingleRunInfo[VISION_BTM_2D];
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_NGRV_SINGLE_RUN_INFO, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send 2D Image Info to NGRV"));

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_2D_Vision_Image_Info_To_NGRV() // NGRV에 보낼 2D Vision 정보를 Host로 송부한다 - JHB_MED#6
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionType2D() == false)
        return;

    auto& primaryUnit = GetPrimaryVisionUnit();

    std::vector<BYTE> vecbyImage(0);

    // 이미지 정보
    Ngrv2DImageInfo ngrv2DImageInfo;
    ngrv2DImageInfo.Init();

    // 2D BTM/TOP
    ngrv2DImageInfo.m_visionID = SystemConfig::GetInstance().m_nVisionInfo_NumType;

    // Image Size X/Y 및 Image transfer
    auto& illum = primaryUnit.getIllumInfo2D();
    auto& imageLot = primaryUnit.getImageLot();

    // Image Total Size
    long totalImageSize(0);

    if (illum.m_2D_colorFrame == FALSE)
    {
        auto alignProc = primaryUnit.GetVisionAlignProcessing();
        Ipvm::Image8u image = alignProc->GetInspectionFrameImage(FALSE, 0);

        long imageSizeX = image.GetSizeX();
        long imageSizeY = image.GetSizeY();

        long nImageNum = (long)alignProc->GetImageFrameCount();
        if (nImageNum < 1)
            return;

        //Ipvm::ImageProcessing::Copy(imageLot.GetImageFrame(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT), Ipvm::Rect32s(image), image);

        // Recipe 용량 최소화를 위한 Image Resize
        long resizeImageSizeX(0), resizeImageSizeY(0);
        resizeImageSizeX = ((image.GetSizeX() / 4 + 3) / 4) * 4;
        resizeImageSizeY = ((image.GetSizeY() / 4 + 3) / 4) * 4;

        Ipvm::Image8u resizeImage(resizeImageSizeX, resizeImageSizeY);
        Ipvm::ImageProcessing::ResizeLinearInterpolation(image, resizeImage);
        ///////////////////////////////////////////

        ngrv2DImageInfo.m_imageSizeX = resizeImageSizeX;
        ngrv2DImageInfo.m_imageSizeY = resizeImageSizeY;

        static const long sizeOfByte = sizeof(BYTE);
        totalImageSize = resizeImageSizeX * resizeImageSizeY * sizeOfByte;

        ngrv2DImageInfo.m_vecbyImage.resize(totalImageSize);

        BYTE* imageMem = &ngrv2DImageInfo.m_vecbyImage[0];
        BYTE* imageCopy = (BYTE*)resizeImage.GetMem();

        memset(imageMem, 0, totalImageSize);
        memcpy(imageMem, imageCopy, totalImageSize);
    }
    else
    {
        long frameIDs[3];
        frameIDs[2] = illum.m_2D_colorFrameIndex_red;
        frameIDs[1] = illum.m_2D_colorFrameIndex_green;
        frameIDs[0] = illum.m_2D_colorFrameIndex_blue;

        long nImageNum = (long)imageLot.GetImageFrameCount();
        if (nImageNum < 1)
            return;

        Ipvm::Image8u3 imageColor(imageLot.GetImageSizeX(), imageLot.GetImageSizeY());

        for (long i = 0; i <= 2; i++)
            frameIDs[i] = (long)min(nImageNum - 1, frameIDs[i]);

        Ipvm::ImageProcessing::CombineRGB(
            imageLot.GetImageFrame(frameIDs[2], enSideVisionModule::SIDE_VISIONMODULE_FRONT),
            imageLot.GetImageFrame(frameIDs[1], enSideVisionModule::SIDE_VISIONMODULE_FRONT),
            imageLot.GetImageFrame(frameIDs[0], enSideVisionModule::SIDE_VISIONMODULE_FRONT), Ipvm::Rect32s(imageColor),
            imageColor);

        // Recipe 용량 최소화를 위한 Image Resize
        long resizeImageSizeX(0), resizeImageSizeY(0);
        resizeImageSizeX = ((imageColor.GetSizeX() / 4 + 3) / 4) * 4;
        resizeImageSizeY = ((imageColor.GetSizeY() / 4 + 3) / 4) * 4;

        Ipvm::Image8u3 resizeImage(resizeImageSizeX, resizeImageSizeY);
        Ipvm::ImageProcessing::ResizeLinearInterpolation(imageColor, resizeImage);
        ///////////////////////////////////////////

        static const long sizeOfByte = sizeof(BYTE);
        totalImageSize = resizeImageSizeX * resizeImageSizeY * 3 * sizeOfByte;
        ngrv2DImageInfo.m_vecbyImage.resize(totalImageSize);

        ngrv2DImageInfo.m_imageSizeX = resizeImageSizeX * 3;
        ngrv2DImageInfo.m_imageSizeY = resizeImageSizeY;

        BYTE* imageMem = &ngrv2DImageInfo.m_vecbyImage[0];
        BYTE* imageCopy = (BYTE*)imageColor.GetMem();

        memset(imageMem, 0, totalImageSize);
        memcpy(imageMem, imageCopy, totalImageSize);

        ngrv2DImageInfo.m_isImageColor = TRUE;
    }

    // Image ScaleX/Y
    ngrv2DImageInfo.m_scaleX = imageLot.GetScale().pixelToUm().m_x;
    ngrv2DImageInfo.m_scaleY = imageLot.GetScale().pixelToUm().m_y;

    // Image Total Size
    ngrv2DImageInfo.m_totalImageSize = totalImageSize;

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << ngrv2DImageInfo;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_SEND_2D_VISION_IMAGE_INFO, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send 2D Image Info to NGRV"));

    ::free(pByte);
}

//2D ImageInfo Transfer Done : Host에게 완료되었다고 전송! - MED#6_JHB
void VisionMainTR::iPIS_Send_NGRV_2DVisionInfo_Receive_Done()
{
    m_pMessageSocket->Write(MSG_PROBE_NGRV_SEND_2D_VISION_INFO_ACK, 0, NULL);
}

// NGRV RunMode 정보 Transfer Done : Host에게 잘 받았음을 전달 - MED#6_JHB
void VisionMainTR::iPIS_Send_NGRV_RunMode_Receive_Done()
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    BOOL isReceived = TRUE;

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    std::vector<CString> vecStrTxtRecipe = ExportRecipeToText();
    long nSize = (long)vecStrTxtRecipe.size();

    try
    {
        ar << isReceived;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_NGRV_RUN_METHOD_INFO_ACK, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send NGRV SingleRunMode ACKOK"));

    ::free(pByte);
}

// NGRV 영상 획득 Vision Type 정보 Transfer Done : Host에게 잘 받았음을 전달 - MED#6_JHB
void VisionMainTR::iPIS_Send_NGRV_SingleRun_Visiontype_Receive_Done()
{
    m_pMessageSocket->Write(MSG_PROBE_NGRV_SINGLE_RUN_VISION_TYPE_INFO_ACK, 0, NULL);
}

void VisionMainTR::iPIS_Send_TextRecipe() //kircheis_TxtRecipe
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    std::vector<CString> vecStrTxtRecipe = ExportRecipeToText();
    long nSize = (long)vecStrTxtRecipe.size();

    try
    {
        ar << nSize;
        for (long nIdx = 0; nIdx < nSize; nIdx++)
            ar << vecStrTxtRecipe[nIdx];
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_SEND_TEXT_RECIPE, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send Text Recipe"));

    ::free(pByte);
    vecStrTxtRecipe.clear();
}

void VisionMainTR::iPIS_Send_VisionLogFile(const CString i_strFileName, const CString i_strFilePath)
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    CMemFile memFile;
    CFile file;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << i_strFileName;

        if (file.Open(i_strFilePath, CFile::modeRead))
        {
            const ULONGLONG fileSize = file.GetLength();
            BYTE* pbybuffer = new BYTE[fileSize];

            file.Read(pbybuffer, CAST_UINT(fileSize));

            file.Close();

            ar << fileSize;
            ar.Write(pbybuffer, CAST_UINT(fileSize));
        }
        else
            return;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_pMessageSocket->Write(MSG_PROBE_REQUEST_VISION_LOG_ACK, length, pByte);

    CString strSendFileName("");
    strSendFileName.Format(_T("Send Vision Log, File Name :%s"), (LPCTSTR)i_strFileName);

    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, strSendFileName);

    ::free(pByte);
}

void VisionMainTR::iPIS_Send_3D_Vision_StartIO_NotOff()
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    m_pMessageSocket->Write(MSG_PROBE_REQUEST_3D_VISION_START_IO_CHECK, NULL, NULL);
}

void VisionMainTR::iPIS_Send_VersionMismatchErrorPopup()
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    UINT Cur_FirmwareVersion = 0x00, Need_FirmwareVersion = 0x00;
    UINT Cur_LibraryVersion = 0x00, Need_LibraryVersion = 0x00;

    FrameGrabber::GetInstance().get_firmware_version(0, Cur_FirmwareVersion, Need_FirmwareVersion);
    FrameGrabber::GetInstance().get_library_version(Cur_LibraryVersion, Need_LibraryVersion);

    CString strCurVersionInfo = FrameGrabber::GetInstance().GetstrVersioninfo(Cur_FirmwareVersion, Cur_LibraryVersion);
    CString strNeedVersionInfo
        = FrameGrabber::GetInstance().GetstrVersioninfo(Need_FirmwareVersion, Need_LibraryVersion);

    CString strErrorMsg("");

    strErrorMsg.Format(_T("Version is mismatch!!\r\nExpected version : %s\r\nLoaded version : %s\r\n Cannot normal ")
                       _T("running Vision S/W!!!!"),
        (LPCTSTR)strCurVersionInfo, (LPCTSTR)strNeedVersionInfo); //Vision Type을 붙여줘야 하나..

    iPIS_Send_ErrorMessageForPopup(strErrorMsg);
}