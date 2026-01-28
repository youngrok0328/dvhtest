//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Result.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingAlign3D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Result::Result()
    : m_fovImageSizeX(0)
    , m_fovImageSizeY(0)
    , m_deviceLeftInImage(0.f)
    , m_deviceRightInImage(0.f)
    , m_paneOffsetX_um(0.)
    , m_paneOffsetY_um(0.)
{
}

Result::~Result()
{
}

void Result::Prepare(VisionProcessingAlign3D& process, double paneOffsetX_um, double paneOffsetY_um)
{
    m_paneOffsetX_um = paneOffsetX_um;
    m_paneOffsetY_um = paneOffsetY_um;

    // 현재 Pane ID에 대해 FOV별로 예측되는 Gerber Unit 상의 영역을 계산한다
    // 넘겨받은 정보에도 해당 정보가 있으나 정확한 FOV 크기 정보를 가지고 만든 것이 아닐 수 있을 것 같아서
    // 다시 계산하였다. 불필요하다면 추후 삭제하고 TrayScanSpec에 있는 정보를 그냥 이용하자

    auto& trayScanSpec = process.getTrayScanSpec();
    auto& areaInfo = process.getInspectionAreaInfo();
    auto& scale = process.getScale();

    int rawImageSizeX = process.getImageLot().GetImageSizeX();
    int rawImageSizeY = process.getImageLot().GetImageSizeY();
    int stitchCountX = areaInfo.m_stichCountX;
    int stitchCountY = areaInfo.m_stichCountY;
    int paneID = process.GetCurrentPaneID();
    m_fovImageSizeX = (stitchCountX <= 0) ? 0 : rawImageSizeX / stitchCountX;
    m_fovImageSizeY = (stitchCountY <= 0) ? 0 : rawImageSizeY / stitchCountY;
    m_deviceLeftInImage = 0.f;
    m_deviceRightInImage = 0.f;

    if (paneID < 0 || paneID >= (int)areaInfo.m_unitIndexList.size())
    {
        m_fovs.clear();
        return;
    }

    m_fovs.resize(stitchCountX * stitchCountY);
    auto unitID = areaInfo.m_unitIndexList[paneID];
    auto fovSizeX_mm = scale.convert_pixelToUmX((float)m_fovImageSizeX) / 1000.f;
    auto fovSizeY_mm = scale.convert_pixelToUmY((float)m_fovImageSizeY) / 1000.f;

    Ipvm::Point32r2 fovImageCenter(m_fovImageSizeX * 0.5f, m_fovImageSizeY * 0.5f);
    Ipvm::Point32r2 offset((float)trayScanSpec.GetHorPaneOffset_um(), (float)trayScanSpec.GetVerPaneOffset_um());

    for (int fovIndex = 0; fovIndex < (int)areaInfo.m_fovList.size(); fovIndex++)
    {
        int fovID = areaInfo.m_fovList[fovIndex];
        auto& fovInfo = m_fovs[fovIndex];
        int stitchX = fovIndex % stitchCountX;
        int stitchY = fovIndex % stitchCountY;

        fovInfo.m_imageRoi = Ipvm::Rect32s(stitchX * m_fovImageSizeX, stitchY * m_fovImageSizeY,
            (stitchX + 1) * m_fovImageSizeX, (stitchY + 1) * m_fovImageSizeY);

        auto specUnitRect = trayScanSpec.m_vecUnits[unitID].m_position;
        auto fovCenter = trayScanSpec.GetFovCenter(fovID, m_paneOffsetX_um, m_paneOffsetY_um);

        fovInfo.m_fovRoi_mm.m_left = fovCenter.m_x - fovSizeX_mm * 0.5f;
        fovInfo.m_fovRoi_mm.m_top = fovCenter.m_y - fovSizeY_mm * 0.5f;
        fovInfo.m_fovRoi_mm.m_right = fovCenter.m_x + fovSizeX_mm * 0.5f;
        fovInfo.m_fovRoi_mm.m_bottom = fovCenter.m_y + fovSizeY_mm * 0.5f;

        fovInfo.m_validUnitRoi_mm = (specUnitRect & fovInfo.m_fovRoi_mm);

        fovInfo.m_deviceImageRoi = (specUnitRect - fovCenter) * scale.mmToPixel() + fovImageCenter;

        if (fovIndex == 0)
        {
            m_deviceLeftInImage = scale.convert_mmToPixelX(specUnitRect.m_left - fovCenter.m_x) + fovImageCenter.m_x;
            m_deviceRightInImage = scale.convert_mmToPixelX(specUnitRect.m_right - fovCenter.m_x) + fovImageCenter.m_x;
        }
    }
}
