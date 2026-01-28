//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DMSPassiveChip.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Chip
{
BOOL DMSPassiveChip::LinkDataBase(BOOL bSave, CiDataBase& db, long nCount)
{
    long nSize(0);
    CString strTemp;
    //strTemp.Format(_T("DMSPassiveChip_nType_%d"), nCount);
    //if(!db[strTemp].Link(bSave, nType))
    //	nType = 0;

    strTemp.Format(_T("DMSPassiveChip_%d_strName"), nCount);
    if (!db[strTemp].Link(bSave, strCompType))
        strCompType = _T("Default");

    if (!bSave)
    {
        strTemp.Format(_T("DMSPassiveChip_frtPad_%d_Num"), nCount);
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;
        vecfrtPad.resize(nSize);
    }
    else
    {
        nSize = (long)vecfrtPad.size();
        strTemp.Format(_T("DMSPassiveChip_frtPad_%d_Num"), nCount);
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;
    }

    for (long i = 0; i < nSize; i++)
    {
        strTemp.Format(_T("DMSPassiveChip_frtPad_%d_%d_left"), nCount, i);
        if (!db[strTemp].Link(bSave, vecfrtPad[i].m_left))
            vecfrtPad[i].m_left = 0.f;
        strTemp.Format(_T("DMSPassiveChip_frtPad_%d_%d_top"), nCount, i);
        if (!db[strTemp].Link(bSave, vecfrtPad[i].m_top))
            vecfrtPad[i].m_top = 0.f;
        strTemp.Format(_T("DMSPassiveChip_frtPad_%d_%d_right"), nCount, i);
        if (!db[strTemp].Link(bSave, vecfrtPad[i].m_right))
            vecfrtPad[i].m_right = 0.f;
        strTemp.Format(_T("DMSPassiveChip_frtPad_%d_%d_bottom"), nCount, i);
        if (!db[strTemp].Link(bSave, vecfrtPad[i].m_bottom))
            vecfrtPad[i].m_bottom = 0.f;
    }

    strTemp.Format(_T("DMSPassiveChip_frtChip_%d_left"), nCount);
    if (!db[strTemp].Link(bSave, frtChip.m_left))
        frtChip.m_left = 0.f;
    strTemp.Format(_T("DMSPassiveChip_frtChip_%d_top"), nCount);
    if (!db[strTemp].Link(bSave, frtChip.m_top))
        frtChip.m_top = 0.f;
    strTemp.Format(_T("DMSPassiveChip_frtChip_%d_right"), nCount);
    if (!db[strTemp].Link(bSave, frtChip.m_right))
        frtChip.m_right = 0.f;
    strTemp.Format(_T("DMSPassiveChip_frtChip_%d_bottom"), nCount);
    if (!db[strTemp].Link(bSave, frtChip.m_bottom))
        frtChip.m_bottom = 0.f;

    if (!bSave)
    {
        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_Num"), nCount);
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;
        vecsfrtPad.resize(nSize);
    }
    else
    {
        nSize = (long)vecsfrtPad.size();
        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_Num"), nCount);
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;
    }

    for (long i = 0; i < nSize; i++)
    {
        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_%d_ltX"), nCount, i);
        if (!db[strTemp].Link(bSave, vecsfrtPad[i].fptLT.m_x))
            vecsfrtPad[i].fptLT.m_x = 0.f;
        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_%d_ltY"), nCount, i);
        if (!db[strTemp].Link(bSave, vecsfrtPad[i].fptLT.m_y))
            vecsfrtPad[i].fptLT.m_y = 0.f;

        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_%d_rtX"), nCount, i);
        if (!db[strTemp].Link(bSave, vecsfrtPad[i].fptRT.m_x))
            vecsfrtPad[i].fptRT.m_x = 0.f;
        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_%d_rtY"), nCount, i);
        if (!db[strTemp].Link(bSave, vecsfrtPad[i].fptRT.m_y))
            vecsfrtPad[i].fptRT.m_y = 0.f;

        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_%d_lbX"), nCount, i);
        if (!db[strTemp].Link(bSave, vecsfrtPad[i].fptLB.m_x))
            vecsfrtPad[i].fptLB.m_x = 0.f;
        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_%d_lbY"), nCount, i);
        if (!db[strTemp].Link(bSave, vecsfrtPad[i].fptLB.m_y))
            vecsfrtPad[i].fptLB.m_y = 0.f;

        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_%d_rbX"), nCount, i);
        if (!db[strTemp].Link(bSave, vecsfrtPad[i].fptRB.m_x))
            vecsfrtPad[i].fptRB.m_x = 0.f;
        strTemp.Format(_T("DMSPassiveChip_sfrtPad_%d_%d_rbY"), nCount, i);
        if (!db[strTemp].Link(bSave, vecsfrtPad[i].fptRB.m_y))
            vecsfrtPad[i].fptRB.m_y = 0.f;
    }

    strTemp.Format(_T("DMSPassiveChip_sfrtChip_%d_ltX"), nCount);
    if (!db[strTemp].Link(bSave, sfrtChip.fptLT.m_x))
        sfrtChip.fptLT.m_x = 0.f;
    strTemp.Format(_T("DMSPassiveChip_sfrtChip_%d_ltY"), nCount);
    if (!db[strTemp].Link(bSave, sfrtChip.fptLT.m_y))
        sfrtChip.fptLT.m_y = 0.f;

    strTemp.Format(_T("DMSPassiveChip_sfrtChip_%d_rtX"), nCount);
    if (!db[strTemp].Link(bSave, sfrtChip.fptRT.m_x))
        sfrtChip.fptRT.m_x = 0.f;
    strTemp.Format(_T("DMSPassiveChip_sfrtChip_%d_rtY"), nCount);
    if (!db[strTemp].Link(bSave, sfrtChip.fptRT.m_y))
        sfrtChip.fptRT.m_y = 0.f;

    strTemp.Format(_T("DMSPassiveChip_sfrtChip_%d_lbX"), nCount);
    if (!db[strTemp].Link(bSave, sfrtChip.fptLB.m_x))
        sfrtChip.fptLB.m_x = 0.f;
    strTemp.Format(_T("DMSPassiveChip_sfrtChip_%d_lbY"), nCount);
    if (!db[strTemp].Link(bSave, sfrtChip.fptLB.m_y))
        sfrtChip.fptLB.m_y = 0.f;

    strTemp.Format(_T("DMSPassiveChip_sfrtChip_%d_rbX"), nCount);
    if (!db[strTemp].Link(bSave, sfrtChip.fptRB.m_x))
        sfrtChip.fptRB.m_x = 0.f;
    strTemp.Format(_T("DMSPassiveChip_sfrtChip_%d_rbY"), nCount);
    if (!db[strTemp].Link(bSave, sfrtChip.fptRB.m_y))
        sfrtChip.fptRB.m_y = 0.f;

    strTemp.Format(_T("DMSPassiveChip_nPassiveType_%d"), nCount);
    if (!db[strTemp].Link(bSave, nPassiveType))
        nPassiveType = 0;

    strTemp.Format(_T("DMSPassiveChip_fChipWidth_%d"), nCount);
    if (!db[strTemp].Link(bSave, fChipWidth))
        fChipWidth = 0.45f;

    strTemp.Format(_T("DMSPassiveChip_fChipLength_%d"), nCount);
    if (!db[strTemp].Link(bSave, fChipLength))
        fChipLength = 0.95f;

    strTemp.Format(_T("DMSPassiveChip_fChipAngle_%d"), nCount);
    if (!db[strTemp].Link(bSave, fChipAngle))
        fChipAngle = 90.f;

    strTemp.Format(_T("DMSPassiveChip_fChipThickness_%d"), nCount);
    if (!db[strTemp].Link(bSave, fChipThickness))
        fChipThickness = 0.5f;

    strTemp.Format(_T("DMSPassiveChip_fChipPADGap_%d"), nCount);
    if (!db[strTemp].Link(bSave, fChipPAD_Gap))
        fChipPAD_Gap = 0.15f;

    strTemp.Format(_T("DMSPassiveChip_fChipPADWith_%d"), nCount);
    if (!db[strTemp].Link(bSave, fChipPAD_Width))
        fChipPAD_Width = 0.5f;

    strTemp.Format(_T("DMSPassiveChip_fChipPADLength_%d"), nCount);
    if (!db[strTemp].Link(bSave, fChipPAD_Length))
        fChipPAD_Length = 1.2f;

    strTemp.Format(_T("DMSPassiveChip_fElectWidth_%d"), nCount);
    if (!db[strTemp].Link(bSave, fElectWidth))
        fElectWidth = 0.22f;

    strTemp.Format(_T("DMSPassiveChip_fElectThickness_%d"), nCount);
    if (!db[strTemp].Link(bSave, fElectThickness))
        fElectThickness = 0.5f;

    return TRUE;
}

} // namespace Chip