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

#include "hs_platform_cfg.h"
#include "hs_custom.h"
#include "hs_test_utils.h"
#include "hs_msgids.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>

/* hs_custom_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

int32 HS_CUSTOM_TEST_CFE_ES_IncrementTaskCounterHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                     const UT_StubContext_t *Context)
{
    HS_CustomData.IdleTaskRunStatus = -1;

    return CFE_SUCCESS;
}

void HS_IdleTask_Test(void)
{
    HS_CustomData.UtilMask         = 1;
    HS_CustomData.ThisIdleTaskExec = 0;
    HS_CustomData.UtilArrayIndex   = 0;

    /* Set to make the while loop exit after the first call to CFE_ES_IncrementTaskCounter */
    UT_SetHookFunction(UT_KEY(CFE_ES_IncrementTaskCounter), HS_CUSTOM_TEST_CFE_ES_IncrementTaskCounterHook, NULL);

    /* Execute the function being tested */
    HS_IdleTask();

    /* Will loop incrementing ThisIdleTaskExec until update/stamp, covering all branches */
    UtAssert_UINT32_NEQ(HS_CustomData.UtilArray[0], 0);
    UtAssert_UINT32_EQ(HS_CustomData.UtilArrayIndex, 1);
    UtAssert_UINT32_EQ(HS_CustomData.ThisIdleTaskExec, 4);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_CustomInit_Test_Nominal(void)
{
    uint32 i;

    /* Fill to confirm init */
    memset(&HS_CustomData, 0xFF, sizeof(HS_CustomData));

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_CustomInit(), CFE_SUCCESS);

    UtAssert_INT32_EQ(HS_CustomData.UtilMult1, HS_UTIL_CONV_MULT1);
    UtAssert_INT32_EQ(HS_CustomData.UtilMult2, HS_UTIL_CONV_MULT2);
    UtAssert_INT32_EQ(HS_CustomData.UtilDiv, HS_UTIL_CONV_DIV);

    UtAssert_UINT32_EQ(HS_CustomData.UtilMask, HS_UTIL_DIAG_MASK);
    UtAssert_UINT32_EQ(HS_CustomData.UtilArrayIndex, 0);
    UtAssert_UINT32_EQ(HS_CustomData.UtilArrayMask, HS_UTIL_TIME_DIAG_ARRAY_MASK);

    UtAssert_UINT32_EQ(HS_CustomData.ThisIdleTaskExec, 0);
    UtAssert_UINT32_EQ(HS_CustomData.LastIdleTaskExec, 0);
    UtAssert_UINT32_EQ(HS_CustomData.LastIdleTaskInterval, 0);
    UtAssert_UINT32_EQ(HS_CustomData.UtilCycleCounter, 0);

    for (i = 0; i < sizeof(HS_CustomData.UtilArray) / sizeof(HS_CustomData.UtilArray[0]); i++)
    {
        UtAssert_UINT32_EQ(HS_CustomData.UtilArray[i], 0);
    }

    UtAssert_UINT32_EQ(HS_CustomData.UtilCallsPerMark, HS_UTIL_CALLS_PER_MARK);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_CustomInit_Test_CreateChildTaskError(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error Creating Child Task for CPU Utilization Monitoring,RC=0x%%08X");

    /* Causes event message to be generated */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_CreateChildTask), 1, -1);

    /* Execute the function being tested */
    Result = HS_CustomInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CR_CHILD_TASK_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_CustomInit_Test_RegisterSynchCallbackError(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error Registering Sync Callback for CPU Utilization Monitoring,RC=0x%%08X");

    /* Causes event message to be generated */
    UT_SetDeferredRetcode(UT_KEY(CFE_TIME_RegisterSynchCallback), 1, -1);

    /* Execute the function being tested */
    Result = HS_CustomInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_True(HS_CustomData.UtilMult1 == HS_UTIL_CONV_MULT1, "HS_CustomData.UtilMult1 == HS_UTIL_CONV_MULT1");
    UtAssert_True(HS_CustomData.UtilMult2 == HS_UTIL_CONV_MULT2, "HS_CustomData.UtilMult2 == HS_UTIL_CONV_MULT2");
    UtAssert_True(HS_CustomData.UtilDiv == HS_UTIL_CONV_DIV, "HS_CustomData.UtilDiv == HS_UTIL_CONV_DIV");
    UtAssert_True(HS_CustomData.UtilCycleCounter == 0, "HS_CustomData.UtilCycleCounter == 0");
    UtAssert_True(HS_CustomData.UtilMask == HS_UTIL_DIAG_MASK, "HS_CustomData.UtilMask == HS_UTIL_DIAG_MASK");
    UtAssert_True(HS_CustomData.UtilArrayIndex == 0, "HS_CustomData.UtilArrayIndex == 0");
    UtAssert_True(HS_CustomData.UtilArrayMask == HS_UTIL_TIME_DIAG_ARRAY_MASK,
                  "HS_CustomData.UtilArrayMask == HS_UTIL_TIME_DIAG_ARRAY_MASK");
    UtAssert_True(HS_CustomData.ThisIdleTaskExec == 0, "HS_CustomData.ThisIdleTaskExec == 0");
    UtAssert_True(HS_CustomData.LastIdleTaskExec == 0, "HS_CustomData.LastIdleTaskExec == 0");
    UtAssert_True(HS_CustomData.LastIdleTaskInterval == 0, "HS_CustomData.LastIdleTaskInterval == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CR_SYNC_CALLBACK_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_CustomCleanup_Test(void)
{
    /* No setup required for this test */

    /* Execute the function being tested */
    HS_CustomCleanup();

    /* Verify results */
    UtAssert_True(HS_CustomData.IdleTaskRunStatus == !CFE_SUCCESS, "HS_CustomData.IdleTaskRunStatus == !CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_UtilizationIncrement_Test(void)
{
    /* No setup required for this test */

    /* Execute the function being tested */
    HS_UtilizationIncrement();

    /* Verify results */
    UtAssert_True(HS_CustomData.ThisIdleTaskExec == 1, "HS_CustomData.ThisIdleTaskExec == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_UtilizationMark_Test(void)
{
    HS_CustomData.ThisIdleTaskExec = 3;
    HS_CustomData.LastIdleTaskExec = 1;

    /* Force calls per mark to be able to get branch coverage */
    HS_CustomData.UtilCallsPerMark = 2;

    /* First call will bypass */
    HS_UtilizationMark();

    UtAssert_UINT32_EQ(HS_CustomData.LastIdleTaskInterval, 0);
    UtAssert_UINT32_EQ(HS_CustomData.LastIdleTaskExec, 1);

    /* Next call will update variables */
    HS_UtilizationMark();

    /* Verify results */
    UtAssert_True(HS_CustomData.LastIdleTaskInterval == 2, "HS_CustomData.LastIdleTaskInterval == 2");
    UtAssert_True(HS_CustomData.LastIdleTaskExec == 3, "HS_CustomData.LastIdleTaskExec == 3");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MarkIdleCallback_Test(void)
{
    /* No setup required for this test */

    /* Execute the function being tested */
    HS_MarkIdleCallback();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_CustomMonitorUtilization_Test(void)
{
    /* Setting to max will cause rollover for coverage */
    HS_CustomData.UtilCycleCounter = UINT32_MAX;

    /* First call should skip stub call */
    HS_CustomMonitorUtilization();

    UtAssert_STUB_COUNT(HS_MonitorUtilization, 0);

    /* Next call exercises logic */
    HS_CustomMonitorUtilization();

    UtAssert_STUB_COUNT(HS_MonitorUtilization, 1);
    UtAssert_UINT32_EQ(HS_CustomData.UtilCycleCounter, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_CustomCommands_Test_UtilDiagReport(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             Result;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_REPORT_DIAG_CC;
    MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    /* Execute the function being tested */
    Result = HS_CustomCommands(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    /* Generates 1 message we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_CustomCommands_Test_SetUtilParamsCmd(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             Result;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_UTIL_PARAMS_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetUtilParamsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    /* Execute the function being tested */
    Result = HS_CustomCommands(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    /* Generates 1 message we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_CustomCommands_Test_SetUtilDiagCmd(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             Result;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_UTIL_DIAG_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetUtilDiagCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    /* Execute the function being tested */
    Result = HS_CustomCommands(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    /* Generates 1 message we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_CustomCommands_Test_InvalidCommandCode(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             Result;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = 99;
    MsgSize   = sizeof(UT_CmdBuf.NoArgsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    Result = HS_CustomCommands(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(Result == !CFE_SUCCESS, "Result == !CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_UtilDiagReport_Test(void)
{
    uint32 i;
    int32  strCmpResult;
    char   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Mask 0x%%08X Base Time Ticks per Idle Ticks (frequency): %%i(%%i), %%i(%%i), %%i(%%i), %%i(%%i)");

    HS_CustomData.UtilArray[0] = 0xFFFFFFFF;
    HS_CustomData.UtilArray[1] = 0x00000111;

    /* Sets all other elements to 0 */
    for (i = 2; i < HS_UTIL_TIME_DIAG_ARRAY_LENGTH; i++)
    {
        HS_CustomData.UtilArray[i] = 0;
    }

    HS_CustomData.UtilMask = 0xFFFFFFFE;

    /* Execute the function being tested */
    HS_UtilDiagReport();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_UTIL_DIAG_REPORT_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_UtilDiagReport_Test_Loops(void)
{
    /* ensure all values produce unique time jumps */
    HS_CustomData.UtilArray[0]  = 0xFFFFFFFE;
    HS_CustomData.UtilArray[1]  = 0x00000111;
    HS_CustomData.UtilArray[2]  = 0xFFFFFFD;
    HS_CustomData.UtilArray[3]  = 0x00000112;
    HS_CustomData.UtilArray[4]  = 0xFFFFFC;
    HS_CustomData.UtilArray[5]  = 0x00000113;
    HS_CustomData.UtilArray[6]  = 0xFFFFB;
    HS_CustomData.UtilArray[7]  = 0x00000114;
    HS_CustomData.UtilArray[8]  = 0xFFFA;
    HS_CustomData.UtilArray[9]  = 0x00000115;
    HS_CustomData.UtilArray[10] = 0xFF9;
    HS_CustomData.UtilArray[11] = 0x00000116;
    HS_CustomData.UtilArray[12] = 0xF8;
    HS_CustomData.UtilArray[13] = 0x00000117;
    HS_CustomData.UtilArray[14] = 0x7;
    HS_CustomData.UtilArray[15] = 0x00000118;

    HS_CustomData.UtilMask = 0;

    /* Execute the function being tested */
    HS_UtilDiagReport();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_UTIL_DIAG_REPORT_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_CustomGetUtil_Test(void)
{
    int32 Result;

    HS_CustomData.LastIdleTaskInterval = 9999;
    HS_CustomData.UtilMult1            = 1;
    HS_CustomData.UtilDiv              = 1;
    HS_CustomData.UtilMult2            = 1;

    /* Execute the function being tested */
    Result = HS_CustomGetUtil();

    /* Verify results */
    UtAssert_True(Result == 1, "Result == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_CustomGetUtil_Test_DivZero(void)
{
    int32 Result;

    HS_CustomData.LastIdleTaskInterval = 9999;
    HS_CustomData.UtilMult1            = 1;
    HS_CustomData.UtilDiv              = 1;
    HS_CustomData.UtilMult2            = 1;

    HS_CustomData.UtilDiv = 0;

    /* Execute the function being tested */
    Result = HS_CustomGetUtil();

    /* Verify results */
    UtAssert_True(Result == 0, "Result == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SetUtilParamsCmd_Test_Nominal(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Utilization Parms set: Mult1: %%d Div: %%d Mult2: %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_UTIL_PARAMS_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetUtilParamsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    UT_CmdBuf.SetUtilParamsCmd.Mult1 = 1;
    UT_CmdBuf.SetUtilParamsCmd.Mult2 = 2;
    UT_CmdBuf.SetUtilParamsCmd.Div   = 3;

    /* Execute the function being tested */
    HS_SetUtilParamsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(HS_CustomData.UtilMult1 == 1, "HS_CustomData.UtilMult1 == 1");
    UtAssert_True(HS_CustomData.UtilMult2 == 2, "HS_CustomData.UtilMult2 == 2");
    UtAssert_True(HS_CustomData.UtilDiv == 3, "HS_CustomData.UtilDiv == 3");
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SET_UTIL_PARAMS_DBG_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SetUtilParamsCmd_Test_NominalMultZero(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Utilization Parms Error: No parameter may be 0: Mult1: %%d Div: %%d Mult2: %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_UTIL_PARAMS_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetUtilParamsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    UT_CmdBuf.SetUtilParamsCmd.Mult1 = 1;
    UT_CmdBuf.SetUtilParamsCmd.Mult2 = 0;
    UT_CmdBuf.SetUtilParamsCmd.Div   = 3;

    /* Execute the function being tested */
    HS_SetUtilParamsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(HS_CustomData.UtilMult1, 0);
    UtAssert_INT32_EQ(HS_CustomData.UtilMult2, 0);
    UtAssert_INT32_EQ(HS_CustomData.UtilDiv, 0);
    UtAssert_INT32_EQ(HS_AppData.CmdCount, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SET_UTIL_PARAMS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SetUtilParamsCmd_Test_NominalDivZero(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Utilization Parms Error: No parameter may be 0: Mult1: %%d Div: %%d Mult2: %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_UTIL_PARAMS_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetUtilParamsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    UT_CmdBuf.SetUtilParamsCmd.Mult1 = 1;
    UT_CmdBuf.SetUtilParamsCmd.Mult2 = 2;
    UT_CmdBuf.SetUtilParamsCmd.Div   = 0;

    /* Execute the function being tested */
    HS_SetUtilParamsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(HS_CustomData.UtilMult1, 0);
    UtAssert_INT32_EQ(HS_CustomData.UtilMult2, 0);
    UtAssert_INT32_EQ(HS_CustomData.UtilDiv, 0);
    UtAssert_INT32_EQ(HS_AppData.CmdCount, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SET_UTIL_PARAMS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SetUtilParamsCmd_Test_Error(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Utilization Parms Error: No parameter may be 0: Mult1: %%d Div: %%d Mult2: %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_UTIL_PARAMS_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetUtilParamsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    UT_CmdBuf.SetUtilParamsCmd.Mult1 = 0;
    UT_CmdBuf.SetUtilParamsCmd.Mult2 = 2;
    UT_CmdBuf.SetUtilParamsCmd.Div   = 3;

    /* Execute the function being tested */
    HS_SetUtilParamsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdErrCount == 1, "HS_AppData.CmdErrCount == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SET_UTIL_PARAMS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SetUtilParamsCmd_Test_MsgLengthError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_UTIL_PARAMS_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetUtilParamsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), false);

    UT_CmdBuf.SetUtilParamsCmd.Mult1 = 1;
    UT_CmdBuf.SetUtilParamsCmd.Mult2 = 2;
    UT_CmdBuf.SetUtilParamsCmd.Div   = 3;

    /* Execute the function being tested */
    HS_SetUtilParamsCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_INT32_EQ(HS_CustomData.UtilMult1, 0);
    UtAssert_INT32_EQ(HS_CustomData.UtilMult2, 0);
    UtAssert_INT32_EQ(HS_CustomData.UtilDiv, 0);
    UtAssert_INT32_EQ(HS_AppData.CmdCount, 0);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SetUtilDiagCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Utilization Diagnostics Mask has been set to %%08X");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_UTIL_DIAG_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetUtilDiagCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* ignore dummy message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    UT_CmdBuf.SetUtilDiagCmd.Mask = 2;

    /* Execute the function being tested */
    HS_SetUtilDiagCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");
    UtAssert_True(HS_CustomData.UtilMask == 2, "HS_AppData.CmdCount == 2");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SET_UTIL_DIAG_DBG_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SetUtilDiagCmd_Test_MsgLengthError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_UTIL_DIAG_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetUtilDiagCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* set message length check */
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), false);

    UT_CmdBuf.SetUtilDiagCmd.Mask = 2;

    /* Execute the function being tested */
    HS_SetUtilDiagCmd(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 0, "HS_AppData.CmdCount == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_IdleTask_Test, HS_Test_Setup, HS_Test_TearDown, "HS_IdleTask_Test");

    UtTest_Add(HS_CustomInit_Test_Nominal, HS_Test_Setup, HS_Test_TearDown, "HS_CustomInit_Test_Nominal");
    UtTest_Add(HS_CustomInit_Test_CreateChildTaskError, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomInit_Test_CreateChildTaskError");
    UtTest_Add(HS_CustomInit_Test_RegisterSynchCallbackError, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomInit_Test_RegisterSynchCallbackError");

    UtTest_Add(HS_CustomCleanup_Test, HS_Test_Setup, HS_Test_TearDown, "HS_CustomCleanup_Test");

    UtTest_Add(HS_UtilizationIncrement_Test, HS_Test_Setup, HS_Test_TearDown, "HS_UtilizationIncrement_Test");

    UtTest_Add(HS_UtilizationMark_Test, HS_Test_Setup, HS_Test_TearDown, "HS_UtilizationMark_Test");

    UtTest_Add(HS_MarkIdleCallback_Test, HS_Test_Setup, HS_Test_TearDown, "HS_MarkIdleCallback_Test");

    UtTest_Add(HS_CustomMonitorUtilization_Test, HS_Test_Setup, HS_Test_TearDown, "HS_CustomMonitorUtilization_Test");

    UtTest_Add(HS_CustomCommands_Test_UtilDiagReport, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomCommands_Test_UtilDiagReport");
    UtTest_Add(HS_CustomCommands_Test_SetUtilParamsCmd, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomCommands_Test_SetUtilParamsCmd");
    UtTest_Add(HS_CustomCommands_Test_SetUtilDiagCmd, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomCommands_Test_SetUtilDiagCmd");
    UtTest_Add(HS_CustomCommands_Test_InvalidCommandCode, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomCommands_Test_InvalidCommandCode");

    UtTest_Add(HS_UtilDiagReport_Test, HS_Test_Setup, HS_Test_TearDown, "HS_UtilDiagReport_Test");
    UtTest_Add(HS_UtilDiagReport_Test_Loops, HS_Test_Setup, HS_Test_TearDown, "HS_UtilDiagReport_Test_Loops");

    UtTest_Add(HS_CustomGetUtil_Test, HS_Test_Setup, HS_Test_TearDown, "HS_CustomGetUtil_Test");
    UtTest_Add(HS_CustomGetUtil_Test_DivZero, HS_Test_Setup, HS_Test_TearDown, "HS_CustomGetUtil_Test_DivZero");

    UtTest_Add(HS_SetUtilParamsCmd_Test_Nominal, HS_Test_Setup, HS_Test_TearDown, "HS_SetUtilParamsCmd_Test_Nominal");
    UtTest_Add(HS_SetUtilParamsCmd_Test_NominalMultZero, HS_Test_Setup, HS_Test_TearDown,
               "HS_SetUtilParamsCmd_Test_NominalMultZero");
    UtTest_Add(HS_SetUtilParamsCmd_Test_NominalDivZero, HS_Test_Setup, HS_Test_TearDown,
               "HS_SetUtilParamsCmd_Test_NominalDivZero");
    UtTest_Add(HS_SetUtilParamsCmd_Test_Error, HS_Test_Setup, HS_Test_TearDown, "HS_SetUtilParamsCmd_Test_Error");
    UtTest_Add(HS_SetUtilParamsCmd_Test_MsgLengthError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SetUtilParamsCmd_Test_MsgLengthError");

    UtTest_Add(HS_SetUtilDiagCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_SetUtilDiagCmd_Test");
    UtTest_Add(HS_SetUtilDiagCmd_Test_MsgLengthError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SetUtilDiagCmd_Test_MsgLengthError");
}
