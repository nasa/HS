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
#include "hs_custom_internal.h"
#include "hs_custom_dispatch.h"
#include "hs_dispatch.h"
#include "hs_msgids.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>

/* Command buffer typedef for any handler */
typedef union
{
    CFE_SB_Buffer_t       Buf;
    HS_ReportDiagCmd_t    ReportDiagCmd;
    HS_SetUtilParamsCmd_t SetUtilParamsCmd;
    HS_SetUtilDiagCmd_t   SetUtilDiagCmd;
} UT_CustomCmdBuf_t;

UT_CustomCmdBuf_t UT_CustomCmdBuf;

void HS_CustomCommands_Test_ReportDiagCmd(void)
{
    CFE_MSG_FcnCode_t FcnCode;

    FcnCode = HS_REPORT_DIAG_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_CustomCommands(&UT_CustomCmdBuf.Buf), CFE_SUCCESS);
    UtAssert_STUB_COUNT(HS_ReportDiagCmd, 1);

    /* Now with an invalid size */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), false);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_CustomCommands(&UT_CustomCmdBuf.Buf), CFE_SUCCESS);

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(HS_ReportDiagCmd, 1);
}

void HS_CustomCommands_Test_SetUtilParamsCmd(void)
{
    CFE_MSG_FcnCode_t FcnCode;

    FcnCode = HS_SET_UTIL_PARAMS_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_CustomCommands(&UT_CustomCmdBuf.Buf), CFE_SUCCESS);
    UtAssert_STUB_COUNT(HS_SetUtilParamsCmd, 1);

    /* Now with an invalid size */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), false);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_CustomCommands(&UT_CustomCmdBuf.Buf), CFE_SUCCESS);

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(HS_SetUtilParamsCmd, 1);
}

void HS_CustomCommands_Test_SetUtilDiagCmd(void)
{
    CFE_MSG_FcnCode_t FcnCode;

    FcnCode = HS_SET_UTIL_DIAG_CC;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), true);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_CustomCommands(&UT_CustomCmdBuf.Buf), CFE_SUCCESS);
    UtAssert_STUB_COUNT(HS_SetUtilDiagCmd, 1);

    /* Now with an invalid size */
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);
    UT_SetDefaultReturnValue(UT_KEY(HS_VerifyMsgLength), false);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_CustomCommands(&UT_CustomCmdBuf.Buf), CFE_SUCCESS);

    /* Should NOT have invoked the handler this time */
    UtAssert_STUB_COUNT(HS_SetUtilDiagCmd, 1);
}

void HS_CustomCommands_Test_InvalidCommandCode(void)
{
    CFE_MSG_FcnCode_t FcnCode;

    FcnCode = 99;
    UT_SetDataBuffer(UT_KEY(CFE_MSG_GetFcnCode), &FcnCode, sizeof(FcnCode), false);

    /* Execute the function being tested */
    UtAssert_INT32_EQ(HS_CustomCommands(&UT_CustomCmdBuf.Buf), CFE_STATUS_BAD_COMMAND_CODE);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_CustomCommands_Test_ReportDiagCmd, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomCommands_Test_ReportDiagCmd");
    UtTest_Add(HS_CustomCommands_Test_SetUtilParamsCmd, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomCommands_Test_SetUtilParamsCmd");
    UtTest_Add(HS_CustomCommands_Test_SetUtilDiagCmd, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomCommands_Test_SetUtilDiagCmd");
    UtTest_Add(HS_CustomCommands_Test_InvalidCommandCode, HS_Test_Setup, HS_Test_TearDown,
               "HS_CustomCommands_Test_InvalidCommandCode");
}
