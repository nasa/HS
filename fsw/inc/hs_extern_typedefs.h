/************************************************************************
 * NASA Docket No. GSC-18,447-1, and identified as “CFS CFDP (CF)
 * Application version 3.0.0”
 *
 * Copyright (c) 2019 United States Government as represented by the
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
 *
 * Declarations and prototypes for fm_extern_typedefs module
 */

#ifndef HS_EXTERN_TYPEDEFS_H
#define HS_EXTERN_TYPEDEFS_H

#include <cfe.h>
#include <hs_platform_cfg.h>

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
    char   AppName[OS_MAX_API_NAME]; /**< \brief Name of application to be monitored */
    uint16 NullTerm;                 /**< \brief Buffer of nulls to terminate string */
    uint16 CycleCount;               /**< \brief Number of cycles before application is missing */
    uint16 ActionType;               /**< \brief Action to take if application is missing */
} HS_AMTEntry_t;

/**
 *  \brief Event Monitor Table (EMT) Entry
 */
typedef struct
{
    char   AppName[OS_MAX_API_NAME]; /**< \brief Name of application generating event */
    uint16 NullTerm;                 /**< \brief Buffer of nulls to terminate string */
    uint16 EventID;                  /**< \brief Event number of monitored event */
    uint16 ActionType;               /**< \brief Action to take if event is received */
} HS_EMTEntry_t;

/**
 *  \brief Execution Counters Table (XCT) Entry
 */
typedef struct
{
    char   ResourceName[OS_MAX_API_NAME]; /**< \brief Name of resource being monitored */
    uint32 NullTerm;                      /**< \brief Buffer of nulls to terminate string */
    uint32 ResourceType;                  /**< \brief Type of execution counter */
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

#endif /* HS_EXTERN_TYPEDEFS_H */
