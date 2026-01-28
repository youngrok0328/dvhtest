#pragma once
#define IPIS500I_DEVELOPER_VERSION_BUILD_NUMBER 99999

//###############################################################
// Versioning Rule : major.minor.revision.build
//###############################################################

// MAJOR : 대규모의 변경이나 기능 추가
// MINOR : 소규모의 변경이나 기능 추가
//         뒷 두자리 : MINOR
//		   나머지 앞자리 : HOST 와의 통신 버전
//         
// REVISION : 버그 수정 수준
// BUILD : 버전업 1회시 무조건 1 증가
//			IPIS500I_DEVELOPER_VERSION_BUILD_NUMBER : 개발버전
//
//			* 정식 RELEASE 시 JOB 이 변경되면 JOB VERSION을 BUILD_NUM 로 하기로 하였으나
//			  개발 버전에서 JOB이 수정된 경우에는 BUILD_NUMBER 가 아직 결정되지 않았으므로 넣을 수 없다
//			  이렇게 개발버전 중에 JOB이 수정되면 JOB VERSION 을 IPIS500I_DEVELOPER_VERSION_BUILD_NUMBER로
//			  해 놓았다가 정식 RELEASE 때는 빌드 당시 BUILD_NUMBER를 JOB VERSION으로 바꾸어 준다.
//
//			  정식 RELEASE 가 나오면 혹 저장해서 쓰던 개발버전의 JOB 은 사용할 수 없겠지만 어쩔 수 없을 것 같다. *
//
// * MINOR 에 Host 통신 버전이 추가되었으므로 주의하자.
//   MINOR 버전은 0~99까지만 사용 *

#define IPIS500I_MAJOR_VERSION	1
#define IPIS500I_MINOR_VERSION	313
#define IPIS500I_REVISION		805
#define IPIS500I_BUILD_NUMBER	221

#define IPIS500I_MACRO_COMPANY					"IntekPlus"
#define	IPIS500I_MACRO_PRODUCT					"iPIS-500I"
#define IPIS500I_MACRO_VERSION_STR_EXPAND(tok)	#tok
#define IPIS500I_MACRO_VERSION_STR(tok)			IPIS500I_MACRO_VERSION_STR_EXPAND(tok)
#define IPIS500I_MACRO_VERSION					IPIS500I_MACRO_VERSION_STR(IPIS500I_MAJOR_VERSION) "." IPIS500I_MACRO_VERSION_STR(IPIS500I_MINOR_VERSION) "." IPIS500I_MACRO_VERSION_STR(IPIS500I_REVISION) "." IPIS500I_MACRO_VERSION_STR(IPIS500I_BUILD_NUMBER)

#define IPIS500I_MACRO_STRVERSION1				IPIS500I_MACRO_VERSION_STR(IPIS500I_MAJOR_VERSION) "." IPIS500I_MACRO_VERSION_STR(IPIS500I_MINOR_VERSION) "." IPIS500I_MACRO_VERSION_STR(IPIS500I_REVISION) "." IPIS500I_MACRO_VERSION_STR(IPIS500I_BUILD_NUMBER) "\0"
#define IPIS500I_MACRO_STRVERSION2				IPIS500I_MAJOR_VERSION,IPIS500I_MINOR_VERSION,IPIS500I_REVISION,IPIS500I_BUILD_NUMBER
#define IPIS500I_MACRO_COMPANY_NAME_STR			"(c) IntekPlus"
#define IPIS500I_MACRO_LEGAL_COPYRIGHT_STR		"Copyright (C) 2017 IntekPlus Corporation"
#define IPIS500I_MACRO_PRODUCT_NAME_STR			"IntekPlus iPIS-500I"

