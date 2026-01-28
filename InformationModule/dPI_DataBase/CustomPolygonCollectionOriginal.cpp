//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CustomPolygonCollectionOriginal.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <fstream>
#include <map>
#include <string>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_POINT_NUM 100

//CPP_7_________________________________ Implementation body
//
namespace Package
{
BOOL CustomPolygonCollectionOriginal::LoadMap(CString strFilePath, CString& strErrorMessage)
{
    CFileFind ff;

    vecOriginCustomPolygonData.clear();
    // CSV File에 써진 Data를 읽어온다.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    if (ff.FindFile(strFilePath))
    {
        std::vector<unsigned char> strTemp(500000);
        CFile FileIn(strFilePath, CFile::modeRead);
        long nLength = (long)FileIn.GetLength();

        if (nLength < 10)
        {
            strErrorMessage.Format(_T("File length is too short !!!"));
            return FALSE;
        }

        // Check Encoding
        char temp[10];
        FileIn.Read(temp, 10);
        FileIn.Close();

        // Read
        bool isWindowsUnicode = (temp[0] == -1 && temp[1] == -2) // BOM Check
            && !(temp[3] | temp[5] | temp[7] | temp[9]); // 영문자, 숫자, 쉼표만 있으므로 0 이어야 한다.

        if (isWindowsUnicode)
        {
            strErrorMessage.Format(_T("Invalid file encoding"));
            return FALSE;
        }
    }
    else
    {
        strErrorMessage.Format(_T("Do not Find File."));
        return FALSE;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////

    // 분리된 Data 추출
    ////////////////////////////////////////////////////////////////////////////////////////////////
    m_fUnit = 1.f; //mm m_fUnit = 1000.f // um;
    if (!Set_MapData(strFilePath, strErrorMessage))
    {
        return FALSE;
    }

    if (vecOriginCustomPolygonData.size() <= 0)
    {
        strErrorMessage = _T("No CustomPolygonLayer Datas !!!");
        return FALSE;
    }

    // 아이디 중복 체크
    {
        std::map<CString, long> indexCounts;

        for (const auto& CustomPolygonData : vecOriginCustomPolygonData)
        {
            indexCounts[CustomPolygonData.m_strCustomPolygonName]++;
        }

        CString strDuplicatedIDs(_T("Custom Polygon ID duplicated!!\n"));
        long duplicatedIDCounts = 0;

        for (const auto& pair : indexCounts)
        {
            if (pair.second > 1)
            {
                if (duplicatedIDCounts > 10)
                {
                    strDuplicatedIDs += _T(".....");
                    break;
                }

                CString str;
                str.Format(_T("%s\n"), LPCTSTR(pair.first));

                strDuplicatedIDs += str;
                duplicatedIDCounts++;
            }
        }

        if (duplicatedIDCounts > 0)
        {
            strErrorMessage = strDuplicatedIDs;
            return FALSE;
        }
    }

    //// Pad 좌표 중복 체크
    //{
    //	std::map<std::pair<double, double>, long> coordCounts;

    //	for (const auto &Pad : vecOriginCustomData)
    //	{
    //		coordCounts[std::make_pair(Pad.GetCenter().m_x, Pad.GetCenter().m_y)]++;
    //	}

    //	CString strDuplicatedPadCoords(_T("Pad coordination duplicated!!\n"));
    //	long duplicatedPadCounts = 0;

    //	for (const auto &pair : coordCounts)
    //	{
    //		if (pair.second > 1)
    //		{
    //			if (duplicatedPadCounts > 10)
    //			{
    //				strDuplicatedPadCoords += _T(".....");
    //				break;
    //			}

    //			CString str;
    //			str.Format(_T("X:%.2lf Y:%.2lf Count:%d\n"), pair.first.first, pair.first.second, pair.second);

    //			strDuplicatedPadCoords += str;
    //			duplicatedPadCounts++;
    //		}
    //	}

    //	if (duplicatedPadCounts > 0)
    //	{
    //		strErrorMessage = strDuplicatedPadCoords;
    //		return FALSE;
    //	}
    //}

    return TRUE;
}

BOOL CustomPolygonCollectionOriginal::Set_MapData(CString strpathName, CString& strErrorMessage)
{
    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ifstream infile(strpathName);
    bool headerRead = false;

    char buffer[5000];

    long columnLink[FX_END];
    long headerColumnCount = 0;
    long rowIndex = 0;

    std::map<CStringA, long> mapDieNameToIndex;

    while (infile.getline(buffer, 5000))
    {
        rowIndex++;

        std::basic_string<char> temp(buffer);

        temp_string_size_type findStart = 0;
        temp_string_size_type findEnd = 0;

        if (!headerRead)
        {
            //---------------------------------------------------------------------------------------
            // Read Header
            //---------------------------------------------------------------------------------------

            std::map<CStringA, long> headerIndexMap;
            long index = 0;
            while (1)
            {
                findEnd = temp.find_first_of(',', findStart);
                CStringA text;
                if (findEnd == std::basic_string<char>::npos)
                {
                    text.SetString(buffer + findStart, int(temp.length() - findStart));
                    headerIndexMap[text] = index;
                    break;
                }
                else
                {
                    text.SetString(buffer + findStart, int(findEnd - findStart));
                    headerIndexMap[text] = index;
                    findStart = findEnd + 1;
                    index++;
                }
            }

            // 헤더 정보중 필수 항목들이 모두 들어왔는지 확인한다

            for (long item = 0; item < FX_END; item++)
            {
                CStringA columnName;
                switch (item)
                {
                    case FX_ID:
                        columnName = _T("PolygonCustomID");
                        break;
                    case FX_CATEGORY:
                        columnName = _T("Category");
                        break;
                    case FX_POINTNUM:
                        columnName = _T("PointNum");
                        break;
                    case FX_HEIGHT:
                        columnName = _T("Height(mm)");
                        break;
                    case FX_VISIBLE:
                        columnName = _T("Visible");
                        break;
                }

                //mc_Point개수는 최대 100개이므로 이후에 추가적으로 확인해야한다

                if (columnName.IsEmpty())
                    break;

                if (headerIndexMap.find(columnName) == headerIndexMap.end())
                {
                    strErrorMessage.Format(_T("'%s' Column not found."), LPCTSTR(CString(columnName)));
                    return FALSE;
                }

                columnLink[item] = headerIndexMap[columnName];
            }

            headerColumnCount = long(headerIndexMap.size());
            headerRead = true;
            continue;
        }

        std::vector<CStringA> dataList;
        while (1)
        {
            findEnd = temp.find_first_of(',', findStart);
            CStringA text;
            if (findEnd == std::basic_string<char>::npos)
            {
                text.SetString(buffer + findStart, int(temp.length() - findStart));
                dataList.push_back(text);
                break;
            }
            else
            {
                text.SetString(buffer + findStart, int(findEnd - findStart));
                dataList.push_back(text);
                findStart = findEnd + 1;
            }
        }

        if (dataList.size())
        {
            if (dataList.size() != headerColumnCount)
            {
                // 헤더 컬럼과 데이터 컴럼의 수가 다르다
                strErrorMessage.Format(_T("The number of columns is different. (%d Line)"), rowIndex);
                return FALSE;
            }

            if (GetCustomPolygonCategory(dataList[columnLink[fixColumn_CustomPolygonMap::FX_CATEGORY]])
                == CustomPolygonCategory::CustomPolygonCategory_Unknown)
            {
                strErrorMessage.Format(_T("'%s' Catrgory is Unknown type."),
                    LPCTSTR(CString(dataList[columnLink[fixColumn_CustomPolygonMap::FX_ID]])));
                return FALSE;
            }

            if (::atol(dataList[columnLink[fixColumn_CustomPolygonMap::FX_POINTNUM]]) > MAX_POINT_NUM)
            {
                strErrorMessage.Format(_T("'%s' \nThe number of input polygons exceeds the maximum."),
                    LPCTSTR(CString(dataList[columnLink[fixColumn_CustomPolygonMap::FX_ID]])));
                return FALSE;
            }

            CustomPolygon cCustomPolygonMapInfo;

            cCustomPolygonMapInfo.m_strCustomPolygonName = dataList[columnLink[fixColumn_CustomPolygonMap::FX_ID]];
            cCustomPolygonMapInfo.m_eCustomPolygonCategory
                = GetCustomPolygonCategory(dataList[columnLink[fixColumn_CustomPolygonMap::FX_CATEGORY]]);
            cCustomPolygonMapInfo.m_nPointNum = ::atol(dataList[columnLink[fixColumn_CustomPolygonMap::FX_POINTNUM]]);
            cCustomPolygonMapInfo.m_fSpecHeight_mm
                = fGetValue_mm(dataList[columnLink[fixColumn_CustomPolygonMap::FX_HEIGHT]], true);
            cCustomPolygonMapInfo.m_bIgnore = bGetVisble(dataList[columnLink[fixColumn_CustomPolygonMap::FX_VISIBLE]]);

            if (cCustomPolygonMapInfo.m_nPointNum > 0)
                cCustomPolygonMapInfo.m_vecfptPointInfo_mm = GetPointInfo(dataList, cCustomPolygonMapInfo.m_nPointNum);

            if ((long)cCustomPolygonMapInfo.m_vecfptPointInfo_mm.size() != cCustomPolygonMapInfo.m_nPointNum)
            {
                strErrorMessage.Format(_T("'%s' \nPoint info count is not matching Point num."),
                    LPCTSTR(CString(cCustomPolygonMapInfo.m_strCustomPolygonName)));
                cCustomPolygonMapInfo.m_vecfptPointInfo_mm.clear();
                return FALSE;
            }

            vecOriginCustomPolygonData.emplace_back(cCustomPolygonMapInfo);
        }
    }

    return TRUE;
}

float CustomPolygonCollectionOriginal::fGetValue_mm(
    const CStringA i_strValue, const bool i_bisHeight, const bool i_bisPointValue)
{
    if (i_bisHeight == true)
    {
        if (i_strValue == _T("NULL") || i_strValue.IsEmpty())
            return 0.f * m_fUnit;
    }

    if (i_bisPointValue == true)
    {
        if (i_strValue == _T("X") || i_strValue.IsEmpty())
            return Ipvm::k_noiseValue32r * m_fUnit;
    }

    return static_cast<float>(::atof(i_strValue)) * m_fUnit;
}

BOOL CustomPolygonCollectionOriginal::bGetVisble(const CStringA i_strVisble)
{
    bool bisalpha = isalpha(i_strVisble.GetAt(0));
    bool bisNumber = isdigit(i_strVisble.GetAt(0));
    if (i_strVisble == CStringA("0") && bisalpha || i_strVisble == CStringA("1"))
        return FALSE;
    else if (i_strVisble == CStringA("X") || i_strVisble == CStringA("O") && bisNumber)
        return TRUE;

    return FALSE;
}

CustomPolygonCategory CustomPolygonCollectionOriginal::GetCustomPolygonCategory(const CStringA i_strCustomCategory)
{
    CStringA strCurrentCustomCatrgory = i_strCustomCategory;
    strCurrentCustomCatrgory.MakeUpper();

    if (strCurrentCustomCatrgory == _T("POLYGON"))
        return CustomPolygonCategory_Polygon;

    return CustomPolygonCategory_Unknown;
}

std::vector<Ipvm::Point32r2> CustomPolygonCollectionOriginal::GetPointInfo(
    const std::vector<CStringA> i_vecstrDataList, const long i_nPointNum)
{
    UNREFERENCED_PARAMETER(i_nPointNum);

    std::vector<Ipvm::Point32r2> vecfptPointInfo;

    //const long nPointTotalNum = i_nPointNum * 2; //X,Y 2개씩
    const long nPointTotalNum = MAX_POINT_NUM * 2; //X,Y 2개씩

    bool bisDataCenterPosY(false);
    bool bisDataPush(false);
    Ipvm::Point32r2 fptCenterPos_mm(0.f, 0.f);
    for (long nPointInfoidx = 0; nPointInfoidx < nPointTotalNum; nPointInfoidx++)
    {
        long nInfoidx = (long)fixColumn_CustomPolygonMap::FX_VISIBLE + nPointInfoidx + 1;
        if (bisDataCenterPosY == true)
        {
            fptCenterPos_mm.m_y = fGetValue_mm(i_vecstrDataList[nInfoidx], false, true);
            bisDataCenterPosY = false;
            bisDataPush = true;
        }
        else
        {
            fptCenterPos_mm.m_x = fGetValue_mm(i_vecstrDataList[nInfoidx], false, true);
            bisDataCenterPosY = true;
            bisDataPush = false;
        }

        if (fptCenterPos_mm.m_x == Ipvm::k_noiseValue32r || fptCenterPos_mm.m_y == Ipvm::k_noiseValue32r)
            break;

        if (bisDataPush == true)
            vecfptPointInfo.push_back(fptCenterPos_mm);
    }

    //if (vecfptPointInfo.size() != i_nPointNum)
    //{
    //	::AfxMessageBox(_T("Point info count is not matching Point num"));
    //	vecfptPointInfo.clear();
    //}

    return vecfptPointInfo;
}

BOOL CustomPolygonCollectionOriginal::SaveMap(CString strFilePath)
{
    CString m_strPath;
    CStdioFile file;
    CFileException ex;

    CString Data = _T("PolygonCustomID,Category,PointNum,Height(mm),Visible");

    CString tmpData = _T("");

    for (long nPolygonidx = 0; nPolygonidx < MAX_POINT_NUM; nPolygonidx++)
    {
        CString strPolygonX(""), strPolygonY("");

        strPolygonX.Format(_T(",P%dX(mm)"), nPolygonidx + 1);
        strPolygonY.Format(_T(",P%dY(mm)"), nPolygonidx + 1);

        Data += strPolygonX;
        Data += strPolygonY;

        if (nPolygonidx == MAX_POINT_NUM - 1) //마지막에는 줄바꿈을 해줘야 한다
            Data += _T("\n");
    }

    for (auto OriginPolygonCustomInfo : vecOriginCustomPolygonData)
    {
        CString strCustomCategory;

        switch (OriginPolygonCustomInfo.GetCatrgory())
        {
            case CustomPolygonCategory::CustomPolygonCategory_Polygon:
                strCustomCategory = _T("Polygon");
                break;
        }

        CString strIgnore;
        if (OriginPolygonCustomInfo.m_bIgnore)
            strIgnore = _T("0");
        else
            strIgnore = _T("1");

        CString strTotalPointInfoData = GetTotalPointInfo(OriginPolygonCustomInfo.m_vecfptPointInfo_mm, MAX_POINT_NUM);

        tmpData = _T("");
        tmpData.Format(_T("%s,%s,%d,%f,%s,%s\n"), (LPCTSTR)OriginPolygonCustomInfo.m_strCustomPolygonName,
            (LPCTSTR)strCustomCategory, OriginPolygonCustomInfo.m_nPointNum, OriginPolygonCustomInfo.m_fSpecHeight_mm,
            (LPCTSTR)strIgnore, (LPCTSTR)strTotalPointInfoData);

        Data += tmpData;
    }

    m_strPath = strFilePath;
    file.Open(m_strPath, CFile::modeCreate | CFile::modeReadWrite, &ex);
    file.WriteString(Data);
    file.Close();

    return TRUE;
}

CString CustomPolygonCollectionOriginal::GetTotalPointInfo(
    const std::vector<Ipvm::Point32r2> i_vecfptTotalPointInfo, const long i_nMax_PointNum)
{
    std::vector<CString> vecstrPointInfoTemp; //X,Y 데이터 들이 들어갈것이다
    CString strRetunValue("");
    long nVaildPointCount = (long)min(i_vecfptTotalPointInfo.size(), i_nMax_PointNum);

    for (long nidx = 0; nidx < i_nMax_PointNum; nidx++)
    {
        if (nidx < nVaildPointCount)
        {
            CString strCurrentData("");
            strCurrentData.Format(_T("%f"), i_vecfptTotalPointInfo[nidx].m_x);
            vecstrPointInfoTemp.push_back(strCurrentData);
            strCurrentData.Format(_T("%f"), i_vecfptTotalPointInfo[nidx].m_y);
            vecstrPointInfoTemp.push_back(strCurrentData);
        }
        else
        {
            //mc_임시방편 딱히 떠오르지 않는다
            vecstrPointInfoTemp.push_back(_T("X"));
            vecstrPointInfoTemp.push_back(_T("X"));
        }
    }

    for (long nTotalDataidx = 0; nTotalDataidx < vecstrPointInfoTemp.size(); nTotalDataidx++)
        strRetunValue += vecstrPointInfoTemp[nTotalDataidx] + _T(",");

    return strRetunValue;
}

BOOL CustomPolygonCollectionOriginal::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long nSize(0);
    CString strTemp;

    if (!bSave)
    {
        strTemp.Format(_T("CustomPolygonLayerNumInMap"));
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;

        vecOriginCustomPolygonData.clear();
        vecOriginCustomPolygonData.resize(nSize);
    }
    else
    {
        nSize = (long)vecOriginCustomPolygonData.size();

        strTemp.Format(_T("CustomPolygonLayerNumInMap"));
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;
    }

    for (long i = 0; i < nSize; i++)
    {
        auto& CustomDB = db.GetSubDBFmt(_T("CustomPolygon%d"), i + 1);
        vecOriginCustomPolygonData[i].LinkDataBase(bSave, CustomDB);
    }

    strTemp = _T("CustomPolygonDataInMap_FlipX");
    if (!db[strTemp].Link(bSave, m_bCustomMapFlipX))
        m_bCustomMapFlipX = FALSE;

    strTemp = _T("CustomPolygonDataInMap_FlipY");
    if (!db[strTemp].Link(bSave, m_bCustomMapFlipY))
        m_bCustomMapFlipY = FALSE;

    strTemp = _T("CustomPolygonDataInMap_RotateIndex");
    if (!db[strTemp].Link(bSave, m_nRotateIdx))
        m_nRotateIdx = 0;

    return TRUE;
}

} // namespace Package