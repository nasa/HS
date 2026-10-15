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
 *   Specification for the CFS Health and Safety (HS) command and telemetry
 *   message constant definitions.
 *
 *  For HS this is only the function/command code definitions
 */
#ifndef DEFAULT_HS_MSGDEFS_H
#define DEFAULT_HS_MSGDEFS_H

#include "common_types.h"
#include "hs_interface_cfg.h"

/**
 *  \brief Set Max Resets Payload
 */
typedef struct
{
    uint16 MaxResets; /**< \brief Maximum Resets */
    uint16 Padding;   /**< \brief Structure padding */
} HS_SetMaxResets_Payload_t;

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
    uint32 InactiveEventMonCount; /**< \brief Total count of Invalid Event Monitors */

    uint8 AppMonEnables[(HS_MAX_MONITORED_APPS + 7) / 8];
    /**< \brief Enable states of App Monitor Entries */

    uint32 MsgActExec;  /**< \brief Number of Software Bus Message Actions Executed */
    uint32 UtilCpuAvg;  /**< \brief Current CPU Utilization Average */
    uint32 UtilCpuPeak; /**< \brief Current CPU Utilization Peak */

    uint32 ExeCounts[HS_MAX_EXEC_CNT_SLOTS]; /**< \brief Execution Counters */
} HS_HkTlm_Payload_t;

/**
 * Macro to set single enable bit in the standard TLM data structure
 *
 * Each entry is a single bit so it needs to be set via a bitmask
 * The left-most bit should be the first entry (index 0)
 */
#define HS_SET_TLM_ENABLE_BITMASK(arr, p)                 \
    do                                                    \
    {                                                     \
        ((uint8 *)(arr))[(p) / 8] |= (0x80 >> ((p) % 8)); \
    } while (0)

#endif
