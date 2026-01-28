#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageViewEx;

//HDR_6_________________________________ Header body
//
class CDlgMarkTeachTotalResultViewer : public CDialog
{
    DECLARE_DYNAMIC(CDlgMarkTeachTotalResultViewer)

public:
    CDlgMarkTeachTotalResultViewer(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgMarkTeachTotalResultViewer();

    void SetImageData(
        const Ipvm::Image8u& image, Ipvm::Rect32s i_rtViewArea, long i_nMarkCharNum, Ipvm::Rect32s* i_prtEachCharROI);
    void UpdateImage();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_MARK_TEACH_RESULT_VIEW
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()
    ImageViewEx* m_imageView;

    Ipvm::Image8u m_imageTeach;
    Ipvm::Rect32s m_rtViewArea;
    long m_nMarkCharNum;
    std::vector<Ipvm::Rect32s> m_vecrtEachCharROI;

public:
    virtual BOOL OnInitDialog();
};
