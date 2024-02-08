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
 *   Contains CFS Health and Safety (HS) macros that run preprocessor checks
 *   on mission and platform configurable parameters
 */
#ifndef HS_VERIFY_H
#define HS_VERIFY_H

/*************************************************************************
 * Macro Definitions
 *************************************************************************/

/*
 * Maximum number execution counters
 */
#if HS_MAX_EXEC_CNT_SLOTS < 1
#error HS_MAX_EXEC_CNT_SLOTS cannot be less than 1
#elif HS_MAX_EXEC_CNT_SLOTS > 4294967295
#error HS_MAX_EXEC_CNT_SLOTS can not exceed 4294967295
#endif

/*
 * Maximum number of message actions
 */
#if HS_MAX_MSG_ACT_TYPES < 1
#error HS_MAX_MSG_ACT_TYPES cannot be less than 1
#elif HS_MAX_MSG_ACT_TYPES > 65531
#error HS_MAX_MSG_ACT_TYPES can not exceed 65531
#endif

/*
 * Maximum length of message actions
 */
#ifdef MESSAGE_FORMAT_IS_CCSDS
#if HS_MAX_MSG_ACT_SIZE < 7
#error HS_MAX_MSG_ACT_SIZE cannot be less than 7
#endif
#else
#if HS_MAX_MSG_ACT_SIZE < 1
#error HS_MAX_MSG_ACT_SIZE cannot be less than 1
#endif
#endif

#if HS_MAX_MSG_ACT_SIZE > CFE_MISSION_SB_MAX_SB_MSG_SIZE
#error HS_MAX_MSG_ACT_SIZE can not exceed CFE_MISSION_SB_MAX_SB_MSG_SIZE
#endif

/*
 * Maximum number of monitored applications
 */
#if HS_MAX_MONITORED_APPS < 1
#error HS_MAX_MONITORED_APPS cannot be less than 1
#elif HS_MAX_MONITORED_APPS > 4294967295
#error HS_MAX_MONITORED_APPS can not exceed 4294967295
#endif

/*
 * Maximum number of critical events
 */
#if HS_MAX_MONITORED_EVENTS < 1
#error HS_MAX_MONITORED_EVENTS cannot be less than 1
#elif HS_MAX_MONITORED_EVENTS > 4294967295
#error HS_MAX_MONITORED_EVENTS can not exceed 4294967295
#endif

/*
 * JPH 2015-06-29 - Removed check of Watchdog timer values
 *
 * This is not a valid check anymore, as the HS app does not have knowledge
 * of PSP Watchdog definitions
 */

/*
 * Post Processing Delay
 */
#if HS_POST_PROCESSING_DELAY < 0
#error HS_POST_PROCESSING_DELAY can not be less than 0
#elif HS_POST_PROCESSING_DELAY > 4294967295
#error HS_POST_PROCESSING_DELAY can not exceed 4294967295
#endif

/*
 * Wakeup Timeout
 */
#if (HS_WAKEUP_TIMEOUT < -1) || (HS_WAKEUP_TIMEOUT > 2147483647)
#error HS_WAKEUP_TIMEOUT not defined as a proper SB Timeout value
#endif

/*
 * CPU Aliveness Period
 */
#if HS_CPU_ALIVE_PERIOD < 1
#error HS_CPU_ALIVE_PERIOD cannot be less than 1
#elif HS_CPU_ALIVE_PERIOD > 4294967295
#error HS_CPU_ALIVE_PERIOD can not exceed 4294967295
#endif

/*
 * Maximum processor restart actions
 */
#if HS_MAX_RESTART_ACTIONS < 0
#error HS_MAX_RESTART_ACTIONS can not be less than 0
#elif HS_MAX_RESTART_ACTIONS > 65535
#error HS_MAX_RESTART_ACTIONS can not exceed 65535
#endif

/*
 * Pipe depths
 */
#if HS_CMD_PIPE_DEPTH < 1
#error HS_CMD_PIPE_DEPTH cannot be less than 1
#endif

/*
 * JPH 2015-06-29 - Removed check of:
 *  HS_CMD_PIPE_DEPTH > CFE_SB_MAX_PIPE_DEPTH
 *
 * This is not a valid check anymore, as the HS app does not have knowledge
 * of CFE_SB_MAX_PIPE_DEPTH.  But if the configuration violates this rule it will
 * show up as an obvious run-time error so the compile-time check is redundant.
 */

#if HS_EVENT_PIPE_DEPTH < 1
#error HS_EVENT_PIPE_DEPTH cannot be less than 1
#endif

/*
 * JPH 2015-06-29 - Removed check of:
 *  HS_EVENT_PIPE_DEPTH > CFE_SB_MAX_PIPE_DEPTH
 *
 * This is not a valid check anymore, as the HS app does not have knowledge
 * of CFE_SB_MAX_PIPE_DEPTH.  But if the configuration violates this rule it will
 * show up as an obvious run-time error so the compile-time check is redundant.
 */

#if HS_WAKEUP_PIPE_DEPTH < 1
#error HS_WAKEUP_PIPE_DEPTH cannot be less than 1
#endif

/*
 * JPH 2015-06-29 - Removed check of:
 *  HS_WAKEUP_PIPE_DEPTH > CFE_SB_MAX_PIPE_DEPTH
 *
 * This is not a valid check anymore, as the HS app does not have knowledge
 * of CFE_SB_MAX_PIPE_DEPTH.  But if the configuration violates this rule it will
 * show up as an obvious run-time error so the compile-time check is redundant.
 */

/*
 * Reset Task Delay
 */
#if HS_RESET_TASK_DELAY < 0
#error HS_RESET_TASK_DELAY can not be less than 0
#elif HS_RESET_TASK_DELAY > 4294967295
#error HS_RESET_TASK_DELAY can not exceed 4294967295
#endif

/*
 * Startup Sync Timeout
 */
#if HS_STARTUP_SYNC_TIMEOUT < 0
#error HS_STARTUP_SYNC_TIMEOUT can not be less than 0
#elif HS_STARTUP_SYNC_TIMEOUT > 4294967295
#error HS_STARTUP_SYNC_TIMEOUT can not exceed 4294967295
#endif

/*
 * Default Application Monitor State
 */
#if (HS_APPMON_DEFAULT_STATE != HS_STATE_DISABLED) && (HS_APPMON_DEFAULT_STATE != HS_STATE_ENABLED)
#error HS_APPMON_DEFAULT_STATE not defined as a supported enumerated type
#endif

/*
 * Default Event Monitor State
 */
#if (HS_EVENTMON_DEFAULT_STATE != HS_STATE_DISABLED) && (HS_EVENTMON_DEFAULT_STATE != HS_STATE_ENABLED)
#error HS_EVENTMON_DEFAULT_STATE not defined as a supported enumerated type
#endif

/*
 * Default Aliveness Indicator State
 */
#if (HS_ALIVENESS_DEFAULT_STATE != HS_STATE_DISABLED) && (HS_ALIVENESS_DEFAULT_STATE != HS_STATE_ENABLED)
#error HS_ALIVENESS_DEFAULT_STATE not defined as a supported enumerated type
#endif

/*
 * Default CPU Hogging Indicator State
 */
#if (HS_CPUHOG_DEFAULT_STATE != HS_STATE_DISABLED) && (HS_CPUHOG_DEFAULT_STATE != HS_STATE_ENABLED)
#error HS_CPUHOG_DEFAULT_STATE not defined as a supported enumerated type
#endif

/*
 * Total number of Utils per Interval
 */
#if HS_CPU_UTILIZATION_MAX < 1
#error HS_CPU_UTILIZATION_MAX cannot be less than 1
#elif HS_CPU_UTILIZATION_MAX > 4294967295
#error HS_CPU_UTILIZATION_MAX can not exceed 4294967295
#endif

/*
 * Hogging number of Utils per Interval
 */
#if HS_UTIL_PER_INTERVAL_HOGGING < 1
#error HS_UTIL_PER_INTERVAL_HOGGING cannot be less than 1
#elif HS_UTIL_PER_INTERVAL_HOGGING > HS_CPU_UTILIZATION_MAX
#error HS_UTIL_PER_INTERVAL_HOGGING can not exceed HS_CPU_UTILIZATION_MAX
#endif

/*
 * Hogging Timeout in Intervals
 */
#if HS_UTIL_HOGGING_TIMEOUT < 1
#error HS_UTIL_HOGGING_TIMEOUT cannot be less than 1
#elif HS_UTIL_HOGGING_TIMEOUT > 4294967295
#error HS_UTIL_HOGGING_TIMEOUT can not exceed 4294967295
#endif

/*
 * Utilization Peak Number of Intervals
 */
#if HS_UTIL_PEAK_NUM_INTERVAL < 1
#error HS_UTIL_PEAK_NUM_INTERVAL cannot be less than 1
#elif HS_UTIL_PEAK_NUM_INTERVAL > 4294967295
#error HS_UTIL_PEAK_NUM_INTERVAL can not exceed 4294967295
#endif

/*
 * Utilization Average Number of Intervals
 */
#if HS_UTIL_AVERAGE_NUM_INTERVAL < 1
#error HS_UTIL_AVERAGE_NUM_INTERVAL cannot be less than 1
#elif HS_UTIL_AVERAGE_NUM_INTERVAL > HS_UTIL_PEAK_NUM_INTERVAL
#error HS_UTIL_AVERAGE_NUM_INTERVAL can not exceed HS_UTIL_PEAK_NUM_INTERVAL
#endif

#ifndef HS_MISSION_REV
#error HS_MISSION_REV must be defined!
#elif (HS_MISSION_REV < 0)
#error HS_MISSION_REV must be greater than or equal to zero!
#endif

#endif
