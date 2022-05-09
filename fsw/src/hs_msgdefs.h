/************************************************************************
** File: hs_msgdefs.h
**
** NASA Docket No. GSC-18,476-1, and identified as "Core Flight System
** (cFS) Health and Safety (HS) Application version 2.3.2"
**
** Copyright © 2020 United States Government as represented by the
** Administrator of the National Aeronautics and Space Administration.
** All Rights Reserved.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** http://www.apache.org/licenses/LICENSE-2.0
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** Purpose:
**   Specification for the CFS Health and Safety (HS) command and telemetry
**   message constant definitions.
**
** Notes:
**   These Macro definitions have been put in this file (instead of
**   hs_msg.h) so this file can be included directly into ASIST build
**   test scripts. ASIST RDL files can accept C language #defines but
**   can't handle type definitions. As a result: DO NOT PUT ANY
**   TYPEDEFS OR STRUCTURE DEFINITIONS IN THIS FILE!
**   ADD THEM TO hs_msg.h IF NEEDED!
**
**
*************************************************************************/
#ifndef _hs_msgdefs_h_
#define _hs_msgdefs_h_

/************************************************************************
** Macro Definitions
*************************************************************************/
/**
** \name HS Switch States (AppMon, EventMon, Aliveness) */
/** \{ */
#define HS_STATE_DISABLED 0
#define HS_STATE_ENABLED  1
/** \} */

/**
** \name HS Internal Status Flags */
/** \{ */
#define HS_LOADED_XCT 0x01
#define HS_LOADED_MAT 0x02
#define HS_LOADED_AMT 0x04
#define HS_LOADED_EMT 0x08
#define HS_CDS_IN_USE 0x10
/** \} */

/**
** \name HS Invalid Execution Counter */
/** \{ */
#define HS_INVALID_EXECOUNT 0xFFFFFFFF
/** \} */

/** \hscmd Noop
**
**  \par Description
**       Implements the Noop command that insures the HS task is alive
**
**  \hscmdmnemonic \HS_NOOP
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC - command counter will increment
**       - The #HS_NOOP_INF_EID informational event message will be
**         generated when the command is received
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_RESET_CC
*/
#define HS_NOOP_CC 0

/** \hscmd Reset Counters
**
**  \par Description
**       Resets the HS housekeeping counters
**
**  \hscmdmnemonic \HS_RESETCTRS
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC        - command counter will be cleared
**       - \b \c \HS_CMDEC        - command error counter will be cleared
**       - \b \c \HS_EVENTCOUNT   - events monitored counter will be cleared
**       - The #HS_RESET_DBG_EID debug event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_NOOP_CC
*/
#define HS_RESET_CC 1

/** \hscmd Enable Applications Monitor
**
**  \par Description
**       Enables the Applications Monitor
**
**  \hscmdmnemonic \HS_ENABLEAPPMON
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC       - command counter will increment
**       - \b \c \HS_APPMONSTAT  - will be set to Enabled
**       - The #HS_ENABLE_APPMON_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_DISABLE_APPMON_CC
*/
#define HS_ENABLE_APPMON_CC 2

/** \hscmd Disable Applications Monitor
**
**  \par Description
**       Disables the Applications Monitor
**
**  \hscmdmnemonic \HS_DISABLEAPPMON
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC       - command counter will increment
**       - \b \c \HS_APPMONSTAT  - will be set to Disabled
**       - The #HS_DISABLE_APPMON_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_ENABLE_APPMON_CC
*/
#define HS_DISABLE_APPMON_CC 3

/** \hscmd Enable Events Monitor
**
**  \par Description
**       Enables the Events Monitor
**
**  \hscmdmnemonic \HS_ENABLEEVENTMON
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC         - command counter will increment
**       - \b \c \HS_EVENTMONSTAT  - will be set to Enabled
**       - The #HS_ENABLE_EVENTMON_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_DISABLE_EVENTMON_CC
*/
#define HS_ENABLE_EVENTMON_CC 4

/** \hscmd Disable Events Monitor
**
**  \par Description
**       Disables the Events Monitor
**
**  \hscmdmnemonic \HS_DISABLEEVENTMON
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC         - command counter will increment
**       - \b \c \HS_EVENTMONSTAT  - will be set to Disabled
**       - The #HS_DISABLE_EVENTMON_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_ENABLE_EVENTMON_CC
*/
#define HS_DISABLE_EVENTMON_CC 5

/** \hscmd Enable Aliveness Indicator
**
**  \par Description
**       Enables the Aliveness Indicator UART output
**
**  \hscmdmnemonic \HS_ENABLEALIVENESS
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC          - command counter will increment
**       - \b \c \HS_ALIVENESSSTAT  - will be set to Enabled
**       - The #HS_ENABLE_ALIVENESS_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_DISABLE_ALIVENESS_CC
*/
#define HS_ENABLE_ALIVENESS_CC 6

/** \hscmd Disable Aliveness Indicator
**
**  \par Description
**       Disables the Aliveness Indicator UART output
**
**  \hscmdmnemonic \HS_DISABLEALIVENESS
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC          - command counter will increment
**       - \b \c \HS_ALIVENESSSTAT  - will be set to Disabled
**       - The #HS_DISABLE_ALIVENESS_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_ENABLE_ALIVENESS_CC
*/
#define HS_DISABLE_ALIVENESS_CC 7

/** \hscmd Reset Processor Resets Performed Count
**
**  \par Description
**       Resets the count of HS performed resets maintained by HS
**
**  \hscmdmnemonic \HS_RESETRESETS
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC          - command counter will increment
**       - \b \c \HS_PROCRESETCNT   - will be set to 0
**       - The #HS_RESET_RESETS_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_SET_MAX_RESETS_CC
*/
#define HS_RESET_RESETS_PERFORMED_CC 8

/** \hscmd Set Max Processor Resets Performed Count
**
**  \par Description
**       Sets the max allowable count of processor resets to the provided value
**
**  \hscmdmnemonic \HS_SETMAXRESETS
**
**  \par Command Structure
**       #HS_SetMaxResetsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC          - command counter will increment
**       - \b \c \HS_MAXRESETS      - will be set to the provided value
**       - The #HS_SET_MAX_RESETS_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_RESET_RESETS_PERFORMED_CC
*/
#define HS_SET_MAX_RESETS_CC 9

/** \hscmd Enable CPU Hogging Indicator
**
**  \par Description
**       Enables the CPU Hogging Indicator Event Message
**
**  \hscmdmnemonic \HS_ENABLECPUHOG
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC          - command counter will increment
**       - \b \c \HS_CPUHOGSTAT  - will be set to Enabled
**       - The #HS_ENABLE_CPUHOG_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_DISABLE_CPUHOG_CC
*/
#define HS_ENABLE_CPUHOG_CC 10

/** \hscmd Disable CPU Hogging Indicator
**
**  \par Description
**       Disables the CPU Hogging Indicator Event Message
**
**  \hscmdmnemonic \HS_DISABLECPUHOG
**
**  \par Command Structure
**       #HS_NoArgsCmd_t
**
**  \par Command Verification
**       Successful execution of this command may be verified with
**       the following telemetry:
**       - \b \c \HS_CMDPC          - command counter will increment
**       - \b \c \HS_CPUHOGSTAT  - will be set to Disabled
**       - The #HS_DISABLE_CPUHOG_DBG_EID informational event message will be
**         generated when the command is executed
**
**  \par Error Conditions
**       This command may fail for the following reason(s):
**       - Command packet length not as expected
**
**  \par Evidence of failure may be found in the following telemetry:
**       - \b \c \HS_CMDEC - command error counter will increment
**       - Error specific event message #HS_LEN_ERR_EID
**
**  \par Criticality
**       None
**
**  \sa #HS_ENABLE_CPUHOG_CC
*/
#define HS_DISABLE_CPUHOG_CC 11

#endif /* _hs_msgdefs_h_ */

/************************/
/*  End of File Comment */
/************************/
