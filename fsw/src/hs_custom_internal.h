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
 *   Specification for the CFS Health and Safety (HS) mission specific
 *   custom function private interface
 */
#ifndef HS_CUSTOM_INTERNAL_H
#define HS_CUSTOM_INTERNAL_H

/*************************************************************************
 * Includes
 ************************************************************************/
#include "cfe.h"

/*************************************************************************
 * Custom Global Data Structure
 *************************************************************************/

/**
 * \brief HS custom global structure
 */
typedef struct
{
    int32 UtilMult1; /**< \brief CPU Utilization Conversion Factor Multiplication 1 */
    int32 UtilDiv;   /**< \brief CPU Utilization Conversion Factor Division */
    int32 UtilMult2; /**< \brief CPU Utilization Conversion Factor Multiplication 2 */

    uint32 UtilMask;                                  /**< \brief Mask for determining Idle Tick length */
    uint32 UtilArrayIndex;                            /**< \brief Index for determining where to write in Util Array */
    uint32 UtilArrayMask;                             /**< \brief Mask for determining where to write in Util Array */
    uint32 UtilArray[HS_UTIL_TIME_DIAG_ARRAY_LENGTH]; /**< \brief Array to store time stamps for determining idle tick
                                                         length */

    uint32 ThisIdleTaskExec;     /**< \brief Idle Task Exec Counter */
    uint32 LastIdleTaskExec;     /**< \brief Idle Task Exec Counter at Previous Interval */
    uint32 LastIdleTaskInterval; /**< \brief Idle Task Increments during Previous Interval */
    uint32 UtilCycleCounter;     /**< \brief Counter to determine when to monitor utilization */

    int32 UtilCallsPerMark; /**< \brief CPU Utilization Calls per mark */

    int32           IdleTaskRunStatus; /**< \brief HS Idle Task Run Status */
    CFE_ES_TaskId_t IdleTaskID;        /**< \brief HS Idle Task Task ID */
} HS_CustomData_t;

/**
 * \brief HS custom global
 */
extern HS_CustomData_t HS_CustomData;

/*************************************************************************
 * Exported Functions
 *************************************************************************/

/**
 * \brief Initialize things needed for CPU Monitoring
 *
 *  \par Description
 *       This function is intended to set up everything necessary for
 *       CPU Utilization Monitoring at application startup. Currently,
 *       this initializes the Idle Task, spawning the task itself,
 *       and creating a 1Hz sync callback to mark the idle time.
 *       It is called at the end of #HS_AppInit .
 *       It may be updated to include other initializations, or
 *       modifications to already set parameters.
 *
 *  \par Assumptions, External Events, and Notes:
 *       CFE_SUCCESS will be returned if all creation was performed
 *       properly.
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
int32 HS_CustomInit(void);

/**
 * \brief Clean up the functionality used for Utilization Monitoring
 *
 *  \par Description
 *       This function is intended to clean up everything necessary for
 *       CPU Utilization Monitoring at application termination. Currently,
 *       this terminates the Idle Task, deleting the task itself,
 *       and uncreating the 1Hz sync callback that marks the idle time.
 *       It is called at the end of #HS_AppMain if HS is exiting cleanly.
 *
 *  \par Assumptions, External Events, and Notes:
 *       Any resources that will not be cleaned up automatically be CFE
 *       need to be cleaned up in this function.
 */
void HS_CustomCleanup(void);

/**
 * \brief Stub function for Utilization Monitoring
 *
 *  \par Description
 *       This function is used as a passthrough to call #HS_MonitorUtilization
 *       but can be modified to monitor utilization differently.
 *       It is called during #HS_ProcessMain every HS cycle.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_CustomMonitorUtilization(void);

/**
 * \brief Stub function for Getting the Current Cycle Utilization
 *
 *  \par Description
 *       This function is used to inform the Monitor Utilization function
 *       of the current cycle utilization.
 *       It is called during #HS_MonitorUtilization and should return a
 *       value between 0 and #HS_UTIL_PER_INTERVAL_TOTAL.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \return Current cycle utilization
 */
int32 HS_CustomGetUtil(void);

/**
 * \brief Process Custom Commands
 *
 *  \par Description
 *       This function allows for hs_custom.c to define custom commands.
 *       It will be called for any command code not already allocated
 *       to a Health and Safety command. If a custom command is found,
 *       then it is responsible for incrementing the command processed
 *       or command error counter as appropriate.
 *
 *  \par Assumptions, External Events, and Notes:
 *       If a command is found, this function MUST return #CFE_SUCCESS,
 *       otherwise is must not return #CFE_SUCCESS
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
int32 HS_CustomCommands(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Task that increments counters
 *
 *  \par Description
 *       This child task is started by the HS initialization process. It
 *       runs at the lowest priority on the system, incrementing a counter
 *       when all other tasks are idle. This counter is used to determine
 *       CPU Hogging (by being non-zero each cycle) and Utilization.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_IdleTask(void);

/**
 * \brief Increment the CPU Utilization Tracker Counter
 *
 *  \par Description
 *       Utility function that increments the CPU Utilization tracking
 *       counter, called by Idle Task. This counter is used to determine
 *       both utilization and CPU Hogging.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \sa #HS_UtilizationMark
 */
void HS_UtilizationIncrement(void);

/**
 * \brief Mark the CPU Utilization Tracker Counter
 *
 *  \par Description
 *       Utility function that marks the CPU Utilization tracking
 *       counter while saving the previous value to a variable for use in
 *       calculating CPU Utilization and hogging.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \sa #HS_UtilizationIncrement
 */
void HS_UtilizationMark(void);

/**
 * \brief Mark Idle Time Callback from Time App
 *
 *  \par Description
 *       This function marks the idle time for the current interval.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_MarkIdleCallback(void);

/**
 * \brief Report Utilization Diagnostics information
 *
 *  \par Description
 *       This function reports the Utilization Diagnostics data.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_UtilDiagReport(void);

/**
 * \brief Set Utilization Parameters
 *
 *  \par Description
 *       This function sets the Utilization Parameters.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 */
void HS_SetUtilParamsCmd(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Set Utilization Diagnostics Parameter
 *
 *  \par Description
 *       This function sets the utilization diagnostics parameter.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 */
void HS_SetUtilDiagCmd(const CFE_SB_Buffer_t *BufPtr);

#endif
