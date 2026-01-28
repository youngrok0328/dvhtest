//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingNGRV.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingNGRV_BTM2D.h"
#include "DlgVisionProcessingNGRV_DetailSetup.h"
#include "DlgVisionProcessingNGRV_TOP2D.h"
#include "VisionProcessingNGRV.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonGridGroupDlg.h"
#include "../../VisionNeedLibrary//VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)
#define UM_IMAGE_LOT_VIEW_ROI_CHANGED (WM_USER + 1012)
#define UM_IMAGE_LOT_VIEW_SELCHANGED_RAW (WM_USER + 1013)
#define UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE (WM_USER + 1014)

//CPP_7_________________________________ Implementation body

enum enumImageColor
{
    COLOR_B = 0,
    COLOR_G,
    COLOR_R,
};

IMPLEMENT_DYNAMIC(CDlgVisionProcessingNGRV, CDialog)

CDlgVisionProcessingNGRV::CDlgVisionProcessingNGRV(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingNGRV* pVisionInsp, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DIALOG, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_procCommonGridGroup(nullptr)
    , m_propertyGrid(nullptr)
{
    m_visionInsp = pVisionInsp;
    m_visionParaBTM = m_visionInsp->m_VisionParaBTM;
    m_visionParaTOP = m_visionInsp->m_VisionParaTOP;
}

CDlgVisionProcessingNGRV::~CDlgVisionProcessingNGRV()
{
    delete m_imageLotView;
    if (m_procCommonGridGroup != nullptr)
        delete m_procCommonGridGroup;
    delete m_propertyGrid;
}

void CDlgVisionProcessingNGRV::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_NGRV_SINGLERUN, m_resultTab);
    DDX_Control(pDX, IDC_TAB_NGRV_SIDE_SELECTION, m_selectionTab);
}

BEGIN_MESSAGE_MAP(CDlgVisionProcessingNGRV, CDialog)
ON_WM_DESTROY()
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionProcessingNGRV::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &CDlgVisionProcessingNGRV::OnImageLotViewRoiChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW, &CDlgVisionProcessingNGRV::OnImageLotViewSelChangedRaw)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE, CDlgVisionProcessingNGRV::OnImageLotViewSelChangedImage)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_NGRV_SIDE_SELECTION, &CDlgVisionProcessingNGRV::OnTcnSelchangeTabNgrvSideSelection)
END_MESSAGE_MAP()

// DlgVisionProcessingNGRV 메시지 처리기

BOOL CDlgVisionProcessingNGRV::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_visionInsp, true, true, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
    m_imageLotView->SetMessage_SelChangedRaw(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW);
    m_imageLotView->SetMessage_SelChangedImage(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE);

    //Caption
    SetWindowText(m_visionInsp->m_strModuleName);

    CRect resultTabRect = m_procDlgInfo.m_rtDataArea;
    resultTabRect.right = m_procDlgInfo.m_rtParaArea.left - 5;
    m_resultTab.MoveWindow(resultTabRect);
    m_resultTab.ShowWindow(SW_HIDE);

    CRect selectionTabRect;
    selectionTabRect.top = m_procDlgInfo.m_rtParaArea.top;
    selectionTabRect.bottom = resultTabRect.bottom;
    selectionTabRect.left = m_procDlgInfo.m_rtParaArea.left;
    selectionTabRect.right = m_procDlgInfo.m_rtParaArea.right;
    m_selectionTab.MoveWindow(selectionTabRect);

    m_selectionTab.InsertItem(0, _T("BTM2D"));
    m_selectionTab.InsertItem(1, _T("TOP2D"));

    m_selectionTab.SetCurSel(0);

    ShowImage(false);

    m_top2Dtab = new DlgVisionProcessingNGRV_TOP2D(m_visionInsp, *m_visionParaTOP);
    m_top2Dtab->Create(DlgVisionProcessingNGRV_TOP2D::IDD, &m_selectionTab);
    m_top2Dtab->ShowWindow(SW_HIDE);

    m_bottom2Dtab = new DlgVisionProcessingNGRV_BTM2D(m_visionInsp, *m_visionParaBTM);
    m_bottom2Dtab->Create(DlgVisionProcessingNGRV_BTM2D::IDD, &m_selectionTab);
    m_bottom2Dtab->ShowWindow(SW_SHOW);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionProcessingNGRV::ShowImage(bool isTop)
{
    // 입력받은 visionID 정보에 따라 Top/BTM을 구분한다
    long visionID(0);

    if (isTop == true)
    {
        visionID = VISION_TOP_2D;
    }
    else
    {
        visionID = VISION_BTM_2D;
    }

    // 2D Image 정보 불러온다.
    auto& visionUnit = m_visionInsp->m_visionUnit;
    auto& imageLot = visionUnit.getImageLot();
    Ngrv2DImageInfo& imageInfo = visionUnit.Get2DImageInfo_NGRV(visionID);

    bool isColor = imageInfo.m_isImageColor;
    bool isImage = imageInfo.m_totalImageSize <= 0 ? false : true;

    if (isImage == false)
    {
        if (m_visionInsp->GetImageFrameCount() > 0)
        {
            if (m_visionInsp->getIllumInfo2D().m_2D_colorFrame == TRUE)
            {
                long nFrameIDs[3];
                nFrameIDs[enumImageColor::COLOR_R] = m_visionInsp->getIllumInfo2D().m_2D_colorFrameIndex_red;
                nFrameIDs[enumImageColor::COLOR_G] = m_visionInsp->getIllumInfo2D().m_2D_colorFrameIndex_green;
                nFrameIDs[enumImageColor::COLOR_B] = m_visionInsp->getIllumInfo2D().m_2D_colorFrameIndex_blue;

                //BYTE* imageMems[3];
                long nImageNum = (long)imageLot.GetImageFrameCount();
                if (nImageNum < 1)
                    return;

                Ipvm::Image8u3 imageColor(imageLot.GetImageSizeX(), imageLot.GetImageSizeY());
                for (long i = enumImageColor::COLOR_B; i <= enumImageColor::COLOR_R; i++)
                    nFrameIDs[i] = (long)min(nImageNum - 1, nFrameIDs[i]);

                Ipvm::ImageProcessing::CombineRGB(imageLot.GetImageFrame(nFrameIDs[enumImageColor::COLOR_R],
                                                      enSideVisionModule::SIDE_VISIONMODULE_FRONT),
                    imageLot.GetImageFrame(
                        nFrameIDs[enumImageColor::COLOR_G], enSideVisionModule::SIDE_VISIONMODULE_FRONT),
                    imageLot.GetImageFrame(
                        nFrameIDs[enumImageColor::COLOR_B], enSideVisionModule::SIDE_VISIONMODULE_FRONT),
                    Ipvm::Rect32s(imageColor), imageColor);
                m_imageLotView->SetImage(imageColor);

                // Recipe 용량 최소화를 위한 Image Resize
                long resizeImageSizeX(0), resizeImageSizeY(0);
                resizeImageSizeX = ((imageColor.GetSizeX() / 4 + 3) / 4) * 4;
                resizeImageSizeY = ((imageColor.GetSizeY() / 4 + 3) / 4) * 4;

                Ipvm::Image8u3 resizeImage(resizeImageSizeX, resizeImageSizeY);
                Ipvm::ImageProcessing::ResizeLinearInterpolation(imageColor, resizeImage);

                // ImageSize 및 Scale 설정
                imageInfo.m_visionID = visionID;
                imageInfo.m_imageSizeX = imageLot.GetImageSizeX();
                imageInfo.m_imageSizeY = imageLot.GetImageSizeY();
                imageInfo.m_scaleX = imageLot.GetScale().pixelToUm().m_x;
                imageInfo.m_scaleY = imageLot.GetScale().pixelToUm().m_y;
                imageInfo.m_totalImageSize = resizeImageSizeX * resizeImageSizeY * sizeof(BYTE) * 3;
                imageInfo.m_vecbyImage.resize(imageInfo.m_totalImageSize);

                BYTE* memdst = &imageInfo.m_vecbyImage[0];
                BYTE* memsrc = (BYTE*)resizeImage.GetMem();

                memset(memdst, 0, imageInfo.m_totalImageSize);
                memcpy(memdst, memsrc, imageInfo.m_totalImageSize);

                visionUnit.Set2DImageInfo_NGRV(visionID, imageInfo);
            }
            else
            {
                Ipvm::Image8u imageMono(imageLot.GetImageSizeX(), imageLot.GetImageSizeY());
                imageMono = imageLot.GetImageFrame(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                m_imageLotView->SetImage(imageMono);

                // Recipe 용량 최소화를 위한 Image Resize
                long resizeImageSizeX(0), resizeImageSizeY(0);
                resizeImageSizeX = ((imageMono.GetSizeX() / 4 + 3) / 4) * 4;
                resizeImageSizeY = ((imageMono.GetSizeY() / 4 + 3) / 4) * 4;

                Ipvm::Image8u resizeImage(resizeImageSizeX, resizeImageSizeY);
                Ipvm::ImageProcessing::ResizeLinearInterpolation(imageMono, resizeImage);

                // ImageSize 및 Scale 설정
                imageInfo.m_visionID = visionID;
                imageInfo.m_imageSizeX = resizeImageSizeX;
                imageInfo.m_imageSizeY = resizeImageSizeY;
                imageInfo.m_scaleX = imageLot.GetScale().pixelToUm().m_x;
                imageInfo.m_scaleY = imageLot.GetScale().pixelToUm().m_y;
                imageInfo.m_totalImageSize = resizeImageSizeX * resizeImageSizeY * sizeof(BYTE);
                imageInfo.m_vecbyImage.resize(imageInfo.m_totalImageSize);

                BYTE* memdst = &imageInfo.m_vecbyImage[0];
                BYTE* memsrc = (BYTE*)imageMono.GetMem();

                memset(memdst, 0, imageInfo.m_totalImageSize);
                memcpy(memdst, memsrc, imageInfo.m_totalImageSize);

                visionUnit.Set2DImageInfo_NGRV(visionID, imageInfo);
            }
        }
        else
        {
            Ipvm::Image8u imageMono(imageLot.GetImageSizeX(), imageLot.GetImageSizeY());
            imageMono.FillZero();
            m_imageLotView->SetImage(imageMono);
        }
    }
    else
    {
        if (isColor == true)
        {
            Ipvm::Image8u3 imageColor(imageInfo.m_imageSizeX, imageInfo.m_imageSizeY);

            BYTE* memsrc = &imageInfo.m_vecbyImage[0];
            BYTE* memdst = (BYTE*)imageColor.GetMem();

            memset(memdst, 0, imageInfo.m_totalImageSize);
            memcpy(memdst, memsrc, imageInfo.m_totalImageSize);

            // Image Resize 원복
            long resizeImageSizeX(0), resizeImageSizeY(0);
            resizeImageSizeX = ((imageInfo.m_imageSizeX * 4 + 3) / 4) * 4;
            resizeImageSizeY = ((imageInfo.m_imageSizeY * 4 + 3) / 4) * 4;

            Ipvm::Image8u3 resizeImage(resizeImageSizeX, resizeImageSizeY);
            Ipvm::ImageProcessing::ResizeLinearInterpolation(imageColor, resizeImage);

            imageLot.SetRealImageSize(resizeImageSizeX, resizeImageSizeY);
            imageLot.SetScales(imageInfo.m_scaleX, imageInfo.m_scaleY, 0.f);

            m_imageLotView->SetImage(resizeImage);
        }
        else
        {
            const long NGRV_MaxSizeX = 9344;
            const long NGRV_MaxSizeY = 7000;

            if (imageInfo.m_imageSizeX % NGRV_MaxSizeX == 0 && imageInfo.m_imageSizeY % NGRV_MaxSizeY == 0)
            {
                ::SimpleMessage(_T("The review image on the recipe is incorrect.\n Please check it."));
                return;
            }

            Ipvm::Image8u image(imageInfo.m_imageSizeX, imageInfo.m_imageSizeY);

            BYTE* memsrc = &imageInfo.m_vecbyImage[0];
            BYTE* memdst = (BYTE*)image.GetMem();

            memset(memdst, 0, imageInfo.m_totalImageSize);
            memcpy(memdst, memsrc, imageInfo.m_totalImageSize);

            // Image Resize 원복
            long resizeImageSizeX(0), resizeImageSizeY(0);
            resizeImageSizeX = ((imageInfo.m_imageSizeX * 4 + 3) / 4) * 4;
            resizeImageSizeY = ((imageInfo.m_imageSizeY * 4 + 3) / 4) * 4;

            Ipvm::Image8u resizeImage(resizeImageSizeX, resizeImageSizeY);
            Ipvm::ImageProcessing::ResizeLinearInterpolation(image, resizeImage);

            imageLot.SetRealImageSize(resizeImageSizeX, resizeImageSizeY);
            imageLot.SetScales(imageInfo.m_scaleX, imageInfo.m_scaleY, 0.f);

            if (resizeImage.GetMem() != nullptr)
                m_imageLotView->SetImage(resizeImage);
        }
    }
}

void CDlgVisionProcessingNGRV::OnDestroy()
{
    CDialog::OnDestroy();

    // TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

LRESULT CDlgVisionProcessingNGRV::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_procCommonGridGroup->event_changedPane();

    return 0L;
}

LRESULT CDlgVisionProcessingNGRV::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_procCommonGridGroup->event_changedRoi();
    if (m_selectionTab.GetCurSel() == 0)
        m_bottom2Dtab->SetDefectROIinfo();
    else
        m_top2Dtab->SetDefectROIinfo();

    return 0L;
}

LRESULT CDlgVisionProcessingNGRV::OnImageLotViewSelChangedRaw(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_procCommonGridGroup->event_changedRaw();

    return 0L;
}

LRESULT CDlgVisionProcessingNGRV::OnImageLotViewSelChangedImage(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    //m_procCommonGridGroup->event_changedImage();

    return 0L;
}

void CDlgVisionProcessingNGRV::OnTcnSelchangeTabNgrvSideSelection(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
    // Tab Control을 바꿀 경우 Window를 Show/Hide 처리
    int currentSel = m_selectionTab.GetCurSel();

    switch (currentSel)
    {
        case 1:
            m_bottom2Dtab->ShowWindow(SW_HIDE);
            m_top2Dtab->ShowWindow(SW_SHOW);
            ShowImage(currentSel);
            m_top2Dtab->SetROI();
            break;
        case 0:
        default:
            m_top2Dtab->ShowWindow(SW_HIDE);
            m_bottom2Dtab->ShowWindow(SW_SHOW);
            ShowImage(currentSel);
            m_bottom2Dtab->SetROI();
            break;
    }

    UpdateData(TRUE);

    *pResult = 0;
}
