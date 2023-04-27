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
#include "hs_monitors.h"
#include "hs_msgids.h"
#include "hs_events.h"
#include "hs_utils.h"
#include "hs_version.h"

/**
 * \brief Internal Macro to access the internal payload structure of a message
 *
 * This is done as a macro so it can be applied consistently to all
 * message processing functions, based on the way FM defines its messages.
 */
#define HS_GET_CMD_PAYLOAD(ptr, type) (&((const type *)(ptr))->Payload)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Housekeeping request                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_SendHkCmd(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_ES_AppId_t AppId = CFE_ES_APPID_UNDEFINED;

    uint32             ExeCount;
    CFE_ES_TaskId_t    TaskId;
    CFE_ES_CounterId_t CounterId;
    CFE_ES_TaskInfo_t  TaskInfo;
    int32              Status;
    uint32             TableIndex;

    HS_HkTlm_Payload_t *PayloadPtr;

    memset(&TaskInfo, 0, sizeof(TaskInfo));

    PayloadPtr = &HS_AppData.HkPacket.Payload;

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

    /*
    ** Calculate the current number of invalid event monitor entries
    */
    PayloadPtr->InvalidEventMonCount = 0;

    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_EVENTS; TableIndex++)
    {
        if (HS_AppData.EMTablePtr[TableIndex].ActionType != HS_EMT_ACT_NOACT)
        {
            Status = CFE_ES_GetAppIDByName(&AppId, HS_AppData.EMTablePtr[TableIndex].AppName);

            if (Status != CFE_SUCCESS)
            {
                PayloadPtr->InvalidEventMonCount++;
            }
        }
    }

    /*
    ** Build the HK status flags byte
    */
    PayloadPtr->StatusFlags = 0;
    if (HS_AppData.ExeCountState == HS_STATE_ENABLED)
    {
        PayloadPtr->StatusFlags |= HS_LOADED_XCT;
    }
    if (HS_AppData.MsgActsState == HS_STATE_ENABLED)
    {
        PayloadPtr->StatusFlags |= HS_LOADED_MAT;
    }
    if (HS_AppData.AppMonLoaded == HS_STATE_ENABLED)
    {
        PayloadPtr->StatusFlags |= HS_LOADED_AMT;
    }
    if (HS_AppData.EventMonLoaded == HS_STATE_ENABLED)
    {
        PayloadPtr->StatusFlags |= HS_LOADED_EMT;
    }
    if (HS_AppData.CDSState == HS_STATE_ENABLED)
    {
        PayloadPtr->StatusFlags |= HS_CDS_IN_USE;
    }

    /*
    ** Update the AppMon Enables
    */
    for (TableIndex = 0; TableIndex <= ((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE); TableIndex++)
    {
        PayloadPtr->AppMonEnables[TableIndex] = HS_AppData.AppMonEnables[TableIndex];
    }

    PayloadPtr->UtilCpuAvg  = HS_AppData.UtilCpuAvg;
    PayloadPtr->UtilCpuPeak = HS_AppData.UtilCpuPeak;

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
                    Status = CFE_ES_GetGenCounterIDByName(&CounterId, HS_AppData.XCTablePtr[TableIndex].ResourceName);

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

        PayloadPtr->ExeCounts[TableIndex] = ExeCount;
    }

    /*
    ** Timestamp and send housekeeping packet
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(HS_AppData.HkPacket.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(HS_AppData.HkPacket.TelemetryHeader), true);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Noop command                                                    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_NoopCmd(const CFE_SB_Buffer_t *BufPtr)
{
    HS_AppData.CmdCount++;

    CFE_EVS_SendEvent(HS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION, "No-op command: Version %d.%d.%d.%d",
                      HS_MAJOR_VERSION, HS_MINOR_VERSION, HS_REVISION, HS_MISSION_REV);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset counters command                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_ResetCmd(const CFE_SB_Buffer_t *BufPtr)
{
    HS_ResetCounters();

    CFE_EVS_SendEvent(HS_RESET_DBG_EID, CFE_EVS_EventType_DEBUG, "Reset counters command");
}

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
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable applications monitor command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableAppMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    HS_AppData.CmdCount++;
    HS_AppMonStatusRefresh();
    HS_AppData.CurrentAppMonState = HS_STATE_ENABLED;
    CFE_EVS_SendEvent(HS_ENABLE_APPMON_DBG_EID, CFE_EVS_EventType_DEBUG, "Application Monitoring Enabled");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable applications monitor command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableAppMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    HS_AppData.CmdCount++;
    HS_AppData.CurrentAppMonState = HS_STATE_DISABLED;
    CFE_EVS_SendEvent(HS_DISABLE_APPMON_DBG_EID, CFE_EVS_EventType_DEBUG, "Application Monitoring Disabled");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable events monitor command                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableEventMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    CFE_Status_t Status = CFE_SUCCESS;

    /*
    ** Subscribe to Event Messages if currently disabled
    */
    if (HS_AppData.CurrentEventMonState == HS_STATE_DISABLED)
    {
        Status = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(CFE_EVS_LONG_EVENT_MSG_MID), HS_AppData.EventPipe,
                                    CFE_SB_DEFAULT_QOS, HS_EVENT_PIPE_DEPTH);
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
        else
        {
            CFE_EVS_SendEvent(HS_EVENTMON_LONG_SUB_EID, CFE_EVS_EventType_ERROR,
                              "Event Monitor Enable: Error Subscribing to long-format Events,RC=0x%08X",
                              (unsigned int)Status);
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable event monitor command                                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableEventMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    int32 Status = CFE_SUCCESS;

    /*
    ** Unsubscribe from Event Messages if currently enabled
    */
    if (HS_AppData.CurrentEventMonState == HS_STATE_ENABLED)
    {
        Status = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(CFE_EVS_LONG_EVENT_MSG_MID), HS_AppData.EventPipe);

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
        else
        {
            CFE_EVS_SendEvent(HS_EVENTMON_LONG_UNSUB_EID, CFE_EVS_EventType_ERROR,
                              "Event Monitor Disable: Error Unsubscribing from long-format Events,RC=0x%08X",
                              (unsigned int)Status);
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable aliveness indicator command                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableAlivenessCmd(const CFE_SB_Buffer_t *BufPtr)
{
    HS_AppData.CmdCount++;
    HS_AppData.CurrentAlivenessState = HS_STATE_ENABLED;
    CFE_EVS_SendEvent(HS_ENABLE_ALIVENESS_DBG_EID, CFE_EVS_EventType_DEBUG, "Aliveness Indicator Enabled");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable aliveness indicator command                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableAlivenessCmd(const CFE_SB_Buffer_t *BufPtr)
{
    HS_AppData.CmdCount++;
    HS_AppData.CurrentAlivenessState = HS_STATE_DISABLED;
    CFE_EVS_SendEvent(HS_DISABLE_ALIVENESS_DBG_EID, CFE_EVS_EventType_DEBUG, "Aliveness Indicator Disabled");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Enable cpu hogging indicator command                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_EnableCpuHogCmd(const CFE_SB_Buffer_t *BufPtr)
{
    HS_AppData.CmdCount++;
    HS_AppData.CurrentCPUHogState = HS_STATE_ENABLED;
    CFE_EVS_SendEvent(HS_ENABLE_CPUHOG_DBG_EID, CFE_EVS_EventType_DEBUG, "CPU Hogging Indicator Enabled");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Disable cpu hogging indicator command                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_DisableCpuHogCmd(const CFE_SB_Buffer_t *BufPtr)
{
    HS_AppData.CmdCount++;
    HS_AppData.CurrentCPUHogState = HS_STATE_DISABLED;
    CFE_EVS_SendEvent(HS_DISABLE_CPUHOG_DBG_EID, CFE_EVS_EventType_DEBUG, "CPU Hogging Indicator Disabled");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Reset processor resets performed count command                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_ResetResetsPerformedCmd(const CFE_SB_Buffer_t *BufPtr)
{
    HS_AppData.CmdCount++;
    HS_SetCDSData(0, HS_AppData.CDSData.MaxResets);
    CFE_EVS_SendEvent(HS_RESET_RESETS_DBG_EID, CFE_EVS_EventType_DEBUG,
                      "Processor Resets Performed by HS Counter has been Reset");
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set max processor resets command                                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_SetMaxResetsCmd(const CFE_SB_Buffer_t *BufPtr)
{
    const HS_SetMaxResets_Payload_t *CmdPtr;

    HS_AppData.CmdCount++;
    CmdPtr = HS_GET_CMD_PAYLOAD(BufPtr, HS_SetMaxResetsCmd_t);

    HS_SetCDSData(HS_AppData.CDSData.ResetsPerformed, CmdPtr->MaxResets);

    CFE_EVS_SendEvent(HS_SET_MAX_RESETS_DBG_EID, CFE_EVS_EventType_DEBUG,
                      "Max Resets Performable by HS has been set to %d", HS_AppData.CDSData.MaxResets);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Acquire table pointers                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_AcquirePointers(void)
{
    int32 Status;

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
}

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
            HS_AppData.AppMonEnables[TableIndex / HS_BITS_PER_APPMON_ENABLE] |=
                (1 << (TableIndex % HS_BITS_PER_APPMON_ENABLE));
        }
    }
}

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
}
