#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
struct sRejectInfo
{
    long nNumFrame;
    long nImageWidth;
    long nImageHeight;
    long nImageDepth;

    std::vector<CString> vecstrFilePath;
    std::vector<CRect> vecrtRejectROI;
};

struct sCollectMvResultInfo
{
    long nTotalRejectNum;
    CString strRejectItemName;
    CString strVisionType;
    CString strLotStartTime;
    CString strLotID;
    CString strJobName;
    std::vector<sRejectInfo> vecsRejectMainInfo;

    void Serialization(CArchive& ar)
    {
        if (ar.IsStoring())
        {
            ar << nTotalRejectNum;
            ar << strRejectItemName;
            ar << strVisionType;
            ar << strLotStartTime;
            ar << strLotID;
            ar << strJobName;

            for (long i = 0; i < vecsRejectMainInfo.size(); i++)
            {
                long nNumFrame = vecsRejectMainInfo[i].nNumFrame;
                ar << nNumFrame;
                ar << vecsRejectMainInfo[i].nImageWidth;
                ar << vecsRejectMainInfo[i].nImageHeight;
                ar << vecsRejectMainInfo[i].nImageDepth;

                for (long j = 0; j < vecsRejectMainInfo[i].vecstrFilePath.size(); j++)
                {
                    ar << vecsRejectMainInfo[i].vecstrFilePath[j];
                }

                long nRoiNum = (long)vecsRejectMainInfo[i].vecrtRejectROI.size();
                ar << nRoiNum;

                for (long j = 0; j < vecsRejectMainInfo[i].vecrtRejectROI.size(); j++)
                {
                    ar << vecsRejectMainInfo[i].vecrtRejectROI[j];
                }
            }
        }
        else
        {
            vecsRejectMainInfo.clear();

            ar >> nTotalRejectNum;
            ar >> strRejectItemName;
            ar >> strVisionType;
            ar >> strLotStartTime;
            ar >> strLotID;
            ar >> strJobName;

            vecsRejectMainInfo.resize(nTotalRejectNum);

            for (long i = 0; i < nTotalRejectNum; i++)
            {
                long nNumFrame;
                ar >> nNumFrame;
                vecsRejectMainInfo[i].vecstrFilePath.resize(nNumFrame);
                ar >> vecsRejectMainInfo[i].nImageWidth;
                ar >> vecsRejectMainInfo[i].nImageHeight;
                ar >> vecsRejectMainInfo[i].nImageDepth;

                for (long j = 0; j < nNumFrame; j++)
                {
                    ar >> vecsRejectMainInfo[i].vecstrFilePath[j];
                }

                long nRoiNum;
                ar >> nRoiNum;
                vecsRejectMainInfo[i].vecrtRejectROI.resize(nRoiNum);

                for (long j = 0; j < nRoiNum; j++)
                {
                    ar >> vecsRejectMainInfo[i].vecrtRejectROI[j];
                }
            }
        }
    }
};