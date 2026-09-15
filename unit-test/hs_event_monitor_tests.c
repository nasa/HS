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

#include "hs_event_monitor.h"
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

void HS_EventMon_Check_Test_EventMonTblPtrNull(void)
{
    CFE_EVS_LongEventTlm_t Packet;

    /*
    ** Initialize a basic packet dummy input.
    ** the UUT assumes this input pointer has been checked for null
    ** by the calling function, `HS_ProcessCommands`
    */
    memset(&Packet, 0, sizeof(Packet));
    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    /* Initialize the EMT to null to drive our pointer check test case */
    HS_AppData.EMTablePtr = NULL;

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    /*
    ** Verify UUT exited before processing info from the EMT
    ** Since the UUT simply returns during this, the best thing we can do is
    ** verify that none of the stub functions were called
    */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(OS_TaskDelay, 0);
    UtAssert_STUB_COUNT(CFE_ES_WriteToSysLog, 0);
    UtAssert_STUB_COUNT(CFE_ES_ResetCFE, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetAppIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_RestartApp, 0);
    UtAssert_STUB_COUNT(CFE_ES_DeleteApp, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
}

void HS_EventMon_Check_Test_AppName(void)
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
    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_PROC_RESET;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;
    HS_AppData.CDSData.MaxResets        = 10;
    HS_AppData.CDSData.ResetsPerformed  = 1;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);

    /* Active table entry where event doesn't match */
    HS_AppData.EMTablePtr[1].ActionType = HS_EMTActType_PROC_RESET;

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

    /* Verify results */

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_EventMon_Check_Test_ProcErrorReset(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    char                   ExpectedSysLogString[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Processor Reset");
    snprintf(ExpectedSysLogString,
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "HS App: Event Monitor: APP:(%%s) EID:(%%d): Action: Processor Reset\n");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_PROC_RESET;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;
    HS_AppData.CDSData.MaxResets        = 10;
    HS_AppData.CDSData.ResetsPerformed  = 1;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_PROC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);

    strCmpResult = strncmp(ExpectedSysLogString, context_CFE_ES_WriteToSysLog.Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Sys Log string matched expected result, '%s'", context_CFE_ES_WriteToSysLog.Spec);

    UtAssert_True(HS_AppData.ServiceWatchdogFlag == HS_State_DISABLED,
                  "HS_AppData.ServiceWatchdogFlag == HS_State_DISABLED");
}

void HS_EventMon_Check_Test_ProcErrorNoReset(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Processor Reset");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Processor Reset Action Limit Reached: No Reset Performed");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_PROC_RESET;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;
    HS_AppData.CDSData.MaxResets        = 10;
    HS_AppData.CDSData.ResetsPerformed  = 11;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

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

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_EventMon_Check_Test_AppRestartErrors(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Restart Application");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Restart App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_APP_RESTART;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_RestartApp to return -1, in order to generate error message HS_EVENTMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_RestartApp), 1, -1);

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

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

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_EventMon_Check_Test_OnlySecondAppRestartError(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Restart App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_APP_RESTART;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to return -1, in order to generate error message HS_EVENTMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_NOT_RESTARTED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EventMon_Check_Test_NoSecondAppRestartError(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Restart Application");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_APP_RESTART;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to succeed on first call and bypass error HS_EVENTMON_NOT_RESTARTED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_RESTART_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EventMon_Check_Test_DeleteErrors(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Delete Application");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Delete App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_APP_DELETE;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_DeleteApp to return -1, in order to generate error message HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_DeleteApp), 1, -1);

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

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

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_EventMon_Check_Test_OnlySecondDeleteError(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Call to Delete App Failed: APP:(%%s) ERR: 0x%%08X");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_APP_DELETE;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to fail on first call, to generate error HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, -1);

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_NOT_DELETED_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EventMon_Check_Test_NoSecondDeleteError(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_APPS];
    HS_MATEntry_t          MATable[HS_MAX_MSG_ACT_TYPES];
    CFE_EVS_LongEventTlm_t Packet;
    int32                  strCmpResult;
    char                   ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    memset(EMTable, 0, sizeof(EMTable));
    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Event Monitor: APP:(%%s) EID:(%%d): Action: Delete Application");

    HS_AppData.MATablePtr = &MATable[0];

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;
    HS_AppData.MATablePtr = MATable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_APP_DELETE;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", 10);
    strncpy(Packet.Payload.PacketID.AppName, "AppName", 10);

    /* Set CFE_ES_GetAppIDByName to succeed on first call and bypass error HS_EVENTMON_NOT_DELETED_ERR_EID */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 1, CFE_SUCCESS);

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_DELETE_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EventMon_Check_Test_MsgAction(void)
{
    HS_EMTEntry_t          EMTable[HS_MAX_MONITORED_EVENTS];
    CFE_EVS_LongEventTlm_t Packet;
    HS_MsgActState_t      *MAStatePtr;

    memset(EMTable, 0, sizeof(EMTable));

    CFE_MSG_Init((CFE_MSG_Message_t *)&Packet, CFE_SB_ValueToMsgId(HS_CMD_MID), sizeof(CFE_EVS_LongEventTlm_t));

    Packet.Payload.PacketID.EventID = 3;

    HS_AppData.EMTablePtr = EMTable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_LAST_NONMSG + 1;
    HS_AppData.EMTablePtr[0].EventID    = Packet.Payload.PacketID.EventID;

    strcpy(HS_AppData.EMTablePtr[0].AppName, "ut");
    strcpy(Packet.Payload.PacketID.AppName, "ut");

    HS_AppData.MsgActsLoaded = HS_State_ENABLED;
    MAStatePtr               = HS_GetMAStateByIndex(0);
    MAStatePtr->Cooldown     = 0;

    /* Execute the function being tested */
    HS_EventMon_Check(&Packet);

    /* Verify results */
    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_EventMon_TriggerAction_Test_EventCallback(void)
{
    HS_EMTEntry_t        EMTable[HS_MAX_MONITORED_EVENTS];
    HS_EventMon_State_t *EMStatePtr;

    memset(EMTable, 0, sizeof(EMTable));

    HS_AppData.EMTablePtr = EMTable;

    HS_AppData.EMTablePtr[0].ActionType = HS_EMTActType_LAST_NONMSG + 1;
    strncpy(HS_AppData.EMTablePtr[0].AppName, "ut", sizeof(HS_AppData.EMTablePtr[0].AppName));

    EMStatePtr = HS_GetEMStateByIndex(0);

    EMStatePtr->Enable = true;
    EMStatePtr->AppId  = CFE_ES_APPID_C(CFE_ResourceId_FromInteger(111));

    /* Prevents "failure to get an execution counter" */
    UT_SetHandlerFunction(UT_KEY(HS_MsgAct_TriggerAction), UT_Handler_HS_MsgAct_TriggerAction, &EMTable[0]);

    /* Execute the function being tested */
    HS_EventMon_TriggerAction(&EMTable[0]);

    /* Verify results */
    UtAssert_STUB_COUNT(HS_MsgAct_TriggerAction, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_MSGACTS_ERR_EID);
}

void HS_EventMon_ValidateTable_Test_UnusedTableEntryEventIDZero(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.EMTablePtr = EMTable;

    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].ActionType = 99;
        HS_AppData.EMTablePtr[i].EventID    = 0;
        HS_AppData.EMTablePtr[i].NullTerm   = 0;
    }

    /* Execute the function being tested */
    Result = HS_EventMon_ValidateTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EventMon_ValidateTable_Test_UnusedTableEntryActionTypeNOACT(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.EMTablePtr = EMTable;

    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].ActionType = HS_EMTActType_NOACT;
        HS_AppData.EMTablePtr[i].EventID    = 1;
        HS_AppData.EMTablePtr[i].NullTerm   = 0;
    }

    /* Execute the function being tested */
    Result = HS_EventMon_ValidateTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EventMon_ValidateTable_Test_BufferNotNull(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify err: Entry = %%d, Err = %%d, Action = %%d, ID = %%d App = %%s");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
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
    Result = HS_EventMon_ValidateTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVal_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_EMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_EMTVal_ERR_NUL, "Result == HS_EMTVal_ERR_NUL");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_EventMon_ValidateTable_Test_ActionTypeNotValid(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify err: Entry = %%d, Err = %%d, Action = %%d, ID = %%d App = %%s");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.EMTablePtr = EMTable;

    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].ActionType = HS_EMTActType_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1;
        HS_AppData.EMTablePtr[i].EventID    = 1;
        HS_AppData.EMTablePtr[i].NullTerm   = 0;
    }

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_EventMon_ValidateTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVal_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_EMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_AMTVal_ERR_ACT, "Result == HS_AMTVal_ERR_ACT");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_EventMon_ValidateTable_Test_EntryGood(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_EMTEntry_t EMTable[HS_MAX_MONITORED_EVENTS];

    memset(EMTable, 0, sizeof(EMTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "EventMon verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.EMTablePtr = EMTable;

    for (i = 0; i < HS_MAX_MONITORED_EVENTS; i++)
    {
        HS_AppData.EMTablePtr[i].ActionType = HS_EMTActType_LAST_NONMSG;
        HS_AppData.EMTablePtr[i].EventID    = 1;
        HS_AppData.EMTablePtr[i].NullTerm   = 0;
    }

    strncpy(HS_AppData.EMTablePtr[0].AppName, "AppName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_EventMon_ValidateTable(HS_AppData.EMTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EMTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EventMon_ValidateTable_Test_Null(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error in EM Table Validation. Table is null.");

    /* Execute the function being tested */
    Result = HS_EventMon_ValidateTable(NULL);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EM_TBL_NULL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == HS_TBL_VAL_ERR, "Result == HS_TBL_VAL_ERR");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_EventMon_AcquirePointers_Test_NominalNotLoaded(void)
{
    HS_EMTEntry_t        EMTable[HS_MAX_MONITORED_EVENTS];
    HS_EMTEntry_t       *EMTablePtr = EMTable;
    HS_EventMon_State_t *EMStatePtr;

    memset(EMTable, 0, sizeof(EMTable));

    EMTable[0].ActionType = HS_EMTActType_LAST_NONMSG + 1;
    EMTable[1].ActionType = HS_EMTActType_PROC_RESET;
    EMTable[2].ActionType = HS_EMTActType_APP_RESTART;
    EMTable[3].ActionType = HS_EMTActType_APP_DELETE;

    HS_AppData.EventMonLoaded       = HS_State_DISABLED;
    HS_AppData.CurrentEventMonState = HS_State_DISABLED;

    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_SUCCESS);
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &EMTablePtr, sizeof(EMTablePtr), false);

    /* Execute the function being tested */
    HS_EventMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.EventMonLoaded, HS_State_ENABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    EMStatePtr = HS_GetEMStateByIndex(0);
    UtAssert_BOOL_TRUE(EMStatePtr->Enable);

    EMStatePtr = HS_GetEMStateByIndex(HS_MAX_MONITORED_EVENTS - 1);
    UtAssert_BOOL_FALSE(EMStatePtr->Enable);
}

void HS_EventMon_AcquirePointers_Test_NominalUpdate(void)
{
    HS_EMTEntry_t        EMTable[HS_MAX_MONITORED_EVENTS];
    HS_EMTEntry_t       *EMTablePtr = EMTable;
    HS_EventMon_State_t *EMStatePtr;

    memset(EMTable, 0, sizeof(EMTable));

    EMTable[0].ActionType = HS_EMTActType_LAST_NONMSG + 1;
    EMTable[1].ActionType = HS_EMTActType_PROC_RESET;
    EMTable[2].ActionType = HS_EMTActType_APP_RESTART;
    EMTable[3].ActionType = HS_EMTActType_APP_DELETE;

    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &EMTablePtr, sizeof(EMTablePtr), false);

    HS_AppData.EventMonLoaded       = HS_State_ENABLED;
    HS_AppData.CurrentEventMonState = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_EventMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_ENABLED);
    UtAssert_UINT32_EQ(HS_AppData.EventMonLoaded, HS_State_ENABLED);

    EMStatePtr = HS_GetEMStateByIndex(0);
    UtAssert_BOOL_TRUE(EMStatePtr->Enable);

    EMStatePtr = HS_GetEMStateByIndex(HS_MAX_MONITORED_EVENTS - 1);
    UtAssert_BOOL_FALSE(EMStatePtr->Enable);
}

void HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonNotLoaded(void)
{
    HS_AppData.EventMonLoaded       = HS_State_DISABLED;
    HS_AppData.CurrentEventMonState = HS_State_DISABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_EventMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_DISABLED);
    UtAssert_UINT32_EQ(HS_AppData.EventMonLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_SB_Unsubscribe, 0);
}

void HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonDisabled(void)
{
    HS_AppData.EventMonLoaded       = HS_State_ENABLED;
    HS_AppData.CurrentEventMonState = HS_State_DISABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_EventMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_DISABLED);
    UtAssert_UINT32_EQ(HS_AppData.EventMonLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_STUB_COUNT(CFE_SB_Unsubscribe, 0);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonEnabled(void)
{
    HS_AppData.EventMonLoaded       = HS_State_ENABLED;
    HS_AppData.CurrentEventMonState = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Causes event message HS_BADEMT_LONG_UNSUB_EID to be generated */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 1, -1);

    /* Causes event message HS_BADEMT_SHORT_UNSUB_EID to be generated */
    UT_SetDeferredRetcode(UT_KEY(CFE_SB_Unsubscribe), 1, -1);

    /* Execute the function being tested */
    HS_EventMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_DISABLED);
    UtAssert_UINT32_EQ(HS_AppData.EventMonLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 3);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_BADEMT_LONG_UNSUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventID, HS_BADEMT_SHORT_UNSUB_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[2].EventType, CFE_EVS_EventType_ERROR);
}

void HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonNotLoadedEnabled(void)
{
    /* this is a combo that should never happen but needs coverage testing */
    HS_AppData.EventMonLoaded       = HS_State_DISABLED;
    HS_AppData.CurrentEventMonState = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_EventMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_DISABLED);
    UtAssert_UINT32_EQ(HS_AppData.EventMonLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonEnabledNoSubscribeError(void)
{
    HS_AppData.EventMonLoaded       = HS_State_ENABLED;
    HS_AppData.CurrentEventMonState = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_EventMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentEventMonState, HS_State_DISABLED);
    UtAssert_UINT32_EQ(HS_AppData.EventMonLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EVENTMON_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_EventMon_CheckEntries_Test_InvalidName(void)
{
    HS_EMTEntry_t        EMTable[HS_MAX_MONITORED_EVENTS];
    HS_EventMon_State_t *EMStatePtr;

    memset(EMTable, 0, sizeof(EMTable));

    EMTable[0].ActionType = HS_EMTActType_LAST_NONMSG + 1;
    strncpy(EMTable[0].AppName, "UT", sizeof(EMTable[0].AppName));
    EMStatePtr         = HS_GetEMStateByIndex(0);
    EMStatePtr->Enable = true;
    EMStatePtr->AppId  = CFE_ES_APPID_C(CFE_ResourceId_FromInteger(100));

    EMTable[1].ActionType = HS_EMTActType_PROC_RESET;
    strncpy(EMTable[1].AppName, "UT", sizeof(EMTable[1].AppName));
    EMStatePtr         = HS_GetEMStateByIndex(1);
    EMStatePtr->Enable = true;
    EMStatePtr->AppId  = CFE_ES_APPID_C(CFE_ResourceId_FromInteger(101));

    EMTable[2].ActionType = HS_EMTActType_APP_RESTART;
    strncpy(EMTable[2].AppName, "x", sizeof(EMTable[2].AppName));
    EMStatePtr         = HS_GetEMStateByIndex(2);
    EMStatePtr->Enable = true;
    EMStatePtr->AppId  = CFE_ES_APPID_C(CFE_ResourceId_FromInteger(102));

    EMTable[3].ActionType = HS_EMTActType_APP_DELETE;
    strncpy(EMTable[3].AppName, "x", sizeof(EMTable[3].AppName));
    EMStatePtr         = HS_GetEMStateByIndex(3);
    EMStatePtr->Enable = true;
    EMStatePtr->AppId  = CFE_ES_APPID_C(CFE_ResourceId_FromInteger(103));

    HS_AppData.EMTablePtr           = EMTable;
    HS_AppData.EventMonLoaded       = HS_State_ENABLED;
    HS_AppData.CurrentEventMonState = HS_State_ENABLED;

    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppName), 2, -1);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetAppIDByName), 2, -1);
    UtAssert_VOIDCALL(HS_EventMon_CheckEntries());

    UtAssert_UINT32_EQ(HS_AppData.InactiveEventMonCount, 1);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_EventMon_Check_Test_EventMonTblPtrNull,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_Check_Test_EventMonTblPtrNull");
    UtTest_Add(HS_EventMon_Check_Test_AppName, HS_Test_Setup, HS_Test_TearDown, "HS_EventMon_Check_Test_AppName");
    UtTest_Add(HS_EventMon_Check_Test_ProcErrorReset,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_Check_Test_ProcErrorReset");
    UtTest_Add(HS_EventMon_Check_Test_ProcErrorNoReset,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_Check_Test_ProcErrorNoReset");
    UtTest_Add(HS_EventMon_Check_Test_AppRestartErrors,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_Check_Test_AppRestartErrors");
    UtTest_Add(HS_EventMon_Check_Test_OnlySecondAppRestartError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_Check_Test_OnlySecondAppRestartError");
    UtTest_Add(HS_EventMon_Check_Test_NoSecondAppRestartError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_Check_Test_NoSecondAppRestartError");
    UtTest_Add(HS_EventMon_Check_Test_DeleteErrors,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_Check_Test_DeleteErrors");
    UtTest_Add(HS_EventMon_Check_Test_OnlySecondDeleteError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_Check_Test_OnlySecondDeleteError");
    UtTest_Add(HS_EventMon_Check_Test_NoSecondDeleteError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_Check_Test_NoSecondDeleteError");
    UtTest_Add(HS_EventMon_Check_Test_MsgAction, HS_Test_Setup, HS_Test_TearDown, "HS_EventMon_Check_Test_MsgAction");

    UtTest_Add(HS_EventMon_TriggerAction_Test_EventCallback,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_TriggerAction_Test_EventCallback");

    UtTest_Add(HS_EventMon_ValidateTable_Test_UnusedTableEntryEventIDZero,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_ValidateTable_Test_UnusedTableEntryEventIDZero");
    UtTest_Add(HS_EventMon_ValidateTable_Test_UnusedTableEntryActionTypeNOACT,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_ValidateTable_Test_UnusedTableEntryActionTypeNOACT");
    UtTest_Add(HS_EventMon_ValidateTable_Test_BufferNotNull,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_ValidateTable_Test_BufferNotNull");
    UtTest_Add(HS_EventMon_ValidateTable_Test_ActionTypeNotValid,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_ValidateTable_Test_ActionTypeNotValid");
    UtTest_Add(HS_EventMon_ValidateTable_Test_EntryGood,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_ValidateTable_Test_EntryGood");
    UtTest_Add(HS_EventMon_ValidateTable_Test_Null,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_ValidateTable_Test_Null");

    UtTest_Add(HS_EventMon_AcquirePointers_Test_NominalNotLoaded,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_AcquirePointers_Test_NominalNotLoaded");
    UtTest_Add(HS_EventMon_AcquirePointers_Test_NominalUpdate,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_AcquirePointers_Test_NominalUpdate");
    UtTest_Add(HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonEnabled");
    UtTest_Add(HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonNotLoaded,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonNotLoaded");
    UtTest_Add(HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonNotLoadedEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonNotLoadedEnabled");
    UtTest_Add(HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonDisabled");
    UtTest_Add(HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonEnabledNoSubscribeError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_AcquirePointers_Test_ErrorsWithEventMonEnabledNoSubscribeError");

    UtTest_Add(HS_EventMon_CheckEntries_Test_InvalidName,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_EventMon_CheckEntries_Test_InvalidName");
}
