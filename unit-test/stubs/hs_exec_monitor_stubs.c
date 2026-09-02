/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 * Auto-Generated stub implementations for functions defined in hs_exec_monitor header
 */

#include "hs_exec_monitor.h"
#include "utgenstub.h"

void UT_DefaultHandler_HS_ExecMon_GetCounters(void *, UT_EntryKey_t, const UT_StubContext_t *);

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ExecMon_GetCounters()
 * ----------------------------------------------------
 */
void HS_ExecMon_GetCounters(uint32 *ExeCounts)
{
    UT_GenStub_AddParam(HS_ExecMon_GetCounters, uint32 *, ExeCounts);

    UT_GenStub_Execute(HS_ExecMon_GetCounters, Basic, UT_DefaultHandler_HS_ExecMon_GetCounters);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ExecMon_AcquirePointers()
 * ----------------------------------------------------
 */
void HS_ExecMon_AcquirePointers(void)
{
    UT_GenStub_Execute(HS_ExecMon_AcquirePointers, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ExecMon_StatusRefresh()
 * ----------------------------------------------------
 */
void HS_ExecMon_StatusRefresh(void)
{
    UT_GenStub_Execute(HS_ExecMon_StatusRefresh, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ExecMon_CheckUtilization()
 * ----------------------------------------------------
 */
void HS_ExecMon_CheckUtilization(void)
{
    UT_GenStub_Execute(HS_ExecMon_CheckUtilization, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ExecMon_ValidateTable()
 * ----------------------------------------------------
 */
int32 HS_ExecMon_ValidateTable(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(HS_ExecMon_ValidateTable, int32);

    UT_GenStub_AddParam(HS_ExecMon_ValidateTable, void *, TableData);

    UT_GenStub_Execute(HS_ExecMon_ValidateTable, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_ExecMon_ValidateTable, int32);
}
