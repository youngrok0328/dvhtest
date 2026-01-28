#pragma once

// 2007.05.13 RaSTeR
// 본 클래스의 정보는
// 잡이 변경되었을때나, 디바이스 검사중에 서피스를 제외한 나머지 것이 모두 끝났을 경우에만
// 갱신되게 되어 있으므로
// 별다른 시리얼라이즈나, 복사 연산 등은 필요 없음

/**	\brief	마스크 정보를 주고받기 위한 구조체.
	다른 정보와는 어울리지 않으니 SI의 Mask Group지정용으로만 사용 바람.

	- 1가지 타입의 도형 여러개를 가질 수 있다.
	- 현재 지원되는 타입은 Ipvm::Rect32s, CArbitrary, Polygon_32f
	- 각 도형을 지원하기 위해 CPoint를 사용하여 모든 도형을 표현한다.
	- 각 그룹은 해당하는 이름을 가질 수있다.
*/

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
struct PI_RECT;
struct SSurfaceCustomROIItem;

//HDR_6_________________________________ Header body
//
class __DPI_SURFACE_API__ CSurfaceCustomROI
{
public: // Ctor & Dtor
    CSurfaceCustomROI();
    virtual ~CSurfaceCustomROI();

private: // Default Operator
    CSurfaceCustomROI(const CSurfaceCustomROI& Src);
    CSurfaceCustomROI& operator=(const CSurfaceCustomROI& Src);

public:
    void Init();
    void PrepareCustomROI();

    void CustomROI_Delete(LPCTSTR name); // 마스크 그룹 제거
    void CustomROI_DeleteAll(); // 마스크 그룹을 모두 지움
    int CustomROI_GetCount() const; // 마스크 그룹 갯수 얻기
    SSurfaceCustomROIItem* CustomROI_Search(LPCTSTR name); // 마스크 그룹 이름으로 찾기
    const SSurfaceCustomROIItem* CustomROI_Search(LPCTSTR name) const; // 마스크 그룹 이름으로 찾기
    CString CustomROI_GetName(int nID) const; // 마스크 그룹 이름 얻기
    BOOL CustomROI_AddRect(LPCTSTR name, Ipvm::Rect32s* pRectArray, long nShapeNum); // Type : 0
    BOOL CustomROI_AddEllipse(LPCTSTR name, Ipvm::Rect32s* pEllpiseArray, long nShapeNum); // Type : 1
    BOOL CustomROI_SetImage(LPCTSTR name, const Ipvm::Image8u& image); // Type : 2
    BOOL CustomROI_AddPolygon(
        LPCTSTR name, Ipvm::Point32s2* pPolygonArray, long nShapeNum, long nType); // Type : 3, 4, 5, 6 ...
    BOOL CustomROI_AddPolygon(
        LPCTSTR name, std::vector<std::vector<Ipvm::Point32s2>> vecPolygonArray, long nShapeNum, long nType);

private:
    BOOL m_bUseLeadMask; // Lead Mask 를 사용하는 녀석이 있는가

    std::map<CString, SSurfaceCustomROIItem>* m_pMapMaskShape; // < 마스크 도형 그룹
    std::map<CString, SSurfaceCustomROIItem>& m_mapMaskShape;
};