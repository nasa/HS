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
 *   Specification for the CFS Health and Safety (HS) routines that
 *   handle sending of messages after monitored conditions are triggered.
 *
 *   This specificially deals with the management of the Message Action
 *   table which is shared by App Mon and Event Mon entries that are configured
 *   to send a software bus message when a matching condition is detected.
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_msg_action.h"
#include "hs_tbldefs.h"
#include "hs_eventids.h"

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_MsgAct_TriggerAction(uint16 ActionType, HS_MsgAct_Callback_t SendEventCb, const void *CbArg)
{
    uint32            MsgActsIndex;
    HS_MATEntry_t    *MAEntryPtr;
    HS_MsgActState_t *MAStatePtr;

    /* Calculate the requested message action index */
    MsgActsIndex = ActionType - 1;

    /*
    ** Check to see if this is a valid Message Action Type
    */
    if (HS_AppData.MsgActsLoaded == HS_State_ENABLED)
    {
        MAEntryPtr = HS_GetMATEntryByIndex(MsgActsIndex);
        MAStatePtr = HS_GetMAStateByIndex(MsgActsIndex);

        /*
        ** Send the message if off cooldown and not disabled
        */
        if (MAEntryPtr != NULL && MAStatePtr->Cooldown == 0 && MAEntryPtr->EnableState != HS_MATState_DISABLED)
        {
            CFE_SB_TransmitMsg((const CFE_MSG_Message_t *)&MAEntryPtr->MsgBuf, true);

            HS_AppData.MsgActExec++;
            MAStatePtr->Cooldown = MAEntryPtr->Cooldown;

            /* Send the event via callback */
            if (MAEntryPtr->EnableState != HS_MATState_NOEVENT && SendEventCb != NULL)
            {
                SendEventCb(MsgActsIndex, CbArg);
            }
        }
    }
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 HS_MsgAct_ValidateTable(void *TableData)
{
    HS_MATEntry_t *TableArray = (HS_MATEntry_t *)TableData;

    int32  TableResult = CFE_SUCCESS;
    uint32 TableIndex  = 0;
    size_t Length      = 0;
    uint16 EnableState;
    int32  EntryResult;

    CFE_SB_MsgId_t           MessageID = CFE_SB_INVALID_MSG_ID;
    const CFE_MSG_Message_t *MsgPtr    = NULL;

    uint32 GoodCount   = 0;
    uint32 BadCount    = 0;
    uint32 UnusedCount = 0;

    if (TableData == NULL)
    {
        CFE_EVS_SendEvent(HS_MA_TBL_NULL_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Error in MA Table Validation. Table is null.");

        return HS_TBL_VAL_ERR;
    }

    for (TableIndex = 0; TableIndex < HS_MAX_MSG_ACT_TYPES; ++TableIndex)
    {
        EntryResult = HS_MATVal_NO_ERR;
        MsgPtr      = (const CFE_MSG_Message_t *)&TableArray[TableIndex].MsgBuf;
        CFE_MSG_GetSize(MsgPtr, &Length);
        CFE_MSG_GetMsgId(MsgPtr, &MessageID);
        EnableState = TableArray[TableIndex].EnableState;

        if (EnableState == HS_MATState_DISABLED)
        {
            /*
            ** Unused table entry
            */
            UnusedCount++;
        }
        else if ((EnableState != HS_MATState_ENABLED) && (EnableState != HS_MATState_NOEVENT))
        {
            /*
            ** Enable State is Invalid
            */
            EntryResult = HS_MATVal_ERR_ENA;
            BadCount++;
        }
        else if (!CFE_SB_IsValidMsgId(MessageID))
        {
            /*
            ** Message ID isn't valid
            */
            EntryResult = HS_MATVal_ERR_ID;
            BadCount++;
        }
        else if (Length > CFE_MISSION_SB_MAX_SB_MSG_SIZE)
        {
            /*
            ** Length is too high
            */
            EntryResult = HS_MATVal_ERR_LEN;
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
        if ((EntryResult != HS_MATVal_NO_ERR) && (TableResult == CFE_SUCCESS))
        {
            CFE_EVS_SendEvent(HS_MATVal_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "MsgActs verify err: Entry = %d, Err = %d, Length = %d, ID = 0x%08lX",
                              (int)TableIndex,
                              (int)EntryResult,
                              (int)Length,
                              (unsigned long)CFE_SB_MsgIdToValue(MessageID));
            TableResult = EntryResult;
        }
    }

    /*
    ** Generate informational event with error totals
    */
    CFE_EVS_SendEvent(HS_MATVal_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "MsgActs verify results: good = %d, bad = %d, unused = %d",
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
void HS_MsgAct_StatusRefresh(void)
{
    uint32            TableIndex;
    HS_MsgActState_t *MAStatePtr;

    /*
    ** Clear all MsgActs Cooldowns
    */
    for (TableIndex = 0; TableIndex < HS_MAX_MSG_ACT_TYPES; ++TableIndex)
    {
        MAStatePtr = HS_GetMAStateByIndex(TableIndex);

        MAStatePtr->Cooldown = 0;
    }
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_MsgAct_AcquirePointers(void)
{
    CFE_Status_t    Status;
    HS_State_Enum_t WasLoaded;
    void           *TableTempPtr;

    /* Capture the previous loading status (to detect changes) */
    WasLoaded = HS_AppData.MsgActsLoaded;

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
    Status = CFE_TBL_GetAddress(&TableTempPtr, HS_AppData.MATableHandle);

    /*
    ** If Address acquisition fails report and disable (MsgActs)
    */
    if (Status < CFE_SUCCESS)
    {
        /*
        ** To prevent redundant reporting, only report if enabled (MsgActs)
        */
        if (HS_AppData.MsgActsLoaded == HS_State_ENABLED)
        {
            CFE_EVS_SendEvent(HS_MSGACTS_GETADDR_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Error getting MsgActs Table address, RC=0x%08X",
                              (unsigned int)Status);
            HS_AppData.MsgActsLoaded = HS_State_DISABLED;
        }

        TableTempPtr = NULL;
    }
    /*
    ** Otherwise, make sure it is enabled (MsgActs)
    */
    else
    {
        HS_AppData.MsgActsLoaded = HS_State_ENABLED;
    }

    HS_AppData.MATablePtr = TableTempPtr;

    /*
    ** If there is a new table, refresh status (MsgActs)
    */
    if (HS_AppData.MsgActsLoaded != WasLoaded || Status == CFE_TBL_INFO_UPDATED)
    {
        HS_MsgAct_StatusRefresh();
    }
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_MsgAct_Cooldown(void)
{
    uint32            i = 0;
    HS_MsgActState_t *MAStatePtr;

    /*
    ** Decrement Cooldowns for Message Actions
    */
    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        MAStatePtr = HS_GetMAStateByIndex(i);

        if (MAStatePtr->Cooldown != 0)
        {
            --MAStatePtr->Cooldown;
        }
    }
}
