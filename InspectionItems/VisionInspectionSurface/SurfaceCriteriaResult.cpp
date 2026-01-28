//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceCriteriaResult.h"

//CPP_2_________________________________ This project's headers
#include "SurfaceCriteria.h"
#include "SurfaceItemResult.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CSurfaceCriteriaResult::CSurfaceCriteriaResult()
    : nResult(NOT_MEASURED)
{
}

CSurfaceCriteriaResult::CSurfaceCriteriaResult(const CSurfaceCriteriaResult& Src)
    : nResult(Src.nResult)
#if !defined(USE_BY_HANDLER)
    , vecnResult(Src.vecnResult)
#endif
{
}

CSurfaceCriteriaResult& CSurfaceCriteriaResult::operator=(const CSurfaceCriteriaResult& Src)
{
    nResult = Src.nResult;

#if !defined(USE_BY_HANDLER)
    vecnResult = Src.vecnResult;
#endif

    return *this;
}

#if !defined(USE_BY_HANDLER)
void CSurfaceCriteriaResult::InitnResults(long nrInit)
{
    nResult = nrInit;

    vecnResult.clear();
}
#endif

#if !defined(USE_BY_HANDLER)
void CSurfaceCriteriaResult::UpdateEachResult(const VisionScale& scale, bool is3D, const CSurfaceCriteria& Spec,
    long nID, const Ipvm::Point32r2& imageCenter, SDetectedSurfaceObject& DSOValue,
    const Para_PolygonRects& decisionArea)
{
    BOOL bInclude = FALSE;
    int decisionNum = (long)Spec.vecbUseDecisionArea.size();

    if (decisionNum == 0)
    {
        // Decision Area가 설정되어 있지 않을 때는 영역 조건은 만족한 것으로 한다
        bInclude = TRUE;
    }
    else
    {
        for (int i = 0; i < decisionNum; i++)
        {
            if (Spec.vecbUseDecisionArea[i])
            {
                Ipvm::Rect32s rtObj = DSOValue.rtROI;
                if (decisionArea.GetAt(i).IsRectIntersectWithPolygon(scale, imageCenter, rtObj))
                {
                    bInclude = TRUE;
                    break;
                }
            }
        }
    }

    if (bInclude)
        vecnResult[nID] = Spec.Judge(is3D, DSOValue);
    else
        vecnResult[nID] = PASS;
}
#endif

//k 좌표 Marginal
#if !defined(USE_BY_HANDLER)
void CSurfaceCriteriaResult::UpdateTotalResult()
{
    int nElementsNum = (long)vecnResult.size();

    BOOL bIsReject = FALSE;

    for (int i = 0; i < nElementsNum; i++)
    {
        if (vecnResult[i] == REJECT)
        {
            nResult = REJECT;
            bIsReject = TRUE;
            break;
        }
    }

    BOOL bIsMarginal = FALSE;

    if (!bIsReject)
    {
        for (int i = 0; i < nElementsNum; i++)
        {
            if (vecnResult[i] == MARGINAL)
            {
                nResult = MARGINAL;
                bIsMarginal = TRUE;
                break;
            }
        }
    }

    if (!bIsReject && !bIsMarginal)
        nResult = PASS;

    return;
}
#endif
