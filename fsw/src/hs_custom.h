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
 *   custom function interface
 */
#ifndef HS_CUSTOM_H
#define HS_CUSTOM_H

/*************************************************************************
 * Includes
 ************************************************************************/
#include "cfe.h"

/*************************************************************************
 * Constants
 ************************************************************************/
#define HS_UTIL_DIAG_REPORTS 4

/**
 * \ingroup cfshscmdcodes
 *
 * Custom command codes must not conflict with those in hs_msgdefs.h
 * \{
 */

/**
 * \brief Report Util Diagnostics
 *
 *  \par Description
 *       Reports the Utilization Diagnostics
 *
 *  \par Command Structure
 *       #HS_NoArgsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #HS_HkPacket_t.CmdCount will increment
 *       - The #HS_UTIL_DIAG_REPORT_EID informational event message will be
 *         generated when the command is executed
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #HS_HkPacket_t.CmdErrCount will increment
 *
 *  \par Criticality
 *       None
 */
#define HS_REPORT_DIAG_CC 12

/**
 * \brief Set Utilization Calibration Parameters
 *
 *  \par Description
 *       Sets the Utilization Calibration Parameter
 *
 *  \par Command Structure
 *       #HS_SetUtilParamsCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #HS_HkPacket_t.CmdCount will increment
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *       - Any parameter is set to 0.
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #HS_HkPacket_t.CmdErrCount will increment
 *
 *  \par Criticality
 *       None
 */
#define HS_SET_UTIL_PARAMS_CC 13

/**
 * \brief Set Utilization Diagnostics Parameter
 *
 *  \par Description
 *       Sets the Utilization Diagnostics parameter
 *
 *  \par Command Structure
 *       #HS_SetUtilDiagCmd_t
 *
 *  \par Command Verification
 *       Successful execution of this command may be verified with
 *       the following telemetry:
 *       - #HS_HkPacket_t.CmdCount will increment
 *
 *  \par Error Conditions
 *       This command may fail for the following reason(s):
 *       - Command packet length not as expected
 *
 *  \par Evidence of failure may be found in the following telemetry:
 *       - #HS_HkPacket_t.CmdErrCount will increment
 *
 *  \par Criticality
 *       None
 */
#define HS_SET_UTIL_DIAG_CC 14

/**\}*/

/**
 * \ingroup cfshsevents
 *
 * Custom Event IDs must not conflict with those in hs_events.h
 * \{
 */

/**
 * \brief HS CPU Utilization Monitoring Create Child Task Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety
 *  is unable to create its child task via the #CFE_ES_CreateChildTask
 *  API
 */
#define HS_CR_CHILD_TASK_ERR_EID 101

/**
 * \brief HS CPU Utilization Monitoring Register Sync Callback Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety
 *  is unable to create its sync callback via the #CFE_TIME_RegisterSynchCallback
 *  API
 */
#define HS_CR_SYNC_CALLBACK_ERR_EID 102

/**
 * \brief HS Report Diagnostics Command Event ID
 *
 *  \par Type: INFORMATION
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety receives the #HS_REPORT_DIAG_CC command.
 */
#define HS_UTIL_DIAG_REPORT_EID 103

/**
 * \brief HS Set Utilization Paramaters Command Event ID
 *
 *  \par Type: Debug
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety successfully processes the #HS_SET_UTIL_PARAMS_CC command.
 */
#define HS_SET_UTIL_PARAMS_DBG_EID 104

/**
 * \brief HS Set Utilization Parameters Zero Value Event ID
 *
 *  \par Type: Error
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety fails to processes the #HS_SET_UTIL_PARAMS_CC command.
 *  due to a 0 as at least one of the parameters.
 */
#define HS_SET_UTIL_PARAMS_ERR_EID 105

/**
 * \brief HS Set Utilization Diagnostics Command Event ID
 *
 *  \par Type: Debug
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety successfully processes the #HS_SET_UTIL_DIAG_CC command.
 */
#define HS_SET_UTIL_DIAG_DBG_EID 106

/**\}*/

/*************************************************************************
 * Command Structure Definitions
 ************************************************************************/

/**
 * \ingroup cfshscmdstructs
 * \{
 */

/**
 * \brief Set Utilitiliztion Parameters Command
 *
 * See #HS_SET_UTIL_PARAMS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    int32 Mult1; /**< \brief Multiplier 1 parameter */
    int32 Div;   /**< \brief Divisior parameter */
    int32 Mult2; /**< \brief Multiplier 2 parameter */
} HS_SetUtilParamsCmd_t;

/**
 * \brief Set Utilization Diagnostics Command
 *
 * See #HS_SET_UTIL_DIAG_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */

    uint32 Mask; /**< \brief Utilization Diagnostics Mask */
} HS_SetUtilDiagCmd_t;

/**\}*/

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
 * \brief Set Utilization Paramters
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
 * \brief Set Utilization Diagnostics Paramater
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
