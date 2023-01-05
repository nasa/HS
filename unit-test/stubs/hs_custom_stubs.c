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

#include "cfe.h"
#include "cfe_psp.h"
#include "osapi.h"
#include "hs_app.h"
#include "hs_cmds.h"
#include "hs_msg.h"
#include "hs_utils.h"
#include "hs_custom_internal.h"
#include "hs_events.h"
#include "hs_monitors.h"
#include "hs_perfids.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

HS_CustomData_t HS_CustomData;

int32 HS_CustomInit(void)
{
    return UT_DEFAULT_IMPL(HS_CustomInit);
}

void HS_CustomCleanup(void)
{
    UT_DEFAULT_IMPL(HS_CustomCleanup);
}

void HS_CustomMonitorUtilization(void)
{
    UT_DEFAULT_IMPL(HS_CustomMonitorUtilization);
}

int32 HS_CustomGetUtil(void)
{
    return UT_DEFAULT_IMPL(HS_CustomGetUtil);
}

int32 HS_CustomCommands(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_CustomCommands), BufPtr);
    return UT_DEFAULT_IMPL(HS_CustomCommands);
}

void HS_IdleTask(void)
{
    UT_DEFAULT_IMPL(HS_IdleTask);
}

void HS_UtilizationIncrement(void)
{
    UT_DEFAULT_IMPL(HS_UtilizationIncrement);
}

void HS_UtilizationMark(void)
{
    UT_DEFAULT_IMPL(HS_UtilizationMark);
}

void HS_MarkIdleCallback(void)
{
    UT_DEFAULT_IMPL(HS_MarkIdleCallback);
}

void HS_UtilDiagReport(void)
{
    UT_DEFAULT_IMPL(HS_UtilDiagReport);
}

void HS_SetUtilParamsCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_SetUtilParamsCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_SetUtilParamsCmd);
}

void HS_SetUtilDiagCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_SetUtilDiagCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_SetUtilDiagCmd);
}
