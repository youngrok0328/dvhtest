//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Dialog3D.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h> //kircheis_Test3DV
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Widget/ImageView.h>
#include <Ipvm/Widget/SurfaceView.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_ROI_SIZE_X 2000
#define MAX_ROI_SIZE_Y 2000
#define UM_UPDATE_3DVIEW (WM_USER + 100)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(Dialog3D, CDialog)

Dialog3D::Dialog3D(const Ipvm::Image8u& grayZmap, const Ipvm::Image8u& grayVmap, const Ipvm::Image32r& zmap,
    const Ipvm::Image16u& vmap, const VisionScale& scale, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DIALOG_3D, pParent)
    , m_grayZmap(grayZmap)
    , m_grayVmap(grayVmap)
    , m_zmap(zmap)
    , m_vmap(vmap)
    , m_scale(scale)
    , m_imageView(nullptr)
    , m_surfaceView(nullptr)
    , m_bExecutedResizeROI(FALSE) //kircheis_Test3DV
{
}

Dialog3D::~Dialog3D()
{
    delete m_surfaceView;
    delete m_imageView;
}

void Dialog3D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Dialog3D, CDialog)
ON_WM_SIZE()
ON_MESSAGE(UM_UPDATE_3DVIEW, OnUpdate3DView)
END_MESSAGE_MAP()

// Dialog3D message handlers

BOOL Dialog3D::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_bExecutedResizeROI = FALSE; //kircheis_Test3DV

    CRect rect;
    GetClientRect(rect);

    m_imageView = new Ipvm::ImageView(GetSafeHwnd(), Ipvm::FromMFC(rect));
    m_surfaceView = new Ipvm::SurfaceView(GetSafeHwnd(), Ipvm::FromMFC(rect));

    UpdateUIPosition();

    m_roi = GetMapROI(Ipvm::Rect32s(m_zmap));

    DisplayImage(true);

    m_imageView->SetRoi(_T("ROI"), _T("ROI"), m_roi, RGB(0, 255, 0));
    m_imageView->ShowRoi();

    m_imageView->RegisterRoiChangeEndCallback(GetSafeHwnd(), this, ROI_Changed);
    m_imageView->RegisterPixelInfoCallback(GetSafeHwnd(), this, PixelInfo);

    Update3DROI();

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void Dialog3D::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    UpdateUIPosition();
}

void Dialog3D::UpdateUIPosition()
{
    CRect rectImage;
    CRect rect3D;
    GetClientRect(rectImage);
    GetClientRect(rect3D);

    rect3D.right = rect3D.Width() / 2;
    rectImage.left = rect3D.right;

    ::MoveWindow(
        m_imageView->GetSafeHwnd(), rectImage.left, rectImage.top, rectImage.Width(), rectImage.Height(), TRUE);
    ::MoveWindow(m_surfaceView->GetSafeHwnd(), rect3D.left, rect3D.top, rect3D.Width(), rect3D.Height(), TRUE);
}

void Dialog3D::Update3DROI()
{
    auto mapROI = GetMapROI(m_roi);

    m_imageView->ClearImageOverlay();
    m_imageView->AddImageOverlay(mapROI, RGB(255, 0, 0));
    m_imageView->ShowImageOverlay();

    //{{//kircheis_Test3DV
    static const long nMaxROISize = MAX_ROI_SIZE_X * MAX_ROI_SIZE_Y; //기존의 최대 면적 계산
    long nCurROISize = (long)(mapROI.Width() * mapROI.Height()); //현재 Setting된 ROI의 면적 계산
    long nReductionScale = (long)(nCurROISize / nMaxROISize)
        + 1; //기존의 Max 사이즈 기준 면적을 기반으로 배율을 계산한다. +1을 한 이유는 기존 Max Size 이내 일 때는 1배율인데, 이 계산식의 배율은 0부터 계산되니까 1 증가

    if (nReductionScale == 1) // 축소 배율이 1이면 귀찮은 연산하지 말고 바로 뿌리자
    {
        m_surfaceView->Load(Ipvm::Image32r(m_zmap, mapROI), m_scale.pixelToUm().m_x, m_scale.pixelToUm().m_y,
            Ipvm::k_noiseValue32r, Ipvm::ColorMapIndex::e_rainbow);

        return;
    }
    //전체 영상 크기 계산
    long nSizeX = m_zmap.GetSizeX();
    long nSizeY = m_zmap.GetSizeY();

    Ipvm::Image32r image; //축소용 버퍼 선언
    image.Create(
        nSizeX / nReductionScale, nSizeY / nReductionScale); //축소용 버퍼 할당. 이때 원본 크기에 축소 배율 적용
    if (Ipvm::ImageProcessing::ResizeNearestInterpolation(m_zmap, image)
        != Ipvm::Status::
            e_ok) //영상 크기 변환. 확대 개념이 아니므로 속도가 빠른 최근방 보간 사용. (Code 완성 후 다른 보간법을 Test 했는데 모두 Data가 깨짐)
        return;

    //ROI를 축소 배율에 맞게 조정
    Ipvm::Rect32s rtROI;
    rtROI.SetRect(mapROI.m_left / nReductionScale, mapROI.m_top / nReductionScale, mapROI.m_right / nReductionScale,
        mapROI.m_bottom / nReductionScale);

    //재구성된 축소 영상과 축소 ROI 적용.
    m_surfaceView->Load(Ipvm::Image32r(image, rtROI), (m_scale.pixelToUm().m_x * (float)nReductionScale),
        (m_scale.pixelToUm().m_y * (float)nReductionScale), Ipvm::k_noiseValue32r, Ipvm::ColorMapIndex::e_rainbow);
    //}}
}

Ipvm::Rect32s Dialog3D::GetMapROI(const Ipvm::Rect32s& roi)
{
    // ZMap을 볼 ROI가 최대 Size를 벗어나지 않게 한다
    // ROI Size가 너무 커지면 SurfaceView가 죽기도 해서 크기를 제한했다

    auto roiInImage = roi & Ipvm::Rect32s(m_zmap);

    auto mapROI = roiInImage;

    //{{//kircheis_Test3DV
    if (m_bExecutedResizeROI) //ROI를 기존처럼 Max 사이즈 적용하는거는 처음 한번만 하는거다.
        return mapROI;
    m_bExecutedResizeROI = TRUE;
    //}}

    long roiSizeX = roiInImage.Width();
    long roiSizeY = roiInImage.Height();

    if (roiSizeX > MAX_ROI_SIZE_X)
    {
        mapROI.m_left = roiInImage.CenterPoint().m_x - MAX_ROI_SIZE_X / 2;
        mapROI.m_right = roiInImage.CenterPoint().m_x + MAX_ROI_SIZE_X / 2;
    }

    if (roiSizeY > MAX_ROI_SIZE_Y)
    {
        mapROI.m_top = roiInImage.CenterPoint().m_y - MAX_ROI_SIZE_Y / 2;
        mapROI.m_bottom = roiInImage.CenterPoint().m_y + MAX_ROI_SIZE_Y / 2;
    }

    mapROI &= roiInImage;

    return mapROI;
}

void Dialog3D::ROI_Changed(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength)
{
    ((Dialog3D*)userData)->ROI_Changed(id, key, keyLength);
}

void Dialog3D::PixelInfo(void* userData, const int32_t id, const Ipvm::Point32r2& imagePoint,
    const uint32_t infoBufferLength, wchar_t* infoBuffer)
{
    UNREFERENCED_PARAMETER(id);

    ((Dialog3D*)userData)->PixelInfo(id, imagePoint, infoBufferLength, infoBuffer);
}

void Dialog3D::PixelInfo(
    const int32_t id, const Ipvm::Point32r2& imagePoint, const uint32_t infoBufferLength, wchar_t* infoBuffer)
{
    UNREFERENCED_PARAMETER(id);

    if (m_zmap.GetMem() != nullptr)
    {
        Ipvm::Point32s2 ptXY(long(imagePoint.m_x + 0.5f), long(imagePoint.m_y + 0.5f));

        if (Ipvm::Rect32s(m_zmap).PtInRect(ptXY) && m_zmap[ptXY.m_y][ptXY.m_x] != Ipvm::k_noiseValue32r)
        {
            ::_snwprintf_s(infoBuffer, infoBufferLength, _TRUNCATE, _T("POS[%.2f %.2f] [%.02f] um (V:%d)"),
                imagePoint.m_x, imagePoint.m_y, m_zmap[ptXY.m_y][ptXY.m_x], m_vmap[ptXY.m_y][ptXY.m_x]);
        }
        else
        {
            ::_snwprintf_s(
                infoBuffer, infoBufferLength, _TRUNCATE, _T("POS[%.2f %.2f]"), imagePoint.m_x, imagePoint.m_y);
        }
    }
}

void Dialog3D::ROI_Changed(const int32_t id, const wchar_t* key, const size_t keyLength)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(keyLength);

    m_imageView->GetRoi(key, m_roi);
    PostMessage(UM_UPDATE_3DVIEW);
}

LRESULT Dialog3D::OnUpdate3DView(WPARAM, LPARAM)
{
    Update3DROI();
    return 0;
}

BOOL Dialog3D::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        switch (pMsg->wParam)
        {
            case _T('1'):
                DisplayImage(true);
                break;
            case _T('2'):
                DisplayImage(false);
                break;
            case _T('5'):
                SaveHeightDataInROI();
                break;
        }
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void Dialog3D::DisplayImage(bool isZmap)
{
    if (isZmap)
    {
        m_imageView->SetImage(m_grayZmap);
        return;
    }

    m_imageView->SetImage(m_grayVmap);
}

void Dialog3D::SaveHeightDataInROI()
{
    Ipvm::Rect32s roi = m_roi;
    roi.NormalizeRect();

    const int roiSizeX = roi.Width();
    const int roiSizeY = roi.Height();

    const int imageSizeX = m_zmap.GetSizeX();
    const int imageSizeY = m_zmap.GetSizeY();

    if (roiSizeX <= 0 || roiSizeY <= 0 || imageSizeX <= 0 || imageSizeY <= 0)
    {
        return; // Invalid ROI or image size
    }

    Ipvm::Rect32s imageRect(0, 0, imageSizeX - 1, imageSizeY - 1);
    roi &= imageRect; // Ensure ROI is within image bounds

    const int roiLeft = roi.m_left;
    const int roiTop = roi.m_top;
    const int roiRight = roi.m_right;
    const int roiBottom = roi.m_bottom;

    //ROI안의 높이 데이터를 csv파일로 저장하는 로직
    FILE* file = nullptr;
    _tfopen_s(&file, _T("d:\\HeightData.csv"), _T("w"));

    if (file == nullptr)
    {
        AfxMessageBox(_T("Failed to open file for writing height data."), MB_ICONERROR);
        return;
    }

    float height = 0.f;
    for (long y = roiTop; y <= roiBottom; ++y)
    {
        for (long x = roiLeft; x <= roiRight; ++x)
        {
            height = m_zmap[y][x];
            if (height != Ipvm::k_noiseValue32r)
            {
                fprintf(file, "%.2f,", height);
            }
            else
            {
                fprintf(file, " ,");
            }
        }
        fprintf(file, "\n");
    }
    fclose(file);
    AfxMessageBox(_T("Height data saved to d:\\HeightData.csv"), MB_ICONINFORMATION);
}