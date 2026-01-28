#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h> //kircheis_NGRVINLINE
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageViewEx;

//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionInspectionOverlayResult
{
public:
    VisionInspectionOverlayResult();
    ~VisionInspectionOverlayResult();

    void Reset();
    void Apply(ImageViewEx* display);
    void AddOverlay(const VisionInspectionOverlayResult& overlay);

    void SetImage(const Ipvm::Image8u& image);
    void SetImage(const Ipvm::Image8u3& image); //kircheis_NGRVINLINE
    void SetThresholdImage(const Ipvm::Image8u& image);

    void OverlayReset();
    void AddCrPoint(const Ipvm::Point32r2& pos, COLORREF color);
    void AddLine(const Ipvm::Point32r2& start, const Ipvm::Point32r2& end, COLORREF color);
    void AddRectangles(const std::vector<Ipvm::Rect32r>& objects, COLORREF color);
    void AddRectangles(const std::vector<Ipvm::Rect32s>& objects, COLORREF color);
    void AddRectangle(const Ipvm::Rect32r& object, COLORREF color);
    void AddRectangle(const Ipvm::Rect32s& object, COLORREF color);
    void AddRectangle(const PI_RECT& object, COLORREF color);
    void AddEllipse(const Ipvm::Rect32r& object, COLORREF color);
    void AddEllipse(const Ipvm::EllipseEq32r& object, COLORREF color);
    void AddPolygon(const Ipvm::Polygon32r& object, COLORREF color);
    void AddText(const Ipvm::Point32s2& lt, LPCTSTR text, COLORREF color, float fontSize = 10.f);

private:
    Ipvm::Image8u m_image;
    Ipvm::Image8u3 m_imageColor; //kircheis_NGRVINLINE
    Ipvm::Image8u m_imageThreshold;

    struct CrPoint
    {
        Ipvm::Point32r2 m_pos;
        COLORREF m_color;

        CrPoint(const Ipvm::Point32r2& pos, COLORREF color);
    };

    struct Line
    {
        Ipvm::LineSeg32r m_line;
        COLORREF m_color;

        Line(const Ipvm::Point32r2& start, const Ipvm::Point32r2& end, COLORREF color);
    };

    struct Rect
    {
        Ipvm::Rect32r m_rect;
        COLORREF m_color;

        Rect(const Ipvm::Rect32r& rect, COLORREF color);
    };

    struct Rect2
    {
        Ipvm::Quadrangle32r m_rect;
        COLORREF m_color;

        Rect2(const PI_RECT& rect, COLORREF color);
    };

    struct Text
    {
        Ipvm::Point32s2 m_lt;
        CString m_text;
        COLORREF m_color;
        float m_fontSize;

        Text(const Ipvm::Point32s2& lt, LPCTSTR text, COLORREF color, float fontSize);
    };

    struct Polygon
    {
        Ipvm::Polygon32r m_object;
        COLORREF m_color;

        Polygon(const Ipvm::Polygon32r& object, COLORREF color);
    };

    std::vector<CrPoint> m_list_crPoint;
    std::vector<Line> m_list_line;
    std::vector<Rect> m_list_rect;
    std::vector<Rect2> m_list_pi_rect;
    std::vector<Rect> m_list_ellipse;
    std::vector<Polygon> m_list_polygon;
    std::vector<Text> m_list_text;
};
