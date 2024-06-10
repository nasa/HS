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
 *   Functions used for CFS Health and Safety Monitors for Applications
 *   and Events
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_monitors.h"
#include "hs_sysmon.h"
#include "hs_tbldefs.h"
#include "hs_eventids.h"
#include "hs_utils.h"
#include "cfe_evs_msg.h"

#include <string.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Monitor Applications                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_MonitorApplications(void)
{
    CFE_ES_AppInfo_t AppInfo;
    CFE_ES_AppId_t   AppId = CFE_ES_APPID_UNDEFINED;
    CFE_Status_t     Status;
    uint32           TableIndex = 0;
    uint16           ActionType;
    uint32           MsgActsIndex = 0;

    memset(&AppInfo, 0, sizeof(AppInfo));

    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_APPS; TableIndex++)
    {
        ActionType = HS_AppData.AMTablePtr[TableIndex].ActionType;

        /*
        ** Check this App if it has an action, and hasn't already expired
        */
        if ((ActionType != HS_AMT_ACT_NOACT) && (HS_AppData.AppMonCheckInCountdown[TableIndex] != 0))
        {
            Status = CFE_ES_GetAppIDByName(&AppId, HS_AppData.AMTablePtr[TableIndex].AppName);

            if (Status == CFE_SUCCESS)
            {
                Status = CFE_ES_GetAppInfo(&AppInfo, AppId);
            }
            else if (HS_AppData.AppMonCheckInCountdown[TableIndex] == HS_AppData.AMTablePtr[TableIndex].CycleCount)
            {
                /*
                ** Only send an error event the first time the App fails to resolve
                */
                CFE_EVS_SendEvent(HS_APPMON_APPNAME_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "App Monitor App Name not found: APP:(%s)",
                                  HS_AppData.AMTablePtr[TableIndex].AppName);
            }
            else
            {
                /* For repeated errors, send a debug event */
                CFE_EVS_SendEvent(HS_APPMON_APPNAME_DBG_EID, CFE_EVS_EventType_DEBUG,
                                  "App Monitor App Name not found: APP:(%s)",
                                  HS_AppData.AMTablePtr[TableIndex].AppName);
            }

            /*
            ** Failure to get an execution counter is not considered an automatic failure (or eventworthy)
            */
            if ((Status == CFE_SUCCESS) && (HS_AppData.AppMonLastExeCount[TableIndex] != AppInfo.ExecutionCounter))
            {
                /*
                ** Set the current count, and reset the timeout
                */
                HS_AppData.AppMonCheckInCountdown[TableIndex] = HS_AppData.AMTablePtr[TableIndex].CycleCount;
                HS_AppData.AppMonLastExeCount[TableIndex]     = AppInfo.ExecutionCounter;
            }
            else
            {
                HS_AppData.AppMonCheckInCountdown[TableIndex]--;

                /*
                ** Take Action once the counter reaches zero
                */
                if (HS_AppData.AppMonCheckInCountdown[TableIndex] == 0)
                {
                    /*
                    ** Unset the enabled bit flag
                    */
                    HS_AppData.AppMonEnables[TableIndex / HS_BITS_PER_APPMON_ENABLE] &=
                        ~(1 << (TableIndex % HS_BITS_PER_APPMON_ENABLE));
                    switch (ActionType)
                    {
                        case HS_AMT_ACT_PROC_RESET:
                            CFE_EVS_SendEvent(HS_APPMON_PROC_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "App Monitor Failure: APP:(%s): Action: Processor Reset",
                                              HS_AppData.AMTablePtr[TableIndex].AppName);

                            /*
                            ** Perform a reset if we can
                            */
                            if (HS_AppData.CDSData.ResetsPerformed < HS_AppData.CDSData.MaxResets)
                            {
                                HS_SetCDSData((HS_AppData.CDSData.ResetsPerformed + 1), HS_AppData.CDSData.MaxResets);

                                OS_TaskDelay(HS_RESET_TASK_DELAY);
                                CFE_ES_WriteToSysLog("HS App: App Monitor Failure: APP:(%s): Action: Processor Reset\n",
                                                     HS_AppData.AMTablePtr[TableIndex].AppName);
                                HS_AppData.ServiceWatchdogFlag = HS_STATE_DISABLED;
                                CFE_ES_ResetCFE(CFE_PSP_RST_TYPE_PROCESSOR);
                            }
                            else
                            {
                                CFE_EVS_SendEvent(HS_RESET_LIMIT_ERR_EID, CFE_EVS_EventType_ERROR,
                                                  "Processor Reset Action Limit Reached: No Reset Performed");
                            }

                            break;

                        case HS_AMT_ACT_APP_RESTART:
                            CFE_EVS_SendEvent(HS_APPMON_RESTART_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "App Monitor Failure: APP:(%s) Action: Restart Application",
                                              HS_AppData.AMTablePtr[TableIndex].AppName);
                            /*
                            ** Attempt to restart the App if we resolved the AppId
                            */
                            if (Status == CFE_SUCCESS)
                            {
                                Status = CFE_ES_RestartApp(AppId);
                            }

                            /*
                            ** Report an error; either no valid AppId, or RestartApp failed
                            */
                            if (Status != CFE_SUCCESS)
                            {
                                CFE_EVS_SendEvent(HS_APPMON_NOT_RESTARTED_ERR_EID, CFE_EVS_EventType_ERROR,
                                                  "Call to Restart App Failed: APP:(%s) ERR: 0x%08X",
                                                  HS_AppData.AMTablePtr[TableIndex].AppName, (unsigned int)Status);
                            }

                            break;

                        case HS_AMT_ACT_EVENT:
                            CFE_EVS_SendEvent(HS_APPMON_FAIL_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "App Monitor Failure: APP:(%s): Action: Event Only",
                                              HS_AppData.AMTablePtr[TableIndex].AppName);
                            break;

                        /*
                        ** Message Action types processing (invalid will be skipped)
                        */
                        default:

                            /* Calculate the requested message action index */
                            MsgActsIndex = ActionType - HS_AMT_ACT_LAST_NONMSG - 1;

                            /*
                            ** Check to see if this is a valid Message Action Type
                            */
                            if ((HS_AppData.MsgActsState == HS_STATE_ENABLED) && (MsgActsIndex < HS_MAX_MSG_ACT_TYPES))
                            {
                                /*
                                ** Send the message if off cooldown and not disabled
                                */
                                if ((HS_AppData.MsgActCooldown[MsgActsIndex] == 0) &&
                                    (HS_AppData.MATablePtr[MsgActsIndex].EnableState != HS_MAT_STATE_DISABLED))
                                {
                                    CFE_SB_TransmitMsg(
                                        (const CFE_MSG_Message_t *)&HS_AppData.MATablePtr[MsgActsIndex].MsgBuf, true);
                                    HS_AppData.MsgActExec++;
                                    HS_AppData.MsgActCooldown[MsgActsIndex] =
                                        HS_AppData.MATablePtr[MsgActsIndex].Cooldown;
                                    if (HS_AppData.MATablePtr[MsgActsIndex].EnableState != HS_MAT_STATE_NOEVENT)
                                    {
                                        CFE_EVS_SendEvent(
                                            HS_APPMON_MSGACTS_ERR_EID, CFE_EVS_EventType_ERROR,
                                            "App Monitor Failure: APP:(%s): Action: Message Action Index: %d",
                                            HS_AppData.AMTablePtr[TableIndex].AppName, (int)MsgActsIndex);
                                    }
                                }
                            }

                            /* Otherwise, Take No Action */
                            break;
                    } /* end switch */

                } /* end (HS_AppData.AppMonCheckInCountdown[TableIndex] == 0) if */

            } /* end "failed to update counter" else */

        } /* end (HS_AppData.AppMonCheckInCountdown[TableIndex] != 0) if */

    } /* end for loop */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Monitor Events                                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_MonitorEvent(const CFE_EVS_LongEventTlm_t *EventPtr)
{
    uint32           TableIndex = 0;
    CFE_Status_t     Status     = CFE_SUCCESS;
    CFE_ES_AppId_t   AppId      = CFE_ES_APPID_UNDEFINED;
    uint16           ActionType;
    uint32           MsgActsIndex = 0;
    CFE_SB_Buffer_t *SendPtr      = NULL;

    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_EVENTS; TableIndex++)
    {
        ActionType = HS_AppData.EMTablePtr[TableIndex].ActionType;

        /*
        ** Check this Event Monitor if it has an action, and the event IDs match
        */
        if ((ActionType != HS_EMT_ACT_NOACT) &&
            (HS_AppData.EMTablePtr[TableIndex].EventID == EventPtr->Payload.PacketID.EventID))
        {
            if (strncmp(HS_AppData.EMTablePtr[TableIndex].AppName, EventPtr->Payload.PacketID.AppName,
                        OS_MAX_API_NAME) == 0)
            {
                /*
                ** Perform the action if the strings also match
                */
                switch (ActionType)
                {
                    case HS_EMT_ACT_PROC_RESET:
                        CFE_EVS_SendEvent(HS_EVENTMON_PROC_ERR_EID, CFE_EVS_EventType_ERROR,
                                          "Event Monitor: APP:(%s) EID:(%d): Action: Processor Reset",
                                          HS_AppData.EMTablePtr[TableIndex].AppName,
                                          HS_AppData.EMTablePtr[TableIndex].EventID);

                        /*
                        ** Perform a reset if we can
                        */
                        if (HS_AppData.CDSData.ResetsPerformed < HS_AppData.CDSData.MaxResets)
                        {
                            HS_SetCDSData((HS_AppData.CDSData.ResetsPerformed + 1), HS_AppData.CDSData.MaxResets);

                            OS_TaskDelay(HS_RESET_TASK_DELAY);
                            CFE_ES_WriteToSysLog("HS App: Event Monitor: APP:(%s) EID:(%d): Action: Processor Reset\n",
                                                 HS_AppData.EMTablePtr[TableIndex].AppName,
                                                 (int)HS_AppData.EMTablePtr[TableIndex].EventID);
                            HS_AppData.ServiceWatchdogFlag = HS_STATE_DISABLED;
                            CFE_ES_ResetCFE(CFE_PSP_RST_TYPE_PROCESSOR);
                        }
                        else
                        {
                            CFE_EVS_SendEvent(HS_RESET_LIMIT_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "Processor Reset Action Limit Reached: No Reset Performed");
                        }

                        break;

                    case HS_EMT_ACT_APP_RESTART:
                        /*
                        ** Check to see if the App is still there, and try to restart if it is
                        */
                        Status = CFE_ES_GetAppIDByName(&AppId, HS_AppData.EMTablePtr[TableIndex].AppName);
                        if (Status == CFE_SUCCESS)
                        {
                            CFE_EVS_SendEvent(HS_EVENTMON_RESTART_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "Event Monitor: APP:(%s) EID:(%d): Action: Restart Application",
                                              HS_AppData.EMTablePtr[TableIndex].AppName,
                                              HS_AppData.EMTablePtr[TableIndex].EventID);
                            Status = CFE_ES_RestartApp(AppId);
                        }

                        if (Status != CFE_SUCCESS)
                        {
                            CFE_EVS_SendEvent(HS_EVENTMON_NOT_RESTARTED_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "Call to Restart App Failed: APP:(%s) ERR: 0x%08X",
                                              HS_AppData.EMTablePtr[TableIndex].AppName, (unsigned int)Status);
                        }

                        break;

                    case HS_EMT_ACT_APP_DELETE:
                        /*
                        ** Check to see if the App is still there, and try to delete if it is
                        */
                        Status = CFE_ES_GetAppIDByName(&AppId, HS_AppData.EMTablePtr[TableIndex].AppName);
                        if (Status == CFE_SUCCESS)
                        {
                            CFE_EVS_SendEvent(HS_EVENTMON_DELETE_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "Event Monitor: APP:(%s) EID:(%d): Action: Delete Application",
                                              HS_AppData.EMTablePtr[TableIndex].AppName,
                                              HS_AppData.EMTablePtr[TableIndex].EventID);
                            Status = CFE_ES_DeleteApp(AppId);
                        }

                        if (Status != CFE_SUCCESS)
                        {
                            CFE_EVS_SendEvent(HS_EVENTMON_NOT_DELETED_ERR_EID, CFE_EVS_EventType_ERROR,
                                              "Call to Delete App Failed: APP:(%s) ERR: 0x%08X",
                                              HS_AppData.EMTablePtr[TableIndex].AppName, (unsigned int)Status);
                        }

                        break;

                    /*
                    ** Message Action types processing (invalid will be skipped)
                    */
                    default:

                        /* Calculate the requested message action index */
                        MsgActsIndex = ActionType - HS_EMT_ACT_LAST_NONMSG - 1;

                        /*
                        ** Check to see if this is a valid Message Action Type
                        */
                        if ((HS_AppData.MsgActsState == HS_STATE_ENABLED) && (MsgActsIndex < HS_MAX_MSG_ACT_TYPES))
                        {
                            /*
                            ** Send the message if off cooldown and not disabled
                            */
                            if ((HS_AppData.MsgActCooldown[MsgActsIndex] == 0) &&
                                (HS_AppData.MATablePtr[MsgActsIndex].EnableState != HS_MAT_STATE_DISABLED))
                            {
                                SendPtr = (CFE_SB_Buffer_t *)&HS_AppData.MATablePtr[MsgActsIndex].MsgBuf;
                                CFE_SB_TransmitMsg(&SendPtr->Msg, true);

                                HS_AppData.MsgActExec++;
                                HS_AppData.MsgActCooldown[MsgActsIndex] = HS_AppData.MATablePtr[MsgActsIndex].Cooldown;
                                if (HS_AppData.MATablePtr[MsgActsIndex].EnableState != HS_MAT_STATE_NOEVENT)
                                {
                                    CFE_EVS_SendEvent(
                                        HS_EVENTMON_MSGACTS_ERR_EID, CFE_EVS_EventType_ERROR,
                                        "Event Monitor: APP:(%s) EID:(%d): Action: Message Action Index: %d",
                                        HS_AppData.EMTablePtr[TableIndex].AppName,
                                        HS_AppData.EMTablePtr[TableIndex].EventID, (int)MsgActsIndex);
                                }
                            }
                        }

                        /* Otherwise, Take No Action */
                        break;
                } /* end switch */

            } /* end AppName comparison */

        } /* end EventID comparison */

    } /* end for loop */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Monitor CPU Utilization and Hogging                             */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_MonitorUtilization(void)
{
    int32  CurrentUtil;
    uint32 UtilIndex;
    uint32 CombinedUtil  = 0;
    uint32 PeakUtil      = 0;
    uint32 ThisUtilIndex = HS_AppData.CurrentCPUUtilIndex;

    HS_AppData.CurrentCPUUtilIndex++;

    if (HS_AppData.CurrentCPUUtilIndex >= HS_UTIL_PEAK_NUM_INTERVAL)
    {
        HS_AppData.CurrentCPUUtilIndex = 0;
    }

    CurrentUtil = HS_SysMonGetCpuUtilization();

    if (CurrentUtil < 0)
    {
        /* CPU utilization not known, report unique value that indicates this */
        HS_AppData.UtilCpuAvg  = 0xFFFFFFFF;
        HS_AppData.UtilCpuPeak = 0xFFFFFFFF;
    }
    else
    {
        if (CurrentUtil > HS_CPU_UTILIZATION_MAX)
        {
            CurrentUtil = HS_CPU_UTILIZATION_MAX;
        }

        if ((CurrentUtil >= HS_UTIL_PER_INTERVAL_HOGGING) && (HS_AppData.CurrentCPUHogState == HS_STATE_ENABLED))
        {
            HS_AppData.CurrentCPUHoggingTime++;

            if (HS_AppData.CurrentCPUHoggingTime == HS_AppData.MaxCPUHoggingTime)
            {
                CFE_EVS_SendEvent(HS_CPUMON_HOGGING_ERR_EID, CFE_EVS_EventType_ERROR, "CPU Hogging Detected");
                CFE_ES_WriteToSysLog("HS App: CPU Hogging Detected\n");
            }
        }
        else
        {
            HS_AppData.CurrentCPUHoggingTime = 0;
        }

        /* SAD: The wrap around logic prior to index access, ensures the index stays within the bounds of the buffer */
        HS_AppData.UtilizationTracker[ThisUtilIndex] = CurrentUtil;

        for (UtilIndex = 0; UtilIndex < HS_UTIL_PEAK_NUM_INTERVAL; UtilIndex++)
        {
            if (HS_AppData.UtilizationTracker[UtilIndex] > PeakUtil)
            {
                PeakUtil = HS_AppData.UtilizationTracker[UtilIndex];
            }

            if (ThisUtilIndex >= HS_UTIL_AVERAGE_NUM_INTERVAL)
            {
                if ((UtilIndex > (ThisUtilIndex - HS_UTIL_AVERAGE_NUM_INTERVAL)) && (UtilIndex <= ThisUtilIndex))
                {
                    CombinedUtil += HS_AppData.UtilizationTracker[UtilIndex];
                }
            }
            else
            {
                if ((UtilIndex <= ThisUtilIndex) ||
                    (UtilIndex > (HS_UTIL_PEAK_NUM_INTERVAL - (HS_UTIL_AVERAGE_NUM_INTERVAL - ThisUtilIndex))))
                {
                    CombinedUtil += HS_AppData.UtilizationTracker[UtilIndex];
                }
            }
        }

        HS_AppData.UtilCpuAvg  = (CombinedUtil / HS_UTIL_AVERAGE_NUM_INTERVAL);
        HS_AppData.UtilCpuPeak = PeakUtil;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Validate the Application Monitor Table                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 HS_ValidateAMTable(void *TableData)
{
    HS_AMTEntry_t *TableArray = (HS_AMTEntry_t *)TableData;

    int32  TableResult = CFE_SUCCESS;
    uint32 TableIndex  = 0;
    int32  EntryResult;

    uint16 ActionType;
    uint16 CycleCount;
    uint16 NullTerm;

    uint32 GoodCount                = 0;
    uint32 BadCount                 = 0;
    uint32 UnusedCount              = 0;
    char   BadName[OS_MAX_API_NAME] = "";

    if (TableData == NULL)
    {
        CFE_EVS_SendEvent(HS_AM_TBL_NULL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error in AM Table Validation. Table is null.");

        return HS_TBL_VAL_ERR;
    }

    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_APPS; TableIndex++)
    {
        ActionType  = TableArray[TableIndex].ActionType;
        CycleCount  = TableArray[TableIndex].CycleCount;
        NullTerm    = TableArray[TableIndex].NullTerm;
        EntryResult = HS_AMTVAL_NO_ERR;

        if ((CycleCount == 0) || (ActionType == HS_AMT_ACT_NOACT))
        {
            /*
            ** Unused table entry
            */
            UnusedCount++;
        }
        else if (NullTerm != 0)
        {
            /*
            ** Null Terminator Safety Buffer is not Null
            */
            EntryResult = HS_AMTVAL_ERR_NUL;
            BadCount++;
        }
        else if (!HS_AMTActionIsValid(ActionType))
        {
            /*
            ** Action Type is not valid
            */
            EntryResult = HS_AMTVAL_ERR_ACT;
            BadCount++;
        }
        else
        {
            /*
            ** Otherwise, this entry is good
            */
            GoodCount++;
        }
        /*
        ** Generate detailed event for "first" error
        */
        if ((EntryResult != HS_AMTVAL_NO_ERR) && (TableResult == CFE_SUCCESS))
        {
            strncpy(BadName, TableArray[TableIndex].AppName, OS_MAX_API_NAME - 1);
            BadName[OS_MAX_API_NAME - 1] = '\0';
            CFE_EVS_SendEvent(HS_AMTVAL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "AppMon verify err: Entry = %d, Err = %d, Action = %d, App = %s", (int)TableIndex,
                              (int)EntryResult, ActionType, BadName);
            TableResult = EntryResult;
        }
    }

    /*
    ** Generate informational event with error totals
    */
    CFE_EVS_SendEvent(HS_AMTVAL_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "AppMon verify results: good = %d, bad = %d, unused = %d", (int)GoodCount, (int)BadCount,
                      (int)UnusedCount);

    return TableResult;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Validate the Event Monitor Table                                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 HS_ValidateEMTable(void *TableData)
{
    HS_EMTEntry_t *TableArray = (HS_EMTEntry_t *)TableData;

    int32  TableResult = CFE_SUCCESS;
    uint32 TableIndex  = 0;
    int32  EntryResult;

    uint16 ActionType;
    uint16 EventID;
    uint16 NullTerm;

    uint32 GoodCount                = 0;
    uint32 BadCount                 = 0;
    uint32 UnusedCount              = 0;
    char   BadName[OS_MAX_API_NAME] = "";

    if (TableData == NULL)
    {
        CFE_EVS_SendEvent(HS_EM_TBL_NULL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error in EM Table Validation. Table is null.");

        return HS_TBL_VAL_ERR;
    }

    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_EVENTS; TableIndex++)
    {
        ActionType  = TableArray[TableIndex].ActionType;
        EventID     = TableArray[TableIndex].EventID;
        NullTerm    = TableArray[TableIndex].NullTerm;
        EntryResult = HS_EMTVAL_NO_ERR;

        if ((EventID == 0) || (ActionType == HS_EMT_ACT_NOACT))
        {
            /*
            ** Unused table entry
            */
            UnusedCount++;
        }
        else if (NullTerm != 0)
        {
            /*
            ** Null Terminator Safety Buffer is not Null
            */
            EntryResult = HS_EMTVAL_ERR_NUL;
            BadCount++;
        }
        else if (!HS_EMTActionIsValid(ActionType))
        {
            /*
            ** Action Type is not valid
            */
            EntryResult = HS_EMTVAL_ERR_ACT;
            BadCount++;
        }
        else
        {
            /*
            ** Otherwise, this entry is good
            */
            GoodCount++;
        }
        /*
        ** Generate detailed event for "first" error
        */
        if ((EntryResult != HS_EMTVAL_NO_ERR) && (TableResult == CFE_SUCCESS))
        {
            strncpy(BadName, TableArray[TableIndex].AppName, OS_MAX_API_NAME - 1);
            BadName[OS_MAX_API_NAME - 1] = '\0';
            CFE_EVS_SendEvent(HS_EMTVAL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "EventMon verify err: Entry = %d, Err = %d, Action = %d, ID = %d App = %s",
                              (int)TableIndex, (int)EntryResult, ActionType, EventID, BadName);
            TableResult = EntryResult;
        }
    }

    /*
    ** Generate informational event with error totals
    */
    CFE_EVS_SendEvent(HS_EMTVAL_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "EventMon verify results: good = %d, bad = %d, unused = %d", (int)GoodCount, (int)BadCount,
                      (int)UnusedCount);

    return TableResult;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Validate the Execution Counters Table                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 HS_ValidateXCTable(void *TableData)
{
    HS_XCTEntry_t *TableArray = (HS_XCTEntry_t *)TableData;

    int32  TableResult = CFE_SUCCESS;
    uint32 TableIndex;
    int32  EntryResult;
    uint16 ResourceType;
    uint32 NullTerm;

    uint32 GoodCount                = 0;
    uint32 BadCount                 = 0;
    uint32 UnusedCount              = 0;
    char   BadName[OS_MAX_API_NAME] = "";

    if (TableData == NULL)
    {
        CFE_EVS_SendEvent(HS_XC_TBL_NULL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error in XC Table Validation. Table is null.");

        return HS_TBL_VAL_ERR;
    }

    for (TableIndex = 0; TableIndex < HS_MAX_EXEC_CNT_SLOTS; TableIndex++)
    {
        ResourceType = TableArray[TableIndex].ResourceType;
        NullTerm     = TableArray[TableIndex].NullTerm;
        EntryResult  = HS_XCTVAL_NO_ERR;

        if (ResourceType == HS_XCT_TYPE_NOTYPE)
        {
            /*
            ** Unused table entry
            */
            UnusedCount++;
        }
        else if (NullTerm != 0)
        {
            /*
            ** Null Terminator Safety Buffer is not Null
            */
            EntryResult = HS_XCTVAL_ERR_NUL;
            BadCount++;
        }
        else if ((ResourceType != HS_XCT_TYPE_APP_MAIN) && (ResourceType != HS_XCT_TYPE_APP_CHILD) &&
                 (ResourceType != HS_XCT_TYPE_DEVICE) && (ResourceType != HS_XCT_TYPE_ISR))
        {
            /*
            ** Resource Type is not valid
            */
            EntryResult = HS_XCTVAL_ERR_TYPE;
            BadCount++;
        }
        else
        {
            /*
            ** Otherwise, this entry is good
            */
            GoodCount++;
        }

        /*
        ** Generate detailed event for "first" error
        */
        if ((EntryResult != HS_XCTVAL_NO_ERR) && (TableResult == CFE_SUCCESS))
        {
            strncpy(BadName, TableArray[TableIndex].ResourceName, OS_MAX_API_NAME - 1);
            BadName[OS_MAX_API_NAME - 1] = '\0';
            CFE_EVS_SendEvent(HS_XCTVAL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ExeCount verify err: Entry = %d, Err = %d, Type = %d, Name = %s", (int)TableIndex,
                              (int)EntryResult, ResourceType, BadName);
            TableResult = EntryResult;
        }
    }

    /*
    ** Generate informational event with error totals
    */
    CFE_EVS_SendEvent(HS_XCTVAL_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "ExeCount verify results: good = %d, bad = %d, unused = %d", (int)GoodCount, (int)BadCount,
                      (int)UnusedCount);

    return TableResult;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Validate the Message Actions Table                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 HS_ValidateMATable(void *TableData)
{
    HS_MATEntry_t *TableArray = (HS_MATEntry_t *)TableData;

    int32  TableResult = CFE_SUCCESS;
    uint32 TableIndex  = 0;
    size_t Length      = 0;
    uint16 EnableState;
    int32  EntryResult;

    CFE_SB_MsgId_t   MessageID = CFE_SB_INVALID_MSG_ID;
    CFE_SB_Buffer_t *BufPtr    = (CFE_SB_Buffer_t *)NULL;

    uint32 GoodCount   = 0;
    uint32 BadCount    = 0;
    uint32 UnusedCount = 0;

    if (TableData == NULL)
    {
        CFE_EVS_SendEvent(HS_MA_TBL_NULL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error in MA Table Validation. Table is null.");

        return HS_TBL_VAL_ERR;
    }

    for (TableIndex = 0; TableIndex < HS_MAX_MSG_ACT_TYPES; TableIndex++)
    {
        EntryResult = HS_MATVAL_NO_ERR;
        BufPtr      = (CFE_SB_Buffer_t *)&TableArray[TableIndex].MsgBuf;
        CFE_MSG_GetSize(&BufPtr->Msg, &Length);
        CFE_MSG_GetMsgId(&BufPtr->Msg, &MessageID);
        EnableState = TableArray[TableIndex].EnableState;

        if (EnableState == HS_MAT_STATE_DISABLED)
        {
            /*
            ** Unused table entry
            */
            UnusedCount++;
        }
        else if ((EnableState != HS_MAT_STATE_ENABLED) && (EnableState != HS_MAT_STATE_NOEVENT))
        {
            /*
            ** Enable State is Invalid
            */
            EntryResult = HS_MATVAL_ERR_ENA;
            BadCount++;
        }
        else if (!CFE_SB_IsValidMsgId(MessageID))
        {
            /*
            ** Message ID isn't valid
            */
            EntryResult = HS_MATVAL_ERR_ID;
            BadCount++;
        }
        else if (Length > CFE_MISSION_SB_MAX_SB_MSG_SIZE)
        {
            /*
            ** Length is too high
            */
            EntryResult = HS_MATVAL_ERR_LEN;
            BadCount++;
        }
        else
        {
            /*
            ** Otherwise, this entry is good
            */
            GoodCount++;
        }
        /*
        ** Generate detailed event for "first" error
        */
        if ((EntryResult != HS_MATVAL_NO_ERR) && (TableResult == CFE_SUCCESS))
        {
            CFE_EVS_SendEvent(HS_MATVAL_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MsgActs verify err: Entry = %d, Err = %d, Length = %d, ID = 0x%08lX", (int)TableIndex,
                              (int)EntryResult, (int)Length, (unsigned long)CFE_SB_MsgIdToValue(MessageID));
            TableResult = EntryResult;
        }
    }

    /*
    ** Generate informational event with error totals
    */
    CFE_EVS_SendEvent(HS_MATVAL_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "MsgActs verify results: good = %d, bad = %d, unused = %d", (int)GoodCount, (int)BadCount,
                      (int)UnusedCount);

    return TableResult;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Set the values being stored in the CDS                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_SetCDSData(uint16 ResetsPerformed, uint16 MaxResets)
{
    /*
    ** Set CDS data and verification inverses
    */
    HS_AppData.CDSData.ResetsPerformed    = ResetsPerformed;
    HS_AppData.CDSData.ResetsPerformedNot = ~HS_AppData.CDSData.ResetsPerformed;
    HS_AppData.CDSData.MaxResets          = MaxResets;
    HS_AppData.CDSData.MaxResetsNot       = ~HS_AppData.CDSData.MaxResets;
    /*
    ** Copy the data to the CDS if CDS Creation was successful
    */
    if (HS_AppData.CDSState == HS_STATE_ENABLED)
    {
        CFE_ES_CopyToCDS(HS_AppData.MyCDSHandle, &HS_AppData.CDSData);
    }
}
