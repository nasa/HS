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
 *   CFS Health and Safety (HS) command handling routines
 */

/************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_cmds.h"
#include "hs_app_monitor.h"
#include "hs_event_monitor.h"
#include "hs_exec_monitor.h"
#include "hs_msgids.h"
#include "hs_eventids.h"
#include "hs_version.h"

/**
 * \brief Internal Macro to access the internal payload structure of a message
 *
 * This is done as a macro so it can be applied consistently to all
 * message processing functions, based on the way FM defines its messages.
 */
#define HS_GET_PAYLOAD(ptr, type) (&((type *)(ptr))->Payload)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Housekeeping request                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_SendHkCmd(const HS_SendHkCmd_t *BufPtr)
{
    CFE_SB_Buffer_t    *Buf;
    CFE_Status_t        Status;
    HS_HkTlm_Payload_t *PayloadPtr;

    /* start by getting a buffer to populate */
    Buf = CFE_SB_AllocateMessageBuffer(sizeof(HS_HkPacket_t));
    if (Buf == NULL)
    {
        Status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }
    else
    {
        CFE_MSG_Init(&Buf->Msg, CFE_SB_ValueToMsgId(HS_HK_TLM_MID), sizeof(HS_HkPacket_t));
        PayloadPtr = HS_GET_PAYLOAD(Buf, HS_HkPacket_t);

        /*
        ** Update HK variables
        */
        PayloadPtr->CmdCount              = HS_AppData.CmdCount;
        PayloadPtr->CmdErrCount           = HS_AppData.CmdErrCount;
        PayloadPtr->CurrentAppMonState    = HS_AppData.CurrentAppMonState;
        PayloadPtr->CurrentEventMonState  = HS_AppData.CurrentEventMonState;
        PayloadPtr->CurrentAlivenessState = HS_AppData.CurrentAlivenessState;
        PayloadPtr->CurrentCPUHogState    = HS_AppData.CurrentCPUHogState;
        PayloadPtr->ResetsPerformed       = HS_AppData.CDSData.ResetsPerformed;
        PayloadPtr->MaxResets             = HS_AppData.CDSData.MaxResets;
        PayloadPtr->EventsMonitoredCount  = HS_AppData.EventsMonitoredCount;
        PayloadPtr->MsgActExec            = HS_AppData.MsgActExec;
        PayloadPtr->UtilCpuAvg            = HS_AppData.UtilCpuAvg;
        PayloadPtr->UtilCpuPeak           = HS_AppData.UtilCpuPeak;
        PayloadPtr->InactiveEventMonCount = HS_AppData.InactiveEventMonCount;

        /*
        ** Build the HK status flags byte
        */
        HS_ComputeStatusFlags(&PayloadPtr->StatusFlags);

        /*
        ** Update the AppMon Enables
        */
        memset(PayloadPtr->AppMonEnables, 0, sizeof(PayloadPtr->AppMonEnables));
        HS_AppMon_ComputeEnableBits(PayloadPtr->AppMonEnables);

        /*
        ** Add the execution counters
        */
        HS_ExecMon_GetCounters(PayloadPtr->ExeCounts);

        /*
        ** Timestamp and send housekeeping packet
        */
        Status = CFE_SB_TransmitBuffer(Buf, true);
    }

    return Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Noop command                                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_NoopCmd(const HS_NoopCmd_t *BufPtr)
{
    HS_AppData.CmdCount++;

    CFE_EVS_SendEvent(HS_NOOP_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "No-op command: Version %d.%d.%d.%d",
                      HS_MAJOR_VERSION,
                      HS_MINOR_VERSION,
                      HS_REVISION,
                      HS_MISSION_REV);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset counters command                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_ResetCmd(const HS_ResetCmd_t *BufPtr)
{
    HS_AppData.CmdCount             = 0;
    HS_AppData.CmdErrCount          = 0;
    HS_AppData.EventsMonitoredCount = 0;
    HS_AppData.MsgActExec           = 0;

    CFE_EVS_SendEvent(HS_RESET_INF_EID, CFE_EVS_EventType_INFORMATION, "Reset counters command");
    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable applications monitor command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_EnableAppMonCmd(const HS_EnableAppMonCmd_t *BufPtr)
{
    HS_AppData.CmdCount++;

    if (HS_AppData.CurrentAppMonState == HS_State_ENABLED)
    {
        CFE_EVS_SendEvent(HS_ENABLE_APPMON_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "Application Monitoring is *already* Enabled");
    }
    else
    {
        HS_AppMon_StatusRefresh();
        HS_AppData.CurrentAppMonState = HS_State_ENABLED;
        CFE_EVS_SendEvent(HS_ENABLE_APPMON_INF_EID, CFE_EVS_EventType_INFORMATION, "Application Monitoring Enabled");
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable applications monitor command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_DisableAppMonCmd(const HS_DisableAppMonCmd_t *BufPtr)
{
    HS_AppData.CmdCount++;

    if (HS_AppData.CurrentAppMonState == HS_State_DISABLED)
    {
        CFE_EVS_SendEvent(HS_DISABLE_APPMON_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "Application Monitoring is *already* Disabled");
    }
    else
    {
        HS_AppData.CurrentAppMonState = HS_State_DISABLED;
        CFE_EVS_SendEvent(HS_DISABLE_APPMON_INF_EID, CFE_EVS_EventType_INFORMATION, "Application Monitoring Disabled");
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable events monitor command                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_EnableEventMonCmd(const HS_EnableEventMonCmd_t *BufPtr)
{
    CFE_Status_t Status = CFE_SUCCESS;

    if (HS_AppData.CurrentEventMonState == HS_State_ENABLED)
    {
        CFE_EVS_SendEvent(HS_ENABLE_EVENTMON_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "Event Monitoring is *already* Enabled");
    }
    else
    {
        Status = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(CFE_EVS_LONG_EVENT_MSG_MID),
                                    HS_AppData.EventPipe,
                                    CFE_SB_DEFAULT_QOS,
                                    HS_EVENT_PIPE_DEPTH);
        if (Status == CFE_SUCCESS)
        {
            Status = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(CFE_EVS_SHORT_EVENT_MSG_MID),
                                        HS_AppData.EventPipe,
                                        CFE_SB_DEFAULT_QOS,
                                        HS_EVENT_PIPE_DEPTH);

            if (Status == CFE_SUCCESS)
            {
                HS_AppData.CurrentEventMonState = HS_State_ENABLED;
                CFE_EVS_SendEvent(HS_ENABLE_EVENTMON_INF_EID,
                                  CFE_EVS_EventType_INFORMATION,
                                  "Event Monitoring Enabled");
            }
            else
            {
                CFE_EVS_SendEvent(HS_EVENTMON_SHORT_SUB_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "Event Monitor Enable: Error Subscribing to short-format Events,RC=0x%08X",
                                  (unsigned int)Status);
            }
        }
        else
        {
            CFE_EVS_SendEvent(HS_EVENTMON_LONG_SUB_EID,
                              CFE_EVS_EventType_ERROR,
                              "Event Monitor Enable: Error Subscribing to long-format Events,RC=0x%08X",
                              (unsigned int)Status);
        }
    }

    if (Status == CFE_SUCCESS)
    {
        HS_AppData.CmdCount++;
    }
    else
    {
        HS_AppData.CmdErrCount++;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable event monitor command                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_DisableEventMonCmd(const HS_DisableEventMonCmd_t *BufPtr)
{
    CFE_Status_t Status = CFE_SUCCESS;

    if (HS_AppData.CurrentEventMonState == HS_State_DISABLED)
    {
        CFE_EVS_SendEvent(HS_DISABLE_EVENTMON_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "Event Monitoring is *already* Disabled");
    }
    else
    {
        Status = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(CFE_EVS_LONG_EVENT_MSG_MID), HS_AppData.EventPipe);

        if (Status == CFE_SUCCESS)
        {
            Status = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(CFE_EVS_SHORT_EVENT_MSG_MID), HS_AppData.EventPipe);

            if (Status == CFE_SUCCESS)
            {
                HS_AppData.CurrentEventMonState = HS_State_DISABLED;
                CFE_EVS_SendEvent(HS_DISABLE_EVENTMON_INF_EID,
                                  CFE_EVS_EventType_INFORMATION,
                                  "Event Monitoring Disabled");
            }
            else
            {
                CFE_EVS_SendEvent(HS_EVENTMON_SHORT_UNSUB_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "Event Monitor Disable: Error Unsubscribing from short-format Events,RC=0x%08X",
                                  (unsigned int)Status);
            }
        }
        else
        {
            CFE_EVS_SendEvent(HS_EVENTMON_LONG_UNSUB_EID,
                              CFE_EVS_EventType_ERROR,
                              "Event Monitor Disable: Error Unsubscribing from long-format Events,RC=0x%08X",
                              (unsigned int)Status);
        }
    }

    if (Status == CFE_SUCCESS)
    {
        HS_AppData.CmdCount++;
    }
    else
    {
        HS_AppData.CmdErrCount++;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable aliveness indicator command                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_EnableAlivenessCmd(const HS_EnableAlivenessCmd_t *BufPtr)
{
    HS_AppData.CmdCount++;

    if (HS_AppData.CurrentAlivenessState == HS_State_ENABLED)
    {
        CFE_EVS_SendEvent(HS_ENABLE_ALIVENESS_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "Aliveness Indicator is *already* Enabled");
    }
    else
    {
        HS_AppData.CurrentAlivenessState = HS_State_ENABLED;
        CFE_EVS_SendEvent(HS_ENABLE_ALIVENESS_INF_EID, CFE_EVS_EventType_INFORMATION, "Aliveness Indicator Enabled");
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable aliveness indicator command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_DisableAlivenessCmd(const HS_DisableAlivenessCmd_t *BufPtr)
{
    HS_AppData.CmdCount++;

    if (HS_AppData.CurrentAlivenessState == HS_State_DISABLED)
    {
        CFE_EVS_SendEvent(HS_DISABLE_ALIVENESS_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "Aliveness Indicator is *already* Disabled");
    }
    else
    {
        HS_AppData.CurrentAlivenessState = HS_State_DISABLED;
        CFE_EVS_SendEvent(HS_DISABLE_ALIVENESS_INF_EID, CFE_EVS_EventType_INFORMATION, "Aliveness Indicator Disabled");
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable cpu hogging indicator command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_EnableCpuHogCmd(const HS_EnableCpuHogCmd_t *BufPtr)
{
    HS_AppData.CmdCount++;

    if (HS_AppData.CurrentCPUHogState == HS_State_ENABLED)
    {
        CFE_EVS_SendEvent(HS_ENABLE_CPUHOG_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "CPU Hogging Indicator is *already* Enabled");
    }
    else
    {
        HS_AppData.CurrentCPUHogState = HS_State_ENABLED;
        CFE_EVS_SendEvent(HS_ENABLE_CPUHOG_INF_EID, CFE_EVS_EventType_INFORMATION, "CPU Hogging Indicator Enabled");
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable cpu hogging indicator command                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_DisableCpuHogCmd(const HS_DisableCpuHogCmd_t *BufPtr)
{
    HS_AppData.CmdCount++;

    if (HS_AppData.CurrentCPUHogState == HS_State_DISABLED)
    {
        CFE_EVS_SendEvent(HS_DISABLE_CPUHOG_INF_EID,
                          CFE_EVS_EventType_INFORMATION,
                          "CPU Hogging Indicator is *already* Disabled");
    }
    else
    {
        HS_AppData.CurrentCPUHogState = HS_State_DISABLED;
        CFE_EVS_SendEvent(HS_DISABLE_CPUHOG_INF_EID, CFE_EVS_EventType_INFORMATION, "CPU Hogging Indicator Disabled");
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset processor resets performed count command                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_ResetResetsPerformedCmd(const HS_ResetResetsPerformedCmd_t *BufPtr)
{
    HS_AppData.CmdCount++;
    HS_SetCDSData(0, HS_AppData.CDSData.MaxResets);
    CFE_EVS_SendEvent(HS_RESET_RESETS_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "Processor Resets Performed by HS Counter has been Reset");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set max processor resets command                                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_SetMaxResetsCmd(const HS_SetMaxResetsCmd_t *BufPtr)
{
    const HS_SetMaxResets_Payload_t *CmdPtr;

    HS_AppData.CmdCount++;
    CmdPtr = HS_GET_PAYLOAD(BufPtr, const HS_SetMaxResetsCmd_t);

    HS_SetCDSData(HS_AppData.CDSData.ResetsPerformed, CmdPtr->MaxResets);

    CFE_EVS_SendEvent(HS_SET_MAX_RESETS_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "Max Resets Performable by HS has been set to %d",
                      HS_AppData.CDSData.MaxResets);

    return CFE_SUCCESS;
}
