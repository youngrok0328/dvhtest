//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DynamicSystemPath.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
class DynamicSystemPathData
{
public:
    CString m_path_root;
    CString m_path_bin;
    CString m_path_config;
    CString m_path_system;
    CString m_path_image;
    CString m_path_job;
    CString m_path_jobChipAlgo;
    CString m_path_jobPadAlignAlgo;
    CString m_path_log;
    CString m_path_PackageViewerData;
    CString m_path_TimeLog;
    CString m_path_markImage;
    CString m_path_temp;

    CString m_path_systemFile;
    CString m_path_systemIllumTableRefFile;
    CString m_path_systemIllumTableFile;
    CString m_path_systemSlitbeamDistortionProfileFile;
    CString m_path_systemSlitbeamDistortionFullFileHead_N;
    CString m_path_systemSlitbeamDistortionFullFileHead_B;
    CString m_path_systemIllumTableRefFileAll; //kircheis_IllumCal
    CString m_path_systemIllumTableFileAll; //kircheis_IllumCal
    CString m_path_systemIllumTableRefFileAllMirror; //SD_DualCal
    CString m_path_systemIllumTableFileAllMirror; //SD_DualCal

    //SDY_Side Vision Calibration
    CString m_path_systemIllumTableFileAll_Front; //SDY_SideVision
    CString m_path_systemIllumTableFileAllMirror_Front; //SDY_SideVision

    CString m_path_systemIllumTableFileAll_Rear; //SDY_SideVision
    CString m_path_systemIllumTableFileAllMirror_Rear; //SDY_SideVision

    DynamicSystemPathData()
    {
        m_path_root = _T("C:\\IntekPlus\\iPIS-500I\\Vision\\");
        m_path_bin = m_path_root + _T("Bin\\");

        Update();
    }

    void Update()
    {
        // 폴더 경로
        m_path_config = m_path_root + _T("Config\\");
        m_path_system = m_path_root + _T("System\\");
        m_path_image = m_path_root + _T("Image\\");
        m_path_job = m_path_root + _T("Job\\");
        m_path_jobChipAlgo = m_path_job + _T("AlgoDB\\");
        m_path_jobPadAlignAlgo = m_path_job + _T("PAD AlgoDB\\");
        m_path_log = m_path_root + _T("Log\\");
        m_path_TimeLog = m_path_log + _T("TimeLog\\");
        m_path_PackageViewerData = m_path_log + _T("Package ViewerData\\");
        m_path_markImage = m_path_root + _T("Mark Image\\");
        m_path_temp = m_path_root + _T("Temp\\");

        // File 경로
        m_path_systemFile = m_path_system + _T("System.ini");
        m_path_systemIllumTableRefFile = m_path_system + _T("IlluminationTableRef.csv");
        m_path_systemIllumTableFile = m_path_system + _T("IlluminationTable.csv");
        m_path_systemSlitbeamDistortionProfileFile = m_path_system + _T("SlitbeamDistortionProfile.csv");
        m_path_systemSlitbeamDistortionFullFileHead_N = m_path_system + _T("SlitbeamDistortionFullN_");
        m_path_systemSlitbeamDistortionFullFileHead_B = m_path_system + _T("SlitbeamDistortionFullB_");
        m_path_systemIllumTableRefFileAll = m_path_system + _T("IlluminationTableRefAll.csv"); //kircheis_IllumCal
        m_path_systemIllumTableFileAll = m_path_system + _T("IlluminationTableAll.csv"); //kircheis_IllumCal
        m_path_systemIllumTableRefFileAllMirror = m_path_system + _T("IlluminationTableRefAllMirror.csv"); //SDY_DualCal
        m_path_systemIllumTableFileAllMirror = m_path_system + _T("IlluminationTableAllMirror.csv"); //SDY_DualCal

        //SDY_Side Vision Calibration
        m_path_systemIllumTableFileAll_Front = m_path_system + _T("IlluminationTableAll_F.csv"); //SDY_SideVision
        m_path_systemIllumTableFileAllMirror_Front
            = m_path_system + _T("IlluminationTableAllMirror_F.csv"); //SDY_SideVision

        m_path_systemIllumTableFileAll_Rear = m_path_system + _T("IlluminationTableAll_R.csv"); //SDY_SideVision
        m_path_systemIllumTableFileAllMirror_Rear
            = m_path_system + _T("IlluminationTableAllMirror_R.csv"); //SDY_SideVision
    }
};

DynamicSystemPathData g_data;

void DynamicSystemPath::setModulePath(LPCTSTR modulePath)
{
    CString path = modulePath;

    PathRemoveFileSpec(path.GetBuffer());
    path.ReleaseBuffer();
    path += _T("\\");

    g_data.m_path_bin = path;

    _tfullpath(g_data.m_path_root.GetBuffer(g_data.m_path_bin.GetLength() + 4), g_data.m_path_bin + _T("..\\"),
        g_data.m_path_bin.GetLength() + 4);

    g_data.m_path_root.ReleaseBuffer();

    g_data.Update();
}

CString DynamicSystemPath::get(DefineFolder type)
{
    switch (type)
    {
        case DefineFolder::Root:
            return g_data.m_path_root;
        case DefineFolder::Binary:
            return g_data.m_path_bin;
        case DefineFolder::Config:
            return g_data.m_path_config;
        case DefineFolder::System:
            return g_data.m_path_system;
        case DefineFolder::Image:
            return g_data.m_path_image;
        case DefineFolder::Job:
            return g_data.m_path_job;
        case DefineFolder::Job_ChipAlgo:
            return g_data.m_path_jobChipAlgo;
        case DefineFolder::Job_PadAlignAlgo:
            return g_data.m_path_jobPadAlignAlgo;
        case DefineFolder::Log:
            return g_data.m_path_log;
        case DefineFolder::TimeLog:
            return g_data.m_path_TimeLog;
        case DefineFolder::Pacakge_ViewerData:
            return g_data.m_path_PackageViewerData;
        case DefineFolder::MarkImage:
            return g_data.m_path_markImage;
        case DefineFolder::Temp:
            return g_data.m_path_temp;
    }

    ASSERT(_T("??"));
    return _T("");
}

CString DynamicSystemPath::get(DefineFile type)
{
    switch (type)
    {
        case DefineFile::System:
            return g_data.m_path_systemFile;
        case DefineFile::IllumTableRefTable:
            return g_data.m_path_systemIllumTableRefFile;
        case DefineFile::IllumTableTable:
            return g_data.m_path_systemIllumTableFile;
        case DefineFile::SlitbeamDistortionProfile:
            return g_data.m_path_systemSlitbeamDistortionProfileFile;
        case DefineFile::IllumTableRefTableAll:
            return g_data.m_path_systemIllumTableRefFileAll; //kircheis_IllumCal
        case DefineFile::IllumTableTableAll:
            return g_data.m_path_systemIllumTableFileAll; //kircheis_IllumCal
        case DefineFile::IllumTableRefTableAllMirror:
            return g_data.m_path_systemIllumTableRefFileAllMirror; //SDY_DualCal
        case DefineFile::IllumTableTableAllMirror:
            return g_data.m_path_systemIllumTableFileAllMirror; //SDY_DualCal
        case DefineFile::IllumTableTableAll_F:
            return g_data.m_path_systemIllumTableFileAll_Front; //SDY_SideVision
        case DefineFile::IllumTableTableAllMirror_F:
            return g_data.m_path_systemIllumTableFileAllMirror_Front; //SDY_SideVision
        case DefineFile::IllumTableTableAll_R:
            return g_data.m_path_systemIllumTableFileAll_Rear; //SDY_SideVision
        case DefineFile::IllumTableTableAllMirror_R:
            return g_data.m_path_systemIllumTableFileAllMirror_Rear; //SDY_SideVision
    }

    ASSERT(_T("??"));
    return _T("");
}

CString DynamicSystemPath::getDistortionFullFilePath(
    double scanDepth_um, bool cameraVerticalBinning, bool isToUseNearestDistortionXEnabled)
{
    CString retValue;
    CString fileHead = cameraVerticalBinning ? g_data.m_path_systemSlitbeamDistortionFullFileHead_B
                                             : g_data.m_path_systemSlitbeamDistortionFullFileHead_N;

    if (isToUseNearestDistortionXEnabled)
    {
        // Distortion File 목록을 얻는다.
        CFileFind ff;
        auto filter = fileHead + _T("*.zmap");
        auto isFind = ff.FindFile(filter);

        std::vector<std::pair<double, CString>> distortionList;

        while (isFind)
        {
            isFind = ff.FindNextFile();
            if (ff.IsDirectory())
                continue;

            CString filePath = ff.GetFilePath();
            CString fileName = ff.GetFileName();
            long findUnderbar = fileName.Find(_T("_"));
            if (findUnderbar < 0)
                continue;

            double depth = _ttof(fileName.Mid(findUnderbar + 1));

            distortionList.push_back(std::make_pair(depth, filePath));
        }

        ff.Close();

        // ScanDepth 중 가장 가까운 Distortion File 이름을 전달해준자
        if (distortionList.size())
        {
            double distance = fabs(scanDepth_um - distortionList[0].first);
            retValue = distortionList[0].second;

            for (auto& check : distortionList)
            {
                double checkDistance = fabs(scanDepth_um - check.first);
                if (distance > checkDistance)
                {
                    distance = checkDistance;
                    retValue = check.second;
                }
            }
        }

        return retValue;
    }

    retValue.Format(_T("%s_%04d.zmap"), LPCTSTR(fileHead), int(scanDepth_um));
    return retValue;
}
