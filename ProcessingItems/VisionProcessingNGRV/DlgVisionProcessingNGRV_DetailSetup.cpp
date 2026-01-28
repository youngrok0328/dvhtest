//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingNGRV_DetailSetup.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingNGRV.h"
#include "DlgVisionProcessingNGRV_BTM2D.h"
#include "DlgVisionProcessingNGRV_TOP2D.h"
#include "VisionProcessingNGRV.h"
#include "VisionProcessingNGRVPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/SimpleFunction.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NGRV_CAMERA_X_UM 19000.f
#define NGRV_CAMERA_Y_UM 15000.f

//CPP_7_________________________________ Implementation body
enum GridParameterItemID
{
    ITEM_ID_START = 0,
    ITEM_ID_ITEM_ID = ITEM_ID_START,
    ITEM_ID_LOSS_CODE,
    ITEM_ID_DISPOSITION,
    ITEM_ID_DECISION,
    ITEM_ID_STITCH_COUNT_X,
    ITEM_ID_STITCH_COUNT_Y,
    ITEM_ID_AUTOFOCUS_TYPE,
    ITEM_ID_GRAB_HEIGHT_OFFSET,
    ITEM_ID_GRAB_1ST_FRAME,
    ITEM_ID_GRAB_2ND_FRAME,
    ITEM_ID_GRAB_3RD_FRAME,
    ITEM_ID_GRAB_4TH_FRAME,
    ITEM_ID_GRAB_5TH_FRAME,
    ITEM_ID_GRAB_6TH_FRAME,
    ITEM_ID_GRAB_7TH_FRAME,
    ITEM_ID_GRAB_RV_FRAME,
    ITEM_ID_GRAB_UV_FRAME,
    ITEM_ID_GRAB_IR_FRAME,
    ITEM_ID_END,
};

enum GridButtonItemID
{
    ITEM_BUTTON_START = 0,
    ITEM_BUTTON_APPLY = ITEM_BUTTON_START,
    ITEM_BUTTON_CANCEL,
    ITEM_BUTTON_END,
};

enum enum2DVisionType
{
    VISION_2D_BTM = 0,
    VISION_2D_TOP,
};

#define MAX_STITCH_COUNT 3

// DlgVisionProcessingNGRV_DetailSetup 대화 상자

IMPLEMENT_DYNAMIC(DlgVisionProcessingNGRV_DetailSetup, CDialog)

DlgVisionProcessingNGRV_DetailSetup::DlgVisionProcessingNGRV_DetailSetup(VisionProcessingNGRV* visionInsp,
    long defectID, BOOL isBTM, std::vector<Ipvm::Image8u3> grabbedImage, const long& ref_Input_ImageSizeX,
    const long& ref_Input_ImageSizeY, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DLG_NGRV_DETAIL_SETUP, pParent)
    , m_imageView(NULL)
    , m_visionInsp(visionInsp)
    , m_defectID(defectID)
    , m_grid(new CXTPPropertyGrid)
    , m_ref_ImageSizeX(ref_Input_ImageSizeX)
    , m_ref_ImageSizeY(ref_Input_ImageSizeY)
{
    auto& visionProcessingDlg = m_visionInsp->m_pVisionInspDlg;
    auto& visionUnit = visionProcessingDlg->m_visionInsp->m_visionUnit;

    cur_imageInfo.Init();
    if (isBTM == TRUE)
    {
        m_visionPara = m_visionInsp->m_VisionParaBTM;
        cur_imageInfo = visionUnit.Get2DImageInfo_NGRV(VISION_MODULE_ID_2D_BTM);
    }
    else
    {
        m_visionPara = m_visionInsp->m_VisionParaTOP;
        cur_imageInfo = visionUnit.Get2DImageInfo_NGRV(VISION_MODULE_ID_2D_TOP);
    }

    m_grabbedImages.resize((long)grabbedImage.size());
    for (long index = 0; index < (long)m_grabbedImages.size(); index++)
    {
        m_grabbedImages[index] = grabbedImage[index];
    }
}

DlgVisionProcessingNGRV_DetailSetup::~DlgVisionProcessingNGRV_DetailSetup()
{
    delete m_imageView;
    delete m_grid;
}

void DlgVisionProcessingNGRV_DetailSetup::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgVisionProcessingNGRV_DetailSetup, CDialog)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// DlgVisionProcessingNGRV_DetailSetup 메시지 처리기

BOOL DlgVisionProcessingNGRV_DetailSetup::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString title;
    title.Format(_T("NGRV Detail Setup - %s"), (LPCTSTR)m_visionPara->m_vecDefectName[m_defectID]);
    SetWindowText(title);

    // UI 총 크기 설정 : 900 x 600
    CRect uiViewArea;
    GetWindowRect(&uiViewArea);
    /*MoveWindow(uiViewArea.left, uiViewArea.top, uiViewArea.left + 900, uiViewArea.top + 600);

    uiViewArea.SetRect(uiViewArea.left, uiViewArea.top, uiViewArea.left + 900, uiViewArea.top + 600);*/

    // UI 중 반을 GridControl로 사용
    CRect gridControlArea;
    gridControlArea.left = uiViewArea.left;
    gridControlArea.top = uiViewArea.top;
    gridControlArea.right = uiViewArea.left + uiViewArea.Width() / 3;
    gridControlArea.bottom = static_cast<long>(uiViewArea.Height() * 0.92f + .5f);

    m_grid->Create(gridControlArea, this, static_cast<UINT>(IDC_STATIC));

    OnInitGrid();

    // UI 중 나머지 반을 ImageLotView로 사용
    CRect imageViewArea;
    imageViewArea.left = gridControlArea.right;
    imageViewArea.top = gridControlArea.top;
    imageViewArea.right = imageViewArea.left + static_cast<long>(gridControlArea.Width() * 1.9f + .5f);
    imageViewArea.bottom = imageViewArea.top + static_cast<long>(gridControlArea.Height() * 0.85f + .5f);

    m_imageView = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(imageViewArea), 0);

    ShowImage();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void DlgVisionProcessingNGRV_DetailSetup::OnInitGrid()
{
    m_grid->ResetContent();

    //Ref_imageSize에 따라 사용할 수 있는 Stitch Count를 다르게 지정한다
    // 기본 ROI 영역 크기 설정 : NGRV Camera FOV 19mm x 15mm
    Ipvm::Rect32r cameraFOV(0.f, 0.f, NGRV_CAMERA_X_UM, NGRV_CAMERA_Y_UM);
    Ipvm::Point32s2 stitch_count = get_Max_Stitch_Count(
        cur_imageInfo.m_scaleX, cur_imageInfo.m_scaleY, cameraFOV, m_ref_ImageSizeX, m_ref_ImageSizeY);

    if (auto* category = m_grid->AddCategory(_T("Parameter")))
    {
        CString strItemID = _T("000");
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItem(
                _T("Item ID"), m_visionPara->m_vecItemID[m_defectID], &m_visionPara->m_vecItemID[m_defectID])))
        {
            item->SetID(ITEM_ID_ITEM_ID);
        }

        CString strLossCode = _T("NN001");
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItem(_T("Defect Code"),
                m_visionPara->m_vecDefectCode[m_defectID], &m_visionPara->m_vecDefectCode[m_defectID])))
        {
            item->SetID(ITEM_ID_LOSS_CODE);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Disposition"),
                (int)m_visionPara->m_vecDisposition[m_defectID], &(int&)m_visionPara->m_vecDisposition[m_defectID])))
        {
            item->GetConstraints()->AddConstraint(GetDispositionText(DISPOSITION_NULL, true), DISPOSITION_NULL);
            item->GetConstraints()->AddConstraint(GetDispositionText(DISPOSITION_PASS, true), DISPOSITION_PASS);
            item->GetConstraints()->AddConstraint(GetDispositionText(DISPOSITION_MARGINAL, true), DISPOSITION_MARGINAL);
            item->GetConstraints()->AddConstraint(GetDispositionText(DISPOSITION_INVALID, true), DISPOSITION_INVALID);
            item->GetConstraints()->AddConstraint(GetDispositionText(DISPOSITION_PASSIVE, true), DISPOSITION_PASSIVE);
            item->GetConstraints()->AddConstraint(GetDispositionText(DISPOSITION_REJECT, true), DISPOSITION_REJECT);
            item->SetID(ITEM_ID_DISPOSITION);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Decision"),
                (int)m_visionPara->m_vecDecision[m_defectID], &(int&)m_visionPara->m_vecDecision[m_defectID])))
        {
            item->GetConstraints()->AddConstraint(GetResultText(NOT_MEASURED, true, true), NOT_MEASURED);
            item->GetConstraints()->AddConstraint(GetResultText(PASS, true, true), PASS);
            item->GetConstraints()->AddConstraint(GetResultText(MARGINAL, true, true), MARGINAL);
            item->GetConstraints()->AddConstraint(GetResultText(REJECT, true, true), REJECT);
            item->GetConstraints()->AddConstraint(GetResultText(INVALID, true, true), INVALID);
            item->SetID(ITEM_ID_DECISION);
        }

        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Stitch Count X"), (int)m_visionPara->m_vecStitchCount[m_defectID].m_x,
                    &(int&)m_visionPara->m_vecStitchCount[m_defectID].m_x)))
        {
            for (long count_stitch_x = 0; count_stitch_x < stitch_count.m_x; count_stitch_x++)
            {
                CString stitch_count_X("");
                stitch_count_X.Format(_T("%d"), count_stitch_x + 1);
                item->GetConstraints()->AddConstraint(stitch_count_X, count_stitch_x + 1);

                stitch_count_X.Empty();
            }

            item->SetID(ITEM_ID_STITCH_COUNT_X);
        }

        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Stitch Count Y"), (int)m_visionPara->m_vecStitchCount[m_defectID].m_y,
                    &(int&)m_visionPara->m_vecStitchCount[m_defectID].m_y)))
        {
            for (long count_stitch_y = 0; count_stitch_y < stitch_count.m_y; count_stitch_y++)
            {
                CString stitch_count_Y("");
                stitch_count_Y.Format(_T("%d"), count_stitch_y + 1);
                item->GetConstraints()->AddConstraint(stitch_count_Y, count_stitch_y + 1);

                stitch_count_Y.Empty();
            }

            item->SetID(ITEM_ID_STITCH_COUNT_Y);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("AutoFocus Type"),
                (int)m_visionPara->m_vecAFtype[m_defectID], &(int&)m_visionPara->m_vecAFtype[m_defectID])))
        {
            item->GetConstraints()->AddConstraint(_T("Not Use"), 0);
            item->GetConstraints()->AddConstraint(_T("Each Point"), 1);
            item->GetConstraints()->AddConstraint(_T("Plane"), 2);
            item->SetID(ITEM_ID_AUTOFOCUS_TYPE);
        }

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Grab Height Offset(um)"),
                m_visionPara->m_vecGrabHeightOffset_um[m_defectID], _T("%.0f"),
                m_visionPara->m_vecGrabHeightOffset_um[m_defectID], -500.f, 500.f, 0.f)))
        {
            item->SetID(ITEM_ID_GRAB_HEIGHT_OFFSET);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab 1st Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][0], &m_visionPara->m_vec2IllumUsage[m_defectID][0])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_1ST_FRAME);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab 2nd Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][1], &m_visionPara->m_vec2IllumUsage[m_defectID][1])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_2ND_FRAME);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab 3rd Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][2], &m_visionPara->m_vec2IllumUsage[m_defectID][2])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_3RD_FRAME);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab 4th Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][3], &m_visionPara->m_vec2IllumUsage[m_defectID][3])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_4TH_FRAME);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab 5th Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][4], &m_visionPara->m_vec2IllumUsage[m_defectID][4])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_5TH_FRAME);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab 6th Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][5], &m_visionPara->m_vec2IllumUsage[m_defectID][5])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_6TH_FRAME);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab 7th Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][6], &m_visionPara->m_vec2IllumUsage[m_defectID][6])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_7TH_FRAME);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab RV Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][7], &m_visionPara->m_vec2IllumUsage[m_defectID][7])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_RV_FRAME);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab UV Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][8], &m_visionPara->m_vec2IllumUsage[m_defectID][8])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_UV_FRAME);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Grab IR Frame"),
                m_visionPara->m_vec2IllumUsage[m_defectID][9], &m_visionPara->m_vec2IllumUsage[m_defectID][9])))
        {
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
            item->SetID(ITEM_ID_GRAB_IR_FRAME);
        }

        category->AddChildItem(new CCustomItemButton(_T("Apply"), TRUE, FALSE))->SetID(ITEM_BUTTON_APPLY);
        category->AddChildItem(new CCustomItemButton(_T("Cancel"), TRUE, FALSE))->SetID(ITEM_BUTTON_CANCEL);

        m_grid->FindItem(ITEM_BUTTON_APPLY)->SetHeight(50);
        m_grid->FindItem(ITEM_BUTTON_CANCEL)->SetHeight(50);

        category->Expand();
    }

    m_grid->SetViewDivider(0.60);
    m_grid->HighlightChangedButtonItems(TRUE);
    m_grid->ShowWindow(SW_SHOW);
}

LRESULT DlgVisionProcessingNGRV_DetailSetup::OnGridNotify(WPARAM wParam, LPARAM lParam)
{
    if (wParam == XTP_PGN_EDIT_CHANGED)
    {
        return 0;
    }

    if (wParam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lParam;

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (value->GetID())
        {
            case ITEM_BUTTON_APPLY:
                ClickButtonApply();
                return 0;
            case ITEM_BUTTON_CANCEL:
                ClickButtonCancel();
                return 0;
            default:
                break;
        }

        value->SetBool(FALSE);
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemBool*>(item))
    {
        switch (value->GetID())
        {
            case ITEM_ID_GRAB_2ND_FRAME:
                ShowFrameImage(1);
                break;
            case ITEM_ID_GRAB_3RD_FRAME:
                ShowFrameImage(2);
                break;
            case ITEM_ID_GRAB_4TH_FRAME:
                ShowFrameImage(3);
                break;
            case ITEM_ID_GRAB_5TH_FRAME:
                ShowFrameImage(4);
                break;
            case ITEM_ID_GRAB_6TH_FRAME:
                ShowFrameImage(5);
                break;
            case ITEM_ID_GRAB_7TH_FRAME:
                ShowFrameImage(6);
                break;
            case ITEM_ID_GRAB_RV_FRAME:
                ShowFrameImage(7);
                break;
            case ITEM_ID_GRAB_UV_FRAME:
                ShowFrameImage(8);
                break;
            case ITEM_ID_GRAB_IR_FRAME:
                ShowFrameImage(9);
                break;
            case ITEM_ID_GRAB_1ST_FRAME:
            default:
                ShowFrameImage(0);
                break;
        }

        // CheckBox를 선택했을 시, 변경 점을 Update - MED#6_JHB
        if (value->IsCheckBoxStyle() == TRUE)
        {
            value->SetCheckBoxStyle();
        }
        else
        {
            value->SetCheckBoxStyle();
        }
    }

    //CXTPPropertyGridItem
    if (auto* value = dynamic_cast<CXTPPropertyGridItem*>(item))
    {
        switch (value->GetID())
        {
            case ITEM_ID_ITEM_ID:
            {
                CString keyInItemID = value->GetValue();
                CString itemID = SimpleFunction::CleanAndFormatNumber(keyInItemID);
                value->SetValue(itemID);
                m_visionPara->m_vecItemID[m_defectID] = itemID;
            }
            break;
        }
    }

    return 0;
}

void DlgVisionProcessingNGRV_DetailSetup::ShowImage()
{
    if ((long)m_grabbedImages.size() <= 0)
        return;

    // Frame을 사용하는게 있는지 없는지 판단 후, 몇 번 Frame을 사용하는지 체크
    if (m_visionPara->m_vec2IllumUsage[m_defectID].size() <= 0)
        return;

    std::vector<long> vecfirstFrame(0);

    for (long index = 0; index < (long)m_visionPara->m_vec2IllumUsage[m_defectID].size(); index++)
    {
        if (m_visionPara->m_vec2IllumUsage[m_defectID][index] == TRUE)
        {
            vecfirstFrame.push_back(index);
        }
    }

    // Image Show
    // 먼저 Frame을 쓰지 않을 경우 Black Image
    if ((long)vecfirstFrame.size() <= 0)
    {
        Ipvm::Image8u3 image;
        image.FillZero();

        m_imageView->SetImage(image);
    }
    else
    {
        Ipvm::Image8u3 image(m_grabbedImages[(long)vecfirstFrame[0]]);
        m_imageView->SetImage(image);
    }
}

void DlgVisionProcessingNGRV_DetailSetup::ShowFrameImage(long frameNum)
{
    if ((long)m_grabbedImages.size() <= 0)
        return;

    m_imageView->SetImage(m_grabbedImages[frameNum]);
}

void DlgVisionProcessingNGRV_DetailSetup::ClickButtonApply()
{
    SetCurrentRoiStatus();

    return OnOK();
}

void DlgVisionProcessingNGRV_DetailSetup::ClickButtonCancel()
{
    return OnCancel();
}

void DlgVisionProcessingNGRV_DetailSetup::SetCurrentRoiStatus()
{
    // 현재 선택되어진 탭을 확인
    auto& visionProcessingDlg = m_visionInsp->m_pVisionInspDlg;
    auto& visionUnit = visionProcessingDlg->m_visionInsp->m_visionUnit;
    long selectedTap = visionProcessingDlg->m_selectionTab.GetCurSel();

    // Image Info 정보를 가져옴
    Ngrv2DImageInfo imageInfo;
    imageInfo.Init();

    if (selectedTap == enum2DVisionType::VISION_2D_BTM)
    {
        imageInfo = visionUnit.Get2DImageInfo_NGRV(VISION_MODULE_ID_2D_BTM);
    }
    else if (selectedTap == enum2DVisionType::VISION_2D_TOP)
    {
        imageInfo = visionUnit.Get2DImageInfo_NGRV(VISION_MODULE_ID_2D_TOP);
    }

    // 현재 이미지의 Scale 및 Stitch Count 확인
    const auto& scaleX = imageInfo.m_scaleX;
    const auto& scaleY = imageInfo.m_scaleY;
    const auto& stitchCount = m_visionPara->m_vecStitchCount[m_defectID];

    // 기본 ROI 영역 크기 설정 : NGRV Camera FOV 19mm x 15mm
    Ipvm::Rect32r cameraFOV(0.f, 0.f, NGRV_CAMERA_X_UM, NGRV_CAMERA_Y_UM);
    Ipvm::Rect32s singleROI(0, 0, (int)(cameraFOV.m_right / scaleX), (int)(cameraFOV.m_bottom / scaleY));

    // 현재 이미지의 Scale 및 Stitch Count를 이용하여 ROI 크기 설정
    Ipvm::Rect32s tempROI = m_visionPara->m_vecGrabPosROI[m_defectID];

    tempROI.m_right = tempROI.m_left + singleROI.Width() * stitchCount.m_x;
    tempROI.m_bottom = tempROI.m_top + singleROI.Height() * stitchCount.m_y;

    // Defect ROI 변경
    m_visionPara->m_vecGrabPosROI[m_defectID] = tempROI;

    // 변경된 ROI를 Image에 적용
    switch (selectedTap)
    {
        case 1:
            visionProcessingDlg->m_top2Dtab->SetROI();
            break;
        case 0:
        default:
            visionProcessingDlg->m_bottom2Dtab->SetROI();
            break;
    }

    UpdateData(TRUE);
}

Ipvm::Point32s2 DlgVisionProcessingNGRV_DetailSetup::get_Max_Stitch_Count(const float& scaleX, const float& scaleY,
    const Ipvm::Rect32r& CameraSpecFOV, const long& ref_ImageSizeX, const long& ref_ImageSizeY)
{
    UNREFERENCED_PARAMETER(scaleX);
    UNREFERENCED_PARAMETER(scaleY);
    UNREFERENCED_PARAMETER(CameraSpecFOV);
    UNREFERENCED_PARAMETER(ref_ImageSizeX);
    UNREFERENCED_PARAMETER(ref_ImageSizeY);
    ;

    //Ipvm::Point32s2 stitch_count(1, 1);
    Ipvm::Point32s2 stitch_count(MAX_STITCH_COUNT, MAX_STITCH_COUNT); //일단 3으로 고정

    return stitch_count;

    //Ipvm::Rect32s singleROI(0, 0, (int)(CameraSpecFOV.m_right / scaleX), (int)(CameraSpecFOV.m_bottom / scaleY));

    //float stitch_count_X = (ref_ImageSizeX / CameraSpecFOV.Width());
    //float stitch_count_Y = (ref_ImageSizeY / CameraSpecFOV.Height());

    //if (stitch_count_X > MAX_STITCH_COUNT)
    //    stitch_count.m_x = MAX_STITCH_COUNT;

    //if (stitch_count_Y > MAX_STITCH_COUNT)
    //    stitch_count.m_y = MAX_STITCH_COUNT;

    //return stitch_count;
}