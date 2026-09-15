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

/*
 * Function Definitions
 */

void HS_SendHkCmd_Test_InvalidEventMon(void)
{
    CFE_SB_MsgId_t      TestMsgId;
    CFE_MSG_FcnCode_t   FcnCode;
    size_t              MsgSize;
    HS_EMTEntry_t       EMTable[HS_MAX_MONITORED_EVENTS];
    HS_AppMon_State_t  *AMStatePtr;
    HS_HkTlm_Payload_t *PayloadPtr;
    HS_HkPacket_t       HkPkt;
    void               *HkPtr = &HkPkt;

    memset(EMTable, 0, sizeof(EMTable));

    HS_AppData.EMTablePtr = EMTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    memset(&HkPkt, 0xEE, sizeof(HkPkt));
    UT_SetDataBuffer(UT_KEY(CFE_SB_AllocateMessageBuffer), &HkPtr, sizeof(HkPtr), false);

    /* 2 entries that are not HS_EMTActType_NOACT for branch coverage */
    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_NOACT + 1;
    HS_AppData.EMTablePtr[1].ActionType = HS_EMTActType_NOACT + 1;

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
    HS_AppData.InactiveEventMonCount   = 11;

    AMStatePtr         = HS_GetAMStateByIndex(0);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    AMStatePtr->Enable = true;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SendHkCmd(&UT_CmdBuf.SendHkCmd), CFE_SUCCESS);

    /* Verify results */
    PayloadPtr = &HkPkt.Payload;
    UtAssert_UINT32_EQ(PayloadPtr->CmdCount, 1);
    UtAssert_UINT32_EQ(PayloadPtr->CmdErrCount, 2);
    UtAssert_UINT32_EQ(PayloadPtr->CurrentAppMonState, 3);
    UtAssert_UINT32_EQ(PayloadPtr->CurrentEventMonState, 4);
    UtAssert_UINT32_EQ(PayloadPtr->CurrentAlivenessState, 5);
    UtAssert_UINT32_EQ(PayloadPtr->CurrentCPUHogState, 6);
    UtAssert_UINT32_EQ(PayloadPtr->ResetsPerformed, 7);
    UtAssert_UINT32_EQ(PayloadPtr->MaxResets, 8);
    UtAssert_UINT32_EQ(PayloadPtr->EventsMonitoredCount, 9);
    UtAssert_UINT32_EQ(PayloadPtr->MsgActExec, 10);
    UtAssert_UINT32_EQ(PayloadPtr->InactiveEventMonCount, 11);

    /* Check first, middle, and last element */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_SendHkCmd_Test_NullEventMonTable(void)
{
    CFE_SB_MsgId_t      TestMsgId;
    CFE_MSG_FcnCode_t   FcnCode;
    size_t              MsgSize;
    HS_AppMon_State_t  *AMStatePtr;
    HS_HkTlm_Payload_t *PayloadPtr;
    HS_HkPacket_t       HkPkt;
    void               *HkPtr = &HkPkt;

    /* setting this to null should prevent UUT from incrementing InactiveEventMonCount */
    HS_AppData.EMTablePtr = NULL;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    memset(&HkPkt, 0xEE, sizeof(HkPkt));
    UT_SetDataBuffer(UT_KEY(CFE_SB_AllocateMessageBuffer), &HkPtr, sizeof(HkPtr), false);

    /* Fail first, succeed on second (shouldn't get called, but set up anyway)*/
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);

    /* initialize hk counters */
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

    AMStatePtr         = HS_GetAMStateByIndex(0);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    AMStatePtr->Enable = true;
    AMStatePtr         = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    AMStatePtr->Enable = true;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SendHkCmd(&UT_CmdBuf.SendHkCmd), CFE_SUCCESS);

    /* Verify general housekeeping fields weren't affected */
    PayloadPtr = &HkPkt.Payload;
    UtAssert_UINT8_EQ(PayloadPtr->CmdCount, 1);
    UtAssert_UINT8_EQ(PayloadPtr->CmdErrCount, 2);
    UtAssert_UINT8_EQ(PayloadPtr->CurrentAppMonState, 3);
    UtAssert_UINT8_EQ(PayloadPtr->CurrentEventMonState, 4);
    UtAssert_UINT8_EQ(PayloadPtr->CurrentAlivenessState, 5);
    UtAssert_UINT8_EQ(PayloadPtr->CurrentCPUHogState, 6);
    UtAssert_UINT16_EQ(PayloadPtr->ResetsPerformed, 7);
    UtAssert_UINT16_EQ(PayloadPtr->MaxResets, 8);
    UtAssert_UINT32_EQ(PayloadPtr->EventsMonitoredCount, 9);
    UtAssert_UINT32_EQ(PayloadPtr->MsgActExec, 10);

    /* if InactiveEventMonCount was incremented, we did something wrong */
    UtAssert_UINT32_EQ(PayloadPtr->InactiveEventMonCount, 0);

    /* Check first, middle, and last element */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    UtAssert_BOOL_TRUE(AMStatePtr->Enable);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetAppIDByName, 0);
}

void HS_SendHkCmd_Test_AllFlagsEnabled(void)
{
    CFE_SB_MsgId_t      TestMsgId;
    CFE_MSG_FcnCode_t   FcnCode;
    size_t              MsgSize;
    HS_EMTEntry_t       EMTable[HS_MAX_MONITORED_EVENTS];
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    uint8               ExpectedStatusFlags = 0;
    int                 i;
    HS_AppMon_State_t  *AMStatePtr;
    HS_HkPacket_t       HkPkt;
    void               *HkPtr = &HkPkt;
    HS_HkTlm_Payload_t *PayloadPtr;

    memset(EMTable, 0, sizeof(EMTable));
    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_NOTYPE;
    }

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.XCTablePtr = XCTable;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    memset(&HkPkt, 0xEE, sizeof(HkPkt));
    UT_SetDataBuffer(UT_KEY(CFE_SB_AllocateMessageBuffer), &HkPtr, sizeof(HkPtr), false);
    UT_SetDataBuffer(UT_KEY(HS_ComputeStatusFlags), &ExpectedStatusFlags, sizeof(ExpectedStatusFlags), false);

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_NOACT;

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

    HS_AppData.ExecMonLoaded  = HS_State_ENABLED;
    HS_AppData.MsgActsLoaded  = HS_State_ENABLED;
    HS_AppData.AppMonLoaded   = HS_State_ENABLED;
    HS_AppData.EventMonLoaded = HS_State_ENABLED;
    HS_AppData.CDSState       = HS_State_ENABLED;

    ExpectedStatusFlags |= HS_StatusFlag_LOADED_XCT;
    ExpectedStatusFlags |= HS_StatusFlag_LOADED_MAT;
    ExpectedStatusFlags |= HS_StatusFlag_LOADED_AMT;
    ExpectedStatusFlags |= HS_StatusFlag_LOADED_EMT;
    ExpectedStatusFlags |= HS_StatusFlag_CDS_IN_USE;

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_SendHkCmd(&UT_CmdBuf.SendHkCmd), CFE_SUCCESS);

    /* Verify results */
    PayloadPtr = &HkPkt.Payload;
    UtAssert_UINT32_EQ(PayloadPtr->CmdCount, 1);
    UtAssert_UINT32_EQ(PayloadPtr->CmdErrCount, 2);
    UtAssert_UINT32_EQ(PayloadPtr->CurrentAppMonState, 3);
    UtAssert_UINT32_EQ(PayloadPtr->CurrentEventMonState, 4);
    UtAssert_UINT32_EQ(PayloadPtr->CurrentAlivenessState, 5);
    UtAssert_UINT32_EQ(PayloadPtr->CurrentCPUHogState, 6);
    UtAssert_UINT32_EQ(PayloadPtr->ResetsPerformed, 7);
    UtAssert_UINT32_EQ(PayloadPtr->MaxResets, 8);
    UtAssert_UINT32_EQ(PayloadPtr->EventsMonitoredCount, 9);
    UtAssert_UINT32_EQ(PayloadPtr->MsgActExec, 10);
    UtAssert_UINT32_EQ(PayloadPtr->InactiveEventMonCount, 0);

    UtAssert_UINT32_EQ(PayloadPtr->StatusFlags, ExpectedStatusFlags);

    /* Check first, middle, and last element */
    AMStatePtr = HS_GetAMStateByIndex(0);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS / 2);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);

    AMStatePtr = HS_GetAMStateByIndex(HS_MAX_MONITORED_APPS - 1);
    UtAssert_BOOL_FALSE(AMStatePtr->Enable);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_SendHkCmd_Test_AllocateFail(void)
{
    /* with no set up, CFE_SB_AllocateMessageBuffer() returns NULL and this immediately returns */
    UtAssert_INT32_EQ(HS_SendHkCmd(NULL), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);
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
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_NOOP_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 0);
    UtAssert_UINT32_EQ(HS_AppData.CmdErrCount, 0);
    UtAssert_UINT32_EQ(HS_AppData.EventsMonitoredCount, 0);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 0);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_RESET_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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
    HS_AppData.CurrentAppMonState = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_EnableAppMonCmd(&UT_CmdBuf.EnableAppMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_APPMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EnableAppMonCmd_Test_NotLoaded(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_APP_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableAppMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.AMTablePtr = NULL;

    /* Set to Disabled so the function can freshly set it to Enabled (without it already being set that way) */
    HS_AppData.CurrentAppMonState = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_EnableAppMonCmd(&UT_CmdBuf.EnableAppMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_APPMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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
    HS_AppData.CurrentAppMonState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_EnableAppMonCmd(&UT_CmdBuf.EnableAppMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_APPMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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
    HS_AppData.CurrentAppMonState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_DisableAppMonCmd(&UT_CmdBuf.DisableAppMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_APPMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_DisableAppMonCmd_Test_AlreadyDisabled(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Application Monitoring is *already* Disabled");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_APP_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableAppMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    /* Set to Disabled to test response when already in the commanded state. */
    HS_AppData.CurrentAppMonState = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_DisableAppMonCmd(&UT_CmdBuf.DisableAppMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentAppMonState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_APPMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentEventMonState = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_EnableEventMonCmd(&UT_CmdBuf.EnableEventMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_EVENTMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentEventMonState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_EnableEventMonCmd(&UT_CmdBuf.EnableEventMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_EVENTMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EnableEventMonCmd_Test_SubscribeLongError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor Enable: Error Subscribing to long-format Events,RC=0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_State_DISABLED;

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_SubscribeEx), 1, -1);

    /* Execute the function being tested */
    HS_EnableEventMonCmd(&UT_CmdBuf.EnableEventMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdErrCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_LONG_SUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EnableEventMonCmd_Test_SubscribeShortError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor Enable: Error Subscribing to short-format Events,RC=0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_ENABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.EnableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_State_DISABLED;

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_SubscribeEx), 2, -1);

    /* Execute the function being tested */
    HS_EnableEventMonCmd(&UT_CmdBuf.EnableEventMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdErrCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_SHORT_SUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentEventMonState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_DisableEventMonCmd(&UT_CmdBuf.DisableEventMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_EVENTMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentEventMonState = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_DisableEventMonCmd(&UT_CmdBuf.DisableEventMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_EVENTMON_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_DisableEventMonCmd_Test_UnsubscribeLongError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor Disable: Error Unsubscribing from long-format Events,RC=0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_State_ENABLED;

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 1, -1);

    /* Execute the function being tested */
    HS_DisableEventMonCmd(&UT_CmdBuf.DisableEventMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdErrCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_LONG_UNSUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_DisableEventMonCmd_Test_UnsubscribeShortError(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor Disable: Error Unsubscribing from short-format Events,RC=0x%%08X");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    FcnCode   = HS_DISABLE_EVENT_MON_CC;
    MsgSize   = sizeof(UT_CmdBuf.DisableEventMonCmd);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);

    HS_AppData.CurrentEventMonState = HS_State_ENABLED;

    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 2, -1);

    /* Execute the function being tested */
    HS_DisableEventMonCmd(&UT_CmdBuf.DisableEventMonCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdErrCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_SHORT_UNSUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentAlivenessState = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_EnableAlivenessCmd(&UT_CmdBuf.EnableAlivenessCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentAlivenessState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_ALIVENESS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentAlivenessState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_EnableAlivenessCmd(&UT_CmdBuf.EnableAlivenessCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentAlivenessState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_ALIVENESS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentAlivenessState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_DisableAlivenessCmd(&UT_CmdBuf.DisableAlivenessCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentAlivenessState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_ALIVENESS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentAlivenessState = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_DisableAlivenessCmd(&UT_CmdBuf.DisableAlivenessCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentAlivenessState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_ALIVENESS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentCPUHogState = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_EnableCpuHogCmd(&UT_CmdBuf.EnableCpuHogCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentCPUHogState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_CPUHOG_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentCPUHogState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_EnableCpuHogCmd(&UT_CmdBuf.EnableCpuHogCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentCPUHogState, HS_State_ENABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_ENABLE_CPUHOG_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentCPUHogState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_DisableCpuHogCmd(&UT_CmdBuf.DisableCpuHogCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentCPUHogState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_CPUHOG_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
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

    HS_AppData.CurrentCPUHogState = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_DisableCpuHogCmd(&UT_CmdBuf.DisableCpuHogCmd);

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_UINT32_EQ(HS_AppData.CurrentCPUHogState, HS_State_DISABLED);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_DISABLE_CPUHOG_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_ResetResetsPerformedCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
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
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_RESET_RESETS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_SetMaxResetsCmd_Test(void)
{
    CFE_SB_MsgId_t    TestMsgId;
    CFE_MSG_FcnCode_t FcnCode;
    size_t            MsgSize;
    int32             strCmpResult;
    char              ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    HS_SetMaxResets_Payload_t *PayloadPtr;

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
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
    UtAssert_UINT32_EQ(HS_AppData.CmdCount, 1);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_SET_MAX_RESETS_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_UINT32_EQ(strCmpResult, 0);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_SendHkCmd_Test_InvalidEventMon, HS_Test_Setup, HS_Test_TearDown, "HS_SendHkCmd_Test_InvalidEventMon");
    UtTest_Add(HS_SendHkCmd_Test_NullEventMonTable,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_SendHkCmd_Test_NullEventMonTable");

    UtTest_Add(HS_SendHkCmd_Test_AllFlagsEnabled, HS_Test_Setup, HS_Test_TearDown, "HS_SendHkCmd_Test_AllFlagsEnabled");
    UtTest_Add(HS_SendHkCmd_Test_AllocateFail, HS_Test_Setup, HS_Test_TearDown, "HS_SendHkCmd_Test_AllocateFail");

    UtTest_Add(HS_Noop_Test, HS_Test_Setup, HS_Test_TearDown, "HS_Noop_Test");

    UtTest_Add(HS_ResetCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_ResetCmd_Test");

    UtTest_Add(HS_EnableAppMonCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_EnableAppMonCmd_Test");

    UtTest_Add(HS_EnableAppMonCmd_Test_NotLoaded, HS_Test_Setup, HS_Test_TearDown, "HS_EnableAppMonCmd_Test_NotLoaded");
    UtTest_Add(HS_EnableAppMonCmd_Test_AlreadyEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EnableAppMonCmd_Test_AlreadyEnabled");

    UtTest_Add(HS_DisableAppMonCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_DisableAppMonCmd_Test");

    UtTest_Add(HS_DisableAppMonCmd_Test_AlreadyDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_DisableAppMonCmd_Test_AlreadyDisabled");

    UtTest_Add(HS_EnableEventMonCmd_Test_Disabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EnableEventMonCmd_Test_Disabled");

    UtTest_Add(HS_EnableEventMonCmd_Test_AlreadyEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EnableEventMonCmd_Test_AlreadyEnabled");
    UtTest_Add(HS_EnableEventMonCmd_Test_SubscribeLongError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EnableEventMonCmd_Test_SubscribeLongError");
    UtTest_Add(HS_EnableEventMonCmd_Test_SubscribeShortError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EnableEventMonCmd_Test_SubscribeShortError");

    UtTest_Add(HS_DisableEventMonCmd_Test_Enabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_DisableEventMonCmd_Test_Enabled");
    UtTest_Add(HS_DisableEventMonCmd_Test_AlreadyDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_DisableEventMonCmd_Test_AlreadyDisabled");
    UtTest_Add(HS_DisableEventMonCmd_Test_UnsubscribeLongError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_DisableEventMonCmd_Test_UnsubscribeLongError");
    UtTest_Add(HS_DisableEventMonCmd_Test_UnsubscribeShortError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_DisableEventMonCmd_Test_UnsubscribeShortError");

    UtTest_Add(HS_EnableAlivenessCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_EnableAlivenessCmd_Test");

    UtTest_Add(HS_EnableAlivenessCmd_Test_AlreadyEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EnableAlivenessCmd_Test_AlreadyEnabled");

    UtTest_Add(HS_DisableAlivenessCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_DisableAlivenessCmd_Test");

    UtTest_Add(HS_DisableAlivenessCmd_Test_AlreadyDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_DisableAlivenessCmd_Test_AlreadyDisabled");

    UtTest_Add(HS_EnableCpuHogCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_EnableCpuHogCmd_Test");

    UtTest_Add(HS_EnableCpuHogCmd_Test_AlreadyEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EnableCpuHogCmd_Test_AlreadyEnabled");

    UtTest_Add(HS_DisableCpuHogCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_DisableCpuHogCmd_Test");

    UtTest_Add(HS_DisableCpuHogCmd_Test_AlreadyDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_DisableCpuHogCmd_Test_AlreadyDisabled");

    UtTest_Add(HS_ResetResetsPerformedCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_ResetResetsPerformedCmd_Test");

    UtTest_Add(HS_SetMaxResetsCmd_Test, HS_Test_Setup, HS_Test_TearDown, "HS_SetMaxResetsCmd_Test");
}
