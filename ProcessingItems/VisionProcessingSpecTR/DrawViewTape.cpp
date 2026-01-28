//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DrawViewTape.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//

CRect ToCRect(Ipvm::Rect32r& rect)
{
    return CRect((int32_t)rect.m_left, (int32_t)rect.m_top, (int32_t)rect.m_right, (int32_t)rect.m_bottom);
}

bool DrawViewTape::Create(CWnd* parentWnd, const CRect& rect)
{
    return CStatic::Create(L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_OWNERDRAW, rect, parentWnd) != NULL;
}

void DrawViewTape::Set(const VisionTapeSpec& spec)
{
    m_spec = spec;
    Invalidate(); // 다시 그리기
}
BEGIN_MESSAGE_MAP(DrawViewTape, CStatic)
END_MESSAGE_MAP()

void DrawViewTape::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CDC dc;
    dc.Attach(lpDrawItemStruct->hDC);
    CRect rc = lpDrawItemStruct->rcItem;

    float drawSizeX = m_spec.m_w0;
    float drawSizeY = m_spec.m_p1 * 3.f; // 최대 Pocket Pitch x 3만큼 그리기
    float scale = 1.f;
    if (true)
    {
        float scaleX = rc.Width() / drawSizeX;
        float scaleY = rc.Height() / drawSizeY;

        // scaleX와 scaleY 중 작은 값을 선택하여 비율을 유지
        scale = min(scaleX, scaleY);
    }

    float tapeSizeX_px = drawSizeX * scale; // 실제 그릴 크기 (픽셀 단위)
    float tapeSizeY_px = drawSizeY * scale; // 실제 그릴 크기 (픽셀 단위)

    // 드로잉 영역을 중앙에 위치시키기 위해 오프셋 계산
    float offsetX = (rc.Width() - tapeSizeX_px) / 2;
    float offsetY = (rc.Height() - tapeSizeY_px) / 2;
    // 배경색 설정
    dc.FillSolidRect(rc, RGB(0, 0, 0)); // 검은 배경

    // 테이프 그리기
    Ipvm::Rect32r tapeRect(offsetX, offsetY, offsetX + tapeSizeX_px, offsetY + tapeSizeY_px);
    dc.FillSolidRect(ToCRect(tapeRect), RGB(200, 200, 200)); // 회색 테이프

    // 테이프 내의 포켓 위치 그리기
    auto firstPocketY = offsetY + m_spec.m_p1 * scale * 0.5f; // 첫번째 포켓 위치 (반 Pitch 위치)
    float pocketCenterX = rc.Width() * 0.5f;
    float lineHalfWidth = 1.f;

    switch (m_spec.m_sprocketHoleExistType)
    {
        case TapeSprocketHoleExistType::e_both:
        {
            float holeX = tapeRect.m_right - m_spec.m_e1 * scale; // 테이프 오른쪽 끝에서부터 홈의 시작 위치

            // Hole Guard
            Ipvm::Rect32r holeGuardL = tapeRect;
            Ipvm::Rect32r holeGuardR = tapeRect;
            holeGuardR.m_left = holeX - m_spec.m_f1 * scale;
            holeGuardL.m_right = holeGuardR.m_left - m_spec.m_s1 * scale;
            dc.FillSolidRect(ToCRect(holeGuardL), RGB(128, 128, 128));
            dc.FillSolidRect(ToCRect(holeGuardR), RGB(128, 128, 128));

            // Line
            Ipvm::Rect32r lineR = tapeRect;
            lineR.m_left = holeX - m_spec.m_f2 * scale - lineHalfWidth;
            lineR.m_right = holeX - m_spec.m_f2 * scale + lineHalfWidth;
            Ipvm::Rect32r lineL = lineR;
            lineL.m_left -= m_spec.m_s2 * scale;
            lineL.m_right -= m_spec.m_s2 * scale;
            dc.FillSolidRect(ToCRect(lineL), RGB(0, 0, 0));
            dc.FillSolidRect(ToCRect(lineR), RGB(0, 0, 0));

            pocketCenterX = holeX - m_spec.m_f0 * scale; // 포켓의 중심 X 위치
            DrawHoles(dc, holeX, firstPocketY, scale);
            holeX -= m_spec.m_s0 * scale; // 왼쪽 홈 위치로 이동
            DrawHoles(dc, holeX, firstPocketY, scale);
            break;
        }
        case TapeSprocketHoleExistType::e_rightOnly:
        {
            float holeX = tapeRect.m_right - m_spec.m_e1 * scale; // 테이프 오른쪽 끝에서부터 홈의 시작 위치

            // Hole Guard
            Ipvm::Rect32r holeGuardL = tapeRect;
            Ipvm::Rect32r holeGuardR = tapeRect;
            holeGuardR.m_left = holeX - m_spec.m_f1 * scale;
            holeGuardL.m_right = holeGuardR.m_left - m_spec.m_s1 * scale;
            dc.FillSolidRect(ToCRect(holeGuardL), RGB(128, 128, 128));
            dc.FillSolidRect(ToCRect(holeGuardR), RGB(128, 128, 128));

            // Line
            Ipvm::Rect32r lineR = tapeRect;
            lineR.m_left = holeX - m_spec.m_f2 * scale - lineHalfWidth;
            lineR.m_right = holeX - m_spec.m_f2 * scale + lineHalfWidth;
            Ipvm::Rect32r lineL = lineR;
            lineL.m_left -= m_spec.m_s2 * scale;
            lineL.m_right -= m_spec.m_s2 * scale;
            dc.FillSolidRect(ToCRect(lineL), RGB(0, 0, 0));
            dc.FillSolidRect(ToCRect(lineR), RGB(0, 0, 0));

            pocketCenterX = holeX - m_spec.m_f0 * scale; // 포켓의 중심 X 위치
            DrawHoles(dc, holeX, firstPocketY, scale);
            break;
        }

        case TapeSprocketHoleExistType::e_leftOnly:
        {
            float holeX = tapeRect.m_left + m_spec.m_e1 * scale; // 테이프 왼쪽 끝에서부터 홈의 시작 위치

            // Hole Guard
            Ipvm::Rect32r holeGuardL = tapeRect;
            Ipvm::Rect32r holeGuardR = tapeRect;
            holeGuardL.m_right = holeX + m_spec.m_f1 * scale;
            holeGuardR.m_left = holeGuardL.m_right + m_spec.m_s1 * scale;
            dc.FillSolidRect(ToCRect(holeGuardL), RGB(128, 128, 128));
            dc.FillSolidRect(ToCRect(holeGuardR), RGB(128, 128, 128));

            // Line
            Ipvm::Rect32r lineL = tapeRect;
            lineL.m_left = holeX + m_spec.m_f2 * scale - lineHalfWidth;
            lineL.m_right = holeX + m_spec.m_f2 * scale + lineHalfWidth;
            Ipvm::Rect32r lineR = lineL;
            lineR.m_left += m_spec.m_s2 * scale;
            lineR.m_right += m_spec.m_s2 * scale;
            dc.FillSolidRect(ToCRect(lineL), RGB(0, 0, 0));
            dc.FillSolidRect(ToCRect(lineR), RGB(0, 0, 0));

            pocketCenterX = holeX + m_spec.m_f0 * scale; // 포켓의 중심 X 위치
            DrawHoles(dc, holeX, firstPocketY, scale);
            break;
        }
    }

    for (int pocketIndex = 0; pocketIndex < 3; ++pocketIndex)
    {
        // 포켓의 Y 위치 계산
        float pocketSizeX = m_spec.m_b0 * scale; // 포켓의 너비
        float pocketSizeY = m_spec.m_a0 * scale; // 포켓의 높 (가로로 그리기)
        float pocketPosY = firstPocketY + (pocketIndex * m_spec.m_p1 * scale);

        // 포켓 그리기
        Ipvm::Rect32r pocketRect(pocketCenterX - (pocketSizeX) / 2, pocketPosY - (pocketSizeY) / 2,
            pocketCenterX + (pocketSizeX) / 2, pocketPosY + (pocketSizeY) / 2);
        dc.FillSolidRect(ToCRect(pocketRect), RGB(96, 96, 96)); // 포켓
        auto* oldBrush = dc.SelectStockObject(NULL_BRUSH);
        dc.Rectangle(ToCRect(pocketRect));
        dc.SelectObject(oldBrush);

        // 포켓 내의 Circle 그리기
        float circleRadius = m_spec.m_d0 * scale / 2; // Circle의 반지름
        Ipvm::Rect32r circleRect(pocketCenterX - circleRadius, pocketPosY - circleRadius, pocketCenterX + circleRadius,
            pocketPosY + circleRadius);
        dc.Ellipse(ToCRect(circleRect)); // Circle 그리기
    }

    // 정리
    dc.Detach();
}

void DrawViewTape::DrawHoles(CDC& dc, float holeCenterX_px, float firstPocketY_px, float scale)
{
    CRect clientRect;
    GetClientRect(clientRect);
    auto cy = clientRect.Height();

    // First Pocket Y 로부터 P0 만큼씩 위로 올라가서 화면을 벗어나는 지점을 찾는다.
    float holePitchY_px = m_spec.m_p0 * scale;
    float holeY_px = firstPocketY_px - holePitchY_px * 0.5f; // 반 Pitch 차이가 난다

    while (holeY_px > 0)
    {
        holeY_px -= holePitchY_px; // 위로 올라감
    }

    // 화면 안에 들어오는 모든 홈을 그린다
    CPen pen;
    pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    auto* oldPen = dc.SelectObject(&pen);

    // 홈의 크기 계산
    float holeSizeX_px = m_spec.m_d1 * scale; // 홈의 너비
    float holeSizeY_px = m_spec.m_d1 * scale; // 홈의 높
    while (holeY_px - holeSizeY_px / 2 < cy)
    {
        // 홈 그리기
        // 빨간 색으로 홈을 그린다
        Ipvm::Rect32r holeRect(holeCenterX_px - (holeSizeX_px / 2), holeY_px - (holeSizeY_px / 2),
            holeCenterX_px + (holeSizeX_px / 2), holeY_px + (holeSizeY_px / 2));
        dc.Ellipse(ToCRect(holeRect));
        // 다음 Hole 위치로 이동
        holeY_px += holePitchY_px;
    }

    dc.SelectObject(oldPen); // 이전 펜으로 복원
}
