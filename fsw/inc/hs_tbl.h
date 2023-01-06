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
 * @note
 *   Constants and enumerated types related to these table structures
 *   are defined in hs_tbldefs.h.
 */
#ifndef HS_TBL_H
#define HS_TBL_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include <cfe.h>
#include <hs_tbldefs.h>
#include <hs_platform_cfg.h>

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/**
 * \name Macros for Action Type numbers of Message Actions
 * \{
 */
#define HS_AMT_ACT_MSG(num) (HS_AMT_ACT_LAST_NONMSG + 1 + (num))
#define HS_EMT_ACT_MSG(num) (HS_EMT_ACT_LAST_NONMSG + 1 + (num))
/**\}*/

#endif
