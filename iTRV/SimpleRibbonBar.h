#pragma once
//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class SimpleRibbonBar : public CStatic
{
    DECLARE_DYNAMIC(SimpleRibbonBar)

public:
    ~SimpleRibbonBar() final;

    bool Create(CWnd* parent);
    bool LoadResourceBitmap(UINT resourceID, const wchar_t* resourceType = L"PNG");

    int32_t AddGroup(const wchar_t* groupName);

    bool AddButton(int32_t groupId, int32_t imageId, const wchar_t* name, int32_t width, int32_t id);
    void CheckButtons();
    void StartCheckButtonTimer(int32_t intervalMs);
    void StopCheckButtonTimer();

    void SetCallbackButtonCheck(std::function<void(int32_t, bool&)> callback);
    void SetCallbackButtonClicked(std::function<void(int32_t)> callback);

protected:
    Gdiplus::Bitmap* m_resourceBitmap = nullptr;

    class ButtonInfo
    {
    public:
        CRect m_position;

        std::wstring m_name;
        bool m_enabled = true;
        int32_t m_width;
        int32_t m_imageId;
        int32_t m_id;
    };

    class GroupInfo
    {
    public:
        CRect m_position;

        std::wstring m_name;
        std::vector<std::shared_ptr<ButtonInfo>> m_buttons;
    };

    std::vector<std::shared_ptr<GroupInfo>> m_groups;
    ButtonInfo* m_activatedButton = nullptr;
    ButtonInfo* m_clickedButton = nullptr;

    std::function<void(int32_t, bool&)> m_callbackButtonCheck;
    std::function<void(int32_t)> m_callbackButtonClicked;
    CBitmap m_layerBitmap;

    void CreateLayerBitmap();
    void DrawButton(CDC& dc, ButtonInfo& button);
    void DrawResourceWithAlpha(CDC& dc, const CRect& targetRect, int32_t imageID, float alpha = 1.f);

    ButtonInfo* FindButton(CPoint point);
    void UpdateLayout();
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

    DECLARE_MESSAGE_MAP()
};
