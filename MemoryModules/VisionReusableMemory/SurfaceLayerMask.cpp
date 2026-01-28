//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceLayerMask.h"

//CPP_2_________________________________ This project's headers
#include "VisionReusableMemory.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/ConversionEx.h"
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/BlobInfo.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/Rectangle32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ROTATE_MASK_BINARIZATION_THRESHOLD (240)

//CPP_7_________________________________ Implementation body
//
SurfaceLayerMask::SurfaceLayerMask(VisionReusableMemory* memory)
    : SurfaceLayerRoi(memory)
{
}

SurfaceLayerMask::~SurfaceLayerMask()
{
}

bool SurfaceLayerMask::Ready(const Ipvm::Point32r2& bodyCenter)
{
    if (!SurfaceLayerRoi::Ready(bodyCenter))
    {
        return false;
    }

    if (!m_memory->GetInspByteImage(m_maskImage))
    {
        return false;
    }

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(m_maskImage), 0, m_maskImage);

    return true;
}

void SurfaceLayerMask::Reset()
{
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(m_maskImage), 0, m_maskImage);
}

bool SurfaceLayerMask::CopyFrom(const Ipvm::Image8u& image)
{
    if (image.GetSizeX() == m_maskImage.GetSizeX() && image.GetSizeY() == m_maskImage.GetSizeY())
        return Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), m_maskImage) == Ipvm::Status::e_ok;

    return ResizeFrom(image);
}

bool SurfaceLayerMask::ResizeFrom(const Ipvm::Image8u& image)
{
    return Ipvm::ImageProcessing::ResizeLinearInterpolation(image, m_maskImage) == Ipvm::Status::e_ok;
}

bool SurfaceLayerMask::Add(const Ipvm::Point32r2& center, const Ipvm::EllipseEq32r& objectShape, bool fill)
{
    auto object_new = objectShape;
    object_new.Offset(center.m_x, center.m_y);

    if (Ipvm::ImageProcessing::Fill(object_new, fill ? 255 : 0, m_maskImage) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SurfaceLayerMask::Add(const Ipvm::Point32r2& center, const Ipvm::Polygon32r& objectShape, bool fill)
{
    auto object_new = objectShape;
    object_new.Offset(center.m_x, center.m_y);

    if (Ipvm::ImageProcessing::Fill(object_new, fill ? 255 : 0, m_maskImage) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SurfaceLayerMask::Add(const Ipvm::EllipseEq32r& objectShape, bool fill)
{
    if (Ipvm::ImageProcessing::Fill(objectShape, fill ? 255 : 0, m_maskImage) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SurfaceLayerMask::Add(const Ipvm::Polygon32r& object, bool fill)
{
    if (Ipvm::ImageProcessing::Fill(object, fill ? 255 : 0, m_maskImage) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SurfaceLayerMask::Add(const FPI_RECT& object, bool fill)
{
    Ipvm::Quadrangle32r object_new(ConversionEx::ToQuadrangle32r(object));

    //{{//kircheis_20230216
    const long nImageSizeX = m_maskImage.GetSizeX();
    const long nImageSizeY = m_maskImage.GetSizeY();

    if (object_new.m_lbX < 0 || object_new.m_lbX >= nImageSizeX || object_new.m_lbY < 0
        || object_new.m_lbY >= nImageSizeY || object_new.m_ltX < 0 || object_new.m_ltX >= nImageSizeX
        || object_new.m_ltY < 0 || object_new.m_ltY >= nImageSizeY || object_new.m_rbX < 0
        || object_new.m_rbX >= nImageSizeX || object_new.m_rbY < 0 || object_new.m_rbY >= nImageSizeY
        || object_new.m_rtX < 0 || object_new.m_rtX >= nImageSizeX || object_new.m_rtY < 0
        || object_new.m_rtY >= nImageSizeY)
        return false;
    //}}

    if (Ipvm::ImageProcessing::Fill(object_new, fill ? 255 : 0, m_maskImage) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SurfaceLayerMask::Add(const PI_RECT& object, bool fill)
{
    Ipvm::Quadrangle32r object_new(ConversionEx::ToQuadrangle32r(object));

    if (Ipvm::ImageProcessing::Fill(object_new, fill ? 255 : 0, m_maskImage) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SurfaceLayerMask::Add(const std::vector<PI_RECT>& objects, bool fill)
{
    for (auto& object : objects)
    {
        if (!Add(object, fill))
        {
            return false;
        }
    }

    return true;
}

bool SurfaceLayerMask::Add(const Ipvm::Image8u& mask)
{
    if (mask.GetSizeX() != m_maskImage.GetSizeX() || mask.GetSizeY() != m_maskImage.GetSizeY())
    {
        ASSERT(!_T("입력 Mask Image는 출력 영상과 크기가 같아야 한다."));
        return false;
    }
    if (Ipvm::ImageProcessing::BinarizeGreater(
            mask, Ipvm::Rect32s(mask), ROTATE_MASK_BINARIZATION_THRESHOLD, m_maskImage)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SurfaceLayerMask::Add(const Ipvm::Image8u& mask, float i_fAngleParameter_deg)
{
    if (mask.GetSizeX() != m_maskImage.GetSizeX() || mask.GetSizeY() != m_maskImage.GetSizeY())
    {
        ASSERT(!_T("입력 Mask Image는 출력 영상과 크기가 같아야 한다."));
        return false;
    }

    Ipvm::Point32r2 offset;
    offset.m_x = (mask.GetSizeX() * 0.5f) - m_bodyCenter.m_x;
    offset.m_y = (mask.GetSizeY() * 0.5f) - m_bodyCenter.m_y;

    Ipvm::Image8u newMask_Gray;
    if (!m_memory->GetByteImage(newMask_Gray, m_maskImage.GetSizeX(), m_maskImage.GetSizeY()))
    {
        return false;
    }

    if (Ipvm::ImageProcessing::RotateLinearInterpolation(
            mask, Ipvm::Rect32s(mask), m_bodyCenter, i_fAngleParameter_deg, offset, newMask_Gray)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::BinarizeGreater(
            newMask_Gray, Ipvm::Rect32s(newMask_Gray), ROTATE_MASK_BINARIZATION_THRESHOLD, m_maskImage)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SurfaceLayerMask::Add(
    const Ipvm::Image8u& mask, const Ipvm::Point32r2& center, const Ipvm::Point32r2& bodyCenter, float markAngle_deg)
{
    Ipvm::Point32r2 mask_center(mask.GetSizeX() * 0.5f, mask.GetSizeY() * 0.5f);
    Ipvm::Rectangle32r rectangle(mask_center.m_x, mask_center.m_y, float(mask.GetSizeX()), float(mask.GetSizeY()),
        (float)(-markAngle_deg * IV_DEG_TO_RAD));
    Ipvm::Rect32s newMaskROI = Ipvm::Conversion::ToRect32s(rectangle);

    Ipvm::Image8u newMask_Gray;
    Ipvm::Image8u newMask_Bin;

    if (!m_memory->GetByteImage(newMask_Gray, newMaskROI.Width(), newMaskROI.Height()))
        return false;
    if (!m_memory->GetByteImage(newMask_Bin, newMaskROI.Width(), newMaskROI.Height()))
        return false;

    Ipvm::Point32r2 newMask_center(newMask_Gray.GetSizeX() * 0.5f, newMask_Gray.GetSizeY() * 0.5f);

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(newMask_Gray), 0, newMask_Gray);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(newMask_Bin), 0, newMask_Bin);

    Ipvm::Point32r2 maskOffset = newMask_center - mask_center;

    if (Ipvm::ImageProcessing::Copy(
            mask, Ipvm::Rect32s(mask), Ipvm::Rect32s(mask) + Ipvm::Conversion::ToPoint32s2(maskOffset), newMask_Bin)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::RotateLinearInterpolation(newMask_Bin, Ipvm::Rect32s(newMask_Bin), newMask_center,
            markAngle_deg, Ipvm::Point32r2(0.f, 0.f), // maskOffset,
            newMask_Gray)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::BinarizeGreater(
            newMask_Gray, Ipvm::Rect32s(newMask_Gray), ROTATE_MASK_BINARIZATION_THRESHOLD, newMask_Bin)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    Ipvm::Point32r2 rotate_center = makeSurfacePos(center, bodyCenter, markAngle_deg);
    Ipvm::Rect32s targetROI;
    targetROI.m_left = (int)(rotate_center.m_x - newMask_Bin.GetSizeX() / 2);
    targetROI.m_top = (int)(rotate_center.m_y - newMask_Bin.GetSizeY() / 2);
    targetROI.m_right = targetROI.m_left + newMask_Bin.GetSizeX();
    targetROI.m_bottom = targetROI.m_top + newMask_Bin.GetSizeY();

    Ipvm::Rect32s valid_targetROI = targetROI & Ipvm::Rect32s(m_maskImage);

    Ipvm::Rect32s sourceROI;
    sourceROI.m_left = valid_targetROI.m_left - targetROI.m_left;
    sourceROI.m_top = valid_targetROI.m_top - targetROI.m_top;
    sourceROI.m_right = sourceROI.m_left + valid_targetROI.Width();
    sourceROI.m_bottom = sourceROI.m_top + valid_targetROI.Height();

    if (valid_targetROI.IsRectEmpty())
    {
        return false;
    }

    Ipvm::Image8u src(newMask_Bin, sourceROI);
    Ipvm::Image8u dst(m_maskImage, valid_targetROI);

    if (Ipvm::ImageProcessing::BitwiseOr(src, Ipvm::Rect32s(src), dst) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SurfaceLayerMask::AddLabel(const Ipvm::Image32s& labelImage, const Ipvm::BlobInfo& blobInfo, bool fill)
{
    UNREFERENCED_PARAMETER(fill);

    Ipvm::Rect32s roi = Ipvm::Rect32s(m_maskImage) & blobInfo.m_roi;

    auto label = blobInfo.m_label;

    for (long y = roi.m_top; y < roi.m_bottom; y++)
    {
        auto* maskY = m_maskImage.GetMem(0, y);
        auto* labelY = labelImage.GetMem(0, y);
        for (long x = roi.m_left; x < roi.m_right; x++)
        {
            if (labelY[x] == label)
            {
                maskY[x] = 255;
            }
        }
    }

    return true;
}

const Ipvm::Image8u& SurfaceLayerMask::GetMask() const
{
    return m_maskImage;
}
