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
 *   Specification for the CFS Health and Safety (HS) table structures
 *
 * Provides default definitions for HK table structures
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef HS_TBLSTRUCT_H
#define HS_TBLSTRUCT_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "hs_tbldefs.h"
#include "hs_mission_cfg.h"
#include "cfe_mission_cfg.h"
#include "cfe_sb_api_typedefs.h"

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/*************************************************************************
 * Type Definitions
 *************************************************************************/

/*
** In the following definitions, NullTerm may have a differing size for alignment purposes
** specifically it must be 32 bits in the XCT to align Resource Type, while it can be 16 bits
** in the other two.
*/

/**
 *  \brief Application Monitor Table (AMT) Entry
 */
typedef struct
{
    char   AppName[CFE_MISSION_MAX_API_LEN]; /**< \brief Name of application to be monitored */
    uint16 NullTerm;                         /**< \brief Buffer of nulls to terminate string */
    uint16 CycleCount;                       /**< \brief Number of cycles before application is missing */
    uint16 ActionType;                       /**< \brief Action to take if application is missing */
} HS_AMTEntry_t;

/**
 *  \brief Event Monitor Table (EMT) Entry
 */
typedef struct
{
    char   AppName[CFE_MISSION_MAX_API_LEN]; /**< \brief Name of application generating event */
    uint16 NullTerm;                         /**< \brief Buffer of nulls to terminate string */
    uint16 EventID;                          /**< \brief Event number of monitored event */
    uint16 ActionType;                       /**< \brief Action to take if event is received */
} HS_EMTEntry_t;

/**
 *  \brief Execution Counters Table (XCT) Entry
 */
typedef struct
{
    char   ResourceName[CFE_MISSION_MAX_API_LEN]; /**< \brief Name of resource being monitored */
    uint32 NullTerm;                              /**< \brief Buffer of nulls to terminate string */
    uint32 ResourceType;                          /**< \brief Type of execution counter */
} HS_XCTEntry_t;

/**
 *  \brief Message Action Table buffer
 */
typedef union
{
    uint8           Message[HS_MAX_MSG_ACT_SIZE]; /**< \brief Raw message array for sizing */
    CFE_SB_Buffer_t Buffer;                       /**< \brief Message Buffer for alignment */
} HS_MATMsgBuf_t;

/**
 *  \brief Message Actions Table (MAT) Entry
 */
typedef struct
{
    uint16         EnableState; /**< \brief If entry contains message */
    uint16         Cooldown;    /**< \brief Minimum rate at which message can be sent */
    HS_MATMsgBuf_t MsgBuf;      /**< \brief Message to be sent */
} HS_MATEntry_t;

#endif
