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
 *
 * Auto-Generated stub implementations for functions defined in hs_monitors header
 */

#include "hs_monitors.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MonitorApplications()
 * ----------------------------------------------------
 */
void HS_MonitorApplications(void)
{

    UT_GenStub_Execute(HS_MonitorApplications, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MonitorEvent()
 * ----------------------------------------------------
 */
void HS_MonitorEvent(const CFE_EVS_LongEventTlm_t *EventPtr)
{
    UT_GenStub_AddParam(HS_MonitorEvent, const CFE_EVS_LongEventTlm_t *, EventPtr);

    UT_GenStub_Execute(HS_MonitorEvent, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MonitorUtilization()
 * ----------------------------------------------------
 */
void HS_MonitorUtilization(void)
{

    UT_GenStub_Execute(HS_MonitorUtilization, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_SetCDSData()
 * ----------------------------------------------------
 */
void HS_SetCDSData(uint16 ResetsPerformed, uint16 MaxResets)
{
    UT_GenStub_AddParam(HS_SetCDSData, uint16, ResetsPerformed);
    UT_GenStub_AddParam(HS_SetCDSData, uint16, MaxResets);

    UT_GenStub_Execute(HS_SetCDSData, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ValidateAMTable()
 * ----------------------------------------------------
 */
int32 HS_ValidateAMTable(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(HS_ValidateAMTable, int32);

    UT_GenStub_AddParam(HS_ValidateAMTable, void *, TableData);

    UT_GenStub_Execute(HS_ValidateAMTable, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_ValidateAMTable, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ValidateEMTable()
 * ----------------------------------------------------
 */
int32 HS_ValidateEMTable(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(HS_ValidateEMTable, int32);

    UT_GenStub_AddParam(HS_ValidateEMTable, void *, TableData);

    UT_GenStub_Execute(HS_ValidateEMTable, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_ValidateEMTable, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ValidateMATable()
 * ----------------------------------------------------
 */
int32 HS_ValidateMATable(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(HS_ValidateMATable, int32);

    UT_GenStub_AddParam(HS_ValidateMATable, void *, TableData);

    UT_GenStub_Execute(HS_ValidateMATable, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_ValidateMATable, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ValidateXCTable()
 * ----------------------------------------------------
 */
int32 HS_ValidateXCTable(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(HS_ValidateXCTable, int32);

    UT_GenStub_AddParam(HS_ValidateXCTable, void *, TableData);

    UT_GenStub_Execute(HS_ValidateXCTable, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_ValidateXCTable, int32);
}
