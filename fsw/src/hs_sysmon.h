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
 *   Specification for the CFS Health and Safety (HS) system monitoring routines
 *
 * System health monitoring is inherently platform-specific and therefore should
 * be abstracted via the PSP.  This provides local access methods to obtain the
 * information from the PSP.
 */
#ifndef HS_SYSMON_H
#define HS_SYSMON_H

/*************************************************************************
 * Includes
 ************************************************************************/
#include "cfe.h"
#include "hs_platform_cfg.h"

/*************************************************************************
 * Exported Functions
 *************************************************************************/

/**
 * \brief Initialize things needed for system monitoring
 *
 *  \par Description
 *       This function is intended to set up everything necessary for
 *       System Monitoring at application startup.
 *
 *  \par Assumptions, External Events, and Notes:
 *       CFE_SUCCESS will be returned if all creation was performed
 *       properly.
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
CFE_Status_t HS_SysMonInit(void);

/**
 * \brief De-Initialize system monitoring
 *
 *  \par Description
 *       This function stops everything that was started during HS_SysMonInit()
 *
 *  \par Assumptions, External Events, and Notes:
 *
 */
void HS_SysMonCleanup(void);

/**
 * \brief Read the system monitor CPU utilization
 *
 *  \par Description
 *       This function is used to read the current value of the cpu utilization
 *
 *  \par Assumptions, External Events, and Notes:
 *       The range of output is controlled by the #HS_CPU_UTILIZATION_MAX config
 *       macro.  Higher numbers represent higher usage, less idle time.
 *
 *  \return Utilization value as fixed-point integer
 */
CFE_Status_t HS_SysMonGetCpuUtilization(void);

#endif
