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
 *   CFS Health and Safety (HS) Application Private Config Definitions
 *
 * This provides default values for configurable items that are internal
 * to this module and do NOT affect the interface(s) of this module.  Changes
 * to items in this file only affect the local module and will be transparent
 * to external entities that are using the public interface(s).
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef HS_INTERNAL_CFG_H
#define HS_INTERNAL_CFG_H

/**
 * \defgroup cfshsplatformcfg CFS Health and Safety Platform Configuration
 * \{
 */

/**
 * \brief Watchdog Timeout Value
 *
 *  \par Description:
 *       Number of milliseconds before a watchdog timeout occurs.
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 *
 *       This parameter must be greater than 0.
 */
#define HS_WATCHDOG_TIMEOUT_VALUE 10000

/**
 *  \brief Time to wait after performing processing (in milliseconds)
 *
 *  \par Description:
 *       Dictates the length of a task delay performed prior to checking
 *       the Software Bus for a Wakeup Message. This ensures that HS will
 *       run no more often than a certain rate. If this parameter is set to 0,
 *       no task delay will be performed. Time is in milliseconds.
 *
 *  \par Limits
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 */
#define HS_POST_PROCESSING_DELAY 0

/**
 *  \brief Wakeup Message Software Bus Timeout
 *
 *  \par Description:
 *       This parameter is passed into #CFE_SB_ReceiveBuffer as the timeout value.
 *       It can specify #CFE_SB_POLL, #CFE_SB_PEND_FOREVER, or a timeout
 *       value in milliseconds.
 *
 *  \par Limits
 *       This Parameter must be #CFE_SB_POLL, #CFE_SB_PEND_FOREVER,
 *       or greater than 0 and less than 2^31 - 1
 *
 *       As a timeout, this parameter should be less than
 *       (#HS_WATCHDOG_TIMEOUT_VALUE * 1000) - HS runtime in ms
 *       otherwise HS may not be able to service the watchdog in time.
 */
#define HS_WAKEUP_TIMEOUT 1200

/**
 * \brief CPU aliveness output string
 *
 *  \par Description:
 *       String that is output to via #OS_printf periodically if aliveness
 *       is enabled.
 *
 *  \par Limits:
 *       None.
 *
 */
#define HS_CPU_ALIVE_STRING "."

/**
 * \brief CPU aliveness output period
 *
 *  \par Description:
 *       Rate in number of HS cycles at which the HS_CPU_ALIVE_STRING
 *       is output via the UART.
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 *
 */
#define HS_CPU_ALIVE_PERIOD 5

/**
 * \brief Max Number of Processor Resets that may be performed by HS
 *
 *  \par Description:
 *       Maximum number of times that the HS App will attempt a processor
 *       reset as the result of either an Application Monitor or
 *       Event Monitor Failure
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 16 bit
 *       integer (65535).
 *
 *       Although not enforced, if this parameter is greater than or
 *       equal to #CFE_PLATFORM_ES_MAX_PROCESSOR_RESETS then a POWER-ON reset
 *       will occur before the max count is reached, resetting the remaining
 *       actions to the value set here.
 *
 */
#define HS_MAX_RESTART_ACTIONS 3

/**
 * \brief Software bus command pipe depth
 *
 *  \par Description:
 *       Depth of the software bus pipe HS uses for commands and HK requests.
 *       Used during initialization in the call to #CFE_SB_CreatePipe
 *
 *  \par Limits:
 *       This parameter must be greater than 0.
 */
#define HS_CMD_PIPE_DEPTH 12

/**
 * \brief Software bus event pipe depth
 *
 *  \par Description:
 *       Depth of the software bus pipe HS uses for event monitoring.
 *       This should be set to supply sufficient room for the expected event
 *       message load per second. Used during initialization in the call to
 *       #CFE_SB_CreatePipe
 *
 *  \par Limits:
 *       This parameter must be greater than 0.
 */
#define HS_EVENT_PIPE_DEPTH 32

/**
 * \brief Software bus wakeup pipe depth
 *
 *  \par Description:
 *       Depth of the software bus pipe HS uses for wakeup messages.
 *       Used during initialization in the call to #CFE_SB_CreatePipe
 *
 *  \par Limits:
 *       This parameter must be greater than 0.
 */
#define HS_WAKEUP_PIPE_DEPTH 1

/**
 *  \brief Time to wait before a processor reset (in milliseconds)
 *
 *  \par Description:
 *       Dictates the length of the task delay (milliseconds) performed prior
 *       to calling #CFE_ES_ResetCFE to allow for any event message to go out.
 *
 *  \par Limits
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 */
#define HS_RESET_TASK_DELAY 50

/**
 *  \brief Time to wait for all apps to be started (in milliseconds)
 *
 *  \par Description:
 *       Dictates the timeout for the #CFE_ES_WaitForStartupSync call that
 *       HS uses to wait for all of the Applications specified in the startup
 *       script to finish initialization. HS will wait this amount of time
 *       before assuming all startup script applications have been started and
 *       will then begin nominal processing.
 *
 *  \par Limits
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 *
 *       This should be greater than or equal to the Startup Sync timeout for
 *       any application in the Application Monitor Table.
 */
#define HS_STARTUP_SYNC_TIMEOUT 65000

/**
 * \brief Default State of the Application Monitor
 *
 *  \par Description:
 *       State the Application Monitor is set to when the HS
 *       application starts.
 *
 *  \par Limits:
 *       Must be HS_STATE_ENABLED or HS_STATE_DISABLED
 */
#define HS_APPMON_DEFAULT_STATE HS_STATE_ENABLED

/**
 * \brief Default State of the Event Monitor
 *
 *  \par Description:
 *       State the Event Monitor is set to when the HS
 *       application starts.
 *
 *  \par Limits:
 *       Must be HS_STATE_ENABLED or HS_STATE_DISABLED
 */
#define HS_EVENTMON_DEFAULT_STATE HS_STATE_ENABLED

/**
 * \brief Default State of the Aliveness Indicator
 *
 *  \par Description:
 *       State the Aliveness Indicator is set to when the HS application
 *       starts.
 *
 *  \par Limits:
 *       Must be HS_STATE_ENABLED or HS_STATE_DISABLED
 */
#define HS_ALIVENESS_DEFAULT_STATE HS_STATE_ENABLED

/**
 * \brief Default State of the CPU Hogging Indicator
 *
 *  \par Description:
 *       State the CPU Hogging Event Message is set to when the HS application
 *       starts.
 *
 *  \par Limits:
 *       Must be HS_STATE_ENABLED or HS_STATE_DISABLED
 */
#define HS_CPUHOG_DEFAULT_STATE HS_STATE_ENABLED

/**
 * \brief Application Monitor Table (AMT) filename
 *
 *  \par Description:
 *       Default file to load the Applications Monitor Table from
 *       during a power-on reset sequence
 *
 *  \par Limits:
 *       This string shouldn't be longer than #OS_MAX_PATH_LEN for the
 *       target platform in question
 */
#define HS_AMT_FILENAME "/cf/hs_amt.tbl"

/**
 * \brief Event Monitor Table (EMT) filename
 *
 *  \par Description:
 *       Default file to load the Event Monitor Table from
 *       during a power-on reset sequence
 *
 *  \par Limits:
 *       This string shouldn't be longer than #OS_MAX_PATH_LEN for the
 *       target platform in question
 */
#define HS_EMT_FILENAME "/cf/hs_emt.tbl"

/**
 * \brief Execution Counter Table (XCT) filename
 *
 *  \par Description:
 *       Default file to load the Execution Counters Table from
 *       during a power-on reset sequence
 *
 *  \par Limits:
 *       This string shouldn't be longer than #OS_MAX_PATH_LEN for the
 *       target platform in question
 */
#define HS_XCT_FILENAME "/cf/hs_xct.tbl"

/**
 * \brief Message Actions Table (MAT) filename
 *
 *  \par Description:
 *       Default file to load the Message Actions Table from
 *       during a power-on reset sequence
 *
 *  \par Limits:
 *       This string shouldn't be longer than #OS_MAX_PATH_LEN for the
 *       target platform in question
 */
#define HS_MAT_FILENAME "/cf/hs_mat.tbl"

/**
 * \brief System monitor PSP
 *
 *  \par Description:
 *       Variable to use for system monitor.  If
 *       the module name is not found, CPU monitoring
 *       will be disabled.
 *
 *  \par Limits:
 *       Must be a name of a module that is implemented by the PSP
 */
#define HS_SYSTEM_MONITOR_DEVICE "linux_sysmon"

/**
 * \brief CPU Utilization subsystem name
 *
 *  \par Description:
 *       Subsystem name to use for system CPU utilization monitor
 *       Reverts to 0 if the name is not found.
 *
 */
#define HS_SYSTEM_MONITOR_SUBSYSTEM_NAME "aggregate"

/**
 * \brief CPU Utilization subchannel name
 *
 *  \par Description:
 *       Subchannel name to use for system CPU utilization monitor
 *       Reverts to 0 if the name is not found.
 *
 */
#define HS_SYSTEM_MONITOR_SUBCHANNEL_NAME "cpu-load"

/**
 * \brief CPU Utilization Cycles per Interval
 *
 *  \par Description:
 *       Number of HS Cycles it takes to complete a CPU Utilization Interval.
 *       HS will monitor the utilization after this number of HS wakeup cycles.
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 *
 *       This interval should not be less than the collection period of the
 *       underlying PSP module, otherwise the same data will be reported.
 *
 */
#define HS_CPU_UTILIZATION_CYCLES_PER_INTERVAL 30

/**
 * \brief CPU Utilization Maximum Range
 *
 *  \par Description:
 *       Number of steps equal to full utilization. This allows for higher
 *       resolution than percentages, and non decimal based values.
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 */
#define HS_CPU_UTILIZATION_MAX 10000

/**
 * \brief CPU Utilization Hogging Utils Per Interval
 *
 *  \par Description:
 *       Number of Utils (counts) equal to utilization which is considered hogging
 *       during one interval. A greater number of counts is also considered hogging.
 *
 *  \par Limits:
 *       This parameter can't be larger than #HS_CPU_UTILIZATION_MAX.
 */
#define HS_UTIL_PER_INTERVAL_HOGGING 7500

/**
 * \brief CPU Utilization Hogging Timeout
 *
 *  \par Description:
 *       Number of intervals for which the hogging limit must be exceeded before hogging
 *       is reported.
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 */
#define HS_UTIL_HOGGING_TIMEOUT 5

/**
 * \brief CPU Peak Utilization Number of Intervals
 *
 *  \par Description:
 *       Number of intervals over which the peak utilization is determined.
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 *
 *       This parameter controls the size of the array which stores
 *       previously measured utilization values.
 */
#define HS_UTIL_PEAK_NUM_INTERVAL 64

/**
 * \brief CPU Average Utilization Number of Intervals
 *
 *  \par Description:
 *       Number of intervals over which the average utilization is computed.
 *
 *  \par Limits:
 *       This parameter can't be larger than #HS_UTIL_PEAK_NUM_INTERVAL .
 */
#define HS_UTIL_AVERAGE_NUM_INTERVAL 4

/**
 * \brief Mission specific version number for HS application
 *
 *  \par Description:
 *       An application version number consists of four parts:
 *       major version number, minor version number, revision
 *       number and mission specific revision number. The mission
 *       specific revision number is defined here and the other
 *       parts are defined in "hs_version.h".
 *
 *  \par Limits:
 *       Must be defined as a numeric value that is greater than
 *       or equal to zero.
 */
#define HS_MISSION_REV 0

/**\}*/

#endif
