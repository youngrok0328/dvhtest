//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PadCollectionOriginal.h"

//CPP_2_________________________________ This project's headers
#include "Enum.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
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
BOOL PadCollectionOriginal::LoadMap(CString strFilePath, CString& strErrorMessage)
{
    CFileFind ff;

    vecOriginPadData.clear();
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

    if (vecOriginPadData.size() <= 0)
    {
        strErrorMessage = _T("No Pads !!!");
        return FALSE;
    }

    // 아이디 중복 체크
    {
        std::map<CString, long> indexCounts;

        for (const auto& Pad : vecOriginPadData)
        {
            indexCounts[Pad.strPadName]++;
        }

        CString strDuplicatedIDs(_T("Pad ID duplicated!!\n"));
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

    // Pad 좌표 중복 체크
    {
        std::map<std::pair<double, double>, long> coordCounts;

        for (const auto& Pad : vecOriginPadData)
        {
            coordCounts[std::make_pair(Pad.GetCenter().m_x, Pad.GetCenter().m_y)]++;
        }

        CString strDuplicatedPadCoords(_T("Pad coordination duplicated!!\n"));
        long duplicatedPadCounts = 0;

        for (const auto& pair : coordCounts)
        {
            if (pair.second > 1)
            {
                if (duplicatedPadCounts > 10)
                {
                    strDuplicatedPadCoords += _T(".....");
                    break;
                }

                CString str;
                str.Format(_T("X:%.2lf Y:%.2lf Count:%d\n"), pair.first.first, pair.first.second, pair.second);

                strDuplicatedPadCoords += str;
                duplicatedPadCounts++;
            }
        }

        if (duplicatedPadCounts > 0)
        {
            strErrorMessage = strDuplicatedPadCoords;
            return FALSE;
        }
    }

    return TRUE;
}

BOOL PadCollectionOriginal::Set_MapData(CString strpathName, CString& strErrorMessage)
{
    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ifstream infile(strpathName);
    bool headerRead = false;

    char buffer[1000];

    enum fixColumn
    {
        FX_NAME,
        FX_X,
        FX_Y,
        FX_WIDTH,
        FX_LENGTH,
        FX_HEIGHT,
        FX_ANGLE,
        FX_TYPE,
        FX_MOUNT,
        FX_VISIBLE,
        FX_END,
    };

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
                    case FX_NAME:
                        columnName = _T("PadName");
                        break;
                    case FX_X:
                        columnName = _T("Xcoord(mm)");
                        break;
                    case FX_Y:
                        columnName = _T("Ycoord(mm)");
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
                    case FX_ANGLE:
                        columnName = _T("Angle(degree)");
                        break;
                    case FX_TYPE:
                        columnName = _T("Type");
                        break;
                    case FX_MOUNT:
                        columnName = _T("Mount");
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

            BOOL bVisible = FALSE;
            bool bisalpha = isalpha(dataList[columnLink[FX_VISIBLE]].GetAt(0));
            bool bisNumber = isdigit(dataList[columnLink[FX_VISIBLE]].GetAt(0));
            if (dataList[columnLink[FX_VISIBLE]] == CStringA("0") && bisalpha
                || dataList[columnLink[FX_VISIBLE]] == CStringA("1"))
                bVisible = FALSE;
            else if (dataList[columnLink[FX_VISIBLE]] == CStringA("X")
                || dataList[columnLink[FX_VISIBLE]] == CStringA("O") && bisNumber)
                bVisible = TRUE;

            float fHeight = 0.f;

            if (dataList[columnLink[FX_HEIGHT]] == _T("NULL") || dataList[columnLink[FX_HEIGHT]].IsEmpty())
            {
                fHeight = 0.f * m_fUnit;
            }
            else
                fHeight = static_cast<float>(::atof(dataList[columnLink[FX_HEIGHT]])) * m_fUnit;

            PAD_TYPE nPadType;
            dataList[columnLink[FX_TYPE]].MakeUpper();

            if (dataList[columnLink[FX_TYPE]] == _T("TRIANGLE"))
                nPadType = _typeTriangle;
            else if (dataList[columnLink[FX_TYPE]] == _T("CIRCLE"))
                nPadType = _typeCircle;
            else if (dataList[columnLink[FX_TYPE]] == _T("RECTANGLE"))
                nPadType = _typeRectangle;
            else if (dataList[columnLink[FX_TYPE]] == _T("PININDEX"))
                nPadType = _typePinIndex;

            BOOL bMount = FALSE;

            if (dataList[columnLink[FX_MOUNT]] == "NO")
                bMount = FALSE;
            else if (dataList[columnLink[FX_MOUNT]] == "YES")
                bMount = TRUE;

            vecOriginPadData.emplace_back(CString(dataList[columnLink[FX_NAME]]),
                ::atof(dataList[columnLink[FX_X]]) * m_fUnit, ::atof(dataList[columnLink[FX_Y]]) * m_fUnit,
                ::atof(dataList[columnLink[FX_WIDTH]]) * m_fUnit, ::atof(dataList[columnLink[FX_LENGTH]]) * m_fUnit,
                fHeight, ::atoi(dataList[columnLink[FX_ANGLE]]), nPadType, bMount, bVisible);
        }
    }

    if (vecOriginPadData.size() > 10000)
        ::SimpleMessage(
            _T("The number of pads is over 10,000, which is too many.\nIt will cause a decrease in speed."));

    return TRUE;
}

BOOL PadCollectionOriginal::VerifyFile(CString csvTitle)
{
    BOOL b_result;
    CString strKey;
    CString passiveTitle
        = _T("PadName,Xcoord(mm),Ycoord(mm),Width(mm),Length(mm),Height(mm),Angle(degree),Type,Mount,Visible");
    // 항목 값 확인
    csvTitle.Remove(_T(' '));
    passiveTitle.Remove(_T(' '));
    if (csvTitle.Find(passiveTitle) > -1)
    {
        b_result = TRUE;
    }
    else
    {
        b_result = FALSE;
    }
    return b_result;
}

void PadCollectionOriginal::Set_MapData(std::vector<CString>& vecstrToken, long nObjectNumber)
{
    CString strKey;
    CString strData;
    CString strTemp;
    BOOL bDataChanged = FALSE;

    vecOriginPadData.clear();
    vecOriginPadData.resize(nObjectNumber);

    for (long n = 0; n < nObjectNumber; n++)
    {
        vecOriginPadData[n].SetDefault();

        bDataChanged = FALSE;

        // 항목 값 확인
        if (!AfxExtractSubString(strKey, vecstrToken[0], 0, ','))
            break;

        //항목일 경우 Pass
        if (strKey.Find(_T("PadName")) > -1)
        {
            vecstrToken.erase(vecstrToken.begin());
            n = -1;
            continue;
        }
        else
        {
            long nCount = 0;

            for (long nData = 0; nData < 11; nData++)
            {
                // Data 추출
                if (!AfxExtractSubString(strTemp, vecstrToken[0], nCount++, ','))
                    break;

                // 빈공간은 그냥 뛰어넘도록 한다.
                if (strTemp.IsEmpty())
                {
                    nData--;
                    continue;
                }

                switch (nData)
                {
                    case PadDataType_PadName: // CompName
                        vecOriginPadData[n].strPadName = strTemp;
                        bDataChanged = TRUE;
                        break;
                    case PadDataType_CenterX: //Position X
                        vecOriginPadData[n].SetOffsetX((float)_ttof(strTemp) * m_fUnit);
                        bDataChanged = TRUE;
                        break;
                    case PadDataType_CenterY: //Position Y
                        vecOriginPadData[n].SetOffsetY((float)_ttof(strTemp) * m_fUnit);
                        bDataChanged = TRUE;
                        break;
                    case PadDataType_PadWidth: // Width
                        vecOriginPadData[n].fWidth = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case PadDataType_PadLength: // Length
                        vecOriginPadData[n].fLength = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case PadDataType_PadHeight: // Heigth
                        if (strTemp == _T("NULL") || strTemp.IsEmpty())
                        {
                            vecOriginPadData[n].fHeight = 0.f * m_fUnit;
                        }
                        else
                            vecOriginPadData[n].fHeight = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case PadDataType_PadAngle: // Angle
                        vecOriginPadData[n].nAngle = (long)_ttoi(strTemp);
                        bDataChanged = TRUE;
                        break;
                    case PadDataType_PadType: // PadType
                        strTemp.MakeUpper();
                        if (strTemp == _T("TRIANGLE"))
                            vecOriginPadData[n].SetType(_typeTriangle);
                        else if (strTemp == _T("CIRCLE"))
                            vecOriginPadData[n].SetType(_typeCircle);
                        else if (strTemp == _T("RECTANGLE"))
                            vecOriginPadData[n].SetType(_typeRectangle);
                        else if (strTemp == _T("PININDEX"))
                            vecOriginPadData[n].SetType(_typePinIndex);
                        bDataChanged = TRUE;
                        break;
                    case PadDataType_Mount: // Mount
                        strTemp.MakeUpper();
                        if (strTemp == "NO")
                            vecOriginPadData[n].bMount = FALSE;
                        else if (strTemp == "YES")
                            vecOriginPadData[n].bMount = TRUE;
                        bDataChanged = TRUE;
                        break;
                    case PadDataType_Visible:
                    {
                        strTemp.MakeUpper();
                        if (strTemp == "O")
                            vecOriginPadData[n].bIgnore = FALSE;
                        else if (strTemp == "X")
                            vecOriginPadData[n].bIgnore = TRUE;

                        bDataChanged = TRUE;
                        break;
                    }
                }
            }

            if (bDataChanged)
                vecstrToken.erase(vecstrToken.begin());
        }
    }
}

BOOL PadCollectionOriginal::SaveMap(CString strFilePath)
{
    CString m_strPath;
    CStdioFile file;
    CFileException ex;

    CString Data
        = _T("PadName,Xcoord(mm),Ycoord(mm),Width(mm),Length(mm),Height(mm),Angle(degree),Type,Mount,Visible\n");
    CString tmpData = _T("");

    for (auto OriginPadInfo : vecOriginPadData)
    {
        CString strPadType;

        switch (OriginPadInfo.GetType())
        {
            case _typeTriangle:
                strPadType = _T("Triangle");
                break;
            case _typeCircle:
                strPadType = _T("Circle");
                break;
            case _typeRectangle:
                strPadType = _T("Rectangle");
                break;
            case _typePinIndex:
                strPadType = _T("PinIndex");
                break;
        }

        CString strMount;
        if (OriginPadInfo.bMount)
        {
            strMount = _T("YES");
        }
        else
        {
            strMount = _T("NO");
        }

        CString strIgnore;
        if (OriginPadInfo.bIgnore)
            strIgnore = _T("0");
        else
            strIgnore = _T("1");

        tmpData = _T("");
        tmpData.Format(_T("%s,%f,%f,%f,%f,%f,%ld,%s,%s,%s\n"), (LPCTSTR)OriginPadInfo.strPadName,
            OriginPadInfo.GetOffset().m_x, OriginPadInfo.GetOffset().m_y, OriginPadInfo.fWidth, OriginPadInfo.fLength,
            OriginPadInfo.fHeight, OriginPadInfo.nAngle, (LPCTSTR)strPadType, (LPCTSTR)strMount, (LPCTSTR)strIgnore);
        Data += tmpData;
    }

    m_strPath = strFilePath;
    file.Open(m_strPath, CFile::modeCreate | CFile::modeReadWrite, &ex);
    file.WriteString(Data);
    file.Close();

    return TRUE;
}

BOOL PadCollectionOriginal::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 2;
    if (!db[_T("Pad Version")].Link(bSave, version))
        version = 0;

    long nSize(0);
    CString strTemp;

    if (!bSave)
    {
        strTemp.Format(_T("PadNumInMap"));
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;

        vecOriginPadData.clear();
        vecOriginPadData.resize(nSize);
    }
    else
    {
        nSize = (long)vecOriginPadData.size();

        strTemp.Format(_T("PadNumInMap"));
        if (!db[strTemp].Link(bSave, nSize))
            nSize = 0;
    }

    if (version < 2)
    {
        Pad padMapData;
        CString strData;
        for (long i = 0; i < nSize; i++)
        {
            vecOriginPadData[i].LinkDataBase(version, i, bSave, db);
        }
    }
    else
    {
        Pad padMapData;
        CString strData;
        for (long i = 0; i < nSize; i++)
        {
            auto& padDB = db.GetSubDBFmt(_T("PAD%d"), i + 1);
            vecOriginPadData[i].LinkDataBase(version, i, bSave, padDB);
        }
    }

    strTemp = _T("PadDataInMap_FlipX");
    if (!db[strTemp].Link(bSave, m_bPadMapFlipX))
        m_bPadMapFlipX = FALSE;

    strTemp = _T("PadDataInMap_FlipY");
    if (!db[strTemp].Link(bSave, m_bPadMapFlipY))
        m_bPadMapFlipY = FALSE;

    strTemp = _T("PadDataInMap_RotateIndex");
    if (!db[strTemp].Link(bSave, m_nRotateIdx))
        m_nRotateIdx = 0;

    return TRUE;
}

} // namespace Package