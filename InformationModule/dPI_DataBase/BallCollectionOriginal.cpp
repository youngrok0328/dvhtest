//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BallCollectionOriginal.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>
#include <fstream>
#include <map>
#include <set>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Package
{
BallCollectionOriginal::BallCollectionOriginal()
    : m_validBallMap(FALSE)
    , m_ballMapFlipX(FALSE)
    , m_ballMapFlipY(FALSE)
    , m_ballMapRotation(RotationIndex::Deg_0)
{
}

BallCollectionOriginal::BallCollectionOriginal(const long gridCountX, const long gridCountY, const double gridPitchX_um,
    const double gridPitchY_um, const double ballDiameter_um, const double ballHeight_um, const bool bIgnore,
    CString& errorString)
    : m_validBallMap(FALSE)
    , m_ballMapFlipX(FALSE)
    , m_ballMapFlipY(FALSE)
    , m_ballMapRotation(RotationIndex::Deg_0)
{
    UNREFERENCED_PARAMETER(errorString);

    const char rowIndexChar[]
        = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'R', 'T', 'U', 'V', 'W', 'Z'};
    const long rowIndexCharCount = sizeof(rowIndexChar) / sizeof(char);

    std::vector<CStringA> columnIndices;

    for (long x = 0; x < gridCountX; x++)
    {
        CStringA str;
        str.Format("%d", x + 1);
        columnIndices.push_back(str);
    }

    std::vector<CStringA> rowIndices;

    for (long y = 0; y < gridCountY; y++)
    {
        long temp = y;

        CStringA str(rowIndexChar[temp % rowIndexCharCount]);

        temp /= rowIndexCharCount;

        while (temp)
        {
            str.Insert(0, rowIndexChar[(temp - 1) % rowIndexCharCount]);

            temp /= rowIndexCharCount;
        }

        rowIndices.push_back(str);
    }

    const double centerOffsetX = 0.5f * (gridCountX - 1) * gridPitchX_um;
    const double centerOffsetY = 0.5f * (gridCountY - 1) * gridPitchY_um;

    long ballIndex = 0;

    for (long y = 0; y < gridCountY; y++)
    {
        for (long x = 0; x < gridCountX; x++)
        {
            m_balls.emplace_back(_T(""), CString(rowIndices[y] + columnIndices[x]), ballIndex,
                x * gridPitchX_um - centerOffsetX, y * gridPitchY_um - centerOffsetY, ballDiameter_um, ballHeight_um,
                bIgnore);

            ballIndex++;
        }
    }

    // Diameter 리스트를 만들고, 어떤 Ball 이 어떤 Diameter Index 를 가지는지 정리한다.
    makeGroupID_usingShape();
    updateTypeInfo();

    m_validBallMap = true;
}

void BallCollectionOriginal::Import(LPCTSTR pathName, CString& errorString)
{
    m_validBallMap = FALSE;
    m_ballMapFlipX = FALSE;
    m_ballMapFlipY = FALSE;
    m_ballMapRotation = RotationIndex::Deg_0;

    Ipvm::AsyncProgress progress(_T("Load ball map file"));

    CFile file;

    if (!file.Open(pathName, CFile::modeRead))
    {
        errorString = _T("Cannot open file !!!");
        return;
    }

    if (file.GetLength() < 10)
    {
        errorString = _T("File length is too short !!!");
        return;
    }

    // Check Encoding
    char encodingCharector[10];
    file.Read(encodingCharector, 10);
    file.Close();

    // Read
    bool isWindowsUnicode = (encodingCharector[0] == -1 && encodingCharector[1] == -2) // BOM Check
        && !(encodingCharector[3] | encodingCharector[5] | encodingCharector[7]
            | encodingCharector[9]); // 영문자, 숫자, 쉼표만 있으므로 0 이어야 한다.

    if (isWindowsUnicode)
    {
        errorString = _T("Invalid file encoding");
        return;
    }

    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ifstream infile(pathName);
    bool headerRead = false;

    char buffer[1000];

    enum fixColumn
    {
        FX_NAME,
        FX_X,
        FX_Y,
        FX_DIAMETER,
        FX_HEIGHT,
        FX_GROUPID,
        FX_VISIBLE,
        FX_END,
    };

    long ballIndex = 0;
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
                    text.SetString(buffer + findStart, long(temp.length() - findStart));
                    headerIndexMap[text] = index;
                    break;
                }
                else
                {
                    text.SetString(buffer + findStart, long(findEnd - findStart));
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
                    case FX_DIAMETER:
                        columnName = _T("Diameter");
                        break;
                    case FX_HEIGHT:
                        columnName = _T("Height");
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
                    errorString.Format(_T("'%s' Column not found."), LPCTSTR(CString(columnName)));
                    return;
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
                text.SetString(buffer + findStart, long(temp.length() - findStart));
                dataList.push_back(text);
                break;
            }
            else
            {
                text.SetString(buffer + findStart, long(findEnd - findStart));
                dataList.push_back(text);
                findStart = findEnd + 1;
            }
        }

        if (dataList.size())
        {
            if (dataList.size() != headerColumnCount)
            {
                // 헤더 컬럼과 데이터 컴럼의 수가 다르다
                errorString.Format(_T("The number of columns is different. (%d Line)"), rowIndex);
                return;
            }

            BOOL bVisible = FALSE;
            bool bisalpha = isalpha(dataList[columnLink[FX_VISIBLE]].GetAt(0));
            bool bisNumber = isdigit(dataList[columnLink[FX_VISIBLE]].GetAt(0));
            if (dataList[columnLink[FX_VISIBLE]] == CStringA("0") && bisalpha
                || dataList[columnLink[FX_VISIBLE]] == CStringA("1"))
                bVisible = FALSE;
            else if (dataList[columnLink[FX_VISIBLE]] == CStringA("X")
                || dataList[columnLink[FX_VISIBLE]] == CStringA("0") && bisNumber)
                bVisible = TRUE;

            m_balls.emplace_back(CString(dataList[columnLink[FX_GROUPID]]), CString(dataList[columnLink[FX_NAME]]),
                ballIndex, ::atof(dataList[columnLink[FX_X]]) * 1000., ::atof(dataList[columnLink[FX_Y]]) * 1000.,
                ::atof(dataList[columnLink[FX_DIAMETER]]) * 1000., ::atof(dataList[columnLink[FX_HEIGHT]]) * 1000.,
                bVisible);

            ballIndex++;
        }
    }

    if (m_balls.size() <= 0)
    {
        errorString = _T("No balls !!!");
        return;
    }

    progress.Post(_T("Consistency check"));

    // Diameter 종류 체크
    std::set<float> diameters;

    for (const auto& ball : m_balls)
    {
        diameters.insert(static_cast<float>(ball.m_diameter_um));
    }

    if (diameters.size() > 10)
    {
        errorString = _T("Too many diameters !!!");
        return;
    }

    // 아이디 중복 체크
    {
        std::map<CString, long> indexCounts;

        for (const auto& ball : m_balls)
        {
            indexCounts[ball.m_name]++;
        }

        CString strDuplicatedIDs(_T("Ball ID duplicated!!\n\n"));
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
                str.Format(_T("G%s\n"), LPCTSTR(pair.first));

                strDuplicatedIDs += str;
                duplicatedIDCounts++;
            }
        }

        if (duplicatedIDCounts > 0)
        {
            errorString = strDuplicatedIDs;
            return;
        }
    }

    // 볼 좌표 중복 체크
    {
        std::map<std::pair<double, double>, long> coordCounts;

        for (const auto& ball : m_balls)
        {
            coordCounts[std::make_pair(ball.m_posX_um, ball.m_posY_um)]++;
        }

        CString strDuplicatedBallCoords(_T("Ball coordination duplicated!!\n\n"));
        long duplicatedBallCounts = 0;

        for (const auto& pair : coordCounts)
        {
            if (pair.second > 1)
            {
                if (duplicatedBallCounts > 10)
                {
                    strDuplicatedBallCoords += _T(".....");
                    break;
                }

                CString str;
                str.Format(_T("X:%.2lf Y:%.2lf Count:%d\n"), pair.first.first, pair.first.second, pair.second);

                strDuplicatedBallCoords += str;
                duplicatedBallCounts++;
            }
        }

        if (duplicatedBallCounts > 0)
        {
            errorString = strDuplicatedBallCoords;
            return;
        }
    }

    progress.Post(_T("Post processing"));

    // Diameter 리스트를 만들고, 어떤 Ball 이 어떤 Diameter Index 를 가지는지 정리한다.
    if (!updateTypeInfo())
    {
        errorString = _T("Balls in the same group have different shapes.");
        return;
    }

    m_validBallMap = true;

    return;
}

void BallCollectionOriginal::makeGroupID_usingShape()
{
    struct GroupInfo
    {
        double m_diameter_um;
        double m_height_um;

        GroupInfo(double diameter, double height)
            : m_diameter_um(diameter)
            , m_height_um(height)
        {
        }
    };

    std::vector<GroupInfo> ballGroup;

    // Diameter Index 생성
    for (const auto& ball : m_balls)
    {
        bool found = false;

        for (long j = 0; j < (long)ballGroup.size(); j++)
        {
            if (ball.m_diameter_um == ballGroup[j].m_diameter_um && ball.m_height_um == ballGroup[j].m_height_um)
            {
                found = true;
                break;
            }
        }

        if (found == false)
        {
            ballGroup.emplace_back(ball.m_diameter_um, ball.m_height_um);
        }
    }

    // 큰 녀석 기준으로 정렬
    //kk (우선순위 Diameter -> Height)
    std::sort(ballGroup.begin(), ballGroup.end(),
        [](const GroupInfo& lhs, const GroupInfo& rhs)
        {
            if (lhs.m_diameter_um != rhs.m_diameter_um)
            {
                return lhs.m_diameter_um > rhs.m_diameter_um;
            }
            else if (lhs.m_height_um != rhs.m_height_um)
            {
                return lhs.m_height_um > rhs.m_height_um;
            }

            return false;
        });

    // 각 범프 정보에 Diameter Index 를 넣어준다.
    for (auto& ball : m_balls)
    {
        for (long j = 0; j < (long)ballGroup.size(); j++)
        {
            if (ball.m_diameter_um == ballGroup[j].m_diameter_um && ball.m_height_um == ballGroup[j].m_height_um)
            {
                ball.m_groupID.Format(_T("G%d"), j + 1); //kircheis_MED2.5
                break;
            }
        }
    }
}

bool BallCollectionOriginal::updateTypeInfo()
{
    struct GroupInfo
    {
        CString m_groupID;
        double m_diameter_um;
        double m_height_um;
    };
    std::map<CString, GroupInfo> groupCandidate;

    for (const auto& ball : m_balls)
    {
        groupCandidate[ball.m_groupID].m_groupID = ball.m_groupID;
        groupCandidate[ball.m_groupID].m_diameter_um = ball.m_diameter_um;
        groupCandidate[ball.m_groupID].m_height_um = ball.m_height_um;
    }

    // Group 내에 다른 모양이 존재하지는 않는지 체크

    for (auto& itGroup : groupCandidate)
    {
        const auto& group = itGroup.second;

        for (const auto& ball : m_balls)
        {
            if (ball.m_groupID != group.m_groupID)
                continue;
            if (ball.m_diameter_um != group.m_diameter_um || ball.m_height_um != group.m_height_um)
            {
                return false;
            }
        }
    }

    m_ballTypes.clear();

    for (auto& itGroup : groupCandidate)
    {
        const auto& group = itGroup.second;
        long nBallNum = 0; //kircheis_AIBC
        for (auto& ball : m_balls)
        {
            if (ball.m_groupID != group.m_groupID)
                continue;

            ball.m_typeIndex = long(m_ballTypes.size());
            nBallNum++; //kircheis_AIBC
        }

        m_ballTypes.emplace_back(group.m_groupID, group.m_diameter_um, group.m_height_um, nBallNum); //kircheis_AIBC
    }

    return true;
}

BOOL BallCollectionOriginal::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    // m_balls
    {
        long count = long(m_balls.size());

        db[_T("{7406DD17-E425-4396-9DCD-1203B09E299A}")].Link(bSave, count);

        m_balls.resize(count, BallOriginal(_T(""), _T(""), 0, 0., 0., 0., 0., false));

        for (long n = 0; n < count; n++)
        {
            m_balls[n].LinkDataBase(bSave, db.GetSubDBFmt(_T("{7406DD17-E425-4396-9DCD-1203B09E299A}_%d"), n));
        }
    }

    db[_T("{84158BBE-3969-4584-A93C-7A1D7C101212}")].Link(bSave, m_validBallMap);
    db[_T("{A7307333-D758-4393-80BD-108C37B42C48}")].Link(bSave, m_ballMapFlipX);
    db[_T("{AF4CB709-14EC-494B-B3E8-6C5C3B8CF7AD}")].Link(bSave, m_ballMapFlipY);
    db[_T("{16953E1A-2524-4AD3-9F0C-D67CD55FC7D1}")].Link(bSave, *(long*)&m_ballMapRotation);

    updateTypeInfo();

    m_strAllGroupName = GetAllGroupName(); //kircheis_MED2.5

    return TRUE;
}

BOOL BallCollectionOriginal::Export(LPCTSTR pathName)
{
    CFile file;

    if (!file.Open(pathName, CFile::modeCreate | CFile::modeWrite))
    {
        return FALSE;
    }

    file.Close();

    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ofstream outfile(pathName);

    outfile << "PinName,Xcoord(mm),Ycoord(mm),Diameter,Height,GroupID,Visible" << std::endl;

    for (const auto& ball : m_balls)
    {
        outfile << CStringA(ball.m_name).GetBuffer() << ",";
        outfile << ball.m_posX_um * 0.001 << ",";
        outfile << ball.m_posY_um * 0.001 << ",";
        outfile << ball.m_diameter_um * 0.001 << ",";
        outfile << ball.m_height_um * 0.001 << ",";
        outfile << CStringA(ball.m_groupID).GetBuffer() << ",";

        CString strIgnore;
        if (ball.m_ignored)
            strIgnore = _T("0");
        else
            strIgnore = _T("1");

        outfile << CStringA(strIgnore).GetBuffer() << std::endl;
    }

    return TRUE;
}

CString BallCollectionOriginal::GetAllGroupName() //kircheis_MED2.5
{
    CString strAllGroupName;
    strAllGroupName.Empty();

    for (const auto& group : m_ballTypes)
        strAllGroupName += group.m_groupID;

    return strAllGroupName;
}

} // namespace Package