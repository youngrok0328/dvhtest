// dA_Base.h : dA_Base DLL의 기본 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.


// CdA_BaseApp
// 이 클래스의 구현을 보려면 dA_Base.cpp를 참조하십시오.
//

class CdA_BaseApp : public CWinApp
{
public:
	CdA_BaseApp();

// 재정의입니다.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	HWND m_hMainView;

	DECLARE_MESSAGE_MAP()
};
