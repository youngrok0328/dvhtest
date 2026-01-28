#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace VisionEdgeAlign
{
class Para;
class PropertyGrid_Impl;
} // namespace VisionEdgeAlign

class ImageLotView;
class ImageViewEx;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
class CDlgAutoFocusImageSet : public CDialog
{
    DECLARE_DYNAMIC(CDlgAutoFocusImageSet)

public:
    CDlgAutoFocusImageSet(VisionEdgeAlign::Para& para, VisionEdgeAlign::PropertyGrid_Impl* gridEngine,
        VisionProcessing& proc, long nBaseFrameIndex, CWnd* pParent = nullptr); // 표준 생성자입니다.
    virtual ~CDlgAutoFocusImageSet();

    // 대화 상자 데이터입니다.
    //#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_AUTO_FOCUS_IMAGE_SET
    };
    //#endif

public:
    void SetAFImage();
    void SetROI(Ipvm::Image8u i_CurrentImage);

public:
    VisionProcessing& m_proc;

    ImageLotView* m_ImageLotView;
    ImageViewEx* m_ImageViewEx;
    VisionEdgeAlign::PropertyGrid_Impl* m_gridEngine;
    VisionEdgeAlign::Para& m_para;

    long m_nImageFrameCount;
    long m_nBaseFrameIndex;

    //{{//kircheis_NGRVAF
    BOOL m_bIsValidPlaneRefInfo;
    long m_nNGRVAF_FrameID;
    std::vector<Ipvm::Point32s2> m_vecptRefPos_UM;
    long m_nAFImgSizeX;
    long m_nAFImgSizeY;
    std::vector<BYTE>
        m_vecbyAFImage; //원래 BYTE*로 만들려 했으나 메모리 관리의 문제로 인해 vector화. 필요시 BYTE*로 변환 사용할 것
    //}}

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnBnClickedApply();
    afx_msg void OnBnClickedButtonSetDefault_AF_ROI();
};
