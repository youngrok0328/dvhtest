//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "LandCollectionOriginal.h"

//CPP_2_________________________________ This project's headers
#include "Enum.h"
#include "Land.h"
#include "Pad.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <algorithm>
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
BOOL LandCollectionOriginal::LoadMap(CString strFilePath, CString& strErrorMessage)
{
    CFileFind ff;
    m_bTitleVisible = FALSE;

    vecOriginLandData.clear();

    // CSV File에 써진 Data를 읽어온다.
    ////////////////////////////////////////////////////////////////////////////////////////////////
    if (ff.FindFile(strFilePath))
    {
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
        strErrorMessage.Format(_T("Cannot open file !!!"));
        return FALSE;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////

    // 분리된 Data 추출
    ////////////////////////////////////////////////////////////////////////////////////////////////
    m_fUnit = 1.f; //mm m_fUnit = 1000.f // um;
    if (!Set_MapData_Shape(strFilePath, strErrorMessage))
    {
        if (!Set_MapData(strFilePath, strErrorMessage))
        {
            if (!Set_MapData_IgnoreVisible(strFilePath, strErrorMessage))
                return FALSE;
        }
    }

    if (vecOriginLandData.size() <= 0)
    {
        strErrorMessage = _T("No Lands !!!");
        return FALSE;
    }

    // 아이디 중복 체크
    {
        std::map<CString, long> indexCounts;

        for (const auto& Land : vecOriginLandData)
        {
            indexCounts[Land.strLandID]++;
        }

        CString strDuplicatedIDs(_T("Land ID duplicated!!\n"));
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
                str.Format(_T("%s "), LPCTSTR(pair.first));

                strDuplicatedIDs += str;
                duplicatedIDCounts++;
            }
        }

        strDuplicatedIDs += _T("\n");

        if (duplicatedIDCounts > 0)
        {
            strErrorMessage = strDuplicatedIDs;
            return FALSE;
        }
    }

    // Land 좌표 중복 체크
    {
        std::map<std::pair<double, double>, long> coordCounts;

        for (const auto& Land : vecOriginLandData)
        {
            coordCounts[std::make_pair(Land.fOffsetX, Land.fOffsetY)]++;
        }

        CString strDuplicatedLgaCoords(_T("Land coordination duplicated!!\n"));
        long duplicatedLgaCounts = 0;

        for (const auto& pair : coordCounts)
        {
            if (pair.second > 1)
            {
                if (duplicatedLgaCounts > 10)
                {
                    strDuplicatedLgaCoords += _T(".....");
                    break;
                }

                CString str;
                str.Format(_T("X:%.2lf Y:%.2lf Count:%d\n"), pair.first.first, pair.first.second, pair.second);

                strDuplicatedLgaCoords += str;
                duplicatedLgaCounts++;
            }
        }

        if (duplicatedLgaCounts > 0)
        {
            strErrorMessage = strDuplicatedLgaCoords;
            return FALSE;
        }
    }

    return TRUE;
}

BOOL LandCollectionOriginal::Set_MapData(CString strpathName, CString& strErrorMessage)
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
        FX_GROUPID,
        FX_VISIBLE,
        FX_END,
    };

    long LandIndex = 0;
    long columnLink[FX_END];
    long headerColumnCount = 0;
    long rowIndex = 0;

    //{{kircheis_LandShape
    float fOffsetX = 0.f;
    float fOffsetY = 0.f;
    float fWidth = 0.f;
    float fLength = 0.f;
    long nAngle = 0;
    long nLandShapeType = Shape_Base_Rect;
    //}}

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
                        columnName = _T("PinName");
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
                    case FX_GROUPID:
                        columnName = _T("GroupID");
                        break;
                    case FX_VISIBLE:
                        columnName = _T("Visible");
                        break;
                }

                if (columnName.IsEmpty())
                    break;

                if (headerIndexMap.find(columnName) == headerIndexMap.end())
                {
                    if (item == FX_VISIBLE)
                    {
                        m_bTitleVisible = FALSE;
                        continue;
                    }
                    else
                    {
                        strErrorMessage.Format(_T("'%s' Column not found."), LPCTSTR(CString(columnName)));
                        return FALSE;
                    }
                }

                columnLink[item] = headerIndexMap[columnName];

                if (item == FX_VISIBLE)
                {
                    m_bTitleVisible = TRUE;
                }
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
            if (m_bTitleVisible)
            {
                if (dataList[columnLink[FX_VISIBLE]] == CStringA("0") && bisalpha
                    || dataList[columnLink[FX_VISIBLE]] == CStringA("1"))
                    bVisible = FALSE;
                else if (dataList[columnLink[FX_VISIBLE]] == CStringA("X")
                    || dataList[columnLink[FX_VISIBLE]] == CStringA("0") && bisNumber)
                    bVisible = TRUE;
            }
            else
                bVisible = FALSE;

            float fHeight = 0.f;

            if (dataList[columnLink[FX_HEIGHT]] == _T("NULL") || dataList[columnLink[FX_HEIGHT]].IsEmpty())
            {
                fHeight = 0.f * m_fUnit;
            }
            else
                fHeight = static_cast<float>(::atof(dataList[columnLink[FX_HEIGHT]])) * m_fUnit;

            //{{kircheis_LandShape
            fOffsetX = static_cast<float>(::atof(dataList[columnLink[FX_X]])) * m_fUnit;
            fOffsetY = static_cast<float>(::atof(dataList[columnLink[FX_Y]])) * m_fUnit;
            fWidth = static_cast<float>(::atof(dataList[columnLink[FX_WIDTH]])) * m_fUnit;
            fLength = static_cast<float>(::atof(dataList[columnLink[FX_LENGTH]])) * m_fUnit;
            nAngle = (long)::atoi(dataList[columnLink[FX_ANGLE]]);
            nLandShapeType = Shape_Base_Rect;
            if (nAngle == 360 && fWidth == fLength) //kircheis_LandShape
            {
                nLandShapeType = Shape_Base_Circle;

                if (fWidth <= 0.f && fLength <= 0.f)
                    continue;
            }
            //}}

            vecOriginLandData.emplace_back(
                CString(dataList[columnLink[FX_GROUPID]]), CString(dataList[columnLink[FX_NAME]]), fOffsetX, fOffsetY,
                fWidth, fLength, fHeight, nAngle, LandIndex, bVisible,
                nLandShapeType //kircheis_LandShape
            );

            LandIndex++;
        }
    }

    return TRUE;
}

BOOL LandCollectionOriginal::Set_MapData_Shape(CString strpathName, CString& strErrorMessage)
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
        FX_GROUPID,
        FX_VISIBLE,
        FX_SHAPE,
        FX_END,
    };

    long LandIndex = 0;
    long columnLink[FX_END];
    long headerColumnCount = 0;
    long rowIndex = 0;

    //{{kircheis_LandShape
    float fOffsetX = 0.f;
    float fOffsetY = 0.f;
    float fWidth = 0.f;
    float fLength = 0.f;
    long nAngle = 0;
    long nLandShapeType = Shape_Base_Rect;
    //}}

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
                        columnName = _T("PinName");
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
                    case FX_GROUPID:
                        columnName = _T("GroupID");
                        break;
                    case FX_VISIBLE:
                        columnName = _T("Visible");
                        break;
                    case FX_SHAPE:
                        columnName = _T("ShapeType");
                        break;
                }

                if (columnName.IsEmpty())
                    break;

                if (headerIndexMap.find(columnName) == headerIndexMap.end())
                {
                    if (item == FX_VISIBLE)
                    {
                        m_bTitleVisible = FALSE;
                        continue;
                    }
                    else
                    {
                        strErrorMessage.Format(_T("'%s' Column not found."), LPCTSTR(CString(columnName)));
                        return FALSE;
                    }
                }

                columnLink[item] = headerIndexMap[columnName];

                if (item == FX_VISIBLE)
                {
                    m_bTitleVisible = TRUE;
                }
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

            if (m_bTitleVisible)
            {
                if (dataList[columnLink[FX_VISIBLE]] == CStringA("0")
                    || dataList[columnLink[FX_VISIBLE]] == CStringA("1"))
                    bVisible = FALSE;
                else if (dataList[columnLink[FX_VISIBLE]] == CStringA("X")
                    || dataList[columnLink[FX_VISIBLE]] == CStringA("O"))
                    bVisible = TRUE;
            }
            else
                bVisible = FALSE;

            nLandShapeType = Shape_Base_Rect;
            if (dataList[columnLink[FX_SHAPE]] == CStringA("R") || dataList[columnLink[FX_SHAPE]] == CStringA("Rect")
                || dataList[columnLink[FX_SHAPE]] == CStringA("RECT")
                || dataList[columnLink[FX_SHAPE]] == CStringA("rect")) //kircheis_LandShape
                nLandShapeType = Shape_Base_Rect;
            else if (dataList[columnLink[FX_SHAPE]] == CStringA("C")
                || dataList[columnLink[FX_SHAPE]] == CStringA("Circle")
                || dataList[columnLink[FX_SHAPE]] == CStringA("CIRCLE")
                || dataList[columnLink[FX_SHAPE]] == CStringA("circle")) //kircheis_LandShape
                nLandShapeType = Shape_Base_Circle;

            float fHeight = 0.f;

            if (dataList[columnLink[FX_HEIGHT]] == _T("NULL") || dataList[columnLink[FX_HEIGHT]].IsEmpty())
            {
                fHeight = 0.f * m_fUnit;
            }
            else
                fHeight = static_cast<float>(::atof(dataList[columnLink[FX_HEIGHT]])) * m_fUnit;

            //{{kircheis_LandShape
            fOffsetX = static_cast<float>(::atof(dataList[columnLink[FX_X]])) * m_fUnit;
            fOffsetY = static_cast<float>(::atof(dataList[columnLink[FX_Y]])) * m_fUnit;
            fWidth = static_cast<float>(::atof(dataList[columnLink[FX_WIDTH]])) * m_fUnit;
            fLength = static_cast<float>(::atof(dataList[columnLink[FX_LENGTH]])) * m_fUnit;
            nAngle = static_cast<long>(::atoi(dataList[columnLink[FX_ANGLE]]));

            if (nLandShapeType == Shape_Base_Circle) //kircheis_LandShape
            {
                if (fWidth <= 0.f && fLength <= 0.f)
                    continue;
                if (fWidth <= 0.f || fLength <= 0.f)
                    fWidth = fLength = (float)max(fWidth, fLength);
                else if (fWidth != fLength)
                    fWidth = fLength = (float)min(fWidth, fLength);
            }
            //}}

            vecOriginLandData.emplace_back(
                CString(dataList[columnLink[FX_GROUPID]]), CString(dataList[columnLink[FX_NAME]]), fOffsetX, fOffsetY,
                fWidth, fLength, fHeight, nAngle, LandIndex, bVisible,
                nLandShapeType //kircheis_LandShape
            );

            LandIndex++;
        }
    }

    return TRUE;
}

BOOL LandCollectionOriginal::Set_MapData_IgnoreVisible(CString strpathName, CString& strErrorMessage)
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
        FX_GROUPID,
        FX_END,
    };

    long LandIndex = 0;
    long columnLink[FX_END];
    long headerColumnCount = 0;
    long rowIndex = 0;

    //{{kircheis_LandShape
    float fOffsetX = 0.f;
    float fOffsetY = 0.f;
    float fWidth = 0.f;
    float fLength = 0.f;
    long nAngle = 0;
    long nLandShapeType = Shape_Base_Rect;
    //}}

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
                        columnName = _T("PinName");
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
                    case FX_GROUPID:
                        columnName = _T("GroupID");
                        break;
                }

                if (columnName.IsEmpty())
                    break;

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

            float fHeight = 0.f;

            if (dataList[columnLink[FX_HEIGHT]] == _T("NULL") || dataList[columnLink[FX_HEIGHT]].IsEmpty())
            {
                fHeight = 0.f * m_fUnit;
            }
            else
                fHeight = static_cast<float>(::atof(dataList[columnLink[FX_HEIGHT]])) * m_fUnit;

            //{{kircheis_LandShape
            fOffsetX = static_cast<float>(::atof(dataList[columnLink[FX_X]])) * m_fUnit;
            fOffsetY = static_cast<float>(::atof(dataList[columnLink[FX_Y]])) * m_fUnit;
            fWidth = static_cast<float>(::atof(dataList[columnLink[FX_WIDTH]])) * m_fUnit;
            fLength = static_cast<float>(::atof(dataList[columnLink[FX_LENGTH]])) * m_fUnit;
            nAngle = static_cast<long>(::atoi(dataList[columnLink[FX_ANGLE]]));
            nLandShapeType = Shape_Base_Rect;
            if (nAngle == 360 && fWidth == fLength) //kircheis_LandShape
            {
                nLandShapeType = Shape_Base_Circle;

                if (fWidth <= 0.f && fLength <= 0.f)
                    continue;
            }
            //}}

            vecOriginLandData.emplace_back(
                CString(dataList[columnLink[FX_GROUPID]]), CString(dataList[columnLink[FX_NAME]]), fOffsetX, fOffsetY,
                fWidth, fLength, fHeight, nAngle, LandIndex, bVisible,
                nLandShapeType //kircheis_LandShape
            );

            LandIndex++;
        }
    }

    return TRUE;
}

void LandCollectionOriginal::Set_MapData(std::vector<CString>& vecstrToken, long nObjectNumber)
{
    CString strKey;
    CString strData;
    CString strTemp;
    BOOL bDataChanged = FALSE;

    vecOriginLandData.clear();
    vecOriginLandData.resize(nObjectNumber);

    for (long n = 0; n < nObjectNumber; n++)
    {
        vecOriginLandData[n].m_groupID = _T("");
        vecOriginLandData[n].strLandID = _T("");
        vecOriginLandData[n].fOffsetX = 0.f;
        vecOriginLandData[n].fOffsetY = 0.f;
        vecOriginLandData[n].fWidth = 0.f;
        vecOriginLandData[n].fLength = 0.f;
        vecOriginLandData[n].fHeight = 0.f;
        vecOriginLandData[n].nAngle = 0;
        vecOriginLandData[n].bIgnore = FALSE;

        bDataChanged = FALSE;

        // 항목 값 확인
        if (!AfxExtractSubString(strKey, vecstrToken[0], 0, ','))
            break;

        //항목일 경우 Pass
        if (strKey.Find(_T("PinName")) > -1)
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
                    case LandDataType_Name: // LandName
                        vecOriginLandData[n].strLandID = strTemp;
                        bDataChanged = TRUE;
                        break;
                    case LandDataType_CenterX: //Position X
                        vecOriginLandData[n].fOffsetX = (float)_ttof(strTemp) * m_fUnit;
                        ;
                        bDataChanged = TRUE;
                        break;
                    case LandDataType_CenterY: //Position Y
                        vecOriginLandData[n].fOffsetY = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case LandDataType_Width: // Width
                        vecOriginLandData[n].fWidth = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case LandDataType_Length: // Length
                        vecOriginLandData[n].fLength = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case LandDataType_Height: // Heigth
                        if (strTemp == _T("NULL") || strTemp.IsEmpty())
                        {
                            vecOriginLandData[n].fHeight = 0.f * m_fUnit;
                        }
                        else
                            vecOriginLandData[n].fHeight = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case LandDataType_Angle: // Angle
                        vecOriginLandData[n].nAngle = (long)_ttoi(strTemp);
                        bDataChanged = TRUE;
                        break;
                    case LandDataType_GroupID: // GroupID
                        vecOriginLandData[n].m_groupID = strTemp;
                        bDataChanged = TRUE;
                        break;
                    case LandDataType_Visible:
                    {
                        strTemp.MakeUpper();
                        if (m_bTitleVisible)
                        {
                            if (strTemp == "0")
                                vecOriginLandData[n].bIgnore = FALSE;
                            else if (strTemp == "X")
                                vecOriginLandData[n].bIgnore = TRUE;
                        }
                        else
                            vecOriginLandData[n].bIgnore = FALSE;

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

BOOL LandCollectionOriginal::SaveMap(CString strFilePath)
{
    CString m_strPath;
    CStdioFile file;
    CFileException ex;

    CString Data = _T("PinName,Xcoord(mm),Ycoord(mm),Width(mm),Length(mm),Height(mm),Angle(degree),GroupID,Visible,")
                   _T("ShapeType\n"); //kircheis_LandShape
    CString tmpData = _T("");

    for (auto OriginLandInfo : vecOriginLandData)
    {
        CString strIgnore;
        if (OriginLandInfo.bIgnore)
            strIgnore = _T("0");
        else
            strIgnore = _T("1");

        //{{//kircheis_LandShape
        CString strLandShape;
        if (OriginLandInfo.nLandShapeType == Shape_Base_Circle)
            strLandShape = _T("Circle");
        else
            strLandShape = _T("Rect");
        //}}

        tmpData = _T("");
        tmpData.Format(_T("%s,%f,%f,%f,%f,%f,%ld,%s,%s,%s\n"), (LPCTSTR)OriginLandInfo.strLandID,
            OriginLandInfo.fOffsetX,
            OriginLandInfo.fOffsetY, //kircheis_LandShape
            OriginLandInfo.fWidth, OriginLandInfo.fLength, OriginLandInfo.fHeight, OriginLandInfo.nAngle,
            LPCTSTR(OriginLandInfo.m_groupID), LPCTSTR(strIgnore),
            LPCTSTR(strLandShape)); //kircheis_LandShape
        Data += tmpData;
    }

    m_strPath = strFilePath;
    file.Open(m_strPath, CFile::modeCreate | CFile::modeReadWrite, &ex);
    file.WriteString(Data);
    file.Close();

    return TRUE;
}

BOOL LandCollectionOriginal::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 2;
    if (!db[_T("Land Version")].Link(bSave, version))
        version = 0;

    long nSize(0);

    if (!bSave)
    {
        if (!db[_T("LandNumInMap")].Link(bSave, nSize))
            nSize = 0;

        vecOriginLandData.clear();
        vecOriginLandData.resize(nSize);
    }
    else
    {
        nSize = (long)vecOriginLandData.size();

        if (!db[_T("LandNumInMap")].Link(bSave, nSize))
            nSize = 0;
    }

    std::vector<CString> vecStrGroupName(0); //kircheis_MED2.5
    long nGroupNum = 0;

    if (version < 1)
    {
        Pad padMapData;
        CString strData;
        for (long i = 0; i < nSize; i++)
        {
            CString strTemp;
            strTemp.Format(_T("LandDataInMap_LandName_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginLandData[i].strLandID))
                vecOriginLandData[i].strLandID.Format(_T("%d"), i);

            strTemp.Format(_T("LandDataInMap_Xcoord_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginLandData[i].fOffsetX))
                vecOriginLandData[i].fOffsetX = 0.f;

            strTemp.Format(_T("LandDataInMap_Ycoord_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginLandData[i].fOffsetY))
                vecOriginLandData[i].fOffsetY = 0.f;

            strTemp.Format(_T("LandDataInMap_Width_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginLandData[i].fWidth))
                vecOriginLandData[i].fWidth = 0.5f;

            strTemp.Format(_T("LandDataInMap_Length_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginLandData[i].fLength))
                vecOriginLandData[i].fLength = 1.f;

            strTemp.Format(_T("LandDataInMap_Height_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginLandData[i].fHeight))
                vecOriginLandData[i].fHeight = 0.3f;

            strTemp.Format(_T("LandDataInMap_Angle_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginLandData[i].nAngle))
                vecOriginLandData[i].nAngle = 0;

            strTemp.Format(_T("LandDataInMap_Mount_%d"), i);

            long oldGroupID = 0;
            if (!db[strTemp].Link(bSave, oldGroupID))
                oldGroupID = 0;
            vecOriginLandData[i].m_groupID.Format(_T("G%d"), oldGroupID);

            strTemp.Format(_T("LandDataInMap_Visible_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginLandData[i].bIgnore))
                vecOriginLandData[i].bIgnore = FALSE;

            strTemp.Format(_T("LandDataInMap_LandShape_%d"), i); //kircheis_LandShape
            if (!db[strTemp].Link(bSave, vecOriginLandData[i].nLandShapeType))
            {
                vecOriginLandData[i].nLandShapeType = Shape_Base_Rect;
                if (!bSave && vecOriginLandData[i].nAngle == 360
                    && vecOriginLandData[i].fWidth == vecOriginLandData[i].fLength)
                    vecOriginLandData[i].nLandShapeType = Shape_Base_Circle;
            }
            //{{//kircheis_MED2.5
            nGroupNum = (long)vecStrGroupName.size();
            if (nGroupNum == 0)
                vecStrGroupName.push_back(vecOriginLandData[i].m_groupID);
            else
            {
                BOOL bCompared = FALSE;
                for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
                {
                    if (vecStrGroupName[nGroup] == vecOriginLandData[i].m_groupID)
                    {
                        bCompared = TRUE;
                        break;
                    }
                }
                if (bCompared == FALSE)
                    vecStrGroupName.push_back(vecOriginLandData[i].m_groupID);
            }
            //}}
        }
    }
    else
    {
        Pad padMapData;
        CString strData;
        for (long i = 0; i < nSize; i++)
        {
            auto& landDB = db.GetSubDBFmt(_T("Land%d"), i + 1);
            auto& landData = vecOriginLandData[i];

            if (!landDB[_T("Name")].Link(bSave, landData.strLandID))
                landData.strLandID.Format(_T("%d"), i);

            if (!landDB[_T("Xcoord")].Link(bSave, landData.fOffsetX))
                landData.fOffsetX = 0.f;
            if (!landDB[_T("Ycoord")].Link(bSave, landData.fOffsetY))
                landData.fOffsetY = 0.f;
            if (!landDB[_T("Width")].Link(bSave, landData.fWidth))
                landData.fWidth = 0.5f;
            if (!landDB[_T("Length")].Link(bSave, landData.fLength))
                landData.fLength = 1.f;
            if (!landDB[_T("Height")].Link(bSave, landData.fHeight))
                landData.fHeight = 0.3f;
            if (!landDB[_T("Angle")].Link(bSave, landData.nAngle))
                landData.nAngle = 0;

            if (version < 2)
            {
                long oldGroupID = 0;
                if (!landDB[_T("GroupID")].Link(bSave, oldGroupID))
                    oldGroupID = 0;

                landData.m_groupID.Format(_T("G%d"), oldGroupID); //kircheis_MED2.5
            }
            else
            {
                if (!landDB[_T("GroupID")].Link(bSave, landData.m_groupID))
                    landData.m_groupID = _T("0");
                CString strGroupID = landData.m_groupID;
                strGroupID.MakeUpper();
                char chHeader = static_cast<char>(strGroupID.GetAt(0));
                if (chHeader != 'G') //kircheis_MED2.5
                    landData.m_groupID.Insert(0, _T("G"));
            }

            if (!landDB[_T("Ignore")].Link(bSave, landData.bIgnore))
                landData.bIgnore = FALSE;
            if (!landDB[_T("Shape Type")].Link(bSave, landData.nLandShapeType))
            {
                landData.nLandShapeType = Shape_Base_Rect;
                if (!bSave && landData.nAngle == 360 && landData.fWidth == landData.fLength)
                    landData.nLandShapeType = Shape_Base_Circle;
            }
            //{{//kircheis_MED2.5
            nGroupNum = (long)vecStrGroupName.size();
            if (nGroupNum == 0)
                vecStrGroupName.push_back(vecOriginLandData[i].m_groupID);
            else
            {
                BOOL bCompared = FALSE;
                for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
                {
                    if (vecStrGroupName[nGroup] == vecOriginLandData[i].m_groupID)
                    {
                        bCompared = TRUE;
                        break;
                    }
                }
                if (bCompared == FALSE)
                    vecStrGroupName.push_back(vecOriginLandData[i].m_groupID);
            }
            //}}
        }
    }

    //{{//kircheis_MED2.5
    std::sort(vecStrGroupName.begin(), vecStrGroupName.end());
    m_strAllGroupName.Empty();
    nGroupNum = (long)vecStrGroupName.size();
    for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
        m_strAllGroupName += vecStrGroupName[nGroup];
    //}}

    if (!db[_T("LandDataInMap_FlipX")].Link(bSave, m_bLandMapFlipX))
        m_bLandMapFlipX = FALSE;

    if (!db[_T("LandDataInMap_FlipY")].Link(bSave, m_bLandMapFlipY))
        m_bLandMapFlipY = FALSE;

    if (!db[_T("LandDataInMap_RotateIndex")].Link(bSave, m_nRotateIdx))
        m_nRotateIdx = 0;

    if (!db[_T("LandDataInMap_RotateLandIdx")].Link(bSave, m_nRotateLandIdx))
        m_nRotateLandIdx = 0;

    return TRUE;
}

} // namespace Package