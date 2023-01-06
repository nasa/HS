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
#include <cfe.h>

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

#endif
