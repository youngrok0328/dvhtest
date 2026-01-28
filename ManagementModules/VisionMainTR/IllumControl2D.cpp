//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "IllumControl2D.h"

//CPP_2_________________________________ This project's headers
#include "VisionMainTR.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GROUP_ID_UNUSED (9999)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(IllumControl2D, CButton)

void IllumControl2D::ItemInfo::initFrame(long maxFrameCount)
{
    m_useFrames.resize(maxFrameCount);

    for (long index = 0; index < maxFrameCount; index++)
    {
        m_useFrames[index].m_checked = false;
    }
}

long IllumControl2D::ItemInfo::getFrameCount() const
{
    return long(m_useFrames.size());
}

long IllumControl2D::ItemInfo::getCheckedFrameCount() const
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

IllumControl2D::SItemFrameInfo& IllumControl2D::ItemInfo::getFrame(long index)
{
    return m_useFrames[index];
}

void IllumControl2D::ItemInfo::checkFrame(long frameIndex, bool enabled)
{
    m_useFrames[frameIndex].m_checked = enabled;
}

void IllumControl2D::ItemInfo::incFrameIndex(long baseFrameIndex)
{
    for (long index = long(m_useFrames.size()) - 1; index >= baseFrameIndex + 1; index--)
    {
        m_useFrames[index] = m_useFrames[index - 1];
    }

    m_useFrames[baseFrameIndex].m_checked = false;
}

void IllumControl2D::ItemInfo::eraseFrame(long frameIndex, bool frameShift)
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

long IllumControl2D::ItemInfo::getFirstFrameIndex() const
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

IllumControl2D::IllumControl2D(long maxFrameCount)
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

    resetData();
}

IllumControl2D::~IllumControl2D()
{
}

BEGIN_MESSAGE_MAP(IllumControl2D, CButton)
ON_WM_PAINT()
ON_WM_VSCROLL()
ON_WM_LBUTTONDOWN()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONUP()
ON_WM_MOUSEWHEEL()
ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

// IllumControl2D message handlers

void IllumControl2D::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect view;
    GetClientRect(view);

    CDC memDC;
    CBitmap memBitmap;
    memDC.CreateCompatibleDC(&dc);
    memBitmap.CreateCompatibleBitmap(&dc, view.Width(), view.Height());
    auto* oldMemBitmap = memDC.SelectObject(&memBitmap);

    //{{ SystemConfig를 미리 설정 - JHB
    auto& SystemConfig = SystemConfig::GetInstance();
    //}}

    // 먼저 검사항목을 그린다
    long group_marginX = 20;
    long checkRegionX = m_maxFrameCount * m_design_rowHeight;
    long name_width = 0;

    if (SystemConfig.GetVisionType()
        != VISIONTYPE_NGRV_INSP) //2021.03.12_JHB, NGRV일 때는 Unuse를 제외한 나머지만 할당 해야함
    {
        name_width = view.Width() - checkRegionX - long(m_frameGroups.size()) * group_marginX * 2;
    }
    else // Group이 하나 사라졌으니 group_marginX 방향만큼 한 번만 곱해줘서 빼야함
    {
        name_width = view.Width() - checkRegionX - (long(m_frameGroups.size()) - 1) * group_marginX;
    }

    CRect nameRegion(0, m_design_illumHeadHeight, name_width, view.Height());
    memDC.FillSolidRect(nameRegion, RGB(144, 177, 220));

    long check_sizeX = checkRegionX / m_maxFrameCount;

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
            auto& frames = group.second.m_frames;

            auto& group_region = group.second.m_roi;

            group_region.left = drawStartX;
            group_region.right = drawStartX + check_sizeX * long(frames.size()) + group_marginX * 2;
            group_region.top = 0;
            group_region.bottom = view.bottom;

            if (group.first == GROUP_ID_UNUSED)
            {
                // 마지막 GROUP은 가로를 끝까지 다쓰자.
                group_region.right = view.right;
            }

            memDC.FillSolidRect(group_region, group.second.m_backgroundColor);
            drawRowText(memDC, drawStartX, m_design_illumHeadHeight + 16, group_marginX, 16, group.second.m_fontColor,
                group.second.m_name);

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
            drawStartX += group_marginX;

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

            drawStartX += group_marginX;
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
            drawStartX += group_marginX;

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

            drawStartX += group_marginX;
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
                text.Format(_T("%d"), frameIndex + 1);
                memDC.DrawText(text, frameRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                text.Empty();
            }
        }

        memDC.SetTextColor(RGB(255, 255, 255));
        memDC.DrawText(_T(" CATEGORY"), headRegion, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    memDC.SelectObject(oldFont);
    memDC.SetBkMode(OPAQUE);
    memDC.Draw3dRect(nameRegion, RGB(128, 128, 0), RGB(0, 0, 0));

    if (m_dragAction && SystemConfig.GetVisionType() != VISIONTYPE_NGRV_INSP)
    {
        // Drag 중인 Frame을 마우스 위치에 반 투명하게 표시한다
        CRect dragFrameRegion = getFrameRegion(m_dragFrameIndex, false);

        CDC memDC2;
        CBitmap memBitmap2;
        memDC2.CreateCompatibleDC(&dc);
        memBitmap2.CreateCompatibleBitmap(&dc, dragFrameRegion.Width(), dragFrameRegion.Height());
        auto* oldMemBitmap2 = memDC2.SelectObject(&memBitmap2);

        memDC2.BitBlt(0, 0, dragFrameRegion.Width(), dragFrameRegion.Height(), &memDC, dragFrameRegion.left,
            dragFrameRegion.top, SRCCOPY);

        BLENDFUNCTION blend;
        blend.AlphaFormat = 0;
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = 156;

        memDC.AlphaBlend(m_dragMousePoint.x, 0, dragFrameRegion.Width(), dragFrameRegion.Height(), &memDC2, 0, 0,
            dragFrameRegion.Width(), dragFrameRegion.Height(), blend);

        memDC2.SelectObject(oldMemBitmap2);
    }

    dc.BitBlt(0, 0, view.Width(), view.Height(), &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(oldMemBitmap);
}

void IllumControl2D::resetData()
{
    m_frameGroups.clear();
    m_itemInfos.clear();
    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_NGRV_INSP)
    {
        m_realFrameCount = 0;

        addFrameGroup(GROUP_ID_UNUSED, _T("unused"), RGB(100, 100, 100), RGB(255, 255, 255));
    }
    else // NGRV Mode일 경우 Unused를 빼고 Frame을 10개로 고정 - JHB_NGRV
    {
        m_realFrameCount = 10;
    }

    if (GetSafeHwnd())
        Invalidate(FALSE);
}

bool IllumControl2D::addFrameGroup(long groupID, LPCTSTR name, COLORREF backgroundColor, COLORREF fontColor)
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

bool IllumControl2D::setFrameGroup(long frameIndex, long groupID)
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

    if (SystemConfig::GetInstance().GetVisionType()
        != VISIONTYPE_NGRV_INSP) // NGRV Mode일 경우 Unuse 사용 안함 - JHB_NGRV
    {
        bool existFrame = false;

        for (auto& itemInfo : m_itemInfos)
        {
            if (itemInfo.second.getFrame(frameIndex).m_checked)
            {
                existFrame = true;
                break;
            }
        }

        if (!existFrame)
        {
            // 사용되지 않는 Frame이다 Group을 강제로 UNUSED로 바꾸어 준다
            groupID = GROUP_ID_UNUSED;
        }
    }

    m_frameGroups[groupID].m_frames.push_back(frameIndex);
    m_frameGroups[groupID].m_frames_Rect.push_back(CRect(0, 0, 0, 0));
    Invalidate(FALSE);
    return true;
}

bool IllumControl2D::addItem(long itemID, LPCTSTR name, bool onlyOneFrameCanBeUsed)
{
    if (m_itemInfos.find(itemID) != m_itemInfos.end())
    {
        ASSERT(!_T("이미 존재하는 item 이다"));
        return false;
    }

    auto& info = m_itemInfos[itemID];
    info.m_name = name;
    info.m_onlyOneFrameCanBeUsed = onlyOneFrameCanBeUsed;
    info.initFrame(m_maxFrameCount);

    CRect view;
    GetClientRect(view);
    long scrollY = (m_design_rowHeight * long(m_itemInfos.size()) + m_design_illumHeadHeight) - view.Height();

    ModifyStyle(0, WS_VSCROLL, SWP_DRAWFRAME);
    SetScrollRange(SB_VERT, 0, max(0, scrollY), TRUE);
    ShowScrollBar(SB_VERT, scrollY >= 0);

    Invalidate(FALSE);
    return true;
}

bool IllumControl2D::addItemFrame(long itemID, long frameIndex)
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

long IllumControl2D::getItemCount() const
{
    return long(m_itemInfos.size());
}

void IllumControl2D::setEditMode(bool enabled)
{
    m_isEditMode = enabled;
    Invalidate(FALSE);
}

void IllumControl2D::setCallback_changedSelectFrame(LPVOID userData, CALLBACK_CHANGED_SELECT_FRAME function)
{
    m_callback_changedSelectFrame = std::make_pair(userData, function);
}

void IllumControl2D::setCallback_changedSelectItem(LPVOID userData, CALLBACK_CHANGED_SELECT_ITEM function)
{
    m_callback_changedSelectItem = std::make_pair(userData, function);
}

void IllumControl2D::setCallback_changedItemFrame(LPVOID userData, CALLBACK_CHANGED_ITEM_FRAME function)
{
    m_callback_changedItemFrame = std::make_pair(userData, function);
}

void IllumControl2D::setCallback_deletedFrame(LPVOID userData, CALLBACK_DELETED_FRAME function)
{
    m_callback_deletedFrame = std::make_pair(userData, function);
}

void IllumControl2D::setCallback_insertedFrame(LPVOID userData, CALLBACK_INSERTED_FRAME function)
{
    m_callback_insertedFrame = std::make_pair(userData, function);
}

void IllumControl2D::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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

void IllumControl2D::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_dragMousePoint = point;
    m_dragAction = false;
    SetCapture();

    checkFrameRegion(point, m_dragGroupIndex, m_dragFrameIndex, false);

    CButton::OnLButtonDown(nFlags, point);
}

void IllumControl2D::OnMouseMove(UINT nFlags, CPoint point)
{
    if (SystemConfig::GetInstance().GetVisionType()
        != VISIONTYPE_NGRV_INSP) // NGRV 일 때는 Mouse Move 사용 안함 - JHB_NGRV
    {
        if (m_dragFrameIndex >= 0)
        {
            if (!m_dragAction && (abs(m_dragMousePoint.x - point.x) > 10 || abs(m_dragMousePoint.y - point.y) > 10))
            {
                m_dragAction = true;
            }

            if (m_dragAction)
            {
                m_dragMousePoint = point;
                Invalidate(FALSE);
            }
        }
    }

    CButton::OnMouseMove(nFlags, point);
}

void IllumControl2D::OnLButtonUp(UINT nFlags, CPoint point)
{
    long dropGroupIndex = -1;
    long dropFrameIndex = -1;

    checkFrameRegion(point, dropGroupIndex, dropFrameIndex, true);

    if (m_dragFrameIndex >= 0
        && SystemConfig::GetInstance().GetVisionType()
            != VISIONTYPE_NGRV_INSP) // NGRV 일 때는 Frame을 이동할 이유가 없음 - JHB_NGRV
    {
        if (dropGroupIndex >= 0)
        {
            if (m_dragAction)
            {
                bool unused = false;
                if (dropGroupIndex == GROUP_ID_UNUSED)
                    unused = true;
                if (m_dragGroupIndex == GROUP_ID_UNUSED)
                    unused = true;

                if (dropFrameIndex >= 0)
                {
                    // 특정 Item 앞에 Insert하였다
                    if (dropFrameIndex != m_dragFrameIndex)
                    {
                        if (dropFrameIndex > m_dragFrameIndex)
                        {
                            // Drag Position에 있는 Frame은 제거될 것이므로 Drop 위치가 바뀔거다
                            //dropFrameIndex--;
                        }
                        // Drag Frame을 백업하여 Drop 포지션에 넣는다
                        setEvent_deletedFrame(m_dragGroupIndex, m_dragFrameIndex, !unused);
                        setEvent_insertFrame(dropGroupIndex, dropFrameIndex, !unused);
                    }
                }
                else
                {
                    // Drag Frame을 백업하여 Drop 포지션에 넣는다
                    setEvent_deletedFrame(m_dragGroupIndex, m_dragFrameIndex, !unused);
                    setEvent_insertFrame(dropGroupIndex, -1, !unused);
                }

                if (unused)
                {
                    // 기존 Frame을 쓰레기통에 넣었는데
                    // 혹시 Frame이 설정되어 있던 놈을 버렸는데 검사항목중
                    // 선택된 Frame이 하나도 없는 경우 첫번째 것이라도 선택하여 주자
                    for (auto& itemInfo : m_itemInfos)
                    {
                        long firstFrame = itemInfo.second.getFirstFrameIndex();
                        if (firstFrame < 0)
                        {
                            setEvent_changedItemFrame(itemInfo.first, 0, true);
                        }
                    }
                }
            }
            else
            {
                if (m_dragFrameIndex == dropFrameIndex)
                {
                    setEvent_changedSelectFrame(m_dragFrameIndex);
                }
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

                if (findFrameGroupID(index1) == GROUP_ID_UNUSED)
                    continue;

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

BOOL IllumControl2D::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
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

void IllumControl2D::setEvent_changedSelectItem(long itemID)
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

void IllumControl2D::setEvent_changedSelectFrame(long realFrameIndex)
{
    m_selectRealFrameIndex = realFrameIndex;

    if (m_callback_changedSelectFrame.first)
    {
        m_callback_changedSelectFrame.second(m_callback_changedSelectFrame.first, m_selectRealFrameIndex);
    }

    Invalidate(FALSE);
}

void IllumControl2D::setEvent_changedItemFrame(long itemID, long realFrameIndex, bool enabled)
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

void IllumControl2D::setEvent_deletedFrame(long groupIndex, long realFrameIndex, bool isBackup)
{
    if (isBackup)
    {
        // 현재 정보를 백업한다
        m_backupFrame.clear();

        for (auto& itemInfo : m_itemInfos)
        {
            if (itemInfo.second.getFrame(realFrameIndex).m_checked)
            {
                m_backupFrame[itemInfo.first] = TRUE;
            }
        }
    }

    // Frame Group 정보를 지운다
    for (auto& frameGroup : m_frameGroups)
    {
        for (long check = 0; check < long(frameGroup.second.m_frames.size()); check++)
        {
            auto& groupInfo = frameGroup.second;
            if (groupInfo.m_frames[check] == realFrameIndex)
            {
                groupInfo.m_frames.erase(groupInfo.m_frames.begin() + check);
                if (groupInfo.m_frames_Rect.size())
                {
                    groupInfo.m_frames_Rect.erase(groupInfo.m_frames_Rect.begin() + check);
                }

                check--;
                if (groupIndex != GROUP_ID_UNUSED)
                {
                    m_realFrameCount--;
                }
            }
            else if (groupInfo.m_frames[check] > realFrameIndex)
            {
                groupInfo.m_frames[check]--;
            }
        }
    }

    // Item에서 Frame 정보를 지운다
    for (auto& itemInfo : m_itemInfos)
    {
        itemInfo.second.eraseFrame(realFrameIndex, true);
    }

    Invalidate(FALSE);

    if (groupIndex != GROUP_ID_UNUSED)
    {
        if (m_callback_deletedFrame.first)
        {
            m_callback_deletedFrame.second(m_callback_deletedFrame.first, realFrameIndex, isBackup);
        }
    }
}

void IllumControl2D::setEvent_insertFrame(long groupIndex, long realFrameIndex, bool isBackup)
{
    if (realFrameIndex < 0)
    {
        // 다음 그룹의 첫번째 위치에 넣어주자
        for (auto& frameGroup : m_frameGroups)
        {
            if (groupIndex < frameGroup.first)
            {
                if (frameGroup.second.m_frames.size())
                {
                    realFrameIndex = frameGroup.second.m_frames[0];
                    break;
                }
            }
        }

        if (realFrameIndex < 0)
        {
            realFrameIndex = m_realFrameCount;
        }
    }

    long insertIndex = -1;
    for (auto& frameGroup : m_frameGroups)
    {
        for (long check = 0; check < long(frameGroup.second.m_frames.size()); check++)
        {
            if (groupIndex == frameGroup.first && frameGroup.second.m_frames[check] == realFrameIndex)
            {
                insertIndex = check;
            }

            if (frameGroup.second.m_frames[check] >= realFrameIndex)
            {
                frameGroup.second.m_frames[check]++;
            }
        }
    }

    for (auto& frameGroup : m_frameGroups)
    {
        if (frameGroup.first != groupIndex)
            continue;

        if (insertIndex < 0)
        {
            if (frameGroup.second.m_frames_Rect.size() == frameGroup.second.m_frames.size())
            {
                frameGroup.second.m_frames_Rect.push_back(CRect(0, 0, 0, 0));
            }

            frameGroup.second.m_frames.push_back(realFrameIndex);

            if (groupIndex != GROUP_ID_UNUSED)
            {
                m_realFrameCount++;
            }
        }
        else
        {
            if (frameGroup.second.m_frames_Rect.size() == frameGroup.second.m_frames.size())
            {
                frameGroup.second.m_frames_Rect.push_back(CRect(0, 0, 0, 0));
            }

            frameGroup.second.m_frames.insert(frameGroup.second.m_frames.begin() + insertIndex, realFrameIndex);

            if (groupIndex != GROUP_ID_UNUSED)
            {
                m_realFrameCount++;
            }
        }
    }

    // Item에서 Frame을 집어넣을 수 있게 기존 Index를 Shift 시킨다
    for (auto& itemInfo : m_itemInfos)
    {
        itemInfo.second.incFrameIndex(realFrameIndex);
    }

    if (isBackup)
    {
        for (auto& itemInfo : m_itemInfos)
        {
            if (m_backupFrame.find(itemInfo.first) != m_backupFrame.end())
            {
                itemInfo.second.checkFrame(realFrameIndex, true);
            }
        }
    }

    Invalidate(FALSE);

    if (groupIndex != GROUP_ID_UNUSED)
    {
        if (m_callback_insertedFrame.first)
        {
            m_callback_insertedFrame.second(m_callback_insertedFrame.first, groupIndex, realFrameIndex, isBackup);
        }

        setEvent_changedSelectFrame(realFrameIndex);
    }
    else
    {
        setEvent_changedSelectFrame(-1);
    }
}

long IllumControl2D::findFrameGroupID(long frameIndex)
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

CRect IllumControl2D::getFrameRegion(long frameIndex, bool isColumnOnly)
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

void IllumControl2D::checkFrameRegion(const CPoint& mousePos, long& o_groupIndex, long& o_frameIndex, bool checkOnlyX)
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

void IllumControl2D::drawRowText(
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

    textStr.Empty();
}

void IllumControl2D::drawAlphaRectangle(CDC& dc, const CRect& region, COLORREF color, BYTE alphaBlend)
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

void IllumControl2D::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if (::GetKeyState(VK_CONTROL) < 0)
    {
        long groupIndex = -1;
        long frameIndex = -1;

        checkFrameRegion(point, groupIndex, frameIndex, false);
        if (frameIndex >= 0)
        {
            if (groupIndex != GROUP_ID_UNUSED)
            {
                // Unused Group으로 넣는다
                setEvent_deletedFrame(groupIndex, frameIndex, false);
                setEvent_insertFrame(GROUP_ID_UNUSED, -1, false);
            }
            else
            {
                // 0번 Group으로 넣어준다
                setEvent_deletedFrame(groupIndex, frameIndex, false);
                setEvent_insertFrame(0, -1, false);
            }
        }
    }

    CButton::OnLButtonDblClk(nFlags, point);
}
