#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageViewEx;
class VisionInspectionSurface;

//HDR_6_________________________________ Header body
//
class CSurfaceUserCustomMaskDlg : public CDialog
{
    // Construction

public:
    Ipvm::Image8u* m_pEditSurfaceBitmapMask;

    CSurfaceUserCustomMaskDlg(const Ipvm::Image8u& SurfaceBitmapMask, Ipvm::Image8u& image,
        VisionInspectionSurface* pVisionInsp); // standard constructor
    ~CSurfaceUserCustomMaskDlg();

    enum
    {
        IDD = IDD_SURFACE_USER_CUSTOM_MASK
    };
    CEdit m_edtRadius;

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    void DeleteObject(long x, long y);
    void AddCircle(long x, long y, long nRadius);

    Ipvm::Image8u m_displayImage;
    Ipvm::Image8u m_image;
    CPoint m_ptLatestSelPoint;

    ImageViewEx* m_imageView;
    VisionInspectionSurface* m_pVisionInsp;

    void RefreshImage();

    virtual BOOL OnInitDialog();
    afx_msg void OnBtnExport();
    afx_msg void OnBtnImport();
    afx_msg void OnBTNErode();
    afx_msg void OnBTNDilate();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedOk();

    static void callbackMouseClicked(void* userData, const int32_t id, const Ipvm::Point32r2& imagePoint);
    void callbackMouseClicked(const int32_t id, const Ipvm::Point32r2& imagePoint);
};
