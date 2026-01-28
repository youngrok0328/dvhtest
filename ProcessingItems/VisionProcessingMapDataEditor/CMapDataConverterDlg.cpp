//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CMapDataConverterDlg.h"

//CPP_2_________________________________ This project's headers
#include "BallDataChangerDlg.h"
#include "CompDataChangerDlg.h"
#include "LandGroupIdChangerDlg.h"
#include "Markup.h"
#include "PadTypeChangerDlg.h"
#include "VisionProcessingMapDataEditor.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Widget/ImageView.h>
#include <atlconv.h>

//CPP_5_________________________________ Standard library headers
#include <fstream>
#include <regex>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define IDC_FRAME_PROPERTY_GRID2 101
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)
#define PROPERTY_GRID_ITEM_FIRST_ID 30

//CPP_7_________________________________ Implementation body
//
enum MapDataType
{
    TYPE_PAD,
    TYPE_LAND,
    TYPE_BALL,
    TYPE_COMPONENT,
    TYPE_ALL,
};

enum PropertyGridItemID
{
    ITEM_ID_MAPDATA_TYPE = 1,
    ITEM_ID_GROUPING,
    ITEM_ID_IMPORT_XML,
    ITEM_ID_EXPORT_CSV,
    ITEM_ID_CONVERT,
    ITEM_ID_CONVERT_LOAD,
    ITEM_ID_CONVERT_CLEAR,
    ITEM_ID_CONVERT_CANCEL,
    ITEM_ID_MAPDATA_PREVIEW,
};

IMPLEMENT_DYNAMIC(CMapDataConverterDlg, CDialog)

CMapDataConverterDlg::CMapDataConverterDlg(CWnd* pParent /*=nullptr*/, CPackageSpec* pSpec,
    std::vector<std::pair<long, CString>>& convMapData, BOOL modifyMode)
    : CDialog(IDD_DLG_MAPDATA_CONVERTER, pParent)
    , m_propertyGrid(nullptr)
    , m_pGridMapDataType(nullptr)
    , m_MapDataType(MapDataType::TYPE_ALL)
    , m_PreviewDataType(MapDataType::TYPE_ALL)
    , m_packageSpec(*pSpec)
    , m_nAutoGroup(0)
    , m_fZoomingValue(0.)
    , m_bInputGridMode(false)
    , m_nSelectedLayer(0)
    , m_nSelectedGroup(-1)
    , convMapData(convMapData)
    , m_bReEditMode(modifyMode)
    , m_fLandHeight(0.)
    , m_nGroupIdx(0)
    , m_bVerifyMode(FALSE)
{
}

CMapDataConverterDlg::~CMapDataConverterDlg()
{
    delete m_propertyGrid;
    delete m_pGridMapDataType;
}

void CMapDataConverterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STC_VIEWER, m_stcViewer);
}

BEGIN_MESSAGE_MAP(CMapDataConverterDlg, CDialog)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &CMapDataConverterDlg::OnGridNotify)
ON_WM_PAINT()
ON_WM_LBUTTONDOWN()
ON_WM_RBUTTONUP()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_COMMAND(ID_COMPONENTMENU_EDITDIMENSIONS, &CMapDataConverterDlg::OnComponentmenuEditdimensions)
ON_COMMAND(ID_GROUPMENU_MAKEGROUP, &CMapDataConverterDlg::OnGroupmenuMakegroup)
ON_COMMAND(ID_COMPONENTMENU_REMOVEGROUP, &CMapDataConverterDlg::OnComponentmenuRemovegroup)
ON_COMMAND(ID_COMPONENTMENU_SELECTALL, &CMapDataConverterDlg::OnComponentmenuSelectall)
ON_COMMAND(ID_COMPONENTMENU_UNSELECTALL, &CMapDataConverterDlg::OnComponentmenuUnselectall)
ON_COMMAND(ID_PADMENU_CHANGETYPE, &CMapDataConverterDlg::OnPadmenuChangetype)
ON_COMMAND(ID_LANDMENU_CHANGEGROUPID, &CMapDataConverterDlg::OnLandmenuChangegroupid)
ON_COMMAND(ID_PADMENU_SELECTALL, &CMapDataConverterDlg::OnPadmenuSelectall)
ON_COMMAND(ID_PADMENU_UNSELECTALL, &CMapDataConverterDlg::OnPadmenuUnselectall)
ON_COMMAND(ID_LANDMENU_SELECTALL, &CMapDataConverterDlg::OnLandmenuSelectall)
ON_COMMAND(ID_LANDMENU_UNSELECTALL, &CMapDataConverterDlg::OnLandmenuUnselectall)
ON_COMMAND(ID_BALLMENU_BALLSPEC, &CMapDataConverterDlg::OnBallmenuBallspec)
ON_COMMAND(ID_BALLMENU_SELECTALL, &CMapDataConverterDlg::OnBallmenuSelectall)
ON_COMMAND(ID_BALLMENU_UNSELECTALL, &CMapDataConverterDlg::OnBallmenuUnselectall)
ON_COMMAND(ID_PADMENU_DELETE, &CMapDataConverterDlg::OnPadmenuDelete)
ON_COMMAND(ID_COMPONENTMENU_DELETE, &CMapDataConverterDlg::OnComponentmenuDelete)
ON_COMMAND(ID_LANDMENU_DELETE, &CMapDataConverterDlg::OnLandmenuDelete)
ON_COMMAND(ID_BALLMENU_DELTE, &CMapDataConverterDlg::OnBallmenuDelte)
ON_COMMAND(ID_OTHER_DELETE, &CMapDataConverterDlg::OnOtherDelete)
ON_COMMAND(ID_COMPONENTMENU_ADD, &CMapDataConverterDlg::OnComponentmenuAdd)
ON_COMMAND(ID_PREVMENU_EDITDIMENSIONS, &CMapDataConverterDlg::OnPrevmenuEditdimensions)
ON_COMMAND(ID_PREVMENU_SELECTALL, &CMapDataConverterDlg::OnPrevmenuSelectall)
ON_COMMAND(ID_PREVMENU_UNSELECTALL, &CMapDataConverterDlg::OnPrevmenuUnselectall)
ON_COMMAND(ID_PREVMENU_ADD, &CMapDataConverterDlg::OnPrevmenuAdd)
ON_COMMAND(ID_PREVMENU_DELETE, &CMapDataConverterDlg::OnPrevmenuDelete)
END_MESSAGE_MAP()

// CMapDataConverterDlg 메시지 처리기

BOOL CMapDataConverterDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CStatic* rtStaticGrid = (CStatic*)GetDlgItem(IDC_ST_GRID);
    CStatic* rtStaticGrid2 = (CStatic*)GetDlgItem(IDC_ST_GRID2);
    CStatic* rtStaticViewer = (CStatic*)GetDlgItem(IDC_STC_VIEWER);

    CRect rtGrid1;
    CRect rtGrid2;
    CRect rtViewer;

    rtStaticGrid->GetClientRect(rtGrid1);
    rtStaticGrid2->GetClientRect(rtGrid2);
    rtStaticViewer->GetClientRect(rtViewer);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtGrid1, this, IDC_FRAME_PROPERTY_GRID);

    rtGrid2.top = rtGrid1.bottom + 5;
    rtGrid2.bottom = rtGrid2.bottom + rtGrid1.bottom;
    m_pGridMapDataType = new CXTPPropertyGrid;
    m_pGridMapDataType->Create(rtGrid2, this, IDC_FRAME_PROPERTY_GRID2);

    rtViewer.left = rtViewer.left + rtGrid1.right + 7;
    rtViewer.right = rtViewer.right + rtGrid1.right + 7;
    m_stcViewer.MoveWindow(rtViewer);

    SetPropertyGrid();
    SetPropertyGridMapData();

    if (m_bReEditMode && ::FILEEXISTS_S(DynamicSystemPath::get(DefineFolder::Temp) + _T("TempMapDataConverter.xml")))
        ReEdit_LoadXMLModifyData();

    m_propertyGrid->Refresh();

    Initialize_View();
    Update_Background((long)m_MapDataType);
    Update_Foreground((long)m_MapDataType);

    Invalidate(FALSE);

    return TRUE; // return TRUE  unless you set the focus to a control
}

INT_PTR CMapDataConverterDlg::DoModal()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return CDialog::DoModal();
}

void CMapDataConverterDlg::SetPropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Map Data Editor")))
    {
        if (auto* pItem = category->AddChildItem(new CXTPPropertyGridItem(_T("Import XML file"))))
        {
            pItem->SetFlags(xtpGridItemHasEdit | xtpGridItemHasExpandButton);
            pItem->SetID(ITEM_ID_IMPORT_XML);
        }

        category->AddChildItem(new CCustomItemButton(_T("Append and exit"), TRUE, FALSE))->SetID(ITEM_ID_CONVERT_LOAD);
        category->AddChildItem(new CCustomItemButton(_T("Convert to CSV"), TRUE, FALSE))->SetID(ITEM_ID_CONVERT);
        category->AddChildItem(new CCustomItemButton(_T("Clear map data"), TRUE, FALSE))->SetID(ITEM_ID_CONVERT_CLEAR);
        category->AddChildItem(new CCustomItemButton(_T("Cancel"), TRUE, FALSE))->SetID(ITEM_ID_CONVERT_CANCEL);

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Map Data")))
    {
        auto* item_rotate
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("MapData Layout"), long(m_MapDataType)));
        item_rotate->GetConstraints()->AddConstraint(_T("All"), long(MapDataType::TYPE_ALL));
        item_rotate->GetConstraints()->AddConstraint(_T("Pad"), long(MapDataType::TYPE_PAD));
        item_rotate->GetConstraints()->AddConstraint(_T("Land"), long(MapDataType::TYPE_LAND));
        item_rotate->GetConstraints()->AddConstraint(_T("Ball"), long(MapDataType::TYPE_BALL));
        item_rotate->GetConstraints()->AddConstraint(_T("Comp"), long(MapDataType::TYPE_COMPONENT));
        item_rotate->SetID(ITEM_ID_MAPDATA_TYPE);

        auto* item_grouping = category->AddChildItem(
            new CXTPPropertyGridItemEnum(_T("Auto Grouping ( Component )"), m_nAutoGroup, &m_nAutoGroup));
        item_grouping->GetConstraints()->AddConstraint(_T("Disable"), 0);
        item_grouping->GetConstraints()->AddConstraint(_T("Enable"), 1);
        item_grouping->SetID(ITEM_ID_GROUPING);

        category->Expand();
    }

    m_propertyGrid->ShowHelp(FALSE);
    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

void CMapDataConverterDlg::SetPropertyGridMapData()
{
    m_pGridMapDataType->ResetContent();

    if (auto* category = m_pGridMapDataType->AddCategory(_T("Map Data Types")))
    {
        category->Expand();
    }

    m_pGridMapDataType->ShowHelp(FALSE);
    m_pGridMapDataType->SetViewDivider(0.60);
    m_pGridMapDataType->HighlightChangedItems(TRUE);
}

void CMapDataConverterDlg::UpdatePropertyGridMapData()
{
    m_pGridMapDataType->ResetContent();

    if (auto* category = m_pGridMapDataType->AddCategory(_T("Map Data Types")))
    {
        for (int index = 0; index < m_vecstShapeInfo.size(); index++)
        {
            auto* item_rotate = category->AddChildItem(new CXTPPropertyGridItemEnum(
                m_vecst_XML_PAS[index].strName, long(m_vecstShapeInfo[index].nGerberType)));
            item_rotate->GetConstraints()->AddConstraint(_T("Pad"), long(TYPE_PAD));
            item_rotate->GetConstraints()->AddConstraint(_T("Ball"), long(TYPE_BALL));
            item_rotate->GetConstraints()->AddConstraint(_T("Land"), long(TYPE_LAND));
            item_rotate->GetConstraints()->AddConstraint(_T("Comp"), long(TYPE_COMPONENT));
            item_rotate->SetID(PROPERTY_GRID_ITEM_FIRST_ID + index);
        }

        category->Expand();
    }

    if (auto* category = m_pGridMapDataType->AddCategory(_T("Verify Map Data")))
    {
        auto* item_rotate
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Map Data Layout"), long(m_PreviewDataType)));
        item_rotate->GetConstraints()->AddConstraint(_T("All"), long(MapDataType::TYPE_ALL));
        item_rotate->GetConstraints()->AddConstraint(_T("Pad"), long(MapDataType::TYPE_PAD));
        item_rotate->GetConstraints()->AddConstraint(_T("Land"), long(MapDataType::TYPE_LAND));
        item_rotate->GetConstraints()->AddConstraint(_T("Ball"), long(MapDataType::TYPE_BALL));
        item_rotate->GetConstraints()->AddConstraint(_T("Comp"), long(MapDataType::TYPE_COMPONENT));
        item_rotate->SetID(ITEM_ID_MAPDATA_PREVIEW);

        category->Expand();
    }

    m_pGridMapDataType->Refresh();
}

LRESULT CMapDataConverterDlg::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam == XTP_PGN_INPLACEBUTTONDOWN)
    {
        CXTPPropertyGridInplaceButton* pButton = (CXTPPropertyGridInplaceButton*)lparam;
        if (pButton->GetItem()->GetID() == ITEM_ID_IMPORT_XML)
        {
            ImportXML();
            UpdatePropertyGridMapData();
            Calc_DrawingPosition_Ball();
            Calc_DrawingPosition_Land();
            Calc_DrawingPosition_Comp();
            Calc_DrawingPosition_Pad();

            if (m_nAutoGroup == 1)
                CreateComponent_Auto();

            Update_Background(long(m_MapDataType));
            Update_Foreground(long(m_MapDataType));

            Invalidate(FALSE);

            m_propertyGrid->Refresh();

            return TRUE;
        }
    }

    if (wparam == XTP_PGN_SELECTION_CHANGED)
    {
        CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

        if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
        {
            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
                m_vecstObjectInfo[n].bSelected = false;

            int data = value->GetEnum();
            data = (int)max(data, 0);

            m_bVerifyMode = FALSE;
            m_nSelectedGroup = -1;

            if (item->GetID() == ITEM_ID_MAPDATA_TYPE)
            {
                Update_Background(data);
                Update_Foreground(data);
                Invalidate(FALSE);
            }

            if (item->GetID() == ITEM_ID_MAPDATA_PREVIEW)
            {
                m_bVerifyMode = TRUE;
                Update_Background(data);
                Update_Foreground(data);
                Invalidate(FALSE);
            }

            for (int index = 0; index < m_vecstShapeInfo.size(); index++)
            {
                if (item->GetID() == CAST_UINT(PROPERTY_GRID_ITEM_FIRST_ID + index))
                {
                    if (data == TYPE_BALL)
                        Calc_DrawingPosition_Ball();
                    else if (data == TYPE_LAND)
                        Calc_DrawingPosition_Land();
                    else if (data == TYPE_COMPONENT)
                        Calc_DrawingPosition_Comp();
                    else if (data == TYPE_PAD)
                        Calc_DrawingPosition_Pad();

                    Update_Background(data);
                    Update_Foreground(data, index);
                    Invalidate(FALSE);
                }
            }
        }
    }

    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_CONVERT:
                if (ConvertToCSV())
                {
                    ReEdit_SaveXMLModifyData();
                    AfxMessageBox(_T("Complete"));
                }
                break;
            case ITEM_ID_CONVERT_LOAD:
                if (ConvertToCSV())
                {
                    ReEdit_SaveXMLModifyData();
                    OnOK();
                }
                break;
            case ITEM_ID_CONVERT_CLEAR:
                if (::SimpleMessage(_T("Warning! Are you sure you want to clear?"), MB_OKCANCEL) == IDOK)
                {
                    ClearMapData();
                }
                break;
            case ITEM_ID_CONVERT_CANCEL:
                OnCancel();
                break;
        }

        return 0;
    }

    if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = value->GetEnum();
        data = (int)max(data, 0);
        switch (item->GetID())
        {
            case ITEM_ID_MAPDATA_TYPE:
                Update_Background(data);
                Update_Foreground(data);
                Invalidate(FALSE);
                break;

            case ITEM_ID_MAPDATA_PREVIEW:
                Update_Background(data);
                Update_Foreground(data);
                Invalidate(FALSE);
                break;

            case ITEM_ID_GROUPING:
                m_nAutoGroup = data;

                UpdatePropertyGridMapData();
                Calc_DrawingPosition_Ball();
                Calc_DrawingPosition_Land();
                Calc_DrawingPosition_Comp();
                Calc_DrawingPosition_Pad();

                if (m_nAutoGroup == 1)
                    CreateComponent_Auto();
                else
                    ClearAllComponents();

                Update_Background(m_nSelectedLayer);
                Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);

                Invalidate(FALSE);
                break;
        }

        for (int index = 0; index < m_vecstShapeInfo.size(); index++)
        {
            if (item->GetID() == CAST_UINT(PROPERTY_GRID_ITEM_FIRST_ID + index))
            {
                m_vecstShapeInfo[index].nGerberType = data;

                if (data == TYPE_BALL)
                    Calc_DrawingPosition_Ball();
                else if (data == TYPE_LAND)
                    Calc_DrawingPosition_Land();
                else if (data == TYPE_COMPONENT)
                    Calc_DrawingPosition_Comp();
                else if (data == TYPE_PAD)
                    Calc_DrawingPosition_Pad();

                Update_Background(data);
                Update_Foreground(data, m_nSelectedGroup);
                Invalidate(FALSE);

                if (data == TYPE_LAND)
                {
                    long GroupNo = -1;
                    long nLandCnt = 0;

                    for (int i = 0; i < m_vecstObjectInfo.size(); i++)
                    {
                        if (m_vecstObjectInfo[i].nShapeNumber == 0)
                            continue;

                        if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_LAND)
                        {
                            m_fLandHeight = m_vecstObjectInfo[i].fTickness;
                            nLandCnt++;
                            break;
                        }
                    }

                    if (nLandCnt != 0)
                    {
                        CLandGroupIdChangerDlg c_landGroupIDChanger(this, GroupNo, m_fLandHeight);
                        if (c_landGroupIDChanger.DoModal() == IDOK)
                        {
                            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
                            {
                                if (m_vecstObjectInfo[n].nShapeNumber == 0)
                                    continue;

                                if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_LAND)
                                {
                                    m_vecstObjectInfo[n].fTickness = (float)m_fLandHeight;
                                }
                            }
                        }
                    }
                }
                else if (data == TYPE_BALL)
                {
                    long nBallCnt = 0;

                    for (int i = 0; i < m_vecstObjectInfo.size(); i++)
                    {
                        if (m_vecstObjectInfo[i].nShapeNumber == 0)
                            continue;

                        if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_BALL)
                        {
                            nBallCnt++;
                            m_fBallHeight = m_vecstObjectInfo[i].fTickness;
                            break;
                        }
                    }

                    if (nBallCnt != 0)
                    {
                        CBallDataChangerDlg c_BallDataChanger(this, m_fBallHeight);
                        if (c_BallDataChanger.DoModal() == IDOK)
                        {
                            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
                            {
                                if (m_vecstObjectInfo[n].nShapeNumber == 0)
                                    continue;

                                if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_BALL)
                                {
                                    m_vecstObjectInfo[n].fTickness = (float)m_fBallHeight;
                                }
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }
    return 0;
}

void CMapDataConverterDlg::Initialize_View()
{
    m_stcViewer.GetWindowRect(m_rtView);

    m_rtView.OffsetRect(-m_rtView.left, -m_rtView.top);

    // 그려질 디바이스 크기 결정
    m_fViewAspectRatio = float(m_rtView.Width()) / m_rtView.Height();

    if (m_packageSpec.m_bodyInfoMaster->fBodySizeX > m_packageSpec.m_bodyInfoMaster->fBodySizeY)
        m_fDeviceAspectRatio = m_packageSpec.m_bodyInfoMaster->fBodySizeX / m_packageSpec.m_bodyInfoMaster->fBodySizeY;
    else
        m_fDeviceAspectRatio = m_packageSpec.m_bodyInfoMaster->fBodySizeY / m_packageSpec.m_bodyInfoMaster->fBodySizeX;

    long nMargin = 80;
    if (m_fViewAspectRatio >= m_fDeviceAspectRatio)
    {
        float fDeviceHalfHeightPixel = 0.5f * (m_rtView.Height() - nMargin);
        float fDeviceHalfWidthPixel = 0.5f * (m_rtView.Height() - nMargin) * m_fDeviceAspectRatio;

        m_rtDevice.left = long(m_rtView.CenterPoint().x - fDeviceHalfWidthPixel + 0.5f);
        m_rtDevice.right = long(m_rtView.CenterPoint().x + fDeviceHalfWidthPixel + 0.5f) + 1;
        m_rtDevice.top = long(m_rtView.CenterPoint().y - fDeviceHalfHeightPixel + 0.5f);
        m_rtDevice.bottom = long(m_rtView.CenterPoint().y + fDeviceHalfHeightPixel + 0.5f) + 1;
    }
    else
    {
        float fDeviceHalfHeightPixel = 0.5f * (m_rtView.Width() - nMargin) / m_fDeviceAspectRatio;
        float fDeviceHalfWidthPixel = 0.5f * (m_rtView.Width() - nMargin);

        m_rtDevice.left = long(m_rtView.CenterPoint().x - fDeviceHalfWidthPixel + 0.5f);
        m_rtDevice.right = long(m_rtView.CenterPoint().x + fDeviceHalfWidthPixel + 0.5f) + 1;
        m_rtDevice.top = long(m_rtView.CenterPoint().y - fDeviceHalfHeightPixel + 0.5f);
        m_rtDevice.bottom = long(m_rtView.CenterPoint().y + fDeviceHalfHeightPixel + 0.5f) + 1;
    }

    if (m_packageSpec.m_bodyInfoMaster->fBodySizeX > m_packageSpec.m_bodyInfoMaster->fBodySizeY)
    {
        m_fZoom = m_rtDevice.Width() / (m_packageSpec.m_bodyInfoMaster->fBodySizeX);
        m_fZoom = m_rtDevice.Height() / (m_packageSpec.m_bodyInfoMaster->fBodySizeY);
    }
    else
    {
        m_fZoom = m_rtDevice.Width() / (m_packageSpec.m_bodyInfoMaster->fBodySizeY);
        m_fZoom = m_rtDevice.Height() / (m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    }
}

void CMapDataConverterDlg::Calc_DrawingPosition_Land()
{
    long nDataCount = (long)m_vecstObjectInfo.size();

    if (nDataCount <= 0)
        return;

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    Ipvm::Rect32s rtROI;
    Ipvm::Rect32r tmp_sf_Roi;
    float fOffsetX(0.f), fOffsetY(0.f);
    float fWidth(0.f), fLength(0.f), fAngle(0.f);

    for (long n = 0; n < nDataCount; n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_LAND)
        {
            float fPosX = 0.;
            float fPosY = 0.;

            GetPosition(m_vecstObjectInfo[n].fPosX, m_vecstObjectInfo[n].fPosY, fPosX, fPosY);

            fOffsetX = fGridOriginX + fPosX * m_fZoom;
            fOffsetY = fGridOriginY - fPosY * m_fZoom;

            fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
            fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;

            fAngle = (float)(abs(m_vecstObjectInfo[n].fRotation
                                 - m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation)
                + 180);

            rtROI.m_left = (long)(fOffsetX - fWidth + 0.5f);
            rtROI.m_right = (long)(fOffsetX + fWidth + 0.5f);
            rtROI.m_top = (long)(fOffsetY - fLength + 0.5f);
            rtROI.m_bottom = (long)(fOffsetY + fLength + 0.5f);
            m_vecstObjectInfo[n].rtROI = rtROI;

            tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
            tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
            tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
            tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);

            FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

            if (fAngle != 0)
            {
                double lfRadian = (double)fAngle * DEF_DEG_TO_RAD;

                // 시계 반대 방향이 +
                frtChip = frtChip.Rotate(lfRadian);
            }
            m_vecstObjectInfo[n].fsrtSpecROI = frtChip;
        }
    }
}

void CMapDataConverterDlg::Calc_DrawingPosition_Ball()
{
    long nDataCount = (long)m_vecstObjectInfo.size();

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    for (long n = 0; n < nDataCount; n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_BALL)
        {
            float fPosX = 0.;
            float fPosY = 0.;

            GetPosition(m_vecstObjectInfo[n].fPosX, m_vecstObjectInfo[n].fPosY, fPosX, fPosY);

            const float fCenX = fGridOriginX + (float)(fPosX * m_fZoom);
            const float fCenY = fGridOriginY - (float)(fPosY * m_fZoom);
            const float fRadius
                = (float)(0.5 * m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom);

            m_vecstObjectInfo[n].ballPosition = CRect(long(fCenX - fRadius + 0.5f), long(fCenY - fRadius + 0.5f),
                long(fCenX + fRadius + 1.5f), long(fCenY + fRadius + 1.5f));
        }
    }
}

void CMapDataConverterDlg::Calc_DrawingPosition_Comp(bool bPreviewMode)
{
    long nDataCount = (long)m_vecstObjectInfo.size();

    if (bPreviewMode)
        nDataCount = long(m_vecCompInfo.size());

    if (nDataCount <= 0)
        return;

    Ipvm::Rect32r tmp_sf_Roi;
    float fOffsetX(0.f), fOffsetY(0.f);
    float fWidth(0.f), fLength(0.f);

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    long gIndex = 0;
    long pad1Index(0), pad2Index(0);
    float fRotation = 0;
    m_nGroupIdx = 0;

    for (long n = 0; n < nDataCount; n++)
    {
        if (!bPreviewMode)
        {
            if (m_vecstObjectInfo[n].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_COMPONENT)
            {
                if (abs(m_vecstObjectInfo[n].fRotation
                        - m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation)
                        == 180
                    || abs(m_vecstObjectInfo[n].fRotation
                           - m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation)
                        == 0)
                {
                    fRotation = 0;
                }
                else
                {
                    fRotation = 90;
                }

                if (m_nAutoGroup == 1)
                {
                    if (gIndex == 0)
                    {
                        pad1Index = n;
                        gIndex++;
                        continue;
                    }
                    else
                    {
                        pad2Index = n;
                        gIndex = 0;
                    }

                    Calc_DrawingPosition_ComponentAutoGroup(pad1Index, pad2Index, fRotation);
                }
                else
                {
                    if (m_vecstObjectInfo[n].nCompGrouping != 0)
                    {
                        Calc_DrawingPosition_ComponentAutoGroup(n);
                        continue;
                    }

                    float fPosX = 0.;
                    float fPosY = 0.;

                    GetPosition(m_vecstObjectInfo[n].fPosX, m_vecstObjectInfo[n].fPosY, fPosX, fPosY);

                    fOffsetX = fGridOriginX + fPosX * m_fZoom;
                    fOffsetY = fGridOriginY - fPosY * m_fZoom;

                    if (m_vecstObjectInfo[n].fRotation == 0 || m_vecstObjectInfo[n].fRotation == 180)
                    {
                        if (abs(m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation) == 0
                            || abs(m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation) == 180)
                        {
                            fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
                            fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
                        }
                        else
                        {
                            fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
                            fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
                        }
                    }
                    else
                    {
                        if (abs(m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation) == 0
                            || abs(m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation) == 180)
                        {
                            fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
                            fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
                        }
                        else
                        {
                            fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
                            fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
                        }
                    }

                    tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
                    tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
                    tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
                    tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);

                    FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
                        Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
                        Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
                        Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

                    m_vecstObjectInfo[n].fsrtSpecROI = frtChip; //SpecPos
                }
            }
        }
        else
        {
            fOffsetX = fGridOriginX + m_vecCompInfo[n].fOffsetX * m_fZoom;
            fOffsetY = fGridOriginY - m_vecCompInfo[n].fOffsetY * m_fZoom;

            if (m_vecCompInfo[n].nAngle == 0 || m_vecCompInfo[n].nAngle == 180)
            {
                fWidth = (m_vecCompInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
                fLength = (m_vecCompInfo[n].fLength * 1000.f) * m_fZoom * 0.5f;
            }
            else
            {
                fWidth = (m_vecCompInfo[n].fLength * 1000.f) * m_fZoom * 0.5f;
                fLength = (m_vecCompInfo[n].fWidth * 1000.f) * m_fZoom * 0.5f;
            }

            tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
            tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
            tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
            tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);

            FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

            m_vecCompInfo[n].fsrtROI = frtChip; //SpecPos
        }
    }
}

void CMapDataConverterDlg::Calc_DrawingPosition_Pad()
{
    long nDataCount = (long)m_vecstObjectInfo.size();

    if (nDataCount <= 0)
        return;

    Ipvm::Rect32s rtROI;
    Ipvm::Rect32r tmp_sf_Roi;
    float fOffsetX(0.f), fOffsetY(0.f);
    float fWidth(0.f), fLength(0.f);

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    for (long n = 0; n < nDataCount; n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_PAD)
        {
            float fPosX = 0.;
            float fPosY = 0.;

            GetPosition(m_vecstObjectInfo[n].fPosX, m_vecstObjectInfo[n].fPosY, fPosX, fPosY);

            fOffsetX = fGridOriginX + fPosX * m_fZoom;
            fOffsetY = fGridOriginY - fPosY * m_fZoom;

            if (m_vecstObjectInfo[n].nPadType == _typeRectangle)
            {
                if (m_vecstObjectInfo[n].fRotation == 0 || m_vecstObjectInfo[n].fRotation == 180)
                {
                    fWidth = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth) * m_fZoom * 0.5f;
                    fLength = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth) * m_fZoom * 0.5f;
                }
                else
                {
                    fWidth = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth) * m_fZoom * 0.5f;
                    fLength = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth) * m_fZoom * 0.5f;
                }
            }
            else if (m_vecstObjectInfo[n].nPadType == _typeCircle)
            {
                fWidth = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth) * m_fZoom * 0.5f;
                fLength = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth) * m_fZoom * 0.5f;
            }
            else
            {
                if (m_vecstObjectInfo[n].fRotation == 0 || m_vecstObjectInfo[n].fRotation == 180)
                {
                    fWidth = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth) * m_fZoom * 0.5f;
                    fLength = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth) * m_fZoom * 0.5f;
                }
                else
                {
                    fWidth = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth) * m_fZoom * 0.5f;
                    fLength = (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth) * m_fZoom * 0.5f;
                }
            }

            rtROI.m_left = (long)(fOffsetX - fWidth + 0.5f);
            rtROI.m_right = (long)(fOffsetX + fWidth + 0.5f);
            rtROI.m_top = (long)(fOffsetY - fLength + 0.5f);
            rtROI.m_bottom = (long)(fOffsetY + fLength + 0.5f);
            m_vecstObjectInfo[n].rtROI = rtROI;

            tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
            tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
            tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
            tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);

            FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

            m_vecstObjectInfo[n].fsrtSpecROI = frtChip;
            if (m_vecstObjectInfo[n].nPadType == _typePinIndex || m_vecstObjectInfo[n].nPadType == _typeTriangle)
            {
                for (long i = 0; i < 3; i++)
                    m_ptPinIndex[i] = CPoint(0, 0);
            }
        }
    }
}

void CMapDataConverterDlg::Calc_DrawingPosition_ComponentAutoGroup(long nPadIndex1, long nPadIndex2, float fRotation)
{
    Ipvm::Rect32r tmp_sf_Roi;
    Ipvm::Rect32r tmp_sf_Roi2;
    float fOffsetX(0.f), fOffsetY(0.f);
    float fWidth(0.f), fLength(0.f);

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    float tmp_fOffsetX(0.f), tmp_fOffsetY(0.f);
    float tmp_fPosX(0.f), tmp_fPosY(0.f);
    float tmp2_fPosX(0.f), tmp2_fPosY(0.f);

    float fPosX = 0.;
    float fPosY = 0.;

    float fPosX2 = 0.;
    float fPosY2 = 0.;

    GetPosition(m_vecstObjectInfo[nPadIndex1].fPosX, m_vecstObjectInfo[nPadIndex1].fPosY, fPosX, fPosY);
    GetPosition(m_vecstObjectInfo[nPadIndex2].fPosX, m_vecstObjectInfo[nPadIndex2].fPosY, fPosX2, fPosY2);

    tmp_fPosX = fPosX;
    tmp_fPosY = fPosY;

    fOffsetX = fGridOriginX + fPosX * m_fZoom;
    fOffsetY = fGridOriginY - fPosY * m_fZoom;

    if (fRotation == 0 || fRotation == 180)
    {
        fWidth = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex1].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
        fLength = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex1].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;

        tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
        tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
        tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
        tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);
    }
    else
    {
        fWidth = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex1].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
        fLength = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex1].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;

        tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
        tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
        tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);
        tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
    }

    tmp2_fPosX = fPosX2;
    tmp2_fPosY = fPosY2;

    tmp_fOffsetX = fGridOriginX + fPosX2 * m_fZoom;
    tmp_fOffsetY = fGridOriginY - fPosY2 * m_fZoom;

    if (fRotation == 0 || fRotation == 180)
    {
        fWidth = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex2].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
        fLength = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex2].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;

        tmp_sf_Roi2.m_left = (float)(tmp_fOffsetX - fWidth);
        tmp_sf_Roi2.m_right = (float)(tmp_fOffsetX + fWidth);
        tmp_sf_Roi2.m_top = (float)(tmp_fOffsetY - fLength);
        tmp_sf_Roi2.m_bottom = (float)(tmp_fOffsetY + fLength);

        if (fOffsetY > tmp_fOffsetY)
        {
            tmp_sf_Roi.m_left = tmp_sf_Roi2.m_left;
            tmp_sf_Roi.m_right = tmp_sf_Roi2.m_right;
            tmp_sf_Roi.m_top = tmp_sf_Roi2.m_top;
        }
        else
        {
            tmp_sf_Roi.m_bottom = tmp_sf_Roi2.m_bottom;
        }
    }
    else
    {
        if (abs(m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex2].nShapeNumber - 1].nOrientation) == 0
            || abs(m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex2].nShapeNumber - 1].nOrientation) == 180)
        {
            fWidth = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex2].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
            fLength = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex2].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
        }
        else
        {
            fWidth = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex2].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
            fLength = m_vecstShapeInfo[m_vecstObjectInfo[nPadIndex2].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
        }

        tmp_sf_Roi2.m_right = (float)(tmp_fOffsetX + fWidth);
        tmp_sf_Roi2.m_top = (float)(tmp_fOffsetY - fLength);
        tmp_sf_Roi2.m_bottom = (float)(tmp_fOffsetY + fLength);
        tmp_sf_Roi2.m_left = (float)(tmp_fOffsetX - fWidth);

        if (fOffsetX > tmp_fOffsetX)
        {
            tmp_sf_Roi.m_left = tmp_sf_Roi2.m_left;
        }
        else
        {
            tmp_sf_Roi.m_right = tmp_sf_Roi2.m_right;
            tmp_sf_Roi.m_top = tmp_sf_Roi2.m_top;
            tmp_sf_Roi.m_bottom = tmp_sf_Roi2.m_bottom;
        }
    }

    FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
        Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
        Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
        Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

    m_vecstObjectInfo[nPadIndex1].fsrtSpecROI = frtChip; //SpecPos
    m_vecstObjectInfo[nPadIndex2].fsrtSpecROI = frtChip; //SpecPos

    m_nGroupIdx++;
    m_vecstObjectInfo[nPadIndex1].nCompGrouping = m_nGroupIdx;
    m_vecstObjectInfo[nPadIndex2].nCompGrouping = m_nGroupIdx;
}

void CMapDataConverterDlg::Calc_DrawingPosition_ComponentAutoGroup(long nPadIndex)
{
    long compIdx = GetComponentIndexFromMountMap(nPadIndex);

    if (compIdx != -1)
    {
        m_vecstObjectInfo[nPadIndex].fsrtSpecROI = m_vecCompInfo[compIdx].fsrtRealROI; //SpecPos
    }
}

long CMapDataConverterDlg::GetGroupIndex()
{
    long n_group = 0;
    for (long n = 0; n < (long)m_vecstObjectInfo.size(); n++)
    {
        if (n_group < m_vecstObjectInfo[n].nCompGrouping)
            n_group = m_vecstObjectInfo[n].nCompGrouping;
    }

    return n_group;
}

void CMapDataConverterDlg::GetPinIndexPoint(FPI_RECT i_frtPinIndex, CPoint* o_pPinIndexPoint)
{
    long nPinIndexPos(0);

    if (m_rtDevice.CenterPoint().x < i_frtPinIndex.GetCenter().m_x
        && m_rtDevice.CenterPoint().y < i_frtPinIndex.GetCenter().m_x)
        nPinIndexPos = PinIndexPos_LT;

    else if (m_rtDevice.CenterPoint().x >= i_frtPinIndex.GetCenter().m_x
        && m_rtDevice.CenterPoint().y < i_frtPinIndex.GetCenter().m_y)
        nPinIndexPos = PinIndexPos_RT;

    if (m_rtDevice.CenterPoint().x >= i_frtPinIndex.GetCenter().m_x
        && m_rtDevice.CenterPoint().y >= i_frtPinIndex.GetCenter().m_y)
        nPinIndexPos = PinIndexPos_RB;

    if (m_rtDevice.CenterPoint().x < i_frtPinIndex.GetCenter().m_x
        && m_rtDevice.CenterPoint().y >= i_frtPinIndex.GetCenter().m_y)
        nPinIndexPos = PinIndexPos_LB;

    switch (nPinIndexPos)
    {
        case PinIndexPos_LT:
            o_pPinIndexPoint[0].x = long(i_frtPinIndex.fptLB.m_x + 0.5f);
            o_pPinIndexPoint[0].y = long(i_frtPinIndex.fptLB.m_y + 0.5f);
            o_pPinIndexPoint[1].x = long(i_frtPinIndex.fptRB.m_x + 0.5f);
            o_pPinIndexPoint[1].y = long(i_frtPinIndex.fptRB.m_y + 0.5f);
            o_pPinIndexPoint[2].x = long(i_frtPinIndex.fptRT.m_x + 0.5f);
            o_pPinIndexPoint[2].y = long(i_frtPinIndex.fptRT.m_y + 0.5f);
            break;
        case PinIndexPos_RT:
            o_pPinIndexPoint[0].x = long(i_frtPinIndex.fptLT.m_x + 0.5f);
            o_pPinIndexPoint[0].y = long(i_frtPinIndex.fptLT.m_y + 0.5f);
            o_pPinIndexPoint[1].x = long(i_frtPinIndex.fptLB.m_x + 0.5f);
            o_pPinIndexPoint[1].y = long(i_frtPinIndex.fptLB.m_y + 0.5f);
            o_pPinIndexPoint[2].x = long(i_frtPinIndex.fptRB.m_x + 0.5f);
            o_pPinIndexPoint[2].y = long(i_frtPinIndex.fptRB.m_y + 0.5f);
            break;
        case PinIndexPos_RB:
            o_pPinIndexPoint[0].x = long(i_frtPinIndex.fptLT.m_x + 0.5f);
            o_pPinIndexPoint[0].y = long(i_frtPinIndex.fptLT.m_y + 0.5f);
            o_pPinIndexPoint[1].x = long(i_frtPinIndex.fptLB.m_x + 0.5f);
            o_pPinIndexPoint[1].y = long(i_frtPinIndex.fptLB.m_y + 0.5f);
            o_pPinIndexPoint[2].x = long(i_frtPinIndex.fptRT.m_x + 0.5f);
            o_pPinIndexPoint[2].y = long(i_frtPinIndex.fptRT.m_y + 0.5f);
            break;
        case PinIndexPos_LB:
            o_pPinIndexPoint[0].x = long(i_frtPinIndex.fptLT.m_x + 0.5f);
            o_pPinIndexPoint[0].y = long(i_frtPinIndex.fptLT.m_y + 0.5f);
            o_pPinIndexPoint[1].x = long(i_frtPinIndex.fptRB.m_x + 0.5f);
            o_pPinIndexPoint[1].y = long(i_frtPinIndex.fptRB.m_y + 0.5f);
            o_pPinIndexPoint[2].x = long(i_frtPinIndex.fptRT.m_x + 0.5f);
            o_pPinIndexPoint[2].y = long(i_frtPinIndex.fptRT.m_y + 0.5f);
            break;
        default:
            break;
    }
}

void CMapDataConverterDlg::CreateComponent_Auto()
{
    m_vecCompInfo.clear();
    long nDataCount = (long)m_vecstObjectInfo.size();

    if (nDataCount <= 0)
        return;

    CString PadID1{};
    CString PadID2{};
    Ipvm::Rect32r tmp_sf_Roi{};
    Ipvm::Rect32r tmp_sf_Roi2{};
    float fOffsetX(0.f), fOffsetY(0.f);
    float fWidth(0.f), fLength(0.f);

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    long gIndex = 0;
    float tmp_fOffsetX(0.f), tmp_fOffsetY(0.f);
    float tmp_fPosX(0.f), tmp_fPosY(0.f);
    float tmp2_fPosX(0.f), tmp2_fPosY(0.f);
    float comp_fOffsetX(0.f), comp_fOffsetY(0.f);
    float fRotation = 0;
    for (long n = 0; n < nDataCount; n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber != 0)
        {
            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_COMPONENT)
            {
                if (abs(m_vecstObjectInfo[n].fRotation
                        - m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation)
                        == 180
                    || abs(m_vecstObjectInfo[n].fRotation
                           - m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation)
                        == 0)
                {
                    fRotation = 0;
                }
                else
                {
                    fRotation = 90;
                }

                float fPosX = 0.;
                float fPosY = 0.;

                GetPosition(m_vecstObjectInfo[n].fPosX, m_vecstObjectInfo[n].fPosY, fPosX, fPosY);

                if (gIndex == 0)
                {
                    PadID1.Format(_T("%d"), m_vecstObjectInfo[n].nID);

                    tmp_fPosX = fPosX;
                    tmp_fPosY = fPosY;

                    fOffsetX = fGridOriginX + fPosX * m_fZoom;
                    fOffsetY = fGridOriginY - fPosY * m_fZoom;

                    if (fRotation == 0 || fRotation == 180)
                    {
                        fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
                        fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;

                        tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
                        tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
                        tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
                        tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);
                    }
                    else
                    {
                        fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
                        fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;

                        tmp_sf_Roi.m_right = (float)(fOffsetX + fWidth);
                        tmp_sf_Roi.m_top = (float)(fOffsetY - fLength);
                        tmp_sf_Roi.m_bottom = (float)(fOffsetY + fLength);
                        tmp_sf_Roi.m_left = (float)(fOffsetX - fWidth);
                    }

                    gIndex++;
                    continue;
                }
                else
                {
                    PadID2.Format(_T("%d"), m_vecstObjectInfo[n].nID);

                    tmp2_fPosX = fPosX;
                    tmp2_fPosY = fPosY;

                    tmp_fOffsetX = fGridOriginX + fPosX * m_fZoom;
                    tmp_fOffsetY = fGridOriginY - fPosY * m_fZoom;

                    if (fRotation == 0 || fRotation == 180)
                    {
                        fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
                        fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;

                        tmp_sf_Roi2.m_left = (float)(tmp_fOffsetX - fWidth);
                        tmp_sf_Roi2.m_right = (float)(tmp_fOffsetX + fWidth);
                        tmp_sf_Roi2.m_top = (float)(tmp_fOffsetY - fLength);
                        tmp_sf_Roi2.m_bottom = (float)(tmp_fOffsetY + fLength);

                        if (fOffsetY > tmp_fOffsetY)
                        {
                            tmp_sf_Roi.m_left = tmp_sf_Roi2.m_left;
                            tmp_sf_Roi.m_right = tmp_sf_Roi2.m_right;
                            tmp_sf_Roi.m_top = tmp_sf_Roi2.m_top;

                            comp_fOffsetX = tmp_fPosX;
                            comp_fOffsetY = tmp2_fPosY + ((tmp_fPosY - tmp2_fPosY) / 2);
                        }
                        else
                        {
                            tmp_sf_Roi.m_bottom = tmp_sf_Roi2.m_bottom;

                            comp_fOffsetX = tmp_fPosX;
                            comp_fOffsetY = tmp_fPosY + ((tmp2_fPosY - tmp_fPosY) / 2);
                        }
                    }
                    else
                    {
                        if (fRotation == 0 || fRotation == 180)
                        {
                            fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
                            fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
                        }
                        else
                        {
                            fWidth = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
                            fLength = m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
                        }

                        tmp_sf_Roi2.m_right = (float)(tmp_fOffsetX + fWidth);
                        tmp_sf_Roi2.m_top = (float)(tmp_fOffsetY - fLength);
                        tmp_sf_Roi2.m_bottom = (float)(tmp_fOffsetY + fLength);
                        tmp_sf_Roi2.m_left = (float)(tmp_fOffsetX - fWidth);

                        if (fOffsetX > tmp_fOffsetX)
                        {
                            tmp_sf_Roi.m_left = tmp_sf_Roi2.m_left;

                            comp_fOffsetX = tmp2_fPosX + ((tmp_fPosX - tmp2_fPosX) / 2);
                            comp_fOffsetY = tmp_fPosY;
                        }
                        else
                        {
                            tmp_sf_Roi.m_right = tmp_sf_Roi2.m_right;
                            tmp_sf_Roi.m_top = tmp_sf_Roi2.m_top;
                            tmp_sf_Roi.m_bottom = tmp_sf_Roi2.m_bottom;

                            comp_fOffsetX = tmp_fPosX + ((tmp2_fPosX - tmp_fPosX) / 2);
                            comp_fOffsetY = tmp_fPosY;
                        }
                    }

                    gIndex = 0;
                }

                Package::Component compMap;

                FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
                    Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
                    Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
                    Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

                m_vecstObjectInfo[n].fsrtSpecROI = frtChip; //SpecPos

                if (fRotation == 90)
                {
                    if (m_bReEditMode)
                    {
                        compMap.fLength = m_vecstObjectInfo[n].fCompLenght;
                        compMap.fWidth = m_vecstObjectInfo[n].fCompWidth;
                    }
                    else
                    {
                        compMap.fLength = round((tmp_sf_Roi.Width() / m_fZoom) * (float)0.001);
                        compMap.fWidth = round((tmp_sf_Roi.Height() / m_fZoom) * (float)0.001);
                    }
                }
                else
                {
                    if (m_bReEditMode)
                    {
                        compMap.fLength = m_vecstObjectInfo[n].fCompLenght;
                        compMap.fWidth = m_vecstObjectInfo[n].fCompWidth;
                    }
                    else
                    {
                        compMap.fLength = round((tmp_sf_Roi.Height() / m_fZoom) * (float)0.001);
                        compMap.fWidth = round((tmp_sf_Roi.Width() / m_fZoom) * (float)0.001);
                    }
                }

                compMap.fOffsetX = comp_fOffsetX;
                compMap.fOffsetY = comp_fOffsetY;
                compMap.fsrtRealROI = frtChip;
                compMap.fsrtROI = frtChip;
                compMap.nAngle = (long)fRotation;
                compMap.fHeight = m_vecstObjectInfo[n].fTickness;
                compMap.rtROI = Ipvm::Conversion::ToRect32s(tmp_sf_Roi);
                if (m_vecstObjectInfo[n].nCompChipType == _typeChip)
                {
                    compMap.strCompName = _T("D01");
                    compMap.strCompType = _T("D1106_02_270");
                }
                else if (m_vecstObjectInfo[n].nCompChipType == _typePassive)
                {
                    if (m_vecstObjectInfo[n].nCompPassiveType == PassiveType_Capacitor)
                    {
                        compMap.strCompName = _T("C01");
                        compMap.strCompType = _T("C1106_02_270");
                    }
                    else if (m_vecstObjectInfo[n].nCompPassiveType == PassiveType_Resitor)
                    {
                        compMap.strCompName = _T("R01");
                        compMap.strCompType = _T("R1106_02_270");
                    }
                    else if (m_vecstObjectInfo[n].nCompPassiveType == PassiveType_Tantalum)
                    {
                        compMap.strCompName = _T("T01");
                        compMap.strCompType = _T("T1106_02_270");
                    }
                    else if (m_vecstObjectInfo[n].nCompPassiveType == PassiveType_LandSideCapacitor)
                    {
                        compMap.strCompName = _T("LSC01");
                        compMap.strCompType = _T("LSC1106_02_270");
                    }
                }
                else if (m_vecstObjectInfo[n].nCompChipType == _typeArray)
                {
                    compMap.strCompName = _T("A01");
                    compMap.strCompType = _T("A1106_02_270");
                }
                else if (m_vecstObjectInfo[n].nCompChipType == _typeHeatSync)
                {
                    compMap.strCompName = _T("H01");
                    compMap.strCompType = _T("H1106_02_270");
                }
                else
                {
                    compMap.strCompName = _T("C01");
                    compMap.strCompType = _T("C1106_02_270");
                }
                //compMap.strPad_ID1 = PadID1;
                //compMap.strPad_ID2 = PadID2;

                m_vecCompInfo.push_back(compMap);
            }
        }
        else
        {
            Package::Component compMap;

            if (m_vecstObjectInfo[n].fRotation == 0. || m_vecstObjectInfo[n].fRotation == 180.)
            {
                compMap.fWidth = m_vecstObjectInfo[n].fCompWidth;
                compMap.fLength = m_vecstObjectInfo[n].fCompLenght;
            }
            else
            {
                compMap.fWidth = m_vecstObjectInfo[n].fCompLenght;
                compMap.fLength = m_vecstObjectInfo[n].fCompWidth;
            }

            tmp_fOffsetX = fGridOriginX + m_vecstObjectInfo[n].fPosX;
            tmp_fOffsetY = fGridOriginY + m_vecstObjectInfo[n].fPosY;

            tmp_sf_Roi.m_right = (float)(tmp_fOffsetX + compMap.fWidth);
            tmp_sf_Roi.m_top = (float)(tmp_fOffsetY - compMap.fLength);
            tmp_sf_Roi.m_bottom = (float)(tmp_fOffsetY + compMap.fLength);
            tmp_sf_Roi.m_left = (float)(tmp_fOffsetX - compMap.fWidth);

            FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
                Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

            m_vecstObjectInfo[n].fsrtSpecROI = frtChip; //SpecPos

            compMap.fOffsetX = m_vecstObjectInfo[n].fPosX;
            compMap.fOffsetY = m_vecstObjectInfo[n].fPosY;
            compMap.fsrtRealROI = frtChip;
            compMap.fsrtROI = frtChip;
            compMap.nAngle = (long)m_vecstObjectInfo[n].fRotation;
            compMap.fHeight = m_vecstObjectInfo[n].fTickness;
            compMap.rtROI = Ipvm::Conversion::ToRect32s(tmp_sf_Roi);
            if (m_vecstObjectInfo[n].nCompChipType == _typeChip)
            {
                compMap.strCompName = _T("D01");
                compMap.strCompType = _T("D1106_02_270");
            }
            else if (m_vecstObjectInfo[n].nCompChipType == _typePassive)
            {
                if (m_vecstObjectInfo[n].nCompPassiveType == PassiveType_Capacitor)
                {
                    compMap.strCompName = _T("C01");
                    compMap.strCompType = _T("C1106_02_270");
                }
                else if (m_vecstObjectInfo[n].nCompPassiveType == PassiveType_Resitor)
                {
                    compMap.strCompName = _T("R01");
                    compMap.strCompType = _T("R1106_02_270");
                }
                else if (m_vecstObjectInfo[n].nCompPassiveType == PassiveType_Tantalum)
                {
                    compMap.strCompName = _T("T01");
                    compMap.strCompType = _T("T1106_02_270");
                }
                else if (m_vecstObjectInfo[n].nCompPassiveType == PassiveType_LandSideCapacitor)
                {
                    compMap.strCompName = _T("LSC01");
                    compMap.strCompType = _T("LSC1106_02_270");
                }
            }
            else if (m_vecstObjectInfo[n].nCompChipType == _typeArray)
            {
                compMap.strCompName = _T("A01");
                compMap.strCompType = _T("A1106_02_270");
            }
            else if (m_vecstObjectInfo[n].nCompChipType == _typeHeatSync)
            {
                compMap.strCompName = _T("H01");
                compMap.strCompType = _T("H1106_02_270");
            }
            else
            {
                compMap.strCompName = _T("C01");
                compMap.strCompType = _T("C1106_02_270");
            }

            m_vecCompInfo.push_back(compMap);
        }
    }

    CompTypeConvert();
}

void CMapDataConverterDlg::CreateComponent_Manual()
{
    m_vecCompInfo.clear();
    long nDataCount = (long)m_vecstObjectInfo.size();

    if (nDataCount <= 0)
        return;

    std::vector<long> existCompType;

    for (long index = 0; index < nDataCount; index++)
    {
        if (m_vecstShapeInfo[m_vecstObjectInfo[index].nShapeNumber - 1].nGerberType == TYPE_COMPONENT)
        {
            if (m_vecstObjectInfo[index].nCompGrouping != 0)
            {
                long index2 = GetPadGroupingIndex(index);
                BOOL bComponentExist
                    = std::find(existCompType.begin(), existCompType.end(), index) != existCompType.end();
                if (!bComponentExist)
                {
                    existCompType.push_back(index);
                    existCompType.push_back(index2);
                    CreateComponent(index, index2);
                }
            }
        }
    }
}

void CMapDataConverterDlg::CreateComponent(long nMountPadIdx1, long nMountPadIdx2)
{
    long nDataCount = (long)m_vecstObjectInfo.size();

    if (nDataCount <= 0)
        return;

    CString PadID1;
    CString PadID2;
    Ipvm::Rect32r tmp_sf_Roi;
    CRect rtUnionRect;
    CRect rtTmpRoi1;
    CRect rtTmpRoi2;
    float fOffsetX(0.f), fOffsetY(0.f);
    float fWidth(0.f), fLength(0.f);
    float fWidth2(0.f), fLength2(0.f);

    const float fGridOriginX = 0.5f * (m_rtDevice.left + m_rtDevice.right - 1);
    const float fGridOriginY = 0.5f * (m_rtDevice.top + m_rtDevice.bottom - 1);

    float tmp_fOffsetX(0.f), tmp_fOffsetY(0.f);
    float tmp_fPosX(0.f), tmp_fPosY(0.f);
    float tmp2_fPosX(0.f), tmp2_fPosY(0.f);
    float comp_fOffsetX(0.f), comp_fOffsetY(0.f);
    float fRotation = 0;
    float fRotation2 = 0;

    if (abs(m_vecstObjectInfo[nMountPadIdx1].fRotation
            - m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx1].nShapeNumber - 1].nOrientation)
            == 180
        || abs(m_vecstObjectInfo[nMountPadIdx1].fRotation
               - m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx1].nShapeNumber - 1].nOrientation)
            == 0)
    {
        fRotation = 0;
    }
    else
    {
        fRotation = 90;
    }

    if (abs(m_vecstObjectInfo[nMountPadIdx2].fRotation
            - m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx2].nShapeNumber - 1].nOrientation)
            == 180
        || abs(m_vecstObjectInfo[nMountPadIdx2].fRotation
               - m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx2].nShapeNumber - 1].nOrientation)
            == 0)
    {
        fRotation2 = 0;
    }
    else
    {
        fRotation2 = 90;
    }

    float fPosX1 = 0.;
    float fPosY1 = 0.;
    float fPosX2 = 0.;
    float fPosY2 = 0.;

    GetPosition(m_vecstObjectInfo[nMountPadIdx1].fPosX, m_vecstObjectInfo[nMountPadIdx1].fPosY, fPosX1, fPosY1);
    GetPosition(m_vecstObjectInfo[nMountPadIdx2].fPosX, m_vecstObjectInfo[nMountPadIdx2].fPosY, fPosX2, fPosY2);

    PadID1.Format(_T("%d"), m_vecstObjectInfo[nMountPadIdx1].nID);

    tmp_fPosX = fPosX1;
    tmp_fPosY = fPosY1;

    fOffsetX = fGridOriginX + fPosX1 * m_fZoom;
    fOffsetY = fGridOriginY - fPosY1 * m_fZoom;

    if (fRotation == 0 || fRotation == 180)
    {
        fWidth = m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx1].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
        fLength = m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx1].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;

        rtTmpRoi1.left = (long)(fOffsetX - fWidth);
        rtTmpRoi1.right = (long)(fOffsetX + fWidth);
        rtTmpRoi1.top = (long)(fOffsetY - fLength);
        rtTmpRoi1.bottom = (long)(fOffsetY + fLength);
    }
    else
    {
        fWidth = m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx1].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;
        fLength = m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx1].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;

        rtTmpRoi1.right = (long)(fOffsetX + fWidth);
        rtTmpRoi1.top = (long)(fOffsetY - fLength);
        rtTmpRoi1.bottom = (long)(fOffsetY + fLength);
        rtTmpRoi1.left = (long)(fOffsetX - fWidth);
    }

    PadID2.Format(_T("%d"), m_vecstObjectInfo[nMountPadIdx2].nID);

    tmp2_fPosX = fPosX2;
    tmp2_fPosY = fPosY2;

    tmp_fOffsetX = fGridOriginX + fPosX2 * m_fZoom;
    tmp_fOffsetY = fGridOriginY - fPosY2 * m_fZoom;

    fWidth2 = m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx2].nShapeNumber - 1].fHeigth * m_fZoom * 0.5f;
    fLength2 = m_vecstShapeInfo[m_vecstObjectInfo[nMountPadIdx2].nShapeNumber - 1].fWidth * m_fZoom * 0.5f;

    rtTmpRoi2.left = (long)(tmp_fOffsetX - fWidth2);
    rtTmpRoi2.right = (long)(tmp_fOffsetX + fWidth2);
    rtTmpRoi2.top = (long)(tmp_fOffsetY - fLength2);
    rtTmpRoi2.bottom = (long)(tmp_fOffsetY + fLength2);

    rtUnionRect.UnionRect(rtTmpRoi1, rtTmpRoi2);

    tmp_sf_Roi.m_bottom = (float)rtUnionRect.bottom;
    tmp_sf_Roi.m_left = (float)rtUnionRect.left;
    tmp_sf_Roi.m_right = (float)rtUnionRect.right;
    tmp_sf_Roi.m_top = (float)rtUnionRect.top;

    comp_fOffsetX = (tmp_sf_Roi.CenterPoint().m_x - fGridOriginX) / m_fZoom;
    comp_fOffsetY = ((tmp_sf_Roi.CenterPoint().m_y - fGridOriginY) / m_fZoom) * -1;

    Package::Component compMap;

    FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_top),
        Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_top),
        Ipvm::Point32r2((float)tmp_sf_Roi.m_left, (float)tmp_sf_Roi.m_bottom),
        Ipvm::Point32r2((float)tmp_sf_Roi.m_right, (float)tmp_sf_Roi.m_bottom));

    m_vecstObjectInfo[nMountPadIdx1].fsrtSpecROI = frtChip; //SpecPos
    m_vecstObjectInfo[nMountPadIdx2].fsrtSpecROI = frtChip; //SpecPos

    if (fRotation == 90)
    {
        if (m_bReEditMode)
        {
            compMap.fLength = m_vecstObjectInfo[nMountPadIdx1].fCompLenght;
            compMap.fWidth = m_vecstObjectInfo[nMountPadIdx1].fCompWidth;
        }
        else
        {
            compMap.fLength = round((tmp_sf_Roi.Width() / m_fZoom) * (float)0.001);
            compMap.fWidth = round((tmp_sf_Roi.Height() / m_fZoom) * (float)0.001);
        }
    }
    else
    {
        if (m_bReEditMode)
        {
            compMap.fLength = m_vecstObjectInfo[nMountPadIdx1].fCompLenght;
            compMap.fWidth = m_vecstObjectInfo[nMountPadIdx1].fCompWidth;
        }
        else
        {
            compMap.fLength = round((tmp_sf_Roi.Height() / m_fZoom) * (float)0.001);
            compMap.fWidth = round((tmp_sf_Roi.Width() / m_fZoom) * (float)0.001);
        }
    }

    compMap.fOffsetX = comp_fOffsetX;
    compMap.fOffsetY = comp_fOffsetY;
    compMap.fsrtRealROI = frtChip;
    compMap.fsrtROI = frtChip;
    compMap.nAngle = (long)fRotation;
    compMap.fHeight = m_vecstObjectInfo[nMountPadIdx2].fTickness;
    compMap.rtROI = Ipvm::Conversion::ToRect32s(tmp_sf_Roi);
    compMap.strCompName = _T("Test");
    compMap.strCompType = _T("C1106_02_270");
    //compMap.strPad_ID1	= PadID1;
    //compMap.strPad_ID2	= PadID2;

    m_vecCompInfo.push_back(compMap);

    long GroupIndex = GetGroupIndex() + 1;
    m_vecstObjectInfo[nMountPadIdx1].nCompGrouping = GroupIndex;
    m_vecstObjectInfo[nMountPadIdx2].nCompGrouping = GroupIndex;

    CompTypeConvert();
}

long CMapDataConverterDlg::GetPadGroupingIndex(long nGroupIDX)
{
    long nResult = 0;
    long nDataCount = (long)m_vecstObjectInfo.size();
    if (nDataCount <= 0)
        return 0;

    long nGroupIndex = m_vecstObjectInfo[nGroupIDX].nCompGrouping;

    for (long n = 0; n < nDataCount; n++)
    {
        if (nGroupIDX != n)
        {
            if (m_vecstObjectInfo[n].nCompGrouping == nGroupIndex)
            {
                nResult = n;
                break;
            }
        }
    }

    return nResult;
}

void CMapDataConverterDlg::Update_Background(long i_nCurLayerIndex)
{
    CDC* pDC = GetDC();
    m_bitmapGrid.DeleteObject();
    m_bitmapGrid.CreateCompatibleBitmap(pDC, m_rtView.Width(), m_rtView.Height());

    CDC MemDC;
    MemDC.CreateCompatibleDC(pDC);
    CBitmap* OldBitmap = MemDC.SelectObject(&m_bitmapGrid);

    // 디바이스 테두리 그리기
    CBrush brushDarkGreen(RGB(0, 120, 0));
    CBrush* pOldBrush = MemDC.SelectObject(&brushDarkGreen);

    CPen penBrightGreen(PS_SOLID, 3, RGB(0, 80, 0));
    CPen* pOldPen = MemDC.SelectObject(&penBrightGreen);

    MemDC.Rectangle(m_rtDevice);

    // Ball Index by Bottom View
    CPen penLayerPinIndex(PS_SOLID, 1, RGB(0, 230, 0));
    MemDC.SelectObject(&penLayerPinIndex);
    CBrush brushLayerPinIndex(RGB(0, 230, 0));
    MemDC.SelectObject(&brushLayerPinIndex);
    MemDC.Polygon(m_ptPinIndex, 3);

    if (i_nCurLayerIndex == TYPE_ALL)
    {
        // Index by Bottom View
        CPen penWholePinIndex(PS_SOLID, 1, RGB(0, 230, 0));
        MemDC.SelectObject(&penWholePinIndex);
        CBrush brushWholePinIndex(RGB(0, 230, 0));
        MemDC.SelectObject(&brushWholePinIndex);
        MemDC.Polygon(m_ptPinIndex, 3);
    }

    MemDC.SelectObject(OldBitmap);
    MemDC.SelectObject(pOldBrush);
    MemDC.SelectObject(pOldPen);
}

void CMapDataConverterDlg::Update_Foreground(long i_nCurLayerIndex, long i_nPartIndex)
{
    m_nSelectedLayer = i_nCurLayerIndex;
    m_nSelectedGroup = i_nPartIndex;

    if (i_nCurLayerIndex < 0)
        return;

    CDC* pDC = GetDC();
    m_bitmapView.DeleteObject();
    m_bitmapView.CreateCompatibleBitmap(pDC, m_rtView.Width(), m_rtView.Height());

    //mc_Combo에 선택된 Index에 따라 그리기를 달리한다.
    switch (i_nCurLayerIndex)
    {
        case TYPE_BALL:
            Calc_DrawingPosition_Ball();
            SetDrawing_BallForegroundData(i_nCurLayerIndex, pDC, i_nPartIndex);
            break;
        case TYPE_LAND:
            Calc_DrawingPosition_Land();
            SetDrawing_LandForegroundData(i_nCurLayerIndex, pDC, i_nPartIndex);
            break;
        case TYPE_COMPONENT:
            Calc_DrawingPosition_Comp(m_bVerifyMode);
            SetDrawing_CompForegroundData(i_nCurLayerIndex, pDC, i_nPartIndex, m_bVerifyMode);
            break;
        case TYPE_PAD:
            Calc_DrawingPosition_Pad();
            SetDrawing_PadForegroundData(i_nCurLayerIndex, pDC, i_nPartIndex);
            break;
        case TYPE_ALL:
            Calc_DrawingPosition_Ball();
            Calc_DrawingPosition_Land();
            Calc_DrawingPosition_Comp(m_bVerifyMode);
            Calc_DrawingPosition_Pad();
            SetDrawing_WholeForegroundData(pDC, m_bVerifyMode);
            break;
    }

    ReleaseDC(pDC);
}

void CMapDataConverterDlg::SetDrawing_BallForegroundData(long i_nCurLayerIndex, CDC* i_pDC, long i_nPartIndex)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    // 볼을 그립니다
    COLORREF rgbMain = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
    COLORREF rgbSelected = RGB(0, 120, 215);

    CBrush brushMain(rgbMain);
    CBrush brushSelected(rgbSelected);
    CBrush* pOldBrush = MemDC.SelectObject(&brushMain);

    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_BALL)
        {
            if (i_nPartIndex == -1)
            {
                if (m_vecstObjectInfo[n].bSelected)
                    MemDC.SelectObject(&brushSelected);
                else
                    MemDC.SelectObject(&brushMain);

                MemDC.Ellipse(m_vecstObjectInfo[n].ballPosition);
            }
            else if (i_nPartIndex == (m_vecstObjectInfo[n].nShapeNumber - 1))
            {
                if (m_vecstObjectInfo[n].bSelected)
                    MemDC.SelectObject(&brushSelected);
                else
                    MemDC.SelectObject(&brushMain);

                MemDC.Ellipse(m_vecstObjectInfo[n].ballPosition);
            }
        }
    }

    MemDC.SelectObject(pOldBrush);

    MemDC.SelectObject(pOldBitmap);
}

void CMapDataConverterDlg::SetDrawing_LandForegroundData(long i_nCurLayerIndex, CDC* i_pDC, long i_nPartIndex)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    CPoint ptLandROI[4];

    CBrush* pOldBrush = NULL;
    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_LAND)
        {
            COLORREF rgbLand(0);
            COLORREF rgbSelected = RGB(0, 120, 215);

            switch (m_vecstObjectInfo[n].nLandGroupNo)
            {
                case 0:
                    rgbLand = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
                    break;
                case 1:
                    rgbLand = RGB(255 - (1 + i_nCurLayerIndex / 2) * 100, 128, 255);
                    break;
                case 2:
                    rgbLand = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
                    break;
                default:
                    rgbLand = RGB(243, 97, 166);
                    break;
            }

            if (i_nPartIndex == -1)
            {
                CBrush brushLand(rgbLand);
                CBrush brushSelected(rgbSelected);
                pOldBrush = MemDC.SelectObject(&brushLand);

                if (m_vecstObjectInfo[n].bSelected)
                    MemDC.SelectObject(&brushSelected);
                else
                    MemDC.SelectObject(&brushLand);

                ptLandROI[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                ptLandROI[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptLandROI[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptLandROI[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                ptLandROI[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                ptLandROI[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                ptLandROI[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                ptLandROI[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);

                MemDC.Polygon(ptLandROI, 4);
            }
            else if (i_nPartIndex == (m_vecstObjectInfo[n].nShapeNumber - 1))
            {
                CBrush brushLand(rgbLand);
                CBrush brushSelected(rgbSelected);
                pOldBrush = MemDC.SelectObject(&brushLand);

                if (m_vecstObjectInfo[n].bSelected)
                    MemDC.SelectObject(&brushSelected);
                else
                    MemDC.SelectObject(&brushLand);

                ptLandROI[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                ptLandROI[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptLandROI[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptLandROI[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                ptLandROI[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                ptLandROI[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                ptLandROI[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                ptLandROI[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);

                MemDC.Polygon(ptLandROI, 4);
            }
        }
    }

    // 마우스가 눌렸을 경우에 박스 그려주기
    if (m_bLButtonDown)
    {
        CRect rtSelection;
        rtSelection.left = min(m_ptSelection[0].x, m_ptSelection[1].x);
        rtSelection.top = min(m_ptSelection[0].y, m_ptSelection[1].y);
        rtSelection.right = max(m_ptSelection[0].x, m_ptSelection[1].x) + 1;
        rtSelection.bottom = max(m_ptSelection[0].y, m_ptSelection[1].y) + 1;

        MemDC.DrawFocusRect(rtSelection);
    }

    MemDC.SelectObject(pOldBrush);

    MemDC.SelectObject(pOldBitmap);
}

void CMapDataConverterDlg::SetDrawing_CompForegroundData(
    long i_nCurLayerIndex, CDC* i_pDC, long i_nPartIndex, bool bPreviewMode)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    // Comp를 그립니다
    COLORREF rgbComp = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
    COLORREF rgbSelected = RGB(0, 120, 215);

    CBrush brushComp(rgbComp);
    CBrush brushSelected(rgbSelected);
    CBrush* pOldBrush = MemDC.SelectObject(&brushComp);

    CPoint ptCompROI[4];

    if (!bPreviewMode)
    {
        for (long n = 0; n < m_vecstObjectInfo.size(); n++)
        {
            if (m_vecstObjectInfo[n].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_COMPONENT)
            {
                if (i_nPartIndex == -1)
                {
                    if (m_vecstObjectInfo[n].bSelected)
                        MemDC.SelectObject(&brushSelected);
                    else
                        MemDC.SelectObject(&brushComp);

                    ptCompROI[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                    ptCompROI[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptCompROI[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptCompROI[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                    ptCompROI[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                    ptCompROI[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                    ptCompROI[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                    ptCompROI[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);

                    MemDC.Polygon(ptCompROI, 4);
                }
                else if (i_nPartIndex == (m_vecstObjectInfo[n].nShapeNumber - 1))
                {
                    if (m_vecstObjectInfo[n].bSelected)
                        MemDC.SelectObject(&brushSelected);
                    else
                        MemDC.SelectObject(&brushComp);

                    ptCompROI[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                    ptCompROI[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptCompROI[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptCompROI[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                    ptCompROI[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                    ptCompROI[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                    ptCompROI[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                    ptCompROI[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);

                    MemDC.Polygon(ptCompROI, 4);
                }
            }
        }
    }
    else
    {
        for (const auto& CompInfo : m_vecCompInfo)
        {
            // Selected 대신에 Ignore 사용
            if (CompInfo.bIgnore)
                MemDC.SelectObject(&brushSelected);
            else
                MemDC.SelectObject(&brushComp);

            ptCompROI[0].x = long(CompInfo.fsrtROI.fptLT.m_x + 0.5f);
            ptCompROI[0].y = long(CompInfo.fsrtROI.fptLT.m_y + 0.5f);
            ptCompROI[1].x = long(CompInfo.fsrtROI.fptLB.m_x + 0.5f);
            ptCompROI[1].y = long(CompInfo.fsrtROI.fptLB.m_y + 0.5f);
            ptCompROI[2].x = long(CompInfo.fsrtROI.fptRB.m_x + 0.5f);
            ptCompROI[2].y = long(CompInfo.fsrtROI.fptRB.m_y + 0.5f);
            ptCompROI[3].x = long(CompInfo.fsrtROI.fptRT.m_x + 0.5f);
            ptCompROI[3].y = long(CompInfo.fsrtROI.fptRT.m_y + 0.5f);

            MemDC.Polygon(ptCompROI, 4);
        }
    }

    // 마우스가 눌렸을 경우에 박스 그려주기
    if (m_bLButtonDown)
    {
        CRect rtSelection;
        rtSelection.left = min(m_ptSelection[0].x, m_ptSelection[1].x);
        rtSelection.top = min(m_ptSelection[0].y, m_ptSelection[1].y);
        ;
        rtSelection.right = max(m_ptSelection[0].x, m_ptSelection[1].x) + 1;
        rtSelection.bottom = max(m_ptSelection[0].y, m_ptSelection[1].y) + 1;

        MemDC.DrawFocusRect(rtSelection);
    }

    MemDC.SelectObject(pOldBrush);

    MemDC.SelectObject(pOldBitmap);
}

void CMapDataConverterDlg::SetDrawing_PadForegroundData(long i_nCurLayerIndex, CDC* i_pDC, long i_nPartIndex)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    COLORREF rgbPad = RGB(255, 128, 255 - (1 + i_nCurLayerIndex / 2) * 100);
    COLORREF rgbSelected = RGB(0, 120, 215);
    CRect rtROI;

    CBrush brushPad(rgbPad);
    CBrush brushSelected(rgbSelected);
    CBrush* pOldBrush = MemDC.SelectObject(&brushPad);

    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_PAD)
        {
            if (i_nPartIndex == -1)
            {
                if (m_vecstObjectInfo[n].bSelected)
                    MemDC.SelectObject(&brushSelected);
                else
                    MemDC.SelectObject(&brushPad);

                if (m_vecstObjectInfo[n].nPadType == _typeRectangle)
                {
                    CPoint ptPad_Rect[4];

                    ptPad_Rect[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                    ptPad_Rect[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptPad_Rect[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptPad_Rect[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                    ptPad_Rect[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                    ptPad_Rect[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                    ptPad_Rect[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                    ptPad_Rect[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);

                    MemDC.Polygon(ptPad_Rect, 4);
                }
                else if (m_vecstObjectInfo[n].nPadType == _typeCircle)
                {
                    CRect rtPad_Circle;
                    rtPad_Circle = Ipvm::ToMFC(m_vecstObjectInfo[n].fsrtSpecROI.GetCRect());

                    MemDC.Ellipse(rtPad_Circle);
                }
                else // PinIndex _typeTriangle
                {
                    CPoint ptPad_PinIndex[3];
                    GetPinIndexPoint(m_vecstObjectInfo[n].fsrtSpecROI, ptPad_PinIndex);

                    MemDC.Polygon(ptPad_PinIndex, 3);
                }
            }
            else if (i_nPartIndex == (m_vecstObjectInfo[n].nShapeNumber - 1))
            {
                if (m_vecstObjectInfo[n].bSelected)
                    MemDC.SelectObject(&brushSelected);
                else
                    MemDC.SelectObject(&brushPad);

                if (m_vecstObjectInfo[n].nPadType == _typeRectangle)
                {
                    CPoint ptPad_Rect[4];

                    ptPad_Rect[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                    ptPad_Rect[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptPad_Rect[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptPad_Rect[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                    ptPad_Rect[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                    ptPad_Rect[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                    ptPad_Rect[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                    ptPad_Rect[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);

                    MemDC.Polygon(ptPad_Rect, 4);
                }
                else if (m_vecstObjectInfo[n].nPadType == _typeCircle)
                {
                    CRect rtPad_Circle;
                    rtPad_Circle = Ipvm::ToMFC(m_vecstObjectInfo[n].fsrtSpecROI.GetCRect());

                    MemDC.Ellipse(rtPad_Circle);
                }
                else // PinIndex _typeTriangle
                {
                    CPoint ptPad_PinIndex[3];
                    GetPinIndexPoint(m_vecstObjectInfo[n].fsrtSpecROI, ptPad_PinIndex);

                    MemDC.Polygon(ptPad_PinIndex, 3);
                }
            }
        }
    }

    // 마우스가 눌렸을 경우에 박스 그려주기
    if (m_bLButtonDown)
    {
        CRect rtSelection;
        rtSelection.left = min(m_ptSelection[0].x, m_ptSelection[1].x);
        rtSelection.top = min(m_ptSelection[0].y, m_ptSelection[1].y);
        ;
        rtSelection.right = max(m_ptSelection[0].x, m_ptSelection[1].x) + 1;
        rtSelection.bottom = max(m_ptSelection[0].y, m_ptSelection[1].y) + 1;

        MemDC.DrawFocusRect(rtSelection);
    }

    MemDC.SelectObject(pOldBrush);

    MemDC.SelectObject(pOldBitmap);
}

void CMapDataConverterDlg::SetDrawing_WholeForegroundData(CDC* i_pDC, bool bPreviewMode)
{
    if (i_pDC == NULL)
        return;

    CDC MemDC;
    MemDC.CreateCompatibleDC(i_pDC);
    MemDC.SetBkMode(TRANSPARENT);
    CBitmap* pOldBitmap = MemDC.SelectObject(&m_bitmapView);

    // Grid 이미지를 붙입니다
    CDC MemDC2;
    MemDC2.CreateCompatibleDC(i_pDC);
    CBitmap* OldBitmap2 = MemDC2.SelectObject(&m_bitmapGrid);
    MemDC.BitBlt(0, 0, m_rtView.Width(), m_rtView.Height(), &MemDC2, 0, 0, SRCCOPY);
    MemDC2.SelectObject(&OldBitmap2);

    // 볼을 그립니다
    COLORREF rgbWholeDevice_Ball = RGB(242, 150, 97);

    CBrush brushBall(rgbWholeDevice_Ball);
    CBrush* pBall_OldBrush = MemDC.SelectObject(&brushBall);

    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_BALL)
        {
            CRect m_ballPosition;
            MemDC.SelectObject(&brushBall);

            MemDC.Ellipse(m_ballPosition);
        }
    }

    //Land를 그립니다.
    COLORREF rgbWholeDevice_Land = RGB(242, 150, 97);
    CBrush brushLand(rgbWholeDevice_Land);
    CBrush* pLand_OldBrush = MemDC.SelectObject(&brushLand);

    CPoint ptLandROI[4];

    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_LAND)
        {
            MemDC.SelectObject(&brushLand);

            ptLandROI[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
            ptLandROI[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
            ptLandROI[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
            ptLandROI[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
            ptLandROI[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
            ptLandROI[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
            ptLandROI[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
            ptLandROI[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);

            MemDC.Polygon(ptLandROI, 4);
        }
    }

    //Comp를 그립니다
    COLORREF rgbWholeDevice_Comp = RGB(255, 255, 72);
    CBrush brushComp(rgbWholeDevice_Comp);
    CBrush* pComp_OldBrush = MemDC.SelectObject(&brushComp);

    CPoint ptCompROI[4];

    if (!bPreviewMode)
    {
        for (long n = 0; n < m_vecstObjectInfo.size(); n++)
        {
            if (m_vecstObjectInfo[n].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_COMPONENT)
            {
                MemDC.SelectObject(&brushComp);

                ptCompROI[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                ptCompROI[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptCompROI[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptCompROI[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                ptCompROI[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                ptCompROI[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                ptCompROI[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                ptCompROI[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);

                MemDC.Polygon(ptCompROI, 4);
            }
        }
    }
    else
    {
        for (const auto& CompInfo : m_vecCompInfo)
        {
            ptCompROI[0].x = long(CompInfo.fsrtROI.fptLT.m_x + 0.5f);
            ptCompROI[0].y = long(CompInfo.fsrtROI.fptLT.m_y + 0.5f);
            ptCompROI[1].x = long(CompInfo.fsrtROI.fptLB.m_x + 0.5f);
            ptCompROI[1].y = long(CompInfo.fsrtROI.fptLB.m_y + 0.5f);
            ptCompROI[2].x = long(CompInfo.fsrtROI.fptRB.m_x + 0.5f);
            ptCompROI[2].y = long(CompInfo.fsrtROI.fptRB.m_y + 0.5f);
            ptCompROI[3].x = long(CompInfo.fsrtROI.fptRT.m_x + 0.5f);
            ptCompROI[3].y = long(CompInfo.fsrtROI.fptRT.m_y + 0.5f);

            MemDC.Polygon(ptCompROI, 4);
        }
    }

    //Pad를 그립니다.
    COLORREF rgbWholeDevice_Pad = RGB(201, 138, 255);
    CBrush brushPad(rgbWholeDevice_Pad);
    CBrush* pPad_OldBrush = MemDC.SelectObject(&brushPad);

    CRect rtROI;
    float fOffsetX(0.f), fOffsetY(0.f);
    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_PAD)
        {
            MemDC.SelectObject(&brushPad);

            rtROI = Ipvm::ToMFC(m_vecstObjectInfo[n].rtROI);
            fOffsetX = (rtROI.left + rtROI.right) * 0.5f;
            fOffsetY = (rtROI.top + rtROI.bottom) * 0.5f;

            if (m_vecstObjectInfo[n].nPadType == _typeTriangle || m_vecstObjectInfo[n].nPadType == _typePinIndex)
            {
                CPoint ptPadIndex[4];

                if ((m_rtDevice.right / 2) >= m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x
                    && (m_rtDevice.bottom / 2) >= m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y)
                {
                    ptPadIndex[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                    ptPadIndex[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptPadIndex[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptPadIndex[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                    ptPadIndex[2].x = long(
                        (m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x - m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x) / 2
                        + m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptPadIndex[2].y = long(
                        (m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y - m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y) / 2
                        + m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptPadIndex[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                    ptPadIndex[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);
                }
                else if ((m_rtDevice.right / 2) < m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x
                    && (m_rtDevice.bottom / 2) < m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y)
                {
                    ptPadIndex[0].x = long(
                        (m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x - m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x) / 2
                        + m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptPadIndex[0].y = long(
                        (m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y - m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y) / 2
                        + m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptPadIndex[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptPadIndex[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                    ptPadIndex[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                    ptPadIndex[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                    ptPadIndex[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                    ptPadIndex[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);
                }
                else if ((m_rtDevice.right / 2) >= m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x
                    && (m_rtDevice.bottom / 2) < m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y)
                {
                    ptPadIndex[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                    ptPadIndex[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptPadIndex[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptPadIndex[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                    ptPadIndex[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                    ptPadIndex[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                    ptPadIndex[3].x = long(
                        (m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x - m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x) / 2
                        + m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptPadIndex[3].y = long(
                        (m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y - m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y) / 2
                        + m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                }
                else if ((m_rtDevice.right / 2) < m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x
                    && (m_rtDevice.bottom / 2) >= m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y)
                {
                    ptPadIndex[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                    ptPadIndex[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptPadIndex[1].x = long(
                        (m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x - m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x) / 2
                        + m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                    ptPadIndex[1].y = long(
                        (m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y - m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y) / 2
                        + m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                    ptPadIndex[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                    ptPadIndex[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                    ptPadIndex[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                    ptPadIndex[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);
                }

                MemDC.Polygon(ptPadIndex, 4);
                MemDC.SelectObject(brushPad);
            }
            else if (m_vecstObjectInfo[n].nPadType == _typeRectangle)
            {
                CPoint ptPadIndex[4];

                ptPadIndex[0].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_x + 0.5f);
                ptPadIndex[0].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLT.m_y + 0.5f);
                ptPadIndex[1].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_x + 0.5f);
                ptPadIndex[1].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptLB.m_y + 0.5f);
                ptPadIndex[2].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_x + 0.5f);
                ptPadIndex[2].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRB.m_y + 0.5f);
                ptPadIndex[3].x = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_x + 0.5f);
                ptPadIndex[3].y = long(m_vecstObjectInfo[n].fsrtSpecROI.fptRT.m_y + 0.5f);

                MemDC.Polygon(ptPadIndex, 4);
                MemDC.SelectObject(brushPad);
            }
            else if (m_vecstObjectInfo[n].nPadType == _typeCircle)
            {
                CRect m_PadPosition;
                m_PadPosition = Ipvm::ToMFC(m_vecstObjectInfo[n].fsrtSpecROI.GetCRect());
                MemDC.Ellipse(m_PadPosition);
            }
        }
    }

    // 마우스가 눌렸을 경우에 박스 그려주기
    if (m_bLButtonDown)
    {
        CRect rtSelection;
        rtSelection.left = min(m_ptSelection[0].x, m_ptSelection[1].x);
        rtSelection.top = min(m_ptSelection[0].y, m_ptSelection[1].y);
        ;
        rtSelection.right = max(m_ptSelection[0].x, m_ptSelection[1].x) + 1;
        rtSelection.bottom = max(m_ptSelection[0].y, m_ptSelection[1].y) + 1;

        MemDC.DrawFocusRect(rtSelection);
    }

    MemDC.SelectObject(pBall_OldBrush);
    MemDC.SelectObject(pLand_OldBrush);
    MemDC.SelectObject(pComp_OldBrush);
    MemDC.SelectObject(pPad_OldBrush);

    MemDC.SelectObject(pOldBitmap);
}

void CMapDataConverterDlg::ImportXML()
{
    CFileDialog dlg(
        TRUE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("XML (*.xml)|*.xml||"));

    if (dlg.DoModal() == IDOK)
    {
        m_vecCompInfo.clear();
        m_vecSelectedCompInfo.clear();
        m_propertyGrid->FindItem(ITEM_ID_IMPORT_XML)->SetValue(dlg.GetPathName());

        CMarkup xml;
        // File open
        xml.Load(dlg.GetPathName());

        // File Read
        xml.FindElem(_T("ePM-SPI"));
        xml.IntoElem();
        // Coding
        Get_GerberHeaderInfo_XML(xml);
        Get_GerberShapeInfo_XML(xml);
        Get_GerberObjectInfo_XML(xml);
        xml.OutOfElem();

        m_nAutoGroup = 0;
        m_bReEditMode = FALSE;
        m_strImportFilePathName = dlg.GetPathName().Left(dlg.GetPathName().GetLength() - 3) + _T("csv");
    }
}

void CMapDataConverterDlg::Get_GerberHeaderInfo_XML(CMarkup& xml)
{
    m_stHeaderInfo.nObjectNumber = 0;
    m_stHeaderInfo.nZoneID_Column = 0;
    m_stHeaderInfo.nReflowZCompensation_Column = 0;
    m_stHeaderInfo.nConrnerBending_column = 0;
    m_stHeaderInfo.nShapeNumber = 0;
    m_stHeaderInfo.fDominantRotation = 0.f;

    xml.FindElem(_T("format"));
    m_stHeaderInfo.strVersion = xml.GetAttrib(_T("ver"));

    xml.FindElem(_T("unit"));
    m_stHeaderInfo.strUnit = xml.GetAttrib(_T("name"));

    // 분리된 Data 추출
    if (m_stHeaderInfo.strUnit.Find(_T("micron")) > -1)
        m_fUnit = 1.f;
    else if (m_stHeaderInfo.strUnit.Find(_T("mil")) > -1)
        m_fUnit = 25.4f;
    else if (m_stHeaderInfo.strUnit.Find(_T("mm")) > -1)
        m_fUnit = 1000.f;

    xml.FindElem(_T("board"));
    m_stHeaderInfo.fPackageSizeX = (float)_ttof(xml.GetAttrib(_T("w"))) * m_fUnit;
    m_stHeaderInfo.fPackageSizeY = (float)_ttof(xml.GetAttrib(_T("h"))) * m_fUnit;
}

void CMapDataConverterDlg::Get_GerberShapeInfo_XML(CMarkup& xml)
{
    m_vecstShapeInfo.clear();
    m_vecst_XML_PAS.clear();
    m_vecst_XML_FPS.clear();

    Gerber_Shape GShapeTemp;
    Gerber_XML_PAS GXMLPasTemp;
    Gerber_XML_FPS GXMLFpsTemp;

    CString strTemp;

    xml.FindElem(_T("pds"));
    xml.IntoElem();
    BOOL bPDSEnd = FALSE;
    do
    {
        bPDSEnd = xml.FindElem(_T("pd"));
        if (bPDSEnd)
        {
            GShapeTemp.strBalltop = _T("");
            GShapeTemp.nRounding = 0;
            GShapeTemp.nOrientation = _ttoi(xml.GetAttrib(_T("rot")));
            GShapeTemp.strType = xml.GetAttrib(_T("roish"));
            GShapeTemp.fHeigth = (float)_ttof(xml.GetAttrib(_T("h"))) * m_fUnit;
            GShapeTemp.fWidth = (float)_ttof(xml.GetAttrib(_T("w"))) * m_fUnit;

            xml.IntoElem();
            if (xml.FindElem(_T("rc")))
            {
                GShapeTemp.fWidth = (float)_ttof(xml.GetAttrib(_T("h"))) * m_fUnit;
                GShapeTemp.fHeigth = (float)_ttof(xml.GetAttrib(_T("w"))) * m_fUnit;
                //GShapeTemp.nOrientation = GShapeTemp.nOrientation;
                GShapeTemp.fOffsetX = (float)_ttof(xml.GetAttrib(_T("offx"))) * m_fUnit;
                GShapeTemp.fOffsetY = (float)_ttof(xml.GetAttrib(_T("offy"))) * m_fUnit;
                GShapeTemp.nGerberType = TYPE_COMPONENT;
            }
            else if (xml.FindElem(_T("ob")))
            {
                GShapeTemp.nOrientation = _ttoi(xml.GetAttrib(_T("rot")));
                GShapeTemp.fHeigth = (float)_ttof(xml.GetAttrib(_T("h"))) * m_fUnit;
                GShapeTemp.fWidth = (float)_ttof(xml.GetAttrib(_T("w"))) * m_fUnit;
                GShapeTemp.fOffsetX = (float)_ttof(xml.GetAttrib(_T("offx"))) * m_fUnit;
                GShapeTemp.fOffsetY = (float)_ttof(xml.GetAttrib(_T("offy"))) * m_fUnit;
                GShapeTemp.nGerberType = TYPE_BALL;
            }
            else if (xml.FindElem(_T("cir")))
            {
                GShapeTemp.fHeigth = (float)_ttof(xml.GetAttrib(_T("dia"))) * m_fUnit;
                GShapeTemp.fWidth = GShapeTemp.fHeigth;
                GShapeTemp.fOffsetX = (float)_ttof(xml.GetAttrib(_T("offx"))) * m_fUnit;
                GShapeTemp.fOffsetY = (float)_ttof(xml.GetAttrib(_T("offy"))) * m_fUnit;
                GShapeTemp.nGerberType = TYPE_PAD;
            }
            else if (xml.FindElem(_T("poly")))
            {
                GShapeTemp.fHeigth = GShapeTemp.fHeigth;
                GShapeTemp.fWidth = GShapeTemp.fWidth;
                xml.IntoElem();
                if (xml.FindElem(_T("pt")))
                {
                    GShapeTemp.fOffsetX = (float)_ttof(xml.GetAttrib(_T("x"))) * m_fUnit;
                    GShapeTemp.fOffsetY = (float)_ttof(xml.GetAttrib(_T("y"))) * m_fUnit;
                }

                if (xml.FindElem(_T("arc")))
                {
                    GShapeTemp.nGerberType = TYPE_PAD;
                }
                else if (xml.FindElem(_T("pt")))
                {
                    GShapeTemp.nGerberType = TYPE_LAND;
                }
                xml.OutOfElem();
            }
            xml.OutOfElem();
            m_vecstShapeInfo.push_back(GShapeTemp);
        }
    }
    while (bPDSEnd);
    xml.OutOfElem();

    xml.FindElem(_T("fps"));
    xml.IntoElem();
    BOOL bFPSEnd = FALSE;
    do
    {
        bFPSEnd = xml.FindElem(_T("fp"));
        if (bFPSEnd)
        {
            GXMLFpsTemp.strName = xml.GetAttrib(_T("name"));

            xml.IntoElem();
            xml.FindElem(_T("pin"));
            GXMLFpsTemp.nShapeNum = _ttoi(xml.GetAttrib(_T("pd")));
            xml.OutOfElem();

            m_vecst_XML_FPS.push_back(GXMLFpsTemp);
        }
    }
    while (bFPSEnd);
    xml.OutOfElem();

    xml.FindElem(_T("pas"));
    xml.IntoElem();
    BOOL bPasEnd = FALSE;
    do
    {
        bPasEnd = xml.FindElem(_T("pa"));
        if (bPasEnd)
        {
            GXMLPasTemp.strName = xml.GetAttrib(_T("name"));
            GXMLPasTemp.strFoot = xml.GetAttrib(_T("foot"));

            m_vecst_XML_PAS.push_back(GXMLPasTemp);
        }
    }
    while (bPasEnd);

    xml.OutOfElem();
}

void CMapDataConverterDlg::Get_GerberObjectInfo_XML(CMarkup& xml)
{
    m_vecstObjectInfo.clear();
    Gerber_Object GObjectTemp;
    long nCount = 0;
    CString strTemp;

    xml.FindElem(_T("coms"));
    xml.IntoElem();
    BOOL bGroupEnd = FALSE; // 파일이 끝날때까지 읽는다.
    do
    {
        bGroupEnd = xml.FindElem(_T("group"));

        if (bGroupEnd)
        {
            xml.IntoElem();
            BOOL bComEnd = FALSE; // 파일이 끝날때까지 읽는다.

            do
            {
                bComEnd = xml.FindElem(_T("com"));
                if (bComEnd)
                {
                    GObjectTemp.nID = nCount++;
                    GObjectTemp.fRotation = (float)_ttof(xml.GetAttrib(_T("rot")));
                    GObjectTemp.fPosX = (float)_ttof(xml.GetAttrib(_T("x"))) * m_fUnit;
                    ;
                    GObjectTemp.fPosY = (float)_ttof(xml.GetAttrib(_T("y"))) * m_fUnit;

                    strTemp = xml.GetAttrib(_T("pa"));
                    for (long nPas = 0; nPas < (long)m_vecst_XML_PAS.size();
                        nPas++) // 여기서 Shape Number를 매칭시킨다. 너무 빙빙 둘러가니까..
                    {
                        if (strTemp == m_vecst_XML_PAS[nPas].strName)
                        {
                            for (long nFps = 0; nFps < (long)m_vecst_XML_FPS.size(); nFps++)
                            {
                                if (m_vecst_XML_PAS[nPas].strFoot == m_vecst_XML_FPS[nPas].strName)
                                {
                                    GObjectTemp.nShapeNumber = m_vecst_XML_FPS[nPas].nShapeNum; // 매칭 완료
                                    break;
                                }
                            }
                            break;
                        }
                    }

                    GObjectTemp.nPadType = 2;

                    m_vecstObjectInfo.push_back(GObjectTemp); // 최종 저장
                }
            }
            while (bComEnd);

            xml.OutOfElem();
        }
    }
    while (bGroupEnd);

    xml.OutOfElem();
}

bool CMapDataConverterDlg::ConvertToCSV()
{
    bool bBall = false;
    bool bPad = false;
    bool bComp = false;
    bool bLand = false;

    CheckMapDataTypes(bBall, bComp, bPad, bLand);

    typedef std::basic_string<char>::size_type temp_string_size_type;

    if (bBall)
    {
        std::ofstream outfile(
            (m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4)) + _T("_ball.csv"));

        outfile << "PinName,Xcoord(mm),Ycoord(mm),Diameter,Height,GroupID" << std::endl;

        for (int i = 0; i < m_vecstObjectInfo.size(); i++)
        {
            if (m_vecstObjectInfo[i].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_BALL)
            {
                float fPosX = 0.;
                float fPosY = 0.;

                GetPosition(m_vecstObjectInfo[i].fPosX, m_vecstObjectInfo[i].fPosY, fPosX, fPosY);

                outfile << "T" << m_vecstObjectInfo[i].nID << "," << fPosX * 0.001 << "," << fPosY * 0.001 << ","
                        << m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].fHeigth * 0.001 << ","
                        << m_vecstObjectInfo[i].fTickness << "," << 1 << std::endl;
            }
        }

        convMapData.push_back(std::make_pair(
            1, (m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4)) + _T("_ball.csv")));
    }

    if (bLand)
    {
        std::ofstream outfile(
            (m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4)) + _T("_land.csv"));

        outfile << "PinName,Xcoord(mm),Ycoord(mm),Width(mm),Length(mm),Height(mm),Angle(degree),GroupID,Visible"
                << std::endl;

        for (int i = 0; i < m_vecstObjectInfo.size(); i++)
        {
            if (m_vecstObjectInfo[i].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_LAND)
            {
                float fPosX = 0.;
                float fPosY = 0.;

                GetPosition(m_vecstObjectInfo[i].fPosX, m_vecstObjectInfo[i].fPosY, fPosX, fPosY);

                CStringA strVisible = "O";
                if (!m_vecstObjectInfo[i].bVisible)
                    strVisible = "X";

                outfile << "T" << m_vecstObjectInfo[i].nID << "," << fPosX * 0.001 << "," << fPosY * 0.001 << ","
                        << m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].fWidth * 0.001 << ","
                        << m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].fHeigth * 0.001 << ","
                        << m_vecstObjectInfo[i].fTickness << ","
                        << abs(m_vecstObjectInfo[i].fRotation
                               - m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nOrientation)
                        + 180
                        << "," << m_vecstObjectInfo[i].nLandGroupNo << "," << strVisible << std::endl;
            }
        }

        convMapData.push_back(std::make_pair(
            2, (m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4)) + _T("_land.csv")));
    }

    if (bComp)
    {
        std::ofstream outfile(
            (m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4)) + _T("_mount_pad.csv"));

        outfile << "CompName,Xcoord(mm),Ycoord(mm),CompType,Width(mm),Length(mm),Height(mm),Angle(degree),Pad_ID1,Pad_"
                   "ID2,Visible"
                << std::endl;

        for (int i = 0; i < m_vecstObjectInfo.size(); i++)
        {
            if (m_vecstObjectInfo[i].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_COMPONENT)
            {
                float fPosX = 0.;
                float fPosY = 0.;

                GetPosition(m_vecstObjectInfo[i].fPosX, m_vecstObjectInfo[i].fPosY, fPosX, fPosY);

                CStringA strVisible = "O";
                //if (!m_vecstObjectInfo[i].bVisible)	strVisible = "X";

                outfile << "T" << m_vecstObjectInfo[i].nID << "," << fPosX * 0.001 << "," << fPosY * 0.001 << ","
                        << "C1106_02_270"
                        << "," << m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].fWidth * 0.001 << ","
                        << m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].fHeigth * 0.001 << ","
                        << m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].fHeigth * 0.001 << ","
                        << m_vecstObjectInfo[i].fRotation << ","
                        << "NULL"
                        << ","
                        << "NULL"
                        << "," << strVisible << std::endl;
            }
        }

        std::ofstream outfile2(
            (m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4)) + _T("_comp.csv"));

        outfile2 << "CompName,Xcoord(mm),Ycoord(mm),CompType,Width(mm),Length(mm),Height(mm),Angle(degree),Pad_ID1,Pad_"
                    "ID2,Visible"
                 << std::endl;

        for (int i = 0; i < m_vecCompInfo.size(); i++)
        {
            CStringA strVisible = "O";
            //if (m_vecCompInfo[i].bIgnore)	strVisible = "X";
            // 수정 필요
            ///outfile2 << CStringA(m_vecCompInfo[i].strCompName) << "," << m_vecCompInfo[i].fOffsetX * 0.001 << "," << m_vecCompInfo[i].fOffsetY * 0.001 << "," << CStringA(m_vecCompInfo[i].strCompType) << "," << m_vecCompInfo[i].fWidth << "," << m_vecCompInfo[i].fLength << "," << m_vecCompInfo[i].fHeight << "," << m_vecCompInfo[i].nAngle << "," << CStringA(m_vecCompInfo[i].strPad_ID1) << "," << CStringA(m_vecCompInfo[i].strPad_ID2) << "," << strVisible << std::endl;
        }

        convMapData.push_back(std::make_pair(
            3, (m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4)) + _T("_comp.csv")));
    }

    if (bPad)
    {
        std::ofstream outfile((m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4)) + _T("_pad.csv"));
        outfile << "PadName,Xcoord(mm),Ycoord(mm),Width(mm),Length(mm),Height(mm),Angle(degree),Type,Mount,Visible"
                << std::endl;

        for (int i = 0; i < m_vecstObjectInfo.size(); i++)
        {
            if (m_vecstObjectInfo[i].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_PAD)
            {
                float fPosX = 0.;
                float fPosY = 0.;

                GetPosition(m_vecstObjectInfo[i].fPosX, m_vecstObjectInfo[i].fPosY, fPosX, fPosY);

                CStringA strPadType;
                if (m_vecstObjectInfo[i].nPadType == _typeRectangle)
                    strPadType = _T("Rectangle");
                else if (m_vecstObjectInfo[i].nPadType == _typeCircle)
                    strPadType = _T("Circle");
                else if (m_vecstObjectInfo[i].nPadType == _typeTriangle)
                    strPadType = _T("Triangle");
                else if (m_vecstObjectInfo[i].nPadType == _typePinIndex)
                    strPadType = _T("Pinindex");

                CStringA strVisible = "O";
                if (!m_vecstObjectInfo[i].bVisible)
                    strVisible = "X";

                outfile << "T" << m_vecstObjectInfo[i].nID << "," << fPosX * 0.001 << "," << fPosY * 0.001 << ","
                        << m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].fWidth * 0.001 << ","
                        << m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].fHeigth * 0.001 << ","
                        << "NULL"
                        << "," << m_vecstObjectInfo[i].fRotation << "," << strPadType << ","
                        << "NO"
                        << "," << strVisible << std::endl;
            }
        }

        convMapData.push_back(std::make_pair(
            0, (m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4)) + _T("_pad.csv")));
    }

    return TRUE;
}

bool CMapDataConverterDlg::CheckMapDataTypes(bool& Ball, bool& Comp, bool& Pad, bool& Land)
{
    ::DeleteFile(m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4) + _T("_ball.csv"));
    ::DeleteFile(m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4) + _T("_comp.csv"));
    ::DeleteFile(m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4) + _T("_pad.csv"));
    ::DeleteFile(m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4) + _T("_land.csv"));

    for (int i = 0; i < m_vecstObjectInfo.size(); i++)
    {
        if (m_vecstObjectInfo[i].nShapeNumber == 0)
            continue;

        if (!Ball)
        {
            if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_BALL)
            {
                Ball = true;
                CFile file;
                CString m_fileName
                    = m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4) + _T("_ball.csv");
                if (!file.Open(m_fileName, CFile::modeCreate | CFile::modeWrite))
                {
                    return false;
                }
                file.Close();
            }
        }

        if (!Comp)
        {
            if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_COMPONENT)
            {
                Comp = true;
                CFile file;
                CString m_fileName
                    = m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4) + _T("_comp.csv");
                if (!file.Open(m_fileName, CFile::modeCreate | CFile::modeWrite))
                {
                    return false;
                }
                file.Close();
            }
        }

        if (!Pad)
        {
            if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_PAD)
            {
                Pad = true;
                CFile file;
                CString m_fileName
                    = m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4) + _T("_pad.csv");
                if (!file.Open(m_fileName, CFile::modeCreate | CFile::modeWrite))
                {
                    return false;
                }
                file.Close();
            }
        }

        if (!Land)
        {
            if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_LAND)
            {
                Land = true;
                CFile file;
                CString m_fileName
                    = m_strImportFilePathName.Left(m_strImportFilePathName.GetLength() - 4) + _T("_land.csv");
                if (!file.Open(m_fileName, CFile::modeCreate | CFile::modeWrite))
                {
                    return false;
                }
                file.Close();
            }
        }
    }

    return true;
}

bool CMapDataConverterDlg::GetPosition(const float fPosX, const float fPosY, float& outPosX, float& outPosY)
{
    outPosX = fPosX - (float)(m_stHeaderInfo.fPackageSizeX / 2);
    outPosY = fPosY - (float)(m_stHeaderInfo.fPackageSizeY / 2);

    return true;
}

void CMapDataConverterDlg::ClearMapData()
{
    m_stHeaderInfo.Init();
    m_vecstShapeInfo.clear();
    m_vecst_XML_PAS.clear();
    m_vecst_XML_FPS.clear();
    m_vecstObjectInfo.clear();
    m_pGridMapDataType->ResetContent();

    Calc_DrawingPosition_Ball();
    Calc_DrawingPosition_Land();
    Calc_DrawingPosition_Comp();
    Calc_DrawingPosition_Pad();

    Update_Background(long(m_MapDataType));
    Update_Foreground(long(m_MapDataType));

    Invalidate(FALSE);
}

void CMapDataConverterDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CRect ViewRect;
    m_stcViewer.GetWindowRect(ViewRect);
    ScreenToClient(ViewRect);

    CDC MemDC;
    MemDC.CreateCompatibleDC(&dc);
    CBitmap* OldBitmap = MemDC.SelectObject(&m_bitmapView);

    dc.StretchBlt(ViewRect.left, ViewRect.top, ViewRect.Width() + (long)m_fZoomingValue,
        ViewRect.Height() + (long)m_fZoomingValue, &MemDC, 0, 0, ViewRect.Width(), ViewRect.Height(), SRCCOPY);

    MemDC.SelectObject(OldBitmap);
}

void CMapDataConverterDlg::SetMenu()
{
    CMenu menu;
    CMenu* p_sub_Menu;
    menu.LoadMenu(IDR_MENU_CONVERTER);
    if ((m_nSelectedLayer == TYPE_COMPONENT) && (m_nSelectedGroup != -1))
        p_sub_Menu = menu.GetSubMenu(0);
    else if ((m_nSelectedLayer == TYPE_PAD) && (m_nSelectedGroup != -1))
        p_sub_Menu = menu.GetSubMenu(1);
    else if ((m_nSelectedLayer == TYPE_LAND) && (m_nSelectedGroup != -1))
        p_sub_Menu = menu.GetSubMenu(2);
    else if ((m_nSelectedLayer == TYPE_BALL) && (m_nSelectedGroup != -1))
        p_sub_Menu = menu.GetSubMenu(3);
    else if ((m_nSelectedLayer == TYPE_COMPONENT) && (m_bVerifyMode == TRUE))
        p_sub_Menu = menu.GetSubMenu(5);
    else
        p_sub_Menu = menu.GetSubMenu(4);

    // 컨텍스트 메뉴 호출
    GetCursorPos(&m_ptMenu);
    p_sub_Menu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, m_ptMenu.x, m_ptMenu.y, this);

    menu.DestroyMenu();
}

void CMapDataConverterDlg::Set_SelectedData(CRect i_rtSelect, bool bOneClick)
{
    CRect rtTemp;

    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
        {
            CRect rtSpecROI = Ipvm::ToMFC(m_vecstObjectInfo[n].fsrtSpecROI.GetCRect());

            if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
            {
                if (m_vecstObjectInfo[n].bSelected)
                    m_vecstObjectInfo[n].bSelected = false;
                else
                    m_vecstObjectInfo[n].bSelected = true;
            }
            continue;
        }

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
        {
            if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
            {
                CRect rtSpecROI = Ipvm::ToMFC(m_vecstObjectInfo[n].fsrtSpecROI.GetCRect());

                if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
                {
                    if (m_vecstObjectInfo[n].bSelected)
                        m_vecstObjectInfo[n].bSelected = false;
                    else
                        m_vecstObjectInfo[n].bSelected = true;
                }
            }
        }
    }

    if (m_bVerifyMode)
    {
        std::vector<long> vecSelIndex;
        for (long n = 0; n < m_vecCompInfo.size(); n++)
        {
            CRect rtSpecROI = Ipvm::ToMFC(m_vecCompInfo[n].fsrtROI.GetCRect());

            if (rtTemp.IntersectRect(i_rtSelect, rtSpecROI))
            {
                if (!bOneClick)
                {
                    // ignore를 선택으로 사용
                    if (m_vecCompInfo[n].bIgnore)
                        m_vecCompInfo[n].bIgnore = false;
                    else
                        m_vecCompInfo[n].bIgnore = true;
                }
                else
                {
                    // 겹쳤을 경우 제일 위에 있는 데이터 선택하기
                    vecSelIndex.push_back(n);
                }
            }
        }
        if (bOneClick)
            ClearSelection(vecSelIndex);
    }
}

void CMapDataConverterDlg::ClearSelection(std::vector<long> i_idx)
{
    if (i_idx.size() <= 0)
        return;

    for (long n = 0; n < i_idx.size() - 1; n++)
    {
        if (m_vecCompInfo[i_idx[n]].bIgnore)
            m_vecCompInfo[i_idx[n]].bIgnore = false;
    }

    if (m_vecCompInfo[i_idx[i_idx.size() - 1]].bIgnore)
        m_vecCompInfo[i_idx[i_idx.size() - 1]].bIgnore = false;
    else
        m_vecCompInfo[i_idx[i_idx.size() - 1]].bIgnore = true;
}

void CMapDataConverterDlg::Select_AllData()
{
    if (!m_bVerifyMode)
    {
        for (long n = 0; n < m_vecstObjectInfo.size(); n++)
        {
            if (m_vecstObjectInfo[n].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
            {
                if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
                {
                    m_vecstObjectInfo[n].bSelected = true;
                }
            }
        }
    }
    else
    {
        for (long n = 0; n < m_vecCompInfo.size(); n++)
        {
            m_vecCompInfo[n].bIgnore = true;
        }
    }

    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);

    Invalidate(FALSE);
}

void CMapDataConverterDlg::Unselect_AllData()
{
    if (!m_bVerifyMode)
    {
        for (long n = 0; n < m_vecstObjectInfo.size(); n++)
        {
            if (m_vecstObjectInfo[n].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
            {
                if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
                {
                    m_vecstObjectInfo[n].bSelected = false;
                }
            }
        }
    }
    else
    {
        for (long n = 0; n < m_vecCompInfo.size(); n++)
        {
            m_vecCompInfo[n].bIgnore = false;
        }
    }

    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);

    Invalidate(FALSE);
}

void CMapDataConverterDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_bLButtonDown == TRUE)
    {
        CRect rtViewer;
        m_stcViewer.GetWindowRect(rtViewer);
        ScreenToClient(rtViewer);

        CPoint ptRelativePos(point);
        ptRelativePos -= rtViewer.TopLeft();

        m_ptSelection[1].x = ptRelativePos.x + 1;
        m_ptSelection[1].y = ptRelativePos.y + 1;

        Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);

        Invalidate(FALSE);
    }

    CDialog::OnMouseMove(nFlags, point);
}

void CMapDataConverterDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_bLButtonDown = TRUE;

    SetCapture();

    CRect rtViewer;
    m_stcViewer.GetWindowRect(rtViewer);
    ScreenToClient(rtViewer);

    CPoint ptRelativePos(point);
    ptRelativePos -= rtViewer.TopLeft();

    m_ptSelection[0].x = ptRelativePos.x;
    m_ptSelection[0].y = ptRelativePos.y;
    m_ptSelection[1].x = ptRelativePos.x + 1;
    m_ptSelection[1].y = ptRelativePos.y + 1;

    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);

    Invalidate(FALSE);

    CDialog::OnLButtonDown(nFlags, point);
}

void CMapDataConverterDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    short iKeyDown = GetAsyncKeyState(VK_CONTROL);

    if (iKeyDown == 0)
    {
    }

    if (m_bLButtonDown)
    {
        ReleaseCapture();

        CRect rtViewer;
        m_stcViewer.GetWindowRect(rtViewer);
        ScreenToClient(rtViewer);

        CPoint ptRelativePos(point);
        ptRelativePos -= rtViewer.TopLeft();

        m_ptSelection[1].x = ptRelativePos.x + 1;
        m_ptSelection[1].y = ptRelativePos.y + 1;

        CRect rtSelection;
        rtSelection.left = min(m_ptSelection[0].x, m_ptSelection[1].x);
        rtSelection.top = min(m_ptSelection[0].y, m_ptSelection[1].y);
        rtSelection.right = max(m_ptSelection[0].x, m_ptSelection[1].x) + 1;
        rtSelection.bottom = max(m_ptSelection[0].y, m_ptSelection[1].y) + 1;

        if ((rtSelection.right - rtSelection.left) == 2 && (rtSelection.bottom - rtSelection.top) == 2)
        {
            Set_SelectedData(rtSelection, TRUE);
        }
        else
        {
            Set_SelectedData(rtSelection);
        }

        m_ptSelection[0].x = 0;
        m_ptSelection[0].y = 0;
        m_ptSelection[1].x = 0;
        m_ptSelection[1].y = 0;

        Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
        Invalidate(FALSE);

        m_bLButtonDown = FALSE;
    }

    CDialog::OnLButtonUp(nFlags, point);
}

void CMapDataConverterDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
    CRect rtViewer;
    m_stcViewer.GetWindowRect(rtViewer);
    ScreenToClient(rtViewer);

    CPoint ptRelativePos(point);
    ptRelativePos -= rtViewer.TopLeft();

    m_ptMenu = ptRelativePos;

    SetMenu();

    CDialog::OnRButtonUp(nFlags, point);
}

BOOL CMapDataConverterDlg::CompTypeConvert()
{
    if (m_vecCompInfo.size() <= 0)
        return FALSE;

    BOOL hRes = TRUE;

    //std::regex pattern("(^[a-zA-Z]{1,5})[0-9_]*");
    std::regex pattern("(^[a-zA-Z]{1,5})[0-9_]*[0-9a-zA-Z_]*");

    for (auto& OriginCompData : m_vecCompInfo)
    {
        //std::string compType = CT2A(OriginCompData.strCompType);
        CT2A asciiStr(OriginCompData.strCompType);
        std::string compType(asciiStr);
        CString strNewCompType = _T("");
        CString strCompName = _T("");
        std::smatch m;

        if (std::regex_match(compType, m, pattern))
        {
            std::string strMatch = m[1].str();

            long tmpCalcAngle = 0;
            tmpCalcAngle = OriginCompData.nAngle;

            strNewCompType.Format(_T("%S%s%s_%s_%03d"), strMatch.c_str(),
                (LPCTSTR)CompTypeFloattoString(OriginCompData.fLength),
                (LPCTSTR)CompTypeFloattoString(OriginCompData.fWidth),
                (LPCTSTR)CompTypeFloattoString(OriginCompData.fHeight), tmpCalcAngle);

            OriginCompData.strCompType = strNewCompType;

            /*if (strMatch == "D")
			{
				OriginCompData.nCompType = _typeChip;
				m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID1)].nCompChipType = _typeChip;
				if(!m_bVerifyMode)
					m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID2)].nCompChipType = _typeChip;

				nDieIdx++;
				strCompName.Format(_T("D%d"), nDieIdx);
				OriginCompData.strCompName = strCompName;
			}
			else if (strMatch == "C" || strMatch == "Capacitor" || strMatch == "R" || strMatch == "Resistor"
				|| strMatch == "T" || strMatch == "Tantalum" || strMatch == "LSC")
			{
				OriginCompData.nCompType = _typePassive;
				m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID1)].nCompChipType = _typePassive;
				if (!m_bVerifyMode)
					m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID2)].nCompChipType = _typePassive;

				if (strMatch == "C" || strMatch == "Capacitor")
				{
					m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID1)].nCompPassiveType = PassiveType_Capacitor;
					if (!m_bVerifyMode)
						m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID2)].nCompPassiveType = PassiveType_Capacitor;

					nCapacitorIdx++;
					strCompName.Format(_T("C%d"), nCapacitorIdx);
					OriginCompData.strCompName = strCompName;
				}
				else if (strMatch == "R" || strMatch == "Resistor")
				{
					m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID1)].nCompPassiveType = PassiveType_Resitor;
					if (!m_bVerifyMode)
						m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID2)].nCompPassiveType = PassiveType_Resitor;

					nResistorIdx++;
					strCompName.Format(_T("R%d"), nResistorIdx);
					OriginCompData.strCompName = strCompName;
				}
				else if (strMatch == "T" || strMatch == "Tantalum")
				{
					m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID1)].nCompPassiveType = PassiveType_Tantalum;
					if (!m_bVerifyMode)
						m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID2)].nCompPassiveType = PassiveType_Tantalum;

					nTantalumIdx++;
					strCompName.Format(_T("T%d"), nTantalumIdx);
					OriginCompData.strCompName = strCompName;
				}
				else if (strMatch == "LSC")
				{
					m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID1)].nCompPassiveType = PassiveType_LandSideCapacitor;
					if (!m_bVerifyMode)
						m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID2)].nCompPassiveType = PassiveType_LandSideCapacitor;

					nLSCIdx++;
					strCompName.Format(_T("LSC%d"), nLSCIdx);
					OriginCompData.strCompName = strCompName;
				}
			}
			else if (strMatch == "A" || strMatch == "Array")
			{
				OriginCompData.nCompType = _typeArray;
				m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID1)].nCompChipType = _typeArray;
				if (!m_bVerifyMode)
					m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID2)].nCompChipType = _typeArray;

				nArrayIdx++;
				strCompName.Format(_T("A%d"), nArrayIdx);
				OriginCompData.strCompName = strCompName;
			}
			else if (strMatch == "H" || strMatch == "HeatSync")
			{
				OriginCompData.nCompType = _typeHeatSync;
				m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID1)].nCompChipType = _typeHeatSync;
				if (!m_bVerifyMode)
					m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID2)].nCompChipType = _typeHeatSync;

				nHeatSyncIdx++;
				strCompName.Format(_T("H%d"), nHeatSyncIdx);
				OriginCompData.strCompName = strCompName;
			}
			else if (strMatch == "P" || strMatch == "Patch" || strMatch == "PATCH")//kircheis_POI
			{
				OriginCompData.nCompType = _typePatch;
				m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID1)].nCompChipType = _typePatch;
				if (!m_bVerifyMode)
					m_vecstObjectInfo[_ttoi(OriginCompData.strPad_ID2)].nCompChipType = _typePatch;

				nPatchIdx++;
				strCompName.Format(_T("P%d"), nPatchIdx);
				OriginCompData.strCompName = strCompName;
			}*/
        }
        else
        {
            // TypeName이 문자로 시작하지 않으면 FALSE
            ::AfxMessageBox(_T("Component type is not match."));
            hRes = FALSE;
            break;
        }
    }

    return hRes;
}

CString CMapDataConverterDlg::CompTypeFloattoString(float fValue)
{
    long resultVal(0);
    CString strResult = _T("");
    CString tmpVal = _T("");
    tmpVal.Format(_T("%f"), fValue);

    int index = tmpVal.ReverseFind('.');

    if (index == 1)
    {
        resultVal = (long)(fValue * 10);
        if (fValue < 1)
            strResult.Format(_T("0%d"), resultVal);
        else
            strResult.Format(_T("%d"), resultVal);
    }
    else if (index == 2)
    {
        resultVal = (long)(fValue * 100);
        if (fValue < 1)
            strResult.Format(_T("0%d"), resultVal);
        else
            strResult.Format(_T("%d"), resultVal);
    }
    else if (index == 3)
    {
        resultVal = (long)(fValue * 1000);
        if (fValue < 1)
            strResult.Format(_T("0%d"), resultVal);
        else
            strResult.Format(_T("%d"), resultVal);
    }

    return strResult;
}

float CMapDataConverterDlg::round(float var)
{
    char str[40];
    sprintf(str, "%.1f", var);
    sscanf(str, "%f", &var);
    return var;
}

void CMapDataConverterDlg::OnComponentmenuEditdimensions()
{
    m_vecSelectedCompInfo.clear();

    long nCompIdx = 0;

    bool isSelected = false;

    if (m_vecCompInfo.size() == 0)
    {
        AfxMessageBox(_T("There is not have a Chip"));
        return;
    }

    std::vector<CString> compType;
    long nFirstComp = -1;
    if (!m_bVerifyMode)
    {
        for (long n = 0; n < m_vecstObjectInfo.size(); n++)
        {
            if (m_vecstObjectInfo[n].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
            {
                if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
                {
                    if (m_vecstObjectInfo[n].bSelected)
                    {
                        nCompIdx = GetComponentIndexFromMountMap(m_vecstObjectInfo[n].nID);

                        if (nCompIdx != -1)
                        {
                            if (!isSelected)
                                nFirstComp = nCompIdx;
                            isSelected = true;

                            BOOL bChipOverlapValue
                                = std::find(compType.begin(), compType.end(), m_vecCompInfo[nCompIdx].strCompType)
                                != compType.end();
                            if (!bChipOverlapValue)
                            {
                                compType.push_back(m_vecCompInfo[nCompIdx].strCompType);
                                m_vecSelectedCompInfo.push_back(m_vecCompInfo[nCompIdx]);
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (long n = 0; n < m_vecCompInfo.size(); n++)
        {
            if (m_vecCompInfo[n].bIgnore)
            {
                if (!isSelected)
                    nFirstComp = n;
                isSelected = true;

                BOOL bChipOverlapValue
                    = std::find(compType.begin(), compType.end(), m_vecCompInfo[n].strCompType) != compType.end();
                if (!bChipOverlapValue)
                {
                    compType.push_back(m_vecCompInfo[n].strCompType);
                    m_vecSelectedCompInfo.push_back(m_vecCompInfo[n]);
                }
            }
        }
    }

    if (!isSelected)
    {
        AfxMessageBox(_T("Not have a selected chip"));
        return;
    }

    //CompDBVerify(m_vecCompInfo[nFirstComp]);

    Update_Background(m_nSelectedLayer);
    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
    Invalidate(FALSE);

    CompTypeConvert();
}

long CMapDataConverterDlg::GetComponentIndexFromMountMap(long nPadID)
{
    long nComponentIndex = -1;
    CString strPadID;
    strPadID.Format(_T("%ld"), nPadID);

    for (long n = 0; n < m_vecCompInfo.size(); n++)
    {
        //if (m_vecCompInfo[n].strPad_ID1 == strPadID || m_vecCompInfo[n].strPad_ID2 == strPadID)
        {
            nComponentIndex = n;
            break;
        }
    }

    return nComponentIndex;
}

void CMapDataConverterDlg::ClearAllComponents()
{
    m_vecCompInfo.clear();
    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == TYPE_COMPONENT)
        {
            m_vecstObjectInfo[n].nCompGrouping = 0;
        }
    }
}

void CMapDataConverterDlg::OnGroupmenuMakegroup()
{
    if (m_nAutoGroup != 1)
    {
        long nPadMount1 = 0;
        long nPadMount2 = 0;
        if (CheckPadGrouping(nPadMount1, nPadMount2))
        {
            CreateComponent(nPadMount1, nPadMount2);
            Unselect_AllData();
        }
    }

    Update_Background(m_nSelectedLayer);
    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
    Invalidate(FALSE);
}

void CMapDataConverterDlg::OnComponentmenuRemovegroup()
{
    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
        {
            if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
            {
                if (m_vecstObjectInfo[n].bSelected)
                {
                    long compIndex = GetComponentIndexFromMountMap(m_vecstObjectInfo[n].nID);
                    if (compIndex != -1)
                    {
                        //RemoveGroup(_ttoi(m_vecCompInfo[compIndex].strPad_ID1), _ttoi(m_vecCompInfo[compIndex].strPad_ID2));
                        m_vecCompInfo.erase(m_vecCompInfo.begin() + compIndex);
                        m_nAutoGroup = 0;
                    }
                }
            }
        }
    }

    Update_Background(m_nSelectedLayer);
    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
    Invalidate(FALSE);

    Unselect_AllData();
}

BOOL CMapDataConverterDlg::CheckPadGrouping(long& nPadMountIdx1, long& nPadMountIdx2)
{
    long selectedCount = 0;
    float pad1_posX(0.), pad1_posY(0.);
    float pad2_posX(0.), pad2_posY(0.);
    float pad1_rot(0.), pad2_rot(0.);

    nPadMountIdx1 = 0;
    nPadMountIdx2 = 0;

    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
        {
            if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
            {
                if (m_vecstObjectInfo[n].bSelected)
                {
                    selectedCount++;
                    if (selectedCount == 1)
                    {
                        pad1_posX = m_vecstObjectInfo[n].fPosX;
                        pad1_posY = m_vecstObjectInfo[n].fPosY;
                        pad1_rot = m_vecstObjectInfo[n].fRotation;
                        nPadMountIdx1 = n;
                    }
                    else if (selectedCount == 2)
                    {
                        pad2_posX = m_vecstObjectInfo[n].fPosX;
                        pad2_posY = m_vecstObjectInfo[n].fPosY;
                        pad2_rot = m_vecstObjectInfo[n].fRotation;
                        nPadMountIdx2 = n;
                    }
                }
            }
        }
    }

    if (selectedCount != 2)
    {
        AfxMessageBox(_T("Select the 2 mount pad"));
        return FALSE;
    }

    if (pad1_posX == pad2_posX)
    {
        if (pad1_posY > pad2_posY)
        {
            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
            {
                if (m_vecstObjectInfo[n].nShapeNumber == 0)
                    continue;

                if (pad1_posX == m_vecstObjectInfo[n].fPosX)
                {
                    if ((pad1_posY > m_vecstObjectInfo[n].fPosY && m_vecstObjectInfo[n].fPosY > pad2_posY))
                    {
                        AfxMessageBox(_T("Cannot Grouping this mounts"));
                        return FALSE;
                    }
                }
            }
        }
        else
        {
            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
            {
                if (m_vecstObjectInfo[n].nShapeNumber == 0)
                    continue;

                if (pad1_posX == m_vecstObjectInfo[n].fPosX)
                {
                    if ((pad2_posY > m_vecstObjectInfo[n].fPosY && m_vecstObjectInfo[n].fPosY > pad1_posY))
                    {
                        AfxMessageBox(_T("Cannot Grouping this mounts"));
                        return FALSE;
                    }
                }
            }
        }
    }
    else if (pad1_posY == pad2_posY)
    {
        if (pad1_posX > pad2_posX)
        {
            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
            {
                if (m_vecstObjectInfo[n].nShapeNumber == 0)
                    continue;

                if (pad1_posY == m_vecstObjectInfo[n].fPosY)
                {
                    if ((pad1_posX < m_vecstObjectInfo[n].fPosX && m_vecstObjectInfo[n].fPosX < pad2_posX))
                    {
                        AfxMessageBox(_T("Cannot Grouping this mounts"));
                        return FALSE;
                    }
                }
            }
        }
        else
        {
            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
            {
                if (m_vecstObjectInfo[n].nShapeNumber == 0)
                    continue;

                if (pad1_posY == m_vecstObjectInfo[n].fPosY)
                {
                    if ((pad2_posX < m_vecstObjectInfo[n].fPosX && m_vecstObjectInfo[n].fPosX < pad1_posX))
                    {
                        AfxMessageBox(_T("Cannot Grouping this mounts"));
                        return FALSE;
                    }
                }
            }
        }
    }
    else
    {
        AfxMessageBox(_T("Cannot Grouping this mounts"));
        return FALSE;
    }

    if (pad1_rot != pad2_rot)
    {
        AfxMessageBox(_T("Rotation not Matching!!!"));
        return FALSE;
    }

    return TRUE;
}

void CMapDataConverterDlg::RemoveGroup(long nPadMountIdx1, long nPadMountIdx2)
{
    m_vecstObjectInfo[nPadMountIdx1].nCompGrouping = 0;
    m_vecstObjectInfo[nPadMountIdx2].nCompGrouping = 0;
}

void CMapDataConverterDlg::DeleteSelectedMapData()
{
    if (::SimpleMessage(_T("Warning! Are you sure you want to delete selected datas?"), MB_OKCANCEL) == IDOK)
    {
        if (!m_bVerifyMode)
        {
            std::vector<long> selDatas;
            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
            {
                if (m_vecstObjectInfo[n].nShapeNumber == 0)
                    continue;

                if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
                {
                    if (m_vecstObjectInfo[n].bSelected)
                    {
                        selDatas.push_back(m_vecstObjectInfo[n].nID);
                    }
                }
            }

            for (long i = 0; i < selDatas.size(); i++)
            {
                long dataIndex = GetDataIndexFromID(selDatas[i]);
                m_vecstObjectInfo.erase(m_vecstObjectInfo.begin() + dataIndex);
            }
        }
        else
        {
            std::vector<CString> selDatas;
            for (long n = 0; n < m_vecCompInfo.size(); n++)
            {
                if (m_vecCompInfo[n].bIgnore)
                {
                    selDatas.push_back(m_vecCompInfo[n].strCompName);
                }
            }

            for (long i = 0; i < selDatas.size(); i++)
            {
                long dataIndex = GetDataIndexFromID(selDatas[i]);
                m_vecCompInfo.erase(m_vecCompInfo.begin() + dataIndex);
            }

            std::vector<long> selObjDatas;
            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
            {
                if (m_vecstObjectInfo[n].bSelected)
                {
                    selObjDatas.push_back(m_vecstObjectInfo[n].nID);
                }
            }

            for (long i = 0; i < selObjDatas.size(); i++)
            {
                long dataIndex = GetDataIndexFromID(selObjDatas[i]);
                m_vecstObjectInfo.erase(m_vecstObjectInfo.begin() + dataIndex);
            }
        }

        Update_Background(m_nSelectedLayer);
        Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
        Invalidate(FALSE);

        Unselect_AllData();
    }
}

long CMapDataConverterDlg::GetDataIndexFromID(long dataID)
{
    long hValue = -1;
    for (long n = 0; n < m_vecstObjectInfo.size(); n++)
    {
        if (m_vecstObjectInfo[n].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
        {
            if (dataID == m_vecstObjectInfo[n].nID)
            {
                hValue = n;
                break;
            }
        }
    }
    return hValue;
}

long CMapDataConverterDlg::GetDataIndexFromID(CString dataID)
{
    long hValue = -1;
    for (long n = 0; n < m_vecCompInfo.size(); n++)
    {
        if (dataID == m_vecCompInfo[n].strCompName)
        {
            hValue = n;
            break;
        }
    }
    return hValue;
}

void CMapDataConverterDlg::OnPadmenuChangetype()
{
    long selectedCount = 0;
    long changedPadType = 2;
    CPadTypeChangerDlg c_padTypeChanger(this, changedPadType);
    if (c_padTypeChanger.DoModal() == IDOK)
    {
        if (changedPadType == 0)
        {
            for (long n = 0; n < m_vecstObjectInfo.size(); n++)
            {
                if (m_vecstObjectInfo[n].nShapeNumber == 0)
                    continue;

                if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
                {
                    if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
                    {
                        if (m_vecstObjectInfo[n].bSelected)
                        {
                            selectedCount++;
                        }

                        if (selectedCount > 1)
                        {
                            AfxMessageBox(_T("Pin Index must have one."));
                            return;
                        }
                    }
                }
            }
        }

        for (long n = 0; n < m_vecstObjectInfo.size(); n++)
        {
            if (m_vecstObjectInfo[n].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
            {
                if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
                {
                    if (m_vecstObjectInfo[n].bSelected)
                    {
                        m_vecstObjectInfo[n].nPadType = changedPadType;
                    }
                }
            }
        }
        Unselect_AllData();
    }

    Update_Background(m_nSelectedLayer);
    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
    Invalidate(FALSE);
}

void CMapDataConverterDlg::OnLandmenuChangegroupid()
{
    long GroupNo = 1;
    double LandHeight = 1;

    for (int i = 0; i < m_vecstObjectInfo.size(); i++)
    {
        if (m_vecstObjectInfo[i].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_LAND)
        {
            if (m_vecstObjectInfo[i].bSelected)
            {
                GroupNo = m_vecstObjectInfo[i].nLandGroupNo;
                LandHeight = m_vecstObjectInfo[i].fTickness;
                break;
            }
        }
    }

    CLandGroupIdChangerDlg c_landGroupIDChanger(this, GroupNo, LandHeight);
    if (c_landGroupIDChanger.DoModal() == IDOK)
    {
        for (long n = 0; n < m_vecstObjectInfo.size(); n++)
        {
            if (m_vecstObjectInfo[n].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
            {
                if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
                {
                    if (m_vecstObjectInfo[n].bSelected)
                    {
                        m_vecstObjectInfo[n].nLandGroupNo = GroupNo;
                        m_vecstObjectInfo[n].fTickness = (float)LandHeight;
                    }
                }
            }
        }
        Unselect_AllData();
    }

    Update_Background(m_nSelectedLayer);
    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
    Invalidate(FALSE);
}

void CMapDataConverterDlg::OnBallmenuBallspec()
{
    double BallHeight = 1;

    for (int i = 0; i < m_vecstObjectInfo.size(); i++)
    {
        if (m_vecstObjectInfo[i].nShapeNumber == 0)
            continue;

        if (m_vecstShapeInfo[m_vecstObjectInfo[i].nShapeNumber - 1].nGerberType == TYPE_BALL)
        {
            if (m_vecstObjectInfo[i].bSelected)
            {
                BallHeight = m_vecstObjectInfo[i].fTickness;
                break;
            }
        }
    }

    CBallDataChangerDlg BallDataChanger(this, BallHeight);
    if (BallDataChanger.DoModal() == IDOK)
    {
        for (long n = 0; n < m_vecstObjectInfo.size(); n++)
        {
            if (m_vecstObjectInfo[n].nShapeNumber == 0)
                continue;

            if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
            {
                if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
                {
                    if (m_vecstObjectInfo[n].bSelected)
                    {
                        m_vecstObjectInfo[n].fTickness = (float)BallHeight;
                    }
                }
            }
        }
        Unselect_AllData();
    }

    Update_Background(m_nSelectedLayer);
    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
    Invalidate(FALSE);
}

void CMapDataConverterDlg::OnComponentmenuAdd()
{
    Package::Component SelectedData;
    std::vector<Package::Component> vecExistCompData;

    Chip::DMSChip DmsChipInfo;
    Chip::DMSPassiveChip DMSPassiveInfo;
    Chip::DMSArrayChip DmsArrayInfo;
    Chip::DMSHeatsink DmsHeatsink;
    Chip::DMSPatch DmsPatch;
    Chip::DMSBall DmsBallInfo;
    long DmsType;

    CCompDataChangerDlg CompDataEditor(this, &m_packageSpec, &DmsChipInfo, &DMSPassiveInfo, &DmsArrayInfo, &DmsHeatsink,
        &DmsPatch, &DmsBallInfo, &DmsType, vecExistCompData, SelectedData, TRUE, m_bVerifyMode);

    if (CompDataEditor.DoModal() == IDOK)
    {
        if (DmsType == CommonItemID_DMS_TYPE_CHIP)
        {
            SelectedData.fHeight = DmsChipInfo.fChipThickness;
            SelectedData.fWidth = DmsChipInfo.fChipWidth;
            SelectedData.fLength = DmsChipInfo.fChipLength;
            SelectedData.fOffsetX = DmsChipInfo.fChipOffsetX * 1000;
            SelectedData.fOffsetY = DmsChipInfo.fChipOffsetY * 1000;
            SelectedData.nAngle = CAST_LONG(DmsChipInfo.fChipAngle);
            SelectedData.strCompType = DmsChipInfo.strCompType;
            SelectedData.strCompName = DmsChipInfo.strCompID;
            m_vecCompInfo.push_back(SelectedData);
        }
        else if (DmsType == CommonItemID_DMS_TYPE_HEATSYNC)
        {
            SelectedData.fHeight = DmsHeatsink.fChipThickness;
            SelectedData.fWidth = DmsHeatsink.fChipWidth;
            SelectedData.fLength = DmsHeatsink.fChipLength;
            SelectedData.fOffsetX = DmsHeatsink.fChipOffsetX * 1000;
            SelectedData.fOffsetY = DmsHeatsink.fChipOffsetY * 1000;
            SelectedData.nAngle = CAST_LONG(DmsHeatsink.fChipAngle);
            SelectedData.strCompType = DmsHeatsink.strCompType;
            SelectedData.strCompName = DmsHeatsink.strCompID;
            m_vecCompInfo.push_back(SelectedData);
        }
    }

    Update_Background(m_nSelectedLayer);
    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
    Invalidate(FALSE);
}

void CMapDataConverterDlg::ReEdit_Get_GerberHeaderInfo_XML(CMarkup& xml)
{
    m_stHeaderInfo.nObjectNumber = 0;
    m_stHeaderInfo.nZoneID_Column = 0;
    m_stHeaderInfo.nReflowZCompensation_Column = 0;
    m_stHeaderInfo.nConrnerBending_column = 0;
    m_stHeaderInfo.nShapeNumber = 0;
    m_stHeaderInfo.fDominantRotation = 0.f;

    xml.FindElem(_T("format"));
    m_stHeaderInfo.strVersion = xml.GetAttrib(_T("ver"));

    xml.FindElem(_T("unit"));
    m_stHeaderInfo.strUnit = xml.GetAttrib(_T("name"));
    m_stHeaderInfo.bAutoGroping = _ttoi(xml.GetAttrib(_T("autoGrouping")));

    // 분리된 Data 추출
    if (m_stHeaderInfo.strUnit.Find(_T("micron")) > -1)
        m_fUnit = 1.f;
    else if (m_stHeaderInfo.strUnit.Find(_T("mil")) > -1)
        m_fUnit = 25.4f;
    else if (m_stHeaderInfo.strUnit.Find(_T("mm")) > -1)
        m_fUnit = 1000.f;

    xml.FindElem(_T("board"));
    m_stHeaderInfo.fPackageSizeX = (float)_ttof(xml.GetAttrib(_T("w"))) * m_fUnit;
    m_stHeaderInfo.fPackageSizeY = (float)_ttof(xml.GetAttrib(_T("h"))) * m_fUnit;
}

void CMapDataConverterDlg::ReEdit_Get_GerberShapeInfo_XML(CMarkup& xml)
{
    m_vecstShapeInfo.clear();
    m_vecst_XML_PAS.clear();
    m_vecst_XML_FPS.clear();

    Gerber_Shape GShapeTemp;
    Gerber_XML_PAS GXMLPasTemp;
    Gerber_XML_FPS GXMLFpsTemp;

    CString strTemp;

    xml.FindElem(_T("pds"));
    xml.IntoElem();
    BOOL bPDSEnd = FALSE;
    do
    {
        bPDSEnd = xml.FindElem(_T("pd"));
        if (bPDSEnd)
        {
            GShapeTemp.strBalltop = _T("");
            GShapeTemp.nRounding = 0;
            GShapeTemp.nOrientation = _ttoi(xml.GetAttrib(_T("rot")));
            GShapeTemp.strType = xml.GetAttrib(_T("roish"));
            GShapeTemp.fHeigth = (float)_ttof(xml.GetAttrib(_T("h"))) * m_fUnit;
            GShapeTemp.fWidth = (float)_ttof(xml.GetAttrib(_T("w"))) * m_fUnit;
            GShapeTemp.nGerberType = _ttoi(xml.GetAttrib(_T("type")));
            ;

            xml.IntoElem();
            if (xml.FindElem(_T("rc")))
            {
                //GShapeTemp.fWidth = (float)_ttof(xml.GetAttrib(_T("h"))) * m_fUnit;
                //GShapeTemp.fHeigth = (float)_ttof(xml.GetAttrib(_T("w"))) * m_fUnit;
                GShapeTemp.fOffsetX = (float)_ttof(xml.GetAttrib(_T("offx"))) * m_fUnit;
                GShapeTemp.fOffsetY = (float)_ttof(xml.GetAttrib(_T("offy"))) * m_fUnit;
            }
            else if (xml.FindElem(_T("ob")))
            {
                GShapeTemp.nOrientation = _ttoi(xml.GetAttrib(_T("rot")));
                GShapeTemp.fHeigth = (float)_ttof(xml.GetAttrib(_T("h"))) * m_fUnit;
                GShapeTemp.fWidth = (float)_ttof(xml.GetAttrib(_T("w"))) * m_fUnit;
                GShapeTemp.fOffsetX = (float)_ttof(xml.GetAttrib(_T("offx"))) * m_fUnit;
                GShapeTemp.fOffsetY = (float)_ttof(xml.GetAttrib(_T("offy"))) * m_fUnit;
            }
            else if (xml.FindElem(_T("cir")))
            {
                GShapeTemp.fHeigth = (float)_ttof(xml.GetAttrib(_T("dia"))) * m_fUnit;
                GShapeTemp.fWidth = GShapeTemp.fHeigth;
                GShapeTemp.fOffsetX = (float)_ttof(xml.GetAttrib(_T("offx"))) * m_fUnit;
                GShapeTemp.fOffsetY = (float)_ttof(xml.GetAttrib(_T("offy"))) * m_fUnit;
            }
            else if (xml.FindElem(_T("poly")))
            {
                GShapeTemp.fHeigth = GShapeTemp.fHeigth;
                GShapeTemp.fWidth = GShapeTemp.fWidth;
                xml.IntoElem();
                if (xml.FindElem(_T("pt")))
                {
                    GShapeTemp.fOffsetX = (float)_ttof(xml.GetAttrib(_T("x"))) * m_fUnit;
                    GShapeTemp.fOffsetY = (float)_ttof(xml.GetAttrib(_T("y"))) * m_fUnit;
                }

                xml.OutOfElem();
            }
            xml.OutOfElem();
            m_vecstShapeInfo.push_back(GShapeTemp);
        }
    }
    while (bPDSEnd);
    xml.OutOfElem();

    xml.FindElem(_T("fps"));
    xml.IntoElem();
    BOOL bFPSEnd = FALSE;
    do
    {
        bFPSEnd = xml.FindElem(_T("fp"));
        if (bFPSEnd)
        {
            GXMLFpsTemp.strName = xml.GetAttrib(_T("name"));

            xml.IntoElem();
            xml.FindElem(_T("pin"));
            GXMLFpsTemp.nShapeNum = _ttoi(xml.GetAttrib(_T("pd")));
            xml.OutOfElem();

            m_vecst_XML_FPS.push_back(GXMLFpsTemp);
        }
    }
    while (bFPSEnd);
    xml.OutOfElem();

    xml.FindElem(_T("pas"));
    xml.IntoElem();
    BOOL bPasEnd = FALSE;
    do
    {
        bPasEnd = xml.FindElem(_T("pa"));
        if (bPasEnd)
        {
            GXMLPasTemp.strName = xml.GetAttrib(_T("name"));
            GXMLPasTemp.strFoot = xml.GetAttrib(_T("foot"));

            m_vecst_XML_PAS.push_back(GXMLPasTemp);
        }
    }
    while (bPasEnd);

    xml.OutOfElem();
}

void CMapDataConverterDlg::ReEdit_Get_GerberObjectInfo_XML(CMarkup& xml)
{
    m_vecstObjectInfo.clear();
    Gerber_Object GObjectTemp;
    long nCount = 0;
    CString strTemp;

    xml.FindElem(_T("coms"));
    xml.IntoElem();
    BOOL bGroupEnd = FALSE; // 파일이 끝날때까지 읽는다.
    do
    {
        bGroupEnd = xml.FindElem(_T("group"));

        if (bGroupEnd)
        {
            xml.IntoElem();
            BOOL bComEnd = FALSE; // 파일이 끝날때까지 읽는다.
            do
            {
                bComEnd = xml.FindElem(_T("com"));
                if (bComEnd)
                {
                    GObjectTemp.nID = nCount++;
                    GObjectTemp.fRotation = (float)_ttof(xml.GetAttrib(_T("rot")));
                    GObjectTemp.fPosX = (float)_ttof(xml.GetAttrib(_T("x"))) * m_fUnit;
                    ;
                    GObjectTemp.fPosY = (float)_ttof(xml.GetAttrib(_T("y"))) * m_fUnit;

                    bool bIsShape = false;
                    strTemp = xml.GetAttrib(_T("pa"));
                    for (long nPas = 0; nPas < (long)m_vecst_XML_PAS.size();
                        nPas++) // 여기서 Shape Number를 매칭시킨다. 너무 빙빙 둘러가니까..
                    {
                        if (strTemp == m_vecst_XML_PAS[nPas].strName)
                        {
                            bIsShape = true;
                            for (long nFps = 0; nFps < (long)m_vecst_XML_FPS.size(); nFps++)
                            {
                                if (m_vecst_XML_PAS[nPas].strFoot == m_vecst_XML_FPS[nPas].strName)
                                {
                                    GObjectTemp.nShapeNumber = m_vecst_XML_FPS[nPas].nShapeNum; // 매칭 완료
                                    break;
                                }
                            }
                            break;
                        }

                        if (nPas == (long)m_vecst_XML_PAS.size() - 1)
                        {
                            if (!bIsShape)
                            {
                                GObjectTemp.nShapeNumber = 0;
                            }
                        }
                    }

                    GObjectTemp.nCompGrouping = _ttoi(xml.GetAttrib(_T("compGroup")));
                    GObjectTemp.nCompChipType = _ttoi(xml.GetAttrib(_T("compChipType")));
                    GObjectTemp.nCompPassiveType = _ttoi(xml.GetAttrib(_T("compPassiveType")));
                    GObjectTemp.fTickness = (float)_ttof(xml.GetAttrib(_T("tickness")));
                    GObjectTemp.fCompWidth = (float)_ttof(xml.GetAttrib(_T("compWidth")));
                    GObjectTemp.fCompLenght = (float)_ttof(xml.GetAttrib(_T("compLenght")));
                    GObjectTemp.nLandGroupNo = _ttoi(xml.GetAttrib(_T("landGroupNo")));
                    GObjectTemp.nPadType = _ttoi(xml.GetAttrib(_T("padType")));

                    m_vecstObjectInfo.push_back(GObjectTemp); // 최종 저장
                }
            }
            while (bComEnd);

            xml.OutOfElem();
        }
    }
    while (bGroupEnd);

    xml.OutOfElem();
}

void CMapDataConverterDlg::ReEdit_SaveXMLModifyData()
{
    CMarkup xml;
    CString strFilename = DynamicSystemPath::get(DefineFolder::Temp) + _T("TempMapDataConverter.xml");

    CString strWidth;
    CString strLength;
    CString strShapeTemp;
    int nShapeSize = 0;

    xml.AddElem(_T("ePM-SPI"));
    xml.IntoElem();

    xml.AddElem(_T("format"));
    xml.AddAttrib(_T("name"), _T("ePM_Demo"));
    xml.AddAttrib(_T("ver"), _T("1.0"));

    xml.AddElem(_T("unit"));
    xml.AddAttrib(_T("name"), _T("mm"));
    xml.AddAttrib(_T("decpt"), 3);
    xml.AddAttrib(_T("coord"), _T("UL"));
    xml.AddAttrib(_T("autoGrouping"), m_nAutoGroup);

    xml.AddElem(_T("board"));
    strWidth.Format(_T("%.3f"), m_stHeaderInfo.fPackageSizeX * 0.001);
    strLength.Format(_T("%.3f"), m_stHeaderInfo.fPackageSizeY * 0.001);
    xml.AddAttrib(_T("name"), _T("modify_data"));
    xml.AddAttrib(_T("w"), strWidth);
    xml.AddAttrib(_T("h"), strLength);

    xml.AddElem(_T("pds")); // -------------------- shape 등록
    xml.IntoElem();

    Gerber_Shape GShapeTemp;
    Gerber_XML_PAS GXMLPasTemp;
    Gerber_XML_FPS GXMLFpsTemp;

    CString strTemp;
    CString fOffsetX;
    CString fOffsetY;

    for (long n = 0; n < m_vecstShapeInfo.size(); n++)
    {
        nShapeSize++;
        xml.AddElem(_T("pd"));
        xml.AddAttrib(_T("num"), nShapeSize);
        xml.AddAttrib(_T("roish"), m_vecstShapeInfo[n].strType);
        strWidth.Format(_T("%.3f"), m_vecstShapeInfo[n].fWidth * 0.001);
        strLength.Format(_T("%.3f"), m_vecstShapeInfo[n].fHeigth * 0.001);
        xml.AddAttrib(_T("w"), strWidth);
        xml.AddAttrib(_T("h"), strLength);

        xml.AddAttrib(_T("rot"), m_vecstShapeInfo[n].nOrientation);
        if (m_vecstShapeInfo[n].strType == _T("rc"))
        {
            if (m_vecstShapeInfo[n].nGerberType != 3)
            {
                fOffsetX.Format(_T("%.3f"), m_vecstShapeInfo[n].fOffsetX * 0.001);
                fOffsetY.Format(_T("%.3f"), m_vecstShapeInfo[n].fOffsetY * 0.001);
                xml.AddAttrib(_T("offx"), fOffsetX);
                xml.AddAttrib(_T("offy"), fOffsetY);
            }
        }
        xml.AddAttrib(_T("type"), m_vecstShapeInfo[n].nGerberType);
        xml.IntoElem();

        if (m_vecstShapeInfo[n].strType == _T("rc"))
        {
            if (m_vecstShapeInfo[n].nGerberType != 3)
            {
                xml.AddElem(_T("poly"));
                xml.AddAttrib(_T("area"), _T(""));
            }
            else
            {
                xml.AddElem(_T("rc"));
                if (abs(m_vecstObjectInfo[n].fRotation
                        - m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation)
                        == 180
                    || abs(m_vecstObjectInfo[n].fRotation
                           - m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nOrientation)
                        == 0)
                {
                    xml.AddAttrib(_T("w"), strLength);
                    xml.AddAttrib(_T("h"), strWidth);
                }
                else
                {
                    xml.AddAttrib(_T("w"), strWidth);
                    xml.AddAttrib(_T("h"), strLength);
                }
                xml.AddAttrib(_T("rot"), _T("0.0"));
                xml.AddAttrib(_T("area"), _T(""));
            }
        }
        else if (m_vecstShapeInfo[n].strType == _T("cir"))
        {
            xml.AddElem(_T("cir"));
            strTemp.Format(_T("%.3f"), m_vecstShapeInfo[n].fHeigth * 0.001);
            xml.AddAttrib(_T("dia"), strTemp);
            xml.AddAttrib(_T("area"), _T(""));
        }
        xml.OutOfElem();
    }
    xml.OutOfElem();

    xml.AddElem(_T("fps")); //fps ---------------------- FOOT Number 등록(shape - obj 연결)
    xml.IntoElem(); //fps

    CString strName;
    for (int nNum = 0; nNum < m_vecst_XML_FPS.size(); nNum++)
    {
        strName.Format(_T("FOOT%d"), nNum);
        xml.AddElem(_T("fp"));
        xml.AddAttrib(_T("name"), m_vecst_XML_FPS[nNum].strName);
        xml.IntoElem(); //fp
        xml.AddElem(_T("pin"));
        xml.AddAttrib(_T("pd"), m_vecst_XML_FPS[nNum].nShapeNum);
        xml.OutOfElem(); //fp
    }
    xml.OutOfElem(); //fpsa -------------------- FOOT Number 등록(shape - obj 연결)

    xml.AddElem(_T("pas")); //pas ---------------------- PART Number 등록(shape - obj 연결)
    xml.IntoElem(); //pas

    for (int nNum = 0; nNum < m_vecst_XML_PAS.size(); nNum++)
    {
        xml.AddElem(_T("pa"));
        strName.Format(_T("PART%d"), nNum);
        xml.AddAttrib(_T("name"), m_vecst_XML_PAS[nNum].strName);
        strName.Format(_T("FOOT%d"), nNum);
        xml.AddAttrib(_T("foot"), m_vecst_XML_PAS[nNum].strFoot);
    }
    xml.OutOfElem(); //pas -------------------- PART Number 등록(shape - obj 연결)

    xml.AddElem(_T("coms")); //coms ------------------- Object 등록
    xml.IntoElem(); //coms

    CString strOffset_X;
    CString strOffset_Y;
    CString strRotate;
    CString strTickness;
    CString strCompWidth;
    CString strCompLenght;
    CString strObjName;

    xml.AddElem(_T("group"));
    xml.AddAttrib(_T("name"), _T("0"));
    xml.IntoElem(); //group

    for (long nNum = 0; nNum < m_vecstObjectInfo.size(); nNum++)
    {
        xml.AddElem(_T("com"));
        strObjName.Format(_T("T%d"), nNum);
        xml.AddAttrib(_T("name"), strObjName);
        strName.Format(_T("PART%d"), m_vecstObjectInfo[nNum].nShapeNumber - 1);
        xml.AddAttrib(_T("pa"), strName);
        strOffset_X.Format(_T("%.3f"), m_vecstObjectInfo[nNum].fPosX * 0.001);
        strOffset_Y.Format(_T("%.3f"), m_vecstObjectInfo[nNum].fPosY * 0.001);
        xml.AddAttrib(_T("x"), strOffset_X);
        xml.AddAttrib(_T("y"), strOffset_Y);
        strRotate.Format(_T("%.1f"), m_vecstObjectInfo[nNum].fRotation);
        xml.AddAttrib(_T("rot"), strRotate);
        xml.AddAttrib(_T("compGroup"), m_vecstObjectInfo[nNum].nCompGrouping);
        xml.AddAttrib(_T("compChipType"), m_vecstObjectInfo[nNum].nCompChipType);
        xml.AddAttrib(_T("compPassiveType"), m_vecstObjectInfo[nNum].nCompPassiveType);
        strCompWidth.Format(_T("%.2f"), m_vecstObjectInfo[nNum].fCompWidth);
        strCompLenght.Format(_T("%.2f"), m_vecstObjectInfo[nNum].fCompLenght);
        xml.AddAttrib(_T("compWidth"), strCompWidth);
        xml.AddAttrib(_T("compLenght"), strCompLenght);
        strTickness.Format(_T("%.2f"), m_vecstObjectInfo[nNum].fTickness);
        xml.AddAttrib(_T("tickness"), strTickness);
        xml.AddAttrib(_T("landGroupNo"), m_vecstObjectInfo[nNum].nLandGroupNo);
        xml.AddAttrib(_T("padType"), m_vecstObjectInfo[nNum].nPadType);
    }

    xml.OutOfElem(); //group
    xml.OutOfElem(); //coms ---------------------------- Object 등록
    xml.OutOfElem(); //ePM-SPI

    xml.Save(strFilename);

    return;
}

void CMapDataConverterDlg::ReEdit_LoadXMLModifyData()
{
    CString strFilename = DynamicSystemPath::get(DefineFolder::Temp) + _T("TempMapDataConverter.xml");

    CMarkup xml;
    // File open
    xml.Load(strFilename);

    // File Read
    xml.FindElem(_T("ePM-SPI"));
    xml.IntoElem();
    // Coding
    ReEdit_Get_GerberHeaderInfo_XML(xml);
    ReEdit_Get_GerberShapeInfo_XML(xml);
    ReEdit_Get_GerberObjectInfo_XML(xml);
    xml.OutOfElem();

    m_nAutoGroup = m_stHeaderInfo.bAutoGroping;

    Initialize_View();
    UpdatePropertyGridMapData();
    Calc_DrawingPosition_Ball();
    Calc_DrawingPosition_Land();
    Calc_DrawingPosition_Comp();
    Calc_DrawingPosition_Pad();

    if (m_nAutoGroup == 1)
        CreateComponent_Auto();
    else
        CreateComponent_Manual();

    Invalidate(FALSE);
}

//void CMapDataConverterDlg::CompDBVerify(Component selectedComp)
//{
//	std::vector<std::vector<CString>> vec2strCompType;
//	vec2strCompType.resize(_typeComponentEnd);
//
//	LoadCompTypeInCompDB(vec2strCompType);
//
//	std::vector<Component> vecExistCompData;
//	std::vector<CString> existCompType;
//
//	for (auto CompData : m_vecSelectedCompInfo)
//	{
//		switch (CompData.nCompType)
//		{
//		case _typeChip:
//		{
//			BOOL bChipDBCheck(FALSE);
//			if (vec2strCompType[_typeChip].size() > 0)
//			{
//				for (auto &ChipData : vec2strCompType[_typeChip])
//				{
//					if (CompData.strCompType == ChipData)
//					{
//						bChipDBCheck = TRUE;
//						break;
//					}
//				}
//			}
//			if (!bChipDBCheck)
//			{
//				BOOL bChipOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType) != existCompType.end();
//				if (!bChipOverlapValue)
//				{
//					existCompType.push_back(CompData.strCompType);
//					vecExistCompData.push_back(CompData); //없는게 들어가는 정보
//				}
//			}
//			break;
//		}
//		case _typePassive:
//		{
//			BOOL bPassiveDBCheck(FALSE);
//			if (vec2strCompType[_typePassive].size() > 0)
//			{
//				for (auto PassiveData : vec2strCompType[_typePassive])
//				{
//					if (CompData.strCompType == PassiveData)
//					{
//						bPassiveDBCheck = TRUE;
//						break;
//					}
//				}
//			}
//
//			if (!bPassiveDBCheck)
//			{
//				BOOL bPassiveOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType) != existCompType.end();
//				if (!bPassiveOverlapValue)
//				{
//					existCompType.push_back(CompData.strCompType);
//					vecExistCompData.push_back(CompData); //없는게 들어가는 정보
//				}
//			}
//			break;
//		}
//		case _typeArray:
//		{
//			BOOL bArrayDBCheck(FALSE);
//			if (vec2strCompType[_typeArray].size() > 0)
//			{
//				for (auto &ArrayData : vec2strCompType[_typeArray])
//				{
//					if (CompData.strCompType == ArrayData)
//					{
//						bArrayDBCheck = TRUE;
//						break;
//					}
//				}
//			}
//			if (!bArrayDBCheck)
//			{
//				BOOL bArrayOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType) != existCompType.end();
//				if (!bArrayOverlapValue)
//				{
//					existCompType.push_back(CompData.strCompType);
//					vecExistCompData.push_back(CompData); //없는게 들어가는 정보
//				}
//			}
//			break;
//		}
//		case _typeHeatSync:
//		{
//			BOOL bHeatSyncDBCheck(FALSE);
//			if (vec2strCompType[_typeHeatSync].size() > 0)
//			{
//				for (auto &HeatSyncData : vec2strCompType[_typeHeatSync])
//				{
//					if (CompData.strCompType == HeatSyncData)
//					{
//						bHeatSyncDBCheck = TRUE;
//						break;
//					}
//				}
//			}
//			if (!bHeatSyncDBCheck)
//			{
//				BOOL bHeatSyncOverlapValue = std::find(existCompType.begin(), existCompType.end(), CompData.strCompType) != existCompType.end();
//				if (!bHeatSyncOverlapValue)
//				{
//					existCompType.push_back(CompData.strCompType);
//					vecExistCompData.push_back(CompData); //없는게 들어가는 정보
//				}
//			}
//			break;
//		}
//		}
//	}
//
//	//없는걸 다 찾았다면
//	long nExistCompDataNum = (long)vecExistCompData.size();
//	if (nExistCompDataNum > 0)
//	{
//		CString strMsg;
//		strMsg.Format(_T("%dData does Not Exist."), nExistCompDataNum);
//		SimpleMessage((strMsg), MB_OK);
//	}
//
//	DMSChip			DmsChipInfo;
//	DMSPassiveChip	DMSPassiveInfo;
//	DMSArrayChip	DmsArrayInfo;
//	DMSHeatsink		DmsHeatsink;
//	DMSBall			DmsBallInfo;
//	long			DmsType;
//
//	CCompDataChangerDlg CompDataEditor(this, &m_packageSpec, &DmsChipInfo, &DMSPassiveInfo, &DmsArrayInfo, &DmsHeatsink, &DmsBallInfo, &DmsType, vecExistCompData, selectedComp, TRUE, m_bVerifyMode, TRUE);
//
//	if (CompDataEditor.DoModal() == IDOK)
//	{
//		if (!m_bVerifyMode)
//		{
//			for (long n = 0; n < m_vecstObjectInfo.size(); n++)
//			{
//				if (m_vecstObjectInfo[n].nShapeNumber == 0)
//					continue;
//
//				if (m_vecstShapeInfo[m_vecstObjectInfo[n].nShapeNumber - 1].nGerberType == m_nSelectedLayer)
//				{
//					if (m_nSelectedGroup == (m_vecstObjectInfo[n].nShapeNumber - 1))
//					{
//						if (m_vecstObjectInfo[n].bSelected)
//						{
//							if (DmsType == CommonItemID_DMS_TYPE_PASSIVE)
//							{
//								m_vecstObjectInfo[n].fCompWidth = DMSPassiveInfo.fChipWidth;
//								m_vecstObjectInfo[n].fCompLenght = DMSPassiveInfo.fChipLength;
//								m_vecstObjectInfo[n].fTickness = DMSPassiveInfo.fChipThickness;
//
//								long nCompIdx = GetComponentIndexFromMountMap(m_vecstObjectInfo[n].nID);
//								if (nCompIdx != -1)
//								{
//									m_vecCompInfo[nCompIdx].nAngle = DMSPassiveInfo.fChipAngle;
//									m_vecCompInfo[nCompIdx].strCompType = DMSPassiveInfo.strCompType;
//									m_vecCompInfo[nCompIdx].fWidth = DMSPassiveInfo.fChipWidth;
//									m_vecCompInfo[nCompIdx].fLength = DMSPassiveInfo.fChipLength;
//									m_vecCompInfo[nCompIdx].fHeight = DMSPassiveInfo.fChipThickness;
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//		else {
//			for (long n = 0; n < m_vecCompInfo.size(); n++)
//			{
//				if (m_vecCompInfo[n].bIgnore)
//				{
//					if (DmsType == CommonItemID_DMS_TYPE_CHIP)
//					{
//						m_vecCompInfo[n].strCompType = DmsChipInfo.strCompType;
//						m_vecCompInfo[n].fWidth = DmsChipInfo.fChipWidth;
//						m_vecCompInfo[n].fLength = DmsChipInfo.fChipLength;
//						m_vecCompInfo[n].fHeight = DmsChipInfo.fChipThickness;
//						m_vecCompInfo[n].fOffsetX = DmsChipInfo.fChipOffsetX * 1000;
//						m_vecCompInfo[n].fOffsetY = DmsChipInfo.fChipOffsetY * 1000;
//					}
//					else if (DmsType == CommonItemID_DMS_TYPE_PASSIVE)
//					{
//						m_vecCompInfo[n].nAngle = DMSPassiveInfo.fChipAngle;
//						m_vecCompInfo[n].strCompType = DMSPassiveInfo.strCompType;
//						m_vecCompInfo[n].fWidth = DMSPassiveInfo.fChipWidth;
//						m_vecCompInfo[n].fLength = DMSPassiveInfo.fChipLength;
//						m_vecCompInfo[n].fHeight = DMSPassiveInfo.fChipThickness;
//						m_vecCompInfo[n].fOffsetX = DMSPassiveInfo.fChipOffsetX * 1000;
//						m_vecCompInfo[n].fOffsetY = DMSPassiveInfo.fChipOffsetY * 1000;
//					}
//					else if (DmsType == CommonItemID_DMS_TYPE_ARRAY)
//					{
//						m_vecCompInfo[n].strCompType = DmsArrayInfo.strCompType;
//						m_vecCompInfo[n].fWidth = DmsArrayInfo.fChipWidth;
//						m_vecCompInfo[n].fLength = DmsArrayInfo.fChipLength;
//						m_vecCompInfo[n].fHeight = DmsArrayInfo.fChipThickness;
//						m_vecCompInfo[n].fOffsetX = DmsArrayInfo.fChipOffsetX * 1000;
//						m_vecCompInfo[n].fOffsetY = DmsArrayInfo.fChipOffsetY * 1000;
//					}
//					else if (DmsType == CommonItemID_DMS_TYPE_HEATSYNC)
//					{
//						m_vecCompInfo[n].strCompType = DmsHeatsink.strCompType;
//						m_vecCompInfo[n].fWidth = DmsHeatsink.fChipWidth;
//						m_vecCompInfo[n].fLength = DmsHeatsink.fChipLength;
//						m_vecCompInfo[n].fHeight = DmsHeatsink.fChipThickness;
//						m_vecCompInfo[n].fOffsetX = DmsHeatsink.fChipOffsetX * 1000;
//						m_vecCompInfo[n].fOffsetY = DmsHeatsink.fChipOffsetY * 1000;
//					}
//				}
//			}
//		}
//
//		Unselect_AllData();
//	}
//	else
//		return;
//}
//
//BOOL CMapDataConverterDlg::LoadCompTypeInCompDB(std::vector<std::vector<CString>> &o_vec2strCompType)
//{
//	if (o_vec2strCompType.size() <= 0)
//		return FALSE;
//
//	//Chip
//	CString strChipDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Chip.ini"));
//	int nChipTypeNum = Ipvm::IniHelper::LoadINT(strChipDB, _T("Chip"), _T("Chip Type Num"), 0);
//	o_vec2strCompType[_typeChip].resize(nChipTypeNum);
//	for (long nType = 0; nType < nChipTypeNum; nType++)
//	{
//		CString strSection;
//		strSection.Format(_T("Chip_%d"), nType);
//		o_vec2strCompType[_typeChip][nType] = Ipvm::IniHelper::LoadSTRING(strChipDB, strSection, _T("Type Name"), _T(""));
//	}
//
//	//Passive
//	CString strPassiveDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Passive.ini"));
//	int nPaasiveTypeNum = Ipvm::IniHelper::LoadINT(strPassiveDB, _T("PassiveChip"), _T("Chip Type Num"), 0);
//	o_vec2strCompType[_typePassive].resize(nPaasiveTypeNum);
//	for (long nType = 0; nType < nPaasiveTypeNum; nType++)
//	{
//		CString strSection;
//		strSection.Format(_T("PassiveChip_%d"), nType);
//		o_vec2strCompType[_typePassive][nType] = Ipvm::IniHelper::LoadSTRING(strPassiveDB, strSection, _T("Type Name"), _T(""));
//	}
//
//	//Array
//	CString strArrayDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Array.ini"));
//	int nArrayTypeNum = Ipvm::IniHelper::LoadINT(strArrayDB, _T("ArrayChip"), _T("Chip Type Num"), 0);
//	o_vec2strCompType[_typeArray].resize(nArrayTypeNum);
//	for (long nType = 0; nType < nArrayTypeNum; nType++)
//	{
//		CString strSection;
//		strSection.Format(_T("ArrayChip_%d"), nType);
//		o_vec2strCompType[_typeArray][nType] = Ipvm::IniHelper::LoadSTRING(strArrayDB, strSection, _T("Type Name"), _T(""));
//	}
//
//	//HeatSync
//	CString strHeatSyncDB(DynamicSystemPath::get(DefineFolder::Config) + _T("HeatSync.ini"));
//	int nHeatSyncTypeNum = Ipvm::IniHelper::LoadINT(strHeatSyncDB, _T("HeatSync"), _T("Chip Type Num"), 0);
//	o_vec2strCompType[_typeHeatSync].resize(nHeatSyncTypeNum);
//	for (long nType = 0; nType < nHeatSyncTypeNum; nType++)
//	{
//		CString strSection;
//		strSection.Format(_T("HeatSync_%d"), nType);
//		o_vec2strCompType[_typeHeatSync][nType] = Ipvm::IniHelper::LoadSTRING(strHeatSyncDB, strSection, _T("Type Name"), _T(""));
//	}
//
//	return TRUE;
//}

void CMapDataConverterDlg::OnPrevmenuAdd()
{
    Package::Component AddCompData;
    Gerber_Object AddGerber;
    std::vector<Package::Component> vecExistCompData;

    Chip::DMSChip DmsChipInfo;
    Chip::DMSPassiveChip DMSPassiveInfo;
    Chip::DMSArrayChip DmsArrayInfo;
    Chip::DMSHeatsink DmsHeatsink;
    Chip::DMSPatch DmsPatch;
    Chip::DMSBall DmsBallInfo;
    long DmsType;

    CCompDataChangerDlg CompDataEditor(this, &m_packageSpec, &DmsChipInfo, &DMSPassiveInfo, &DmsArrayInfo, &DmsHeatsink,
        &DmsPatch, &DmsBallInfo, &DmsType, vecExistCompData, AddCompData, TRUE, m_bVerifyMode);

    if (CompDataEditor.DoModal() == IDOK)
    {
        if (DmsType == CommonItemID_DMS_TYPE_CHIP)
        {
            AddCompData.fHeight = DmsChipInfo.fChipThickness;
            AddCompData.fWidth = DmsChipInfo.fChipWidth;
            AddCompData.fLength = DmsChipInfo.fChipLength;
            AddCompData.fOffsetX = DmsChipInfo.fChipOffsetX * 1000.f;
            AddCompData.fOffsetY = DmsChipInfo.fChipOffsetY * 1000.f;
            AddCompData.nAngle = CAST_LONG(DmsChipInfo.fChipAngle);
            AddCompData.strCompType = DmsChipInfo.strCompType;
            AddCompData.strCompName = DmsChipInfo.strCompID;
            AddCompData.nCompType = _typeChip;
            m_vecCompInfo.push_back(AddCompData);

            AddGerber.fTickness = DmsChipInfo.fChipThickness;
            AddGerber.fCompWidth = DmsChipInfo.fChipWidth;
            AddGerber.fCompLenght = DmsChipInfo.fChipLength;
            AddGerber.fPosX = DmsChipInfo.fChipOffsetX * 1000.f;
            AddGerber.fPosY = DmsChipInfo.fChipOffsetY * 1000.f;
            AddGerber.fRotation = DmsChipInfo.fChipAngle;
            AddGerber.nID = -1;
            AddGerber.nCompChipType = _typeChip;
            m_vecstObjectInfo.push_back(AddGerber);
        }
        else if (DmsType == CommonItemID_DMS_TYPE_HEATSYNC)
        {
            AddCompData.fHeight = DmsHeatsink.fChipThickness;
            AddCompData.fWidth = DmsHeatsink.fChipWidth;
            AddCompData.fLength = DmsHeatsink.fChipLength;
            AddCompData.fOffsetX = DmsHeatsink.fChipOffsetX * 1000.f;
            AddCompData.fOffsetY = DmsHeatsink.fChipOffsetY * 1000.f;
            AddCompData.nAngle = CAST_LONG(DmsHeatsink.fChipAngle);
            AddCompData.strCompType = DmsHeatsink.strCompType;
            AddCompData.strCompName = DmsHeatsink.strCompID;
            AddCompData.nCompType = _typeHeatSync;
            m_vecCompInfo.push_back(AddCompData);

            AddGerber.fTickness = DmsHeatsink.fChipThickness;
            AddGerber.fCompWidth = DmsHeatsink.fChipWidth;
            AddGerber.fCompLenght = DmsHeatsink.fChipLength;
            AddGerber.fPosX = DmsHeatsink.fChipOffsetX * 1000.f;
            AddGerber.fPosY = DmsHeatsink.fChipOffsetY * 1000.f;
            AddGerber.fRotation = DmsHeatsink.fChipAngle;
            AddGerber.nID = -1;
            AddGerber.nCompChipType = _typeChip;
            m_vecstObjectInfo.push_back(AddGerber);
        }
    }
    CompTypeConvert();

    Update_Background(m_nSelectedLayer);
    Update_Foreground(m_nSelectedLayer, m_nSelectedGroup);
    Invalidate(FALSE);
}

void CMapDataConverterDlg::OnPrevmenuDelete()
{
    DeleteSelectedMapData();
}

void CMapDataConverterDlg::OnPadmenuDelete()
{
    DeleteSelectedMapData();
}

void CMapDataConverterDlg::OnComponentmenuDelete()
{
    DeleteSelectedMapData();
}

void CMapDataConverterDlg::OnLandmenuDelete()
{
    DeleteSelectedMapData();
}

void CMapDataConverterDlg::OnBallmenuDelte()
{
    DeleteSelectedMapData();
}

void CMapDataConverterDlg::OnOtherDelete()
{
    DeleteSelectedMapData();
}

void CMapDataConverterDlg::OnBallmenuSelectall()
{
    Select_AllData();
}

void CMapDataConverterDlg::OnBallmenuUnselectall()
{
    Unselect_AllData();
}

void CMapDataConverterDlg::OnComponentmenuSelectall()
{
    Select_AllData();
}

void CMapDataConverterDlg::OnComponentmenuUnselectall()
{
    Unselect_AllData();
}

void CMapDataConverterDlg::OnPadmenuSelectall()
{
    Select_AllData();
}

void CMapDataConverterDlg::OnPadmenuUnselectall()
{
    Unselect_AllData();
}

void CMapDataConverterDlg::OnLandmenuSelectall()
{
    Select_AllData();
}

void CMapDataConverterDlg::OnLandmenuUnselectall()
{
    Unselect_AllData();
}

void CMapDataConverterDlg::OnPrevmenuEditdimensions()
{
    OnComponentmenuEditdimensions();
}

void CMapDataConverterDlg::OnPrevmenuSelectall()
{
    Select_AllData();
}

void CMapDataConverterDlg::OnPrevmenuUnselectall()
{
    Unselect_AllData();
}