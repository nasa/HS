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
 *
 * CFS Health and Safety (HS) Application Mission Configuration Header File
 *
 * This is a compatibility header for the "mission_cfg.h" file that has
 * traditionally provided public config definitions for each CFS app.
 *
 * @note This file may be overridden/superceded by mission-provided definitions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef HS_MISSION_CFG_H
#define HS_MISSION_CFG_H

#include "hs_interface_cfg.h"

/**
 * \brief Application Name
 *
 *  \par Description:
 *       This definition must match the name used at startup by the cFE
 *       Executive Services when creating the HS application.  Note that
 *       application names are also an argument to certain cFE commands.
 *       For example, the application name is needed to access tables
 *       via cFE Table Services commands.
 *
 *  \par Limits:
 *       HS requires that this name be defined, but otherwise places
 *       no limits on the definition.  Refer to CFE Executive Services
 *       for specific information on limits related to application names.
 */
#define HS_APP_NAME "HS"

#endif
