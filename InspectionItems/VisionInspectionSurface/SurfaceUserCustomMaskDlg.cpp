//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceUserCustomMaskDlg.h"

//CPP_2_________________________________ This project's headers
#include "SurfaceItem.h"
#include "SurfaceSetMaskNameDlg.h"
#include "VisionInspectionSurface.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../UserInterfaceModules/ImageViewEx/PatternImage.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/CircleEq32r.h>
#include <Ipvm/Base/ImageFile.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum enumMenuSurfaceIgnore
{
    ID_MENU_SURFACE_MASK_CIRCLE = 32835,
    ID_MENU_SURFACE_MASK_DELETE,
    ID_MENU_SURFACE_MASK_CANCEL
};

CSurfaceUserCustomMaskDlg::CSurfaceUserCustomMaskDlg(
    const Ipvm::Image8u& SurfaceBitmapMask, Ipvm::Image8u& image, VisionInspectionSurface* pVisionInsp)
    : CDialog(CSurfaceUserCustomMaskDlg::IDD)
    , m_pEditSurfaceBitmapMask(new Ipvm::Image8u)
    , m_imageView(nullptr)
    , m_image(image)
    , m_pVisionInsp(pVisionInsp)
{
    m_pEditSurfaceBitmapMask->Create(SurfaceBitmapMask.GetSizeX(), SurfaceBitmapMask.GetSizeY());
    Ipvm::ImageProcessing::Copy(SurfaceBitmapMask, Ipvm::Rect32s(SurfaceBitmapMask), *m_pEditSurfaceBitmapMask);
    m_displayImage.Create(image.GetSizeX(), image.GetSizeY());
}

CSurfaceUserCustomMaskDlg::~CSurfaceUserCustomMaskDlg()
{
    delete m_pEditSurfaceBitmapMask;
    delete m_imageView;
}

void CSurfaceUserCustomMaskDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSurfaceUserCustomMaskDlg)
    DDX_Control(pDX, IDC_EDIT_RADIUS, m_edtRadius);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSurfaceUserCustomMaskDlg, CDialog)
//{{AFX_MSG_MAP(CSurfaceUserCustomMaskDlg)
ON_BN_CLICKED(IDC_BTN_EXPORT, OnBtnExport)
ON_BN_CLICKED(IDC_BTN_IMPORT, OnBtnImport)
ON_BN_CLICKED(IDC_BTN_Erode, OnBTNErode)
ON_BN_CLICKED(IDC_BTN_Dilate, OnBTNDilate)
//}}AFX_MSG_MAP
ON_BN_CLICKED(IDOK, &CSurfaceUserCustomMaskDlg::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSurfaceUserCustomMaskDlg message handlers

BOOL CSurfaceUserCustomMaskDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect view;
    GetDlgItem(IDC_STATIC_IMAGE_FRAME)->GetWindowRect(view);
    ScreenToClient(view);

    m_imageView = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(view), 0, m_pVisionInsp->GetCurVisionModule_Status());
    m_imageView->RegisterCallback_MouseLButtonDown(GetSafeHwnd(), this, callbackMouseClicked);

    m_edtRadius.SetWindowText(_T("7"));

    RefreshImage();

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void CSurfaceUserCustomMaskDlg::RefreshImage()
{
    auto& surfaceMask = *m_pEditSurfaceBitmapMask;

    const long nBitmapMaskSizeX = m_image.GetSizeX();
    const long nBitmapMaskSizeY = m_image.GetSizeY();

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(m_displayImage), 0, m_displayImage);

    bool displayMask = true;

    if (surfaceMask.GetSizeX() != m_image.GetSizeX() || surfaceMask.GetSizeY() != m_image.GetSizeY())
    {
        displayMask = false;
    }

    for (long y = 0; y < nBitmapMaskSizeY; y++)
    {
        auto* mask_y = surfaceMask.GetMem(0, y);
        auto* background_y = m_image.GetMem(0, y);
        auto* display_y = m_displayImage.GetMem(0, y);

        for (long x = 0; x < nBitmapMaskSizeX; x++)
        {
            if (displayMask && mask_y[x])
            {
                display_y[x] = 255;
            }
            else
            {
                display_y[x] = 252 * background_y[x] / 255 + 1;
            }
        }
    }

    PatternImage pattern;
    if (pattern.setThresholdImage(m_displayImage))
    {
        m_imageView->SetImage(pattern);
    }
    else
    {
        MessageBox(_T("Not enough memory."));
    }
}

void CSurfaceUserCustomMaskDlg::OnBtnExport()
{
    CFileDialog dlg(FALSE, _T("*.BMP"), NULL, OFN_OVERWRITEPROMPT, _T("*.BMP|*.BMP|"));

    if (dlg.DoModal() == IDOK)
    {
        Ipvm::ImageFile::SaveAsBmp(*m_pEditSurfaceBitmapMask, dlg.GetPathName());
    }
}

void CSurfaceUserCustomMaskDlg::OnBtnImport()
{
    CFileDialog dlg(TRUE, _T("*.BMP"), NULL, OFN_HIDEREADONLY, _T("*.BMP|*.BMP|"));

    if (dlg.DoModal() == IDOK)
    {
        Ipvm::ImageFile::Load(dlg.GetPathName(), *m_pEditSurfaceBitmapMask);

        RefreshImage();
    }
}

void CSurfaceUserCustomMaskDlg::OnBTNErode()
{
    auto& image = *m_pEditSurfaceBitmapMask;

    Ipvm::ImageProcessing::MorphErodeIter(Ipvm::Rect32s(image), 3, 3, 1, image);

    RefreshImage();
}

void CSurfaceUserCustomMaskDlg::OnBTNDilate()
{
    auto& image = *m_pEditSurfaceBitmapMask;

    Ipvm::ImageProcessing::MorphDilateIter(Ipvm::Rect32s(image), 3, 3, 1, image);

    RefreshImage();
}

void CSurfaceUserCustomMaskDlg::callbackMouseClicked(
    void* userData, const int32_t id, const Ipvm::Point32r2& imagePoint)
{
    ((CSurfaceUserCustomMaskDlg*)userData)->callbackMouseClicked(id, imagePoint);
}

void CSurfaceUserCustomMaskDlg::callbackMouseClicked(const int32_t id, const Ipvm::Point32r2& imagePoint)
{
    UNREFERENCED_PARAMETER(id);
    
    m_ptLatestSelPoint.x = long(imagePoint.m_x + 0.5f);
    m_ptLatestSelPoint.y = long(imagePoint.m_y + 0.5f);

    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING, ID_MENU_SURFACE_MASK_CIRCLE, _T("Add Circle"));
    menu.AppendMenu(MF_STRING, ID_MENU_SURFACE_MASK_DELETE, _T("Delete Object"));
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, ID_MENU_SURFACE_MASK_CANCEL, _T("Cancel"));

    CPoint ptMouse;
    ::GetCursorPos(&ptMouse);

    menu.TrackPopupMenu(TPM_RIGHTALIGN | TPM_LEFTBUTTON, ptMouse.x, ptMouse.y, this);

    menu.DestroyMenu();
}

BOOL CSurfaceUserCustomMaskDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (HIWORD(wParam) == 0)
    {
        long nID = LOWORD(wParam);

        switch (nID)
        {
            case ID_MENU_SURFACE_MASK_CIRCLE:
            {
                AddCircle(m_ptLatestSelPoint.x, m_ptLatestSelPoint.y, 5);
            }
                return TRUE;
            case ID_MENU_SURFACE_MASK_DELETE:
            {
                DeleteObject(m_ptLatestSelPoint.x, m_ptLatestSelPoint.y);
            }
                return TRUE;
            case ID_MENU_SURFACE_MASK_CANCEL:
            {
                // Do nothing
            }
                return TRUE;
        }
    }

    return CDialog::OnCommand(wParam, lParam);
}

void CSurfaceUserCustomMaskDlg::AddCircle(long x, long y, long nRadius)
{
    nRadius = 0;

    CString str;
    m_edtRadius.GetWindowText(str);
    float fRadius = (float)_ttof(str);

    if (fRadius < 5)
        return;

    auto& mask = *m_pEditSurfaceBitmapMask;

    Ipvm::CircleEq32r circle;
    circle.m_x = (float)x;
    circle.m_y = (float)y;
    circle.m_radius = fRadius;

    Ipvm::ImageProcessing::Fill(circle, 255, mask);

    RefreshImage();

    str.Empty();
}

void CSurfaceUserCustomMaskDlg::DeleteObject(long x, long y)
{
    auto& mask = *m_pEditSurfaceBitmapMask;
    auto targetValue = mask[y][x];

    if (targetValue)
    {
        int32_t bufferSize;
        Ipvm::ImageProcessing::FloodFillBufferSize(Ipvm::Rect32s(mask), bufferSize);
        BYTE* buffer = new BYTE[bufferSize];

        Ipvm::BlobInfo blobInfo;
        Ipvm::ImageProcessing::FloodFill(Ipvm::Rect32s(mask), true, x, y, 0, buffer, mask, blobInfo);

        delete[] buffer;

        RefreshImage();
    }
}

void CSurfaceUserCustomMaskDlg::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    OnOK();
}
