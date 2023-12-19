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
 *   Specification for the CFS Health and Safety (HS) routines that
 *   handle command processing
 */
#ifndef HS_CMDS_H
#define HS_CMDS_H

/*************************************************************************
 * Includes
 ************************************************************************/
#include "cfe_error.h"
#include "hs_msg.h"

/*************************************************************************
 * Exported Functions
 ************************************************************************/

/**
 * \brief Reset counters
 *
 *  \par Description
 *       Utility function that resets housekeeping counters to zero
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \sa #HS_ResetCmd
 */
void HS_ResetCounters(void);

/**
 * \brief Manages HS tables
 *
 *  \par Description
 *       Manages load requests for the AppMon, EventMon, ExeCount and MsgActs
 *       tables and update notification for the AppMon and MsgActs tables.
 *       Also releases and acquires table addresses. Gets called at the start
 *       of each processing cycle and on initialization.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \sa #CFE_TBL_Manage
 */
void HS_AcquirePointers(void);

/**
 * \brief Housekeeping request
 *
 *  \par Description
 *       Processes an on-board housekeeping request message.
 *
 *  \par Assumptions, External Events, and Notes:
 *       This message does not affect the command execution counter
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 */
CFE_Status_t HS_SendHkCmd(const HS_SendHkCmd_t *BufPtr);

/**
 * \brief Noop command
 *
 *  \par Description
 *       Processes a noop ground command.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_NOOP_CC
 */
CFE_Status_t HS_NoopCmd(const HS_NoopCmd_t *BufPtr);

/**
 * \brief Reset counters command
 *
 *  \par Description
 *       Processes a reset counters ground command which will reset
 *       the following HS application counters to zero:
 *         - Command counter
 *         - Command error counter
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_RESET_CC
 */
CFE_Status_t HS_ResetCmd(const HS_ResetCmd_t *BufPtr);

/**
 * \brief Process an enable critical applications monitor command
 *
 *  \par Description
 *       Allows the critical applications to be monitored.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_ENABLE_APP_MON_CC
 */
CFE_Status_t HS_EnableAppMonCmd(const HS_EnableAppMonCmd_t *BufPtr);

/**
 * \brief Process a disable critical applications monitor command
 *
 *  \par Description
 *       Stops the critical applications from be monitored.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_DISABLE_APP_MON_CC
 */
CFE_Status_t HS_DisableAppMonCmd(const HS_DisableAppMonCmd_t *BufPtr);

/**
 * \brief Process an enable critical events monitor command
 *
 *  \par Description
 *       Allows the critical events to be monitored.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_ENABLE_EVENT_MON_CC
 */
CFE_Status_t HS_EnableEventMonCmd(const HS_EnableEventMonCmd_t *BufPtr);

/**
 * \brief Process a disable critical events monitor command
 *
 *  \par Description
 *       Stops the critical events from be monitored.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_DISABLE_EVENT_MON_CC
 */
CFE_Status_t HS_DisableEventMonCmd(const HS_DisableEventMonCmd_t *BufPtr);

/**
 * \brief Process an enable aliveness indicator command
 *
 *  \par Description
 *       Allows the aliveness indicator to be output to the UART.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_ENABLE_ALIVENESS_CC
 */
CFE_Status_t HS_EnableAlivenessCmd(const HS_EnableAlivenessCmd_t *BufPtr);

/**
 * \brief Process a disable aliveness indicator command
 *
 *  \par Description
 *       Stops the aliveness indicator from being output on the UART.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_DISABLE_ALIVENESS_CC
 */
CFE_Status_t HS_DisableAlivenessCmd(const HS_DisableAlivenessCmd_t *BufPtr);

/**
 * \brief Process an enable CPU Hogging indicator command
 *
 *  \par Description
 *       Allows the CPU Hogging indicator to be output as an event.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_ENABLE_CPU_HOG_CC
 */
CFE_Status_t HS_EnableCpuHogCmd(const HS_EnableCpuHogCmd_t *BufPtr);

/**
 * \brief Process a disable CPU Hogging indicator command
 *
 *  \par Description
 *       Stops the CPU Hogging indicator from being output as an event.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_DISABLE_CPU_HOG_CC
 */
CFE_Status_t HS_DisableCpuHogCmd(const HS_DisableCpuHogCmd_t *BufPtr);

/**
 * \brief Process a reset resets performed command
 *
 *  \par Description
 *       Resets the count of HS performed resets maintained by HS.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_SET_MAX_RESETS_CC
 */
CFE_Status_t HS_ResetResetsPerformedCmd(const HS_ResetResetsPerformedCmd_t *BufPtr);

/**
 * \brief Process a set max resets command
 *
 *  \par Description
 *       Sets the max number of HS performed resets to the specified value.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \sa #HS_RESET_RESETS_PERFORMED_CC
 */
CFE_Status_t HS_SetMaxResetsCmd(const HS_SetMaxResetsCmd_t *BufPtr);

/**
 * \brief Refresh Critical Applications Monitor Status
 *
 *  \par Description
 *       This function gets called when HS detects that a new critical
 *       applications monitor table has been loaded or when a command
 *       to enable the critical applications monitor is received: it then
 *       refreshes the timeouts for application being monitored
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_AppMonStatusRefresh(void);

/**
 * \brief Refresh Message Actions Status
 *
 *  \par Description
 *       This function gets called when HS detects that a new
 *       message actions table has been loaded: it then
 *       resets the cooldowns for all actions.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_MsgActsStatusRefresh(void);

#endif
