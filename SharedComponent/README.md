# 요약 #
이 저장소는 전사 공용으로 사용하는 라이브러리를 수집, 관리한다.

이 라이브러리의 프로젝트 설정 파일에서는 각종 컴파일/링크 옵션을 부모로부터 상속받게 해 놓았으므로, IntekPlus.Default.v17.props을 사용해야 한다.

[솔루션 및 프로젝트 파일 구성 (VS2022)](https://do-intekplus.atlassian.net/wiki/spaces/SWDEVS/pages/384008232/C#%EC%86%94%EB%A3%A8%EC%85%98-%EB%B0%8F-%ED%94%84%EB%A1%9C%EC%A0%9D%ED%8A%B8-%ED%8C%8C%EC%9D%BC-%EA%B5%AC%EC%84%B1-(VS2022)) 링크를 참조할 것.

### Button ###
MFC GUI에서 사용되는 각종 버튼 컨트롤을 제공하는 DLL 프로젝트

### CrashDumper ###
크래시가 발생했을 때, 덤프 파일을 저장하는 기능을 담당하는 EXE 프로젝트

### CrashHandler ###
크래시가 발생했을 때, 관련 정보를 CrashHandler로 넘기는 일을 담당하는 DLL 프로젝트

### GridCtrl ###
GridCtrl Written by Chris Maunder <chris@codeproject.com>

회사에서 작성된 Cell 확장을 포함

가능하면 Codejock Toolkit에서 제공하는 그리드 컨트롤로 대체할 것

### IntelSpecific ###
인텔(고객사) 전용 특수 알고리즘을 제공하는 DLL 프로젝트

### Label ###
MFC GUI에서 사용되는 각종 레이블 컨트롤을 제공하는 DLL 프로젝트

### LogControl ###
비전마당 로그 함수와 연동해서 사용되는 로그 리스트 MFC GUI 컨트트을 제공하는 DLL 프로젝트

### Persistence ###
설정을 저장하거나, 읽어오는 클래스를 제공하는 DLL 프로젝트

### Serialization ###
CArchive 기반 데이터 직렬화 관련 클래스 제공하는 DLL 프로젝트

### SupportCsv ###
CSV 파일 저장 기능을 제공하는 DLL 프로젝트

### Utility ###
어디에 있기도 애매한 유틸리티 DLL 프로젝트
