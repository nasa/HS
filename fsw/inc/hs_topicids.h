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
 *   CFS Health and Safety (HS) Application Topic IDs
 */
#ifndef HS_TOPICIDS_H
#define HS_TOPICIDS_H

#include "hs_topicid_values.h"

#define CFE_MISSION_HS_CMD_TOPICID         CFE_MISSION_HS_TIDVAL(CMD)
#define DEFAULT_CFE_MISSION_HS_CMD_TOPICID 0xAE /**< \brief Msg ID for cmds to HS                */

#define CFE_MISSION_HS_SEND_HK_TOPICID         CFE_MISSION_HS_TIDVAL(SEND_HK)
#define DEFAULT_CFE_MISSION_HS_SEND_HK_TOPICID 0xAF /**< \brief Msg ID to request HS housekeeping    */

#define CFE_MISSION_HS_WAKEUP_TOPICID         CFE_MISSION_HS_TIDVAL(WAKEUP)
#define DEFAULT_CFE_MISSION_HS_WAKEUP_TOPICID 0xB0 /**< \brief Msg ID to wake up HS                 */

#define CFE_MISSION_HS_HK_TLM_TOPICID         CFE_MISSION_HS_TIDVAL(HK_TLM)
#define DEFAULT_CFE_MISSION_HS_HK_TLM_TOPICID 0xAD /**< \brief HS Housekeeping Telemetry            */

#endif
