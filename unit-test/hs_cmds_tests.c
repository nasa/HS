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

#include "hs_cmds.h"
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

/* hs_cmds_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

int32 HS_CMDS_TEST_CFE_ES_GetTaskInfoHook(void *UserObj, int32 StubRetcode, uint32 CallCount,
                                          const UT_StubContext_t *Context)
{
    CFE_ES_TaskInfo_t *TaskInfo = UserObj;

    TaskInfo->ExecutionCounter = 5;

    return CFE_SUCCESS;
}

void HS_SendHkCmd_Test_InvalidEventMon(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    uint32            TableIndex;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));

    HS_AppData.EMTablePtr = EMTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* 2 entries that are not HS_EMT_ACT_NOACT for branch coverage */
    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT + 1;
    HS_AppData.EMTablePtr[1].ActionType = HS_EMT_ACT_NOACT + 1;

    /* Fail first, succeed on second */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
    {
        HS_AppData.AppMonEnables[TableIndex] = TableIndex;
    }

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 1, "PayloadPtr->InvalidEventMonCount == 1");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SendHkCmd_Test_AllFlagsEnabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t     XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint8             ExpectedStatusFlags = 0;
    int               i;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCT_TYPE_NOTYPE;
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT;

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    HS_AppData.ExeCountState  = HS_STATE_ENABLED;
    HS_AppData.MsgActsState   = HS_STATE_ENABLED;
    HS_AppData.AppMonLoaded   = HS_STATE_ENABLED;
    HS_AppData.EventMonLoaded = HS_STATE_ENABLED;
    HS_AppData.CDSState       = HS_STATE_ENABLED;

    ExpectedStatusFlags |= HS_LOADED_XCT;
    ExpectedStatusFlags |= HS_LOADED_MAT;
    ExpectedStatusFlags |= HS_LOADED_AMT;
    ExpectedStatusFlags |= HS_LOADED_EMT;
    ExpectedStatusFlags |= HS_CDS_IN_USE;

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 0, "PayloadPtr->InvalidEventMonCount == 0");

    UtAssert_True(PayloadPtr->StatusFlags == ExpectedStatusFlags, "PayloadPtr->StatusFlags == ExpectedStatusFlags");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SendHkCmd_Test_ResourceTypeAppMain(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t     XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint32            TableIndex;
    CFE_ES_TaskInfo_t TaskInfo;

    HS_HkTlm_Payload_t *PayloadPtr;

    int i;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCT_TYPE_APP_MAIN;
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT;

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
    {
        HS_AppData.AppMonEnables[TableIndex] = TableIndex;
    }

    HS_AppData.ExeCountState              = HS_STATE_ENABLED;
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCT_TYPE_APP_MAIN;

    /* Causes line "Status = CFE_ES_GetTaskInfo(&TaskInfo, TaskId)" to be reached */
    UT_SetDeferredRetcode(UT_KEY(OS_TaskGetIdByName), 1, OS_SUCCESS);

    /* Sets TaskInfo.ExecutionCounter to 5, returns CFE_SUCCESS, goes to line "ExeCount = TaskInfo.ExecutionCounter" */
    TaskInfo.ExecutionCounter = 5;
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetTaskInfo), &TaskInfo, sizeof(TaskInfo), false);
    UT_SetHookFunction(UT_KEY(CFE_ES_GetTaskInfo), HS_CMDS_TEST_CFE_ES_GetTaskInfoHook, &TaskInfo);

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 0, "PayloadPtr->InvalidEventMonCount == 0");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    UtAssert_True(PayloadPtr->ExeCounts[0] == 5, "PayloadPtr->ExeCounts[0] == 5");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SendHkCmd_Test_ResourceTypeAppChild(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t     XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint32            TableIndex;
    CFE_ES_TaskInfo_t TaskInfo;
    int               i;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCT_TYPE_APP_CHILD;
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT;

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
    {
        HS_AppData.AppMonEnables[TableIndex] = TableIndex;
    }

    HS_AppData.ExeCountState              = HS_STATE_ENABLED;
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCT_TYPE_APP_CHILD;

    /* Causes line "Status = CFE_ES_GetTaskInfo(&TaskInfo, TaskId)" to be reached */
    UT_SetDeferredRetcode(UT_KEY(OS_TaskGetIdByName), 1, OS_SUCCESS);

    /* Sets TaskInfo.ExecutionCounter to 5, returns CFE_SUCCESS, goes to line "ExeCount = TaskInfo.ExecutionCounter" */
    TaskInfo.ExecutionCounter = 5;
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetTaskInfo), &TaskInfo, sizeof(TaskInfo), false);
    UT_SetHookFunction(UT_KEY(CFE_ES_GetTaskInfo), HS_CMDS_TEST_CFE_ES_GetTaskInfoHook, &TaskInfo);

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 0, "PayloadPtr->InvalidEventMonCount == 0");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    UtAssert_True(PayloadPtr->ExeCounts[0] == 5, "PayloadPtr->ExeCounts[0] == 5");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SendHkCmd_Test_ResourceTypeAppChildTaskIdError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t     XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint32            TableIndex;
    CFE_ES_TaskInfo_t TaskInfo;
    int               i;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCT_TYPE_APP_CHILD;
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT;

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
    {
        HS_AppData.AppMonEnables[TableIndex] = TableIndex;
    }

    HS_AppData.ExeCountState              = HS_STATE_ENABLED;
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCT_TYPE_APP_CHILD;

    /* Causes line "Status = CFE_ES_GetTaskInfo(&TaskInfo, TaskId)" to be skipped */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetTaskIDByName), 1, -1);

    /* Sets TaskInfo.ExecutionCounter to 5, returns CFE_SUCCESS, goes to line "ExeCount = TaskInfo.ExecutionCounter" */
    TaskInfo.ExecutionCounter = 5;
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetTaskInfo), &TaskInfo, sizeof(TaskInfo), false);
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetTaskInfo), &TaskInfo, sizeof(TaskInfo), false);
    UT_SetHookFunction(UT_KEY(CFE_ES_GetTaskInfo), HS_CMDS_TEST_CFE_ES_GetTaskInfoHook, &TaskInfo);

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 0, "PayloadPtr->InvalidEventMonCount == 0");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    UtAssert_True(PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT, "PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SendHkCmd_Test_ResourceTypeAppChildTaskInfoError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t     XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint32            TableIndex;
    int               i;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCT_TYPE_APP_CHILD;
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT;

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
    {
        HS_AppData.AppMonEnables[TableIndex] = TableIndex;
    }

    HS_AppData.ExeCountState              = HS_STATE_ENABLED;
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCT_TYPE_APP_CHILD;

    /* Causes line "Status = CFE_ES_GetTaskInfo(&TaskInfo, TaskId)" to be skipped */
    UT_SetDeferredRetcode(UT_KEY(OS_TaskGetIdByName), 1, OS_SUCCESS);

    /* Sets CFE_ES_GetTaskInfo to return an error and bypass "ExeCount = TaskInfo.ExecutionCounter" */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetTaskInfo), 1, -1);

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 0, "PayloadPtr->InvalidEventMonCount == 0");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    UtAssert_True(PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT, "PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SendHkCmd_Test_ResourceTypeDevice(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t     XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint32            TableIndex;
    CFE_ES_TaskInfo_t TaskInfo;
    int               i;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCT_TYPE_DEVICE;
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT;

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
    {
        HS_AppData.AppMonEnables[TableIndex] = TableIndex;
    }

    HS_AppData.ExeCountState              = HS_STATE_ENABLED;
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCT_TYPE_DEVICE;

    /* Causes line "Status = CFE_ES_GetTaskInfo(&TaskInfo, TaskId)" to be reached */
    UT_SetDeferredRetcode(UT_KEY(OS_TaskGetIdByName), 1, OS_SUCCESS);

    /* Sets TaskInfo.ExecutionCounter to 5, returns CFE_SUCCESS, goes to line "ExeCount = TaskInfo.ExecutionCounter" */
    UT_SetHookFunction(UT_KEY(CFE_ES_GetTaskInfo), HS_CMDS_TEST_CFE_ES_GetTaskInfoHook, &TaskInfo);

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 0, "PayloadPtr->InvalidEventMonCount == 0");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    UtAssert_True(PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT, "PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SendHkCmd_Test_ResourceTypeISR(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t     XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint32            TableIndex;
    int               i;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCT_TYPE_ISR;
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT;

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
    {
        HS_AppData.AppMonEnables[TableIndex] = TableIndex;
    }

    HS_AppData.ExeCountState              = HS_STATE_ENABLED;
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCT_TYPE_ISR;

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 0, "PayloadPtr->InvalidEventMonCount == 0");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    UtAssert_True(PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT, "PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SendHkCmd_Test_ResourceTypeISRGenCounterError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t     XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint32            TableIndex;
    int               i;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCT_TYPE_ISR;
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT;

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
    {
        HS_AppData.AppMonEnables[TableIndex] = TableIndex;
    }

    HS_AppData.ExeCountState              = HS_STATE_ENABLED;
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCT_TYPE_ISR;

    /* Set CFE_ES_GetGenCounterIDByName to return error and skip CFE_ES_GetGenCount(TaskId, &ExeCount) */
    UT_SetDefaultReturnValue(UT_KEY(CFE_ES_GetGenCounterIDByName), -1);

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 0, "PayloadPtr->InvalidEventMonCount == 0");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    UtAssert_True(PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT, "PayloadPtr->ExeCounts[0] == HS_INVALID_EXECOUNT");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SendHkCmd_Test_ResourceTypeUnknown(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_EMTEntry_t     EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t     XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint8             ExpectedStatusFlags = 0;
    int               i;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    /* Set the XCTable Resource type to something invalid */
    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = (HS_XCT_TYPE_ISR * 2);
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMT_ACT_NOACT;

    HS_AppData.CmdCount                = 1;
    HS_AppData.CmdErrCount             = 2;
    HS_AppData.CurrentAppMonState      = 3;
    HS_AppData.CurrentEventMonState    = 4;
    HS_AppData.CurrentAlivenessState   = 5;
    HS_AppData.CurrentCPUHogState      = 6;
    HS_AppData.CDSData.ResetsPerformed = 7;
    HS_AppData.CDSData.MaxResets       = 8;
    HS_AppData.EventsMonitoredCount    = 9;
    HS_AppData.MsgActExec              = 10;

    HS_AppData.ExeCountState  = HS_STATE_ENABLED;
    HS_AppData.MsgActsState   = HS_STATE_ENABLED;
    HS_AppData.AppMonLoaded   = HS_STATE_ENABLED;
    HS_AppData.EventMonLoaded = HS_STATE_ENABLED;
    HS_AppData.CDSState       = HS_STATE_ENABLED;

    ExpectedStatusFlags |= HS_LOADED_XCT;
    ExpectedStatusFlags |= HS_LOADED_MAT;
    ExpectedStatusFlags |= HS_LOADED_AMT;
    ExpectedStatusFlags |= HS_LOADED_EMT;
    ExpectedStatusFlags |= HS_CDS_IN_USE;

    /* Execute the function being tested */
    HS_SendHkCmd(&UT_CmdBuf.SendHkCmd);

    /* Verify results */
    PayloadPtr = &HS_AppData.HkPacket.Payload;
    UtAssert_True(PayloadPtr->CmdCount == 1, "PayloadPtr->CmdCount == 1");
    UtAssert_True(PayloadPtr->CmdErrCount == 2, "PayloadPtr->CmdErrCount == 2");
    UtAssert_True(PayloadPtr->CurrentAppMonState == 3, "PayloadPtr->CurrentAppMonState == 3");
    UtAssert_True(PayloadPtr->CurrentEventMonState == 4, "PayloadPtr->CurrentEventMonState == 4");
    UtAssert_True(PayloadPtr->CurrentAlivenessState == 5, "PayloadPtr->CurrentAlivenessState == 5");
    UtAssert_True(PayloadPtr->CurrentCPUHogState == 6, "PayloadPtr->CurrentCPUHogState == 6");
    UtAssert_True(PayloadPtr->ResetsPerformed == 7, "PayloadPtr->ResetsPerformed == 7");
    UtAssert_True(PayloadPtr->MaxResets == 8, "PayloadPtr->MaxResets == 8");
    UtAssert_True(PayloadPtr->EventsMonitoredCount == 9, "PayloadPtr->EventsMonitoredCount == 9");
    UtAssert_True(PayloadPtr->MsgActExec == 10, "PayloadPtr->MsgActExec == 10");
    UtAssert_True(PayloadPtr->InvalidEventMonCount == 0, "PayloadPtr->InvalidEventMonCount == 0");

    UtAssert_True(PayloadPtr->StatusFlags == ExpectedStatusFlags, "PayloadPtr->StatusFlags == ExpectedStatusFlags");

    /* Check first, middle, and last element */
    UtAssert_True(PayloadPtr->AppMonEnables[0] == 0, "PayloadPtr->AppMonEnables[0] == 0");

    UtAssert_True(PayloadPtr->AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] ==
                      ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == ((HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE) / 2");

    UtAssert_True(PayloadPtr->AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] ==
                      (HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == (HS_MAX_MONITORED_APPS -1) / "
                  "HS_BITS_PER_APPMON_ENABLE");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 32, "CFE_EVS_SendEvent was called %u time(s), expected 32",
                  call_count_CFE_EVS_SendEvent);
}

void HS_Noop_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "No-op command: Version %%d.%%d.%%d.%%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_NOOP_CC;
    MsgSize   = sizeof(UT_CmdBuf.NoopCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    HS_NoopCmd(&UT_CmdBuf.NoopCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_NOOP_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ResetCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Reset counters command");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_RESET_CC;
    MsgSize   = sizeof(UT_CmdBuf.ResetCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    HS_ResetCmd(&UT_CmdBuf.ResetCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 0, "HS_AppData.CmdCount == 0");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_RESET_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ResetCounters_Test(void)
{
    /* Execute the function being tested */
    HS_ResetCounters();

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 0, "HS_AppData.CmdCount == 0");
    UtAssert_True(HS_AppData.CmdErrCount == 0, "HS_AppData.CmdErrCount == 0");
    UtAssert_True(HS_AppData.EventsMonitoredCount == 0, "HS_AppData.EventsMonitoredCount == 0");
    UtAssert_True(HS_AppData.MsgActExec == 0, "HS_AppData.MsgActExec == 0");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableAppMonCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_AMTEntry_t     AMTable[HS_MAX_MONITORED_APPS];
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Application Monitoring Enabled");

    memset(AMTable, 0, sizeof(AMTable));

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_APP_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableAppMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.AMTablePtr = AMTable;

    /* Set to Disabled so the function can freshly set it to Enabled (without it already being set that way) */
    HS_AppData.CurrentAppMonState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_EnableAppMonCmd(&UT_CmdBuf.EnableAppMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_APPMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableAppMonCmd_Test_AlreadyEnabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    HS_AMTEntry_t     AMTable[HS_MAX_MONITORED_APPS];
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Application Monitoring is *already* Enabled");

    memset(AMTable, 0, sizeof(AMTable));

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_APP_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableAppMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.AMTablePtr = AMTable;

    /* Set to Enabled to test response when already in the commanded state. */
    HS_AppData.CurrentAppMonState = HS_STATE_ENABLED;

    /* Execute the function being tested */
    HS_EnableAppMonCmd(&UT_CmdBuf.EnableAppMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_APPMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableAppMonCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Application Monitoring Disabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_APP_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableAppMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set to Enabled so the function can freshly set it to Disabled (without it already being set that way) */
    HS_AppData.CurrentAppMonState = HS_STATE_ENABLED;

    /* Execute the function being tested */
    HS_DisableAppMonCmd(&UT_CmdBuf.DisableAppMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_APPMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableAppMonCmd_Test_AlreadyDisabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Application Monitoring is *already* Disabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_APP_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableAppMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set to Disabled to test response when already in the commanded state. */
    HS_AppData.CurrentAppMonState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_DisableAppMonCmd(&UT_CmdBuf.DisableAppMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_APPMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableEventMonCmd_Test_Disabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Event Monitoring Enabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_EnableEventMonCmd(&UT_CmdBuf.EnableEventMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_EVENTMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableEventMonCmd_Test_AlreadyEnabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Event Monitoring is *already* Enabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;

    /* Execute the function being tested */
    HS_EnableEventMonCmd(&UT_CmdBuf.EnableEventMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_EVENTMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableEventMonCmd_Test_SubscribeLongError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor Enable: Error Subscribing to long-format Events,RC=0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_SubscribeEx), 1, -1);

    /* Execute the function being tested */
    HS_EnableEventMonCmd(&UT_CmdBuf.EnableEventMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdErrCount == 1, "HS_AppData.CmdErrCount == 1");

    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_LONG_SUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableEventMonCmd_Test_SubscribeShortError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor Enable: Error Subscribing to short-format Events,RC=0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_SubscribeEx), 2, -1);

    /* Execute the function being tested */
    HS_EnableEventMonCmd(&UT_CmdBuf.EnableEventMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdErrCount == 1, "HS_AppData.CmdErrCount == 1");

    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_SHORT_SUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableEventMonCmd_Test_Enabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Event Monitoring Disabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;

    /* Execute the function being tested */
    HS_DisableEventMonCmd(&UT_CmdBuf.DisableEventMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_EVENTMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableEventMonCmd_Test_AlreadyDisabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Event Monitoring is *already* Disabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_DisableEventMonCmd(&UT_CmdBuf.DisableEventMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_EVENTMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableEventMonCmd_Test_UnsubscribeLongError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor Disable: Error Unsubscribing from long-format Events,RC=0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 1, -1);

    /* Execute the function being tested */
    HS_DisableEventMonCmd(&UT_CmdBuf.DisableEventMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdErrCount == 1, "HS_AppData.CmdErrCount == 1");

    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_LONG_UNSUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableEventMonCmd_Test_UnsubscribeShortError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor Disable: Error Unsubscribing from short-format Events,RC=0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 2, -1);

    /* Execute the function being tested */
    HS_DisableEventMonCmd(&UT_CmdBuf.DisableEventMonCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdErrCount == 1, "HS_AppData.CmdErrCount == 1");

    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_SHORT_UNSUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableAlivenessCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Aliveness Indicator Enabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_ALIVENESS_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableAlivenessCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentAlivenessState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_EnableAlivenessCmd(&UT_CmdBuf.EnableAlivenessCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentAlivenessState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_ALIVENESS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableAlivenessCmd_Test_AlreadyEnabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Aliveness Indicator is *already* Enabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_ALIVENESS_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableAlivenessCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentAlivenessState = HS_STATE_ENABLED;

    /* Execute the function being tested */
    HS_EnableAlivenessCmd(&UT_CmdBuf.EnableAlivenessCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentAlivenessState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_ALIVENESS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableAlivenessCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Aliveness Indicator Disabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_ALIVENESS_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableAlivenessCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentAlivenessState = HS_STATE_ENABLED;

    /* Execute the function being tested */
    HS_DisableAlivenessCmd(&UT_CmdBuf.DisableAlivenessCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAlivenessState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_ALIVENESS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableAlivenessCmd_Test_AlreadyDisabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "Aliveness Indicator is *already* Disabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_ALIVENESS_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableAlivenessCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentAlivenessState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_DisableAlivenessCmd(&UT_CmdBuf.DisableAlivenessCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentAlivenessState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAlivenessState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_ALIVENESS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableCpuHogCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "CPU Hogging Indicator Enabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_CPU_HOG_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableCpuHogCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentCPUHogState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_EnableCpuHogCmd(&UT_CmdBuf.EnableCpuHogCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentCPUHogState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_CPUHOG_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_EnableCpuHogCmd_Test_AlreadyEnabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "CPU Hogging Indicator is *already* Enabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_CPU_HOG_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableCpuHogCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentCPUHogState = HS_STATE_ENABLED;

    /* Execute the function being tested */
    HS_EnableCpuHogCmd(&UT_CmdBuf.EnableCpuHogCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_STATE_ENABLED,
                  "HS_AppData.CurrentCPUHogState == HS_STATE_ENABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_CPUHOG_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableCpuHogCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "CPU Hogging Indicator Disabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_CPU_HOG_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableCpuHogCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentCPUHogState = HS_STATE_ENABLED;

    /* Execute the function being tested */
    HS_DisableCpuHogCmd(&UT_CmdBuf.DisableCpuHogCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentCPUHogState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_CPUHOG_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_DisableCpuHogCmd_Test_AlreadyDisabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH, "CPU Hogging Indicator is *already* Disabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_CPU_HOG_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableCpuHogCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentCPUHogState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_DisableCpuHogCmd(&UT_CmdBuf.DisableCpuHogCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_True(HS_AppData.CurrentCPUHogState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentCPUHogState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_CPUHOG_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_ResetResetsPerformedCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Processor Resets Performed by HS Counter has been Reset");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_RESET_RESETS_PERFORMED_CC;
    MsgSize   = sizeof(UT_CmdBuf.ResetResetsPerformedCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Execute the function being tested */
    HS_ResetResetsPerformedCmd(&UT_CmdBuf.ResetResetsPerformedCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_RESET_RESETS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_SetMaxResetsCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    HS_SetMaxResets_Payload_t *PayloadPtr;

    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Max Resets Performable by HS has been set to %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_SET_MAX_RESETS_CC;
    MsgSize   = sizeof(UT_CmdBuf.SetMaxResetsCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    PayloadPtr = &UT_CmdBuf.SetMaxResetsCmd.Payload;

    PayloadPtr->MaxResets = 5;

    /* Execute the function being tested */
    HS_SetMaxResetsCmd(&UT_CmdBuf.SetMaxResetsCmd);

    /* Verify results */
    UtAssert_True(HS_AppData.CmdCount == 1, "HS_AppData.CmdCount == 1");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SET_MAX_RESETS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AcquirePointers_Test_Nominal(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    /* Satisfies all instances of (Status == CFE_TBL_INFO_UPDATED), skips all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);

    HS_AppData.CurrentAppMonState = HS_STATE_DISABLED;

    /* Execute the function being tested */
    HS_AcquirePointers();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonLoaded == HS_STATE_ENABLED, "HS_AppData.AppMonLoaded == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.EventMonLoaded == HS_STATE_ENABLED, "HS_AppData.EventMonLoaded == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.MsgActsState == HS_STATE_ENABLED, "HS_AppData.MsgActsState == HS_STATE_ENABLED");
    UtAssert_True(HS_AppData.ExeCountState == HS_STATE_ENABLED, "HS_AppData.ExeCountState == HS_STATE_ENABLED");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AcquirePointers_Test_ErrorsWithAppMonLoadedAndEventMonLoadedEnabled(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[4][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting AppMon Table address, RC=0x%%08X, Application Monitoring Disabled");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting EventMon Table address, RC=0x%%08X, Event Monitoring Disabled");
    snprintf(ExpectedEventString[2], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting MsgActs Table address, RC=0x%%08X");
    snprintf(ExpectedEventString[3], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting ExeCount Table address, RC=0x%%08X");

    HS_AppData.AppMonLoaded         = HS_STATE_ENABLED;
    HS_AppData.EventMonLoaded       = HS_STATE_ENABLED;
    HS_AppData.CurrentAppMonState   = HS_STATE_DISABLED;
    HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;
    HS_AppData.MsgActsState         = HS_STATE_ENABLED;
    HS_AppData.ExeCountState        = HS_STATE_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_AcquirePointers();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.AppMonLoaded == HS_STATE_DISABLED, "HS_AppData.AppMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.EventMonLoaded == HS_STATE_DISABLED, "HS_AppData.EventMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.MsgActsState == HS_STATE_DISABLED, "HS_AppData.MsgActsState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.ExeCountState == HS_STATE_DISABLED, "HS_AppData.ExeCountState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_EVENTMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, HS_MSGACTS_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[2], context_CFE_EVS_SendEvent[2].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[2].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[3].EventID, HS_EXECOUNT_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[3].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[3], context_CFE_EVS_SendEvent[3].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[3].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 4, "CFE_EVS_SendEvent was called %u time(s), expected 4",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabled2(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[5][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting AppMon Table address, RC=0x%%08X, Application Monitoring Disabled");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting EventMon Table address, RC=0x%%08X, Event Monitoring Disabled");
    snprintf(ExpectedEventString[2], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error Unsubscribing from short-format Events,RC=0x%%08X");
    snprintf(ExpectedEventString[3], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting MsgActs Table address, RC=0x%%08X");
    snprintf(ExpectedEventString[4], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting ExeCount Table address, RC=0x%%08X");

    HS_AppData.AppMonLoaded         = HS_STATE_DISABLED;
    HS_AppData.EventMonLoaded       = HS_STATE_DISABLED;
    HS_AppData.CurrentAppMonState   = HS_STATE_ENABLED;
    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;
    HS_AppData.MsgActsState         = HS_STATE_ENABLED;
    HS_AppData.ExeCountState        = HS_STATE_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Causes event message HS_BADEMT_LONG_UNSUB_EID to be generated */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 2, -1);

    /* Execute the function being tested */
    HS_AcquirePointers();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.AppMonLoaded == HS_STATE_DISABLED, "HS_AppData.AppMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.EventMonLoaded == HS_STATE_DISABLED, "HS_AppData.EventMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.MsgActsState == HS_STATE_DISABLED, "HS_AppData.MsgActsState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.ExeCountState == HS_STATE_DISABLED, "HS_AppData.ExeCountState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_EVENTMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, HS_BADEMT_SHORT_UNSUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[2], context_CFE_EVS_SendEvent[2].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[2].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[3].EventID, HS_MSGACTS_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[3].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[3], context_CFE_EVS_SendEvent[3].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[3].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[4].EventID, HS_EXECOUNT_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[4].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[4], context_CFE_EVS_SendEvent[4].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[4].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 5, "CFE_EVS_SendEvent was called %u time(s), expected 5",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabled(void)
{
    int32 strCmpResult;
    char  ExpectedEventString[5][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting AppMon Table address, RC=0x%%08X, Application Monitoring Disabled");
    snprintf(ExpectedEventString[1], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting EventMon Table address, RC=0x%%08X, Event Monitoring Disabled");
    snprintf(ExpectedEventString[2], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error Unsubscribing from long-format Events,RC=0x%%08X");
    snprintf(ExpectedEventString[3], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting MsgActs Table address, RC=0x%%08X");
    snprintf(ExpectedEventString[4], CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error getting ExeCount Table address, RC=0x%%08X");

    HS_AppData.AppMonLoaded         = HS_STATE_DISABLED;
    HS_AppData.EventMonLoaded       = HS_STATE_DISABLED;
    HS_AppData.CurrentAppMonState   = HS_STATE_ENABLED;
    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;
    HS_AppData.MsgActsState         = HS_STATE_ENABLED;
    HS_AppData.ExeCountState        = HS_STATE_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Causes event message HS_BADEMT_LONG_UNSUB_EID to be generated */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 1, -1);

    /* Execute the function being tested */
    HS_AcquirePointers();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.AppMonLoaded == HS_STATE_DISABLED, "HS_AppData.AppMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.EventMonLoaded == HS_STATE_DISABLED, "HS_AppData.EventMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.MsgActsState == HS_STATE_DISABLED, "HS_AppData.MsgActsState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.ExeCountState == HS_STATE_DISABLED, "HS_AppData.ExeCountState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_EVENTMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, HS_BADEMT_LONG_UNSUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[2], context_CFE_EVS_SendEvent[2].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[2].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[3].EventID, HS_MSGACTS_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[3].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[3], context_CFE_EVS_SendEvent[3].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[3].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[4].EventID, HS_EXECOUNT_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[4].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[4], context_CFE_EVS_SendEvent[4].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[4].Spec);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 5, "CFE_EVS_SendEvent was called %u time(s), expected 5",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabledNoSubscribeError(void)
{
    HS_AppData.AppMonLoaded         = HS_STATE_DISABLED;
    HS_AppData.EventMonLoaded       = HS_STATE_DISABLED;
    HS_AppData.CurrentAppMonState   = HS_STATE_ENABLED;
    HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;
    HS_AppData.MsgActsState         = HS_STATE_ENABLED;
    HS_AppData.ExeCountState        = HS_STATE_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Causes event message HS_BADEMT_UNSUB_EID to not be generated */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 1, -1);

    /* Execute the function being tested */
    HS_AcquirePointers();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.AppMonLoaded == HS_STATE_DISABLED, "HS_AppData.AppMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.EventMonLoaded == HS_STATE_DISABLED, "HS_AppData.EventMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.MsgActsState == HS_STATE_DISABLED, "HS_AppData.MsgActsState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.ExeCountState == HS_STATE_DISABLED, "HS_AppData.ExeCountState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_MSGACTS_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, HS_EXECOUNT_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventType, CFE_EVS_EventType_ERROR);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 3, "CFE_EVS_SendEvent was called %u time(s), expected 3",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabledNoSubscribeError2(void)
{
    HS_AppData.AppMonLoaded         = HS_STATE_DISABLED;
    HS_AppData.EventMonLoaded       = HS_STATE_DISABLED;
    HS_AppData.CurrentAppMonState   = HS_STATE_ENABLED;
    HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;
    HS_AppData.MsgActsState         = HS_STATE_ENABLED;
    HS_AppData.ExeCountState        = HS_STATE_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Causes event message HS_BADEMT_UNSUB_EID to not be generated */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 2, -1);

    /* Execute the function being tested */
    HS_AcquirePointers();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.AppMonLoaded == HS_STATE_DISABLED, "HS_AppData.AppMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.EventMonLoaded == HS_STATE_DISABLED, "HS_AppData.EventMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.MsgActsState == HS_STATE_DISABLED, "HS_AppData.MsgActsState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.ExeCountState == HS_STATE_DISABLED, "HS_AppData.ExeCountState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_APPMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_MSGACTS_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, HS_EXECOUNT_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventType, CFE_EVS_EventType_ERROR);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 3, "CFE_EVS_SendEvent was called %u time(s), expected 3",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AcquirePointers_Test_ErrorsWithCurrentAppMonLoadedDisabledAndCurrentAppMonStateDisabled(void)
{
    HS_AppData.AppMonLoaded         = HS_STATE_DISABLED;
    HS_AppData.EventMonLoaded       = HS_STATE_DISABLED;
    HS_AppData.CurrentAppMonState   = HS_STATE_DISABLED;
    HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;
    HS_AppData.MsgActsState         = HS_STATE_DISABLED;
    HS_AppData.ExeCountState        = HS_STATE_DISABLED;
    HS_AppData.MsgActsState         = HS_STATE_DISABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Causes event message HS_BADEMT_UNSUB_EID to not be generated */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_AcquirePointers();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentAppMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentAppMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.AppMonLoaded == HS_STATE_DISABLED, "HS_AppData.AppMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.CurrentEventMonState == HS_STATE_DISABLED,
                  "HS_AppData.CurrentEventMonState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.EventMonLoaded == HS_STATE_DISABLED, "HS_AppData.EventMonLoaded == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.MsgActsState == HS_STATE_DISABLED, "HS_AppData.MsgActsState == HS_STATE_DISABLED");
    UtAssert_True(HS_AppData.ExeCountState == HS_STATE_DISABLED, "HS_AppData.ExeCountState == HS_STATE_DISABLED");

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 1, "CFE_EVS_SendEvent was called %u time(s), expected 1",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppMonStatusRefresh_Test_CycleCountZero(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];
    uint32        i;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); i++)
    {
        HS_AppData.AppMonEnables[i] = 1 + i;
    }

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].CycleCount = 0;
    }

    /* Execute the function being tested */
    HS_AppMonStatusRefresh();

    /* Verify results */
    /* Check first, middle, and last element */
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_True(HS_AppData.AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] == 0,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == 0");

    UtAssert_True(HS_AppData.AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] == 0,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == 0");

    UtAssert_True(HS_AppData.AppMonLastExeCount[0] == 0, "HS_AppData.AppMonLastExeCount[0] == 0");
    UtAssert_True(HS_AppData.AppMonLastExeCount[HS_MAX_MONITORED_APPS / 2] == 0,
                  "HS_AppData.AppMonLastExeCount[HS_MAX_MONITORED_APPS / 2] == 0");
    UtAssert_UINT32_EQ(HS_AppData.AppMonLastExeCount[HS_MAX_MONITORED_APPS - 1], 0);

    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS / 2] == 0,
                  "HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS / 2] == 0");
    UtAssert_UINT16_EQ(HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS - 1], 0);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppMonStatusRefresh_Test_ActionTypeNOACT(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];
    uint32        i;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); i++)
    {
        HS_AppData.AppMonEnables[i] = 1 + i;
    }

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].CycleCount = 1;
        HS_AppData.AMTablePtr[i].ActionType = HS_AMT_ACT_NOACT;
    }

    /* Execute the function being tested */
    HS_AppMonStatusRefresh();

    /* Verify results */
    /* Check first, middle, and last element */
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0, "HS_AppData.AppMonEnables[0] == 0");

    UtAssert_True(HS_AppData.AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) / 2] == 0,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE) / 2] == 0");

    UtAssert_True(HS_AppData.AppMonEnables[(HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE] == 0,
                  "((HS_MAX_MONITORED_APPS -1) / HS_BITS_PER_APPMON_ENABLE] == 0");

    UtAssert_True(HS_AppData.AppMonLastExeCount[0] == 0, "HS_AppData.AppMonLastExeCount[0] == 0");
    UtAssert_True(HS_AppData.AppMonLastExeCount[HS_MAX_MONITORED_APPS / 2] == 0,
                  "HS_AppData.AppMonLastExeCount[HS_MAX_MONITORED_APPS / 2] == 0");
    UtAssert_UINT32_EQ(HS_AppData.AppMonLastExeCount[HS_MAX_MONITORED_APPS - 1], 0);

    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 0, "HS_AppData.AppMonCheckInCountdown[0] == 0");
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS / 2] == 0,
                  "HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS / 2] == 0");
    UtAssert_UINT16_EQ(HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS - 1], 0);

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_AppMonStatusRefresh_Test_ElseCase(void)
{
    HS_AMTEntry_t AMTable[HS_MAX_MONITORED_APPS];
    uint32        i;

    memset(AMTable, 0, sizeof(AMTable));

    HS_AppData.AMTablePtr = AMTable;

    for (i = 0; i <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); i++)
    {
        HS_AppData.AppMonEnables[i] = 1 + i;
    }

    for (i = 0; i < HS_MAX_MONITORED_APPS; i++)
    {
        HS_AppData.AMTablePtr[i].CycleCount = 1 + i;
        HS_AppData.AMTablePtr[i].ActionType = 99;
    }

    /* Execute the function being tested */
    HS_AppMonStatusRefresh();

    /* Verify results */
    UtAssert_True(HS_AppData.AppMonLastExeCount[0] == 0, "HS_AppData.AppMonLastExeCount[0] == 0");
    UtAssert_True(HS_AppData.AppMonLastExeCount[HS_MAX_MONITORED_APPS / 2] == 0,
                  "HS_AppData.AppMonLastExeCount[HS_MAX_MONITORED_APPS / 2] == 0");
    UtAssert_UINT32_EQ(HS_AppData.AppMonLastExeCount[HS_MAX_MONITORED_APPS - 1], 0);

    UtAssert_True(HS_AppData.AppMonCheckInCountdown[0] == 1, "HS_AppData.AppMonCheckInCountdown[0] == 1");
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS / 2] == (HS_MAX_MONITORED_APPS / 2) + 1,
                  "HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS / 2] == (HS_MAX_MONITORED_APPS / 2) + 1");
    UtAssert_True(HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS - 1] == (HS_MAX_MONITORED_APPS - 1) + 1,
                  "HS_AppData.AppMonCheckInCountdown[HS_MAX_MONITORED_APPS] == (HS_MAX_MONITORED_APPS - 1) + 1");

    /* Check first, middle, and last element */
    UtAssert_True(HS_AppData.AppMonEnables[0] == 0xFFFFFFFF, "HS_AppData.AppMonEnables[0] == 0xFFFFFFFF");

    UtAssert_True(
        HS_AppData.AppMonEnables[(((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) + 1) / 2] == 0xFFFFFFFF,
        "HS_AppData.AppMonEnables[(((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE)+1) / 2] == 0xFFFFFFFF");

    UtAssert_True(
        HS_AppData.AppMonEnables[(((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) + 1) - 1] == 0xFFFFFFFF,
        "HS_AppData.AppMonEnables[(((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE)+1) - 1] == 0xFFFFFFFF");

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

void HS_MsgActsStatusRefresh_Test(void)
{
    uint32 i;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MsgActCooldown[i] = 1 + i;
    }

    /* Execute the function being tested */
    HS_MsgActsStatusRefresh();

    /* Verify results */
    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        /* Check first, middle, and last element */
        UtAssert_True(HS_AppData.MsgActCooldown[0] == 0, "HS_AppData.MsgActCooldown[0] == 0");
        UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 0,
                      "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES / 2] == 0");
        UtAssert_True(HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES - 1] == 0,
                      "HS_AppData.MsgActCooldown[HS_MAX_MSG_ACT_TYPES -1] == 0");
    }

    call_count_CFE_EVS_SendEvent = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));
    UtAssert_True(call_count_CFE_EVS_SendEvent == 0, "CFE_EVS_SendEvent was called %u time(s), expected 0",
                  call_count_CFE_EVS_SendEvent);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_SendHkCmd_Test_InvalidEventMon, HS_Test_Setup, HS_Test_TearDown, "HS_SendHkCmd_Test_InvalidEventMon");

    UtTest_Add(HS_SendHkCmd_Test_AllFlagsEnabled, HS_Test_Setup, HS_Test_TearDown, "HS_SendHkCmd_Test_AllFlagsEnabled");
    UtTest_Add(HS_SendHkCmd_Test_ResourceTypeAppMain, HS_Test_Setup, HS_Test_TearDown,
               "HS_SendHkCmd_Test_ResourceTypeAppMain");
    UtTest_Add(HS_SendHkCmd_Test_ResourceTypeAppChild, HS_Test_Setup, HS_Test_TearDown,
               "HS_SendHkCmd_Test_ResourceTypeAppChild");
    UtTest_Add(HS_SendHkCmd_Test_ResourceTypeAppChildTaskIdError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SendHkCmd_Test_ResourceTypeAppChildTaskIdError");
    UtTest_Add(HS_SendHkCmd_Test_ResourceTypeAppChildTaskInfoError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SendHkCmd_Test_ResourceTypeAppChildTaskInfoError");
    UtTest_Add(HS_SendHkCmd_Test_ResourceTypeDevice, HS_Test_Setup, HS_Test_TearDown,
               "HS_SendHkCmd_Test_ResourceTypeDevice");
    UtTest_Add(HS_SendHkCmd_Test_ResourceTypeISR, HS_Test_Setup, HS_Test_TearDown, "HS_SendHkCmd_Test_ResourceTypeISR");
    UtTest_Add(HS_SendHkCmd_Test_ResourceTypeISRGenCounterError, HS_Test_Setup, HS_Test_TearDown,
               "HS_SendHkCmd_Test_ResourceTypeISRGenCounterError");
    UtTest_Add(HS_SendHkCmd_Test_ResourceTypeUnknown, HS_Test_Setup, HS_Test_TearDown,
               "HS_SendHkCmd_Test_ResourceTypeAppMain");

    UtTest_Add(HS_Noop_Test, HS_Test_Setup, HS_Test_TearDown, "HS_Noop_Test");

    UtTest_Add(HS_ResetCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_ResetCmd_Test");
    UtTest_Add(HS_ResetCounters_Test, HS_Test_Setup, HS_Test_TearDown, "HS_ResetCounters_Test");

    UtTest_Add(HS_EnableAppMonCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_EnableAppMonCmd_Test");

    UtTest_Add(HS_EnableAppMonCmd_Test_AlreadyEnabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_EnableAppMonCmd_Test_AlreadyEnabled");

    UtTest_Add(HS_DisableAppMonCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_DisableAppMonCmd_Test");

    UtTest_Add(HS_DisableAppMonCmd_Test_AlreadyDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_DisableAppMonCmd_Test_AlreadyDisabled");

    UtTest_Add(HS_EnableEventMonCmd_Test_Disabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_EnableEventMonCmd_Test_Disabled");

    UtTest_Add(HS_EnableEventMonCmd_Test_AlreadyEnabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_EnableEventMonCmd_Test_AlreadyEnabled");
    UtTest_Add(HS_EnableEventMonCmd_Test_SubscribeLongError, HS_Test_Setup, HS_Test_TearDown,
               "HS_EnableEventMonCmd_Test_SubscribeLongError");
    UtTest_Add(HS_EnableEventMonCmd_Test_SubscribeShortError, HS_Test_Setup, HS_Test_TearDown,
               "HS_EnableEventMonCmd_Test_SubscribeShortError");

    UtTest_Add(HS_DisableEventMonCmd_Test_Enabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_DisableEventMonCmd_Test_Enabled");
    UtTest_Add(HS_DisableEventMonCmd_Test_AlreadyDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_DisableEventMonCmd_Test_AlreadyDisabled");
    UtTest_Add(HS_DisableEventMonCmd_Test_UnsubscribeLongError, HS_Test_Setup, HS_Test_TearDown,
               "HS_DisableEventMonCmd_Test_UnsubscribeLongError");
    UtTest_Add(HS_DisableEventMonCmd_Test_UnsubscribeShortError, HS_Test_Setup, HS_Test_TearDown,
               "HS_DisableEventMonCmd_Test_UnsubscribeShortError");

    UtTest_Add(HS_EnableAlivenessCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_EnableAlivenessCmd_Test");

    UtTest_Add(HS_EnableAlivenessCmd_Test_AlreadyEnabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_EnableAlivenessCmd_Test_AlreadyEnabled");

    UtTest_Add(HS_DisableAlivenessCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_DisableAlivenessCmd_Test");

    UtTest_Add(HS_DisableAlivenessCmd_Test_AlreadyDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_DisableAlivenessCmd_Test_AlreadyDisabled");

    UtTest_Add(HS_EnableCpuHogCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_EnableCpuHogCmd_Test");

    UtTest_Add(HS_EnableCpuHogCmd_Test_AlreadyEnabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_EnableCpuHogCmd_Test_AlreadyEnabled");

    UtTest_Add(HS_DisableCpuHogCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_DisableCpuHogCmd_Test");

    UtTest_Add(HS_DisableCpuHogCmd_Test_AlreadyDisabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_DisableCpuHogCmd_Test_AlreadyDisabled");

    UtTest_Add(HS_ResetResetsPerformedCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_ResetResetsPerformedCmd_Test");

    UtTest_Add(HS_SetMaxResetsCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_SetMaxResetsCmd_Test");

    UtTest_Add(HS_AcquirePointers_Test_Nominal, HS_Test_Setup, HS_Test_TearDown, "HS_AcquirePointers_Test_Nominal");
    UtTest_Add(HS_AcquirePointers_Test_ErrorsWithAppMonLoadedAndEventMonLoadedEnabled, HS_Test_Setup, HS_Test_TearDown,
               "HS_AcquirePointers_Test_ErrorsWithAppMonLoadedAndEventMonLoadedEnabled");
    UtTest_Add(HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabled, HS_Test_Setup,
               HS_Test_TearDown, "HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabled");
    UtTest_Add(HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabled2, HS_Test_Setup,
               HS_Test_TearDown, "HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabled2");
    UtTest_Add(HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabledNoSubscribeError, HS_Test_Setup,
               HS_Test_TearDown,
               "HS_AcquirePointers_Test_ErrorsWithCurrentAppMonAndCurrentEventMonEnabledNoSubscribeError");
    UtTest_Add(HS_AcquirePointers_Test_ErrorsWithCurrentAppMonLoadedDisabledAndCurrentAppMonStateDisabled,
               HS_Test_Setup, HS_Test_TearDown,
               "HS_AcquirePointers_Test_ErrorsWithCurrentAppMonLoadedDisabledAndCurrentAppMonStateDisabled");

    UtTest_Add(HS_AppMonStatusRefresh_Test_CycleCountZero, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMonStatusRefresh_Test_CycleCountZero");
    UtTest_Add(HS_AppMonStatusRefresh_Test_ActionTypeNOACT, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMonStatusRefresh_Test_ActionTypeNOACT");
    UtTest_Add(HS_AppMonStatusRefresh_Test_ElseCase, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppMonStatusRefresh_Test_ElseCase");

    UtTest_Add(HS_MsgActsStatusRefresh_Test, HS_Test_Setup, HS_Test_TearDown, "HS_MsgActsStatusRefresh_Test");
}
