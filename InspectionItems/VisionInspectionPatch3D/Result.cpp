//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Result.h"

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
Result::Result()
{
}

Result::~Result()
{
}

void Result::Initialize(const VisionScale& scale, const VisionInspectionPatch3DPara& para)
{
    ConvertionInterposerROIInfoUmToPxl(scale, para.m_vecsInterposerRefROI_Info_um);
}

void Result::ConvertionInterposerROIInfoUmToPxl(
    const VisionScale& scale, const std::vector<SInterposerRefROI_Info>& vecsPatch3DGroupInfo_um)
{
    long nROINum = (long)vecsPatch3DGroupInfo_um.size();
    m_vecsInterposerRefROI_Info_pxl.clear();
    if (nROINum <= 0)
        return;

    m_vecsInterposerRefROI_Info_pxl.resize(nROINum);

    const auto& um2px = scale.umToPixel();

    for (long nROI = 0; nROI < nROINum; nROI++)
    {
        auto& interposer3DROIInfo_um = vecsPatch3DGroupInfo_um[nROI];
        auto& interposer3DROIInfo_pxl = m_vecsInterposerRefROI_Info_pxl[nROI];

        interposer3DROIInfo_pxl.fOffsetX = interposer3DROIInfo_um.fOffsetX * um2px.m_x;
        interposer3DROIInfo_pxl.fOffsetY = interposer3DROIInfo_um.fOffsetY * um2px.m_y;
        interposer3DROIInfo_pxl.fWidth = interposer3DROIInfo_um.fWidth * um2px.m_x;
        interposer3DROIInfo_pxl.fLength = interposer3DROIInfo_um.fLength * um2px.m_y;
    }
}
