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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify message packet length                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool HS_VerifyMsgLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_MSG_FcnCode_t CommandCode  = 0;
    CFE_SB_MsgId_t    MessageID    = CFE_SB_INVALID_MSG_ID;

    /*
    ** Verify the message packet length...
    */

    CFE_MSG_GetSize(MsgPtr, &ActualLength);
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MessageID);
        CFE_MSG_GetFcnCode(MsgPtr, &CommandCode);

        if (CFE_SB_MsgIdToValue(MessageID) == HS_SEND_HK_MID)
        {
            /*
            ** For a bad HK request, just send the event. We only increment
            ** the error counter for ground commands and not internal messages.
            */
            CFE_EVS_SendEvent(HS_HKREQ_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid HK request msg length: ID = 0x%08lX, CC = %d, Len = %d, Expected = %d",
                              (unsigned long)CFE_SB_MsgIdToValue(MessageID), (int)CommandCode, (int)ActualLength,
                              (int)ExpectedLength);
        }
        else
        {
            /*
            ** All other cases, increment error counter
            */
            CFE_EVS_SendEvent(HS_CMD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid msg length: ID = 0x%08lX, CC = %d, Len = %d, Expected = %d",
                              (unsigned long)CFE_SB_MsgIdToValue(MessageID), (int)CommandCode, (int)ActualLength,
                              (int)ExpectedLength);
            HS_AppData.CmdErrCount++;
        }

        result = false;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process a command pipe message                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_AppPipe(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;
    CFE_SB_MsgId_t    MessageID   = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&BufPtr->Msg, &MessageID);

    switch (CFE_SB_MsgIdToValue(MessageID))
    {
        /*
        ** Housekeeping telemetry request
        */
        case HS_SEND_HK_MID:
            HS_SendHkVerifyDispatch(BufPtr);
            break;

        /*
        ** HS application commands...
        */
        case HS_CMD_MID:

            CFE_MSG_GetFcnCode(&BufPtr->Msg, &CommandCode);

            switch (CommandCode)
            {
                case HS_NOOP_CC:
                    HS_NoopVerifyDispatch(BufPtr);
                    break;

                case HS_RESET_CC:
                    HS_ResetVerifyDispatch(BufPtr);
                    break;

                case HS_ENABLE_APP_MON_CC:
                    HS_EnableAppMonVerifyDispatch(BufPtr);
                    break;

                case HS_DISABLE_APP_MON_CC:
                    HS_DisableAppMonVerifyDispatch(BufPtr);
                    break;

                case HS_ENABLE_EVENT_MON_CC:
                    HS_EnableEventMonVerifyDispatch(BufPtr);
                    break;

                case HS_DISABLE_EVENT_MON_CC:
                    HS_DisableEventMonVerifyDispatch(BufPtr);
                    break;

                case HS_ENABLE_ALIVENESS_CC:
                    HS_EnableAlivenessVerifyDispatch(BufPtr);
                    break;

                case HS_DISABLE_ALIVENESS_CC:
                    HS_DisableAlivenessVerifyDispatch(BufPtr);
                    break;

                case HS_RESET_RESETS_PERFORMED_CC:
                    HS_ResetResetsPerformedVerifyDispatch(BufPtr);
                    break;

                case HS_SET_MAX_RESETS_CC:
                    HS_SetMaxResetsVerifyDispatch(BufPtr);
                    break;

                case HS_ENABLE_CPU_HOG_CC:
                    HS_EnableCpuHogVerifyDispatch(BufPtr);
                    break;

                case HS_DISABLE_CPU_HOG_CC:
                    HS_DisableCpuHogVerifyDispatch(BufPtr);
                    break;

                default:
                    CFE_EVS_SendEvent(HS_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Invalid command code: ID = 0x%08lX, CC = %d",
                                      (unsigned long)CFE_SB_MsgIdToValue(MessageID), CommandCode);

                    HS_AppData.CmdErrCount++;
                    break;

            } /* end CommandCode switch */
            break;

        /*
        ** Unrecognized Message ID
        */
        default:
            HS_AppData.CmdErrCount++;
            CFE_EVS_SendEvent(HS_MID_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid command pipe message ID: 0x%08lX",
                              (unsigned long)CFE_SB_MsgIdToValue(MessageID));
            break;

    } /* end MessageID switch */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Housekeeping request                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_SendHkVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_SendHkCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_SendHkCmd((const HS_SendHkCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Noop command                                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_NoopVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoopCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_NoopCmd((const HS_NoopCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset counters command                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_ResetVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_ResetCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_ResetCmd((const HS_ResetCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable applications monitor command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableAppMonVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_EnableAppMonCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_EnableAppMonCmd((const HS_EnableAppMonCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable applications monitor command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableAppMonVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_DisableAppMonCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_DisableAppMonCmd((const HS_DisableAppMonCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable events monitor command                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableEventMonVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_EnableEventMonCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_EnableEventMonCmd((const HS_EnableEventMonCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable event monitor command                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableEventMonVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_DisableEventMonCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_DisableEventMonCmd((const HS_DisableEventMonCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable aliveness indicator command                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableAlivenessVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_EnableAlivenessCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_EnableAlivenessCmd((const HS_EnableAlivenessCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable aliveness indicator command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableAlivenessVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_DisableAlivenessCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_DisableAlivenessCmd((const HS_DisableAlivenessCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable cpu hogging indicator command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableCpuHogVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_EnableCpuHogCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_EnableCpuHogCmd((const HS_EnableCpuHogCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable cpu hogging indicator command                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableCpuHogVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_DisableCpuHogCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_DisableCpuHogCmd((const HS_DisableCpuHogCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset processor resets performed count command                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_ResetResetsPerformedVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_ResetResetsPerformedCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_ResetResetsPerformedCmd((const HS_ResetResetsPerformedCmd_t *)BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set max processor resets command                                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_SetMaxResetsVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_SetMaxResetsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_SetMaxResetsCmd((const HS_SetMaxResetsCmd_t *)BufPtr);
    }
}
