//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SlitBeam3DParameters.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SlitBeam3DParameters::SlitBeam3DParameters(void)
    : m_noiseThreshold(2.)
    , m_noiseThreshold_FinalFiltering(1.) //kircheis_SDK127
    , m_beamThickness_um(70)
    , m_cameraVerticalBinning(FALSE)
    , m_scanStep_um(10)
    , m_scanDepth_um(2000.)
    , m_scanLength_mm(130.)
    , m_illuminationTime_ms(0.2)
    , m_illuminationValue(255)
    , m_cameraGrabMode(0)
    , m_illuminationTimeDefused_ms(0.2)
    , m_noiseThreshold_Defused1(2.)
    , m_noiseThreshold_Defused2(2.)
    , m_noiseThreshold_FinalFiltering_Defused1(1.)
    , m_noiseThreshold_FinalFiltering_Defused2(1.)
    , m_beamThickness_Defused1_um(70)
    , m_beamThickness_Defused2_um(70)
{
}

SlitBeam3DParameters::~SlitBeam3DParameters(void)
{
}

BOOL SlitBeam3DParameters::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{84E06101-4FC3-4223-BFEA-5AF4F6F1CD39}")].Link(bSave, m_illuminationTime_ms))
        m_illuminationTime_ms = 0.2;
    if (!db[_T("{E2FDC20B-2E34-4BA6-8345-75F9433D669E}")].Link(bSave, m_illuminationValue))
        m_illuminationValue = 255;
    if (!db[_T("{EF6AAC1D-6975-4E4E-9DEA-D0A0808F13D3}")].Link(bSave, m_cameraVerticalBinning))
        m_cameraVerticalBinning = FALSE;
    if (!db[_T("{EF0C5937-5848-4152-B73E-51CBD9FB324E}")].Link(bSave, m_scanLength_mm))
        m_scanLength_mm = 130.;
    if (!db[_T("{57F0CE0A-98E4-40C3-AA59-15AAF0824126}")].Link(bSave, m_scanStep_um))
        m_scanStep_um = 10;
    if (!db[_T("{BFCEC65B-2417-494A-B426-46EBA0540816}")].Link(bSave, m_scanDepth_um))
        m_scanDepth_um = 2000.;
    if (!db[_T("{76357797-40B8-4D29-9584-062432A32D11}")].Link(bSave, m_noiseThreshold))
        m_noiseThreshold = 1;
    if (!db[_T("{55FE0A1D-CC7E-4926-B1E7-92142A0CEC92}")].Link(bSave, m_noiseThreshold_FinalFiltering))
        m_noiseThreshold_FinalFiltering = -1; //kircheis_SDK127
    if (!db[_T("{EBC12A1E-BAAA-4E9E-9CB6-E30097417FC4}")].Link(bSave, m_beamThickness_um))
        m_beamThickness_um = 50;

    if (!bSave && m_noiseThreshold_FinalFiltering == -1) //kircheis_SDK127
        m_noiseThreshold_FinalFiltering = m_noiseThreshold * .5;

    if (!db[_T("{05B5A072-2C13-464F-9251-FD9DDF46AE03}")].Link(bSave, m_cameraGrabMode))
        m_cameraGrabMode = 0;
    if (!db[_T("{7446B23B-93F9-4257-8C86-9A7E9523BB3F}")].Link(bSave, m_illuminationTimeDefused_ms))
        m_illuminationTimeDefused_ms = 0.2;
    if (!db[_T("{DCEF943A-4BE6-4676-B957-60337E4760BB}")].Link(bSave, m_noiseThreshold_Defused1))
        m_noiseThreshold_Defused1 = 1;
    if (!db[_T("{D0AB6824-4336-49D0-A1B4-CCB4EE58A227}")].Link(bSave, m_noiseThreshold_Defused2))
        m_noiseThreshold_Defused2 = 1;
    if (!db[_T("{F75A8592-8197-4C80-A7DB-33A8425FECB7}")].Link(bSave, m_noiseThreshold_FinalFiltering_Defused1))
        m_noiseThreshold_FinalFiltering_Defused1 = -1;
    if (!db[_T("{B3330104-E248-45A9-8AAB-C1EB5A03EA2F}")].Link(bSave, m_noiseThreshold_FinalFiltering_Defused2))
        m_noiseThreshold_FinalFiltering_Defused2 = -1;
    if (!db[_T("{8AAF5CCB-AB97-4874-93DE-C79E42CB060C}")].Link(bSave, m_beamThickness_Defused1_um))
        m_beamThickness_Defused1_um = 50;
    if (!db[_T("{47470AE3-8448-4721-8A9E-2C104A8A2F8A}")].Link(bSave, m_beamThickness_Defused2_um))
        m_beamThickness_Defused2_um = 50;

    if (!bSave && m_noiseThreshold_FinalFiltering_Defused1 == -1) //kircheis_SDK127
        m_noiseThreshold_FinalFiltering_Defused1 = m_noiseThreshold_Defused1 * .5;

    if (!bSave && m_noiseThreshold_FinalFiltering_Defused2 == -1) //kircheis_SDK127
        m_noiseThreshold_FinalFiltering_Defused2 = m_noiseThreshold_Defused2 * .5;

    return TRUE;
}

long SlitBeam3DParameters::GetPhysicalScanLength_px() const
{
    // 4 의 배수로 맞추어야 한다
    long physicalScanLength_pxTmp = long(m_scanLength_mm * 1000. / m_scanStep_um + 0.5);
    long physicalScanLength_px = physicalScanLength_pxTmp - (physicalScanLength_pxTmp % 4);
    return physicalScanLength_px;
}

long SlitBeam3DParameters::GetLogicalScanLength_px() const
{
    if (this == nullptr)
        return 0;

    // 4 의 배수로 맞추어야 한다
    long physicalScanLength_pxTmp = long(
        m_scanLength_mm * 1000. / SystemConfig::GetInstance().Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
        + 0.5);
    long physicalScanLength_px = physicalScanLength_pxTmp - (physicalScanLength_pxTmp % 4);
    return physicalScanLength_px;
}
