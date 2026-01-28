//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionOverlayResult.h"

//CPP_2_________________________________ This project's headers
#include "DevelopmentLog.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../UserInterfaceModules/ImageViewEx/PatternImage.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionOverlayResult::CrPoint::CrPoint(const Ipvm::Point32r2& pos, COLORREF color)
    : m_pos(pos)
    , m_color(color)
{
}

VisionInspectionOverlayResult::Line::Line(const Ipvm::Point32r2& start, const Ipvm::Point32r2& end, COLORREF color)
    : m_color(color)
{
    m_line.m_sx = start.m_x;
    m_line.m_sy = start.m_y;
    m_line.m_ex = end.m_x;
    m_line.m_ey = end.m_y;
}

VisionInspectionOverlayResult::Rect::Rect(const Ipvm::Rect32r& rect, COLORREF color)
    : m_rect(rect)
    , m_color(color)
{
}

VisionInspectionOverlayResult::Rect2::Rect2(const PI_RECT& rect, COLORREF color)
    : m_color(color)
{
    m_rect.m_ltX = (float)rect.ltX;
    m_rect.m_ltY = (float)rect.ltY;
    m_rect.m_rtX = (float)rect.rtX;
    m_rect.m_rtY = (float)rect.rtY;
    m_rect.m_lbX = (float)rect.blX;
    m_rect.m_lbY = (float)rect.blY;
    m_rect.m_rbX = (float)rect.brX;
    m_rect.m_rbY = (float)rect.brY;
}

VisionInspectionOverlayResult::Text::Text(const Ipvm::Point32s2& lt, LPCTSTR text, COLORREF color, float fontSize)
    : m_lt(lt)
    , m_text(text)
    , m_color(color)
    , m_fontSize(fontSize)
{
}

VisionInspectionOverlayResult::Polygon::Polygon(const Ipvm::Polygon32r& object, COLORREF color)
    : m_object(object)
    , m_color(color)
{
}

VisionInspectionOverlayResult::VisionInspectionOverlayResult()
{
}

VisionInspectionOverlayResult::~VisionInspectionOverlayResult()
{
}

void VisionInspectionOverlayResult::Reset()
{
    m_image.Free();
    m_imageColor.Free(); //kircheis_NGRVINLINE
    m_imageThreshold.Free();

    OverlayReset();
}

void VisionInspectionOverlayResult::Apply(ImageViewEx* display)
{
    if (m_imageThreshold.GetSizeX() > 0 && m_imageThreshold.GetSizeY() > 0)
    {
        PatternImage pattern;
        if (pattern.setThresholdImage(m_imageThreshold))
        {
            display->SetImage(pattern);
        }
        else
        {
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, _T("Failed to display image : Not enough memory"));
        }
    }
    else if (m_image.GetSizeX() > 0 && m_image.GetSizeY() > 0)
    {
        display->SetImage(m_image);
    }
    else if (m_imageColor.GetSizeX() > 0 && m_imageColor.GetSizeY() > 0
        && SystemConfig::GetInstance().IsVisionTypeNGRV() == TRUE) //kircheis_NGRVINLINE
    {
        display->SetImage(m_imageColor);
    }

    display->ImageOverlayClear();

    for (auto& object : m_list_crPoint)
    {
        Ipvm::Point32r2 pt = object.m_pos;

        display->ImageOverlayAdd(Ipvm::LineSeg32r(pt.m_x, pt.m_y - 5.f, pt.m_x, pt.m_y + 5.f), object.m_color, 3.f);
        display->ImageOverlayAdd(Ipvm::LineSeg32r(pt.m_x - 5.f, pt.m_y, pt.m_x + 5.f, pt.m_y), object.m_color, 3.f);
        //display->ImageOverlayAdd(object.m_pos, object.m_color, 3.f);
    }

    for (auto& object : m_list_line)
        display->ImageOverlayAdd(object.m_line, object.m_color);
    for (auto& object : m_list_rect)
        display->ImageOverlayAdd(object.m_rect, object.m_color);
    for (auto& object : m_list_pi_rect)
        display->ImageOverlayAdd(object.m_rect, object.m_color);
    for (auto& object : m_list_ellipse)
        display->ImageOverlayAdd(object.m_rect, object.m_color);
    for (auto& object : m_list_text)
        display->ImageOverlayAdd(object.m_lt, object.m_text, object.m_color, object.m_fontSize);
    for (auto& object : m_list_polygon)
    {
        display->ImageOverlayAdd(object.m_object, object.m_color);
    }

    display->ImageOverlayShow();
}

void VisionInspectionOverlayResult::AddOverlay(const VisionInspectionOverlayResult& overlay)
{
    m_list_crPoint.insert(m_list_crPoint.end(), overlay.m_list_crPoint.begin(), overlay.m_list_crPoint.end());
    m_list_line.insert(m_list_line.end(), overlay.m_list_line.begin(), overlay.m_list_line.end());
    m_list_rect.insert(m_list_rect.end(), overlay.m_list_rect.begin(), overlay.m_list_rect.end());
    m_list_pi_rect.insert(m_list_pi_rect.end(), overlay.m_list_pi_rect.begin(), overlay.m_list_pi_rect.end());
    m_list_ellipse.insert(m_list_ellipse.end(), overlay.m_list_ellipse.begin(), overlay.m_list_ellipse.end());
    m_list_polygon.insert(m_list_polygon.end(), overlay.m_list_polygon.begin(), overlay.m_list_polygon.end());
    m_list_text.insert(m_list_text.end(), overlay.m_list_text.begin(), overlay.m_list_text.end());
}

void VisionInspectionOverlayResult::SetImage(const Ipvm::Image8u& image)
{
    m_image = image;
    m_imageColor.Free(); //kircheis_NGRVINLINE
    m_imageThreshold.Free();
}

void VisionInspectionOverlayResult::SetImage(const Ipvm::Image8u3& image) //kircheis_NGRVINLINE
{
    m_image.Free(); //kircheis_NGRVINLINE
    m_imageColor = image;
    m_imageThreshold.Free();
}

void VisionInspectionOverlayResult::SetThresholdImage(const Ipvm::Image8u& image)
{
    m_image.Free();
    m_imageColor.Free(); //kircheis_NGRVINLINE
    m_imageThreshold = image;
}

void VisionInspectionOverlayResult::OverlayReset()
{
    m_list_crPoint.clear();
    m_list_line.clear();
    m_list_rect.clear();
    m_list_pi_rect.clear();
    m_list_ellipse.clear();
    m_list_text.clear();
    m_list_polygon.clear();
}

void VisionInspectionOverlayResult::AddCrPoint(const Ipvm::Point32r2& pos, COLORREF color)
{
    m_list_crPoint.emplace_back(pos, color);
}

void VisionInspectionOverlayResult::AddLine(const Ipvm::Point32r2& start, const Ipvm::Point32r2& end, COLORREF color)
{
    m_list_line.emplace_back(start, end, color);
}

void VisionInspectionOverlayResult::AddRectangles(const std::vector<Ipvm::Rect32r>& objects, COLORREF color)
{
    for (auto& object : objects)
    {
        AddRectangle(object, color);
    }
}

void VisionInspectionOverlayResult::AddRectangles(const std::vector<Ipvm::Rect32s>& objects, COLORREF color)
{
    for (auto& object : objects)
    {
        AddRectangle(object, color);
    }
}

void VisionInspectionOverlayResult::AddRectangle(const Ipvm::Rect32r& object, COLORREF color)
{
    m_list_rect.emplace_back(object, color);
}

void VisionInspectionOverlayResult::AddRectangle(const Ipvm::Rect32s& object, COLORREF color)
{
    Ipvm::Rect32r modify_object;
    modify_object.m_left = (float)object.m_left;
    modify_object.m_top = (float)object.m_top;
    modify_object.m_right = (float)object.m_right - 1;
    modify_object.m_bottom = (float)object.m_bottom - 1;

    m_list_rect.emplace_back(modify_object, color);
}

void VisionInspectionOverlayResult::AddRectangle(const PI_RECT& object, COLORREF color)
{
    m_list_pi_rect.emplace_back(object, color);
}

void VisionInspectionOverlayResult::AddEllipse(const Ipvm::Rect32r& object, COLORREF color)
{
    m_list_ellipse.emplace_back(object, color);
}

void VisionInspectionOverlayResult::AddEllipse(const Ipvm::EllipseEq32r& object, COLORREF color)
{
    Ipvm::Rect32r rect;
    rect.m_left = object.m_x - object.m_xradius;
    rect.m_top = object.m_y - object.m_yradius;
    rect.m_right = object.m_x + object.m_xradius;
    rect.m_bottom = object.m_y + object.m_yradius;

    m_list_ellipse.emplace_back(rect, color);
}

void VisionInspectionOverlayResult::AddPolygon(const Ipvm::Polygon32r& object, COLORREF color)
{
    m_list_polygon.emplace_back(object, color);
}

void VisionInspectionOverlayResult::AddText(const Ipvm::Point32s2& lt, LPCTSTR text, COLORREF color, float fontSize)
{
    m_list_text.emplace_back(lt, text, color, fontSize);
}
