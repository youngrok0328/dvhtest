//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgMarkAlgorithmParameter.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionMarkPara.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgMarkAlgorithmParameter, CDialog)

enum enumPropertyItemID
{
    ITEM_ID_MARK_MERGEGAP = 0,
    ITEM_ID_MARK_BLOBSIZEMERGEGAP,
};

CDlgMarkAlgorithmParameter::CDlgMarkAlgorithmParameter(
    VisionInspectionMarkPara* pVisionInspectionMarkPara, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgMarkAlgorithmParameter::IDD, pParent)
    , m_propertyGrid(nullptr)
{
    m_pVisionInspectionMarkPara = pVisionInspectionMarkPara;
}

CDlgMarkAlgorithmParameter::~CDlgMarkAlgorithmParameter()
{
    if (m_propertyGrid != nullptr)
        delete m_propertyGrid;
}

void CDlgMarkAlgorithmParameter::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgMarkAlgorithmParameter, CDialog)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)

END_MESSAGE_MAP()

// CDlgMarkAlgorithmParameter 메시지 처리기입니다.
BOOL CDlgMarkAlgorithmParameter::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtPropertyGrid;
    GetClientRect(rtPropertyGrid);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtPropertyGrid, this, IDC_FRAME_PROPERTY_GRID);
    m_propertyGrid->ShowHelp(FALSE);

    UpdatePropertyGrid();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgMarkAlgorithmParameter::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Mark Algorithm Parameter")))
    {
        if (auto* MarkBlob = m_propertyGrid->AddCategory(_T("Mark Blob")))
        {
            if (auto* Item = MarkBlob->AddChildItem(new CXTPPropertyGridItemNumber(_T("Min Blob Size (Px)"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nBlobSizeMin,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.nBlobSizeMin)))
            {
            }
            if (auto* Item = MarkBlob->AddChildItem(new CXTPPropertyGridItemNumber(_T("Max Blob Size (Px)"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nBlobSizeMax,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.nBlobSizeMax)))
            {
            }
            if (auto* Item
                = MarkBlob->AddChildItem(new CXTPPropertyGridItemNumber(_T("Min Mark Contrast for [Mark Count] (GV)"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nMinContrastForMarkCnt,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.nMinContrastForMarkCnt)))
            {
            }
            if (auto* Item = MarkBlob->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Merge Gap (Px)"), m_pVisionInspectionMarkPara->sMarkAlgoParam.fMarkMergeGap, _T("%.3f"))))
            {
                Item->SetID(ITEM_ID_MARK_MERGEGAP);
            }

            MarkBlob->Expand();
        }

        if (auto* MarkBlobSize = m_propertyGrid->AddCategory(_T("Mark Blob Size")))
        {
            if (auto* Item = MarkBlobSize->AddChildItem(new CXTPPropertyGridItemNumber(_T("Blob Thinning Intensity"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nThinningIntensity,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.nThinningIntensity)))
            {
            }
            if (auto* Item = MarkBlobSize->AddChildItem(new CXTPPropertyGridItemDouble(_T("Dist Blob to Mark"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.fBlobSizeMergeGap, _T("%.3f"))))
            {
                Item->SetID(ITEM_ID_MARK_BLOBSIZEMERGEGAP);
            }

            MarkBlobSize->Expand();
        }

        if (auto* ReInspctionByAllChar = m_propertyGrid->AddCategory(_T("Under/Over ReInsp (All Characters)")))
        {
            if (auto* Item = ReInspctionByAllChar->AddChildItem(new CXTPPropertyGridItemBool(_T("Use All Charcters"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.bUseAllCharReinsp,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.bUseAllCharReinsp)))
            {
            }
            if (auto* Item = ReInspctionByAllChar->AddChildItem(new CXTPPropertyGridItemNumber(_T("ReInsp Number"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nAllCharReinspNumber,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.nAllCharReinspNumber)))
            {
            }
            if (auto* Item = ReInspctionByAllChar->AddChildItem(new CXTPPropertyGridItemNumber(_T("Threshold Dev(GV)"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nAllCharThresholdDeviation,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.nAllCharThresholdDeviation)))
            {
            }

            ReInspctionByAllChar->Expand();
        }

        if (auto* ReInspctionByEachChar = m_propertyGrid->AddCategory(_T("Under/Over ReInsp (Each Characters)")))
        {
            if (auto* Item = ReInspctionByEachChar->AddChildItem(new CXTPPropertyGridItemBool(_T("Use Each Charcters"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.bUseEachCharReinsp,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.bUseEachCharReinsp)))
            {
            }
            if (auto* Item = ReInspctionByEachChar->AddChildItem(new CXTPPropertyGridItemNumber(_T("ReInsp Number"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nEachCharReinspNumber,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.nEachCharReinspNumber)))
            {
            }
            if (auto* Item = ReInspctionByEachChar->AddChildItem(new CXTPPropertyGridItemNumber(_T("Threshold Dev(GV)"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nEachCharThresholdDeviation,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.nEachCharThresholdDeviation)))
            {
            }

            ReInspctionByEachChar->Expand();
        }

        if (auto* ETC = m_propertyGrid->AddCategory(_T("ETC")))
        {
            if (auto* Item = ETC->AddChildItem(new CXTPPropertyGridItemEnum(_T("Threshold Mode"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nThresholdMode,
                    (int*)&m_pVisionInspectionMarkPara->sMarkAlgoParam.nThresholdMode)))
            {
                Item->GetConstraints()->AddConstraint(_T("Threshold - Manual"), 0);
                Item->GetConstraints()->AddConstraint(_T("Threshold - Semi-Auto"), 1);
                Item->GetConstraints()->AddConstraint(_T("Threshold - Full-Auto"), 2);
            }

            if (auto* Item = ETC->AddChildItem(new CXTPPropertyGridItemNumber(_T("Mark Charcter Contrast ROI Margin"),
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.nMarkRoiMargin,
                    &m_pVisionInspectionMarkPara->sMarkAlgoParam.nMarkRoiMargin)))
            {
            }

            ETC->Expand();
        }

        category->Expand();
    }

    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgMarkAlgorithmParameter::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* Item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (Item->GetID())
        {
            case ITEM_ID_MARK_MERGEGAP:
                if (auto* MergeGapValue = dynamic_cast<CXTPPropertyGridItemDouble*>(Item))
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.fMarkMergeGap = float(MergeGapValue->GetDouble());
                break;
            case ITEM_ID_MARK_BLOBSIZEMERGEGAP:
                if (auto* BlobMergeGapValue = dynamic_cast<CXTPPropertyGridItemDouble*>(Item))
                    m_pVisionInspectionMarkPara->sMarkAlgoParam.fBlobSizeMergeGap
                        = float(BlobMergeGapValue->GetDouble());
                break;
            default:
                break;
        }
    }

    return 0;
}