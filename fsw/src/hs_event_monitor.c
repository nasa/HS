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
 *   CFS Health and Safety (HS) routines that implement the event
 *   monitoring subsystem
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_event_monitor.h"
#include "hs_msg_action.h"
#include "hs_tbldefs.h"
#include "hs_eventids.h"

/*----------------------------------------------------------------
 *
 * Local helper function
 * Sends the event associated with a message action failure
 *
 *-----------------------------------------------------------------*/
static bool HS_EventMon_ActionIsValid(uint16 ActionType)
{
    bool IsValid = true;

    if (ActionType < HS_EMTActType_NOACT)
    {
        IsValid = false;
    }
    else if (ActionType >= (HS_EMTActType_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES))
    {
        /* HS allows for HS_EMTActType_LAST_NONMSG actions by default and
           HS_MAX_MSG_ACT_TYPES message actions defined in the Message
           Action Table. */
        IsValid = false;
    }

    return IsValid;
}

/*----------------------------------------------------------------
 *
 * Local helper function
 * Sends the event associated with a message action failure
 *
 *-----------------------------------------------------------------*/
static void HS_EventMon_FailCallback(uint32 MsgActsIndex, const void *Arg)
{
    const HS_EMTEntry_t *EMEntryPtr = Arg;

    CFE_EVS_SendEvent(HS_EVENTMON_MSGACTS_ERR_EID,
                      CFE_EVS_EventType_ERROR,
                      "Event Monitor: APP:(%s) EID:(%d): Action: Message Action Index: %d",
                      EMEntryPtr->AppName,
                      EMEntryPtr->EventID,
                      (int)MsgActsIndex);
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_EventMon_TriggerAction(const HS_EMTEntry_t *EMEntryPtr)
{
    CFE_Status_t   Status = CFE_SUCCESS;
    CFE_ES_AppId_t AppId  = CFE_ES_APPID_UNDEFINED;

    /*
    ** Perform the action if the strings also match
    */
    switch (EMEntryPtr->ActionType)
    {
        case HS_EMTActType_PROC_RESET:
            CFE_EVS_SendEvent(HS_EVENTMON_PROC_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Event Monitor: APP:(%s) EID:(%d): Action: Processor Reset",
                              EMEntryPtr->AppName,
                              EMEntryPtr->EventID);

            /*
            ** Perform a reset if we can
            */
            if (HS_AppData.CDSData.ResetsPerformed < HS_AppData.CDSData.MaxResets)
            {
                HS_SetCDSData((HS_AppData.CDSData.ResetsPerformed + 1), HS_AppData.CDSData.MaxResets);

                OS_TaskDelay(HS_RESET_TASK_DELAY);
                CFE_ES_WriteToSysLog("HS App: Event Monitor: APP:(%s) EID:(%d): Action: Processor Reset\n",
                                     EMEntryPtr->AppName,
                                     (int)EMEntryPtr->EventID);
                HS_AppData.ServiceWatchdogFlag = HS_State_DISABLED;
                CFE_ES_ResetCFE(CFE_PSP_RST_TYPE_PROCESSOR);
            }
            else
            {
                CFE_EVS_SendEvent(HS_RESET_LIMIT_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "Processor Reset Action Limit Reached: No Reset Performed");
            }

            break;

        case HS_EMTActType_APP_RESTART:
            /*
            ** Check to see if the App is still there, and try to restart if it is
            */
            Status = CFE_ES_GetAppIDByName(&AppId, EMEntryPtr->AppName);
            if (Status == CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(HS_EVENTMON_RESTART_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "Event Monitor: APP:(%s) EID:(%d): Action: Restart Application",
                                  EMEntryPtr->AppName,
                                  EMEntryPtr->EventID);
                Status = CFE_ES_RestartApp(AppId);
            }

            if (Status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(HS_EVENTMON_NOT_RESTARTED_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "Call to Restart App Failed: APP:(%s) ERR: 0x%08X",
                                  EMEntryPtr->AppName,
                                  (unsigned int)Status);
            }

            break;

        case HS_EMTActType_APP_DELETE:
            /*
            ** Check to see if the App is still there, and try to delete if it is
            */
            Status = CFE_ES_GetAppIDByName(&AppId, EMEntryPtr->AppName);
            if (Status == CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(HS_EVENTMON_DELETE_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "Event Monitor: APP:(%s) EID:(%d): Action: Delete Application",
                                  EMEntryPtr->AppName,
                                  EMEntryPtr->EventID);
                Status = CFE_ES_DeleteApp(AppId);
            }

            if (Status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(HS_EVENTMON_NOT_DELETED_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "Call to Delete App Failed: APP:(%s) ERR: 0x%08X",
                                  EMEntryPtr->AppName,
                                  (unsigned int)Status);
            }

            break;

        /*
        ** Message Action types processing (invalid will be skipped)
        */
        default:
            HS_MsgAct_TriggerAction(EMEntryPtr->ActionType - HS_EMTActType_LAST_NONMSG,
                                    HS_EventMon_FailCallback,
                                    EMEntryPtr);
            break;
    } /* end switch */
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_EventMon_Check(const CFE_EVS_LongEventTlm_t *EventPtr)
{
    uint32         TableIndex = 0;
    HS_EMTEntry_t *EMEntryPtr;

    for (TableIndex = 0;; ++TableIndex)
    {
        EMEntryPtr = HS_GetEMTEntryByIndex(TableIndex);
        if (EMEntryPtr == NULL)
        {
            /* end of processing */
            break;
        }

        /*
        ** Check this Event Monitor if it has an action, and the event IDs match
        */
        if (EMEntryPtr->ActionType != HS_EMTActType_NOACT && EMEntryPtr->EventID == EventPtr->Payload.PacketID.EventID
            && strncmp(EMEntryPtr->AppName, EventPtr->Payload.PacketID.AppName, sizeof(EMEntryPtr->AppName)) == 0)
        {
            HS_EventMon_TriggerAction(EMEntryPtr);
        }
    } /* end for loop */
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 HS_EventMon_ValidateTable(void *TableData)
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
        CFE_EVS_SendEvent(HS_EM_TBL_NULL_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Error in EM Table Validation. Table is null.");

        return HS_TBL_VAL_ERR;
    }

    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_EVENTS; ++TableIndex)
    {
        ActionType  = TableArray[TableIndex].ActionType;
        EventID     = TableArray[TableIndex].EventID;
        NullTerm    = TableArray[TableIndex].NullTerm;
        EntryResult = HS_EMTVal_NO_ERR;

        if ((EventID == 0) || (ActionType == HS_EMTActType_NOACT))
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
            EntryResult = HS_EMTVal_ERR_NUL;
            BadCount++;
        }
        else if (!HS_EventMon_ActionIsValid(ActionType))
        {
            /*
            ** Action Type is not valid
            */
            EntryResult = HS_EMTVal_ERR_ACT;
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
        if ((EntryResult != HS_EMTVal_NO_ERR) && (TableResult == CFE_SUCCESS))
        {
            strncpy(BadName, TableArray[TableIndex].AppName, OS_MAX_API_NAME - 1);
            BadName[OS_MAX_API_NAME - 1] = '\0';
            CFE_EVS_SendEvent(HS_EMTVal_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "EventMon verify err: Entry = %d, Err = %d, Action = %d, ID = %d App = %s",
                              (int)TableIndex,
                              (int)EntryResult,
                              ActionType,
                              EventID,
                              BadName);
            TableResult = EntryResult;
        }
    }

    /*
    ** Generate informational event with error totals
    */
    CFE_EVS_SendEvent(HS_EMTVal_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "EventMon verify results: good = %d, bad = %d, unused = %d",
                      (int)GoodCount,
                      (int)BadCount,
                      (int)UnusedCount);

    return TableResult;
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_EventMon_StatusRefresh(void)
{
    uint32               TableIndex;
    HS_EMTEntry_t       *EMEntryPtr;
    HS_EventMon_State_t *EMStatePtr;

    /*
    ** Set AppMon enable bits and reset Countups and Exec Counter comparisons
    */
    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_EVENTS; ++TableIndex)
    {
        EMEntryPtr = HS_GetEMTEntryByIndex(TableIndex);
        EMStatePtr = HS_GetEMStateByIndex(TableIndex);

        EMStatePtr->AppId  = CFE_ES_APPID_UNDEFINED;
        EMStatePtr->Enable = (EMEntryPtr != NULL && EMEntryPtr->ActionType != HS_EMTActType_NOACT);
    }
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_EventMon_AcquirePointers(void)
{
    CFE_Status_t    Status;
    HS_State_Enum_t WasLoaded;
    void           *TableTempPtr;

    /* Capture the previous loading status (to detect changes) */
    WasLoaded = HS_AppData.EventMonLoaded;

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
    Status = CFE_TBL_GetAddress(&TableTempPtr, HS_AppData.EMTableHandle);

    /*
    ** If Address acquisition fails and currently enabled, report and disable (EventMon)
    */
    if (Status < CFE_SUCCESS)
    {
        /*
        ** Only report and disable if enabled or the table was previously loaded (EventMon)
        */
        if ((HS_AppData.EventMonLoaded == HS_State_ENABLED) || (HS_AppData.CurrentEventMonState == HS_State_ENABLED))
        {
            CFE_EVS_SendEvent(HS_EVENTMON_GETADDR_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Error getting EventMon Table address, RC=0x%08X, Event Monitoring Disabled",
                              (unsigned int)Status);

            if (HS_AppData.CurrentEventMonState == HS_State_ENABLED)
            {
                Status = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(CFE_EVS_LONG_EVENT_MSG_MID), HS_AppData.EventPipe);

                if (Status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(HS_BADEMT_LONG_UNSUB_EID,
                                      CFE_EVS_EventType_ERROR,
                                      "Error Unsubscribing from long-format Events,RC=0x%08X",
                                      (unsigned int)Status);
                }

                Status = CFE_SB_Unsubscribe(CFE_SB_ValueToMsgId(CFE_EVS_SHORT_EVENT_MSG_MID), HS_AppData.EventPipe);

                if (Status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(HS_BADEMT_SHORT_UNSUB_EID,
                                      CFE_EVS_EventType_ERROR,
                                      "Error Unsubscribing from short-format Events,RC=0x%08X",
                                      (unsigned int)Status);
                }
            }

            HS_AppData.CurrentEventMonState = HS_State_DISABLED;
            HS_AppData.EventMonLoaded       = HS_State_DISABLED;
        }

        TableTempPtr = NULL;
    }
    /*
    ** Otherwise, mark that the table is loaded (EventMon)
    */
    else
    {
        HS_AppData.EventMonLoaded = HS_State_ENABLED;
    }

    HS_AppData.EMTablePtr = TableTempPtr;

    /*
    ** If there is a new table, refresh status (EventMon)
    */
    if (HS_AppData.EventMonLoaded != WasLoaded || Status == CFE_TBL_INFO_UPDATED)
    {
        HS_EventMon_StatusRefresh();
    }

    HS_EventMon_CheckEntries();
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_EventMon_CheckEntries(void)
{
    uint32               TableIndex;
    HS_EMTEntry_t       *EMEntryPtr;
    HS_EventMon_State_t *EMStatePtr;
    CFE_Status_t         Status;
    uint32               InactiveCount;
    char                 AppNameBuf[CFE_MISSION_MAX_API_LEN];

    InactiveCount = 0;
    for (TableIndex = 0;; ++TableIndex)
    {
        EMEntryPtr = HS_GetEMTEntryByIndex(TableIndex);
        if (EMEntryPtr == NULL)
        {
            /* end of processing */
            break;
        }

        EMStatePtr = HS_GetEMStateByIndex(TableIndex);
        if (EMStatePtr->Enable)
        {
            /* Getting the app name is a less expensive call as it maps directly to a table entry */
            Status = CFE_ES_GetAppName(AppNameBuf, EMStatePtr->AppId, sizeof(AppNameBuf));
            if (Status != CFE_SUCCESS || strncmp(AppNameBuf, EMEntryPtr->AppName, sizeof(AppNameBuf)) != 0)
            {
                /* this is a more expensive call because it searches sequentially through all app names
                 * to find a match, so it is only done when something changed. */
                Status = CFE_ES_GetAppIDByName(&EMStatePtr->AppId, EMEntryPtr->AppName);
            }
            if (Status != CFE_SUCCESS)
            {
                /* This means the app name configured for this event mon is not currently running.
                 * This may or may not be an operational problem, but the event monitor will be inactive. */
                EMStatePtr->AppId = CFE_ES_APPID_UNDEFINED;
                ++InactiveCount;
            }
        }
    }

    /* The number of inactive event monitor entries is reported in TLM */
    HS_AppData.InactiveEventMonCount = InactiveCount;
}
