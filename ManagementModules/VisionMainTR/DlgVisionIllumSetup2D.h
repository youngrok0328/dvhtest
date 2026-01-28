#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "IllumControl2D.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedComponent/Label/NewLabel.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <array>
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
class SystemConfig;
class VisionMainTR;
class VisionUnit;
class ImageViewEx;
class CGridCtrl;
class IllumInfo2D;

//HDR_6_________________________________ Header body
//
class DlgVisionIllumSetup2D : public CDialog
{
    DECLARE_DYNAMIC(DlgVisionIllumSetup2D)

public:
    DlgVisionIllumSetup2D(VisionMainTR& visionMain, VisionUnit& visionUnit, CWnd* pParent,
        const enSideVisionModule i_eSideVisionModule
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // 표준 생성자입니다.
    virtual ~DlgVisionIllumSetup2D();

    BOOL callJobOpen();
    BOOL callJobSave();

    VisionMainTR& m_visionMain;
    VisionUnit& m_visionUnit;
    ImageViewEx* m_imageView;
    ImageViewEx* m_imageViewRearSide;
    CXTPPropertyGrid* m_propertyGrid;
    Ipvm::ProfileView* m_profileView;

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_ILLUM_SETUP_2D
    };
    enum
    {
        IDD_SIDE = IDD_DIALOG_ILLUM_SETUP_SIDE
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    void SetIllumiParameter(long nFrame);

    BOOL UpdateIllumChSetupUI(long nFrame, BOOL bRecommendSetForColor, long nModifyCh = -1); //kircheis_WB

public:
    Ipvm::Image8u* m_grabImage;
    Ipvm::Image8u3* m_colorgrabImage;
    Ipvm::Image32u* m_histogram;

    CString m_backupInfo_name;
    std::map<long, BOOL> m_backupInfo_items;
    std::map<long, std::map<CString, BOOL>> m_backupInfo_items_inspFrames;
    std::array<float, 16> m_backupInfo_illuminations_ms;
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

    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

    afx_msg LRESULT OnGrabEndEvent(WPARAM wparam, LPARAM lparam);

    long m_nCurrentFrameIndex;
    afx_msg void OnDestroy();

    void updateSpecToIllumView();
    IllumInfo2D& illumJob();

private:
    IllumControl2D m_illumView;

    std::vector<float> m_vecfDestIllumSet; //kircheis_WB
    void GrabAndDisplayColorImage(); //kircheis_WB
    void IllumLink(LPCTSTR filePath, bool save, IllumInfo2D& io_illum);

    enSideVisionModule m_eCurVisionModule;

public:
    afx_msg void OnStnClickedStaticSideFrontIllum();
    afx_msg void OnStnClickedStaticSideRearIllum();

    CNewLabel m_Label_Side_Front_Status;
    CNewLabel m_Label_Side_Rear_Status;

    void SetGeneralVisionIllumUI();
    void SetSideVisionIllumUI();
};
