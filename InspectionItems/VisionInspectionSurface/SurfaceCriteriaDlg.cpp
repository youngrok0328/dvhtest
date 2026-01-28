//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceCriteriaDlg.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionSurface.h"
#include "SurfaceItem.h"
#include "SurfaceItemResult.h"
#include "VisionInspectionSurface.h"
#include "VisionInspectionSurfacePara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/Config.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCellCheck2.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridSlideBox.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../UserInterfaceModules/ImageViewEx/PatternImage.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
const float gf_MILS_TO_UM = 25.4f;
const float gf_MM_TO_UM = 1000.f;
const float gf_UM_TO_MILS = 1.f / 25.4f;
const float gf_UM_TO_MM = 1.f / 1000.f;

enum ColumnType
{
    ColumnType_Text,
    ColumnType_Number
};

enum enumColumnID
{
    COL_COLOR,
    COL_TYPE,
    COL_SPEC_COLUMN_START,
};

static LPCTSTR g_szResult[] = {_T(" "), _T("Pass"), _T("Marginal"), _T("Reject"), _T("Invalid")};
static LPCTSTR g_szResultSymbol[] = {_T(" "), _T(" "), _T("m"), _T("x"), _T("-")};
static LPCTSTR g_szColor[] = {_T("Dark"), _T("Bright"), _T("Any")};

enum enumObjectColumnID
{
    COL_OBJECT_INDEX,
    COL_OBJECT_COLOR,
    COL_OBJECT_POS_X,
    COL_OBJECT_POS_Y,
    COL_OBJECT_SPEC_COLUMN_START,
};

CSurfaceCriteriaDlg::CSurfaceCriteriaDlg(VisionInspectionSurface* pVisionInsp, const CSurfaceItem& SurfaceItem,
    long nCurrentUnit, long nSurfaceID, BOOL bMaskGenMode, long bUseVmap, CWnd* pParent /*=NULL*/)
    : CDialog(CSurfaceCriteriaDlg::IDD, pParent)
    , m_pSurfaceItem(new CSurfaceItem(SurfaceItem))
    , m_nCurrentUnit(nCurrentUnit)
    , m_pVisionInsp(pVisionInsp)
    , m_imageView(nullptr)
    , m_is3D(Config::getVisionType() == VISIONTYPE_3D_INSP && bUseVmap == SURFACE_3D_USE_ZMAP)
{
    //{{AFX_DATA_INIT(CSurfaceCriteriaDlg)
    m_nPolygonNum = 3;
    m_strROIName = _T("Unknown");
    m_nROIType = 0;
    //}}AFX_DATA_INIT

    m_nSurfaceID = nSurfaceID;
    m_bMaskGenMode = bMaskGenMode;
}

CSurfaceCriteriaDlg::~CSurfaceCriteriaDlg()
{
    delete m_pSurfaceItem;
    delete m_imageView;
}

void CSurfaceCriteriaDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSurfaceCriteriaDlg)
    DDX_Control(pDX, IDC_LIST_ROI, m_lstROI);
    DDX_Control(pDX, IDC_COMBO_UNIT, m_cboUnit);
    DDX_Text(pDX, IDC_EDIT_POLYGON_NUM, m_nPolygonNum);
    DDX_Text(pDX, IDC_EDIT_ROI_NAME, m_strROIName);
    DDX_Radio(pDX, IDC_RADIO_Rect, m_nROIType);
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_GRID_CRITERIA, m_gridCriteria);
    DDX_Control(pDX, IDC_GRID_OBJECTS, m_gridObjects);
}

BEGIN_MESSAGE_MAP(CSurfaceCriteriaDlg, CDialog)
//{{AFX_MSG_MAP(CSurfaceCriteriaDlg)
ON_CBN_SELCHANGE(IDC_COMBO_UNIT, OnSelchangeComboUnit)
ON_BN_CLICKED(IDC_BUTTON_SURFACE_INSERT_CRITERIA, OnButtonSurfaceInsertCriteria)
ON_BN_CLICKED(IDC_BUTTON_SURFACE_REMOVE_CRITERIA, OnButtonSurfaceRemoveCriteria)
ON_BN_CLICKED(IDC_BTN_ADD_ROI, OnBtnAddRoi)
ON_BN_CLICKED(IDC_BTN_DEL_ROI, OnBtnDelRoi)
//}}AFX_MSG_MAP
ON_NOTIFY(GVN_SELCHANGED, IDC_GRID_OBJECTS, OnGridObjectSelChanged)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_CRITERIA, OnGridCriteriaEditEnd)
ON_BN_CLICKED(IDOK, &CSurfaceCriteriaDlg::OnBnClickedOk)
ON_BN_CLICKED(IDC_BUTTON_INSPECTION, &CSurfaceCriteriaDlg::OnBnClickedButtonInspection)
ON_BN_CLICKED(IDC_CHECK_SHOW_SELECTED_DEFECTS_ONLY, &CSurfaceCriteriaDlg::OnBnClickedCheckShowSelectedDefectsOnly)
ON_BN_CLICKED(IDC_CHECK_SHOW_SELECTED_DEFECTS_LABEL, &CSurfaceCriteriaDlg::OnBnClickedCheckShowSelectedDefectsLabel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSurfaceCriteriaDlg message handlers
BOOL CSurfaceCriteriaDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect view;
    GetDlgItem(IDC_STATIC_IMAGE_FRAME)->GetWindowRect(view);
    ScreenToClient(view);

    m_imageView = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(view), 0, m_pVisionInsp->GetCurVisionModule_Status());
    m_imageView->RegisterCallback_ROIChangeEnd(GetSafeHwnd(), this, ROI_Changed);

    //==============================
    // ID Window 에 이미지를 보여줌
    //==============================

    if (m_pVisionInsp->GetDisplayAlignImage(*m_pSurfaceItem, false, m_image))
    {
        m_imageView->SetImage(m_image);
        m_imageView->NavigateTo(Ipvm::Rect32s(m_image));
    }

    InitializeCriteriaGrid();
    InitializeObjectsGrid();

    m_cboUnit.ResetContent();
    m_cboUnit.AddString(_T("mm"));
    m_cboUnit.AddString(_T("mil"));
    m_cboUnit.AddString(_T("um"));

    UpdateSpec(FALSE);

    m_lstROI.InsertColumn(0, _T("No"), LVCFMT_LEFT, 100);
    m_lstROI.InsertColumn(1, _T("Name"), LVCFMT_LEFT, 300);
    m_lstROI.SetExtendedStyle(LVS_EX_FULLROWSELECT);

    RefreshROIList();
    SetROI();

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CSurfaceCriteriaDlg::UpdateSpec(BOOL bSaveAndValidate)
{
    int nSPos = GetColSPos();

    if (bSaveAndValidate)
    {
        long nRowCount = m_gridCriteria.GetRowCount();

        std::vector<CSurfaceCriteria>& vecCriteria = m_pSurfaceItem->vecCriteria;
        m_pSurfaceItem->bInsp = FALSE;
        vecCriteria.clear();

        BOOL bInsp = FALSE;

        for (long nRow = 1; nRow < nRowCount; nRow++)
        {
            vecCriteria.push_back(CSurfaceCriteria());

            CSurfaceCriteria& Criteria = vecCriteria.back();

            // Get Criteria Check
            Criteria.bInsp = ((CGridCellCheck2*)m_gridCriteria.GetCell(nRow, 0))->GetCheck();

            Criteria.vecbUseDecisionArea.resize(nSPos - 2, FALSE);

            for (int n = 2; n < nSPos; n++)
            {
                // ROI Check
                Criteria.vecbUseDecisionArea[n - 2] = ((CGridCellCheck2*)m_gridCriteria.GetCell(nRow, n))->GetCheck();
            }

            // Get Criteria Name
            Criteria.strName = LPCTSTR(m_gridCriteria.GetItemText(nRow, 1));

            // Get Color Type
            Criteria.m_color = ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nSPos + COL_COLOR))->GetCurSel();

            // Get Criteria Type
            Criteria.m_type = ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nSPos + COL_TYPE))->GetCurSel();

            for (long n = 0, colIndex = COL_SPEC_COLUMN_START; n < GetCriteriaColumnCount(m_is3D); n++, colIndex += 2)
            {
                auto columnID = GetCriteriaColumnID(m_is3D, n);
                GetCriteriaMin(nRow, nSPos + colIndex, Criteria.GetSpecMin(columnID));
                GetCriteriaMax(nRow, nSPos + colIndex + 1, Criteria.GetSpecMax(columnID));
            }

            // Reject 으로 뽑아내야하는 Criteria 가 하나도 없을 때에는
            // 검사를 하지 말자는 얘기지요?
            // m_pSurfaceItem->bInsp |= ((Criteria.nType == 1) && (Criteria.bInsp));

            // 영훈 20160315 : Surface Mask를 사용할 때 Ignore를 선택하면 매 검사시 Mask를 생성하도록 해야하므로 주석처리 합니다.
            bInsp |= Criteria.bInsp;
        }

        // 영훈 20160530 : Criteria가 2개 이상일 경우 하나라도 켜져있다면 검사를 하도록 해주자.
        m_pSurfaceItem->bInsp = bInsp;
    }
    else
    {
        m_cboUnit.SetCurSel(m_nCurrentUnit);

        long nSpecNum = (long)(m_pSurfaceItem->vecCriteria.size());

        m_gridCriteria.DeleteAllItems();

        // Items + Header
        m_gridCriteria.SetRowCount(nSpecNum + 1);
        m_gridCriteria.SetFixedRowCount(1);

        m_gridCriteria.SetColumnCount(nSPos + COL_SPEC_COLUMN_START + GetCriteriaColumnCount(m_is3D) * 2);

        m_gridCriteria.SetItemText(0, 0, _T("")); // Check
        m_gridCriteria.SetItemText(0, 1, _T("Spec")); // Name

        for (long idx = 2; idx < nSPos; idx++)
        {
            m_gridCriteria.SetItemTextFmt(0, idx, _T("%d"), idx - 1);
        }

        m_gridCriteria.SetItemText(0, nSPos + COL_COLOR, _T("Color"));
        m_gridCriteria.SetItemText(0, nSPos + COL_TYPE, _T("Type"));
        m_gridCriteria.SetColumnWidth(nSPos + COL_COLOR, 100);
        m_gridCriteria.SetColumnWidth(nSPos + COL_TYPE, 100);

        for (long idx = 0; idx < GetCriteriaColumnCount(m_is3D); idx++)
        {
            auto columnID = GetCriteriaColumnID(m_is3D, idx);

            CString specNameMin;
            CString specNameMax;
            specNameMin.Format(_T("%s>"), GetCriteriaColumnName(columnID));
            specNameMax.Format(_T("%s<"), GetCriteriaColumnName(columnID));

            m_gridCriteria.SetItemText(0, COL_SPEC_COLUMN_START + nSPos + idx * 2 + 0, specNameMin);
            m_gridCriteria.SetItemText(0, COL_SPEC_COLUMN_START + nSPos + idx * 2 + 1, specNameMax);

            // 영훈 : 글자 개수에 따라 Size를 맞춰준다.

            long nCellWidth_min = ((specNameMin.GetLength() * 7) > 100) ? specNameMin.GetLength() * 7 : 100;
            long nCellWidth_max = ((specNameMax.GetLength() * 7) > 100) ? specNameMax.GetLength() * 7 : 100;

            m_gridCriteria.SetColumnWidth(COL_SPEC_COLUMN_START + nSPos + idx * 2 + 0, nCellWidth_min);
            m_gridCriteria.SetColumnWidth(COL_SPEC_COLUMN_START + nSPos + idx * 2 + 1, nCellWidth_max);

            specNameMin.Empty();
            specNameMax.Empty();
        }

        // Resize Check Cells
        CRect rtCtrl;
        m_gridCriteria.GetWindowRect(rtCtrl);

        long nCheckWidth = 30; // m_gridCriteria.GetRowHeight(1);
        long nTitleWidth = 150;
        m_gridCriteria.SetColumnWidth(0, nCheckWidth);
        m_gridCriteria.SetColumnWidth(1, nTitleWidth);
        for (long columnIndex = 2; columnIndex < nSPos; columnIndex++)
        {
            m_gridCriteria.SetColumnWidth(columnIndex, nCheckWidth);
        }

        long nSubItemNum = (long)(m_pSurfaceItem->vecCriteria.size());

        for (long nSubItem = 0; nSubItem < nSubItemNum; nSubItem++)
        {
            const CSurfaceCriteria& SubItem = m_pSurfaceItem->vecCriteria[nSubItem];

            long nRow = nSubItem + 1;

            // Set Check Box
            m_gridCriteria.SetCellType(nRow, 0, RUNTIME_CLASS(CGridCellCheck2));
            ((CGridCellCheck2*)m_gridCriteria.GetCell(nRow, 0))->SetCheck(SubItem.bInsp);

            for (int n = 2; n < nSPos; n++)
            {
                // ROI Check
                m_gridCriteria.SetCellType(nRow, n, RUNTIME_CLASS(CGridCellCheck2));
                ((CGridCellCheck2*)m_gridCriteria.GetCell(nRow, n))->SetCheck(SubItem.vecbUseDecisionArea[n - 2]);
            }

            // Set Criteria Name
            m_gridCriteria.GetCell(nRow, 1)->SetFormat(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            m_gridCriteria.GetCell(nRow, 1)->SetText(SubItem.strName);

            // Set Criteria Target Object Color
            SetCellColor(nRow, nSPos + COL_COLOR);
            ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nSPos + COL_COLOR))->SetCurSel(SubItem.m_color);

            // Set Criteria Type
            SetCellType(nRow, nSPos + COL_TYPE);
            ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nSPos + COL_TYPE))->SetCurSel(SubItem.m_type);

            // Set Criteria Attributes
            for (long item = 0; item < GetCriteriaColumnCount(m_is3D); item++)
            {
                auto criteriaID = GetCriteriaColumnID(m_is3D, item);
                SetCriteriaMin(nRow, nSPos + COL_SPEC_COLUMN_START + item * 2 + 0, SubItem.GetSpecMin(criteriaID));
                SetCriteriaMax(nRow, nSPos + COL_SPEC_COLUMN_START + item * 2 + 1, SubItem.GetSpecMax(criteriaID));
            }
        }
    }
}

void CSurfaceCriteriaDlg::ConvertUnit(CGridCtrl& GridCtrl, long nRow, long nCol, float fFactor)
{
    CString strValue(GridCtrl.GetItemText(nRow, nCol));
    if (!strValue.IsEmpty())
        GridCtrl.SetItemTextFmt(nRow, nCol, _T("%0.2f"), _ttof(strValue) * fFactor);

    strValue.Empty();
}

void CSurfaceCriteriaDlg::ConvertUnit(float fFactor)
{
    int nSPos = GetColSPos();

    ////////////////////////////////////////////////////
    {
        long nRowCount = m_gridCriteria.GetRowCount();

        for (long nRow = 1; nRow < nRowCount; nRow++)
        {
            for (long item = 0; item < GetCriteriaColumnCount(m_is3D); item++)
            {
                auto columnID = GetCriteriaColumnID(m_is3D, item);
                auto specType = GetCriteriaColumnSpecType(columnID);

                switch (specType)
                {
                    case SItemSpec::enumSpecType::MEASURE2:
                        ConvertUnit(
                            m_gridCriteria, nRow, nSPos + COL_SPEC_COLUMN_START + item * 2 + 0, fFactor * fFactor);
                        ConvertUnit(
                            m_gridCriteria, nRow, nSPos + COL_SPEC_COLUMN_START + item * 2 + 1, fFactor * fFactor);
                        break;

                    case SItemSpec::enumSpecType::MEASURE:
                        ConvertUnit(m_gridCriteria, nRow, nSPos + COL_SPEC_COLUMN_START + item * 2 + 0, fFactor);
                        ConvertUnit(m_gridCriteria, nRow, nSPos + COL_SPEC_COLUMN_START + item * 2 + 1, fFactor);
                }
            }
        }
    }

    /////////////////////////////////////////////////////
    {
        long nRowCount = m_gridObjects.GetRowCount();
        long nColOffset = GetCriterialNum() + 1 /*(Total)*/;

        for (long nRow = 1; nRow < nRowCount; nRow++)
        {
            for (long item = 0; item < GetCriteriaColumnCount(m_is3D); item++)
            {
                auto columnID = GetCriteriaColumnID(m_is3D, item);
                auto specType = GetCriteriaColumnSpecType(columnID);

                switch (specType)
                {
                    case SItemSpec::enumSpecType::MEASURE2:
                        ConvertUnit(
                            m_gridObjects, nRow, nColOffset + COL_OBJECT_SPEC_COLUMN_START + item, fFactor * fFactor);
                        break;

                    case SItemSpec::enumSpecType::MEASURE:
                        ConvertUnit(m_gridObjects, nRow, nColOffset + COL_OBJECT_SPEC_COLUMN_START + item, fFactor);
                }
            }
            m_gridObjects.UpdateData(TRUE);
        }
    }
}

void CSurfaceCriteriaDlg::OnSelchangeComboUnit()
{
    UpdateData(TRUE);

    m_nCurrentUnit = m_cboUnit.GetCurSel();

    m_gridObjects.Refresh();
    InitializeObjectsGrid();

    m_gridCriteria.Refresh();
    InitializeCriteriaGrid();

    UpdateSpec(FALSE);
}

void CSurfaceCriteriaDlg::ConvertUnit(long nCurrentUnit, long nNewUnit)
{
    switch (nCurrentUnit)
    {
        case SItemSpec::MM:
            switch (nNewUnit)
            {
                case SItemSpec::MM:
                    break;
                case SItemSpec::MIL:
                    ConvertUnit(gf_MM_TO_MILS);
                    break;
                case SItemSpec::UM:
                    ConvertUnit(gf_MM_TO_UM);
                    break;
            }
            break;
        case SItemSpec::MIL:
            switch (nNewUnit)
            {
                case SItemSpec::MM:
                    ConvertUnit(gf_MILS_TO_MM);
                    break;
                case SItemSpec::MIL:
                    break;
                case SItemSpec::UM:
                    ConvertUnit(gf_MILS_TO_UM);
                    break;
            }
            break;
        case SItemSpec::UM:
            switch (nNewUnit)
            {
                case SItemSpec::MM:
                    ConvertUnit(gf_UM_TO_MM);
                    break;
                case SItemSpec::MIL:
                    ConvertUnit(gf_UM_TO_MILS);
                    break;
                case SItemSpec::UM:
                    break;
            }
            break;
    }
}

float CSurfaceCriteriaDlg::GetConvertUnitFactor(long nCurrentUnit, long nNewUnit)
{
    float fFactor = 1.f;
    switch (nCurrentUnit)
    {
        case SItemSpec::MM:
            switch (nNewUnit)
            {
                case SItemSpec::MM:
                    break;
                case SItemSpec::MIL:
                    fFactor = gf_MM_TO_MILS;
                    break;
                case SItemSpec::UM:
                    fFactor = gf_MM_TO_UM;
                    break;
            }
            break;
        case SItemSpec::MIL:
            switch (nNewUnit)
            {
                case SItemSpec::MM:
                    fFactor = gf_MILS_TO_MM;
                    break;
                case SItemSpec::MIL:
                    break;
                case SItemSpec::UM:
                    fFactor = gf_MILS_TO_UM;
                    break;
            }
            break;
        case SItemSpec::UM:
            switch (nNewUnit)
            {
                case SItemSpec::MM:
                    fFactor = gf_UM_TO_MM;
                    break;
                case SItemSpec::MIL:
                    fFactor = gf_UM_TO_MILS;
                    break;
                case SItemSpec::UM:
                    break;
            }
            break;
    }

    return fFactor;
}

void CSurfaceCriteriaDlg::OnButtonSurfaceInsertCriteria()
{
    int nSPos = GetColSPos();
    m_gridCriteria.SetRowCount(m_gridCriteria.GetRowCount() + 1);

    long nNewRow = m_gridCriteria.GetRowCount() - 1;

    m_gridCriteria.SetCellType(nNewRow, 0, RUNTIME_CLASS(CGridCellCheck2));
    m_gridCriteria.GetCell(nNewRow, 1)->SetFormat(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    m_gridCriteria.SetItemText(nNewRow, 1, _T("Unknown"));

    for (int n = 2; n < nSPos; n++)
    {
        // ROI Check
        m_gridCriteria.SetCellType(nNewRow, n, RUNTIME_CLASS(CGridCellCheck2));

        if (n == 2)
            ((CGridCellCheck2*)m_gridCriteria.GetCell(nNewRow, n))->SetCheck(TRUE);
        else
            ((CGridCellCheck2*)m_gridCriteria.GetCell(nNewRow, n))->SetCheck(FALSE);
    }

    SetCellColor(nNewRow, nSPos + COL_COLOR);
    ((CGridSlideBox*)m_gridCriteria.GetCell(nNewRow, nSPos + COL_COLOR))->SetCurSel(0);

    //k Marginal 버튼 누르면 초기값 변경?
    SetCellType(nNewRow, nSPos + COL_TYPE);
    ((CGridSlideBox*)m_gridCriteria.GetCell(nNewRow, nSPos + COL_TYPE))->SetCurSel(2);
}

void CSurfaceCriteriaDlg::OnButtonSurfaceRemoveCriteria()
{
    CCellRange CellRange = m_gridCriteria.GetSelectedCellRange();

    if (!CellRange.IsValid() || CellRange.Count() < 1 || CellRange.GetTopLeft().row < 1)
        return;

    if (m_gridCriteria.GetRowCount() <= 2)
        return;

    long nCurRow = CellRange.GetTopLeft().row;

    m_gridCriteria.DeleteRow(nCurRow);

    m_gridCriteria.Refresh();
}

void CSurfaceCriteriaDlg::OnOK()
{
    UpdateSpec(TRUE);

    CDialog::OnOK();
}

CSurfaceItem& CSurfaceCriteriaDlg::GetSurfaceItem()
{
    return *m_pSurfaceItem;
}

void CSurfaceCriteriaDlg::SetCriteriaMin(long nRow, long nCol, const SItemSpec& ItemSpec)
{
    BOOL bInsp = FALSE;
    float fMarginalMin = 0.f;
    float fPassMin = 0.f;
    float fNominal = 0.f;
    float fPassMax = 0.f;
    float fMarginalMax = 0.f;

    BOOL bAbsolute = FALSE;

    ItemSpec.Get(bInsp, fMarginalMin, fPassMin, fNominal, fPassMax, fMarginalMax, m_nCurrentUnit, bAbsolute);

    if (bInsp)
    {
        m_gridCriteria.SetItemTextFmt(nRow, nCol, _T("%.3f"), fPassMax);
    }
}

void CSurfaceCriteriaDlg::SetCriteriaMax(long nRow, long nCol, const SItemSpec& ItemSpec)
{
    BOOL bInsp = FALSE;
    float fMarginalMin = 0.f;
    float fPassMin = 0.f;
    float fNominal = 0.f;
    float fPassMax = 0.f;
    float fMarginalMax = 0.f;

    BOOL bAbsolute = FALSE;

    ItemSpec.Get(bInsp, fMarginalMin, fPassMin, fNominal, fPassMax, fMarginalMax, m_nCurrentUnit, bAbsolute);

    if (bInsp)
    {
        m_gridCriteria.SetItemTextFmt(nRow, nCol, _T("%.3f"), fPassMin);
    }
}

void CSurfaceCriteriaDlg::GetCriteriaMin(long nRow, long nCol, SItemSpec& ItemSpec)
{
    CString strMin(m_gridCriteria.GetItemText(nRow, nCol));
    float fMin = (float)_ttof(strMin);
    ItemSpec.Set(strMin.IsEmpty() ? FALSE : TRUE, SItemSpec::fLimitMin, SItemSpec::fLimitMin, 0.f, fMin, fMin,
        m_nCurrentUnit, ItemSpec.IsAbsoluteType());

    strMin.Empty();
}

void CSurfaceCriteriaDlg::GetCriteriaMax(long nRow, long nCol, SItemSpec& ItemSpec)
{
    CString strMax(m_gridCriteria.GetItemText(nRow, nCol));
    float fMax = (float)_ttof(strMax);
    ItemSpec.Set(strMax.IsEmpty() ? FALSE : TRUE, fMax, fMax, 0.f, SItemSpec::fLimitMax, SItemSpec::fLimitMax,
        m_nCurrentUnit, ItemSpec.IsAbsoluteType());

    strMax.Empty();
}

void CSurfaceCriteriaDlg::InitializeCriteriaGrid()
{
    m_gridCriteria.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridCriteria.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridCriteria.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridCriteria.SetDefCellMargin(0);
    m_gridCriteria.SetFixedColumnSelection(FALSE);
    m_gridCriteria.SetFixedRowSelection(FALSE);
    m_gridCriteria.SetSingleColSelection(FALSE);
    m_gridCriteria.SetSingleRowSelection(FALSE);
    m_gridCriteria.SetTrackFocusCell(FALSE);
    m_gridCriteria.SetRowResize(FALSE);
    m_gridCriteria.SetColumnResize(FALSE);

    CString str;
    str.Format(_T("Surface Criteria"));
    SetWindowText(str);

    str.Empty();
}

void CSurfaceCriteriaDlg::InitializeObjectsGrid()
{
    const auto& SurfaceItemResult = m_pVisionInsp->GetResult();
    const auto& vecObject = SurfaceItemResult.vecDSOValue;
    const auto& vecnObjectResult = SurfaceItemResult.vecnResult;
    const auto& vecCriteriaResult = SurfaceItemResult.vecCriteriaResult;

    m_gridObjects.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridObjects.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridObjects.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridObjects.SetDefCellMargin(0);
    m_gridObjects.SetFixedColumnSelection(FALSE);
    m_gridObjects.SetFixedRowSelection(FALSE);
    m_gridObjects.SetSingleColSelection(FALSE);
    m_gridObjects.SetSingleRowSelection(FALSE);
    m_gridObjects.SetTrackFocusCell(FALSE);
    m_gridObjects.SetRowResize(FALSE);
    m_gridObjects.SetColumnResize(TRUE);
    m_gridObjects.SetEditable(FALSE);

    if ((long)vecObject.size() <= 0 || (long)vecnObjectResult.size() <= 0 || (long)vecCriteriaResult.size() <= 0)
        return;

    long nRowNum = 1 + (long)(vecObject.size());
    long nResultNum = 1 + (long)(vecCriteriaResult.size());

    m_gridObjects.SetRowCount(nRowNum);
    m_gridObjects.SetFixedRowCount(1);
    m_gridObjects.SetColumnCount(nResultNum + COL_OBJECT_SPEC_COLUMN_START + GetCriteriaColumnCount(m_is3D));
    m_gridObjects.SetFixedColumnCount(nResultNum);
    m_gridObjects.SetHeaderSort(TRUE);
    m_gridObjects.SetCompareFunction(CompareFunction);

    // Total Result
    m_gridObjects.SetItemText(0, 0, _T("T"));

    for (long rowIndex = 0; rowIndex < nRowNum; rowIndex++)
    {
        m_gridObjects.SetItemData(rowIndex, 0, rowIndex);
    }

    m_gridObjects.SetColumnWidth(0, 25);

    // Criteria Result
    for (long nCol = 1; nCol < m_gridObjects.GetColumnCount(); nCol++)
    {
        m_gridObjects.SetItemData(0, nCol, ColumnType_Number);
        m_gridObjects.SetItemTextFmt(0, nCol, _T("C%d"), nCol - 1);
        m_gridObjects.SetColumnWidth(nCol, 25);
    }

    m_gridObjects.SetItemData(0, 0, ColumnType_Text);
    m_gridObjects.SetItemData(0, nResultNum + COL_OBJECT_INDEX, ColumnType_Text);
    m_gridObjects.SetItemData(0, nResultNum + COL_OBJECT_COLOR, ColumnType_Text);
    m_gridObjects.SetItemText(0, nResultNum + COL_OBJECT_INDEX, _T("No"));
    m_gridObjects.SetItemText(0, nResultNum + COL_OBJECT_COLOR, _T("Color"));
    m_gridObjects.SetItemText(0, nResultNum + COL_OBJECT_POS_X, _T("X"));
    m_gridObjects.SetItemText(0, nResultNum + COL_OBJECT_POS_Y, _T("Y"));

    // Attributes

    for (long item = 0; item < GetCriteriaColumnCount(m_is3D); item++)
    {
        auto columnID = GetCriteriaColumnID(m_is3D, item);
        CString specName = GetCriteriaColumnName(columnID);
        m_gridObjects.SetItemText(0, nResultNum + COL_OBJECT_SPEC_COLUMN_START + item, specName);

        // 영훈 : 글자 개수에 따라 Size를 맞춰준다.
        long nNameCount = specName.GetLength();
        long nCellWidth = ((nNameCount * 7) > 100) ? nNameCount * 7 : 100;
        m_gridObjects.SetColumnWidth(nResultNum + COL_OBJECT_SPEC_COLUMN_START + item, nCellWidth);

        specName.Empty();
    }

    std::vector<long> vecRow;

    m_vecfCen.clear();
    m_vecrtObj.clear();
    m_objects_label.clear();

    long ResultOrder[] = {REJECT, MARGINAL, -1};

    for (long nRow = 1; nRow < nRowNum; nRow++)
    {
        vecRow.push_back(nRow - 1);
    }

    long rowIndex = 1;

    for (long orderIndex = 0; orderIndex < 3; orderIndex++)
    {
        auto order = ResultOrder[orderIndex];
        auto checkRow = vecRow;
        vecRow.clear();

        for (auto& objectID : checkRow)
        {
            // Total Result
            if (order >= 0 && vecnObjectResult[objectID] != order)
            {
                vecRow.push_back(objectID);
                continue;
            }

            m_gridObjects.SetItemText(rowIndex, 0, g_szResultSymbol[vecnObjectResult[objectID]]);

            // Criteria Result
            for (long nCol = 1; nCol < nResultNum; nCol++)
            {
                m_gridObjects.SetItemTextFmt(
                    rowIndex, nCol, g_szResultSymbol[vecCriteriaResult[nCol - 1].vecnResult[objectID]]);
            }

            CString textNo;
            textNo.Format(_T("%d"), objectID + 1);
            m_gridObjects.SetItemText(rowIndex, nResultNum + COL_OBJECT_INDEX, textNo);
            m_gridObjects.SetItemText(rowIndex, nResultNum + COL_OBJECT_COLOR, g_szColor[vecObject[objectID].m_color]);

            textNo.Empty();

            //초기화 Criterial Result는 무조건 단위가 Mil이다 이걸 현재 Spec Unit과 일치시킨다
            float fFactor = GetConvertUnitFactor(SItemSpec::MIL, m_nCurrentUnit);

            Ipvm::Point32r2 fptCenter = Ipvm::Point32r2(vecObject[objectID].fCenX, vecObject[objectID].fCenY);
            m_vecfCen.push_back(fptCenter);
            m_vecrtObj.push_back(vecObject[objectID].rtROI);
            m_objects_label.push_back(vecObject[objectID].nLabel);

            m_gridObjects.SetItemTextFmt(
                rowIndex, nResultNum + COL_OBJECT_POS_X, _T("%.3f"), vecObject[objectID].fObjectRealCenterX * fFactor);
            m_gridObjects.SetItemTextFmt(
                rowIndex, nResultNum + COL_OBJECT_POS_Y, _T("%.3f"), vecObject[objectID].fObjectRealCenterY * fFactor);

            for (long item = 0; item < GetCriteriaColumnCount(m_is3D); item++)
            {
                auto columnID = GetCriteriaColumnID(m_is3D, item);

                auto specType = GetCriteriaColumnSpecType(columnID);
                float value = 0;
                if (specType == SItemSpec::enumSpecType::MEASURE)
                    value = vecObject[objectID].m_inspValue[long(columnID)] * fFactor;
                else if (specType == SItemSpec::enumSpecType::MEASURE2)
                    value = vecObject[objectID].m_inspValue[long(columnID)] * fFactor * fFactor;
                else
                    value = vecObject[objectID].m_inspValue[long(columnID)];

                m_gridObjects.SetItemTextFmt(
                    rowIndex, nResultNum + COL_OBJECT_SPEC_COLUMN_START + item, _T("%.3f"), value);
            }
            rowIndex++;
        }
    }

    m_gridObjects.Refresh();
}

void CSurfaceCriteriaDlg::OnGridCriteriaEditEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;

    const long nDefaultCol(3);
    const long nSelectCriteriaColidx = (nDefaultCol + COL_SPEC_COLUMN_START) - 1;

    long nRow = pGridView->iRow;
    long nCol = pGridView->iColumn;

    if (nRow < 1 || nCol < nSelectCriteriaColidx)
        return;

    CString strVal = m_gridCriteria.GetItemText(nRow, nCol);
    long nDotCount(0); //소수점이 2개이상이면 문제가 있는거다
    long nMinusSignCount(0); //소수점이 2개이상이면 문제가 있는거다
    bool bisAvailable(true);

    for (long strlength = 0; strlength < strVal.GetLength(); strlength++)
    {
        if ((strVal.GetAt(strlength) >= '0' && strVal.GetAt(strlength) <= '9'))
            continue;
        else if (strVal.GetAt(strlength) == '-')
        {
            nMinusSignCount++;

            if (strlength != 0) //부호는 맨첫번째 자리에 위치하여야 한다
                bisAvailable = false;
        }
        else if (strVal.GetAt(strlength) == '.')
            nDotCount++;
        else
            bisAvailable = false;

        if (bisAvailable == false || nDotCount > 1 || nMinusSignCount > 1)
        {
            m_gridCriteria.SetItemText(nRow, nCol, _T(""));
            break;
        }
    }

    *pResult = 0;
}

void CSurfaceCriteriaDlg::OnGridObjectSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;

    long nRow = pGridView->iRow;

    if (nRow < 1)
        return;

    UpdateDefectOverlay();

    *pResult = 0;
}

void CSurfaceCriteriaDlg::OnBtnAddRoi()
{
    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return;

    UpdateData(TRUE);
    GetROI();
    UpdateSpec(TRUE);

    Para_PolygonRects::SPolygonRectNamePair data;

    switch (m_nROIType)
    {
        case 0:
        case 1:
        case 3: //kircheis_Hy
            // Rect Or Ellipse
            {
                data.nPolygonType = (short)m_nROIType;
                data.strName = m_strROIName;
                data.vecptPolygon_BCU.resize(2);

                if (m_pSurfaceItem->m_decisionArea.GetCount())
                {
                    for (int n = 0; n < 2; n++)
                    {
                        data.vecptPolygon_BCU[n].m_x = (n + 1) * 50.f;
                        data.vecptPolygon_BCU[n].m_y = (n + 1) * 50.f;
                    }
                }
                else
                {
                    auto rtROI_BCU = m_pSurfaceItem->AlgoPara.m_rtROI_BCU;
                    data.vecptPolygon_BCU[0].m_x = rtROI_BCU.m_left;
                    data.vecptPolygon_BCU[0].m_y = rtROI_BCU.m_top;
                    data.vecptPolygon_BCU[1].m_x = rtROI_BCU.m_right;
                    data.vecptPolygon_BCU[1].m_y = rtROI_BCU.m_bottom;
                }

                m_pSurfaceItem->m_decisionArea.Add(data);
            }
            break;
        case 2:
            if (m_nPolygonNum >= 3)
            {
                data.nPolygonType = (short)m_nPolygonNum;
                data.strName = m_strROIName;
                data.vecptPolygon_BCU.resize(m_nPolygonNum);

                for (int n = 0; n < m_nPolygonNum; n++)
                {
                    data.vecptPolygon_BCU[n].m_x = (n + 1) * 50.f;
                    data.vecptPolygon_BCU[n].m_y = (n + 1) * 50.f;
                }

                m_pSurfaceItem->m_decisionArea.Add(data);
            }
            else
            {
                AfxMessageBox(_T("Polygon Num > 3"));
                return;
            }
            break;
        default:
            return;
    }

    // ROI 가 추가되었으므로 모든 BOOL VECTOR 의 SIZE 를 늘려준다

    for (int nCriteria = 0; nCriteria < (long)(m_pSurfaceItem->vecCriteria.size()); nCriteria++)
    {
        BOOL bNewValue = FALSE;
        m_pSurfaceItem->vecCriteria[nCriteria].vecbUseDecisionArea.push_back(bNewValue);
    }

    UpdateSpec(FALSE);
    RefreshROIList();
    SetROI();
}

void CSurfaceCriteriaDlg::RefreshROIList()
{
    m_lstROI.DeleteAllItems();

    for (int n = 0; n < m_pSurfaceItem->m_decisionArea.GetCount(); n++)
    {
        CString strNum;
        strNum.Format(_T("%d"), n + 1);

        m_lstROI.InsertItem(n, strNum);
        m_lstROI.SetItemText(n, 1, m_pSurfaceItem->m_decisionArea.GetAt(n).strName);

        strNum.Empty();
    }
}

void CSurfaceCriteriaDlg::GetROI()
{
    const auto& scale = m_pVisionInsp->getScale();

    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    // IDWindow 에서 ROI 값을 읽어옴
    for (int n = 0; n < m_pSurfaceItem->m_decisionArea.GetCount(); n++)
    {
        CString key;
        key.Format(_T("%d"), n);

        auto& areaInfo = m_pSurfaceItem->m_decisionArea.GetAt(n);
        if (areaInfo.nPolygonType >= 3)
        {
            Ipvm::Polygon32r polygon;
            m_imageView->ROIGet(key, polygon);

            std::vector<Ipvm::Point32s2> points;

            ImageViewEx::PolygonToPoints(polygon, points);

            areaInfo.vecptPolygon_BCU = scale.convert_PixelToBCU(points, imageCenter);
        }
        else
        {
            Ipvm::Rect32s roi;
            m_imageView->ROIGet(key, roi);

            auto roi_BCU = scale.convert_PixelToBCU(roi, imageCenter);

            areaInfo.vecptPolygon_BCU[0] = Ipvm::Point32r2(roi_BCU.m_left, roi_BCU.m_top);
            areaInfo.vecptPolygon_BCU[1] = Ipvm::Point32r2(roi_BCU.m_right, roi_BCU.m_bottom);
        }

        key.Empty();
    }
}

void CSurfaceCriteriaDlg::SetROI()
{
    const auto& scale = m_pVisionInsp->getScale();

    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    // IDWindow 에 ROI 를 설정함
    m_imageView->ROIClear();

    for (int n = 0; n < m_pSurfaceItem->m_decisionArea.GetCount(); n++)
    {
        CString key;
        key.Format(_T("%d"), n);

        auto& areaInfo = m_pSurfaceItem->m_decisionArea.GetAt(n);
        auto points = scale.convert_BCUToPixel(areaInfo.vecptPolygon_BCU, imageCenter);

        if (areaInfo.nPolygonType >= 3)
        {
            Ipvm::Polygon32r polygon;
            ImageViewEx::PointsToPolygon(points, polygon);

            m_imageView->ROISet(key, areaInfo.strName, polygon, RGB(0, 255, 255), TRUE, TRUE);
        }
        else
        {
            Ipvm::Rect32s rtROI = Ipvm::Rect32s(points[0].m_x, points[0].m_y, points[1].m_x, points[1].m_y);
            m_imageView->ROISet(key, areaInfo.strName, rtROI, RGB(0, 255, 255), TRUE, TRUE);
        }

        key.Empty();
    }

    m_imageView->ROIShow();
}

void CSurfaceCriteriaDlg::OnBtnDelRoi()
{
    // List Control 에서 선택된 ROI 를 제거한다

    UpdateData(TRUE);
    UpdateSpec(TRUE);
    GetROI();

    if (m_lstROI.GetSelectedCount() == 1)
    {
        int nCurSel = m_lstROI.GetSelectionMark();

        if (nCurSel < 0)
            return;

        // 지우려는 ROI 가 사용중이니 체크한다
        BOOL bUseROI = FALSE;

        int nCriteria;

        for (nCriteria = 0; nCriteria < (long)(m_pSurfaceItem->vecCriteria.size()); nCriteria++)
        {
            if (m_pSurfaceItem->vecCriteria[nCriteria].vecbUseDecisionArea[nCurSel])
            {
                bUseROI = TRUE;
            }
        }

        if (bUseROI)
        {
            if (AfxMessageBox(_T("Warning! using Decision Area! Continue?"), MB_OKCANCEL) != IDOK)
            {
                return;
            }
        }

        // 제거

        m_pSurfaceItem->m_decisionArea.Del(nCurSel);

        for (nCriteria = 0; nCriteria < (long)(m_pSurfaceItem->vecCriteria.size()); nCriteria++)
        {
            std::vector<BOOL>& vecbUseDecisionArea = m_pSurfaceItem->vecCriteria[nCriteria].vecbUseDecisionArea;
            vecbUseDecisionArea.erase(vecbUseDecisionArea.begin() + nCurSel);
        }
        RefreshROIList();
        SetROI();

        UpdateSpec(FALSE);
    }
}

int CSurfaceCriteriaDlg::GetColSPos()
{
    return m_pSurfaceItem->m_decisionArea.GetCount() + 2;
}
int CSurfaceCriteriaDlg::GetCriterialNum()
{
    return (long)(m_pSurfaceItem->vecCriteria.size());
}

void CSurfaceCriteriaDlg::ROI_Changed(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength)
{
    ((CSurfaceCriteriaDlg*)userData)->ROI_Changed(id, key, keyLength);
}

void CSurfaceCriteriaDlg::ROI_Changed(const int32_t id, const wchar_t* key, const size_t keyLength)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(keyLength);

    GetROI();
}

void CSurfaceCriteriaDlg::SetCellColor(int nRow, int nCol)
{
    m_gridCriteria.SetCellType(nRow, nCol, RUNTIME_CLASS(CGridSlideBox));
    ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nCol))->AddString(_T("Dark"));
    ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nCol))->AddString(_T("Bright"));
    ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nCol))->AddString(_T("Any"));

    // 영훈 20140814_Surface_Hybrid : Hybrid는 사용하지 않도록 한다. 나중에 사용할 일이 있으면 주석 해제
    // 	((CGridSlideBox *)m_gridCriteria.GetCell(nRow, nCol))->AddString(_T("Hybrid"));
    // 	((CGridSlideBox *)m_gridCriteria.GetCell(nRow, nCol))->AddString(_T("All"));
}

//k Marginal 추가
void CSurfaceCriteriaDlg::SetCellType(int nRow, int nCol)
{
    m_gridCriteria.SetCellType(nRow, nCol, RUNTIME_CLASS(CGridSlideBox));
    ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nCol))->AddString(_T("Ignore"));
    ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nCol))->AddString(_T("Marginal"));
    ((CGridSlideBox*)m_gridCriteria.GetCell(nRow, nCol))->AddString(_T("Reject"));
}

void CSurfaceCriteriaDlg::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    OnOK();
}

int CSurfaceCriteriaDlg::CompareFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    UNUSED_ALWAYS(lParamSort);

    CGridCellBase* pCell1 = (CGridCellBase*)lParam1;
    CGridCellBase* pCell2 = (CGridCellBase*)lParam2;
    if (!pCell1 || !pCell2)
        return 0;

    long columnIndex = pCell1->GetGrid()->GetSortColumn();
    if (pCell1->GetGrid()->GetItemData(0, columnIndex) == ColumnType_Text)
    {
        return _tcscmp(pCell1->GetText(), pCell2->GetText());
    }

    float fValue1 = (float)_ttof(pCell1->GetText());
    float fValue2 = (float)_ttof(pCell2->GetText());

    if (fValue1 < fValue2)
        return -1;
    else if (fValue1 == fValue2)
        return 0;
    else
        return 1;
}

void CSurfaceCriteriaDlg::OnBnClickedButtonInspection()
{
    Ipvm::AsyncProgress progress(_T("Please wait..."));

    UpdateSpec(TRUE);
    auto backupPara = m_pVisionInsp->m_surfacePara->m_SurfaceItem;
    m_pVisionInsp->m_surfacePara->m_SurfaceItem = *m_pSurfaceItem;

    m_pVisionInsp->DoInspection(true, VisionInspectionSurface::enumRunMode::CriteriaTest, &m_labelImage);
    m_imageView->SetImage(m_image);

    m_pVisionInsp->m_surfacePara->m_SurfaceItem = backupPara;
    InitializeObjectsGrid();

    UpdateDefectOverlay();
}

void CSurfaceCriteriaDlg::OnBnClickedCheckShowSelectedDefectsOnly()
{
    UpdateDefectOverlay();
}

void CSurfaceCriteriaDlg::OnBnClickedCheckShowSelectedDefectsLabel()
{
    if (!((CButton*)GetDlgItem(IDC_CHECK_SHOW_SELECTED_DEFECTS_LABEL))->GetCheck())
    {
        m_imageView->SetImage(m_image);
    }

    UpdateDefectOverlay();
}

void CSurfaceCriteriaDlg::UpdateDefectOverlay()
{
    m_imageView->ImageOverlayClear();

    auto selectedDefectsOnly = ((CButton*)GetDlgItem(IDC_CHECK_SHOW_SELECTED_DEFECTS_ONLY))->GetCheck();
    auto selectedCell = m_gridObjects.GetSelectedCellRange();

    const auto& SurfaceItemResult = m_pVisionInsp->GetResult();
    const auto& vecCriteriaResult = SurfaceItemResult.vecCriteriaResult;

    long col_no = 1 + (long)(vecCriteriaResult.size()) + COL_OBJECT_INDEX;

    PatternImage blobImage;
    bool makeBlobImage = false;

    if (((CButton*)GetDlgItem(IDC_CHECK_SHOW_SELECTED_DEFECTS_LABEL))->GetCheck())
    {
        blobImage.setImage(m_image);
        makeBlobImage = true;
    }

    for (long defectIndex = 0; defectIndex < long(m_vecfCen.size()); defectIndex++)
    {
        BYTE value = 128;
        BOOL select = FALSE;
        BOOL displayDefect = !selectedDefectsOnly;
        auto realDataIndex = m_gridObjects.GetItemData(defectIndex + 1, 0) - 1;

        auto pos = m_vecfCen[realDataIndex];

        if (selectedCell.GetMinRow() <= defectIndex + 1 && selectedCell.GetMaxRow() >= defectIndex + 1)
        {
            displayDefect = TRUE;
            select = TRUE;
            value = 255;
        }

        COLORREF crColor = RGB(0, value, 0);
        auto result = m_gridObjects.GetItemText(defectIndex + 1, 0);

        for (long n = 0; n < sizeof(g_szResultSymbol) / sizeof(LPCTSTR); n++)
        {
            if (result == g_szResultSymbol[n])
            {
                switch (n)
                {
                    case 2:
                        crColor = RGB(value, value, 0);
                        break;
                    case 3:
                        crColor = RGB(value, 0, 0);
                        break;
                }
            }
        }

        if (displayDefect)
        {
            CString noText = m_gridObjects.GetItemText(defectIndex + 1, col_no);
            m_imageView->ImageOverlayAdd(pos, crColor);

            auto roi = m_vecrtObj[realDataIndex];
            m_imageView->ImageOverlayAdd(roi, crColor);

            if (select)
            {
                roi.InflateRect(10, 10);
                m_imageView->ImageOverlayAdd(roi, crColor, 2);

                if (makeBlobImage)
                {
                    blobImage.addPattern(m_labelImage, m_objects_label[realDataIndex], roi, crColor);
                }
            }

            m_imageView->ImageOverlayAdd(
                Ipvm::Point32s2(m_vecrtObj[realDataIndex].m_left, m_vecrtObj[realDataIndex].m_top), noText, crColor);

            noText.Empty();
        }
    }

    if (makeBlobImage)
    {
        m_imageView->SetImage(blobImage);
    }

    m_imageView->ImageOverlayShow();
}
