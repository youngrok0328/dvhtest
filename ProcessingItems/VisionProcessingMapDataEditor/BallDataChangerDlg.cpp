//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BallDataChangerDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100

//CPP_7_________________________________ Implementation body
//
enum PropertyGridItemID
{
    PROPERTY_ITEM_HEIGHT = 1,
};

IMPLEMENT_DYNAMIC(CBallDataChangerDlg, CDialog)

CBallDataChangerDlg::CBallDataChangerDlg(CWnd* pParent /*=nullptr*/, double& nBallHeight)
    : CDialog(IDD_DLG_LANDGROUP_CHANGER, pParent)
    , m_BallHeight(&nBallHeight)
{
}

CBallDataChangerDlg::~CBallDataChangerDlg()
{
}

void CBallDataChangerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBallDataChangerDlg, CDialog)
ON_BN_CLICKED(IDOK, &CBallDataChangerDlg::OnBnClickedOk)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// CBallDataChangerDlg 메시지 처리기

BOOL CBallDataChangerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtCustomGrid;
    GetDlgItem(IDC_STATIC)->GetWindowRect(rtCustomGrid);
    ScreenToClient(rtCustomGrid);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtCustomGrid, this, IDC_FRAME_PROPERTY_GRID);
    m_propertyGrid->ShowHelp(FALSE);

    SetPropertyGrid();

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CBallDataChangerDlg::SetPropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Ball Information")))
    {
        if (auto* Item
            = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Ball Height"), *m_BallHeight, _T("%.2f mm"))))
            Item->SetID(PROPERTY_ITEM_HEIGHT);

        category->Expand();
    }

    m_propertyGrid->SetViewDivider(0.50);
    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->Refresh();
}

LRESULT CBallDataChangerDlg::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
    {
        double data = value->GetDouble();
        switch (item->GetID())
        {
            case PROPERTY_ITEM_HEIGHT:
                *m_BallHeight = (float)data;
                break;
        }
    }

    m_propertyGrid->Refresh();

    return 0;
}

void CBallDataChangerDlg::OnBnClickedOk()
{
    CDialog::OnOK();
}
