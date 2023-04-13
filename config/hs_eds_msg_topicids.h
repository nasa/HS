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
 *   CFS Health and Safety (HS) Application Message IDs
 */
#ifndef HS_MSGIDS_H
#define HS_MSGIDS_H

#include "cfe_msgids.h"

/**
 * \defgroup cfshscmdmid CFS Health and Safety Command Message IDs
 * \{
 */

/** \brief Msg ID for cmds to HS */
#define HS_CMD_MID CFE_PLATFORM_CMD_TOPICID_TO_MID(CFE_MISSION_HS_CMD_TOPICID)

/** \brief Msg ID to request HS housekeeping */
#define HS_SEND_HK_MID CFE_PLATFORM_CMD_TOPICID_TO_MID(CFE_MISSION_HS_SEND_HK_TOPICID)

/** \brief Msg ID to wake up HS */
#define HS_WAKEUP_MID CFE_PLATFORM_CMD_TOPICID_TO_MID(CFE_MISSION_HS_WAKEUP_TOPICID)

/**\}*/

/**
 * \defgroup cfshstlmmid CFS Health and Safety Telemetry Message IDs
 * \{
 */

/** \brief HS Housekeeping Telemetry */
#define HS_HK_TLM_MID CFE_PLATFORM_TLM_TOPICID_TO_MID(CFE_MISSION_HS_HK_TLM_TOPICID)

/**\}*/

#endif
