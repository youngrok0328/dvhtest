//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "IllumControlNGRV.h"

//CPP_2_________________________________ This project's headers
#include "VisionMain.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NGRV_NORMAL_MAX_FRAME 7

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(IllumControlNGRV, CButton)

void IllumControlNGRV::ItemInfo::initFrame(long maxFrameCount)
{
    m_useFrames.resize(maxFrameCount);

    for (long index = 0; index < maxFrameCount; index++)
    {
        m_useFrames[index].m_checked = false;
    }
}

long IllumControlNGRV::ItemInfo::getFrameCount() const
{
    return long(m_useFrames.size());
}

long IllumControlNGRV::ItemInfo::getCheckedFrameCount() const
{
    long count = 0;
    for (long index = 0; index < long(m_useFrames.size()); index++)
    {
        if (m_useFrames[index].m_checked)
        {
            count++;
        }
    }

    return count;
}

IllumControlNGRV::SItemFrameInfo& IllumControlNGRV::ItemInfo::getFrame(long index)
{
    return m_useFrames[index];
}

void IllumControlNGRV::ItemInfo::checkFrame(long frameIndex, bool enabled)
{
    m_useFrames[frameIndex].m_checked = enabled;
}

void IllumControlNGRV::ItemInfo::incFrameIndex(long baseFrameIndex)
{
    for (long index = long(m_useFrames.size()) - 1; index >= baseFrameIndex + 1; index--)
    {
        m_useFrames[index] = m_useFrames[index - 1];
    }

    m_useFrames[baseFrameIndex].m_checked = false;
}

void IllumControlNGRV::ItemInfo::eraseFrame(long frameIndex, bool frameShift)
{
    if (frameShift == false)
    {
        checkFrame(frameIndex, false);
    }
    else
    {
        for (long index = frameIndex; index < long(m_useFrames.size()) - 1; index++)
        {
            m_useFrames[index] = m_useFrames[index + 1];
        }

        m_useFrames[(m_useFrames.size()) - 1].m_checked = false;
    }
}

long IllumControlNGRV::ItemInfo::getFirstFrameIndex() const
{
    for (long index = 0; index < long(m_useFrames.size()); index++)
    {
        if (m_useFrames[index].m_checked)
        {
            return index;
        }
    }

    return -1;
}

IllumControlNGRV::IllumControlNGRV(long maxFrameCount)
    : m_maxFrameCount(maxFrameCount)
    , m_realFrameCount(0)
    , m_selectItemID(-1)
    , m_selectRealFrameIndex(-1)
    , m_isEditMode(false)
    , m_dragGroupIndex(-1)
    , m_dragFrameIndex(-1)
    , m_dragAction(false)
    , m_design_illumHeadHeight(40)
    , m_design_rowHeight(30)
    , m_designFactor(1.f)
{
    HDC hdc = ::GetDC(NULL);

    if (hdc)
    {
        m_designFactor = CAST_FLOAT(GetDeviceCaps(hdc, LOGPIXELSY) / 96.0);
        m_design_rowHeight = long(m_design_rowHeight * m_designFactor);
    }

    resetNGRVData();
}

IllumControlNGRV::~IllumControlNGRV()
{
}

BEGIN_MESSAGE_MAP(IllumControlNGRV, CButton)
ON_WM_PAINT()
ON_WM_VSCROLL()
ON_WM_LBUTTONDOWN()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONUP()
ON_WM_MOUSEWHEEL()
ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

// IllumControl2D message handlers

void IllumControlNGRV::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect view;
    GetClientRect(view);

    CDC memDC;
    CBitmap memBitmap;
    memDC.CreateCompatibleDC(&dc);
    memBitmap.CreateCompatibleBitmap(&dc, view.Width(), view.Height());
    auto* oldMemBitmap = memDC.SelectObject(&memBitmap);

    // 먼저 검사항목을 그린다
    long checkRegionX = (LED_ILLUM_FRAME_MAX + 1) * m_design_rowHeight;
    long name_width = 0;

    // Group이 하나 사라졌으니 group_marginX 방향만큼 한 번만 곱해줘서 빼야함
    name_width = view.Width() - checkRegionX;

    CRect nameRegion(0, m_design_illumHeadHeight, name_width, view.Height());
    memDC.FillSolidRect(nameRegion, RGB(144, 177, 220));

    long check_sizeX = checkRegionX / (LED_ILLUM_FRAME_MAX + 1);
    check_sizeX += check_sizeX / 10;

    CFont font;
    font.CreateFont((int)(20 * m_designFactor), (int)(10 * m_designFactor), 0, 0, 0, FALSE, FALSE, FALSE, FALSE, 255,
        255, 255, 16, _T("Calibri"));

    auto* oldFont = memDC.SelectObject(&font);

    memDC.SetBkMode(TRANSPARENT);

    if (1)
    {
        long drawStartX = name_width;
        // Group 표시
        for (auto& group : m_frameGroups)
        {
            auto& group_region = group.second.m_roi;

            switch (group.first) // 4개의 Type(Normal, Reverse, UV, IR)으로 나누어 조명 설정 칸을 나눈다. - JHB_NGRV
            {
                case NORMAL_FRAME:
                {
                    group_region.left = drawStartX;
                    group_region.right = drawStartX + check_sizeX * NGRV_NORMAL_MAX_FRAME;
                    group_region.top = 0;
                    group_region.bottom = view.bottom;
                }
                break;

                case REVERSE_FRAME:
                {
                    group_region.left = drawStartX;
                    group_region.right = drawStartX + check_sizeX;
                    group_region.top = 0;
                    group_region.bottom = view.bottom;
                }
                break;

                case UV_FRAME:
                {
                    group_region.left = drawStartX;
                    group_region.right = drawStartX + check_sizeX;
                    group_region.top = 0;
                    group_region.bottom = view.bottom;
                }
                break;

                case IR_FRAME:
                {
                    group_region.left = drawStartX;
                    group_region.right = drawStartX + check_sizeX;
                    group_region.top = 0;
                    group_region.bottom = view.bottom;
                }
                break;
            }

            memDC.FillSolidRect(group_region, group.second.m_backgroundColor);
            drawRowText(
                memDC, drawStartX, m_design_illumHeadHeight + 16, 0, 16, group.second.m_fontColor, group.second.m_name);

            drawStartX = group_region.right;
        }
    }

    if (1)
    {
        // Item Name 및 선택 Draw
        long drawStartY = -GetScrollPos(SB_VERT) + m_design_illumHeadHeight;

        for (auto& itemInfo : m_itemInfos)
        {
            CRect rowRegion(name_width, drawStartY, view.right, drawStartY + m_design_rowHeight);

            CRect column_name = rowRegion;
            column_name.left = 0;
            column_name.right = name_width;

            itemInfo.second.m_name_Rect = column_name;

            if (m_selectItemID == itemInfo.first)
            {
                memDC.FillSolidRect(column_name, RGB(64, 110, 169));
                memDC.SetTextColor(RGB(255, 0, 0));
                memDC.DrawText(_T(" ") + itemInfo.second.m_name, column_name, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

                // 선택된 ITEM에 삼각형 표시
                CPoint pt[3];
                pt[0].x = name_width - 15;
                pt[0].y = column_name.CenterPoint().y;

                pt[1].x = name_width - 5;
                pt[1].y = column_name.CenterPoint().y - 7;

                pt[2].x = name_width - 5;
                pt[2].y = column_name.CenterPoint().y + 7;

                memDC.SelectStockObject(NULL_PEN);
                memDC.Polygon(pt, 3);

                drawAlphaRectangle(memDC, rowRegion, RGB(64, 110, 169), 128);
            }
            else
            {
                memDC.SetTextColor(RGB(0, 0, 0));
                memDC.Draw3dRect(itemInfo.second.m_name_Rect, RGB(128, 128, 128), RGB(64, 64, 64));
                memDC.DrawText(_T(" ") + itemInfo.second.m_name, column_name, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            }

            drawStartY += m_design_rowHeight;
        }
    }

    if (1)
    {
        // Item 별 Check 박스 표시
        long drawStartX = name_width;

        for (auto& group : m_frameGroups)
        {
            auto& frames = group.second.m_frames;

            for (auto& frameIndex : frames)
            {
                long drawStartY = -GetScrollPos(SB_VERT) + m_design_illumHeadHeight;

                for (auto& itemInfo : m_itemInfos)
                {
                    memDC.SetTextColor(RGB(255, 255, 255));

                    CRect column_check;
                    column_check.left = drawStartX;
                    column_check.right = drawStartX + check_sizeX;
                    column_check.top = drawStartY;
                    column_check.bottom = drawStartY + m_design_rowHeight;
                    column_check.DeflateRect(1, 1);

                    memDC.Draw3dRect(column_check, RGB(255, 255, 255), RGB(128, 128, 128));

                    column_check.DeflateRect(1, 1);
                    memDC.Draw3dRect(column_check, RGB(255, 255, 255), RGB(128, 128, 128));

                    auto& frameInfo = itemInfo.second.getFrame(frameIndex);
                    frameInfo.m_roi = column_check;

                    column_check.DeflateRect(8, 8);

                    if (frameInfo.m_checked)
                    {
                        CPen checkPen;
                        checkPen.CreatePen(PS_SOLID, 2, group.second.m_fontColor);
                        auto* oldPen = memDC.SelectObject(&checkPen);

                        memDC.MoveTo(column_check.left, column_check.CenterPoint().y);
                        memDC.LineTo(column_check.CenterPoint().x, column_check.bottom);
                        memDC.LineTo(column_check.right, column_check.top);

                        memDC.SelectObject(oldPen);
                    }

                    drawStartY += m_design_rowHeight;
                }
                drawStartX += check_sizeX;
            }
        }
    }

    if (1)
    {
        long headerLineY = 0;

        // Illum Header를 그린다
        CRect headRegion = CRect(0, 0, view.Width(), m_design_illumHeadHeight);
        memDC.FillSolidRect(headRegion, RGB(255, 79, 119));
        memDC.MoveTo(name_width - 1, 0);
        memDC.LineTo(name_width - 1, m_design_illumHeadHeight);

        for (auto& group : m_frameGroups)
        {
            auto group_region = group.second.m_roi;
            group_region.bottom = m_design_illumHeadHeight - headerLineY;

            memDC.FillSolidRect(group_region, group.second.m_backgroundColor);
        }

        long drawStartX = name_width;

        for (auto& group : m_frameGroups)
        {
            auto& frames = group.second.m_frames;

            for (long index = 0; index < long(frames.size()); index++)
            {
                auto& frameIndex = group.second.m_frames[index];
                auto& frameRect = group.second.m_frames_Rect[index];

                frameRect.left = drawStartX;
                frameRect.right = drawStartX + check_sizeX;
                frameRect.top = 0;
                frameRect.bottom = m_design_illumHeadHeight - headerLineY;
                frameRect.DeflateRect(1, 1);

                if (m_selectRealFrameIndex == frameIndex)
                {
                    memDC.FillSolidRect(frameRect, RGB(24, 43, 65));
                    memDC.SetTextColor(RGB(255, 0, 0));
                }
                else
                {
                    memDC.SetTextColor(group.second.m_fontColor);
                }

                memDC.Draw3dRect(frameRect, RGB(255, 255, 255), RGB(128, 128, 128));

                drawStartX += check_sizeX;
            }
        }

        if (m_selectRealFrameIndex >= 0)
        {
            CRect selectFrameRegion = getFrameRegion(m_selectRealFrameIndex, false);
            selectFrameRegion.top = m_design_illumHeadHeight;
            drawAlphaRectangle(memDC, selectFrameRegion, RGB(24, 43, 65), 128);
        }

        drawAlphaRectangle(memDC, headRegion, RGB(0, 0, 0), 60);

        for (auto& group : m_frameGroups)
        {
            auto& frames = group.second.m_frames;

            for (long index = 0; index < long(frames.size()); index++)
            {
                auto& frameIndex = group.second.m_frames[index];
                auto& frameRect = group.second.m_frames_Rect[index];

                if (m_selectRealFrameIndex == frameIndex)
                {
                    memDC.SetTextColor(RGB(255, 0, 0));
                }
                else
                {
                    memDC.SetTextColor(group.second.m_fontColor);
                }

                CString text;

                switch (frameIndex)
                {
                    case REVERSE_FRAME_NUM - 1:
                        text.Format(_T("RV"));
                        memDC.DrawText(text, frameRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        break;

                    case UV_FRAME_NUM - 1:
                        text.Format(_T("UV"));
                        memDC.DrawText(text, frameRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        break;

                    case IR_FRAME_NUM - 1:
                        text.Format(_T("IR"));
                        memDC.DrawText(text, frameRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        break;

                    default:
                        text.Format(_T("%d"), frameIndex + 1);
                        memDC.DrawText(text, frameRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                        break;
                }
            }
        }

        memDC.SetTextColor(RGB(255, 255, 255));
        memDC.DrawText(_T(" CATEGORY"), headRegion, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    memDC.SelectObject(oldFont);
    memDC.SetBkMode(OPAQUE);
    memDC.Draw3dRect(nameRegion, RGB(128, 128, 0), RGB(0, 0, 0));

    dc.BitBlt(0, 0, view.Width(), view.Height(), &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(oldMemBitmap);
}

void IllumControlNGRV::resetNGRVData()
{
    m_frameGroups.clear();
    m_itemInfos.clear();
    // NGRV Mode일 경우 Unused를 빼고 Frame을 10개로 고정 - JHB_NGRV
    m_realFrameCount = 0;

    if (GetSafeHwnd())
        Invalidate(FALSE);
}

bool IllumControlNGRV::addFrameGroup(long groupID, LPCTSTR name, COLORREF backgroundColor, COLORREF fontColor)
{
    if (m_frameGroups.find(groupID) != m_frameGroups.end())
    {
        ASSERT(!_T("이미 존재하는 Group 이다"));
        return false;
    }

    auto& group = m_frameGroups[groupID];
    group.m_name = name;
    group.m_fontColor = fontColor;
    group.m_backgroundColor = backgroundColor;

    if (GetSafeHwnd())
        Invalidate(FALSE);
    return true;
}

bool IllumControlNGRV::setFrameGroup(long frameIndex, long groupID)
{
    if (m_frameGroups.find(groupID) == m_frameGroups.end())
    {
        return false;
    }

    // 이상 없나 체크
    for (auto& group : m_frameGroups)
    {
        if (group.first > groupID)
            continue;

        if (group.second.m_frames.size())
        {
            if (group.second.m_frames[group.second.m_frames.size() - 1] > frameIndex)
            {
                // Group 별로 frame들은 겹치지 않아야 하며,
                // 항상 다음 Group은 이상 Group보다 높은 FrameIndex를 가져야 한다 (이전 규칙이다..)

                ASSERT(!_T("이전 Group 중 더 큰 Frame Index를 가진 녀석이 있다"));
                return FALSE;
            }
        }
    }

    m_frameGroups[groupID].m_frames.push_back(frameIndex);
    m_frameGroups[groupID].m_frames_Rect.push_back(CRect(0, 0, 0, 0));
    Invalidate(FALSE);
    return true;
}

bool IllumControlNGRV::addItem(long itemID, LPCTSTR name, bool onlyOneFrameCanBeUsed)
{
    if (m_itemInfos.find(itemID) != m_itemInfos.end())
    {
        ASSERT(!_T("이미 존재하는 item 이다"));
        return false;
    }

    auto& info = m_itemInfos[itemID];
    info.m_name = name;
    info.m_onlyOneFrameCanBeUsed = onlyOneFrameCanBeUsed;
    info.initFrame(LED_ILLUM_FRAME_MAX /*m_maxFrameCount*/);

    CRect view;
    GetClientRect(view);
    long scrollY = (m_design_rowHeight * long(m_itemInfos.size()) + m_design_illumHeadHeight) - view.Height();

    ModifyStyle(0, WS_VSCROLL, SWP_DRAWFRAME);
    SetScrollRange(SB_VERT, 0, max(0, scrollY), TRUE);
    ShowScrollBar(SB_VERT, scrollY >= 0);

    Invalidate(FALSE);
    return true;
}

bool IllumControlNGRV::addItemFrame(long itemID, long frameIndex)
{
    if (m_itemInfos.find(itemID) == m_itemInfos.end())
    {
        return false;
    }

    auto& item = m_itemInfos[itemID];
    item.checkFrame(frameIndex, true);
    m_realFrameCount = max(m_realFrameCount, frameIndex + 1);

    Invalidate(FALSE);
    return true;
}

long IllumControlNGRV::getItemCount() const
{
    return long(m_itemInfos.size());
}

void IllumControlNGRV::setEditMode(bool enabled)
{
    m_isEditMode = enabled;
    Invalidate(FALSE);
}

void IllumControlNGRV::setCallback_changedSelectFrame(LPVOID userData, CALLBACK_CHANGED_SELECT_FRAME function)
{
    m_callback_changedSelectFrame = std::make_pair(userData, function);
}

void IllumControlNGRV::setCallback_changedSelectItem(LPVOID userData, CALLBACK_CHANGED_SELECT_ITEM function)
{
    m_callback_changedSelectItem = std::make_pair(userData, function);
}

void IllumControlNGRV::setCallback_changedItemFrame(LPVOID userData, CALLBACK_CHANGED_ITEM_FRAME function)
{
    m_callback_changedItemFrame = std::make_pair(userData, function);
}

void IllumControlNGRV::setCallback_deletedFrame(LPVOID userData, CALLBACK_DELETED_FRAME function)
{
    m_callback_deletedFrame = std::make_pair(userData, function);
}

void IllumControlNGRV::setCallback_insertedFrame(LPVOID userData, CALLBACK_INSERTED_FRAME function)
{
    m_callback_insertedFrame = std::make_pair(userData, function);
}

void IllumControlNGRV::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int step = 1;
    int Mstep = 5;
    int nNext, nmin, nmax;

    GetScrollRange(SB_VERT, &nmin, &nmax);

    switch (nSBCode)
    {
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            nNext = nPos;
            if (nNext > nmax)
                nNext = nmax;
            else if (nNext < nmin)
                nNext = nmin;
            SetScrollPos(SB_VERT, nNext);
            break;
        case SB_LINEDOWN:
            nNext = GetScrollPos(SB_VERT);
            if (nNext + step <= nmax)
            {
                SetScrollPos(SB_VERT, nNext + step);
                nNext += step;
            }
            break;
        case SB_LINEUP:
            nNext = GetScrollPos(SB_VERT);
            if (nNext - step >= nmin)
            {
                SetScrollPos(SB_VERT, nNext - step);
                nNext -= step;
            }
            break;
        case SB_PAGEDOWN:
            nNext = GetScrollPos(SB_VERT);
            if (nNext + Mstep <= nmax)
            {
                SetScrollPos(SB_VERT, nNext + Mstep);
                nNext += Mstep;
            }
            else
            {
                nNext = nmax;
                SetScrollPos(SB_VERT, nNext);
            }
            break;
        case SB_PAGEUP:
            nNext = GetScrollPos(SB_VERT);
            if (nNext - Mstep >= nmin)
            {
                SetScrollPos(SB_VERT, nNext - Mstep);
                nNext -= Mstep;
            }
            else
            {
                nNext = nmin;
                SetScrollPos(SB_VERT, nNext);
            }
            break;
        default:
            return;
            break;
    }

    Invalidate(FALSE);
    CButton::OnVScroll(nSBCode, nPos, pScrollBar);
}

void IllumControlNGRV::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_dragMousePoint = point;
    m_dragAction = false;
    SetCapture();

    checkFrameRegion(point, m_dragGroupIndex, m_dragFrameIndex, false);

    CButton::OnLButtonDown(nFlags, point);
}

void IllumControlNGRV::OnMouseMove(UINT nFlags, CPoint point)
{
    CButton::OnMouseMove(nFlags, point);
}

void IllumControlNGRV::OnLButtonUp(UINT nFlags, CPoint point)
{
    long dropGroupIndex = -1;
    long dropFrameIndex = -1;

    checkFrameRegion(point, dropGroupIndex, dropFrameIndex, true);

    if (m_dragFrameIndex >= 0)
    {
        if (dropGroupIndex >= 0)
        {
            if (m_dragFrameIndex == dropFrameIndex)
            {
                setEvent_changedSelectFrame(m_dragFrameIndex);

                ////	항목에 하나라도 체크가 되어있으면 Frame을 넣는다 - JHB_NGRV
                //for (auto& itemInfo : m_itemInfos)
                //{
                //	for (long index1 = 0; index1 < itemInfo.second.getFrameCount(); index1++)
                //	{
                //		auto& frameInfo1 = itemInfo.second.getFrame(index1);

                //		if (frameInfo1.m_roi.PtInRect(point))
                //		{
                //			bool bChecked = false;

                //			m_itemInfos[itemInfo.first].checkFrame(index1, bChecked);

                //			if (bChecked)
                //			{
                //				setEvent_insertFrame(dropGroupIndex, dropFrameIndex, bChecked);
                //			}
                //			break;
                //		}
                //	}
                //}
            }
        }
    }
    else
    {
        for (auto& itemInfo : m_itemInfos)
        {
            if (itemInfo.second.m_name_Rect.PtInRect(point))
            {
                setEvent_changedSelectItem(itemInfo.first);
                break;
            }
        }

        for (auto& itemInfo : m_itemInfos)
        {
            for (long index1 = 0; index1 < itemInfo.second.getFrameCount(); index1++)
            {
                auto& frameInfo1 = itemInfo.second.getFrame(index1);

                if (frameInfo1.m_roi.PtInRect(point))
                {
                    if (itemInfo.second.m_onlyOneFrameCanBeUsed)
                    {
                        // 모든 Frame Check 해체
                        for (long index2 = 0; index2 < itemInfo.second.getFrameCount(); index2++)
                        {
                            auto& frameInfo2 = itemInfo.second.getFrame(index2);
                            if (frameInfo2.m_checked)
                            {
                                setEvent_changedItemFrame(itemInfo.first, index2, false);
                            }
                        }
                    }

                    if (itemInfo.second.getCheckedFrameCount() != 1 || !frameInfo1.m_checked)
                    {
                        setEvent_changedItemFrame(itemInfo.first, index1, !frameInfo1.m_checked);
                        setEvent_changedSelectItem(itemInfo.first);
                        setEvent_changedSelectFrame(index1);
                    }
                    break;
                }
            }
        }
    }

    m_dragFrameIndex = -1;
    ReleaseCapture();

    CButton::OnLButtonUp(nFlags, point);
}

BOOL IllumControlNGRV::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    int nmin, nmax;

    GetScrollRange(SB_VERT, &nmin, &nmax);
    auto scrollPos = GetScrollPos(SB_VERT);

    if (zDelta != 0)
    {
        SetScrollPos(SB_VERT, max(0, min(nmax, scrollPos - zDelta / 10)));
        Invalidate(FALSE);
    }

    return CButton::OnMouseWheel(nFlags, zDelta, pt);
}

void IllumControlNGRV::setEvent_changedSelectItem(long itemID)
{
    m_selectItemID = itemID;

    if (m_callback_changedSelectItem.first)
    {
        long firstRealFrameIndex = -1;

        auto itemInfo = m_itemInfos.find(itemID);

        if (itemInfo != m_itemInfos.end())
        {
            firstRealFrameIndex = itemInfo->second.getFirstFrameIndex();
        }

        m_callback_changedSelectItem.second(m_callback_changedSelectItem.first, m_selectItemID);
        setEvent_changedSelectFrame(firstRealFrameIndex);
    }

    Invalidate(FALSE);
}

void IllumControlNGRV::setEvent_changedSelectFrame(long realFrameIndex)
{
    m_selectRealFrameIndex = realFrameIndex;

    if (m_callback_changedSelectFrame.first)
    {
        m_callback_changedSelectFrame.second(m_callback_changedSelectFrame.first, m_selectRealFrameIndex);
    }

    Invalidate(FALSE);
}

void IllumControlNGRV::setEvent_changedItemFrame(long itemID, long realFrameIndex, bool enabled)
{
    if (m_itemInfos.find(itemID) == m_itemInfos.end())
        return;

    m_itemInfos[itemID].checkFrame(realFrameIndex, enabled);

    if (m_callback_changedItemFrame.first)
    {
        m_callback_changedItemFrame.second(m_callback_changedItemFrame.first, itemID, realFrameIndex, enabled);
    }

    Invalidate(FALSE);
}

long IllumControlNGRV::findFrameGroupID(long frameIndex)
{
    for (auto& frameGroup : m_frameGroups)
    {
        for (long check = 0; check < long(frameGroup.second.m_frames.size()); check++)
        {
            auto index = frameGroup.second.m_frames[check];
            if (index == frameIndex)
            {
                return frameGroup.first;
            }
        }
    }

    return -1;
}

CRect IllumControlNGRV::getFrameRegion(long frameIndex, bool isColumnOnly)
{
    for (auto& frameGroup : m_frameGroups)
    {
        for (long check = 0; check < long(frameGroup.second.m_frames.size()); check++)
        {
            auto index = frameGroup.second.m_frames[check];
            if (index != frameIndex)
                continue;

            CRect frameRegion = frameGroup.second.m_frames_Rect[check];

            if (!isColumnOnly)
            {
                CRect view;
                GetClientRect(view);
                frameRegion.top = view.top;
                frameRegion.bottom = view.bottom;
                return frameRegion;
            }

            return frameRegion;
        }
    }

    return CRect(0, 0, 0, 0);
}

void IllumControlNGRV::checkFrameRegion(const CPoint& mousePos, long& o_groupIndex, long& o_frameIndex, bool checkOnlyX)
{
    o_groupIndex = -1;
    o_frameIndex = -1;

    for (auto& frameGroup : m_frameGroups)
    {
        for (long index = 0; index < long(frameGroup.second.m_frames_Rect.size()); index++)
        {
            auto& frameRect = frameGroup.second.m_frames_Rect[index];
            if ((checkOnlyX && mousePos.x >= frameRect.left && mousePos.x < frameRect.right)
                || frameRect.PtInRect(mousePos))
            {
                o_groupIndex = frameGroup.first;
                o_frameIndex = frameGroup.second.m_frames[index];
                return;
            }
        }
    }

    for (auto& frameGroup : m_frameGroups)
    {
        if (frameGroup.second.m_roi.PtInRect(mousePos))
        {
            o_groupIndex = frameGroup.first;
            return;
        }
    }
}

void IllumControlNGRV::drawRowText(
    CDC& dc, long startOffsetX, long startOffsetY, long charSizeX, long charStepY, COLORREF color, LPCTSTR text)
{
    CString textStr = text;

    CRect charRegion(startOffsetX, startOffsetY, startOffsetX + charSizeX, startOffsetY + charStepY);

    dc.SetTextColor(color);

    for (long charIndex = 0; charIndex < textStr.GetLength(); charIndex++)
    {
        dc.DrawText(CString(textStr.GetAt(charIndex)), charRegion, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        charRegion.OffsetRect(0, charStepY);
    }
}

void IllumControlNGRV::drawAlphaRectangle(CDC& dc, const CRect& region, COLORREF color, BYTE alphaBlend)
{
    CDC memDC2;
    CBitmap memBitmap2;
    memDC2.CreateCompatibleDC(&dc);
    memBitmap2.CreateCompatibleBitmap(&dc, region.Width(), region.Height());
    auto* oldMemBitmap2 = memDC2.SelectObject(&memBitmap2);

    memDC2.FillSolidRect(0, 0, region.Width(), region.Height(), color);

    BLENDFUNCTION blend;
    blend.AlphaFormat = 0;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = alphaBlend;

    dc.AlphaBlend(region.left, region.top, region.Width(), region.Height(), &memDC2, 0, 0, region.Width(),
        region.Height(), blend);

    memDC2.SelectObject(oldMemBitmap2);
}

void IllumControlNGRV::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CButton::OnLButtonDblClk(nFlags, point);
}