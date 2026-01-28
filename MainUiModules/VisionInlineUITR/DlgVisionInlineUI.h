#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "MachineVisionCollectInfo.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u3.h> //kircheis_NGRVINLINE
#include <afxcmn.h>
#include <afxmt.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
class ImageLotView;
class CVisionInlineUI;
class VisionUnit;
class SystemConfig;
class VisionInspectionOverlayResult;
class VisionDeviceResult;

//HDR_6_________________________________ Header body
//
struct sinlineSequenceLogInfo
{
    long m_nInspectionRepeatindex;
    long m_nTrayindex;
    long m_nScanID;
    long m_nPaneIndex;
    std::vector<std::map<CString, CString>> m_vecmapInlineSequenceTimeLog;

    void Init()
    {
        m_nInspectionRepeatindex = -1;
        m_nTrayindex = -1;
        m_nScanID = -1;
        m_nPaneIndex = -1;
        m_vecmapInlineSequenceTimeLog.clear();
    }

    void SetInlineSequenceTrayInfo(
        const long i_nInspectionRepeatindex, const long i_nTrayindex, const long i_nScanID, const long i_nPaneindex)
    {
        m_nInspectionRepeatindex = i_nInspectionRepeatindex;
        m_nTrayindex = i_nTrayindex;
        m_nScanID = i_nScanID;
        m_nPaneIndex = i_nPaneindex;
    }

    void SetInlineSequenceLog(const CString i_strFunctionName, const float i_fFunctionExcuteTime_ms)
    {
        std::map<CString, CString> mapstrInlineTimeLog;
        CString strFunctionExcuteTime("");
        strFunctionExcuteTime.Format(_T("%.3f"), i_fFunctionExcuteTime_ms);
        mapstrInlineTimeLog[i_strFunctionName] = strFunctionExcuteTime;
        m_vecmapInlineSequenceTimeLog.push_back(mapstrInlineTimeLog);
    }
};

class CDlgVisionInlineUI : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInlineUI)

public:
    CDlgVisionInlineUI(CVisionInlineUI& visionInlineUI, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInlineUI();

    enum
    {
        IDD = IDD_DIALOG_MAIN
    };

protected:
    ImageLotView* m_imageLotView;
    VisionInspectionOverlayResult* m_overlayResult;
    CCriticalSection m_csOverlay;

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnInspectionStart(WPARAM, LPARAM);
    afx_msg LRESULT OnInspectionEnd(WPARAM, LPARAM);

public: // 함수 정의
    long OnThreadInspect(bool manualInsp);
    BOOL Send2ndInspInfo(VisionDeviceResult& devResult,
        const enSideVisionModule i_eSideVisionModule
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // ksy for 2nd inspection
    BOOL SaveAllInspImage(VisionDeviceResult& devResult,
        const enSideVisionModule i_eSideVisionModule
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // ksy for collecting learning images (All image)
    BOOL CollectMVRejectData(VisionDeviceResult& devResult,
        const enSideVisionModule i_eSideVisionModule
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // ksy for collecting learning images (Insp item)
    void SetDebugStop(long& nSumResult, long nResult);

    BYTE* m_pbyDevResultSaveBuffer;
    void ConvertDeviceResultToBuffer(BYTE* pbyBuffer, VisionDeviceResult& deviceResult, BOOL bSave);

    void SetColorImageForNGRV(Ipvm::Image8u3 image); //kircheis_NGRVINLINE

public: // 변수 정의
    CVisionInlineUI& m_visionInlineUI;
    VisionUnit& m_visionUnit;
    SystemConfig& m_systemConfig;

private:
    sinlineSequenceLogInfo m_sinlineSequenceLogInfo;
    void SaveInspectionTimeLog();
};
