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
 * Auto-Generated stub implementations for functions defined in hs_msg_action header
 */

#include "hs_msg_action.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MsgAct_TriggerAction()
 * ----------------------------------------------------
 */
void HS_MsgAct_TriggerAction(uint16 ActionType, HS_MsgAct_Callback_t SendEventCb, const void *CbArg)
{
    UT_GenStub_AddParam(HS_MsgAct_TriggerAction, uint16, ActionType);
    UT_GenStub_AddParam(HS_MsgAct_TriggerAction, HS_MsgAct_Callback_t, SendEventCb);
    UT_GenStub_AddParam(HS_MsgAct_TriggerAction, const void *, CbArg);

    UT_GenStub_Execute(HS_MsgAct_TriggerAction, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MsgAct_AcquirePointers()
 * ----------------------------------------------------
 */
void HS_MsgAct_AcquirePointers(void)
{
    UT_GenStub_Execute(HS_MsgAct_AcquirePointers, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MsgAct_Cooldown()
 * ----------------------------------------------------
 */
void HS_MsgAct_Cooldown(void)
{
    UT_GenStub_Execute(HS_MsgAct_Cooldown, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MsgAct_StatusRefresh()
 * ----------------------------------------------------
 */
void HS_MsgAct_StatusRefresh(void)
{
    UT_GenStub_Execute(HS_MsgAct_StatusRefresh, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MsgAct_ValidateTable()
 * ----------------------------------------------------
 */
int32 HS_MsgAct_ValidateTable(void *TableData)
{
    UT_GenStub_SetupReturnBuffer(HS_MsgAct_ValidateTable, int32);

    UT_GenStub_AddParam(HS_MsgAct_ValidateTable, void *, TableData);

    UT_GenStub_Execute(HS_MsgAct_ValidateTable, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_MsgAct_ValidateTable, int32);
}
