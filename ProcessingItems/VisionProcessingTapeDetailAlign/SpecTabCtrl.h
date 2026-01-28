#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageProcPara;
class VisionInspFrameIndex;
class VisionProcessingTapeDetailAlign;

//HDR_6_________________________________ Header body
//
class SpecTabCtrl : public CTabCtrl
{
public:
    SpecTabCtrl(VisionProcessingTapeDetailAlign& processor);
    bool Create(const RECT& rect, CWnd* parent);
    void UpdatePropertyGrid();

private:
    VisionProcessingTapeDetailAlign& m_processor;
    CXTPPropertyGrid m_propertySprocketHole;
    CXTPPropertyGrid m_propertyPocket;
    CXTPPropertyGrid m_propertyDevice;

    void CallImageCombine(VisionInspFrameIndex& imageFrameIndex, ImageProcPara& imageProcPara);

    void UpdatePropertySprocketHole(CXTPPropertyGrid& propertyGrid);
    void UpdatePropertyPocket(CXTPPropertyGrid& propertyGrid);
    void UpdatePropertyDevice(CXTPPropertyGrid& propertyGrid);

    DECLARE_MESSAGE_MAP()
    afx_msg void OnDestroy();
    afx_msg void OnTcnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
};
