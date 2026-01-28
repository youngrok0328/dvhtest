//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "StitchPara_BasedOnMatching.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
StitchPara_BasedOnMatching::StitchPara_BasedOnMatching()
{
}

StitchPara_BasedOnMatching::~StitchPara_BasedOnMatching()
{
}

BOOL StitchPara_BasedOnMatching::LinkDataBase(BOOL bSave, long version, CiDataBase& db)
{
    if (version < 2)
    {
        // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
        return FALSE;
    }

    if (!db[_T("nTemplateROICount")].Link(bSave, m_nTemplateROICount))
        m_nTemplateROICount = 2;

    if (!db[_T("Template Search Offset um")].Link(bSave, m_templateSearchOffset_um))
    {
        m_templateSearchOffset_um = 500.f;
    }

    long nStitchROISize;
    if (!bSave)
    {
        if (!db[_T("Stitch_Size")].Link(bSave, nStitchROISize))
            nStitchROISize = 0;
        m_vecStitchROI.resize(nStitchROISize);
    }
    else
    {
        nStitchROISize = (long)m_vecStitchROI.size();
        if (!db[_T("Stitch_Size")].Link(bSave, nStitchROISize))
            nStitchROISize = 0;
    }

    for (long stitchIndex = 0; stitchIndex < nStitchROISize; stitchIndex++)
    {
        auto& curROI = m_vecStitchROI[stitchIndex];
        for (int i = 0; i < 2; i++)
        {
            CString strTemp;
            strTemp.Format(_T("Stitch%d_Template_ROI_Vertical_Ref%d"), stitchIndex, i);
            if (!LinkEx(bSave, db[strTemp], curROI.m_rtVerRef_BCU[i]))
                curROI.m_rtVerRef_BCU[i].SetRectEmpty();
            strTemp.Format(_T("Stitch%d_Template_ROI_Horizontal_Ref%d"), stitchIndex, i);
            if (!LinkEx(bSave, db[strTemp], curROI.m_rtHorRef_BCU[i]))
                curROI.m_rtHorRef_BCU[i].SetRectEmpty();

            strTemp.Empty();
        }
    }

    return TRUE;
}

void StitchPara_BasedOnMatching::Init()
{
    m_templateSearchOffset_um = 500.f;
    m_nTemplateROICount = 2;
}
