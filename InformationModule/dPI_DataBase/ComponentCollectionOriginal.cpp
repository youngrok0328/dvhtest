//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ComponentCollectionOriginal.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "Enum.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <fstream>
#include <map>
#include <minwindef.h>
#include <regex>
#include <string>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Package
{
BOOL ComponentCollectionOriginal::LoadMap(CString strFilePath, CString& strErrorMessage)
{
    CFileFind ff;

    vecOriginCompData.clear();
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

    if (vecOriginCompData.size() <= 0)
    {
        strErrorMessage = _T("No Components !!!");
        return FALSE;
    }

    // 아이디 중복 체크
    {
        std::map<CString, long> indexCounts;

        for (const auto& comp : vecOriginCompData)
        {
            indexCounts[comp.strCompName]++;
        }

        CString strDuplicatedIDs(_T("Component ID duplicated!!\n"));
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

    // Component 좌표 중복 체크
    {
        std::map<std::pair<double, double>, long> coordCounts;

        for (const auto& comp : vecOriginCompData)
        {
            coordCounts[std::make_pair(comp.fOffsetX, comp.fOffsetY)]++;
        }

        CString strDuplicatedCompCoords(_T("Component coordination duplicated!!\n"));
        long duplicatedCompCounts = 0;

        for (const auto& pair : coordCounts)
        {
            if (pair.second > 1)
            {
                if (duplicatedCompCounts > 10)
                {
                    strDuplicatedCompCoords += _T(".....");
                    break;
                }

                CString str;
                str.Format(_T("X:%.2lf Y:%.2lf Count:%d\n"), pair.first.first, pair.first.second, pair.second);

                strDuplicatedCompCoords += str;
                duplicatedCompCounts++;
            }
        }

        if (duplicatedCompCounts > 0)
        {
            strErrorMessage = strDuplicatedCompCoords;
            return FALSE;
        }
    }

    return TRUE;
}

BOOL ComponentCollectionOriginal::Set_MapData(CString strpathName, CString& strErrorMessage)
{
    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ifstream infile(strpathName);
    bool headerRead = false;

    char buffer[1000];

    enum fixColumnOld
    {
        FX_NAME_EX,
        FX_X_EX,
        FX_Y_EX,
        FX_TYPE_EX,
        FX_WIDTH_EX,
        FX_LENGTH_EX,
        FX_HEIGHT_EX,
        FX_ANGLE_EX,
        FX_PAD_ID1_EX,
        FX_PAD_ID2_EX,
        FX_VISIBLE_EX,
        FX_END_EX,
    };

    enum fixColumn
    {
        FX_NAME,
        FX_X,
        FX_Y,
        FX_CATEGORY,
        FX_TYPE,
        FX_WIDTH,
        FX_LENGTH,
        FX_HEIGHT,
        FX_ANGLE,
        FX_ELECT_LENGTH,
        FX_ELECT_HEIGHT,
        FX_PAD_GAP,
        FX_PAD_WIDTH,
        FX_PAD_LENGHT,
        FX_VISIBLE,
        FX_END,
    };

    long columnLink[FX_END];
    long headerColumnCount = 0;
    long oldMapDataColumnCount = 11;
    bool isOldMapData = false;
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

            if (headerIndexMap.size() == oldMapDataColumnCount)
            {
                isOldMapData = true;
            }
            // 헤더 정보중 필수 항목들이 모두 들어왔는지 확인한다

            for (long item = 0; item < FX_END; item++)
            {
                CStringA columnName;
                switch (item)
                {
                    case FX_NAME:
                        columnName = _T("CompName");
                        break;
                    case FX_X:
                        columnName = _T("Xcoord(mm)");
                        break;
                    case FX_Y:
                        columnName = _T("Ycoord(mm)");
                        break;
                    case FX_CATEGORY:
                        columnName = _T("CompCategory");
                        break;
                    case FX_TYPE:
                        columnName = _T("CompType");
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
                    case FX_ELECT_LENGTH:
                        columnName = _T("PassiveElectrodeLength(mm)");
                        break;
                    case FX_ELECT_HEIGHT:
                        columnName = _T("PassiveElectrodeHeight(mm)");
                        break;
                    case FX_PAD_GAP:
                        columnName = _T("PassivePadGap(mm)");
                        break;
                    case FX_PAD_WIDTH:
                        columnName = _T("PassivePadWidth(mm)");
                        break;
                    case FX_PAD_LENGHT:
                        columnName = _T("PassivePadLength(mm)");
                        break;
                    case FX_VISIBLE:
                        columnName = _T("Visible");
                        break;
                }

                if (columnName.IsEmpty())
                    break;

                if (headerIndexMap.find(columnName) == headerIndexMap.end())
                {
                    if (item != FX_CATEGORY && item != FX_ELECT_LENGTH && item != FX_ELECT_HEIGHT && item != FX_PAD_GAP
                        && item != FX_PAD_WIDTH && item != FX_PAD_LENGHT)
                    {
                        strErrorMessage.Format(_T("'%s' Column not found."), LPCTSTR(CString(columnName)));
                        return FALSE;
                    }
                }
                if (isOldMapData)
                {
                    if (item != FX_CATEGORY && item != FX_ELECT_LENGTH && item != FX_ELECT_HEIGHT && item != FX_PAD_GAP
                        && item != FX_PAD_WIDTH && item != FX_PAD_LENGHT)
                    {
                        columnLink[item] = headerIndexMap[columnName];
                    }
                }
                else
                {
                    columnLink[item] = headerIndexMap[columnName];
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
                if (dataList.size() == oldMapDataColumnCount)
                {
                    isOldMapData = true;
                }
                else
                {
                    strErrorMessage.Format(_T("The number of columns is different. (%d Line)"), rowIndex);
                    return FALSE;
                }
            }

            BOOL bVisible = FALSE;
            float fHeight = 0.3f * m_fUnit;
            float fAngle = 0.f;
            CString strCategory = _T("CAP");
            CString strType = _T("C1005_03");
            float fWidth = 0.5f * m_fUnit;
            float fLenght = 1.f * m_fUnit;
            float fElectLength = 0.15f * m_fUnit;
            float fElectHeight = fHeight;
            float fPadGap = 0.15f * m_fUnit;
            float fPadWidth = 0.15f * m_fUnit;
            float fPadLenght = 0.15f * m_fUnit;

            if (!isOldMapData)
            {
                if (dataList[columnLink[FX_HEIGHT]] != _T("NULL") && !dataList[columnLink[FX_HEIGHT]].IsEmpty())
                    fHeight = static_cast<float>(::atof(dataList[columnLink[FX_HEIGHT]])) * m_fUnit;

                fAngle = static_cast<float>(::atoi(dataList[columnLink[FX_ANGLE]]));
                if (fAngle == 180.f)
                    fAngle = 0.f;
                else if (fAngle == 270.f)
                    fAngle = 90.f;

                if (dataList[columnLink[FX_CATEGORY]] != _T("NULL") && !dataList[columnLink[FX_CATEGORY]].IsEmpty())
                    strCategory = dataList[columnLink[FX_CATEGORY]];

                if (dataList[columnLink[FX_TYPE]] != _T("NULL") && !dataList[columnLink[FX_TYPE]].IsEmpty())
                    strType = dataList[columnLink[FX_TYPE]];

                if (dataList[columnLink[FX_WIDTH]] != _T("NULL") && !dataList[columnLink[FX_WIDTH]].IsEmpty())
                    fWidth = static_cast<float>(::atof(dataList[columnLink[FX_WIDTH]])) * m_fUnit;

                if (dataList[columnLink[FX_LENGTH]] != _T("NULL") && !dataList[columnLink[FX_LENGTH]].IsEmpty())
                    fLenght = static_cast<float>(::atof(dataList[columnLink[FX_LENGTH]])) * m_fUnit;

                if (dataList[columnLink[FX_ELECT_LENGTH]] != _T("NULL")
                    && !dataList[columnLink[FX_ELECT_LENGTH]].IsEmpty())
                    fElectLength = static_cast<float>(::atof(dataList[columnLink[FX_ELECT_LENGTH]])) * m_fUnit;

                if (dataList[columnLink[FX_ELECT_HEIGHT]] != _T("NULL")
                    && !dataList[columnLink[FX_ELECT_HEIGHT]].IsEmpty())
                    fElectHeight = static_cast<float>(::atof(dataList[columnLink[FX_ELECT_HEIGHT]])) * m_fUnit;

                if (dataList[columnLink[FX_PAD_GAP]] != _T("NULL") && !dataList[columnLink[FX_PAD_GAP]].IsEmpty())
                    fPadGap = static_cast<float>(::atof(dataList[columnLink[FX_PAD_GAP]])) * m_fUnit;

                if (dataList[columnLink[FX_PAD_WIDTH]] != _T("NULL") && !dataList[columnLink[FX_PAD_WIDTH]].IsEmpty())
                    fPadWidth = static_cast<float>(::atof(dataList[columnLink[FX_PAD_WIDTH]])) * m_fUnit;

                if (dataList[columnLink[FX_PAD_LENGHT]] != _T("NULL") && !dataList[columnLink[FX_PAD_LENGHT]].IsEmpty())
                    fPadLenght = static_cast<float>(::atof(dataList[columnLink[FX_PAD_LENGHT]])) * m_fUnit;

                bool bisalpha = isalpha(dataList[columnLink[FX_VISIBLE]].GetAt(0));
                bool bisNumber = isdigit(dataList[columnLink[FX_VISIBLE]].GetAt(0));
                if (dataList[columnLink[FX_VISIBLE]] == CStringA("1")
                    || dataList[columnLink[FX_VISIBLE]] == CStringA("O") && bisalpha)
                    bVisible = FALSE;
                else if (dataList[columnLink[FX_VISIBLE]] == CStringA("0") && bisNumber
                    || dataList[columnLink[FX_VISIBLE]] == CStringA("X"))
                    bVisible = TRUE;
            }
            else
            {
                if (dataList[columnLink[FX_HEIGHT]] != _T("NULL") && !dataList[columnLink[FX_HEIGHT]].IsEmpty())
                    fHeight = static_cast<float>(::atof(dataList[columnLink[FX_HEIGHT]])) * m_fUnit;

                fAngle = static_cast<float>(::atoi(dataList[columnLink[FX_ANGLE]]));
                if (fAngle == 180.f)
                    fAngle = 0.f;
                else if (fAngle == 270.f)
                    fAngle = 90.f;

                if (dataList[columnLink[FX_TYPE]] != _T("NULL") && !dataList[columnLink[FX_TYPE]].IsEmpty())
                    strType = dataList[columnLink[FX_TYPE]];

                if (dataList[columnLink[FX_WIDTH]] != _T("NULL") && !dataList[columnLink[FX_WIDTH]].IsEmpty())
                    fWidth = static_cast<float>(::atof(dataList[columnLink[FX_WIDTH]])) * m_fUnit;

                if (dataList[columnLink[FX_LENGTH]] != _T("NULL") && !dataList[columnLink[FX_LENGTH]].IsEmpty())
                    fLenght = static_cast<float>(::atof(dataList[columnLink[FX_LENGTH]])) * m_fUnit;

                if (dataList[columnLink[FX_VISIBLE]] == CStringA("1")
                    || dataList[columnLink[FX_VISIBLE]] == CStringA("O"))
                    bVisible = FALSE;
                else if (dataList[columnLink[FX_VISIBLE]] == CStringA("0")
                    || dataList[columnLink[FX_VISIBLE]] == CStringA("X"))
                    bVisible = TRUE;
            }

            vecOriginCompData.emplace_back(CString(dataList[columnLink[FX_NAME]]),
                ::atof(dataList[columnLink[FX_X]]) * m_fUnit, ::atof(dataList[columnLink[FX_Y]]) * m_fUnit, strCategory,
                strType, fWidth, fLenght, fHeight, (long)fAngle, fElectLength, fElectHeight, fPadGap, fPadWidth,
                fPadLenght, bVisible);
        }
    }

    return TRUE;
}

BOOL ComponentCollectionOriginal::VerifyFile(CString csvTitle)
{
    BOOL b_result;
    CString strKey;
    CString passiveTitle = _T("CompName,Xcoord(mm),Ycoord(mm),CompType,Width(mm),Length(mm),Height(mm),Angle(degree)");
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

void ComponentCollectionOriginal::Set_MapData(std::vector<CString>& vecstrToken, long nObjectNumber)
{
    CString strKey;
    CString strData;
    CString strTemp;
    BOOL bDataChanged = FALSE;

    vecOriginCompData.clear();
    vecOriginCompData.resize(nObjectNumber);

    for (long n = 0; n < nObjectNumber; n++)
    {
        vecOriginCompData[n].strCompName = _T("");
        vecOriginCompData[n].fOffsetX = 0.f;
        vecOriginCompData[n].fOffsetY = 0.f;
        vecOriginCompData[n].strCompCategory = _T("");
        vecOriginCompData[n].strCompType = _T("");
        vecOriginCompData[n].fWidth = 0.f;
        vecOriginCompData[n].fLength = 0.f;
        vecOriginCompData[n].fHeight = 0.f;
        vecOriginCompData[n].nAngle = 0;
        vecOriginCompData[n].bIgnore = FALSE;
        vecOriginCompData[n].fPassiveElectLenght = 0.f;
        vecOriginCompData[n].fPassiveElectHeight = 0.f;
        vecOriginCompData[n].fPassivePadGap = 0.f;
        vecOriginCompData[n].fPassivePadWidth = 0.f;
        vecOriginCompData[n].fPassivePadLenght = 0.f;
        vecOriginCompData[n].nCompType = 0;

        bDataChanged = FALSE;

        // 항목 값 확인
        if (!AfxExtractSubString(strKey, vecstrToken[0], 0, ','))
            break;

        //항목일 경우 Pass
        if (strKey.Find(_T("CompName")) > -1)
        {
            vecstrToken.erase(vecstrToken.begin());
            n = -1;
            continue;
        }
        else
        {
            long nCount(0);

            for (long nData = 0; nData < MapDataTypeEnd; nData++)
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
                    case MapDataType_CompID: // CompName
                        vecOriginCompData[n].strCompName = strTemp;
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_CenterX: //Position X
                        vecOriginCompData[n].fOffsetX = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_CenterY: //Position Y
                        vecOriginCompData[n].fOffsetY = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_CompCategory:
                        vecOriginCompData[n].strCompCategory = strTemp;
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_CompType: // CompType
                        vecOriginCompData[n].strCompType = strTemp;
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_CompWidth: // Width
                        vecOriginCompData[n].fWidth = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_CompLength: // Length
                        vecOriginCompData[n].fLength = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_CompHeight: // Heigth
                        if (strTemp == _T("NULL") || strTemp.IsEmpty())
                        {
                            vecOriginCompData[n].fHeight = 0.3f * m_fUnit;
                        }
                        else
                            vecOriginCompData[n].fHeight = (float)_ttof(strTemp) * m_fUnit;
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_CompAngle: // Angle
                        vecOriginCompData[n].nAngle = (long)_ttoi(strTemp);
                        if (vecOriginCompData[n].nAngle == 180.f)
                            vecOriginCompData[n].nAngle = 0;
                        else if (vecOriginCompData[n].nAngle == 270.f)
                            vecOriginCompData[n].nAngle = 90;
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_ElectLenght:
                        vecOriginCompData[n].fPassiveElectLenght = static_cast<float>(_ttof(strTemp));
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_ElectHeight:
                        vecOriginCompData[n].fPassiveElectHeight = static_cast<float>(_ttof(strTemp));
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_PassivePadGap:
                        vecOriginCompData[n].fPassivePadGap = static_cast<float>(_ttof(strTemp));
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_PassivePadWidth:
                        vecOriginCompData[n].fPassivePadWidth = static_cast<float>(_ttof(strTemp));
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_PassivePadLenght:
                        vecOriginCompData[n].fPassivePadLenght = static_cast<float>(_ttof(strTemp));
                        bDataChanged = TRUE;
                        break;
                    case MapDataType_Visible:
                    {
                        strTemp.MakeUpper();
                        if (strTemp == "1")
                            vecOriginCompData[n].bIgnore = FALSE;
                        else if (strTemp == "0")
                            vecOriginCompData[n].bIgnore = TRUE;

                        bDataChanged = TRUE;
                        break;
                    }
                }
            }
        }

        if (bDataChanged)
            vecstrToken.erase(vecstrToken.begin());
    }

    CompTypeConv();
}

BOOL ComponentCollectionOriginal::CompTypeConv()
{
    if (vecOriginCompData.size() <= 0)
        return FALSE;

    BOOL hRes = TRUE;

    //std::regex pattern("(^[a-zA-Z]{1,5})[0-9_]*");
    std::regex pattern("(^[a-zA-Z]{1,5})[0-9_]*[0-9a-zA-Z_]*");

    for (auto& OriginCompData : vecOriginCompData)
    {
        //std::string compType = CT2A(OriginCompData.strCompType);
        CT2A asciiStr(OriginCompData.strCompType);
        std::string compType(asciiStr);

        CString strNewCompType = _T("");
        std::smatch m;

        if (std::regex_match(compType, m, pattern))
        {
            std::string strMatch = m[1].str();

            long tmpCalcAngle = 0;
            tmpCalcAngle = OriginCompData.nAngle;

            strNewCompType.Format(_T("%S%s%s_%s_%03d"), strMatch.c_str(),
                (LPCTSTR)CompTypeFloattoString(OriginCompData.fLength),
                (LPCTSTR)CompTypeFloattoString(OriginCompData.fWidth),
                (LPCTSTR)CompTypeFloattoString(OriginCompData.fHeight), tmpCalcAngle);

            OriginCompData.strCompType = strNewCompType;

            if (strMatch == "Die")
            {
                OriginCompData.nCompType = _typeChip;
            }
            else if (strMatch == "C" || strMatch == "CAP" || strMatch == "Capacitor" || strMatch == "R"
                || strMatch == "RES" || strMatch == "Resistor")
            {
                OriginCompData.nCompType = _typePassive;
            }
            else if (strMatch == "A" || strMatch == "Array")
            {
                OriginCompData.nCompType = _typeArray;
            }
            else if (strMatch == "H" || strMatch == "LID" || strMatch == "HeatSync")
            {
                OriginCompData.nCompType = _typeHeatSync;
            }
            else if (strMatch == "P" || strMatch == "Patch" || strMatch == "PATCH") //kircheis_POI
            {
                OriginCompData.nCompType = _typePatch;
            }
        }
        else
        {
            // TypeName이 문자로 시작하지 않으면 FALSE
            ::SimpleMessage(_T("Component type is not match."));
            hRes = FALSE;
            break;
        }
    }

    return hRes;
}

CString ComponentCollectionOriginal::CompTypeFloattoString(float fValue)
{
    long resultVal(0);
    CString strResult = _T("");
    CString tmpVal = _T("");
    tmpVal.Format(_T("%f"), fValue);

    int index = tmpVal.ReverseFind('.');

    if (index == 1)
    {
        resultVal = static_cast<long>(fValue * 10 + .5f);
        if (fValue < 1)
            strResult.Format(_T("0%d"), resultVal);
        else
            strResult.Format(_T("%d"), resultVal);
    }
    else if (index == 2)
    {
        resultVal = static_cast<long>(fValue * 100 + .5f);
        if (fValue < 1)
            strResult.Format(_T("0%d"), resultVal);
        else
            strResult.Format(_T("%d"), resultVal);
    }
    else if (index == 3)
    {
        resultVal = static_cast<long>(fValue * 1000 + .5f);
        if (fValue < 1)
            strResult.Format(_T("0%d"), resultVal);
        else
            strResult.Format(_T("%d"), resultVal);
    }

    return strResult;
}

BOOL ComponentCollectionOriginal::SaveMap(CString strFilePath)
{
    if (vecOriginCompData.size() <= 0)
        return FALSE;

    CString m_strPath;
    CStdioFile file;
    CFileException ex;

    CString Data = _T("CompName,Xcoord(mm),Ycoord(mm),CompCategory,CompType,Width(mm),Length(mm),Height(mm),Angle(")
                   _T("degree),PassiveElectrodeLength(mm),PassiveElectrodeHeight(mm),PassivePadGap(mm),")
                   _T("PassivePadWidth(mm),PassivePadLength(mm),Visible\n");
    CString tmpData = _T("");

    for (auto OriginCompInfo : vecOriginCompData)
    {
        CString strVisible;
        if (OriginCompInfo.bIgnore)
            strVisible = _T("0");
        else
            strVisible = _T("1");

        if (OriginCompInfo.strCompCategory.Find(_T("CAP")) == 0 && OriginCompInfo.strCompType.Find(_T("Die")) == 0)
        {
        }

        tmpData = _T("");
        tmpData.Format(_T("%s,%f,%f,%s,%s,%f,%f,%f,%ld,%f,%f,%f,%f,%f,%s\n"), (LPCTSTR)OriginCompInfo.strCompName,
            OriginCompInfo.fOffsetX, OriginCompInfo.fOffsetY, (LPCTSTR)OriginCompInfo.strCompCategory,
            (LPCTSTR)OriginCompInfo.strCompType, OriginCompInfo.fWidth, OriginCompInfo.fLength, OriginCompInfo.fHeight,
            OriginCompInfo.nAngle, OriginCompInfo.fPassiveElectLenght, OriginCompInfo.fPassiveElectHeight,
            OriginCompInfo.fPassivePadGap, OriginCompInfo.fPassivePadWidth, OriginCompInfo.fPassivePadLenght,
            (LPCTSTR)strVisible);
        Data += tmpData;
    }

    m_strPath = strFilePath;
    file.Open(m_strPath, CFile::modeCreate | CFile::modeReadWrite, &ex);
    file.WriteString(Data);
    file.Close();

    return TRUE;
}

BOOL ComponentCollectionOriginal::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Component Version")].Link(bSave, version))
        version = 0;

    long nSize(0);

    if (!bSave)
    {
        if (!db[_T("CompnentNumInMap")].Link(bSave, nSize))
            nSize = 0;

        vecOriginCompData.clear();
        vecOriginCompData.resize(nSize);
    }
    else
    {
        nSize = (long)vecOriginCompData.size();

        if (!db[_T("CompnentNumInMap")].Link(bSave, nSize))
            nSize = 0;
    }

    if (version < 1)
    {
        for (long i = 0; i < nSize; i++)
        {
            CString strTemp;
            strTemp.Format(_T("CompDataInMap_CompName_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].strCompName))
                vecOriginCompData[i].strCompName.Format(_T("%d"), i);

            strTemp.Format(_T("CompDataInMap_Xcoord_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fOffsetX))
                vecOriginCompData[i].fOffsetX = 0.f;

            strTemp.Format(_T("CompDataInMap_Ycoord_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fOffsetY))
                vecOriginCompData[i].fOffsetY = 0.f;

            strTemp.Format(_T("CompDataInMap_CompType_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].strCompType))
                vecOriginCompData[i].strCompType.Format(_T("C1005_03_000"));

            strTemp.Format(_T("CompDataInMap_CompCategory_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].strCompCategory))
                vecOriginCompData[i].strCompCategory.Format(_T("CAP"));

            if (!bSave)
            {
                CString strCompType = vecOriginCompData[i].strCompType;
                strCompType.MakeUpper();
                if (strCompType.Find(_T("LID")) == 0 || strCompType.Find(_T("HEATSYNC")) == 0)
                    vecOriginCompData[i].strCompCategory.Format(_T("LID"));
                else if (strCompType.Find(_T("DIE")) == 0)
                    vecOriginCompData[i].strCompCategory.Format(_T("DIE"));
            }

            strTemp.Format(_T("CompDataInMap_Width_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fWidth))
                vecOriginCompData[i].fWidth = 0.5f;

            strTemp.Format(_T("CompDataInMap_Length_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fLength))
                vecOriginCompData[i].fLength = 1.f;

            strTemp.Format(_T("CompDataInMap_Height_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fHeight))
                vecOriginCompData[i].fHeight = 0.3f;

            strTemp.Format(_T("CompDataInMap_Angle_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].nAngle))
                vecOriginCompData[i].nAngle = 0;

            strTemp.Format(_T("CompDataInMap_PassiveElectLenght_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fPassiveElectLenght))
                vecOriginCompData[i].fPassiveElectLenght = 0.08f;

            strTemp.Format(_T("CompDataInMap_PassiveElectHeight_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fPassiveElectHeight))
                vecOriginCompData[i].fPassiveElectHeight = vecOriginCompData[i].fHeight;

            strTemp.Format(_T("CompDataInMap_PassivePadGap_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fPassivePadGap))
                vecOriginCompData[i].fPassivePadGap = 0.15f;

            strTemp.Format(_T("CompDataInMap_PAssivePadWidth_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fPassivePadWidth))
                vecOriginCompData[i].fPassivePadWidth = 0.15f;

            strTemp.Format(_T("CompDataInMap_PassivePadLenght_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].fPassivePadLenght))
                vecOriginCompData[i].fPassivePadLenght = 0.15f;

            strTemp.Format(_T("CompDataInMap_Visible_%d"), i);
            if (!db[strTemp].Link(bSave, vecOriginCompData[i].bIgnore))
                vecOriginCompData[i].bIgnore = FALSE;
        }
    }
    else
    {
        // Component 별로 iDatabase를 분리하여 저장 (단순 정리 의도)
        for (long i = 0; i < nSize; i++)
        {
            auto& comDB = db.GetSubDBFmt(_T("Com%d"), i + 1);
            auto& comData = vecOriginCompData[i];

            if (!comDB[_T("Name")].Link(bSave, comData.strCompName))
                comData.strCompName.Format(_T("%d"), i);

            if (!comDB[_T("Xcoord")].Link(bSave, comData.fOffsetX))
                comData.fOffsetX = 0.f;
            if (!comDB[_T("Ycoord")].Link(bSave, comData.fOffsetY))
                comData.fOffsetY = 0.f;
            if (!comDB[_T("Category")].Link(bSave, comData.strCompCategory))
                comData.strCompCategory.Format(_T("CAP"));
            if (!comDB[_T("Type")].Link(bSave, comData.strCompType))
                comData.strCompType.Format(_T("C1005_03_000"));

            if (!bSave)
            {
                CString strCompType = comData.strCompType;
                strCompType.MakeUpper();
                if (strCompType.Find(_T("LID")) == 0 || strCompType.Find(_T("HEATSYNC")) == 0)
                    comData.strCompCategory.Format(_T("LID"));
                else if (strCompType.Find(_T("DIE")) == 0)
                    comData.strCompCategory.Format(_T("DIE"));
            }

            if (!comDB[_T("Width")].Link(bSave, comData.fWidth))
                comData.fWidth = 0.5f;
            if (!comDB[_T("Length")].Link(bSave, comData.fLength))
                comData.fLength = 1.f;
            if (!comDB[_T("Height")].Link(bSave, comData.fHeight))
                comData.fHeight = 0.3f;
            if (!comDB[_T("Angle")].Link(bSave, comData.nAngle))
                comData.nAngle = 0;
            if (!comDB[_T("PassiveElectLenght")].Link(bSave, comData.fPassiveElectLenght))
                comData.fPassiveElectLenght = 0.15f;
            if (!comDB[_T("PassiveElectHeight")].Link(bSave, comData.fPassiveElectHeight))
                comData.fPassiveElectHeight = comData.fHeight;
            if (!comDB[_T("PassivePadGap")].Link(bSave, comData.fPassivePadGap))
                comData.fPassivePadGap = 0.15f;
            if (!comDB[_T("PassivePadWidth")].Link(bSave, comData.fPassivePadWidth))
                comData.fPassivePadWidth = 0.15f;
            if (!comDB[_T("PassivePadLength")].Link(bSave, comData.fPassivePadLenght))
                comData.fPassivePadLenght = 0.15f;
            if (!comDB[_T("Ignore")].Link(bSave, comData.bIgnore))
                comData.bIgnore = FALSE;
        }
    }

    if (!db[_T("CompDataInMap_FlipX")].Link(bSave, m_bCompMapFlipX))
        m_bCompMapFlipX = FALSE;

    if (!db[_T("CompDataInMap_FlipY")].Link(bSave, m_bCompMapFlipY))
        m_bCompMapFlipX = FALSE;

    if (!db[_T("CompDataInMap_RotateIndex")].Link(bSave, m_nRotateIdx))
        m_nRotateIdx = 0;

    if (!db[_T("CompDataInMap_RotateCompIndex")].Link(bSave, m_nRotateCompIdx))
        m_nRotateIdx = 0;

    return TRUE;
}

BOOL ComponentCollectionOriginal::CheckIntegrityOfCompMapData(
    long& o_nErrorNum, long& o_nTotalNum) //kircheis_Comp3DHeightBug
{
    o_nErrorNum = o_nTotalNum = 0;

    long nSize = (long)vecOriginCompData.size();
    if (nSize <= 0)
        return true;

    o_nTotalNum = nSize;
    float fCheckSum = 0.f;

    for (long nComp = 0; nComp < nSize; nComp++)
    {
        if (vecOriginCompData[nComp].bIgnore)
            continue;
        if (vecOriginCompData[nComp].GetCompType() != _typePassive)
            continue;

        fCheckSum
            = vecOriginCompData[nComp].fWidth * vecOriginCompData[nComp].fLength * vecOriginCompData[nComp].fHeight;
        if (fCheckSum <= 0.f)
            o_nErrorNum++;
    }

    return (o_nErrorNum == 0);
}
} // namespace Package
