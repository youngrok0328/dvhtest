#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#define DECLARE_A2V_MSG_STRUCT(MSG_NAME) \
    struct MSG_A2V_##MSG_NAME : public MSG_A2V_TEMPLATE<MSG_A2V::##MSG_NAME, MSG_A2V_##MSG_NAME>

////////////////////////////////////////////////
//Automation에서 Vision S/W에 보내는 MSG
//현재는 간단만 MSG만 주고받을 예정이라 나중에 파일 및 크기가 필요한 구조가 요구될때에는 Template를 이용하여 구성
////////////////////////////////////////////////
enum class MSG_A2V : long
{
    AUTOMATION_MSG_VISION_CONNECT,
    AUTOMATION_MSG_RUN_INFO, //검증시 필요한 Data, Recipe,Image Path, RunMode
    AUTOMATION_MSG_RECIPE_OPEN, //Vision s/w에서 Recipe를 automation에 지정된 recipe 를 연다.
    AUTOMATION_MSG_RECIPE_SAVE_ANOTHER_NAME, //Open된 Recipe를 다른 이름으로 저장
    AUTOMATION_MSG_RECIPE_OPEN_ANOTHER_NAME, //다른 이름으로 저장된 Recipe를 다시 Open
    AUTOMATION_MSG_TEXT_RECIPE_EXPORT, //Text Recipe Export [이것도 Option화가 되어야 할꺼 같다 다른 설비군의 S/W는 Text Recipe Export 기능이 존재하지 않는다]
    AUTOMATION_MSG_BATCH_INSPECTION_UI_OPEN, //Batch Inspection UI로 들어간다.
    AUTOMATION_MSG_BATCH_INSPECTION_IMAGE_ADD, //이미지 vector를 통해 Vision s/w에 이미지 open
    AUTOMATION_MSG_BATCH_INSPECTION_RUN, //Ack가 날라오면 Data Save까지 끝났다고 본다 이후에는 Report 폴더 Copy??
    AUTOMATION_MSG_BATCH_INSPECTION_END_ACK, //All inspection end 신호의 송신 완료
    AUTOMATION_MSG_BATCH_INSPECTION_UI_CLOSE, //Main UI로 돌아간다. 해당 과정에서 Batch Inspection이 Stop 호출된다.
    AUTOMATION_MSG_VISION_SW_TERMINATE, //Vision S/W 종료
};

struct MSG_A2V_DEF
{
    MSG_A2V_DEF(const MSG_A2V messageIndex, const long structSize)
        : m_messageIndex(messageIndex)
        , m_structSize(structSize)
        , m_messageToken(0)
        , m_messageLength(structSize)
    {
    }

    const MSG_A2V m_messageIndex;
    const long m_structSize;
    mutable UINT m_messageToken;
    size_t m_messageLength;
};

template<MSG_A2V messageIndex, typename ChildStructure>
struct MSG_A2V_TEMPLATE : public MSG_A2V_DEF
{
    MSG_A2V_TEMPLATE(const MSG_A2V_TEMPLATE& rhs) = default;
    MSG_A2V_TEMPLATE()
        : MSG_A2V_DEF(messageIndex, sizeof(ChildStructure))
    {
    }

    MSG_A2V_TEMPLATE& operator=(const MSG_A2V_TEMPLATE& rhs) = default;
};

DECLARE_A2V_MSG_STRUCT(AUTOMATION_MSG_RUN_INFO)
{
    char m_strRecipeFileName[255] = {
        NULL,
    };
    ;
    char m_strImageFilePath[255] = {
        NULL,
    };
    ;
    long m_eAutomationRunMode;
};

////////////////////////////////////////////////
//Vision S/W에서 Automation에게 보내는 MSG (ACK)
//현재는 간단만 MSG만 주고받을 예정이라 나중에 파일 및 크기가 필요한 구조가 요구될때에는 Template를 이용하여 구성
////////////////////////////////////////////////
enum class MSG_V2A : long
{
    AUTOMATION_MSG_VISION_CONNECT_ACK,
    AUTOMATION_MSG_RUN_INFO_ACK,
    AUTOMATION_MSG_RECIPE_OPEN_ACK,
    AUTOMATION_MSG_RECIPE_SAVE_ANOTHER_NAME_ACK,
    AUTOMATION_MSG_RECIPE_OPEN_ANOTHER_NAME_ACK,
    AUTOMATION_MSG_TEXT_RECIPE_EXPORT_ACK,
    AUTOMATION_MSG_BATCH_INSPECTION_UI_OPEN_ACK,
    AUTOMATION_MSG_BATCH_INSPECTION_IMAGE_ADD_ACK,
    AUTOMATION_MSG_BATCH_INSPECTION_RUN_ACK,
    AUTOMATION_MSG_BATCH_INSPECTION_END,
    AUTOMATION_MSG_BATCH_INSPECTION_UI_CLOSE_ACK,
    AUTOMATION_MSG_VISION_SW_TERMINATE_ACK,
};

struct MSG_V2A_DEF
{
    MSG_V2A_DEF(const MSG_V2A messageIndex, const long structSize, UINT token)
        : m_messageIndex(messageIndex)
        , m_structSize(structSize)
        , m_messageToken(token)
        , m_messageLength(structSize)
    {
    }

    MSG_V2A m_messageIndex;
    long m_structSize;
    UINT m_messageToken;
    long m_messageLength;
};

template<MSG_V2A messageIndex, typename ChildStructure>
struct MSG_V2A_TEMPLATE : public MSG_V2A_DEF
{
    MSG_V2A_TEMPLATE(const MSG_V2A_TEMPLATE& rhs) = default;
    MSG_V2A_TEMPLATE(UINT token)
        : MSG_V2A_DEF(messageIndex, sizeof(ChildStructure), token)
    {
    }

    MSG_V2A_TEMPLATE& operator=(const MSG_V2A_TEMPLATE& rhs) = default;
};