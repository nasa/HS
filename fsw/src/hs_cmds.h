/*************************************************************************
** File: hs_cmds.h 
**
** NASA Docket No. GSC-16,151-1, and identified as "Core Flight Software System (CFS)
** Health and Safety Application Version 2"
** 
** Copyright © 2007-2014 United States Government as represented by the
** Administrator of the National Aeronautics and Space Administration. All Rights
** Reserved. 
** 
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
** http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License. 
**
** Purpose:
**   Specification for the CFS Health and Safety (HS) routines that
**   handle command processing
**
** Notes:
**
**
**************************************************************************/
#ifndef _hs_cmds_h_
#define _hs_cmds_h_

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

/*************************************************************************
** Exported Functions
*************************************************************************/
/************************************************************************/
/** \brief Process a command pipe message
**
**  \par Description
**       Processes a single software bus command pipe message. Checks
**       the message and command IDs and calls the appropriate routine
**       to handle the message.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #CFE_SB_RcvMsg
**
*************************************************************************/
void HS_AppPipe(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Reset counters
**
**  \par Description
**       Utility function that resets housekeeping counters to zero
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \sa #HS_ResetCmd
**
*************************************************************************/
void HS_ResetCounters(void);

/************************************************************************/
/** \brief Verify message length
**
**  \par Description
**       Checks if the actual length of a software bus message matches
**       the expected length and sends an error event if a mismatch
**       occurs
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg              A #CFE_SB_MsgPtr_t pointer that
**                                 references the software bus message
**
**  \param [in]   ExpectedLength   The expected length of the message
**                                 based upon the command code
**
**  \returns
**  \retstmt Returns TRUE if the length is as expected      \endcode
**  \retstmt Returns FALSE if the length is not as expected \endcode
**  \endreturns
**
**  \sa #HS_LEN_ERR_EID
**
*************************************************************************/
boolean HS_VerifyMsgLength(CFE_SB_MsgPtr_t msg,
                           uint16          ExpectedLength);
/************************************************************************/
/** \brief Manages HS tables
**
**  \par Description
**       Manages load requests for the AppMon, EventMon, ExeCount and MsgActs
**       tables and update notification for the AppMon and MsgActs tables.
**       Also releases and acquires table addresses. Gets called at the start
**       of each processing cycle and on initialization.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \sa #CFE_TBL_Manage
**
*************************************************************************/
void HS_AcquirePointers(void);

/************************************************************************/
/** \brief Housekeeping request
**
**  \par Description
**       Processes an on-board housekeeping request message.
**
**  \par Assumptions, External Events, and Notes:
**       This message does not affect the command execution counter
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
*************************************************************************/
void HS_HousekeepingReq(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Noop command
**
**  \par Description
**       Processes a noop ground command.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_NOOP_CC
**
*************************************************************************/
void HS_NoopCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Reset counters command
**
**  \par Description
**       Processes a reset counters ground command which will reset
**       the following HS application counters to zero:
**         - Command counter
**         - Command error counter
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_RESET_CC
**
*************************************************************************/
void HS_ResetCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process an enable critical applications monitor command
**
**  \par Description
**       Allows the critical applications to be monitored.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_ENABLE_APPMON_CC
**
*************************************************************************/
void HS_EnableAppMonCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process a disable critical applications monitor command
**
**  \par Description
**       Stops the critical applications from be monitored.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_DISABLE_APPMON_CC
**
*************************************************************************/
void HS_DisableAppMonCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process an enable critical events monitor command
**
**  \par Description
**       Allows the critical events to be monitored.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_ENABLE_EVENTMON_CC
**
*************************************************************************/
void HS_EnableEventMonCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process a disable critical events monitor command
**
**  \par Description
**       Stops the critical events from be monitored.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_DISABLE_EVENTMON_CC
**
*************************************************************************/
void HS_DisableEventMonCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process an enable aliveness indicator command
**
**  \par Description
**       Allows the aliveness indicator to be output to the UART.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_ENABLE_ALIVENESS_CC
**
*************************************************************************/
void HS_EnableAlivenessCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process a disable aliveness indicator command
**
**  \par Description
**       Stops the aliveness indicator from being output on the UART.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_DISABLE_ALIVENESS_CC
**
*************************************************************************/
void HS_DisableAlivenessCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process an enable CPU Hogging indicator command
**
**  \par Description
**       Allows the CPU Hogging indicator to be output as an event.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_ENABLE_CPUHOG_CC
**
*************************************************************************/
void HS_EnableCPUHogCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process a disable CPU Hogging indicator command
**
**  \par Description
**       Stops the CPU Hogging indicator from being output as an event.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_DISABLE_CPUHOG_CC
**
*************************************************************************/
void HS_DisableCPUHogCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process a reset resets performed command
**
**  \par Description
**       Resets the count of HS performed resets maintained by HS.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_SET_MAX_RESETS_CC
**
*************************************************************************/
void HS_ResetResetsPerformedCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Process a set max resets command
**
**  \par Description
**       Sets the max number of HS performed resets to the specified value.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   MessagePtr   A #CFE_SB_MsgPtr_t pointer that
**                             references the software bus message
**
**  \sa #HS_RESET_RESETS_PERFORMED_CC
**
*************************************************************************/
void HS_SetMaxResetsCmd(CFE_SB_MsgPtr_t MessagePtr);

/************************************************************************/
/** \brief Refresh Critical Applications Monitor Status
**
**  \par Description
**       This function gets called when HS detects that a new critical
**       applications monitor table has been loaded or when a command
**       to enable the critical applications monitor is received: it then
**       refreshes the timeouts for application being monitored
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void HS_AppMonStatusRefresh(void);

/************************************************************************/
/** \brief Refresh Message Actions Status
**
**  \par Description
**       This function gets called when HS detects that a new
**       message actions table has been loaded: it then
**       resets the cooldowns for all actions.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
*************************************************************************/
void HS_MsgActsStatusRefresh(void);

#endif /* _hs_cmds_h_ */

/************************/
/*  End of File Comment */
/************************/
