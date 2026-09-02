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
 *   handle task execution, generic counter, and CPU monitoring
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_exec_monitor.h"
#include "hs_sysmon.h"
#include "hs_tbldefs.h"
#include "hs_eventids.h"

/* declarations for local helper routines */
static CFE_Status_t HS_ExecMon_FindInvalidResId(const char *Name, CFE_ResourceId_t *ResIdPtr);
static CFE_Status_t HS_ExecMon_FindTaskResId(const char *TaskName, CFE_ResourceId_t *ResIdPtr);
static CFE_Status_t HS_ExecMon_FindCounterResId(const char *CounterName, CFE_ResourceId_t *ResIdPtr);
static CFE_Status_t HS_ExecMon_ReadInvalid(CFE_ResourceId_t ResId, uint32 *CountPtr);
static CFE_Status_t HS_ExecMon_ReadTaskCount(CFE_ResourceId_t ResId, uint32 *CountPtr);
static CFE_Status_t HS_ExecMon_ReadGenCounter(CFE_ResourceId_t ResId, uint32 *CountPtr);

typedef struct
{
    CFE_Status_t (*Lookup)(const char *, CFE_ResourceId_t *);
    CFE_Status_t (*Read)(CFE_ResourceId_t, uint32 *);
} HS_ExecMon_TypeLookup_t;

/*----------------------------------------------------------------
 *
 * Local helper function
 *
 *-----------------------------------------------------------------*/
static CFE_Status_t HS_ExecMon_FindTaskResId(const char *TaskName, CFE_ResourceId_t *ResIdPtr)
{
    CFE_ES_TaskId_t TaskId;
    CFE_Status_t    Status;

    /* NOTE: This sets the TaskID output in all cases - will set to UNDEFINED if it fails */
    Status = CFE_ES_GetTaskIDByName(&TaskId, TaskName);

    *ResIdPtr = CFE_RESOURCEID_UNWRAP(TaskId);

    return Status;
}

/*----------------------------------------------------------------
 *
 * Local helper function
 *
 *-----------------------------------------------------------------*/
static CFE_Status_t HS_ExecMon_FindCounterResId(const char *CounterName, CFE_ResourceId_t *ResIdPtr)
{
    CFE_ES_CounterId_t CounterId;
    CFE_Status_t       Status;

    /* NOTE: This sets the CounterID output in all cases - will set to UNDEFINED if it fails */
    Status = CFE_ES_GetGenCounterIDByName(&CounterId, CounterName);

    *ResIdPtr = CFE_RESOURCEID_UNWRAP(CounterId);

    return Status;
}

/*----------------------------------------------------------------
 *
 * Local helper function
 *
 *-----------------------------------------------------------------*/
static CFE_Status_t HS_ExecMon_ReadTaskCount(CFE_ResourceId_t ResId, uint32 *CountPtr)
{
    CFE_ES_TaskInfo_t TaskInfo;
    CFE_Status_t      Status;

    Status = CFE_ES_GetTaskInfo(&TaskInfo, CFE_ES_TASKID_C(ResId));

    *CountPtr = TaskInfo.ExecutionCounter;

    return Status;
}

/*----------------------------------------------------------------
 *
 * Local helper function
 *
 *-----------------------------------------------------------------*/
static CFE_Status_t HS_ExecMon_ReadGenCounter(CFE_ResourceId_t ResId, uint32 *CountPtr)
{
    return CFE_ES_GetGenCount(CFE_ES_COUNTERID_C(ResId), CountPtr);
}

/*----------------------------------------------------------------
 *
 * Local helper function
 *
 *-----------------------------------------------------------------*/
static CFE_Status_t HS_ExecMon_FindInvalidResId(const char *Name, CFE_ResourceId_t *ResIdPtr)
{
    /* This means the ResourceType in the table is bad.
     * Should be impossible to get this -- Table validation should've caught it. */
    CFE_EVS_SendEvent(HS_HKREQ_RESOURCE_DBG_EID,
                      CFE_EVS_EventType_DEBUG,
                      "Housekeeping req invalid resource type for \'%s\'",
                      Name);

    return CFE_STATUS_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------
 *
 * Local helper function
 *
 *-----------------------------------------------------------------*/
static CFE_Status_t HS_ExecMon_ReadInvalid(CFE_ResourceId_t ResId, uint32 *CountPtr)
{
    return CFE_STATUS_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------
 *
 * Local helper function
 * Gets the bindings for a given XC Resource Type
 * This always returns an object, never NULL.
 *
 *-----------------------------------------------------------------*/
const HS_ExecMon_TypeLookup_t *HS_ExecMon_GetHandlerForType(uint32 ResourceType)
{
    static const HS_ExecMon_TypeLookup_t HS_EXECMON_HANDLER_TABLE[] = {
        [HS_XCTResType_NOTYPE]    = { .Lookup = HS_ExecMon_FindInvalidResId, .Read = HS_ExecMon_ReadInvalid    },
        [HS_XCTResType_APP_MAIN]  = { .Lookup = HS_ExecMon_FindTaskResId,    .Read = HS_ExecMon_ReadTaskCount  },
        [HS_XCTResType_APP_CHILD] = { .Lookup = HS_ExecMon_FindTaskResId,    .Read = HS_ExecMon_ReadTaskCount  },
        [HS_XCTResType_DEVICE]    = { .Lookup = HS_ExecMon_FindCounterResId, .Read = HS_ExecMon_ReadGenCounter },
        [HS_XCTResType_ISR]       = { .Lookup = HS_ExecMon_FindCounterResId, .Read = HS_ExecMon_ReadGenCounter },
    };

    if (ResourceType >= (sizeof(HS_EXECMON_HANDLER_TABLE) / sizeof(HS_ExecMon_TypeLookup_t)))
    {
        ResourceType = HS_XCTResType_NOTYPE;
    }

    return &HS_EXECMON_HANDLER_TABLE[ResourceType];
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_ExecMon_CheckCpuHogging(uint32 CurrentUtil)
{
    if (CurrentUtil >= HS_UTIL_PER_INTERVAL_HOGGING && HS_AppData.CurrentCPUHogState == HS_State_ENABLED)
    {
        ++HS_AppData.CurrentCPUHoggingTime;

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
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_ExecMon_UpdateStats(uint32 CurrentUtil, uint32 AverageIntervalCount)
{
    uint32 HistUtil;
    uint32 CombinedUtil;
    uint32 CombinedCount;
    uint32 PeakUtil;
    uint32 PeakCount;
    uint32 HeadIdx;

    /* The index points to the pending (oldest) entry */
    HeadIdx = HS_AppData.CurrentCPUUtilIndex;

    CombinedCount = 1;
    CombinedUtil  = CurrentUtil;
    PeakUtil      = CurrentUtil;

    for (PeakCount = 0; PeakCount < HS_UTIL_PEAK_NUM_INTERVAL; ++PeakCount)
    {
        /* Walk through the history backwards, so it goes from most to least recent */
        /* Note that the data is zeroed at boot, so even if we do not have any history yet,
         * this is still safe because they will all be zero. */
        if (HeadIdx == 0)
        {
            HeadIdx = HS_UTIL_PEAK_NUM_INTERVAL;
        }
        --HeadIdx;
        HistUtil = HS_AppData.UtilizationTracker[HeadIdx];

        if (CombinedCount < AverageIntervalCount)
        {
            ++CombinedCount;
            CombinedUtil += HistUtil;
        }

        if (HistUtil > PeakUtil)
        {
            PeakUtil = HistUtil;
        }
    }

    /* convert from sum to average */
    /* Note that CombinedCount is always at least 1, it cannot be 0. */
    HS_AppData.UtilCpuAvg  = CombinedUtil / CombinedCount;
    HS_AppData.UtilCpuPeak = PeakUtil;
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_ExecMon_CheckUtilization(void)
{
    int32  CurrentUtil;
    uint32 ThisUtilIndex;

    ThisUtilIndex = HS_AppData.CurrentCPUUtilIndex;
    if (ThisUtilIndex >= HS_UTIL_PEAK_NUM_INTERVAL)
    {
        ThisUtilIndex = 0;
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

        HS_ExecMon_CheckCpuHogging(CurrentUtil);
        HS_ExecMon_UpdateStats(CurrentUtil, HS_UTIL_AVERAGE_NUM_INTERVAL);

        /* Update the global for next cycle */
        /* this is only done when we read a valid CPU usage */
        HS_AppData.UtilizationTracker[ThisUtilIndex] = CurrentUtil;
        HS_AppData.CurrentCPUUtilIndex               = ThisUtilIndex + 1;
    }
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 HS_ExecMon_ValidateTable(void *TableData)
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
        CFE_EVS_SendEvent(HS_XC_TBL_NULL_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Error in XC Table Validation. Table is null.");

        return HS_TBL_VAL_ERR;
    }

    for (TableIndex = 0; TableIndex < HS_MAX_EXEC_CNT_SLOTS; ++TableIndex)
    {
        ResourceType = TableArray[TableIndex].ResourceType;
        NullTerm     = TableArray[TableIndex].NullTerm;
        EntryResult  = HS_XCTVal_NO_ERR;

        if (ResourceType == HS_XCTResType_NOTYPE)
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
            EntryResult = HS_XCTVal_ERR_NUL;
            BadCount++;
        }
        else if ((ResourceType != HS_XCTResType_APP_MAIN) && (ResourceType != HS_XCTResType_APP_CHILD)
                 && (ResourceType != HS_XCTResType_DEVICE) && (ResourceType != HS_XCTResType_ISR))
        {
            /*
            ** Resource Type is not valid
            */
            EntryResult = HS_XCTVal_ERR_TYPE;
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
        if ((EntryResult != HS_XCTVal_NO_ERR) && (TableResult == CFE_SUCCESS))
        {
            strncpy(BadName, TableArray[TableIndex].ResourceName, OS_MAX_API_NAME - 1);
            BadName[OS_MAX_API_NAME - 1] = '\0';
            CFE_EVS_SendEvent(HS_XCTVal_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "ExeCount verify err: Entry = %d, Err = %d, Type = %d, Name = %s",
                              (int)TableIndex,
                              (int)EntryResult,
                              ResourceType,
                              BadName);
            TableResult = EntryResult;
        }
    }

    /*
    ** Generate informational event with error totals
    */
    CFE_EVS_SendEvent(HS_XCTVal_INF_EID,
                      CFE_EVS_EventType_INFORMATION,
                      "ExeCount verify results: good = %d, bad = %d, unused = %d",
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
void HS_ExecMon_StatusRefresh(void)
{
    uint32              TableIndex;
    HS_XCTEntry_t      *XCEntryPtr;
    HS_ExecMon_State_t *XCStatePtr;

    /*
    ** Clear all MsgActs Cooldowns
    */
    for (TableIndex = 0; TableIndex < HS_MAX_MSG_ACT_TYPES; ++TableIndex)
    {
        XCEntryPtr = HS_GetXCTEntryByIndex(TableIndex);
        XCStatePtr = HS_GetXCStateByIndex(TableIndex);

        XCStatePtr->ResourceId = CFE_RESOURCEID_UNDEFINED;
        XCStatePtr->Enable     = (XCEntryPtr != NULL && XCEntryPtr->ResourceType != HS_XCTResType_NOTYPE);
    }
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_ExecMon_GetCounters(uint32 *ExeCounts)
{
    uint32              ExeCount;
    uint32              TableIndex;
    HS_ExecMon_State_t *XCStatePtr;

    /*
    ** Add the execution counters
    */
    for (TableIndex = 0; TableIndex < HS_MAX_EXEC_CNT_SLOTS; ++TableIndex)
    {
        XCStatePtr = HS_GetXCStateByIndex(TableIndex);

        if (XCStatePtr->Enable)
        {
            ExeCount = XCStatePtr->LastSample;
        }
        else
        {
            ExeCount = HS_INVALID_EXECOUNT;
        }

        ExeCounts[TableIndex] = ExeCount;
    }
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_ExecMon_AcquirePointers(void)
{
    CFE_Status_t    Status;
    HS_State_Enum_t WasLoaded;
    void           *TableTempPtr;

    /* Capture the previous loading status (to detect changes) */
    WasLoaded = HS_AppData.ExecMonLoaded;

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
    Status = CFE_TBL_GetAddress(&TableTempPtr, HS_AppData.XCTableHandle);

    /*
    ** If Address acquisition fails report and disable (ExeCount)
    */
    if (Status < CFE_SUCCESS)
    {
        /*
        ** To prevent redundant reporting, only report if enabled (ExeCount)
        */
        if (HS_AppData.ExecMonLoaded == HS_State_ENABLED)
        {
            CFE_EVS_SendEvent(HS_EXECOUNT_GETADDR_ERR_EID,
                              CFE_EVS_EventType_ERROR,
                              "Error getting ExeCount Table address, RC=0x%08X",
                              (unsigned int)Status);
            HS_AppData.ExecMonLoaded = HS_State_DISABLED;
        }

        TableTempPtr = NULL;
    }
    /*
    ** Otherwise, make sure it is enabled (ExeCount)
    */
    else
    {
        HS_AppData.ExecMonLoaded = HS_State_ENABLED;
    }

    HS_AppData.XCTablePtr = TableTempPtr;

    /*
    ** If there is a new table, refresh status (EventMon)
    */
    if (HS_AppData.ExecMonLoaded != WasLoaded || Status == CFE_TBL_INFO_UPDATED)
    {
        HS_ExecMon_StatusRefresh();
    }

    HS_ExecMon_CheckEntries();
}

/*----------------------------------------------------------------
 *
 * Application-scope internal function
 * See description in header file for argument/return detail
 *
 *-----------------------------------------------------------------*/
void HS_ExecMon_CheckEntries(void)
{
    uint32                         TableIndex;
    HS_XCTEntry_t                 *XCEntryPtr;
    HS_ExecMon_State_t            *XCStatePtr;
    uint32                         InactiveCount;
    CFE_Status_t                   Status;
    const HS_ExecMon_TypeLookup_t *ImplPtr;

    InactiveCount = 0;
    for (TableIndex = 0;; ++TableIndex)
    {
        XCEntryPtr = HS_GetXCTEntryByIndex(TableIndex);
        if (XCEntryPtr == NULL)
        {
            /* end of processing */
            break;
        }

        Status     = CFE_STATUS_NOT_IMPLEMENTED; /* placeholder*/
        XCStatePtr = HS_GetXCStateByIndex(TableIndex);
        if (XCStatePtr->Enable)
        {
            ImplPtr = HS_ExecMon_GetHandlerForType(XCEntryPtr->ResourceType);
            if (CFE_ResourceId_IsDefined(XCStatePtr->ResourceId))
            {
                /* use cached resource ID */
                Status = CFE_SUCCESS;
            }
            else
            {
                /* find the resource ID.  This is a search so only do this when needed */
                Status = ImplPtr->Lookup(XCEntryPtr->ResourceName, &XCStatePtr->ResourceId);
            }

            if (Status == CFE_SUCCESS)
            {
                /* resourceid cache is valid, read value */
                Status = ImplPtr->Read(XCStatePtr->ResourceId, &XCStatePtr->LastSample);
            }

            if (Status != CFE_SUCCESS)
            {
                /* this monitor is not readable - clear cache */
                XCStatePtr->ResourceId = CFE_RESOURCEID_UNDEFINED;
                XCStatePtr->LastSample = HS_INVALID_EXECOUNT;
                ++InactiveCount;
            }
        }
    }

    HS_AppData.InactiveExecMonitorCount = InactiveCount;
}
