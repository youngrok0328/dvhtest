//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BatchManager.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BatchManager::BatchManager()
    : m_nGridNum(2)
    , m_nImageNum(0)
    , m_lastFrameIndex(0)
    , m_bInfiniteInspectionMode(FALSE)
{
}

BatchManager::~BatchManager()
{
}

void BatchManager::Reset()
{
    m_barcodeData.Reset();
    m_vecBatchList.clear();
    m_reportData.Reset();
    m_reportWarpageData.Reset();
    m_reportSurfaceData.Reset();
    m_reportBodyData.Reset();
}

void BatchManager::Inspect_StartReady(VisionUnit& visionUnit)
{
    m_bInfiniteInspectionMode = FALSE;
    m_mapRnRData.clear();
    m_reportData.Reset();
    m_reportWarpageData.Reset();
    m_reportSurfaceData.Reset();
    m_reportBodyData.Reset();

    auto visionInspectionResults = visionUnit.GetVisionInspectionResults();

    long ninspNum = (long)visionInspectionResults.size();
    m_vecstBatchInspResult.clear();
    m_vecstBatchInspResult.resize(ninspNum);

    for (long n = 0; n < ninspNum; n++)
    {
        m_vecstBatchInspResult[n].strInspName = _T("Untitle");
        m_vecstBatchInspResult[n].nNotMeasured = 0;
        m_vecstBatchInspResult[n].nPass = 0;
        m_vecstBatchInspResult[n].nReject = 0;
        m_vecstBatchInspResult[n].nInvalid = 0;
        m_vecstBatchInspResult[n].nEmpty = 0;
        m_vecstBatchInspResult[n].nDouble = 0;
    }
}

void BatchManager::Inspect_DataCollect(VisionUnit& visionUnit, long index)
{
    auto& batchInfo = m_vecBatchList[index];

    batchInfo.m_result = visionUnit.GetInspTotalResult();

    auto visionInspectionResults = visionUnit.GetVisionInspectionResults();

    batchInfo.m_subResults.clear();
    batchInfo.m_subResults.resize(visionInspectionResults.size());

    for (long i = 0; i < (long)(visionInspectionResults.size()); i++)
    {
        if (visionInspectionResults[i]->m_itemUnit == _T("S"))
        {
            batchInfo.m_subResults[i].m_value = Result2String(visionInspectionResults[i]->m_totalResult);
        }
        else
        {
            if (visionInspectionResults[i]->getObjectWorstErrorValue() != Ipvm::k_noiseValue32r)
            {
                batchInfo.m_subResults[i].m_value.Format(
                    _T("%.2f"), visionInspectionResults[i]->getObjectWorstErrorValue());
            }
        }

        batchInfo.m_subResults[i].m_color = Result2Color(visionInspectionResults[i]->m_totalResult);

        if (m_bInfiniteInspectionMode == FALSE)
        {
            m_vecstBatchInspResult[i].strInspName.Format(_T("[%s] %s"), (LPCTSTR)visionInspectionResults[i]->m_inspName,
                (LPCTSTR)visionInspectionResults[i]->m_resultName);

            switch (visionInspectionResults[i]->m_totalResult)
            {
                case NOT_MEASURED:
                    m_vecstBatchInspResult[i].nNotMeasured++;
                    break;
                case PASS:
                    m_vecstBatchInspResult[i].nPass++;
                    break;
                case REJECT:
                    m_vecstBatchInspResult[i].nReject++;
                    break;
                case INVALID:
                    m_vecstBatchInspResult[i].nInvalid++;
                    break;
                case EMPTY:
                    m_vecstBatchInspResult[i].nEmpty++;
                    break;
                case DOUBLEDEVICE:
                    m_vecstBatchInspResult[i].nDouble++;
                    break;
            }
        }
    }
}

void BatchManager::UpdateGridCtrl_List(CGridCtrl& ctrl)
{
    ctrl.SetRowCount(2);

    if (m_barcodeData.IsEnabled())
    {
        ctrl.SetColumnWidth(BatchPreColumn_Barcode, 100);
    }
    else
    {
        ctrl.SetColumnWidth(BatchPreColumn_Barcode, 0);
    }

    for (long listIndex = 0; listIndex < long(m_vecBatchList.size()); listIndex++)
    {
        auto& batchInfo = m_vecBatchList[listIndex];
        long rowIndex = listIndex + 2;

        ///////////////////////////////////////////////////
        //{{ Batch List UI Update
        long nNameCount, nCellWidth = 0, nOldCellWidth = 0;

        ctrl.InsertRow(_T(""));
        CString str;
        ctrl.SetItemBkColour(rowIndex, BatchPreColumn_Index, RGB(250, 250, 220));

        // Index
        str.Format(_T("%d"), listIndex + 1);
        ctrl.SetItemText(rowIndex, BatchPreColumn_Index, str);
        ctrl.SetItemFormat(rowIndex, BatchPreColumn_Index, DT_CENTER);

        // File Name
        CString fileName = batchInfo.strFileName;
        nNameCount = fileName.GetLength();
        nCellWidth = ((nNameCount * 7) > 100) ? nNameCount * 7 : 100;
        if (nCellWidth > nOldCellWidth)
        {
            ctrl.SetColumnWidth(BatchPreColumn_FileName, nCellWidth);
            nOldCellWidth = nCellWidth;
        }
        ctrl.SetItemText(rowIndex, BatchPreColumn_FileName, fileName);

        ctrl.SetItemText(rowIndex, BatchPreColumn_Barcode, m_barcodeData.GetBarcode(batchInfo.m_paneIndex, fileName));

        str.Format(_T("%02d"), batchInfo.m_scanIndex + 1);
        ctrl.SetItemText(rowIndex, BatchPreColumn_ScanID, str);
        ctrl.SetItemFormat(rowIndex, BatchPreColumn_ScanID, DT_CENTER);

        str.Format(_T("%02d"), batchInfo.m_paneIndex + 1);
        ctrl.SetItemText(rowIndex, BatchPreColumn_PaneID, str);
        ctrl.SetItemFormat(rowIndex, BatchPreColumn_PaneID, DT_CENTER);
        rowIndex++;
    }

    ctrl.Refresh();
}

void BatchManager::UpdateGridCtrl_Batch(CGridCtrl& ctrl)
{
    for (long rowIndex = 2; rowIndex < ctrl.GetRowCount(); rowIndex++)
    {
        UpdateGridCtrl_Batch(ctrl, rowIndex);
    }
    ctrl.Refresh();
}

void BatchManager::UpdateGridCtrl_Batch(CGridCtrl& ctrl, long gridIndex)
{
    long index = _ttoi(ctrl.GetItemText(gridIndex, BatchPreColumn_Index)) - 1;

    auto& info = m_vecBatchList[index];

    if (info.m_result >= 0)
    {
        ctrl.SetItemFormat(gridIndex, BatchPreColumn_TotalResult, DT_CENTER);
        ctrl.SetItemText(gridIndex, BatchPreColumn_TotalResult, Result2String(info.m_result));
        ctrl.SetItemBkColour(gridIndex, BatchPreColumn_TotalResult, Result2Color(info.m_result));

        if (info.m_subResults.size() == ctrl.GetColumnCount() - BatchPreColumn_InspItemStart)
        {
            for (long i = 0; i < (long)(info.m_subResults.size()); i++)
            {
                ctrl.SetItemFormat(gridIndex, BatchPreColumn_InspItemStart + i, DT_CENTER);
                ctrl.SetItemText(gridIndex, BatchPreColumn_InspItemStart + i, info.m_subResults[i].m_value);
                ctrl.SetItemBkColour(gridIndex, BatchPreColumn_InspItemStart + i, info.m_subResults[i].m_color);
            }
        }
    }
}
