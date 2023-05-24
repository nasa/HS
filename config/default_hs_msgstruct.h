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
 *   Specification for the CFS Health and Safety (HS) command and telemetry
 *   message data types.
 *
 * @note
 *   Constants and enumerated types related to these message structures
 *   are defined in hs_msgdefs.h.
 */
#ifndef HS_MSGSTRUCT_H
#define HS_MSGSTRUCT_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "hs_msgdefs.h"
#include "hs_mission_cfg.h"
#include "cfe_msg_hdr.h"

/************************************************************************
 * Macro Definitions
 ************************************************************************/

#define HS_BITS_PER_APPMON_ENABLE 32 /**< \brief HS Bits per AppMon Enable entry */

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 * \defgroup cfshscmdstructs CFS Health and Safety Command Structures
 * \{
 */

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_NOOP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_NoopCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_RESET_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_ResetCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_ENABLE_APP_MON_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_EnableAppMonCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_DISABLE_APP_MON_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_DisableAppMonCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_ENABLE_EVENT_MON_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_EnableEventMonCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_DISABLE_EVENT_MON_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_DisableEventMonCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_ENABLE_ALIVENESS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_EnableAlivenessCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_DISABLE_ALIVENESS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_DisableAlivenessCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_RESET_RESETS_PERFORMED_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_ResetResetsPerformedCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_RESET_RESETS_PERFORMED_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_EnableCpuHogCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_RESET_RESETS_PERFORMED_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_DisableCpuHogCmd_t;

/**
 *  \brief Set Max Resets Payload
 */
typedef struct
{
    uint16 MaxResets; /**< \brief Maximum Resets */
    uint16 Padding;   /**< \brief Structure padding */
} HS_SetMaxResets_Payload_t;

/**
 *  \brief Set Max Resets Command
 *
 *  For command details see #HS_SET_MAX_RESETS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    HS_SetMaxResets_Payload_t Payload;
} HS_SetMaxResetsCmd_t;

/**
 *  \brief No Arguments Command
 *
 *  For command details see #HS_SEND_HK_MID
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} HS_SendHkCmd_t;

/**\}*/

/**
 * \defgroup cfshstlm CFS Health and Safety Telemetry
 * \{
 */

/**
 *  \brief Housekeeping Packet Payload
 */
typedef struct
{
    uint8  CmdCount;              /**< \brief HS Application Command Counter */
    uint8  CmdErrCount;           /**< \brief HS Application Command Error Counter */
    uint8  CurrentAppMonState;    /**< \brief Status of HS Application Monitor */
    uint8  CurrentEventMonState;  /**< \brief Status of HS Event Monitor */
    uint8  CurrentAlivenessState; /**< \brief Status of HS Aliveness Indicator */
    uint8  CurrentCPUHogState;    /**< \brief Status of HS Hogging Indicator */
    uint8  StatusFlags;           /**< \brief Internal HS Error States */
    uint8  SpareBytes;            /**< \brief Alignment Spares */
    uint16 ResetsPerformed;       /**< \brief HS Performed Processor Reset Count */
    uint16 MaxResets;             /**< \brief HS Maximum Processor Reset Count */
    uint32 EventsMonitoredCount;  /**< \brief Total count of Event Messages Monitored */
    uint32 InvalidEventMonCount;  /**< \brief Total count of Invalid Event Monitors */

    uint32 AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) + 1];
    /**< \brief Enable states of App Monitor Entries */

    uint32 MsgActExec;  /**< \brief Number of Software Bus Message Actions Executed */
    uint32 UtilCpuAvg;  /**< \brief Current CPU Utilization Average */
    uint32 UtilCpuPeak; /**< \brief Current CPU Utilization Peak */

    uint32 ExeCounts[HS_MAX_EXEC_CNT_SLOTS]; /**< \brief Execution Counters */
} HS_HkTlm_Payload_t;

/**
 *  \brief Housekeeping Packet Structure
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry Header */

    HS_HkTlm_Payload_t Payload;
} HS_HkPacket_t;

/**\}*/

#endif
