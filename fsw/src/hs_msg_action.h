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
 *   handle sending of messages after monitored conditions are triggered.
 *
 *   This specificially deals with the management of the Message Action
 *   table which is shared by App Mon and Event Mon entries that are configured
 *   to send a software bus message when a matching condition is detected.
 */
#ifndef HS_MSG_ACTION_H
#define HS_MSG_ACTION_H

#include "common_types.h"

/**
 * \brief Run-time State information for message action entry
 *
 * This is paired with a configuration entry from the MA Table
 *
 * In order to prevent "spamming" of messages for conditions, this
 * implements a cooldown delay before messages will be re-sent.
 */
typedef struct
{
    uint16 Cooldown; /**< \brief Counts until Message Actions is available */
} HS_MsgActState_t;

/**
 * Callback for HS_MsgAct_TriggerAction() function
 *
 * This is invoked when a message is sent via the message action handler
 *
 * \param Idx  table entry number (array index)
 * \param Arg  opaque argument from original caller
 */
typedef void (*HS_MsgAct_Callback_t)(uint32 Idx, const void *Arg);

/**
 * \brief Handle message action for triggered condition
 *
 *  \par Description
 *       This function gets called when HS detects an active
 *       condition that is configured for a message action.  This
 *       can be an App monitor, Event monitor, or any other monitor
 *       which allows messages to be generated when that monitor
 *       gets triggered.
 *
 *  \par Assumptions, External Events, and Notes:
 *       The ActionType parameter is expected to be 1-based, not 0-based.
 *       This is because the message actions start at one past
 *       the correspondig "last" marker, for example
 *       (HS_EMTActType_LAST_NONMSG + 1) refers to Message Action 0.
 *
 * \param ActionType   The 1-based message action value
 * \param SendEventCb  Callback routine to invoke after sending the message
 * \param CbArg        Opaque argument passed to callback
 */
void HS_MsgAct_TriggerAction(uint16 ActionType, HS_MsgAct_Callback_t SendEventCb, const void *CbArg);

/**
 * \brief Validate message action table
 *
 *  \par Description
 *       This function is called by table services when a validation of
 *       the message action table is required
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   *TableData     Pointer to the table data to validate
 *
 *  \return Table validation status
 *  \retval #CFE_SUCCESS        \copydoc CFE_SUCCESS
 *  \retval #HS_MATVal_ERR_LEN  \copydoc HS_MATVal_ERR_LEN
 *  \retval #HS_MATVal_ERR_ENA  \copydoc HS_MATVal_ERR_ENA
 *
 *  \sa #HS_AppMon_ValidateTable, #HS_EventMon_ValidateTable, #HS_ExecMon_ValidateTable
 */
int32 HS_MsgAct_ValidateTable(void *TableData);

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
void HS_MsgAct_StatusRefresh(void);

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
void HS_MsgAct_AcquirePointers(void);

/**
 * \brief Process cooldown timers for all message actions
 *
 *  \par Description
 *       This is invoked on a periodic basis to implement the
 *       cooldown delays.  When a message action is triggered,
 *       the cooldown will be set to the configured value.  The
 *       message will not be sent again until the cooldown timer
 *       reaches 0.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \sa #CFE_TBL_Manage
 */
void HS_MsgAct_Cooldown(void);

#endif
