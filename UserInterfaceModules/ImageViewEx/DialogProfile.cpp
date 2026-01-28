//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DialogProfile.h"

//CPP_2_________________________________ This project's headers
#include "ImageViewExImpl.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Widget/ProfileView.h>
#include <afxdialogex.h>

//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DialogProfile, CDialogEx)

DialogProfile::DialogProfile(ImageViewExImpl* pParent)
    : CDialogEx(IDD_DIALOG_PROFILE, pParent)
    , m_parent(pParent)
    , m_view(nullptr)
{
}

DialogProfile::~DialogProfile()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    delete m_view;
}

void DialogProfile::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DialogProfile, CDialogEx)
ON_WM_SIZE()
ON_WM_DESTROY()
END_MESSAGE_MAP()

// DialogProfile message handlers

BOOL DialogProfile::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CRect window;
    GetClientRect(window);
    m_view = new Ipvm::ProfileView(GetSafeHwnd(), Ipvm::FromMFC(window));

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DialogProfile::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_view)
    {
        ::MoveWindow(m_view->GetSafeHwnd(), 0, 0, cx, cy, TRUE);
    }
}

void DialogProfile::UpdateProfile()
{
    UpdateProfile(m_line);
}

void DialogProfile::UpdateProfile(const Ipvm::LineSeg32r& line)
{
    m_line = line;

    auto& image = m_parent->m_display_8u_C1;
    float fdx = m_line.m_ex - m_line.m_sx;
    float fdy = m_line.m_ey - m_line.m_sy;
    float fdist = float(sqrt(fdx * fdx + fdy * fdy));

    long nDataNum = long(fdist + 0.5f);

    if (nDataNum == 0)
    {
        m_view->Clear();
        return;
    }

    float angle = (float)atan2(m_line.m_ey - m_line.m_sy, m_line.m_ex - m_line.m_sx); // radian
    long length = (long)sqrt(pow(m_line.m_ey - m_line.m_sy, 2.f) + pow(m_line.m_ex - m_line.m_sx, 2.f));

    std::vector<Ipvm::Point32r2> lineData;

    float srcCos = (float)cos(angle);
    float srcSin = (float)sin(angle);

    float fx = (float)m_line.m_sx;
    float fy = (float)m_line.m_sy;

    for (int i = 0; i < length; i++, fx += srcCos, fy += srcSin)
    {
        long x = (int)(fx + 0.5f);
        long y = (int)(fy + 0.5f);

        if (x < 0 || y < 0 || x >= image.GetSizeX() || y >= image.GetSizeY())
        {
            lineData.emplace_back(float(i), 0.f);
            continue;
        }

        lineData.emplace_back(float(i), float(image[y][x]));
    }

    if (lineData.size())
    {
        m_view->SetData(0, &lineData[0], long(lineData.size()), Ipvm::k_noiseValue32r, RGB(255, 0, 0), RGB(255, 0, 0));
    }
    else
    {
        m_view->Clear();
    }
}

void DialogProfile::OnOK()
{
    m_parent->PostMessage(UM_PROFILEVIEW_DEAD);
    CDialogEx::OnOK();
}

void DialogProfile::OnCancel()
{
    m_parent->PostMessage(UM_PROFILEVIEW_DEAD);
    CDialogEx::OnCancel();
}

void DialogProfile::OnDestroy()
{
    CDialogEx::OnDestroy();

    m_parent->PostMessage(UM_PROFILEVIEW_DEAD);
}
