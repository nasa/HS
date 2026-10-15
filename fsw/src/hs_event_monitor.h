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

/**
 * @file
 *   Specification for the CFS Health and Safety (HS) routines that
 *   handle event monitoring
 */
#ifndef HS_EVENT_MONITOR_H
#define HS_EVENT_MONITOR_H

#include "common_types.h"
#include "hs_tbl.h"
#include "cfe_evs_msg.h"
#include "cfe_es_extern_typedefs.h"

/**
 * \brief Run-time State information for event monitor
 *
 * This is paired with a configuration entry from the EM Table
 */
typedef struct
{
    bool           Enable; /**< Whether the corresponding config entry is valid */
    CFE_ES_AppId_t AppId;  /**< Cache of the CFE ES AppId corresponding to this name */
} HS_EventMon_State_t;

/**
 * \brief Check Event Monitor Configuration Entries
 *
 *  \par Description
 *       Checks the validity of the entries in the event monitor
 *       configuration table.  Specifically, this includes whether
 *       the app names in the configuration entries correlate to
 *       running apps as tracked by CFE ES.  For each entry that
 *       has a valid configuration, the cached AppId will be
 *       [re]validated and/or updated as needed to match the current
 *       state of the system.
 *
 *       This updates the InactiveEventMonCount global with a count
 *       of entries that do _not_ map to a running app, as this is
 *       reported in the housekeeping telemetry.
 *
 *  \par Assumptions, External Events, and Notes:
 *       An entry that does _not_ map to a running application is
 *       not necessarily a configuration error, as the app simply
 *       may be stopped for other reasons.  Since apps can be started
 *       or stopped at runtime via commands, this function must be
 *       called repeatedly in a polling fashion, regardless of
 *       whether the configuration has changed, to keep information
 *       correct.
 *
 */
void HS_EventMon_CheckEntries(void);

/**
 * \brief Refresh Event Monitor Status
 *
 *  \par Description
 *       This function gets called when HS detects that a new
 *       event monitor table has been loaded: it then
 *       resets the status for all monitors.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_EventMon_StatusRefresh(void);

/**
 * \brief Handle Event Monitor Action
 *
 *  \par Description
 *       This is invoked when an event monitor is activated
 *       by receiving a matching event message.  In turn, this
 *       invokes the specified handling method, which may be
 *       to restart the app or send another message.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 * \param[in] EMEntryPtr  Matched event configuration
 */
void HS_EventMon_TriggerAction(const HS_EMTEntry_t *EMEntryPtr);

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
void HS_EventMon_AcquirePointers(void);

/**
 * \brief Search the EventMon table for matches to the incoming event
 *
 *  \par Description
 *       Searches the Event Monitor Table for matches to the incoming
 *       event message. If a match is found, the table specified action is
 *       taken.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] EventPtr  Pointer to the event message
 */
void HS_EventMon_Check(const CFE_EVS_LongEventTlm_t *EventPtr);

/**
 * \brief Validate event monitor table
 *
 *  \par Description
 *       This function is called by table services when a validation of
 *       the event monitor table is required
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   *TableData     Pointer to the table data to validate
 *
 *  \return Table validation status
 *  \retval #CFE_SUCCESS       \copydoc CFE_SUCCESS
 *  \retval #HS_EMTVal_ERR_ACT \copydoc HS_EMTVal_ERR_ACT
 *  \retval #HS_EMTVal_ERR_NUL \copydoc HS_EMTVal_ERR_NUL
 *
 *  \sa #HS_AppMon_ValidateTable, #HS_ExecMon_ValidateTable, #HS_MsgAct_ValidateTable
 */
int32 HS_EventMon_ValidateTable(void *TableData);

#endif
