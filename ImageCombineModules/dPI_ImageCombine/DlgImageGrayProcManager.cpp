//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgImageGrayProcManager.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"
#include "../dPI_ippModules/ippModules.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum
{
    enumFrame_Begin = 0,
    enumFrame_First = enumFrame_Begin,
    enumFrame_Second,
    enumFrame_End,
};

enum
{
    enumFrame1_Begin = 0,
    enumFrame1_Current = enumFrame1_Begin,
    enumFrame1_Aux,
    enumFrame1_Combine,
    enumFrame1_End,
};

enum
{
    enumFrame2_Begin = 3,
    enumFrame2_FirstCombine = enumFrame2_Begin,
    enumFrame2_Aux,
    enumFrame2_Combine,
    enumFrame2_End,
};

const LPCTSTR gl_szStrFrameNames[] = {
    _T("First Combine"),
    _T("Second Combine"),
};

const LPCTSTR gl_szStrFrame1Names[] = {
    _T("Current Frame"),
    _T("Aux1 Frame"),
    _T("Combine Frame"),
};

const LPCTSTR gl_szStrFrame2Names[] = {
    _T("First Combine Frame"),
    _T("Aux2 Frame"),
    _T("Combine Frame"),
};

const LPCTSTR gl_szStrCombineType[] = {
    _T("Not Thing"), //enumImageCombine_NotThing
    _T("Cur Frame + Aux Frame"), //enumImageCombine_Add
    _T("Cur Frame - Aux Frame"), //enumImageCombine_Sub1_2
    _T("Aux Frame - Cur Frame"), //enumImageCombine_Sub2_1
    _T("Cur Frame && Aux Frame"), // enumImageCombine_And
    _T("Cur Frame || Aux Frame"), //enumImageCombine_Or
    _T("Cur Frame + Aux Frame Not Sat."), //enumImageCombine_Add_NS //kircheis_PLIT 091126
    _T("Cur Frame - Aux Frame Not Sat."), //enumImageCombine_Sub1_2_NS
    _T("Aux Frame - Cur Frame Not Sat."), //enumImageCombine_Sub2_1_NS
    _T("Cur Frame / Aux Frame Not Sat."), //enumImageCombine_Div_NS
    _T("Cur Frame * Aux Frame Not Sat."), //enumImageCombine_Mul_NS
    _T("Max Inten.(Cur Frame, Aux Frame)"), //enumImageCombine_MAX//kircheis_DieCrack
    _T("Min Inten.(Cur Frame, Aux Frame)"), //enumImageCombine_MIN//kircheis_DieCrack
};

const LPCTSTR gl_szStrAlgoName[] = {
    _T("Median Filter"), //enumImageProc_Median
    _T("Low Pass Filter"), //enumImageProc_LowPass
    _T("Shapen"), //enumImageProc_Sharpen
    _T("Invert"), //enumImageProc_Invert
    _T("Histogram Equalization"), //enumImageProc_HistogramEqualization
    _T("Morphological Dilate"), //enumImageProc_MorphologicalDilate
    _T("Morphological Erode"), //enumImageProc_MorphologicalErode
    _T("Add Self Frame"), //enumImageProc_AddSelf
    _T("Add Result Frame"), //enumImageProc_AddResult
    _T("Image Offset"), //enumImageProc_Offset
    _T("Image Contrast"), //enumImageProc_Contrast
};

IMPLEMENT_DYNAMIC(CDlgImageGrayProcManager, CDialog)

CDlgImageGrayProcManager::CDlgImageGrayProcManager(
    VisionProcessing& proc, bool isRaw, long baseFrameIndex, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgImageGrayProcManager::IDD, pParent)
    , m_proc(proc)
    , m_isRaw(isRaw)
    , m_nFrame(0)
    , m_bImagePaneView(TRUE)
    , m_imageView(nullptr)
    , m_ProcManagePara(proc)
    , m_baseFrameIndex(baseFrameIndex)
{
    m_imageFrameCount = m_proc.getImageLot().GetImageFrameCount(m_proc.GetCurVisionModule_Status());
}

CDlgImageGrayProcManager::~CDlgImageGrayProcManager()
{
    delete m_imageView;
}

void CDlgImageGrayProcManager::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_COMBINE_MODE_GLOBAL, m_cmbCombineMode);
    DDX_Control(pDX, IDC_COMBO_COMBINE_AUX_FRAME_GLOBAL, m_cmbAuxFrame);
    DDX_Control(pDX, IDC_TAB_FRAME, m_tabMainFrame);
    DDX_Control(pDX, IDC_TAB_ALGORITHM_GLOBAL, m_tabSubFrame);
    DDX_Control(pDX, IDC_LIST_ALGORITHM_GLOBAL, m_lstAlgorithm);
    DDX_Control(pDX, IDC_GRID_APPLY_GLOBAL, m_gridApply);
    DDX_Control(pDX, IDC_STATIC_AUX, m_staticAuxCmbName);
}

BEGIN_MESSAGE_MAP(CDlgImageGrayProcManager, CDialog)
ON_BN_CLICKED(IDC_BUTTON_UP_GLOBAL, &CDlgImageGrayProcManager::OnBnClickedButtonUp)
ON_BN_CLICKED(IDC_BUTTON_DOWN_GLOBAL, &CDlgImageGrayProcManager::OnBnClickedButtonDown)
ON_CBN_SELCHANGE(IDC_COMBO_COMBINE_MODE_GLOBAL, &CDlgImageGrayProcManager::OnCbnSelchangeComboCombineMode)
ON_CBN_SELCHANGE(IDC_COMBO_COMBINE_AUX_FRAME_GLOBAL, &CDlgImageGrayProcManager::OnCbnSelchangeComboCombineAuxFrame)
ON_LBN_DBLCLK(IDC_LIST_ALGORITHM_GLOBAL, &CDlgImageGrayProcManager::OnLbnDblclkListAlgorithm)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_ALGORITHM_GLOBAL, &CDlgImageGrayProcManager::OnTcnSelchangeTabAlgorithm)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_FRAME, &CDlgImageGrayProcManager::OnTcnSelchangeTabFrame)
ON_NOTIFY(NM_DBLCLK, IDC_GRID_APPLY_GLOBAL, OnGridDbClick)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_APPLY_GLOBAL, OnGridEditEnd)
END_MESSAGE_MAP()

// CDlgImageGrayProcManager 메시지 처리기입니다.

BOOL CDlgImageGrayProcManager::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect view;
    GetDlgItem(IDC_COMBINE_PREVIEW_GLOBAL)->GetWindowRect(view);
    ScreenToClient(view);

    m_imageView = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(view), 0, m_proc.GetCurVisionModule_Status());
    m_gridApply.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridApply.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridApply.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridApply.SetDefCellMargin(0);
    m_gridApply.SetFixedColumnSelection(FALSE);
    m_gridApply.SetFixedRowSelection(FALSE);
    m_gridApply.SetSingleColSelection(FALSE);
    m_gridApply.SetSingleRowSelection(FALSE);
    m_gridApply.SetTrackFocusCell(FALSE);
    m_gridApply.SetRowResize(FALSE);
    m_gridApply.SetColumnResize(FALSE);

    auto& illumInfo2D = m_proc.getIllumInfo2D();

    static_assert(
        (sizeof(gl_szStrCombineType) / sizeof(LPCTSTR)) == enumImageCombine_End, "Combine mode string count mismatch");

    int nIdx;

    for (nIdx = enumImageCombine_Begin; nIdx < enumImageCombine_End; nIdx++)
        m_cmbCombineMode.AddString(gl_szStrCombineType[nIdx]);

    //CJobFile &JobFile = ::GetCurJobFile();
    for (long frameIndex = 0; frameIndex < m_imageFrameCount; frameIndex++)
    {
        m_cmbAuxFrame.AddString(illumInfo2D.getIllumFullName(frameIndex));
    }

    for (nIdx = enumFrame_Begin; nIdx < enumFrame_End; nIdx++)
        m_tabMainFrame.InsertItem(nIdx, gl_szStrFrameNames[nIdx]);

    for (nIdx = enumImageProc_Begin; nIdx < enumImageProc_End; nIdx++)
        m_lstAlgorithm.AddString(gl_szStrAlgoName[nIdx]);

    m_tabMainFrame.SetCurSel(enumFrame_First);

    Initialize();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CDlgImageGrayProcManager::Initialize()
{
    m_tabSubFrame.DeleteAllItems();

    int nIdx;

    if (m_tabMainFrame.GetCurSel() == enumFrame_First)
    {
        m_staticAuxCmbName.SetWindowTextW(_T("Combine Aux1 Frame :"));
        m_cmbCombineMode.SetCurSel(m_ProcManagePara.nFirstCombineType);
        m_cmbAuxFrame.SetCurSel(m_ProcManagePara.m_aux1FrameID.getFrameIndex());
        m_cmbAuxFrame.EnableWindow(TRUE);

        for (nIdx = enumFrame1_Begin; nIdx < enumFrame1_End; nIdx++)
            m_tabSubFrame.InsertItem(nIdx, gl_szStrFrame1Names[nIdx]);

        m_tabSubFrame.SetCurSel(enumFrame1_Current);
        UpdateApplyList(enumFrame1_Current);
    }
    else
    {
        m_staticAuxCmbName.SetWindowTextW(_T("Combine Aux2 Frame :"));
        m_cmbCombineMode.SetCurSel(m_ProcManagePara.nSecondCombineType);
        m_cmbAuxFrame.SetCurSel(m_ProcManagePara.m_aux2FrameID.getFrameIndex());
        m_cmbAuxFrame.EnableWindow(TRUE);

        for (nIdx = enumFrame2_Begin - 3; nIdx < enumFrame2_End - 3; nIdx++)
            m_tabSubFrame.InsertItem(nIdx, gl_szStrFrame2Names[nIdx]);

        m_tabSubFrame.SetCurSel(enumFrame2_FirstCombine - 3);
        UpdateApplyList(enumFrame2_FirstCombine);
    }

    if (m_imageFrameCount <= 1)
    {
        m_cmbCombineMode.EnableWindow(FALSE);
        m_cmbCombineMode.SetCurSel(enumImageCombine_NotThing);
        m_cmbAuxFrame.EnableWindow(FALSE);
        m_tabMainFrame.EnableWindow(FALSE);
        m_tabSubFrame.EnableWindow(FALSE);
    }

    return TRUE;
}

void CDlgImageGrayProcManager::SetParameter(ImageProcPara* pProcManagePara, Ipvm::Rect32s rtPaneROI)
{
    m_rtPaneROI = rtPaneROI;

    m_pProcManagePara_ = pProcManagePara;
    m_ProcManagePara = *pProcManagePara;

    Ipvm::Image8u image;
    if (m_isRaw)
    {
        if (m_baseFrameIndex >= 0
            && m_baseFrameIndex < m_proc.getImageLot().GetImageFrameCount(m_proc.GetCurVisionModule_Status()))
        {
            image = m_proc.getImageLot().GetImageFrame(m_baseFrameIndex, m_proc.GetCurVisionModule_Status());
        }
    }
    else
    {
        image = m_proc.GetInspectionFrameImage(FALSE, m_baseFrameIndex);
    }

    m_nImageSizeX = image.GetSizeX();
    m_nImageSizeY = image.GetSizeY();
    m_rtROI = CRect(0, 0, m_nImageSizeX, m_nImageSizeY);
}

void CDlgImageGrayProcManager::UpdateApplyList(int nFrame)
{
    std::vector<short> vecnApplyProc;
    std::vector<short> vecnApplyParams;

    switch (nFrame)
    {
        case enumFrame1_Current: // Cur Frame
            vecnApplyProc = m_ProcManagePara.vecnCurProc;
            vecnApplyParams = m_ProcManagePara.vecnCurProcParams;
            break;
        case enumFrame1_Aux:
            vecnApplyProc = m_ProcManagePara.vecnAux1Proc;
            vecnApplyParams = m_ProcManagePara.vecnAux1ProcParams;
            break;
        case enumFrame2_Aux:
            vecnApplyProc = m_ProcManagePara.vecnAux2Proc;
            vecnApplyParams = m_ProcManagePara.vecnAux2ProcParams;
            break;
        case enumFrame2_FirstCombine:
        case enumFrame1_Combine:
            vecnApplyProc = m_ProcManagePara.vecnFirstCombineProc;
            vecnApplyParams = m_ProcManagePara.vecnFirstCombineProcParams;
            break;
        case enumFrame2_Combine:
            vecnApplyProc = m_ProcManagePara.vecnSecondCombineProc;
            vecnApplyParams = m_ProcManagePara.vecnSecondCombineProcParams;
            break;
    }

    DrawPreViewImage(nFrame, vecnApplyProc, vecnApplyParams); //영훈_ImageCombine_20101221

    int nProcNum = (long)vecnApplyProc.size();
    COLORREF crNameCellBK = RGB(100, 100, 100);
    m_gridApply.DeleteAllItems();
    m_gridApply.SetRowCount(nProcNum + 1); // Items + Header//kircheis_Hy
    m_gridApply.SetFixedRowCount(1);
    m_gridApply.SetColumnCount(2);
    m_gridApply.SetColumnWidth(0, 153);
    m_gridApply.SetColumnWidth(1, 60);
    m_gridApply.SetItemBkColour(0, 0, crNameCellBK);
    m_gridApply.SetItemState(0, 0, GVIS_READONLY);
    m_gridApply.SetItemText(0, 0, _T("Process List"));
    m_gridApply.SetItemBkColour(0, 1, crNameCellBK);
    m_gridApply.SetItemState(0, 1, GVIS_READONLY);
    m_gridApply.SetItemText(0, 1, _T("Param"));

    for (int nIdx = 0; nIdx < nProcNum; nIdx++)
    {
        m_gridApply.SetItemText(nIdx + 1, 0, gl_szStrAlgoName[vecnApplyProc[nIdx]]);
        m_gridApply.SetItemState(nIdx + 1, 0, GVIS_READONLY);
        if (vecnApplyProc[nIdx] == enumImageProc_Offset || vecnApplyProc[nIdx] == enumImageProc_Contrast)
        {
            m_gridApply.SetItemTextFmt(nIdx + 1, 1, _T("%d"), vecnApplyParams[nIdx]);
        }
        else
        {
            m_gridApply.SetItemState(nIdx + 1, 0, GVIS_READONLY);
            m_gridApply.SetItemBkColour(nIdx + 1, 1, crNameCellBK);
        }
    }

    if (nFrame == enumFrame2_FirstCombine)
    {
        m_lstAlgorithm.EnableWindow(FALSE);
        m_gridApply.EnableWindow(FALSE);
    }
    else
    {
        m_lstAlgorithm.EnableWindow(TRUE);
        m_gridApply.EnableWindow(TRUE);
    }

    m_gridApply.Refresh();
}

void CDlgImageGrayProcManager::OnOK()
{
    // TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
    UpdateData();
    if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
    {
        m_ProcManagePara.nSecondCombineType = (short)m_cmbCombineMode.GetCurSel();
        m_ProcManagePara.m_aux2FrameID.setFrameIndex((short)m_cmbAuxFrame.GetCurSel());
    }
    else
    {
        m_ProcManagePara.nFirstCombineType = (short)m_cmbCombineMode.GetCurSel();
        m_ProcManagePara.m_aux1FrameID.setFrameIndex((short)m_cmbAuxFrame.GetCurSel());
    }

    //CJobFile &JobFile = ::GetCurJobFile();
    if (m_ProcManagePara.nFirstCombineType < enumImageCombine_Begin
        || m_ProcManagePara.nFirstCombineType >= enumImageCombine_End)
        m_ProcManagePara.nFirstCombineType = enumImageCombine_Begin;
    if (m_ProcManagePara.nSecondCombineType < enumImageCombine_Begin
        || m_ProcManagePara.nSecondCombineType >= enumImageCombine_End)
        m_ProcManagePara.nSecondCombineType = enumImageCombine_Begin;

    if (!m_ProcManagePara.m_aux1FrameID.isValid())
    {
        m_ProcManagePara.m_aux1FrameID.setFrameIndex(0);
    }

    if (!m_ProcManagePara.m_aux2FrameID.isValid())
    {
        m_ProcManagePara.m_aux2FrameID.setFrameIndex(0);
    }

    *m_pProcManagePara_ = m_ProcManagePara;

    CDialog::OnOK();
}

void CDlgImageGrayProcManager::OnBnClickedButtonUp()
{
    short nCurSel = (short)GetSelectedGridCellRow();

    if (nCurSel <= 0)
        return;

    int nFrame = m_tabSubFrame.GetCurSel();

    if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
    {
        nFrame += 3;
    }

    switch (nFrame)
    {
        case enumFrame1_Current: // Cur Frame
            if (nCurSel < m_ProcManagePara.vecnCurProc.size())
            {
                short nBuf = m_ProcManagePara.vecnCurProc[nCurSel];
                m_ProcManagePara.vecnCurProc[nCurSel] = m_ProcManagePara.vecnCurProc[nCurSel - 1];
                m_ProcManagePara.vecnCurProc[nCurSel - 1] = nBuf;

                nBuf = m_ProcManagePara.vecnCurProcParams[nCurSel];
                m_ProcManagePara.vecnCurProcParams[nCurSel] = m_ProcManagePara.vecnCurProcParams[nCurSel - 1];
                m_ProcManagePara.vecnCurProcParams[nCurSel - 1] = nBuf;
            }
            break;
        case enumFrame1_Aux:
            if (nCurSel < m_ProcManagePara.vecnAux1Proc.size())
            {
                short nBuf = m_ProcManagePara.vecnAux1Proc[nCurSel];
                m_ProcManagePara.vecnAux1Proc[nCurSel] = m_ProcManagePara.vecnAux1Proc[nCurSel - 1];
                m_ProcManagePara.vecnAux1Proc[nCurSel - 1] = nBuf;

                nBuf = m_ProcManagePara.vecnAux1ProcParams[nCurSel];
                m_ProcManagePara.vecnAux1ProcParams[nCurSel] = m_ProcManagePara.vecnAux1ProcParams[nCurSel - 1];
                m_ProcManagePara.vecnAux1ProcParams[nCurSel - 1] = nBuf;
            }
            break;
        case enumFrame2_Aux:
            if (nCurSel < m_ProcManagePara.vecnAux2Proc.size())
            {
                short nBuf = m_ProcManagePara.vecnAux2Proc[nCurSel];
                m_ProcManagePara.vecnAux2Proc[nCurSel] = m_ProcManagePara.vecnAux2Proc[nCurSel - 1];
                m_ProcManagePara.vecnAux2Proc[nCurSel - 1] = nBuf;

                nBuf = m_ProcManagePara.vecnAux2ProcParams[nCurSel];
                m_ProcManagePara.vecnAux2ProcParams[nCurSel] = m_ProcManagePara.vecnAux2ProcParams[nCurSel - 1];
                m_ProcManagePara.vecnAux2ProcParams[nCurSel - 1] = nBuf;
            }
            break;
        case enumFrame2_FirstCombine:
        case enumFrame1_Combine:
            if (nCurSel < m_ProcManagePara.vecnFirstCombineProc.size())
            {
                short nBuf = m_ProcManagePara.vecnFirstCombineProc[nCurSel];
                m_ProcManagePara.vecnFirstCombineProc[nCurSel] = m_ProcManagePara.vecnFirstCombineProc[nCurSel - 1];
                m_ProcManagePara.vecnFirstCombineProc[nCurSel - 1] = nBuf;

                nBuf = m_ProcManagePara.vecnFirstCombineProcParams[nCurSel];
                m_ProcManagePara.vecnFirstCombineProcParams[nCurSel]
                    = m_ProcManagePara.vecnFirstCombineProcParams[nCurSel - 1];
                m_ProcManagePara.vecnFirstCombineProcParams[nCurSel - 1] = nBuf;
            }
            break;
        case enumFrame2_Combine:
            if (nCurSel < m_ProcManagePara.vecnSecondCombineProc.size())
            {
                short nBuf = m_ProcManagePara.vecnSecondCombineProc[nCurSel];
                m_ProcManagePara.vecnSecondCombineProc[nCurSel] = m_ProcManagePara.vecnSecondCombineProc[nCurSel - 1];
                m_ProcManagePara.vecnSecondCombineProc[nCurSel - 1] = nBuf;

                nBuf = m_ProcManagePara.vecnSecondCombineProcParams[nCurSel];
                m_ProcManagePara.vecnSecondCombineProcParams[nCurSel]
                    = m_ProcManagePara.vecnSecondCombineProcParams[nCurSel - 1];
                m_ProcManagePara.vecnSecondCombineProcParams[nCurSel - 1] = nBuf;
            }
            break;
    }

    UpdateData(FALSE);
    UpdateApplyList(nFrame);
}

void CDlgImageGrayProcManager::OnBnClickedButtonDown()
{
    short nCurSel = (short)GetSelectedGridCellRow();
    if (nCurSel < 0)
        return;

    int nFrame = m_tabSubFrame.GetCurSel();

    if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
    {
        nFrame += 3;
    }

    switch (nFrame)
    {
        case enumFrame1_Current: // Cur Frame
            if (nCurSel < m_ProcManagePara.vecnCurProc.size() - 1)
            {
                short nBuf = m_ProcManagePara.vecnCurProc[nCurSel];
                m_ProcManagePara.vecnCurProc[nCurSel] = m_ProcManagePara.vecnCurProc[nCurSel + 1];
                m_ProcManagePara.vecnCurProc[nCurSel + 1] = nBuf;

                nBuf = m_ProcManagePara.vecnCurProcParams[nCurSel];
                m_ProcManagePara.vecnCurProcParams[nCurSel] = m_ProcManagePara.vecnCurProcParams[nCurSel + 1];
                m_ProcManagePara.vecnCurProcParams[nCurSel + 1] = nBuf;
            }
            break;
        case enumFrame1_Aux:
            if (nCurSel < m_ProcManagePara.vecnAux1Proc.size() - 1)
            {
                short nBuf = m_ProcManagePara.vecnAux1Proc[nCurSel];
                m_ProcManagePara.vecnAux1Proc[nCurSel] = m_ProcManagePara.vecnAux1Proc[nCurSel + 1];
                m_ProcManagePara.vecnAux1Proc[nCurSel + 1] = nBuf;

                nBuf = m_ProcManagePara.vecnAux1ProcParams[nCurSel];
                m_ProcManagePara.vecnAux1ProcParams[nCurSel] = m_ProcManagePara.vecnAux1ProcParams[nCurSel + 1];
                m_ProcManagePara.vecnAux1ProcParams[nCurSel + 1] = nBuf;
            }
            break;
        case enumFrame2_Aux:
            if (nCurSel < m_ProcManagePara.vecnAux2Proc.size() - 1)
            {
                short nBuf = m_ProcManagePara.vecnAux2Proc[nCurSel];
                m_ProcManagePara.vecnAux2Proc[nCurSel] = m_ProcManagePara.vecnAux2Proc[nCurSel + 1];
                m_ProcManagePara.vecnAux2Proc[nCurSel + 1] = nBuf;

                nBuf = m_ProcManagePara.vecnAux2ProcParams[nCurSel];
                m_ProcManagePara.vecnAux2ProcParams[nCurSel] = m_ProcManagePara.vecnAux2ProcParams[nCurSel + 1];
                m_ProcManagePara.vecnAux2ProcParams[nCurSel + 1] = nBuf;
            }
            break;
        case enumFrame2_FirstCombine:
        case enumFrame1_Combine:
            if (nCurSel < m_ProcManagePara.vecnFirstCombineProc.size() - 1)
            {
                short nBuf = m_ProcManagePara.vecnFirstCombineProc[nCurSel];
                m_ProcManagePara.vecnFirstCombineProc[nCurSel] = m_ProcManagePara.vecnFirstCombineProc[nCurSel + 1];
                m_ProcManagePara.vecnFirstCombineProc[nCurSel + 1] = nBuf;

                nBuf = m_ProcManagePara.vecnFirstCombineProcParams[nCurSel];
                m_ProcManagePara.vecnFirstCombineProcParams[nCurSel]
                    = m_ProcManagePara.vecnFirstCombineProcParams[nCurSel + 1];
                m_ProcManagePara.vecnFirstCombineProcParams[nCurSel + 1] = nBuf;
            }
            break;
        case enumFrame2_Combine:
            if (nCurSel < m_ProcManagePara.vecnSecondCombineProc.size() - 1)
            {
                short nBuf = m_ProcManagePara.vecnSecondCombineProc[nCurSel];
                m_ProcManagePara.vecnSecondCombineProc[nCurSel] = m_ProcManagePara.vecnSecondCombineProc[nCurSel + 1];
                m_ProcManagePara.vecnSecondCombineProc[nCurSel + 1] = nBuf;

                nBuf = m_ProcManagePara.vecnSecondCombineProcParams[nCurSel];
                m_ProcManagePara.vecnSecondCombineProcParams[nCurSel]
                    = m_ProcManagePara.vecnSecondCombineProcParams[nCurSel + 1];
                m_ProcManagePara.vecnSecondCombineProcParams[nCurSel + 1] = nBuf;
            }
            break;
    }

    UpdateApplyList(nFrame);
}

void CDlgImageGrayProcManager::OnCbnSelchangeComboCombineMode()
{
    UpdateData(TRUE);

    if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
        m_ProcManagePara.nSecondCombineType = (short)m_cmbCombineMode.GetCurSel();
    else
        m_ProcManagePara.nFirstCombineType = (short)m_cmbCombineMode.GetCurSel();
    UpdateApplyList(m_nFrame);

    UpdateData(FALSE);
}

void CDlgImageGrayProcManager::OnCbnSelchangeComboCombineAuxFrame()
{
    UpdateData(TRUE);

    if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
        m_ProcManagePara.m_aux2FrameID.setFrameIndex(m_cmbAuxFrame.GetCurSel());
    else
        m_ProcManagePara.m_aux1FrameID.setFrameIndex(m_cmbAuxFrame.GetCurSel());
    UpdateApplyList(m_nFrame);

    UpdateData(FALSE);
}

void CDlgImageGrayProcManager::OnLbnDblclkListAlgorithm()
{
    short nAlgo = (short)m_lstAlgorithm.GetCurSel();
    short nDefault = 0;
    if (nAlgo >= enumImageProc_Begin && nAlgo < enumImageProc_End)
    {
        int nFrame = m_tabSubFrame.GetCurSel();

        if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
        {
            nFrame += 3;
        }

        switch (nFrame)
        {
            case enumFrame1_Current: // Cur Frame
                m_ProcManagePara.vecnCurProc.push_back(nAlgo);
                m_ProcManagePara.vecnCurProcParams.push_back(nDefault);
                break;
            case enumFrame1_Aux:
                m_ProcManagePara.vecnAux1Proc.push_back(nAlgo);
                m_ProcManagePara.vecnAux1ProcParams.push_back(nDefault);
                break;
            case enumFrame2_Aux:
                m_ProcManagePara.vecnAux2Proc.push_back(nAlgo);
                m_ProcManagePara.vecnAux2ProcParams.push_back(nDefault);
                break;
            case enumFrame2_FirstCombine:
            case enumFrame1_Combine:
                m_ProcManagePara.vecnFirstCombineProc.push_back(nAlgo);
                m_ProcManagePara.vecnFirstCombineProcParams.push_back(nDefault);
                break;
            case enumFrame2_Combine:
                m_ProcManagePara.vecnSecondCombineProc.push_back(nAlgo);
                m_ProcManagePara.vecnSecondCombineProcParams.push_back(nDefault);
                break;
        }

        UpdateApplyList(nFrame);
    }
}

void CDlgImageGrayProcManager::OnTcnSelchangeTabAlgorithm(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;
    if (pGridView == NULL)
        return;

    m_nFrame = m_tabSubFrame.GetCurSel();
    if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
    {
        m_nFrame += 3;
    }
    UpdateApplyList(m_nFrame);
    *pResult = 0;
}

void CDlgImageGrayProcManager::OnTcnSelchangeTabFrame(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;
    if (pGridView == NULL)
        return;

    Initialize();

    m_nFrame = m_tabSubFrame.GetCurSel();
    if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
    {
        m_nFrame += 3;
    }
    UpdateApplyList(m_nFrame);
    *pResult = 0;
}

void CDlgImageGrayProcManager::OnGridDbClick(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    int row = pNotify->iRow;
    int col = pNotify->iColumn;

    if (row >= 1 && col == 0)
    {
        short nCurSel = (short)row - 1;
        int nFrame = m_tabSubFrame.GetCurSel();

        if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
        {
            nFrame += 3;
        }

        switch (nFrame) // 20110325_GridBugCrash
        {
            case enumFrame1_Current: // Cur Frame
                if (nCurSel < m_ProcManagePara.vecnCurProc.size())
                {
                    m_ProcManagePara.vecnCurProc.erase(m_ProcManagePara.vecnCurProc.begin() + nCurSel);
                    m_ProcManagePara.vecnCurProcParams.erase(m_ProcManagePara.vecnCurProcParams.begin() + nCurSel);
                }
                break;
            case enumFrame1_Aux:
                if (nCurSel < m_ProcManagePara.vecnAux1Proc.size())
                {
                    m_ProcManagePara.vecnAux1Proc.erase(m_ProcManagePara.vecnAux1Proc.begin() + nCurSel);
                    m_ProcManagePara.vecnAux1ProcParams.erase(m_ProcManagePara.vecnAux1ProcParams.begin() + nCurSel);
                }
                break;
            case enumFrame2_Aux:
                if (nCurSel < m_ProcManagePara.vecnAux2Proc.size())
                {
                    m_ProcManagePara.vecnAux2Proc.erase(m_ProcManagePara.vecnAux2Proc.begin() + nCurSel);
                    m_ProcManagePara.vecnAux2ProcParams.erase(m_ProcManagePara.vecnAux2ProcParams.begin() + nCurSel);
                }
                break;
            case enumFrame2_FirstCombine:
            case enumFrame1_Combine:
                if (nCurSel < m_ProcManagePara.vecnFirstCombineProc.size())
                {
                    m_ProcManagePara.vecnFirstCombineProc.erase(
                        m_ProcManagePara.vecnFirstCombineProc.begin() + nCurSel);
                    m_ProcManagePara.vecnFirstCombineProcParams.erase(
                        m_ProcManagePara.vecnFirstCombineProcParams.begin() + nCurSel);
                }
                break;
            case enumFrame2_Combine:
                if (nCurSel < m_ProcManagePara.vecnSecondCombineProc.size())
                {
                    m_ProcManagePara.vecnSecondCombineProc.erase(
                        m_ProcManagePara.vecnSecondCombineProc.begin() + nCurSel);
                    m_ProcManagePara.vecnSecondCombineProcParams.erase(
                        m_ProcManagePara.vecnSecondCombineProcParams.begin() + nCurSel);
                }
                break;
        }

        UpdateApplyList(nFrame);
    }

    *result = 0;
}

void CDlgImageGrayProcManager::OnGridEditEnd(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    long row = pNotify->iRow;
    long col = pNotify->iColumn;

    if (row >= 1 && col == 1)
    {
        long nVectorSize = 0;
        int nItem = row - 1;
        short nParam = (short)_ttoi(m_gridApply.GetItemText(row, 1));

        int nFrame = m_tabSubFrame.GetCurSel();
        if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
        {
            nFrame += 3;
        }
        switch (nFrame)
        {
            case enumFrame1_Current: // Cur Frame
            {
                nVectorSize = (long)(m_ProcManagePara.vecnCurProcParams.size());
                /*그리드 버그로 인해 그리드 ROW값이 벡터보다 같거나 작아야 적용하도록 한다. */
                if (nVectorSize > nItem)
                {
                    m_ProcManagePara.vecnCurProcParams[nItem] = nParam;
                }
            }
            break;
            case enumFrame1_Aux:
            {
                nVectorSize = (long)(m_ProcManagePara.vecnAux1ProcParams.size());

                if (nVectorSize > nItem)
                {
                    m_ProcManagePara.vecnAux1ProcParams[nItem] = nParam;
                }
            }
            break;
            case enumFrame2_Aux:
            {
                nVectorSize = (long)(m_ProcManagePara.vecnAux2ProcParams.size());

                if (nVectorSize > nItem)
                {
                    m_ProcManagePara.vecnAux2ProcParams[nItem] = nParam;
                }
            }
            break;
            case enumFrame2_FirstCombine:
            case enumFrame1_Combine:
            {
                nVectorSize = (long)(m_ProcManagePara.vecnFirstCombineProcParams.size());

                if (nVectorSize > nItem)
                {
                    m_ProcManagePara.vecnFirstCombineProcParams[nItem] = nParam;
                }
            }
            break;
            case enumFrame2_Combine:
            {
                nVectorSize = (long)(m_ProcManagePara.vecnSecondCombineProcParams.size());

                if (nVectorSize > nItem)
                {
                    m_ProcManagePara.vecnSecondCombineProcParams[nItem] = nParam;
                }
            }
            break;
        }
    }

    UpdateApplyList(m_nFrame); // 영훈_ImageCombine_20101221

    *result = 0;
}

long CDlgImageGrayProcManager::GetSelectedGridCellRow()
{
    CCellRange CellRange = m_gridApply.GetSelectedCellRange();

    if (!CellRange.IsValid())
        return -1;

    if (CellRange.GetMinRow() != CellRange.GetMaxRow())
        return -1;

    long nSelItem = CellRange.GetTopLeft().row - 1;

    return nSelItem;
}

//영훈_ImageCombine_20101221
void CDlgImageGrayProcManager::DrawPreViewImage(
    long nFrame, std::vector<short> vecnApplyProc, std::vector<short> vecnApplyParams)
{
    auto& reusableMemory = m_proc.getReusableMemory();

    if (m_proc.GetImageFrameCount())
    {
        Ipvm::Image8u image;
        if (m_isRaw)
        {
            if (m_baseFrameIndex >= 0
                && m_baseFrameIndex < m_proc.getImageLot().GetImageFrameCount(m_proc.GetCurVisionModule_Status()))
            {
                image = m_proc.getImageLot().GetImageFrame(m_baseFrameIndex, m_proc.GetCurVisionModule_Status());
            }
        }
        else
        {
            image = m_proc.GetInspectionFrameImage(FALSE, m_baseFrameIndex);
        }

        if (m_tabMainFrame.GetCurSel() == enumFrame_Second)
        {
            if (m_imageFrameCount <= m_ProcManagePara.m_aux2FrameID.getFrameIndex())
            {
                m_cmbAuxFrame.SetCurSel(0);
                m_ProcManagePara.m_aux2FrameID.setFrameIndex(0);
            }

            m_ProcManagePara.nSecondCombineType = (short)m_cmbCombineMode.GetCurSel(); //Combine Type을 얻기
            m_ProcManagePara.m_aux2FrameID.setFrameIndex(m_cmbAuxFrame.GetCurSel()); //AUX FrameID를 얻기
        }
        else
        {
            if (m_imageFrameCount <= m_ProcManagePara.m_aux1FrameID.getFrameIndex())
            {
                m_cmbAuxFrame.SetCurSel(0);
                m_ProcManagePara.m_aux1FrameID.setFrameIndex(0);
            }

            m_ProcManagePara.nFirstCombineType = (short)m_cmbCombineMode.GetCurSel(); //Combine Type을 얻기
            m_ProcManagePara.m_aux1FrameID.setFrameIndex(m_cmbAuxFrame.GetCurSel()); //AUX FrameID를 얻기
        }

        Ipvm::Image8u aux1Image = m_ProcManagePara.m_aux1FrameID.getImage(m_isRaw);
        Ipvm::Image8u aux2Image = m_ProcManagePara.m_aux2FrameID.getImage(m_isRaw);

        Ipvm::Image8u rawImage; // SDY isRaw 옵션이 활성화 되어있을 때를 위한 이미지 추가 생성
        Ipvm::Image8u combineImage;

        // SDY isRaw 옵션이 켜져 있을때에도 기존 stitching 된 이미지가 들어와 raw 이미지 받아서 연산하도록 수정
        if (m_isRaw)
        {
            rawImage = m_proc.getImageLot().GetImageFrame(m_baseFrameIndex, m_proc.GetCurVisionModule_Status());
            if (!reusableMemory.GetByteImage(combineImage, rawImage.GetSizeX(), rawImage.GetSizeY()))
                return;
        }
        else
        {
            if (!reusableMemory.GetInspByteImage(combineImage))
                return;
        }
        switch (nFrame) //현재 선택된 탭에 대해서
        {
            case enumFrame1_Current: // Cur Tab
                CippModules::SingleImageProcessingManage(reusableMemory, image,
                    Ipvm::Rect32s(0, 0, m_nImageSizeX, m_nImageSizeY), m_ProcManagePara, combineImage,
                    enumFrame1_Current);
                break;
            case enumFrame1_Aux: // Aux Tab
                CippModules::SingleImageProcessingManage(reusableMemory, aux1Image,
                    Ipvm::Rect32s(0, 0, m_nImageSizeX, m_nImageSizeY), m_ProcManagePara, combineImage, enumFrame1_Aux);
                break;
            case enumFrame2_Aux: // Aux Tab
                CippModules::SingleImageProcessingManage(reusableMemory, aux2Image,
                    Ipvm::Rect32s(0, 0, m_nImageSizeX, m_nImageSizeY), m_ProcManagePara, combineImage, enumFrame2_Aux);
                break;
            case enumFrame2_FirstCombine:
            case enumFrame1_Combine: // Combine Tab
                CippModules::GrayImageProcessingManage(reusableMemory, &image, m_isRaw,
                    Ipvm::Rect32s(0, 0, m_nImageSizeX, m_nImageSizeY), m_ProcManagePara, combineImage, FALSE);
                break;
            case enumFrame2_Combine: // Combine Tab
                CippModules::GrayImageProcessingManage(reusableMemory, &image, m_isRaw,
                    Ipvm::Rect32s(0, 0, m_nImageSizeX, m_nImageSizeY), m_ProcManagePara, combineImage, TRUE);
                break;
        }

        m_imageView->SetImage(combineImage);

        if (m_bImagePaneView)
        {
            m_imageView->NavigateTo(m_rtPaneROI); //전체 이미지중 실제 화면에 보여줄 영역(Pane Rect)
            m_bImagePaneView = FALSE; //처음만 Pane 영역을 보여주고 다음엔 수정한 영역을 보여주기 위해
        }
    }
}
