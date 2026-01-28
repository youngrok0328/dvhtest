//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceItemResult.h"

//CPP_2_________________________________ This project's headers
#include "SurfaceItem.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CSurfaceItemResult::CSurfaceItemResult()
    : nResult(NOT_MEASURED)
{
}

CSurfaceItemResult::CSurfaceItemResult(const CSurfaceItemResult& Src)
    : nResult(Src.nResult)
    , vecCriteriaResult(Src.vecCriteriaResult)

    , vecnResult(Src.vecnResult)
    , vecDSOValue(Src.vecDSOValue)
{
}

CSurfaceItemResult::~CSurfaceItemResult()
{
}

// 시리얼라이즈하지 않는 놈들은 카피할 필요가 없다!!!
// 복사 생성자 및 복사 연산자에 모두 적용!!
CSurfaceItemResult& CSurfaceItemResult::operator+=(const CSurfaceItemResult& Src)
{
    nResult = max(nResult, Src.nResult);
    if (vecCriteriaResult.size() <= Src.vecCriteriaResult.size())
    {
        vecCriteriaResult.resize(Src.vecCriteriaResult.size());
        for (int n = 0; n < vecCriteriaResult.size(); n++)
        {
            vecCriteriaResult[n].nResult = max(vecCriteriaResult[n].nResult, Src.vecCriteriaResult[n].nResult);
        }
    }

    return *this;
}

CSurfaceItemResult& CSurfaceItemResult::operator=(const CSurfaceItemResult& Src)
{
    nResult = Src.nResult;
    vecCriteriaResult = Src.vecCriteriaResult;

    vecnResult = Src.vecnResult;
    vecDSOValue = Src.vecDSOValue;

    return *this;
}

void CSurfaceItemResult::InitnResults(const CSurfaceItem& SurfaceItem)
{
    vecCriteriaResult.resize((long)(SurfaceItem.vecCriteria.size()));
    nResult = VISION_NOT_MEASURED;

    std::vector<CSurfaceCriteriaResult>::iterator itr = vecCriteriaResult.begin();

    while (itr != vecCriteriaResult.end())
    {
        itr->InitnResults(VISION_NOT_MEASURED);
        itr++;
    }

    vecDSOValue.clear();
    vecnResult.clear();
}

void CSurfaceItemResult::UpdateResults(
    const VisionScale& scale, bool is3D, const Ipvm::Point32r2& imageCenter, const CSurfaceItem& SurfItemSpec)
{
    UpdateCriteriaResults(scale, is3D, imageCenter, SurfItemSpec);
    UpdateTotalResult();
}

void CSurfaceItemResult::UpdateCriteriaResults(
    const VisionScale& scale, bool is3D, const Ipvm::Point32r2& imageCenter, const CSurfaceItem& SurfItemSpec)
{
    const std::vector<CSurfaceCriteria>& vecCriteria = SurfItemSpec.vecCriteria;
    const auto& decisionArea = SurfItemSpec.m_decisionArea;

    long nCriteriaNum = (long)vecCriteria.size();
    long nDSONum = (long)vecDSOValue.size();

    for (long nCriteriaID = 0; nCriteriaID < nCriteriaNum; nCriteriaID++)
    {
        vecCriteriaResult[nCriteriaID].vecnResult.resize(nDSONum);
        std::fill(
            vecCriteriaResult[nCriteriaID].vecnResult.begin(), vecCriteriaResult[nCriteriaID].vecnResult.end(), 0);
    }

    vecnResult.resize(nDSONum);
    if (nDSONum > 0)
        memset(&vecnResult[0], 0, sizeof(long) * (long)vecnResult.size());

    // 발견된 Surface Object 들에 대해서 Criteria들을 적용하여 결과를 구한다.
    // 모든 Criteria 들에 적용한 결과를 총합하여 각 Object가 결론적으로 Reject인지의 여부를 정한다.
    const CSurfaceCriteria* pCriteria = &vecCriteria[0];
    CSurfaceCriteriaResult* pCriteriaResult = &vecCriteriaResult[0];
    SDetectedSurfaceObject* pDSOValue = nDSONum ? &vecDSOValue[0] : NULL;
    long* pnResult = nDSONum ? &vecnResult[0] : NULL;

    std::vector<std::vector<long>> nvecResult(nCriteriaNum);
    std::vector<long> vecnDefect(nDSONum);

    for (long nCriteriaID = 0; nCriteriaID < nCriteriaNum; nCriteriaID++)
    {
        nvecResult[nCriteriaID].resize(nDSONum);
    }

    for (long nCriteriaID = 0; nCriteriaID < nCriteriaNum; nCriteriaID++)
    {
        long nPowSeed = nCriteriaNum - nCriteriaID - 1;
        long nTotalDefectCount = 0; //Criteria 별로 카운트 해야한다
        auto criteriaType = pCriteria[nCriteriaID].m_type;

        if (pCriteria[nCriteriaID].bInsp)
        {
            for (long nID = 0; nID < nDSONum; nID++)
            {
                // i번째 Object를 j번재 Criteria에 대입하여 결과를 판단한다.
                pCriteriaResult[nCriteriaID].UpdateEachResult(
                    scale, is3D, pCriteria[nCriteriaID], nID, imageCenter, pDSOValue[nID], decisionArea);

                // 영훈 20160315 : Surface Mask가 매 검사마다 Mask를 생성하도록 합니다.
                nvecResult[nCriteriaID][nID] = pCriteriaResult[nCriteriaID].vecnResult[nID];
                pDSOValue[nID].m_inspValue[long(VisionSurfaceCriteria_Column::Defect_num)] = 0.f;

                if (criteriaType == 1 && pCriteriaResult[nCriteriaID].vecnResult[nID] == MARGINAL
                    || criteriaType == 2 && pCriteriaResult[nCriteriaID].vecnResult[nID] == REJECT)
                {
                    nTotalDefectCount++;

                    // 예를 들어 Criteria가 3개고 해당 Obj가 1,3번 Criteria에서 불량이면 101.000으로 표기 하기 위함.
                    // Count로 인해 Pass가 나더라도 원래 불량인 넘을 알아보기 위함이다.

                    vecnDefect[nID] += (long)pow(10, nPowSeed);
                }
            }

            // if, else if 로 하지 않은 이유는 5와 3 사이의 값만 pass를 하는 and 기능을 함게 하기 위해서..
            const auto& specDefectMin
                = SurfItemSpec.vecCriteria[nCriteriaID].GetSpecMin(VisionSurfaceCriteria_Column::Defect_num);
            const auto& specDefectMax
                = SurfItemSpec.vecCriteria[nCriteriaID].GetSpecMax(VisionSurfaceCriteria_Column::Defect_num);

            if (specDefectMin.GetInsp())
            {
                float fDefectNumMax = specDefectMin.GetPassMax();

                if (nTotalDefectCount <= fDefectNumMax)
                {
                    for (long nID = 0; nID < nDSONum; nID++)
                    {
                        if (nvecResult[nCriteriaID][nID] != PASS)
                        {
                            nvecResult[nCriteriaID][nID] = PASS;
                            pCriteriaResult[nCriteriaID].vecnResult[nID] = PASS;
                        }
                    }
                }
            }
            if (specDefectMax.GetInsp())
            {
                float fDefectNumMin = specDefectMax.GetPassMin();

                if (nTotalDefectCount >= fDefectNumMin)
                {
                    for (long nID = 0; nID < nDSONum; nID++)
                    {
                        if (nvecResult[nCriteriaID][nID] != PASS)
                        {
                            nvecResult[nCriteriaID][nID] = PASS;
                            pCriteriaResult[nCriteriaID].vecnResult[nID] = PASS;
                        }
                    }
                }
            }
        }
    }

    for (long nID = 0; nID < nDSONum; nID++) //앞에서 버퍼에 모은 결과를 기반으로 Obj별 Total Result를 계산한다.
    {
        pnResult[nID] = PASS;
        for (long nCriteriaID = 0; nCriteriaID < nCriteriaNum; nCriteriaID++)
        {
            pnResult[nID] = (long)max(pnResult[nID], nvecResult[nCriteriaID][nID]);
        }
        pDSOValue[nID].m_inspValue[long(VisionSurfaceCriteria_Column::Defect_num)]
            = (float)vecnDefect[nID]; //다 모은 Defect 유형도 써주자
    }

    for (long nCriteriaID = 0; nCriteriaID < nCriteriaNum; nCriteriaID++)
    {
        pCriteriaResult[nCriteriaID].UpdateTotalResult();
    }
}

void CSurfaceItemResult::UpdateCurrentResult(
    BOOL& o_bNotMeasured, BOOL& o_bInvalid, BOOL& o_bReject, BOOL& o_bMarginal, const int i_nResult)
{
    o_bNotMeasured = o_bNotMeasured && (i_nResult == NOT_MEASURED);
    o_bInvalid = o_bInvalid || (i_nResult == INVALID);
    o_bReject = o_bReject || (i_nResult == REJECT);
    o_bMarginal = o_bMarginal || (i_nResult == MARGINAL);
}

void CSurfaceItemResult::UpdateTotalResult()
{
    std::vector<CSurfaceCriteriaResult>::iterator itr = vecCriteriaResult.begin();

    BOOL bNotMeasured = TRUE;
    BOOL bInvalid = FALSE;
    BOOL bReject = FALSE;
    BOOL bMarginal = FALSE;

    while (itr != vecCriteriaResult.end())
    {
        UpdateCurrentResult(bNotMeasured, bInvalid, bReject, bMarginal, itr->nResult);
        itr++;
    }

    if (bInvalid)
    {
        nResult = INVALID;
    }
    else if (bReject)
    {
        nResult = REJECT;
    }
    else if (bMarginal)
    {
        nResult = MARGINAL;
    }
    else
    {
        nResult = PASS;
    }
}
