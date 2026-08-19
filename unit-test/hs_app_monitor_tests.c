/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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

#include "hs_app_monitor.h"
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

/*
 * Function Definitions
 */

int32 HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1(void                   *UserObj,
                                              int32                   StubRetcode,
                                              uint32                  CallCount,
                                              const UT_StubContext_t *Context)
{
    CFE_ES_AppInfo_t *AppInfo = UserObj;

    AppInfo->ExecutionCounter = 3;

    return CFE_SUCCESS;
}

void HS_AppMon_CheckAllApps_Test_AppMonTblPtrNull(void)
{
    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    /*
    ** Verify UUT exited before processing app info from the table entries
    ** Since the UUT simply returns during this, the best thing we can do is
    ** verify that none of the stub functions were called
    */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetAppIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetAppInfo, 0);
    UtAssert_STUB_COUNT(OS_TaskDelay, 0);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 0);
    UtAssert_STUB_COUNT(CFE_ES_ResetCFE, 0);
    UtAssert_STUB_COUNT(CFE_ES_RestartApp, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
}

void HS_AppMon_CheckAllApps_Test_AppNameNotFound(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    /* Element 0 will run through logic with action and not expired */
    HS_AppData.AMTablePtr[0].ActionType = -1;
    HS_AppData.AMTablePtr[0].CycleCount = 1;
    AMStatePtr                          = HS_GetAMStateByIndex(0);
    AMStatePtr->Enable                  = true;
    AMStatePtr->CheckInCountdown        = 1;

    /* Element 1 has action but expired */
    HS_AppData.AMTablePtr[1].ActionType = -1;
    AMStatePtr                          = HS_GetAMStateByIndex(1);
    AMStatePtr->Enable                  = true;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to fail on first call, to generate error HS_APPMON_APPNAME_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, -1);

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_APPNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_APPMON_APPNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 1);
}

void HS_AppMon_CheckAllApps_Test_AppNameNotFoundDebugEvent(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = -1;
    HS_AppData.AMTablePtr[0].CycleCount = 2;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->Enable           = true;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to fail on first call, to generate error HS_APPMON_APPNAME_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_APPNAME_DBG_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 1);
}

void HS_AppMon_CheckAllApps_Test_GetExeCountFailure(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t   AppInfo;
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = -1;
    HS_AppData.AMTablePtr[0].CycleCount = 2;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->Enable           = true;
    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->LastExeCount     = 1;
    AMStatePtr->Enable           = true;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Causes "failure to get an execution counter" */
    AppInfo.ExecutionCounter = 3;
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetAppInfo), &AppInfo, sizeof(AppInfo), false);
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 2);

    /* Execution count does not get updated from AppInfo */
    UtAssert_UINT32_EQ(AMStatePtr->LastExeCount, 0);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_AppMon_CheckAllApps_Test_ProcessorResetError(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t   AppInfo;
    HS_AppMon_State_t *AMStatePtr;
    int32              strCmpResult;
    char               ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    char               ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s): Action: Processor Reset");
    snprintf(ExpectedSysLogString,
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: App Monitor Failure: APP:(%%s): Action: Processor Reset\n");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_PROC_RESET;
    HS_AppData.AMTablePtr[0].CycleCount = 1;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->Enable           = true;
    AMStatePtr->LastExeCount     = 0;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.CDSData.MaxResets       = 10;
    HS_AppData.CDSData.ResetsPerformed = 1;

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 0);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);
    UtAssert_UINT32_EQ(HS_AppData.ServiceWatchdogFlag, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_PROC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);
}

void HS_AppMon_CheckAllApps_Test_ProcessorResetActionLimitError(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t   AppInfo;
    HS_AppMon_State_t *AMStatePtr;
    int32              strCmpResult;
    char               ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s): Action: Processor Reset");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Processor Reset Action Limit Reached: No Reset Performed");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_PROC_RESET;
    HS_AppData.AMTablePtr[0].CycleCount = 1;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->LastExeCount     = 0;
    AMStatePtr->Enable           = true;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    HS_AppData.CDSData.MaxResets       = 10;
    HS_AppData.CDSData.ResetsPerformed = 11;

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 0);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_PROC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_RESET_LIMIT_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_AppMon_CheckAllApps_Test_RestartAppErrorsGetAppInfoSuccess(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t   AppInfo;
    HS_AppMon_State_t *AMStatePtr;
    int32              strCmpResult;
    char               ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s) Action: Restart Application");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Restart App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_APP_RESTART;
    HS_AppData.AMTablePtr[0].CycleCount = 1;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->LastExeCount     = 0;
    AMStatePtr->Enable           = true;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);

    /* Set CFE_ES_RestartApp to fail on first call, to generate error HS_APPMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestartApp), 1, 0xFFFFFFFF);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 0);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_APPMON_NOT_RESTARTED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_AppMon_CheckAllApps_Test_RestartAppErrorsGetAppInfoNotSuccess(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AppMon_State_t *AMStatePtr;
    int32              strCmpResult;
    char               ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s) Action: Restart Application");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Restart App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_APP_RESTART;
    HS_AppData.AMTablePtr[0].CycleCount = 2;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->LastExeCount     = 0;
    AMStatePtr->Enable           = true;

    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Set to generate error HS_APPMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestartApp), 1, -1);

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 0);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_APPMON_NOT_RESTARTED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
}

void HS_AppMon_CheckAllApps_Test_RestartAppRestartSuccess(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_APP_RESTART;
    HS_AppData.AMTablePtr[0].CycleCount = 1;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->LastExeCount     = 0;
    AMStatePtr->Enable           = true;

    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Set CFE_ES_GetAppInfo to succeed on first call and avoid error HS_APPMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestartApp), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 0);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_AppMon_CheckAllApps_Test_FailError(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t   AppInfo;
    HS_AppMon_State_t *AMStatePtr;
    int32              strCmpResult;
    char               ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "App Monitor Failure: APP:(%%s): Action: Event Only");

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_EVENT;
    HS_AppData.AMTablePtr[0].CycleCount = 1;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->LastExeCount     = 0;
    AMStatePtr->Enable           = true;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 0);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_FAIL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_AppMon_CheckApp_Test_Cached(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t   AppInfo;
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_EVENT;
    HS_AppData.AMTablePtr[0].CycleCount = 1;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->LastExeCount     = 3;
    AMStatePtr->Enable           = true;
    AMStatePtr->AppId            = CFE_ES_APPID_C(CFE_ResourceId_FromInteger(111));

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);

    /* Execute the function being tested */
    HS_AppMon_CheckApp(&HS_AppData.AMTablePtr[0], AMStatePtr);

    /* Verify results - GetAppIDByName should be skipped */
    UtAssert_STUB_COUNT(CFE_ES_GetAppIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetAppInfo, 1);
    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_UINT32_EQ(CFE_RESOURCEID_TO_ULONG(AMStatePtr->AppId), 111);
}

void HS_AppMon_CheckApp_Test_CachedWithFailure(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_APP_RESTART;
    HS_AppData.AMTablePtr[0].CycleCount = 1;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->LastExeCount     = 3;
    AMStatePtr->Enable           = true;
    AMStatePtr->AppId            = CFE_ES_APPID_C(CFE_ResourceId_FromInteger(112));

    /* Set up for failure of CFE_ES_RestartApp() API call to get HS_APPMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, -1);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestartApp), 1, -1);

    /* Execute the function being tested */
    HS_AppMon_CheckApp(&HS_AppData.AMTablePtr[0], AMStatePtr);

    /* Verify results - GetAppIDByName should be skipped */
    UtAssert_STUB_COUNT(CFE_ES_GetAppIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetAppInfo, 1);
    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 3);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_APPNAME_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_APPMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, HS_APPMON_NOT_RESTARTED_ERR_EID);
    UtAssert_BOOL_FALSE(CFE_RESOURCEID_TEST_DEFINED(AMStatePtr->AppId));
}

void HS_AppMon_CheckApp_Test_EventCallback(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_LAST_NONMSG + 1;
    HS_AppData.AMTablePtr[0].CycleCount = 1;
    strncpy(HS_AppData.AMTablePtr[0].AppName, "ut", sizeof(HS_AppData.AMTablePtr[0].AppName));

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->LastExeCount     = 0;
    AMStatePtr->Enable           = true;
    AMStatePtr->AppId            = CFE_ES_APPID_C(CFE_ResourceId_FromInteger(111));

    /* Prevents "failure to get an execution counter" */
    UT_SetHandlerFunction(UT_KEY(HS_MsgAct_TriggerAction), UT_Handler_HS_MsgAct_TriggerAction, &AMTable[0]);

    /* Execute the function being tested */
    HS_AppMon_CheckApp(&AMTable[0], AMStatePtr);

    /* Verify results - GetAppIDByName should be skipped */
    UtAssert_STUB_COUNT(CFE_ES_GetAppIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetAppInfo, 1);
    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_MSGACTS_ERR_EID);
}

void HS_AppMon_CheckAllApps_CheckInCountdownNotZero(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    CFE_ES_AppInfo_t   AppInfo;
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AMTablePtr[0].ActionType = HS_AMTActType_EVENT;
    HS_AppData.AMTablePtr[0].CycleCount = 1;

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->CheckInCountdown = 2;
    AMStatePtr->LastExeCount     = 0;
    AMStatePtr->Enable           = true;

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", 10);

    /* Prevents "failure to get an execution counter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetAppInfo), HS_MONITORS_TEST_CFE_ES_GetAppInfoHook1, &AppInfo);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppInfo), 1, CFE_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_AppMon_CheckAllApps();

    /* Verify results */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 1);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_AppMon_ValidateTable_Test_UnusedTableEntryCycleCountZero(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].ActionType = 99;
        HS_AppData.AMTablePtr[i].CycleCount = 0;
        HS_AppData.AMTablePtr[i].NullTerm   = 0;
    }

    /* Execute the function being tested */
    Result = HS_AppMon_ValidateTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_UINT32_EQ(Result, CFE_SUCCESS);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_AppMon_ValidateTable_Test_UnusedTableEntryActionTypeNOACT(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].ActionType = HS_AMTActType_NOACT;
        HS_AppData.AMTablePtr[i].CycleCount = 1;
        HS_AppData.AMTablePtr[i].NullTerm   = 0;
    }

    /* Execute the function being tested */
    Result = HS_AppMon_ValidateTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_UINT32_EQ(Result, CFE_SUCCESS);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_AppMon_ValidateTable_Test_BufferNotNull(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify err: Entry = %%d, Err = %%d, Action = %%d, App = %%s");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
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
    Result = HS_AppMon_ValidateTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVal_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_AMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_UINT32_EQ(Result, HS_AMTVal_ERR_NUL);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_AppMon_ValidateTable_Test_ActionTypeNotValid(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify err: Entry = %%d, Err = %%d, Action = %%d, App = %%s");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].ActionType = HS_AMTActType_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1;
        HS_AppData.AMTablePtr[i].CycleCount = 1;
        HS_AppData.AMTablePtr[i].NullTerm   = 0;
    }

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_AppMon_ValidateTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVal_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_AMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_UINT32_EQ(Result, HS_AMTVal_ERR_ACT);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_AppMon_ValidateTable_Test_EntryGood(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "AppMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].ActionType = HS_AMTActType_LAST_NONMSG;
        HS_AppData.AMTablePtr[i].CycleCount = 1;
        HS_AppData.AMTablePtr[i].NullTerm   = 0;
    }

    strncpy(HS_AppData.AMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_AppMon_ValidateTable(HS_AppData.AMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_UINT32_EQ(Result, CFE_SUCCESS);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_AppMon_ValidateTable_Test_Null(void)
{
    int32 Result;

    /* Execute the function being tested */
    Result = HS_AppMon_ValidateTable(NULL);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_AM_TBL_NULL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_UINT32_EQ(Result, HS_TBL_VAL_ERR);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_AppMon_AcquirePointers_Test_NominalNotLoaded(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AMTEntry_t     *AMTablePtr = AMTable;
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    AMTable[0].ActionType = HS_AMTActType_APP_RESTART;
    AMTable[0].CycleCount = 33;

    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &AMTablePtr, sizeof(AMTablePtr), false);

    /* Execute the function being tested */
    HS_AppMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.AppMonLoaded, HS_State_ENABLED);
    UtAssert_UINT32_EQ(HS_AppData.AppMonLoaded, HS_State_ENABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, AMTable[0].CycleCount);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->CheckInCountdown);
}

void HS_AppMon_AcquirePointers_Test_NominalUpdate(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AMTEntry_t     *AMTablePtr = AMTable;
    HS_AppMon_State_t *AMStatePtr;

    memset(AMTable, 0, sizeof(AMTable));

    AMTable[0].ActionType = HS_AMTActType_EVENT;
    AMTable[0].CycleCount = 23;

    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &AMTablePtr, sizeof(AMTablePtr), false);

    AMStatePtr                   = HS_GetAMStateByIndex(0);
    AMStatePtr->Enable           = true;
    AMStatePtr->CheckInCountdown = 1;
    AMStatePtr->LastExeCount     = 2;

    HS_AppData.AMTablePtr         = AMTablePtr;
    HS_AppData.AppMonLoaded       = HS_State_ENABLED;
    HS_AppData.CurrentAppMonState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_AppMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_ENABLED);
    UtAssert_UINT32_EQ(HS_AppData.AppMonLoaded, HS_State_ENABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, AMTable[0].CycleCount);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->CheckInCountdown);
}

void HS_AppMon_AcquirePointers_Test_ErrorsWithNotLoaded(void)
{
    HS_AppData.AppMonLoaded       = HS_State_DISABLED;
    HS_AppData.CurrentAppMonState = HS_State_DISABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_AppMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_DISABLED);
    UtAssert_UINT32_EQ(HS_AppData.AppMonLoaded, HS_State_DISABLED);

    /* Event only sent if it was loaded before */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_AppMon_AcquirePointers_Test_ErrorsWithNotLoadedEnabled(void)
{
    /* this is an invalid combo that is exercised for coverage testing */
    HS_AppData.AppMonLoaded       = HS_State_DISABLED;
    HS_AppData.CurrentAppMonState = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_AppMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_DISABLED);
    UtAssert_UINT32_EQ(HS_AppData.AppMonLoaded, HS_State_DISABLED);

    /* Event only sent if it was loaded before */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_AppMon_AcquirePointers_Test_ErrorsWithLoadedEnabled(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AppMonLoaded       = HS_State_ENABLED;
    HS_AppData.CurrentAppMonState = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_AppMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_DISABLED);
    UtAssert_UINT32_EQ(HS_AppData.AppMonLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_AppMon_AcquirePointers_Test_ErrorsWithLoadedDisabled(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    HS_AppData.AppMonLoaded       = HS_State_ENABLED;
    HS_AppData.CurrentAppMonState = HS_State_DISABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_AppMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_DISABLED);
    UtAssert_UINT32_EQ(HS_AppData.AppMonLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_AppMon_ComputeEnableBits_Test(void)
{
    /*
     * Test Case For:
     * void HS_AppMon_ComputeEnableBits(void *AppMonEnableBits)
     */
    int                i;
    HS_HkTlm_Payload_t Hk;
    HS_AppMon_State_t *AMStatePtr;
    uint8             *FirstEnablePtr;
    uint8             *LastEnablePtr;

    memset(&Hk, 0, sizeof(Hk));
    FirstEnablePtr = (void *)&Hk.AppMonEnables;
    LastEnablePtr  = FirstEnablePtr + sizeof(Hk.AppMonEnables) - 1;
    for (i = 0; i < HS_MAX_MONITORED_APPS; ++i)
    {
        AMStatePtr = HS_GetAMStateByIndex(i);

        AMStatePtr->Enable = (~i & 1);
    }

    /* called for coverage, the output varies depending on config */
    UtAssert_VOIDCALL(HS_AppMon_ComputeEnableBits(FirstEnablePtr));
    UtAssert_NONZERO(*LastEnablePtr | *FirstEnablePtr);
}

void HS_AppMon_StatusRefresh_Test_CycleCountZero(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AppMon_State_t *AMStatePtr;
    uint32             i;

    memset(AMTable, 0, sizeof(AMTable));
    HS_AppData.AMTablePtr = AMTable;

    memset(HS_AppData.AppMonState, 0, sizeof(HS_AppData.AppMonState));
    AMStatePtr         = HS_GetAMStateByIndex(0);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    AMStatePtr->Enable = true;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].CycleCount = 0;
    }

    /* Execute the function being tested */
    HS_AppMon_StatusRefresh();

    /* Verify results */
    /* Check first, middle, and last element */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->LastExeCount);
    UtAssert_ZERO(AMStatePtr->CheckInCountdown);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->LastExeCount);
    UtAssert_ZERO(AMStatePtr->CheckInCountdown);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->LastExeCount);
    UtAssert_ZERO(AMStatePtr->CheckInCountdown);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_AppMon_StatusRefresh_Test_ActionTypeNOACT(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AppMon_State_t *AMStatePtr;
    uint32             i;

    memset(AMTable, 0, sizeof(AMTable));
    HS_AppData.AMTablePtr = AMTable;

    memset(HS_AppData.AppMonState, 0, sizeof(HS_AppData.AppMonState));
    AMStatePtr         = HS_GetAMStateByIndex(0);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    AMStatePtr->Enable = true;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].CycleCount = 1;
        HS_AppData.AMTablePtr[i].ActionType = HS_AMTActType_NOACT;
    }

    /* Execute the function being tested */
    HS_AppMon_StatusRefresh();

    /* Verify results */
    /* Check first, middle, and last element */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->LastExeCount);
    UtAssert_ZERO(AMStatePtr->CheckInCountdown);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->LastExeCount);
    UtAssert_ZERO(AMStatePtr->CheckInCountdown);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->LastExeCount);
    UtAssert_ZERO(AMStatePtr->CheckInCountdown);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_AppMon_StatusRefresh_Test_ElseCase(void)
{
    HS_AMTEntry_t      AMTable[HS_MAX_MONITORED_APPS];
    HS_AppMon_State_t *AMStatePtr;
    uint32             i;

    memset(AMTable, 0, sizeof(AMTable));
    HS_AppData.AMTablePtr = AMTable;

    memset(HS_AppData.AppMonState, 0, sizeof(HS_AppData.AppMonState));
    AMStatePtr         = HS_GetAMStateByIndex(0);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    AMStatePtr->Enable = true;

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].CycleCount = 1 + i;
        HS_AppData.AMTablePtr[i].ActionType = 99;
    }

    /* Execute the function being tested */
    HS_AppMon_StatusRefresh();

    /* Verify results */
    /* Check first, middle, and last element */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->LastExeCount);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 1);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->LastExeCount);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, 1 + HS_MAX_MONITORED_APPS / 2);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);
    UtAssert_ZERO(AMStatePtr->LastExeCount);
    UtAssert_UINT32_EQ(AMStatePtr->CheckInCountdown, HS_MAX_MONITORED_APPS);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_AppMon_CheckAllApps_Test_AppMonTblPtrNull,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_AppMonTblPtrNull");
    UtTest_Add(HS_AppMon_CheckAllApps_Test_AppNameNotFound,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_AppNameNotFound");
    UtTest_Add(HS_AppMon_CheckAllApps_Test_AppNameNotFoundDebugEvent,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_AppNameNotFoundDebugEvent");
    UtTest_Add(HS_AppMon_CheckAllApps_Test_GetExeCountFailure,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_GetExeCountFailure");
    UtTest_Add(HS_AppMon_CheckAllApps_Test_ProcessorResetError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_ProcessorResetError");
    UtTest_Add(HS_AppMon_CheckAllApps_Test_ProcessorResetActionLimitError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_ProcessorResetActionLimitError");
    UtTest_Add(HS_AppMon_CheckAllApps_Test_RestartAppErrorsGetAppInfoSuccess,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_RestartAppErrorsGetAppInfoSuccess");
    UtTest_Add(HS_AppMon_CheckAllApps_Test_RestartAppErrorsGetAppInfoNotSuccess,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_RestartAppErrorsGetAppInfoNotSuccess");
    UtTest_Add(HS_AppMon_CheckAllApps_Test_RestartAppRestartSuccess,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_RestartAppRestartSuccess");
    UtTest_Add(HS_AppMon_CheckAllApps_Test_FailError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_Test_FailError");
    UtTest_Add(HS_AppMon_CheckAllApps_CheckInCountdownNotZero,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckAllApps_CheckInCountdownNotZero");

    UtTest_Add(HS_AppMon_CheckApp_Test_Cached, HS_Test_Setup, HS_Test_TearDown, "HS_AppMon_CheckApp_Test_Cached");
    UtTest_Add(HS_AppMon_CheckApp_Test_CachedWithFailure,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckApp_Test_CachedWithFailure");
    UtTest_Add(HS_AppMon_CheckApp_Test_EventCallback,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_CheckApp_Test_EventCallback");

    UtTest_Add(HS_AppMon_ValidateTable_Test_UnusedTableEntryCycleCountZero,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_ValidateTable_Test_UnusedTableEntryCycleCountZero");
    UtTest_Add(HS_AppMon_ValidateTable_Test_UnusedTableEntryActionTypeNOACT,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_ValidateTable_Test_UnusedTableEntryActionTypeNOACT");
    UtTest_Add(HS_AppMon_ValidateTable_Test_BufferNotNull,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_ValidateTable_Test_BufferNotNull");
    UtTest_Add(HS_AppMon_ValidateTable_Test_ActionTypeNotValid,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_ValidateTable_Test_ActionTypeNotValid");
    UtTest_Add(HS_AppMon_ValidateTable_Test_EntryGood,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_ValidateTable_Test_EntryGood");
    UtTest_Add(HS_AppMon_ValidateTable_Test_Null, HS_Test_Setup, HS_Test_TearDown, "HS_AppMon_ValidateTable_Test_Null");

    UtTest_Add(HS_AppMon_AcquirePointers_Test_NominalNotLoaded,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_AcquirePointers_Test_NominalNotLoaded");
    UtTest_Add(HS_AppMon_AcquirePointers_Test_NominalUpdate,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_AcquirePointers_Test_NominalUpdate");
    UtTest_Add(HS_AppMon_AcquirePointers_Test_ErrorsWithNotLoaded,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_AcquirePointers_Test_ErrorsWithNotLoaded");
    UtTest_Add(HS_AppMon_AcquirePointers_Test_ErrorsWithNotLoadedEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_AcquirePointers_Test_ErrorsWithNotLoadedEnabled");
    UtTest_Add(HS_AppMon_AcquirePointers_Test_ErrorsWithLoadedEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_AcquirePointers_Test_ErrorsWithLoadedEnabled");
    UtTest_Add(HS_AppMon_AcquirePointers_Test_ErrorsWithLoadedDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_AcquirePointers_Test_ErrorsWithLoadedDisabled");

    UtTest_Add(HS_AppMon_StatusRefresh_Test_CycleCountZero,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_StatusRefresh_Test_CycleCountZero");
    UtTest_Add(HS_AppMon_StatusRefresh_Test_ActionTypeNOACT,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_StatusRefresh_Test_ActionTypeNOACT");
    UtTest_Add(HS_AppMon_StatusRefresh_Test_ElseCase,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AppMon_StatusRefresh_Test_ElseCase");

    UtTest_Add(HS_AppMon_ComputeEnableBits_Test, HS_Test_Setup, HS_Test_TearDown, "HS_AppMon_ComputeEnableBits_Test");
}
