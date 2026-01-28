//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SimpleRibbonBar.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define RIBBON_ICON_SIZE (32)
#define RIBBON_BACKGROUND_COLOR RGB(195, 202, 217)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(SimpleRibbonBar, CStatic)

SimpleRibbonBar::~SimpleRibbonBar()
{
    delete m_resourceBitmap;
}

bool SimpleRibbonBar::Create(CWnd* parent)
{
    return CStatic::Create(
        L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SS_NOTIFY, CRect(0, 0, 1, 1), parent);
}

bool SimpleRibbonBar::LoadResourceBitmap(UINT resourceID, const wchar_t* resourceType)
{
    if (m_resourceBitmap)
    {
        delete m_resourceBitmap;
        m_resourceBitmap = nullptr;
    }

    HRSRC hRes = FindResource(nullptr, MAKEINTRESOURCE(resourceID), resourceType);
    if (!hRes)
    {
        return false;
    }

    DWORD size = SizeofResource(nullptr, hRes);
    HGLOBAL hGlobal = LoadResource(nullptr, hRes);
    if (!hGlobal)
    {
        return false;
    }

    void* pData = LockResource(hGlobal);
    if (!pData)
    {
        return false;
    }

    // GDI+는 IStream에서 로드할 수 있음
    HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hBuffer)
    {
        return false;
    }

    void* pBuffer = ::GlobalLock(hBuffer);
    memcpy(pBuffer, pData, size);
    ::GlobalUnlock(hBuffer);

    IStream* pStream = nullptr;
    if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) != S_OK)
    {
        GlobalFree(hBuffer);
        return false;
    }

    auto* bitmap = Gdiplus::Bitmap::FromStream(pStream);
    pStream->Release(); // Bitmap 내부에서 스트림 복사함
    m_resourceBitmap = bitmap;
    return true;
}

void SimpleRibbonBar::CreateLayerBitmap()
{
    if (m_resourceBitmap == nullptr)
    {
        return; // 리소스 비트맵이 로드되지 않았으면 아무것도 그리지 않음
    }

    CRect viewRect;
    GetClientRect(viewRect);
    CDC* pDC = GetDC();
    if (pDC == nullptr)
    {
        return; // DC를 가져오지 못하면 아무것도 그리지 않음
    }

    if (m_layerBitmap.m_hObject)
    {
        m_layerBitmap.DeleteObject();
    }

    m_layerBitmap.CreateCompatibleBitmap(pDC, viewRect.Width(), viewRect.Height());

    CDC memDC;
    memDC.CreateCompatibleDC(pDC);
    CBitmap* pOldBitmap = memDC.SelectObject(&m_layerBitmap);

    memDC.FillSolidRect(viewRect, RIBBON_BACKGROUND_COLOR); // 배경색 설정
    memDC.SelectObject(GetParent()->GetFont()); // 폰트 설정
    memDC.SetBkMode(TRANSPARENT);
    memDC.SelectStockObject(NULL_BRUSH);

    int32_t x = 0;

    for (auto group : m_groups)
    {
        auto& groupRect = group->m_position;
        groupRect = CRect(x, 2, x, viewRect.bottom - 2);
        x += 5;

        for (auto button : group->m_buttons)
        {
            auto& buttonRect = button->m_position;
            buttonRect.left = x;
            buttonRect.top = groupRect.top + 1;
            buttonRect.right = x + button->m_width;
            buttonRect.bottom = groupRect.bottom - 20;
            x += button->m_width + 2; // 버튼 간격 추가
        }

        x += 5;

        groupRect.right = x;
        auto groupText = CRect(groupRect.left, groupRect.bottom - 20, groupRect.right, groupRect.bottom);
        groupText.DeflateRect(1, 1);

        memDC.SelectStockObject(WHITE_PEN);
        memDC.Rectangle(groupRect);
        memDC.FillSolidRect(groupText, RGB(128, 128, 128));
        memDC.SetTextColor(RGB(255, 255, 255));
        memDC.DrawText(group->m_name.c_str(), groupText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        x += 5; // 그룹 간격 추가

        for (auto button : group->m_buttons)
        {
            DrawButton(memDC, *button);
        }
    }

    memDC.SelectObject(pOldBitmap);
    ReleaseDC(pDC);
}
void SimpleRibbonBar::DrawButton(CDC& dc, ButtonInfo& button)
{
    dc.SetBkMode(TRANSPARENT);
    dc.SelectStockObject(NULL_BRUSH);
    dc.SelectObject(GetParent()->GetFont()); // 부모 폰트 사용

    const auto& buttonRect = button.m_position;

    // 기존 버튼 그림을 지우고 배경색으로 채움
    dc.FillSolidRect(buttonRect, RIBBON_BACKGROUND_COLOR);

    if (m_activatedButton == &button)
    {
        if (m_clickedButton == m_activatedButton)
        {
            // 클릭된 버튼은 배경색을 강조 표시
            dc.FillSolidRect(buttonRect, RGB(255, 255, 255)); // 노란색으로 강조
        }
        else
        {
            // 활성화된 버튼은 배경색을 약간 어둡게 표시
            dc.FillSolidRect(buttonRect, RGB(200, 200, 200)); // 회색으로 강조
        }
        // 활성화된 버튼은 테두리를 강조 표시
        dc.SelectStockObject(BLACK_PEN);
        dc.Rectangle(buttonRect);
    }

    CRect textRect = buttonRect;
    textRect.top = buttonRect.bottom - 40; // 텍스트 영역 높이 설정
    textRect.bottom = buttonRect.bottom - 2; // 텍스트 영역 높이 설정

    CRect imageRect = buttonRect;
    imageRect.top = buttonRect.top + 2;
    imageRect.bottom = textRect.top - 2;

    auto imageScale = imageRect.Height() / (float)RIBBON_ICON_SIZE;

    imageRect.left = (int32_t)(buttonRect.CenterPoint().x - RIBBON_ICON_SIZE / 2 * imageScale);
    imageRect.right = (int32_t)(imageRect.left + RIBBON_ICON_SIZE * imageScale);

    DrawResourceWithAlpha(dc, imageRect, button.m_imageId, (button.m_enabled ? 1.f : 0.2f));

    dc.SetTextColor(button.m_enabled ? RGB(0, 0, 128) : RGB(128, 128, 128));
    dc.DrawText(button.m_name.c_str(), textRect, DT_CENTER | DT_VCENTER);
}

void SimpleRibbonBar::DrawResourceWithAlpha(CDC& dc, const CRect& targetRect, int32_t imageID, float alpha)
{
    if (m_resourceBitmap == nullptr)
    {
        return; // 리소스 비트맵이 로드되지 않았으면 아무것도 그리지 않음
    }

    // GDI+ Graphics 생성
    Gdiplus::Graphics graphics(dc.GetSafeHdc());

    // 알파 블렌딩 품질 향상 설정 (선택사항)
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver); // 알파 사용
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

    Gdiplus::RectF dstRect(
        (float)targetRect.left, (float)targetRect.top, (float)targetRect.Width(), (float)targetRect.Height());
    Gdiplus::RectF srcRect(
        (float)imageID * RIBBON_ICON_SIZE, (float)0, (float)RIBBON_ICON_SIZE, (float)RIBBON_ICON_SIZE);

    // Draw
    if (alpha == 1.f)
    {
        graphics.DrawImage(m_resourceBitmap, dstRect, srcRect, Gdiplus::UnitPixel);
    }
    else
    {
        // 전체 알파 조정용 ColorMatrix 설정
        Gdiplus::ColorMatrix colorMatrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, alpha, 0.0f, // <== 알파값 조정!
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

        Gdiplus::ImageAttributes imageAttr;
        imageAttr.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

        graphics.DrawImage(m_resourceBitmap, dstRect, srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height,
            Gdiplus::UnitPixel, &imageAttr);
    }
}

SimpleRibbonBar::ButtonInfo* SimpleRibbonBar::FindButton(CPoint point)
{
    SimpleRibbonBar::ButtonInfo* selectedButton = nullptr;
    for (auto& group : m_groups)
    {
        for (auto& button : group->m_buttons)
        {
            if (!button->m_enabled)
            {
                continue;
            }

            if (!button->m_position.PtInRect(point))
            {
                continue;
            }

            selectedButton = button.get();
        }
    }

    return selectedButton;
}

void SimpleRibbonBar::UpdateLayout()
{
    CreateLayerBitmap();
    Invalidate(FALSE);
}

int32_t SimpleRibbonBar::AddGroup(const wchar_t* groupName)
{
    auto group = std::make_shared<GroupInfo>();
    group->m_name = groupName;

    m_groups.push_back(group);
    UpdateLayout();

    return (int32_t)m_groups.size();
}

bool SimpleRibbonBar::AddButton(int32_t groupId, int32_t imageId, const wchar_t* name, int32_t width, int32_t id)
{
    auto groupIndex = groupId - 1;

    if (groupIndex < 0 || groupIndex >= (int32_t)m_groups.size())
    {
        return false;
    }
    auto group = m_groups[groupIndex];

    auto info = std::make_shared<ButtonInfo>();

    info->m_name = name;
    info->m_width = width;
    info->m_imageId = imageId;
    info->m_id = id;
    group->m_buttons.push_back(info);
    UpdateLayout();

    return true;
}

void SimpleRibbonBar::CheckButtons()
{
    CDC* pDC = GetDC();
    if (pDC == nullptr)
    {
        return; // DC를 가져오지 못하면 아무것도 그리지 않음
    }

    CDC memDC;
    memDC.CreateCompatibleDC(pDC);
    CBitmap* pOldBitmap = memDC.SelectObject(&m_layerBitmap);

    bool changed = false;
    for (auto& group : m_groups)
    {
        for (auto& button : group->m_buttons)
        {
            bool isEnabled = false;
            if (m_callbackButtonCheck)
            {
                m_callbackButtonCheck(button->m_id, isEnabled);
            }

            if (button->m_enabled != isEnabled)
            {
                button->m_enabled = isEnabled;
                DrawButton(memDC, *button);
                changed = true;
            }
        }
    }

    memDC.SelectObject(pOldBitmap);

    if (changed)
    {
        Invalidate(FALSE);
    }
}

void SimpleRibbonBar::StartCheckButtonTimer(int32_t intervalMs)
{
    SetTimer(1, intervalMs, nullptr);
}

void SimpleRibbonBar::StopCheckButtonTimer()
{
    KillTimer(1);
}

void SimpleRibbonBar::SetCallbackButtonCheck(std::function<void(int32_t, bool&)> callback)
{
    m_callbackButtonCheck = callback;
}

void SimpleRibbonBar::SetCallbackButtonClicked(std::function<void(int32_t)> callback)
{
    m_callbackButtonClicked = callback;
}

void SimpleRibbonBar::DoDataExchange(CDataExchange* pDX)
{
    CStatic::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(SimpleRibbonBar, CStatic)
ON_WM_SIZE()
ON_WM_TIMER()
ON_WM_PAINT()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// ViewMenu 메시지 처리기

void SimpleRibbonBar::OnSize(UINT nType, int cx, int cy)
{
    CStatic::OnSize(nType, cx, cy);

    UpdateLayout();
}

void SimpleRibbonBar::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1) // 타이머 ID 확인
    {
        CheckButtons();
    }
    CStatic::OnTimer(nIDEvent);
}
void SimpleRibbonBar::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect viewRect;
    GetClientRect(viewRect);

    if (m_layerBitmap.m_hObject != nullptr)
    {
        CDC memDC;
        memDC.CreateCompatibleDC(&dc);
        CBitmap* pOldBitmap = memDC.SelectObject(&m_layerBitmap);
        dc.BitBlt(0, 0, viewRect.Width(), viewRect.Height(), &memDC, 0, 0, SRCCOPY);
        memDC.SelectObject(pOldBitmap);
    }
}

void SimpleRibbonBar::OnMouseMove(UINT nFlags, CPoint point)
{
    auto selectedButton = FindButton(point);

    if (selectedButton == m_activatedButton)
    {
        CStatic::OnMouseMove(nFlags, point);
        return;
    }
    auto oldButton = m_activatedButton;
    m_activatedButton = selectedButton;

    CDC* pDC = GetDC();
    CDC memDC;
    memDC.CreateCompatibleDC(pDC);
    CBitmap* pOldBitmap = memDC.SelectObject(&m_layerBitmap);

    if (oldButton != nullptr)
    {
        // 이전 버튼을 다시 그려서 원래 상태로 되돌림
        DrawButton(memDC, *oldButton);
    }

    if (m_activatedButton != nullptr)
    {
        DrawButton(memDC, *m_activatedButton);
    }

    memDC.SelectObject(pOldBitmap);
    ReleaseDC(pDC);

    Invalidate(FALSE);
    CStatic::OnMouseMove(nFlags, point);
}

void SimpleRibbonBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_activatedButton != nullptr && m_activatedButton->m_enabled)
    {
        m_clickedButton = m_activatedButton;
        CDC* pDC = GetDC();
        CDC memDC;
        memDC.CreateCompatibleDC(pDC);
        CBitmap* pOldBitmap = memDC.SelectObject(&m_layerBitmap);
        DrawButton(memDC, *m_activatedButton);
        memDC.SelectObject(pOldBitmap);
        ReleaseDC(pDC);

        SetCapture();
        Invalidate(FALSE);
    }

    CStatic::OnLButtonDown(nFlags, point);
}

void SimpleRibbonBar::OnLButtonUp(UINT nFlags, CPoint point)
{
    ReleaseCapture(); // 마우스 캡처 해제

    if (m_activatedButton != nullptr && m_clickedButton == m_activatedButton)
    {
        m_clickedButton = nullptr;
        CDC* pDC = GetDC();
        CDC memDC;
        memDC.CreateCompatibleDC(pDC);
        CBitmap* pOldBitmap = memDC.SelectObject(&m_layerBitmap);
        DrawButton(memDC, *m_activatedButton); // 버튼 상태를 다시 그리기
        memDC.SelectObject(pOldBitmap);
        ReleaseDC(pDC);
        Invalidate(FALSE); // 화면 갱신

        if (m_callbackButtonClicked)
        {
            m_callbackButtonClicked(m_activatedButton->m_id);
        }

        CheckButtons(); // 버튼 상태 업데이트
        CStatic::OnLButtonUp(nFlags, point);

        return;
    }

    m_clickedButton = nullptr;

    if (m_activatedButton != nullptr)
    {
        CDC* pDC = GetDC();
        CDC memDC;
        memDC.CreateCompatibleDC(pDC);
        CBitmap* pOldBitmap = memDC.SelectObject(&m_layerBitmap);
        DrawButton(memDC, *m_activatedButton); // 버튼 상태를 다시 그리기
        memDC.SelectObject(pOldBitmap);
        ReleaseDC(pDC);
        Invalidate(FALSE); // 화면 갱신
    }

    CStatic::OnLButtonUp(nFlags, point);
}
