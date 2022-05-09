/************************************************************************
 * NASA Docket No. GSC-18,920-1, and identified as “Core Flight
 * System (cFS) Health & Safety (HS) Application version 2.4.0”
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

#include "hs_app.h"
#include "hs_monitors.h"
#include "hs_custom.h"
#include "hs_tbldefs.h"
#include "hs_events.h"
#include "hs_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

void HS_MonitorApplications(void)
{
    UT_DEFAULT_IMPL(HS_MonitorApplications);
}

void HS_MonitorEvent(const CFE_EVS_LongEventTlm_t *EventPtr)
{
    UT_DEFAULT_IMPL(HS_MonitorEvent);
}

void HS_MonitorUtilization(void)
{
    UT_DEFAULT_IMPL(HS_MonitorUtilization);
}

int32 HS_ValidateAMTable(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ValidateAMTable), TableData);
    return UT_DEFAULT_IMPL(HS_ValidateAMTable);
}

int32 HS_ValidateEMTable(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ValidateEMTable), TableData);
    return UT_DEFAULT_IMPL(HS_ValidateEMTable);
}

int32 HS_ValidateXCTable(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ValidateXCTable), TableData);
    return UT_DEFAULT_IMPL(HS_ValidateXCTable);
}

int32 HS_ValidateMATable(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ValidateMATable), TableData);
    return UT_DEFAULT_IMPL(HS_ValidateMATable);
}

void HS_SetCDSData(uint16 ResetsPerformed, uint16 MaxResets)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_SetCDSData), ResetsPerformed);
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_SetCDSData), MaxResets);
    UT_DEFAULT_IMPL(HS_SetCDSData);
}
