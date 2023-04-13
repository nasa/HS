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

#include "hs_sysmon.h"
#include "hs_app.h"
#include "hs_platform_cfg.h"
#include "hs_test_utils.h"

#include "cfe_psp.h"
#include "iodriver_base.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <stdlib.h>

void Test_HS_SysMonInit(void)
{
    /* Test function for:
     * int32 HS_SysMonInit(void)
     */

    /* nominal */
    UtAssert_INT32_EQ(HS_SysMonInit(), CFE_SUCCESS);

    /* fail to find device - this is a non-fatal error */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_FindByName), 1, CFE_PSP_ERROR);
    UtAssert_INT32_EQ(HS_SysMonInit(), CFE_SUCCESS);

    /* fail to start device - this is an error */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_Command), 1, CFE_PSP_ERROR);
    UtAssert_INT32_EQ(HS_SysMonInit(), CFE_STATUS_EXTERNAL_RESOURCE_FAIL);

    /* fail to lookup subsystem - non-fatal */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_Command), 2, CFE_PSP_ERROR);
    UtAssert_INT32_EQ(HS_SysMonInit(), CFE_SUCCESS);

    /* fail to lookup subchannel - non-fatal */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_Command), 3, CFE_PSP_ERROR);
    UtAssert_INT32_EQ(HS_SysMonInit(), CFE_SUCCESS);
}

void Test_HS_SysMonCleanup(void)
{
    /* Test function for:
     * void HS_SysMonCleanup(void)
     */

    /* nominal - no psp module  */
    HS_AppData.SysMonPspModuleId = 0;
    UtAssert_VOIDCALL(HS_SysMonCleanup());

    /* nominal - with psp module  */
    HS_AppData.SysMonPspModuleId = 0x1111;
    UtAssert_VOIDCALL(HS_SysMonCleanup());

    /* Unable to stop */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_Command), 1, CFE_PSP_ERROR);
    UtAssert_VOIDCALL(HS_SysMonCleanup());
}

void Test_HS_SysMonGetCpuUtilization(void)
{
    /* Test function for:
     * int32 HS_SysMonGetCpuUtilization(void)
     */

    /* nominal - no psp module, reports -1  */
    HS_AppData.SysMonPspModuleId = 0;
    UtAssert_INT32_EQ(HS_SysMonGetCpuUtilization(), -1);

    /* nominal - with psp module  */
    HS_AppData.SysMonPspModuleId = 0x1111;
    UtAssert_INT32_EQ(HS_SysMonGetCpuUtilization(), 0);

    /* error reading from psp module, reports -1  */
    UT_SetDeferredRetcode(UT_KEY(CFE_PSP_IODriver_Command), 1, CFE_PSP_ERROR);
    UtAssert_INT32_EQ(HS_SysMonGetCpuUtilization(), -1);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(Test_HS_SysMonInit, HS_Test_Setup, HS_Test_TearDown, "Test_HS_SysMonInit");

    UtTest_Add(Test_HS_SysMonCleanup, HS_Test_Setup, HS_Test_TearDown, "Test_HS_SysMonCleanup");
    UtTest_Add(Test_HS_SysMonGetCpuUtilization, HS_Test_Setup, HS_Test_TearDown, "Test_HS_SysMonGetCpuUtilization");
}
