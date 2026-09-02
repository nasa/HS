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
 *   CFS Health and Safety (HS) routines that implement the application
 *   monitoring subsystem
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_app_monitor.h"
#include "hs_msg_action.h"
#include "hs_tbldefs.h"
#include "hs_eventids.h"

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
static bool HS_AppMon_ActionIsValid(uint16 ActionType)
{
    bool IsValid = true;

    if (ActionType < HS_AMTActType_NOACT)
    {
        IsValid = false;
    }
    else if (ActionType >= (HS_AMTActType_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES))
    {
        /* HS allows for HS_AMTActType_LAST_NONMSG actions by default and
           HS_MAX_MSG_ACT_TYPES message actions defined in the Message
           Action Table. */
        IsValid = false;
    }

    return IsValid;
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_AppMon_ComputeEnableBits(void *AppMonEnableBits)
{
    uint32             TableIndex;
    HS_AppMon_State_t *AMStatePtr;

    /*
    ** Update the AppMon Enables
    */
    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_APPS; ++TableIndex)
    {
        AMStatePtr = HS_GetAMStateByIndex(TableIndex);

        if (AMStatePtr->Enable)
        {
            HS_SET_TLM_ENABLE_BITMASK(AppMonEnableBits, TableIndex);
        }
    }
}

/*----------------------------------------------------------------
 *
 * Local helper function
 * Sends the event associated with a message action failure
 *
 *-----------------------------------------------------------------*/
static void HS_AppMon_FailCallback(uint32 MsgActsIndex, const void *Arg)
{
    const HS_AMTEntry_t *AMEntryPtr = Arg;

    CFE_EVS_SendEvent(HS_APPMON_MSGACTS_ERR_EID,
                      CFE_EVS_EventType_ERROR,
                      "App Monitor Failure: APP:(%s): Action: Message Action Index: %d",
                      AMEntryPtr->AppName,
                      (int)MsgActsIndex);
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_AppMon_TriggerAction(const HS_AMTEntry_t *AMEntryPtr, CFE_ES_AppId_t AppId)
{
    CFE_Status_t Status;

    switch (AMEntryPtr->ActionType)
    {
        case HS_AMTActType_PROC_RESET:
            CFE_EVS_SendEvent(HS_APPMON_PROC_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "App Monitor Failure: APP:(%s): Action: Processor Reset",
                              AMEntryPtr->AppName);

            /*
            ** Perform a reset if we can
            */
            if (HS_AppData.CDSData.ResetsPerformed < HS_AppData.CDSData.MaxResets)
            {
                HS_SetCDSData((HS_AppData.CDSData.ResetsPerformed + 1), HS_AppData.CDSData.MaxResets);

                OS_TaskDelay(HS_RESET_TASK_DELAY);
                CFE_ES_WriteToSysLog("HS App: App Monitor Failure: APP:(%s): Action: Processor Reset\n",
                                     AMEntryPtr->AppName);
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

        case HS_AMTActType_APP_RESTART:
            CFE_EVS_SendEvent(HS_APPMON_RESTART_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "App Monitor Failure: APP:(%s) Action: Restart Application",
                              AMEntryPtr->AppName);
            /*
            ** Attempt to restart the App if we resolved the AppId
            */
            Status = CFE_ES_RestartApp(AppId);

            /*
            ** Report an error; either no valid AppId, or RestartApp failed
            */
            if (Status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(HS_APPMON_NOT_RESTARTED_ERR_EID,
                                  CFE_EVS_EventType_ERROR,
                                  "Call to Restart App Failed: APP:(%s) ERR: 0x%08X",
                                  AMEntryPtr->AppName,
                                  (unsigned int)Status);
            }

            break;

        case HS_AMTActType_EVENT:
            CFE_EVS_SendEvent(HS_APPMON_FAIL_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "App Monitor Failure: APP:(%s): Action: Event Only",
                              AMEntryPtr->AppName);
            break;

        /*
        ** Message Action types processing (invalid will be skipped)
        */
        default:
            HS_MsgAct_TriggerAction(AMEntryPtr->ActionType - HS_AMTActType_LAST_NONMSG,
                                    HS_AppMon_FailCallback,
                                    AMEntryPtr);
            break;
    } /* end switch */
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_AppMon_CheckApp(const HS_AMTEntry_t *AMEntryPtr, HS_AppMon_State_t *AMStatePtr)
{
    CFE_ES_AppInfo_t AppInfo;
    CFE_Status_t     Status;

    memset(&AppInfo, 0, sizeof(AppInfo));

    if (CFE_RESOURCEID_TEST_DEFINED(AMStatePtr->AppId))
    {
        /* appid cache is valid, skip lookup */
        Status = CFE_SUCCESS;
    }
    else
    {
        /* appid not known, check with ES to see if its running now */
        Status = CFE_ES_GetAppIDByName(&AMStatePtr->AppId, AMEntryPtr->AppName);
    }

    if (Status == CFE_SUCCESS)
    {
        Status = CFE_ES_GetAppInfo(&AppInfo, AMStatePtr->AppId);
    }

    if (Status != CFE_SUCCESS)
    {
        AMStatePtr->AppId = CFE_ES_APPID_UNDEFINED; /* clear cache of appid */

        if (AMStatePtr->CheckInCountdown == AMEntryPtr->CycleCount)
        {
            /*
            ** Only send an error event the first time the App fails to resolve
            */
            CFE_EVS_SendEvent(HS_APPMON_APPNAME_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "App Mon not found: %s, status=%08x",
                              AMEntryPtr->AppName,
                              (unsigned int)Status);
        }
        else
        {
            /* For repeated errors, send a debug event */
            CFE_EVS_SendEvent(HS_APPMON_APPNAME_DBG_EID,
                              CFE_EVS_EventType_DEBUG,
                              "App Mon not found: %s, status=%08x",
                              AMEntryPtr->AppName,
                              (unsigned int)Status);
        }
    }

    /*
    ** Failure to get an execution counter is not considered an automatic failure (or eventworthy)
    */
    if ((Status == CFE_SUCCESS) && (AMStatePtr->LastExeCount != AppInfo.ExecutionCounter))
    {
        /*
        ** Set the current count, and reset the timeout
        */
        AMStatePtr->CheckInCountdown = AMEntryPtr->CycleCount;
        AMStatePtr->LastExeCount     = AppInfo.ExecutionCounter;
    }
    else
    {
        AMStatePtr->CheckInCountdown--;

        /*
        ** Take Action once the counter reaches zero
        */
        if (AMStatePtr->CheckInCountdown == 0)
        {
            /*
            ** Unset the enabled bit flag
            */
            AMStatePtr->Enable = false;

            /* Do the configured action */
            HS_AppMon_TriggerAction(AMEntryPtr, AMStatePtr->AppId);

        } /* end (AMStatePtr->CheckInCountdown == 0) if */
    }
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_AppMon_CheckAllApps(void)
{
    uint32             TableIndex;
    HS_AMTEntry_t     *AMEntryPtr;
    HS_AppMon_State_t *AMStatePtr;

    for (TableIndex = 0;; ++TableIndex)
    {
        AMEntryPtr = HS_GetAMTEntryByIndex(TableIndex);

        if (AMEntryPtr == NULL)
        {
            /* end of processing */
            break;
        }

        AMStatePtr = HS_GetAMStateByIndex(TableIndex);

        if (AMStatePtr->Enable)
        {
            HS_AppMon_CheckApp(AMEntryPtr, AMStatePtr);
        }
    } /* end for loop */
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 HS_AppMon_ValidateTable(void *TableData)
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
        CFE_EVS_SendEvent(HS_AM_TBL_NULL_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Error in AM Table Validation. Table is null.");

        return HS_TBL_VAL_ERR;
    }

    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_APPS; ++TableIndex)
    {
        ActionType  = TableArray[TableIndex].ActionType;
        CycleCount  = TableArray[TableIndex].CycleCount;
        NullTerm    = TableArray[TableIndex].NullTerm;
        EntryResult = HS_AMTVal_NO_ERR;

        if ((CycleCount == 0) || (ActionType == HS_AMTActType_NOACT))
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
            EntryResult = HS_AMTVal_ERR_NUL;
            BadCount++;
        }
        else if (!HS_AppMon_ActionIsValid(ActionType))
        {
            /*
            ** Action Type is not valid
            */
            EntryResult = HS_AMTVal_ERR_ACT;
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
        if ((EntryResult != HS_AMTVal_NO_ERR) && (TableResult == CFE_SUCCESS))
        {
            strncpy(BadName, TableArray[TableIndex].AppName, OS_MAX_API_NAME - 1);
            BadName[OS_MAX_API_NAME - 1] = '\0';
            CFE_EVS_SendEvent(HS_AMTVal_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "AppMon verify err: Entry = %d, Err = %d, Action = %d, App = %s",
                              (int)TableIndex,
                              (int)EntryResult,
                              ActionType,
                              BadName);
            TableResult = EntryResult;
        }
    }

    /*
    ** Generate informational event with error totals
    */
    CFE_EVS_SendEvent(HS_AMTVal_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "AppMon verify results: good = %d, bad = %d, unused = %d",
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
void HS_AppMon_StatusRefresh(void)
{
    uint32             TableIndex;
    HS_AMTEntry_t     *AMEntryPtr;
    HS_AppMon_State_t *AMStatePtr;

    /*
    ** Set AppMon enable bits and reset Countups and Exec Counter comparisons
    */
    for (TableIndex = 0; TableIndex < HS_MAX_MONITORED_APPS; ++TableIndex)
    {
        AMEntryPtr = HS_GetAMTEntryByIndex(TableIndex);
        AMStatePtr = HS_GetAMStateByIndex(TableIndex);

        AMStatePtr->AppId        = CFE_ES_APPID_UNDEFINED;
        AMStatePtr->LastExeCount = 0;

        if (AMEntryPtr == NULL || AMEntryPtr->CycleCount == 0 || AMEntryPtr->ActionType == HS_AMTActType_NOACT)
        {
            AMStatePtr->CheckInCountdown = 0;
            AMStatePtr->Enable           = false;
        }
        else
        {
            AMStatePtr->CheckInCountdown = AMEntryPtr->CycleCount;
            AMStatePtr->Enable           = true;
        }
    }
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_AppMon_AcquirePointers(void)
{
    CFE_Status_t    Status;
    HS_State_Enum_t WasLoaded;
    void           *TableTempPtr;

    /* Capture the previous loading status (to detect changes) */
    WasLoaded = HS_AppData.AppMonLoaded;

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
    Status = CFE_TBL_GetAddress(&TableTempPtr, HS_AppData.AMTableHandle);

    /*
    ** If Address acquisition fails and currently enabled, report and disable (AppMon)
    */
    if (Status < CFE_SUCCESS)
    {
        /*
        ** Only report and disable if enabled or the table was previously loaded (AppMon)
        */
        if ((HS_AppData.AppMonLoaded == HS_State_ENABLED) || (HS_AppData.CurrentAppMonState == HS_State_ENABLED))
        {
            CFE_EVS_SendEvent(HS_APPMON_GETADDR_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Error getting AppMon Table address, RC=0x%08X, Application Monitoring Disabled",
                              (unsigned int)Status);
            HS_AppData.CurrentAppMonState = HS_State_DISABLED;
            HS_AppData.AppMonLoaded       = HS_State_DISABLED;
        }

        TableTempPtr = NULL;
    }
    /*
    ** Otherwise, mark that the table is loaded (AppMon)
    */
    else
    {
        HS_AppData.AppMonLoaded = HS_State_ENABLED;
    }

    HS_AppData.AMTablePtr = TableTempPtr;

    /*
    ** If there is a new table, refresh status (AppMon)
    */
    if (HS_AppData.AppMonLoaded != WasLoaded || Status == CFE_TBL_INFO_UPDATED)
    {
        HS_AppMon_StatusRefresh();
    }
}
