#pragma once
//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageViewEx;
class VisionProcessingNGRV;
class VisionProcessingNGRVPara;

//HDR_6_________________________________ Header body

// DlgVisionProcessingNGRV_BTM2D 대화 상자

class DlgVisionProcessingNGRV_BTM2D : public CDialog
{
    DECLARE_DYNAMIC(DlgVisionProcessingNGRV_BTM2D)

public:
    DlgVisionProcessingNGRV_BTM2D(
        VisionProcessingNGRV* processor, VisionProcessingNGRVPara& para, CWnd* pParent = nullptr); // 표준 생성자입니다.
    virtual ~DlgVisionProcessingNGRV_BTM2D();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DLG_NGRV_BTM_TAB
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnItemButtonClickBTM(NMHDR* pNotifyStruct, LRESULT*);
    afx_msg void OnDefectNameChangedBTM(NMHDR* pNotifyStruct, LRESULT*);
    afx_msg LRESULT OnGrabEndEvent(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnGridNotify(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedButtonChangeFrameBw();
    afx_msg void OnBnClickedButtonChangeFrameFw();
    afx_msg void OnBnClickedButtonLive();
    afx_msg void OnBnClickedButtonGrab();
    afx_msg void OnItemClickBTM(NMHDR* pNMHDR, LRESULT* pResult);

public:
    // Factors
    ImageViewEx* m_imageView;
    VisionProcessingNGRV* m_processor;
    VisionProcessingNGRVPara& m_para;
    CXTPPropertyGrid* m_grid;
    CXTPGridControl m_parameterList;
    Ipvm::SocketMessaging* m_pMessageSocket;

    Ipvm::Image8u* m_grabImage;
    Ipvm::Image8u3* m_bayerImage;

    CRect m_gridArea;
    CRect m_listArea;
    CRect m_frameSelectionArea;
    CRect m_imageArea;

    long m_selectedDefectID;

    bool m_isLive;

    long m_imageID;

    CButton m_frameBW;
    CButton m_frameFW;
    CButton m_buttonLive;
    CButton m_buttonGrab;

    CEdit m_frameNum;

    // Functions
    void OnInitGrid();
    void OnInitList();
    void OnInitImage();

    void ResetListControl();
    void AddItem();
    void DeleteItem();
    void ApplyItem();

    bool SetROI();
    bool GetROI();

    void SetDefectROIinfo();
    void ChangeROItitle();

    bool StartLive(long frameNum);
    bool StartGrab(long defectNum);
    bool ReadyToGrabNGRV(std::vector<long> vecFrameID, bool useIR, bool isInline);

    void ShowPreviousImage();
    void ShowNextImage();
    void ShowImage(BOOL change);

    BOOL SetupToGrab(std::vector<long> frameID, BOOL isUseIR, BOOL isInline);

    void iPIS_Send_SingleRun_MoveTo(); // NGRV Defect Item 위치 정보를 보내는 메세지 추가

    bool IsRoiSizeOverThanImageSize();

protected:
    struct FOVBuffer
    {
        BYTE* m_imagePtrs[LED_ILLUM_NGRV_CHANNEL_MAX];
    };

    std::vector<std::shared_ptr<FOVBuffer>> m_buffers;    
};
