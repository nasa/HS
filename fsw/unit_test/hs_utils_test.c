/*************************************************************************
** File: hs_utils_test.c 
**
** NASA Docket No. GSC-18,476-1, and identified as "Core Flight System 
** (cFS) Health and Safety (HS) Application version 2.3.2” 
**
** Copyright © 2020 United States Government as represented by the 
** Administrator of the National Aeronautics and Space Administration.  
** All Rights Reserved. 
** 
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
** http://www.apache.org/licenses/LICENSE-2.0 
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License. 
**
** Purpose: 
**   This file contains unit test cases for the functions contained in 
**   the file hs_utils.c
**
*************************************************************************/

/*
 * Includes
 */

#include "hs_monitors_test.h"
#include "hs_app.h"
#include "hs_msg.h"
#include "hs_msgdefs.h"
#include "hs_msgids.h"
#include "hs_events.h"
#include "hs_version.h"
#include "hs_test_utils.h"
#include "ut_osapi_stubs.h"
#include "ut_cfe_sb_stubs.h"
#include "ut_cfe_es_stubs.h"
#include "ut_cfe_es_hooks.h"
#include "ut_cfe_evs_stubs.h"
#include "ut_cfe_evs_hooks.h"
#include "ut_cfe_time_stubs.h"
#include "ut_cfe_psp_memutils_stubs.h"
#include "ut_cfe_psp_watchdog_stubs.h"
#include "ut_cfe_psp_timer_stubs.h"
#include "ut_cfe_tbl_stubs.h"
#include "ut_cfe_fs_stubs.h"
#include "ut_cfe_time_stubs.h"
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * Function Definitions
 */

void HS_VerifyMsgLength_Test_Nominal(void)
{
    HS_NoArgsCmd_t    CmdPacket;
    boolean           Result;

    CFE_SB_InitMsg (&CmdPacket, HS_CMD_MID, sizeof(HS_NoArgsCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdPacket, HS_ENABLE_CPUHOG_CC);

    /* Execute the function being tested */
    Result = HS_VerifyMsgLength((CFE_SB_MsgPtr_t)&CmdPacket, sizeof(HS_NoArgsCmd_t));

    /* Verify results */
    UtAssert_True (Result == TRUE, "Result == TRUE");

    UtAssert_True (Ut_CFE_EVS_GetEventQueueDepth() == 0, "Ut_CFE_EVS_GetEventQueueDepth() == 0");

} /* end HS_VerifyMsgLength_Test_Nominal */

void HS_VerifyMsgLength_Test_LengthErrorHK(void)
{
    HS_NoArgsCmd_t    CmdPacket;
    boolean           Result;

    CFE_SB_InitMsg (&CmdPacket, HS_SEND_HK_MID, 1, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdPacket, 0);

    /* Execute the function being tested */
    Result = HS_VerifyMsgLength((CFE_SB_MsgPtr_t)&CmdPacket, sizeof(HS_NoArgsCmd_t));

    /* Verify results */
    UtAssert_True (Result == FALSE, "Result == FALSE");

    UtAssert_True
        (Ut_CFE_EVS_EventSent(HS_HKREQ_LEN_ERR_EID, CFE_EVS_ERROR, "Invalid HK request msg length: ID = 0x18AF, CC = 0, Len = 1, Expected = 8"),
        "Invalid HK request msg length: ID = 0x18AF, CC = 0, Len = 1, Expected = 8");

    UtAssert_True (Ut_CFE_EVS_GetEventQueueDepth() == 1, "Ut_CFE_EVS_GetEventQueueDepth() == 1");

} /* end HS_VerifyMsgLength_Test_LengthErrorHK */

void HS_VerifyMsgLength_Test_LengthErrorNonHK(void)
{
    HS_NoArgsCmd_t    CmdPacket;
    boolean           Result;

    CFE_SB_InitMsg (&CmdPacket, HS_CMD_MID, 1, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdPacket, 0);

    /* Execute the function being tested */
    Result = HS_VerifyMsgLength((CFE_SB_MsgPtr_t)&CmdPacket, sizeof(HS_NoArgsCmd_t));

    /* Verify results */
    UtAssert_True (Result == FALSE, "Result == FALSE");

    UtAssert_True (HS_AppData.CmdErrCount == 1, "HS_AppData.CmdErrCount == 1");

    UtAssert_True
        (Ut_CFE_EVS_EventSent(HS_LEN_ERR_EID, CFE_EVS_ERROR, "Invalid msg length: ID = 0x18AE, CC = 0, Len = 1, Expected = 8"),
        "Invalid msg length: ID = 0x18AE, CC = 0, Len = 1, Expected = 8");

    UtAssert_True (Ut_CFE_EVS_GetEventQueueDepth() == 1, "Ut_CFE_EVS_GetEventQueueDepth() == 1");

} /* end HS_VerifyMsgLength_Test_LengthErrorNonHK */


void HS_AMTActionIsValid_Valid(void)
{
    uint16 Action = (HS_AMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES);

    boolean Result = HS_AMTActionIsValid(Action);

    UtAssert_True(Result == TRUE, "Result == TRUE");
}

void HS_AMTActionIsValid_Invalid(void)
{
    uint16 Action = (HS_AMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1);

    boolean Result = HS_AMTActionIsValid(Action);

    UtAssert_True(Result == FALSE, "Result == FALSE");

}

void HS_EMTActionIsValid_Valid(void)
{
    uint16 Action = (HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES);

    boolean Result = HS_EMTActionIsValid(Action);

    UtAssert_True(Result == TRUE, "Result == TRUE");

}

void HS_EMTActionIsValid_Invalid(void)
{
    uint16 Action = (HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1);

    boolean Result = HS_EMTActionIsValid(Action);

    UtAssert_True(Result == FALSE, "Result == FALSE");

}

void HS_Utils_Test_AddTestCases(void)
{
    UtTest_Add(HS_VerifyMsgLength_Test_Nominal, HS_Test_Setup, HS_Test_TearDown, "HS_VerifyMsgLength_Test_Nominal");
    UtTest_Add(HS_VerifyMsgLength_Test_LengthErrorHK, HS_Test_Setup, HS_Test_TearDown, "HS_VerifyMsgLength_Test_LengthErrorHK");
    UtTest_Add(HS_VerifyMsgLength_Test_LengthErrorNonHK, HS_Test_Setup, HS_Test_TearDown, "HS_VerifyMsgLength_Test_LengthErrorNonHK");

    UtTest_Add(HS_AMTActionIsValid_Valid, HS_Test_Setup, HS_Test_TearDown, "HS_AMTActionIsValid_Valid");

    UtTest_Add(HS_AMTActionIsValid_Invalid, HS_Test_Setup, HS_Test_TearDown, "HS_AMTActionIsValid_Invalid");

    UtTest_Add(HS_EMTActionIsValid_Valid, HS_Test_Setup, HS_Test_TearDown, "HS_EMTActionIsValid_Valid");

    UtTest_Add(HS_EMTActionIsValid_Invalid, HS_Test_Setup, HS_Test_TearDown, "HS_EMTActionIsValid_Invalid");

}
