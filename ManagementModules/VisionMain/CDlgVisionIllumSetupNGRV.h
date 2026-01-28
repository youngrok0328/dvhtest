#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "IllumControlNGRV.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <array>
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
class SystemConfig;
class CVisionMain;
class VisionUnit;
class ImageViewEx;
class CGridCtrl;
class IllumInfo2D;

//HDR_6_________________________________ Header body
//
class CDlgVisionIllumSetupNGRV : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionIllumSetupNGRV)

public:
    CDlgVisionIllumSetupNGRV(CVisionMain& visionMain, VisionUnit& visionUnit, CWnd* pParent); // 표준 생성자입니다.
    virtual ~CDlgVisionIllumSetupNGRV();

    BOOL callJobOpen();
    BOOL callJobSave();

    CVisionMain& m_visionMain;
    VisionUnit& m_visionUnit;
    ImageViewEx* m_imageView;
    CXTPPropertyGrid* m_propertyGrid;
    Ipvm::ProfileView* m_profileView;

    // 대화 상자 데이터입니다.
    //#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_ILLUM_SETUP_NGRV
    };
    //#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();

    void SetIllumParameter(long i_nFrame);

    BOOL UpdateIllumChSetupUI(long nFrame, BOOL bRecommendSetForColor, long nModifyCh = -1); //kircheis_WB

public:
    Ipvm::Image8u* m_grabImage;
    Ipvm::Image8u3* m_bayerImage;
    Ipvm::Image32u* m_histogram;
    Ipvm::Image32u* m_histogramRed;
    Ipvm::Image32u* m_histogramGreen;
    Ipvm::Image32u* m_histogramBlue;

    CString m_backupInfo_name;
    std::map<long, BOOL> m_backupInfo_items;
    std::map<long, std::map<CString, BOOL>> m_backupInfo_items_inspFrames;
    std::array<float, 10> m_backupInfo_illuminations_ms;
    bool m_backupInfo_colorFrame[3];

    static void callBack_roiChanged(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength);
    static void callBack_changedSelectFrame(LPVOID userData, long realFrameIndex);
    static void callBack_changedSelectItem(LPVOID userData, long itemID);
    static void callBack_changedItemFrame(LPVOID userData, long itemID, long realFrameIndex, bool checked);
    static void callBack_deletedFrame(LPVOID userData, long realFrameIndex, bool isBackup);
    static void callBack_insertedFrame(LPVOID userData, long groupID, long realFrameIndex, bool isBackup);

    void callBack_changedSelectFrame(long realFrameIndex);
    void callBack_changedSelectItem(long itemID);
    void callBack_changedItemFrame(long itemID, long realFrameIndex, bool checked);
    void callBack_deletedFrame(long realFrameIndex, bool isBackup);
    void callBack_insertedFrame(long groupID, long realFrameIndex, bool isBackup);

    void inspFrameInvalidCheck();
    void calcHistogram(const Ipvm::Image8u* image);
    void calcHistogramRGB(const Ipvm::Image8u3* image);

    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

    afx_msg LRESULT OnGrabEndEvent(WPARAM wparam, LPARAM lparam);

    long m_nCurrentFrameIndex;

    void UpdateSpecToIllumView();
    IllumInfo2D& illumJob();

    BOOL m_bUseGain;

private:
    IllumControlNGRV m_NGRV_illumView;

    std::vector<float> m_vecfDestIllumSet; //kircheis_WB
    void GrabAndDisplayColorImage(); //kircheis_WB
    void IllumLink(LPCTSTR filePath, bool save, IllumInfo2D& io_illum);

    Ipvm::Image8u* m_ImageRed;
    Ipvm::Image8u* m_ImageGreen;
    Ipvm::Image8u* m_ImageBlue;

    CWnd* m_pParent;

public:
    afx_msg void OnDestroy();
};
