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
 *   custom function private interface
 */
#ifndef HS_CUSTOM_DISPATCH_H
#define HS_CUSTOM_DISPATCH_H

/*************************************************************************
 * Includes
 ************************************************************************/
#include "cfe.h"
#include "hs_platform_cfg.h"
#include "hs_custom_internal.h"

/*************************************************************************
 * Exported Functions
 *************************************************************************/

/**
 * \brief Process Custom Commands
 *
 *  \par Description
 *       This function allows for hs_custom.c to define custom commands.
 *       It will be called for any command code not already allocated
 *       to a Health and Safety command. If a custom command is found,
 *       then it is responsible for incrementing the command processed
 *       or command error counter as appropriate.
 *
 *  \par Assumptions, External Events, and Notes:
 *       If a command is found, this function MUST return #CFE_SUCCESS,
 *       otherwise is must not return #CFE_SUCCESS
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
int32 HS_CustomCommands(const CFE_SB_Buffer_t *BufPtr);

#endif
