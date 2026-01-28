#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ComponentAlign.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CComponentAlign;
class CDlgHistogramView;
struct sPassive_InfoDB;

#define MAX_HISTOGRMA_VIEWER 8
//HDR_6_________________________________ Header body
//
class CDlgComponentAlign : public CDialog
{
    DECLARE_DYNAMIC(CDlgComponentAlign)

public:
    CDlgComponentAlign(CComponentAlign* pComponentAlign, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgComponentAlign();

    CComponentAlign* m_pCompAlign;
    CDlgHistogramView* m_pDlgHistoViewer[MAX_HISTOGRMA_VIEWER];

    enum
    {
        IDD = IDD_DLG_COMPONENT_ALIGN
    };

    void MakeHistogram(BYTE* pbySrc, long* pnHistogram, long nSizeX, const Ipvm::Rect32s& rtROI);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    CString GetValueString(float value);
    ComponentDebugResult m_debugResult;
    CComboBox m_cmbDebugResult;
    CComboBox m_cmbInsp;

    afx_msg void OnBnClickedBtnTeach();

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

    CXTPPropertyGrid* m_propertyGrid;
    void SetPropertyGrid(sPassive_InfoDB* i_pPassiveInfoDB);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

    afx_msg void OnCbnSelchangeComboDebugResult();
    afx_msg void OnBnClickedBtnViewResult();
    afx_msg void OnCbnSelchangeComboInspection();
    afx_msg void OnBnClickedBtnViewHistogram();

    afx_msg LRESULT OnDrawHistogram(WPARAM wParam, LPARAM lParam);
    afx_msg void OnCbnSelchangeComboMakeland();

private:
    void ResultText(long nInsp, BOOL bOverlay);
    void SetCurrentComboDebugResult(long nSetCurSelVal);
    void SetCurrentComboInspection(long nSetCurIdx);
    void ShowOverlay(long i_nCuridx);
};
