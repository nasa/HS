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
 *   handle task execution, generic counter, and CPU monitoring
 */
#ifndef HS_EXEC_MONITOR_H
#define HS_EXEC_MONITOR_H

#include "common_types.h"
#include "cfe_resourceid_api_typedefs.h"

/**
 * \brief Run-time State information for execution count monitor
 *
 * This is paired with a configuration entry from the XC Table
 *
 * The ResourceId might represent a task or counter (or something else
 * if the scope is expanded in the future).  The configuration table
 * entry must be checked to determine which resource type this is
 * monitoring.
 */
typedef struct
{
    bool             Enable;     /**< Whether the corresponding config entry is valid */
    CFE_ResourceId_t ResourceId; /**< Cache of the resource ID corresponding to this name */
    uint32           LastSample; /**< Cache of the most recent value from reading the counter */
} HS_ExecMon_State_t;

/*************************************************************************
 * Includes
 *************************************************************************/

/**
 * \brief Check and Update Execution Monitor Entries
 *
 *  \par Description
 *       This checks all configured items in the execution monitor (XC)
 *       table and updates the counts accordingly.  The latest values are
 *       cached in the corresponding State entry.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_ExecMon_CheckEntries(void);

/**
 * \brief Read values to populate the Housekeeping TLM data
 *
 *  \par Description
 *       Copies latest counters from internal cache into the HK
 *       TLM data structure.  This is intended to be an array of
 *       32-bit values.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_ExecMon_GetCounters(uint32 *ExeCounts);

/**
 * \brief Refresh Execution Count Status
 *
 *  \par Description
 *       This function gets called when HS detects that a new
 *       exec count table has been loaded: it then looks up the
 *       IDs and resets the state for all entries.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_ExecMon_StatusRefresh(void);

/**
 * \brief Manages HS tables
 *
 *  \par Description
 *       Manages load requests for the ExeCount table.
 *       Also releases and acquires table addresses. Gets called at the start
 *       of each processing cycle and on initialization.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \sa #CFE_TBL_Manage
 */
void HS_ExecMon_AcquirePointers(void);

/**
 * \brief Monitor the utilization tracker counter
 *
 *  \par Description
 *       Monitors the utilization tracker counter incremented by the Idle
 *       Task, converting it into an estimated CPU Utilization for the
 *       previous cycle. If the utilization is over a certain theshold
 *       for a certain amount of time, an event is output.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_ExecMon_CheckUtilization(void);

/**
 * \brief Validate execution counter table
 *
 *  \par Description
 *       This function is called by table services when a validation of
 *       the execution counter table is required
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   *TableData     Pointer to the table data to validate
 *
 *  \return Table validation status
 *  \retval #CFE_SUCCESS        \copydoc CFE_SUCCESS
 *  \retval #HS_XCTVal_ERR_TYPE \copydoc HS_XCTVal_ERR_TYPE
 *  \retval #HS_XCTVal_ERR_NUL  \copydoc HS_XCTVal_ERR_NUL
 *
 *  \sa #HS_AppMon_ValidateTable, #HS_EventMon_ValidateTable, #HS_MsgAct_ValidateTable
 */
int32 HS_ExecMon_ValidateTable(void *TableData);

#endif
