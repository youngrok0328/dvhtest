#pragma once

//// Host-Probe TCP/IP communication
// Odd number ID is for replying command
///////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
/*
    1. 특별한 형식 지정이 없은 parameter는 long임.
    2. P1:Job File(CFile)와 같이 특별한 형식 지정이 있는 parameter는 아래 parameter type list 참조.
    3. P1은 첫번째 파라미터, P2는 두번째 파라미터.....
    4. R: Report message의 default parameter로 0->OK, 그외 숫자->Error Code.
    5. R1은 Report Message의 첫번째 parameter, R2는 두번째 parameter......... 
*/
////////////////////////////////////////////////////////////////////////////////////////////////

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum class enumVirtualVisionIO
{
    Ready,
    Acqusition,
    SlowAcqusition,
    Start,
    ScanID,
    StitchID,
};

enum enumHandlerVisionMsgDefinition : int
{
    // Range
    MSG_PROBE_ID_MIN
        //												   Host <> Probe	Reply

        // - Mode
        ,
    MSG_PROBE_SET_ACCESS_MODE = MSG_PROBE_ID_MIN // 		->			P1:Access Mode                          R:

        // - Job
        ,
    MSG_PROBE_JOB_DOWNLOAD // 		->			P1:Job File(CFile)                      R:
        ,
    MSG_IMAGE_SAVE_OPTION_DOWNLOAD,
    MSG_DEBUG_STOP_OPTION_DOWNLOAD,
    MSG_PROBE_HANDLER_STOP

        /// Lot Info
        ,
    MSG_PROBE_LOTINFO //		->			P1:LotInfo(LPCTSTR)						R:    //kircheis_OCR
        ,
    MSG_PROBE_WEEKLY_CODE,
    MSG_PROBE_MAKER_CODE,
    MSG_PROBE_WEEKCODE_MIX_INFO,
    MSG_PROBE_WEEKLY_CODE2,
    MSG_PROBE_MANUAL_INFO

        // Scanning Info
        ,
    MSG_PROBE_TRAY_SCAN_INFO,
    MSG_PROBE_TRAY_SCAN_INFO_ACK

        // - Status
        ,
    MSG_PROBE_NOT_MAINWND //		<-			Px:										R1
        ,
    MSG_PROBE_EXCUTE_ERROR //		<-			P1:Host Command ID!
        ,
    MSG_PROBE_CAL_FILE_NOT_FOUND //		<-			P1:Zoom Level

        // - Setup
        ,
    MSG_PROBE_START_TEACH //		->			Px:                                     R:
        ,
    MSG_PROBE_TEACH_END //		<-			P1:Job File                             R:
        ,
    MSG_PROBE_START_SETUP //		->			Px:                                     R:
        ,
    MSG_PROBE_SETUP_END //		<-
        ,
    MSG_PROBE_UPDATE_JOB //		<-			P1:Job File                             R:

        ,
    MSG_PROBE_LOT_START //		->			P1:Initial Teach	P2:Reset Mark Teach R:
        ,
    MSG_PROBE_LOT_END //		->			P1:			                            R:
        ,
    MSG_PROBE_TEACH_BEGIN //		<-

        ,
    MSG_PROBE_GRNR_START //		->			P1:			                            R:
        ,
    MSG_PROBE_GRNR_END //		->			P1:			                            R:

        ,
    MSG_PROBE_TEST_INSPECT //		->

        ,
    MSG_PROBE_INLINE_START,
    MSG_PROBE_INLINE_STOP

        ,
    MSG_PROBE_CAL_FILE

        ,
    MSG_PROBE_MARK_TEACH //		->			P1:Option
        //						0 : Nothing
        //						1 : Auto-Testing
        //						2 : Auto-Testing and Verification
        //						3 : Auto-Teaching
        //						4 : Auto-Teaching and Verification
        ,
    MSG_PROBE_MARK_MULTI_TEACH //		->			P1:Option //kircheis_MAT  P2:Skip Number
        //						0 : Nothing
        //						1 : Auto-Testing
        //						2 : Auto-Testing and Verification
        //						3 : Auto-Teaching Fixed Spec
        //						4 : Auto-Teaching Modified Spec
        ,
    MSG_PROBE_MARK_MULTI_TEACH_ACK

        // - Motion
        ,
    MSG_PROBE_HOME //		->			Px:                                     R:
        ,
    MSG_PROBE_STOP //		->			Px:                                     R:
        ,
    MSG_PROBE_HOMEX //		->			Px:                                     R:
        ,
    MSG_PROBE_HOMEY //		->			Px:                                     R:
        ,
    MSG_PROBE_HOMEZ //		->			Px:                                     R:
        ,
    MSG_PROBE_HOMEXY //		->			Px:                                     R:

        // -- Relative Move
        ,
    MSG_PROBE_MOVEX_R //		->			P1:+(-)Distance                         R:
        ,
    MSG_PROBE_MOVEY_R //		->			P1:+(-)Distance                         R:
        ,
    MSG_PROBE_MOVEZ_R //		->			P1:+(-)Distance                         R:
        ,
    MSG_PROBE_MOVEXY_R //		->			P1:+(-)DistanceX, P2:+(-)DistanceY      R:

        // -- Absolute Move
        ,
    MSG_PROBE_MOVEX_A //		->			P1:+(-)Position                         R:
        ,
    MSG_PROBE_MOVEY_A //		->			P1:+(-)Position                         R:
        ,
    MSG_PROBE_MOVEZ_A //		->			P1:+(-)Position                         R:
        ,
    MSG_PROBE_MOVEXY_A //		->			P1:+(-)PositionX, P2:+(-)PositionY      R:

        // - Move to Pocket
        ,
    MSG_PROBE_MOVE_POCKET //		->			P1:Pocket Number                        R:

        ,
    MSG_PROBE_TEACH_SKIP //					P1: Skip Count
        ,
    MSG_PROBE_TEACH_SKIP_MotionEnd //					P1: Skip 동작 완료 했삼.

        // *****- MESSAGE FOR THR DIAGNOSIS 2005-01-05 Song Kyung Soo -*****
        // --- When IPIS   receive this message, It's a QUERY MESSAGE. So No Parameters are nessary.
        // --- When IPACK  receive this message, It's a REPLY MESSAGE. So Parameters are nessary.
        ,
    MSG_PROBE_DIAGNOSIS_MSG_START,
    MSG_PROBE_HANDLER_INIT
        = MSG_PROBE_DIAGNOSIS_MSG_START //		->			P1:	1==INITILIZED,	0==NOTINITILIZED	R:
        ,
    MSG_PROBE_PROBEX_POS //		->			P1: POSITION							R:
        ,
    MSG_PROBE_PROBEX_DONE //		->			P1: 1==DONE, 0==INMOTION				R:
        ,
    MSG_PROBE_PROBEY_POS //		->			P1: POSITION							R:
        ,
    MSG_PROBE_PROBEY_DONE //		->			P1: 1==DONE, 0==INMOTION				R:
        ,
    MSG_PROBE_PROBEZ_POS //		->			P1: POSITION							R:
        ,
    MSG_PROBE_PROBEZ_DONE //		->			P1: 1==DONE, 0==INMOTION				R:
        ,
    MSG_PROBE_PROBEZ_REF_POS //		->			P1: REF_POSITION						R:
        ,
    MSG_PROBE_DIAGNOSIS_MSG_END = MSG_PROBE_PROBEZ_REF_POS

        // - Inspection
        ,
    MSG_PROBE_INSPECT_RESULT //		<-			P1:Inspection Result Structure          R:
        ,
    MSG_PROBE_IMAGE_SEND_1,
    MSG_PROBE_IMAGE_SEND_2,
    MSG_PROBE_IMAGE_SEND_3 // 영훈 - 20111207_ADResult
        //	, MSG_PROBE_GET_IMAGE										//		->													R1: Image Data
        ,
    MSG_PROBE_MEASURE_INFO
        // - Image

        // 영훈 20140103 : Host에서 Position 및 Focus Teaching 이미지를 Live로 획득하여 전송하도록 한다.
        ,
    MSG_PROBE_IMAGE_LIVE //		->			P1:1(On) / 0(Off)                       R:
        ,
    MSG_PROBE_IMAGE_SIZE //		->			P1:SizeX P2:SizeY						R:

        // Golden Block 관련
        ,
    MSG_PROBE_READY_INSP_GOLDEN_BLOCK //		->			Px:                                     R:
        ,
    MSG_PROBE_START_INSP_GOLDEN_BLOCK //		->			Px:                                     R:
        ,
    MSG_PROBE_END_INSP_GOLDEN_BLOCK //		->			Px:                                     R:

        // - Error
        //	, MSG_PROBE_HOST_ERROR										//		->			P1:Error Code                           R:
        ,
    MSG_PROBE_VISION_ERROR_POPUP //		<-			P1:Error Code                           R:

        // ETC
        ,
    MSG_PROBE_CHANGE_SCREEN //					P1:Screen ID
        //						0 : Host
        //						1 : Vision1
        //						2 : Vision2

        ,
    MSG_PROBE_START_PREPARED //		->			Nothing
        // 20071127 RaSTeR
        ,
    MSG_PROBE_CHECK_INLINE_MODE
        //		<-			P1 : 1(TRUE) / 0(FALSE)
        ,
    MSG_PROBE_NOT_USE001,
    MSG_PROBE_NOT_USE002,
    MSG_PROBE_NOT_USE003,
    MSG_PROBE_NOT_USE004,
    MSG_PROBE_NOT_USE005,
    MSG_PROBE_NOT_USE006

        ,
    MSG_PROBE_VERSION_INFO //		Send Probe Software(iPack) Version to iPIS

        ,
    MSG_PROBE_BACKUP_SYSTEM_CONFIG

        //	, MSG_PROBE_ID_MAX = MSG_PROBE_BACKUP_SYSTEM_CONFIG,

        ,
    MSG_PROBE_NOT_USE007,
    MSG_PROBE_NOT_USE008,
    MSG_PROBE_NOT_USE009,
    MSG_PROBE_NOT_USE010,
    MSG_PROBE_VISION_ERROR_LOG,
    MSG_PROBE_JOBTEACH_DISABLE,
    MSG_PROBE_JOBTEACH_ENABLE,
    MSG_PROBE_SYSTEM_UNIT_CHANGED,
    MSG_PROBE_DOWNLOAD_PRINT_OCR,
    MSG_PROBE_ALGOPARAM_DOWNLOAD,
    MSG_PROBE_ALGOSPEC_DOWNLOAD,
    MSG_PROBE_3DVISION_GRAB_MOVE,
    MSG_PROBE_3DVISION_TEST_INSPECT

        ,
    MSG_PROBE_ADRESULT // 영훈 - 20111207_ADResult

        // Vision Auto Cal
        ,
    MSG_PROBE_1VISION_AUTO_CALC,
    MSG_PROBE_1VISION_AUTO_CALC_RESULT,
    MSG_PROBE_2VISION_AUTO_CALC,
    MSG_PROBE_2VISION_AUTO_CALC_RESULT,
    MSG_PROBE_3VISION_AUTO_CALC,
    MSG_PROBE_3VISION_AUTO_CALC_RESULT,
    MSG_PROBE_PARTITIONID,
    MSG_PROBE_GRAB_MOVE

        ,
    MSG_PROBE_GRAB_START

        //  현식 마크티치관련 수정
        // 마크 티치 관련
        ,
    MSG_MARKTEACH_START,
    MSG_MARKTEACH_SKIP,
    MSG_MARKTEACH_DONE,
    MSG_PROBE_IMAGE_FREEZE //		->

        ,
    MSG_PROBE_NOT_USE011

        ,
    MSG_PROBE_REJECT_IMAGE

        ,
    MSG_PROBE_MARK_IMAGE

        ,
    MSG_PROBE_CHECK_READY_STATUS //kircheis_150417

        ,
    MSG_PROBE_JOB_MODIFY_HISTORY //  영훈 20150622 : Job에서 변경된 내용을 호스트로 전송한다.

        ,
    MSG_PROBE_SYSTEM_MODIFY_HISTORY //  영훈 20160122 : System 에서 변경된 내용을 호스트로 전송한다.

        ,
    MSG_PROBE_START_HANDLER //  영훈 20151021 : 호스트에게 스타트 신호를 보낸다.

        ,
    MSG_PROBE_SEPARATION_HARDWARE_JOB //  영훈 20151203 : One Job Parameter 관련으로 모든 job을 분리해주는 메세지. 설비당 한번만 실행된다.

        ,
    MSG_PROBE_RESTART_PROGRAM // 영훈 20151213 : iPack이 하루에 한번씩 다시 시작하도록 해준다.

        ,
    MSG_PROBE_AUTO_MARK_TEACH // H -> V  // P1 : 1(Auto Test) or 2(Auto Teach)  //Auto Mark Teach Not Use시엔 보내면 안 되는 MSG //kircheis_AutoTeach

        ,
    MSG_PROBE_AUTO_MARK_TEACH_RESULT // H <- V  // P1 : 0(Fail-사람이 와서 Teach하라 전해라~~) or 1(성공)

        ,
    MSG_PROBE_SLITBEAM_SCAN_SPEED_MM_PER_SEC //	sizeof(double)

        ,
    MSG_PROBE_IMAGE_SAVE_OPTION //k  //define => 120
        ,
    MSG_PROBE_SEND_SYSTEM_PARAMETERS_TO_HOST // H <-V  //ASNC용으로 Vision System Parameter를 전송한다
        ,
    MSG_PROBE_SEND_PACKAGE_SPEC_TO_HOST // H <- V  //JobDB에 Package Spec만 넣어서 보내기. Package Spec은 Btm2D <-(Host)->Btm3D,  Top2D<-(Host)->Top3D 끼리만 공유하자
        ,
    MSG_PROBE_SEND_PACKAGE_SPEC_TO_VISION // H -> V   // 같은 일을하지만 Vision용 Host용을 나눈다.  [H <-> V] 요런 메세지 만들기 싫어서...
        ,
    MSG_PROBE_STITCH_GRAB_DIRECTION // H->V   // Grab Direction 전달 : P1 : 0 or 1  0-Normal(Btm Vision)   1-Revierse(TopVision)
        ,
    MSG_PROBE_MOVE_TO_GRAB_POS_Z // H <- V   // Multi Grab 시 조명 설정할 떄 원하는 Grab Pos Z를 전달 : P1 : 0 or 1  0-Normal(Low)   1-High

        ,
    MSG_PROBE_SEND_SYSTEM_PARAMETERS_TO_VISION // H -> V  //ASNC 서버에서 받은 Vision System Parameter를 전송한다
        ,
    MSG_PROBE_SEND_SYSTEM_PARAMETERS_MATCH_RESULT_TO_HOST // H <- V  //ASNC 서버에서 받은 Vision System Parameter를 현재 값과 비교한 후 Key Value가 다른 녀석들을 보낸다. ar << (mismatch)VisionSystemParameters;}
        ,
    MSG_PROBE_SEND_SYSTEM_PARAMETERS_TO_VISION_FOR_APPLY // H -> V  //Vision에 적용할 Vision System Parameter를 전송한다
        ,
    MSG_PROBE_SEND_SYSTEM_PARAMETERS_CHANGED // H <- V  //Host가 Vision System Parameter 보내준 걸로 변경 했음을 알린다. P1:1, 0이면 안 바꿨음.
        ,
    MSG_PROBE_SCAN_SPEC // H -> V
        ,
    MSG_PROBE_JOB_DOWNLOAD_ACK // H <- V  //JobDownload가 완료되었는가
        ,
    MSG_PROBE_VISION_INFO //define => 132 //kk					//H -> V //Vision의 정보를 전송한다
        //Top -> 0, BTM -> 1, Side1 -> 2, Side 2 -> 3
        //2D -> 0, 3D -> 10
        //ex> 2D TOP = 0 + 0 = 0, 3D TOP = 10 + 0 = 10
        //kk GrabFail Sequence
        ,
    MSG_PROBE_3D_GRAB_RETRY // H <- V GrabFail시 Host에게 GrabFail이라고 알려준다. (재시도 횟수 1회, 실패 시 넘김)
        ,
    MSG_PROBE_3D_GRAB_RETRY_ACK //define => 134					// H -> V Host에서 신호를 줄 시 다시 Grab을 시작한다.
        ,
    MSG_PROBE_HOST_RECIPE_UPDATED,
    MSG_PROBE_SAMPLE_IMAGE_FILES,
    MSG_PROBE_TRAY_SCAN_SPEC // H -> V

        //{{ //kircheis_NGRV_MSG
        ,
    MSG_PROBE_BEGIN_TYPE_FOR_NGRV,
    MSG_PROBE_VISION_INSPECTION_ITEMS
        = MSG_PROBE_BEGIN_TYPE_FOR_NGRV //V -> H				vector CString :: 각 Vision이 검사하는 검사 Module 이름.Recipe Upload시 송부
        ,
    MSG_PROBE_NGRV_ALL_INSPECTION_LIST //H -> NGRV			vector CString :: Host에 모인 각 Vision의 검사 모듈 이름. 앞에 [Vision Name] 붙인다. Recipe Download 시 송부
        ,
    MSG_PROBE_NGRV_GRAB_Z_POSITION_INFO //NGRV -> H			vector Class NgrvGrabPositionInfo :: IR 조명 사용에 따른 Grab Height 관련 정보를 전달.  NGRV의 Recipe Upload 시 송부
        ,
    MSG_PROBE_NGRV_PACKAGE_INFO_IN_TRAY //H -> NGRV			vector Class NgrvPackageGrabInfo :: Tray의 전체 Package에 대한 정보를 Vision으로 송부. Review 대상 Tray가 NGRV Probe 구간으로 진입 전 송부
        ,
    MSG_PROBE_NGRV_PACKAGE_INFO_IN_TRAY_ACK //NGRV -> H			MSG_PROBE_NGRV_PACKAGE_INFO_IN_TRAY 를 잘 받았어
        ,
    MSG_PROBE_NGRV_WARNING_VISION_NAME //NGRV -> H			vector CString :: 검사 모듈 이름에 Vision Name이 누락된 경우 발생
        ,
    MSG_PROBE_NGRV_WARNING_RECIPE_TYPE //NGRV -> H			NULL :: Download 받은 Recipe가 NGRV 전용 Recipe가 아닐 경우 발생
        ,
    MSG_PROBE_LOT_START_FOR_NGRV //H -> V&NGRV		NULL :: NGRV Mode의 Lot이 시작됨을 알림. MSG_PROBE_LOT_START 대체
        ,
    MSG_PROBE_WARNING_BTM2D_RECIPE_FOR_BYPASS //BTM2D- > H		NULL :: NGRV Lot Start가 수신되었을 때, BTM2D가 NGRV Recipe가 아님을 알림
        ,
    MSG_PROBE_JOB_DOWNLOAD_FOR_BYPASS //H -> BTM2D		Recipe :: BTM2D에 NGRV용 Recipe를 Download할 때 쓰자. MSG_PROBE_JOB_DOWNLOAD 대체
        ,
    MSG_PROBE_UPDATE_JOB_FOR_BYPASS //BTM2D -> H		Recipe :: BTM2D에서 NGRV용 Recipe를 Upload할 때 쓰자. MSG_PROBE_UPDATE_JOB 대체
        ,
    MSG_PROBE_TEACH_END_FOR_BYPASS //BTM2D -> H		Recipe :: BTM2D에서 NGRV용 Recipe를 Upload할 때 쓰자. MSG_PROBE_TEACH_END 대체
        ,
    MSG_PROBE_NGRV_COMPLETE_SAVE_IMG //NGRV -> H			Class NgrvGrabbedImageInfo :: 하나의 Defect에 대한 영상 저장이 완료되었음을 알림.
        ,
    MSG_PROBE_NGRV_GRAB_FRAME_INFO //NGRV -> H			Class NgrvFrameInfo :: 각 Frame에서 획득하는 검사 모듈에 대한 정보를 전송..Recipe Upload시 송부
        ,
    MSG_PROBE_EACH_VISION_NAME_FOR_NGRV //H -> NGRV			vector CString :: 각 Vision의 이름.
        ,
    MSG_PROBE_NGRV_VISION_SINGLE_GRAB //H -> NGRV    영상 1장 Grab 후 Handler로 전송
        ,
    MSG_PROBE_AF_PLANE_REF_INFO //2D Vision -> H	Class NgrvAfRefInfo		Align UI에서 지정한 Plane AF의 관련 정보를 Recipe Upload 시 송부
        ,
    MSG_PROBE_NGRV_AF_PLANE_REF_INFO //H -> NGRV			Class NgrvAfRefInfo		2D Vision에서 받은 Plane Ref 관련 정보를 Recipe Download시 송부
        ,
    //{{ NGRV SingleRun 관련 MSG
    MSG_PROBE_NGRV_RUN_METHOD_INFO // H -> NGRV             BOOL                            NGRV가 Normal Run(FALSE)인지 Single Run(TRUE)인지 알려줌 : Lot Start시 선택 후 송부
        ,
    MSG_PROBE_NGRV_RUN_METHOD_INFO_ACK // NGRV -> H                                        NGRV Run method 확인 완료
        ,
    MSG_PROBE_NGRV_SINGLE_RUN_VISION_TYPE_INFO // H -> NGRV    BOOL                            NGRV Single Run일 때, Grab해야하는 Vision Type이 Bottom(TRUE)인지 Top(FALSE)인지 알려줌
        ,
    MSG_PROBE_NGRV_SINGLE_RUN_VISION_TYPE_INFO_ACK // NGRV -> H                                NGRV Single Run Vision Type 확인 완료
        ,
    MSG_PROBE_NGRV_SINGLE_RUN_INFO // NGRV -> H		            Class 	NgrvSingleRunInfo      Recipe download 및 Open 후에 송부
        ,
    MSG_PROBE_NGRV_SINGLE_RUN_INFO_ACK // H -> NGRV		        Class 	NgrvSingleRunInfo      SingleRunInfo 잘 받았음을 알림
        ,
    MSG_PROBE_SEND_2D_VISION_IMAGE_INFO // 2D Vision -> H		Class C2DImageInfo		        2D Vision에서 영상을 전송할 때 영상 정보를 함께 전송
        ,
    MSG_PROBE_NGRV_SEND_2D_VISION_INFO // H -> NGRV		        Class C2DImageInfo		        2D Vision에서 받은 정보를 NGRV로 전송
        ,
    MSG_PROBE_NGRV_SEND_2D_VISION_INFO_ACK // NGRV -> H                                         NGRV 에서의 2D Vision 정보 체크 및 성공 여부 전송
        ,
    MSG_PROBE_SEND_2D_VISION_IMAGE_INFO_ACK // H -> 2D Vision	Class C2DImageInfo		        NGRV에서 받은 정보에 대한 최종 성공 여부 전송
        ,
    MSG_PROBE_NGRV_SEND_GRAB_RESULT // NGRV -> H		        Class CNGRVGrabResult	    	NGRV에서 Grab 결과를 전송, One Package 기준
        ,
    MSG_PROBE_NGRV_SEND_GRAB_RESULT_ACK // H -> NGRV		                        	    	NGRV에서 받은 Grab 결과 잘 받았음을 확인
        ,
    MSG_PROBE_NGRV_SEND_MOVE_SELECTED_POSITION // NGRV -> H                                     NGRV Single Run에서 "Move" 버튼 클릭 시 이동 해줘야 하는 명령 전송
        //}}
        ,
    MSG_PROBE_END_TYPE_FOR_NGRV,
    MSG_PROBE_SYSTEM_OPTION //H -> NGRV, NGRV Save Image format 데이터 정보
        //}}

        //{{//kircheis_TxtRecipe
        ,
    MSG_PROBE_REQUEST_TEXT_RECIPE // H -> V //Text화된 Recipe를 보내줄 것을 요청. 파라미터 없음
        ,
    MSG_PROBE_SEND_TEXT_RECIPE // V -> H //Text화된 Recipe를 전송. std::vector<CString>
        //}}

        ,
    MSG_PROBE_SET_TIME_SYNCHRONIZE // H -> V	SYSTEMTIME  ::  Host의 시간을 Vision으로 전송
        ,
    MSG_PROBE_REQUEST_VISION_CONDITION // H -> V	//Vision의 상태를 회신할 것을 요청
        ,
    MSG_PROBE_REQUEST_VISION_CONDITION_ACK // V -> H	//long : Host에 현재 Vision상태가 죽었음을 알려준다
        ,
    MSG_PROBE_SEND_IGRAB_BOARD_TEMPERATURE // V -> H	//float : Host에 F/G 온도를 전송
        ,
    MSG_PROBE_REQUEST_VISION_LOG // H -> V	//Vision 관련 Log를 요청
        ,
    MSG_PROBE_REQUEST_VISION_LOG_ACK // V -> H	//Vision 관련 Log를 Host에 Archive 형태로 전송
        ,
    MSG_PROBE_REQUEST_3D_VISION_START_IO_CHECK // V -> H	//V.Start I/O가 Off되지 않았을 경우 Handler에 확인 요청을 한다

        ,
    MSG_PROBE_TYPE_NUM
};

////////////////////////////////////////////////////////////
/* Parameter Structure List*/
///////////////////////////////////////////////////////////
//{{ Not Excute Menu  P1 : 해당 커맨드  P2 : 원인.
#define DEVELOPER_MODE 0 //현재 개발자 모드임.
#define NOT_MAINWINDOW 1 //현재 메인 화면이 아님.
#define WRONG_COMMAND 2 //수행할 수 없는 명령
//}}

enum
{
    VISION_TYPE_TOP,
    VISION_TYPE_BOTTOM,
    VISION_TYPE_SIDE1,
    VISION_TYPE_SIDE2,
    VISION_TYPE_3D = 10,
};

enum enumverifyRecipe
{ //kircheis_VerifyRecipe
    RECIPE_VERIFY_START = 0,
    RECIPE_VALID = RECIPE_VERIFY_START,
    RECIPE_ERROR_COMP_MAP,
    RECIPE_VERIFY_END
};

#ifndef DEFMSGPARAM_DEF
#define DEFMSGPARAM_DEF
struct DEFMSGPARAM
{
    long P1;
    long P2;
    long P3;
    long P4;
    long P5;
    long P6;
    long P7;
    long P8;
};
#endif
/*
1. Default 형식.
    P = P1(long) + P2(long) + P3(long) + P4(long)..........P8(long)
    Reciever는 P1~P8중에서 필요한 parameter만 사용한다. 
    ex>
    #define 	MSG_PROBE_SET_ID				1002	// 		->			P1:ID(long)                 R:(long)	-Long 0:OK, 1:ERROR
    위 message의 경우는 P1만 의미가 있음.

2. Job File(CFile) 형식.
    CFile이 넘어간다.


3. Inspection Result Structure 형식.
    CReportData* 를 넘긴다.
    
    int	  m_nTotResult;
	CString   m_strResult;

	CLeadResult      *m_pLResult;
	CMarkResult	     *m_pMResult;	
	CPinResult		 *m_pPResult;
	
	다음 pointer가 가리키는 data가 측정결과와 관계없는 경우 pointer에 NULL을 넣는다.
	m_pLResult, m_pMResult, m_pPResult
	
	
4. Byte Image Data 형식. - 강박사님이 정해주세요. 


5. (Original image size + Overlay) structure 형식. - 강박사님이 정해주세요.

6. P1:(Diff1~Diff4 + Image1~Image5) Structure 형식.  - 강박사님이 정해주세요.

   

*/
