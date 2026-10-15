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

/*
 * Includes
 */

#include "hs_exec_monitor.h"
#include "hs_sysmon.h"
#include "hs_test_utils.h"
#include "hs_msgids.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

#include <unistd.h>
#include <stdlib.h>
#include "cfe.h"
#include "cfe_msgids.h"
#include "cfe_evs_msg.h"

/*
 * Function Definitions
 */
void HS_TestHook_CFE_ES_GetTaskInfo(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    CFE_ES_TaskInfo_t *TaskInfo = UT_Hook_GetArgValueByName(Context, "TaskInfo", CFE_ES_TaskInfo_t *);

    if (TaskInfo)
    {
        TaskInfo->ExecutionCounter = 5;
    }
}

void HS_TestHook_CFE_ES_GetGenCount(void *UserObj, UT_EntryKey_t FuncKey, const UT_StubContext_t *Context)
{
    uint32 *Count = UT_Hook_GetArgValueByName(Context, "Count", uint32 *);

    if (Count)
    {
        *Count = 7;
    }
}

void HS_ExecMon_CheckUtilization_Test_HighCurrentUtil(void)
{
    HS_AppData.CurrentCPUUtilIndex = HS_UTIL_PEAK_NUM_INTERVAL - 2;

    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_CPU_UTILIZATION_MAX);

    /* Execute the function being tested */
    HS_ExecMon_CheckUtilization();

    /* Verify results */
    UtAssert_INT32_EQ(HS_AppData.UtilizationTracker[HS_AppData.CurrentCPUUtilIndex - 1], HS_CPU_UTILIZATION_MAX);
    /* For this test case, we don't care about any messages or variables changed after this is set */

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_ExecMon_CheckUtilization_Test_CurrentUtilLessThanZero(void)
{
    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, -1);

    /* Execute the function being tested */
    HS_ExecMon_CheckUtilization();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuAvg, 0xFFFFFFFF);
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuPeak, 0xFFFFFFFF);
}

void HS_ExecMon_CheckUtilization_Test_CPUHogging(void)
{
    HS_AppData.CurrentCPUHogState    = HS_State_ENABLED;
    HS_AppData.MaxCPUHoggingTime     = 1;
    HS_AppData.CurrentCPUHoggingTime = 0;
    HS_AppData.CurrentCPUUtilIndex   = HS_UTIL_PEAK_NUM_INTERVAL - 1;

    /* test below the hogging threshold */
    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_UTIL_PER_INTERVAL_HOGGING - 1);

    /* Execute the function being tested */
    HS_ExecMon_CheckUtilization();

    /* Verify results - no hogging or event reported here */
    UtAssert_ZERO(HS_AppData.CurrentCPUHoggingTime);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* now above the hogging threshold */
    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_UTIL_PER_INTERVAL_HOGGING + 1);

    /* Execute the function being tested */
    HS_ExecMon_CheckUtilization();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.CurrentCPUHoggingTime, 1);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_CPUMON_HOGGING_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);
}

void HS_ExecMon_CheckUtilization_Test_CPUHoggingNotMax(void)
{
    HS_AppData.CurrentCPUHogState = HS_State_ENABLED;
    HS_AppData.MaxCPUHoggingTime  = 2;

    HS_AppData.CurrentCPUUtilIndex = HS_UTIL_PEAK_NUM_INTERVAL - 1;

    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_CPU_UTILIZATION_MAX + 1);

    /* Execute the function being tested */
    HS_ExecMon_CheckUtilization();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentCPUHoggingTime == 1, "HS_AppData.CurrentCPUHoggingTime == 1");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* For this test case, we don't care about any variables changed after this message */
}

void HS_ExecMon_CheckUtilization_Test_CurrentCPUHogStateDisabled(void)
{
    HS_AppData.CurrentCPUHogState = HS_State_DISABLED;
    HS_AppData.MaxCPUHoggingTime  = 1;

    HS_AppData.CurrentCPUUtilIndex = HS_UTIL_PEAK_NUM_INTERVAL - 1;

    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, HS_CPU_UTILIZATION_MAX);

    /* Execute the function being tested */
    HS_ExecMon_CheckUtilization();

    /* Verify results */
    UtAssert_True(HS_AppData.CurrentCPUHoggingTime == 0, "HS_AppData.CurrentCPUHoggingTime == 0");

    /* For this test case, we don't care about any variables changed after this variable is set */

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_ExecMon_CheckUtilization_Test_HighUtilIndex(void)
{
    uint32 ExpectedUtil;

    HS_AppData.CurrentCPUHogState = HS_State_DISABLED;
    HS_AppData.MaxCPUHoggingTime  = 1;

    HS_AppData.CurrentCPUUtilIndex                               = HS_UTIL_PEAK_NUM_INTERVAL - 1;
    HS_AppData.UtilizationTracker[HS_UTIL_PEAK_NUM_INTERVAL / 2] = HS_CPU_UTILIZATION_MAX - 1;

    ExpectedUtil = (HS_CPU_UTILIZATION_MAX / 2);
    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, ExpectedUtil);
    ExpectedUtil /= HS_UTIL_AVERAGE_NUM_INTERVAL;

    /* Execute the function being tested */
    HS_ExecMon_CheckUtilization();

    /* Verify results */
    UtAssert_ZERO(HS_AppData.CurrentCPUHoggingTime);
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuAvg, ExpectedUtil);
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuPeak, HS_CPU_UTILIZATION_MAX - 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_ExecMon_CheckUtilization_Test_LowUtilIndex(void)
{
    uint32 ExpectedUtil;

    HS_AppData.CurrentCPUHogState = HS_State_DISABLED;
    HS_AppData.MaxCPUHoggingTime  = 1;

    HS_AppData.UtilizationTracker[HS_UTIL_PEAK_NUM_INTERVAL / 2] = HS_CPU_UTILIZATION_MAX - 1;
    HS_AppData.CurrentCPUUtilIndex                               = 1;

    ExpectedUtil = (HS_CPU_UTILIZATION_MAX / 2);
    UT_SetDeferredRetcode(UT_KEY(HS_SysMonGetCpuUtilization), 1, ExpectedUtil);

    ExpectedUtil /= HS_UTIL_AVERAGE_NUM_INTERVAL;

    /* Execute the function being tested */
    HS_ExecMon_CheckUtilization();

    /* Verify results */
    UtAssert_ZERO(HS_AppData.CurrentCPUHoggingTime);
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuAvg, ExpectedUtil);
    UtAssert_UINT32_EQ(HS_AppData.UtilCpuPeak, HS_CPU_UTILIZATION_MAX - 1);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_ExecMon_ValidateTable_Test_UnusedTableEntry(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_XCTEntry_t XCTable[HS_MAX_EXEC_CNT_SLOTS];

    memset(XCTable, 0, sizeof(XCTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.XCTablePtr = XCTable;

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        HS_AppData.XCTablePtr[i].ResourceType = HS_XCTResType_NOTYPE;
        HS_AppData.XCTablePtr[i].NullTerm     = 0;
    }

    /* Execute the function being tested */
    Result = HS_ExecMon_ValidateTable(HS_AppData.XCTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_ExecMon_ValidateTable_Test_BufferNotNull(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_XCTEntry_t XCTable[HS_MAX_EXEC_CNT_SLOTS];

    memset(XCTable, 0, sizeof(XCTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify err: Entry = %%d, Err = %%d, Type = %%d, Name = %%s");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.XCTablePtr = XCTable;

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        HS_AppData.XCTablePtr[i].ResourceType = 99;
        HS_AppData.XCTablePtr[i].NullTerm     = 1;
    }

    strncpy(HS_AppData.XCTablePtr[0].ResourceName, "ResourceName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_ExecMon_ValidateTable(HS_AppData.XCTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCTVal_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_XCTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_XCTVal_ERR_NUL, "Result == HS_XCTVal_ERR_NUL");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_ExecMon_ValidateTable_Test_ResourceTypeNotValid(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_XCTEntry_t XCTable[HS_MAX_EXEC_CNT_SLOTS];

    memset(XCTable, 0, sizeof(XCTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify err: Entry = %%d, Err = %%d, Type = %%d, Name = %%s");
    snprintf(ExpectedEventString[1],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.XCTablePtr = XCTable;

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        HS_AppData.XCTablePtr[i].ResourceType = 99;
        HS_AppData.XCTablePtr[i].NullTerm     = 0;
    }

    /* take all branches for "Resource Type is not valid" */
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCTResType_APP_MAIN;
    HS_AppData.XCTablePtr[1].ResourceType = HS_XCTResType_APP_CHILD;
    HS_AppData.XCTablePtr[2].ResourceType = HS_XCTResType_DEVICE;
    HS_AppData.XCTablePtr[3].ResourceType = HS_XCTResType_ISR;

    strncpy(HS_AppData.XCTablePtr[0].ResourceName, "ResourceName", OS_MAX_API_NAME);

    /* Execute the function being tested */
    Result = HS_ExecMon_ValidateTable(HS_AppData.XCTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCTVal_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventID, HS_XCTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[1].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[1], context_CFE_EVS_SendEvent[1].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[1].Spec);

    UtAssert_True(Result == HS_XCTVal_ERR_TYPE, "Result == HS_XCTVal_ERR_TYPE");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 2);
}

void HS_ExecMon_ValidateTable_Test_EntryGood(void)
{
    int32         Result;
    uint32        i;
    int32         strCmpResult;
    char          ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
    HS_XCTEntry_t XCTable[HS_MAX_EXEC_CNT_SLOTS];

    memset(XCTable, 0, sizeof(XCTable));

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "ExeCount verify results: good = %%d, bad = %%d, unused = %%d");

    HS_AppData.XCTablePtr = XCTable;

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        HS_AppData.XCTablePtr[i].ResourceType = HS_XCTResType_APP_MAIN;
        HS_AppData.XCTablePtr[i].NullTerm     = 0;
    }

    /* Execute the function being tested */
    Result = HS_ExecMon_ValidateTable(HS_AppData.XCTablePtr);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XCTVal_INF_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_INFORMATION);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == CFE_SUCCESS, "Result == CFE_SUCCESS");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_ExecMon_ValidateTable_Test_Null(void)
{
    int32 Result;
    int32 strCmpResult;
    char  ExpectedEventString[2][CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];

    snprintf(ExpectedEventString[0],
             CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
             "Error in XC Table Validation. Table is null.");

    /* Execute the function being tested */
    Result = HS_ExecMon_ValidateTable(NULL);

    /* Verify results */
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_XC_TBL_NULL_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    strCmpResult =
        strncmp(ExpectedEventString[0], context_CFE_EVS_SendEvent[0].Spec, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH);

    UtAssert_True(strCmpResult == 0, "Event string matched expected result, '%s'", context_CFE_EVS_SendEvent[0].Spec);

    UtAssert_True(Result == HS_TBL_VAL_ERR, "Result == HS_TBL_VAL_ERR");

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
}

void HS_ExecMon_GetCounters_Test(void)
{
    uint32              ExeCounts[HS_MAX_EXEC_CNT_SLOTS];
    int                 i;
    HS_ExecMon_State_t *XCStatePtr;

    memset(ExeCounts, 0xEE, sizeof(ExeCounts));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCStatePtr             = HS_GetXCStateByIndex(i);
        XCStatePtr->Enable     = (~i & 1);
        XCStatePtr->LastSample = (i + 1000);
        XCStatePtr->ResourceId = CFE_ResourceId_FromInteger(i + 10);
    }

    HS_ExecMon_GetCounters(ExeCounts);

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        if (i & 1)
        {
            UtAssert_UINT32_EQ(ExeCounts[i], HS_INVALID_EXECOUNT);
        }
        else
        {
            UtAssert_UINT32_EQ(ExeCounts[i], i + 1000);
        }
    }
}

void HS_ExecMon_CheckEntries_Test_XCTablePtrNull(void)
{
    HS_ExecMon_State_t *XCStatePtr;

    /*
    ** initialize app data to inject error scenario
    ** normally, XCTablePtr gets iniitalized during the table load (at app init)
    */
    HS_AppData.XCTablePtr    = NULL;
    HS_AppData.ExecMonLoaded = HS_State_ENABLED;

    /* For good measure, set one entry to be enabled.  This is not normally
     * possible if the table is not loaded, but it should still not access the
     * null table pointer */
    XCStatePtr         = HS_GetXCStateByIndex(0);
    XCStatePtr->Enable = true;

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    /* Check that these stubs weren't called */
    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskInfo, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCount, 0);

    UtAssert_ZERO(HS_AppData.InactiveExecMonitorCount);
}

void HS_ExecMon_CheckEntries_Test_XCTablePtrNullAndDisabled(void)
{
    HS_ExecMon_State_t *XCStatePtr;

    /*
    ** initialize app data to inject error scenario
    ** normally, XCTablePtr gets iniitalized during the table load (at app init)
    ** and when that's success
    */
    HS_AppData.XCTablePtr    = NULL;
    HS_AppData.ExecMonLoaded = HS_State_DISABLED;

    /* For good measure, set one entry to be enabled.  This is not normally
     * possible if the table is not loaded, but it should still not access the
     * null table pointer */
    XCStatePtr         = HS_GetXCStateByIndex(0);
    XCStatePtr->Enable = true;

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    /* Check that these stubs weren't called */
    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskInfo, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCount, 0);

    UtAssert_ZERO(HS_AppData.InactiveExecMonitorCount);
}

void HS_ExecMon_CheckEntries_Test_XCTablePtrNotNullAndDisabled(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    int                 i;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_APP_MAIN;
    }

    /*
    ** initialize app data to inject error scenario
    ** normally, XCTablePtr gets iniitalized during the table load (at app init)
    ** and when that's success
    */
    HS_AppData.XCTablePtr    = XCTable;
    HS_AppData.ExecMonLoaded = HS_State_DISABLED;

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    /*
    ** these values are zero from the test case setup
    */
    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCStatePtr = HS_GetXCStateByIndex(i);
        UtAssert_UINT32_EQ(XCStatePtr->LastSample, 0);
    }

    /* Check that these stubs weren't called */
    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskInfo, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCount, 0);

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 0);
}

void HS_ExecMon_CheckEntries_Test_ResourceTypeUnknown(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    int                 i;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    /* Set the XCTable Resource type to something invalid */
    XCTable[0].ResourceType = 0xAAAAAAA;
    XCStatePtr              = HS_GetXCStateByIndex(0);
    XCStatePtr->Enable      = true;
    XCStatePtr->ResourceId  = CFE_RESOURCEID_UNDEFINED;

    XCTable[1].ResourceType = 0xBBBBBBB;
    XCStatePtr              = HS_GetXCStateByIndex(1);
    XCStatePtr->Enable      = true;
    XCStatePtr->ResourceId  = CFE_ResourceId_FromInteger(1);

    HS_AppData.XCTablePtr    = XCTable;
    HS_AppData.ExecMonLoaded = HS_State_ENABLED;

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_HKREQ_RESOURCE_DBG_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_DEBUG);

    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskInfo, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCount, 0);

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCStatePtr = HS_GetXCStateByIndex(i);
        if (XCStatePtr->Enable)
        {
            UtAssert_UINT32_EQ(XCStatePtr->LastSample, HS_INVALID_EXECOUNT);
        }
    }

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 2);
}

void HS_ExecMon_CheckEntries_Test_ResourceTypeAppMain(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    CFE_ES_TaskInfo_t   TaskInfo;
    int                 i;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_APP_MAIN;
    }

    HS_AppData.XCTablePtr = XCTable;

    HS_AppData.ExecMonLoaded = HS_State_ENABLED;
    XCStatePtr               = HS_GetXCStateByIndex(0);
    XCStatePtr->Enable       = true;

    /* Causes line "Status = CFE_ES_GetTaskInfo(&TaskInfo, TaskId)" to be reached */
    UT_SetDeferredRetcode(UT_KEY(OS_TaskGetIdByName), 1, OS_SUCCESS);

    /* Sets TaskInfo.ExecutionCounter to 5, returns CFE_SUCCESS, goes to line "ExeCount = TaskInfo.ExecutionCounter" */
    TaskInfo.ExecutionCounter = 5;
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetTaskInfo), &TaskInfo, sizeof(TaskInfo), false);
    UT_SetHandlerFunction(UT_KEY(CFE_ES_GetTaskInfo), HS_TestHook_CFE_ES_GetTaskInfo, &TaskInfo);

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    /* Verify results */
    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 1);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskInfo, 1);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCount, 0);
    UtAssert_UINT32_EQ(XCStatePtr->LastSample, 5);

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 0);
}

void HS_ExecMon_CheckEntries_Test_ResourceTypeAppChild(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    CFE_ES_TaskInfo_t   TaskInfo;
    int                 i;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_APP_CHILD;
    }

    HS_AppData.XCTablePtr = XCTable;

    HS_AppData.ExecMonLoaded = HS_State_ENABLED;
    XCStatePtr               = HS_GetXCStateByIndex(0);
    XCStatePtr->Enable       = true;

    /* Causes line "Status = CFE_ES_GetTaskInfo(&TaskInfo, TaskId)" to be reached */
    UT_SetDeferredRetcode(UT_KEY(OS_TaskGetIdByName), 1, OS_SUCCESS);

    /* Sets TaskInfo.ExecutionCounter to 5, returns CFE_SUCCESS, goes to line "ExeCount = TaskInfo.ExecutionCounter" */
    TaskInfo.ExecutionCounter = 5;
    UT_SetDataBuffer(UT_KEY(CFE_ES_GetTaskInfo), &TaskInfo, sizeof(TaskInfo), false);
    UT_SetHandlerFunction(UT_KEY(CFE_ES_GetTaskInfo), HS_TestHook_CFE_ES_GetTaskInfo, &TaskInfo);

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 1);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskInfo, 1);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCount, 0);

    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_UINT32_EQ(XCStatePtr->LastSample, 5);

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 0);
}

void HS_ExecMon_CheckEntries_Test_ResourceTypeAppChildTaskInfoError(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    int                 i;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_APP_CHILD;
    }

    HS_AppData.XCTablePtr = XCTable;

    HS_AppData.ExecMonLoaded = HS_State_ENABLED;
    XCStatePtr               = HS_GetXCStateByIndex(0);
    XCStatePtr->Enable       = true;
    XCStatePtr               = HS_GetXCStateByIndex(1);
    XCStatePtr->Enable       = true;

    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetTaskIDByName), 1, -1);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetTaskInfo), 1, -1);

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 2);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskInfo, 1);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCount, 0);

    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_UINT32_EQ(XCStatePtr->LastSample, HS_INVALID_EXECOUNT);
    XCStatePtr = HS_GetXCStateByIndex(1);
    UtAssert_UINT32_EQ(XCStatePtr->LastSample, HS_INVALID_EXECOUNT);

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 2);
}

void HS_ExecMon_CheckEntries_Test_ResourceTypeCounterError(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    int                 i;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_DEVICE;
    }

    HS_AppData.XCTablePtr = XCTable;

    HS_AppData.ExecMonLoaded = HS_State_ENABLED;
    XCStatePtr               = HS_GetXCStateByIndex(0);
    XCStatePtr->Enable       = true;
    XCStatePtr               = HS_GetXCStateByIndex(1);
    XCStatePtr->Enable       = true;

    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetGenCounterIDByName), 1, -1);
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetGenCount), 1, -1);

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 2);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskInfo, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCount, 1);

    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_UINT32_EQ(XCStatePtr->LastSample, HS_INVALID_EXECOUNT);
    XCStatePtr = HS_GetXCStateByIndex(1);
    UtAssert_UINT32_EQ(XCStatePtr->LastSample, HS_INVALID_EXECOUNT);

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 2);
}

void HS_ExecMon_CheckEntries_Test_ResourceTypeDevice(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    int                 i;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_DEVICE;
    }

    HS_AppData.XCTablePtr = XCTable;

    HS_AppData.ExecMonLoaded = HS_State_ENABLED;
    XCStatePtr               = HS_GetXCStateByIndex(0);
    XCStatePtr->Enable       = true;

    /* Sets Counter to 7, returns CFE_SUCCESS */
    UT_SetHandlerFunction(UT_KEY(CFE_ES_GetGenCount), HS_TestHook_CFE_ES_GetGenCount, NULL);

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 1);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 0);

    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_UINT32_EQ(XCStatePtr->LastSample, 7);

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 0);
}

void HS_ExecMon_CheckEntries_Test_ResourceTypeISR(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    int                 i;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_ISR;
    }

    HS_AppData.XCTablePtr = XCTable;

    HS_AppData.ExecMonLoaded = HS_State_ENABLED;
    XCStatePtr               = HS_GetXCStateByIndex(0);
    XCStatePtr->Enable       = true;

    /* Sets Counter to 7, returns CFE_SUCCESS */
    UT_SetHandlerFunction(UT_KEY(CFE_ES_GetGenCount), HS_TestHook_CFE_ES_GetGenCount, NULL);

    /* Execute the function being tested */
    HS_ExecMon_CheckEntries();

    /* Verify results */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
    UtAssert_STUB_COUNT(CFE_ES_GetGenCounterIDByName, 1);
    UtAssert_STUB_COUNT(CFE_ES_GetTaskIDByName, 0);

    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_UINT32_EQ(XCStatePtr->LastSample, 7);

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 0);
}

void HS_ExecMon_StatusRefresh_Test_ResourceTypeAppChildTaskIdError(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    HS_ExecMon_State_t *XCStatePtr;
    int                 i;

    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_APP_CHILD;
    }

    HS_AppData.XCTablePtr = XCTable;

    HS_AppData.ExecMonLoaded              = HS_State_ENABLED;
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCTResType_APP_MAIN;

    /* Causes line "Status = CFE_ES_GetTaskInfo(&TaskInfo, TaskId)" to be skipped */
    UT_SetDeferredRetcode(UT_KEY(CFE_ES_GetTaskIDByName), 2, -1);

    /* Execute the function being tested */
    HS_ExecMon_StatusRefresh();

    /* Verify results */
    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_BOOL_TRUE(XCStatePtr->Enable);
    XCStatePtr = HS_GetXCStateByIndex(1);
    UtAssert_BOOL_TRUE(XCStatePtr->Enable);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_ExecMon_StatusRefresh_Test_ResourceTypeISRGenCounterError(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    HS_ExecMon_State_t *XCStatePtr;
    int                 i;

    memset(XCTable, 0, sizeof(XCTable));

    for (i = 0; i < HS_MAX_EXEC_CNT_SLOTS; i++)
    {
        XCTable[i].ResourceType = HS_XCTResType_ISR;
    }

    HS_AppData.XCTablePtr = XCTable;

    HS_AppData.ExecMonLoaded              = HS_State_ENABLED;
    HS_AppData.XCTablePtr[0].ResourceType = HS_XCTResType_DEVICE;

    /* Execute the function being tested */
    HS_ExecMon_StatusRefresh();

    /* Verify results */
    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_BOOL_TRUE(XCStatePtr->Enable);
    XCStatePtr = HS_GetXCStateByIndex(1);
    UtAssert_BOOL_TRUE(XCStatePtr->Enable);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);
}

void HS_ExecMon_AcquirePointers_Test_NominalNotLoaded(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    HS_XCTEntry_t      *XCTablePtr = XCTable;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    XCTable[0].ResourceType = HS_XCTResType_DEVICE;
    XCTable[1].ResourceType = HS_XCTResType_APP_CHILD;

    HS_AppData.XCTablePtr = NULL;

    /* Satisfies all instances of (Status == CFE_TBL_INFO_UPDATED), skips all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &XCTablePtr, sizeof(XCTablePtr), false);

    /* Execute the function being tested */
    HS_ExecMon_AcquirePointers();

    /* Verify results */
    UtAssert_ADDRESS_EQ(HS_AppData.XCTablePtr, XCTablePtr);
    UtAssert_UINT32_EQ(HS_AppData.ExecMonLoaded, HS_State_ENABLED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* Verify results */
    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_BOOL_TRUE(XCStatePtr->Enable);
    XCStatePtr = HS_GetXCStateByIndex(1);
    UtAssert_BOOL_TRUE(XCStatePtr->Enable);
    XCStatePtr = HS_GetXCStateByIndex(HS_MAX_EXEC_CNT_SLOTS - 1);
    UtAssert_BOOL_FALSE(XCStatePtr->Enable);
}

void HS_ExecMon_AcquirePointers_Test_NominalUpdate(void)
{
    HS_XCTEntry_t       XCTable[HS_MAX_EXEC_CNT_SLOTS];
    HS_XCTEntry_t      *XCTablePtr = XCTable;
    HS_ExecMon_State_t *XCStatePtr;

    memset(XCTable, 0, sizeof(XCTable));

    XCTable[0].ResourceType = HS_XCTResType_ISR;
    XCTable[1].ResourceType = HS_XCTResType_APP_MAIN;

    HS_AppData.XCTablePtr    = XCTablePtr;
    HS_AppData.ExecMonLoaded = HS_State_ENABLED;

    /* Satisfies all instances of (Status == CFE_TBL_INFO_UPDATED), skips all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), CFE_TBL_INFO_UPDATED);
    UT_SetDataBuffer(UT_KEY(CFE_TBL_GetAddress), &XCTablePtr, sizeof(XCTablePtr), false);

    /* Execute the function being tested */
    HS_ExecMon_AcquirePointers();

    /* Verify results */
    UtAssert_ADDRESS_EQ(HS_AppData.XCTablePtr, XCTablePtr);
    UtAssert_UINT32_EQ(HS_AppData.ExecMonLoaded, HS_State_ENABLED);
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    /* Verify results */
    XCStatePtr = HS_GetXCStateByIndex(0);
    UtAssert_BOOL_TRUE(XCStatePtr->Enable);
    XCStatePtr = HS_GetXCStateByIndex(1);
    UtAssert_BOOL_TRUE(XCStatePtr->Enable);
    XCStatePtr = HS_GetXCStateByIndex(HS_MAX_EXEC_CNT_SLOTS - 1);
    UtAssert_BOOL_FALSE(XCStatePtr->Enable);
}

void HS_ExecMon_AcquirePointers_Test_ErrorsWithEnabled(void)
{
    HS_XCTEntry_t  XCTable[HS_MAX_EXEC_CNT_SLOTS];
    HS_XCTEntry_t *XCTablePtr = XCTable;

    memset(XCTable, 0, sizeof(XCTable));

    XCTable[0].ResourceType = HS_XCTResType_ISR;
    XCTable[1].ResourceType = HS_XCTResType_APP_MAIN;

    HS_AppData.XCTablePtr    = XCTablePtr;
    HS_AppData.ExecMonLoaded = HS_State_ENABLED;

    /* Causes to enter all (Status < CFE_SUCCESS) blocks */
    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_ExecMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.ExecMonLoaded, HS_State_DISABLED);

    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 1);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventID, HS_EXECOUNT_GETADDR_ERR_EID);
    UtAssert_INT32_EQ(context_CFE_EVS_SendEvent[0].EventType, CFE_EVS_EventType_ERROR);

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 0);
}

void HS_ExecMon_AcquirePointers_Test_ErrorsWithDisabled(void)
{
    HS_AppData.XCTablePtr    = NULL;
    HS_AppData.ExecMonLoaded = HS_State_DISABLED;

    UT_SetDefaultReturnValue(UT_KEY(CFE_TBL_GetAddress), -1);

    /* Execute the function being tested */
    HS_ExecMon_AcquirePointers();

    /* Verify results */
    UtAssert_UINT32_EQ(HS_AppData.ExecMonLoaded, HS_State_DISABLED);

    /* event only sent if it _was_ enabled before */
    UtAssert_STUB_COUNT(CFE_EVS_SendEvent, 0);

    UtAssert_UINT32_EQ(HS_AppData.InactiveExecMonitorCount, 0);
}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    UtTest_Add(HS_ExecMon_CheckUtilization_Test_HighCurrentUtil,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckUtilization_Test_HighCurrentUtil");
    UtTest_Add(HS_ExecMon_CheckUtilization_Test_CurrentUtilLessThanZero,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckUtilization_Test_CurrentUtilLessThanZero");
    UtTest_Add(HS_ExecMon_CheckUtilization_Test_CPUHogging,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckUtilization_Test_CPUHogging");
    UtTest_Add(HS_ExecMon_CheckUtilization_Test_CPUHoggingNotMax,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckUtilization_Test_CPUHoggingNotMax");
    UtTest_Add(HS_ExecMon_CheckUtilization_Test_CurrentCPUHogStateDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckUtilization_Test_CurrentCPUHogStateDisabled");
    UtTest_Add(HS_ExecMon_CheckUtilization_Test_HighUtilIndex,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckUtilization_Test_HighUtilIndex");
    UtTest_Add(HS_ExecMon_CheckUtilization_Test_LowUtilIndex,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckUtilization_Test_LowUtilIndex");

    UtTest_Add(HS_ExecMon_ValidateTable_Test_UnusedTableEntry,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_ValidateTable_Test_UnusedTableEntry");
    UtTest_Add(HS_ExecMon_ValidateTable_Test_BufferNotNull,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_ValidateTable_Test_BufferNotNull");
    UtTest_Add(HS_ExecMon_ValidateTable_Test_ResourceTypeNotValid,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_ValidateTable_Test_ResourceTypeNotValid");
    UtTest_Add(HS_ExecMon_ValidateTable_Test_EntryGood,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_ValidateTable_Test_EntryGood");
    UtTest_Add(HS_ExecMon_ValidateTable_Test_Null,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_ValidateTable_Test_Null");

    UtTest_Add(HS_ExecMon_GetCounters_Test, HS_Test_Setup, HS_Test_TearDown, "HS_ExecMon_GetCounters_Test");

    UtTest_Add(HS_ExecMon_CheckEntries_Test_XCTablePtrNull,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_XCTablePtrNull");
    UtTest_Add(HS_ExecMon_CheckEntries_Test_XCTablePtrNullAndDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_XCTablePtrNullAndDisabled");
    UtTest_Add(HS_ExecMon_CheckEntries_Test_XCTablePtrNotNullAndDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_XCTablePtrNotNullAndDisabled");
    UtTest_Add(HS_ExecMon_CheckEntries_Test_ResourceTypeAppMain,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_ResourceTypeAppMain");
    UtTest_Add(HS_ExecMon_CheckEntries_Test_ResourceTypeAppChild,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_ResourceTypeAppChild");
    UtTest_Add(HS_ExecMon_CheckEntries_Test_ResourceTypeAppChildTaskInfoError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_ResourceTypeAppChildTaskInfoError");
    UtTest_Add(HS_ExecMon_CheckEntries_Test_ResourceTypeDevice,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_ResourceTypeDevice");
    UtTest_Add(HS_ExecMon_CheckEntries_Test_ResourceTypeISR,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_ResourceTypeISR");
    UtTest_Add(HS_ExecMon_CheckEntries_Test_ResourceTypeCounterError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_ResourceTypeCounterError");
    UtTest_Add(HS_ExecMon_CheckEntries_Test_ResourceTypeUnknown,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_CheckEntries_Test_ResourceTypeUnknown");

    UtTest_Add(HS_ExecMon_StatusRefresh_Test_ResourceTypeAppChildTaskIdError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_StatusRefresh_Test_ResourceTypeAppChildTaskIdError");
    UtTest_Add(HS_ExecMon_StatusRefresh_Test_ResourceTypeISRGenCounterError,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_StatusRefresh_Test_ResourceTypeISRGenCounterError");

    UtTest_Add(HS_ExecMon_AcquirePointers_Test_NominalNotLoaded,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_AcquirePointers_Test_NominalNotLoaded");
    UtTest_Add(HS_ExecMon_AcquirePointers_Test_NominalUpdate,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_AcquirePointers_Test_NominalUpdate");
    UtTest_Add(HS_ExecMon_AcquirePointers_Test_ErrorsWithEnabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_AcquirePointers_Test_ErrorsWithEnabled");
    UtTest_Add(HS_ExecMon_AcquirePointers_Test_ErrorsWithDisabled,
               HS_Test_Setup,
               HS_Test_TearDown,
               "HS_ExecMon_AcquirePointers_Test_ErrorsWithDisabled");
}
