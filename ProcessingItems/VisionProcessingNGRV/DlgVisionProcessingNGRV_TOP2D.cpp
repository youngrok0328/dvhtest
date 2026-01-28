//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingNGRV_TOP2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingNGRV.h"
#include "DlgVisionProcessingNGRV_DetailSetup.h"
#include "VisionProcessingNGRV.h"
#include "VisionProcessingNGRVPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ArchiveAllType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvSingleRunInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/iPIS_MsgDefinitions.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_GRAB_END_EVENT (WM_USER + 1054)
#define NGRV_TOTAL_FRAME_NUM 8
#define NGRV_CAMERA_X_UM 19000.f
#define NGRV_CAMERA_Y_UM 15000.f

//CPP_7_________________________________ Implementation body
enum GridItemID_TOP
{
    ITEM_START = 0,
    ITEM_ADD_TOP = ITEM_START,
    ITEM_DELETE_TOP,
    ITEM_APPLY_TOP,
    ITEM_END,
};

enum ListItemID_TOP
{
    COLUMN_START = 0,
    COLUMN_DEFECT_NAME_TOP = COLUMN_START,
    COLUMN_ITEM_SETUP_TOP,
    COLUMN_MOVE_POS_TOP,
    COLUMN_END,
};

// DlgVisionProcessingNGRV_TOP2D 대화 상자

IMPLEMENT_DYNAMIC(DlgVisionProcessingNGRV_TOP2D, CDialog)

DlgVisionProcessingNGRV_TOP2D::DlgVisionProcessingNGRV_TOP2D(
    VisionProcessingNGRV* processor, VisionProcessingNGRVPara& para, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DLG_NGRV_TOP_TAB, pParent)
    , m_processor(processor)
    , m_imageView(nullptr)
    , m_para(para)
    , m_grid(new CXTPPropertyGrid)
    , m_selectedDefectID(0)
    , m_isLive(FALSE)
    , m_grabImage(new Ipvm::Image8u)
    , m_bayerImage(new Ipvm::Image8u3)
    , m_imageID(0)
{
}

DlgVisionProcessingNGRV_TOP2D::~DlgVisionProcessingNGRV_TOP2D()
{
    delete m_imageView;
    delete m_grid;
    delete m_grabImage;
    delete m_bayerImage;
}

void DlgVisionProcessingNGRV_TOP2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_NGRV_TOP2D, m_parameterList);
    DDX_Control(pDX, IDC_EDIT_FRAME_NUM_TOP, m_frameNum);
    DDX_Control(pDX, IDC_BUTTON_CHANGE_FRAME_BACK, m_frameBW);
    DDX_Control(pDX, IDC_BUTTON_CHANGE_FRAME_FOWARD, m_frameFW);
    DDX_Control(pDX, IDC_BUTTON_LIVE_TOP, m_buttonLive);
    DDX_Control(pDX, IDC_BUTTON_GRAB_TOP, m_buttonGrab);
}

BEGIN_MESSAGE_MAP(DlgVisionProcessingNGRV_TOP2D, CDialog)
ON_WM_CREATE()
ON_WM_DESTROY()
ON_MESSAGE(UM_GRAB_END_EVENT, &DlgVisionProcessingNGRV_TOP2D::OnGrabEndEvent)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_NOTIFY(NM_CLICK, IDC_LIST_NGRV_TOP2D, &DlgVisionProcessingNGRV_TOP2D::OnItemClickTop)
ON_NOTIFY(XTP_NM_GRID_ITEMBUTTONCLICK, IDC_LIST_NGRV_TOP2D, &DlgVisionProcessingNGRV_TOP2D::OnItemButtonClickTOP)
ON_NOTIFY(XTP_NM_GRID_VALUECHANGED, IDC_LIST_NGRV_TOP2D, &DlgVisionProcessingNGRV_TOP2D::OnDefectNameChangedTOP)
ON_BN_CLICKED(IDC_BUTTON_CHANGE_FRAME_BACK, &DlgVisionProcessingNGRV_TOP2D::OnBnClickedButtonChangeFrameBack)
ON_BN_CLICKED(IDC_BUTTON_CHANGE_FRAME_FOWARD, &DlgVisionProcessingNGRV_TOP2D::OnBnClickedButtonChangeFrameFoward)
ON_BN_CLICKED(IDC_BUTTON_LIVE_TOP, &DlgVisionProcessingNGRV_TOP2D::OnBnClickedButtonLiveTop)
ON_BN_CLICKED(IDC_BUTTON_GRAB_TOP, &DlgVisionProcessingNGRV_TOP2D::OnBnClickedButtonGrabTop)
END_MESSAGE_MAP()

// DlgVisionProcessingNGRV_TOP2D 메시지 처리기

BOOL DlgVisionProcessingNGRV_TOP2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect parentRect;
    GetParent()->GetClientRect(parentRect);
    parentRect.top = parentRect.top + 21;
    parentRect.left = parentRect.left + 1;
    parentRect.right = parentRect.right - 3;
    parentRect.bottom = parentRect.bottom - 2;
    MoveWindow(parentRect);

    CRect tempArea, gridArea;
    tempArea = m_processor->m_pVisionInspDlg->m_procDlgInfo.m_rtParaArea;
    tempArea.left = parentRect.left + 10;
    tempArea.right = parentRect.right - 10;
    tempArea.bottom = tempArea.bottom - 430;
    tempArea.top = tempArea.top - 20;

    gridArea.SetRect(tempArea.left, tempArea.top, tempArea.right, tempArea.bottom);
    m_grid->Create(gridArea, this, static_cast<UINT>(IDC_STATIC));
    m_grid->ShowHelp(FALSE);

    m_gridArea = gridArea;

    OnInitGrid();
    OnInitList();
    OnInitImage();

    ResetListControl();

    SetROI();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

int DlgVisionProcessingNGRV_TOP2D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  여기에 특수화된 작성 코드를 추가합니다.

    return 0;
}

void DlgVisionProcessingNGRV_TOP2D::OnDestroy()
{
    CDialog::OnDestroy();

    // TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

void DlgVisionProcessingNGRV_TOP2D::OnInitGrid()
{
    m_grid->ResetContent();

    if (auto* category = m_grid->AddCategory(_T("Item Control Parameter")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Add Item"), TRUE, FALSE))->SetID(ITEM_ADD_TOP);
        category->AddChildItem(new CCustomItemButton(_T("Delete Item"), TRUE, FALSE))->SetID(ITEM_DELETE_TOP);
        category->AddChildItem(new CCustomItemButton(_T("Apply"), TRUE, FALSE))->SetID(ITEM_APPLY_TOP);

        m_grid->FindItem(ITEM_ADD_TOP)->SetHeight(50);
        m_grid->FindItem(ITEM_DELETE_TOP)->SetHeight(50);
        m_grid->FindItem(ITEM_APPLY_TOP)->SetHeight(50);

        category->Expand();
    }

    m_grid->SetViewDivider(0.60);
    m_grid->HighlightChangedItems(TRUE);
    m_grid->Refresh();
}

void DlgVisionProcessingNGRV_TOP2D::OnInitList()
{
    CRect tempArea, listArea;
    tempArea = m_processor->m_pVisionInspDlg->m_procDlgInfo.m_rtParaArea;
    tempArea.bottom = tempArea.bottom - 400;

    listArea = m_processor->m_pVisionInspDlg->m_procDlgInfo.m_rtParaArea;

    listArea.top = m_gridArea.bottom + 10;
    listArea.bottom = listArea.bottom - 110;
    listArea.left = m_gridArea.left;
    listArea.right = m_gridArea.right;

    m_parameterList.MoveWindow(listArea);
    m_listArea = listArea;

    int length = listArea.Width() / 4;
    m_parameterList.AddColumn(new CXTPGridColumn(COLUMN_DEFECT_NAME_TOP, _T("Defect Name"), length * 2));
    m_parameterList.AddColumn(new CXTPGridColumn(COLUMN_ITEM_SETUP_TOP, _T("Item Setup"), length));
    m_parameterList.AddColumn(new CXTPGridColumn(COLUMN_MOVE_POS_TOP, _T("Move Position"), length));

    m_parameterList.Populate();
    m_parameterList.SetTheme(xtpGridThemeVisualStudio2012Light);
    m_parameterList.SetGridStyle(0, xtpGridSmallDots);
    COLORREF rgbGridColor = RGB(0, 0, 0);
    m_parameterList.SetGridColor(rgbGridColor);
    m_parameterList.FocusSubItems(TRUE);
    m_parameterList.AllowEdit(TRUE);

    //// Live/Grab Button
    CRect grabButtonArea;
    grabButtonArea.left = m_listArea.left;
    grabButtonArea.top = m_listArea.bottom + 5;
    grabButtonArea.right = m_listArea.right;
    grabButtonArea.bottom = grabButtonArea.top + 20;

    int grabButtonLength = grabButtonArea.Width() / 5;

    m_buttonLive.MoveWindow(grabButtonArea.TopLeft().x, grabButtonArea.TopLeft().y, grabButtonLength, 20);
    m_buttonGrab.MoveWindow(
        grabButtonArea.TopLeft().x + grabButtonLength, grabButtonArea.TopLeft().y, grabButtonLength, 20);

    m_buttonLive.ShowWindow(SW_SHOW);
    m_buttonGrab.ShowWindow(SW_SHOW);
    //////////////////////

    //// Frame Button
    CRect buttonArea;
    buttonArea.left = grabButtonArea.left;
    buttonArea.top = grabButtonArea.bottom + 5;
    buttonArea.right = grabButtonArea.right;
    buttonArea.bottom = buttonArea.top + 20;

    m_frameSelectionArea = buttonArea;
    int buttonLength = m_frameSelectionArea.Width() / 5;

    m_frameBW.MoveWindow(m_frameSelectionArea.TopLeft().x, m_frameSelectionArea.TopLeft().y, buttonLength * 2, 20);
    m_frameNum.MoveWindow(
        m_frameSelectionArea.TopLeft().x + (buttonLength * 2), m_frameSelectionArea.TopLeft().y, buttonLength, 20);
    m_frameFW.MoveWindow(
        m_frameSelectionArea.TopLeft().x + (buttonLength * 3), m_frameSelectionArea.TopLeft().y, buttonLength * 2, 20);

    m_frameBW.ShowWindow(SW_SHOW);
    m_frameNum.ShowWindow(SW_SHOW);
    m_frameFW.ShowWindow(SW_SHOW);
    ///////////////////////
}

void DlgVisionProcessingNGRV_TOP2D::OnInitImage()
{
    Ipvm::Rect32s imageArea;
    imageArea.m_left = m_frameSelectionArea.left;
    imageArea.m_top = m_frameSelectionArea.bottom + 5;
    imageArea.m_right = m_frameSelectionArea.right;
    imageArea.m_bottom = m_processor->m_pVisionInspDlg->m_procDlgInfo.m_rtDataArea.bottom;
    imageArea.m_bottom = imageArea.m_top + static_cast<int32_t>(imageArea.Height() * 0.85f + .5f);

    m_imageView = new ImageViewEx(GetSafeHwnd(), imageArea);
}

void DlgVisionProcessingNGRV_TOP2D::ResetListControl()
{
    m_parameterList.ResetContent(FALSE);

    long coulmnLength = m_listArea.Width() / 4;

    CString temp("");
    for (long inspID = 0; inspID < m_para.m_defectNumber; inspID++)
    {
        CXTPGridRecord* record = new CXTPGridRecord();

        // Inspection Name
        temp.Format(_T("%s"), (LPCTSTR)m_para.m_vecDefectName[inspID]);
        record->AddItem(new CXTPGridRecordItemText(temp));

        // Inspection Setup
        temp.Format(_T(""));
        record->AddItem(new CXTPGridRecordItemText(temp));

        // Inspection Position
        temp.Format(_T(""));
        record->AddItem(new CXTPGridRecordItemText(temp));

        // Add Record to List Control
        m_parameterList.AddRecord(record);

        //Add Setup Button
        CXTPGridRecordItem* itemSetup = record->GetItem(1);
        itemSetup->SetEditable(FALSE);
        auto* button = itemSetup->GetItemControls()->AddControl(xtpItemControlTypeButton);
        if (button->GetState() == FALSE)
        {
            continue;
        }
        button->SetIconIndex(PBS_NORMAL, 0);
        button->SetIconIndex(PBS_PRESSED, 1);
        button->SetCaption(_T("Setup"));
        button->SetSize(CSize(coulmnLength, 0));

        // Add Move Button
        CXTPGridRecordItem* itemMovePos = record->GetItem(2);
        itemMovePos->SetEditable(FALSE);
        button = itemMovePos->GetItemControls()->AddControl(xtpItemControlTypeButton);
        if (button->GetState() == FALSE)
        {
            continue;
        }
        button->SetIconIndex(PBS_NORMAL, 1);
        button->SetIconIndex(PBS_PRESSED, 2);
        button->SetCaption(_T("Move to"));
        button->SetSize(CSize(coulmnLength, 0));
    }
    m_parameterList.Populate();

    temp.Empty();
}

void DlgVisionProcessingNGRV_TOP2D::AddItem()
{
    long oldNum = m_para.m_defectNumber;
    //Add Item 상황에서 ROI가 Image Size보다 크면 ROI는 1개만 있어야 한다.
    const bool isRoiOverImageSize = IsRoiSizeOverThanImageSize();
    if (isRoiOverImageSize && oldNum >= 1)
    {
        return;
    }

    long newNum = oldNum + 1;

    m_para.m_vecDefectName.resize(newNum);
    m_para.m_vecStitchCount.resize(newNum);
    m_para.m_vecAFtype.resize(newNum);
    m_para.m_vecGrabHeightOffset_um.resize(newNum);
    m_para.m_vecGrabPosROI.resize(newNum);
    m_para.m_vec2IllumUsage.resize(newNum);

    CString temp("");
    for (long index = oldNum; index < newNum; index++)
    {
        temp.Format(_T("Untiltle Name_%d"), index);
        m_para.m_vecDefectName[index] = temp;
        m_para.m_vecItemID[index] = _T("000");
        m_para.m_vecDefectCode[index] = _T(""); // Default empty loss code
        m_para.m_vecDisposition[index] = DISPOSITION_NULL; // Default empty disposition
        m_para.m_vecDecision[index] = NOT_MEASURED; // Default to Not Measured
        m_para.m_vecStitchCount[index] = Ipvm::Point32s2(1, 1);
        m_para.m_vecAFtype[index] = 0;
        m_para.m_vecGrabHeightOffset_um[index] = 0.0f;
        m_para.m_vecGrabPosROI[index] = Ipvm::Rect32s(0, 0, 0, 0);
        m_para.m_vec2IllumUsage[index].resize(LED_ILLUM_FRAME_MAX);
    }

    m_para.m_defectNumber = newNum;

    ResetListControl();

    SetROI();

    long selectDefectID = m_para.m_defectNumber - 1;

    if (selectDefectID < 0)
    {
        selectDefectID = 0;
    }

    temp.Empty();
}

void DlgVisionProcessingNGRV_TOP2D::DeleteItem()
{
    if (m_para.m_defectNumber <= 1)
    {
        return;
    }

    long selectDefectID = m_selectedDefectID;

    CString selectedDefectName = m_para.m_vecDefectName[selectDefectID];

    CString temp("");
    temp.Format(_T("Press OK button to delete %s item."), (LPCTSTR)selectedDefectName);
    if (::SimpleMessage(temp, MB_OKCANCEL) == IDCANCEL)
    {
        temp.Empty();
        return;
    }

    long oldNum = m_para.m_defectNumber;
    long newNum = oldNum - 1;

    m_para.m_vecDefectName.erase(m_para.m_vecDefectName.begin() + selectDefectID);
    m_para.m_vecItemID.erase(m_para.m_vecItemID.begin() + selectDefectID);
    m_para.m_vecDefectCode.erase(m_para.m_vecDefectCode.begin() + selectDefectID);
    m_para.m_vecDisposition.erase(m_para.m_vecDisposition.begin() + selectDefectID);
    m_para.m_vecDecision.erase(m_para.m_vecDecision.begin() + selectDefectID);
    m_para.m_vecStitchCount.erase(m_para.m_vecStitchCount.begin() + selectDefectID);
    m_para.m_vecAFtype.erase(m_para.m_vecAFtype.begin() + selectDefectID);
    //m_para.m_vec2IllumUsage[selectDefectID].erase(m_para.m_vec2IllumUsage[selectDefectID].begin() + (long)m_para.m_vec2IllumUsage[selectDefectID].size());
    m_para.m_vec2IllumUsage.erase(m_para.m_vec2IllumUsage.begin() + selectDefectID);
    m_para.m_vecGrabHeightOffset_um.erase(m_para.m_vecGrabHeightOffset_um.begin() + selectDefectID);

    m_para.m_defectNumber = newNum;

    //m_visionInsp->DeleteSpec

    if (selectDefectID > m_para.m_defectNumber)
    {
        selectDefectID = m_para.m_defectNumber - 1;
    }

    if (selectDefectID < 0)
    {
        selectDefectID = 0;
    }

    ResetListControl();

    SetROI();

    temp.Empty();
}

void DlgVisionProcessingNGRV_TOP2D::ApplyItem()
{
    const auto& scale = m_processor->getScale();

    // 현재까지 편집된 item들을 저장
    // 1. NgrvSingleRunInfo 생성
    std::vector<NgrvOnePointGrabInfo> onePointGrabInfo;
    onePointGrabInfo.clear();
    onePointGrabInfo.resize(m_para.m_defectNumber);

    std::vector<Ipvm::Point32s2> eachCenterPoints;

    // 2. 현재 Setup된 item 정보 복사
    for (long index = 0; index < m_para.m_defectNumber; index++)
    {
        eachCenterPoints.clear();

        // a. Single Run Info 초기화
        onePointGrabInfo[index].Init();

        // b. Defect Name, Image center position, Z-offset
        onePointGrabInfo[index].m_defectPositionName = m_para.m_vecDefectName[index];
        onePointGrabInfo[index].m_itemID = m_para.m_vecItemID[index];
        onePointGrabInfo[index].m_defectCode = m_para.m_vecDefectCode[index];
        onePointGrabInfo[index].m_disposition
            = GetDispositionText(static_cast<enDispositionType>(m_para.m_vecDisposition[index]), true);
        onePointGrabInfo[index].m_decision
            = GetResultText(static_cast<II_RESULT_VALUE>(m_para.m_vecDecision[index]), true, true);

        // Stitching X/Y 둘 중 하나라도 1 이상이면 각 ROI의 center와 전체 ROI의 center 좌표를 보내준다.
        if (m_para.m_vecStitchCount[index].m_x > 1 || m_para.m_vecStitchCount[index].m_y > 1)
        {
            long centerX(0), centerY(0);
            long stitchX = m_para.m_vecStitchCount[index].m_x;
            long stitchY = m_para.m_vecStitchCount[index].m_y;

            for (long nY = 0; nY < stitchY; nY++)
            {
                centerY = m_para.m_vecGrabPosROI[index].m_top
                    + (m_para.m_vecGrabPosROI[index].Height() / (stitchY * 2) * (nY * 2 + 1));

                for (long nX = 0; nX < stitchX; nX++)
                {
                    //long roiIndex = stitchX * nY + nX;
                    centerX = m_para.m_vecGrabPosROI[index].m_left
                        + (m_para.m_vecGrabPosROI[index].Width() / (stitchX * 2) * (nX * 2 + 1));
                    eachCenterPoints.push_back(Ipvm::Point32s2(centerX, centerY));
                }
            }
            //eachCenterPoints.push_back(m_para.m_vecGrabPosROI[index].CenterPoint());
        }
        else // Stitching 안할 경우, 해당 ROI의 center 좌표를 보내준다.
        {
            eachCenterPoints.push_back(Ipvm::Point32s2(
                m_para.m_vecGrabPosROI[index].CenterPoint().m_x, m_para.m_vecGrabPosROI[index].CenterPoint().m_y));
        }

        for (long nSize = 0; nSize < eachCenterPoints.size(); nSize++)
        {
            // Real Image Center Position in pixel
            Ipvm::Point32r2 imageCenterPoint(static_cast<float>(m_processor->getImageLot().GetImageSizeX() * .5f),
                static_cast<float>(m_processor->getImageLot().GetImageSizeY() * .5f));
            // Push back with real value with convert pixel to BCU function
            onePointGrabInfo[index].m_vecPointGrabPosition_um.push_back(
                scale.convert_PixelToBCU(eachCenterPoints[nSize], imageCenterPoint));

            // CenterY 좌표 기준이기 때문에, Y좌표의 최종 결과에 (-)부호를 곱해줘서 극 좌표계로 변경시킨 후 전달 필요.
            onePointGrabInfo[index].m_vecPointGrabPosition_um[nSize].m_y
                = onePointGrabInfo[index].m_vecPointGrabPosition_um[nSize].m_y * (-1);
        }

        onePointGrabInfo[index].m_grabHeightOffset_um = m_para.m_vecGrabHeightOffset_um[index];
        onePointGrabInfo[index].m_afType = m_para.m_vecAFtype[index];
        onePointGrabInfo[index].m_stitchX = m_para.m_vecStitchCount[index].m_x;
        onePointGrabInfo[index].m_stitchY = m_para.m_vecStitchCount[index].m_y;

        long illumSize = (long)m_para.m_vec2IllumUsage[index].size();
        for (long illumIndex = 0; illumIndex < illumSize; illumIndex++)
        {
            onePointGrabInfo[index].m_vecIllumUse.push_back(m_para.m_vec2IllumUsage[index][illumIndex]);
        }
    }

    // 3. parameter에 저장
    NgrvSingleRunInfo singleRunInfo;
    singleRunInfo.Init();

    singleRunInfo.m_vecOnePointGrabInfo.resize((long)onePointGrabInfo.size());

    for (long infoIndex = 0; infoIndex < (long)onePointGrabInfo.size(); infoIndex++)
    {
        singleRunInfo.m_vecOnePointGrabInfo[infoIndex] = onePointGrabInfo[infoIndex];
    }

    // 4. visionUnit을 거쳐 visionmain에 NGRV parameter에 저장
    m_processor->m_visionUnit.SetNGRVSingleRunInfo(VISION_TOP_2D, singleRunInfo);
}

LRESULT DlgVisionProcessingNGRV_TOP2D::OnGridNotify(WPARAM wParam, LPARAM lParam)
{
    if (wParam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lParam;

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (value->GetID())
        {
            case ITEM_ADD_TOP:
                AddItem();
                break;
            case ITEM_DELETE_TOP:
                DeleteItem();
                break;
            case ITEM_APPLY_TOP:
                ApplyItem();
                break;
            default:
                break;
        }

        value->SetBool(FALSE);
    }

    return 0;
}

void DlgVisionProcessingNGRV_TOP2D::OnItemButtonClickTOP(NMHDR* pNotifyStruct, LRESULT*)
{
    XTP_NM_GRIDITEMCONTROL* pItemNotify = (XTP_NM_GRIDITEMCONTROL*)pNotifyStruct;
    if (!(pItemNotify->pRow && pItemNotify->pItem && pItemNotify->pItemControl))
        return;

    if (pItemNotify->pItemControl->GetCaption() == _T("Setup"))
    {
        long selectedDefectID = pItemNotify->pRow->GetRecord()->GetIndex();
        m_selectedDefectID = selectedDefectID;

        DlgVisionProcessingNGRV_DetailSetup dlg(m_processor, selectedDefectID, FALSE,
            m_processor->getImageLotInsp().m_vecColorImages, m_processor->getImageLot().GetImageSizeX(),
            m_processor->getImageLot().GetImageSizeY());
        if (dlg.DoModal() == IDOK)
        {
        }
    }

    if (pItemNotify->pItemControl->GetCaption() == _T("Move to"))
    {
        long selectedDefectID = pItemNotify->pRow->GetRecord()->GetIndex();
        m_selectedDefectID = selectedDefectID;

        iPIS_Send_SingleRun_MoveTo();
    }
}

void DlgVisionProcessingNGRV_TOP2D::OnDefectNameChangedTOP(NMHDR* pNotifyStruct, LRESULT*)
{
    XTP_NM_GRIDRECORDITEM* itemNotify = (XTP_NM_GRIDRECORDITEM*)pNotifyStruct;
    ASSERT(itemNotify != NULL);

    long selectDefectID = itemNotify->pRow->GetRecord()->GetIndex();

    CString defectNameTemp("");
    CString defectNameTemp0("");
    CString defectNameTemp1("");
    BOOL sameName = FALSE;

    srand((unsigned int)time(NULL));
    long randomID = rand() % 100;

    defectNameTemp = itemNotify->pItem->GetCaption();

    defectNameTemp0 = defectNameTemp;

    defectNameTemp0.MakeUpper();

    long count = 0;
    while (1)
    {
        if (count > 100)
        {
            break;
        }

        ++count;

        if (defectNameTemp0.Right(1) == 32) // 아스키 코드 '띄어쓰기'
        {
            defectNameTemp0 = defectNameTemp0.Left(defectNameTemp0.GetLength() - 1);
            continue;
        }
        if (defectNameTemp0.Right(1) == 32) // 아스키 코드 '띄어쓰기'
        {
            defectNameTemp0 = defectNameTemp0.Right(defectNameTemp0.GetLength() - 1);
            continue;
        }
        break;
    }

    for (long idx = 0; idx < (long)m_para.m_vecDefectName.size(); idx++)
    {
        if (selectDefectID == idx)
        {
            continue;
        }

        defectNameTemp1 = m_para.m_vecDefectName[idx];
        defectNameTemp1.MakeUpper();

        if (defectNameTemp0 == defectNameTemp1)
        {
            sameName = TRUE;
            break;
        }
    }

    if (sameName == TRUE)
    {
        ::SimpleMessage(_T("It already has the same name.\nPlease check again."), MB_OK);
        m_para.m_vecDefectName[selectDefectID].Format(_T("%s_%d"), (LPCTSTR)defectNameTemp, randomID);
    }
    else
    {
        m_para.m_vecDefectName[selectDefectID] = defectNameTemp;
    }

    ChangeROItitle();

    defectNameTemp.Empty();
    defectNameTemp0.Empty();
    defectNameTemp1.Empty();
}

bool DlgVisionProcessingNGRV_TOP2D::IsRoiSizeOverThanImageSize()
{
    const auto& scale = m_processor->getScale();

    const long imageSizeX = m_processor->getImageLot().GetImageSizeX();
    const long imageSizeY = m_processor->getImageLot().GetImageSizeY();

    const Ipvm::Rect32r cameraFOV(0.f, 0.f, NGRV_CAMERA_X_UM, NGRV_CAMERA_Y_UM);
    const Ipvm::Rect32s roiSize = scale.convert_umToPixel(cameraFOV);
    const long roiWidth = roiSize.Width();
    const long roiHeight = roiSize.Height();

    return static_cast<bool>((roiWidth > imageSizeX) && (roiHeight > imageSizeY));
}

bool DlgVisionProcessingNGRV_TOP2D::SetROI()
{
    // Main Dialog에서 ImageLotView, Scale을 가져온다.
    auto& imageLotView = *m_processor->m_pVisionInspDlg->m_imageLotView;
    const auto& scale = m_processor->getScale();

    // Main UI에 ROI를 지운다.
    imageLotView.ROI_RemoveAll();

    const long imageSizeX = m_processor->getImageLot().GetImageSizeX();
    const long imageSizeY = m_processor->getImageLot().GetImageSizeY();
    const long imageCenterX = imageSizeX / 2;
    const long imageCenterY = imageSizeY / 2;

    // NGRV Camera FOV : 19mm x 15mm
    Ipvm::Rect32r cameraFOV(0, 0, NGRV_CAMERA_X_UM, NGRV_CAMERA_Y_UM);

    // NGRV Camera FOV를 Pixel로 변환한다 : 불러오는 건 2D Scale이지만 실제 ROI는 NGRV의 Scale에 맞춰야함
    Ipvm::Rect32s roiSize = scale.convert_umToPixel(cameraFOV);

    // Pixel로 변환된 ROI의 크기를 Image Center 기준으로 좌표를 변경한다.
    const long roiWidth = roiSize.Width();
    const long roiHeight = roiSize.Height();
    const long leftBasedCenterX = imageCenterX - static_cast<long>(roiWidth * 0.5f);
    const long topBasedCenterY = imageCenterY - static_cast<long>(roiHeight * 0.5f);
    roiSize.SetRect(leftBasedCenterX, topBasedCenterY, leftBasedCenterX + roiWidth, topBasedCenterY + roiHeight);

    // 만약 ROI크기가 Image크기보다 크면 ROI를 움직이지 못하게 한다.
    const bool isRoiSizeOverThanImageSize = (roiWidth > imageSizeX) && (roiHeight > imageSizeY);
    const BOOL isMovableROI = isRoiSizeOverThanImageSize == false ? TRUE : FALSE;

    // 기존에 저장되어있는 Rect크기와 다르면 새로 설정한다.
    if (m_para.m_vecGrabPosROI.size() != m_para.m_defectNumber)
    {
        m_para.m_vecGrabPosROI.resize(m_para.m_defectNumber);
    }

    // 만약 GrabROI Rect의 크기가 0이면 다 기본 크기로 설정해줘야한다
    for (long index = 0; index < m_para.m_defectNumber; index++)
    {
        if (m_para.m_vecGrabPosROI[index].IsRectEmpty() == TRUE)
        {
            m_para.m_vecGrabPosROI[index].SetRect(roiSize.TopLeft(), roiSize.BottomRight());
        }
    }

    // ROI를 설정한다.
    if (m_para.m_defectNumber < 0)
    {
        return false;
    }
    else
    {
        for (long defectID = 0; defectID < m_para.m_defectNumber; defectID++)
        {
            CString key("");
            key.Format(_T("Defect_%d"), defectID);

            CString name("");
            name.Format(_T("%s"), (LPCTSTR)m_para.m_vecDefectName[defectID]);
            // ROI를 설정한다.
            if (m_para.m_vecGrabPosROI[defectID].CenterPoint() != roiSize.CenterPoint() && isRoiSizeOverThanImageSize == false)
            {
                m_processor->VerifyROIusingimageSize(
                    imageSizeX, imageSizeY, m_para.m_vecGrabPosROI[defectID], m_para.m_vecGrabPosROI[defectID]);
                imageLotView.ROI_Add(key, name, m_para.m_vecGrabPosROI[defectID], RGB(255, 0, 0), TRUE, FALSE);
            }
            else
            {
                if (isRoiSizeOverThanImageSize == false)
                {
                    m_processor->VerifyROIusingimageSize(imageSizeX, imageSizeY, roiSize, roiSize);
                }

                imageLotView.ROI_Add(key, name, roiSize, RGB(255, 0, 0), isMovableROI, FALSE);
            }

            key.Empty();
            name.Empty();
        }
    }

    imageLotView.ROI_Show(TRUE);

    return true;
}

void DlgVisionProcessingNGRV_TOP2D::ChangeROItitle()
{
    auto& imageLotView = *m_processor->m_pVisionInspDlg->m_imageLotView;

    // 이름이 바뀌면 지우고 다시 그려야되는 것 아닐까...?
    GetROI();

    imageLotView.ROI_RemoveAll();

    for (long defectID = 0; defectID < m_para.m_defectNumber; defectID++)
    {
        CString key("");
        key.Format(_T("Defect_%d"), defectID);

        CString name("");
        name.Format(_T("%s"), (LPCTSTR)m_para.m_vecDefectName[defectID]);
        // ROI를 설정한다.
        imageLotView.ROI_Add(key, name, m_para.m_vecGrabPosROI[defectID], RGB(255, 0, 0), TRUE, FALSE);

        key.Empty();
        name.Empty();
    }
}

void DlgVisionProcessingNGRV_TOP2D::SetDefectROIinfo()
{
    if (m_para.m_defectNumber < 0)
    {
        return;
    }

    auto& imageLotView = *m_processor->m_pVisionInspDlg->m_imageLotView;

    GetROI();

    SetROI();

    imageLotView.ROI_Show(TRUE);
}

bool DlgVisionProcessingNGRV_TOP2D::GetROI()
{
    if (m_para.m_defectNumber < 0)
    {
        return FALSE;
    }

    auto& imageLotView = *m_processor->m_pVisionInspDlg->m_imageLotView;

    for (long index = 0; index < m_para.m_defectNumber; index++)
    {
        CString temp("");
        temp.Format(_T("Defect_%d"), index);
        imageLotView.ROI_Get(temp, m_para.m_vecGrabPosROI[index]);

        temp.Empty();
    }

    return true;
}

/// Live 관련 함수
bool DlgVisionProcessingNGRV_TOP2D::StartLive(long frameNum)
{
    auto& illum = m_processor->m_visionUnit.getIllumInfo2D();
    auto& imageLot = m_processor->m_visionUnit.getImageLot();

    frameNum = min(frameNum, illum.getTotalFrameCount());

    if (SystemConfig::GetInstance().IsHardwareExist() == TRUE)
    {
        SyncController::GetInstance().SetIllumiParameter(frameNum, illum.getIllum(frameNum), true, false, FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        SyncController::GetInstance().TurnOnLight(frameNum, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        FrameGrabber::GetInstance().live_on(
            GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    }
    else
    {
        long frame = (long)imageLot.GetImageFrameCount();

        if (frameNum >= frame)
        {
            frameNum = frame - 1;
        }

        m_imageView->SetImage(imageLot.GetColorImageFrame(frameNum));
    }

    m_isLive = TRUE;

    return TRUE;
}

bool DlgVisionProcessingNGRV_TOP2D::StartGrab(long defectNum)
{
    UNREFERENCED_PARAMETER(defectNum);

    auto& visionunit = m_processor->m_visionUnit;
    auto& visionReusableMemory = visionunit.getReusableMemory();
    auto& imageLot = visionunit.getImageLot();

    //{{ 메뉴얼 Grab시 총 Frame 개수와 Buffer 할당 개수를 맞춰줘야 함 - JHB_NGRV
    const static int totalFrameCount = 0;
    const static int normalWhiteFrameCount = 8;

    std::vector<long> normalWhiteFrameID(8);
    for (long i = 0; i < normalWhiteFrameCount; i++)
        normalWhiteFrameID[i] = i;

    std::vector<long> uvFrameID(1);
    uvFrameID[0] = NGRV_FrameID_UV;
    std::vector<long> irFrameID(1);
    irFrameID[0] = NGRV_FrameID_IR;

    SetupToGrab(normalWhiteFrameID, FALSE, TRUE); // NGRV H/W Setup
    //}}

    //{{ Buffer Image의 할당 개수를 Grab하려는 이미지 수와 맞춰줌 - JHB_NGRV
    long nImageIndexSize = 10; // m_illum2D.getTotalFrameCount();

    std::vector<BYTE*> bufferMem;

    for (long imageIndex = 0; imageIndex < nImageIndexSize; imageIndex++)
    {
        bufferMem.push_back(visionReusableMemory.GetByteMemory());
    }
    //}}

    //long scanAreaIndex = 0;
    long stitchGrabIndex = 0;

    //Normal White Frame Grab
    const int& idx_graaber = 0;
    const int& idx_camera = 0;
    FrameGrabber::GetInstance().StartGrab2D(idx_graaber, idx_camera, &bufferMem[0], normalWhiteFrameCount, nullptr);
    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    FrameGrabber::GetInstance().wait_grab_end(idx_graaber, idx_camera, nullptr);

    //UV Frame Grab
    FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fUV_Camera_Gain, FALSE);
    SetupToGrab(uvFrameID, FALSE, TRUE); // NGRV H/W Setup

    FrameGrabber::GetInstance().StartGrab2D(idx_graaber, idx_camera, &bufferMem[NGRV_FrameID_UV], 1, nullptr);
    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    FrameGrabber::GetInstance().wait_grab_end(idx_graaber, idx_camera, nullptr);

    //IR Frame Grab
    FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fIR_Camera_Gain, FALSE);
    SetupToGrab(irFrameID, FALSE, TRUE); // NGRV H/W Setup

    FrameGrabber::GetInstance().StartGrab2D(idx_graaber, idx_camera, &bufferMem[NGRV_FrameID_IR], 1, nullptr);
    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    FrameGrabber::GetInstance().wait_grab_end(idx_graaber, idx_camera, nullptr);

    FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fDefault_Camera_Gain, FALSE);

    SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    imageLot.PasteNGRV(stitchGrabIndex, &bufferMem[0], (long)bufferMem.size(), 1, 1,
        NGRV_FrameID_Reverse); // Grab 도중에 Color conversion을 하면 시간이 오래 걸리니, Grab을 다 한 후에 conversion 진행
    m_processor->getImageLotInsp().Set(imageLot, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    // 버퍼 해제
    for (auto* mem : bufferMem)
    {
        visionReusableMemory.Release_ByteMemory(mem);
    }

    return true;
}

LRESULT DlgVisionProcessingNGRV_TOP2D::OnGrabEndEvent(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    FrameGrabber::GetInstance().get_live_image(*m_grabImage);
    FrameGrabber::GetInstance().get_live_image(*m_bayerImage);

    m_imageView->SetImage(*m_bayerImage);

    return 1L;
}

void DlgVisionProcessingNGRV_TOP2D::ShowPreviousImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_processor->getImageLotInsp().m_vecColorImages.size() == 0)
        return;

    if (m_imageID <= 0)
        return;

    m_imageID--;

    ShowImage(TRUE);
}

void DlgVisionProcessingNGRV_TOP2D::ShowNextImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_processor->getImageLotInsp().m_vecColorImages.size() == 0)
        return;

    if (m_imageID >= (long)(m_processor->getImageLotInsp().m_vecColorImages.size()) - 1)
        return;

    m_imageID++;

    ShowImage(TRUE);
}

void DlgVisionProcessingNGRV_TOP2D::ShowImage(BOOL change)
{
    if (m_processor->GetImageFrameCount() == 0)
    {
        return;
    }

    if (!change)
    {
        m_imageID = max(0, m_processor->GetImageFrameIndex(0));
    }

    if ((long)m_processor->getImageLotInsp().m_vecColorImages.size() <= 0)
        return;

    if ((long)m_processor->getImageLotInsp().m_vecColorImages.size() <= m_imageID)
        m_imageID = (long)m_processor->getImageLotInsp().m_vecColorImages.size() - 1;

    m_imageView->SetImage(m_processor->getImageLotInsp().m_vecColorImages[m_imageID]);
}

void DlgVisionProcessingNGRV_TOP2D::OnBnClickedButtonChangeFrameBack()
{
    ShowPreviousImage();

    CString temp("");
    temp.Format(_T("frame_%d"), m_imageID);
    SetDlgItemText(IDC_EDIT_FRAME_NUM_TOP, temp);

    temp.Empty();
}

void DlgVisionProcessingNGRV_TOP2D::OnBnClickedButtonChangeFrameFoward()
{
    ShowNextImage();

    CString temp("");
    temp.Format(_T("frame_%d"), m_imageID);
    SetDlgItemText(IDC_EDIT_FRAME_NUM_TOP, temp);

    temp.Empty();
}

void DlgVisionProcessingNGRV_TOP2D::OnBnClickedButtonLiveTop()
{
    if (m_isLive == FALSE)
    {
        StartLive(m_imageID);
    }
    else
    {
        FrameGrabber::GetInstance().live_off();
        SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

        m_isLive = FALSE;
    }
}

void DlgVisionProcessingNGRV_TOP2D::OnBnClickedButtonGrabTop()
{
    if (m_isLive == TRUE)
    {
        FrameGrabber::GetInstance().live_off();
        SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

        StartGrab(m_selectedDefectID);
    }
    else
    {
        StartGrab(m_selectedDefectID);

        SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    }

    ShowImage(FALSE);
}

void DlgVisionProcessingNGRV_TOP2D::OnItemClickTop(NMHDR* pNotifyStruct, LRESULT* result)
{
    UNREFERENCED_PARAMETER(result);

    XTP_NM_GRIDRECORDITEM* pItemNotify = (XTP_NM_GRIDRECORDITEM*)pNotifyStruct;

    if (!pItemNotify->pRow || !pItemNotify->pColumn)
        return;

    long nID = pItemNotify->pRow->GetRecord()->GetIndex();
    m_selectedDefectID = nID;
}

BOOL DlgVisionProcessingNGRV_TOP2D::SetupToGrab(std::vector<long> frameID, BOOL isUseIR, BOOL isInline)
{
    SyncController::GetInstance().SetTriggerEdgeMode(TRUE, TRUE, TRUE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    auto& visionUnit = m_processor->m_visionUnit;
    auto& illum2D = visionUnit.getIllumInfo2D();
    auto& system = SystemConfig::GetInstance();

    long totalFrameCount = (isInline ? (long)frameID.size() : illum2D.getTotalFrameCount());
    long highPosGrabCount = (isUseIR ? 1 : 0); //kircheis_MultiGrab //추후 Job에 있는 Hig Pos Grab Count를 쓰자
    SyncController::GetInstance().SetFrameCount(
        totalFrameCount - highPosGrabCount, highPosGrabCount, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    //{{//kircheis_CamTrans
    float frameExposureSum = 0.f;
    float frameExposureMax = 0.f;
    //}}

    // Set 2D Illumination
    long realTotalFrameNum = illum2D.getTotalFrameCount();

    for (int i = 0; i < totalFrameCount; i++)
    {
        auto illum = isInline ? illum2D.getIllum(frameID[i]) : illum2D.getIllum(i);

        if (isInline)
        {
            if (frameID[i] > realTotalFrameNum)
            {
                for (long channel = 0; channel < 12; channel++)
                {
                    illum[channel] = 0.f;
                }
            }
        }
        else
        {
            if (i > realTotalFrameNum)
            {
                for (long channel = 0; channel < 12; channel++)
                {
                    illum[channel] = 0.f;
                }
            }
        }
        SyncController::GetInstance().SetIllumiParameter(
            i, illum, true, false, FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

        //{{//kircheis_CamTrans
        frameExposureSum = 0.f;
        for (long channel = 0; channel < 12; channel++)
            frameExposureSum += illum[channel];

        frameExposureMax = (float)max(frameExposureMax, frameExposureSum);
        //}}
    }

    //{{//kircheis_CamTrans//두영아
    static const int duration = system.GetGrabDuration();
    int maxExposure = (int)(frameExposureMax + .5f) + system.m_nCameraTransferTimeMS;
    system.m_nLastDownloadDuration = maxExposure;
    SyncController::GetInstance().SetFramePeriod(
        CAST_FLOAT(max(duration, maxExposure)), enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    //}}

    return TRUE;
}

void DlgVisionProcessingNGRV_TOP2D::iPIS_Send_SingleRun_MoveTo()
{
    if (m_processor->m_visionUnit.GetSocketInfo().IsConnected() == false)
        return;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    long selectedItemID = m_selectedDefectID;
    auto& grabInfo = m_processor->m_visionUnit.GetNGRVSingleRunInfo(VISION_TOP_2D);
    auto& onePointGrabInfo = grabInfo.m_vecOnePointGrabInfo[selectedItemID];

    CMemFile memFile;
    ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);

    try
    {
        ar << VISION_TOP_2D;
        ar << onePointGrabInfo;
    }
    catch (CArchiveException* ae)
    {
        ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
        ae->Delete();
    }
    catch (CFileException* fe)
    {
        ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
        fe->Delete();
    }

    ar.Close();

    long length = CAST_LONG(((CFile*)&memFile)->GetLength());
    BYTE* pByte = memFile.Detach();

    m_processor->m_visionUnit.GetSocketInfo().Write(MSG_PROBE_NGRV_SEND_MOVE_SELECTED_POSITION, length, pByte);
    DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send NGRV SingleRun MoveTo Position"));

    ::free(pByte);
}