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

#include "hs_msg_action.h"
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

static void UT_HS_MsgAct_Callback(uint32 Idx, const void *Arg)
{
    UT_DEFAULT_IMPL(UT_HS_MsgAct_Callback);
}

void HS_MsgAct_TriggerAction_Test_Nominal_Enabled(void)
{
    /*
     * Test Case For:
     * void HS_MsgAct_TriggerAction(uint16 ActionType, HS_MsgAct_Callback_t SendEventCb, const void *CbArg)
     */
    HS_MATEntry_t     MATable[HS_MAX_MSG_ACT_TYPES];
    HS_MsgActState_t *MAStatePtr;

    memset(MATable, 0, sizeof(MATable));
    HS_AppData.MATablePtr = MATable;

    HS_AppData.MsgActsLoaded             = HS_State_ENABLED;
    HS_AppData.MATablePtr[0].EnableState = HS_MATState_ENABLED;
    HS_AppData.MATablePtr[0].Cooldown    = 4;
    HS_AppData.MATablePtr[1].EnableState = HS_MATState_DISABLED;
    HS_AppData.MATablePtr[1].Cooldown    = 5;

    /* enabled entry */
    HS_MsgAct_TriggerAction(1, UT_HS_MsgAct_Callback, NULL);

    UtAssert_STUB_COUNT(UT_HS_MsgAct_Callback, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 1);

    /* cooldown is reset because the entry is enabled */
    MAStatePtr = HS_GetMAStateByIndex(0);
    UtAssert_UINT16_EQ(MAStatePtr->Cooldown, 4);

    /* disabled entry */
    HS_MsgAct_TriggerAction(2, UT_HS_MsgAct_Callback, NULL);

    UtAssert_STUB_COUNT(UT_HS_MsgAct_Callback, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 1);

    /* cooldown is NOT reset because the entry is disabled */
    MAStatePtr = HS_GetMAStateByIndex(1);
    UtAssert_UINT16_EQ(MAStatePtr->Cooldown, 0);
}

void HS_MsgAct_TriggerAction_Test_Nominal_NoEvent(void)
{
    /*
     * Test Case For:
     * void HS_MsgAct_TriggerAction(uint16 ActionType, HS_MsgAct_Callback_t SendEventCb, const void *CbArg)
     */
    HS_MATEntry_t     MATable[HS_MAX_MSG_ACT_TYPES];
    HS_MsgActState_t *MAStatePtr;

    memset(MATable, 0, sizeof(MATable));
    HS_AppData.MATablePtr = MATable;

    HS_AppData.MsgActsLoaded             = HS_State_ENABLED;
    HS_AppData.MATablePtr[0].EnableState = HS_MATState_ENABLED;
    HS_AppData.MATablePtr[0].Cooldown    = 6;
    HS_AppData.MATablePtr[1].EnableState = HS_MATState_NOEVENT;
    HS_AppData.MATablePtr[1].Cooldown    = 7;

    /* sanity check - normal enable - confirm callback is invoked */
    HS_MsgAct_TriggerAction(1, UT_HS_MsgAct_Callback, NULL);

    UtAssert_STUB_COUNT(UT_HS_MsgAct_Callback, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 1);

    MAStatePtr = HS_GetMAStateByIndex(0);
    UtAssert_UINT16_EQ(MAStatePtr->Cooldown, 6);

    /* This should be mostly the same except does not invoke callback */
    HS_MsgAct_TriggerAction(2, UT_HS_MsgAct_Callback, NULL);

    UtAssert_STUB_COUNT(UT_HS_MsgAct_Callback, 1);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 2);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 2);

    MAStatePtr = HS_GetMAStateByIndex(1);
    UtAssert_UINT16_EQ(MAStatePtr->Cooldown, 7);
}

void HS_MsgAct_TriggerAction_Test_Invalid(void)
{
    /*
     * Test Case For:
     * void HS_MsgAct_TriggerAction(uint16 ActionType, HS_MsgAct_Callback_t SendEventCb, const void *CbArg)
     */
    HS_MATEntry_t MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));
    HS_AppData.MATablePtr = MATable;

    HS_AppData.MsgActsLoaded             = HS_State_ENABLED;
    HS_AppData.MATablePtr[0].EnableState = HS_MATState_ENABLED;
    HS_AppData.MATablePtr[1].EnableState = HS_MATState_NOEVENT;

    /* Test invalid entry */
    HS_MsgAct_TriggerAction(0, UT_HS_MsgAct_Callback, NULL);

    UtAssert_STUB_COUNT(UT_HS_MsgAct_Callback, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 0);

    /* Test NULL callback */
    HS_MsgAct_TriggerAction(1, NULL, NULL);

    UtAssert_STUB_COUNT(UT_HS_MsgAct_Callback, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 1);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 1);
}

void HS_MsgAct_TriggerAction_Test_Nominal_Cooldown(void)
{
    /*
     * Test Case For:
     * void HS_MsgAct_TriggerAction(uint16 ActionType, HS_MsgAct_Callback_t SendEventCb, const void *CbArg)
     */
    HS_MATEntry_t     MATable[HS_MAX_MSG_ACT_TYPES];
    HS_MsgActState_t *MAStatePtr;

    memset(MATable, 0, sizeof(MATable));
    HS_AppData.MATablePtr = MATable;

    HS_AppData.MsgActsLoaded             = HS_State_ENABLED;
    HS_AppData.MATablePtr[0].EnableState = HS_MATState_ENABLED;
    HS_AppData.MATablePtr[1].EnableState = HS_MATState_NOEVENT;

    MAStatePtr           = HS_GetMAStateByIndex(0);
    MAStatePtr->Cooldown = 10;
    MAStatePtr           = HS_GetMAStateByIndex(1);
    MAStatePtr->Cooldown = 11;

    HS_MsgAct_TriggerAction(1, UT_HS_MsgAct_Callback, NULL);

    UtAssert_STUB_COUNT(UT_HS_MsgAct_Callback, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 0);

    MAStatePtr = HS_GetMAStateByIndex(0);
    UtAssert_UINT16_EQ(MAStatePtr->Cooldown, 10);

    HS_MsgAct_TriggerAction(2, UT_HS_MsgAct_Callback, NULL);

    UtAssert_STUB_COUNT(UT_HS_MsgAct_Callback, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 0);

    MAStatePtr = HS_GetMAStateByIndex(1);
    UtAssert_UINT16_EQ(MAStatePtr->Cooldown, 11);
}

void HS_MsgAct_TriggerAction_Test_Nominal_Disabled(void)
{
    /*
     * Test Case For:
     * void HS_MsgAct_TriggerAction(uint16 ActionType, HS_MsgAct_Callback_t SendEventCb, const void *CbArg)
     */

    HS_AppData.MsgActsLoaded = HS_State_DISABLED;

    HS_MsgAct_TriggerAction(1, UT_HS_MsgAct_Callback, NULL);

    UtAssert_STUB_COUNT(UT_HS_MsgAct_Callback, 0);
    UtAssert_STUB_COUNT(CFE_SB_TransmitMsg, 0);
    UtAssert_UINT32_EQ(HS_AppData.MsgActExec, 0);
}

void HS_MsgAct_Cooldown_Test_Nominal(void)
{
    /*
     * Test Case For:
     * void HS_MsgAct_Cooldown(void)
     */
    HS_MsgActState_t *MAStatePtr;
    int               i;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        MAStatePtr           = HS_GetMAStateByIndex(i);
        MAStatePtr->Cooldown = i;
    }

    /* Test invalid entry */
    UtAssert_VOIDCALL(HS_MsgAct_Cooldown());

    /* index 0 should remain 0 */
    MAStatePtr = HS_GetMAStateByIndex(0);
    UtAssert_UINT16_EQ(MAStatePtr->Cooldown, 0);

    /* the rest should have decremented */
    for (i = 1; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        MAStatePtr = HS_GetMAStateByIndex(i);
        UtAssert_UINT16_EQ(MAStatePtr->Cooldown, i - 1);
    }
}

void HS_MsgAct_ValidateTable_Test_UnusedTableEntry(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify results: good = %%d, bad = %%d, unused = %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    MsgSize   = sizeof(HS_NoopCmd_t);

    HS_AppData.MATablePtr = MATable;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MATablePtr[i].EnableState = HS_MATState_DISABLED;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* take secondary branch for "Enable State is Invalid" */
    HS_AppData.MATablePtr[1].EnableState = HS_MATState_NOEVENT;

    /* Execute the function being tested */
    Result = HS_MsgAct_ValidateTable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_MsgAct_ValidateTable_Test_InvalidEnableState(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify err: Entry = %%d, Err = %%d, Length = %%d, ID = 0x%%08lX");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
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
    Result = HS_MsgAct_ValidateTable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVal_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_MATVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_MATVal_ERR_ENA, "Result == HS_MATVal_ERR_ENA");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_MsgAct_ValidateTable_Test_MessageIDTooHigh(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify err: Entry = %%d, Err = %%d, Length = %%d, ID = 0x%%08lX");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify results: good = %%d, bad = %%d, unused = %%d");

    TestMsgId = CFE_SB_INVALID_MSG_ID;
    MsgSize   = sizeof(HS_NoopCmd_t);

    HS_AppData.MATablePtr = MATable;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MATablePtr[i].EnableState = HS_MATState_ENABLED;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* Execute the function being tested */
    Result = HS_MsgAct_ValidateTable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVal_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_MATVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_MATVal_ERR_ID, "Result == HS_MATVal_ERR_ID");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_MsgAct_ValidateTable_Test_LengthTooHigh(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify err: Entry = %%d, Err = %%d, Length = %%d, ID = 0x%%08lX");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify results: good = %%d, bad = %%d, unused = %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    MsgSize   = CFE_MISSION_SB_MAX_SB_MSG_SIZE + 1;

    HS_AppData.MATablePtr = MATable;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MATablePtr[i].EnableState = HS_MATState_ENABLED;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* Execute the function being tested */
    Result = HS_MsgAct_ValidateTable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVal_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_MATVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_MATVal_ERR_LEN, "Result == HS_MATVal_ERR_LEN");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_MsgAct_ValidateTable_Test_EntryGood(void)
{
    int32          Result;
    uint32         i;
    CFE_SB_MsgId_t TestMsgId;
    size_t         MsgSize;
    int32          strCmpResult;
    char           ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_MATEntry_t  MATable[HS_MAX_MSG_ACT_TYPES];

    memset(MATable, 0, sizeof(MATable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "MsgActs verify results: good = %%d, bad = %%d, unused = %%d");

    TestMsgId = CFE_SB_ValueToMsgId(HS_CMD_MID);
    MsgSize   = sizeof(HS_NoopCmd_t);

    HS_AppData.MATablePtr = MATable;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        HS_AppData.MATablePtr[i].EnableState = HS_MATState_ENABLED;

        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), &TestMsgId, sizeof(TestMsgId), false);
        UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), &MsgSize, sizeof(MsgSize), false);
    }

    /* Execute the function being tested */
    Result = HS_MsgAct_ValidateTable(HS_AppData.MATablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MATVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_MsgAct_ValidateTable_Test_Null(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error in MA Table Validation. Table is null.");

    /* Execute the function being tested */
    Result = HS_MsgAct_ValidateTable(NULL);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MA_TBL_NULL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == HS_TBL_VAL_ERR, "Result == HS_TBL_VAL_ERR");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_MsgAct_AcquirePointers_Test_NominalNotLoaded(void)
{
    HS_MATEntry_t     MATable[HS_MAX_MSG_ACT_TYPES];
    HS_MATEntry_t    *MATablePtr = MATable;
    HS_MsgActState_t *MAStatePtr;

    MAStatePtr           = HS_GetMAStateByIndex(0);
    MAStatePtr->Cooldown = 1;

    memset(MATable, 0, sizeof(MATable));

    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_SUCCESS);
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &MATablePtr, sizeof(MATablePtr), false);

    /* Execute the function being tested */
    HS_MsgAct_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.MsgActsLoaded, HS_State_ENABLED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_ZERO(MAStatePtr->Cooldown);
}

void HS_MsgAct_AcquirePointers_Test_NominalUpdate(void)
{
    HS_MATEntry_t     MATable[HS_MAX_MSG_ACT_TYPES];
    HS_MATEntry_t    *MATablePtr = MATable;
    HS_MsgActState_t *MAStatePtr;

    MAStatePtr           = HS_GetMAStateByIndex(0);
    MAStatePtr->Cooldown = 1;

    memset(MATable, 0, sizeof(MATable));

    HS_AppData.MsgActsLoaded = HS_State_ENABLED;
    HS_AppData.MATablePtr    = MATable;

    /* Satisfies all instances of (Status == CFE_TBL_INFO_UPDATED), skips all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &MATablePtr, sizeof(MATablePtr), false);

    /* Execute the function being tested */
    HS_MsgAct_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.MsgActsLoaded, HS_State_ENABLED);
    /* jphfix more asserts */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_ZERO(MAStatePtr->Cooldown);
}

void HS_MsgAct_AcquirePointers_Test_ErrorsWithDisabled(void)
{
    HS_MsgActState_t *MAStatePtr;

    MAStatePtr           = HS_GetMAStateByIndex(0);
    MAStatePtr->Cooldown = 1;

    HS_AppData.MsgActsLoaded = HS_State_DISABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_MsgAct_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.MsgActsLoaded, HS_State_DISABLED);

    /* Event only generated if it was enabled before */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_UINT16_EQ(MAStatePtr->Cooldown, 1);
}

void HS_MsgAct_AcquirePointers_Test_ErrorsWithEnabled(void)
{
    HS_MATEntry_t     MATable[HS_MAX_MSG_ACT_TYPES];
    HS_MATEntry_t    *MATablePtr = MATable;
    HS_MsgActState_t *MAStatePtr;

    memset(MATable, 0, sizeof(MATable));

    MAStatePtr           = HS_GetMAStateByIndex(0);
    MAStatePtr->Cooldown = 1;

    HS_AppData.MATablePtr    = MATablePtr;
    HS_AppData.MsgActsLoaded = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_MsgAct_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.MsgActsLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MSGACTS_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_ZERO(MAStatePtr->Cooldown);
}

#ifdef jphfix
void HS_MsgAct_AcquirePointers_Test_ErrorsWith2(void)
{
    HS_AppData.MsgActsLoaded = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_MsgAct_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.MsgActsLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MSGACTS_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_MsgAct_AcquirePointers_Test_ErrorsWith(void)
{
    HS_AppData.MsgActsLoaded = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_MsgAct_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.MsgActsLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MSGACTS_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_MsgAct_AcquirePointers_Test_ErrorsWithNoSubscribeError(void)
{
    HS_AppData.MsgActsLoaded = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_MsgAct_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.MsgActsLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MSGACTS_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_MsgAct_AcquirePointers_Test_ErrorsWithCurrentAppMonLoadedDisabledAndCurrentAppMonStateDisabled(void)
{
    HS_AppData.MsgActsLoaded = HS_State_DISABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_MsgAct_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.MsgActsLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}
#endif
void HS_MsgAct_StatusRefresh_Test(void)
{
    HS_MsgActState_t *MAStatePtr;
    uint32            i;

    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        MAStatePtr           = HS_GetMAStateByIndex(i);
        MAStatePtr->Cooldown = 1 + i;
    }

    /* Execute the function being tested */
    HS_MsgAct_StatusRefresh();

    /* Verify results */
    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        MAStatePtr = HS_GetMAStateByIndex(i);
        UtAssert_UINT16_EQ(MAStatePtr->Cooldown, 0);
    }

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_MsgAct_TriggerAction_Test_Nominal_Enabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_TriggerAction_Test_Nominal_Enabled");
    UtTest_Add(HS_MsgAct_TriggerAction_Test_Nominal_Disabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_TriggerAction_Test_Nominal_Disabled");
    UtTest_Add(HS_MsgAct_TriggerAction_Test_Nominal_Cooldown,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_TriggerAction_Test_Nominal_Cooldown");
    UtTest_Add(HS_MsgAct_TriggerAction_Test_Nominal_NoEvent,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_TriggerAction_Test_Nominal_NoEvent");
    UtTest_Add(HS_MsgAct_TriggerAction_Test_Invalid,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_TriggerAction_Test_Invalid");

    UtTest_Add(HS_MsgAct_Cooldown_Test_Nominal, HS_Test_Setup, HS_Test_TearDown, "HS_MsgAct_Cooldown_Test_Nominal");

    UtTest_Add(HS_MsgAct_ValidateTable_Test_UnusedTableEntry,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_ValidateTable_Test_UnusedTableEntry");
    UtTest_Add(HS_MsgAct_ValidateTable_Test_InvalidEnableState,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_ValidateTable_Test_InvalidEnableState");
    UtTest_Add(HS_MsgAct_ValidateTable_Test_MessageIDTooHigh,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_ValidateTable_Test_MessageIDTooHigh");
    UtTest_Add(HS_MsgAct_ValidateTable_Test_LengthTooHigh,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_ValidateTable_Test_LengthTooHigh");
    UtTest_Add(HS_MsgAct_ValidateTable_Test_EntryGood,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_ValidateTable_Test_EntryGood");
    UtTest_Add(HS_MsgAct_ValidateTable_Test_Null, HS_Test_Setup, HS_Test_TearDown, "HS_MsgAct_ValidateTable_Test_Null");

    UtTest_Add(HS_MsgAct_AcquirePointers_Test_NominalNotLoaded,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_AcquirePointers_Test_NominalNotLoaded");
    UtTest_Add(HS_MsgAct_AcquirePointers_Test_NominalUpdate,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_AcquirePointers_Test_NominalUpdate");
    UtTest_Add(HS_MsgAct_AcquirePointers_Test_ErrorsWithEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_AcquirePointers_Test_ErrorsWithEnabled");
    UtTest_Add(HS_MsgAct_AcquirePointers_Test_ErrorsWithDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_MsgAct_AcquirePointers_Test_ErrorsWithDisabled");

    UtTest_Add(HS_MsgAct_StatusRefresh_Test, HS_Test_Setup, HS_Test_TearDown, "HS_MsgAct_StatusRefresh_Test");
}
