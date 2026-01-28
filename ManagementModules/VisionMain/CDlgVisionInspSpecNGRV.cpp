//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CDlgVisionInspSpecNGRV.h"

//CPP_2_________________________________ This project's headers
#include "DlgInspSpecList.h"
#include "VisionMain.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCellCheck2.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCellCombo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"

//CPP_4_________________________________ External library headers

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_INSPITEM_2D_BTM 104
#define IDC_GRID_INSPITEM_3D_BTM 105
#define IDC_GRID_INSPITEM_2D_TOP 106
#define IDC_GRID_INSPITEM_3D_TOP 107

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgVisionInspSpecNGRV, CDialog)

CDlgVisionInspSpecNGRV::CDlgVisionInspSpecNGRV(CVisionMain* visionMain, CWnd* pParent /*=NULL*/)
    : CDialog(IDD_DIALOG_NGRV_VISION_INSP_SPEC, pParent)
    , m_ImageView_BTM_2D(nullptr)
    , m_ImageView_TOP_2D(nullptr)
{
    m_visionMain = visionMain; //kircheis_NGRV

    for (long i = 0; i < INSP_ITEM_END; i++)
    {
        m_gridVisionFixed[i] = nullptr;
    }

    //--------------------------------------------------------------------------
    // 설정전 Job을 백업한다
    //--------------------------------------------------------------------------

    //ArchiveAllType ar(&m_dbBackup, ArchiveAllType::store);
    //CiDataBase db;

    //m_visionUnit->LinkDataBase(LAST_JOB_VERSION, TRUE, db);
    //db.Serialize(ar);
}

CDlgVisionInspSpecNGRV::~CDlgVisionInspSpecNGRV()
{
    for (long nInspItem = 0; nInspItem < enumInspectionItem::INSP_ITEM_END; nInspItem++)
    {
        if (m_gridVisionFixed[nInspItem] != nullptr)
        {
            delete m_gridVisionFixed[nInspItem];
        }
    }

    delete m_ImageView_BTM_2D;
    delete m_ImageView_TOP_2D;
}

void CDlgVisionInspSpecNGRV::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspSpecNGRV, CDialog)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_INSPITEM_2D_BTM, OnGridEditEnd_InspItem_2D_BTM)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_INSPITEM_2D_BTM, OnGridEditEnd_InspItem_2D_BTM)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_INSPITEM_3D_BTM, OnGridEditEnd_InspItem_3D_BTM)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_INSPITEM_3D_BTM, OnGridEditEnd_InspItem_3D_BTM)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_INSPITEM_2D_TOP, OnGridEditEnd_InspItem_2D_TOP)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_INSPITEM_2D_TOP, OnGridEditEnd_InspItem_2D_TOP)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_INSPITEM_3D_TOP, OnGridEditEnd_InspItem_3D_TOP)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_INSPITEM_3D_TOP, OnGridEditEnd_InspItem_3D_TOP)
ON_BN_CLICKED(IDC_BUTTON_NGRV_APPLY, &CDlgVisionInspSpecNGRV::OnBnClickedNgrvApply)
ON_BN_CLICKED(IDC_BUTTON_NGRV_CANCEL, &CDlgVisionInspSpecNGRV::OnBnClickedNgrvCancel)
END_MESSAGE_MAP()

// CDlgVisionInspSpecNGRV 메시지 처리기

BOOL CDlgVisionInspSpecNGRV::OnInitDialog()
{
    CDialog::OnInitDialog();

    for (long nInspGrid = 0; nInspGrid < INSP_ITEM_END; nInspGrid++)
    {
        m_gridVisionFixed[nInspGrid] = new CGridCtrl;
    }

    CRect rect;
    GetClientRect(rect);

    CString strFontName = _T("Arial");
    CFont fontGrid;
    fontGrid.CreateFont(20, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, strFontName);

    CRect rtVisionItem_2D_BTM;
    CRect rtVisionItem_3D_BTM;
    CRect rtVisionItem_2D_TOP;
    CRect rtVisionItem_3D_TOP;

    GetDlgItem(IDC_NGRV_REGION_VISION_ITEM_2D_BTM)->GetWindowRect(rtVisionItem_2D_BTM);
    GetDlgItem(IDC_NGRV_REGION_VISION_ITEM_3D_BTM)->GetWindowRect(rtVisionItem_3D_BTM);
    GetDlgItem(IDC_NGRV_REGION_VISION_ITEM_2D_TOP)->GetWindowRect(rtVisionItem_2D_TOP);
    GetDlgItem(IDC_NGRV_REGION_VISION_ITEM_3D_TOP)->GetWindowRect(rtVisionItem_3D_TOP);

    ScreenToClient(rtVisionItem_2D_BTM);
    ScreenToClient(rtVisionItem_3D_BTM);
    ScreenToClient(rtVisionItem_2D_TOP);
    ScreenToClient(rtVisionItem_3D_TOP);

    for (long nInspItemGrid = 0; nInspItemGrid < INSP_ITEM_END; nInspItemGrid++)
    {
        switch (nInspItemGrid)
        {
            case INSP_ITEM_2D_BTM:
            {
                m_gridVisionFixed[nInspItemGrid]->Create(rtVisionItem_2D_BTM, this, IDC_GRID_INSPITEM_2D_BTM);
                m_gridVisionFixed[nInspItemGrid]->SetEditable(TRUE);
                m_gridVisionFixed[nInspItemGrid]->EnableDragAndDrop(FALSE);
            }
            break;

            case INSP_ITEM_3D_BTM:
            {
                m_gridVisionFixed[nInspItemGrid]->Create(rtVisionItem_3D_BTM, this, IDC_GRID_INSPITEM_3D_BTM);
                m_gridVisionFixed[nInspItemGrid]->SetEditable(TRUE);
                m_gridVisionFixed[nInspItemGrid]->EnableDragAndDrop(FALSE);
            }
            break;

            case INSP_ITEM_2D_TOP:
            {
                m_gridVisionFixed[nInspItemGrid]->Create(rtVisionItem_2D_TOP, this, IDC_GRID_INSPITEM_2D_TOP);
                m_gridVisionFixed[nInspItemGrid]->SetEditable(TRUE);
                m_gridVisionFixed[nInspItemGrid]->EnableDragAndDrop(FALSE);
            }
            break;

            case INSP_ITEM_3D_TOP:
            {
                m_gridVisionFixed[nInspItemGrid]->Create(rtVisionItem_3D_TOP, this, IDC_GRID_INSPITEM_3D_TOP);
                m_gridVisionFixed[nInspItemGrid]->SetEditable(TRUE);
                m_gridVisionFixed[nInspItemGrid]->EnableDragAndDrop(FALSE);
            }
            break;

            default:
                break;
        }
    }

    for (long nGridSetFont = 0; nGridSetFont < INSP_ITEM_END; nGridSetFont++)
    {
        m_gridVisionFixed[nGridSetFont]->SetFont(&fontGrid);
    }

    SetGrid();
    SetAFImage();

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionInspSpecNGRV::ClearFrameInfoBuffer() //kircheis_NGRV
{
    long nSize = (long)m_vec2GrabItemFrameInfoNGRV_Buf.size();
    for (long i = 0; i < nSize; i++)
        m_vec2GrabItemFrameInfoNGRV_Buf[i].clear();

    m_vec2GrabItemFrameInfoNGRV_Buf.clear();
}

void CDlgVisionInspSpecNGRV::SetGrid()
{
    std::vector<CString> vecVisionName = m_visionMain->m_vecstrEachVisionName;

    long nVisionNum = (long)vecVisionName.size();
    std::vector<CGrabItemFrameInfoNGRV> vecGrabItemFrameInfoNGRV = m_visionMain->m_vecGrabItemFrameInfo;
    std::vector<long> vecnEachVisionIndex(nVisionNum);
    for (long i = 0; i < nVisionNum; i++)
        vecnEachVisionIndex[i] = -1;

    long nInspModuleNum = (long)vecGrabItemFrameInfoNGRV.size();

    for (long nDrawGrid = 0; nDrawGrid < INSP_ITEM_END; nDrawGrid++)
    {
        long nEachVisionInspModuleNum = (long)m_visionMain->m_vec2InspItemNGRV[nDrawGrid].size();
        m_gridVisionFixed[nDrawGrid]->SetColumnCount(INSP_ITEM_COL_END);
        m_gridVisionFixed[nDrawGrid]->SetRowCount(nEachVisionInspModuleNum + 1);
        m_gridVisionFixed[nDrawGrid]->SetFixedRowCount(1);
        m_gridVisionFixed[nDrawGrid]->SetFixedColumnCount(1);
        m_gridVisionFixed[nDrawGrid]->SetEditable(TRUE);
    }

    CString strInspModuleName;
    CString str;
    CString strPlaneOffsetUM;
    BOOL bSave = false;
    long nVisionID = -1;
    long nIndex = 0;
    long nAFType = 0;
    long nPlaneOffsetUM = 0;
    ClearFrameInfoBuffer();
    m_vec2GrabItemFrameInfoNGRV_Buf.resize(4);

    CFont fFont;
    fFont.CreateFont(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));

    for (long nModule = 0; nModule < nInspModuleNum; nModule++)
    {
        strInspModuleName = vecGrabItemFrameInfoNGRV[nModule].m_strGrabItemName;
        bSave = vecGrabItemFrameInfoNGRV[nModule].m_bIsSaveImage;
        nAFType = vecGrabItemFrameInfoNGRV[nModule].m_nAFType;
        nPlaneOffsetUM = vecGrabItemFrameInfoNGRV[nModule].m_nPlaneOffsetUM;
        nVisionID = m_visionMain->GetVisionID_InspModule(strInspModuleName);

        if (nVisionID < 0 || nVisionID > 3)
        {
            continue;
        }

        m_gridVisionFixed[nVisionID]->SetFont(&fFont, TRUE);

        m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID].push_back(vecGrabItemFrameInfoNGRV[nModule]);

        nIndex = ++vecnEachVisionIndex[nVisionID];
        m_gridVisionFixed[nVisionID]->SetItemBkColour(nIndex + 1, INSP_ITEM_COL_NUMBER, RGB(250, 250, 200));

        str.Format(_T("%d"), vecnEachVisionIndex[nVisionID]);

        m_gridVisionFixed[nVisionID]->SetItemText(nIndex + 1, INSP_ITEM_COL_NUMBER, str);

        // Probe Inspection Name
        m_gridVisionFixed[nVisionID]
            ->GetCell(nIndex + 1, INSP_ITEM_COL_NUMBER)
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        m_gridVisionFixed[nVisionID]->SetItemText(nIndex + 1, INSP_ITEM_COL_NAME, strInspModuleName);
        m_gridVisionFixed[nVisionID]
            ->GetCell(nIndex + 1, INSP_ITEM_COL_NAME)
            ->SetFormat(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        m_gridVisionFixed[nVisionID]->SetItemState(nIndex + 1, INSP_ITEM_COL_NAME, GVIS_READONLY);

        m_gridVisionFixed[nVisionID]->SetCellType(nIndex + 1, INSP_ITEM_COL_AF_TYPE, RUNTIME_CLASS(CGridCellCombo));
        ((CGridCellCombo*)m_gridVisionFixed[nVisionID]->GetCell(nIndex + 1, INSP_ITEM_COL_AF_TYPE))
            ->AddString(_T("Each Point"));
        ((CGridCellCombo*)m_gridVisionFixed[nVisionID]->GetCell(nIndex + 1, INSP_ITEM_COL_AF_TYPE))
            ->AddString(_T("Plane Offset"));
        ((CGridCellCombo*)m_gridVisionFixed[nVisionID]->GetCell(nIndex + 1, INSP_ITEM_COL_AF_TYPE))->SetCurSel(nAFType);
        ((CGridCellCombo*)m_gridVisionFixed[nVisionID]->GetCell(nIndex + 1, INSP_ITEM_COL_AF_TYPE))
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        strPlaneOffsetUM.Format(_T("%d"), nPlaneOffsetUM);
        m_gridVisionFixed[nVisionID]->SetItemText(nIndex + 1, INSP_ITEM_COL_PLANE_OFFSET, strPlaneOffsetUM);
        m_gridVisionFixed[nVisionID]
            ->GetCell(nIndex + 1, INSP_ITEM_COL_PLANE_OFFSET)
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        m_gridVisionFixed[nVisionID]->SetCellType(nIndex + 1, INSP_ITEM_COL_USAGE, RUNTIME_CLASS(CGridCellCheck2));
        m_gridVisionFixed[nVisionID]->SetItemText(nIndex + 1, INSP_ITEM_COL_USAGE, _T(""));
        ((CGridCellCheck2*)m_gridVisionFixed[nVisionID]->GetCell(nIndex + 1, INSP_ITEM_COL_USAGE))->SetCheck(bSave);
        ((CGridCellCheck2*)m_gridVisionFixed[nVisionID]->GetCell(nIndex + 1, INSP_ITEM_COL_USAGE))
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    for (long nSetGridInfo = 0; nSetGridInfo < INSP_ITEM_END; nSetGridInfo++)
    {
        // 0 번째 Row 에 각각의 Title 을 정의
        m_gridVisionFixed[nSetGridInfo]->SetItemText(0, INSP_ITEM_COL_NUMBER, _T("No"));
        m_gridVisionFixed[nSetGridInfo]
            ->GetCell(0, INSP_ITEM_COL_NUMBER)
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        m_gridVisionFixed[nSetGridInfo]->SetItemText(0, INSP_ITEM_COL_NAME, _T("Name"));
        m_gridVisionFixed[nSetGridInfo]
            ->GetCell(0, INSP_ITEM_COL_NAME)
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        m_gridVisionFixed[nSetGridInfo]->SetItemText(0, INSP_ITEM_COL_AF_TYPE, _T("A/F Type"));
        m_gridVisionFixed[nSetGridInfo]
            ->GetCell(0, INSP_ITEM_COL_AF_TYPE)
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        m_gridVisionFixed[nSetGridInfo]->SetItemText(0, INSP_ITEM_COL_PLANE_OFFSET, _T("Offset(um)"));
        m_gridVisionFixed[nSetGridInfo]
            ->GetCell(0, INSP_ITEM_COL_PLANE_OFFSET)
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        m_gridVisionFixed[nSetGridInfo]->SetItemText(0, INSP_ITEM_COL_USAGE, _T("Use"));
        m_gridVisionFixed[nSetGridInfo]
            ->GetCell(0, INSP_ITEM_COL_USAGE)
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        m_gridVisionFixed[nSetGridInfo]->SetItemBkColour(0, INSP_ITEM_COL_NUMBER, RGB(200, 200, 250));
        m_gridVisionFixed[nSetGridInfo]->SetItemBkColour(0, INSP_ITEM_COL_NAME, RGB(200, 200, 250));
        m_gridVisionFixed[nSetGridInfo]->SetItemBkColour(0, INSP_ITEM_COL_AF_TYPE, RGB(200, 200, 250));
        m_gridVisionFixed[nSetGridInfo]->SetItemBkColour(0, INSP_ITEM_COL_PLANE_OFFSET, RGB(200, 200, 250));
        m_gridVisionFixed[nSetGridInfo]->SetItemBkColour(0, INSP_ITEM_COL_USAGE, RGB(200, 200, 250));

        m_gridVisionFixed[nSetGridInfo]->SetColumnWidth(INSP_ITEM_COL_NUMBER, 40);
        m_gridVisionFixed[nSetGridInfo]->SetColumnWidth(INSP_ITEM_COL_NAME, 150);
        m_gridVisionFixed[nSetGridInfo]->SetColumnWidth(INSP_ITEM_COL_AF_TYPE, 120);
        m_gridVisionFixed[nSetGridInfo]->SetColumnWidth(INSP_ITEM_COL_PLANE_OFFSET, 100);
        m_gridVisionFixed[nSetGridInfo]->SetColumnWidth(INSP_ITEM_COL_USAGE, 35);

        // Grid 정렬
        m_gridVisionFixed[nSetGridInfo]->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        m_gridVisionFixed[nSetGridInfo]->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        m_gridVisionFixed[nSetGridInfo]
            ->GetDefaultCell(FALSE, FALSE)
            ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        m_gridVisionFixed[nSetGridInfo]->Refresh();
    }
}

void CDlgVisionInspSpecNGRV::SetAFImage()
{
    CRect rtImage2DBottom;
    GetDlgItem(IDC_NGRV_AF_IMAGE_2D_BOTTOM)->GetWindowRect(rtImage2DBottom);
    ScreenToClient(rtImage2DBottom);

    CRect rtImage2DTop;
    GetDlgItem(IDC_NGRV_AF_IMAGE_2D_TOP)->GetWindowRect(rtImage2DTop);
    ScreenToClient(rtImage2DTop);

    m_ImageView_BTM_2D = new ImageViewEx(
        GetSafeHwnd(), Ipvm::FromMFC(rtImage2DBottom), 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    m_ImageView_TOP_2D
        = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(rtImage2DTop), 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    Ipvm::Image8u3 imageBTM;
    Ipvm::Image8u3 imageTOP;

    BYTE* pbyColorImageBTM
        = &m_visionMain->m_ngrvAFRefInfor_BTM.m_vecbyImage[0]; //	BTM memcpy SRC Image - 2023.04.26_JHB
    BYTE* pbyColorImageTOP
        = &m_visionMain->m_ngrvAFRefInfor_TOP.m_vecbyImage[0]; //	TOP memcpy SRC Image - 2023.04.26_JHB

    auto& ngrvAFRefInfo_BTM = m_visionMain->m_ngrvAFRefInfor_BTM;
    auto& ngrvAFRefInfo_TOP = m_visionMain->m_ngrvAFRefInfor_TOP;

    const long nBottomImageSizeX = ngrvAFRefInfo_BTM.m_nImageSizeX;
    const long nBottomImageSizeY = ngrvAFRefInfo_BTM.m_nImageSizeY;
    const long nBottomImageSize = nBottomImageSizeX * nBottomImageSizeY * 3;

    const long nTopImageSizeX = ngrvAFRefInfo_TOP.m_nImageSizeX;
    const long nTopImageSizeY = ngrvAFRefInfo_TOP.m_nImageSizeY;
    const long nTopImageSize = nTopImageSizeX * nTopImageSizeY * 3;

    imageBTM.Create(nBottomImageSizeX, nBottomImageSizeY);
    imageTOP.Create(nTopImageSizeX, nTopImageSizeY);

    std::vector<BYTE> vecbImageBTM(nBottomImageSize);
    std::vector<BYTE> vecbImageTop(nTopImageSize);
    BYTE* pbyImageBTM = &vecbImageBTM[0]; //	BTM memcpy DST Image - 2023.04.26_JHB
    BYTE* pbyImageTOP = &vecbImageTop[0]; //	TOP memcpy DST Image - 2023.04.26_JHB

    // 각 2D Vision에서 받아온 AF Image Display를 위한 Image memcpy - 2023.04.26_JHB
    memset(pbyImageBTM, 0, nBottomImageSize);
    memcpy(pbyImageBTM, pbyColorImageBTM, nBottomImageSize);

    memset(pbyImageTOP, 0, nTopImageSize);
    memcpy(pbyImageTOP, pbyColorImageTOP, nTopImageSize);
    ////////////////////////////////////////////////////////////////////////////////

    if (!ngrvAFRefInfo_BTM.m_bIsValidPlaneRefInfo || (long)ngrvAFRefInfo_BTM.m_vecbyImage.size() <= 0)
    {
        imageBTM.FillZero();
    }
    else
    {
        memcpy(imageBTM.GetMem(), pbyImageBTM, nBottomImageSize); //	Image memcpy 수정 - 2023.04.26_JHB
        //memcpy(imageBTM.GetMem(), &m_visionMain->m_ngrvAFRefInfor_BTM.m_vecbyImage[0], nBottomImageSize);
    }

    if (!ngrvAFRefInfo_TOP.m_bIsValidPlaneRefInfo || (long)ngrvAFRefInfo_TOP.m_vecbyImage.size() <= 0)
    {
        imageTOP.FillZero();
    }
    else
    {
        memcpy(imageTOP.GetMem(), pbyImageTOP, nTopImageSize); //	Image memcpy 수정 - 2023.04.26_JHB
        //memcpy(imageTOP.GetMem(), &m_visionMain->m_ngrvAFRefInfor_TOP.m_vecbyImage[0], nTopImageSize);
    }

    m_ImageView_BTM_2D->SetImage(imageBTM);
    m_ImageView_TOP_2D->SetImage(imageTOP);
}

void CDlgVisionInspSpecNGRV::OnGridEditEnd_InspItem_2D_BTM(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;

    if (row > -1)
    {
        BOOL bUse = ((CGridCellCheck2*)m_gridVisionFixed[INSP_ITEM_2D_BTM]->GetCell(row + 1, INSP_ITEM_COL_USAGE))
                        ->GetCheck();
        long nAFType = ((CGridCellCombo*)m_gridVisionFixed[INSP_ITEM_2D_BTM]->GetCell(row + 1, INSP_ITEM_COL_AF_TYPE))
                           ->GetCurSel();
        CString strPlaneOffsetUM
            = m_gridVisionFixed[INSP_ITEM_2D_BTM]->GetCell(row + 1, INSP_ITEM_COL_PLANE_OFFSET)->GetText();

        long nPlaneOffsetUM = _ttol(strPlaneOffsetUM);
        long nPlaneOffsetUM_Range = (long)max(nPlaneOffsetUM, -10000);
        nPlaneOffsetUM_Range = (long)min(nPlaneOffsetUM_Range, 10000);
        if (nPlaneOffsetUM != nPlaneOffsetUM_Range)
        {
            AfxMessageBox(_T("The allowed range is -10,000um to 10,000um."));
        }
        long nSizeVec1 = (long)m_vec2GrabItemFrameInfoNGRV_Buf.size();
        const long nVisionID = 0;
        if (nSizeVec1 > nVisionID)
        {
            long nSizeVec2 = (long)m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID].size();
            if (row >= 0 && row < nSizeVec2)
            {
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_bIsSaveImage = bUse;
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_nAFType = nAFType;
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_nPlaneOffsetUM = nPlaneOffsetUM_Range;
            }
        }
    }

    *result = 0;
}

void CDlgVisionInspSpecNGRV::OnGridEditEnd_InspItem_3D_BTM(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;

    if (row > -1)
    {
        BOOL bUse = ((CGridCellCheck2*)m_gridVisionFixed[INSP_ITEM_3D_BTM]->GetCell(row + 1, INSP_ITEM_COL_USAGE))
                        ->GetCheck();
        long nAFType = ((CGridCellCombo*)m_gridVisionFixed[INSP_ITEM_3D_BTM]->GetCell(row + 1, INSP_ITEM_COL_AF_TYPE))
                           ->GetCurSel();
        CString strPlaneOffsetUM
            = m_gridVisionFixed[INSP_ITEM_3D_BTM]->GetCell(row + 1, INSP_ITEM_COL_PLANE_OFFSET)->GetText();

        long nPlaneOffsetUM = _ttol(strPlaneOffsetUM);
        long nPlaneOffsetUM_Range = (long)max(nPlaneOffsetUM, -10000);
        nPlaneOffsetUM_Range = (long)min(nPlaneOffsetUM_Range, 10000);
        if (nPlaneOffsetUM != nPlaneOffsetUM_Range)
        {
            AfxMessageBox(_T("The allowed range is -10,000um to 10,000um."));
        }

        long nSizeVec1 = (long)m_vec2GrabItemFrameInfoNGRV_Buf.size();
        const long nVisionID = 1;
        if (nSizeVec1 > nVisionID)
        {
            long nSizeVec2 = (long)m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID].size();
            if (row >= 0 && row < nSizeVec2)
            {
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_bIsSaveImage = bUse;
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_nAFType = nAFType;
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_nPlaneOffsetUM = nPlaneOffsetUM_Range;
            }
        }
    }

    *result = 0;
}

void CDlgVisionInspSpecNGRV::OnGridEditEnd_InspItem_2D_TOP(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;

    if (row > -1)
    {
        BOOL bUse = ((CGridCellCheck2*)m_gridVisionFixed[INSP_ITEM_2D_TOP]->GetCell(row + 1, INSP_ITEM_COL_USAGE))
                        ->GetCheck();
        long nAFType = ((CGridCellCombo*)m_gridVisionFixed[INSP_ITEM_2D_TOP]->GetCell(row + 1, INSP_ITEM_COL_AF_TYPE))
                           ->GetCurSel();
        CString strPlaneOffsetUM
            = m_gridVisionFixed[INSP_ITEM_2D_TOP]->GetCell(row + 1, INSP_ITEM_COL_PLANE_OFFSET)->GetText();

        long nPlaneOffsetUM = _ttol(strPlaneOffsetUM);
        long nPlaneOffsetUM_Range = (long)max(nPlaneOffsetUM, -10000);
        nPlaneOffsetUM_Range = (long)min(nPlaneOffsetUM_Range, 10000);
        if (nPlaneOffsetUM != nPlaneOffsetUM_Range)
        {
            AfxMessageBox(_T("The allowed range is -10,000um to 10,000um."));
        }

        long nSizeVec1 = (long)m_vec2GrabItemFrameInfoNGRV_Buf.size();
        const long nVisionID = 2;
        if (nSizeVec1 > nVisionID)
        {
            long nSizeVec2 = (long)m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID].size();
            if (row >= 0 && row < nSizeVec2)
            {
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_bIsSaveImage = bUse;
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_nAFType = nAFType;
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_nPlaneOffsetUM = nPlaneOffsetUM_Range;
            }
        }
    }

    *result = 0;
}

void CDlgVisionInspSpecNGRV::OnGridEditEnd_InspItem_3D_TOP(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;

    if (row > -1)
    {
        BOOL bUse = ((CGridCellCheck2*)m_gridVisionFixed[INSP_ITEM_3D_TOP]->GetCell(row + 1, INSP_ITEM_COL_USAGE))
                        ->GetCheck();
        long nAFType = ((CGridCellCombo*)m_gridVisionFixed[INSP_ITEM_3D_TOP]->GetCell(row + 1, INSP_ITEM_COL_AF_TYPE))
                           ->GetCurSel();
        CString strPlaneOffsetUM
            = m_gridVisionFixed[INSP_ITEM_3D_TOP]->GetCell(row + 1, INSP_ITEM_COL_PLANE_OFFSET)->GetText();

        long nPlaneOffsetUM = _ttol(strPlaneOffsetUM);
        long nPlaneOffsetUM_Range = (long)max(nPlaneOffsetUM, -10000);
        nPlaneOffsetUM_Range = (long)min(nPlaneOffsetUM_Range, 10000);
        if (nPlaneOffsetUM != nPlaneOffsetUM_Range)
        {
            AfxMessageBox(_T("The allowed range is -10,000um to 10,000um."));
        }

        long nSizeVec1 = (long)m_vec2GrabItemFrameInfoNGRV_Buf.size();
        const long nVisionID = 3;
        if (nSizeVec1 > nVisionID)
        {
            long nSizeVec2 = (long)m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID].size();
            if (row >= 0 && row < nSizeVec2)
            {
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_bIsSaveImage = bUse;
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_nAFType = nAFType;
                m_vec2GrabItemFrameInfoNGRV_Buf[nVisionID][row].m_nPlaneOffsetUM = nPlaneOffsetUM_Range;
            }
        }
    }

    *result = 0;
}

long CDlgVisionInspSpecNGRV::GetSelectedGridCellRow(CGridCtrl* grid)
{
    CCellRange CellRange = grid->GetSelectedCellRange();

    if (!CellRange.IsValid())
        return -1;

    if (CellRange.GetMinRow() != CellRange.GetMaxRow())
        return -1;

    long nSelItem = CellRange.GetTopLeft().row - 1;

    return nSelItem;
}

void CDlgVisionInspSpecNGRV::OnBnClickedNgrvApply()
{
    m_visionMain->m_vecGrabItemFrameInfo.clear(); //일단 다 지우고

    long nVisionSize = (long)m_vec2GrabItemFrameInfoNGRV_Buf.size();

    //수정을 Buffer에 하니까 Buffer의 내용으로 CVisionMain의 변수를 재구성한다.
    for (long nVision = 0; nVision < nVisionSize; nVision++)
    {
        long nItemNum = (long)m_vec2GrabItemFrameInfoNGRV_Buf[nVision].size();
        for (long nItem = 0; nItem < nItemNum; nItem++)
        {
            m_visionMain->m_vecGrabItemFrameInfo.push_back(m_vec2GrabItemFrameInfoNGRV_Buf[nVision][nItem]);
        }
    }

    CDialog::OnOK();
}

void CDlgVisionInspSpecNGRV::OnBnClickedNgrvCancel()
{
    //실제 CVisionMain의 변수를 직접 수정하지 않으므로 Cancel에서는 할거 없다.

    CDialog::OnCancel();
}
