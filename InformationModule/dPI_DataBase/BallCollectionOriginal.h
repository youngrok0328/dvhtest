#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "BallOriginal.h"
#include "BallTypeDefinition.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API BallCollectionOriginal
{
public:
    BallCollectionOriginal();
    BallCollectionOriginal(const long gridCountX, const long gridCountY, const double gridPitchX_um,
        const double gridPitchY_um, const double ballDiameter_um, const double ballHeight_um, const bool bIgnore,
        CString& errorString);
    BallCollectionOriginal(const BallCollectionOriginal& rhs) = default;
    ~BallCollectionOriginal() = default;

    BallCollectionOriginal& operator=(const BallCollectionOriginal& rhs) = default;

    void makeGroupID_usingShape();
    bool updateTypeInfo();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    void Import(LPCTSTR pathName, CString& errorString);
    BOOL Export(LPCTSTR pathName);

public: // Ball Map 파일 로딩에서 결정되는 것.
    std::vector<BallOriginal> m_balls;
    std::vector<BallTypeDefinition> m_ballTypes; // Ball map 에 있는 Ball 의 Diameter 를 큰 녀석부터 내림 차순으로 정렬
    // 이 배열의 인덱스가 PackageSpec_OriginalBall::m_typeIndex 에 사용됨

    CString GetAllGroupName(); //kircheis_MED2.5
    CString m_strAllGroupName; //kirchies_MED2.5

    BOOL m_validBallMap;

public: // 사용자 설정으로 변경되는 것
    // Flip 을 먼저 적용한 후, Rotation 을 적용하는 것을 기준으로 한다.
    BOOL m_ballMapFlipX;
    BOOL m_ballMapFlipY;

    enum class RotationIndex : long
    {
        Deg_0 = 0,
        Deg_90,
        Deg_180,
        Deg_270,
    };

    RotationIndex m_ballMapRotation;
};

} // namespace Package