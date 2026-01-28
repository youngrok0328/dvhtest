//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PropertyGrid.h"

//CPP_2_________________________________ This project's headers
#include "PropertyGrid_Impl.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
using namespace VisionEdgeAlign;

PropertyGrid::PropertyGrid(VisionProcessing& processor, Inspection& inspection, ImageLotView* imageLotView,
    ProcCommonDebugInfoDlg& procDebugInfo, VisionEdgeAlign::Para& para)
{
    m_impl = new PropertyGrid_Impl(processor, inspection, imageLotView, procDebugInfo, para);
}

PropertyGrid::~PropertyGrid()
{
    delete m_impl;
}

long PropertyGrid::getGridCount() const
{
    return m_impl->getGridCount();
}

bool PropertyGrid::create(long gridIndex, const RECT& rect, HWND parentWnd)
{
    return m_impl->create(gridIndex, rect, parentWnd);
}

LPCTSTR PropertyGrid::getName(long gridIndex) const
{
    return m_impl->getName(gridIndex);
}

void PropertyGrid::active(long gridIndex)
{
    m_impl->active(gridIndex);
}

void PropertyGrid::deactivate(long gridIndex)
{
    m_impl->deactivate(gridIndex);
}

bool PropertyGrid::notify(long gridIndex, WPARAM wparam, LPARAM lparam)
{
    return m_impl->notify(gridIndex, wparam, lparam);
}

void PropertyGrid::event_afterInspection(long gridIndex)
{
    m_impl->event_afterInspection(gridIndex);
}

void PropertyGrid::event_changedRoi(long gridIndex)
{
    m_impl->event_changedRoi(gridIndex);
}

void PropertyGrid::event_changedPane(long gridIndex)
{
    m_impl->event_changedPane(gridIndex);
}

void PropertyGrid::event_changedRaw(long gridIndex)
{
    m_impl->event_changedRaw(gridIndex);
}

void PropertyGrid::event_changedImage(long gridIndex)
{
    m_impl->event_changedImage(gridIndex);
}
