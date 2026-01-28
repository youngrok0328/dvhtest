//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ReportSurfaceData.h"

//CPP_2_________________________________ This project's headers
#include "ReportData.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionSurfaceCriteriaDefinitions.h"
#include "../../UserInterfaceModules/CommonControlExtension/AutoHidePopup.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Gadget/Miscellaneous.h>

//CPP_5_________________________________ Standard library headers
#include <utility>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ReportSurfaceData::ReportSurfaceData()
{
}

ReportSurfaceData::~ReportSurfaceData()
{
}

void ReportSurfaceData::Reset()
{
    m_inspData_unit.clear();
}

void ReportSurfaceData::Add(long dataIndex, LPCTSTR unitID, LPCTSTR barcodeID, VisionUnit& visionUnit)
{
    if (!ReportData::isUnitValid(visionUnit))
        return;

    auto visionInspectionResults = visionUnit.GetVisionInspectionResults();

    m_inspData_unit.resize(max(m_inspData_unit.size(), dataIndex + 1));

    m_inspData_unit[dataIndex].m_unitID = unitID;
    m_inspData_unit[dataIndex].m_barcodeID = barcodeID;

    auto& objectlist = m_inspData_unit[dataIndex].m_objectlist;

    for (long resultIndex = 0; resultIndex < long(visionInspectionResults.size()); resultIndex++)
    {
        auto* result = visionInspectionResults[resultIndex];
        if (result->m_hostReportCategory != HostReportCategory::SURFACE)
            continue;
        if (long(result->m_vecSurfaceCriteriaResult.size()) == 0)
            continue;

        for (long obj = 0; obj < long(result->m_vecSurfaceCriteriaResult.size()); obj++)
        {
            auto& criteriaResult = result->m_vecSurfaceCriteriaResult[obj];
            auto& roi = criteriaResult.m_rtCriteriaROI;

            auto point = roi.CenterPoint();
            std::vector<CString> collectResult;
            CString itemID;
            CString defectID;
            itemID.Format(_T("%03d"), resultIndex + 1);
            defectID.Format(_T("%d"), obj + 1);
            collectResult.push_back(itemID);
            collectResult.push_back(defectID);
            collectResult.push_back(result->m_inspName);
            collectResult.push_back(criteriaResult.m_strCriteriaResult);
            collectResult.push_back(criteriaResult.m_strCriteriaName);
            collectResult.push_back(criteriaResult.m_strCriteriaColor);

            CString x;
            CString y;
            CString left;
            CString top;
            CString right;
            CString bottom;
            x.Format(_T("%d"), point.m_x);
            y.Format(_T("%d"), point.m_y);
            left.Format(_T("%d"), roi.m_left);
            top.Format(_T("%d"), roi.m_top);
            right.Format(_T("%d"), roi.m_right);
            bottom.Format(_T("%d"), roi.m_bottom);

            collectResult.push_back(x);
            collectResult.push_back(y);
            collectResult.push_back(left);
            collectResult.push_back(top);
            collectResult.push_back(right);
            collectResult.push_back(bottom);
            collectResult.insert(collectResult.end(), criteriaResult.m_vecstrCriteriaValue.begin(),
                criteriaResult.m_vecstrCriteriaValue.end());

            objectlist.push_back(collectResult);
        }
    }
}

bool ReportSurfaceData::Save(bool saveBarcode, LPCTSTR saveFolder)
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

    CTime time(CTime::GetCurrentTime());
    CString strTime = time.Format(_T("%Y%m%d_%H%M%S"));
    CString strTotalFilePath;
    strTotalFilePath.Format(_T("%sSurface.csv"), saveFolder);

    FILE* fp = nullptr;
    _tfopen_s(&fp, strTotalFilePath, _T("w"));

    if (fp == nullptr)
    {
        return false;
    }

    BOOL bSaveInfo = true;

    fprintf(fp, "Unit,");

    if (saveBarcode)
    {
        fprintf(fp, "BarCode,");
    }

    fprintf(fp, "ITEM_ID,DFECT_ID,ITEM,T,C0,COLOR,Position_X,Position_Y,Area_Left,Area_Top,Area_Right,Area_Bottom,");

    for (long index = 0; index < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); index++)
    {
        fprintf(fp, CStringA(g_strVisionSurfCriteria_Col[index]) + ",");
    }
    fprintf(fp, "\n");

    // 원래 Batch의 UnitID 순으로 데이터를 저장한다
    for (auto& unitInfo : m_inspData_unit)
    {
        for (auto& object : unitInfo.m_objectlist)
        {
            // Write header
            fprintf(fp, CStringA(unitInfo.m_unitID) + ","); // Unit

            if (saveBarcode)
            {
                fprintf(fp, CStringA(unitInfo.m_barcodeID) + ","); // Barcode
            }

            for (auto& value : object)
            {
                // SDY Surface.csv 에서 Surface 내부의 검사 항목이 2종 이상이면 Reject,Reject 형식으로 들어오는 검사 결과를 Reject/Reject 형식으로 수정
                CString SaveTxt = CString(value);
                SaveTxt.Replace(L",", L"/");
                fprintf(fp, CStringA(SaveTxt) + ","); // Value
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

bool ReportSurfaceData::SaveAutomationInfo(
    bool saveBarcode, LPCTSTR saveFolder, VisionUnit& visionUnit) // SDY_Automation Viewer에서 읽을 파일 생성
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

    CTime time(CTime::GetCurrentTime());
    CString strTime = time.Format(_T("%Y%m%d_%H%M%S"));
    CString strTotalFilePath;
    strTotalFilePath.Format(_T("%sSurface.csv"), saveFolder);

    FILE* fp = nullptr;
    _tfopen_s(&fp, strTotalFilePath, _T("w"));

    if (fp == nullptr)
    {
        return false;
    }

    BOOL bSaveInfo = true;

    fprintf(fp, "Unit,");

    if (saveBarcode)
    {
        fprintf(fp, "BarCode,");
    }

    fprintf(fp, "INDEX,ITEM,TOTAL COUNT,PASS COUNT,REJECT COUNT,");

    fprintf(fp, "\n");

    std::vector<CString> SurfaceinspectionList;

    auto visionInspectionResults = visionUnit.GetVisionInspectionResults();

    for (long resultIndex = 0; resultIndex < long(visionInspectionResults.size()); resultIndex++)
    {
        auto* result = visionInspectionResults[resultIndex];
        if (result->m_hostReportCategory != HostReportCategory::SURFACE)
            continue;
        if (long(result->m_vecSurfaceCriteriaResult.size()) == 0)
            continue;

        SurfaceinspectionList.push_back(result->m_inspName);
    }

    // 원래 Batch의 UnitID 순으로 데이터를 저장한다
    for (long nUnitInfoIdx = 0; nUnitInfoIdx < m_inspData_unit.size(); nUnitInfoIdx++)
    {
        std::vector<std::vector<CString>> vec2SurfaceResultCount;

        auto& unitInfo = m_inspData_unit[nUnitInfoIdx];

        long nTotalCount = 0;
        long nPassCount = 0;
        long nRejectCount = 0;

        CString strLastInspItem;

        // 해당 패키지에 대한 데이터를 찾는다.
        for (long nObjectIdx = 0; nObjectIdx < unitInfo.m_objectlist.size(); nObjectIdx++)
        {
            auto& object = unitInfo.m_objectlist[nObjectIdx];
            if (!(object[2] == strLastInspItem)) // 항목이 변경되면 이전까지 데이터를 저장한다.
            {
                std::vector<CString> vecstrAddResult;
                CString strAddResult;

                if (!strLastInspItem
                        .IsEmpty()) // 비어있다는 것은 새 패키지 이미지를 검사하기 시작했다는 의미이므로 해당 부분 skip 하여 남기지 않는다.
                {
                    vecstrAddResult.push_back(strLastInspItem); // item

                    strAddResult.Format(_T(",%d,%d,%d"), nTotalCount, nPassCount, nRejectCount); // count
                    vecstrAddResult.push_back(strAddResult);

                    vec2SurfaceResultCount.push_back(vecstrAddResult);
                }
                strLastInspItem = object[2];

                // 카운트 초기화
                nTotalCount = 0;
                nPassCount = 0;
                nRejectCount = 0;
            }

            // 데이터 분류
            nTotalCount++;
            if (object[3] == _T("Pass"))
            {
                nPassCount++;
            }
            else if (object[3] == _T("Reject"))
            {
                nRejectCount++;
            }

            if (nObjectIdx == unitInfo.m_objectlist.size() - 1) // 마지막 항목의 경우 Report를 남기도록 얘외처리 한다.
            {
                std::vector<CString> vecstrAddResult;
                CString strAddResult;

                vecstrAddResult.push_back(strLastInspItem); // Item

                strAddResult.Format(_T(",%d,%d,%d"), nTotalCount, nPassCount, nRejectCount); // Count
                vecstrAddResult.push_back(strAddResult);

                vec2SurfaceResultCount.push_back(vecstrAddResult);
            }
        }

        // 찾은 데이터를 레포트 양식에 맞게 변환하여 저장
        for (long nInspIdx = 0; nInspIdx < SurfaceinspectionList.size(); nInspIdx++)
        {
            BOOL bNotMatched = TRUE;

            long nResultIdx = 0;
            while (bNotMatched)
            {
                if (vec2SurfaceResultCount.size() == 0)
                {
                    break;
                }

                if (SurfaceinspectionList[nInspIdx] == vec2SurfaceResultCount[nResultIdx][0])
                {
                    fprintf(fp, CStringA(unitInfo.m_unitID) + ","); // Unit

                    if (saveBarcode)
                    {
                        fprintf(fp, CStringA(unitInfo.m_barcodeID) + ","); // Barcode
                    }
                    CString Idx;
                    Idx.Format(_T("%d,"), nInspIdx);
                    fprintf(fp, CStringA(Idx)); // Index

                    fprintf(fp, CStringA(SurfaceinspectionList[nInspIdx])); // Inspname
                    fprintf(fp, CStringA(vec2SurfaceResultCount[nResultIdx][1]) + '\n'); // Value

                    bNotMatched = FALSE;
                }
                nResultIdx++;

                if (nResultIdx == vec2SurfaceResultCount.size())
                {
                    break;
                }
            }

            if (bNotMatched)
            {
                fprintf(fp, CStringA(unitInfo.m_unitID) + ","); // Unit

                if (saveBarcode)
                {
                    fprintf(fp, CStringA(unitInfo.m_barcodeID) + ","); // Barcode
                }
                CString Idx;
                Idx.Format(_T("%d,"), nInspIdx);
                fprintf(fp, CStringA(Idx)); // Index

                fprintf(fp, CStringA(SurfaceinspectionList[nInspIdx])); // Inspname
                fprintf(fp, CStringA(",0,0,0,") + '\n'); // Value
            }
        }
    }

    fclose(fp);

    if (!bSaveInfo)
    {
        AutoHidePopup::add(AutoHidePopupType::Error, _T("Report Data Save Error!"));
    }

    return true;
}
