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
 * Auto-Generated stub implementations for functions defined in hs_event_monitor header
 */

#include "hs_event_monitor.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for HS_EventMon_AcquirePointers()
 * ----------------------------------------------------
 */
void HS_EventMon_AcquirePointers(void)
{
    UT_GenStub_Execute(HS_EventMon_AcquirePointers, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_EventMon_StatusRefresh()
 * ----------------------------------------------------
 */
void HS_EventMon_StatusRefresh(void)
{
    UT_GenStub_Execute(HS_EventMon_StatusRefresh, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_EventMon_Check()
 * ----------------------------------------------------
 */
void HS_EventMon_Check(const CFE_EVS_LongEventTlm_t *EventPtr)
{
    UT_GenStub_AddParam(HS_EventMon_Check, const CFE_EVS_LongEventTlm_t *, EventPtr);

    UT_GenStub_Execute(HS_EventMon_Check, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_EventMon_TriggerAction()
 * ----------------------------------------------------
 */
void HS_EventMon_TriggerAction(const HS_EMTEntry_t *EMEntryPtr)
{
    UT_GenStub_AddParam(HS_EventMon_TriggerAction, const HS_EMTEntry_t *, EMEntryPtr);

    UT_GenStub_Execute(HS_EventMon_TriggerAction, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_EventMon_ValidateTable()
 * ----------------------------------------------------
 */
int32 HS_EventMon_ValidateTable(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(HS_EventMon_ValidateTable, int32);

    UT_GenStub_AddParam(HS_EventMon_ValidateTable, void *, TableData);

    UT_GenStub_Execute(HS_EventMon_ValidateTable, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_EventMon_ValidateTable, int32);
}
