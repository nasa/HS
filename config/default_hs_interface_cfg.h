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
 *   CFS Health and Safety (HS) Application Public Definitions
 *
 * This provides default values for configurable items that affect
 * the interface(s) of this module.  This includes the CMD/TLM message
 * interface, tables definitions, and any other data products that
 * serve to exchange information with other entities.
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef HS_INTERFACE_CFG_H
#define HS_INTERFACE_CFG_H

/**
 * \defgroup cfshsplatformcfg CFS Health and Safety Platform Configuration
 * \{
 */

/**
 * \brief Maximum reported execution counters
 *
 *  \par Description:
 *       Maximum number of execution counters that can be
 *       specified to be reported in telemetry.
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 *
 *       This parameter will dictate the size of the Execution
 *       Counter Table (XCT):
 *
 *       XCT Size = HS_MAX_EXEC_CNT_SLOTS * sizeof(#HS_XCTEntry_t)
 *
 *       The total size of this table should not exceed the
 *       cFE size limit for a single buffered table set by the
 *       #CFE_PLATFORM_TBL_MAX_SNGL_TABLE_SIZE parameter
 */
#define HS_MAX_EXEC_CNT_SLOTS 32

/**
 * \brief Maximum message action types
 *
 *  \par Description:
 *       Maximum number of Message Action action types.
 *
 *  \par Limits:
 *       This parameter can't be larger than 4 less than an
 *       unsigned 16 bit integer (65531).
 *
 *       This parameter must be greater than 0.
 *
 *       This parameter will influence the size of the Message
 *       Action Table (MAT):
 *
 *       MAT Size = HS_MAX_MSG_ACT_TYPES * (HS_MAX_MSG_ACT_SIZE + 4)
 *
 *       The total size of this table should not exceed the
 *       cFE size limit for a single buffered table set by the
 *       #CFE_PLATFORM_TBL_MAX_SNGL_TABLE_SIZE parameter
 */
#define HS_MAX_MSG_ACT_TYPES 8

/**
 * \brief Maximum message action size (in bytes)
 *
 *  \par Description:
 *       Size in bytes of maximum length of software bus message that
 *       can be sent using a Message Action action type.
 *
 *  \par Limits:
 *       This parameter can't be larger than #CFE_MISSION_SB_MAX_SB_MSG_SIZE
 *
 *       This parameter can't be smaller than a packet header
 *
 *       This parameter will influence the size of the Message
 *       Action Table (MAT):
 *
 *       MAT Size = HS_MAX_MSG_ACT_TYPES * (HS_MAX_MSG_ACT_SIZE + 4)
 *
 *       The total size of this table should not exceed the
 *       cFE size limit for a single buffered table set by the
 *       #CFE_PLATFORM_TBL_MAX_SNGL_TABLE_SIZE parameter
 */
#define HS_MAX_MSG_ACT_SIZE 16

/**
 * \brief Maximum number of monitored applications
 *
 *  \par Description:
 *       Maximum number of applications that can be
 *       monitored to assure check-ins
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 *
 *       This parameter must be greater than 0.
 *
 *       This parameter will dictate the size of the Application
 *       Monitor Table (AMT):
 *
 *       AMT Size = HS_MAX_MONITORED_APPS * sizeof(#HS_AMTEntry_t)
 *
 *       The total size of this table should not exceed the
 *       cFE size limit for a single buffered table set by the
 *       #CFE_PLATFORM_TBL_MAX_SNGL_TABLE_SIZE parameter
 */
#define HS_MAX_MONITORED_APPS 32

/**
 * \brief Maximum number of monitored events
 *
 *  \par Description:
 *       Maximum number of events that can be
 *       monitored
 *
 *  \par Limits:
 *       This parameter can't be larger than an unsigned 32 bit
 *       integer (4294967295).
 *
 *       This parameter must be greater than 0.
 *
 *       This parameter will dictate the size of the Event
 *       Monitor Table (EMT):
 *
 *       EMT Size = HS_MAX_MONITORED_EVENTS * sizeof(#HS_EMTEntry_t)
 *
 *       The total size of this table should not exceed the
 *       cFE size limit for a single buffered table set by the
 *       #CFE_PLATFORM_TBL_MAX_SNGL_TABLE_SIZE parameter
 */
#define HS_MAX_MONITORED_EVENTS 16

/**\}*/

#endif
