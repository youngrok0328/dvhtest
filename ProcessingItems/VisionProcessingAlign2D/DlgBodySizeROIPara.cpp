//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgBodySizeROIPara.h"

//CPP_2_________________________________ This project's headers
#include "Property_Stitch.h"
#include "StitchPara.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <afxdialogex.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GRID_ID (1000)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgBodySizeROIPara, CDialogEx)

DlgBodySizeROIPara::DlgBodySizeROIPara(StitchPara& para, Property_Stitch* engine, CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_BODYSIZE_ROI_PARA, pParent)
    , m_para(para)
    , m_engine(engine)
{
}

DlgBodySizeROIPara::~DlgBodySizeROIPara()
{
}

void DlgBodySizeROIPara::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgBodySizeROIPara, CDialogEx)
ON_WM_SIZE()
ON_NOTIFY(GVN_ENDLABELEDIT, GRID_ID, OnGridEditEnd)
ON_NOTIFY(GVN_STATECHANGED, GRID_ID, OnGridEditEnd)
END_MESSAGE_MAP()

// DlgBodySizeROIPara message handlers

BOOL DlgBodySizeROIPara::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CRect rtView;
    GetClientRect(rtView);

    m_grid.Create(rtView, this, GRID_ID);
    m_grid.SetEditable(TRUE);
    m_grid.EnableDragAndDrop(FALSE);

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgBodySizeROIPara::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_grid.GetSafeHwnd())
    {
        m_grid.MoveWindow(0, 0, cx, cy);
    }
}

void DlgBodySizeROIPara::UpdateList()
{
    auto& para = m_para.m_paraBasedOnBodySize;

    m_grid.SetColumnCount(4);
    m_grid.SetRowCount(long(para.m_stitchPara.size()) * 2 + 1);
    m_grid.SetFixedRowCount(1);
    m_grid.SetEditable(TRUE);

    m_grid.SetItemText(0, 0, _T("Name"));
    m_grid.SetItemText(0, 1, _T("S.L (um)"));
    m_grid.SetItemText(0, 2, _T("Start (%)"));
    m_grid.SetItemText(0, 3, _T("End (%)"));

    long rowIndex = 1;
    for (long stitchIndex = 0; stitchIndex < long(para.m_stitchPara.size()); stitchIndex++)
    {
        auto& stitchPara = para.m_stitchPara[stitchIndex];

        for (long direction = 0; direction < 2; direction++)
        {
            CString name;
            name.Format(_T("S%d-%s"), stitchIndex + 1, direction == 0 ? _T("V") : _T("H"));

            m_grid.SetItemText(rowIndex, 0, name);
            m_grid.SetItemState(rowIndex, 0, GVIS_READONLY);

            CString field_sl;
            CString field_start;
            CString field_end;

            field_sl.Format(_T("%.0f"), stitchPara.m_searchLength_um[direction]);
            field_start.Format(_T("%.1f"), stitchPara.m_persent_start[direction]);
            field_end.Format(_T("%.1f"), stitchPara.m_persent_end[direction]);

            m_grid.SetItemText(rowIndex, 1, field_sl);
            m_grid.SetItemText(rowIndex, 2, field_start);
            m_grid.SetItemText(rowIndex, 3, field_end);

            rowIndex++;

            name.Empty();
            field_sl.Empty();
            field_start.Empty();
            field_end.Empty();
        }
    }
}

void DlgBodySizeROIPara::OnGridEditEnd(NMHDR* /*pNotifyStruct*/, LRESULT* result)
{
    auto& para = m_para.m_paraBasedOnBodySize;

    bool change = false;
    long rowIndex = 1;
    for (long stitchIndex = 0; stitchIndex < long(para.m_stitchPara.size()); stitchIndex++)
    {
        auto& stitchPara = para.m_stitchPara[stitchIndex];

        for (long direction = 0; direction < 2; direction++)
        {
            CString field_sl = m_grid.GetItemText(rowIndex, 1);
            CString field_start = m_grid.GetItemText(rowIndex, 2);
            CString field_end = m_grid.GetItemText(rowIndex, 3);

            float v1 = (float)_ttof(field_sl);
            float v2 = (float)_ttof(field_start);
            float v3 = (float)_ttof(field_end);
            if (v1 != stitchPara.m_searchLength_um[direction])
            {
                stitchPara.m_searchLength_um[direction] = v1;
                change = true;
            }
            if (v2 != stitchPara.m_persent_start[direction])
            {
                stitchPara.m_persent_start[direction] = v2;
                change = true;
            }
            if (v3 != stitchPara.m_persent_end[direction])
            {
                stitchPara.m_persent_end[direction] = v3;
                change = true;
            }

            field_sl.Empty();
            field_start.Empty();
            field_end.Empty();

            rowIndex++;
        }
    }

    if (change)
    {
        m_engine->redrawOverlayAndROI();
    }

    *result = 0;
}
