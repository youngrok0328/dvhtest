#pragma once
// clang-format off
// SDKDDKVer.h를 포함하면 최고 수준의 가용성을 가진 Windows 플랫폼이 정의됩니다.

// 이전 Windows 플랫폼에 대해 응용 프로그램을 빌드하려는 경우에는 SDKDDKVer.h를 포함하기 전에
// WinSDKVer.h를 포함하고 _WIN32_WINNT 매크로를 지원하려는 플랫폼으로 설정하십시오.

#include <WinSDKVer.h>

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#ifdef WINVER
#undef WINVER
#endif

#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define WINVER _WIN32_WINNT_WIN7

#ifndef _WIN7_PLATFORM_UPDATE
#define _WIN7_PLATFORM_UPDATE
#endif

#include <SDKDDKVer.h>

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // 일부 CString 생성자는 명시적으로 선언됩니다.
#endif

#define _AFX_ALL_WARNINGS

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

//#define NOMINMAX // windows.h 에서 min, max 매크로 정의하는 것을 막음

#include <ipvmbasedef.h>

#define FRAMEGRABBER__IGRABXE__99_4_6
#define FRAMEGRABBER__IGRABXQ__1_1_8
#define FRAMEGRABBER__IGRABG2__2_1_3

// clang-format on
