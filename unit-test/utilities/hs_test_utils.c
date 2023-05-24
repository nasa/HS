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

/**
 * @file
 *   This file contains unit test utilities for the HS application.
 */

/*
 * Includes
 */

#include "hs_tbldefs.h"
#include "hs_app.h"
#include "hs_verify.h"
#include "hs_tbl.h"
#include "hs_msgdefs.h"
#include "hs_version.h"
#include "hs_eventids.h"
#include "hs_utils.h"
#include "hs_cmds.h"
#include "hs_monitors.h"
#include "hs_msg.h"

#include "hs_test_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#define UT_MAX_SENDEVENT_DEPTH 32
CFE_EVS_SendEvent_context_t    context_CFE_EVS_SendEvent[UT_MAX_SENDEVENT_DEPTH];
CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;

UT_CmdBuf_t UT_CmdBuf;

/*
 * Function Definitions
 */
void UT_Handler_CFE_EVS_SendEvent(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va)
{
    uint16 CallCount;
    uint16 idx;

    CallCount = UT_GetStubCount(UT_KEY(CFE_EVS_SendEvent));

    if (CallCount > (sizeof(context_CFE_EVS_SendEvent) / sizeof(context_CFE_EVS_SendEvent[0])))
    {
        UtAssert_Failed("CFE_EVS_SendEvent UT depth %u exceeded: %u, increase UT_MAX_SENDEVENT_DEPTH",
                        UT_MAX_SENDEVENT_DEPTH, CallCount);
    }
    else
    {
        idx                                      = CallCount - 1;
        context_CFE_EVS_SendEvent[idx].EventID   = UT_Hook_GetArgValueByName(Context, "EventID", uint16);
        context_CFE_EVS_SendEvent[idx].EventType = UT_Hook_GetArgValueByName(Context, "EventType", uint16);

        strncpy(context_CFE_EVS_SendEvent[idx].Spec, UT_Hook_GetArgValueByName(Context, "Spec", const char *),
                CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);
        context_CFE_EVS_SendEvent[idx].Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1] = '\0';
    }
}

void UT_Handler_CFE_ES_WriteToSysLog(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context, va_list va)
{
    strncpy(context_CFE_ES_WriteToSysLog.Spec, UT_Hook_GetArgValueByName(Context, "SpecStringPtr", const char *),
            CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1);
    context_CFE_ES_WriteToSysLog.Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH - 1] = '\0';
}

void CFE_PSP_WatchdogEnable(void)
{
    UT_DEFAULT_IMPL(CFE_PSP_WatchdogEnable);
}

void CFE_PSP_WatchdogService(void)
{
    UT_DEFAULT_IMPL(CFE_PSP_WatchdogService);
}

void CFE_PSP_WatchdogSet(uint32 WatchdogValue)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(CFE_PSP_WatchdogSet), WatchdogValue);
    UT_DEFAULT_IMPL(CFE_PSP_WatchdogSet);
}

void HS_Test_Setup(void)
{
    /* initialize test environment to default state for every test */
    UT_ResetState(0);

    memset(&HS_AppData, 0, sizeof(HS_AppData));
    memset(context_CFE_EVS_SendEvent, 0, sizeof(context_CFE_EVS_SendEvent));
    memset(&context_CFE_ES_WriteToSysLog, 0, sizeof(context_CFE_ES_WriteToSysLog));
    memset(&UT_CmdBuf, 0, sizeof(UT_CmdBuf));

    /* Register custom handlers */
    UT_SetVaHandlerFunction(UT_KEY(CFE_EVS_SendEvent), UT_Handler_CFE_EVS_SendEvent, NULL);
    UT_SetVaHandlerFunction(UT_KEY(CFE_ES_WriteToSysLog), UT_Handler_CFE_ES_WriteToSysLog, NULL);
}

void HS_Test_TearDown(void)
{
    /* cleanup test environment */
}
