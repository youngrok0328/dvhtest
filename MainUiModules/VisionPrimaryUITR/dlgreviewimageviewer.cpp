//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgReviewImageViewer.h"

//CPP_2_________________________________ This project's headers
#include "DlgImageVIewer.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum
{
    LEVEL_DATE = 0,
    LEVEL_JOB_NAME,
    LEVEL_LOT_ID,
    LEVEL_FILE_NAME,
    LEVEL_ALL,
};

IMPLEMENT_DYNAMIC(CDlgReviewImageViewer, CDialog)

CDlgReviewImageViewer::CDlgReviewImageViewer(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgReviewImageViewer::IDD, pParent)
{
    m_pDlgImageViewer = new CDlgImageVIewer;
}

CDlgReviewImageViewer::~CDlgReviewImageViewer()
{
    m_pDlgImageViewer->DestroyWindow();
    delete m_pDlgImageViewer;
}

void CDlgReviewImageViewer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_TAB, m_ctrlTab);
    DDX_Control(pDX, IDC_RICHEDIT, m_reTextResult);
    DDX_Control(pDX, IDC_CMB_DATE, m_cmbDateList);
    DDX_Control(pDX, IDC_CMB_JOB_NAME, m_cmbJobNameList);
    DDX_Control(pDX, IDC_CMB_LOTID, m_cmbLotIDList);
    DDX_Control(pDX, IDC_CMB_FILE_NAME, m_cmbFileNameList);
}

BEGIN_MESSAGE_MAP(CDlgReviewImageViewer, CDialog)
ON_BN_CLICKED(IDOK, &CDlgReviewImageViewer::OnBnClickedOk)
ON_BN_CLICKED(ID_BTN_REFRESH, &CDlgReviewImageViewer::OnBnClickedBtnRefresh)
ON_CBN_SELCHANGE(IDC_CMB_DATE, &CDlgReviewImageViewer::OnCbnSelchangeCmbDate)
ON_CBN_SELCHANGE(IDC_CMB_JOB_NAME, &CDlgReviewImageViewer::OnCbnSelchangeCmbJobName)
ON_CBN_SELCHANGE(IDC_CMB_LOTID, &CDlgReviewImageViewer::OnCbnSelchangeCmbLotid)
ON_CBN_SELCHANGE(IDC_CMB_FILE_NAME, &CDlgReviewImageViewer::OnCbnSelchangeCmbFileName)
ON_BN_CLICKED(ID_BTN_SEARCH_FILE, &CDlgReviewImageViewer::OnBnClickedBtnSearchFile)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CDlgReviewImageViewer::OnTcnSelchangeTab)
END_MESSAGE_MAP()

// CDlgReviewImageViewer 메시지 처리기입니다.
BOOL CDlgReviewImageViewer::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rect;
    GetClientRect(&rect);

    m_pDlgImageViewer->Create(IDD_IMAGEVIEWER, this);
    m_pDlgImageViewer->SetParent(this);
    m_pDlgImageViewer->MoveWindow(rect.left + 12, rect.top + 30, rect.Width() - 24, 750);
    m_pDlgImageViewer->ShowWindow(SW_SHOW);

    m_reTextResult.MoveWindow(rect.left + 12, rect.top + 30, rect.Width() - 24, 750);
    m_reTextResult.ShowWindow(SW_HIDE);

    CHARFORMAT cf;
    m_reTextResult.GetDefaultCharFormat(cf);
    cf.dwMask = CFM_FACE;
    _tcscpy_s(cf.szFaceName, 20, _T("FixedSys"));
    m_reTextResult.SetDefaultCharFormat(cf);

    m_bInitCheck = FALSE;

    OnBnClickedBtnRefresh();

    ((CEdit*)GetDlgItem(IDC_EDIT_SEARCH))->SetWindowText(_T("Find File..."));

    m_ctrlTab.DeleteAllItems();
    m_ctrlTab.InsertItem(0, _T("Image View"));
    m_ctrlTab.InsertItem(1, _T("Text Log"));

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgReviewImageViewer::OnBnClickedOk()
{
    OnOK();
}

void CDlgReviewImageViewer::OnBnClickedBtnRefresh()
{
    if (GetAsyncKeyState(VK_SHIFT))
    {
        m_bInitCheck = FALSE;
    }

    const CString strRealReviewImagePath = SystemConfig::GetInstance().m_strSaveDrive + _T("Review Image\\");
    CString strReviewImagePath = strRealReviewImagePath;
    long nFileSearchLevel;
    if (m_bInitCheck == FALSE)
    {
        // 파일 트리를 작성.
        nFileSearchLevel = LEVEL_DATE;
        FileSearch(strReviewImagePath, nFileSearchLevel);

        strReviewImagePath = strRealReviewImagePath;
        m_bInitCheck = TRUE;
    }
    else
    {
        if (VerifyVectorSize(LEVEL_LOT_ID) == FALSE)
            return;

        strReviewImagePath += m_vecstrDate[m_nSelectDate] + _T("\\") + m_vecstrJobName[m_nSelectJobName] + _T("\\")
            + m_vecstrLotID[m_nSelectLotID];

        // 파일 트리를 작성.
        nFileSearchLevel = LEVEL_FILE_NAME;
        FileSearch(strReviewImagePath, nFileSearchLevel);
    }

    // 트리에 맞춰 콤보박스 컨트롤 초기화.
    SetupComboBox(nFileSearchLevel);

    if (VerifyVectorSize(LEVEL_ALL) == FALSE)
        return;

    strReviewImagePath += m_vecstrDate[m_nSelectDate] + _T("\\") + m_vecstrJobName[m_nSelectJobName] + _T("\\")
        + m_vecstrLotID[m_nSelectLotID] + _T("\\") + m_vecstrFileName[m_nSelectFileName];

    ImageOpen(strReviewImagePath);
}

void CDlgReviewImageViewer::Init(long nLevel)
{
    if (nLevel == LEVEL_DATE || nLevel == LEVEL_ALL)
    {
        m_vecstrDate.clear();
        m_nSelectDate = 0;
    }
    if (nLevel == LEVEL_JOB_NAME || nLevel == LEVEL_ALL)
    {
        m_vecstrJobName.clear();
        m_nSelectJobName = 0;
    }
    if (nLevel == LEVEL_LOT_ID || nLevel == LEVEL_ALL)
    {
        m_vecstrLotID.clear();
        m_nSelectLotID = 0;
    }
    if (nLevel == LEVEL_FILE_NAME || nLevel == LEVEL_ALL)
    {
        m_vecstrFileName.clear();
        m_nSelectFileName = 0;
    }
}

void CDlgReviewImageViewer::FileSearch(CString strFolderPath, long nLevel)
{
    CFileFind ff;

    if (strFolderPath.Right(1) != _T('\\'))
    {
        strFolderPath += _T('\\'); //(오류방지)마지막에 하위루트 경로 표시
    }

    strFolderPath += _T("*.*"); //모든 파일에 대해서 (확장자명을 명시하면 그 파일만 삭제)

    BOOL bSearchDate = FALSE;
    BOOL bSearchJobName = FALSE;
    BOOL bSearchLotID = FALSE;

    if (ff.FindFile(strFolderPath))
    {
        CString strFileName;
        BOOL bFlag = TRUE;

        Init(nLevel); // 초기화 해준다.

        while (bFlag)
        {
            bFlag = ff.FindNextFile(); //다음 파일이 있는지 확인 (없으면 현재파일 삭제 후 while문 종료)
            strFileName = ff.GetFilePath(); // 현재 파일의 경로를 불러옴
            if (ff.IsDots())
            {
                continue;
            }
            if (ff.IsDirectory() && nLevel != LEVEL_FILE_NAME) //현재 파일이 디렉토리면
            {
                // 영훈 20130816 : Review Image 여기까지 하다가 그만뒀음. 주말에 하기..아마도...
                if (nLevel == LEVEL_DATE)
                {
                    m_vecstrDate.push_back(ff.GetFileName());
                    if (bSearchDate == FALSE)
                    {
                        FileSearch(ff.GetFilePath(), LEVEL_JOB_NAME); //하위 디렉토리 경로를 재귀호출
                        bSearchDate = TRUE;
                    }
                }
                else if (nLevel == LEVEL_JOB_NAME)
                {
                    m_vecstrJobName.push_back(ff.GetFileName());
                    if (bSearchJobName == FALSE)
                    {
                        FileSearch(ff.GetFilePath(), LEVEL_LOT_ID); //하위 디렉토리 경로를 재귀호출
                        bSearchJobName = TRUE;
                    }
                }
                else if (nLevel == LEVEL_LOT_ID)
                {
                    m_vecstrLotID.push_back(ff.GetFileName());
                    if (bSearchLotID == FALSE)
                    {
                        FileSearch(ff.GetFilePath(), LEVEL_FILE_NAME); //하위 디렉토리 경로를 재귀호출
                        bSearchLotID = TRUE;
                    }
                }
            }
            else if (nLevel == LEVEL_FILE_NAME)
            {
                if (ff.GetFileName().Right(3) == "jpg" || ff.GetFileName().Right(3) == "JPG")
                    m_vecstrFileName.push_back(ff.GetFileName());
            }
        }
        ff.Close();
    }

    // Search를 위해 백업을 하나 만든다.
    if (nLevel == LEVEL_FILE_NAME)
    {
        m_vecstrFileName_Backup = m_vecstrFileName;
    }
}

void CDlgReviewImageViewer::SetupComboBox(long nLevel)
{
    // Date
    long nFolderCount;
    CString strTemp;

    if (nLevel <= LEVEL_DATE || nLevel == LEVEL_ALL)
    {
        m_cmbDateList.ResetContent();
        nFolderCount = (long)(m_vecstrDate.size());
        for (long n = 0; n < nFolderCount; n++)
        {
            m_cmbDateList.AddString(m_vecstrDate[n]);
        }
        m_cmbDateList.SetCurSel(m_nSelectDate);
    }
    if (nLevel <= LEVEL_JOB_NAME || nLevel == LEVEL_ALL)
    {
        m_cmbJobNameList.ResetContent();
        nFolderCount = (long)(m_vecstrJobName.size());
        for (long n = 0; n < nFolderCount; n++)
        {
            m_cmbJobNameList.AddString(m_vecstrJobName[n]);
        }
        m_cmbJobNameList.SetCurSel(m_nSelectJobName);
    }
    if (nLevel <= LEVEL_LOT_ID || nLevel == LEVEL_ALL)
    {
        m_cmbLotIDList.ResetContent();
        nFolderCount = (long)(m_vecstrLotID.size());
        for (long n = 0; n < nFolderCount; n++)
        {
            m_cmbLotIDList.AddString(m_vecstrLotID[n]);
        }
        m_cmbLotIDList.SetCurSel(m_nSelectLotID);
    }
    if (nLevel <= LEVEL_FILE_NAME || nLevel == LEVEL_ALL)
    {
        m_cmbFileNameList.ResetContent();
        nFolderCount = (long)(m_vecstrFileName.size());
        for (long n = 0; n < nFolderCount; n++)
        {
            strTemp.Format(_T("%d. %s"), n + 1, (LPCTSTR)m_vecstrFileName[n]);
            m_cmbFileNameList.AddString(strTemp);
        }
        m_cmbFileNameList.SetCurSel(m_nSelectFileName);
    }
}

void CDlgReviewImageViewer::OnCbnSelchangeCmbDate()
{
    if (VerifyVectorSize(LEVEL_DATE) == FALSE)
        return;

    const CString strRealReviewImagePath = SystemConfig::GetInstance().m_strSaveDrive + _T("Review Image\\");
    CString strReviewImagePath = strRealReviewImagePath + m_vecstrDate[m_nSelectDate];

    // 파일 트리를 작성.
    FileSearch(strReviewImagePath, LEVEL_JOB_NAME);

    // 트리에 맞춰 콤보박스 컨트롤 초기화.
    SetupComboBox(LEVEL_JOB_NAME);

    if (VerifyVectorSize(LEVEL_ALL) == FALSE)
        return;

    strReviewImagePath += _T("\\") + m_vecstrJobName[0] + _T("\\") + m_vecstrLotID[0] + _T("\\") + m_vecstrFileName[0];
    ImageOpen(strReviewImagePath);
}

void CDlgReviewImageViewer::OnCbnSelchangeCmbJobName()
{
    if (VerifyVectorSize(LEVEL_JOB_NAME) == FALSE)
        return;

    const CString strRealReviewImagePath = SystemConfig::GetInstance().m_strSaveDrive + _T("Review Image\\");
    CString strReviewImagePath
        = strRealReviewImagePath + m_vecstrDate[m_nSelectDate] + _T("\\") + m_vecstrJobName[m_nSelectJobName];

    // 파일 트리를 작성.
    FileSearch(strReviewImagePath, LEVEL_LOT_ID);

    // 트리에 맞춰 콤보박스 컨트롤 초기화.
    SetupComboBox(LEVEL_LOT_ID);

    if (VerifyVectorSize(LEVEL_ALL) == FALSE)
        return;

    strReviewImagePath += _T("\\") + m_vecstrLotID[0] + _T("\\") + m_vecstrFileName[0];
    ImageOpen(strReviewImagePath);
}

void CDlgReviewImageViewer::OnCbnSelchangeCmbLotid()
{
    if (VerifyVectorSize(LEVEL_LOT_ID) == FALSE)
        return;

    const CString strRealReviewImagePath = SystemConfig::GetInstance().m_strSaveDrive + _T("Review Image\\");
    CString strReviewImagePath = strRealReviewImagePath + m_vecstrDate[m_nSelectDate] + _T("\\")
        + m_vecstrJobName[m_nSelectJobName] + _T("\\") + m_vecstrLotID[m_nSelectLotID];

    // 파일 트리를 작성.
    FileSearch(strReviewImagePath, LEVEL_FILE_NAME);

    // 트리에 맞춰 콤보박스 컨트롤 초기화.
    SetupComboBox(LEVEL_FILE_NAME);

    if (VerifyVectorSize(LEVEL_ALL) == FALSE)
        return;

    strReviewImagePath += _T("\\") + m_vecstrFileName[0];
    ImageOpen(strReviewImagePath);
}

void CDlgReviewImageViewer::OnCbnSelchangeCmbFileName()
{
    if (VerifyVectorSize(LEVEL_ALL) == FALSE)
        return;

    const CString strRealReviewImagePath = SystemConfig::GetInstance().m_strSaveDrive + _T("Review Image\\");
    CString strFilePath = strRealReviewImagePath + m_vecstrDate[m_nSelectDate] + _T("\\")
        + m_vecstrJobName[m_nSelectJobName] + _T("\\") + m_vecstrLotID[m_nSelectLotID] + _T("\\")
        + m_vecstrFileName[m_nSelectFileName];

    ImageOpen(strFilePath);
}

void CDlgReviewImageViewer::ImageOpen(CString strFilePath)
{
    // 이미지를 불러오도록 한다.
    m_pDlgImageViewer->SetImage(strFilePath);

    long nSize = (long)m_vecstrFileName.size();
    CString strFileCount;
    strFileCount.Format(_T("FileName [ %04d / %04d ] :"), m_nSelectFileName + 1, nSize);
    ((CStatic*)GetDlgItem(IDC_STATIC_FILE_NAME))->SetWindowText(strFileCount);
    m_cmbFileNameList.SetCurSel(m_nSelectFileName);

    // Text Result를 불러오도록 한다.
    CString strTextLogPath = strFilePath.Left(strFilePath.GetLength() - 3) + _T("txt");
    CFileFind ff;
    CString strOut;
    if (ff.FindFile(strTextLogPath))
    {
        std::vector<unsigned char> strTemp(100000);

        CFile FileIn(strTextLogPath, CFile::modeRead);
        long nLength = (long)FileIn.GetLength();
        FileIn.Read(&strTemp[0], nLength);
        strTemp[nLength] = 0;

        // TODO : 이현민 - 버그
        CString strTemp2(&strTemp[0]);

        strOut = strTemp2;
    }
    else
    {
        strOut.Format(_T("Do not Find File."));
    }

    m_reTextResult.SetWindowText(_T(""));
    m_reTextResult.ReplaceSel(strOut);

    UpdateData(FALSE);
}

BOOL CDlgReviewImageViewer::VerifyVectorSize(long nLevel)
{
    UpdateData(TRUE);

    long nSize = 0;

    if (nLevel == LEVEL_DATE || nLevel == LEVEL_ALL)
    {
        m_nSelectDate = (long)(m_cmbDateList.GetCurSel());
        nSize = (long)m_vecstrDate.size();
        if (nSize <= 0)
        {
            m_cmbDateList.ResetContent();
            m_cmbJobNameList.ResetContent();
            m_cmbLotIDList.ResetContent();
            m_cmbFileNameList.ResetContent();
            m_vecstrJobName.clear();
            m_vecstrLotID.clear();
            m_vecstrFileName.clear();
            return FALSE;
        }
        if (nSize <= m_nSelectDate)
        {
            m_nSelectDate = nSize - 1;
            m_cmbDateList.SetCurSel(m_nSelectDate);
        }
    }
    if (nLevel == LEVEL_JOB_NAME || nLevel == LEVEL_ALL)
    {
        m_nSelectJobName = (long)(m_cmbJobNameList.GetCurSel());
        nSize = (long)m_vecstrJobName.size();
        if (nSize <= 0)
        {
            m_cmbJobNameList.ResetContent();
            m_cmbLotIDList.ResetContent();
            m_cmbFileNameList.ResetContent();
            m_vecstrLotID.clear();
            m_vecstrFileName.clear();
            return FALSE;
        }
        if (nSize <= m_nSelectJobName)
        {
            m_nSelectJobName = nSize - 1;
            m_cmbJobNameList.SetCurSel(m_nSelectJobName);
        }
    }
    if (nLevel == LEVEL_LOT_ID || nLevel == LEVEL_ALL)
    {
        m_nSelectLotID = (long)(m_cmbLotIDList.GetCurSel());
        nSize = (long)m_vecstrLotID.size();
        if (nSize <= 0)
        {
            m_cmbLotIDList.ResetContent();
            m_cmbFileNameList.ResetContent();
            m_vecstrFileName.clear();
            return FALSE;
        }
        if (nSize <= m_nSelectLotID)
        {
            m_nSelectLotID = nSize - 1;
            m_cmbLotIDList.SetCurSel(m_nSelectLotID);
        }
    }
    if (nLevel == LEVEL_FILE_NAME || nLevel == LEVEL_ALL)
    {
        m_nSelectFileName = (long)(m_cmbFileNameList.GetCurSel());
        nSize = (long)m_vecstrFileName.size();
        if (nSize <= 0)
        {
            m_cmbFileNameList.ResetContent();
            return FALSE;
        }
        if (nSize <= m_nSelectFileName)
        {
            m_nSelectFileName = nSize - 1;
            m_cmbFileNameList.SetCurSel(m_nSelectFileName);
        }
    }

    UpdateData(FALSE);

    return TRUE;
}

void CDlgReviewImageViewer::OnBnClickedBtnSearchFile()
{
    UpdateData(TRUE);

    CString strSearchName;
    ((CEdit*)GetDlgItem(IDC_EDIT_SEARCH))->GetWindowText(strSearchName);
    if (strSearchName == _T(""))
    {
        AfxMessageBox(_T("Do not find item name.."));
        return;
    }
    ((CEdit*)GetDlgItem(IDC_EDIT_SEARCH))->SetWindowText(strSearchName);

    std::vector<CString> vecFileNameTemp;
    m_cmbFileNameList.SetCurSel(0);
    long nSize = (long)m_vecstrFileName_Backup.size();

    CString strOriginName;
    strSearchName.MakeLower();
    for (long n = 0; n < nSize; n++)
    {
        strOriginName = m_vecstrFileName_Backup[n];
        strOriginName.MakeLower();
        if (strOriginName.Find(strSearchName) >= 0)
        {
            vecFileNameTemp.push_back(m_vecstrFileName_Backup[n]);
        }
    }

    if ((long)vecFileNameTemp.size() <= 0)
    {
        AfxMessageBox(_T("Do not find item name.."));
        return;
    }

    m_vecstrFileName = vecFileNameTemp;

    // 트리에 맞춰 콤보박스 컨트롤 초기화.
    SetupComboBox(LEVEL_FILE_NAME);

    if (VerifyVectorSize(LEVEL_ALL) == FALSE)
        return;

    const CString strRealReviewImagePath = SystemConfig::GetInstance().m_strSaveDrive + _T("Review Image\\");
    CString strFilePath = strRealReviewImagePath + m_vecstrDate[m_nSelectDate] + _T("\\")
        + m_vecstrJobName[m_nSelectJobName] + _T("\\") + m_vecstrLotID[m_nSelectLotID] + _T("\\") + m_vecstrFileName[0];
    ImageOpen(strFilePath);

    UpdateData(FALSE);
}

void CDlgReviewImageViewer::OnTcnSelchangeTab(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nCursel = m_ctrlTab.GetCurSel();

    if (nCursel == 0) // Image View
    {
        m_pDlgImageViewer->ShowWindow(SW_SHOW);
        ((CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT))->ShowWindow(SW_HIDE);
    }
    else // Text Log
    {
        m_pDlgImageViewer->ShowWindow(SW_HIDE);
        ((CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT))->ShowWindow(SW_SHOW);
    }

    *pResult = 0;
}
