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

/* hs_utils_tests globals */
uint8 call_count_CFE_EVS_SendEvent;

/*
 * Function Definitions
 */

void HS_AMTActionIsValid_Valid(void)
{
    uint16 Action = (HS_AMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES);

    bool Result = HS_AMTActionIsValid(Action);

    UtAssert_True(Result == true, "Result == true");
}

void HS_AMTActionIsValid_Invalid(void)
{
    uint16 Action = (HS_AMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1);

    bool Result = HS_AMTActionIsValid(Action);

    UtAssert_True(Result == false, "Result == false");
}

void HS_EMTActionIsValid_Valid(void)
{
    uint16 Action = (HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES);

    bool Result = HS_EMTActionIsValid(Action);

    UtAssert_True(Result == true, "Result == true");
}

void HS_EMTActionIsValid_Invalid(void)
{
    uint16 Action = (HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES + 1);

    bool Result = HS_EMTActionIsValid(Action);

    UtAssert_True(Result == false, "Result == false");
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_AMTActionIsValid_Valid, HS_Test_Setup, HS_Test_TearDown, "HS_AMTActionIsValid_Valid");

    UtTest_Add(HS_AMTActionIsValid_Invalid, HS_Test_Setup, HS_Test_TearDown, "HS_AMTActionIsValid_Invalid");

    UtTest_Add(HS_EMTActionIsValid_Valid, HS_Test_Setup, HS_Test_TearDown, "HS_EMTActionIsValid_Valid");

    UtTest_Add(HS_EMTActionIsValid_Invalid, HS_Test_Setup, HS_Test_TearDown, "HS_EMTActionIsValid_Invalid");
}
