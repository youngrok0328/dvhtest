#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CDlgImageVIewer : public CDialog
{
    DECLARE_DYNAMIC(CDlgImageVIewer)

public:
    CDlgImageVIewer(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgImageVIewer();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_IMAGEVIEWER
    };

public:
    CScrollView* m_pScrollView;

    long m_nImageSizeX;
    long m_nImageSizeY;

    long m_nScrollMoveX;
    long m_nScrollMoveY;

    BOOL m_bSetImage;

    CString m_strFilePath;

public:
    void SetImage(CString strJPGFullPath);
    void SetupScroll(long nWidth, long nHeight);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnPaint();
};
