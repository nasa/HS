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
 *   handle application monitoring
 */
#ifndef HS_APP_MONITOR_H
#define HS_APP_MONITOR_H

#include "common_types.h"
#include "cfe_es_extern_typedefs.h"
#include "hs_tbl.h"

/**
 * \brief Run-time State information for app monitor
 *
 * This is paired with a configuration entry from the AM Table
 */
typedef struct
{
    bool           Enable;           /**< Whether the corresponding config entry is valid */
    CFE_ES_AppId_t AppId;            /**< Cache of the CFE ES AppId corresponding to this name */
    uint16         CheckInCountdown; /**< Counts until Application Monitor times out */
    uint32         LastExeCount;     /**< Last Execution Count for application being checked */
} HS_AppMon_State_t;

/*************************************************************************
 * Exported Functions
 *************************************************************************/

/**
 * \brief Check execution status of each app in AppMon table
 *
 *  \par Description
 *       Cycles through the Application Monitor Table checking the current
 *       execution count for each monitored application. If the count fails
 *       to increment for the table specified duration, the table specified
 *       action is taken.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_AppMon_CheckAllApps(void);

/**
 * \brief Validate application monitor table
 *
 *  \par Description
 *       This function is called by table services when a validation of
 *       the application monitor table is required
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in]   TableData     Pointer to the table data to validate
 *
 *  \return Table validation status
 *  \retval #CFE_SUCCESS       \copydoc CFE_SUCCESS
 *  \retval #HS_AMTVal_ERR_ACT \copydoc HS_AMTVal_ERR_ACT
 *  \retval #HS_AMTVal_ERR_NUL \copydoc HS_AMTVal_ERR_NUL
 *
 *  \sa #HS_EventMon_ValidateTable, #HS_ExecMon_ValidateTable, #HS_MsgAct_ValidateTable
 */
int32 HS_AppMon_ValidateTable(void *TableData);

/**
 * \brief Compute the appmon enable bitfield for HS telemtry
 *
 *  \par Description
 *       The housekeeping telemetry contains a bitfield to indicate which
 *       of the application monitor entries are enabled.  This function sets
 *       bits in the bitfield corresponding to the current state.
 *
 *  \par Assumptions, External Events, and Notes:
 *       This only sets bits as "true" - it should be memset to 0 before calling
 *       The definition of this bitfield is also configuration dependent, which
 *       is why the pointer is passed as void.
 *
 *  \param[in]   AppMonEnableBits     Pointer to the HK TLM bitfield
 */
void HS_AppMon_ComputeEnableBits(void *AppMonEnableBits);

/**
 * \brief Process a single app monitor entry
 *
 *  \par Description
 *       Queries the state of the application referred to by the given table entry
 *       The state will be updated accordingly.  If the application is not active,
 *       this will trigger the handling routine.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in]    AMEntryPtr   Pointer to app mon config table entry
 *  \param[inout] AMStatePtr   Pointer to app mon state entry
 */
void HS_AppMon_CheckApp(const HS_AMTEntry_t *AMEntryPtr, HS_AppMon_State_t *AMStatePtr);

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
void HS_AppMon_StatusRefresh(void);

/**
 * \brief Manages HS Application Monitor tables
 *
 *  \par Description
 *       Manages load requests for the AppMon tables and update notification.
 *       Also releases and acquires table addresses. Gets called at the start
 *       of each processing cycle and on initialization.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \sa #CFE_TBL_Manage
 */
void HS_AppMon_AcquirePointers(void);

#endif
