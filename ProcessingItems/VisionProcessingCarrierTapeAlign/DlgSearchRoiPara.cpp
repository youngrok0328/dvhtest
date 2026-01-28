//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSearchRoiPara.h"

//CPP_2_________________________________ This project's headers
#include "Para.h"
#include "PropertyGrid.h"
#include "Resource.h"

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
IMPLEMENT_DYNAMIC(DlgSearchRoiPara, CDialogEx)

DlgSearchRoiPara::DlgSearchRoiPara(Para& para, PropertyGrid* gridEngine, int32_t gridIndex, CWnd* pParent)
    : CDialogEx(IDD_SEARCH_ROI_PARA, pParent)
    , m_para(para)
    , m_gridEngine(gridEngine)
    , m_gridIndex(gridIndex)
{
}

DlgSearchRoiPara::~DlgSearchRoiPara()
{
}

void DlgSearchRoiPara::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgSearchRoiPara, CDialogEx)
ON_NOTIFY(GVN_ENDLABELEDIT, GRID_ID, OnGridEditEnd)
ON_NOTIFY(GVN_STATECHANGED, GRID_ID, OnGridEditEnd)
ON_WM_SIZE()
END_MESSAGE_MAP()

// DlgSearchRoiPara message handlers

BOOL DlgSearchRoiPara::OnInitDialog()
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

void DlgSearchRoiPara::UpdateList()
{
    switch (m_gridIndex)
    {
        case 0:
            UpdateCarrierTapeInfo();
            break;
        case 1:
            UpdatePocketInfo();
            break;
        default:
            ASSERT(!_T("??"));
            break;
    }
}

void DlgSearchRoiPara::UpdateCarrierTapeInfo()
{
    m_grid.SetColumnCount(4);
    m_grid.SetRowCount(3);
    m_grid.SetFixedRowCount(1);
    m_grid.SetEditable(TRUE);

    m_grid.SetItemText(0, 0, _T("Name"));
    m_grid.SetItemText(0, 1, _T("S.L (um)"));
    m_grid.SetItemText(0, 2, _T("Start (%)"));
    m_grid.SetItemText(0, 3, _T("End (%)"));

    long rowIndex = 1;
    for (long direction = 0; direction < 2; direction++)
    {
        auto& para = m_para.m_carrierTapeAlign.m_edgeSearchRois[direction];
        CString name = GetDirectionName(m_gridIndex == 1, direction);

        m_grid.SetItemText(rowIndex, 0, name);
        m_grid.SetItemState(rowIndex, 0, GVIS_READONLY);

        CString field_sl;
        CString field_start;
        CString field_end;

        field_sl.Format(_T("%.0f"), para.m_searchLength_um);
        field_start.Format(_T("%.1f"), para.m_persent_start);
        field_end.Format(_T("%.1f"), para.m_persent_end);

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

void DlgSearchRoiPara::UpdatePocketInfo()
{
    m_grid.SetColumnCount(4);
    m_grid.SetRowCount(5);
    m_grid.SetFixedRowCount(1);
    m_grid.SetEditable(TRUE);

    m_grid.SetItemText(0, 0, _T("Name"));
    m_grid.SetItemText(0, 1, _T("S.L (um)"));
    m_grid.SetItemText(0, 2, _T("Start (%)"));
    m_grid.SetItemText(0, 3, _T("End (%)"));

    long rowIndex = 1;
    for (long direction = 0; direction < 4; direction++)
    {
        auto& para = m_para.m_pocketAlign.m_edgeSearchRois[direction];
        CString name = GetDirectionName(m_gridIndex == 1, direction);

        m_grid.SetItemText(rowIndex, 0, name);
        m_grid.SetItemState(rowIndex, 0, GVIS_READONLY);

        CString field_sl;
        CString field_start;
        CString field_end;

        field_sl.Format(_T("%.0f"), para.m_searchLength_um);
        field_start.Format(_T("%.1f"), para.m_persent_start);
        field_end.Format(_T("%.1f"), para.m_persent_end);

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

bool DlgSearchRoiPara::CheckChangedCarrierTapeValues()
{
    bool change = false;
    long rowIndex = 1;
    for (auto& para : m_para.m_carrierTapeAlign.m_edgeSearchRois)
    {
        CString field_sl = m_grid.GetItemText(rowIndex, 1);
        CString field_start = m_grid.GetItemText(rowIndex, 2);
        CString field_end = m_grid.GetItemText(rowIndex, 3);

        float v1 = (float)_ttof(field_sl);
        float v2 = (float)_ttof(field_start);
        float v3 = (float)_ttof(field_end);
        if (v1 != para.m_searchLength_um)
        {
            para.m_searchLength_um = v1;
            change = true;
        }
        if (v2 != para.m_persent_start)
        {
            para.m_persent_start = v2;
            change = true;
        }
        if (v3 != para.m_persent_end)
        {
            para.m_persent_end = v3;
            change = true;
        }

        rowIndex++;

        field_sl.Empty();
        field_start.Empty();
        field_end.Empty();
    }

    return change;
}

bool DlgSearchRoiPara::CheckChangedPocketValues()
{
    bool change = false;
    long rowIndex = 1;
    for (auto& para : m_para.m_pocketAlign.m_edgeSearchRois)
    {
        CString field_sl = m_grid.GetItemText(rowIndex, 1);
        CString field_start = m_grid.GetItemText(rowIndex, 2);
        CString field_end = m_grid.GetItemText(rowIndex, 3);

        float v1 = (float)_ttof(field_sl);
        float v2 = (float)_ttof(field_start);
        float v3 = (float)_ttof(field_end);
        if (v1 != para.m_searchLength_um)
        {
            para.m_searchLength_um = v1;
            change = true;
        }
        if (v2 != para.m_persent_start)
        {
            para.m_persent_start = v2;
            change = true;
        }
        if (v3 != para.m_persent_end)
        {
            para.m_persent_end = v3;
            change = true;
        }

        rowIndex++;

        field_sl.Empty();
        field_start.Empty();
        field_end.Empty();
    }

    return change;
}

void DlgSearchRoiPara::OnGridEditEnd(NMHDR* pNotifyStruct, LRESULT* result)
{
    UNREFERENCED_PARAMETER(pNotifyStruct);

    bool change = false;

    switch (m_gridIndex)
    {
        case 0:
            change = CheckChangedCarrierTapeValues();
            break;
        case 1:
            change = CheckChangedPocketValues();
            break;
        default:
            ASSERT(!_T("??"));
            break;
    }

    if (change)
    {
        m_gridEngine->RedrawEdgeRoi(m_gridIndex);
    }

    *result = 0;
}

void DlgSearchRoiPara::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_grid.GetSafeHwnd())
    {
        m_grid.MoveWindow(0, 0, cx, cy);
    }
}
