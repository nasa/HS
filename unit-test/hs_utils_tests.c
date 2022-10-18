/************************************************************************
 * NASA Docket No. GSC-18,920-1, and identified as “Core Flight
 * System (cFS) Health & Safety (HS) Application version 2.4.1”
 *
 * Copyright (c) 2021 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/*
 * Includes
 */

#include "hs_test_utils.h"
#include "hs_msgids.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>
#include "cfe.h"
#include "cfe_msgids.h"

/* hs_utils_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

void HS_VerifyMsgLength_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    bool              Result;

    /*
     * The CFE_MSG_GetMsgId() stub uses a data buffer to hold the
     * message ID values to return.
     */
    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_CPUHOG_CC;
    MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = HS_VerifyMsgLength(&UT_CmdBuf.Buf.Msg, sizeof(HS_NoArgsCmd_t));

    /* Verify results */
    UtAssert_True(Result == true, "Result == true");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_VerifyMsgLength_Test_LengthErrorHK(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    bool              Result;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid HK request msg length: ID = 0x%%08lX, CC = %%d, Len = %%d, Expected = %%d");

    /*
     * The CFE_MSG_GetMsgId() stub uses a data buffer to hold the
     * message ID values to return.
     */
    TestMsgId = CFE_SB_ValueToMsgId(HS_SEND_HK_MID);
    FcnCode   = 0;
    MsgSize   = 1;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = HS_VerifyMsgLength(&UT_CmdBuf.Buf.Msg, sizeof(HS_NoArgsCmd_t));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_HKREQ_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_VerifyMsgLength_Test_LengthErrorNonHK(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    bool              Result;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Invalid msg length: ID = 0x%%08lX, CC = %%d, Len = %%d, Expected = %%d");

    /*
     * The CFE_MSG_GetMsgId() stub uses a data buffer to hold the
     * message ID values to return.
     */
    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = 0;
    MsgSize   = 1;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = HS_VerifyMsgLength(&UT_CmdBuf.Buf.Msg, sizeof(HS_NoArgsCmd_t));

    /* Verify results */
    UtAssert_True(Result == false, "Result == false");

    UtAssert_True(HS_AppData.CmdErrCount == 1, "HS_AppData.CmdErrCount == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AMTActionIsValid_Valid(void)
{
    uint16 Action = (HS_AMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES);

    bool Result = HS_AMTActionIsValid(Action);

    UtAssert_True(Result == true, "Result == true");
}

void HS_AMTActionIsValid_Invalid(void)
{
    uint16 Action = (HS_AMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1);

    bool Result = HS_AMTActionIsValid(Action);

    UtAssert_True(Result == false, "Result == false");
}

void HS_EMTActionIsValid_Valid(void)
{
    uint16 Action = (HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES);

    bool Result = HS_EMTActionIsValid(Action);

    UtAssert_True(Result == true, "Result == true");
}

void HS_EMTActionIsValid_Invalid(void)
{
    uint16 Action = (HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1);

    bool Result = HS_EMTActionIsValid(Action);

    UtAssert_True(Result == false, "Result == false");
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_VerifyMsgLength_Test_Nominal, HS_Test_Setup, HS_Test_TearDown, "HS_VerifyMsgLength_Test_Nominal");
    UtTest_Add(HS_VerifyMsgLength_Test_LengthErrorHK, HS_Test_Setup, HS_Test_TearDown,
               "HS_VerifyMsgLength_Test_LengthErrorHK");
    UtTest_Add(HS_VerifyMsgLength_Test_LengthErrorNonHK, HS_Test_Setup, HS_Test_TearDown,
               "HS_VerifyMsgLength_Test_LengthErrorNonHK");

    UtTest_Add(HS_AMTActionIsValid_Valid, HS_Test_Setup, HS_Test_TearDown, "HS_AMTActionIsValid_Valid");

    UtTest_Add(HS_AMTActionIsValid_Invalid, HS_Test_Setup, HS_Test_TearDown, "HS_AMTActionIsValid_Invalid");

    UtTest_Add(HS_EMTActionIsValid_Valid, HS_Test_Setup, HS_Test_TearDown, "HS_EMTActionIsValid_Valid");

    UtTest_Add(HS_EMTActionIsValid_Invalid, HS_Test_Setup, HS_Test_TearDown, "HS_EMTActionIsValid_Invalid");
}
