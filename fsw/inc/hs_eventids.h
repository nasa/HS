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
 *   Specification for the CFS Health and Safety (HS) event identifiers.
 */
#ifndef HS_EVENTS_H
#define HS_EVENTS_H

/**
 * \defgroup cfshsevents CFS Health and Safety Event IDs
 * \{
 */

/**
 * \brief HS Initialization Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when the CFS Health and Safety has
 *  completed initialization.
 */
#define HS_INIT_EID 1

/**
 * \brief HS Application Fatal Termination Event ID
 *
 *  \par Type: CRITICAL
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety
 *  exits due to a fatal error condition.
 */
#define HS_APP_EXIT_EID 2

/**
 * \brief HS CDS Restore Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety
 *  is unable to restore data from its critical data store
 */
#define HS_CDS_RESTORE_ERR_EID 3

/**
 * \brief HS Create Software Bus Command Pipe Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety
 *  is unable to create its command pipe via the #CFE_SB_CreatePipe
 *  API
 */
#define HS_CR_CMD_PIPE_ERR_EID 4

/**
 * \brief HS Create Software Bus Event Pipe Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety
 *  is unable to create its event pipe via the #CFE_SB_CreatePipe
 *  API
 */
#define HS_CR_EVENT_PIPE_ERR_EID 5

/**
 * \brief HS Create Software Bus Wakeup Pipe Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety
 *  is unable to create its wakeup pipe via the #CFE_SB_CreatePipe
 *  API
 */
#define HS_CR_WAKEUP_PIPE_ERR_EID 6

/**
 * \brief HS Initialization Subscribe To Long Events Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the call to #CFE_SB_Subscribe
 *  for the #CFE_EVS_LONG_EVENT_MSG_MID, during initialization returns
 *  a value other than CFE_SUCCESS
 */
#define HS_SUB_LONG_EVS_ERR_EID 7

/**
 * \brief HS Subscribe To Housekeeping Request Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the call to #CFE_SB_Subscribe
 *  for the #HS_SEND_HK_MID, during initialization returns
 *  a value other than CFE_SUCCESS
 */
#define HS_SUB_REQ_ERR_EID 8

/**
 * \brief HS Subscribe To Ground Commands Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the call to #CFE_SB_Subscribe
 *  for the #HS_CMD_MID, during initialization returns a value
 *  other than CFE_SUCCESS
 */
#define HS_SUB_CMD_ERR_EID 9

/**
 * \brief HS AppMon Table Register Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when Health and Safety is unable to register its
 *  Application Monitor Table with cFE Table Services via the #CFE_TBL_Register API.
 */
#define HS_AMT_REG_ERR_EID 10

/**
 * \brief HS EventMon Table Register Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when Health and Safety is unable to register its
 *  Event Monitor Table with cFE Table Services via the #CFE_TBL_Register API.
 */
#define HS_EMT_REG_ERR_EID 11

/**
 * \brief HS ExeCount Table Register Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when Health and Safety is unable to register its
 *  Execution Counters Table with cFE Table Services via the #CFE_TBL_Register API.
 */
#define HS_XCT_REG_ERR_EID 12

/**
 * \brief HS MsgActs Table Register Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when Health and Safety is unable to register its
 *  Message Actions Table with cFE Table Services via the #CFE_TBL_Register API.
 */
#define HS_MAT_REG_ERR_EID 13

/**
 * \brief HS AppMon Table Load Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the call to CFE_TBL_Load
 *  for the application monitor table returns a value other than CFE_SUCCESS
 */
#define HS_AMT_LD_ERR_EID 14

/**
 * \brief HS EventMon Table Load Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the call to CFE_TBL_Load
 *  for the event monitor table returns a value other than CFE_SUCCESS
 */
#define HS_EMT_LD_ERR_EID 15

/**
 * \brief HS ExeCount Table Load Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the call to CFE_TBL_Load
 *  for the execution counters table returns a value other than CFE_SUCCESS
 */
#define HS_XCT_LD_ERR_EID 16

/**
 * \brief HS MsgActs Table Load Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the call to CFE_TBL_Load
 *  for the message actions table returns a value other than CFE_SUCCESS
 */
#define HS_MAT_LD_ERR_EID 17

/**
 * \brief HS CDS Data Corrupt Resetting Data Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when CFS Health and Safety
 *  restores data from the CDS that does not pass validation
 */
#define HS_CDS_CORRUPT_ERR_EID 18

/**
 * \brief HS Command Code Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a software bus message is received
 *  with an invalid command code.
 */
#define HS_CC_ERR_EID 19

/**
 * \brief HS Command Pipe Message ID Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a software bus message is received
 *  with an invalid message ID.
 */
#define HS_MID_ERR_EID 20

/**
 * \brief HS Housekeeping Request Message Length Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a housekeeping request is received
 *  with a message length that doesn't match the expected value.
 */
#define HS_HKREQ_LEN_ERR_EID 21

/**
 * \brief HS Ground Command Length Invalid Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a ground command message is received
 *  with a message length that doesn't match the expected value.
 */
#define HS_LEN_ERR_EID 22

/**
 * \brief HS No-op Command Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a NOOP command has been received.
 */
#define HS_NOOP_INF_EID 23

/**
 * \brief HS Reset Counters Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when a reset counters command has
 *  been received.
 */
#define HS_RESET_DBG_EID 24

/**
 * \brief HS Enable Application Monitoring Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when an enable application monitoring
 *  command has been received.
 */
#define HS_ENABLE_APPMON_DBG_EID 25

/**
 * \brief HS Disable Application Monitoring Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when a disable application monitoring
 *  command has been received.
 */
#define HS_DISABLE_APPMON_DBG_EID 26

/**
 * \brief HS Enable Event Monitoring Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when an enable event monitoring
 *  command has been received.
 */
#define HS_ENABLE_EVENTMON_DBG_EID 27

/**
 * \brief HS Disable Event Monitoring Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when a disable event monitoring
 *  command has been received.
 */
#define HS_DISABLE_EVENTMON_DBG_EID 28

/**
 * \brief HS Enable Aliveness Indicator Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when an enable aliveness indicator
 *  command has been received.
 */
#define HS_ENABLE_ALIVENESS_DBG_EID 29

/**
 * \brief HS Disable Aliveness Indicator Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when a disable aliveness indicator
 *  command has been received.
 */
#define HS_DISABLE_ALIVENESS_DBG_EID 30

/**
 * \brief HS Reset Process Reset Counter Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when a reset processor resets count
 *  command has been received.
 */
#define HS_RESET_RESETS_DBG_EID 31

/**
 * \brief HS Set Max Resets Command Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when a set max processor resets
 *  command has been received.
 */
#define HS_SET_MAX_RESETS_DBG_EID 32

/**
 * \brief HS AppMon Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the address can't be obtained
 *  from table services for the applications monitor table.
 */
#define HS_APPMON_GETADDR_ERR_EID 33

/**
 * \brief HS EventMon Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the address can't be obtained
 *  from table services for the event monitor table.
 */
#define HS_EVENTMON_GETADDR_ERR_EID 34

/**
 * \brief HS ExeCount Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the address can't be obtained
 *  from table services for the execution counters table.
 */
#define HS_EXECOUNT_GETADDR_ERR_EID 35

/**
 * \brief HS MsgActs Table Get Address Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the address can't be obtained
 *  from table services for the message actions table.
 */
#define HS_MSGACTS_GETADDR_ERR_EID 36

/**
 * \brief HS Processor Reset Action Limit Reached No Reset Performed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the action specified by an Application or Event
 *  monitor that fails is Processor Reset, and no more Processor Resets are allowed.
 */
#define HS_RESET_LIMIT_ERR_EID 37

/**
 * \brief HS App Monitor App Name Not Found Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when an application name cannot be resolved
 *  into an application ID by the OS.  This event is sent the first time this
 *  error occurs in the HS_MonitorApplications function.  Subsequent
 *  occurrences are captured with a corresponding debug event.
 */
#define HS_APPMON_APPNAME_ERR_EID 38

/**
 * \brief HS App Monitor Application Restart Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a monitored application fails to increment its
 *  execution counter in the table specified number of cycles, and the specified
 *  action type is Application Restart.
 */
#define HS_APPMON_RESTART_ERR_EID 39

/**
 * \brief HS App Monitor Application Restart Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the app monitor attempts to restart a task
 *  and is unable to.
 */
#define HS_APPMON_NOT_RESTARTED_ERR_EID 40

/**
 * \brief HS App Monitor Event Only Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a monitored application fails to increment its
 *  execution counter in the table specified number of cycles, and the specified
 *  action type is Event Only.
 */
#define HS_APPMON_FAIL_ERR_EID 41

/**
 * \brief HS App Monitor Processor Reset Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a monitored application fails to increment its
 *  execution counter in the table specified number of cycles, and the specified
 *  action type is Processor Reset.
 */
#define HS_APPMON_PROC_ERR_EID 42

/**
 * \brief HS App Monitor Message Action Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a monitored application fails to increment its
 *  execution counter in the table specified number of cycles, and the specified
 *  action type is a Message Action.
 */
#define HS_APPMON_MSGACTS_ERR_EID 43

/**
 * \brief HS Event Monitor Message Action Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a monitored event is detected, and the specified
 *  action type is a Message Action.
 */
#define HS_EVENTMON_MSGACTS_ERR_EID 44

/**
 * \brief HS Event Monitor Processor Reset Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when an event is received that matches an event in the
 *  event monitor table that specifies Processor Reset as the action type
 */
#define HS_EVENTMON_PROC_ERR_EID 45

/**
 * \brief HS Event Monitor Application Restart Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when an event is received that matches an event in the
 *  event monitor table that specifies Restart Application as the action type
 */
#define HS_EVENTMON_RESTART_ERR_EID 46

/**
 * \brief HS Event Monitor Application Restart Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the event monitor attempts to restart a task
 *  and is unable to.
 */
#define HS_EVENTMON_NOT_RESTARTED_ERR_EID 47

/**
 * \brief HS Event Monitor Application Delete Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when an event is received that matches an event in the
 *  event monitor table that specifies Delete Application as the action type
 */
#define HS_EVENTMON_DELETE_ERR_EID 48

/**
 * \brief HS Event Monitor Application Delete Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the event monitor attempts to delete a task
 *  and is unable to.
 */
#define HS_EVENTMON_NOT_DELETED_ERR_EID 49

/**
 * \brief HS AppMon Table Verification Results Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a table validation has been
 *  completed for an application monitor table load
 */
#define HS_AMTVAL_INF_EID 50

/**
 * \brief HS AppMon Table Verification Failed Results Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued on the first error when a table validation
 *  fails for an application monitor table load.
 */
#define HS_AMTVAL_ERR_EID 51

/**
 * \brief HS EventMon Table Verification Results Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a table validation has been
 *  completed for an event monitor table load
 */
#define HS_EMTVAL_INF_EID 52

/**
 * \brief HS EventMon Table Verification Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued on the first error when a table validation
 *  fails for an event monitor table load.
 */
#define HS_EMTVAL_ERR_EID 53

/**
 * \brief HS ExeCount Table Verification Results Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a table validation has been
 *  completed for an execution counters table load
 */
#define HS_XCTVAL_INF_EID 54

/**
 * \brief HS ExeCount Table Verification Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued on the first error when a table validation
 *  fails for an execution counter table load.
 */
#define HS_XCTVAL_ERR_EID 55

/**
 * \brief HS MsgActs Table Verification Results Event ID
 *
 *  \par Type: INFORMATIONAL
 *
 *  \par Cause:
 *
 *  This event message is issued when a table validation has been
 *  completed for a message actions table load
 */
#define HS_MATVAL_INF_EID 56

/**
 * \brief HS MsgActs Table Verification Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued on the first error when a table validation
 *  fails for a message actions table load.
 */
#define HS_MATVAL_ERR_EID 57

/**
 * \brief HS Application Monitoring Disabled Due To Table Load Failure Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when application monitoring has been disabled
 *  due to a table load failure.
 */
#define HS_DISABLE_APPMON_ERR_EID 58

/**
 * \brief HS Event Monitoring Disabled Due To Table Load Failure Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when event monitoring has been disabled
 *  due to a table load failure.
 */
#define HS_DISABLE_EVENTMON_ERR_EID 59

/**
 * \brief HS Subscribe To Wakeup Message Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the call to #CFE_SB_Subscribe
 *  for the #HS_WAKEUP_MID, during initialization returns a value
 *  other than CFE_SUCCESS
 */
#define HS_SUB_WAKEUP_ERR_EID 60

/**
 * \brief HS CPU Hogging Detected Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the CPU monitoring detects
 *  that the utilization has exceeded the CPU Hogging threshhold
 *  for longer than the CPU Hogging duration
 */
#define HS_CPUMON_HOGGING_ERR_EID 61

/**
 * \brief HS CPU Hogging Detection Enabled Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when an enable cpu hogging indicator
 *  command has been received.
 */
#define HS_ENABLE_CPUHOG_DBG_EID 64

/**
 * \brief HS CPU Hogging Detection Disabled Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when a disable cpu hogging indicator
 *  command has been received.
 */
#define HS_DISABLE_CPUHOG_DBG_EID 65

/**
 * \brief HS Event Monitor Enable Subscribe To Long Events Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a ground command message is received
 *  to enable event monitoring while it is disabled, and there is an error
 *  subscribing to the long-format event mid (#CFE_EVS_LONG_EVENT_MSG_MID).
 */
#define HS_EVENTMON_LONG_SUB_EID 66

/**
 * \brief HS Event Monitor Enable Subscribe to Short Events Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a ground command message is received
 *  to enable event monitoring while it is disabled, and there is an error
 *  subscribing to the short-format event mid (#CFE_EVS_SHORT_EVENT_MSG_MID).
 */
#define HS_EVENTMON_SHORT_SUB_EID 67

/**
 * \brief HS Event Monitor Disable Unsubscribe From Long Events Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a ground command message is received
 *  to disable event monitoring while it is enabled, and there is an error
 *  unsubscribing from the long-format event mid (#CFE_EVS_LONG_EVENT_MSG_MID).
 */
#define HS_EVENTMON_LONG_UNSUB_EID 68

/**
 * \brief HS Event Monitor Disable Unsubscribe From Short Events Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when a ground command message is received
 *  to disable event monitoring while it is enabled, and there is an error
 *  unsubscribing from the long-format event mid (#CFE_EVS_SHORT_EVENT_MSG_MID).
 */
#define HS_EVENTMON_SHORT_UNSUB_EID 69

/**
 * \brief HS EventMon Table Bad Unsubscribing From Long Events Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued if when acquiring the event monitor table
 *  from table services, it is bad and event monitoring is disabled, but
 *  there is a failure unsubscribing from the event mid.
 */
#define HS_BADEMT_LONG_UNSUB_EID 70

/**
 * \brief HS EventMon Table Bad Unsubscribing From Short Events Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued if when acquiring the event monitor table
 *  from table services, it is bad and event monitoring is disabled, but
 *  there is a failure unsubscribing from the event mid.
 */
#define HS_BADEMT_SHORT_UNSUB_EID 71

/**
 * \brief HS App Monitor App Name Not Found Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when an application name cannot be resolved
 *  into an application ID by the OS.  This event is sent if this error
 *  occurs repeatedly in the HS_MonitorApplications function.  The first such
 *  occurrence is captured with a corresponding error event.
 */
#define HS_APPMON_APPNAME_DBG_EID 72

/**
 * \brief HS Housekeeping Request Unknown Resource Event ID
 *
 *  \par Type: DEBUG
 *
 *  \par Cause:
 *
 *  This event message is issued when the application encounters a reource
 *  with an unknown type while populating the execution counters in the
 *  housekeeping telemetry packet.
 */
#define HS_HKREQ_RESOURCE_DBG_EID 73

/**
 * \brief HS Custom Initialization Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued if the HS_CustomInit routine returns
 *  any value other than CFE_SUCCESS.
 */
#define HS_SYSMON_INIT_ERR_EID 74

/**
 * \brief HS AppMon Table Validation Null Pointer Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued if the TableData pointer passed to
 *  HS_ValidateAMTable is null.
 */
#define HS_AM_TBL_NULL_ERR_EID 75

/**
 * \brief HS EventMon Table Validation Null Pointer Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued if the TableData pointer passed to
 *  HS_ValidateEMTable is null.
 */
#define HS_EM_TBL_NULL_ERR_EID 76

/**
 * \brief HS ExeCount Table Validation Null Pointer Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued if the TableData pointer passed to
 *  HS_ValidateXCTable is null.
 */
#define HS_XC_TBL_NULL_ERR_EID 77

/**
 * \brief HS MsgActs Table Validation Null Pointer Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued if the TableData pointer passed to
 *  HS_ValidateMATable is null.
 */
#define HS_MA_TBL_NULL_ERR_EID 78

/**
 * \brief HS Initialization Subscribe To Short Events Failed Event ID
 *
 *  \par Type: ERROR
 *
 *  \par Cause:
 *
 *  This event message is issued when the call to #CFE_SB_Subscribe
 *  for the #CFE_EVS_SHORT_EVENT_MSG_MID, during initialization returns
 *  a value other than CFE_SUCCESS
 */
#define HS_SUB_SHORT_EVS_ERR_EID 79

/**\}*/

#endif
