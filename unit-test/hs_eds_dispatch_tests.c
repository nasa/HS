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

#include "cfe.h"
#include "cfe_msg_dispatcher.h"

/*
**********************************************************************************
**          TEST CASE FUNCTIONS
**********************************************************************************
*/

void Test_HS_AppPipe(void)
{
    /*
     * Test Case For:
     * void HS_AppPipe
     */
    CFE_SB_Buffer_t UtBuf;

    UT_SetDeferredRetcode(UT_KEY(CFE_MSG_EdsDispatch), 1, CFE_SUCCESS);

    memset(&UtBuf, 0, sizeof(UtBuf));
    UtAssert_VOIDCALL(HS_AppPipe(&UtBuf));
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(Test_HS_AppPipe, HS_Test_Setup, HS_Test_TearDown, "Test_HS_AppPipe");
}
