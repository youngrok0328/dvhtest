//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgGeometryEditor.h"

//CPP_2_________________________________ This project's headers
#include "Resource.h"
#include "VisionInspectionGeometry.h"
#include "VisionInspectionGeometryPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridSlideBox.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Polygon32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum GridCtrlindex
{
    Grid_Index_Begin = 0,
    Grid_Index_Data_List = Grid_Index_Begin,
    Grid_Index_Param_Ref,
    Grid_Index_Param_Tar,
    Grid_Index_Param_Insp,
    Grid_Index_End,
};

IMPLEMENT_DYNAMIC(CDlgGeometryEditor, CDialog)

CDlgGeometryEditor::CDlgGeometryEditor(
    VisionInspectionGeometry* pVisionInsp, long nInspectionID /* = 0*/, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgGeometryEditor::IDD, pParent)
    , m_imageView(nullptr)
{
    m_pVisionInsp = pVisionInsp;
    m_pVisionPara = m_pVisionInsp->m_VisionPara;
    m_pvecSpec = &m_pVisionInsp->m_variableInspectionSpecs;

    m_nSizeX = m_pVisionInsp->getReusableMemory().GetInspImageSizeX();
    m_nSizeY = m_pVisionInsp->getReusableMemory().GetInspImageSizeY();

    m_pVisionInsp->getReusableMemory().GetInspByteImage(m_procImage);

    m_nSelectInspectionID = nInspectionID;

    m_strInspectionName = m_pVisionPara->m_vecstrInspectionName[nInspectionID];
    m_nInspectionType = m_pVisionPara->m_vecnInspectionType[nInspectionID];
    m_nInspectionDistanceResult = m_pVisionPara->m_vecnInspectionDistanceResult[nInspectionID];
    m_nInspectionCircleResult = m_pVisionPara->m_vecnInspectionCircleResult[nInspectionID];

    m_fReferenceSpec = m_pVisionPara->m_vecfReferenceSpec[nInspectionID];

    m_vecInfoParameter_0_Ref = m_pVisionPara->m_vecInfoParameter_0_Ref[nInspectionID];
    m_vecInfoParameter_0_Tar = m_pVisionPara->m_vecInfoParameter_0_Tar[nInspectionID];

    m_vecstrinfoName_Ref = m_pVisionPara->m_vecstrinfoName_Ref[nInspectionID];
    m_vecstrinfoName_Tar = m_pVisionPara->m_vecstrinfoName_Tar[nInspectionID];

    m_vecrtInspectionROI_Ref_BCU = m_pVisionPara->m_vecrtInspectionROI_Ref_BCU[nInspectionID];
    m_vecrtInspectionROI_Tar_BCU = m_pVisionPara->m_vecrtInspectionROI_Tar_BCU[nInspectionID];

    m_bChangedROI = FALSE;
    m_nChangedInspIID = -1;
    m_nSelectRefAndTar = -1;
}

CDlgGeometryEditor::~CDlgGeometryEditor()
{
    delete m_imageView;
}

void CDlgGeometryEditor::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_GRID_INFO_LIST, m_gridAlignInfoDataList);
    DDX_Control(pDX, IDC_GRID_REF_LIST, m_gridSettingParameter_Ref);
    DDX_Control(pDX, IDC_GRID_TAR_LIST, m_gridSettingParameter_Tar);
    DDX_Control(pDX, IDC_GRID_INSP_PARAM, m_gridParameter_Insp);
    DDX_Control(pDX, IDC_EDIT_RESULT, m_editResult);
    DDX_Control(pDX, IDC_IMAGE_VIEW, m_staticImageView);
    DDX_Control(pDX, IDC_STATIC_WARNING_REFERENCE, m_stcWarningReference);
    DDX_Control(pDX, IDC_STATIC_WARNING_TARGET, m_stcWarningTarget);
}

BEGIN_MESSAGE_MAP(CDlgGeometryEditor, CDialog)
ON_BN_CLICKED(IDC_BTN_ADD_REF, &CDlgGeometryEditor::OnBnClickedBtnAddRef)
ON_BN_CLICKED(IDC_BTN_ADD_TAR, &CDlgGeometryEditor::OnBnClickedBtnAddTar)

ON_BN_CLICKED(IDC_BTN_DEL_REF, &CDlgGeometryEditor::OnBnClickedBtnDelRef)
ON_BN_CLICKED(IDC_BTN_DEL_TAR, &CDlgGeometryEditor::OnBnClickedBtnDelTar)

ON_BN_CLICKED(IDC_BTN_RUN_REF, &CDlgGeometryEditor::OnBnClickedBtnRunRef)
ON_BN_CLICKED(IDC_BTN_RUN_TAR, &CDlgGeometryEditor::OnBnClickedBtnRunTar)
ON_BN_CLICKED(IDC_BTN_RUN, &CDlgGeometryEditor::OnBnClickedBtnRun)
ON_BN_CLICKED(IDOK, &CDlgGeometryEditor::OnBnClickedOk)

ON_NOTIFY(NM_CLICK, IDC_GRID_INFO_LIST, &CDlgGeometryEditor::OnGridClicked_AlignInfoData)
ON_NOTIFY(NM_CLICK, IDC_GRID_REF_LIST, &CDlgGeometryEditor::OnGridClicked_DataList_Ref)
ON_NOTIFY(NM_CLICK, IDC_GRID_TAR_LIST, &CDlgGeometryEditor::OnGridClicked_DataList_Tar)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_INSP_PARAM, &CDlgGeometryEditor::OnGridClicked_DataList_InspParam)

ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_REF_LIST, &CDlgGeometryEditor::OnGridEditEnd_DataList_Ref)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_TAR_LIST, &CDlgGeometryEditor::OnGridEditEnd_DataList_Tar)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_INSP_PARAM, &CDlgGeometryEditor::OnGridEditEnd_DataList_InspParam)

ON_BN_CLICKED(IDC_BTN_CANCEL, &CDlgGeometryEditor::OnBnClickedBtnCancel)
ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CDlgGeometryEditor 메시지 처리기입니다.
BOOL CDlgGeometryEditor::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString strTemp;
    strTemp.Format(_T("Geometry Inspection Stup - %s"), (LPCTSTR)(*m_pvecSpec)[m_nSelectInspectionID].m_specName);
    SetWindowText(strTemp);

    CRect rtImageWindow;
    m_staticImageView.GetWindowRect(&rtImageWindow);
    ScreenToClient(rtImageWindow);

    m_imageView
        = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(rtImageWindow), 0, m_pVisionInsp->GetCurVisionModule_Status());
    m_imageView->RegisterCallback_ROIChangeEnd(GetSafeHwnd(), this, callRoiChanged);

    m_pvecGridCtrl.resize(Grid_Index_End);
    m_pvecGridCtrl[Grid_Index_Data_List] = &m_gridAlignInfoDataList;
    m_pvecGridCtrl[Grid_Index_Param_Ref] = &m_gridSettingParameter_Ref;
    m_pvecGridCtrl[Grid_Index_Param_Tar] = &m_gridSettingParameter_Tar;
    m_pvecGridCtrl[Grid_Index_Param_Insp] = &m_gridParameter_Insp;

    for (long n = Grid_Index_Begin; n < Grid_Index_End; n++)
    {
        m_pvecGridCtrl[n]->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        m_pvecGridCtrl[n]->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        m_pvecGridCtrl[n]->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        m_pvecGridCtrl[n]->SetDefCellMargin(0);
        m_pvecGridCtrl[n]->SetFixedColumnSelection(FALSE);
        m_pvecGridCtrl[n]->SetFixedRowSelection(FALSE);
        m_pvecGridCtrl[n]->SetSingleColSelection(FALSE);
        m_pvecGridCtrl[n]->SetSingleRowSelection(FALSE);
        m_pvecGridCtrl[n]->SetTrackFocusCell(FALSE);
        m_pvecGridCtrl[n]->SetRowResize(FALSE);
        m_pvecGridCtrl[n]->SetColumnResize(FALSE);
    }

    // 영훈 20160612 : Align Info를 모아온다.
    if (m_pVisionInsp->GetAlignInfoData())
    {
        SetGridList_DataList();
        SetGridList_Param_Ref_And_Tar(
            m_pvecGridCtrl[Grid_Index_Param_Ref], m_vecstrinfoName_Ref, m_vecInfoParameter_0_Ref);
        SetGridList_Param_Ref_And_Tar(
            m_pvecGridCtrl[Grid_Index_Param_Tar], m_vecstrinfoName_Tar, m_vecInfoParameter_0_Tar);
        SetGridList_Param_Insp();

        ShowImage();
    }

    OnBnClickedBtnRun();

    strTemp.Empty();

    return TRUE;
}

void CDlgGeometryEditor::GetParameter(VisionInspectionGeometryPara* i_pVisionPara, long nInspectionID)
{
    i_pVisionPara = m_pVisionInsp->m_VisionPara;

    i_pVisionPara->m_vecstrInspectionName[nInspectionID] = m_strInspectionName;
    i_pVisionPara->m_vecnInspectionType[nInspectionID] = m_nInspectionType;
    i_pVisionPara->m_vecnInspectionDistanceResult[nInspectionID] = m_nInspectionDistanceResult;
    i_pVisionPara->m_vecnInspectionCircleResult[nInspectionID] = m_nInspectionCircleResult;

    i_pVisionPara->m_vecfReferenceSpec[nInspectionID] = m_fReferenceSpec;

    i_pVisionPara->m_vecInfoParameter_0_Ref[nInspectionID] = m_vecInfoParameter_0_Ref;
    i_pVisionPara->m_vecInfoParameter_0_Tar[nInspectionID] = m_vecInfoParameter_0_Tar;

    // 	m_pvecInfoParameter_1_Ref = &m_pVisionPara->m_vecInfoParameter_1_Ref[nInspectionID];
    // 	m_pvecInfoParameter_1_Tar = &m_pVisionPara->m_vecInfoParameter_1_Tar[nInspectionID];
    //
    // 	m_pvecInfoParameter_2_Ref = &m_pVisionPara->m_vecInfoParameter_2_Ref[nInspectionID];
    // 	m_pvecInfoParameter_2_Tar = &m_pVisionPara->m_vecInfoParameter_2_Tar[nInspectionID];

    i_pVisionPara->m_vecstrinfoName_Ref[nInspectionID] = m_vecstrinfoName_Ref;
    i_pVisionPara->m_vecstrinfoName_Tar[nInspectionID] = m_vecstrinfoName_Tar;

    i_pVisionPara->m_vecrtInspectionROI_Ref_BCU[nInspectionID] = m_vecrtInspectionROI_Ref_BCU;
    i_pVisionPara->m_vecrtInspectionROI_Tar_BCU[nInspectionID] = m_vecrtInspectionROI_Tar_BCU;
}

void CDlgGeometryEditor::ShowImage()
{
    VisionReusableMemory& visionReusableMemory = m_pVisionInsp->getReusableMemory();

    // Cur Image
    long nCurImageID = m_pVisionInsp->GetImageFrameIndex(0);
    const auto* curImage
        = m_pVisionInsp->getImageLotInsp().GetSafeImagePtr(nCurImageID, m_pVisionInsp->GetCurVisionModule_Status());
    if (curImage == nullptr)
        return;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(m_procImage), 0, m_procImage);

    if (!CippModules::GrayImageProcessingManage(
            visionReusableMemory, curImage, false, m_pVisionInsp->m_VisionPara->m_ImageProcMangePara, m_procImage))
    {
        return;
    }

    CString FrameToString;

    m_imageView->SetImage(m_procImage);
    m_imageView->NavigateTo(Ipvm::Rect32s(m_procImage));

    FrameToString.Empty();
}

void CDlgGeometryEditor::OnBnClickedOk()
{
    CDialog::OnOK();
}

void CDlgGeometryEditor::OnBnClickedBtnDelRef()
{
    m_vecstrinfoName_Ref.clear();
    m_vecInfoParameter_0_Ref.clear();

    SetGridList_Param_Ref_And_Tar(m_pvecGridCtrl[Grid_Index_Param_Ref], m_vecstrinfoName_Ref, m_vecInfoParameter_0_Ref);

    OnBnClickedBtnRun();
}

void CDlgGeometryEditor::OnBnClickedBtnDelTar()
{
    m_vecstrinfoName_Tar.clear();
    m_vecInfoParameter_0_Tar.clear();

    SetGridList_Param_Ref_And_Tar(m_pvecGridCtrl[Grid_Index_Param_Tar], m_vecstrinfoName_Tar, m_vecInfoParameter_0_Tar);

    OnBnClickedBtnRun();
}

void CDlgGeometryEditor::OnBnClickedBtnAddRef()
{
    // 이전에 선택된 Type이 뭐가 있는지 알아온다.
    long nInfoCount = (long)m_vecstrinfoName_Ref.size();

    // 이미 Line이든 뭐든 2개가 차있다면 그만둔다.
    if (nInfoCount >= 2)
        return;

    CGridCtrl* pGridList_Data = m_pvecGridCtrl[Grid_Index_Data_List];
    CGridCtrl* pGridList_Param_Ref = m_pvecGridCtrl[Grid_Index_Param_Ref];
    CGridCtrl* pGridList_Param_Tar = m_pvecGridCtrl[Grid_Index_Param_Tar];

    long nSelectRowID = GetSelectedGridCellRow(pGridList_Data);
    CString strInspeName_First = pGridList_Data->GetItemText(nSelectRowID, 0);

    // 영훈 : Circle Round는 항상 Ref가 기준이 되도록 한다. 근데 Ref, Tar 둘다 Circle일 경우는 굳이 바꿀 필요가 없다.
    BOOL bParamChange = TRUE;
    if ((long)m_vecstrinfoName_Tar.size() > 0)
    {
        for (long nID = 0; nID < (long)m_vecstrinfoName_Tar.size(); nID++)
        {
            if (m_vecstrinfoName_Tar[nID] != g_szAlignInfo_List_Name[Insp_Type_User_Round]
                && m_vecstrinfoName_Tar[nID] != g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
            {
                bParamChange = FALSE;
            }
        }
    }
    else
    {
        bParamChange = FALSE;
    }

    // Circle 관련 해서는 항상 Ref에 등록 되도록 해준다.
    if (bParamChange
        && (strInspeName_First != g_szAlignInfo_List_Name[Insp_Type_User_Round]
            && strInspeName_First != g_szAlignInfo_List_Name[Insp_Type_User_Ellips]))
    {
        if (nInfoCount != 0)
        {
            CString strTemp;
            strTemp.Format(_T("[%s] is possible only one."), (LPCTSTR)strInspeName_First);
            ::SimpleMessage(strTemp, MB_OK);

            strTemp.Empty();
            return;
        }
        std::vector<std::vector<long>> vecInfoParameter_0_Swap;
        std::vector<CString> vecstrinfoName_Swap;
        std::vector<Ipvm::Rect32r> vecrtInspectionROI_BCU_Swap;

        vecInfoParameter_0_Swap = m_vecInfoParameter_0_Ref;
        vecstrinfoName_Swap = m_vecstrinfoName_Ref;
        vecrtInspectionROI_BCU_Swap = m_vecrtInspectionROI_Ref_BCU;

        m_vecInfoParameter_0_Ref = m_vecInfoParameter_0_Tar;
        m_vecstrinfoName_Ref = m_vecstrinfoName_Tar;
        m_vecrtInspectionROI_Ref_BCU = m_vecrtInspectionROI_Tar_BCU;

        m_vecInfoParameter_0_Tar = vecInfoParameter_0_Swap;
        m_vecstrinfoName_Tar = vecstrinfoName_Swap;
        m_vecrtInspectionROI_Tar_BCU = vecrtInspectionROI_BCU_Swap;

        OnBnClickedBtnAdd_Data(pGridList_Data, pGridList_Param_Tar, m_vecstrinfoName_Tar, m_vecInfoParameter_0_Tar,
            m_vecrtInspectionROI_Tar_BCU);

        // Target 내용을 지웠으니 Grid를 새로 그려준다.
        SetGridList_Param_Ref_And_Tar(pGridList_Param_Ref, m_vecstrinfoName_Ref, m_vecInfoParameter_0_Ref);

        for (int nIdx = 0; nIdx < vecstrinfoName_Swap.size(); nIdx++)
        {
            vecstrinfoName_Swap[nIdx].Empty();
        }
    }
    else
    {
        OnBnClickedBtnAdd_Data(pGridList_Data, pGridList_Param_Ref, m_vecstrinfoName_Ref, m_vecInfoParameter_0_Ref,
            m_vecrtInspectionROI_Ref_BCU);
    }

    OnBnClickedBtnRun();

    strInspeName_First.Empty();
}

void CDlgGeometryEditor::OnBnClickedBtnAddTar()
{
    // 이전에 선택된 Type이 뭐가 있는지 알아온다.
    long nInfoCount = (long)m_vecstrinfoName_Tar.size();

    // 이미 Line이든 뭐든 2개가 차있다면 그만둔다.
    if (nInfoCount >= 2)
        return;

    CGridCtrl* pGridList_Data = m_pvecGridCtrl[Grid_Index_Data_List];
    CGridCtrl* pGridList_Param_Ref = m_pvecGridCtrl[Grid_Index_Param_Ref];
    CGridCtrl* pGridList_Param_Tar = m_pvecGridCtrl[Grid_Index_Param_Tar];

    long nSelectRowID = GetSelectedGridCellRow(pGridList_Data);
    CString strInspeName_First = pGridList_Data->GetItemText(nSelectRowID, 0);

    // 영훈 : Circle Round는 항상 Ref가 기준이 되도록 한다. 근데 Ref, Tar 둘다 Circle일 경우는 굳이 바꿀 필요가 없다.
    BOOL bParamChange = TRUE;
    for (long nID = 0; nID < (long)m_vecstrinfoName_Ref.size(); nID++)
    {
        if (m_vecstrinfoName_Ref[nID] == g_szAlignInfo_List_Name[Insp_Type_User_Round]
            || m_vecstrinfoName_Ref[nID] == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
        {
            bParamChange = FALSE;
        }
    }

    // Circle 관련 해서는 항상 Ref에 등록 되도록 해준다.
    if (bParamChange
        && (strInspeName_First == g_szAlignInfo_List_Name[Insp_Type_User_Round]
            || strInspeName_First == g_szAlignInfo_List_Name[Insp_Type_User_Ellips]))
    {
        if (nInfoCount != 0)
        {
            CString strTemp;
            strTemp.Format(_T("[%s] is possible only one."), (LPCTSTR)strInspeName_First);
            ::SimpleMessage(strTemp, MB_OK);
            strTemp.Empty();
            return;
        }
        std::vector<std::vector<long>> vecInfoParameter_0_Swap;
        std::vector<CString> vecstrinfoName_Swap;
        std::vector<Ipvm::Rect32r> vecrtInspectionROI_BCU_Swap;

        vecInfoParameter_0_Swap = m_vecInfoParameter_0_Tar;
        vecstrinfoName_Swap = m_vecstrinfoName_Tar;
        vecrtInspectionROI_BCU_Swap = m_vecrtInspectionROI_Tar_BCU;

        m_vecInfoParameter_0_Tar = m_vecInfoParameter_0_Ref;
        m_vecstrinfoName_Tar = m_vecstrinfoName_Ref;
        m_vecrtInspectionROI_Tar_BCU = m_vecrtInspectionROI_Ref_BCU;

        m_vecInfoParameter_0_Ref = vecInfoParameter_0_Swap;
        m_vecstrinfoName_Ref = vecstrinfoName_Swap;
        m_vecrtInspectionROI_Ref_BCU = vecrtInspectionROI_BCU_Swap;

        OnBnClickedBtnAdd_Data(pGridList_Data, pGridList_Param_Ref, m_vecstrinfoName_Ref, m_vecInfoParameter_0_Ref,
            m_vecrtInspectionROI_Ref_BCU);

        // Target 내용을 지웠으니 Grid를 새로 그려준다.
        SetGridList_Param_Ref_And_Tar(pGridList_Param_Tar, m_vecstrinfoName_Tar, m_vecInfoParameter_0_Tar);

        for (int nIdx = 0; nIdx < vecstrinfoName_Swap.size(); nIdx++)
        {
            vecstrinfoName_Swap[nIdx].Empty();
        }
    }
    else
    {
        OnBnClickedBtnAdd_Data(pGridList_Data, pGridList_Param_Tar, m_vecstrinfoName_Tar, m_vecInfoParameter_0_Tar,
            m_vecrtInspectionROI_Tar_BCU);
    }

    OnBnClickedBtnRun();

    strInspeName_First.Empty();
}

void CDlgGeometryEditor::OnBnClickedBtnAdd_Data(CGridCtrl* pGridList_Data, CGridCtrl* pGridList_Param,
    std::vector<CString>& vecstrinfoName, std::vector<std::vector<long>>& vecInfoParameter_0,
    std::vector<Ipvm::Rect32r>& vecrtInspROI_BCU)
{
    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    SAlignInfoDataList FirstData;
    SAlignInfoDataList SecondData;

    FirstData.nDataType = DataTypeList::Unknown;
    FirstData.nDataCount = -1;

    SecondData.nDataType = DataTypeList::Unknown;
    SecondData.nDataCount = -1;

    // 선택된 Align Type이 뭔지 알아온다.
    CString strInspeName_First;
    CString strInspeName_Second;

    // 이전에 선택된 Type이 뭐가 있는지 알아온다.
    long nInfoCount = (long)vecstrinfoName.size();

    // 이미 Line이든 뭐든 2개가 차있다면 그만둔다.
    if (nInfoCount >= 2)
    {
        CString strTemp;
        strTemp.Format(_T("You can register up to two."));
        ::SimpleMessage(strTemp, MB_OK);

        strTemp.Empty();

        return;
    }

    long nSelectRowID = GetSelectedGridCellRow(pGridList_Data);
    strInspeName_First = pGridList_Data->GetItemText(nSelectRowID, 0);

    if (nInfoCount != 0
        && (strInspeName_First == g_szAlignInfo_List_Name[Insp_Type_User_Round]
            || strInspeName_First == g_szAlignInfo_List_Name[Insp_Type_User_Ellips]))
    {
        CString strTemp;
        strTemp.Format(_T("[%s] is possible only one."), (LPCTSTR)strInspeName_First);
        ::SimpleMessage(strTemp, MB_OK);

        strTemp.Empty();

        return;
    }

    for (long nID = 0; nID < (long)m_pVisionInsp->m_vecAlignInfoList.size(); nID++)
    {
        if (m_pVisionInsp->m_vecAlignInfoList[nID].strName == strInspeName_First)
        {
            FirstData = m_pVisionInsp->m_vecAlignInfoList[nID];
            break;
        }
    }

    // 이미 뭔가 한개 등록이 되어 있는데 Circle은 한개만 있으면 되므로 다음으로 진행하지 않는다.
    if (nInfoCount == 1)
    {
        strInspeName_Second = pGridList_Param->GetItemText(1, 0);

        for (long nID = 0; nID < (long)m_pVisionInsp->m_vecAlignInfoList.size(); nID++)
        {
            if (m_pVisionInsp->m_vecAlignInfoList[nID].strName == strInspeName_Second)
            {
                SecondData = m_pVisionInsp->m_vecAlignInfoList[nID];
                break;
            }
        }

        if (SecondData.nDataType == DataTypeList::Point)
        {
            CString strTemp;
            strTemp.Format(_T("[%s] is possible only one."), (LPCTSTR)strInspeName_Second);
            ::SimpleMessage(strTemp, MB_OK);

            strTemp.Empty();

            return;
        }

        if (FirstData.nDataType != SecondData.nDataType)
        {
            CString strTemp;
            strTemp.Format(_T("It is possible same type."));
            ::SimpleMessage(strTemp, MB_OK);

            strTemp.Empty();

            return;
        }
    }

    if (FirstData.nDataType == DataTypeList::Unknown)
    {
        CString strTemp;
        strTemp.Format(_T("Inspection type is wrong."));
        ::SimpleMessage(strTemp, MB_OK);

        strTemp.Empty();

        return;
    }

    if (FirstData.nDataCount < 1)
    {
        CString strTemp;
        strTemp.Format(_T("Inspection data is wrong."));
        ::SimpleMessage(strTemp, MB_OK);

        strTemp.Empty();

        return;
    }

    long nInserID = nInfoCount;
    const auto& scale = m_pVisionInsp->getScale();

    // 추가된 Size
    nInfoCount += 1;
    vecstrinfoName.resize(nInfoCount);
    vecInfoParameter_0.resize(nInfoCount);
    vecrtInspROI_BCU.resize(nInfoCount);
    vecrtInspROI_BCU[nInserID] = scale.convert_PixelToBCU(FirstData.rtROI, imageCenter);

    vecstrinfoName[nInserID] = strInspeName_First;
    vecInfoParameter_0[nInserID].resize(FirstData.nDataCount);
    vecInfoParameter_0[nInserID][0] = 1;

    SetGridList_Param_Ref_And_Tar(pGridList_Param, vecstrinfoName, vecInfoParameter_0);

    strInspeName_First.Empty();
    strInspeName_Second.Empty();
}

void CDlgGeometryEditor::OnGridClicked_AlignInfoData(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    long row = pNotify->iRow;

    m_nSelectRefAndTar = -1;

    m_imageView->ImageOverlayClear();
    m_imageView->ROIClear();

    CGridCtrl* pGridList = m_pvecGridCtrl[Grid_Index_Data_List];

    CString strAlignInfoName = pGridList->GetItemText(row, 0);
    long nAlignInfoCount = (long)m_pVisionInsp->m_vecAlignInfoList.size();

    for (long nRow = 0; nRow < nAlignInfoCount; nRow++)
    {
        if (m_pVisionInsp->m_vecAlignInfoList[nRow].strName == strAlignInfoName)
        {
            m_imageView->ImageOverlayAdd(m_pVisionInsp->m_vecAlignInfoList[nRow].rtROI, RGB(0, 255, 0));
            break;
        }
    }

    m_imageView->ImageOverlayShow();

    result = 0;

    strAlignInfoName.Empty();
}

void CDlgGeometryEditor::OnGridClicked_DataList_Ref(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    long row = pNotify->iRow;
    long col = pNotify->iColumn;

    if (row <= 0)
        return;

    m_nSelectRefAndTar = enum_Select_Reference;

    CGridCtrl* pGridList = m_pvecGridCtrl[Grid_Index_Param_Ref];

    OnGridClicked_DataList(
        pGridList, row, col, m_vecstrinfoName_Ref, m_vecInfoParameter_0_Ref, m_vecrtInspectionROI_Ref_BCU);

    *result = 0;
}

void CDlgGeometryEditor::OnGridClicked_DataList_Tar(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    long row = pNotify->iRow;
    long col = pNotify->iColumn;

    if (row <= 0)
        return;

    m_nSelectRefAndTar = enum_Select_Target;

    CGridCtrl* pGridList = m_pvecGridCtrl[Grid_Index_Param_Tar];

    OnGridClicked_DataList(
        pGridList, row, col, m_vecstrinfoName_Tar, m_vecInfoParameter_0_Tar, m_vecrtInspectionROI_Tar_BCU);

    *result = 0;
}

void CDlgGeometryEditor::OnGridClicked_DataList(CGridCtrl* pGridList, long row, long col,
    std::vector<CString> m_vecstrinfoName, std::vector<std::vector<long>>& vecInfoParameter_0,
    std::vector<Ipvm::Rect32r>& vecrtInspROI_BCU)
{
    BOOL bControlChanged = FALSE;

    long nInspID = -1;
    long nParamID = col - 1;

    CString strInspeName = pGridList->GetItemText(row, 0);

    long nID = 0;
    if (row > 1)
        nID = 1;

    for (; nID < (long)m_vecstrinfoName.size(); nID++)
    {
        if (m_vecstrinfoName[nID] == strInspeName)
        {
            nInspID = nID;
            break;
        }
    }

    if (nInspID < 0)
        return;

    long nDataNum = (long)vecInfoParameter_0[nInspID].size();

    if (nDataNum <= nInspID)
        return;

    if (strInspeName == g_szAlignInfo_List_Name[Insp_Type_Body_Line]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_Other_Line])
    {
        if (nParamID > -1)
        {
            for (long n = 0; n < nDataNum; n++)
            {
                vecInfoParameter_0[nInspID][n] = 0;
            }

            vecInfoParameter_0[nInspID][nParamID] = 1;
        }
        else
        {
            for (long n = 0; n < (long)vecInfoParameter_0[nInspID].size(); n++)
            {
                if (vecInfoParameter_0[nInspID][n] == 1)
                {
                    nParamID = n;
                }
            }
        }

        bControlChanged = TRUE;
    }
    else if (strInspeName == g_szAlignInfo_List_Name[Insp_Type_User_Edge_Detect]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_User_Line]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
    {
        switch (nParamID)
        {
            case ED_Param_Search_Dir:
                vecInfoParameter_0[nInspID][ED_Param_Search_Dir] = GetGrid_SlideBox(pGridList, row, col);
                break;
            case ED_Param_Edge_Dir:
                vecInfoParameter_0[nInspID][ED_Param_Edge_Dir] = GetGrid_SlideBox(pGridList, row, col);
                break;
            case ED_Param_Edge_Type:
                vecInfoParameter_0[nInspID][ED_Param_Edge_Type] = GetGrid_SlideBox(pGridList, row, col);
                break;
            case ED_Param_Edge_Angle:
                vecInfoParameter_0[nInspID][ED_Param_Edge_Angle] = GetGrid_SlideBox(pGridList, row, col);
                break;
            case ED_Param_Edge_Detect_Area:
                vecInfoParameter_0[nInspID][ED_Param_Edge_Detect_Area] = vecInfoParameter_0
                    [nInspID]
                    [ED_Param_Edge_Detect_Area]; //kircheis_NeedToDebug// '=' 뒤에 '!'가 있었다. 다른 파라미터와 다른 이유도 파악해야 한다.
                break;
        }

        bControlChanged = TRUE;
    }
    else if (strInspeName == g_szAlignInfo_List_Name[Insp_Type_Round_LT_Info]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_Round_RT_Info]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_Round_LB_Info]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_Round_RB_Info])
    {
        bControlChanged = TRUE;
    }

    if (bControlChanged)
    {
        SetGridList_Param_Ref_And_Tar(pGridList, m_vecstrinfoName, vecInfoParameter_0);

        ShowOverlay(strInspeName, nInspID, nParamID, vecInfoParameter_0, vecrtInspROI_BCU);
    }

    strInspeName.Empty();
}

void CDlgGeometryEditor::OnGridEditEnd_DataList_Ref(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    long row = pNotify->iRow;
    long col = pNotify->iColumn;

    if (row <= 0 || col <= 0)
        return;

    CGridCtrl* pGridList = m_pvecGridCtrl[Grid_Index_Param_Ref];

    OnGridEditEnd_DataList(pGridList, row, col, m_vecstrinfoName_Ref, m_vecInfoParameter_0_Ref);

    *result = 0;
}

void CDlgGeometryEditor::OnGridEditEnd_DataList_Tar(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    long row = pNotify->iRow;
    long col = pNotify->iColumn;

    if (row <= 0 || col <= 0)
        return;

    CGridCtrl* pGridList = m_pvecGridCtrl[Grid_Index_Param_Tar];

    OnGridEditEnd_DataList(pGridList, row, col, m_vecstrinfoName_Tar, m_vecInfoParameter_0_Tar);

    *result = 0;
}

void CDlgGeometryEditor::OnGridEditEnd_DataList(CGridCtrl* pGridList, long row, long col,
    std::vector<CString> vecstrinfoName, std::vector<std::vector<long>>& vecInfoParameter_0)
{
    long nInspID(0);
    long nParamID = col - 1;

    CString strInspeName = pGridList->GetItemText(row, 0);

    long nID = 0;
    if (row > 1)
        nID = 1;

    for (; nID < (long)vecstrinfoName.size(); nID++)
    {
        if (vecstrinfoName[nID] == strInspeName)
        {
            nInspID = nID;
            break;
        }
    }

    long nDataNum = (long)vecInfoParameter_0[nInspID].size();

    if (nDataNum <= nInspID)
        return;

    if (strInspeName == g_szAlignInfo_List_Name[Insp_Type_User_Edge_Detect]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_User_Line]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
        || strInspeName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
    {
        switch (nParamID)
        {
            case ED_Param_Edge_Thresh:
                vecInfoParameter_0[nInspID][ED_Param_Edge_Thresh] = GetGrid_Text(pGridList, row, col);
                break;
        }
    }

    strInspeName.Empty();
}

void CDlgGeometryEditor::OnGridClicked_DataList_InspParam(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    long row = pNotify->iRow;

    CGridCtrl* pGridList = m_pvecGridCtrl[Grid_Index_Param_Insp];

    switch (row)
    {
        case Insp_Param_Type + 1:
            m_nInspectionType = GetGrid_SlideBox(pGridList, Insp_Param_Type + 1, 1);
            SetGrid_inspParam_Hidding(pGridList, m_nInspectionType);
            OnBnClickedBtnRun();
            break;
        case Insp_Param_Circle_Result + 1:
            m_nInspectionCircleResult = GetGrid_SlideBox(pGridList, Insp_Param_Circle_Result + 1, 1);
            break;
        case Insp_Param_Dist_Result + 1:
            m_nInspectionDistanceResult = GetGrid_SlideBox(pGridList, Insp_Param_Dist_Result + 1, 1);
            break;
        default:
            break;
    }

    pGridList->Refresh();

    *result = 0;
}

void CDlgGeometryEditor::SetGrid_inspParam_Hidding(CGridCtrl* pGridList, long nSelectInspType)
{
    pGridList->SetRowHeight(Insp_Param_Dist_Result + 1, 0);
    pGridList->SetRowHeight(Insp_Param_Circle_Result + 1, 0);

    if (nSelectInspType == Insp_Type_Shape_Distance)
    {
        pGridList->SetRowHeight(Insp_Param_Dist_Result + 1, 20);
    }
    else if (nSelectInspType == Insp_Type_Shape_Circle)
    {
        pGridList->SetRowHeight(Insp_Param_Circle_Result + 1, 20);
    }
}

void CDlgGeometryEditor::OnGridEditEnd_DataList_InspParam(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    if (pNotify == NULL)
        return;

    CGridCtrl* pGridList = m_pvecGridCtrl[Grid_Index_Param_Insp];

    m_fReferenceSpec = (float)_ttof(pGridList->GetItemText(Insp_Param_Spec + 1, 1));

    *result = 0;
}

long CDlgGeometryEditor::GetSelectedGridCellRow(CGridCtrl* pGridList)
{
    CCellRange CellRange = pGridList->GetSelectedCellRange();

    if (!CellRange.IsValid())
        return -1;

    if (CellRange.GetMinRow() != CellRange.GetMaxRow())
        return -1;

    long nSelItem = CellRange.GetTopLeft().row;

    return nSelItem;
}

void CDlgGeometryEditor::SetGrid_SlideBox(
    CGridCtrl* pGridList, long nRow, long nCol, std::vector<CString> vecstrDataList, long nSelectData)
{
    pGridList->GetCell(nRow, nCol)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    pGridList->SetCellType(nRow, nCol, RUNTIME_CLASS(CGridSlideBox));

    for (long nData = 0; nData < (long)vecstrDataList.size(); nData++)
    {
        ((CGridSlideBox*)pGridList->GetCell(nRow, nCol))->AddString(vecstrDataList[nData]);
    }
    ((CGridSlideBox*)pGridList->GetCell(nRow, nCol))->SetCurSel(nSelectData);
}

long CDlgGeometryEditor::GetGrid_SlideBox(CGridCtrl* pGridList, long nRow, long nCol)
{
    long nSelectData = ((CGridSlideBox*)pGridList->GetCell(nRow, nCol))->GetCurSel();
    return nSelectData;
}

void CDlgGeometryEditor::SetGrid_Text(CGridCtrl* pGridList, long nRow, long nCol, CString strText)
{
    pGridList->SetItemText(nRow, nCol, strText);
}

long CDlgGeometryEditor::GetGrid_Text(CGridCtrl* pGridList, long nRow, long nCol)
{
    CString strTemp = pGridList->GetItemText(nRow, nCol);
    long nData = _ttoi(strTemp);

    strTemp.Empty();

    return nData;
}

//영훈 : check box 너무 못생겼다. Grid Cell의 색깔이 바뀌도록 해야겠다. 선택된거 초록색 비선택된거 빨간색.
void CDlgGeometryEditor::SetGrid_CheckBox(
    CGridCtrl* pGridList, CString strTrueName, CString strFalseName, long nRow, long nCol, BOOL bCheck)
{
    pGridList->SetItemState(nRow, nCol, GVIS_READONLY);

    if (bCheck)
    {
        pGridList->SetItemBkColour(nRow, nCol, RGB(100, 255, 100));
        pGridList->SetItemText(nRow, nCol, strTrueName);
    }
    else
    {
        pGridList->SetItemBkColour(nRow, nCol, RGB(255, 150, 150));
        pGridList->SetItemText(nRow, nCol, strFalseName);
    }
}

void CDlgGeometryEditor::SetGridList_DataList()
{
    CGridCtrl* pGridList = m_pvecGridCtrl[Grid_Index_Data_List];

    long nAlignInfoCount = (long)m_pVisionInsp->m_vecAlignInfoList.size();

    pGridList->DeleteAllItems();

    // Items + Header
    pGridList->SetRowCount(nAlignInfoCount + 1);
    pGridList->SetColumnCount(1);
    pGridList->SetItemText(0, 0, _T("Align Data List"));
    pGridList->SetItemBkColour(0, 0, RGB(200, 200, 255));
    pGridList->SetItemState(0, 0, GVIS_READONLY);
    pGridList->SetColumnWidth(0, 165);

    long nItemRow = 1;
    for (long nRow = 0; nRow < nAlignInfoCount; nRow++)
    {
        pGridList->SetItemText(nItemRow, 0, m_pVisionInsp->m_vecAlignInfoList[nRow].strName);
        pGridList->SetItemBkColour(nItemRow, 0, RGB(255, 255, 200));
        pGridList->SetItemState(nItemRow, 0, GVIS_READONLY);

        nItemRow++;
    }

    pGridList->Refresh();
}

void CDlgGeometryEditor::SetGridList_Param_Insp()
{
    CGridCtrl* pGridList = m_pvecGridCtrl[Grid_Index_Param_Insp];
    std::vector<CString> vecstrDataList;

    pGridList->DeleteAllItems();

    // Items + Header
    pGridList->SetRowCount(Insp_Param_End + 1);
    pGridList->SetColumnCount(2);

    pGridList->MergeCells(0, 0, 0, 1);
    pGridList->GetCell(0, 0)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    pGridList->SetItemText(0, 0, _T("Inspection Parameter"));

    pGridList->SetItemBkColour(0, 0, RGB(200, 200, 255));
    pGridList->SetItemState(0, 0, GVIS_READONLY);
    pGridList->SetColumnWidth(0, 90);
    pGridList->SetColumnWidth(1, 130);

    long nRowID = 0;
    for (long nRow = 0; nRow < Insp_Param_End; nRow++)
    {
        nRowID = nRow + 1;

        pGridList->SetItemText(nRowID, 0, g_szInspectionParameterName[nRow]);
        pGridList->SetItemBkColour(nRowID, 0, RGB(255, 255, 200));
        pGridList->SetItemState(nRowID, 0, GVIS_READONLY);
    }

    vecstrDataList.clear();
    for (long n = Insp_Type_Shape_Begin; n < Insp_Type_Shape_End; n++)
    {
        vecstrDataList.push_back(g_szSInspectionType_DistanceName[n]);
    }
    SetGrid_SlideBox(pGridList, Insp_Param_Type + 1, 1, vecstrDataList, m_nInspectionType);

    vecstrDataList.clear();
    vecstrDataList.push_back(_T("Dist XY"));
    vecstrDataList.push_back(_T("Dist X"));
    vecstrDataList.push_back(_T("Dist Y"));
    SetGrid_SlideBox(pGridList, Insp_Param_Dist_Result + 1, 1, vecstrDataList, m_nInspectionDistanceResult);

    vecstrDataList.clear();
    vecstrDataList.push_back(_T("Radius"));
    vecstrDataList.push_back(_T("Diameter"));
    SetGrid_SlideBox(pGridList, Insp_Param_Circle_Result + 1, 1, vecstrDataList, m_nInspectionCircleResult);

    pGridList->SetItemTextFmt(Insp_Param_Spec + 1, 1, _T("%.2f"), m_fReferenceSpec);

    // 숨겨야하는 칸이 있는지 확인한다.
    SetGrid_inspParam_Hidding(pGridList, m_nInspectionType);

    pGridList->Refresh();

    for (int nIdx = 0; nIdx < vecstrDataList.size(); nIdx++)
    {
        vecstrDataList[nIdx].Empty();
    }
}

void CDlgGeometryEditor::SetGridList_Param_Ref_And_Tar(
    CGridCtrl* pGridList, std::vector<CString> vecstrName, std::vector<std::vector<long>>& vecnCheckboxList)
{
    long nRowCount = (long)vecstrName.size();
    long nColCount = 0;

    for (long nRow = 0; nRow < nRowCount; nRow++)
    {
        if (nColCount < (long)vecnCheckboxList[nRow].size())
        {
            nColCount = (long)vecnCheckboxList[nRow].size();
        }
    }

    pGridList->DeleteAllItems();

    // 메뉴바 가로 및 세로 세팅
    pGridList->SetRowCount(1);

    // 각 Grid에 처음 진입하면 메뉴를 그려줘야하므로 추가해주도록 한다.
    BOOL bBodyLineFirst = TRUE;
    BOOL bBodyCenterFirst = TRUE;
    BOOL bRoundCircleFirst = TRUE;
    BOOL bEdgeDetectFirst = TRUE;
    BOOL bEdgeDetectCircleFirst = TRUE;

    for (long nInspID = 0; nInspID < (long)vecstrName.size(); nInspID++)
    {
        if (vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_Body_Line]
            || vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_Other_Line])
        {
            vecnCheckboxList[nInspID].resize(BL_Param_End);

            // 메뉴바 가로 및 세로 세팅
            pGridList->SetColumnCount(BL_Param_End + 1);

            SetGridList_BodyInfo(Insp_Type_Body_Line, pGridList, nInspID, vecstrName[nInspID],
                vecnCheckboxList[nInspID], bBodyLineFirst);
            bBodyLineFirst = FALSE;
        }
        else if (vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_Body_Center_Info])
        {
            vecnCheckboxList[nInspID].resize(1);

            // 메뉴바 가로 및 세로 세팅
            pGridList->SetColumnCount(1 + 1);

            SetGridList_BodyInfo(Insp_Type_Body_Center_Info, pGridList, nInspID, vecstrName[nInspID],
                vecnCheckboxList[nInspID], bBodyCenterFirst);
            bBodyCenterFirst = FALSE;
        }
        else if (vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_User_Edge_Detect]
            || vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_User_Line])
        {
            vecnCheckboxList[nInspID].resize(ED_Param_End);

            // 메뉴바 가로 및 세로 세팅
            pGridList->SetColumnCount(ED_Param_End + 1);

            SetGridList_UserInfo(Insp_Type_User_Edge_Detect, pGridList, nInspID, vecstrName[nInspID],
                vecnCheckboxList[nInspID], bEdgeDetectFirst);
            bEdgeDetectFirst = FALSE;
        }
        else if (vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_User_Round])
        {
            vecnCheckboxList[nInspID].resize(ED_Param_End);

            // 메뉴바 가로 및 세로 세팅
            pGridList->SetColumnCount(ED_Param_End + 1);

            SetGridList_UserInfo(Insp_Type_User_Round, pGridList, nInspID, vecstrName[nInspID],
                vecnCheckboxList[nInspID], bEdgeDetectCircleFirst);
            bEdgeDetectCircleFirst = FALSE;
        }
        else if (vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
        {
            vecnCheckboxList[nInspID].resize(ED_Param_End);

            // 메뉴바 가로 및 세로 세팅
            pGridList->SetColumnCount(ED_Param_End + 1);

            SetGridList_UserInfo(Insp_Type_User_Ellips, pGridList, nInspID, vecstrName[nInspID],
                vecnCheckboxList[nInspID], bEdgeDetectCircleFirst);
            bEdgeDetectCircleFirst = FALSE;
        }
        else if (vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_Round_LT_Info]
            || vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_Round_RT_Info]
            || vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_Round_LB_Info]
            || vecstrName[nInspID] == g_szAlignInfo_List_Name[Insp_Type_Round_RB_Info])
        {
            vecnCheckboxList[nInspID].resize(1);

            // 메뉴바 가로 및 세로 세팅
            pGridList->SetColumnCount(1 + 1);

            SetGridList_RoundCircleInfo(Insp_Type_Round_LT_Info, pGridList, nInspID, vecstrName[nInspID],
                vecnCheckboxList[nInspID], bRoundCircleFirst);
            bRoundCircleFirst = FALSE;
        }
    }

    pGridList->Refresh();
}

void CDlgGeometryEditor::SetGridList_BodyInfo(long nBodyInfoID, CGridCtrl* pGridList, long nRowID, CString strName,
    std::vector<long> vecnParametrData, BOOL bFirst)
{
    long nColCount = BL_Param_End;

    long nRowItem = 0;
    long nColItem = 0;

    // 가로 Parameter Menu Setting
    // 처음 진입했을 경우에만 해준다. 다음 진입일 경우 바로 Data 쓰기
    if (bFirst == TRUE)
    {
        nRowItem = nRowID * 2;
        pGridList->SetRowCount((nRowID + 1) * 2);

        pGridList->SetItemText(nRowItem, 0, _T("Info Name"));
        pGridList->SetItemBkColour(nRowItem, 0, RGB(200, 200, 255));
        pGridList->SetItemState(nRowItem, 0, GVIS_READONLY);
        pGridList->SetColumnWidth(0, 120);

        if (nBodyInfoID == Insp_Type_Body_Line)
        {
            for (long nCol = 0; nCol < nColCount; nCol++)
            {
                nColItem = nCol + 1;

                pGridList->SetItemText(nRowItem, nColItem, g_szBodyLineParameter_Name[nCol]);
                pGridList->SetItemBkColour(nRowItem, nColItem, RGB(200, 200, 255));
                pGridList->SetItemState(nRowItem, nColItem, GVIS_READONLY);
                //pGridList->SetColumnWidth(nColItem, nColWidth);
            }
        }
        else if (nBodyInfoID == Insp_Type_Body_Center_Info)
        {
            nColItem = 1;

            pGridList->SetItemText(nRowItem, nColItem, _T("Use Data"));
            pGridList->SetItemBkColour(nRowItem, nColItem, RGB(200, 200, 255));
            pGridList->SetItemState(nRowItem, nColItem, GVIS_READONLY);
            //pGridList->SetColumnWidth(nColItem, nColWidth);
        }

        nRowItem += 1;
    }
    else
    {
        nRowItem = nRowID + 1;
        pGridList->SetRowCount(nRowItem + 1);
    }

    // 실제 데이터 >> Insp Name
    pGridList->SetItemBkColour(nRowItem, 0, RGB(255, 255, 200));
    pGridList->SetItemState(nRowItem, 0, GVIS_READONLY);
    pGridList->SetItemText(nRowItem, 0, strName);

    // 실제 데이터 >> Insp Parameter
    if (nBodyInfoID == Insp_Type_Body_Line)
    {
        for (long nCol = 0; nCol < nColCount; nCol++)
        {
            nColItem = nCol + 1;

            SetGrid_CheckBox(pGridList, _T("Use"), _T("Not Use"), nRowItem, nColItem, vecnParametrData[nCol]);
        }
    }
    else if (nBodyInfoID == Insp_Type_Body_Center_Info)
    {
        SetGrid_CheckBox(pGridList, _T("Use"), _T("Not Use"), nRowItem, 1, vecnParametrData[0]);
    }
}

void CDlgGeometryEditor::SetGridList_RoundCircleInfo(long nUserInfoID, CGridCtrl* pGridList, long nRowID,
    CString strName, std::vector<long> vecnParametrData, BOOL bFirst)
{
    if (nUserInfoID < 0)
        return;

    long nRowItem = 0;
    long nColItem = 0;

    // 가로 Parameter Menu Setting
    // 처음 진입했을 경우에만 해준다. 다음 진입일 경우 바로 Data 쓰기
    if (bFirst == TRUE)
    {
        nColItem = 1;

        nRowItem = nRowID * 2;
        pGridList->SetRowCount((nRowID + 1) * 2);

        pGridList->SetItemText(nRowItem, 0, _T("Info Name"));
        pGridList->SetItemBkColour(nRowItem, 0, RGB(200, 200, 255));
        pGridList->SetItemState(nRowItem, 0, GVIS_READONLY);
        pGridList->SetColumnWidth(0, 120);

        pGridList->SetItemText(nRowItem, nColItem, _T("Center Point"));
        pGridList->SetItemBkColour(nRowItem, nColItem, RGB(200, 200, 255));
        pGridList->SetItemState(nRowItem, nColItem, GVIS_READONLY);

        nRowItem += 1;
    }
    else
    {
        nRowItem = nRowID + 1;
        pGridList->SetRowCount(nRowItem + 1);
    }

    // 실제 데이터 >> Insp Name
    pGridList->SetItemBkColour(nRowItem, 0, RGB(255, 255, 200));
    pGridList->SetItemState(nRowItem, 0, GVIS_READONLY);
    pGridList->SetItemText(nRowItem, 0, strName);

    // 실제 데이터 >> Insp Parameter
    SetGrid_CheckBox(pGridList, _T("Line"), _T("Point"), nRowItem, 1, vecnParametrData[0]);
}

void CDlgGeometryEditor::SetGridList_UserInfo(long nUserInfoID, CGridCtrl* pGridList, long nRowID, CString strName,
    std::vector<long> vecnParametrData, BOOL bFirst)
{
    long nColCount = ED_Param_End;

    long nRowItem = 0;
    long nColItem = 0;
    long nColWidth = 70;

    std::vector<CString> vecstrTemp;
    CString strTemp;

    // 가로 Parameter Menu Setting
    // 처음 진입했을 경우에만 해준다. 다음 진입일 경우 바로 Data 쓰기
    if (bFirst == TRUE)
    {
        nRowItem = nRowID * 2;
        pGridList->SetRowCount((nRowID + 1) * 2);

        pGridList->SetItemText(nRowItem, 0, _T("Info Name"));
        pGridList->SetItemBkColour(nRowItem, 0, RGB(200, 200, 255));
        pGridList->SetItemState(nRowItem, 0, GVIS_READONLY);
        pGridList->SetColumnWidth(0, 120);

        // 파라미터 메뉴 글자 입력
        if (nUserInfoID == Insp_Type_User_Edge_Detect || nUserInfoID == Insp_Type_User_Line)
        {
            for (long nCol = 0; nCol < nColCount; nCol++)
            {
                nColItem = nCol + 1;

                pGridList->SetItemText(nRowItem, nColItem, g_szEdgeDetectParameterName[nCol]);
                pGridList->SetItemBkColour(nRowItem, nColItem, RGB(200, 200, 255));
                pGridList->SetItemState(nRowItem, nColItem, GVIS_READONLY);
                pGridList->SetColumnWidth(nColItem, nColWidth);
            }
        }
        else if (nUserInfoID == Insp_Type_User_Round || nUserInfoID == Insp_Type_User_Ellips)
        {
            for (long nCol = 0; nCol < nColCount; nCol++)
            {
                nColItem = nCol + 1;

                pGridList->SetItemText(nRowItem, nColItem, g_szEdgeDetectParameterName_Circle[nCol]);
                pGridList->SetItemBkColour(nRowItem, nColItem, RGB(200, 200, 255));
                pGridList->SetItemState(nRowItem, nColItem, GVIS_READONLY);
                pGridList->SetColumnWidth(nColItem, nColWidth);
            }
        }

        nRowItem += 1;
    }
    else
    {
        nRowItem = nRowID + 1;
        pGridList->SetRowCount(nRowItem + 1);
    }

    // 실제 데이터 >> Insp Name
    pGridList->SetItemBkColour(nRowItem, 0, RGB(255, 255, 200));
    pGridList->SetItemState(nRowItem, 0, GVIS_READONLY);
    pGridList->SetItemText(nRowItem, 0, strName);

    // 실제 데이터 >> Insp Parameter
    if (nUserInfoID == Insp_Type_User_Edge_Detect || nUserInfoID == Insp_Type_User_Line
        || nUserInfoID == Insp_Type_User_Round || nUserInfoID == Insp_Type_User_Ellips)
    {
        nColItem = ED_Param_Search_Dir + 1;
        vecstrTemp.clear();
        if (nUserInfoID == Insp_Type_User_Ellips)
        {
            vecstrTemp.push_back(_T("Out"));
            vecstrTemp.push_back(_T("In"));
        }
        else if (nUserInfoID == Insp_Type_User_Round)
        {
            vecstrTemp.push_back(_T("LT"));
            vecstrTemp.push_back(_T("RT"));
            vecstrTemp.push_back(_T("LB"));
            vecstrTemp.push_back(_T("RB"));
        }
        else
        {
            vecstrTemp.push_back(_T("Up"));
            vecstrTemp.push_back(_T("Down"));
            vecstrTemp.push_back(_T("Left"));
            vecstrTemp.push_back(_T("Right"));
        }
        SetGrid_SlideBox(pGridList, nRowItem, nColItem, vecstrTemp, vecnParametrData[ED_Param_Search_Dir]);
        pGridList->SetColumnWidth(nColItem, 80);

        nColItem = ED_Param_Edge_Dir + 1;
        vecstrTemp.clear();
        vecstrTemp.push_back(_T("Falling"));
        vecstrTemp.push_back(_T("Rising"));
        SetGrid_SlideBox(pGridList, nRowItem, nColItem, vecstrTemp, vecnParametrData[ED_Param_Edge_Dir]);
        pGridList->SetColumnWidth(nColItem, 100);

        nColItem = ED_Param_Edge_Type + 1;
        vecstrTemp.clear();
        vecstrTemp.push_back(_T("Best Edge"));
        vecstrTemp.push_back(_T("First Edge"));
        SetGrid_SlideBox(pGridList, nRowItem, nColItem, vecstrTemp, vecnParametrData[ED_Param_Edge_Type]);
        pGridList->SetColumnWidth(nColItem, 120);

        nColItem = ED_Param_Edge_Thresh + 1;
        strTemp.Format(_T("%d"), vecnParametrData[ED_Param_Edge_Thresh]);
        SetGrid_Text(pGridList, nRowItem, nColItem, strTemp);
        pGridList->SetColumnWidth(nColItem, 80);

        nColItem = ED_Param_Edge_Angle + 1;
        vecstrTemp.clear();
        vecstrTemp.push_back(_T("Normal")); // 0도
        vecstrTemp.push_back(_T("Clockwise")); // 45도
        vecstrTemp.push_back(_T("C.C.W")); // -45도
        SetGrid_SlideBox(pGridList, nRowItem, nColItem, vecstrTemp, vecnParametrData[ED_Param_Edge_Angle]);
        pGridList->SetColumnWidth(nColItem, 130);

        nColItem = ED_Param_Edge_Detect_Area + 1;
        if (nUserInfoID == Insp_Type_User_Round || nUserInfoID == Insp_Type_User_Ellips)
        {
            SetGrid_CheckBox(pGridList, _T("Circle Line"), _T("Center P.t"), nRowItem, nColItem,
                vecnParametrData[ED_Param_Edge_Detect_Area]);
        }
        else
        {
            SetGrid_CheckBox(pGridList, _T("Over ROI"), _T("In ROI"), nRowItem, nColItem,
                vecnParametrData[ED_Param_Edge_Detect_Area]);
        }
        pGridList->SetColumnWidth(nColItem, 100);
    }

    for (int nIdx = 0; nIdx < vecstrTemp.size(); nIdx++)
    {
        vecstrTemp[nIdx].Empty();
    }
    strTemp.Empty();
}

void CDlgGeometryEditor::OnBnClickedBtnRunRef()
{
    m_pVisionInsp->m_vecfptlineData_Start.clear();
    m_pVisionInsp->m_vecfptlineData_End.clear();

    // Reference Parameter 계산
    m_pVisionInsp->m_AlignInfoData_Ref.Init();
    if (m_pVisionInsp->DoInspectoin_Align_Ref_Tar(m_pVisionInsp->m_AlignInfoData_Ref, m_vecstrinfoName_Ref,
            m_vecrtInspectionROI_Ref_BCU, m_vecInfoParameter_0_Ref, m_procImage))
    {
        m_imageView->ImageOverlayClear();
        m_imageView->ROIClear();

        if (m_vecstrinfoName_Ref[0] == g_szAlignInfo_List_Name[Insp_Type_User_Round]
            || m_vecstrinfoName_Ref[0] == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
        {
            Ipvm::Rect32r rtCircleROI;
            rtCircleROI.m_left = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_x
                - m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius;
            rtCircleROI.m_right = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_x
                + m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius;
            rtCircleROI.m_top = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_y
                - m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius;
            rtCircleROI.m_bottom = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_y
                + m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius;

            m_imageView->ImageOverlayAdd(rtCircleROI, RGB(0, 255, 0));
        }

        if (m_pVisionInsp->m_vecfptlineData_Start.size() > 0)
        {
            long nLineCount = (long)m_pVisionInsp->m_vecfptlineData_Start[0].size();
            for (long index = 0; index < nLineCount; index++)
            {
                m_imageView->ImageOverlayAdd(Ipvm::LineSeg32r(m_pVisionInsp->m_vecfptlineData_Start[0][index].m_x,
                                                 m_pVisionInsp->m_vecfptlineData_Start[0][index].m_y,
                                                 m_pVisionInsp->m_vecfptlineData_End[0][index].m_x,
                                                 m_pVisionInsp->m_vecfptlineData_End[0][index].m_y),
                    RGB(255, 0, 0));
            }
        }

        for (auto& point : m_pVisionInsp->m_AlignInfoData_Ref.vecfptData)
        {
            m_imageView->ImageOverlayAdd(point, RGB(0, 255, 0));
        }

        m_imageView->ImageOverlayShow();
    }
}

void CDlgGeometryEditor::OnBnClickedBtnRunTar()
{
    m_pVisionInsp->m_vecfptlineData_Start.clear();
    m_pVisionInsp->m_vecfptlineData_End.clear();

    // Target Parameter 계산
    m_pVisionInsp->m_AlignInfoData_Tar.Init();
    if (m_pVisionInsp->DoInspectoin_Align_Ref_Tar(m_pVisionInsp->m_AlignInfoData_Tar, m_vecstrinfoName_Tar,
            m_vecrtInspectionROI_Tar_BCU, m_vecInfoParameter_0_Tar, m_procImage))
    {
        m_imageView->ImageOverlayClear();
        m_imageView->ROIClear();

        if (m_vecstrinfoName_Tar[0] == g_szAlignInfo_List_Name[Insp_Type_User_Round]
            || m_vecstrinfoName_Tar[0] == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
        {
            Ipvm::Rect32r rtCircleROI;
            rtCircleROI.m_left = m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_x
                - m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_radius;
            rtCircleROI.m_right = m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_x
                + m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_radius;
            rtCircleROI.m_top = m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_y
                - m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_radius;
            rtCircleROI.m_bottom = m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_y
                + m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_radius;

            m_imageView->ImageOverlayAdd(rtCircleROI, RGB(0, 255, 0));
        }

        long nLineCount = 0;
        if ((long)m_pVisionInsp->m_vecfptlineData_Start.size())
        {
            nLineCount = (long)m_pVisionInsp->m_vecfptlineData_Start[0].size();
        }
        for (long index = 0; index < nLineCount; index++)
        {
            m_imageView->ImageOverlayAdd(Ipvm::LineSeg32r(m_pVisionInsp->m_vecfptlineData_Start[0][index].m_x,
                                             m_pVisionInsp->m_vecfptlineData_Start[0][index].m_y,
                                             m_pVisionInsp->m_vecfptlineData_End[0][index].m_x,
                                             m_pVisionInsp->m_vecfptlineData_End[0][index].m_y),
                RGB(255, 0, 0));
        }

        for (auto& point : m_pVisionInsp->m_AlignInfoData_Tar.vecfptData)
        {
            m_imageView->ImageOverlayAdd(point, RGB(0, 255, 0));
        }

        m_imageView->ImageOverlayShow();
    }
}

void CDlgGeometryEditor::OnBnClickedBtnRun()
{
    CString textReference;
    CString textTarget;

    std::vector<float> vecfError;
    std::vector<Ipvm::Rect32s> vecrtROI;
    bool runSuccess = false;

    do
    {
        // Reference Parameter 계산
        m_pVisionInsp->m_AlignInfoData_Ref.Init();
        if (!m_pVisionInsp->DoInspectoin_Align_Ref_Tar(m_pVisionInsp->m_AlignInfoData_Ref, m_vecstrinfoName_Ref,
                m_vecrtInspectionROI_Ref_BCU, m_vecInfoParameter_0_Ref, m_procImage))
        {
            break;
        }

        // Target Parameter 계산
        // 영훈 Target은 없을 수도 있다. 그냥 진행한다.
        m_pVisionInsp->m_AlignInfoData_Tar.Init();
        m_pVisionInsp->DoInspectoin_Align_Ref_Tar(m_pVisionInsp->m_AlignInfoData_Tar, m_vecstrinfoName_Tar,
            m_vecrtInspectionROI_Tar_BCU, m_vecInfoParameter_0_Tar, m_procImage);

        // Ref 와 Target의 align 정보를 이용해 최종 계산 진행

        runSuccess = m_pVisionInsp->DoInspectoin_Final(NULL, NULL, m_nInspectionType, m_fReferenceSpec,
                         m_nInspectionDistanceResult, m_nInspectionCircleResult, vecfError, vecrtROI)
            ? true
            : false;

        textReference = m_pVisionInsp->GetFinalCheckError_Reference();
        textTarget = m_pVisionInsp->GetFinalCheckError_Target();
    }
    while (0);

    m_stcWarningReference.SetWindowText(textReference);
    m_stcWarningTarget.SetWindowText(textTarget);

    if (!runSuccess)
        return;

    // 결과를 화면에 표시한다.
    SetEdit_Result(vecfError);

    m_imageView->ImageOverlayClear();
    m_imageView->ROIClear();

    // 화면에 계산 결과를 Overlay 해준다.
    if (m_nInspectionType == Insp_Type_Shape_Distance)
    {
        if ((m_pVisionInsp->m_AlignInfoData_Ref.strName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
                || m_pVisionInsp->m_AlignInfoData_Ref.strName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
            && (m_pVisionInsp->m_AlignInfoData_Tar.strName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
                || m_pVisionInsp->m_AlignInfoData_Tar.strName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips]))
        {
            if (m_vecInfoParameter_0_Ref[0][ED_Param_Edge_Detect_Area] == 0)
            {
                Ipvm::LineSeg32r seg;
                seg.m_sx = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_x;
                seg.m_sy = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_y;
                seg.m_ex = m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_x;
                seg.m_ey = m_pVisionInsp->m_AlignInfoData_Tar.CircleData.m_y;

                m_imageView->ImageOverlayAdd(seg, RGB(0, 255, 0));
            }
        }
        else if ((m_pVisionInsp->m_AlignInfoData_Ref.strName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
                     || m_pVisionInsp->m_AlignInfoData_Ref.strName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips]))
        {
            Ipvm::LineSeg32r seg;

            if (m_vecInfoParameter_0_Ref[0][ED_Param_Edge_Detect_Area] == 0)
            {
                seg.m_sx = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_x;
                seg.m_sy = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_y;
            }
            else
            {
                Ipvm::Point32r2 pftStart(m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_x,
                    m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_y);
                auto basePoint = CPI_Geometry::GetLinePoint(pftStart, m_pVisionInsp->m_AlignInfoData_Tar.vecfptData[0],
                    m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius);
                seg.m_sx = basePoint.m_x;
                seg.m_sy = basePoint.m_y;
            }

            if (m_pVisionInsp->m_AlignInfoData_Tar.vecfptData.size())
            {
                seg.m_ex = m_pVisionInsp->m_AlignInfoData_Tar.vecfptData[0].m_x;
                seg.m_ey = m_pVisionInsp->m_AlignInfoData_Tar.vecfptData[0].m_y;
                m_imageView->ImageOverlayAdd(seg, RGB(0, 255, 0));
            }
        }
        else
        {
            if ((long)m_pVisionInsp->m_AlignInfoData_Ref.vecfptData.size()
                && (long)m_pVisionInsp->m_AlignInfoData_Tar.vecfptData.size())
            {
                if ((m_pVisionInsp->m_AlignInfoData_Ref.nDataType == DataTypeList::Line
                        && m_pVisionInsp->m_AlignInfoData_Tar.nDataType == DataTypeList::All_Point)
                    || (m_pVisionInsp->m_AlignInfoData_Ref.nDataType == DataTypeList::All_Point
                        && m_pVisionInsp->m_AlignInfoData_Tar.nDataType
                            == DataTypeList::Line)) // 라인과 여러개의 포인트 거리
                {
                    Ipvm::LineSeg32r seg1(m_pVisionInsp->m_AlignInfoData_Ref.vecfptData[0].m_x,
                        m_pVisionInsp->m_AlignInfoData_Ref.vecfptData[0].m_y,
                        m_pVisionInsp->m_AlignInfoData_Ref.vecfptData[1].m_x,
                        m_pVisionInsp->m_AlignInfoData_Ref.vecfptData[1].m_y);
                    Ipvm::LineSeg32r seg2(m_pVisionInsp->m_AlignInfoData_Tar.vecfptData[0].m_x,
                        m_pVisionInsp->m_AlignInfoData_Tar.vecfptData[0].m_y,
                        m_pVisionInsp->m_AlignInfoData_Tar.vecfptData[1].m_x,
                        m_pVisionInsp->m_AlignInfoData_Tar.vecfptData[1].m_y);

                    m_imageView->ImageOverlayAdd(seg1, RGB(0, 255, 0));
                    m_imageView->ImageOverlayAdd(seg2, RGB(0, 255, 0));
                }
                else
                {
                    Ipvm::LineSeg32r seg(m_pVisionInsp->m_AlignInfoData_Ref.vecfptData[0].m_x,
                        m_pVisionInsp->m_AlignInfoData_Ref.vecfptData[0].m_y,
                        m_pVisionInsp->m_AlignInfoData_Tar.vecfptData[0].m_x,
                        m_pVisionInsp->m_AlignInfoData_Tar.vecfptData[0].m_y);

                    m_imageView->ImageOverlayAdd(seg, RGB(0, 255, 0));
                }
            }
        }
    }
    else if (m_nInspectionType == Insp_Type_Shape_Angle)
    {
        Ipvm::Point32r2 pftCrossPoint;

        Ipvm::Geometry::GetCrossPoint(m_pVisionInsp->m_AlignInfoData_Ref.LineData_F,
            m_pVisionInsp->m_AlignInfoData_Tar.LineData_F, pftCrossPoint);

        Ipvm::LineSeg32r seg1(m_pVisionInsp->m_AlignInfoData_Ref.fptCenterPoint.m_x,
            m_pVisionInsp->m_AlignInfoData_Ref.fptCenterPoint.m_y, pftCrossPoint.m_x, pftCrossPoint.m_y);
        Ipvm::LineSeg32r seg2(m_pVisionInsp->m_AlignInfoData_Tar.fptCenterPoint.m_x,
            m_pVisionInsp->m_AlignInfoData_Tar.fptCenterPoint.m_y, pftCrossPoint.m_x, pftCrossPoint.m_y);

        m_imageView->ImageOverlayAdd(seg1, RGB(0, 255, 0));
        m_imageView->ImageOverlayAdd(seg2, RGB(0, 255, 0));
    }
    else if (m_nInspectionType == Insp_Type_Shape_Circle)
    {
        Ipvm::Rect32r frtROI;
        frtROI.m_left = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_x
            - m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius;
        frtROI.m_right = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_x
            + m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius;
        frtROI.m_top = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_y
            - m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius;
        frtROI.m_bottom = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_y
            + m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius;

        m_imageView->ImageOverlayAdd(frtROI, RGB(0, 255, 0));

        Ipvm::LineSeg32r seg;
        seg.m_sx = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_x;
        seg.m_sy = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_y;

        seg.m_ex = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_x
            - m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_radius;
        seg.m_ey = m_pVisionInsp->m_AlignInfoData_Ref.CircleData.m_y;

        m_imageView->ImageOverlayAdd(seg, RGB(0, 255, 0));
    }

    m_imageView->ImageOverlayShow();

    textReference.Empty();
    textTarget.Empty();
}

void CDlgGeometryEditor::SetEdit_Result(std::vector<float> vecfError)
{
    CString strTemp;
    CString strOut;

    for (long nID = 0; nID < (long)vecfError.size(); nID++)
    {
        strTemp.Format(_T("Value %2d : %.2f\r\n"), nID, vecfError[nID]);
        strOut += strTemp;
    }

    m_editResult.SetWindowText(strOut);

    UpdateData(FALSE);

    strTemp.Empty();
    strOut.Empty();
}

void CDlgGeometryEditor::callRoiChanged(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(keyLength);

    ((CDlgGeometryEditor*)userData)->callRoiChanged();
}

void CDlgGeometryEditor::callRoiChanged()
{
    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    if (m_bChangedROI == FALSE)
        return;

    if (m_nChangedInspIID < 0)
        return;

    std::vector<Ipvm::Rect32r>* pvecrtInspROI_BCU = NULL;

    if (m_nSelectRefAndTar == enum_Select_Reference)
    {
        pvecrtInspROI_BCU = &m_vecrtInspectionROI_Ref_BCU;
    }
    else if (m_nSelectRefAndTar == enum_Select_Target)
    {
        pvecrtInspROI_BCU = &m_vecrtInspectionROI_Tar_BCU;
    }
    else
    {
        return;
    }

    long nInspTotalCount = (long)(*pvecrtInspROI_BCU).size();
    if (nInspTotalCount <= 0 || nInspTotalCount <= m_nChangedInspIID)
        return;

    double fAngle = 0.;
    if (m_nSelectRefAndTar == enum_Select_Reference)
    {
        if (m_vecInfoParameter_0_Ref[m_nChangedInspIID][ED_Param_Edge_Angle] == 1)
        {
            fAngle = 45.0 * ITP_DEG_TO_RAD;
        }
        else if (m_vecInfoParameter_0_Ref[m_nChangedInspIID][ED_Param_Edge_Angle] == 2)
        {
            fAngle = -45.0 * ITP_DEG_TO_RAD;
        }
    }
    else if (m_nSelectRefAndTar == enum_Select_Target)
    {
        if (m_vecInfoParameter_0_Tar[m_nChangedInspIID][ED_Param_Edge_Angle] == 1)
        {
            fAngle = 45.0 * ITP_DEG_TO_RAD;
        }
        else if (m_vecInfoParameter_0_Tar[m_nChangedInspIID][ED_Param_Edge_Angle] == 2)
        {
            fAngle = -45.0 * ITP_DEG_TO_RAD;
        }
    }
    else
    {
        return;
    }

    Ipvm::Rect32s rtROI;
    if (fAngle == 0.)
    {
        m_imageView->ROIGet(_T(""), rtROI);
    }
    else
    {
        Ipvm::Polygon32r polygon;
        PI_RECT rect;
        if (m_imageView->ROIGet(_T(""), polygon))
        {
            if (polygon.GetVertexNum() != 4)
                return;

            rect.ltX = CAST_INT32T(polygon.GetVertices()[0].m_x + .5f);
            rect.ltY = CAST_INT32T(polygon.GetVertices()[0].m_y + .5f);
            rect.rtX = CAST_INT32T(polygon.GetVertices()[1].m_x + .5f);
            rect.rtY = CAST_INT32T(polygon.GetVertices()[1].m_y + .5f);
            rect.brX = CAST_INT32T(polygon.GetVertices()[2].m_x + .5f);
            rect.brY = CAST_INT32T(polygon.GetVertices()[2].m_y + .5f);
            rect.blX = CAST_INT32T(polygon.GetVertices()[3].m_x + .5f);
            rect.blY = CAST_INT32T(polygon.GetVertices()[3].m_y + .5f);

            rect = rect.Rotate(fAngle);
            rtROI = rect.CreateExternalMinimumRect();
        }
    }

    const auto& scale = m_pVisionInsp->getScale();
    (*pvecrtInspROI_BCU)[m_nChangedInspIID] = scale.convert_PixelToBCU(rtROI, imageCenter);

    return;
}

void CDlgGeometryEditor::ShowOverlay(CString strAlignInfoName, long nInspID, long nSelectID,
    std::vector<std::vector<long>>& vecInfoParameter_0, std::vector<Ipvm::Rect32r>& vecrtInspROI_BCU)
{
    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    m_imageView->ImageOverlayClear();
    m_imageView->ROIClear();

    SAlignInfoDataList AlignInfoList;
    for (long nID = 0; nID < (long)m_pVisionInsp->m_vecAlignInfoList.size(); nID++)
    {
        if (m_pVisionInsp->m_vecAlignInfoList[nID].strName == strAlignInfoName)
        {
            AlignInfoList = m_pVisionInsp->m_vecAlignInfoList[nID];
            break;
        }
    }

    const auto& scale = m_pVisionInsp->getScale();

    // 이동형 ROI를 사용할 때만 TRUE로 바꿔준다.
    m_bChangedROI = FALSE;
    m_nChangedInspIID = -1;

    if (strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_Body_Line]
        || strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_Other_Line])
    {
        long index_s = -1;
        long index_e = -1;
        switch (nSelectID)
        {
            case 0:
                index_s = enum_Direction_LeftTop;
                index_e = enum_Direction_LeftBottom;
                break;
            case 1:
                index_s = enum_Direction_LeftTop;
                index_e = enum_Direction_RightTop;
                break;
            case 2:
                index_s = enum_Direction_RightTop;
                index_e = enum_Direction_RightBottom;
                break;
            case 3:
                index_s = enum_Direction_LeftBottom;
                index_e = enum_Direction_RightBottom;
                break;
            case 4:
                index_s = enum_Direction_LowTop_Left;
                index_e = enum_Direction_LowTop_Right;
                break;
            default:
                break;
        }

        if (index_s >= 0 && index_e >= 0 && index_s < long(AlignInfoList.vecfptData.size())
            && index_e < long(AlignInfoList.vecfptData.size()))
        {
            Ipvm::LineSeg32r seg(AlignInfoList.vecfptData[index_s].m_x, AlignInfoList.vecfptData[index_s].m_y,
                AlignInfoList.vecfptData[index_e].m_x, AlignInfoList.vecfptData[index_e].m_y);

            m_imageView->ImageOverlayAdd(seg, RGB(0, 255, 0));
        }
    }
    else if (strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_User_Edge_Detect]
        || strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_User_Line]
        || strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_User_Round]
        || strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_User_Ellips])
    {
        Ipvm::Rect32s rtROI = scale.convert_BCUToPixel(vecrtInspROI_BCU[nInspID], imageCenter);

        double fAngle = 0.;
        if (vecInfoParameter_0[nInspID][ED_Param_Edge_Angle] == 1)
        {
            fAngle = -45.0 * ITP_DEG_TO_RAD;
        }
        else if (vecInfoParameter_0[nInspID][ED_Param_Edge_Angle] == 2)
        {
            fAngle = 45.0 * ITP_DEG_TO_RAD;
        }

        if (fAngle == 0.f)
        {
            m_imageView->ROISet(_T(""), _T(""), rtROI, RGB(255, 0, 0), true, true);
        }
        else
        {
            PI_RECT rectRotate;
            rectRotate = rtROI;
            rectRotate = rectRotate.Rotate(fAngle);

            Ipvm::Polygon32r polygon;
            polygon.SetVertexNum(4);
            polygon.GetVertices()[0].m_x = CAST_FLOAT(rectRotate.ltX);
            polygon.GetVertices()[0].m_y = CAST_FLOAT(rectRotate.ltY);
            polygon.GetVertices()[1].m_x = CAST_FLOAT(rectRotate.rtX);
            polygon.GetVertices()[1].m_y = CAST_FLOAT(rectRotate.rtY);
            polygon.GetVertices()[2].m_x = CAST_FLOAT(rectRotate.brX);
            polygon.GetVertices()[2].m_y = CAST_FLOAT(rectRotate.brY);
            polygon.GetVertices()[3].m_x = CAST_FLOAT(rectRotate.blX);
            polygon.GetVertices()[3].m_y = CAST_FLOAT(rectRotate.blY);

            m_imageView->ROISet(_T(""), _T(""), polygon, RGB(255, 0, 0), true, false);
        }

        m_bChangedROI = TRUE;
        m_nChangedInspIID = nInspID;
    }
    else if (strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_Round_LT_Info]
        || strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_Round_RT_Info]
        || strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_Round_LB_Info]
        || strAlignInfoName == g_szAlignInfo_List_Name[Insp_Type_Round_RB_Info])
    {
        m_imageView->ImageOverlayAdd(AlignInfoList.rtROI, RGB(0, 255, 0));
        m_imageView->ImageOverlayAdd(AlignInfoList.vecfptData[0], RGB(0, 255, 0));
    }

    m_imageView->ImageOverlayShow();
    m_imageView->ROIShow();
}

void CDlgGeometryEditor::OnBnClickedBtnCancel()
{
    CDialog::OnCancel();
}

HBRUSH CDlgGeometryEditor::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd == &m_stcWarningReference || pWnd == &m_stcWarningTarget)
    {
        pDC->SetTextColor(RGB(255, 0, 0));
    }

    return hbr;
}
