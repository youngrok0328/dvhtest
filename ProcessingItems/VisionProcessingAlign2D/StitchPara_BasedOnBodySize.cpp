//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "StitchPara_BasedOnBodySize.h"

//CPP_2_________________________________ This project's headers
#include "Constants.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
EdgeSearchRoiPara::EdgeSearchRoiPara()
{
    Initialize();
}

void EdgeSearchRoiPara::Initialize()
{
    for (long direction = 0; direction < 2; direction++)
    {
        m_searchLength_um[direction] = 3000.f;
        m_persent_start[direction] = 0.f;
        m_persent_end[direction] = 100.f;
    }
}

BOOL EdgeSearchRoiPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("X - Search Length")].Link(bSave, m_searchLength_um[0]);
    db[_T("X - Percent Start")].Link(bSave, m_persent_start[0]);
    db[_T("X - Percent End")].Link(bSave, m_persent_end[0]);
    db[_T("Y - Search Length")].Link(bSave, m_searchLength_um[1]);
    db[_T("Y - Percent Start")].Link(bSave, m_persent_start[1]);
    db[_T("Y - Percent End")].Link(bSave, m_persent_end[1]);

    return TRUE;
}

void EdgeSearchRoiPara::MakeResult(
    const VisionScale& scale, const Constants& constants, long searchDirection, long stitchIndex)
{
    if (constants.m_stitchCountX > 2 || constants.m_stitchCountY > 2)
    {
        m_resultRois.clear();
        return;
    }

    m_resultRois.resize(2);

    float overlapMarginX = scale.convert_umToPixelX(constants.m_foverlapUmX * 0.5f);
    float overlapMarginY = scale.convert_umToPixelY(constants.m_foverlapUmY * 0.5f);
    float halfPackageSizeX_px = constants.m_packageSizeX_px * 0.5f;
    float halfPackageSizeY_px = constants.m_packageSizeY_px * 0.5f;
    float left = constants.m_rawImageCenter.m_x - halfPackageSizeX_px - overlapMarginX;
    float right = constants.m_rawImageCenter.m_x + halfPackageSizeX_px + overlapMarginX;
    float top = constants.m_rawImageCenter.m_y - halfPackageSizeY_px - overlapMarginY;
    float bottom = constants.m_rawImageCenter.m_y + halfPackageSizeY_px + overlapMarginY;

    float lineX_SY = constants.m_rawImageCenter.m_y - halfPackageSizeY_px;
    float lineX_EY = constants.m_rawImageCenter.m_y + halfPackageSizeY_px;
    float lineY_SX = constants.m_rawImageCenter.m_x - halfPackageSizeX_px;
    float lineY_EX = constants.m_rawImageCenter.m_x + halfPackageSizeX_px;

    if (constants.m_stitchCountY == 2)
    {
        if (stitchIndex / constants.m_stitchCountX)
        {
            lineX_SY = constants.m_rawImageCenter.m_y;
            lineX_EY = constants.m_rawImageCenter.m_y + halfPackageSizeY_px + overlapMarginY;
        }
        else
        {
            lineX_SY = constants.m_rawImageCenter.m_y - halfPackageSizeY_px - overlapMarginY;
            lineX_EY = constants.m_rawImageCenter.m_y;
        }
    }

    if (constants.m_stitchCountX == 2)
    {
        if (stitchIndex % constants.m_stitchCountY)
        {
            lineY_SX = constants.m_rawImageCenter.m_x;
            lineY_EX = constants.m_rawImageCenter.m_x + halfPackageSizeX_px + overlapMarginX;
        }
        else
        {
            lineY_SX = constants.m_rawImageCenter.m_x - halfPackageSizeX_px - overlapMarginX;
            lineY_EX = constants.m_rawImageCenter.m_x;
        }
    }

    if (true)
    {
        //-----------------------------------------------------------------------------------
        // X Line 찾기 ROI
        //-----------------------------------------------------------------------------------

        bool isLeft = ((stitchIndex % constants.m_stitchCountX) == 0);
        long lineX_findPositionX = (long)(isLeft ? left : right);

        float width = lineX_EY - lineX_SY;
        float start = lineX_SY + width * m_persent_start[0] / 100.f;
        float end = lineX_SY + width * m_persent_end[0] / 100.f;

        auto& obj = m_resultRois[0];

        obj.m_searchDirection = ((searchDirection == PI_ED_DIR_INNER) == isLeft) ? RIGHT : LEFT;
        obj.m_roi.m_left = (long)(lineX_findPositionX - scale.convert_umToPixelX(m_searchLength_um[0] * 0.5f));
        obj.m_roi.m_right = (long)(lineX_findPositionX + scale.convert_umToPixelX(m_searchLength_um[0] * 0.5f));
        obj.m_roi.m_top = (long)min(start, end);
        obj.m_roi.m_bottom = (long)max(start, end);

        m_resultRefCornerPoint.m_x = (float)lineX_findPositionX;
    }

    if (true)
    {
        //-----------------------------------------------------------------------------------
        // Y Line 찾기 ROI
        //-----------------------------------------------------------------------------------

        bool isTop = ((stitchIndex / constants.m_stitchCountX) == 0);
        long lineY_findPositionY = (long)(isTop ? top : bottom);

        float width = lineY_EX - lineY_SX;
        float start = lineY_SX + width * m_persent_start[1] / 100.f;
        float end = lineY_SX + width * m_persent_end[1] / 100.f;

        auto& obj = m_resultRois[1];

        obj.m_searchDirection = ((searchDirection == PI_ED_DIR_INNER) == isTop) ? DOWN : UP;
        obj.m_roi.m_top = (long)(lineY_findPositionY - scale.convert_umToPixelY(m_searchLength_um[1] * 0.5f));
        obj.m_roi.m_bottom = (long)(lineY_findPositionY + scale.convert_umToPixelY(m_searchLength_um[1] * 0.5f));
        obj.m_roi.m_left = (long)min(start, end);
        obj.m_roi.m_right = (long)max(start, end);

        m_resultRefCornerPoint.m_y = (float)lineY_findPositionY;
    }
}

StitchPara_BasedOnBodySize::StitchPara_BasedOnBodySize()
    : m_edgeDirection(PI_ED_DIR_RISING)
    , m_searchDirection(PI_ED_DIR_INNER)
    , m_edgeDetectMode(EdgeDetectMode_FirstEdge)
    , m_edgeNum(100)
    , m_firstEdgeMinThreshold(10.f)
{
}

StitchPara_BasedOnBodySize::~StitchPara_BasedOnBodySize()
{
}

BOOL StitchPara_BasedOnBodySize::LinkDataBase(BOOL bSave, long version, CiDataBase& db)
{
    UNREFERENCED_PARAMETER(version);

    long stitchParaCount = long(m_stitchPara.size());
    if (!db[_T("Stitch Para Count")].Link(bSave, stitchParaCount))
        stitchParaCount = 0;
    m_stitchPara.resize(stitchParaCount);

    for (long stitchIndex = 0; stitchIndex < stitchParaCount; stitchIndex++)
    {
        auto& stitchDB = db.GetSubDBFmt(_T("Stitch%d"), stitchIndex + 1);
        auto& stitchPara = m_stitchPara[stitchIndex];

        stitchPara.LinkDataBase(bSave, stitchDB);
    }

    db[_T("Edge Direction")].Link(bSave, m_edgeDirection);
    db[_T("Search Direction")].Link(bSave, m_searchDirection);
    db[_T("Edge Detect Mode")].Link(bSave, m_edgeDetectMode);
    db[_T("Edge Num")].Link(bSave, m_edgeNum);
    db[_T("First Edge Min Threshold")].Link(bSave, m_firstEdgeMinThreshold);

    return TRUE;
}

void StitchPara_BasedOnBodySize::Init()
{
    m_edgeDirection = PI_ED_DIR_RISING;
    m_searchDirection = PI_ED_DIR_INNER;
    m_edgeDetectMode = EdgeDetectMode_FirstEdge;
    m_firstEdgeMinThreshold = 10.f;
}
