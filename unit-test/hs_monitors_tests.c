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

#include "hs_monitors.h"
#include "hs_sysmon.h"
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
#include "cfe_evs_msg.h"

/* hs_monitors_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

int32 HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                              const UT_StubContext_t *Context)
{
    CFE_ES_AppInfo_t *AppInfo = UserObj;

    AppInfo->ExecutionCounter = 3;

    return CFE_SUCCESS;
}

void HS_MonitorApplications_Test_AppNameNotFound(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "App Monitor App Name not found: APP:(%%s)");

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    /* Element 0 will run through logic with action and not expired */
    HS_AppData.AMTablePtr[0].ActionType  = -1;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;

    /* Element 1 has action but expired */
    HS_AppData.AMTablePtr[1].ActionType = -1;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to fail on first call, to generate error HS_APPMON_APPNAME_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_APPNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_AppNameNotFoundDebugEvent(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType  = -1;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 2;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to fail on first call, to generate error HS_APPMON_APPNAME_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_APPNAME_DBG_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_GetExeCountFailure(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t AppInfo;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType  = -1;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 2;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Causes "failure to get an execution counter" */
    AppInfo.ExecutionCounter = 3;
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetAppInfo), &AppInfo, sizeof(AppInfo), false);
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 2, "HS_AppData.AppMonCheckInCountdown[0] == 2");

    /* Execution count does not get updated from AppInfo */
    UtAssert_True(HS_AppData.AppMonLastExeCount[0] == 0, "HS_AppData.AppMonLastExeCount[0] == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_ProcessorResetError(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t AppInfo;
    int32            strCmpResult;
    char             ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    char             ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s): Action: Processor Reset");
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: App Monitor Failure: APP:(%%s): Action: Processor Reset\n");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_PROC_RESET;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.CDSData.MaxResets       = 10;
    HS_AppData.CDSData.ResetsPerformed = 1;

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0 %u",
                  HS_AppData.AppMonCheckInCountdown[0]);
    UtAssert_UINT32_EQ(HS_AppData.AppMonEnables[0], 0);
    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_DISABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_PROC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);
}

void HS_MonitorApplications_Test_ProcessorResetActionLimitError(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t AppInfo;
    int32            strCmpResult;
    char             ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s): Action: Processor Reset");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Processor Reset Action Limit Reached: No Reset Performed");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_PROC_RESET;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.CDSData.MaxResets       = 10;
    HS_AppData.CDSData.ResetsPerformed = 11;

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_PROC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_RESET_LIMIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_RestartAppErrorsGetAppInfoSuccess(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t AppInfo;
    int32            strCmpResult;
    char             ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s) Action: Restart Application");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Restart App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_APP_RESTART;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);

    /* Set CFE_ES_RestartApp to fail on first call, to generate error HS_APPMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestartApp), 1, 0xFFFFFFFF);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_APPMON_NOT_RESTARTED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_RestartAppErrorsGetAppInfoNotSuccess(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s) Action: Restart Application");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Restart App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_APP_RESTART;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;

    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Set CFE_ES_GetAppInfo to fail on first call, to generate error HS_APPMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, -1);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestartApp), 1, -1);

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_APPMON_NOT_RESTARTED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_RestartAppRestartSuccess(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_APP_RESTART;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;

    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Set CFE_ES_GetAppInfo to succeed on first call and avoid error HS_APPMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestartApp), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_FailError(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t AppInfo;
    int32            strCmpResult;
    char             ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s): Action: Event Only");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_EVENT;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_FAIL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_MsgActsNOACT(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t    MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_ES_AppInfo_t AppInfo;

    memset(AMTable, 0, sizeof(AMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, CFE_SB_ValueToMsgId(HS_CMD_MID),
                 sizeof(HS_NoopCmd_t));
    CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, HS_NOOP_CC);

    HS_AppData.AMTablePtr = AMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.AMTablePtr[0].ActionType =
        HS_AMT_ACT_NOACT; /* Causes most of the function to be skipped, due to first if-statement */
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;
    HS_AppData.MsgActsState              = HS_STATE_ENABLED;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.AppMonLastExeCount[0] = 3;

    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.MsgActCooldown[0]         = 0; /* (HS_AMT_ACT_LAST_NONMSG + 1) - HS_AMT_ACT_LAST_NONMSG - 1 = 0 */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_ENABLED;
    HS_AppData.MATablePtr[0].Cooldown    = 1;

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_MsgActsNOACTDisabled(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t    MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_ES_AppInfo_t AppInfo;

    memset(AMTable, 0, sizeof(AMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, CFE_SB_ValueToMsgId(HS_CMD_MID),
                 sizeof(HS_NoopCmd_t));
    CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, HS_NOOP_CC);

    HS_AppData.AMTablePtr = AMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.AMTablePtr[0].ActionType =
        HS_AMT_ACT_NOACT; /* Causes most of the function to be skipped, due to first if-statement */
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;
    HS_AppData.MsgActsState              = HS_STATE_DISABLED;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.AppMonLastExeCount[0] = 3;

    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.MsgActCooldown[0]         = 0; /* (HS_AMT_ACT_LAST_NONMSG + 1) - HS_AMT_ACT_LAST_NONMSG - 1 = 0 */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_ENABLED;
    HS_AppData.MATablePtr[0].Cooldown    = 1;

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_MsgActsErrorDefault(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t    MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_ES_AppInfo_t AppInfo;
    int32            strCmpResult;
    char             ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s): Action: Message Action Index: %%d");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, CFE_SB_ValueToMsgId(HS_CMD_MID),
                 sizeof(HS_NoopCmd_t));
    CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, HS_NOOP_CC);

    HS_AppData.AMTablePtr = AMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_LAST_NONMSG + 1;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;
    HS_AppData.MsgActsState              = HS_STATE_ENABLED;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.MsgActCooldown[0]         = 0; /* (HS_AMT_ACT_LAST_NONMSG + 1) - HS_AMT_ACT_LAST_NONMSG - 1 = 0 */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_ENABLED;
    HS_AppData.MATablePtr[0].Cooldown    = 1;

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_True(HS_AppData.MsgActExec == 1, "HS_AppData.MsgActExec == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 1, "HS_AppData.MsgActCooldown[0] == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_MSGACTS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_MsgActsErrorDisabled(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t    MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_ES_AppInfo_t AppInfo;

    memset(AMTable, 0, sizeof(AMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, CFE_SB_ValueToMsgId(HS_CMD_MID),
                 sizeof(HS_NoopCmd_t));
    CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, HS_NOOP_CC);

    HS_AppData.AMTablePtr = AMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;
    HS_AppData.MsgActsState              = HS_STATE_ENABLED;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.MsgActCooldown[0]         = 0; /* (HS_AMT_ACT_LAST_NONMSG + 1) - HS_AMT_ACT_LAST_NONMSG - 1 = 0 */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_ENABLED;
    HS_AppData.MATablePtr[0].Cooldown    = 1;

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 0, "HS_AppData.MsgActCooldown[0] == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_MsgActsErrorDefaultCoolDown(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t    MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_ES_AppInfo_t AppInfo;

    memset(AMTable, 0, sizeof(AMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, CFE_SB_ValueToMsgId(HS_CMD_MID),
                 sizeof(HS_NoopCmd_t));
    CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, HS_NOOP_CC);

    HS_AppData.AMTablePtr = AMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_LAST_NONMSG + 1;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;
    HS_AppData.MsgActsState              = HS_STATE_ENABLED;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.MsgActCooldown[0]         = 1; /* (HS_AMT_ACT_LAST_NONMSG + 1) - HS_AMT_ACT_LAST_NONMSG - 1 = 0 */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_ENABLED;
    HS_AppData.MATablePtr[0].Cooldown    = 0;

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 1, "HS_AppData.MsgActCooldown[0] == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_MsgActsErrorDefaultDisabled(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t    MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_ES_AppInfo_t AppInfo;

    memset(AMTable, 0, sizeof(AMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, CFE_SB_ValueToMsgId(HS_CMD_MID),
                 sizeof(HS_NoopCmd_t));
    CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, HS_NOOP_CC);

    HS_AppData.AMTablePtr = AMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_LAST_NONMSG + 1;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;
    HS_AppData.MsgActsState              = HS_STATE_ENABLED;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.MsgActCooldown[0]         = 0; /* (HS_AMT_ACT_LAST_NONMSG + 1) - HS_AMT_ACT_LAST_NONMSG - 1 = 0 */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_DISABLED;
    HS_AppData.MATablePtr[0].Cooldown    = 0;

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0s");
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 0, "HS_AppData.MsgActCooldown[0] == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_Test_MsgActsErrorDefaultNoEvent(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t    MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_ES_AppInfo_t AppInfo;

    memset(AMTable, 0, sizeof(AMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, CFE_SB_ValueToMsgId(HS_CMD_MID),
                 sizeof(HS_NoopCmd_t));
    CFE_MSG_SetFcnCode((CFE_MSG_Message_t *)&HS_AppData.MATablePtr[0].MsgBuf, HS_NOOP_CC);

    HS_AppData.AMTablePtr = AMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_LAST_NONMSG + 1;
    HS_AppData.AppMonCheckInCountdown[0] = 1;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;
    HS_AppData.MsgActsState              = HS_STATE_ENABLED;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.MsgActCooldown[0]         = 0; /* (HS_AMT_ACT_LAST_NONMSG + 1) - HS_AMT_ACT_LAST_NONMSG - 1 = 0 */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_NOEVENT;
    HS_AppData.MATablePtr[0].Cooldown    = 1;

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_True(HS_AppData.MsgActExec == 1, "HS_AppData.MsgActExec == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 1, "HS_AppData.MsgActCooldown[0] == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorApplications_CheckInCountdownNotZero(void)
{
    HS_AMTEntry_t    AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t AppInfo;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType  = HS_AMT_ACT_EVENT;
    HS_AppData.AppMonCheckInCountdown[0] = 2;
    HS_AppData.AppMonLastExeCount[0]     = 0;
    HS_AppData.AppMonEnables[0]          = 1;
    HS_AppData.AMTablePtr[0].CycleCount  = 1;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_MonitorApplications();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 1, "HS_AppData.AppMonCheckInCountdown[0] == 1");
    UtAssert_True(HS_AppData.AppMonEnables[0] == 1, "HS_AppData.AppMonEnables[0] == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_AppName(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));
    memset(&Packet, 0, sizeof(Packet));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    /* Active table entry where AppName does not match event */
    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_PROC_RESET;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;
    HS_AppData.CDSData.MaxResets        = 10;
    HS_AppData.CDSData.ResetsPerformed  = 1;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);

    /* Active table entry where event doesn't match */
    HS_AppData.EMTablePtr[1].ActionType = HS_EMT_ACT_PROC_RESET;

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_ProcErrorReset(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    char                   ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Processor Reset");
    snprintf(ExpectedSysLogString, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: Event Monitor: APP:(%%s) EID:(%%d): Action: Processor Reset\n");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_PROC_RESET;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;
    HS_AppData.CDSData.MaxResets        = 10;
    HS_AppData.CDSData.ResetsPerformed  = 1;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_PROC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_STATE_DISABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_STATE_DISABLED");
}

void HS_MonitorEvent_Test_ProcErrorNoReset(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Processor Reset");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Processor Reset Action Limit Reached: No Reset Performed");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_PROC_RESET;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;
    HS_AppData.CDSData.MaxResets        = 10;
    HS_AppData.CDSData.ResetsPerformed  = 11;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_PROC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_RESET_LIMIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_AppRestartErrors(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Restart Application");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Restart App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_APP_RESTART;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_RestartApp to return -1, in order to generate error message HS_EVENTMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestartApp), 1, -1);

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_EVENTMON_NOT_RESTARTED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_OnlySecondAppRestartError(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Restart App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_APP_RESTART;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to return -1, in order to generate error message HS_EVENTMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_NOT_RESTARTED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_NoSecondAppRestartError(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Restart Application");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_APP_RESTART;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to succeed on first call and bypass error HS_EVENTMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_DeleteErrors(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Delete Application");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Delete App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_APP_DELETE;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_DeleteApp to return -1, in order to generate error message HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_DeleteApp), 1, -1);

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_DELETE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_EVENTMON_NOT_DELETED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_OnlySecondDeleteError(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Delete App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_APP_DELETE;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to fail on first call, to generate error HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_NOT_DELETED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_NoSecondDeleteError(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Delete Application");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_APP_DELETE;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to succeed on first call and bypass error HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_DELETE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_MsgActsError(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Message Action Index: %%d");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_LAST_NONMSG + 1;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_DeleteApp to return -1, in order to generate error message HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_DeleteApp), 1, -1);

    HS_AppData.MsgActsState              = HS_STATE_ENABLED;
    HS_AppData.MsgActCooldown[0]         = 0;
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_ENABLED;
    HS_AppData.MATablePtr[0].Cooldown    = 5;

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_MSGACTS_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(HS_AppData.MsgActExec == 1, "HS_AppData.MsgActExec == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 5, "HS_AppData.MsgActCooldown[0] == 5");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_MsgActsErrorNoEvent(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_LAST_NONMSG + 1;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_DeleteApp to return -1, in order to generate error message HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_DeleteApp), 1, -1);

    HS_AppData.MsgActsState      = HS_STATE_ENABLED;
    HS_AppData.MsgActCooldown[0] = 0;

    /* take branch to avoid HS_EVENTMON_MSGACTS_ERR_EID event error */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_NOEVENT;

    HS_AppData.MATablePtr[0].Cooldown = 5;

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    UtAssert_True(HS_AppData.MsgActExec == 1, "HS_AppData.MsgActExec == 1");
    UtAssert_True(HS_AppData.MsgActCooldown[0] == 5, "HS_AppData.MsgActCooldown[0] == 5");
}

void HS_MonitorEvent_Test_MsgActsDefaultDisabled(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_DeleteApp to return -1, in order to generate error message HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_DeleteApp), 1, -1);

    HS_AppData.MsgActsState      = HS_STATE_ENABLED;
    HS_AppData.MsgActCooldown[0] = 0;

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_MsgActsDefaultGreaterLastNonMsg(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_DeleteApp to return -1, in order to generate error message HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_DeleteApp), 1, -1);

    HS_AppData.MsgActsState      = HS_STATE_ENABLED;
    HS_AppData.MsgActCooldown[0] = 0;

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_MsgActsDefaultLessMaxActTypes(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    HS_AppData.MsgActsState      = HS_STATE_DISABLED;
    HS_AppData.MsgActCooldown[0] = 1;

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_MsgActsDefaultMaxActTypes(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    uint32                 MsgActIndex;

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType            = HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES;
    HS_AppData.EMTablePtr[0].EventID               = Packet.Payload.PacketID.EventID;
    MsgActIndex                                    = HS_AppData.EMTablePtr[0].ActionType - HS_EMT_ACT_LAST_NONMSG - 1;
    HS_AppData.MATablePtr[MsgActIndex].EnableState = HS_MAT_STATE_ENABLED;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    HS_AppData.MsgActsState                = HS_STATE_ENABLED;
    HS_AppData.MsgActCooldown[MsgActIndex] = 0;

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    /* 1 event message that we don't care about in this test */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_MsgActsCoolDown(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_LAST_NONMSG + 1;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    HS_AppData.MsgActsState      = HS_STATE_ENABLED;
    HS_AppData.MsgActCooldown[0] = 1;

    /* take branch to avoid "Send the message if off cooldown and not disabled" */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_NOEVENT;

    HS_AppData.MATablePtr[0].Cooldown = 5;

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorEvent_Test_MsgActsMATDisabled(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_LAST_NONMSG + 1;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    HS_AppData.MsgActsState      = HS_STATE_ENABLED;
    HS_AppData.MsgActCooldown[0] = 0;

    /* take branch to avoid "Send the message if off cooldown and not disabled" */
    HS_AppData.MATablePtr[0].EnableState = HS_MAT_STATE_DISABLED;

    HS_AppData.MATablePtr[0].Cooldown = 5;

    /* Execute the function being tested */
    HS_MonitorEvent(&Packet);

    /* Verify results */
    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorUtilization_Test_HighCurrentUtil(void)
{
    HS_AppData.CurrentCPUUtilIndex = HS_UTIL_PEAK_NUM_INTERVAL - 2;

    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_CPU_UTILIZATION_MAX);

    /* Execute the function being tested */
    HS_MonitorUtilization();

    /* Verify results */
    UtAssert_INT32_EQ(HS_AppData.UtilizationTracker[HS_AppData.CurrentCPUUtilIndex - 1], HS_CPU_UTILIZATION_MAX);
    /* For this test case, we don't care about any messages or variables changed after this is set */

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorUtilization_Test_CurrentUtilLessThanZero(void)
{
    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, -1);

    /* Execute the function being tested */
    HS_MonitorUtilization();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuAvg, 0xFFFFFFFF);
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuPeak, 0xFFFFFFFF);
}

void HS_MonitorUtilization_Test_CPUHogging(void)
{
    HS_AppData.CurrentCPUHogState    = HS_STATE_ENABLED;
    HS_AppData.MaxCPUHoggingTime     = 1;
    HS_AppData.CurrentCPUHoggingTime = 0;
    HS_AppData.CurrentCPUUtilIndex   = HS_UTIL_PEAK_NUM_INTERVAL - 1;

    /* test below the hogging threshold */
    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_UTIL_PER_INTERVAL_HOGGING - 1);

    /* Execute the function being tested */
    HS_MonitorUtilization();

    /* Verify results - no hogging or event reported here */
    UtAssert_ZERO(HS_AppData.CurrentCPUHoggingTime);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* now above the hogging threshold */
    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_UTIL_PER_INTERVAL_HOGGING + 1);

    /* Execute the function being tested */
    HS_MonitorUtilization();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentCPUHoggingTime, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CPUMON_HOGGING_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_MonitorUtilization_Test_CPUHoggingNotMax(void)
{
    HS_AppData.CurrentCPUHogState = HS_STATE_ENABLED;
    HS_AppData.MaxCPUHoggingTime  = 2;

    HS_AppData.CurrentCPUUtilIndex = HS_UTIL_PEAK_NUM_INTERVAL - 1;

    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_CPU_UTILIZATION_MAX + 1);

    /* Execute the function being tested */
    HS_MonitorUtilization();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentCPUHoggingTime == 1, "HS_AppData.CurrentCPUHoggingTime == 1");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);

    /* For this test case, we don't care about any variables changed after this message */
}

void HS_MonitorUtilization_Test_CurrentCPUHogStateDisabled(void)
{
    HS_AppData.CurrentCPUHogState = HS_STATE_DISABLED;
    HS_AppData.MaxCPUHoggingTime  = 1;

    HS_AppData.CurrentCPUUtilIndex = HS_UTIL_PEAK_NUM_INTERVAL - 1;

    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_CPU_UTILIZATION_MAX);

    /* Execute the function being tested */
    HS_MonitorUtilization();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentCPUHoggingTime == 0, "HS_AppData.CurrentCPUHoggingTime == 0");

    /* For this test case, we don't care about any variables changed after this variable is set */

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorUtilization_Test_HighUtilIndex(void)
{
    HS_AppData.CurrentCPUHogState = HS_STATE_DISABLED;
    HS_AppData.MaxCPUHoggingTime  = 1;

    HS_AppData.CurrentCPUUtilIndex = HS_UTIL_PEAK_NUM_INTERVAL - 1;

    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_CPU_UTILIZATION_MAX);

    /* Execute the function being tested */
    HS_MonitorUtilization();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentCPUHoggingTime == 0, "HS_AppData.CurrentCPUHoggingTime == 0");
    UtAssert_True(HS_AppData.UtilCpuAvg == (HS_CPU_UTILIZATION_MAX / HS_UTIL_AVERAGE_NUM_INTERVAL),
                  "HS_AppData.UtilCpuAvg  == (HS_CPU_UTILIZATION_MAX / HS_UTIL_AVERAGE_NUM_INTERVAL)");
    UtAssert_True(HS_AppData.UtilCpuPeak == HS_CPU_UTILIZATION_MAX, "HS_AppData.UtilCpuPeak == HS_CPU_UTILIZATION_MAX");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MonitorUtilization_Test_LowUtilIndex(void)
{
    HS_AppData.CurrentCPUHogState = HS_STATE_DISABLED;
    HS_AppData.MaxCPUHoggingTime  = 1;

    HS_AppData.CurrentCPUUtilIndex = 1;

    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_CPU_UTILIZATION_MAX);

    /* Execute the function being tested */
    HS_MonitorUtilization();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentCPUHoggingTime == 0, "HS_AppData.CurrentCPUHoggingTime == 0");
    UtAssert_True(HS_AppData.UtilCpuAvg == (HS_CPU_UTILIZATION_MAX / HS_UTIL_AVERAGE_NUM_INTERVAL),
                  "HS_AppData.UtilCpuAvg  == (HS_CPU_UTILIZATION_MAX / HS_UTIL_AVERAGE_NUM_INTERVAL)");
    UtAssert_True(HS_AppData.UtilCpuPeak == HS_CPU_UTILIZATION_MAX, "HS_AppData.UtilCpuPeak == HS_CPU_UTILIZATION_MAX");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateAMTable_Test_UnusedTableEntryCycleCountZero(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].ActionType = 99;
        HS_AppData.AMTablePtr[i].CycleCount = 0;
        HS_AppData.AMTablePtr[i].NullTerm   = 0;
    }

    /* Execute the function being tested */
    Result = HS_ValidateAMTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateAMTable_Test_UnusedTableEntryActionTypeNOACT(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].ActionType = HS_AMT_ACT_NOACT;
        HS_AppData.AMTablePtr[i].CycleCount = 1;
        HS_AppData.AMTablePtr[i].NullTerm   = 0;
    }

    /* Execute the function being tested */
    Result = HS_ValidateAMTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateAMTable_Test_BufferNotNull(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify err: Entry = %%d, Err = %%d, Action = %%d, App = %%s");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].ActionType = 99;
        HS_AppData.AMTablePtr[i].CycleCount = 1;
        HS_AppData.AMTablePtr[i].NullTerm   = 2;
    }

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_ValidateAMTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVAL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_AMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_AMTVAL_ERR_NUL, "Result == HS_AMTVAL_ERR_NUL");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateAMTable_Test_ActionTypeNotValid(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify err: Entry = %%d, Err = %%d, Action = %%d, App = %%s");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].ActionType = HS_AMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1;
        HS_AppData.AMTablePtr[i].CycleCount = 1;
        HS_AppData.AMTablePtr[i].NullTerm   = 0;
    }

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_ValidateAMTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVAL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_AMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_AMTVAL_ERR_ACT, "Result == HS_AMTVAL_ERR_ACT");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateAMTable_Test_EntryGood(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].ActionType = HS_AMT_ACT_LAST_NONMSG;
        HS_AppData.AMTablePtr[i].CycleCount = 1;
        HS_AppData.AMTablePtr[i].NullTerm   = 0;
    }

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    UT_SetDefaultReturnValue(UT_KEY(HS_AMTActionIsValid), true);

    /* Execute the function being tested */
    Result = HS_ValidateAMTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateAMTable_Test_Null(void)
{
    int32 Result;

    /* Execute the function being tested */
    Result = HS_ValidateAMTable(NULL);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AM_TBL_NULL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_True(Result == HS_TBL_VAL_ERR, "Result == HS_TBL_VAL_ERR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateEMTable_Test_UnusedTableEntryEventIDZero(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.EMTablePtr = EMTable;

    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].ActionType = 99;
        HS_AppData.EMTablePtr[i].EventID    = 0;
        HS_AppData.EMTablePtr[i].NullTerm   = 0;
    }

    /* Execute the function being tested */
    Result = HS_ValidateEMTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateEMTable_Test_UnusedTableEntryActionTypeNOACT(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.EMTablePtr = EMTable;

    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].ActionType = HS_EMT_ACT_NOACT;
        HS_AppData.EMTablePtr[i].EventID    = 1;
        HS_AppData.EMTablePtr[i].NullTerm   = 0;
    }

    /* Execute the function being tested */
    Result = HS_ValidateEMTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateEMTable_Test_BufferNotNull(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify err: Entry = %%d, Err = %%d, Action = %%d, ID = %%d App = %%s");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.EMTablePtr = EMTable;

    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].ActionType = 99;
        HS_AppData.EMTablePtr[i].EventID    = 1;
        HS_AppData.EMTablePtr[i].NullTerm   = 2;
    }

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_ValidateEMTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVAL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_EMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_EMTVAL_ERR_NUL, "Result == HS_EMTVAL_ERR_NUL");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateEMTable_Test_ActionTypeNotValid(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify err: Entry = %%d, Err = %%d, Action = %%d, ID = %%d App = %%s");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.EMTablePtr = EMTable;

    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].ActionType = HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1;
        HS_AppData.EMTablePtr[i].EventID    = 1;
        HS_AppData.EMTablePtr[i].NullTerm   = 0;
    }

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_ValidateEMTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVAL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_EMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_AMTVAL_ERR_ACT, "Result == HS_AMTVAL_ERR_ACT");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateEMTable_Test_EntryGood(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.EMTablePtr = EMTable;

    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].ActionType = HS_EMT_ACT_LAST_NONMSG;
        HS_AppData.EMTablePtr[i].EventID    = 1;
        HS_AppData.EMTablePtr[i].NullTerm   = 0;
    }

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    UT_SetDefaultReturnValue(UT_KEY(HS_EMTActionIsValid), true);

    /* Execute the function being tested */
    Result = HS_ValidateEMTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateEMTable_Test_Null(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error in EM Table Validation. Table is null.");

    /* Execute the function being tested */
    Result = HS_ValidateEMTable(NULL);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EM_TBL_NULL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == HS_TBL_VAL_ERR, "Result == HS_TBL_VAL_ERR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateXCTable_Test_UnusedTableEntry(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_XCTEntry_t XCTable[HS_MAX_EXEC_CNT_SLOTS];

    memset(XCTable, 0, sizeof(XCTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.XCTablePtr = XCTable;

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        HS_AppData.XCTablePtr[i].ResourceType = HS_XCT_TYPE_NOTYPE;
        HS_AppData.XCTablePtr[i].NullTerm     = 0;
    }

    /* Execute the function being tested */
    Result = HS_ValidateXCTable(HS_AppData.XCTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateXCTable_Test_BufferNotNull(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_XCTEntry_t XCTable[HS_MAX_EXEC_CNT_SLOTS];

    memset(XCTable, 0, sizeof(XCTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify err: Entry = %%d, Err = %%d, Type = %%d, Name = %%s");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.XCTablePtr = XCTable;

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        HS_AppData.XCTablePtr[i].ResourceType = 99;
        HS_AppData.XCTablePtr[i].NullTerm     = 1;
    }

    strncpy(HS_AppData.XCTablePtr[0].ResourceName, "ResourceName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_ValidateXCTable(HS_AppData.XCTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCTVAL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_XCTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_XCTVAL_ERR_NUL, "Result == HS_XCTVAL_ERR_NUL");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateXCTable_Test_ResourceTypeNotValid(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_XCTEntry_t XCTable[HS_MAX_EXEC_CNT_SLOTS];

    memset(XCTable, 0, sizeof(XCTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify err: Entry = %%d, Err = %%d, Type = %%d, Name = %%s");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.XCTablePtr = XCTable;

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        HS_AppData.XCTablePtr[i].ResourceType = 99;
        HS_AppData.XCTablePtr[i].NullTerm     = 0;
    }

    /* take all branches for "Resource Type is not valid" */
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCT_TYPE_APP_MAIN;
    HS_AppData.XCTablePtr[1].ResourceType = HS_XCT_TYPE_APP_CHILD;
    HS_AppData.XCTablePtr[2].ResourceType = HS_XCT_TYPE_DEVICE;
    HS_AppData.XCTablePtr[3].ResourceType = HS_XCT_TYPE_ISR;

    strncpy(HS_AppData.XCTablePtr[0].ResourceName, "ResourceName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_ValidateXCTable(HS_AppData.XCTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCTVAL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_XCTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_XCTVAL_ERR_TYPE, "Result == HS_XCTVAL_ERR_TYPE");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateXCTable_Test_EntryGood(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_XCTEntry_t XCTable[HS_MAX_EXEC_CNT_SLOTS];

    memset(XCTable, 0, sizeof(XCTable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.XCTablePtr = XCTable;

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        HS_AppData.XCTablePtr[i].ResourceType = HS_XCT_TYPE_APP_MAIN;
        HS_AppData.XCTablePtr[i].NullTerm     = 0;
    }

    /* Execute the function being tested */
    Result = HS_ValidateXCTable(HS_AppData.XCTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCTVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateXCTable_Test_Null(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error in XC Table Validation. Table is null.");

    /* Execute the function being tested */
    Result = HS_ValidateXCTable(NULL);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XC_TBL_NULL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == HS_TBL_VAL_ERR, "Result == HS_TBL_VAL_ERR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateMATable_Test_UnusedTableEntry(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify results: good = %%d, bad = %%d, unused = %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    MsgSize   = sizeof(HS_NoopCmd_t);

    HS_AppData.MATablePtr = MATable;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MATablePtr[i].EnableState = HS_MAT_STATE_DISABLED;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* take secondary branch for "Enable State is Invalid" */
    HS_AppData.MATablePtr[1].EnableState = HS_MAT_STATE_NOEVENT;

    /* Execute the function being tested */
    Result = HS_ValidateMATable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateMATable_Test_InvalidEnableState(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify err: Entry = %%d, Err = %%d, Length = %%d, ID = 0x%%08lX");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify results: good = %%d, bad = %%d, unused = %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    MsgSize   = sizeof(HS_NoopCmd_t);

    HS_AppData.MATablePtr = MATable;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MATablePtr[i].EnableState = 99;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* Execute the function being tested */
    Result = HS_ValidateMATable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVAL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_MATVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_MATVAL_ERR_ENA, "Result == HS_MATVAL_ERR_ENA");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateMATable_Test_MessageIDTooHigh(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify err: Entry = %%d, Err = %%d, Length = %%d, ID = 0x%%08lX");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify results: good = %%d, bad = %%d, unused = %%d");

    TestMsgId = CFE_SB_INVALID_MSG_ID;
    MsgSize   = sizeof(HS_NoopCmd_t);

    HS_AppData.MATablePtr = MATable;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MATablePtr[i].EnableState = HS_MAT_STATE_ENABLED;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* Execute the function being tested */
    Result = HS_ValidateMATable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVAL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_MATVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_MATVAL_ERR_ID, "Result == HS_MATVAL_ERR_ID");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateMATable_Test_LengthTooHigh(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify err: Entry = %%d, Err = %%d, Length = %%d, ID = 0x%%08lX");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify results: good = %%d, bad = %%d, unused = %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    MsgSize   = CFE_MISSION_SB_MAX_SB_MSG_SIZE + 1;

    HS_AppData.MATablePtr = MATable;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MATablePtr[i].EnableState = HS_MAT_STATE_ENABLED;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* Execute the function being tested */
    Result = HS_ValidateMATable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVAL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_MATVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_MATVAL_ERR_LEN, "Result == HS_MATVAL_ERR_LEN");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 2, "CFE_EVS_SendEvent was called %u time(s), expected 2",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateMATable_Test_EntryGood(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify results: good = %%d, bad = %%d, unused = %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    MsgSize   = sizeof(HS_NoopCmd_t);

    HS_AppData.MATablePtr = MATable;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MATablePtr[i].EnableState = HS_MAT_STATE_ENABLED;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* Execute the function being tested */
    Result = HS_ValidateMATable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVAL_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ValidateMATable_Test_Null(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error in MA Table Validation. Table is null.");

    /* Execute the function being tested */
    Result = HS_ValidateMATable(NULL);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MA_TBL_NULL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == HS_TBL_VAL_ERR, "Result == HS_TBL_VAL_ERR");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SetCDSData_Test(void)
{
    uint16 ResetsPerformed = 1;
    uint16 MaxResets       = 2;

    HS_AppData.CDSState = HS_STATE_ENABLED;

    /* Execute the function being tested */
    HS_SetCDSData(ResetsPerformed, MaxResets);

    /* Verify results */
    UtAssert_True(HS_AppData.CDSData.ResetsPerformed == 1, "HS_AppData.CDSData.ResetsPerformed == 1");
    UtAssert_True(HS_AppData.CDSData.ResetsPerformedNot == (uint16)(~HS_AppData.CDSData.ResetsPerformed),
                  "HS_AppData.CDSData.ResetsPerformedNot == (uint16)(~HS_AppData.CDSData.ResetsPerformed)");
    UtAssert_True(HS_AppData.CDSData.MaxResets == 2, "HS_AppData.CDSData.MaxResets == 2");
    UtAssert_True(HS_AppData.CDSData.MaxResetsNot == (uint16)(~HS_AppData.CDSData.MaxResets),
                  "HS_AppData.CDSData.MaxResetsNot == (uint16)(~HS_AppData.CDSData.MaxResets)");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_MonitorApplications_Test_AppNameNotFound, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_AppNameNotFound");
    UtTest_Add(HS_MonitorApplications_Test_AppNameNotFoundDebugEvent, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_AppNameNotFoundDebugEvent");
    UtTest_Add(HS_MonitorApplications_Test_GetExeCountFailure, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_GetExeCountFailure");
    UtTest_Add(HS_MonitorApplications_Test_ProcessorResetError, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_ProcessorResetError");
    UtTest_Add(HS_MonitorApplications_Test_ProcessorResetActionLimitError, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_ProcessorResetActionLimitError");
    UtTest_Add(HS_MonitorApplications_Test_RestartAppErrorsGetAppInfoSuccess, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_RestartAppErrorsGetAppInfoSuccess");
    UtTest_Add(HS_MonitorApplications_Test_RestartAppErrorsGetAppInfoNotSuccess, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_RestartAppErrorsGetAppInfoNotSuccess");
    UtTest_Add(HS_MonitorApplications_Test_RestartAppRestartSuccess, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_RestartAppRestartSuccess");
    UtTest_Add(HS_MonitorApplications_Test_FailError, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_FailError");
    UtTest_Add(HS_MonitorApplications_Test_MsgActsNOACT, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_MsgActsNOACT");
    UtTest_Add(HS_MonitorApplications_Test_MsgActsNOACTDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_MsgActsNOACTDisabled");
    UtTest_Add(HS_MonitorApplications_Test_MsgActsErrorDefault, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_MsgActsErrorDefault");
    UtTest_Add(HS_MonitorApplications_Test_MsgActsErrorDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_MsgActsErrorDisabled");
    UtTest_Add(HS_MonitorApplications_Test_MsgActsErrorDefaultCoolDown, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_MsgActsErrorDefaultCoolDown");
    UtTest_Add(HS_MonitorApplications_Test_MsgActsErrorDefaultDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_MsgActsErrorDefaultDisabled");
    UtTest_Add(HS_MonitorApplications_Test_MsgActsErrorDefaultNoEvent, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_Test_MsgActsErrorDefaultNoEvent");
    UtTest_Add(HS_MonitorApplications_CheckInCountdownNotZero, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorApplications_CheckInCountdownNotZero");

    UtTest_Add(HS_MonitorEvent_Test_AppName, HS_Test_Setup, HS_Test_TearDown, "HS_MonitorEvent_Test_AppName");
    UtTest_Add(HS_MonitorEvent_Test_ProcErrorReset, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_ProcErrorReset");
    UtTest_Add(HS_MonitorEvent_Test_ProcErrorNoReset, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_ProcErrorNoReset");
    UtTest_Add(HS_MonitorEvent_Test_AppRestartErrors, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_AppRestartErrors");
    UtTest_Add(HS_MonitorEvent_Test_OnlySecondAppRestartError, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_OnlySecondAppRestartError");
    UtTest_Add(HS_MonitorEvent_Test_NoSecondAppRestartError, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_NoSecondAppRestartError");
    UtTest_Add(HS_MonitorEvent_Test_DeleteErrors, HS_Test_Setup, HS_Test_TearDown, "HS_MonitorEvent_Test_DeleteErrors");
    UtTest_Add(HS_MonitorEvent_Test_OnlySecondDeleteError, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_OnlySecondDeleteError");
    UtTest_Add(HS_MonitorEvent_Test_NoSecondDeleteError, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_NoSecondDeleteError");
    UtTest_Add(HS_MonitorEvent_Test_MsgActsError, HS_Test_Setup, HS_Test_TearDown, "HS_MonitorEvent_Test_MsgActsError");
    UtTest_Add(HS_MonitorEvent_Test_MsgActsErrorNoEvent, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_MsgActsErrorNoEvent");
    UtTest_Add(HS_MonitorEvent_Test_MsgActsCoolDown, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_MsgActsCoolDown");
    UtTest_Add(HS_MonitorEvent_Test_MsgActsMATDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_MsgActsMATDisabled");
    UtTest_Add(HS_MonitorEvent_Test_MsgActsDefaultDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_MsgActsDefaultDisabled");
    UtTest_Add(HS_MonitorEvent_Test_MsgActsDefaultGreaterLastNonMsg, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_MsgActsDefaultGreaterLastNonMsg");
    UtTest_Add(HS_MonitorEvent_Test_MsgActsDefaultLessMaxActTypes, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_MsgActsDefaultLessMaxActTypes");
    UtTest_Add(HS_MonitorEvent_Test_MsgActsDefaultMaxActTypes, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorEvent_Test_MsgActsDefaultMaxActTypes");

    UtTest_Add(HS_MonitorUtilization_Test_HighCurrentUtil, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorUtilization_Test_HighCurrentUtil");
    UtTest_Add(HS_MonitorUtilization_Test_CurrentUtilLessThanZero, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorUtilization_Test_CurrentUtilLessThanZero");
    UtTest_Add(HS_MonitorUtilization_Test_CPUHogging, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorUtilization_Test_CPUHogging");
    UtTest_Add(HS_MonitorUtilization_Test_CPUHoggingNotMax, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorUtilization_Test_CPUHoggingNotMax");
    UtTest_Add(HS_MonitorUtilization_Test_CurrentCPUHogStateDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorUtilization_Test_CurrentCPUHogStateDisabled");
    UtTest_Add(HS_MonitorUtilization_Test_HighUtilIndex, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorUtilization_Test_HighUtilIndex");
    UtTest_Add(HS_MonitorUtilization_Test_LowUtilIndex, HS_Test_Setup, HS_Test_TearDown,
               "HS_MonitorUtilization_Test_LowUtilIndex");

    UtTest_Add(HS_ValidateAMTable_Test_UnusedTableEntryCycleCountZero, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateAMTable_Test_UnusedTableEntryCycleCountZero");
    UtTest_Add(HS_ValidateAMTable_Test_UnusedTableEntryActionTypeNOACT, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateAMTable_Test_UnusedTableEntryActionTypeNOACT");
    UtTest_Add(HS_ValidateAMTable_Test_BufferNotNull, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateAMTable_Test_BufferNotNull");
    UtTest_Add(HS_ValidateAMTable_Test_ActionTypeNotValid, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateAMTable_Test_ActionTypeNotValid");
    UtTest_Add(HS_ValidateAMTable_Test_EntryGood, HS_Test_Setup, HS_Test_TearDown, "HS_ValidateAMTable_Test_EntryGood");
    UtTest_Add(HS_ValidateAMTable_Test_Null, HS_Test_Setup, HS_Test_TearDown, "HS_ValidateAMTable_Test_Null");

    UtTest_Add(HS_ValidateEMTable_Test_UnusedTableEntryEventIDZero, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateEMTable_Test_UnusedTableEntryEventIDZero");
    UtTest_Add(HS_ValidateEMTable_Test_UnusedTableEntryActionTypeNOACT, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateEMTable_Test_UnusedTableEntryActionTypeNOACT");
    UtTest_Add(HS_ValidateEMTable_Test_BufferNotNull, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateEMTable_Test_BufferNotNull");
    UtTest_Add(HS_ValidateEMTable_Test_ActionTypeNotValid, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateEMTable_Test_ActionTypeNotValid");
    UtTest_Add(HS_ValidateEMTable_Test_EntryGood, HS_Test_Setup, HS_Test_TearDown, "HS_ValidateEMTable_Test_EntryGood");
    UtTest_Add(HS_ValidateEMTable_Test_Null, HS_Test_Setup, HS_Test_TearDown, "HS_ValidateEMTable_Test_Null");

    UtTest_Add(HS_ValidateXCTable_Test_UnusedTableEntry, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateXCTable_Test_UnusedTableEntry");
    UtTest_Add(HS_ValidateXCTable_Test_BufferNotNull, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateXCTable_Test_BufferNotNull");
    UtTest_Add(HS_ValidateXCTable_Test_ResourceTypeNotValid, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateXCTable_Test_ResourceTypeNotValid");
    UtTest_Add(HS_ValidateXCTable_Test_EntryGood, HS_Test_Setup, HS_Test_TearDown, "HS_ValidateXCTable_Test_EntryGood");
    UtTest_Add(HS_ValidateXCTable_Test_Null, HS_Test_Setup, HS_Test_TearDown, "HS_ValidateXCTable_Test_Null");

    UtTest_Add(HS_ValidateMATable_Test_UnusedTableEntry, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateMATable_Test_UnusedTableEntry");
    UtTest_Add(HS_ValidateMATable_Test_InvalidEnableState, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateMATable_Test_InvalidEnableState");
    UtTest_Add(HS_ValidateMATable_Test_MessageIDTooHigh, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateMATable_Test_MessageIDTooHigh");
    UtTest_Add(HS_ValidateMATable_Test_LengthTooHigh, HS_Test_Setup, HS_Test_TearDown,
               "HS_ValidateMATable_Test_LengthTooHigh");
    UtTest_Add(HS_ValidateMATable_Test_EntryGood, HS_Test_Setup, HS_Test_TearDown, "HS_ValidateMATable_Test_EntryGood");
    UtTest_Add(HS_ValidateMATable_Test_Null, HS_Test_Setup, HS_Test_TearDown, "HS_ValidateMATable_Test_Null");

    UtTest_Add(HS_SetCDSData_Test, HS_Test_Setup, HS_Test_TearDown, "HS_SetCDSData_Test");
}
