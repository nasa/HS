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
#include "hs_dispatch.h"
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
 * Helper functions
 */
static void HS_Dispatch_Test_SetupMsg(CFE_SB_MsgId_t MsgId, CFE_MSG_FcnCode_t FcnCode, size_t MsgSize)
{
    /* Note some paths get the MsgId/FcnCode multiple times, so register accordingly, just in case */
    CFE_SB_MsgId_t    RegMsgId[2]   = {MsgId, MsgId};
    CFE_MSG_FcnCode_t RegFcnCode[2] = {FcnCode, FcnCode};
    size_t            RegMsgSize[2] = {MsgSize, MsgSize};

    UT_ResetState(UT_KEY(CFE_MSG_GetMsgId));
    UT_ResetState(UT_KEY(CFE_MSG_GetFcnCode));
    UT_ResetState(UT_KEY(CFE_MSG_GetSize));

    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetMsgId), RegMsgId, sizeof(RegMsgId), true);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), RegFcnCode, sizeof(RegFcnCode), true);
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetSize), RegMsgSize, sizeof(RegMsgSize), true);
}

/*
 * Function Definitions
 */

void HS_VerifyMsgLength_Test(void)
{
    /* Nominal */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), 0, 1);

    /* Execute the function being tested */
    UtAssert_BOOL_TRUE(HS_VerifyMsgLength(&UT_CmdBuf.Msg, 1));
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* Error on SEND_HK */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_SEND_HK_MID), 0, 1);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(HS_VerifyMsgLength(&UT_CmdBuf.Msg, 2));

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_HKREQ_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_UINT8_EQ(HS_AppData.CmdErrCount, 0);

    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), 0, 1);

    /* Execute the function being tested */
    UtAssert_BOOL_FALSE(HS_VerifyMsgLength(&UT_CmdBuf.Msg, 2));

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_CMD_LEN_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_ERROR);
    UtAssert_UINT8_EQ(HS_AppData.CmdErrCount, 1);
}

void HS_AppPipe_Test_SendHK(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_SEND_HK_MID), 0, sizeof(UT_CmdBuf.SendHkCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should have invoked the handler */
    UtAssert_STUB_COUNT(HS_SendHkCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_SEND_HK_MID), 0, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(HS_SendHkCmd, 1);
}

void HS_AppPipe_Test_Noop(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_NOOP_CC, sizeof(UT_CmdBuf.NoopCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should have invoked the handler */
    UtAssert_STUB_COUNT(HS_NoopCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_NOOP_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(HS_NoopCmd, 1);
}

void HS_AppPipe_Test_Reset(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_RESET_CC, sizeof(UT_CmdBuf.ResetCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_ResetCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_RESET_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_ResetCmd, 1);
}

void HS_AppPipe_Test_EnableAppMon(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_ENABLE_APP_MON_CC, sizeof(UT_CmdBuf.EnableAppMonCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_EnableAppMonCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_ENABLE_APP_MON_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_EnableAppMonCmd, 1);
}

void HS_AppPipe_Test_DisableAppMon(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_DISABLE_APP_MON_CC,
                              sizeof(UT_CmdBuf.DisableAppMonCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_DisableAppMonCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_DISABLE_APP_MON_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_DisableAppMonCmd, 1);
}

void HS_AppPipe_Test_EnableEventMon(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_ENABLE_EVENT_MON_CC,
                              sizeof(UT_CmdBuf.EnableEventMonCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_EnableEventMonCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_ENABLE_EVENT_MON_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_EnableEventMonCmd, 1);
}

void HS_AppPipe_Test_DisableEventMon(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_DISABLE_EVENT_MON_CC,
                              sizeof(UT_CmdBuf.DisableEventMonCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_DisableEventMonCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_DISABLE_EVENT_MON_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_DisableEventMonCmd, 1);
}

void HS_AppPipe_Test_EnableAliveness(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_ENABLE_ALIVENESS_CC,
                              sizeof(UT_CmdBuf.EnableAlivenessCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_EnableAlivenessCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_ENABLE_ALIVENESS_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_EnableAlivenessCmd, 1);
}

void HS_AppPipe_Test_DisableAliveness(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_DISABLE_ALIVENESS_CC,
                              sizeof(UT_CmdBuf.DisableAlivenessCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_DisableAlivenessCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_DISABLE_ALIVENESS_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_DisableAlivenessCmd, 1);
}

void HS_AppPipe_Test_ResetResetsPerformed(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_RESET_RESETS_PERFORMED_CC,
                              sizeof(UT_CmdBuf.ResetResetsPerformedCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_ResetResetsPerformedCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_RESET_RESETS_PERFORMED_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_ResetResetsPerformedCmd, 1);
}

void HS_AppPipe_Test_SetMaxResets(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_SET_MAX_RESETS_CC, sizeof(UT_CmdBuf.SetMaxResetsCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_SetMaxResetsCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_SET_MAX_RESETS_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_SetMaxResetsCmd, 1);
}

void HS_AppPipe_Test_EnableCPUHog(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_ENABLE_CPU_HOG_CC, sizeof(UT_CmdBuf.EnableCpuHogCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_EnableCpuHogCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_ENABLE_CPU_HOG_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_EnableCpuHogCmd, 1);
}

void HS_AppPipe_Test_DisableCPUHog(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_DISABLE_CPU_HOG_CC,
                              sizeof(UT_CmdBuf.DisableCpuHogCmd));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);
    UtAssert_STUB_COUNT(HS_DisableCpuHogCmd, 1);

    /* Now with an invalid size */
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_DISABLE_CPU_HOG_CC, 1);

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Should NOT have invoked the handler */
    UtAssert_STUB_COUNT(HS_DisableCpuHogCmd, 1);
}

void HS_AppPipe_Test_InvalidCC(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_ValueToMsgId(HS_CMD_MID), 99, sizeof(UT_CmdBuf));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT8_EQ(HS_AppData.CmdErrCount, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CC_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_AppPipe_Test_InvalidMID(void)
{
    HS_Dispatch_Test_SetupMsg(CFE_SB_INVALID_MSG_ID, HS_NOOP_CC, sizeof(UT_CmdBuf));

    /* Execute the function being tested */
    HS_AppPipe(&UT_CmdBuf.Buf);

    /* Verify results */
    UtAssert_UINT8_EQ(HS_AppData.CmdErrCount, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_MID_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_VerifyMsgLength_Test, HS_Test_Setup, HS_Test_TearDown, "HS_VerifyMsgLength_Test");

    UtTest_Add(HS_AppPipe_Test_SendHK, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_SendHK");
    UtTest_Add(HS_AppPipe_Test_Noop, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_Noop");
    UtTest_Add(HS_AppPipe_Test_Reset, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_Reset");
    UtTest_Add(HS_AppPipe_Test_EnableAppMon, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_EnableAppMon");
    UtTest_Add(HS_AppPipe_Test_DisableAppMon, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_DisableAppMon");
    UtTest_Add(HS_AppPipe_Test_EnableEventMon, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_EnableEventMon");
    UtTest_Add(HS_AppPipe_Test_DisableEventMon, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_DisableEventMon");
    UtTest_Add(HS_AppPipe_Test_EnableAliveness, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_EnableAliveness");
    UtTest_Add(HS_AppPipe_Test_DisableAliveness, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_DisableAliveness");
    UtTest_Add(HS_AppPipe_Test_ResetResetsPerformed, HS_Test_Setup, HS_Test_TearDown,
               "HS_AppPipe_Test_ResetResetsPerformed");
    UtTest_Add(HS_AppPipe_Test_SetMaxResets, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_SetMaxResets");
    UtTest_Add(HS_AppPipe_Test_EnableCPUHog, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_EnableCPUHog");
    UtTest_Add(HS_AppPipe_Test_DisableCPUHog, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_DisableCPUHog");
    UtTest_Add(HS_AppPipe_Test_InvalidCC, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_InvalidCC");
    UtTest_Add(HS_AppPipe_Test_InvalidMID, HS_Test_Setup, HS_Test_TearDown, "HS_AppPipe_Test_InvalidMID");
}
