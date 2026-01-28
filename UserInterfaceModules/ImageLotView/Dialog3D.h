#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionImageLot;
class VisionImageLotInsp;
class VisionScale;
class Dialog3D : public CDialog
{
    DECLARE_DYNAMIC(Dialog3D)

public:
    Dialog3D(const Ipvm::Image8u& grayZmap, const Ipvm::Image8u& grayVmap, const Ipvm::Image32r& zmap,
        const Ipvm::Image16u& vmap, const VisionScale& scale,
        CWnd* pParent = nullptr); // standard constructor
    virtual ~Dialog3D();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_3D
    };
#endif

protected:
    const Ipvm::Image8u& m_grayZmap;
    const Ipvm::Image8u& m_grayVmap;
    const Ipvm::Image32r& m_zmap;
    const Ipvm::Image16u& m_vmap;
    const VisionScale& m_scale;

    Ipvm::ImageView* m_imageView;
    Ipvm::SurfaceView* m_surfaceView;

    void UpdateUIPosition();
    void Update3DROI();
    Ipvm::Rect32s GetMapROI(const Ipvm::Rect32s& roi);

    static void ROI_Changed(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength);
    void ROI_Changed(const int32_t id, const wchar_t* key, const size_t keyLength);

    static void PixelInfo(void* userData, const int32_t id, const Ipvm::Point32r2& imagePoint,
        const uint32_t infoBufferLength, wchar_t* infoBuffer);
    void PixelInfo(
        const int32_t id, const Ipvm::Point32r2& imagePoint, const uint32_t infoBufferLength, wchar_t* infoBuffer);

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    BOOL m_bExecutedResizeROI; //kircheis_Test3DV

    DECLARE_MESSAGE_MAP()

public:
    void DisplayImage(bool isZmap);

    void SaveHeightDataInROI();

    Ipvm::Rect32s m_roi;

    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnUpdate3DView(WPARAM, LPARAM);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};
