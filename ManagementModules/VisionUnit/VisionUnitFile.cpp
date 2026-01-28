//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionUnit.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../UtilityMacroFunction.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionUnit::OpenImageFile()
{
    // BMP File 을 선택할 File Dialog 팝업
    CFileDialog Dlg(TRUE, _T("bmp"), NULL, /*OFN_NOCHANGEDIR|OFN_FILEMUSTEXIST|*/ OFN_HIDEREADONLY,
        _T("Bmp File Format (*.bmp)|*.bmp||"));

    CString initDir = PersonalConfig::getInstance().getLastAccessDirectory_Image();
    Dlg.GetOFN().lpstrInitialDir = initDir;

    if (Dlg.DoModal() != IDOK)
        return FALSE;

    if (OpenImageFile(Dlg.GetPathName()))
    {
        PersonalConfig::getInstance().setLastAccessDirectory_Image(GET_PATHNAME(Dlg.GetPathName()));
        return TRUE;
    }

    return FALSE;
}

BOOL VisionUnit::SaveImageFile()
{
    if (GetAsyncKeyState(VK_CONTROL) && m_systemConfig.GetVisionType() == VISIONTYPE_3D_INSP)
    {
        CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
            _T("Comma separated values file format (*.csv)|*.csv||"));

        if (dlg.DoModal() != IDOK)
            return FALSE;

        Ipvm::AsyncProgress progress(_T("Saving..."));
        const auto& zmap = m_imageLot->GetZmapImage(0);
        CFile file;
        if (!file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
        {
            AfxMessageBox(_T("CSV File write fail"));
            return FALSE;
        }
        CStringA writeText;
        writeText = "Y\\X,";
        for (long x = 0; x < zmap.GetSizeX(); x++)
        {
            writeText.AppendFormat("%d,", x + 1);
        }

        writeText += "\n";
        file.Write((const char*)writeText, writeText.GetLength());

        for (long y = 0; y < zmap.GetSizeY(); y++)
        {
            progress.PostPrintf(_T("Saving ... (%d/%d)"), y + 1, zmap.GetSizeY());
            auto* zmap_y = zmap.GetMem(0, y);
            writeText.Format("%d,", y + 1);
            for (long x = 0; x < zmap.GetSizeX(); x++)
            {
                if (zmap_y[x] == Ipvm::k_noiseValue32r)
                {
                    writeText += ",";
                }
                else
                {
                    writeText.AppendFormat("%.1f,", zmap_y[x]);
                }
            }

            writeText += "\n";
            file.Write((const char*)writeText, writeText.GetLength());
        }

        file.Close();
        return TRUE;
    }

    CFileDialog Dlg(FALSE, _T("bmp"), NULL, OFN_HIDEREADONLY, _T("Windows bitmap image file format (*.bmp)|*.bmp||"));

    CString initDir = PersonalConfig::getInstance().getLastAccessDirectory_Image();
    Dlg.GetOFN().lpstrInitialDir = initDir;

    if (Dlg.DoModal() != IDOK)
        return FALSE;

    if (m_imageLot->Save(Dlg.GetPathName()))
    {
        DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("SaveImageFile : %s"), LPCTSTR(Dlg.GetPathName()));
        PersonalConfig::getInstance().setLastAccessDirectory_Image(GET_PATHNAME(Dlg.GetPathName()));
        return TRUE;
    }

    return FALSE;
}

BOOL VisionUnit::OpenImageFile(CString strFileName)
{
    long nIndex = strFileName.ReverseFind('.');
    if (nIndex == -1)
    {
        return FALSE;
    }

    CString strFileExt = strFileName.Mid(nIndex + 1);

    if (strFileExt == "bmp")
    {
        CDialog Dlg;

        Dlg.BeginWaitCursor();

        BOOL result = OpenBmpImageFile(strFileName, true);

        // 모래시계 커서
        Dlg.EndWaitCursor();

        return result;
    }

    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("OpenImageFile : %s (Fail)"), LPCTSTR(strFileName));

    return FALSE;
}

BOOL VisionUnit::OpenBmpImageFile(CString strFileName, bool waitPopup)
{
    BOOL bResult = FALSE;

    bResult = m_imageLot->Load(strFileName, waitPopup);

    if (bResult)
    {
        long nRepeatSetCount = SystemConfig::GetInstance().IsVisionTypeSide() == TRUE ? 2 : 1;

        for (long nidx = 0; nidx < nRepeatSetCount; nidx++)
        {
            m_pImageLotInsp->Set(*m_imageLot, enSideVisionModule(nidx));
        }

        if (!m_pPackageSpec->IsSameDepedentScale(m_imageLot->GetScale()))
        {
            m_pPackageSpec->CalcDependentVariables(m_imageLot->GetScale());
        }
    }

    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("OpenImageFile : %s (%s)"), LPCTSTR(strFileName),
        bResult ? _T("Success") : _T("Fail"));

    return bResult;
}