#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/Ngrv2DImageInfo.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Point32s2.h>
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageViewEx;
class VisionProcessingNGRV;
class VisionProcessingNGRVPara;

//HDR_6_________________________________ Header body
//CPP_7_________________________________ Implementation body
//
class DlgVisionProcessingNGRV_DetailSetup : public CDialog
{
    DECLARE_DYNAMIC(DlgVisionProcessingNGRV_DetailSetup)

public:
    DlgVisionProcessingNGRV_DetailSetup(VisionProcessingNGRV* pVisionInsp, long defectID, BOOL isBTM,
        std::vector<Ipvm::Image8u3> grabbedImage, const long& ref_Input_ImageSizeX, const long& ref_Input_ImageSizeY,
        CWnd* pParent = nullptr); // 표준 생성자입니다.
    virtual ~DlgVisionProcessingNGRV_DetailSetup();

    enum
    {
        IDD = IDD_DLG_NGRV_DETAIL_SETUP
    };

public: // Factors
    ImageViewEx* m_imageView;
    VisionProcessingNGRV* m_visionInsp;
    VisionProcessingNGRVPara* m_visionPara;
    CXTPPropertyGrid* m_grid;

    std::vector<Ipvm::Image8u3> m_grabbedImages;

    long m_defectID;

public: // Functions
    void OnInitGrid();
    void ClickButtonApply();
    void ClickButtonCancel();
    void ShowImage();
    void ShowFrameImage(long frameNum);

    void SetCurrentRoiStatus();

    afx_msg LRESULT OnGridNotify(WPARAM wParam, LPARAM lParam);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();

private:
    Ngrv2DImageInfo cur_imageInfo;

    long m_ref_ImageSizeX;
    long m_ref_ImageSizeY;

    Ipvm::Point32s2 get_Max_Stitch_Count(const float& scaleX, const float& scaleY, const Ipvm::Rect32r& CameraSpecFOV,
        const long& ref_ImageSizeX, const long& ref_ImageSizeY);
};
