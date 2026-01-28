//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CDlgAutoFocusImageSet.h"

//CPP_2_________________________________ This project's headers
#include "Para.h"
#include "PropertyGrid_Impl.h"

//CPP_3_________________________________ Other projects' headers
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../VisionCommon/VisionImageLot.h"
#include "../VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgAutoFocusImageSet, CDialog)

CDlgAutoFocusImageSet::CDlgAutoFocusImageSet(VisionEdgeAlign::Para& para,
    VisionEdgeAlign::PropertyGrid_Impl* gridEngine, VisionProcessing& proc, long nBaseFrameIndex,
    CWnd* pParent /*= nullptr*/)
    : CDialog(IDD_AUTO_FOCUS_IMAGE_SET, pParent)
    , m_para(para)
    , m_gridEngine(gridEngine)
    , m_proc(proc)
    , m_nBaseFrameIndex(nBaseFrameIndex)
    , m_ImageLotView(nullptr)
{
    m_nImageFrameCount = m_proc.getImageLot().GetImageFrameCount();

    m_bIsValidPlaneRefInfo = m_para.m_bIsValidPlaneRefInfo;
    m_nNGRVAF_FrameID = m_para.m_nNGRVAF_FrameID;
    long nVecSize = (long)m_para.m_vecptRefPos_UM.size();
    m_vecptRefPos_UM.clear();
    m_vecptRefPos_UM.resize(nVecSize);
    for (long nIdx = 0; nIdx < nVecSize; nIdx++)
        m_vecptRefPos_UM[nIdx] = m_para.m_vecptRefPos_UM[nIdx];
    m_nAFImgSizeX = m_para.m_nAFImgSizeX;
    m_nAFImgSizeY = m_para.m_nAFImgSizeY;
    long nImageSize = m_nAFImgSizeX * m_nAFImgSizeY * 3;
    m_vecbyAFImage.clear();
    m_vecbyAFImage.resize(nImageSize);
    memcpy(&m_vecbyAFImage[0], &m_para.m_vecbyAFImage[0], nImageSize);
}

CDlgAutoFocusImageSet::~CDlgAutoFocusImageSet()
{
    delete m_ImageLotView;
}

void CDlgAutoFocusImageSet::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgAutoFocusImageSet, CDialog)
ON_WM_CREATE()
ON_BN_CLICKED(IDOK, &CDlgAutoFocusImageSet::OnBnClickedApply)
ON_BN_CLICKED(IDC_BUTTON_SET_DEFAULT_AF_ROI, &CDlgAutoFocusImageSet::OnBnClickedButtonSetDefault_AF_ROI)
END_MESSAGE_MAP()

// CDlgAutoFocusImageSet 메시지 처리기

BOOL CDlgAutoFocusImageSet::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtImageView;
    GetDlgItem(IDC_AUTO_FOCUS_SET_IMAGE)->GetWindowRect(rtImageView);
    ScreenToClient(rtImageView);

    m_ImageLotView = new ImageLotView(rtImageView, m_proc, GetSafeHwnd());

    SetAFImage();

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

int CDlgAutoFocusImageSet::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  여기에 특수화된 작성 코드를 추가합니다.

    return 0;
}

void CDlgAutoFocusImageSet::SetAFImage()
{
    Ipvm::Image8u CurrentImage;

    CurrentImage = m_proc.GetInspectionFrameImage(FALSE, m_para.m_nNGRVAF_FrameID);

    if (CurrentImage.GetMem() == nullptr)
    {
        CurrentImage.FillZero();
    }

    m_ImageLotView->ShowImage(m_para.m_nNGRVAF_FrameID);

    SetROI(CurrentImage);
}

void CDlgAutoFocusImageSet::SetROI(Ipvm::Image8u i_CurrentImage)
{
    if (m_ImageLotView == nullptr)
    {
        return;
    }

    //	1. 현재 이미지의 Center Point(Pixel)을 알아온다.
    Ipvm::Point32r2 imageCenter(i_CurrentImage.GetSizeX() * 0.5f, i_CurrentImage.GetSizeY() * 0.5f);

    //	2. 기존 Overlay를 다 삭제
    m_ImageLotView->Overlay_RemoveAll();
    m_ImageLotView->ROI_RemoveAll();

    //	2. 1.5mm X 1.5mm ROI를 만들기 위한 크기 설정
    long nWidth = (long)(1500.f / m_proc.getScale().pixelToUm().m_x + .5f);
    long nHeight = (long)(1500.f / m_proc.getScale().pixelToUm().m_y + .5f);
    long nPointNum = (long)m_vecptRefPos_UM.size();
    std::vector<Ipvm::Rect32s> vecrtRefPos(nPointNum);
    float fPoint(0.f);
    CString strRoiName;

    Ipvm::Rect32s rtImage = (Ipvm::Rect32s)i_CurrentImage;
    Ipvm::Point32s2 ptROICenter;
    std::vector<long> vecnNeedToSetDefaultROI_Idx(0);

    for (long nIdx = 0; nIdx < nPointNum; nIdx++)
    {
        fPoint = (m_vecptRefPos_UM[nIdx].m_x / m_proc.getScale().pixelToUm().m_x) + imageCenter.m_x;
        vecrtRefPos[nIdx].m_left = vecrtRefPos[nIdx].m_right = (long)(fPoint + 0.5f);
        fPoint = (m_vecptRefPos_UM[nIdx].m_y / m_proc.getScale().pixelToUm().m_y) + imageCenter.m_y;
        vecrtRefPos[nIdx].m_top = vecrtRefPos[nIdx].m_bottom = (long)(fPoint + 0.5f);

        ptROICenter = vecrtRefPos[nIdx].TopLeft();
        //	해당 ROI의 Center Point가 Image Size보다 크거나 0보다 작을 경우 Error Message
        if (rtImage.PtInRect(ptROICenter) == false)
        {
            vecnNeedToSetDefaultROI_Idx.push_back(nIdx);
        }
    }

    long nNeedSetROINUM = (long)vecnNeedToSetDefaultROI_Idx.size();

    if (nNeedSetROINUM > 0)
    {
        //Msg Box
        if (MessageBox(_T("Several AF Ref. ROIs are misplaced, Replace them with Default Position?"),
                _T("AF Ref. ROI Position Error"), MB_YESNO)
            == IDYES)
        {
            for (long nIdx = 0; nIdx < nNeedSetROINUM; nIdx++)
            {
                long nRealID = vecnNeedToSetDefaultROI_Idx[nIdx];
                int nX = -50 + ((nRealID % 2) * 100);
                int nY = -50 + ((nRealID / 2) * 100);

                fPoint = ((float)(nX * 100.f) / m_proc.getScale().pixelToUm().m_x) + imageCenter.m_x;
                vecrtRefPos[nRealID].m_left = vecrtRefPos[nRealID].m_right = (long)(fPoint + 0.5f);
                fPoint = ((float)(nY * 100.f) / m_proc.getScale().pixelToUm().m_y) + imageCenter.m_y;
                vecrtRefPos[nRealID].m_top = vecrtRefPos[nRealID].m_bottom = (long)(fPoint + 0.5f);
            }
        }
    }

    for (long nIdx = 0; nIdx < nPointNum; nIdx++)
    {
        vecrtRefPos[nIdx].InflateRect(nWidth, nHeight);
        strRoiName.Format(_T("AF Pos %d"), nIdx + 1);
        m_ImageLotView->ROI_Add(strRoiName, strRoiName, vecrtRefPos[nIdx], RGB(255, 0, 0), TRUE, FALSE);
    }

    m_ImageLotView->ROI_Show(TRUE);
    m_ImageLotView->Overlay_Show(TRUE);
}

void CDlgAutoFocusImageSet::OnBnClickedApply()
{
    // 1. ROI 고정
    long nPointNum = (long)m_vecptRefPos_UM.size();
    CString strRoiName;
    std::vector<Ipvm::Point32s2> vecptRoiCenterPXL(nPointNum);

    // SDY txt recipe를 만들기 위해 변수 전달
    m_para.m_nNGRVRefPosCount = nPointNum;
    m_para.m_vecrtNGRVRefPosROI.resize(nPointNum);

    for (long nIdx = 0; nIdx < nPointNum; nIdx++)
    {
        strRoiName.Format(_T("AF Pos %d"), nIdx + 1);
        Ipvm::Rect32s roi;
        m_ImageLotView->ROI_Get(strRoiName, roi);
        vecptRoiCenterPXL[nIdx] = roi.CenterPoint();

        m_para.m_vecrtNGRVRefPosROI[nIdx] = roi; // txt recipe용 roi 전달
    }

    // 2. Ellipse Fitting, Fill 255 & 0
    long nCurrentImageIndex = 0;

    Ipvm::Image8u imageOriginal;
    Ipvm::Image8u imageRed;
    Ipvm::Image8u imageGreen;
    Ipvm::Image8u imageBlue;
    Ipvm::Image8u3 imageCombine;
    Ipvm::Image8u3 resizeImage;

    long nImageSizeX = 0;
    long nImageSizeY = 0;

    nCurrentImageIndex = m_ImageLotView->GetCurrentImageFrame();

    imageOriginal = m_proc.GetInspectionFrameImage(FALSE, nCurrentImageIndex);
    m_para.m_nNGRVAF_FrameID = nCurrentImageIndex;

    nImageSizeX = imageOriginal.GetSizeX();
    nImageSizeY = imageOriginal.GetSizeY();

    Ipvm::Point32r2 imageCenter((float)nImageSizeX * 0.5f, (float)nImageSizeY * 0.5f);
    std::vector<Ipvm::EllipseEq32r> vecEqEllipse(nPointNum);
    float fWidth = (1500.f / m_proc.getScale().pixelToUm().m_x);
    float fHeight = (1500.f / m_proc.getScale().pixelToUm().m_y);

    for (long nIdx = 0; nIdx < nPointNum; nIdx++)
    {
        vecEqEllipse[nIdx].m_x = (float)vecptRoiCenterPXL[nIdx].m_x;
        vecEqEllipse[nIdx].m_y = (float)vecptRoiCenterPXL[nIdx].m_y;
        vecEqEllipse[nIdx].m_xradius = fWidth;
        vecEqEllipse[nIdx].m_yradius = fHeight;

        float fPointX = (float)vecptRoiCenterPXL[nIdx].m_x - imageCenter.m_x;
        fPointX *= m_proc.getScale().pixelToUm().m_x;
        float fPointY = (float)vecptRoiCenterPXL[nIdx].m_y - imageCenter.m_y;
        fPointY *= m_proc.getScale().pixelToUm().m_y;

        m_para.m_vecptRefPos_UM[nIdx].Set((long)(fPointX + .5f), (long)(fPointY + .5f));
    }

    imageRed.Create(nImageSizeX, nImageSizeY);
    imageGreen.Create(nImageSizeX, nImageSizeY);
    imageBlue.Create(nImageSizeX, nImageSizeY);
    imageCombine.Create(nImageSizeX, nImageSizeY);

    Ipvm::ImageProcessing::Copy(imageOriginal, Ipvm::Rect32s(imageOriginal), imageRed);
    Ipvm::ImageProcessing::Copy(imageOriginal, Ipvm::Rect32s(imageOriginal), imageGreen);
    Ipvm::ImageProcessing::Copy(imageOriginal, Ipvm::Rect32s(imageOriginal), imageBlue);

    for (long nIndex = 0; nIndex < nPointNum; nIndex++)
    {
        Ipvm::ImageProcessing::Fill(vecEqEllipse[nIndex], 255, imageRed);
        Ipvm::ImageProcessing::Fill(vecEqEllipse[nIndex], 0, imageGreen);
        Ipvm::ImageProcessing::Fill(vecEqEllipse[nIndex], 0, imageBlue);
    }

    // Image CombineRGB
    Ipvm::ImageProcessing::CombineRGB(imageRed, imageGreen, imageBlue, Ipvm::Rect32s(imageRed), imageCombine);

    //	이미지 크기가 기본 Camera FOV 크기보다 크면 1/4로 같거나 작으면 1/2로 축소
    long nCameraSensorSizeX = m_proc.getImageLot().GetSensorSizeX();
    long nCameraSensorSizeY = m_proc.getImageLot().GetSensorSizeY();

    if ((nImageSizeX > nCameraSensorSizeX && nImageSizeY > nCameraSensorSizeY) || nImageSizeX > nCameraSensorSizeX
        || nImageSizeY > nCameraSensorSizeY)
    {
        long nResizeScale = 4;

        // Reszie image Size 계산
        long nSendImageSizeX = ((imageCombine.GetSizeX() / nResizeScale + 3) / 4) * 4;
        long nSendImageSizeY = imageCombine.GetSizeY() / nResizeScale;

        // image reszie시 for문 연산을 IPP 연산을 하도록 한다.
        resizeImage.Create(nSendImageSizeX, nSendImageSizeY);
        Ipvm::ImageProcessing::ResizeLinearInterpolation(imageCombine, resizeImage);
    }
    else
    {
        long nResizeScale = 2;

        // Reszie image Size 계산
        long nSendImageSizeX = ((imageCombine.GetSizeX() / nResizeScale + 3) / 4) * 4;
        long nSendImageSizeY = imageCombine.GetSizeY() / nResizeScale;

        // image reszie시 for문 연산을 IPP 연산을 하도록 한다.
        resizeImage.Create(nSendImageSizeX, nSendImageSizeY);
        Ipvm::ImageProcessing::ResizeLinearInterpolation(imageCombine, resizeImage);
    }

    m_para.m_nAFImgSizeX = resizeImage.GetSizeX();
    m_para.m_nAFImgSizeY = resizeImage.GetSizeY();

    long nResizeImageSize = resizeImage.GetSizeX() * resizeImage.GetSizeY() * 3 * sizeof(BYTE);

    m_para.m_vecbyAFImage.resize(nResizeImageSize);

    // Vector Byte안에 직접 memcpy하지 말고 BYTE 이미지로 받은 후에 memcpy를 함 - 2023.04.26_JHB
    BYTE* pbyImage = &m_para.m_vecbyAFImage[0];
    BYTE* pbyColorImage = (BYTE*)resizeImage.GetMem();

    memset(pbyImage, 0, nResizeImageSize);
    memcpy(pbyImage, pbyColorImage, nResizeImageSize);

    //memcpy(&m_para.m_vecbyAFImage[0], resizeImage.GetMem(), nResizeImageSize);
    ////////////////////////////////////////////////////////////////////////////////////////////

    m_para.m_bIsValidPlaneRefInfo = TRUE;

    CDialog::OnOK();
}

void CDlgAutoFocusImageSet::OnBnClickedButtonSetDefault_AF_ROI()
{
    Ipvm::Image8u imageCurrent;

    long nCurrentImageIndex = 0;
    long nROINum = (long)m_vecptRefPos_UM.size();

    for (long nIdx = 0; nIdx < nROINum; nIdx++)
    {
        int nX = -50 + ((nIdx % 2) * 100);
        int nY = -50 + ((nIdx / 2) * 100);
        m_vecptRefPos_UM[nIdx].Set(nX * 100, nY * 100);
    }

    nCurrentImageIndex = m_ImageLotView->GetCurrentImageFrame();

    imageCurrent = m_proc.GetInspectionFrameImage(FALSE, nCurrentImageIndex);

    SetROI(imageCurrent);
}
