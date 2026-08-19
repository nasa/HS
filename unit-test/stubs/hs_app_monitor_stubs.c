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
 * Auto-Generated stub implementations for functions defined in hs_app_monitor header
 */

#include "hs_app_monitor.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for HS_AppMon_AcquirePointers()
 * ----------------------------------------------------
 */
void HS_AppMon_AcquirePointers(void)
{
    UT_GenStub_Execute(HS_AppMon_AcquirePointers, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_AppMon_StatusRefresh()
 * ----------------------------------------------------
 */
void HS_AppMon_StatusRefresh(void)
{
    UT_GenStub_Execute(HS_AppMon_StatusRefresh, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_AppMon_ComputeEnableBits()
 * ----------------------------------------------------
 */
void HS_AppMon_ComputeEnableBits(void *AppMonEnableBits)
{
    UT_GenStub_AddParam(HS_AppMon_ComputeEnableBits, void *, AppMonEnableBits);

    UT_GenStub_Execute(HS_AppMon_ComputeEnableBits, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_AppMon_CheckAllApps()
 * ----------------------------------------------------
 */
void HS_AppMon_CheckAllApps(void)
{
    UT_GenStub_Execute(HS_AppMon_CheckAllApps, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_AppMon_CheckApp()
 * ----------------------------------------------------
 */
void HS_AppMon_CheckApp(const HS_AMTEntry_t *AMEntryPtr, HS_AppMon_State_t *AMStatePtr)
{
    UT_GenStub_AddParam(HS_AppMon_CheckApp, const HS_AMTEntry_t *, AMEntryPtr);
    UT_GenStub_AddParam(HS_AppMon_CheckApp, HS_AppMon_State_t *, AMStatePtr);

    UT_GenStub_Execute(HS_AppMon_CheckApp, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_AppMon_ValidateTable()
 * ----------------------------------------------------
 */
int32 HS_AppMon_ValidateTable(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(HS_AppMon_ValidateTable, int32);

    UT_GenStub_AddParam(HS_AppMon_ValidateTable, void *, TableData);

    UT_GenStub_Execute(HS_AppMon_ValidateTable, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_AppMon_ValidateTable, int32);
}
