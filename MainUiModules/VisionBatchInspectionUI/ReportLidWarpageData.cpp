//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ReportLidWarpageData.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../UserInterfaceModules/CommonControlExtension/AutoHidePopup.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/Miscellaneous.h>

//CPP_5_________________________________ Standard library headers
#include <utility>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ReportLidWarpageData::ReportLidWarpageData()
    : m_inspectionItemCount(0)
{
}

ReportLidWarpageData::~ReportLidWarpageData()
{
}

bool ReportLidWarpageData::isUnitValid(VisionUnit& visionUnit)
{
    long paneResult = visionUnit.GetInspTotalResult();
    if (paneResult == NOT_MEASURED || paneResult == EMPTY || paneResult == DOUBLEDEVICE || paneResult == COUPON
        || paneResult == INVALID)
    {
        // 검사결과가 문제가 있는 것은 데이터로 수집해서는 안된다
        return false;
    }

    return true;
}

void ReportLidWarpageData::Reset()
{
    m_inspectionItemCount = 0;
    m_inspData_unit.clear();
    m_inspIndex = -1;
}

void ReportLidWarpageData::Add(long dataIndex, LPCTSTR unitID, LPCTSTR barcodeID, VisionUnit& visionUnit)
{
    if (!isUnitValid(visionUnit))
        return;

    auto visionInspectionResults = visionUnit.GetVisionInspectionResults();

    if (m_inspectionItemCount == 0)
    {
        for (long resultIndex = 0; resultIndex < long(visionInspectionResults.size()); resultIndex++)
        {
            auto* result = visionInspectionResults[resultIndex];

            // 검증에 필요한 다음 데이터만 저장한다
            if (result->m_resultName == _T("Lid Warpage"))
            {
                m_inspIndex = resultIndex;
                m_inspectionItemCount = long(visionInspectionResults.size());
                break;
            }
        }
    }

    if (m_inspectionItemCount == 0)
        return;
    if (m_inspectionItemCount != visionInspectionResults.size())
    {
        ASSERT(!_T("??"));
        return;
    }

    m_inspData_unit.resize(max(m_inspData_unit.size(), dataIndex + 1));

    m_inspData_unit[dataIndex].m_unitID = unitID;
    m_inspData_unit[dataIndex].m_barcodeID = barcodeID;

    auto& objectlist = m_inspData_unit[dataIndex].m_objectlist;

    for (long inspColumnIndex = 0; inspColumnIndex < long(enumIsnpColumn::END); inspColumnIndex++)
    {
        auto* result = visionInspectionResults[m_inspIndex];
        const auto objCount = long(result->m_objectValues.size());

        if (result->m_objectPositionX.size() != objCount)
            continue;
        if (result->m_objectPositionY.size() != objCount)
            continue;

        for (long obj = 0; obj < objCount; obj++)
        {
            CString objectName = result->m_objectNames[obj];

            auto& objectDataCollect = objectlist[obj];
            auto& values = objectDataCollect.m_inspValues;

            objectDataCollect.m_objectID = objectName;

            values[long(enumIsnpColumn::SEQ)].Format(_T("%d"), obj + 1);
            values[long(enumIsnpColumn::Nominal_X)].Format(_T("%.2f"), result->m_objectPositionX[obj]);
            values[long(enumIsnpColumn::Nominal_Y)].Format(_T("%.2f"), result->m_objectPositionY[obj]);

            if (result->m_objectValues[obj] != Ipvm::k_noiseValue32r)
            {
                float value = result->m_nominalValues[obj] + result->m_objectValues[obj];
                values[long(enumIsnpColumn::Warpage)].Format(_T("%.2f"), value);
            }
        }
    }
}

bool ReportLidWarpageData::Save(bool saveBarcode, LPCTSTR saveFolder)
{
    if (m_inspData_unit.size() == 0)
    {
        // 저장할게 없다
        return true;
    }

    long totalLine = 0;

    for (auto& inspData : m_inspData_unit)
    {
        totalLine += long(inspData.m_objectlist.size());
    }

    if (totalLine == 0)
    {
        // 저장할게 없다
        return true;
    }

    Ipvm::CreateDirectories(saveFolder);

    CString strTotalFilePath;
    strTotalFilePath.Format(_T("%sLid Warpage.csv"), saveFolder);

    FILE* fp = nullptr;
    _tfopen_s(&fp, strTotalFilePath, _T("w"));

    if (fp == nullptr)
    {
        return false;
    }

    BOOL bSaveInfo = true;

    if (saveBarcode)
    {
        fprintf(fp, "Unit,Barcode,SEQ,LID_NOMINAL_X,LID_NOMINAL_Y,Warpage\n");
    }
    else
    {
        fprintf(fp, "Unit,SEQ,LID_NOMINAL_X,LID_NOMINAL_Y,Warpage\n");
    }

    // 원래 Batch의 UnitID 순으로 데이터를 저장한다
    for (auto& unitInfo : m_inspData_unit)
    {
        for (auto& object : unitInfo.m_objectlist)
        {
            // Write header
            CString objID;
            objID.Format(_T("%d"), object.first);
            fprintf(fp, CStringA(unitInfo.m_unitID) + ","); // Unit
            if (saveBarcode)
            {
                fprintf(fp, CStringA(unitInfo.m_barcodeID) + ","); // Barcode
            }

            for (auto& value : object.second.m_inspValues)
            {
                fprintf(fp, CStringA(value) + ","); // Value
            }

            fprintf(fp, "\n");
        }
    }

    fclose(fp);

    if (!bSaveInfo)
    {
        AutoHidePopup::add(AutoHidePopupType::Error, _T("Report Data Save Error!"));
    }

    return true;
}
