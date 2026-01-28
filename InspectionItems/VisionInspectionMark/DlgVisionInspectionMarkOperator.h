#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CDlgVisionInspectionMark;
class VisionInspectionMarkPara;
class VisionScale;
class VisionInspectionMarkSpec;

//HDR_6_________________________________ Header body
//
enum enumTeachSlot // Mark Pre Proc Mode 추가 by kjy, 2007.10.08
{
    enumTeachSlot1,
    enumTeachSlot2,
    enumTeachSlot3,
    //	enumTeachSlot4,
};

class CDlgVisionInspectionMarkOperator : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionMarkOperator)

public:
    CDlgVisionInspectionMarkOperator(const VisionScale& scale, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspectionMarkOperator();

    virtual BOOL OnInitDialog();
    void InitializeBtnCtrl();

    void MultiTeachingSlot(long nTeachSlot);
    void ShowCombineImage(BOOL bAutoThreshold);
    void SetPoint(Ipvm::Point32s2 ptDispoint);
    void SetROI();
    void GetROI();
    void SetCurSpecValue(long nTeachSlot);
    void SetMultiImage();
    void ReSetMarkSlot(long nMarkSlot);
    void SetInspectionImage();
    void RefreshData();

    //kk Mark MapData
    void SetRoiParameter();
    void SetROI(long nMode);
    void GetROI(long nMode);

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_OPERATOR_PARAMETER
    };

protected:
    void clickButton_teach();
    void clickButton_test();
    void clickButton_charSpliteMode();
    void generatePropertyGrid();
    void generatePropertyGrid_teachPara();

    VisionInspectionMarkSpec* getCurrentSpec();

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    const VisionScale& m_scale;
    Ipvm::ImageView* m_imageViews[NUM_OF_MARKMULTI];
    CDlgVisionInspectionMark* m_pVisionInspDlg;
    VisionInspectionMarkPara* m_pVisionPara;
    std::vector<CPoint> m_vecptIgnorePoint;

    long m_nTeachSlot;

    BOOL m_bBtnSplitePush;

    void UpdateMergeROI();
    void UpdateIgnoreROI();

    std::vector<Ipvm::Point32s2> m_vecptSplitePoint;

public:
    CButton m_BtnSelectedSpec;
    CXTPPropertyGrid* m_propertyGrid;

    afx_msg void OnBnClickedButtonRemoveSlot2();
    afx_msg void OnBnClickedButtonRemoveSlot3();
    afx_msg void OnBnClickedRadioMultiTeachingSlot1();
    afx_msg void OnBnClickedRadioMultiTeachingSlot2();
    afx_msg void OnBnClickedRadioMultiTeachingSlot3();

public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
};
