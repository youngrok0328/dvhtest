#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum enumFrameTypeForNGRV
{
    NORMAL_FRAME = 0,
    REVERSE_FRAME,
    UV_FRAME,
    IR_FRAME,
};

enum enumNGRVFrameNum
{
    NORMAL_FRAME_NUM = 7,
    REVERSE_FRAME_NUM,
    UV_FRAME_NUM,
    IR_FRAME_NUM,
};

class IllumControlNGRV : public CButton
{
    DECLARE_DYNAMIC(IllumControlNGRV)

public:
    typedef void (*CALLBACK_CHANGED_SELECT_FRAME)(LPVOID userData, long realFrameIndex);
    typedef void (*CALLBACK_CHANGED_SELECT_ITEM)(LPVOID userData, long itemID);
    typedef void (*CALLBACK_CHANGED_ITEM_FRAME)(LPVOID userData, long itemID, long realFrameIndex, bool checked);
    typedef void (*CALLBACK_DELETED_FRAME)(LPVOID userData, long realFrameIndex, bool isBackup);
    typedef void (*CALLBACK_INSERTED_FRAME)(LPVOID userData, long groupID, long realFrameIndex, bool isBackup);

    IllumControlNGRV(long maxFrameCount);
    virtual ~IllumControlNGRV();

    void resetNGRVData();

    bool addFrameGroup(long groupID, LPCTSTR name, COLORREF backgroundColor, COLORREF fontColor);
    bool setFrameGroup(long frameIndex, long groupID);

    bool addItem(long itemID, LPCTSTR name, bool onlyOneFrameCanBeUsed);
    bool addItemFrame(long itemID, long frameIndex);

    long getItemCount() const;
    void setEditMode(bool enabled);

    void setCallback_changedSelectFrame(LPVOID userData, CALLBACK_CHANGED_SELECT_FRAME function);
    void setCallback_changedSelectItem(LPVOID userData, CALLBACK_CHANGED_SELECT_ITEM function);
    void setCallback_changedItemFrame(LPVOID userData, CALLBACK_CHANGED_ITEM_FRAME function);
    void setCallback_deletedFrame(LPVOID userData, CALLBACK_DELETED_FRAME function);
    void setCallback_insertedFrame(LPVOID userData, CALLBACK_INSERTED_FRAME function);

protected:
    float m_designFactor;
    long m_design_illumHeadHeight;
    long m_design_rowHeight;

    long m_maxFrameCount;
    long m_selectItemID;
    long m_selectRealFrameIndex;
    bool m_isEditMode;

    long m_dragGroupIndex;
    long m_dragFrameIndex;
    CPoint m_dragMousePoint;
    bool m_dragAction;

    struct SFrameGroup
    {
        CRect m_roi;
        CString m_name;
        COLORREF m_fontColor;
        COLORREF m_backgroundColor;
        std::vector<long> m_frames;
        std::vector<CRect> m_frames_Rect;
    };

    struct SItemFrameInfo
    {
        bool m_checked;
        CRect m_roi;
    };

    class ItemInfo
    {
    public:
        CString m_name;
        CRect m_name_Rect;
        bool m_onlyOneFrameCanBeUsed;

        void initFrame(long maxFrameCount);
        long getFrameCount() const;
        long getCheckedFrameCount() const;
        SItemFrameInfo& getFrame(long index);

        void checkFrame(long frameIndex, bool enabled);
        void incFrameIndex(long baseFrameIndex);
        void eraseFrame(long frameIndex, bool frameShift);
        long getFirstFrameIndex() const;

    private:
        std::vector<SItemFrameInfo> m_useFrames;
    };

    std::map<long, SFrameGroup> m_frameGroups;
    std::map<long, ItemInfo> m_itemInfos;
    long m_realFrameCount;
    std::map<long, BOOL> m_backupFrame;

    std::pair<LPVOID, CALLBACK_CHANGED_SELECT_FRAME> m_callback_changedSelectFrame;
    std::pair<LPVOID, CALLBACK_CHANGED_SELECT_ITEM> m_callback_changedSelectItem;
    std::pair<LPVOID, CALLBACK_CHANGED_ITEM_FRAME> m_callback_changedItemFrame;
    std::pair<LPVOID, CALLBACK_DELETED_FRAME> m_callback_deletedFrame;
    std::pair<LPVOID, CALLBACK_INSERTED_FRAME> m_callback_insertedFrame;

    void setEvent_changedSelectItem(long itemID);
    void setEvent_changedSelectFrame(long realFrameIndex);
    void setEvent_changedItemFrame(long itemID, long realFrameIndex, bool enabled);
    void setEvent_deletedFrame(long groupIndex, long realFrameIndex, bool isBackup);
    void setEvent_insertFrame(long groupIndex, long realFrameIndex, bool isBackup);

    long findFrameGroupID(long frameIndex);
    CRect getFrameRegion(long frameIndex, bool isColumnOnly);
    void checkFrameRegion(const CPoint& mousePos, long& o_groupIndex, long& o_frameIndex, bool checkOnlyX);
    void drawRowText(
        CDC& dc, long startOffsetX, long startOffsetY, long charSizeX, long charStepY, COLORREF color, LPCTSTR text);
    void drawAlphaRectangle(CDC& dc, const CRect& region, COLORREF color, BYTE alphaBlend);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnPaint();
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};