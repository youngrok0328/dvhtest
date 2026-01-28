//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgChangeDimension.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/ChipInfo.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../ProcessingItems/VisionProcessingMapDataEditor/VisionMapDataEditorUI.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgChangeDimension, CDialog)

CDlgChangeDimension::CDlgChangeDimension(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgChangeDimension::IDD, pParent)
{
    m_vecSelChipInfo.clear();
    m_nDMSType = -1;

    m_pVisionMapDataEditorUI = NULL;

    m_strCompType = "";
    m_fCompWidth = 0.f;
    m_fCompLength = 0.f;
    m_fCompHeight = 0.f;
    m_nCompAngle = 0;
}

CDlgChangeDimension::~CDlgChangeDimension()
{
}

void CDlgChangeDimension::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_DIMENSION, m_cmbDimension);
}

BEGIN_MESSAGE_MAP(CDlgChangeDimension, CDialog)
ON_BN_CLICKED(IDOK, &CDlgChangeDimension::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CDlgChangeDimension::OnBnClickedCancel)
END_MESSAGE_MAP()

// CDlgChangeDimension 메시지 처리기입니다.

void CDlgChangeDimension::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    long nSel = m_cmbDimension.GetCurSel();
    if (nSel >= 0)
    {
        CString strDmsName;
        m_cmbDimension.GetLBText(nSel, strDmsName);

        for (long i = 0; i < (long)m_vecSelChipInfo.size(); i++)
        {
            Chip::ChipInfo* pChipInfo = m_vecSelChipInfo[i];
            pChipInfo->strDmsName = strDmsName;

            if (m_nDMSType == Chip::ChipInfo::tyCHIP)
            {
                Chip::DMSChip* pDmsChip = (Chip::DMSChip*)pChipInfo->pDmsInfo;
                Chip::DMSChip NewDmsInfo = m_pVisionMapDataEditorUI->GetDMSChipData()[nSel];

                pDmsChip->strCompType = NewDmsInfo.strCompType;
                pDmsChip->fChipWidth = NewDmsInfo.fChipWidth;
                pDmsChip->fChipLength = NewDmsInfo.fChipLength;
                pDmsChip->fChipAngle = NewDmsInfo.fChipAngle;
                pDmsChip->fChipThickness = NewDmsInfo.fChipThickness;

                pDmsChip->fUnderfillWidth = NewDmsInfo.fUnderfillWidth;
                pDmsChip->fUnderfillLength = NewDmsInfo.fUnderfillLength;

                m_strCompType = NewDmsInfo.strCompType;
                m_fCompWidth = NewDmsInfo.fChipWidth;
                m_fCompLength = NewDmsInfo.fChipLength;
                m_fCompHeight = NewDmsInfo.fChipThickness;
                m_nCompAngle = (long)NewDmsInfo.fChipAngle;
            }
            else if (m_nDMSType == Chip::ChipInfo::tyPASSIVE)
            {
                Chip::DMSPassiveChip* pDmsPassive = (Chip::DMSPassiveChip*)pChipInfo->pDmsInfo;
                Chip::DMSPassiveChip NewDmsInfo = m_pVisionMapDataEditorUI->GetDMSPassiveData()[nSel];

                pDmsPassive->strCompType = NewDmsInfo.strCompType;
                pDmsPassive->fChipWidth = NewDmsInfo.fChipWidth;
                pDmsPassive->fChipLength = NewDmsInfo.fChipLength;
                pDmsPassive->fChipAngle = NewDmsInfo.fChipAngle;
                pDmsPassive->fChipThickness = NewDmsInfo.fChipThickness;

                pDmsPassive->fElectWidth = NewDmsInfo.fElectWidth;
                pDmsPassive->fChipPAD_Width = NewDmsInfo.fChipPAD_Width;
                pDmsPassive->fChipPAD_Length = NewDmsInfo.fChipPAD_Length;
                pDmsPassive->fElectThickness = NewDmsInfo.fElectThickness;
                pDmsPassive->nPassiveType = NewDmsInfo.nPassiveType;

                m_strCompType = NewDmsInfo.strCompType;
                m_fCompWidth = NewDmsInfo.fChipWidth;
                m_fCompLength = NewDmsInfo.fChipLength;
                m_fCompHeight = NewDmsInfo.fChipThickness;
                m_nCompAngle = (long)NewDmsInfo.fChipAngle;
            }
            else if (m_nDMSType == Chip::ChipInfo::tyARRAY)
            {
                Chip::DMSArrayChip* pDmsArray = (Chip::DMSArrayChip*)pChipInfo->pDmsInfo;
                Chip::DMSArrayChip NewDmsInfo = m_pVisionMapDataEditorUI->GetDMSArrayData()[nSel];

                pDmsArray->strCompType = NewDmsInfo.strCompType;
                pDmsArray->fChipWidth = NewDmsInfo.fChipWidth;
                pDmsArray->fChipLength = NewDmsInfo.fChipLength;
                pDmsArray->fChipAngle = NewDmsInfo.fChipAngle;
                pDmsArray->fChipThickness = NewDmsInfo.fChipThickness;

                pDmsArray->fElectWidth = NewDmsInfo.fElectWidth;
                pDmsArray->fElectLength = NewDmsInfo.fElectLength;
                pDmsArray->fElectPitch = NewDmsInfo.fElectPitch;
                pDmsArray->fElectTip = NewDmsInfo.fElectTip;
                pDmsArray->fPadSizeWidth = NewDmsInfo.fPadSizeWidth;
                pDmsArray->fPadSizeLength = NewDmsInfo.fPadSizeLength;
                pDmsArray->fElectThickness = NewDmsInfo.fElectThickness;
                pDmsArray->nArrayType = NewDmsInfo.nArrayType;

                m_strCompType = NewDmsInfo.strCompType;
                m_fCompWidth = NewDmsInfo.fChipWidth;
                m_fCompLength = NewDmsInfo.fChipLength;
                m_fCompHeight = NewDmsInfo.fChipThickness;
                m_nCompAngle = (long)NewDmsInfo.fChipAngle;
            }
            else if (m_nDMSType == Chip::ChipInfo::tyHEATSINK)
            {
                Chip::DMSHeatsink* pDmsHeatsink = (Chip::DMSHeatsink*)pChipInfo->pDmsInfo;
                Chip::DMSHeatsink NewDmsInfo = m_pVisionMapDataEditorUI->GetDMSHeatSyncData()[nSel];

                pDmsHeatsink->strCompType = NewDmsInfo.strCompType;
                pDmsHeatsink->fChipWidth = NewDmsInfo.fChipWidth;
                pDmsHeatsink->fChipLength = NewDmsInfo.fChipLength;
                pDmsHeatsink->fChipAngle = NewDmsInfo.fChipAngle;
                pDmsHeatsink->fChipThickness = NewDmsInfo.fChipThickness;

                m_strCompType = NewDmsInfo.strCompType;
                m_fCompWidth = NewDmsInfo.fChipWidth;
                m_fCompLength = NewDmsInfo.fChipLength;
                m_fCompHeight = NewDmsInfo.fChipThickness;
                m_nCompAngle = (long)NewDmsInfo.fChipAngle;
            }
            else if (m_nDMSType == Chip::ChipInfo::tyBALL)
            {
                //추후 구현 예정
                //DMSBall* pDmsBall = (DMSBall*)pChipInfo->pDmsInfo;
                //DMSBall NewDmsInfo = m_pDMSInfoManager->GetDMSBallDB()[nSel];

                //pDmsBall->strCompType = NewDmsInfo.strCompType;
                //pDmsBall->fChipWidth = NewDmsInfo.fChipWidth;
                //pDmsBall->fChipLength = NewDmsInfo.fChipLength;
                //pDmsBall->fChipAngle = NewDmsInfo.fChipAngle;
                //pDmsBall->fChipThickness = NewDmsInfo.fChipThickness;
            }
        }
        strDmsName.Empty();
    }

    CDialog::OnOK();
}

void CDlgChangeDimension::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    CDialog::OnCancel();
}

BOOL CDlgChangeDimension::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_cmbDimension.ResetContent();
    if (m_nDMSType == Chip::ChipInfo::tyCHIP)
    {
        std::vector<Chip::DMSChip> vecChipDB = m_pVisionMapDataEditorUI->GetDMSChipData();
        for (long i = 0; i < (long)vecChipDB.size(); i++)
            m_cmbDimension.AddString(vecChipDB[i].strCompType);
    }
    else if (m_nDMSType == Chip::ChipInfo::tyPASSIVE)
    {
        std::vector<Chip::DMSPassiveChip> vecPassiveDB = m_pVisionMapDataEditorUI->GetDMSPassiveData();
        for (long i = 0; i < (long)vecPassiveDB.size(); i++)
            m_cmbDimension.AddString(vecPassiveDB[i].strCompType);
    }
    else if (m_nDMSType == Chip::ChipInfo::tyARRAY)
    {
        std::vector<Chip::DMSArrayChip> vecArrayDB = m_pVisionMapDataEditorUI->GetDMSArrayData();
        for (long i = 0; i < (long)vecArrayDB.size(); i++)
            m_cmbDimension.AddString(vecArrayDB[i].strCompType);
    }
    else if (m_nDMSType == Chip::ChipInfo::tyHEATSINK)
    {
        //std::vector<DMSHeatsink> vecHeatsinkDB = m_pDMSInfoManager->GetDMSHeatsinkDB();
        std::vector<Chip::DMSHeatsink> vecHeatSync = m_pVisionMapDataEditorUI->GetDMSHeatSyncData();
        for (long i = 0; i < (long)vecHeatSync.size(); i++)
            m_cmbDimension.AddString(vecHeatSync[i].strCompType);
    }
    else if (m_nDMSType == Chip::ChipInfo::tyBALL)
    {
        //std::vector<DMSBall> vecBallDB = m_pDMSInfoManager->GetDMSBallDB();
        //for(long i=0; i<(long)vecBallDB.size(); i++)
        //	m_cmbDimension.AddString(vecBallDB[i].strCompType);
    }

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

INT_PTR CDlgChangeDimension::DoModal(
    std::vector<Chip::ChipInfo*> i_vecSelChipInfo, long i_nDMSType, VisionMapDataEditorUI* i_pVisionMapDataEditorUI)
{
    // TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
    m_vecSelChipInfo = i_vecSelChipInfo;
    m_nDMSType = i_nDMSType;
    m_pVisionMapDataEditorUI = i_pVisionMapDataEditorUI;

    return CDialog::DoModal();
}
