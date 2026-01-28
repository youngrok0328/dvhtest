#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "DrawViewTape.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/Component.h"
#include "../../InformationModule/dPI_DataBase/CustomPolygon.h"
#include "../../InformationModule/dPI_DataBase/DMSArrayChip.h"
#include "../../InformationModule/dPI_DataBase/DMSBall.h"
#include "../../InformationModule/dPI_DataBase/DMSChip.h"
#include "../../InformationModule/dPI_DataBase/DMSHeatsink.h"
#include "../../InformationModule/dPI_DataBase/DMSPassiveChip.h"
#include "../../InformationModule/dPI_DataBase/DMSPatch.h"
#include "../../InformationModule/dPI_DataBase/Land.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/Pad.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionProcessing;
class VisionProcessingSpec;
class VisionMapDataEditorUI;
class VisionTapeSpec;
struct ProcessingDlgInfo;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
class DlgVisionProcessingSpecTR : public CDialog
{
    DECLARE_DYNAMIC(DlgVisionProcessingSpecTR)

public:
    DlgVisionProcessingSpecTR(const ProcessingDlgInfo& procDlgInfo, VisionProcessingSpec* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~DlgVisionProcessingSpecTR();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_TR
    };

protected:
    void Update_Warning();

    void UpdatePropertyGrid();
    CXTPPropertyGridItem* AddPropertyFloat(CXTPPropertyGridItem* parent, const wchar_t* name, float& value);

    CPackageSpec& m_packageSpec;
    VisionTapeSpec& m_tapeSpec;
    DrawViewTape m_drawView;

    void propertyButton_BodySize();
    void RefreshSpecLayout();
    void RefreshView();

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    void FireExchanged();

    DECLARE_MESSAGE_MAP()

public:
    VisionMapDataEditorUI* m_pVisionEditorDlg;
    const ProcessingDlgInfo& m_procDlgInfo;
    VisionProcessingSpec* m_pVisionInsp;
    CXTPPropertyGrid* m_propertyGrid;
    CStatic m_stcViewer;

    CStatic m_staticTapeLayoutBoth;
    CStatic m_staticTapeLayoutLeft;
    CStatic m_staticTapeLayoutRight;

    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
};