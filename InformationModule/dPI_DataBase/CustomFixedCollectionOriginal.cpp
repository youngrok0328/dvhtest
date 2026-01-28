//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CustomFixedCollectionOriginal.h"

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

//CPP_7_________________________________ Implementation body
//
namespace Package
{
BOOL CustomFixedCollectionOriginal::LoadMap(CString strFilePath, CString& strErrorMessage)
{
    CFileFind ff;

    vecOriginCustomFixedData.clear();
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

    if (vecOriginCustomFixedData.size() <= 0)
    {
        strErrorMessage = _T("No CustomFixedLayer Datas !!!");
        return FALSE;
    }

    // 아이디 중복 체크
    {
        std::map<CString, long> indexCounts;

        for (const auto& CustomData : vecOriginCustomFixedData)
        {
            indexCounts[CustomData.m_strCustomFixedName]++;
        }

        CString strDuplicatedIDs(_T("Custom Fixed ID duplicated!!\n"));
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

BOOL CustomFixedCollectionOriginal::Set_MapData(CString strpathName, CString& strErrorMessage)
{
    enum fixColumn_CustomFixedMap
    {
        FX_ID,
        FX_CATEGORY,
        FX_CENTER_X,
        FX_CENTER_Y,
        FX_WIDTH,
        FX_LENGTH,
        FX_HEIGHT,
        FX_VISIBLE,
        FX_END,
    };

    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ifstream infile(strpathName);
    bool headerRead = false;

    char buffer[1000];

    long columnLink[FX_END];
    long headerColumnCount = 0;
    long rowIndex = 0;

    std::map<CStringA, long> mapDieNameToIndex;

    while (infile.getline(buffer, 1000))
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
                        columnName = _T("FixedCustomID");
                        break;
                    case FX_CATEGORY:
                        columnName = _T("Category");
                        break;
                    case FX_CENTER_X:
                        columnName = _T("Center Pos X(mm)");
                        break;
                    case FX_CENTER_Y:
                        columnName = _T("Center Pos Y(mm)");
                        break;
                    case FX_WIDTH:
                        columnName = _T("Width(mm)");
                        break;
                    case FX_LENGTH:
                        columnName = _T("Length(mm)");
                        break;
                    case FX_HEIGHT:
                        columnName = _T("Height(mm)");
                        break;
                    case FX_VISIBLE:
                        columnName = _T("Visible");
                        break;
                }

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

            if (GetCustomFixedCategory(dataList[columnLink[fixColumn_CustomFixedMap::FX_CATEGORY]])
                == CustomFixedCategory::CustomFixedCategory_Unknown)
            {
                strErrorMessage.Format(_T("'%s' Catrgory is Unknown type."),
                    LPCTSTR(CString(dataList[columnLink[fixColumn_CustomFixedMap::FX_ID]])));
                return FALSE;
            }

            CustomFixed cCustomFixedMapInfo;

            cCustomFixedMapInfo.m_strCustomFixedName = dataList[columnLink[fixColumn_CustomFixedMap::FX_ID]];
            cCustomFixedMapInfo.m_eCustomFixedCategory
                = GetCustomFixedCategory(dataList[columnLink[fixColumn_CustomFixedMap::FX_CATEGORY]]);
            cCustomFixedMapInfo.m_fSpecCenterPos_mm
                = Ipvm::Point32r2(fGetValue_mm(dataList[columnLink[fixColumn_CustomFixedMap::FX_CENTER_X]], false),
                    fGetValue_mm(dataList[columnLink[fixColumn_CustomFixedMap::FX_CENTER_Y]], false));
            cCustomFixedMapInfo.m_fSpecWidth_mm
                = fGetValue_mm(dataList[columnLink[fixColumn_CustomFixedMap::FX_WIDTH]], false);
            cCustomFixedMapInfo.m_fSpecLength_mm
                = fGetValue_mm(dataList[columnLink[fixColumn_CustomFixedMap::FX_LENGTH]], false);
            cCustomFixedMapInfo.m_fSpecHeight_mm
                = fGetValue_mm(dataList[columnLink[fixColumn_CustomFixedMap::FX_HEIGHT]], true);
            cCustomFixedMapInfo.m_bIgnore = bGetVisble(dataList[columnLink[fixColumn_CustomFixedMap::FX_VISIBLE]]);

            vecOriginCustomFixedData.emplace_back(cCustomFixedMapInfo);
        }
    }

    return TRUE;
}

float CustomFixedCollectionOriginal::fGetValue_mm(const CStringA i_strValue, const bool i_bisHeight)
{
    if (i_bisHeight == true)
    {
        if (i_strValue == _T("NULL") || i_strValue.IsEmpty())
            return 0.f * m_fUnit;
    }

    return static_cast<float>(::atof(i_strValue)) * m_fUnit;
}

BOOL CustomFixedCollectionOriginal::bGetVisble(const CStringA i_strVisble)
{
    bool bisalpha = isalpha(i_strVisble.GetAt(0));
    bool bisNumber = isdigit(i_strVisble.GetAt(0));
    if (i_strVisble == CStringA("0") && bisalpha || i_strVisble == CStringA("1"))
        return FALSE;
    else if (i_strVisble == CStringA("X") || i_strVisble == CStringA("O") && bisNumber)
        return TRUE;

    return FALSE;
}

CustomFixedCategory CustomFixedCollectionOriginal::GetCustomFixedCategory(const CStringA i_strCustomCategory)
{
    CStringA strCurrentCustomCatrgory = i_strCustomCategory;
    strCurrentCustomCatrgory.MakeUpper();

    if (strCurrentCustomCatrgory == _T("CIRCLE"))
        return CustomFixedCategory_Circle;
    else if (strCurrentCustomCatrgory == _T("RECTANGLE"))
        return CustomFixedCategory_Rectangle;
    else if (strCurrentCustomCatrgory == _T("NOTCHHOLE"))
        return CustomFixedCategory_NotchHole;

    return CustomFixedCategory_Unknown;
}

BOOL CustomFixedCollectionOriginal::SaveMap(CString strFilePath)
{
    CString m_strPath;
    CStdioFile file;
    CFileException ex;

    CString Data
        = _T("FixedCustomID,Category,Center Pos X(mm),Center Pos Y(mm),Width(mm),Length(mm),Height(mm),Visible\n");
    CString tmpData = _T("");

    for (auto OriginCustomInfo : vecOriginCustomFixedData)
    {
        CString strCustomCategory;

        switch (OriginCustomInfo.GetCatrgory())
        {
            case CustomFixedCategory::CustomFixedCategory_Circle:
                strCustomCategory = _T("Circle");
                break;
            case CustomFixedCategory::CustomFixedCategory_Rectangle:
                strCustomCategory = _T("Rectangle");
                break;
            case CustomFixedCategory::CustomFixedCategory_NotchHole:
                strCustomCategory = _T("NotchHole");
                break;
        }

        CString strIgnore;
        if (OriginCustomInfo.m_bIgnore)
            strIgnore = _T("0");
        else
            strIgnore = _T("1");

        tmpData = _T("");
        tmpData.Format(_T("%s,%s,%f,%f,%f,%f,%f,%s\n"), (LPCTSTR)OriginCustomInfo.m_strCustomFixedName,
            (LPCTSTR)strCustomCategory, OriginCustomInfo.m_fSpecCenterPos_mm.m_x,
            OriginCustomInfo.m_fSpecCenterPos_mm.m_y, OriginCustomInfo.m_fSpecWidth_mm,
            OriginCustomInfo.m_fSpecLength_mm, OriginCustomInfo.m_fSpecHeight_mm, (LPCTSTR)strIgnore);

        Data += tmpData;
    }

    m_strPath = strFilePath;
    file.Open(m_strPath, CFile::modeCreate | CFile::modeReadWrite, &ex);
    file.WriteString(Data);
    file.Close();

    return TRUE;
}

BOOL CustomFixedCollectionOriginal::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long nSize(0);
    CString strTemp;

    if (!bSave)
    {
        strTemp.Format(_T("CustomFixedLayerNumInMap"));
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;

        vecOriginCustomFixedData.clear();
        vecOriginCustomFixedData.resize(nSize);
    }
    else
    {
        nSize = (long)vecOriginCustomFixedData.size();

        strTemp.Format(_T("CustomFixedLayerNumInMap"));
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;
    }

    for (long i = 0; i < nSize; i++)
    {
        auto& CustomDB = db.GetSubDBFmt(_T("CustomFixed%d"), i + 1);
        vecOriginCustomFixedData[i].LinkDataBase(bSave, CustomDB);
    }

    strTemp = _T("CustomFixedDataInMap_FlipX");
    if (!db[strTemp].Link(bSave, m_bCustomMapFlipX))
        m_bCustomMapFlipX = FALSE;

    strTemp = _T("CustomFixedDataInMap_FlipY");
    if (!db[strTemp].Link(bSave, m_bCustomMapFlipY))
        m_bCustomMapFlipY = FALSE;

    strTemp = _T("CustomFixedDataInMap_RotateIndex");
    if (!db[strTemp].Link(bSave, m_nRotateIdx))
        m_nRotateIdx = 0;

    return TRUE;
}
} // namespace Package