#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class __DPI_SURFACE_API__ Para_MaskOrder
{
public:
    struct MaskInfo
    {
        CString m_strName; // Mask 이름
        int m_makeType; // 0 : Custom ROI, 1 : Legacy ROI
        BOOL m_validate; // TRUE : 지정된 픽셀을 유효화, FALSE : 지정된 픽셀을 무효화
        Ipvm::Point32r2 m_offset_um;
        BOOL m_useBodyEdge; //영훈 [RoundPad_EdgePoint] 20130821 : "Pad Round"만을 위한 변수이나 중에 따로 쓸수도 있겠다

        float m_dilateInUm; // "Mark Measured" Mask에서만 쓰임
            // 이놈은 기본적으로 SurfaceBitmapMask의 속성이긴하나,
            // 매 Inspection 마다 새로 작성하며, 사용자 사용법이 SurfaceMask와 좀 다르다.

        std::vector<Ipvm::Point32r2> m_pointOffsets_um;

        //{{ Detect Rect용 Parameter //kircheis_Hy
        BOOL m_useAutoThreshDR;
        int m_threshDRX;
        int m_threshDRY;
        int m_cuttingWidth;
    };

    Para_MaskOrder();
    ~Para_MaskOrder();

    void Init();
    bool IsValid(LPCTSTR maskName) const;
    long FindIndex(LPCTSTR maskName) const;
    BOOL LinkDataBase(BOOL save, CiDataBase& db);

    void Add(const MaskInfo& maskInfo);
    bool Del(long orderIndex);
    const MaskInfo& GetAt(long orderIndex) const;
    MaskInfo& GetAt(long orderIndex);
    long GetCount() const;

private:
    std::vector<MaskInfo> m_maskInfos;
};
