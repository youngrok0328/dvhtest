//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "InspResult.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
void ConvertionMarkMapInfo(
    const VisionScale& scale, const sReadMarkMapInfo& i_DataInfo_um, sReadMarkMapInfo& o_ConvertData_px)
{
    const auto& um2px = scale.umToPixel();

    o_ConvertData_px.vecTeachROI_Info.clear();
    o_ConvertData_px.vecIgnoreROI_Info.clear();
    o_ConvertData_px.vecMergeROI_Info.clear();

    long nTeachROIDataCount = (long)i_DataInfo_um.vecTeachROI_Info.size();
    o_ConvertData_px.vecTeachROI_Info.resize(nTeachROIDataCount);

    for (long nTeachDataidx = 0; nTeachDataidx < nTeachROIDataCount; nTeachDataidx++)
    {
        auto& ROIInfo_um = i_DataInfo_um.vecTeachROI_Info[nTeachDataidx];
        auto& ROIInfo_px = o_ConvertData_px.vecTeachROI_Info[nTeachDataidx];

        ROIInfo_px.strID = ROIInfo_um.strID;
        ROIInfo_px.strName = ROIInfo_um.strName;
        ROIInfo_px.fOffsetX = ROIInfo_um.fOffsetX * um2px.m_x;
        ROIInfo_px.fOffsetY = ROIInfo_um.fOffsetY * um2px.m_y;
        ROIInfo_px.fWidth = ROIInfo_um.fWidth * um2px.m_x;
        ROIInfo_px.fLength = ROIInfo_um.fLength * um2px.m_y;
        ROIInfo_px.fAngle = ROIInfo_um.fAngle;
        ROIInfo_px.nType = ROIInfo_um.nType;
    }

    long nMergeROIDataCount = static_cast<long>(i_DataInfo_um.vecMergeROI_Info.size());
    o_ConvertData_px.vecMergeROI_Info.resize(nMergeROIDataCount);

    for (long nMergeDataidx = 0; nMergeDataidx < nMergeROIDataCount; nMergeDataidx++)
    {
        auto& ROIInfo_um = i_DataInfo_um.vecMergeROI_Info[nMergeDataidx];
        auto& ROIInfo_px = o_ConvertData_px.vecMergeROI_Info[nMergeDataidx];

        ROIInfo_px.strID = ROIInfo_um.strID;
        ROIInfo_px.strName = ROIInfo_um.strName;
        ROIInfo_px.fOffsetX = ROIInfo_um.fOffsetX * um2px.m_x;
        ROIInfo_px.fOffsetY = ROIInfo_um.fOffsetY * um2px.m_y;
        ROIInfo_px.fWidth = ROIInfo_um.fWidth * um2px.m_x;
        ROIInfo_px.fLength = ROIInfo_um.fLength * um2px.m_y;
        ROIInfo_px.fAngle = ROIInfo_um.fAngle;
        ROIInfo_px.nType = ROIInfo_um.nType;
    }

    long nIgnoreROIDataCount = static_cast<long>(i_DataInfo_um.vecIgnoreROI_Info.size());
    o_ConvertData_px.vecIgnoreROI_Info.resize(nIgnoreROIDataCount);

    for (long nIgnoreDataidx = 0; nIgnoreDataidx < nIgnoreROIDataCount; nIgnoreDataidx++)
    {
        auto& ROIInfo_um = i_DataInfo_um.vecIgnoreROI_Info[nIgnoreDataidx];
        auto& ROIInfo_px = o_ConvertData_px.vecIgnoreROI_Info[nIgnoreDataidx];

        ROIInfo_px.strID = ROIInfo_um.strID;
        ROIInfo_px.strName = ROIInfo_um.strName;
        ROIInfo_px.fOffsetX = ROIInfo_um.fOffsetX * um2px.m_x;
        ROIInfo_px.fOffsetY = ROIInfo_um.fOffsetY * um2px.m_y;
        ROIInfo_px.fWidth = ROIInfo_um.fWidth * um2px.m_x;
        ROIInfo_px.fLength = ROIInfo_um.fLength * um2px.m_y;
        ROIInfo_px.fAngle = ROIInfo_um.fAngle;
        ROIInfo_px.nType = ROIInfo_um.nType;
    }
}

void ConvertionMarkMapInfo(
    const VisionScale& scale, const sMarkMapInfo& i_RealData, BOOL i_isUMtoPX, sMarkMapInfo& o_ConvertData)
{
    float fScaleX = scale.umToPixel().m_x;
    float fScaleY = scale.umToPixel().m_y;

    if (!i_isUMtoPX)
    {
        fScaleX = scale.pixelToUm().m_x;
        fScaleY = scale.pixelToUm().m_y;
    }

    o_ConvertData.Clear();

    o_ConvertData.sTeachROI_Info.strID = i_RealData.sTeachROI_Info.strID;
    o_ConvertData.sTeachROI_Info.strName = i_RealData.sTeachROI_Info.strName;
    o_ConvertData.sTeachROI_Info.fOffsetX = i_RealData.sTeachROI_Info.fOffsetX * fScaleX;
    o_ConvertData.sTeachROI_Info.fOffsetY = i_RealData.sTeachROI_Info.fOffsetY * fScaleY;
    o_ConvertData.sTeachROI_Info.fWidth = i_RealData.sTeachROI_Info.fWidth * fScaleX;
    o_ConvertData.sTeachROI_Info.fLength = i_RealData.sTeachROI_Info.fLength * fScaleY;
    o_ConvertData.sTeachROI_Info.fAngle = i_RealData.sTeachROI_Info.fAngle;
    o_ConvertData.sTeachROI_Info.nType = i_RealData.sTeachROI_Info.nType;

    long nDataSize = static_cast<long>(i_RealData.vecMergeROI_Info.size());
    o_ConvertData.vecMergeROI_Info.resize(nDataSize);

    for (long nROI = 0; nROI < nDataSize; nROI++)
    {
        auto& MarkROIInfo_um = i_RealData.vecMergeROI_Info[nROI];
        auto& MarkROIInfo_px = o_ConvertData.vecMergeROI_Info[nROI];

        MarkROIInfo_px.strID = MarkROIInfo_um.strID;
        MarkROIInfo_px.strName = MarkROIInfo_um.strName;
        MarkROIInfo_px.fOffsetX = MarkROIInfo_um.fOffsetX * fScaleX;
        MarkROIInfo_px.fOffsetY = MarkROIInfo_um.fOffsetY * fScaleY;
        MarkROIInfo_px.fWidth = MarkROIInfo_um.fWidth * fScaleX;
        MarkROIInfo_px.fLength = MarkROIInfo_um.fLength * fScaleY;
        MarkROIInfo_px.fAngle = MarkROIInfo_um.fAngle;
        MarkROIInfo_px.nType = MarkROIInfo_um.nType;
    }

    nDataSize = (long)i_RealData.vecIgnoreROI_Info.size();
    o_ConvertData.vecIgnoreROI_Info.resize(nDataSize);

    for (long nROI = 0; nROI < nDataSize; nROI++)
    {
        auto& MarkROIInfo_um = i_RealData.vecIgnoreROI_Info[nROI];
        auto& MarkROIInfo_px = o_ConvertData.vecIgnoreROI_Info[nROI];

        MarkROIInfo_px.strID = MarkROIInfo_um.strID;
        MarkROIInfo_px.strName = MarkROIInfo_um.strName;
        MarkROIInfo_px.fOffsetX = MarkROIInfo_um.fOffsetX * fScaleX;
        MarkROIInfo_px.fOffsetY = MarkROIInfo_um.fOffsetY * fScaleY;
        MarkROIInfo_px.fWidth = MarkROIInfo_um.fWidth * fScaleX;
        MarkROIInfo_px.fLength = MarkROIInfo_um.fLength * fScaleY;
        MarkROIInfo_px.fAngle = MarkROIInfo_um.fAngle;
        MarkROIInfo_px.nType = MarkROIInfo_um.nType;
    }
}

InspResult::InspResult()
{
}

InspResult::~InspResult()
{
}

void InspResult::Alloc(long charectorNum)
{
    m_vecrtCharPositionforCalcImage.resize(charectorNum);
    m_vecrtCharPositionforImage.resize(charectorNum);
    m_vecroiCharSearch.resize(charectorNum);

    for (auto& roi : m_vecrtCharPositionforCalcImage)
        roi.SetRectEmpty();
    for (auto& roi : m_vecrtCharPositionforImage)
        roi.SetRectEmpty();
    for (auto& roi : m_vecroiCharSearch)
        roi.SetRectEmpty();

    m_vecfFirstTrialUnderPrint.clear();
    m_vecfFirstTrialOverPrint.clear();
    m_vecfFirstTrialUnderPrint.resize(charectorNum, 0.f);
    m_vecfFirstTrialOverPrint.resize(charectorNum, 0.f);
}

void InspResult::InitializeMarkInfoforPixel(const VisionScale& scale, const VisionInspectionMarkPara& para)
{
    ConvertionMarkMapInfo(scale, para.m_sMarkMapInfo_um, true, m_sMarkMapInfo_px);
}
