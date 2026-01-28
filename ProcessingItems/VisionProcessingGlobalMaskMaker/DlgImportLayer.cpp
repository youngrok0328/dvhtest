//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgImportLayer.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingGlobalMaskMaker.h"
#include "VisionProcessingGlobalMaskMakerPara.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <afxdialogex.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgImportLayer, CDialogEx)

DlgImportLayer::DlgImportLayer(CVisionProcessingGlobalMaskMakerPara& para,
    CVisionProcessingGlobalMaskMakerPara& importPara, CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_IMPORT_LAYER, pParent)
    , m_para(para)
    , m_importPara(importPara)
    , m_layerList(MaskListStyle::AllItemFixed | MaskListStyle::MultiSelectItems)
{
}

DlgImportLayer::~DlgImportLayer()
{
}

void DlgImportLayer::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_IMPORT_TYPE, m_cmbImportType);
    DDX_Control(pDX, IDC_LAYERLIST, m_layerList);
}

BEGIN_MESSAGE_MAP(DlgImportLayer, CDialogEx)
ON_CBN_SELCHANGE(IDC_COMBO_IMPORT_TYPE, &DlgImportLayer::OnCbnSelchangeComboImportType)
END_MESSAGE_MAP()

// DlgImportLayer message handlers

BOOL DlgImportLayer::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_cmbImportType.AddString(_T("Overwrite all layers"));
    m_cmbImportType.AddString(_T("Replace and Append"));
    m_cmbImportType.SetCurSel(0);

    std::map<CString, bool> oldLayerList;
    for (long layerIndex = 0; layerIndex < m_para.getLayerCount(); layerIndex++)
    {
        oldLayerList[m_para.getLayer(layerIndex).m_name] = true;
    }

    for (long layerIndex = 0; layerIndex < m_importPara.getLayerCount(); layerIndex++)
    {
        CString name = m_importPara.getLayer(layerIndex).m_name;
        CString state;
        if (oldLayerList.find(name) != oldLayerList.end())
        {
            state = _T("!");
        }
        m_layerList.AddItem(layerIndex, name, state);

        name.Empty();
        state.Empty();
    }

    m_layerList.EnableWindow(FALSE);

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgImportLayer::OnOK()
{
    CDialogEx::OnOK();
    std::vector<long> selectedIndexList;

    if (m_cmbImportType.GetCurSel() == 0)
    {
        m_para.Init();
        for (long index = 0; index < m_importPara.getLayerCount(); index++)
        {
            selectedIndexList.push_back(index);
        }
    }
    else
    {
        selectedIndexList = m_layerList.GetSelectedMultiItemsKey();
    }

    for (long index = 0; index < m_importPara.getLayerCount(); index++)
    {
        if (std::find(selectedIndexList.begin(), selectedIndexList.end(), index) == selectedIndexList.end())
        {
            continue;
        }

        auto& importLayer = m_importPara.getLayer(index);

        CiDataBase layerDB;
        importLayer.LinkDataBase(TRUE, layerDB);

        CString name = importLayer.m_name;
        long oldLayerIndex = m_para.getLayerIndex(name);

        SpecLayer* targetLayer = nullptr;

        if (oldLayerIndex < 0)
        {
            if (!m_para.addLayer(m_para.getLayerCount(), name))
            {
                ASSERT(!_T("??"));
                continue;
            }

            oldLayerIndex = m_para.getLayerIndex(name);
        }

        if (oldLayerIndex >= 0)
        {
            m_para.getLayer(oldLayerIndex).Init();
            targetLayer = &m_para.getLayer(oldLayerIndex);
        }
        else
        {
            ASSERT(!_T("??"));
            continue;
        }

        targetLayer->LinkDataBase(FALSE, layerDB);

        name.Empty();
    }
}

void DlgImportLayer::OnCbnSelchangeComboImportType()
{
    m_layerList.EnableWindow(m_cmbImportType.GetCurSel() == 1);
}
