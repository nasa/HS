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
 *   CFS Health and Safety (HS) command handling routines
 */

/************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_cmds.h"
#include "hs_msgids.h"
#include "hs_msgdefs.h"
#include "hs_eventids.h"
#include "hs_dispatch.h"
#include "hs_version.h"

#include "hs_eds_dispatcher.h"
#include "hs_eds_dictionary.h"

/* clang-format off */
/*
 * Define a lookup table for SAMPLE app command codes
 */
static const EdsDispatchTable_HS_Application_CFE_SB_Telecommand_t HS_TC_DISPATCH_TABLE =
{
    .CMD =
    {
        .DisableAlivenessCmd_indication = HS_DisableAlivenessCmd,
        .DisableAppMonCmd_indication = HS_DisableAppMonCmd,
        .DisableCpuHogCmd_indication = HS_DisableCpuHogCmd,
        .DisableEventMonCmd_indication = HS_DisableEventMonCmd,
        .EnableAlivenessCmd_indication = HS_EnableAlivenessCmd,
        .EnableAppMonCmd_indication = HS_EnableAppMonCmd,
        .EnableCpuHogCmd_indication = HS_EnableCpuHogCmd,
        .EnableEventMonCmd_indication = HS_EnableEventMonCmd,
        .NoopCmd_indication = HS_NoopCmd,
        .ResetCmd_indication = HS_ResetCmd,
        .ResetResetsPerformedCmd_indication = HS_ResetResetsPerformedCmd,
        .SetMaxResetsCmd_indication = HS_SetMaxResetsCmd,
    },
    .SEND_HK =
    {
        .indication = HS_SendHkCmd
    }
};
/* clang-format on */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SAMPLE    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void HS_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_Status_t      Status;
    CFE_SB_MsgId_t    MsgId;
    CFE_MSG_Size_t    MsgSize;
    CFE_MSG_FcnCode_t MsgFc;

    Status = EdsDispatch_HS_Application_Telecommand(SBBufPtr, &HS_TC_DISPATCH_TABLE);

    if (Status != CFE_SUCCESS)
    {
        CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
        CFE_MSG_GetSize(&SBBufPtr->Msg, &MsgSize);
        CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &MsgFc);

        if (Status == CFE_STATUS_UNKNOWN_MSG_ID)
        {
            CFE_EVS_SendEvent(HS_MID_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid command pipe message ID: 0x%08lX",
                              (unsigned long)CFE_SB_MsgIdToValue(MsgId));
            HS_AppData.CmdErrCount++;
        }
        else if (Status == CFE_STATUS_WRONG_MSG_LENGTH)
        {
            if (CFE_SB_MsgIdToValue(MsgId) == HS_SEND_HK_MID)
            {
                /*
                ** For a bad HK request, just send the event. We only increment
                ** the error counter for ground commands and not internal messages.
                */
                CFE_EVS_SendEvent(HS_HKREQ_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Invalid HK request msg length: ID = 0x%08lX, CC = %d, Len = %d",
                                  (unsigned long)CFE_SB_MsgIdToValue(MsgId), (int)MsgFc, (int)MsgSize);
            }
            else
            {
                /*
                ** All other cases, increment error counter
                */
                CFE_EVS_SendEvent(HS_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Invalid msg length: ID = 0x%08lX, CC = %d, Len = %d",
                                  (unsigned long)CFE_SB_MsgIdToValue(MsgId), (int)MsgFc, (int)MsgSize);
                HS_AppData.CmdErrCount++;
            }
        }
        else
        {
            CFE_EVS_SendEvent(HS_CC_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid command code: ID = 0x%08lX, CC = %d",
                              (unsigned long)CFE_SB_MsgIdToValue(MsgId), (int)MsgFc);
            HS_AppData.CmdErrCount++;
        }
    }
}
