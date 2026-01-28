#pragma once

// Macro function

// REMOVE_FOLDER_FROM_PATHNAME 파일 이름 추출 형식 -> "name.???"
#define		REMOVE_FOLDER_FROM_PATHNAME(x)	( (x).Right( (x).GetLength() - (x).ReverseFind('\\') - 1 ) )
// REMOVE_EXT_FROM_FILENAME 확장자 제거
#define		REMOVE_EXT_FROM_FILENAME(x)		( (x).Left ( (x).ReverseFind('.') ) )
// GET_FILENAME 파일 이름 -> "name"
#define		GET_FILENAME(x)					REMOVE_EXT_FROM_FILENAME(REMOVE_FOLDER_FROM_PATHNAME(x))
// GET_PATHNAME 파일 경로 추출 형식 -> "c:\aaa"
#define		GET_PATHNAME(x)					( (x).Left ( (x).ReverseFind('\\') ) )
