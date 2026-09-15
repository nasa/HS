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
 *   Specification for the CFS Health and Safety (HS) table related
 *   constant definitions.
 *
 * @note
 *   These Macro definitions have been put in this file (instead of
 *   hs_tbl.h). DO NOT PUT ANY TYPEDEFS OR
 *   STRUCTURE DEFINITIONS IN THIS FILE!
 *   ADD THEM TO hs_tbl.h IF NEEDED!
 */
#ifndef DEFAULT_HS_TBLDEFS_H
#define DEFAULT_HS_TBLDEFS_H

#include "common_types.h"

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/* TODO: These flags may not really belong here, but this gets things working */

/**
 * \name HS Switch States (AppMon, EventMon, Aliveness)
 * \{
 */
/**
 * @brief Label definitions associated with HS_State_t
 */
enum HS_State
{

    /**
     * @brief HS Switch States (AppMon, EventMon, Aliveness) Disabled
     */
    HS_State_DISABLED = 0,

    /**
     * @brief HS Switch States (AppMon, EventMon, Aliveness) Enabled
     */
    HS_State_ENABLED = 1
};

/**
 *
 * @sa enum HS_State
 */
typedef uint8 HS_State_Enum_t;

/**\}*/

/**
 * \name HS Internal Status Flags
 * \{
 */
#define HS_StatusFlag_LOADED_XCT 0x01
#define HS_StatusFlag_LOADED_MAT 0x02
#define HS_StatusFlag_LOADED_AMT 0x04
#define HS_StatusFlag_LOADED_EMT 0x08
#define HS_StatusFlag_CDS_IN_USE 0x10
/**\}*/

/**
 * \name Application Monitor Table (AMT) Action Types
 * \{
 */
#define HS_AMTActType_NOACT       0 /**< \brief No action is taken */
#define HS_AMTActType_PROC_RESET  1 /**< \brief Generates Processor Reset on failure */
#define HS_AMTActType_APP_RESTART 2 /**< \brief Attempts to restart application on failure */
#define HS_AMTActType_EVENT       3 /**< \brief Generates event message on failure */
#define HS_AMTActType_LAST_NONMSG 3 /**< \brief Index for finding end of non-message actions */
/**\}*/

/**
 * \name Event Monitor Table (EMT) Action Types
 * \{
 */
#define HS_EMTActType_NOACT       0 /**< \brief No action is taken */
#define HS_EMTActType_PROC_RESET  1 /**< \brief Generates Processor Reset on detection */
#define HS_EMTActType_APP_RESTART 2 /**< \brief Attempts to restart application on detection */
#define HS_EMTActType_APP_DELETE  3 /**< \brief Deletes application on detection */
#define HS_EMTActType_LAST_NONMSG 3 /**< \brief Index for finding end of non-message actions */
/**\}*/

/**
 * \name Execution Counters Table (XCT) Resource Types
 * \{
 */
#define HS_XCTResType_NOTYPE    0 /**< \brief No type */
#define HS_XCTResType_APP_MAIN  1 /**< \brief Counter for Application Main task */
#define HS_XCTResType_APP_CHILD 2 /**< \brief Counter for Application Child task */
#define HS_XCTResType_DEVICE    3 /**< \brief Counter for Device Driver */
#define HS_XCTResType_ISR       4 /**< \brief Counter for Interrupt Service Routine */
/**\}*/

/**
 * \name Message Actions Table (MAT) Enable State
 * \{
 */
#define HS_MATState_DISABLED 0 /**< \brief Message Actions are Disabled */
#define HS_MATState_ENABLED  1 /**< \brief Message Actions are Enabled  */
#define HS_MATState_NOEVENT  2 /**< \brief Message Actions are Enabled but produce no events */
/**\}*/

/**
 * \name Application Monitor Table (AMT) Validation Error Enumerated Types
 * \{
 */
#define HS_AMTVal_NO_ERR  0  /**< \brief No error                          */
#define HS_AMTVal_ERR_ACT -1 /**< \brief Invalid ActionType specified      */
#define HS_AMTVal_ERR_NUL -2 /**< \brief Null Safety Buffer not Null       */
/**\}*/

/**
 * \name Event Monitor Table (EMT) Validation Error Enumerated Types
 * \{
 */
#define HS_EMTVal_NO_ERR  0  /**< \brief No error                          */
#define HS_EMTVal_ERR_ACT -1 /**< \brief Invalid ActionType specified      */
#define HS_EMTVal_ERR_NUL -2 /**< \brief Null Safety Buffer not Null       */
/**\}*/

/**
 * \name Event Counter Table (XCT) Validation Error Enumerated Types
 * \{
 */
#define HS_XCTVal_NO_ERR   0  /**< \brief No error                          */
#define HS_XCTVal_ERR_TYPE -1 /**< \brief Invalid Counter Type specified    */
#define HS_XCTVal_ERR_NUL  -2 /**< \brief Null Safety Buffer not Null       */
/**\}*/

/**
 * \name Message Actions Table (MAT) Validation Error Enumerated Types
 * \{
 */
#define HS_MATVal_NO_ERR  0  /**< \brief No error                          */
#define HS_MATVal_ERR_ID  -1 /**< \brief Invalid Message ID specified      */
#define HS_MATVal_ERR_LEN -2 /**< \brief Invalid Length specified          */
#define HS_MATVal_ERR_ENA -3 /**< \brief Invalid Enable State specified    */
/**\}*/

#endif
