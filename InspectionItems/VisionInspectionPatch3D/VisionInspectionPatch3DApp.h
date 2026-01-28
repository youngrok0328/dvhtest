#pragma once

#ifndef __AFXWIN_H__
#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class VisionInspectionPatch3DApp : public CWinApp
{
public:
    VisionInspectionPatch3DApp();

    // 재정의입니다.

public:
    virtual BOOL InitInstance();

    DECLARE_MESSAGE_MAP()
};
