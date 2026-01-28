//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "LandGroupIdChangerDlg.h"
#include "resource.h"

//CPP_2_________________________________ This project's headers
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
enum PropertyGridItemID_Group
{
    LAND_GROUP_1 = 1,
    LAND_GROUP_2,
    LAND_GROUP_3,
    LAND_GROUP_4,
};

enum PropertyGridItemID
{
    PROPERTY_ITEM_GROUP = 1,
    PROPERTY_ITEM_HEIGHT,
};

IMPLEMENT_DYNAMIC(CLandGroupIdChangerDlg, CDialog)

CLandGroupIdChangerDlg::CLandGroupIdChangerDlg(CWnd* pParent /*=nullptr*/, long& nGroupID, double& nLandHeight)
    : CDialog(IDD_DLG_LANDGROUP_CHANGER, pParent)
    , m_nLandGroup(&nGroupID)
    , m_LandHeight(&nLandHeight)
{
}

CLandGroupIdChangerDlg::~CLandGroupIdChangerDlg()
{
}

void CLandGroupIdChangerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLandGroupIdChangerDlg, CDialog)
ON_BN_CLICKED(IDOK, &CLandGroupIdChangerDlg::OnBnClickedOk)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// CLandGroupIdChangerDlg 메시지 처리기

BOOL CLandGroupIdChangerDlg::OnInitDialog()
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

void CLandGroupIdChangerDlg::SetPropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Land Information")))
    {
        if (*m_nLandGroup != -1)
        {
            if (auto* Item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Land Group"), *m_nLandGroup)))
            {
                Item->GetConstraints()->AddConstraint(_T("Group 1"), LAND_GROUP_1);
                Item->GetConstraints()->AddConstraint(_T("Group 2"), LAND_GROUP_2);
                Item->GetConstraints()->AddConstraint(_T("Group 3"), LAND_GROUP_3);
                Item->GetConstraints()->AddConstraint(_T("Group 4"), LAND_GROUP_4);

                Item->SetID(PROPERTY_ITEM_GROUP);
            }
        }

        if (auto* Item
            = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Land Height"), *m_LandHeight, _T("%.2f mm"))))
            Item->SetID(PROPERTY_ITEM_HEIGHT);

        category->Expand();
    }

    m_propertyGrid->SetViewDivider(0.50);
    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->Refresh();
}

LRESULT CLandGroupIdChangerDlg::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* ComboValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = ComboValue->GetEnum();

        switch (item->GetID())
        {
            case PROPERTY_ITEM_GROUP:
                *m_nLandGroup = data;
                break;
        }
    }
    else if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
    {
        double data = value->GetDouble();
        switch (item->GetID())
        {
            case PROPERTY_ITEM_HEIGHT:
                *m_LandHeight = (float)data;
                break;
        }
    }

    m_propertyGrid->Refresh();

    return 0;
}

void CLandGroupIdChangerDlg::OnBnClickedOk()
{
    CDialog::OnOK();
}
