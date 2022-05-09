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

/**
 * @file
 *   CFS Health and Safety (HS) command handling routines
 */

/************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_cmds.h"
#include "hs_custom.h"
#include "hs_monitors.h"
#include "hs_msgids.h"
#include "hs_events.h"
#include "hs_utils.h"
#include "hs_version.h"

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
            HS_HousekeepingReq(BufPtr);
            break;

        /*
        ** HS application commands...
        */
        case HS_CMD_MID:

            CFE_MSG_GetFcnCode(&BufPtr->Msg, &CommandCode);

            switch (CommandCode)
            {
                case HS_NOOP_CC:
                    HS_NoopCmd(BufPtr);
                    break;

                case HS_RESET_CC:
                    HS_ResetCmd(BufPtr);
                    break;

                case HS_ENABLE_APPMON_CC:
                    HS_EnableAppMonCmd(BufPtr);
                    break;

                case HS_DISABLE_APPMON_CC:
                    HS_DisableAppMonCmd(BufPtr);
                    break;

                case HS_ENABLE_EVENTMON_CC:
                    HS_EnableEventMonCmd(BufPtr);
                    break;

                case HS_DISABLE_EVENTMON_CC:
                    HS_DisableEventMonCmd(BufPtr);
                    break;

                case HS_ENABLE_ALIVENESS_CC:
                    HS_EnableAlivenessCmd(BufPtr);
                    break;

                case HS_DISABLE_ALIVENESS_CC:
                    HS_DisableAlivenessCmd(BufPtr);
                    break;

                case HS_RESET_RESETS_PERFORMED_CC:
                    HS_ResetResetsPerformedCmd(BufPtr);
                    break;

                case HS_SET_MAX_RESETS_CC:
                    HS_SetMaxResetsCmd(BufPtr);
                    break;

                case HS_ENABLE_CPUHOG_CC:
                    HS_EnableCPUHogCmd(BufPtr);
                    break;

                case HS_DISABLE_CPUHOG_CC:
                    HS_DisableCPUHogCmd(BufPtr);
                    break;

                default:
                    if (HS_CustomCommands(BufPtr) != CFE_SUCCESS)
                    {
                        CFE_EVS_SendEvent(HS_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                                          "Invalid command code: ID = 0x%08lX, CC = %d",
                                          (unsigned long)CFE_SB_MsgIdToValue(MessageID), CommandCode);

                        HS_AppData.CmdErrCount++;
                    }
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

    return;

} /* End HS_AppPipe */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Housekeeping request                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_HousekeepingReq(const CFE_SB_Buffer_t *BufPtr)
{
    size_t         ExpectedLength = sizeof(HS_NoArgsCmd_t);
    CFE_ES_AppId_t AppId          = CFE_ES_APPID_UNDEFINED;
#if HS_MAX_EXEC_CNT_SLOTS != 0
    uint32             ExeCount  = 0;
    CFE_ES_TaskId_t    TaskId    = CFE_ES_TASKID_UNDEFINED;
    CFE_ES_CounterId_t CounterId = CFE_ES_COUNTERID_UNDEFINED;
    CFE_ES_TaskInfo_t  TaskInfo;
    CFE_PSP_MemSet(&TaskInfo, 0, sizeof(CFE_ES_TaskInfo_t));
#endif
    int32  Status     = CFE_SUCCESS;
    uint32 TableIndex = 0;

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        /*
        ** Update HK variables
        */
        HS_AppData.HkPacket.CmdCount              = HS_AppData.CmdCount;
        HS_AppData.HkPacket.CmdErrCount           = HS_AppData.CmdErrCount;
        HS_AppData.HkPacket.CurrentAppMonState    = HS_AppData.CurrentAppMonState;
        HS_AppData.HkPacket.CurrentEventMonState  = HS_AppData.CurrentEventMonState;
        HS_AppData.HkPacket.CurrentAlivenessState = HS_AppData.CurrentAlivenessState;
        HS_AppData.HkPacket.CurrentCPUHogState    = HS_AppData.CurrentCPUHogState;
        HS_AppData.HkPacket.ResetsPerformed       = HS_AppData.CDSData.ResetsPerformed;
        HS_AppData.HkPacket.MaxResets             = HS_AppData.CDSData.MaxResets;
        HS_AppData.HkPacket.EventsMonitoredCount  = HS_AppData.EventsMonitoredCount;
        HS_AppData.HkPacket.MsgActExec            = HS_AppData.MsgActExec;

        /*
        ** Calculate the current number of invalid event monitor entries
        */
        HS_AppData.HkPacket.InvalidEventMonCount = 0;

        for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_EVENTS; TableIndex++)
        {
            if (HS_AppData.EMTablePtr[TableIndex].ActionType != HS_EMT_ACT_NOACT)
            {
                Status = CFE_ES_GetAppIDByName(&AppId, HS_AppData.EMTablePtr[TableIndex].AppName);

                if (Status != CFE_SUCCESS)
                {
                    HS_AppData.HkPacket.InvalidEventMonCount++;
                }
            }
        }

        /*
        ** Build the HK status flags byte
        */
        HS_AppData.HkPacket.StatusFlags = 0;
#if HS_MAX_EXEC_CNT_SLOTS != 0
        if (HS_AppData.ExeCountState == HS_STATE_ENABLED)
        {
            HS_AppData.HkPacket.StatusFlags |= HS_LOADED_XCT;
        }
#endif
        if (HS_AppData.MsgActsState == HS_STATE_ENABLED)
        {
            HS_AppData.HkPacket.StatusFlags |= HS_LOADED_MAT;
        }
        if (HS_AppData.AppMonLoaded == HS_STATE_ENABLED)
        {
            HS_AppData.HkPacket.StatusFlags |= HS_LOADED_AMT;
        }
        if (HS_AppData.EventMonLoaded == HS_STATE_ENABLED)
        {
            HS_AppData.HkPacket.StatusFlags |= HS_LOADED_EMT;
        }
        if (HS_AppData.CDSState == HS_STATE_ENABLED)
        {
            HS_AppData.HkPacket.StatusFlags |= HS_CDS_IN_USE;
        }

        /*
        ** Update the AppMon Enables
        */
        for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
        {
            HS_AppData.HkPacket.AppMonEnables[TableIndex] = HS_AppData.AppMonEnables[TableIndex];
        }

        HS_AppData.HkPacket.UtilCpuAvg  = HS_AppData.UtilCpuAvg;
        HS_AppData.HkPacket.UtilCpuPeak = HS_AppData.UtilCpuPeak;

#if HS_MAX_EXEC_CNT_SLOTS != 0
        /*
        ** Add the execution counters
        */
        for (TableIndex = 0; TableIndex < HS_MAX_EXEC_CNT_SLOTS; TableIndex++)
        {

            ExeCount = HS_INVALID_EXECOUNT;

            if (HS_AppData.ExeCountState == HS_STATE_ENABLED)
            {
                switch (HS_AppData.XCTablePtr[TableIndex].ResourceType)
                {
                    case HS_XCT_TYPE_APP_MAIN:
                    case HS_XCT_TYPE_APP_CHILD:
                        Status = CFE_ES_GetTaskIDByName(&TaskId, HS_AppData.XCTablePtr[TableIndex].ResourceName);

                        if (Status == CFE_SUCCESS)
                        {
                            Status = CFE_ES_GetTaskInfo(&TaskInfo, TaskId);
                            if (Status == CFE_SUCCESS)
                            {
                                ExeCount = TaskInfo.ExecutionCounter;
                            }
                        }
                        break;
                    case HS_XCT_TYPE_DEVICE:
                    case HS_XCT_TYPE_ISR:
                        Status =
                            CFE_ES_GetGenCounterIDByName(&CounterId, HS_AppData.XCTablePtr[TableIndex].ResourceName);

                        if (Status == CFE_SUCCESS)
                        {
                            CFE_ES_GetGenCount(CounterId, &ExeCount);
                        }
                        break;
                    case HS_XCT_TYPE_NOTYPE:
                        /* no action - ExeCount remains HS_INVALID_EXECOUNT */
                        break;
                    default:
                        /* ExeCount remains HS_INVALID_EXECOUNT */
                        CFE_EVS_SendEvent(HS_HKREQ_RESOURCE_DBG_EID, CFE_EVS_EventType_DEBUG,
                                          "Housekeeping req found unknown resource.  Type=0x%08X",
                                          HS_AppData.XCTablePtr[TableIndex].ResourceType);
                        break;
                } /* end ResourceType switch statement */
            }     /* end ExeCountState if statement */

            HS_AppData.HkPacket.ExeCounts[TableIndex] = ExeCount;
        }

#endif

        /*
        ** Timestamp and send housekeeping packet
        */
        CFE_SB_TimeStampMsg(&HS_AppData.HkPacket.TlmHeader.Msg);
        CFE_SB_TransmitMsg(&HS_AppData.HkPacket.TlmHeader.Msg, true);

    } /* end HS_VerifyMsgLength if */

    return;

} /* end HS_HousekeepingCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Noop command                                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_NoopCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_AppData.CmdCount++;

        CFE_EVS_SendEvent(HS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "No-op command: Version %d.%d.%d.%d",
                          HS_MAJOR_VERSION, HS_MINOR_VERSION, HS_REVISION, HS_MISSION_REV);
    }

    return;

} /* end HS_NoopCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset counters command                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_ResetCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_ResetCounters();

        CFE_EVS_SendEvent(HS_RESET_DBG_EID, CFE_EVS_EventType_DEBUG, "Reset counters command");
    }

    return;

} /* end HS_ResetCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset housekeeping counters                                     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_ResetCounters(void)
{
    HS_AppData.CmdCount             = 0;
    HS_AppData.CmdErrCount          = 0;
    HS_AppData.EventsMonitoredCount = 0;
    HS_AppData.MsgActExec           = 0;

    return;

} /* end HS_ResetCounters */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable applications monitor command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableAppMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_AppData.CmdCount++;
        HS_AppMonStatusRefresh();
        HS_AppData.CurrentAppMonState = HS_STATE_ENABLED;
        CFE_EVS_SendEvent(HS_ENABLE_APPMON_DBG_EID, CFE_EVS_EventType_DEBUG, "Application Monitoring Enabled");
    }

    return;

} /* end HS_EnableAppMonCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable applications monitor command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableAppMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_AppData.CmdCount++;
        HS_AppData.CurrentAppMonState = HS_STATE_DISABLED;
        CFE_EVS_SendEvent(HS_DISABLE_APPMON_DBG_EID, CFE_EVS_EventType_DEBUG, "Application Monitoring Disabled");
    }

    return;

} /* end HS_DisableAppMonCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable events monitor command                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableEventMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);
    int32  Status         = CFE_SUCCESS;

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        /*
        ** Subscribe to Event Messages if currently disabled
        */
        if (HS_AppData.CurrentEventMonState == HS_STATE_DISABLED)
        {

            Status = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(CFE_EVS_LONG_EVENT_MSG_MID), HS_AppData.EventPipe,
                                        CFE_SB_DEFAULT_QOS, HS_EVENT_PIPE_DEPTH);

            if (Status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(HS_EVENTMON_LONG_SUB_EID, CFE_EVS_EventType_ERROR,
                                  "Event Monitor Enable: Error Subscribing to long-format Events,RC=0x%08X",
                                  (unsigned int)Status);
            }

            if (Status == CFE_SUCCESS)
            {
                Status = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(CFE_EVS_SHORT_EVENT_MSG_MID), HS_AppData.EventPipe,
                                            CFE_SB_DEFAULT_QOS, HS_EVENT_PIPE_DEPTH);

                if (Status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(HS_EVENTMON_SHORT_SUB_EID, CFE_EVS_EventType_ERROR,
                                      "Event Monitor Enable: Error Subscribing to short-format Events,RC=0x%08X",
                                      (unsigned int)Status);
                }
            }
        }

        if (Status == CFE_SUCCESS)
        {
            HS_AppData.CmdCount++;
            HS_AppData.CurrentEventMonState = HS_STATE_ENABLED;
            CFE_EVS_SendEvent(HS_ENABLE_EVENTMON_DBG_EID, CFE_EVS_EventType_DEBUG, "Event Monitoring Enabled");
        }
        else
        {
            HS_AppData.CmdErrCount++;
        }
    }

    return;

} /* end HS_EnableEventMonCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable event monitor command                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableEventMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);
    int32  Status         = CFE_SUCCESS;

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {

        /*
        ** Unsubscribe from Event Messages if currently enabled
        */
        if (HS_AppData.CurrentEventMonState == HS_STATE_ENABLED)
        {

            Status = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(CFE_EVS_LONG_EVENT_MSG_MID), HS_AppData.EventPipe);

            if (Status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(HS_EVENTMON_LONG_UNSUB_EID, CFE_EVS_EventType_ERROR,
                                  "Event Monitor Disable: Error Unsubscribing from long-format Events,RC=0x%08X",
                                  (unsigned int)Status);
            }

            if (Status == CFE_SUCCESS)
            {
                Status = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(CFE_EVS_SHORT_EVENT_MSG_MID), HS_AppData.EventPipe);

                if (Status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(HS_EVENTMON_SHORT_UNSUB_EID, CFE_EVS_EventType_ERROR,
                                      "Event Monitor Disable: Error Unsubscribing from short-format Events,RC=0x%08X",
                                      (unsigned int)Status);
                }
            }
        }

        if (Status == CFE_SUCCESS)
        {
            HS_AppData.CmdCount++;
            HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;
            CFE_EVS_SendEvent(HS_DISABLE_EVENTMON_DBG_EID, CFE_EVS_EventType_DEBUG, "Event Monitoring Disabled");
        }
        else
        {
            HS_AppData.CmdErrCount++;
        }
    }

    return;

} /* end HS_DisableEventMonCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable aliveness indicator command                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableAlivenessCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_AppData.CmdCount++;
        HS_AppData.CurrentAlivenessState = HS_STATE_ENABLED;
        CFE_EVS_SendEvent(HS_ENABLE_ALIVENESS_DBG_EID, CFE_EVS_EventType_DEBUG, "Aliveness Indicator Enabled");
    }

    return;

} /* end HS_EnableAlivenessCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable aliveness indicator command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableAlivenessCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_AppData.CmdCount++;
        HS_AppData.CurrentAlivenessState = HS_STATE_DISABLED;
        CFE_EVS_SendEvent(HS_DISABLE_ALIVENESS_DBG_EID, CFE_EVS_EventType_DEBUG, "Aliveness Indicator Disabled");
    }

    return;

} /* end HS_DisableAlivenessCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable cpu hogging indicator command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableCPUHogCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_AppData.CmdCount++;
        HS_AppData.CurrentCPUHogState = HS_STATE_ENABLED;
        CFE_EVS_SendEvent(HS_ENABLE_CPUHOG_DBG_EID, CFE_EVS_EventType_DEBUG, "CPU Hogging Indicator Enabled");
    }

    return;

} /* end HS_EnableCPUHogCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable cpu hogging indicator command                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableCPUHogCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_AppData.CmdCount++;
        HS_AppData.CurrentCPUHogState = HS_STATE_DISABLED;
        CFE_EVS_SendEvent(HS_DISABLE_CPUHOG_DBG_EID, CFE_EVS_EventType_DEBUG, "CPU Hogging Indicator Disabled");
    }

    return;

} /* end HS_DisableCPUHogCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset processor resets performed count command                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_ResetResetsPerformedCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_NoArgsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_AppData.CmdCount++;
        HS_SetCDSData(0, HS_AppData.CDSData.MaxResets);
        CFE_EVS_SendEvent(HS_RESET_RESETS_DBG_EID, CFE_EVS_EventType_DEBUG,
                          "Processor Resets Performed by HS Counter has been Reset");
    }

    return;

} /* end HS_ResetResetsPerformedCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set max processor resets command                                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_SetMaxResetsCmd(const CFE_SB_Buffer_t *BufPtr)
{
    size_t                ExpectedLength = sizeof(HS_SetMaxResetsCmd_t);
    HS_SetMaxResetsCmd_t *CmdPtr         = NULL;

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_AppData.CmdCount++;
        CmdPtr = ((HS_SetMaxResetsCmd_t *)BufPtr);

        HS_SetCDSData(HS_AppData.CDSData.ResetsPerformed, CmdPtr->MaxResets);

        CFE_EVS_SendEvent(HS_SET_MAX_RESETS_DBG_EID, CFE_EVS_EventType_DEBUG,
                          "Max Resets Performable by HS has been set to %d", HS_AppData.CDSData.MaxResets);
    }

    return;

} /* end HS_SetMaxResetsCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Acquire table pointers                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_AcquirePointers(void)
{
    int32 Status = CFE_SUCCESS;

    /*
    ** Release the table (AppMon)
    */
    CFE_TBL_ReleaseAddress(HS_AppData.AMTableHandle);

    /*
    ** Manage the table (AppMon)
    */
    CFE_TBL_Manage(HS_AppData.AMTableHandle);

    /*
    ** Get a pointer to the table (AppMon)
    */
    Status = CFE_TBL_GetAddress((void *)&HS_AppData.AMTablePtr, HS_AppData.AMTableHandle);

    /*
    ** If there is a new table, refresh status (AppMon)
    */
    if (Status == CFE_TBL_INFO_UPDATED)
    {
        HS_AppMonStatusRefresh();
    }

    /*
    ** If Address acquisition fails and currently enabled, report and disable (AppMon)
    */
    if (Status < CFE_SUCCESS)
    {
        /*
        ** Only report and disable if enabled or the table was previously loaded (AppMon)
        */
        if ((HS_AppData.AppMonLoaded == HS_STATE_ENABLED) || (HS_AppData.CurrentAppMonState == HS_STATE_ENABLED))
        {
            CFE_EVS_SendEvent(HS_APPMON_GETADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Error getting AppMon Table address, RC=0x%08X, Application Monitoring Disabled",
                              (unsigned int)Status);
            HS_AppData.CurrentAppMonState = HS_STATE_DISABLED;
            HS_AppData.AppMonLoaded       = HS_STATE_DISABLED;
        }
    }
    /*
    ** Otherwise, mark that the table is loaded (AppMon)
    */
    else
    {
        HS_AppData.AppMonLoaded = HS_STATE_ENABLED;
    }

    /*
    ** Release the table (EventMon)
    */
    CFE_TBL_ReleaseAddress(HS_AppData.EMTableHandle);

    /*
    ** Manage the table (EventMon)
    */
    CFE_TBL_Manage(HS_AppData.EMTableHandle);

    /*
    ** Get a pointer to the table (EventMon)
    */
    Status = CFE_TBL_GetAddress((void *)&HS_AppData.EMTablePtr, HS_AppData.EMTableHandle);

    /*
    ** If Address acquisition fails and currently enabled, report and disable (EventMon)
    */
    if (Status < CFE_SUCCESS)
    {
        /*
        ** Only report and disable if enabled or the table was previously loaded (EventMon)
        */
        if ((HS_AppData.EventMonLoaded == HS_STATE_ENABLED) || (HS_AppData.CurrentEventMonState == HS_STATE_ENABLED))
        {
            CFE_EVS_SendEvent(HS_EVENTMON_GETADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Error getting EventMon Table address, RC=0x%08X, Event Monitoring Disabled",
                              (unsigned int)Status);

            if (HS_AppData.CurrentEventMonState == HS_STATE_ENABLED)
            {
                Status = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(CFE_EVS_LONG_EVENT_MSG_MID), HS_AppData.EventPipe);

                if (Status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(HS_BADEMT_LONG_UNSUB_EID, CFE_EVS_EventType_ERROR,
                                      "Error Unsubscribing from long-format Events,RC=0x%08X", (unsigned int)Status);
                }

                Status = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(CFE_EVS_SHORT_EVENT_MSG_MID), HS_AppData.EventPipe);

                if (Status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(HS_BADEMT_SHORT_UNSUB_EID, CFE_EVS_EventType_ERROR,
                                      "Error Unsubscribing from short-format Events,RC=0x%08X", (unsigned int)Status);
                }
            }

            HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;
            HS_AppData.EventMonLoaded       = HS_STATE_DISABLED;
        }
    }
    /*
    ** Otherwise, mark that the table is loaded (EventMon)
    */
    else
    {
        HS_AppData.EventMonLoaded = HS_STATE_ENABLED;
    }

    /*
    ** Release the table (MsgActs)
    */
    CFE_TBL_ReleaseAddress(HS_AppData.MATableHandle);

    /*
    ** Manage the table (MsgActs)
    */
    CFE_TBL_Manage(HS_AppData.MATableHandle);

    /*
    ** Get a pointer to the table (MsgActs)
    */
    Status = CFE_TBL_GetAddress((void *)&HS_AppData.MATablePtr, HS_AppData.MATableHandle);

    /*
    ** If there is a new table, refresh status (MsgActs)
    */
    if (Status == CFE_TBL_INFO_UPDATED)
    {
        HS_MsgActsStatusRefresh();
    }

    /*
    ** If Address acquisition fails report and disable (MsgActs)
    */
    if (Status < CFE_SUCCESS)
    {
        /*
        ** To prevent redundant reporting, only report if enabled (MsgActs)
        */
        if (HS_AppData.MsgActsState == HS_STATE_ENABLED)
        {
            CFE_EVS_SendEvent(HS_MSGACTS_GETADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Error getting MsgActs Table address, RC=0x%08X", (unsigned int)Status);
            HS_AppData.MsgActsState = HS_STATE_DISABLED;
        }
    }
    /*
    ** Otherwise, make sure it is enabled (MsgActs)
    */
    else
    {
        HS_AppData.MsgActsState = HS_STATE_ENABLED;
    }

#if HS_MAX_EXEC_CNT_SLOTS != 0
    /*
    ** Release the table (ExeCount)
    */
    CFE_TBL_ReleaseAddress(HS_AppData.XCTableHandle);

    /*
    ** Manage the table (ExeCount)
    */
    CFE_TBL_Manage(HS_AppData.XCTableHandle);

    /*
    ** Get a pointer to the table (ExeCount)
    */
    Status = CFE_TBL_GetAddress((void *)&HS_AppData.XCTablePtr, HS_AppData.XCTableHandle);

    /*
    ** If Address acquisition fails report and disable (ExeCount)
    */
    if (Status < CFE_SUCCESS)
    {
        /*
        ** To prevent redundant reporting, only report if enabled (ExeCount)
        */
        if (HS_AppData.ExeCountState == HS_STATE_ENABLED)
        {
            CFE_EVS_SendEvent(HS_EXECOUNT_GETADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Error getting ExeCount Table address, RC=0x%08X", (unsigned int)Status);
            HS_AppData.ExeCountState = HS_STATE_DISABLED;
        }
    }
    /*
    ** Otherwise, make sure it is enabled (ExeCount)
    */
    else
    {
        HS_AppData.ExeCountState = HS_STATE_ENABLED;
    }

#endif

    return;

} /* End of HS_AcquirePointers */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Refresh AppMon Status (on Table Update or Enable)               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_AppMonStatusRefresh(void)
{
    uint32 TableIndex  = 0;
    uint32 EnableIndex = 0;

    /*
    ** Clear all AppMon Enable bits
    */
    for (EnableIndex = 0; EnableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); EnableIndex++)
    {
        HS_AppData.AppMonEnables[EnableIndex] = 0;
    }

    /*
    ** Set AppMon enable bits and reset Countups and Exec Counter comparisons
    */
    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_APPS; TableIndex++)
    {
        HS_AppData.AppMonLastExeCount[TableIndex] = 0;

        if ((HS_AppData.AMTablePtr[TableIndex].CycleCount == 0) ||
            (HS_AppData.AMTablePtr[TableIndex].ActionType == HS_AMT_ACT_NOACT))
        {
            HS_AppData.AppMonCheckInCountdown[TableIndex] = 0;
        }
        else
        {
            HS_AppData.AppMonCheckInCountdown[TableIndex] = HS_AppData.AMTablePtr[TableIndex].CycleCount;
            CFE_SET((HS_AppData.AppMonEnables[TableIndex / HS_BITS_PER_APPMON_ENABLE]),
                    (TableIndex % HS_BITS_PER_APPMON_ENABLE));
        }
    }

    return;

} /* end HS_AppMonStatusRefresh */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Refresh MsgActs Status (on Table Update or Enable)              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_MsgActsStatusRefresh(void)
{
    uint32 TableIndex = 0;

    /*
    ** Clear all MsgActs Cooldowns
    */
    for (TableIndex = 0; TableIndex < HS_MAX_MSG_ACT_TYPES; TableIndex++)
    {
        HS_AppData.MsgActCooldown[TableIndex] = 0;
    }

    return;

} /* end HS_MsgActsStatusRefresh */

/************************/
/*  End of File Comment */
/************************/
