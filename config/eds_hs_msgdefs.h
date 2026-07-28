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
#ifndef EDS_HS_MSGDEFS_H
#define EDS_HS_MSGDEFS_H

#include "hs_eds_typedefs.h"
#include "hs_fcncodes.h"

/**
 * Macro to set single enable bit in the EDS data structure
 *
 * In EDS each entry gets its own bool so access is very simple
 */
#define HS_SET_TLM_ENABLE_BITMASK(arr, p) \
    do                                    \
    {                                     \
        arr[p] = true;                    \
    } while (0)

#endif