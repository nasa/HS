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

#include "hs_app.h"
#include "hs_sysmon.h"
#include "hs_test_utils.h"
#include "hs_msgids.h"
#include "hs_dispatch.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>
#include "cfe.h"
#include "cfe_msgids.h"
#include "cfe_evs_msg.h"

typedef struct
{
    CFE_SB_Buffer_t **BufPtr;
    CFE_SB_PipeId_t   PipeId;
    int32             Timeout;
} CFE_SB_ReceiveBuffer_context_t;

/* hs_app_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

uint16 HS_APP_TEST_CFE_SB_RcvMsgHookCount;
uint32 UT_WaitForStartupSyncTimeout;

void UT_UpdatedDefaultHandler_CFE_SB_ReceiveBuffer(void *UserObj, UT_EntryKey_t FuncKey,
                                                   const UT_StubContext_t *Context)
{
    CFE_SB_Buffer_t **BufPtr = UT_Hook_GetArgValueByName(Context, "BufPtr", CFE_SB_Buffer_t **);
    UT_Stub_CopyToLocal(UT_KEY(CFE_SB_ReceiveBuffer), BufPtr, sizeof(*BufPtr));
}

int32 HS_APP_TEST_CFE_SB_RcvMsgHook(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    HS_APP_TEST_CFE_SB_RcvMsgHookCount++;

    if (HS_APP_TEST_CFE_SB_RcvMsgHookCount % 2 == 1)
        return CFE_SUCCESS;
    else
        return CFE_SB_NO_MESSAGE;
}

void HS_APP_TEST_CFE_RcvBufferHandler(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    CFE_SB_Buffer_t **BufPtr = UT_Hook_GetArgValueByName(Context, "BufPtr", CFE_SB_Buffer_t **);

    CFE_Status_t status;
    HS_APP_TEST_CFE_SB_RcvMsgHookCount++;

    if (HS_APP_TEST_CFE_SB_RcvMsgHookCount % 2 == 1)
        status = CFE_SUCCESS;
    else
        status = CFE_SB_NO_MESSAGE;

    UT_Stub_GetInt32StatusCode(Context, &status);

    UT_Stub_CopyToLocal(UT_KEY(CFE_SB_ReceiveBuffer), BufPtr, sizeof(*BufPtr));
}

uint16 HS_APP_TEST_CFE_TBL_LoadHookCount;
int32 HS_APP_TEST_CFE_TBL_LoadHook1(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    HS_APP_TEST_CFE_TBL_LoadHookCount++;

    if (HS_APP_TEST_CFE_TBL_LoadHookCount == 2)
        return -1;
    else
        return CFE_SUCCESS;
}

int32 HS_APP_TEST_CFE_TBL_LoadHook2(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    HS_APP_TEST_CFE_TBL_LoadHookCount++;

    if (HS_APP_TEST_CFE_TBL_LoadHookCount == 3)
        return -1;
    else
        return CFE_SUCCESS;
}

int32 HS_APP_TEST_CFE_TBL_LoadHook3(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context)
{
    HS_APP_TEST_CFE_TBL_LoadHookCount++;

    if (HS_APP_TEST_CFE_TBL_LoadHookCount == 4)
        return -1;
    else
        return CFE_SUCCESS;
}

int32 HS_APP_TEST_CFE_ES_RestoreFromCDSHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                            const UT_StubContext_t *Context)
{
    /* Restore the HS_AppData.CDSData that was memset in HS_AppInit */

    memcpy(&HS_AppData.CDSData, UserObj, sizeof(HS_AppData.CDSData));

    return CFE_SUCCESS;
}

int32 HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                                 const UT_StubContext_t *Context)
{
    UT_WaitForStartupSyncTimeout = UT_Hook_GetArgValueByName(Context, "TimeOutMilliseconds", uint32);

    /* This functionality is not directly related to WaitForStartupSync, but WaitForStartupSync is in a place where
       it's necessary to do this for the test case HS_AppMain_Test_Nominal */

    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;

    return 0;
}

int32 HS_APP_TEST_CFE_EVS_RegisterHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                       const UT_StubContext_t *Context)
{
    /* HS_AppInit sets CurrentEventMonState to HS_APPMON_DEFAULT_STATE, so change it to disabled at
       call to CF_EVS_RegisterHook */

    HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;

    return 0;
}

int32 HS_APP_TEST_CFE_ES_ExitAppHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                     const UT_StubContext_t *Context)
{
    HS_AppData.EventsMonitoredCount++;

    return 0;
}

/*
 * Function Definitions
 */
void HS_AppMain_Test_NominalWaitForStartupSync(void)
{
    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Causes HS_ProcessCommands to return CFE_SUCCESS, which is then returned from HS_ProcessMain */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_NO_MESSAGE);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_UINT8_EQ(HS_AppData.CurrentEventMonState, HS_STATE_ENABLED);

    /* This verifies HS8006 and HS8006.1 to wait for startup sync with platform defined timeout */
    UtAssert_STUB_COUNT(CFE_ES_WaitForStartupSync, 1);
    UtAssert_UINT32_EQ(UT_WaitForStartupSyncTimeout, HS_STARTUP_SYNC_TIMEOUT);

    /* 1 event message that we don't care about in this test */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_AppMain_Test_NominalRcvMsgSuccess(void)
{
    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Causes HS_ProcessCommands to return CFE_SUCCESS, which is then returned from HS_ProcessMain */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_NO_MESSAGE);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    /* Used to verify completion of HS_AppMain by incrementing HS_AppData.EventsMonitoredCount. */
    UT_SetHookFunction(UT_KEY(CFE_ES_ExitApp), HS_APP_TEST_CFE_ES_ExitAppHook, NULL);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_True(HS_AppData.EventsMonitoredCount == 1, "HS_AppData.EventsMonitoredCount == 1");

    /* 1 event message that we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppMain_Test_NominalRcvMsgNoMessage(void)
{
    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set return code being tested, to reach "Status = HS_ProcessMain()" as one of the nominal cases */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);
    /* Set CFE_SB_TIME_OUT so HS_ProcessCommands will return to HS_ProcessMain */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_TIME_OUT);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_NO_MESSAGE);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_TIME_OUT);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    /* Used to verify completion of HS_AppMain by incrementing HS_AppData.EventsMonitoredCount. */
    UT_SetHookFunction(UT_KEY(CFE_ES_ExitApp), HS_APP_TEST_CFE_ES_ExitAppHook, NULL);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_True(HS_AppData.EventsMonitoredCount == 1, "HS_AppData.EventsMonitoredCount == 1");

    /* 1 event message that we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppMain_Test_NominalRcvMsgTimeOut(void)
{
    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set return code being tested, to reach "Status = HS_ProcessMain()" as one of the nominal cases */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);
    /* Set CFE_SB_TIME_OUT so HS_ProcessCommands will return to HS_ProcessMain */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_TIME_OUT);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_TIME_OUT);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_TIME_OUT);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    /* Used to verify completion of HS_AppMain by incrementing HS_AppData.EventsMonitoredCount. */
    UT_SetHookFunction(UT_KEY(CFE_ES_ExitApp), HS_APP_TEST_CFE_ES_ExitAppHook, NULL);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_True(HS_AppData.EventsMonitoredCount == 1, "HS_AppData.EventsMonitoredCount == 1");

    /* 2 event messages that we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppMain_Test_NominalRcvMsgError(void)
{
    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set return code being tested, to bypass "Status = HS_ProcessMain()" as one of the nominal cases */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), -1);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    /* Used to verify completion of HS_AppMain by incrementing HS_AppData.EventsMonitoredCount. */
    UT_SetHookFunction(UT_KEY(CFE_ES_ExitApp), HS_APP_TEST_CFE_ES_ExitAppHook, NULL);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_True(HS_AppData.EventsMonitoredCount == 1, "HS_AppData.EventsMonitoredCount == 1");

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_APP_EXIT_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_CRITICAL);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppMain_Test_AppInitNotSuccess(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Application Terminating, err = 0x%%08X");
    char ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: Application Terminating, ERR = 0x%%08X\n");

    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Same return value as default, but bypasses default hook function to make test easier to write */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, -1);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    /* Set CFE_EVS_Register to return -1 in order to reach call to CFE_ES_WriteToSysLog */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APP_EXIT_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_CRITICAL);

    strCmpResult = strncmp(ExpectedEventString, context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void HS_AppMain_Test_ProcessMainNotSuccess(void)
{
    int32 strCmpResult;
    char  ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: Application Terminating, ERR = 0x%%08X\n");

    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* In order to make ProcessMain return -1, we make its call to HS_ProcessCommands
       return -1 by making its call to CFE_SB_ReceiveBuffer return -1 */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), -1);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_APP_EXIT_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_CRITICAL);

    /* 1 event message that we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void HS_AppMain_Test_SBSubscribeEVSLongError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[3][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error Subscribing to long-format Events,RC=0x%%08X");
    snprintf(ExpectedEventString[2], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Application Terminating, err = 0x%%08X");
    char ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: Application Terminating, ERR = 0x%%08X\n");

    /* Set so the loop will never be run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, false);

    /* Same return value as default, but bypasses default hook function to make test easier to write */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    /* Set CFE_SB_SubscribeEx to return -1 in order to generate error message HS_SUB_EVS_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_SubscribeEx), 1, -1);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 3, "CFE_EVS_SendEvent was called %u time(s), expected 3",
                  call_count_CFE_EVS_SendEvent);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_SUB_LONG_EVS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, HS_APP_EXIT_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventType, CFE_EVS_EventType_CRITICAL);

    strCmpResult =
        strncmp(ExpectedEventString[2], context_CFE_EVS_SendEvent[2].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[2].Spec);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void HS_AppMain_Test_SBSubscribeEVSShortError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[3][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error Subscribing to short-format Events,RC=0x%%08X");
    snprintf(ExpectedEventString[2], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Application Terminating, err = 0x%%08X");
    char ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: Application Terminating, ERR = 0x%%08X\n");

    /* Set so the loop will never be run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, false);

    /* Same return value as default, but bypasses default hook function to make test easier to write */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    /* Set CFE_SB_SubscribeEx to return -1 in order to generate error message HS_SUB_EVS_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_SubscribeEx), 2, -1);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_SUB_SHORT_EVS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, HS_APP_EXIT_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventType, CFE_EVS_EventType_CRITICAL);

    strCmpResult =
        strncmp(ExpectedEventString[2], context_CFE_EVS_SendEvent[2].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[2].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 3, "CFE_EVS_SendEvent was called %u time(s), expected 3",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void HS_AppMain_Test_RcvMsgError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Application Terminating, err = 0x%%08X");
    char ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: Application Terminating, ERR = 0x%%08X\n");

    /* Set to make loop execute exactly once */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, true);

    /* Set RcvMsg to return -1 in order to reach "RunStatus = CFE_ES_APP_ERROR" */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, -1);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_ENABLED (because set to 0 inside HS_AppMain) */
    UT_SetHookFunction(UT_KEY(CFE_ES_WaitForStartupSync), HS_APP_TEST_CFE_ES_WaitForStartupSyncHook1, NULL);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_APP_EXIT_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_CRITICAL);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    /* 1 event message that we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void HS_AppMain_Test_StateDisabled(void)
{
    /* Set so the loop will never be run */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RunLoop), 1, false);

    /* Same return value as default, but bypasses default hook function to make test easier to write */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);

    /* Sets HS_AppData.CurrentEventMonState = HS_STATE_DISABLED (because set to 1 inside HS_AppInit) */
    UT_SetHookFunction(UT_KEY(CFE_EVS_Register), HS_APP_TEST_CFE_EVS_RegisterHook, NULL);

    /* Execute the function being tested */
    HS_AppMain();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppInit_Test_Nominal(void)
{
    CFE_Status_t Result;

    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;
    HS_AppData.CmdCount              = 99;
    HS_AppData.CmdErrCount           = 99;
    HS_AppData.UtilCpuPeak           = 99;
    HS_AppData.UtilCpuAvg            = 99;

    HS_AppData.CDSData.ResetsPerformed = 99;
    HS_AppData.CDSData.MaxResets       = 99;

    /* Execute the function being tested */
    Result = HS_AppInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");
    UtAssert_True(HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    /* Not checking that HS_AppData.CurrentAppMonState == HS_APPMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    /* Not checking that HS_AppData.CurrentEventMonState == HS_EVENTMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE,
                  "HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE");
    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE,
                  "HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE");
    UtAssert_UINT8_EQ(HS_AppData.CmdCount, 0);
    UtAssert_UINT8_EQ(HS_AppData.CmdErrCount, 0);
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuPeak, 0);
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuAvg, 0);

    UtAssert_UINT16_EQ(HS_AppData.CDSData.ResetsPerformed, 0);
    UtAssert_UINT16_EQ(HS_AppData.CDSData.MaxResets, 0);

    /* 1 event message that we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppInit_Test_EVSRegisterError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: Error Registering For Event Services, RC = 0x%%08X\n");

    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;

    /* Set CFE_EVS_Register to return -1 in order to reach call to CFE_ES_WriteToSysLog */
    UT_SetDeferredRetcode(UT_KEY(CFE_EVS_Register), 1, -1);

    /* Execute the function being tested */
    Result = HS_AppInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");
    UtAssert_True(HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    /* Not checking that HS_AppData.CurrentAppMonState == HS_APPMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    /* Not checking that HS_AppData.CurrentEventMonState == HS_EVENTMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE,
                  "HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE");
    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE,
                  "HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void HS_AppInit_Test_CorruptCDSResetsPerformed(void)
{
    HS_AppData_t AppData;
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data in CDS was corrupt, initializing resets data");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "HS Initialized.  Version %%d.%%d.%%d.%%d");

    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;

    HS_AppData.CDSData.MaxResets    = 0;
    HS_AppData.CDSData.MaxResetsNot = (~HS_AppData.CDSData.MaxResets);

    HS_AppData.CDSData.ResetsPerformed    = 1;
    HS_AppData.CDSData.ResetsPerformedNot = 3;

    /* To enter if-block after "Create Critical Data Store" */
    AppData = HS_AppData;
    UT_SetHookFunction(UT_KEY(CFE_ES_RestoreFromCDS), HS_APP_TEST_CFE_ES_RestoreFromCDSHook, &AppData.CDSData);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RegisterCDS), 1, CFE_ES_CDS_ALREADY_EXISTS);

    /* Execute the function being tested */
    Result = HS_AppInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");
    UtAssert_True(HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    /* Not checking that HS_AppData.CurrentAppMonState == HS_APPMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    /* Not checking that HS_AppData.CurrentEventMonState == HS_EVENTMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE,
                  "HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE");
    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE,
                  "HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CDS_CORRUPT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppInit_Test_CorruptCDSMaxResets(void)
{
    HS_AppData_t AppData;
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Data in CDS was corrupt, initializing resets data");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "HS Initialized.  Version %%d.%%d.%%d.%%d");

    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;

    HS_AppData.CDSData.MaxResets    = 1;
    HS_AppData.CDSData.MaxResetsNot = 3;

    HS_AppData.CDSData.ResetsPerformed    = 0;
    HS_AppData.CDSData.ResetsPerformedNot = (~HS_AppData.CDSData.ResetsPerformedNot);

    /* To enter if-block after "Create Critical Data Store" */
    AppData = HS_AppData;
    UT_SetHookFunction(UT_KEY(CFE_ES_RestoreFromCDS), HS_APP_TEST_CFE_ES_RestoreFromCDSHook, &AppData.CDSData);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RegisterCDS), 1, CFE_ES_CDS_ALREADY_EXISTS);

    /* Execute the function being tested */
    Result = HS_AppInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");
    UtAssert_True(HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    /* Not checking that HS_AppData.CurrentAppMonState == HS_APPMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    /* Not checking that HS_AppData.CurrentEventMonState == HS_EVENTMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE,
                  "HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE");
    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE,
                  "HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CDS_CORRUPT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppInit_Test_CorruptCDSNoEvent(void)
{
    HS_AppData_t AppData;
    CFE_Status_t Result;

    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;

    HS_AppData.CDSData.MaxResets    = 0;
    HS_AppData.CDSData.MaxResetsNot = (~HS_AppData.CDSData.MaxResets);

    HS_AppData.CDSData.ResetsPerformed    = 0;
    HS_AppData.CDSData.ResetsPerformedNot = (~HS_AppData.CDSData.ResetsPerformedNot);

    /* To enter if-block after "Create Critical Data Store" */
    AppData = HS_AppData;
    UT_SetHookFunction(UT_KEY(CFE_ES_RestoreFromCDS), HS_APP_TEST_CFE_ES_RestoreFromCDSHook, &AppData.CDSData);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RegisterCDS), 1, CFE_ES_CDS_ALREADY_EXISTS);

    /* Execute the function being tested */
    Result = HS_AppInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");
    UtAssert_True(HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    /* Not checking that HS_AppData.CurrentAppMonState == HS_APPMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    /* Not checking that HS_AppData.CurrentEventMonState == HS_EVENTMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE,
                  "HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE");
    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE,
                  "HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppInit_Test_RestoreCDSError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Failed to restore data from CDS (Err=0x%%08x), initializing resets data");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "HS Initialized.  Version %%d.%%d.%%d.%%d");

    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;

    /* To enter if-block after "Create Critical Data Store" */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RegisterCDS), 1, CFE_ES_CDS_ALREADY_EXISTS);

    /* Set CFE_ES_RestoreFromCDS to return -1 in order to generate error HS_CDS_RESTORE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestoreFromCDS), 1, -1);

    /* Execute the function being tested */
    Result = HS_AppInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CDS_RESTORE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppInit_Test_DisableSavingToCDS(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "HS Initialized.  Version %%d.%%d.%%d.%%d");

    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;

    /* Set CFE_ES_RegisterCDS to return -1 in order to reach block of code with comment "Disable saving to CDS" */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RegisterCDS), 1, -1);

    /* Execute the function being tested */
    Result = HS_AppInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");
    UtAssert_True(HS_AppData.CDSState == HS_STATE_DISABLED, "HS_AppData.CDSState == HS_STATE_DISABLED");

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");
    UtAssert_True(HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    /* Not checking that HS_AppData.CurrentAppMonState == HS_APPMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    /* Not checking that HS_AppData.CurrentEventMonState == HS_EVENTMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE,
                  "HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE");
    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE,
                  "HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_INIT_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);
    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppInit_Test_SBInitError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Creating SB Command Pipe,RC=0x%%08X");

    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;

    /* Set CFE_SB_CreatePipe to return -1 on the first call in order to cause HS_SbInit to return -1, in order to enter
     * the if-block immediately after */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, -1);

    /* Execute the function being tested */
    Result = HS_AppInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");
    UtAssert_True(HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    /* Not checking that HS_AppData.CurrentAppMonState == HS_APPMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    /* Not checking that HS_AppData.CurrentEventMonState == HS_EVENTMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE,
                  "HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE");
    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE,
                  "HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE");

    /* This event message is not generated directly by the function under test, but it's useful to check for it to
     * ensure that an SB init error occurred rather than a TBL init error */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CR_PIPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppInit_Test_TblInitError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Registering AppMon Table,RC=0x%%08X");

    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;

    /* Set CFE_TBL_Register to return -1 in order to cause HS_TblInit to return -1, in order to enter the if-block
     * immediately after */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

    /* Execute the function being tested */
    Result = HS_AppInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");
    UtAssert_True(HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    /* Not checking that HS_AppData.CurrentAppMonState == HS_APPMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    /* Not checking that HS_AppData.CurrentEventMonState == HS_EVENTMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE,
                  "HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE");
    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE,
                  "HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE");

    /* This event message is not generated directly by the function under test, but it's useful to check for it to
     * ensure that a TBL init error occurred rather than an SB init error */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMT_REG_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppInit_Test_CustomInitError(void)
{
    HS_AppData.ServiceWatchdogFlag   = 99;
    HS_AppData.AlivenessCounter      = 99;
    HS_AppData.RunStatus             = 99;
    HS_AppData.EventsMonitoredCount  = 99;
    HS_AppData.MsgActExec            = 99;
    HS_AppData.CurrentAppMonState    = 99;
    HS_AppData.CurrentEventMonState  = 99;
    HS_AppData.CurrentAlivenessState = 99;
    HS_AppData.CurrentCPUHogState    = 99;

    /* Causes HS_SysMonInit to return an error */
    UT_SetDeferredRetcode(UT_KEY(HS_SysMonInit), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_AppInit(), -1);

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");
    UtAssert_True(HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN, "HS_AppData.RunStatus == CFE_ES_RunStatus_APP_RUN");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    /* Not checking that HS_AppData.CurrentAppMonState == HS_APPMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    /* Not checking that HS_AppData.CurrentEventMonState == HS_EVENTMON_DEFAULT_STATE, because it's modified in a
     * subfunction that we don't care about here */
    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE,
                  "HS_AppData.CurrentAlivenessState == HS_ALIVENESS_DEFAULT_STATE");
    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE,
                  "HS_AppData.CurrentCPUHogState == HS_CPUHOG_DEFAULT_STATE");

    /* This event message is not generated directly by the function under test, but it's useful to check for it to
     * ensure that a TBL init error occurred rather than an SB init error */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SYSMON_INIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_SbInit_Test_Nominal(void)
{
    CFE_SB_PipeId_t PipeId = HS_UT_PIPEID_1;

    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SbInit(), CFE_SUCCESS);

    /* Verify results */
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.CmdPipe, PipeId));
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.EventPipe, PipeId));
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.WakeupPipe, PipeId));

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SbInit_Test_CreateSBCmdPipeError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Creating SB Command Pipe,RC=0x%%08X");

    CFE_SB_PipeId_t PipeId = HS_UT_PIPEID_1;

    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);

    /* Set CFE_SB_CreatePipe to return -1 on first call, to generate error HS_CR_PIPE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SbInit(), -1);

    /* Verify results */
    UtAssert_BOOL_FALSE(CFE_RESOURCEID_TEST_DEFINED(HS_AppData.CmdPipe));
    UtAssert_BOOL_FALSE(CFE_RESOURCEID_TEST_DEFINED(HS_AppData.EventPipe));
    UtAssert_BOOL_FALSE(CFE_RESOURCEID_TEST_DEFINED(HS_AppData.WakeupPipe));

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CR_PIPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SbInit_Test_CreateSBEventPipeError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Creating SB Event Pipe,RC=0x%%08X");

    CFE_SB_PipeId_t PipeId = HS_UT_PIPEID_1;

    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);

    /* Set CFE_SB_CreatePipe to return -1 on second call, to generate error HS_CR_EVENT_PIPE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 2, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SbInit(), -1);

    /* Verify results */
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.CmdPipe, PipeId));
    UtAssert_BOOL_FALSE(CFE_RESOURCEID_TEST_DEFINED(HS_AppData.EventPipe));
    UtAssert_BOOL_FALSE(CFE_RESOURCEID_TEST_DEFINED(HS_AppData.WakeupPipe));

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CR_EVENT_PIPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SbInit_Test_CreateSBWakeupPipe(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Creating SB Wakeup Pipe,RC=0x%%08X");

    CFE_SB_PipeId_t PipeId = HS_UT_PIPEID_1;

    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);

    /* Set CFE_SB_CreatePipe to return -1 on third call, to generate error HS_CR_WAKEUP_PIPE_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_CreatePipe), 3, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SbInit(), -1);

    /* Verify results */
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.CmdPipe, PipeId));
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.EventPipe, PipeId));
    UtAssert_BOOL_FALSE(CFE_RESOURCEID_TEST_DEFINED(HS_AppData.WakeupPipe));

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CR_WAKEUP_PIPE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SbInit_Test_SubscribeHKRequestError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Subscribing to HK Request,RC=0x%%08X");

    CFE_SB_PipeId_t PipeId = HS_UT_PIPEID_1;

    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);

    /* Set CFE_SB_Subscribe to return -1 on first call, to generate error HS_SUB_REQ_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 1, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SbInit(), -1);

    /* Verify results */
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.CmdPipe, PipeId));
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.EventPipe, PipeId));
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.WakeupPipe, PipeId));

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SUB_REQ_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SbInit_Test_SubscribeGndCmdsError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Subscribing to Gnd Cmds,RC=0x%%08X");

    CFE_SB_PipeId_t PipeId = HS_UT_PIPEID_1;

    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);

    /* Set CFE_SB_Subscribe to return -1 on second call, to generate error HS_SUB_CMD_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 2, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SbInit(), -1);

    /* Verify results */
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.CmdPipe, PipeId));
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.EventPipe, PipeId));
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.WakeupPipe, PipeId));

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SUB_CMD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SbInit_Test_SubscribeWakeupError(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Subscribing to Wakeup,RC=0x%%08X");

    CFE_SB_PipeId_t PipeId = HS_UT_PIPEID_1;

    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);
    UT_SetDataBuffer(UT_KEY(CFE_SB_CreatePipe), &PipeId, sizeof(PipeId), false);

    /* Set CFE_SB_Subscribe to return -1 on third call, to generate error HS_SUB_WAKEUP_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Subscribe), 3, -1);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SbInit(), -1);

    /* Verify results */
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.CmdPipe, PipeId));
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.EventPipe, PipeId));
    UtAssert_BOOL_TRUE(CFE_RESOURCEID_TEST_EQUAL(HS_AppData.WakeupPipe, PipeId));

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SUB_WAKEUP_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_TblInit_Test_Nominal(void)
{
    CFE_Status_t Result;

    /* Same return value as default, but bypasses default hook function to make test easier to write */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    Result = HS_TblInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_TblInit_Test_RegisterAppMonTableError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Registering AppMon Table,RC=0x%%08X");

    /* Set CFE_TBL_Register to return -1 on first call, to generate error HS_AMT_REG_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 1, -1);

    /* Same return value as default, but bypasses default hook function to make test easier to write */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    Result = HS_TblInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMT_REG_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_TblInit_Test_RegisterEventMonTableError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Registering EventMon Table,RC=0x%%08X");

    /* Set CFE_TBL_Register to return -1 on second call, to generate error HS_EMT_REG_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 2, -1);

    /* Same return value as default, but bypasses default hook function to make test easier to write */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    Result = HS_TblInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMT_REG_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_TblInit_Test_RegisterMsgActsTableError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Registering MsgActs Table,RC=0x%%08X");

    /* Set CFE_TBL_Register to return -1 on third call, to generate error HS_MAT_REG_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 3, -1);

    /* Same return value as default, but bypasses default hook function to make test easier to write */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    Result = HS_TblInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MAT_REG_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_TblInit_Test_RegisterExeCountTableError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Registering ExeCount Table,RC=0x%%08X");

    /* Set CFE_TBL_Register to return -1 on fourth call, to generate error HS_XCT_REG_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_TBL_Register), 4, -1);

    /* Same return value as default, but bypasses default hook function to make test easier to write */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), CFE_SUCCESS);

    /* Execute the function being tested */
    Result = HS_TblInit();

    /* Verify results */
    UtAssert_True(Result == -1, "Result == -1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCT_REG_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_TblInit_Test_LoadExeCountTableError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Loading ExeCount Table,RC=0x%%08X");

    /* Set CFE_TBL_Load to fail on first call, to generate error HS_XCT_LD_ERR_EID */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_Load), -1);

    /* Execute the function being tested */
    Result = HS_TblInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");
    /* Note: not verifying that HS_AppData.ExeCountState == HS_STATE_DISABLED, because HS_AppData.ExeCountState is
     * modified by HS_AcquirePointers at the end of HS_TblInit */

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCT_LD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 6, "CFE_EVS_SendEvent was called %u time(s), expected 6",
                  call_count_CFE_EVS_SendEvent);
}

void HS_TblInit_Test_LoadAppMonTableError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Loading AppMon Table,RC=0x%%08X");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Application Monitoring Disabled due to Table Load Failure");

    /* Set CFE_TBL_Load to fail on second call, to generate error HS_AMT_LD_ERR_EID */
    HS_APP_TEST_CFE_TBL_LoadHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_Load), HS_APP_TEST_CFE_TBL_LoadHook1, NULL);

    /* Execute the function being tested */
    Result = HS_TblInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");
    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_DISABLED");
    /* Note: not verifying that HS_AppData.AppMonLoaded == HS_STATE_DISABLED, because HS_AppData.AppMonLoaded is
     * modified by HS_AcquirePointers at the end of HS_TblInit */

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMT_LD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_DISABLE_APPMON_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_TblInit_Test_LoadEventMonTableError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Loading EventMon Table,RC=0x%%08X");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitoring Disabled due to Table Load Failure");

    /* Set CFE_TBL_Load to fail on third call, to generate error HS_EMT_LD_ERR_EID */
    HS_APP_TEST_CFE_TBL_LoadHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_Load), HS_APP_TEST_CFE_TBL_LoadHook2, NULL);

    /* Execute the function being tested */
    Result = HS_TblInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");
    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");
    /* Note: not verifying that HS_AppData.EventMonLoaded == HS_STATE_DISABLED, because HS_AppData.EventMonLoaded is
     * modified by HS_AcquirePointers at the end of HS_TblInit */

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMT_LD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_DISABLE_EVENTMON_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_TblInit_Test_LoadMsgActsTableError(void)
{
    CFE_Status_t Result;
    int32        strCmpResult;
    char         ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Error Loading MsgActs Table,RC=0x%%08X");

    /* Set CFE_TBL_Load to fail on fourth call, to generate error HS_MAT_LD_ERR_EID */
    HS_APP_TEST_CFE_TBL_LoadHookCount = 0;
    UT_SetHookFunction(UT_KEY(CFE_TBL_Load), HS_APP_TEST_CFE_TBL_LoadHook3, NULL);

    /* Execute the function being tested */
    Result = HS_TblInit();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");
    /* Note: not verifying that HS_AppData.MsgActsState == HS_STATE_DISABLED, because HS_AppData.MsgActsState is
     * modified by HS_AcquirePointers at the end of HS_TblInit */

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MAT_LD_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ProcessMain_Test(void)
{
    CFE_Status_t Result;

    HS_AMTEntry_t AMTable;

    HS_AppData.AMTablePtr = &AMTable;

    /* Prevents error messages in call to HS_AcquirePointers */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_SUCCESS);

    /* Causes HS_ProcessCommands to return CFE_SUCCESS, which is then returned from HS_ProcessMain */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_NO_MESSAGE);

    HS_AppData.MsgActCooldown[0]                        = 2;
    HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] = 2;
    HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] = 2;

    HS_AppData.CurrentAppMonState    = HS_STATE_ENABLED;
    HS_AppData.CurrentAlivenessState = HS_STATE_ENABLED;
    HS_AppData.AlivenessCounter      = HS_CPU_ALIVE_PERIOD;
    HS_AppData.ServiceWatchdogFlag   = HS_STATE_ENABLED;

    /* Execute the function being tested */
    Result = HS_ProcessMain();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    /* Check first, middle, and last element */
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 1, "HS_AppData.MsgActCooldown[0] == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 1,
                  "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] == 1,
                  "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] == 1");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");

    /* Ensure the watchdog was serviced when flag is HS_STATE_ENABLED */
    UtAssert_STUB_COUNT(CFE_PSP_WatchdogService, 1);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ProcessMain_Test_MonStateDisabled(void)
{
    CFE_Status_t Result;

    HS_AMTEntry_t AMTable;

    HS_AppData.AMTablePtr = &AMTable;

    /* Prevents error messages in call to HS_AcquirePointers */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_SUCCESS);

    /* Causes HS_ProcessCommands to return CFE_SUCCESS, which is then returned from HS_ProcessMain */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_NO_MESSAGE);

    HS_AppData.MsgActCooldown[0]                        = 2;
    HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] = 2;
    HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] = 2;

    HS_AppData.CurrentAppMonState    = HS_STATE_ENABLED;
    HS_AppData.CurrentAlivenessState = HS_STATE_ENABLED;
    HS_AppData.AlivenessCounter      = HS_CPU_ALIVE_PERIOD;
    HS_AppData.ServiceWatchdogFlag   = HS_STATE_ENABLED;

    /* Prevent HS_MonitorApplications() from executing */
    HS_AppData.CurrentAppMonState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    Result = HS_ProcessMain();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    /* Check first, middle, and last element */
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 1, "HS_AppData.MsgActCooldown[0] == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 1,
                  "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] == 1,
                  "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] == 1");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ProcessMain_Test_AlivenessDisabled(void)
{
    CFE_Status_t Result;

    HS_AMTEntry_t AMTable;

    HS_AppData.AMTablePtr = &AMTable;

    /* Prevents error messages in call to HS_AcquirePointers */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_SUCCESS);

    /* Causes HS_ProcessCommands to return CFE_SUCCESS, which is then returned from HS_ProcessMain */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_NO_MESSAGE);

    HS_AppData.MsgActCooldown[0]                        = 2;
    HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] = 2;
    HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] = 2;

    HS_AppData.CurrentAppMonState    = HS_STATE_ENABLED;
    HS_AppData.CurrentAlivenessState = HS_STATE_ENABLED;
    HS_AppData.AlivenessCounter      = HS_CPU_ALIVE_PERIOD;
    HS_AppData.CurrentAlivenessState = HS_STATE_DISABLED;
    HS_AppData.ServiceWatchdogFlag   = HS_STATE_ENABLED;

    /* Execute the function being tested */
    Result = HS_ProcessMain();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    /* Check first, middle, and last element */
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 1, "HS_AppData.MsgActCooldown[0] == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 1,
                  "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] == 1,
                  "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] == 1");
    UtAssert_True(HS_AppData.AlivenessCounter == HS_CPU_ALIVE_PERIOD,
                  "HS_AppData.AlivenessCounter == HS_CPU_ALIVE_PERIOD");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ProcessMain_Test_WatchdogDisabled(void)
{
    CFE_Status_t Result;

    HS_AMTEntry_t AMTable;

    HS_AppData.AMTablePtr = &AMTable;

    /* Prevents error messages in call to HS_AcquirePointers */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_SUCCESS);

    /* Causes HS_ProcessCommands to return CFE_SUCCESS, which is then returned from HS_ProcessMain */
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SB_NO_MESSAGE);

    HS_AppData.MsgActCooldown[0]                        = 2;
    HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] = 2;
    HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] = 2;

    HS_AppData.CurrentAppMonState    = HS_STATE_ENABLED;
    HS_AppData.CurrentAlivenessState = HS_STATE_ENABLED;
    HS_AppData.AlivenessCounter      = HS_CPU_ALIVE_PERIOD;
    HS_AppData.ServiceWatchdogFlag   = HS_STATE_ENABLED;

    HS_AppData.ServiceWatchdogFlag = HS_STATE_DISABLED;

    /* Execute the function being tested */
    Result = HS_ProcessMain();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    /* Check first, middle, and last element */
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 1, "HS_AppData.MsgActCooldown[0] == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 1,
                  "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] == 1,
                  "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] == 1");
    UtAssert_True(HS_AppData.AlivenessCounter == 0, "HS_AppData.AlivenessCounter == 0");

    /* Ensure the watchdog was not serviced when flag is HS_STATE_DISABLED */
    UtAssert_STUB_COUNT(CFE_PSP_WatchdogService, 0);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ProcessCommands_Test(void)
{
    CFE_Status_t  Result;
    uint32        i;
    uint8         call_count_CFE_SB_ReceiveBuffer = 0;
    uint8         call_count_HS_AppPipe           = 0;
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];
    uint8         call_count_HS_MonitorEvent;

    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;
    HS_AppData.EMTablePtr           = EMTable;

    HS_AppData.EventsMonitoredCount = 0;

    /* This loop is to prevent errors within one of the subfunctions not being tested here */
    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].EventID = -1;
    }

    /* Causes CFE_SB_RcvMsg to alternate returning CFE_SUCCESS and CFE_SB_NO_MESSAGE, to reach all code branches. */
    HS_APP_TEST_CFE_SB_RcvMsgHookCount = 0;
    CFE_SB_Buffer_t *dummy_BufPtr[4];
    CFE_SB_Buffer_t  dummy_Buf[4];
    dummy_BufPtr[0] = &dummy_Buf[0];
    dummy_BufPtr[1] = &dummy_Buf[1];
    dummy_BufPtr[2] = &dummy_Buf[2];
    dummy_BufPtr[3] = &dummy_Buf[3];
    UT_SetDataBuffer(UT_KEY(CFE_SB_ReceiveBuffer), dummy_BufPtr, sizeof(dummy_BufPtr), false);
    UT_SetHandlerFunction(UT_KEY(CFE_SB_ReceiveBuffer), UT_UpdatedDefaultHandler_CFE_SB_ReceiveBuffer, NULL);
    UT_SetDefaultReturnValue(UT_KEY(CFE_SB_ReceiveBuffer), CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 2, CFE_SB_NO_MESSAGE);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 2, CFE_SB_NO_MESSAGE);
    /* UT_SetHookFunction(UT_KEY(CFE_SB_ReceiveBuffer), HS_APP_TEST_CFE_SB_RcvMsgHook, NULL); */

    /* Causes check for non-null buffer pointer to succeed */
    /* UT_SetDataBuffer(UT_KEY(CFE_SB_ReceiveBuffer), &Packet, sizeof(Packet), false); */

    /* Execute the function being tested */
    Result = HS_ProcessCommands();

    /* Verify results */
    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 1, "HS_AppData.EventsMonitoredCount == 1");

    call_count_CFE_SB_ReceiveBuffer = UT_GetStubCount(UT_KEY(CFE_SB_ReceiveBuffer));

    UtAssert_True(call_count_CFE_SB_ReceiveBuffer == 4, "CFE_SB_ReceiveBuffer was called %u time(s), expected 4",
                  call_count_CFE_SB_ReceiveBuffer);

    call_count_HS_AppPipe = UT_GetStubCount(UT_KEY(HS_AppPipe));

    UtAssert_True(call_count_HS_AppPipe == 1, "HS_AppPipe was called %u time(s), expected 1", call_count_HS_AppPipe);

    call_count_HS_MonitorEvent = UT_GetStubCount(UT_KEY(HS_MonitorEvent));

    UtAssert_True(call_count_HS_MonitorEvent == 1, "HS_MonitorEvent was called %u time(s), expected 1",
                  call_count_HS_MonitorEvent);

    UtAssert_True(HS_AppData.CmdErrCount == 0, "HS_AppData.CmdErrCount == 0");
}

void HS_ProcessCommands_Test_NullMsgPtr(void)
{
    CFE_Status_t  Result;
    uint32        i;
    uint8         call_count_CFE_SB_ReceiveBuffer = 0;
    uint8         call_count_HS_AppPipe           = 0;
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;
    HS_AppData.EMTablePtr           = EMTable;

    HS_AppData.EventsMonitoredCount = 0;

    /* This loop is to prevent errors within one of the subfunctions not being tested here */
    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].EventID = -1;
    }

    /* Causes CFE_SB_RcvMsg to alternate returning CFE_SUCCESS and CFE_SB_NO_MESSAGE, to reach all code branches. */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_NO_MESSAGE);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_ReceiveBuffer), 1, CFE_SB_TIME_OUT);

    /* Execute the function being tested */
    Result = HS_ProcessCommands();

    /* Verify results */
    UtAssert_True(Result == CFE_SB_TIME_OUT, "Result == CFE_SB_TIME_OUT");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");

    call_count_CFE_SB_ReceiveBuffer = UT_GetStubCount(UT_KEY(CFE_SB_ReceiveBuffer));

    UtAssert_True(call_count_CFE_SB_ReceiveBuffer == 4, "CFE_SB_ReceiveBuffer was called %u time(s), expected 4",
                  call_count_CFE_SB_ReceiveBuffer);

    call_count_HS_AppPipe = UT_GetStubCount(UT_KEY(HS_AppPipe));

    UtAssert_True(call_count_HS_AppPipe == 0, "HS_AppPipe was called %u time(s), expected 0", call_count_HS_AppPipe);

    UtAssert_True(HS_AppData.CmdErrCount == 0, "HS_AppData.CmdErrCount == 0");
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_AppMain_Test_NominalWaitForStartupSync, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMain_Test_NominalWaitForStartupSync");
    UtTest_Add(HS_AppMain_Test_NominalRcvMsgSuccess, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMain_Test_NominalRcvMsgSuccess");
    UtTest_Add(HS_AppMain_Test_NominalRcvMsgNoMessage, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMain_Test_NominalRcvMsgNoMessage");
    UtTest_Add(HS_AppMain_Test_NominalRcvMsgTimeOut, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMain_Test_NominalRcvMsgTimeOut");
    UtTest_Add(HS_AppMain_Test_NominalRcvMsgError, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMain_Test_NominalRcvMsgError");
    UtTest_Add(HS_AppMain_Test_AppInitNotSuccess, HS_Test_Setup, HS_Test_TearDown, "HS_AppMain_Test_AppInitNotSuccess");
    UtTest_Add(HS_AppMain_Test_ProcessMainNotSuccess, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMain_Test_ProcessMainNotSuccess");
    UtTest_Add(HS_AppMain_Test_SBSubscribeEVSLongError, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMain_Test_SBSubscribeEVSLongError");
    UtTest_Add(HS_AppMain_Test_SBSubscribeEVSShortError, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMain_Test_SBSubscribeEVSShortError");
    UtTest_Add(HS_AppMain_Test_RcvMsgError, HS_Test_Setup, HS_Test_TearDown, "HS_AppMain_Test_RcvMsgError");
    UtTest_Add(HS_AppMain_Test_StateDisabled, HS_Test_Setup, HS_Test_TearDown, "HS_AppMain_Test_StateDisabled");

    UtTest_Add(HS_AppInit_Test_EVSRegisterError, HS_Test_Setup, HS_Test_TearDown, "HS_AppInit_Test_EVSRegisterError");
    UtTest_Add(HS_AppInit_Test_Nominal, HS_Test_Setup, HS_Test_TearDown, "HS_AppInit_Test_Nominal");
    UtTest_Add(HS_AppInit_Test_CorruptCDSResetsPerformed, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppInit_Test_CorruptCDSResetsPerformed");
    UtTest_Add(HS_AppInit_Test_CorruptCDSMaxResets, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppInit_Test_CorruptCDSMaxResets");
    UtTest_Add(HS_AppInit_Test_CorruptCDSNoEvent, HS_Test_Setup, HS_Test_TearDown, "HS_AppInit_Test_CorruptCDSNoEvent");
    UtTest_Add(HS_AppInit_Test_RestoreCDSError, HS_Test_Setup, HS_Test_TearDown, "HS_AppInit_Test_RestoreCDSError");
    UtTest_Add(HS_AppInit_Test_DisableSavingToCDS, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppInit_Test_DisableSavingToCDS");
    UtTest_Add(HS_AppInit_Test_SBInitError, HS_Test_Setup, HS_Test_TearDown, "HS_AppInit_Test_SBInitError");
    UtTest_Add(HS_AppInit_Test_TblInitError, HS_Test_Setup, HS_Test_TearDown, "HS_AppInit_Test_TblInitError");
    UtTest_Add(HS_AppInit_Test_CustomInitError, HS_Test_Setup, HS_Test_TearDown, "HS_AppInit_Test_CustomInitError");

    UtTest_Add(HS_SbInit_Test_Nominal, HS_Test_Setup, HS_Test_TearDown, "HS_SbInit_Test_Nominal");
    UtTest_Add(HS_SbInit_Test_CreateSBCmdPipeError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SbInit_Test_CreateSBCmdPipeError");
    UtTest_Add(HS_SbInit_Test_CreateSBEventPipeError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SbInit_Test_CreateSBEventPipeError");
    UtTest_Add(HS_SbInit_Test_CreateSBWakeupPipe, HS_Test_Setup, HS_Test_TearDown, "HS_SbInit_Test_CreateSBWakeupPipe");
    UtTest_Add(HS_SbInit_Test_SubscribeHKRequestError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SbInit_Test_SubscribeHKRequestError");
    UtTest_Add(HS_SbInit_Test_SubscribeGndCmdsError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SbInit_Test_SubscribeGndCmdsError");
    UtTest_Add(HS_SbInit_Test_SubscribeWakeupError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SbInit_Test_SubscribeWakeupError");

    UtTest_Add(HS_TblInit_Test_Nominal, HS_Test_Setup, HS_Test_TearDown, "HS_TblInit_Test_Nominal");
    UtTest_Add(HS_TblInit_Test_RegisterAppMonTableError, HS_Test_Setup, HS_Test_TearDown,
               "HS_TblInit_Test_RegisterAppMonTableError");
    UtTest_Add(HS_TblInit_Test_RegisterEventMonTableError, HS_Test_Setup, HS_Test_TearDown,
               "HS_TblInit_Test_RegisterEventMonTableError");
    UtTest_Add(HS_TblInit_Test_RegisterMsgActsTableError, HS_Test_Setup, HS_Test_TearDown,
               "HS_TblInit_Test_RegisterMsgActsTableError");
    UtTest_Add(HS_TblInit_Test_RegisterExeCountTableError, HS_Test_Setup, HS_Test_TearDown,
               "HS_TblInit_Test_RegisterExeCountTableError");
    UtTest_Add(HS_TblInit_Test_LoadExeCountTableError, HS_Test_Setup, HS_Test_TearDown,
               "HS_TblInit_Test_LoadExeCountTableError");
    UtTest_Add(HS_TblInit_Test_LoadAppMonTableError, HS_Test_Setup, HS_Test_TearDown,
               "HS_TblInit_Test_LoadAppMonTableError");
    UtTest_Add(HS_TblInit_Test_LoadEventMonTableError, HS_Test_Setup, HS_Test_TearDown,
               "HS_TblInit_Test_LoadEventMonTableError");
    UtTest_Add(HS_TblInit_Test_LoadMsgActsTableError, HS_Test_Setup, HS_Test_TearDown,
               "HS_TblInit_Test_LoadMsgActsTableError");

    UtTest_Add(HS_ProcessMain_Test, HS_Test_Setup, HS_Test_TearDown, "HS_ProcessMain_Test");
    UtTest_Add(HS_ProcessMain_Test_MonStateDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_ProcessMain_Test_MonStateDisabled");
    UtTest_Add(HS_ProcessMain_Test_AlivenessDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_ProcessMain_Test_AlivenessDisabled");
    UtTest_Add(HS_ProcessMain_Test_WatchdogDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_ProcessMain_Test_WatchdogDisabled");

    UtTest_Add(HS_ProcessCommands_Test, HS_Test_Setup, HS_Test_TearDown, "HS_ProcessCommands_Test");
    UtTest_Add(HS_ProcessCommands_Test_NullMsgPtr, HS_Test_Setup, HS_Test_TearDown,
               "HS_ProcessCommands_Test_NullMsgPtr");
}
